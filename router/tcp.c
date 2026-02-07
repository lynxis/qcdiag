/*
 * Copyright (c) 2026 Alexander Couzens <lynxis@fe80.eu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "diag.h"
#include "dm.h"
#include "watch.h"

static int tcp_listen(int fd, void *data)
{
	struct diag_client *dm;
	int client;
	int ret;

	client = accept(fd, NULL, NULL);
	if (client < 0) {
		fprintf(stderr, "tcp: failed to accept.");
		return 0;
	}

	printf("tcp: accepting connection on fd %d\n", fd);

	ret = fcntl(client, F_SETFL, O_NONBLOCK);
	if (ret < 0) {
		fprintf(stderr, "tcp: failed to set O_NONBLOCK on fd %d", client);
		return 0;
	}

	/* Most client require hdlc encapsulating */
	dm = dm_add("tcp46", client, client, true);
	dm_enable(dm);

	return 0;
}

int diag_tcp_bind(int fd, void *data, size_t data_size)
{
	int ret;
	struct sockaddr *addr = data;

	ret = bind(fd, addr, data_size);
	if (ret < 0) {
		fprintf(stderr, "tcp: failed to bind diag socket %d\n", fd);
		return -1;
	}

	ret = listen(fd, 2);
	if (ret < 0) {
		fprintf(stderr, "failed to listen on diag socket %d\n", fd);
		return -1;
	}

	watch_add_readfd(fd, tcp_listen, NULL, NULL);

	return 0;
}

int diag_tcp6_open(const char *bind_addr, uint16_t bind_port)
{
	struct sockaddr_in6 addr = {
		.sin6_family = AF_INET6,
		.sin6_port = htons(bind_port)
	};
	int ret;
	int fd;

	printf("tcp6: binding to [%s]:%d\n", bind_addr, bind_port);

	ret = inet_pton(AF_INET6, bind_addr, &addr.sin6_addr);
	if (ret != 1) {
		fprintf(stderr, "tcp6: failed to parse bind addr %s - %s\n", bind_addr, strerror(errno));
		return -1;
	}

	fd = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		fprintf(stderr, "tcp6: failed to create tcp socket\n");
		return -1;
	}

	return diag_tcp_bind(fd, &addr, sizeof(addr));
}

int diag_tcp4_open(const char *bind_addr, uint16_t bind_port)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(bind_port)
	};

	int ret;
	int fd;

	printf("tcp4: binding to %s:%d\n", bind_addr, bind_port);
	ret = inet_pton(AF_INET, bind_addr, &addr.sin_addr);
	if (ret != 1) {
		fprintf(stderr, "tcp4: failed to parse bind addr %s - %s\n", bind_addr, strerror(errno));
		return -1;
	}

	fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		fprintf(stderr, "tcp4: failed to create tcp socket\n");
		return -1;
	}

	return diag_tcp_bind(fd, &addr, sizeof(addr));
}
