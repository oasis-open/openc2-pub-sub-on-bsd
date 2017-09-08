/*-
 * Copyright (c) 2016 G2, Inc
 * All rights reserved.
 *
 * Author: Shawn Webb <shawn.webb@g2-inc.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#include <ucl.h>

#include "edge.h"

int
read_config(const char *path, edge_context_t *ctx)
{
	struct ucl_parser *parser;
	const ucl_object_t *top, *obj, *section;
	ucl_object_iter_t iter;
	edge_plugin_t *pluginobj;
	edge_config_t *configobj;
	void *tmp;
	size_t i;
	int debugmode;

	iter = NULL;
	i = 0;
	debugmode = 0;

#ifdef DEBUG_MODE
	debugmode = 1;
#endif

	memset(ctx, 0x00, sizeof(edge_context_t));
	ctx->ecx_config_path = strdup(path);
	if (!(ctx->ecx_config_path)) {
		dbglog(ctx, "[-] %s: Could not copy config path\n", __func__);
		return (1);
	}

	ctx->ecx_config = calloc(1, sizeof(edge_config_t));
	if (!(ctx->ecx_config)) {
		dbglog(ctx, "[-] %s: Could not create new config object\n", __func__);
		goto error;
	}

	parser = ucl_parser_new(UCL_PARSER_KEY_LOWERCASE);
	if (!(parser)) {
		dbglog(ctx, "[-] %s: Could not create new parser\n", __func__);
		free(ctx->ecx_config_path);
		return (1);
	}

	if (ucl_parser_add_file(parser, path) == false) {
		dbglog(ctx, "[-] %s: Could not add config file to parser\n", __func__);
		goto error;
	}

	top = ucl_parser_get_object(parser);
	if (!(top)) {
		dbglog(ctx, "[-] %s: Could not get top object\n", __func__);
		goto error;
	}

	obj = ucl_lookup_path(top, "edge.debug.enable");
	if (obj || debugmode) {
		if (debugmode || ucl_object_toboolean(obj)) {
			ctx->ecx_config->ec_flags |= EDGE_FLAG_DEBUG;
		}

		obj = ucl_lookup_path(top, "edge.debug.logfile");
		if (obj) {
			ctx->ecx_config->ec_logfile =
			    ucl_object_tostring(obj);
		} else {
			ctx->ecx_config->ec_logfile =
			    "/tmp/openc2consumer.log";
		}
	}

	section = ucl_lookup_path(top, "edge.plugin");
	if (!(section)) {
		dbglog(ctx, "[-] Missing plugins\n");
		goto error;
	}

	while ((obj = ucl_iterate_object(section, &iter, false))) {
		pluginobj = parse_plugin_config(ctx, obj);
		if (!(pluginobj)) {
			dbglog(ctx, "[-] %s: Could not parse plugin\n", __func__);
			goto error;
		}

		tmp = realloc(ctx->ecx_plugins, sizeof(edge_plugin_t *) *
		    (ctx->ecx_nplugins + 1));

		if (!(tmp)) {
			dbglog(ctx, "[-] %s: realloc failed\n", __func__);
			goto error;
		}

		ctx->ecx_plugins = tmp;

		ctx->ecx_plugins[ctx->ecx_nplugins] = pluginobj;
		ctx->ecx_nplugins++;
	}

	obj = ucl_lookup_path(top, "edge.orchestrator.ip");
	if (!(obj)) {
		dbglog(ctx, "[-] %s: Could not look up orchestrator IP\n", __func__);
		goto error;
	}

	ctx->ecx_config->ec_orchestrator_ip =
	    ucl_object_tostring(obj);
	if (!(ctx->ecx_config->ec_orchestrator_ip)) {
		dbglog(ctx, "[-] %s: Could not read orchestrator IP\n", __func__);
		goto error;
	}

	obj = ucl_lookup_path(top, "edge.orchestrator.port");
	if (!(obj)) {
		dbglog(ctx, "[-] %s: Could not look up orchestrator port\n", __func__);
		goto error;
	}

	ctx->ecx_config->ec_orchestrator_port =
	    ucl_object_tostring_forced(obj);
	if (!(ctx->ecx_config->ec_orchestrator_port)) {
		dbglog(ctx, "[-] %s: Could not read orchestrator port\n", __func__);
		goto error;
	}

	obj = ucl_lookup_path(top, "edge.actuator.type");
	if (obj) {
		ctx->ecx_config->ec_actuator_type =
		    ucl_object_tostring(obj);
		if (!(ctx->ecx_config->ec_actuator_type)) {
			dbglog(ctx, "[-] %s: Could not read actuator type\n", __func__);
			goto error;
		}
	}

	obj = ucl_lookup_path(top, "edge.actuator.id");
	if (obj) {
		ctx->ecx_config->ec_actuator_id =
		    ucl_object_tostring(obj);
		if (!(ctx->ecx_config->ec_actuator_id)) {
			dbglog(ctx, "[-] %s: Could not read actuator id\n", __func__);
			goto error;
		}
	}

	ucl_parser_free(parser);

	return (0);

error:
	if (ctx->ecx_nplugins) {
		free_plugins(ctx);
	}

	if (ctx->ecx_config) {
		free(ctx->ecx_config);
	}

	free(ctx->ecx_config_path);
	ucl_parser_free(parser);

	return (1);
}

edge_plugin_t *
parse_plugin_config(edge_context_t *ctx, const ucl_object_t *top)
{
	const ucl_object_t *obj;
	edge_plugin_t *plugin;
	edge_plugin_ctx_t *plugin_ctx;

	plugin_ctx = NULL;

	plugin = calloc(1, sizeof(edge_plugin_t));
	if (!(plugin)) {
		return NULL;
	}

	obj = ucl_lookup_path(top, "path");
	plugin->ep_path = ucl_object_tostring(obj);
	if (!(plugin->ep_path)) {
		dbglog(ctx, "[-] No path for plugin specified\n");
		free(plugin);
		return (NULL);
	}

	plugin->ep_handle = dlopen(plugin->ep_path, RTLD_GLOBAL | RTLD_NOW);
	if (!(plugin->ep_handle)) {
		dbglog(ctx, "[-] Could not dlopen(%s): %s\n",
		    plugin->ep_path, dlerror());
		goto error;
	}

	plugin->ep_run = (edge_plugin_func_t)dlsym(plugin->ep_handle, "run");
	if (!(plugin->ep_run)) {
		dbglog(ctx, "[-] Could not resolve run symbol in plugin %s: %s\n",
		    plugin->ep_path, dlerror());
		goto error;
	}

	plugin_ctx = init_plugin_ctx(ctx, plugin);
	if (!(plugin_ctx)) {
		dbglog(ctx, "[-] Could not create plugin ctx for plugin %s\n",
		    plugin->ep_path);
		goto error;
	}

	if (plugin->ep_run(EDGE_RUN_INIT, plugin_ctx)) {
		dbglog(ctx, "[-] Initialization of plugin %s failed\n",
		    plugin->ep_path);
		goto error;
	}

	free(plugin_ctx);

	return (plugin);

error:
	if (plugin->ep_handle)
		dlclose(plugin->ep_handle);

	if (plugin->ep_action)
		free(plugin->ep_action);

	if (plugin_ctx)
		free(plugin_ctx);

	free(plugin);
	return (NULL);
}
