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
#include <errno.h>
#include <common/log.h>
#include <common/cmdline.h>
#include <common/clone.h>


int os_clone(unsigned int nr_children)
{
#if 0
    unsigned short *child_ids;
    int rc;

    DEBUG("cloning children=%d\n", nr_children);

    if (!nr_children) {
        rc = -EINVAL;
        goto out;
    }

    child_ids = malloc(sizeof(unsigned short) * nr_children);
    if (!child_ids) {
        rc = -ENOMEM;
        goto out;
    }

    rc = ukplat_clone(nr_children, child_ids);
    if (rc == 0) /* parent */
        rc = 0;
    else if (rc == 1) { /* child */
        rc = 0;
        do_fork = 0; /* only parent forks */
    } else {
        ERROR("Error calling clone() rc=%d\n", rc);
        rc = -1;
    }

    free(child_ids);

out:
    return rc;
#else
    /*TODO*/
    (void) nr_children;
    return -1;
#endif
}

unsigned int os_get_self_id(void)
{
    return 0;
}
