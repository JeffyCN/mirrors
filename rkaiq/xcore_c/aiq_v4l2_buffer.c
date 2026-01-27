/*
 * aiq_v4l2_buffer.h
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

#include "aiq_v4l2_buffer.h"

#include "aiq_base.h"
#include "aiq_mutex.h"
#include "aiq_v4l2_device.h"
#include "linux/rk-video-format.h"

XCAM_BEGIN_DECLARE

static void _v4l2_format_to_video_info(const struct v4l2_format* format,
                                       XCamVideoBufferInfo* info) {
    if (format->type == V4L2_BUF_TYPE_META_CAPTURE || format->type == V4L2_BUF_TYPE_META_OUTPUT) {
        info->format = format->fmt.meta.dataformat;
    } else {
        info->format = format->fmt.pix.pixelformat;
    }
    info->color_bits     = 8;
    info->width          = format->fmt.pix.width;
    info->height         = format->fmt.pix.height;
    info->aligned_width  = 0;
    info->aligned_height = 0;
    info->size           = format->fmt.pix.sizeimage;
    switch (info->format) {
        case V4L2_PIX_FMT_NV12:  // 420
        case V4L2_PIX_FMT_NV21:
            info->components = 2;
            info->strides[0] = format->fmt.pix.bytesperline * 2 / 3;
            info->strides[1] = info->strides[0];
            info->offsets[0] = 0;
            info->offsets[1] = info->strides[0] * format->fmt.pix.height;
            break;
        case V4L2_PIX_FMT_YUV422P:  // 422 Planar
            info->components = 3;
            info->strides[0] = format->fmt.pix.bytesperline / 2;
            info->strides[1] = info->strides[0] / 2;
            info->strides[2] = info->strides[0] / 2;
            info->offsets[0] = 0;
            info->offsets[1] = info->strides[0] * format->fmt.pix.height;
            info->offsets[2] = info->offsets[1] + info->strides[1] * format->fmt.pix.height;
            break;
        case V4L2_PIX_FMT_YUYV:  // 422
            info->components    = 1;
            info->strides[0]    = format->fmt.pix.bytesperline;
            info->offsets[0]    = 0;
            info->aligned_width = info->strides[0] / 2;
            break;
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SRGGB10:
            info->color_bits = 10;
            info->components = 1;
            info->strides[0] = format->fmt.pix.bytesperline;
            info->offsets[0] = 0;
            break;
        case V4L2_PIX_FMT_SBGGR12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SRGGB12:
            info->color_bits = 12;
            info->components = 1;
            info->strides[0] = format->fmt.pix.bytesperline;
            info->offsets[0] = 0;
            break;
        case V4L2_PIX_FMT_SBGGR16:
        case V4L2_PIX_FMT_SGBRG16:
        case V4L2_PIX_FMT_SGRBG16:
        case V4L2_PIX_FMT_SRGGB16:
            info->color_bits = 16;
            info->components = 1;
            info->strides[0] = info->width * 2;
            info->offsets[0] = 0;
            break;
        case V4L2_META_FMT_RK_ISP1_PARAMS:
        case V4L2_META_FMT_RK_ISP1_STAT_3A:
        case V4L2_META_FMT_RK_ISPP_PARAMS:
        case V4L2_META_FMT_RK_ISPP_STAT:
        case V4L2_META_FMT_RK_ISP1_STAT_LUMA:
        case V4L2_PIX_FMT_FBC2:
        case V4L2_PIX_FMT_FBC0:
            // TODO
            break;
        default:
            XCAM_LOG_WARNING("unknown v4l2 format(%s) to video info",
                             xcam_fourcc_to_string(format->fmt.pix.pixelformat));
            break;
    }

    if (!info->aligned_width) info->aligned_width = info->strides[0];

    if (!info->aligned_height) info->aligned_height = info->height;
}

static uint8_t* _AiqV4l2Buffer_map(XCamVideoBuffer* buf) {
    AiqV4l2Buffer_t* pAiqV4l2Buf = (AiqV4l2Buffer_t*)buf;

    if (pAiqV4l2Buf->_buf.memory == V4L2_MEMORY_DMABUF) return NULL;

    if (pAiqV4l2Buf->_expbuf_usrptr) return (uint8_t*)(pAiqV4l2Buf->_expbuf_usrptr);

    if (pAiqV4l2Buf->_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ||
        pAiqV4l2Buf->_buf.type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
        return (uint8_t*)(pAiqV4l2Buf->_buf.m.planes[0].m.userptr);
    else
        return (uint8_t*)(pAiqV4l2Buf->_buf.m.userptr);
}

static void _AiqV4l2Buffer_unmap(XCamVideoBuffer* buf) { return; }

static void _AiqV4l2Buffer_ref(XCamVideoBuffer* buf) {
    AiqV4l2Buffer_t* pAiqV4l2Buf = (AiqV4l2Buffer_t*)buf;

    if (pAiqV4l2Buf->_device) aiqMutex_lock(&pAiqV4l2Buf->_device->_buf_mutex);
    pAiqV4l2Buf->_ref_cnts++;
    if (pAiqV4l2Buf->_device) aiqMutex_unlock(&pAiqV4l2Buf->_device->_buf_mutex);
}

static uint32_t _AiqV4l2Buffer_unref(XCamVideoBuffer* buf) {
    AiqV4l2Buffer_t* pAiqV4l2Buf = (AiqV4l2Buffer_t*)buf;
    uint32_t ref_cnts            = 0;

    if (pAiqV4l2Buf->_device) aiqMutex_lock(&pAiqV4l2Buf->_device->_buf_mutex);
    if (pAiqV4l2Buf->_ref_cnts > 0) pAiqV4l2Buf->_ref_cnts--;
    ref_cnts = pAiqV4l2Buf->_ref_cnts;
    if (pAiqV4l2Buf->_device) aiqMutex_unlock(&pAiqV4l2Buf->_device->_buf_mutex);

    if (ref_cnts == 0 && pAiqV4l2Buf->_device)
        AiqV4l2Device_returnBuf(pAiqV4l2Buf->_device, pAiqV4l2Buf);

    return ref_cnts;
}

static int _AiqV4l2Buffer_getfd(XCamVideoBuffer* buf) {
    AiqV4l2Buffer_t* pAiqV4l2Buf = (AiqV4l2Buffer_t*)buf;

    return pAiqV4l2Buf->_expbuf_fd;
}

void AiqV4l2Buffer_init(AiqV4l2Buffer_t* v4lBuf, const struct v4l2_buffer* buf,
                        const struct v4l2_format* format, AiqV4l2Device_t* device) {
    XCamVideoBufferInfo info;

    aiq_memset(v4lBuf, 0, sizeof(AiqV4l2Buffer_t));
    v4lBuf->_buf    = *buf;
    v4lBuf->_format = *format;

    _v4l2_format_to_video_info(&v4lBuf->_format, &info);

    AiqVideoBuffer_setVideoInfo(&v4lBuf->_vb, &info);
    AiqVideoBuffer_setTimestamp(&v4lBuf->_vb, XCAM_TIMEVAL_2_USEC(v4lBuf->_buf.timestamp));
    AiqVideoBuffer_setSequence(&v4lBuf->_vb, v4lBuf->_buf.sequence);

    v4lBuf->_device    = device;
    v4lBuf->_expbuf_fd = -1;

    v4lBuf->_vb._xcam_vb.ref    = _AiqV4l2Buffer_ref;
    v4lBuf->_vb._xcam_vb.unref  = _AiqV4l2Buffer_unref;
    v4lBuf->_vb._xcam_vb.map    = _AiqV4l2Buffer_map;
    v4lBuf->_vb._xcam_vb.unmap  = _AiqV4l2Buffer_unmap;
    v4lBuf->_vb._xcam_vb.get_fd = _AiqV4l2Buffer_getfd;
}

uintptr_t AiqV4l2Buffer_getPlanarUsrptr(AiqV4l2Buffer_t* v4lBuf, int planar_index) {
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4lBuf->_buf.type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4lBuf->_buf.type)
        return v4lBuf->_buf.m.planes[planar_index].m.userptr;
    else
        return v4lBuf->_buf.m.userptr;
}

void AiqV4l2Buffer_reset(AiqV4l2Buffer_t* v4lBuf) {
    xcam_mem_clear(v4lBuf->_buf.timestamp);
    AiqVideoBuffer_setTimestamp((&v4lBuf->_vb), 0);
    xcam_mem_clear(v4lBuf->_buf.timecode);
    v4lBuf->_buf.sequence = 0;
    AiqVideoBuffer_setSequence((&v4lBuf->_vb), 0);
    v4lBuf->_queued = 0;
}

XCAM_END_DECLARE
