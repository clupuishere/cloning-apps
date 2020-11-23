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
#include <mini-os/xmalloc.h>
#include <mini-os/sched.h>
#include <mini-os/errno.h>
#include <common/log.h>
#include <common/thread.h>


static void thread_func_wrapper(void *arg)
{
    struct os_thread *t = arg;

    t->result = t->func(t->arg);
}

int os_thread_create(char *name, thread_func_t func, void *arg,
        struct os_thread **pt)
{
    struct thread *t;
    struct os_thread *os_t;
    int rc = 0;

    INFO("Running %s app\n", name);

    os_t = malloc(sizeof(*os_t));
    if (!os_t) {
        ERROR("Error allocating OS thread\n");
        rc = -ENOMEM;
        goto out;
    }

    os_t->func = func;
    os_t->arg = arg;
    init_waitqueue_head(&os_t->wq);
    os_t->finished = 0;

    t = create_thread(name, thread_func_wrapper, os_t);
    if (!t) {
        ERROR("Error calling create_thread()\n");
        rc = -ENOMEM;
        goto out;
    }

    *pt = os_t;

out:
    if (rc)
        free(os_t);

    return rc;
}

int os_thread_destroy(struct os_thread *t)
{
    int rc = 0;

    if (!t) {
        rc = -EINVAL;
        goto out;
    }

    free(t);
out:
    return rc;
}

int os_thread_wait(struct os_thread *t, void **thread_return)
{
    wait_event(t->wq, t->finished);
    *thread_return = t->result;
    return 0;
}
