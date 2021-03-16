/*
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Zhiqin Wei <wzq@rock-chips.com>
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

#include "RgaApi.h"

#include "RockchipRga.h"

#ifdef ANDROID
using namespace android;
#endif

int c_RkRgaInit() {
    return 0;
}

void c_RkRgaDeInit() {}

void c_RkRgaGetContext(void **ctx) {
    RockchipRga& c_rkRga(RockchipRga::get());

    c_rkRga.RkRgaGetContext(ctx);
}

int c_RkRgaBlit(rga_info_t *src, rga_info_t *dst, rga_info_t *src1) {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaBlit(src, dst, src1);
}

int c_RkRgaColorFill(rga_info_t *dst) {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaCollorFill(dst);
}

int c_RkRgaFlush() {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaFlush();
}

#ifndef ANDROID /* linux */
int c_RkRgaGetAllocBuffer(bo_t *bo_info, int width, int height, int bpp) {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaGetAllocBuffer(bo_info, width, height, bpp);
}

int c_RkRgaGetAllocBufferCache(bo_t *bo_info, int width, int height, int bpp) {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaGetAllocBufferCache(bo_info, width, height, bpp);
}

int c_RkRgaGetMmap(bo_t *bo_info) {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaGetMmap(bo_info);
}

int c_RkRgaUnmap(bo_t *bo_info) {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaUnmap(bo_info);
}

int c_RkRgaFree(bo_t *bo_info) {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaFree(bo_info);
}

int c_RkRgaGetBufferFd(bo_t *bo_info, int *fd) {
    RockchipRga& c_rkRga(RockchipRga::get());

    return c_rkRga.RkRgaGetBufferFd(bo_info, fd);
}
#endif /* #ifndef Andorid */

