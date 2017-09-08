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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <edge.h>

static int handle_message(edge_plugin_ctx_t *, const ucl_object_t *);

int
run(edge_plugin_run_t runtype, edge_plugin_ctx_t *plugin_ctx)
{

	switch (runtype) {
	case EDGE_RUN_INIT:
		plugin_ctx->epc_plugin->ep_action = strdup("deny");
		if (!(plugin_ctx->epc_plugin->ep_action)) {
			return (1);
		}

		break;

	case EDGE_RUN_MESSAGE:
		return (handle_message(plugin_ctx,
		    (const ucl_object_t *)(plugin_ctx->epc_payload)));
		break;
	default:
		break;
	}

	return (0);
}

static int
handle_message(edge_plugin_ctx_t *plugin_ctx, const ucl_object_t *top)
{
	edge_context_t *ctx;
	const ucl_object_t *obj;
	const char *ip, *str;
	int pid, status;
	char *args[7];

	ctx = plugin_ctx->epc_ctx;

	obj = ucl_lookup_path(top, "target.type");
	if (!(obj)) {
		plugin_ctx->epc_dbglog(ctx, "[-] %s: Could not look up target type\n", __func__);
		return (1);
	}

	str = ucl_object_tostring(obj);
	if (!(str)) {
		plugin_ctx->epc_dbglog(ctx, "[-] %s: Could not read target type\n", __func__);
		return (1);
	}

	if (strcasecmp(str, "cybox:Network_Connection")) {
		/* Not matching isn't an error, it simply means we
		 * don't know/care about this target type. */
		plugin_ctx->epc_dbglog(ctx, "[-] %s: Ignoring deny type \"%s\"\n", __func__, str);
		return (0);
	}

	obj = ucl_lookup_path(top, "target.specifier.id");
	if (!(obj)) {
		plugin_ctx->epc_dbglog(ctx, "[-] %s: Could not look up IP\n", __func__);
		return (1);
	}

	ip = ucl_object_tostring(obj);
	if (!(ip)) {
		plugin_ctx->epc_dbglog(ctx, "[-] %s: Could not read IP\n", __func__);
		return (1);
	}

	switch ((pid = fork())) {
		case -1:
			return (1);
		case 0:
			plugin_ctx->epc_dbglog(ctx, "[*] Blocking %s\n", ip);

			args[0] = "/sbin/pfctl";
			args[1] = "-t";
			args[2] = "blockedpeeps";
			args[3] = "-T";
			args[4] = "add";
			args[5] = (char *)ip;
			args[6] = NULL;

			if (execve(args[0], args, NULL)) {
				plugin_ctx->epc_dbglog(ctx, "[-] Could not execute pfctl: %s\n",
				    strerror(errno));
				_exit(1);
			}
		default:
			waitpid(pid, &status, 0);
			return (WEXITSTATUS(status));
	}

	return (1);
}
