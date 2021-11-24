/*
 *  sync.c
 *
 *   Copyright 2012 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <poll.h>
#include <stdint.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/sync_file.h>

#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
using namespace std;
#endif

#include "rga_sync.h"

/* Legacy Sync API */
struct sync_legacy_merge_data {
 int32_t fd2;
 char name[32];
 int32_t fence;
};

/**
 * DOC: SYNC_IOC_MERGE - merge two fences
 *
 * Takes a struct sync_merge_data.  Creates a new fence containing copies of
 * the sync_pts in both the calling fd and sync_merge_data.fd2.  Returns the
 * new fence's fd in sync_merge_data.fence
 *
 * This is the legacy version of the Sync API before the de-stage that happened
 * on Linux kernel 4.7.
 */
#define SYNC_IOC_LEGACY_MERGE   _IOWR(SYNC_IOC_MAGIC, 1, \
    struct sync_legacy_merge_data)

// ---------------------------------------------------------------------------
// Support for caching the sync uapi version.
//
// This library supports both legacy (android/staging) uapi and modern
// (mainline) sync uapi. Library calls first try one uapi, and if that fails,
// try the other. Since any given kernel only supports one uapi version, after
// the first successful syscall we know what the kernel supports and can skip
// trying the other.
enum uapi_version {
    UAPI_UNKNOWN,
    UAPI_MODERN,
    UAPI_LEGACY
};
static atomic_int g_uapi_version = ATOMIC_VAR_INIT(UAPI_UNKNOWN);

static size_t rga_strlcpy(char *dest, const char *src, size_t size) {
    size_t src_len = strlen(src);
    size_t len = 0;

    if (size) {
        len = src_len >= size ? (size - 1) : src_len;
        // strncpy(dest, src, len);
        memcpy(dest, src, len);
        dest[len] = '\0';
    }

    return src_len;
}

/* sync wait */
int rga_sync_wait(int fd, int timeout)
{
    struct pollfd fds;
    int ret;

    if (fd < 0) {
        errno = EINVAL;
        return -1;
    }

    fds.fd = fd;
    fds.events = POLLIN;

    do {
        ret = poll(&fds, 1, timeout);
        if (ret > 0) {
            if (fds.revents & (POLLERR | POLLNVAL)) {
                errno = EINVAL;
                return -1;
            }
            return 0;
        } else if (ret == 0) {
            errno = ETIME;
            return -1;
        }
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));

    return ret;
}

/* sync merge */
static int legacy_sync_merge(const char *name, int fd1, int fd2)
{
    struct sync_legacy_merge_data data;
    int ret;

    data.fd2 = fd2;
    rga_strlcpy(data.name, name, sizeof(data.name));
    ret = ioctl(fd1, SYNC_IOC_LEGACY_MERGE, &data);
    if (ret < 0)
        return ret;
    return data.fence;
}

static int modern_sync_merge(const char *name, int fd1, int fd2)
{
    struct sync_merge_data data;
    int ret;

    data.fd2 = fd2;
    rga_strlcpy(data.name, name, sizeof(data.name));
    data.flags = 0;
    data.pad = 0;

    ret = ioctl(fd1, SYNC_IOC_MERGE, &data);
    if (ret < 0)
        return ret;
    return data.fence;
}

int rga_sync_merge(const char *name, int fd1, int fd2)
{
    int uapi;
    int ret;

    uapi = atomic_load_explicit(&g_uapi_version, memory_order_acquire);

    if (uapi == UAPI_MODERN || uapi == UAPI_UNKNOWN) {
        ret = modern_sync_merge(name, fd1, fd2);
        if (ret >= 0 || errno != ENOTTY) {
            if (ret >= 0 && uapi == UAPI_UNKNOWN) {
                atomic_store_explicit(&g_uapi_version, (int)UAPI_MODERN,
                                      memory_order_release);
            }
            return ret;
        }
    }

    ret = legacy_sync_merge(name, fd1, fd2);
    if (ret >= 0 && uapi == UAPI_UNKNOWN) {
        atomic_store_explicit(&g_uapi_version, (int)UAPI_LEGACY,
                              memory_order_release);
    }
    return ret;
}
