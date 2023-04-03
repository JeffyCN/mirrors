/*
 * v4l2_buffer_proxy.h - v4l2 buffer proxy
 *
 *  Copyright (c) 2014-2015 Intel Corporation
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#ifndef XCAM_V4L2_BUFFER_PROXY_H
#define XCAM_V4L2_BUFFER_PROXY_H

#include <xcam_std.h>
#include <buffer_pool.h>
#include <linux/videodev2.h>

namespace XCam {

class V4l2Device;

class V4l2Buffer
    : public BufferData
{
public:
    explicit V4l2Buffer (const struct v4l2_buffer &buf, const struct v4l2_format &format);
    virtual ~V4l2Buffer ();

    const struct v4l2_buffer & get_buf () const {
        return _buf;
    }

    void set_timestamp (const struct timeval &time) {
        _buf.timestamp = time;
    }

    void set_timecode (const struct v4l2_timecode &code) {
        _buf.timecode = code;
    }

    void set_sequence (const uint32_t sequence) {
        _buf.sequence = sequence;
    }

    void set_length (const uint32_t value) {
        _length = value;
    }

    uint32_t get_length () const {
        return _length;
    }

    void set_queued (bool queued) {
        if (queued)
            _queued = 1;
        else
            _queued = 0;
    }

    bool get_queued () const {
        return !!_queued;
    }

    void set_expbuf_fd (const int fd) {
        _expbuf_fd = fd;
    }

    int get_expbuf_fd () {
        return _expbuf_fd;
    }

    void set_expbuf_usrptr(uintptr_t ptr) {
        _expbuf_usrptr = ptr;
    }

    uintptr_t get_expbuf_usrptr () {
        return _expbuf_usrptr;
    }

    void reset () {
        xcam_mem_clear (_buf.timestamp);
        xcam_mem_clear (_buf.timecode);
        _buf.sequence = 0;
        _queued = 0;
    }

    const struct v4l2_format & get_format () const {
        return _format;
    }

    // derived from BufferData
    virtual uint8_t *map ();
    virtual bool unmap ();
    virtual int get_fd ();
    void set_reserved(uintptr_t reserved) {
        _reserved = reserved;
    }
    uintptr_t get_reserved() {
        return _reserved;
    }

private:
    XCAM_DEAD_COPY (V4l2Buffer);

private:
    struct v4l2_buffer  _buf;
    struct v4l2_format  _format;
    int _length;
    int _expbuf_fd;
    uintptr_t _expbuf_usrptr;
    std::atomic<char> _queued;
    uintptr_t _reserved;
};

class V4l2BufferProxy
    : public BufferProxy
{
public:
    explicit V4l2BufferProxy (SmartPtr<V4l2Buffer> &buf, SmartPtr<V4l2Device> &device);

    ~V4l2BufferProxy ();

    int get_v4l2_buf_index () {
        return get_v4l2_buf().index;
    }

    enum v4l2_memory get_v4l2_mem_type () {
        return (enum v4l2_memory)(get_v4l2_buf().memory);
    }

    int get_v4l2_buf_length () {
        return get_v4l2_buf().length;
    }

    int get_v4l2_buf_planar_length (int planar_index) {
        return get_v4l2_buf().m.planes[planar_index].length;
    }

    int get_expbuf_fd () {
        SmartPtr<V4l2Buffer> v4l2buf = get_buffer_data().dynamic_cast_ptr<V4l2Buffer> ();
        XCAM_ASSERT (v4l2buf.ptr());
        return v4l2buf->get_expbuf_fd();
    }

    uintptr_t get_v4l2_userptr () {
    #if 0
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == get_v4l2_buf().type ||
                V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == get_v4l2_buf().type)
            return get_v4l2_buf().m.planes[0].m.userptr;
        else
            return get_v4l2_buf().m.userptr;
    #else
        SmartPtr<V4l2Buffer> v4l2buf = get_buffer_data().dynamic_cast_ptr<V4l2Buffer> ();
        XCAM_ASSERT (v4l2buf.ptr());
        return v4l2buf->get_expbuf_usrptr();
    #endif
    }

    uintptr_t get_v4l2_planar_userptr (int planar_index) {
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == get_v4l2_buf().type ||
                V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == get_v4l2_buf().type)
            return get_v4l2_buf().m.planes[planar_index].m.userptr;
        else
            return get_v4l2_buf().m.userptr;
    }

    uint32_t get_sequence () {
        return get_v4l2_buf().sequence;
    }

    uintptr_t get_reserved() {
        SmartPtr<V4l2Buffer> v4l2buf = get_buffer_data().dynamic_cast_ptr<V4l2Buffer> ();
        XCAM_ASSERT (v4l2buf.ptr());
        return v4l2buf->get_reserved();
    }

private:
    const struct v4l2_buffer & get_v4l2_buf ();

    void v4l2_format_to_video_info (
        const struct v4l2_format &format, VideoBufferInfo &info);

    XCAM_DEAD_COPY (V4l2BufferProxy);

private:
    SmartPtr<V4l2Device>  _device;
};
}

#endif //XCAM_V4L2_BUFFER_PROXY_H
