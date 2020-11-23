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
#include <mini-os/types.h>
#include <mini-os/lib.h>
#include <mini-os/errno.h>
#else
#ifdef __Unikraft__
#include <uk/config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#endif
#include <common/log.h>
#include <common/cmdline.h>
#include <common/boot.h>
#include <common/thread.h>
#include <apps.h>


enum app app;
int do_send_time = 0;
int do_fork = 0;
int do_clone = 0;
int children_num = 1;
int sleep_between_clones_msec = 1000;
char *memory_str;

struct app_entry {
    const char *name;
    enum app app;
};

struct app_entry app_entries[] = {
#if CONFIG_CLONING_APP_COUNTER
    { APP_NAME_COUNTER, APP_COUNTER },
#endif
#if CONFIG_CLONING_APP_MEMORY_OVERHEAD
    { APP_NAME_MEMORY_OVERHEAD, APP_MEMORY_OVERHEAD },
#endif
#if CONFIG_CLONING_APP_CHILDREN
    { APP_NAME_CHILDREN, APP_CHILDREN },
#endif
#if CONFIG_CLONING_APP_SLEEPER
    { APP_NAME_SLEEPER, APP_SLEEPER },
#endif
#if CONFIG_CLONING_APP_SERVER_TCP
    { APP_NAME_SERVER_TCP, APP_SERVER_TCP },
#endif
#if CONFIG_CLONING_APP_SERVER_UDP
    { APP_NAME_SERVER_UDP, APP_SERVER_UDP },
#endif
#if CONFIG_CLONING_APP_FILES
    { APP_NAME_FILES, APP_FILES },
#endif
#if CONFIG_CLONING_APP_FUZZ
    { APP_NAME_FUZZ, APP_FUZZ },
#endif
#if CONFIG_CLONING_APP_MEASURE_FORK
    { APP_NAME_MEASURE_FORK, APP_MEASURE_FORK },
#endif
};

enum app string_to_app(const char *s)
{
    int i;

    for (i = 0; i < (int) (sizeof(app_entries) / sizeof(struct app_entry)); i++) {
        if (!strcmp(s, app_entries[i].name))
            return app_entries[i].app;
    }

    return APP_NONE;
}

void print_usage(char *cmd)
{
    OS_PRINT_OUT("Usage: %s [OPTION]..\n", cmd);
    OS_PRINT_OUT("\n");
    OS_PRINT_OUT("Options:\n");
    OS_PRINT_OUT("-h, --help                    Display this help and exit\n");
    OS_PRINT_OUT("-a, --app                     Application name\n");
    OS_PRINT_OUT("-t, --send-time               Report boot time via UDP [default: false]\n");
    OS_PRINT_OUT("-f, --fork                    Create clones [default: false]\n");
    OS_PRINT_OUT("-c, --children                Children number [default: 1]\n");
    OS_PRINT_OUT("-s, --sleep                   # of milliseconds to sleep between each cloning [default: 1]\n");
    OS_PRINT_OUT("-m, --memory                  Memory size\n");
}

#if CONFIG_LIBPROFILING_TRACING
extern int profile_trigger;
#endif

int main(int argc, char **argv)
{
    int rc;

    /* Parse arguments */
    rc = os_parse_args(argc, argv);
    if (rc) {
        ERROR("Error calling os_parse_args() rc=%d\n", rc);
        goto out;
    }

#if CONFIG_LIBPROFILING_TRACING
    profile_trigger = 1;
#endif

    rc = os_app_init();
    if (rc) {
        ERROR("Error calling os_app_init() rc=%d\n", rc);
        goto out;
    }

    switch (app) {
#if CONFIG_CLONING_APP_COUNTER
    case APP_COUNTER:
        rc = (int) (long) thread_func_counter(NULL);
        break;
#endif
#if CONFIG_CLONING_APP_MEMORY_OVERHEAD
    case APP_MEMORY_OVERHEAD:
        rc = (int) (long) thread_func_memory_overhead(NULL);
        break;
#endif
#if CONFIG_CLONING_APP_CHILDREN
    case APP_CHILDREN:
        rc = (int) (long) thread_func_children(NULL);
        break;
#endif
#if CONFIG_CLONING_APP_SLEEPER
    case APP_SLEEPER:
        rc = (int) (long) thread_func_sleeper(NULL);
        break;
#endif
#if CONFIG_CLONING_APP_SERVER_TCP
    case APP_SERVER_TCP:
        rc = (int) (long) thread_func_server_tcp(NULL);
        break;
#endif
#if CONFIG_CLONING_APP_SERVER_UDP
    case APP_SERVER_UDP:
        rc = (int) (long) thread_func_server_udp(NULL);
        break;
#endif
#if CONFIG_CLONING_APP_FILES
    case APP_FILES:
        rc = (int) (long) thread_func_files(NULL);
        break;
#endif
#if CONFIG_CLONING_APP_FUZZ
    case APP_FUZZ:
        rc = (int) (long) thread_func_fuzz(NULL);
        break;
#endif
#if CONFIG_CLONING_APP_MEASURE_FORK
    case APP_MEASURE_FORK:
        rc = (int) (long) thread_func_measure_fork(NULL);
        break;
#endif
    default:
        print_usage(argv[0]);
        rc = -EINVAL;
        goto out;
    }

out:
    return rc;
}
