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
#include <common/log.h>
#include <common/cmdline.h>
#include <common/time.h>
#include <common/mem.h>

unsigned long os_page_size;

int mem_touch_pages(char *start, unsigned long pages_num,
        struct timeval *duration)
{
    char *p;
    struct timeval tv_before, tv_after;
    struct timespec ts_before, ts_after;
    int rc = 0;

    if (!os_page_size)
        os_page_size = os_get_page_size();

    if (duration) {
        rc = clock_gettime(CLOCK_MONOTONIC, &ts_before);
        if (rc) {
            ERROR("Error calling clock_gettime()\n");
            goto out;
        }
    }

    for (unsigned long i = 0; i < pages_num; i++) {
        p = start + i * os_page_size;
        *((unsigned long *) p) = i;
    }

    if (duration) {
        rc = clock_gettime(CLOCK_MONOTONIC, &ts_after);
        if (rc) {
            ERROR("Error calling clock_gettime()\n");
            goto out;
        }
    }

    for (unsigned long i = 0; i < pages_num; i++) {
        p = start + i * os_page_size;

        if (*((unsigned long *) p) != i) {
            DEBUG("Mismatch on page %lu: found %lu expected %lu\n",
                i, *((unsigned long *) p), i);
            rc = -1;
            goto out;
        }
    }

    if (duration) {
        tv_before.tv_sec = ts_before.tv_sec;
        tv_before.tv_usec = ts_before.tv_nsec / 1000;

        tv_after.tv_sec = ts_after.tv_sec;
        tv_after.tv_usec = ts_after.tv_nsec / 1000;

        timersub(&tv_after, &tv_before, duration);
    }

out:
    return rc;
}

unsigned long memsize_str2bytes(const char *size_str)
{
    unsigned long n;
    char *endptr;

    n = strtoul(size_str, &endptr, 10);
    if (!n)
        goto out;

    if (*endptr == '\0' || !strcmp(endptr, "B"))
        goto out;
    else if (!strcmp(endptr, "KB"))
        n *= 1024;
    else if (!strcmp(endptr, "MB"))
        n *= 1024 * 1024;
    else if (!strcmp(endptr, "GB"))
        n *= 1024 * 1024 * 1024;

out:
    return n;
}

#define DIV_ROUND_UP(v, d) (((v) + (d)-1) / (d))

unsigned long memsize_str2pages(const char *size_str)
{
    unsigned long bytes_num, pages_num = 0;

    bytes_num = memsize_str2bytes(size_str);
    if (!bytes_num) {
        ERROR("Invalid memory value\n");
        goto out;
    }

    if (!os_page_size)
        os_page_size = os_get_page_size();

    pages_num = DIV_ROUND_UP(bytes_num, os_page_size);

out:
    return pages_num;
}
