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

#include <errno.h>
#include <assert.h>
#include <uk/alloc.h>
#include <common/mem.h>


int os_alloc_max_contiguous_memory(char **pstart, unsigned long *ppages_num)
{
    char *start;
    unsigned long pages_num;
    int rc, order;

    for (order = 0; ; order++) {
        pages_num = (1 << (order));
        start = uk_palloc(uk_alloc_get_default(), pages_num);
        assert((((unsigned long) start) & (os_get_page_size() - 1)) == 0);

        if (start) {
            *((unsigned long *) start) = order; /* try it out */
            uk_pfree(uk_alloc_get_default(), start, pages_num);
        } else
            break;
    }

    if (order) {
        order--;
        pages_num = (1 << (order));
        *pstart = uk_palloc(uk_alloc_get_default(), pages_num);
        *ppages_num = pages_num;
        rc = 0;
    } else
        rc = -1;

    return rc;
}

int os_free_max_contiguous_memory(char *start, unsigned long pages_num)
{
    return os_free_pages(start, pages_num);
}


int os_alloc_pages(unsigned long pages_num, char **pstart)
{
    char *start;
    int rc;

    start = uk_palloc(uk_alloc_get_default(), pages_num);
    if (start)
        *pstart = start;
    else
        rc = -ENOMEM;

    return rc;
}

int os_free_pages(char *start, unsigned long pages_num)
{
    int rc = 0;

    if (((unsigned long) start) & (os_get_page_size() - 1)) {
        rc = -EINVAL;
        goto out;
    }

    uk_pfree(uk_alloc_get_default(), start, pages_num);

out:
    return rc;
}
