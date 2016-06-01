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

#ifndef __VPU_MEM_H__
#define __VPU_MEM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "vpu_type.h"
/* Linear memory area descriptor */

typedef struct VPUMem {
    RK_S32  index;
    RK_U32  phy_addr;
    RK_U32  *vir_addr;
    RK_U32  size;
    RK_S32  offset;
} VPUMemLinear_t;

#define VPU_MEM_IS_NULL(p)          ((p)->offset < 0)

/* SW/HW shared memory */
RK_S32 VPUMallocLinearOutside(VPUMemLinear_t * p, RK_U32 size); /* exclude mvc data inside the libvpu */
RK_S32 VPUFreeLinear(VPUMemLinear_t *p);
RK_S32 VPUMemDuplicate(VPUMemLinear_t *dst, VPUMemLinear_t *src);
RK_S32 VPUMemGetFD(VPUMemLinear_t *p);
RK_S32 VPUMemLink(VPUMemLinear_t *p);
RK_S32 VPUMemFlush(VPUMemLinear_t *p);
RK_S32 VPUMemClean(VPUMemLinear_t *p);
RK_S32 VPUMemInvalidate(VPUMemLinear_t *p);
RK_U32 *VPUMemVirtual(VPUMemLinear_t *p);
RK_S32 VPUMemPoolSet(RK_U32 size, RK_U32 count);
RK_S32 VPUMemPoolUnset(RK_U32 size, RK_U32 count);
RK_S32 VPUMemGetFreeSize();

// new malloc interface by with allocator for memory management
typedef void*   RK_HANDLE;
typedef struct {
    RK_U32      phy_addr;
    RK_U32     *vir_addr;
    int         fd;
    RK_HANDLE   handle;
} VPUMemHnd;

typedef enum {
    VPU_CACHE_FLUSH,
    VPU_CACHE_CLEAN,
    VPU_CACHE_INVALID,
    VPU_CACHE_OP_NUM,
} VPUCacheOp_E;

typedef enum {
    INVALID_NULL_PTR = 0x8000000,
    INVALID_ALLOCATOR,
    INVALID_MEM_HND,
    INVALID_CACHE_OP,
} VPUMemError;

// maxSize equal to zero means no limit
RK_HANDLE VPUMemAllocatorCreate(RK_U32 maxSize);
RK_S32 VPUMemMalloc(RK_HANDLE allocator, RK_U32 size, RK_S32 timeout, VPUMemHnd **p);
RK_S32 VPUMemImport(RK_HANDLE allocator, RK_S32 mem_fd, VPUMemHnd **p);
RK_S32 VPUMemFree(VPUMemHnd *p);
RK_S32 VPUMemShare(VPUMemHnd *p, RK_S32 *mem_fd);
RK_S32 VPUMemCache(VPUMemHnd *p, VPUCacheOp_E cmd);
void VPUMemAllocatorDestory(RK_HANDLE allocator);

#ifdef __cplusplus
}

#endif

#endif /* __VPU_MEM_H__ */
