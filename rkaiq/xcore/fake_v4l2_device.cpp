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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include "fake_v4l2_device.h"
#include "v4l2_buffer_proxy.h"

namespace XCam {


XCamReturn
FakeV4l2Device::open (bool nonblock)
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
        XCAM_LOG_ERROR ("open device(%s) failed", _name);
        return XCAM_RETURN_ERROR_IOCTL;
    } else {
        XCAM_LOG_DEBUG ("open device(%s) successed, fd: %d", _name, _fd);
    }

    if (create_notify_pipe () < 0) {
        XCAM_LOG_ERROR ("create virtual tx pipe failed");
        return XCAM_RETURN_ERROR_PARAM;
    }


    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FakeV4l2Device::close ()
{

    if (!is_opened())
        return XCAM_RETURN_NO_ERROR;
    ::close (_fd);
    _fd = -1;
    destroy_notify_pipe ();
    XCAM_LOG_INFO ("device(%s) closed", XCAM_STR (_name));
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FakeV4l2Device::start (bool prepared) {
    (void)prepared;
    _active = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FakeV4l2Device::stop () {
    _active = false;
    _buf_list.clear();
    return XCAM_RETURN_NO_ERROR;
}

int
FakeV4l2Device::create_notify_pipe ()
{
    int status = 0;

    destroy_notify_pipe ();
    status = pipe(_pipe_fd);
    if (status < 0) {
        XCAM_LOG_ERROR("Failed to create virtual tx notify poll pipe: %s", strerror(errno));
        goto exit_error;
    }
    status = fcntl(_pipe_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR("Fail to set event virtual tx notify pipe flag: %s", strerror(errno));
        goto exit_error;
    }
    status = fcntl(_pipe_fd[1], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR("Fail to set event virtual tx notify pipe flag: %s", strerror(errno));
        goto exit_error;
    }
    return status;
exit_error:
    destroy_notify_pipe();
    return status;
}

void FakeV4l2Device::destroy_notify_pipe () {
    if (_pipe_fd[0] != -1 || _pipe_fd[1] != -1) {
        ::close(_pipe_fd[0]);
        ::close(_pipe_fd[1]);
        _pipe_fd[0] = -1;
        _pipe_fd[1] = -1;
    }
}

int
FakeV4l2Device::io_control (unsigned long cmd, void *arg)
{
    if (_fd <= 0)
        return -1;

    if (VIDIOC_DQBUF == cmd) {
        struct v4l2_buffer *v4l2_buf = (struct v4l2_buffer *)arg;
        v4l2_buf->index = get_available_buffer_index();
        _mutex.lock();
        struct rk_aiq_vbuf_info vb_info;
        if(!_buf_list.empty()) {
            vb_info = _buf_list.front();
            _buf_list.pop_front();
            v4l2_buf->m.planes[0].length = vb_info.data_length;
            v4l2_buf->m.planes[0].bytesused = vb_info.data_length;
            v4l2_buf->sequence = vb_info.frame_id;
            v4l2_buf->m.planes[0].m.userptr = (unsigned long)vb_info.data_addr;
            v4l2_buf->reserved = vb_info.data_fd;
            gettimeofday(&v4l2_buf->timestamp, NULL);
        }
        _mutex.unlock();
    }
    return 0;
}

XCamReturn FakeV4l2Device::get_format (struct v4l2_format &format)
{
    format = _format;
    return XCAM_RETURN_NO_ERROR;
}

int
FakeV4l2Device::poll_event (int timeout_msec, int stop_fd)
{
    int num_fds = stop_fd == -1 ? 1 : 2;
    struct pollfd poll_fds[num_fds];
    int ret = 0;

    XCAM_ASSERT (_fd > 0);

    memset(poll_fds, 0, sizeof(poll_fds));
    poll_fds[0].fd = _pipe_fd[0];
    poll_fds[0].events = (POLLPRI | POLLIN | POLLOUT);

    if (stop_fd != -1) {
        poll_fds[1].fd = stop_fd;
        poll_fds[1].events = POLLPRI | POLLIN | POLLOUT;
        poll_fds[1].revents = 0;
    }

    ret = poll (poll_fds, num_fds, timeout_msec);
    if (ret > 0) {
        if (stop_fd != -1) {
            if (poll_fds[1].revents & (POLLIN | POLLPRI)) {
                XCAM_LOG_DEBUG ("%s: Poll returning from flush", __FUNCTION__);
                return POLL_STOP_RET;
            }
        }

        if (poll_fds[0].revents & (POLLIN | POLLPRI)) {
            char buf[8];
            read(_pipe_fd[0], buf, sizeof(buf));
            XCAM_LOG_DEBUG ("%s: Poll returning timer pipe", __FUNCTION__);
        }

    }

    return ret;
}

XCamReturn
FakeV4l2Device::dequeue_buffer(SmartPtr<V4l2Buffer> &buf)
{
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[FMT_NUM_PLANES];

    if (!is_activated()) {
        XCAM_LOG_ERROR (
            "device(%s) dequeue buffer failed since not activated", XCAM_STR (_name));
        return XCAM_RETURN_ERROR_PARAM;
    }

    xcam_mem_clear (v4l2_buf);
    v4l2_buf.type = _buf_type;
    v4l2_buf.memory = _memory_type;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type ||
            V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == _buf_type) {
        memset(planes, 0, sizeof(struct v4l2_plane) * FMT_NUM_PLANES);
        v4l2_buf.m.planes = planes;
        v4l2_buf.length = FMT_NUM_PLANES;
    }

    if (this->io_control (VIDIOC_DQBUF, &v4l2_buf) < 0) {
        XCAM_LOG_ERROR ("device(%s) fail to dequeue buffer.", XCAM_STR (_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type ||
            V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == _buf_type) {
        XCAM_LOG_DEBUG ("device(%s) dequeue buffer index:%d, memory:%d, type:%d, multiply planar:%d, length:%d, fd:%d,ptr:%p",
                        XCAM_STR (_name), v4l2_buf.index, v4l2_buf.memory,
                        v4l2_buf.type, v4l2_buf.length, v4l2_buf.m.planes[0].length, v4l2_buf.m.planes[0].m.fd, v4l2_buf.m.planes[0].m.userptr);

        if (V4L2_MEMORY_DMABUF == _memory_type) {
            XCAM_LOG_DEBUG ("device(%s) multi planar index:%d, fd: %d",
                            XCAM_STR (_name), v4l2_buf.index, v4l2_buf.m.planes[0].m.fd);
        }
    } else {
        XCAM_LOG_DEBUG ("device(%s) dequeue buffer index:%d, length: %d",
                        XCAM_STR (_name), v4l2_buf.index, v4l2_buf.length);
    }

    if (v4l2_buf.index >= _buf_count) {
        XCAM_LOG_ERROR (
            "device(%s) dequeue wrong buffer index:%d",
            XCAM_STR (_name), v4l2_buf.index);
        return XCAM_RETURN_ERROR_ISP;
    }

    SmartLock auto_lock(_buf_mutex);

    buf = _buf_pool [v4l2_buf.index];
    buf->set_timestamp (v4l2_buf.timestamp);
    buf->set_timecode (v4l2_buf.timecode);
    buf->set_sequence (v4l2_buf.sequence);
    if (!V4L2_TYPE_IS_OUTPUT(buf->get_buf ().type))
        buf->set_queued(false);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == _buf_type) {
        buf->set_length (v4l2_buf.m.planes[0].length);
       // if (_use_type != 2) {
            buf->set_expbuf_usrptr(v4l2_buf.m.planes[0].m.userptr);
            buf->set_expbuf_fd(v4l2_buf.reserved);
    //    }
    } else {
        buf->set_length (v4l2_buf.length);
    }
    _queued_bufcnt--;

    return XCAM_RETURN_NO_ERROR;
}

uint32_t
FakeV4l2Device::get_available_buffer_index ()
{
    uint32_t idx = 0;
    SmartPtr<V4l2Buffer> buf;
    _buf_mutex.lock();
     for (; idx<_buf_count; idx++) {
        buf = _buf_pool[idx];
        if (buf->get_queued()) {
            break;
        }
     }
     _buf_mutex.unlock();
     return idx;
}

void
FakeV4l2Device::enqueue_rawbuffer(struct rk_aiq_vbuf_info *vbinfo)
{
    if (vbinfo != NULL) {
        _mutex.lock();
        _buf_list.push_back(*vbinfo);
        _mutex.unlock();
    }
}
static int icnt = 0;
void
FakeV4l2Device::on_timer_proc ()
{
    if (!_buf_list.empty() && _queued_bufcnt) {
        if (_pipe_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_pipe_fd[1], &buf, sizeof(char));
            if (size != sizeof(char))
                XCAM_LOG_ERROR("Flush write not completed");
        }
    }
}

}
