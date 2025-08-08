/*
 * aiq_fake_v4l2_device.c
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

#include "aiq_fake_v4l2_device.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

XCAM_BEGIN_DECLARE

static int _create_notify_pipe(AiqFakeV4l2Device_t* fake_v4l2_dev);
static void _destroy_notify_pipe(AiqFakeV4l2Device_t* fake_v4l2_dev);
static uint32_t _get_available_buffer_index(AiqFakeV4l2Device_t* fake_v4l2_dev);

static XCamReturn _open(AiqV4l2Device_t* v4l2_dev, bool nonblock) {
    struct v4l2_streamparm param;

    if (AiqV4l2Device_isOpened(v4l2_dev)) {
        XCAM_LOG_DEBUG("device(%s) was already opened", XCAM_STR(v4l2_dev->_name));
        return XCAM_RETURN_NO_ERROR;
    }

    if (!v4l2_dev->_name) {
        XCAM_LOG_DEBUG("v4l2 device open failed, there's no device name");
        return XCAM_RETURN_ERROR_PARAM;
    }

    v4l2_dev->_fd = open(v4l2_dev->_name, O_RDWR);

    if (v4l2_dev->_fd == -1) {
        XCAM_LOG_ERROR("open device(%s) failed", v4l2_dev->_name);
        return XCAM_RETURN_ERROR_IOCTL;
    } else {
        XCAM_LOG_DEBUG("open device(%s) successed, fd: %d", v4l2_dev->_name, v4l2_dev->_fd);
    }

    if (_create_notify_pipe((AiqFakeV4l2Device_t*)v4l2_dev) < 0) {
        XCAM_LOG_ERROR("create virtual tx pipe failed");
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _close(AiqV4l2Device_t* v4l2_dev) {
    if (AiqV4l2Device_isOpened(v4l2_dev)) return XCAM_RETURN_NO_ERROR;
    close(v4l2_dev->_fd);
    v4l2_dev->_fd = -1;
    _destroy_notify_pipe((AiqFakeV4l2Device_t*)v4l2_dev);
    XCAM_LOG_INFO("device(%s) closed", XCAM_STR(v4l2_dev->_name));
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _start(AiqV4l2Device_t* v4l2_dev, bool prepared) {
    (void)prepared;
    v4l2_dev->_active = true;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _stop(AiqV4l2Device_t* v4l2_dev) {
    AiqFakeV4l2Device_t* fake_v4l2_dev = (AiqFakeV4l2Device_t*)v4l2_dev;
    v4l2_dev->_active                  = false;
    aiqList_reset(fake_v4l2_dev->_buf_list);
    return XCAM_RETURN_NO_ERROR;
}

static int _create_notify_pipe(AiqFakeV4l2Device_t* fake_v4l2_dev) {
    int status = 0;

    _destroy_notify_pipe(fake_v4l2_dev);
    status = pipe(fake_v4l2_dev->_pipe_fd);
    if (status < 0) {
        XCAM_LOG_ERROR("Failed to create virtual tx notify poll pipe: %s", strerror(errno));
        goto exit_error;
    }
    status = fcntl(fake_v4l2_dev->_pipe_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR("Fail to set event virtual tx notify pipe flag: %s", strerror(errno));
        goto exit_error;
    }
    status = fcntl(fake_v4l2_dev->_pipe_fd[1], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR("Fail to set event virtual tx notify pipe flag: %s", strerror(errno));
        goto exit_error;
    }
    return status;
exit_error:
    _destroy_notify_pipe(fake_v4l2_dev);
    return status;
}

static void _destroy_notify_pipe(AiqFakeV4l2Device_t* fake_v4l2_dev) {
    if (fake_v4l2_dev->_pipe_fd[0] != -1 || fake_v4l2_dev->_pipe_fd[1] != -1) {
        close(fake_v4l2_dev->_pipe_fd[0]);
        close(fake_v4l2_dev->_pipe_fd[1]);
        fake_v4l2_dev->_pipe_fd[0] = -1;
        fake_v4l2_dev->_pipe_fd[1] = -1;
    }
}

static int _io_control(AiqV4l2Device_t* v4l2_dev, unsigned long cmd, void* arg) {
    AiqFakeV4l2Device_t* fake_v4l2_dev = (AiqFakeV4l2Device_t*)v4l2_dev;

    if (v4l2_dev->_fd <= 0) return -1;

    if (VIDIOC_DQBUF == cmd) {
        struct v4l2_buffer* v4l2_buf = (struct v4l2_buffer*)arg;
        v4l2_buf->index              = _get_available_buffer_index(fake_v4l2_dev);
        if (v4l2_buf->index == v4l2_dev->_buf_count) {
            LOGE("%s, no free buffer was found!", __BI_FILENAME__);
            return XCAM_RETURN_ERROR_TIMEOUT;
        }

        struct rk_aiq_vbuf_info vb_info;
        aiqMutex_lock(&fake_v4l2_dev->_mutex);
        if (aiqList_size(fake_v4l2_dev->_buf_list) > 0) {
            aiqList_get(fake_v4l2_dev->_buf_list, &vb_info);
            v4l2_buf->m.planes[0].length    = vb_info.data_length;
            v4l2_buf->m.planes[0].bytesused = vb_info.data_length;
            v4l2_buf->sequence              = vb_info.frame_id;
            v4l2_buf->m.planes[0].m.userptr = (unsigned long)vb_info.data_addr;
            v4l2_buf->reserved              = vb_info.data_fd;
            gettimeofday(&v4l2_buf->timestamp, NULL);
        } else {
            LOGE("%s, _buf_list is empty()", __BI_FILENAME__);
        }

        aiqMutex_unlock(&fake_v4l2_dev->_mutex);
    }
    return 0;
}

static XCamReturn _get_format(AiqV4l2Device_t* v4l2_dev, struct v4l2_format* format) {
    *format = v4l2_dev->_format;
    return XCAM_RETURN_NO_ERROR;
}

static int _poll_event(AiqV4l2Device_t* v4l2_dev, int timeout_msec, int stop_fd) {
    int num_fds = stop_fd == -1 ? 1 : 2;
    struct pollfd poll_fds[num_fds];
    int ret                            = 0;
    AiqFakeV4l2Device_t* fake_v4l2_dev = (AiqFakeV4l2Device_t*)v4l2_dev;

    XCAM_ASSERT(v4l2_dev->_fd > 0);

    memset(poll_fds, 0, sizeof(poll_fds));
    poll_fds[0].fd     = fake_v4l2_dev->_pipe_fd[0];
    poll_fds[0].events = (POLLPRI | POLLIN | POLLOUT);

    if (stop_fd != -1) {
        poll_fds[1].fd      = stop_fd;
        poll_fds[1].events  = POLLPRI | POLLIN | POLLOUT;
        poll_fds[1].revents = 0;
    }

    ret = poll(poll_fds, num_fds, timeout_msec);
    if (ret > 0) {
        if (stop_fd != -1) {
            if (poll_fds[1].revents & (POLLIN | POLLPRI)) {
                XCAM_LOG_DEBUG("%s: Poll returning from flush", __FUNCTION__);
                return POLL_STOP_RET;
            }
        }

        if (poll_fds[0].revents & (POLLIN | POLLPRI)) {
            char buf[8];
            read(fake_v4l2_dev->_pipe_fd[0], buf, sizeof(buf));
            XCAM_LOG_DEBUG("%s: Poll returning timer pipe", __FUNCTION__);
        }
    }

    return ret;
}

AiqV4l2Buffer_t* _dequeue_buffer(AiqV4l2Device_t* v4l2_dev) {
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[FMT_NUM_PLANES];
    AiqV4l2Buffer_t* buf = NULL;

    if (!AiqV4l2Device_isActivated(v4l2_dev)) {
        XCAM_LOG_DEBUG("device(%s) dequeue buffer failed since not activated",
                       XCAM_STR(v4l2_dev->_name));
        return NULL;
    }

    xcam_mem_clear(v4l2_buf);
    v4l2_buf.type   = v4l2_dev->_buf_type;
    v4l2_buf.memory = v4l2_dev->_memory_type;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
        memset(planes, 0, sizeof(struct v4l2_plane) * v4l2_dev->_mplanes_count);
        v4l2_buf.m.planes = planes;
        v4l2_buf.length   = v4l2_dev->_mplanes_count;
    }

    if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_DQBUF, &v4l2_buf) < 0) {
        XCAM_LOG_ERROR("device(%s) fail to dequeue buffer.", XCAM_STR(v4l2_dev->_name));
        return NULL;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
        XCAM_LOG_DEBUG("device(%s) multi planar dequeue buffer index:%d, length: %dn",
                       XCAM_STR(v4l2_dev->_name), v4l2_buf.index, v4l2_buf.m.planes[0].length);
        if (V4L2_MEMORY_DMABUF == v4l2_dev->_memory_type) {
            XCAM_LOG_DEBUG("device(%s) multi planar index:%d, fd: %d", XCAM_STR(v4l2_dev->_name),
                           v4l2_buf.index, v4l2_buf.m.planes[0].m.fd);
        }
    } else {
        XCAM_LOG_DEBUG("device(%s) dequeue buffer index:%d, length: %d", XCAM_STR(v4l2_dev->_name),
                       v4l2_buf.index, v4l2_buf.length);
    }

    if (v4l2_buf.index > v4l2_dev->_buf_count) {
        XCAM_LOG_ERROR("device(%s) dequeue wrong buffer index:%d", XCAM_STR(v4l2_dev->_name),
                       v4l2_buf.index);
        return NULL;
    }

    aiqMutex_lock(&v4l2_dev->_buf_mutex);

    buf = &v4l2_dev->_buf_pool[v4l2_buf.index];
    AiqV4l2Buffer_setTimestamp(buf, v4l2_buf.timestamp);
    AiqV4l2Buffer_setTimecode(buf, v4l2_buf.timecode);
    AiqV4l2Buffer_setSequence(buf, v4l2_buf.sequence);

    if (!V4L2_TYPE_IS_OUTPUT(v4l2_dev->_buf_type) &&
        (v4l2_dev->_buf_type != V4L2_BUF_TYPE_META_OUTPUT))
        AiqV4l2Buffer_setQueued(buf, false);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
        AiqV4l2Buffer_setLength(buf, v4l2_buf.m.planes[0].length);
        AiqV4l2Buffer_setExpbufUsrptr(buf, v4l2_buf.m.planes[0].m.userptr);
        AiqV4l2Buffer_setExpbufFd(buf, v4l2_buf.reserved);
    } else {
        AiqV4l2Buffer_setLength(buf, v4l2_buf.length);
    }
    v4l2_dev->_queued_bufcnt--;

    aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    return buf;
}

static uint32_t _get_available_buffer_index(AiqFakeV4l2Device_t* fake_v4l2_dev) {
    uint32_t idx              = 0;
    AiqV4l2Buffer_t* buf      = NULL;
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)fake_v4l2_dev;

    aiqMutex_lock(&v4l2_dev->_buf_mutex);
    for (; idx < v4l2_dev->_buf_count; idx++) {
        buf = &v4l2_dev->_buf_pool[idx];
        if (AiqV4l2Buffer_getQueued(buf)) {
            break;
        }
    }
    aiqMutex_unlock(&v4l2_dev->_buf_mutex);
    return idx;
}

static void _enqueue_rawbuffer(AiqFakeV4l2Device_t* fake_v4l2_dev,
                               struct rk_aiq_vbuf_info* vbinfo) {
    if (vbinfo != NULL) {
        aiqMutex_lock(&fake_v4l2_dev->_mutex);
        aiqList_push(fake_v4l2_dev->_buf_list, vbinfo);
        aiqMutex_unlock(&fake_v4l2_dev->_mutex);
    }
}

static void _on_timer_proc(AiqFakeV4l2Device_t* fake_v4l2_dev) {
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)fake_v4l2_dev;

    if ((aiqList_size(fake_v4l2_dev->_buf_list) > 0) && v4l2_dev->_queued_bufcnt) {
        if (fake_v4l2_dev->_pipe_fd[1] != -1) {
            char buf          = 0xf;  // random value to write to flush fd.
            unsigned int size = write(fake_v4l2_dev->_pipe_fd[1], &buf, sizeof(char));
            if (size != sizeof(char)) XCAM_LOG_ERROR("Flush write not completed");
        }
    }
}

int AiqFakeV4l2Device_init(AiqFakeV4l2Device_t* fake_v4l2_dev, const char* name) {
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)fake_v4l2_dev;

    ret = AiqV4l2Device_init(v4l2_dev, "/dev/zero");

    if (ret) {
        XCAM_LOG_ERROR("dev(%s) init failed", XCAM_STR(v4l2_dev->_name));
        return ret;
    }

    aiqMutex_init(&fake_v4l2_dev->_mutex);

    fake_v4l2_dev->_pipe_fd[0]                = -1;
    fake_v4l2_dev->_pipe_fd[1]                = -1;
    fake_v4l2_dev->on_timer_proc              = _on_timer_proc;
    fake_v4l2_dev->enqueue_rawbuffer          = _enqueue_rawbuffer;
    fake_v4l2_dev->get_available_buffer_index = _get_available_buffer_index;
    v4l2_dev->open                            = _open;
    v4l2_dev->close                           = _close;
    v4l2_dev->start                           = _start;
    v4l2_dev->stop                            = _stop;
    v4l2_dev->get_format                      = _get_format;
    v4l2_dev->poll_event                      = _poll_event;
    v4l2_dev->_dequeue_buffer                 = _dequeue_buffer;
    v4l2_dev->io_control                      = _io_control;
    {
        AiqListConfig_t listCfg;

        listCfg._name            = "fake_buf";
        listCfg._item_size       = sizeof(struct rk_aiq_vbuf_info);
        listCfg._item_nums       = 4;
        fake_v4l2_dev->_buf_list = aiqList_init(&listCfg);
    }
    return ret;
}

void AiqFakeV4l2Device_deinit(AiqFakeV4l2Device_t* fake_v4l2_dev) {
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)fake_v4l2_dev;

    aiqList_deinit(fake_v4l2_dev->_buf_list);
    AiqV4l2Device_deinit(v4l2_dev);
}

XCAM_END_DECLARE
