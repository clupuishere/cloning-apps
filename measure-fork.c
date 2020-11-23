/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Costin Lupu <costin.lupu@cs.pub.ro>
 *
 * Copyright (c) 2021, University Politehnica of Bucharest. All rights reserved.
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

#include <string.h>
#include <unistd.h>
#include <common/log.h>
#include <common/cmdline.h>
#include <common/boot.h>
#include <common/time.h>
#include <common/mem.h>
#include <common/net.h>
#include <common/profile.h>
#include <server-common.h>


#define DIV_ROUND_UP(v, d) (((v) + (d)-1) / (d))

void *thread_func_measure_fork(void *p)
{
    char *start;
    unsigned long pages_num;
    struct os_server server;
    int keep_running;
    struct net_msg msg;
    long rc = -1;

    (void) p;

    if (!memory_str) {
        ERROR("Memory value not provided\n");
        goto out;
    }

    pages_num = memsize_str2pages(memory_str);
    if (!pages_num) {
        ERROR("Invalid memory value\n");
        goto out;
    }

    rc = server_prologue(NULL);
    if (rc) {
        ERROR("Error server_prologue() rc=%ld\n", rc);
        goto out;
    }

    if (!os_page_size)
        os_page_size = os_get_page_size();

    rc = os_alloc_pages(pages_num, &start);
    if (rc) {
        ERROR("Could not allocate %s\n", memory_str);
        goto out;
    }

    rc = mem_touch_pages(start, pages_num, NULL);
    if (rc) {
        ERROR("Could not write on memory\n");
        goto out_free_pages;
    }

    rc = tcp_server_start(&server, DEFAULT_SERVER_PORT);
    if (rc) {
        ERROR("Error tcp_server_start() rc=%ld\n", rc);
        goto out_free_pages;
    }
    INFO("Listening....\n");
    keep_running = 1;
    while (keep_running) {
        rc = tcp_server_accept(&server, &msg);
        if (rc) {
            ERROR("Error tcp_server_accept() rc=%ld\n", rc);
            break;
        }

        rc = tcp_server_recv_msg(&msg);
        if (rc < 0) {
            ERROR("Error tcp_server_recv_msg() rc=%ld\n", rc);
            net_msg_cleanup(&msg);
            break;
        }

        if (!strncmp(msg.netbuf, "fork", strlen("fork"))) {
            pid_t pid;
            const char *label;

            PROFILE_NESTED_TICK();
            pid = fork();
            if (pid == 0)
                label = "fork child";
            else if (pid > 0)
                label = "fork parent";
            else {
                label = "fork error";
                keep_running = 0;
            }

            PROFILE_NESTED_TOCK_MSEC(label);

            if (pid == 0)
                os_exit(0);

        } else if (!strncmp(msg.netbuf, "stop", strlen("stop")))
            keep_running = 0;

        net_msg_cleanup(&msg);
    }

    tcp_server_stop(&server);

out_free_pages:
    os_free_pages(start, pages_num);
out:
    INFO("Exiting\n");
    return (void *) rc;
}
