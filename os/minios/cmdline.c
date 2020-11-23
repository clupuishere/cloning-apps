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
#include <mini-os/types.h>
#include <mini-os/kernel.h>
#include <mini-os/lib.h>
#include <common/log.h>
#include <common/cmdline.h>


int cmdline_tokenize(char *str, char *argv[], int argv_num)
{
    int i, len = strlen(str), argc = 0;
    char *token_start = NULL;

    for (i = 0; i < len && argc < argv_num; i++) {
        switch (str[i]) {
        case '\0':
            goto out;
        case ' ':
        case '\t':
            str[i] = '\0';
            if (token_start) {
                /* end of new token */
                argv[argc++] = token_start;
                token_start = NULL;
            }
            break;
        default:
            if (!token_start)
                /* start of new token */
                token_start = &str[i];
            break;
        }
    }

out:
    if (token_start)
        argv[argc++] = token_start;

    return argc;
}

int os_parse_args(int argc, char **argv)
{
    int i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_usage(argv[0]);
            do_exit();

        } else if (!strcmp(argv[i], "-a") || !strcmp(argv[i], "--app")) {
            app = string_to_app(argv[i + 1]);
            if (app == APP_NONE) {
                ERROR("Unsupported app name: %s\n", argv[i + 1]);
                print_usage(argv[0]);
                do_exit();
            }
            i++;

        } else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--send-time"))
            do_send_time = 1;

        else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--fork"))
            do_fork = 1;

        else if (!strcmp(argv[i], "-x") || !strcmp(argv[i], "--clone"))
            do_clone = 1;

        else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--children")) {
            sscanf(argv[i + 1], "%d", &children_num);
            if (children_num < 1) {
                ERROR("Children number should be positive\n");
                do_exit();
            }
            i++;

        } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--sleep")) {
            sscanf(argv[i + 1], "%d", &sleep_between_clones_msec);
            i++;

        } else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--memory")) {
            memory_str = argv[i + 1];
            i++;

        } else
            ERROR("Invalid argument \'%s\'\n", argv[i]);
    }

    return 0;
}
