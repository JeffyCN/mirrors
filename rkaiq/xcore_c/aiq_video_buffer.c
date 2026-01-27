/*
 * aiq_video_buffer.c - video buffer base
 *
 *  Copyright (c) 2023 Rockchip Corporation
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
#include "aiq_video_buffer.h"

#include "aiq_base.h"

XCAM_BEGIN_DECLARE

void AiqVideoBuffer_init(aiq_VideoBuffer_t* vb, const AiqVideoBufferInfo* info, int64_t timestamp) {
    aiq_memset(vb, 0, sizeof(aiq_VideoBuffer_t));
    if (info) {
        vb->_xcam_vb.info = *info;
    }
}

bool AiqVideoBufferInfo_fill(AiqVideoBufferInfo* vbi, const XCamVideoBufferInfo* info) {
    int i = 0;

    vbi->format         = info->format;
    vbi->color_bits     = info->color_bits;
    vbi->width          = info->width;
    vbi->height         = info->height;
    vbi->aligned_width  = info->aligned_width;
    vbi->aligned_height = info->aligned_height;
    vbi->size           = info->size;
    vbi->components     = info->components;
    for (i = 0; i < XCAM_VIDEO_MAX_COMPONENTS; i++) {
        vbi->strides[i] = info->strides[i];
        vbi->offsets[i] = info->offsets[i];
    }

    return true;
}

uint8_t* AiqVideoBuffer_map(aiq_VideoBuffer_t* vb) {
    if (vb->_xcam_vb.map) return (vb->_xcam_vb.map)((XCamVideoBuffer*)vb);
    return NULL;
}

bool AiqVideoBuffer_unmap(aiq_VideoBuffer_t* vb) {
    if (vb->_xcam_vb.unmap) (vb->_xcam_vb.unmap)((XCamVideoBuffer*)vb);
    return true;
}

int AiqVideoBuffer_getFd(aiq_VideoBuffer_t* vb) {
    if (vb->_xcam_vb.get_fd) return (vb->_xcam_vb.get_fd)((XCamVideoBuffer*)vb);
    return -1;
}

XCAM_END_DECLARE
