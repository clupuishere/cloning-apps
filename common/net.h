/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Costin Lupu <costin.lupu@cs.pub.ro>
 *
 * Copyright (c) 2020, University Politehnica of Bucharest. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#ifndef APP_COMMON_NET_H_
#define APP_COMMON_NET_H_

#define DEFAULT_SERVER_PORT  6613

struct mysocket {
    int s;
    unsigned short port;
};

#ifdef __MINIOS__
#include <os/minios/net.h>
#else
#include <os/posix/net.h>
#endif

void net_msg_cleanup(struct net_msg *m);

int os_net_ip_get_gw(struct os_net_ip *ip);
int os_socket_set_timeout(int s, int timeout_ms);

int mysocket_init(struct mysocket *sock, int type, unsigned short port);
int mysocket_fini(struct mysocket *sock);

int tcp_server_start(struct os_server *srv, unsigned short port);
int tcp_server_stop(struct os_server *srv);
int tcp_server_started(struct os_server *srv);
int tcp_server_accept(struct os_server *srv, struct net_msg *m);
int tcp_server_recv_msg(struct net_msg *m);
int tcp_server_send_msg(struct net_msg *m);

int udp_server_start(struct mysocket *sock, unsigned short port);
int udp_server_recv_msg(struct mysocket *sock, struct net_msg *m);

int udp_client_send(struct mysocket *sock,
        struct os_net_ip *ip, unsigned short port, void *data, int size);
int udp_client_recv(struct mysocket *sock,
        unsigned short port, void *data, int size, int timeout_ms);

#endif /* APP_COMMON_NET_H_ */
