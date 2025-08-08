/*
 * aiq_video_buffer.h - video buffer base
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

#ifndef C_AIQ_VIDEO_BUFFER_H
#define C_AIQ_VIDEO_BUFFER_H

#include <base/xcam_buffer.h>

XCAM_BEGIN_DECLARE

typedef XCamVideoBufferPlanarInfo AiqVideoBufferPlanarInfo;
typedef XCamVideoBufferInfo AiqVideoBufferInfo;

#define AiqVideoBufferInfo_init          xcam_video_buffer_info_reset
#define AiqVideoBufferInfo_getPlanarInfo xcam_video_buffer_get_planar_info
#define AiqVideoBufferInfo_isValid(vbi) \
    (vbi->format && vbi->aligned_width && vbi->aligned_height && vbi->size)

bool AiqVideoBufferInfo_fill(AiqVideoBufferInfo* vbi, const XCamVideoBufferInfo* info);

typedef struct aiq_VideoBuffer_s {
    XCamVideoBuffer _xcam_vb;
    int _buf_type;
} aiq_VideoBuffer_t;

void AiqVideoBuffer_init(aiq_VideoBuffer_t* vb, const AiqVideoBufferInfo* info, int64_t timestamp);
uint8_t* AiqVideoBuffer_map(aiq_VideoBuffer_t* vb);
bool AiqVideoBuffer_unmap(aiq_VideoBuffer_t* vb);
int AiqVideoBuffer_getFd(aiq_VideoBuffer_t* vb);

#define AiqVideoBuffer_getVbInfo(vb) ((vb)->_xcam_vb.info)

#define AiqVideoBuffer_setVideoInfo(vb, pinfo) ((vb)->_xcam_vb.info = *pinfo)

#define AiqVideoBuffer_setTimestamp(vb, time) ((vb)->_xcam_vb.timestamp = time)

#define AiqVideoBuffer_getTimestamp(vb) ((vb)->_xcam_vb.timestamp)

#define AiqVideoBuffer_getSize(vb) ((vb)->_xcam_vb.info.size)

#define AiqVideoBuffer_setSequence(vb, sequence) (vb)->_xcam_vb.frame_id = sequence

#define AiqVideoBuffer_getSequence(vb) ((vb)->_xcam_vb.frame_id)

#define convert_to_external_buffer(vb) (&(vb)->_xcam_vb)

#define AiqVideoBuffer_ref(vb) xcam_video_buffer_ref((XCamVideoBuffer*)(vb))

#define AiqVideoBuffer_unref(vb) xcam_video_buffer_unref((XCamVideoBuffer*)(vb))

XCAM_END_DECLARE

#endif  // C_AIQ_VIDEO_BUFFER_H
