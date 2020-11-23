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

#ifdef __MINIOS__
#include <time.h>
#else
#include <stdlib.h>
#endif
#include <string.h>
#include <sys/time.h>
#include <common/log.h>
#include <common/cmdline.h>
#include <common/time.h>


#if CFG_NETWORK
int send_time(struct mysocket *sock, struct timeval *tv, char *suffix)
{
    struct timeval _tv;
    char message[64];
    struct os_net_ip gw_ip;
    int rc;

    if (!do_send_time) {
        rc = -1;
        goto out;
    }

    DEBUG("Sending udp\n");

    if (!tv) {
        tv = &_tv;
        rc = gettimeofday(tv, NULL);
        if (rc) {
            ERROR("Error calling gettimeofday() rc=%d\n", rc);
            goto out;
        }
    }

    sprintf(message, "%lu.%6.6lu", tv->tv_sec, tv->tv_usec);
    if (suffix)
        sprintf(message + strlen(message), ";%s", suffix);
    DEBUG("message=%s\n", message);

    rc = os_net_ip_get_gw(&gw_ip);
    if (rc) {
        ERROR("Error calling os_net_ip_get_gw() rc=%d\n", rc);
        goto out;
    }

    /*
     * we want to make sure our UDP message is received, hence the loop
     */
    while (1) {
        rc = udp_client_send(sock, &gw_ip, 32764, message, strlen(message));
        if (rc < 0) {
            ERROR("Error calling udp_client_send() rc=%d\n", rc);
            goto out;
        }
        DEBUG("Sent udp rc=%d vs strlen(message)=%d\n", rc, strlen(message));

        /* wait for ack */
        rc = udp_client_recv(sock, 5000, message, sizeof(message), 2000);
        if (rc < 0) {
            ERROR("Error calling udp_client_recv() rc=%d\n", rc);
            goto out;
        }
        if (rc > 0) {
            rc = 0;
            break;
        }
    }

out:
    return rc;
}
#endif

int print_timestamp(const char *message)
{
    struct timeval tv;
    int rc;

    rc = gettimeofday(&tv, NULL);
    if (rc) {
        ERROR("Error calling gettimeofday() rc=%d\n", rc);
        goto out;
    }

    INFO("%lu.%6.6lu %s\n", tv.tv_sec, tv.tv_usec, message);
out:
    return rc;
}
