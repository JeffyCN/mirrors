/*
 * v4l2_device.cpp - v4l2 device
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
 * Author: John Ye <john.ye@intel.com>
 */

#include "v4l2_device.h"
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <sys/mman.h>

#include "v4l2_buffer_proxy.h"

namespace XCam {

#define XCAM_V4L2_DEFAULT_BUFFER_COUNT  6

V4l2Device::V4l2Device (const char *name)
    : _name (NULL)
    , _fd (-1)
    , _sensor_id (0)
    , _capture_mode (0)
    , _buf_type (V4L2_BUF_TYPE_VIDEO_CAPTURE)
    , _memory_type (V4L2_MEMORY_MMAP)
    , _planes (NULL)
    , _fps_n (0)
    , _fps_d (0)
    , _active (false)
    , _buf_count (XCAM_V4L2_DEFAULT_BUFFER_COUNT)
    , _queued_bufcnt(0)
{
    if (name)
        _name = strndup (name, XCAM_MAX_STR_SIZE);
    xcam_mem_clear (_format);
}

V4l2Device::~V4l2Device ()
{
    close();
    if (_name)
        xcam_free (_name);
    if (_planes)
        xcam_free (_planes);
}

bool
V4l2Device::set_device_name (const char *name)
{
    XCAM_ASSERT (name);

    if (is_opened()) {
        XCAM_LOG_WARNING ("can't set device name since device opened");
        return false;
    }
    if (_name)
        xcam_free (_name);
    _name = strndup (name, XCAM_MAX_STR_SIZE);
    return true;
}

bool
V4l2Device::set_sensor_id (int id)
{
    if (is_opened()) {
        XCAM_LOG_WARNING ("can't set sensor id since device opened");
        return false;
    }
    _sensor_id = id;
    return true;
}

bool
V4l2Device::set_capture_mode (uint32_t capture_mode)
{
    if (is_opened()) {
        XCAM_LOG_WARNING ("can't set sensor id since device opened");
        return false;
    }
    _capture_mode = capture_mode;
    return true;
}

bool
V4l2Device::set_framerate (uint32_t n, uint32_t d)
{
    if (_format.fmt.pix.pixelformat) {
        XCAM_LOG_WARNING ("device(%s) set framerate failed since formatted was already set.", XCAM_STR(_name));
        return false;
    }

    _fps_n = n;
    _fps_d = d;

    return true;
}

void
V4l2Device::get_framerate (uint32_t &n, uint32_t &d)
{
    n = _fps_n;
    d = _fps_d;
}

bool
V4l2Device::set_mem_type (enum v4l2_memory type) {
    if (is_activated ()) {
        XCAM_LOG_WARNING ("device(%s) set mem type failed", XCAM_STR (_name));
        return false;
    }
    _memory_type = type;
    return true;
}

bool
V4l2Device::set_buf_type (enum v4l2_buf_type type) {
    if (is_activated ()) {
        XCAM_LOG_WARNING ("device(%s) set buf type failed", XCAM_STR (_name));
        return false;
    }
    _buf_type = type;
    return true;
}

bool
V4l2Device::set_buffer_count (uint32_t buf_count)
{
    if (is_activated ()) {
        XCAM_LOG_WARNING ("device(%s) set buffer count failed", XCAM_STR (_name));
        return false;
    }
    _buf_count = buf_count;

    _planes = (struct v4l2_plane *)xcam_malloc0
        (buf_count * FMT_NUM_PLANES * sizeof(struct v4l2_plane));

    return true;
}


XCamReturn
V4l2Device::open ()
{
    struct v4l2_streamparm param;

    if (is_opened()) {
        XCAM_LOG_DEBUG ("device(%s) was already opened", XCAM_STR(_name));
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_name) {
        XCAM_LOG_DEBUG ("v4l2 device open failed, there's no device name");
        return XCAM_RETURN_ERROR_PARAM;
    }
    _fd = ::open (_name, O_RDWR);
    if (_fd == -1) {
        XCAM_LOG_DEBUG ("open device(%s) failed", _name);
        return XCAM_RETURN_ERROR_IOCTL;
    } else {
        XCAM_LOG_DEBUG ("open device(%s) successed, fd: %d", _name, _fd);
    }
#if 0
    // set sensor id
    if (io_control (VIDIOC_S_INPUT, &_sensor_id) < 0) {
        XCAM_LOG_WARNING ("set sensor id(%d) failed but continue", _sensor_id);
    }

    // set capture mode
    xcam_mem_clear (param);
    param.type = _buf_type;
    param.parm.capture.capturemode = _capture_mode;
    if (io_control (VIDIOC_S_PARM, &param) < 0) {
        XCAM_LOG_WARNING ("set capture mode(0x%08x) failed but continue", _capture_mode);
    }
#endif
    struct v4l2_capability cap;
    // only video node cay query cap
    if (_name && strstr(_name, "video"))
        query_cap(cap);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::close ()
{
    if (!is_opened())
        return XCAM_RETURN_NO_ERROR;
    ::close (_fd);
    _fd = -1;

    XCAM_LOG_INFO ("device(%s) closed", XCAM_STR (_name));
    return XCAM_RETURN_NO_ERROR;
}

int
V4l2Device::io_control (int cmd, void *arg)

{
    if (_fd <= 0)
        return -1;

    return xcam_device_ioctl (_fd, cmd, arg);
}

int
V4l2Device::poll_event (int timeout_msec, int stop_fd)
{
    int num_fds = stop_fd == -1 ? 1 : 2;
    struct pollfd poll_fds[num_fds];
    int ret = 0;

    XCAM_ASSERT (_fd > 0);

    memset(poll_fds, 0, sizeof(poll_fds));
    poll_fds[0].fd = _fd;
    poll_fds[0].events = (POLLPRI | POLLIN | POLLERR | POLLNVAL | POLLHUP);

    if (stop_fd != -1) {
        poll_fds[1].fd = stop_fd;
        poll_fds[1].events = POLLPRI | POLLIN;
        poll_fds[1].revents = 0;
    }

    ret = poll (poll_fds, num_fds, timeout_msec);
    if (stop_fd != -1) {
        if ((poll_fds[1].revents & POLLIN) || (poll_fds[1].revents & POLLPRI)) {
            XCAM_LOG_DEBUG ("%s: Poll returning from flush", __FUNCTION__);
            return POLL_STOP_RET;
        }
    }

    if (ret > 0 && (poll_fds[0].revents & (POLLERR | POLLNVAL | POLLHUP))) {
        XCAM_LOG_DEBUG ("v4l2 subdev(%s) polled error", XCAM_STR(_name));
        return -1;
    }

    return ret;

}

XCamReturn
V4l2Device::query_cap (struct v4l2_capability &cap)
{
    int ret = 0;

    XCAM_FAIL_RETURN (ERROR, is_opened (), XCAM_RETURN_ERROR_FILE,
                      "Cannot query cap from v4l2 device while it is closed.");

    ret = io_control(VIDIOC_QUERYCAP, &cap);

    if (ret < 0) {
        XCAM_LOG_ERROR("VIDIOC_QUERYCAP returned: %d (%s)", ret, strerror(errno));
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
        _buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        _buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
        _buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    else if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE)
        _buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    else if (cap.capabilities & V4L2_CAP_META_CAPTURE)
        _buf_type = V4L2_BUF_TYPE_META_CAPTURE;
    else if (cap.capabilities & V4L2_CAP_META_OUTPUT)
        _buf_type = V4L2_BUF_TYPE_META_OUTPUT;
    else {
        XCAM_LOG_ERROR("@%s: unsupported buffer type.", __FUNCTION__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    XCAM_LOG_INFO("------------------------------");
    XCAM_LOG_INFO("driver:       '%s'", cap.driver);
    XCAM_LOG_INFO("card:         '%s'", cap.card);
    XCAM_LOG_INFO("bus_info:     '%s'", cap.bus_info);
    XCAM_LOG_INFO("version:      %x", cap.version);
    XCAM_LOG_INFO("capabilities: %x", cap.capabilities);
    XCAM_LOG_INFO("device caps:  %x", cap.device_caps);
    XCAM_LOG_INFO("buffer type   %d", _buf_type);
    XCAM_LOG_INFO("------------------------------");

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::set_format (struct v4l2_format &format)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    XCAM_FAIL_RETURN (ERROR, !is_activated (), XCAM_RETURN_ERROR_PARAM,
                      "Cannot set format to v4l2 device while it is active.");

    XCAM_FAIL_RETURN (ERROR, is_opened (), XCAM_RETURN_ERROR_FILE,
                      "Cannot set format to v4l2 device while it is closed.");

    struct v4l2_format tmp_format = format;

    ret = pre_set_format (format);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("device(%s) pre_set_format failed", XCAM_STR (_name));
	    //RKTODO::rkisp no need to do set format op for subdev.
        //return ret;
    }

    if (io_control (VIDIOC_S_FMT, &format) < 0) {
        if (errno == EBUSY) {
            // TODO log device name
            XCAM_LOG_ERROR("Video device is busy, fail to set format.");
        } else {
            // TODO log format details and errno
            XCAM_LOG_ERROR("Fail to set format: %s", strerror(errno));
        }

        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (tmp_format.fmt.pix.width != format.fmt.pix.width || tmp_format.fmt.pix.height != format.fmt.pix.height) {
        XCAM_LOG_ERROR (
            "device(%s) set v4l2 format failed, supported format: width:%d, height:%d",
            XCAM_STR (_name),
            format.fmt.pix.width,
            format.fmt.pix.height);

        return XCAM_RETURN_ERROR_PARAM;
    }

    while (_fps_n && _fps_d) {
        struct v4l2_streamparm param;
        xcam_mem_clear (param);
        param.type = _buf_type;
        if (io_control (VIDIOC_G_PARM, &param) < 0) {
            XCAM_LOG_WARNING ("device(%s) set framerate failed on VIDIOC_G_PARM but continue", XCAM_STR (_name));
            break;
        }

        if (!(param.parm.capture.capability & V4L2_CAP_TIMEPERFRAME))
            break;

        param.parm.capture.timeperframe.numerator = _fps_d;
        param.parm.capture.timeperframe.denominator = _fps_n;

        if (io_control (VIDIOC_S_PARM, &param) < 0) {
            XCAM_LOG_WARNING ("device(%s) set framerate failed on VIDIOC_S_PARM but continue", XCAM_STR (_name));
            break;
        }
        _fps_n = param.parm.capture.timeperframe.denominator;
        _fps_d = param.parm.capture.timeperframe.numerator;
        XCAM_LOG_INFO ("device(%s) set framerate(%d/%d)", XCAM_STR (_name), _fps_n, _fps_d);

        // exit here, otherwise it is an infinite loop
        break;
    }

    ret = post_set_format (format);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("device(%s) post_set_format failed", XCAM_STR (_name));
        return ret;
    }

    _format = format;
    XCAM_LOG_INFO (
        "device(%s) set format(w:%d, h:%d, pixelformat:%s, bytesperline:%d,image_size:%d)",
        XCAM_STR (_name),
        format.fmt.pix.width, format.fmt.pix.height,
        xcam_fourcc_to_string (format.fmt.pix.pixelformat),
        format.fmt.pix.bytesperline,
        format.fmt.pix.sizeimage);

    return XCAM_RETURN_NO_ERROR;
}

/*! \brief v4l2 set format
 *
 * \param[in]    width            format width
 * \param[in]    height           format height
 * \param[in]    pixelformat      fourcc
 * \param[in]    field            V4L2_FIELD_INTERLACED or V4L2_FIELD_NONE
 */
XCamReturn
V4l2Device::set_format (
    uint32_t width,  uint32_t height,
    uint32_t pixelformat, enum v4l2_field field, uint32_t bytes_perline)
{
	XCAM_LOG_INFO (
			"device(%s) set format(w:%d, h:%d, pixelformat:%s, bytesperline:%d)",
			XCAM_STR (_name),
			width, height,
			xcam_fourcc_to_string (pixelformat),
			bytes_perline);

    struct v4l2_format format;
    xcam_mem_clear (format);

    format.type = _buf_type;
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    format.fmt.pix.pixelformat = pixelformat;
    format.fmt.pix.field = field;

    if (bytes_perline != 0)
        format.fmt.pix.bytesperline = bytes_perline;
    return set_format (format);
}

XCamReturn
V4l2Device::pre_set_format (struct v4l2_format &format)
{
    XCAM_UNUSED (format);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::post_set_format (struct v4l2_format &format)
{
    XCAM_UNUSED (format);
    return XCAM_RETURN_NO_ERROR;
}

std::list<struct v4l2_fmtdesc>
V4l2Device::enum_formats ()
{
    std::list<struct v4l2_fmtdesc> formats;
    struct v4l2_fmtdesc format;
    uint32_t i = 0;

    while (1) {
        xcam_mem_clear (format);
        format.index = i++;
        format.type = _buf_type;
        if (this->io_control (VIDIOC_ENUM_FMT, &format) < 0) {
            if (errno == EINVAL)
                break;
            else { // error
                XCAM_LOG_DEBUG ("enum formats failed");
                return formats;
            }
        }
        formats.push_back (format);
    }

    return formats;
}

XCamReturn
V4l2Device::get_format (struct v4l2_format &format)
{
    if (is_activated ()) {
        format = _format;
        return XCAM_RETURN_NO_ERROR;
    }

    if (!is_opened ())
        return XCAM_RETURN_ERROR_IOCTL;

    xcam_mem_clear (format);
    format.type = _buf_type;

    if (this->io_control (VIDIOC_G_FMT, &format) < 0) {
        // FIXME: also log the device name?
        XCAM_LOG_ERROR("Fail to get format via ioctl VIDVIO_G_FMT.");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::start (bool need_queue_bufs)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // request buffer first
    ret = request_buffer ();
    XCAM_FAIL_RETURN (
        ERROR, ret == XCAM_RETURN_NO_ERROR, ret,
        "device(%s) start failed", XCAM_STR (_name));
    _queued_bufcnt = 0;
    //alloc buffers
    ret = init_buffer_pool ();
    XCAM_FAIL_RETURN (
        ERROR, ret == XCAM_RETURN_NO_ERROR, ret,
        "device(%s) start failed", XCAM_STR (_name));

    if (need_queue_bufs) {
        //queue all buffers
        for (uint32_t i = 0; i < _buf_count; ++i) {
            SmartPtr<V4l2Buffer> &buf = _buf_pool [i];
            XCAM_ASSERT (buf.ptr());
            XCAM_ASSERT (buf->get_buf().index == i);
            ret = queue_buffer (buf);
            if (ret != XCAM_RETURN_NO_ERROR) {
                XCAM_LOG_ERROR (
                    "device(%s) start failed on queue index:%d",
                    XCAM_STR (_name), i);
                stop ();
                return ret;
            }
        }
    }
    // stream on
    if (io_control (VIDIOC_STREAMON, &_buf_type) < 0) {
        XCAM_LOG_ERROR (
            "device(%s) start failed on VIDIOC_STREAMON",
            XCAM_STR (_name));
        stop ();
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _active = true;
    XCAM_LOG_INFO ("device(%s) started successfully", XCAM_STR (_name));
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::stop ()
{
    XCAM_LOG_INFO ("device(%s) stop, already start: %d", XCAM_STR (_name), _active);

    // stream off
    if (_active) {
        if (io_control (VIDIOC_STREAMOFF, &_buf_type) < 0) {
            XCAM_LOG_WARNING ("device(%s) steamoff failed", XCAM_STR (_name));
        }
        _active = false;
    }

    fini_buffer_pool ();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::request_buffer ()
{
    struct v4l2_requestbuffers request_buf;

    XCAM_ASSERT (!is_activated());

    xcam_mem_clear (request_buf);
    request_buf.type = _buf_type;
    request_buf.count = _buf_count;
    request_buf.memory = _memory_type;

    XCAM_LOG_INFO ("request buffers in device(%s): type: %d, count: %d, mem_type: %d",
            XCAM_STR (_name),
            request_buf.type,
            request_buf.count,
            request_buf.memory);

    if (io_control (VIDIOC_REQBUFS, &request_buf) < 0) {
        XCAM_LOG_INFO ("device(%s) starts failed on VIDIOC_REQBUFS", XCAM_STR (_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    XCAM_LOG_INFO ("device(%s) request buffer count: %d",
            XCAM_STR (_name), request_buf.count);

    if (request_buf.count != _buf_count) {
        XCAM_LOG_INFO (
            "device(%s) request buffer count doesn't match user settings, reset buffer count to %d",
            XCAM_STR (_name), request_buf.count);
        _buf_count = request_buf.count;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::allocate_buffer (
    SmartPtr<V4l2Buffer> &buf,
    const struct v4l2_format &format,
    const uint32_t index)
{
    struct v4l2_buffer v4l2_buf;

    xcam_mem_clear (v4l2_buf);
    v4l2_buf.index = index;
    v4l2_buf.type = _buf_type;
    v4l2_buf.memory = _memory_type;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type) {
        v4l2_buf.m.planes = &_planes[index];
        v4l2_buf.length = FMT_NUM_PLANES;
    }

    switch (_memory_type) {
    case V4L2_MEMORY_DMABUF:
    {
        struct v4l2_exportbuffer expbuf;
        xcam_mem_clear (expbuf);
        expbuf.type = _buf_type;
        expbuf.index = index;
        expbuf.flags = O_CLOEXEC;
        if (io_control (VIDIOC_EXPBUF, &expbuf) < 0) {
            XCAM_LOG_ERROR ("device(%s) get dma buf(%d) failed", XCAM_STR (_name), index);
            return XCAM_RETURN_ERROR_MEM;
        } else {
            XCAM_LOG_INFO ("device(%s) get dma buf(%d)-fd: %d", XCAM_STR (_name), index, expbuf.fd);
        }
        v4l2_buf.m.fd = expbuf.fd;
        v4l2_buf.length = format.fmt.pix.sizeimage;
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type) {
            v4l2_buf.length = FMT_NUM_PLANES;
            v4l2_buf.m.planes[0].m.fd = expbuf.fd;
            v4l2_buf.m.planes[0].length = format.fmt.pix.sizeimage;
            v4l2_buf.m.planes[0].bytesused = format.fmt.pix.sizeimage;
        }
    }
    break;
    case V4L2_MEMORY_MMAP:
    {
        void *pointer;
        int map_flags = MAP_SHARED;
#ifdef NEED_MAP_32BIT
        map_flags |= MAP_32BIT;
#endif
        if (io_control (VIDIOC_QUERYBUF, &v4l2_buf) < 0) {
            XCAM_LOG_ERROR("device(%s) query MMAP buf(%d) failed", XCAM_STR(_name), index);
            return XCAM_RETURN_ERROR_MEM;
        }

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type) {
            XCAM_LOG_DEBUG ("device(%s) get multiply planar buf(%d) length: %d", XCAM_STR (_name), index, v4l2_buf.m.planes[0].length);
            pointer = mmap (0, v4l2_buf.m.planes[0].length, PROT_READ | PROT_WRITE, map_flags, _fd, v4l2_buf.m.planes[0].m.mem_offset);
        } else {
            XCAM_LOG_DEBUG ("device(%s) get buf(%d) length: %d", XCAM_STR (_name), index, v4l2_buf.length);
            pointer = mmap (0, v4l2_buf.length, PROT_READ | PROT_WRITE, map_flags, _fd, v4l2_buf.m.offset);
        }

        if (pointer == MAP_FAILED) {
            XCAM_LOG_ERROR("device(%s) mmap buf(%d) failed", XCAM_STR(_name), index);
            return XCAM_RETURN_ERROR_MEM;
        }
        v4l2_buf.m.userptr = (uintptr_t) pointer;
    }
    break;
    case V4L2_MEMORY_USERPTR:
    default:
        XCAM_ASSERT (false);
        XCAM_LOG_WARNING (
            "device(%s) allocated buffer mem_type(%d) doesn't support",
            XCAM_STR (_name), _memory_type);
        return XCAM_RETURN_ERROR_MEM;
    }

    buf = new V4l2Buffer (v4l2_buf, _format);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::release_buffer (SmartPtr<V4l2Buffer> &buf) 
{
    int ret = 0;
    switch (_memory_type) {
    case V4L2_MEMORY_DMABUF:
    {
    }
    break;
    case V4L2_MEMORY_MMAP:
    {
        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type) {
            XCAM_LOG_DEBUG("release multi planar buffer length: %d", buf->get_length());
            ret = munmap((void*)buf->get_buf().m.userptr, buf->get_length());
        } else {
            XCAM_LOG_DEBUG("release buffer length: %d", buf->get_buf().length);
            ret = munmap((void*)buf->get_buf().m.userptr, buf->get_buf().length);
        }
        if (ret != 0) {
            XCAM_LOG_ERROR (
                "release buffer: munmap failed");
        }
    }
    break;
    default:
        XCAM_ASSERT (false);
        XCAM_LOG_WARNING (
            "device(%s) allocated buffer mem_type(%d) doesn't support",
            XCAM_STR (_name), _memory_type);
        return XCAM_RETURN_ERROR_MEM;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::init_buffer_pool ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t i = 0;

    _buf_pool.clear ();
    _buf_pool.reserve (_buf_count);

    for (; i < _buf_count; i++) {
        SmartPtr<V4l2Buffer> new_buf;
        XCAM_LOG_DEBUG("allocate_buffer index: %d", i);
        ret = allocate_buffer (new_buf, _format, i);
        if (ret != XCAM_RETURN_NO_ERROR) {
            break;
        }
        _buf_pool.push_back (new_buf);
    }

    for (i = 0; i < _buf_count; i++) {
        SmartPtr<V4l2Buffer> &buf = _buf_pool [i];
        struct v4l2_buffer v4l2_buf = buf->get_buf ();
        XCAM_LOG_DEBUG ("init_buffer_pool device(%s) index:%d, memory: %d, type:%d, length: %d",
            XCAM_STR (_name), v4l2_buf.index, v4l2_buf.memory,
            v4l2_buf.type, v4l2_buf.length);
    }
    if (_buf_pool.empty()) {
        XCAM_LOG_ERROR ("No bufer allocated in device(%s)", XCAM_STR (_name));
        return XCAM_RETURN_ERROR_MEM;
    }

    if (i != _buf_count) {
        XCAM_LOG_WARNING (
            "device(%s) allocate buffer count:%d failback to %d",
            XCAM_STR (_name), _buf_count, i);
        _buf_count = i;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::fini_buffer_pool()
{
    uint32_t i = 0;

    for (; i < _buf_pool.size(); i++) {
        release_buffer(_buf_pool [i]);
    }

    _buf_pool.clear ();

    return XCAM_RETURN_NO_ERROR;
}

SmartPtr<V4l2Buffer>
V4l2Device::get_buffer_by_index (int index) {
    SmartPtr<V4l2Buffer> &buf = _buf_pool [index];

    return buf;
}

XCamReturn
V4l2Device::dequeue_buffer(SmartPtr<V4l2Buffer> &buf)
{
    struct v4l2_buffer v4l2_buf;

    if (!is_activated()) {
        XCAM_LOG_DEBUG (
            "device(%s) dequeue buffer failed since not activated", XCAM_STR (_name));
        return XCAM_RETURN_ERROR_PARAM;
    }

    xcam_mem_clear (v4l2_buf);
    v4l2_buf.type = _buf_type;
    v4l2_buf.memory = _memory_type;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type) {
        struct v4l2_plane planes[FMT_NUM_PLANES];
        memset(planes, 0, sizeof(struct v4l2_plane) * FMT_NUM_PLANES);
        v4l2_buf.m.planes = planes;
        v4l2_buf.length = FMT_NUM_PLANES;
    }

    if (this->io_control (VIDIOC_DQBUF, &v4l2_buf) < 0) {
        XCAM_LOG_ERROR ("device(%s) fail to dequeue buffer.", XCAM_STR (_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type) {
        XCAM_LOG_DEBUG ("device(%s) multi planar dequeue buffer index:%d, length: %d",
            XCAM_STR (_name), v4l2_buf.index,
            v4l2_buf.m.planes[0].length);
        if (V4L2_MEMORY_DMABUF == _memory_type) {
            XCAM_LOG_DEBUG ("device(%s) multi planar index:%d, fd: %d",
                XCAM_STR (_name), v4l2_buf.index, v4l2_buf.m.planes[0].m.fd);
        }
    } else {
        XCAM_LOG_DEBUG ("device(%s) dequeue buffer index:%d, length: %d",
            XCAM_STR (_name), v4l2_buf.index, v4l2_buf.length);
    }

    if (v4l2_buf.index > _buf_count) {
        XCAM_LOG_ERROR (
            "device(%s) dequeue wrong buffer index:%d",
            XCAM_STR (_name), v4l2_buf.index);
        return XCAM_RETURN_ERROR_ISP;
    }

    buf = _buf_pool [v4l2_buf.index];
    buf->set_timestamp (v4l2_buf.timestamp);
    buf->set_timecode (v4l2_buf.timecode);
    buf->set_sequence (v4l2_buf.sequence);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type) {
        buf->set_length (v4l2_buf.m.planes[0].length); 
    } else {
        buf->set_length (v4l2_buf.length); 
    }
    _queued_bufcnt--;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2Device::queue_buffer (SmartPtr<V4l2Buffer> &buf)
{
    XCAM_ASSERT (buf.ptr());
    buf->reset ();

    struct v4l2_buffer v4l2_buf = buf->get_buf ();
    XCAM_ASSERT (v4l2_buf.index < _buf_count);

    XCAM_LOG_DEBUG ("device(%s) queue buffer index:%d, memory: %d, type:%d, length: %d, fd: %d",
        XCAM_STR (_name), v4l2_buf.index, v4l2_buf.memory,
        v4l2_buf.type, v4l2_buf.m.planes[0].length, v4l2_buf.m.planes[0].m.fd);

    if (io_control (VIDIOC_QBUF, &v4l2_buf) < 0) {
        XCAM_LOG_ERROR("fail to enqueue buffer index:%d.", v4l2_buf.index);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _queued_bufcnt++;
    return XCAM_RETURN_NO_ERROR;
}

V4l2SubDevice::V4l2SubDevice (const char *name)
    : V4l2Device (name)
{
}

XCamReturn
V4l2SubDevice::subscribe_event (int event)
{
    struct v4l2_event_subscription sub;
    int ret = 0;

    XCAM_ASSERT (is_opened());

    xcam_mem_clear (sub);
    sub.type = event;

    ret = this->io_control (VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        XCAM_LOG_DEBUG ("subdev(%s) subscribe event(%d) failed", XCAM_STR(_name), event);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2SubDevice::unsubscribe_event (int event)
{
    struct v4l2_event_subscription sub;
    int ret = 0;

    XCAM_ASSERT (is_opened());

    xcam_mem_clear (sub);
    sub.type = event;

    ret = this->io_control (VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        XCAM_LOG_DEBUG ("subdev(%s) unsubscribe event(%d) failed", XCAM_STR(_name), event);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
V4l2SubDevice::dequeue_event (struct v4l2_event &event)
{
    int ret = 0;
    XCAM_ASSERT (is_opened());

    ret = this->io_control (VIDIOC_DQEVENT, &event);
    if (ret < 0) {
        XCAM_LOG_DEBUG ("subdev(%s) dequeue event failed", XCAM_STR(_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn V4l2SubDevice::start ()
{
    if (!is_opened())
        return XCAM_RETURN_ERROR_PARAM;

    _active = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn V4l2SubDevice::stop ()
{
    if (_active)
        _active = false;

    return XCAM_RETURN_NO_ERROR;
}

};
