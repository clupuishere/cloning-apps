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

#define _GNU_SOURCE
#include <string.h>
#include <common/log.h>
#include <server-common.h>
#include <xen/xen.h>
#include <uk/errptr.h>
#include <xenbus/xs.h>
#include <xenbus/client.h>


static char test1[] = "deadbeef";
static char test2[] = "nottbeef";

extern int ukplat_harness(void);

static inline int harness(void)
{
    int rc;

    rc = ukplat_harness();
    DEBUG("ukplat_harness()=%d\n", rc);
    return rc;
}

static int wait_harness_trigger(void)
{
    char *dir = "data", *node = "trigger-harness", *path, *value = NULL;
    struct xenbus_watch *watch = NULL;
    int rc;

    rc = xs_write(XBT_NIL, dir, node, "");
    if (rc) {
        uk_pr_err("Error calling xs_write() rc=%d\n", rc);
        goto out;
    }

    rc = asprintf(&path, "%s/%s", dir, node);
    if (rc <= 0) {
        uk_pr_err("Failed to format back_state_path: %d\n", rc);
        goto out;
    }

    /* create a local watch */
    watch = xs_watch_path(XBT_NIL, path);
    if (PTRISERR(watch)) {
        rc = PTR2ERR(watch);
        goto out;
    }

    DEBUG("waiting harness trigger\n");
    for (;;) {
        value = xs_read(XBT_NIL, dir, node);
        if (value && !strcmp(value, "done"))
            break;

        xenbus_watch_wait_event(watch);
    }

out:
    if (path)
        free(path);
    if (watch)
        xs_unwatch(XBT_NIL, watch);

    return rc;
}

static int path1(int x)
{
    return ++x;
}
static int path2(int x)
{
    return x+12;
}
static int path3(int x)
{
    return x*12;
}
static int path4(int x)
{
    return --x;
}

static int *test(int x)
{
    int *y = NULL;

    switch(x % 4) {
    case 0:
        x = path1(x);
        break;
    case 1:
        x = path2(x);
        break;
    case 2:
        x = path3(x);
        break;
    case 3:
        x = path4(x);
        break;
    };

    if ( !memcmp(test1, test2, 8) )
        *y = x; // NULL-deref oops

    return y;
}

static int my_init_module(void)
{
    int *x = NULL;

    INFO("Kernel Fuzzer Test Module Test1 %p %s Test2 %p %s\n",
            test1, test1, test2, test2);

    wait_harness_trigger();

    DEBUG("harnessing start\n");
    harness();
    DEBUG("harnessed start\n");

    x = test((int)test1[0]);

    DEBUG("harnessing stop\n");
    harness();
    DEBUG("harnessed stop\n");

    INFO("Test: %p\n", x);

    return 0;
}

void *thread_func_fuzz(void *p)
{
    long rc;

    (void) p;

    rc = my_init_module();

    INFO("Exiting\n");
    return (void *) rc;
}
