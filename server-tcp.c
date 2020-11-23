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

#include <common/log.h>
#include <common/net.h>
#include <server-common.h>


void *thread_func_server_tcp(void *p)
{
    struct os_server server;
    struct net_msg msg;
    long rc = -1;

    (void) p;

    rc = server_prologue(NULL);
    if (rc) {
        ERROR("Error server_prologue() rc=%ld\n", rc);
        goto out;
    }

    rc = tcp_server_start(&server, DEFAULT_SERVER_PORT);
    if (rc) {
        ERROR("Error tcp_server_start() rc=%ld\n", rc);
        goto out;
    }
    INFO("Listening....\n");

    /* TODO try fork() here */

    while (1) {
        rc = tcp_server_accept(&server, &msg);
        if (rc) {
            ERROR("Error tcp_server_accept() rc=%ld\n", rc);
            goto out_server_stop;
        }

        while (1) {
            rc = tcp_server_recv_msg(&msg);
            if (rc == 0)
                break;
            if (rc < 0) {
                ERROR("Error tcp_server_recv_msg() rc=%ld\n", rc);
                break;
            }
        }

        net_msg_cleanup(&msg);
        if (rc < 0)
            break;
    }

out_server_stop:
    tcp_server_stop(&server);
out:
    INFO("Exiting\n");
    return (void *) rc;
}

