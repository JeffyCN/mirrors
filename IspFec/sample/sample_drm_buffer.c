/*
 *  Copyright (c) 2025 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "sample_drm_buffer.h"

#include <drm/drm_mode.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <rockchip_drm.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>

#define DRM_CARD_PATH "/dev/dri/card0"

int init_drm(void) {
    int fd;

    fd = open(DRM_CARD_PATH, O_RDWR);
    if (fd < 0) printf("failed to open drm !\n");

    return fd;
}

void deinit_drm(int dma_node_fd) {
    if (dma_node_fd >= 0) close(dma_node_fd);
}

int alloc_drm_buffer(int fd, uint32_t size, struct rkfec_buffer* buf) {
    int ret = -1;
    size_t min_size;
    struct drm_rockchip_gem_create req = {
        .size = size,
        // .flags = 1,
    };
    struct drm_rockchip_gem_map_off map_req;

    /* cma must alloc at least two page */
    min_size = 2 * getpagesize();
    req.size = size < min_size ? min_size : size;

    memset(&map_req, 0x00, sizeof(map_req));

    ret = drmIoctl(fd, DRM_IOCTL_ROCKCHIP_GEM_CREATE, &req);
    if (ret) {
        printf("failed to GEM_CREATE req.size %" PRId64 "\n", req.size);
        return -1;
    }

    ret = drmPrimeHandleToFD(fd, req.handle, 0, &buf->dmabuf_fd);
    if (ret) {
        printf("failed to get dma dma_node_fd.\n");
        goto error;
    }

    map_req.handle = req.handle;
    ret            = drmIoctl(fd, DRM_IOCTL_ROCKCHIP_GEM_MAP_OFFSET, &map_req);
    if (ret) {
        printf("failed to ioctl gem map offset.");
        goto error;
    }

    printf("handle = %u, dma_fd = %d, alloc_size = %u, real_size = %" PRId64 "\n", req.handle,
           buf->dmabuf_fd, size, req.size);

#ifdef __ANDROID__
    buf->map = mmap64(0, req.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_req.offset);
#else
    buf->map = mmap(0, req.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_req.offset);
#endif
    if (buf->map == MAP_FAILED) {
        printf("failed to mmap buffer. offset = %" PRId64 ", reason: %s\n", map_req.offset,
               strerror(errno));
        ret = -1;
        goto error;
    }

    buf->handle = req.handle;
    buf->size   = size;

    return ret;
error:
    drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &req);

    return ret;
}

int free_drm_buffer(int fd, struct rkfec_buffer* buf) {
    struct drm_gem_close req;
    size_t min_size;
    min_size = 2 * getpagesize();

    if (!buf || buf->size == 0) return -1;

    memset(&req, 0x00, sizeof(req));

    req.handle = buf->handle;

    if (buf->map) munmap(buf->map, buf->size < min_size ? min_size : buf->size);

    if (buf->dmabuf_fd >= 0) close(buf->dmabuf_fd);

    drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, &req);

    return 0;
}
