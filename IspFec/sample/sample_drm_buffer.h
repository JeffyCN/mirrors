/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _SAMPLE_DRM_BUFFER_H_
#define _SAMPLE_DRM_BUFFER_H_

#include <stdint.h>

#ifndef RKFEC_HW_V20
struct rkfec_buffer {
    uint32_t handle;
    int dmabuf_fd;
    uint64_t size;
    void* map;
};
#endif

int init_drm(void);
void deinit_drm(int dma_node_fd);
int alloc_drm_buffer(int fd, uint32_t size, struct rkfec_buffer* buf);
int free_drm_buffer(int fd, struct rkfec_buffer* buf);

#endif  // _SAMPLE_DRM_BUFFER_H_