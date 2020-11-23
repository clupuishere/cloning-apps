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

#ifndef APPS_H_
#define APPS_H_

#define APP_NAME_COUNTER                     "counter"
#define APP_NAME_MEMORY_OVERHEAD             "memory-overhead"
#define APP_NAME_CHILDREN                    "children"
#define APP_NAME_SLEEPER                     "sleeper"
#define APP_NAME_SERVER_TCP                  "server-tcp"
#define APP_NAME_SERVER_UDP                  "server-udp"
#define APP_NAME_FILES                       "files"
#define APP_NAME_FUZZ                        "fuzz"
#define APP_NAME_MEASURE_FORK                "measure-fork"

enum app {
    APP_NONE,
    APP_COUNTER,
    APP_MEMORY_OVERHEAD,
    APP_CHILDREN,
    APP_SLEEPER,
    APP_SERVER_TCP,
    APP_SERVER_UDP,
    APP_FILES,
    APP_FUZZ,
    APP_MEASURE_FORK,
};

enum app string_to_app(const char *s);

void *thread_func_counter(void *p);
void *thread_func_memory_overhead(void *p);
void *thread_func_children(void *p);
void *thread_func_sleeper(void *p);
void *thread_func_server_tcp(void *p);
void *thread_func_server_udp(void *p);
void *thread_func_files(void *p);
void *thread_func_fuzz(void *p);
void *thread_func_measure_fork(void *p);

#endif /* APPS_H_ */
