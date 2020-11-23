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

#include <errno.h>
#include <string.h>
#include <common/log.h>
#include <common/net.h>


static void servaddr_init(struct sockaddr_in *servaddr,
        unsigned int net_addr, unsigned short net_port)
{
    memset(servaddr, 0, sizeof(*servaddr));
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = net_port;
    servaddr->sin_addr.s_addr = net_addr;
}

int mysocket_init(struct mysocket *sock, int type, unsigned short port)
{
    struct sockaddr_in servaddr;
#ifndef __MINIOS__
    int enable = 1;
#endif
    int rc = 0;

    if (!sock) {
        rc = -EINVAL;
        goto out;
    }

    sock->s = socket(AF_INET, type, 0);
    if (sock->s < 0) {
        ERROR("Error calling socket()\n");
        rc = -1;
        goto out;
    }

#ifndef __MINIOS__
    rc = setsockopt(sock->s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    if (rc < 0) {
        ERROR("setsockopt(SO_REUSEADDR) failed (%s).\n", strerror(errno));
        close(sock->s);
    }
#endif

    if (port) {
        servaddr_init(&servaddr, htonl(INADDR_ANY), htons(port));

        rc = bind(sock->s, (struct sockaddr *) &servaddr, sizeof(servaddr));
        if (rc < 0) {
            ERROR("Error calling bind() rc=%d\n", rc);
            close(sock->s);
            goto out;
        }
    }

    sock->port = port;

out:
    return rc;
}

int mysocket_fini(struct mysocket *sock)
{
    int rc = 0;

    if (!sock) {
        rc = -EINVAL;
        goto out;
    }

    if (sock->s >= 0)
        close(sock->s);
out:
    return rc;
}

static int net_msg_init_buffer(struct net_msg *m)
{
    int rc = 0;

    m->netbuf_size = 4096; /* TODO */
    m->netbuf = malloc(m->netbuf_size);
    if (!m->netbuf) {
        ERROR("could not allocate netbuf\n");
        rc = -ENOMEM;
        goto out;
    }

out:
    return rc;
}

void net_msg_cleanup(struct net_msg *m)
{
    if (m->netbuf)
        free(m->netbuf);
    if (m->connection >= 0) {
        close(m->connection);
        m->connection = -1;
    }
}

/*******************************************************************************
 * TCP
 ******************************************************************************/

int tcp_server_start(struct os_server *srv, unsigned short port)
{
    int rc = 0;

    if (!srv) {
        rc = -EINVAL;
        goto out;
    }

    INFO("Opening connection\n");

    rc = mysocket_init(&srv->listener_socket, SOCK_STREAM, port);
    if (rc) {
        ERROR("Error creating socket\n");
        goto out;
    }

    rc = listen(srv->listener_socket.s, 5);
    if (rc) {
        ERROR("Error calling listen() rc=%d\n", rc);
        goto out_cleanup;
    }

out_cleanup:
    if (rc) {
        close(srv->listener_socket.s);
        srv->listener_socket.s = -1;
    }
out:
    return rc;
}

int tcp_server_stop(struct os_server *srv)
{
    int rc = 0;

    if (!srv) {
        rc = -EINVAL;
        goto out;
    }

    if (srv->listener_socket.s >= 0) {
        close(srv->listener_socket.s);
        srv->listener_socket.s = -1;
    }

out:
    return rc;
}

int tcp_server_started(struct os_server *srv)
{
    return (srv->listener_socket.s >= 0);
}

int tcp_server_accept(struct os_server *srv, struct net_msg *m)
{
    socklen_t len = sizeof(m->client_addr);
    int rc = -1;

    memset(m, 0, sizeof(*m));

    m->connection = accept(srv->listener_socket.s, (struct sockaddr *) &m->client_addr, &len);
    if (m->connection < 0) {
        ERROR("Error accept()\n");
        goto out;
    }

    rc = getpeername(m->connection, (struct sockaddr *) &m->client_addr, &len);
    if (rc) {
        ERROR("Error getpeername() rc=%d\n", rc);
        goto out;
    }

    INFO("Connection accepted from %s:%d\n",
        inet_ntoa(m->client_addr.sin_addr), ntohs(m->client_addr.sin_port));

    rc = 0;

out:
    if (rc)
        net_msg_cleanup(m);

    return rc;
}

int tcp_server_recv_msg(struct net_msg *m)
{
    int rc = 0;

    if (!m->netbuf) {
        rc = net_msg_init_buffer(m);
        if (rc) {
            ERROR("Error calling net_msg_init_buffer() rc=%d\n", rc);
            goto out;
        }
    }

    rc = recv(m->connection, m->netbuf, m->netbuf_size, 0);
    if (rc < 0) {
        ERROR("Error calling recv() rc=%d\n", rc);
        goto out;
    }

out:
    return rc;
}

int tcp_server_send_msg(struct net_msg *m)
{
    int rc = 0;

    if (!m->netbuf) {
        rc = -EINVAL;
        goto out;
    }

    rc = send(m->connection, m->netbuf, m->netbuf_size, 0);
    if (rc < 0) {
        ERROR("Error calling send() rc=%d\n", rc);
        goto out;
    }

out:
    return rc;
}

/*******************************************************************************
 * UDP
 ******************************************************************************/

int udp_server_start(struct mysocket *sock, unsigned short port)
{
    return mysocket_init(sock, SOCK_DGRAM, port);
}

int udp_server_recv_msg(struct mysocket *sock, struct net_msg *m)
{
    socklen_t len = sizeof(m->client_addr);
    int rc = -1;

    if (!m->netbuf) {
        rc = net_msg_init_buffer(m);
        if (rc) {
            ERROR("Error calling net_msg_init_buffer() rc=%d\n", rc);
            goto out;
        }
    }

    rc = recvfrom(sock->s, m->netbuf, m->netbuf_size, 0,
            (struct sockaddr *) &m->client_addr, &len);
    if (rc < 0) {
        ERROR("Error calling recvfrom() rc=%d\n", rc);
        goto out;
    }

out:
    return rc;
}

int udp_client_send(struct mysocket *sock,
        struct os_net_ip *ip, unsigned short port, void *data, int size)
{
    struct mysocket _sock;
    struct sockaddr_in servaddr;
    int rc, do_cleanup = 0;

    if (!sock) {
        sock = &_sock;
        rc = mysocket_init(sock, SOCK_DGRAM, 0);
        if (rc) {
            ERROR("Error calling mysocket_init rc=%d()\n", rc);
            rc = -1;
            goto out;
        }
        do_cleanup = 1;
    }

    servaddr_init(&servaddr, os_net_ip_value(ip), htons(port));

    rc = sendto(sock->s,
            data, size, 0,
            (struct sockaddr *) &servaddr, sizeof(servaddr));

    if (do_cleanup)
        close(sock->s);
out:
    return rc;
}

int udp_client_recv(struct mysocket *sock,
        unsigned short port, void *data, int size, int timeout_ms)
{
    struct mysocket _sock;
    int rc, do_cleanup = 0;

    if (!sock) {
        sock = &_sock;
        rc = mysocket_init(sock, SOCK_DGRAM, port);
        if (rc) {
            ERROR("Error creating socket\n");
            goto out;
        }
        do_cleanup = 1;
    }

    if (timeout_ms) {
        rc = os_socket_set_timeout(sock->s, timeout_ms);
        if (rc < 0) {
            ERROR("Error calling os_socket_set_timeout() rc=%d\n", rc);
            goto out_close;
        }
    }

    rc = recv(sock->s, data, size, 0);
    if (rc < 0) {
        ERROR("Error calling recv() rc=%d\n", rc);
        goto out_close;
    }

out_close:
    if (do_cleanup)
        close(sock->s);
out:
    return rc;
}
