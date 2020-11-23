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
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <common/log.h>
#include <common/cmdline.h>
#include <common/net.h>
#include <common/clone.h>
#include <common/profile.h>
#include <server-common.h>

#define SIMPLE_TEST_FILENAME "/root/files/test"

#define SIMPLE_TEST_WRITE_STR(fd, str) \
    do { \
        INFO("writing %s", str); \
        rc = write(fd, str, strlen(str)); \
        if (rc != strlen(str)) { \
            ERROR("Error writing '%s' errno=%d\n", str, errno); \
            goto out_close; \
        } else { \
            INFO("done %s", str); \
            rc = 0; \
        } \
    } while (0)

static long simple_test(void)
{
    long rc = -1;
    int fd, is_child = 0;

    fd = open(SIMPLE_TEST_FILENAME, O_WRONLY | O_CREAT);
    if (fd == -1) {
        ERROR("Error opening %s errno=%d\n", SIMPLE_TEST_FILENAME, errno);
        goto out;
    }

    rc = server_prologue(&is_child);
    if (rc) {
        ERROR("Error server_prologue() rc=%ld\n", rc);
        goto out_close;
    }

    SIMPLE_TEST_WRITE_STR(fd, "parent\n");

    if (do_fork) {
        if (is_child)
            SIMPLE_TEST_WRITE_STR(fd, "child-after-fork\n");
        else
            SIMPLE_TEST_WRITE_STR(fd, "parent-after-fork\n");
    } else
        SIMPLE_TEST_WRITE_STR(fd, "parent-without-fork\n");

    /* TODO should we keep running? */
    while (1) {
//        printf("VM id=%d\n", os_get_self_id());
        sleep(1);
    }

out_close:
    if (fd >= 0) {
        int _rc;

        _rc = close(fd);
        if (_rc) {
            ERROR("Error closing file errno=%d\n", errno);
            if (!rc)
                rc = _rc;
            goto out;
        }
    }
out:
    return rc;
}

#if 0
static int create_file_write_string(const char *filename, const char *str)
{
    int fd, rc;

    fd = open(filename, O_WRONLY | O_CREAT);
    if (fd == -1) {
        ERROR("Error opening %s errno=%d\n", filename, errno);
        goto out;
    }

    rc = write(fd, str, strlen(str));
    if (rc != (int) strlen(str)) {
        ERROR("Error writing '%s' errno=%d\n", str, errno);
        goto out_close;
    }
    rc = 0;
out_close:
    close(fd);
out:
    return rc;
}
#endif

enum write_type {
    WRITE_CHARS,
    WRITE_WORDS,
    WRITE_BUFFER
};

static int str_to_write_type(const char *str)
{
    int rc = -1;

#define MYSTRNCMP(str, match) \
    strncmp(str, match, strlen(match))

    if (!MYSTRNCMP(str, "write-chars"))
        rc = WRITE_CHARS;
    else if (!MYSTRNCMP(str, "write-words"))
        rc = WRITE_WORDS;
    else if (!MYSTRNCMP(str, "write-buffer"))
        rc = WRITE_BUFFER;

    return rc;
}

static int write_chars(int fd, unsigned long size)
{
    char c = 'a';
    int rc;

    for (unsigned long i = 0; i < size; i++) {
        rc = write(fd, &c, sizeof(c));
        if (rc != sizeof(c)) {
            ERROR("Error writing '%c' errno=%d\n", c, errno);
            goto out;
        }
    }
    rc = 0;
out:
    return rc;
}

static int write_words(int fd, unsigned long size)
{
    unsigned long data = 0xdeadf00ddeadbeef, count;
    int rc;

    count = 0;
    while (count < size) {
        PROFILE_NESTED_TICK();
        rc = write(fd, &data, sizeof(data));
        PROFILE_NESTED_TOCK_MSEC("write");
        if (rc < 0) {
            ERROR("Error writing buffer rc=%d errno=%d\n", rc, errno);
            goto out;
        }
        assert(rc == sizeof(data));
        count += rc;
    }
    rc = 0;
out:
    return rc;
}

static int write_buffer(int fd, unsigned long size)
{
    void *buf;
    unsigned long offset;
    int rc;

    buf = malloc(size);
    if (!buf) {
        ERROR("Error no memory");
        rc = -ENOMEM;
        goto out;
    }

    offset = 0;
    while (offset < size) {
        rc = write(fd, buf + offset, size - offset);
        if (rc < 0) {
            ERROR("Error writing buffer rc=%d errno=%d\n", rc, errno);
            goto out;
        }
        offset += rc;
    }
    free(buf);
    rc = 0;
out:
    return rc;
}

typedef int (*write_fn_t)(int fd, unsigned long size);

static int create_file_write_data(const char *filename, unsigned long size,
        enum write_type wt)
{
    write_fn_t functions[] = {
        write_chars,
        write_words,
        write_buffer
    };
    int fd, rc = -1;

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {
        ERROR("Error opening %s errno=%d\n", filename, errno);
        goto out;
    }
    INFO("Created %s\n", filename);

    rc = functions[wt](fd, size);

    close(fd);
out:
    return rc;
}

static long run_server(void)
{
    struct os_server server;
    int keep_running;
    struct net_msg msg;
    const char *filename;
    long rc;

    rc = tcp_server_start(&server, DEFAULT_SERVER_PORT);
    if (rc) {
        ERROR("Error tcp_server_start() rc=%ld\n", rc);
        goto out;
    }
    INFO("Listening....\n");

    keep_running = 1;
    while (keep_running) {
        rc = tcp_server_accept(&server, &msg);
        if (rc) {
            ERROR("Error tcp_server_accept() rc=%ld\n", rc);
            break;
        }

        rc = tcp_server_recv_msg(&msg);
        if (rc < 0) {
            ERROR("Error tcp_server_recv_msg() rc=%ld\n", rc);
            keep_running = 0;
            goto cleanup;
        }

        if (!strncmp(msg.netbuf, "stop", strlen("stop")))
            keep_running = 0;

        else {
            rc = str_to_write_type(msg.netbuf);
            if (rc < 0) {
                ERROR("Invalid write type: %s\n", (char *) msg.netbuf);
                goto cleanup;
            }

            filename = "/root/data";

            PROFILE_NESTED_TICK();
            rc = create_file_write_data(filename, 4 * 1024 * 1024, rc);
            if (rc) {
                ERROR("Error creating file '%s' errno=%d\n", filename, errno);
                keep_running = 0;
            }
            PROFILE_NESTED_TOCK_MSEC("create_file_data");
        }
cleanup:
        net_msg_cleanup(&msg);
    }

    tcp_server_stop(&server);
out:
    return rc;
}

static long test_server()
{
    long rc;
    int is_child = 0;

    rc = server_prologue(&is_child);
    if (rc) {
        ERROR("Error server_prologue() rc=%ld\n", rc);
        goto out;
    }

    if (do_fork) {
        if (is_child)
            rc = run_server();

        else {
            /* parent */
            /* TODO should we keep running? */
            while (1) {
                //printf("VM id=%d\n", os_get_self_id());
                sleep(1);
            }
        }
    } else
        rc = run_server();
out:
    return rc;
}

enum test_type {
    TEST_SIMPLE,
    TEST_SERVER
};
enum test_type my_test_type = TEST_SERVER;

void *thread_func_files(void *p)
{
    long rc = -1;

    (void) p;

    switch (my_test_type) {
    case TEST_SIMPLE:
        rc = simple_test();
        break;
    case TEST_SERVER:
        rc = test_server();
        break;
    default:
        break;
    }

    INFO("Exiting\n");
    return (void *) rc;
}
