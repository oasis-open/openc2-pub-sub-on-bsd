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

#include "edge.h"

void
free_plugins(edge_context_t *ctx)
{
	size_t i;

	for (i=0; i < ctx->ecx_nplugins; i++) {
		if (ctx->ecx_plugins[i]->ep_action) {
			free(ctx->ecx_plugins[i]->ep_action);
		}

		if (ctx->ecx_plugins[i]->ep_handle) {
			dlclose(ctx->ecx_plugins[i]->ep_handle);
		}
	}

	ctx->ecx_nplugins = 0;
	ctx->ecx_plugins = NULL;

	return;
}

edge_plugin_ctx_t *
init_plugin_ctx(edge_context_t *ctx, edge_plugin_t *plugin)
{
	edge_plugin_ctx_t *plugin_ctx;

	plugin_ctx = calloc(1, sizeof(edge_plugin_ctx_t));
	if (!(plugin_ctx))
		return NULL;

	plugin_ctx->epc_ctx = ctx;
	plugin_ctx->epc_plugin = plugin;
	plugin_ctx->epc_dbglog = dbglog;

	return plugin_ctx;
}
