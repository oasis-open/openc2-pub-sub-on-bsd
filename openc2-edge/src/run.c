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

#include <zmq.h>

#include "edge.h"

static void parse_message(edge_context_t *, const char *);

void
run_loop(edge_context_t *ctx)
{
	void *zmq_ctx;
	void *sock;
	char buf[BUFSZ+1];
	int len;

	memset(buf, 0x00, sizeof(buf));

	zmq_ctx = zmq_ctx_new();
	if (!(zmq_ctx)) {
		return;
	}

	sock = zmq_socket(zmq_ctx, ZMQ_SUB);
	if (!(sock)) {
		zmq_ctx_shutdown(zmq_ctx);
		return;
	}

	if (zmq_setsockopt(sock, ZMQ_SUBSCRIBE, "", 0)) {
		zmq_close(sock);
		zmq_ctx_shutdown(zmq_ctx);
		return;
	}

	snprintf(buf, sizeof(buf), "tcp://%s:%s",
	    ctx->ecx_config->ec_orchestrator_ip,
	    ctx->ecx_config->ec_orchestrator_port);

	if (zmq_connect(sock, buf)) {
		zmq_close(sock);
		zmq_ctx_shutdown(zmq_ctx);
		return;
	}

	while (1) {
		memset(buf, 0x00, sizeof(buf));

		len = zmq_recv(sock, buf, BUFSZ, 0);
		if (len == -1) {
			return;
		}

		parse_message(ctx, (const char *)buf);
	}
}

static void
parse_message(edge_context_t *ctx, const char *buf)
{
	edge_plugin_ctx_t *plugin_ctx;
	struct ucl_parser *parser;
	const ucl_object_t *top, *obj;
	const char *str, *action;
	size_t i;

	plugin_ctx = NULL;

	parser = ucl_parser_new(UCL_PARSER_KEY_LOWERCASE);
	if (!(parser)) {
		dbglog(ctx, "[-] Could not create a new parser\n");
		return;
	}

	if (ucl_parser_add_string(parser, buf, 0) == false) {
		dbglog(ctx, "[-] Could not parse the payload\n");
		return;
	}

	top = ucl_parser_get_object(parser);
	if (!(top)) {
		dbglog(ctx, "[-] Could not get the top object\n");
		goto end;
	}

	obj = ucl_lookup_path(top, "action");
	if (!(obj)) {
		dbglog(ctx, "[-] Could not look up the action object\n");
		goto end;
	}

	action = ucl_object_tostring(obj);
	if (!(str)) {
		goto end;
	}

	dbglog(ctx, "[*] %s:%d Here\n", __func__, __LINE__);

	obj = ucl_lookup_path(top, "actuator.type");
	if (obj) {
		if (!(ctx->ecx_config->ec_actuator_type)) {
			goto end;
		}

		str = ucl_object_tostring(obj);
		if (!(str)) {
			goto end;
		}

		if (strcasecmp(str, ctx->ecx_config->ec_actuator_type)) {
			goto end;
		}

		obj = ucl_lookup_path(top, "actuator.id");
		if (obj) {
			str = ucl_object_tostring_forced(obj);
			if (str) {
				if (!(ctx->ecx_config->ec_actuator_id)) {
					goto end;
				}

				if (strcasecmp(str, ctx->ecx_config->ec_actuator_id)) {
					goto end;
				}
			}
		}
	}

	dbglog(ctx, "[*] %s:%d Here\n", __func__, __LINE__);

	plugin_ctx = init_plugin_ctx(ctx, NULL);
	if (!(plugin_ctx)) {
		dbglog(ctx, "[-] Could not create plugin ctx\n");
		goto end;
	}

	plugin_ctx->epc_payload = (void *)top;

	for (i=0; i < ctx->ecx_nplugins; i++) {
		/* NULL ep_action means receive all messages */
		if (!(ctx->ecx_plugins[i]->ep_action) ||
		    !strcasecmp(action, ctx->ecx_plugins[i]->ep_action)) {
			plugin_ctx->epc_plugin = ctx->ecx_plugins[i];
			ctx->ecx_plugins[i]->ep_run(EDGE_RUN_MESSAGE,
			    plugin_ctx);
		}
	}

end:
	if (plugin_ctx)
		free(plugin_ctx);

	ucl_parser_free(parser);
}
