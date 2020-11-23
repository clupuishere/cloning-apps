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

#include <string.h>
#include <unistd.h>
#include <common/log.h>
#include <common/cmdline.h>
#include <common/boot.h>
#include <common/time.h>
#include <common/net.h>
#include <common/clone.h>
#include <server-common.h>


static int fork_prologue(struct mysocket *mysock, unsigned short myport,
        int *is_child)
{
    pid_t pid = -1;
    struct timeval tv_before, tv_after, res;
    char suffix[32];
    int rc;

    for (int i = 0; i < children_num; i++) {
        myport -= 1;

        rc = gettimeofday(&tv_before, NULL);
        if (rc) {
            ERROR("Error gettimeofday() rc=%d\n", rc);
            goto out;
        }

        pid = fork();
        if (pid < 0) {
            ERROR("Error fork() pid=%d\n", pid);
            goto out;
        }

        rc = gettimeofday(&tv_after, NULL);
        if (rc) {
            ERROR("Error gettimeofday() rc=%d\n", rc);
            goto out;
        }

        timersub(&tv_after, &tv_before, &res);
        INFO("%lu.%6.6lu\n", res.tv_sec, res.tv_usec);

        if (do_send_time) {
            if (pid > 0) /* parent */
                sprintf(suffix, "parent;%d", i);

            else if (pid == 0) { /* child */
                rc = mysocket_fini(mysock);
                if (rc) {
                    ERROR("Error mysocket_fini() rc=%d\n", rc);
                    goto out;
                }
                rc = mysocket_init(mysock, SOCK_DGRAM, myport);
                if (rc) {
                    ERROR("Error mysocket_init() rc=%d\n", rc);
                    goto out;
                }

                sprintf(suffix, "child;%d", i);
            }
///            if (pid == 0)
            ///            if (pid == 0)
            send_time(mysock, &res, suffix);
        }

        if (pid == 0)
            break;

//            INFO("sleep_between_clones_msec=%d", sleep_between_clones_msec);
//            os_sleep_msec(sleep_between_clones_msec);
    }

    if (is_child)
        *is_child = (pid == 0);
out:
    return rc;
}

static int clone_prologue(struct mysocket *mysock, unsigned short myport,
        int *is_child)
{
    unsigned int myparentid, myid, index;
    struct timeval tv_before, tv_after, res;
    char suffix[32];
    int rc, rc_clone;

    myparentid = os_get_self_id();

    rc = gettimeofday(&tv_before, NULL);
    if (rc) {
        ERROR("Error gettimeofday() rc=%d\n", rc);
        goto out;
    }

    rc = os_clone(children_num);
    if (!(rc >= 0)) {
        ERROR("Error myclone() rc=%d\n", rc);
        goto out;
    }
    INFO("rc=%d\n", rc);
    rc_clone = rc;

    rc = gettimeofday(&tv_after, NULL);
    if (rc) {
        ERROR("Error gettimeofday() rc=%d\n", rc);
        goto out;
    }

    timersub(&tv_after, &tv_before, &res);
    INFO("%lu.%6.6lu\n", res.tv_sec, res.tv_usec);

    if (do_send_time) {
        if (rc_clone == 0) /* parent */
            sprintf(suffix, "parent;%d", myparentid);

        else if (rc_clone == 1) { /* child */
            rc = mysocket_fini(mysock);
            if (rc) {
                ERROR("Error mysocket_fini() rc=%d\n", rc);
                goto out;
            }

            /*
             * This is based on the assumption that domain IDs are consecutive
             * and no other domain is created in the meantime.
             * And no overflow occurs.
             */
            myid = os_get_self_id();
            index = myid - myparentid;
            myport -= index;

            rc = mysocket_init(mysock, SOCK_DGRAM, myport);
            if (rc) {
                ERROR("Error mysocket_init() rc=%d\n", rc);
                goto out;
            }

            sprintf(suffix, "child;%d", index);

        } else {
            ERROR("Unknown clone result: %d\n", rc_clone);
            goto out;
        }

        send_time(mysock, &res, suffix);
    }

    if (is_child)
        *is_child = (rc_clone == 1);
out:
    return rc;
}

void app_resume(void)
{
    struct mysocket su;
    int rc = 0;

    rc = mysocket_init(&su, SOCK_DGRAM, 6666);
    if (rc) {
        ERROR("Error mysocket_init() rc=%d\n", rc);
        goto out;
    }

    rc = send_time(&su, NULL, NULL);
    if (rc)
        ERROR("Error send_time() rc=%d\n", rc);

    rc = mysocket_fini(&su);
    if (rc) {
        ERROR("Error mysocket_fini() rc=%d\n", rc);
        goto out;
    }
out:
    return;
}

int server_prologue(int *is_child)
{
    unsigned short myport;
    struct mysocket su;
    int rc = 0;

    if (do_send_time) {
        myport = PORT_PARENT;

        rc = mysocket_init(&su, SOCK_DGRAM, myport);
        if (rc) {
            ERROR("Error mysocket_init() rc=%d\n", rc);
            goto out;
        }

        rc = send_time(&su, NULL, NULL);
        if (rc) {
            ERROR("Error send_time() rc=%d\n", rc);
            goto out;
        }
    }

    if (do_fork) {
        rc = fork_prologue(&su, myport, is_child);
        if (rc) {
            ERROR("Error fork_prologue() rc=%d\n", rc);
            goto out;
        }

    } else if (do_clone) {
        rc = clone_prologue(&su, myport, is_child);
        if (rc) {
            ERROR("Error clone_prologue() rc=%d\n", rc);
            goto out;
        }

    } else {
        rc = mysocket_fini(&su);
        if (rc) {
            ERROR("Error mysocket_fini() rc=%d\n", rc);
            goto out;
        }
    }

out:
    return rc;
}
