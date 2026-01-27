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

#ifndef C_AIQ_V4L2_BUFFER_H
#define C_AIQ_V4L2_BUFFER_H

#include <linux/videodev2.h>
#include <xcore_c/aiq_video_buffer.h>

XCAM_BEGIN_DECLARE

struct AiqV4l2Device_s;

typedef struct AiqV4l2Buffer_s {
    aiq_VideoBuffer_t _vb;
    struct v4l2_buffer _buf;
    struct v4l2_format _format;
    int _length;
    int _expbuf_fd;
    uintptr_t _expbuf_usrptr;
    bool _queued;
    struct AiqV4l2Device_s* _device;
    uint8_t* _reserved;
    uint32_t _ref_cnts;
} AiqV4l2Buffer_t;

void AiqV4l2Buffer_init(AiqV4l2Buffer_t* v4lBuf, const struct v4l2_buffer* buf,
                        const struct v4l2_format* format, struct AiqV4l2Device_s* device);

#define AiqV4l2Buffer_getBuf(v4lBuf) (&(v4lBuf)->_buf)

#define AiqV4l2Buffer_setTimestamp(v4lBuf, time) \
    v4lBuf->_buf.timestamp = time;               \
    AiqVideoBuffer_setTimestamp(&(v4lBuf)->_vb, XCAM_TIMEVAL_2_USEC(time))

#define AiqV4l2Buffer_getTimestamp(v4lBuf) AiqVideoBuffer_getTimestamp(&(v4lBuf)->_vb)

#define AiqV4l2Buffer_setTimecode(v4lBuf, code) ((v4lBuf)->_buf.timecode = code)

#define AiqV4l2Buffer_setSequence(v4lBuf, seq) \
    (v4lBuf)->_buf.sequence = seq;             \
    AiqVideoBuffer_setSequence(&(v4lBuf)->_vb, seq)

#define AiqV4l2Buffer_getSequence(v4lBuf) AiqVideoBuffer_getSequence(&(v4lBuf)->_vb)

#define AiqV4l2Buffer_setLength(v4lBuf, value) ((v4lBuf)->_length = value)

#define AiqV4l2Buffer_getLength(v4lBuf) ((v4lBuf)->_length)

#define AiqV4l2Buffer_setQueued(v4lBuf, queued) ((v4lBuf)->_queued = queued)

#define AiqV4l2Buffer_getQueued(v4lBuf) ((v4lBuf)->_queued)

#define AiqV4l2Buffer_setExpbufFd(v4lBuf, fd) ((v4lBuf)->_expbuf_fd = fd)

#define AiqV4l2Buffer_getExpbufFd(v4lBuf) ((v4lBuf)->_expbuf_fd)

#define AiqV4l2Buffer_setExpbufUsrptr(v4lBuf, ptr) ((v4lBuf)->_expbuf_usrptr = ptr)

#define AiqV4l2Buffer_getExpbufUsrptr(v4lBuf) ((v4lBuf)->_expbuf_usrptr)

uintptr_t AiqV4l2Buffer_getPlanarUsrptr(AiqV4l2Buffer_t* v4lBuf, int planar_index);
void AiqV4l2Buffer_reset(AiqV4l2Buffer_t* v4lBuf);

#define AiqV4l2Buffer_getFmt(v4lBuf) (&(v4lBuf)->_format)

#define AiqV4l2Buffer_map(v4lBuf) AiqVideoBuffer_map(&(v4lBuf)->_vb)

#define AiqV4l2Buffer_unmap(v4lBuf) AiqVideoBuffer_unmap(&(v4lBuf)->_vb)

#define AiqV4l2Buffer_getFd(v4lBuf) AiqVideoBuffer_getFd(&(v4lBuf)->_vb)

#define AiqV4l2Buffer_setReserved(v4lBuf, reserved) ((v4lBuf)->_reserved = reserved)

#define AiqV4l2Buffer_getReserved(v4lBuf) ((v4lBuf)->_reserved)

#define AiqV4l2Buffer_getV4lBufLength(v4lBuf) ((v4lBuf)->_buf.length)

#define AiqV4l2Buffer_getV4lBufPlanarLength(v4lBuf, planar_index) \
    ((v4lBuf)->_buf.m.planes[planar_index].length)

#define AiqV4l2Buffer_getV4lBufIndex(v4lBuf) ((v4lBuf)->_buf.index)

#define AiqV4l2Buffer_getV4lMemtype(v4lBuf) ((enum v4l2_memory)((v4lBuf)->_buf.memory))

#define AiqV4l2Buffer_ref(v4lBuf) xcam_video_buffer_ref((XCamVideoBuffer*)(v4lBuf))

#define AiqV4l2Buffer_unref(v4lBuf) xcam_video_buffer_unref((XCamVideoBuffer*)(v4lBuf))

XCAM_END_DECLARE

#endif  // C_AIQ_V4L2_BUFFER_H
