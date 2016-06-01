/*
 * Copyright 2010 Rockchip Electronics S.LSI Co. LTD
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
 */

#ifndef _VPU_MEM_POOL_H_
#define _VPU_MEM_POOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ENABLE_VPU_MEMORY_POOL_ALLOCATOR    1
#define VPU_MEMORY_POOL_MANAGER_ENABLE      1

#define vpu_display_mem_pool_FIELDS \
    int (*commit_hdl)(vpu_display_mem_pool *p, int hdl, int size, unsigned int buf_index); \
    void* (*get_free)(vpu_display_mem_pool *p); \
    int (*inc_used)(vpu_display_mem_pool *p, int hdl); \
    int (*put_used)(vpu_display_mem_pool *p, int hdl); \
    int (*reset)(vpu_display_mem_pool *p); \
    int (*get_unused_num)(vpu_display_mem_pool *p); \
    int buff_size;\
    float version; \
    int res[18];

typedef struct vpu_display_mem_pool vpu_display_mem_pool;
struct vpu_display_mem_pool {
    vpu_display_mem_pool_FIELDS
};

vpu_display_mem_pool* open_vpu_memory_pool();
void close_vpu_memory_pool(vpu_display_mem_pool *p);

#ifdef __cplusplus
}
#endif

#endif
