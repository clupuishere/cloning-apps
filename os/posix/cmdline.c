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

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <common/log.h>
#include <common/cmdline.h>


int os_parse_args(int argc, char **argv)
{
    int opt, opt_index, rc = 0;
    const char *short_opts = "ha:t:f:m:";
    const struct option long_opts[] = {
        { "help"               , no_argument       , NULL , 'h' },
        { "app"                , required_argument , NULL , 'a' },
        { "send-time"          , no_argument       , NULL , 't' },
        { "fork"               , no_argument       , NULL , 'f' },
        { "children"           , required_argument , NULL , 'c' },
        { "sleep"              , required_argument , NULL , 's' },
        { "memory"             , required_argument , NULL , 'm' },
        { NULL , 0 , NULL , 0 }
    };

    while (1) {
        opt = getopt_long(argc, argv, short_opts, long_opts, &opt_index);
        if (opt == -1)
            break;

        switch (opt) {
        case 'h':
            print_usage(argv[0]);
            exit(0);
            break;

        case 'a': {
            app = string_to_app(optarg);
            if (app == APP_NONE) {
                ERROR("Unsupported app name: %s\n", optarg);
                print_usage(argv[0]);
                exit(-1);
            }
            break;
        }

        case 't':
            do_send_time = 1;
            break;

        case 'f':
            do_fork = 1;
            break;

        case 'x':
            do_clone = 1;
            break;

        case 'c': {
            children_num = atoi(optarg);
            if (children_num < 1) {
                ERROR("Children number should be positive\n");
                print_usage(argv[0]);
                exit(-1);
            }
            break;
        }

        case 's': {
            sleep_between_clones_msec = atoi(optarg);
            if (sleep_between_clones_msec < 0) {
                ERROR("Unsupported sleep value: %s\n", optarg);
                print_usage(argv[0]);
                exit(-1);
            }
            break;
        }

        case 'm':
            memory_str = optarg;
            break;

        default:
            rc = -1;
            break;
        }
    }

    while (optind < argc) {
        ERROR("%s: invalid argument \'%s\'\n", argv[0], argv[optind]);
        rc = -1;
        optind++;
    }

    if (rc) {
        print_usage(argv[0]);
        exit(rc);
    }

    return rc;
}
