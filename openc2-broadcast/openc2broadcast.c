/*-
 * Copyright (c) 2016 G2, Inc
 * Author: Shawn Webb <shawn.webb@g2-inc.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>

/*
 * USAGE: openc2broadcast -s server [-m message] [-m message] ...
 *
 * Note: -s needs to be the first argument.
 */

int
main(int argc, char *argv[])
{
	struct timeval timeout;
	void *ctx, *pubsock;
	const char *server;
	int ch;

	server = NULL;

	while ((ch = getopt(argc, argv, "m:s:")) != -1) {
		switch (ch) {
		case 's':
			server = optarg;
			ctx = zmq_ctx_new();
			pubsock = zmq_socket(ctx, ZMQ_PUB);

			if (zmq_connect(pubsock, server)) {
				fprintf(stderr, "[-] Could not connect\n");
				return (1);
			}

			break;
		case 'm':
			if (server == NULL)
				return (1);

			timeout.tv_sec = 0;
			timeout.tv_usec = 5000;
			select(1, NULL, NULL, NULL, &timeout);

			if (zmq_send(pubsock, optarg, strlen(optarg), 0) != strlen(optarg)) {
				fprintf(stderr, "[-] Could not send data\n");
				return (1);
			}

			break;
		}
	}

	if (server) {
		zmq_disconnect(pubsock, server);
		zmq_ctx_shutdown(ctx);
	}

	return (0);
}
