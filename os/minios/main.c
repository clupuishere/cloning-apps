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

#include <mini-os/types.h>
#include <mini-os/lib.h>
#include <mini-os/errno.h>
#include <mini-os/kernel.h>
#include <mini-os/sched.h>
#include <common/log.h>


#define MAX_ARGV_NUM 16
static char *__argv[MAX_ARGV_NUM];
static int __argc;

extern int cmdline_tokenize(char *str, char *argv[], int argv_num);
extern int main(int argc, char **argv);

static void threaded_main_wrapper(void *p)
{
    __argv[__argc++] = "cloning-apps"; /* TODO macrodefinition */
    __argc += cmdline_tokenize(cmdline, &__argv[__argc], MAX_ARGV_NUM - 1);
    main(__argc, __argv);
    do_exit();
}

int app_main(void *p)
{
    struct thread *t;
    int rc = 0;

    t = create_thread((char *) __FUNCTION__, threaded_main_wrapper, NULL);
    if (!t) {
        ERROR("Error creating thread\n");
        rc = -ENOMEM;
    }
    return rc;
}
