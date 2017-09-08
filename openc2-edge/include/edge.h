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

#ifndef _EDGE_H
#define	_EDGE_H

#include <ucl.h>

#define	EDGE_VERSION	2016030801UL

#define EDGE_FLAG_NONE	0x00000000
#define EDGE_FLAG_DEBUG	0x00000001

struct _edge_context;
struct _edge_plugin_ctx;

typedef enum _edge_plugin_run {
	EDGE_RUN_INIT=0,
	EDGE_RUN_MESSAGE
} edge_plugin_run_t;

typedef int (*edge_plugin_func_t)(edge_plugin_run_t,
    struct _edge_plugin_ctx *);

typedef struct _edge_plugin {
	char			*ep_action;
	const char		*ep_path;
	void			*ep_handle;
	edge_plugin_func_t	 ep_run;
} edge_plugin_t;

typedef struct _edge_config {
	const char	*ec_orchestrator_ip;
	const char	*ec_orchestrator_port;
	const char	*ec_actuator_type;
	const char	*ec_actuator_id;
	const char	*ec_plugin_dir;
	const char	*ec_logfile;
	FILE		*ec_logfp;
	uint32_t	 ec_flags;
} edge_config_t;

typedef struct _edge_context {
	uint32_t	 ecx_version;
	char		*ecx_config_path;
	edge_config_t	*ecx_config;
	edge_plugin_t	**ecx_plugins;
	size_t		 ecx_nplugins;
} edge_context_t;

typedef struct _edge_plugin_ctx {
	edge_context_t	*epc_ctx;
	edge_plugin_t	*epc_plugin;
	void		*epc_payload;
	void		(*epc_dbglog)(edge_context_t *, const char *, ...);
} edge_plugin_ctx_t;

#if !defined(_EDGE_PLUGIN)

#define	BUFSZ	8192
#define DEBUG_MODE

int read_config(const char *, edge_context_t *);
void run_loop(edge_context_t *);
edge_plugin_t *parse_plugin_config(edge_context_t *, const ucl_object_t *);
void free_plugins(edge_context_t *);
void dbglog(edge_context_t *, const char *, ...);

edge_plugin_ctx_t *init_plugin_ctx(edge_context_t *, edge_plugin_t *);

#endif /* !_EDGE_PLUGIN */

#endif /* _EDGE_H */
