/*
 * aiq_v4l2_device.c
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

#include "aiq_v4l2_device.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "aiq_base.h"

XCAM_BEGIN_DECLARE

#define XCAM_V4L2_DEFAULT_BUFFER_COUNT 4

static XCamReturn _v4l_allocate_buffer(AiqV4l2Device_t* v4l2_dev, AiqV4l2Buffer_t* buf,
                                       const struct v4l2_format* format, const uint32_t index) {
    struct v4l2_buffer v4l2_buf;
    int expbuf_fd        = -1;
    uintptr_t expbuf_ptr = 0;
    int i                = 0;

    xcam_mem_clear(v4l2_buf);
    v4l2_buf.index  = index;
    v4l2_buf.type   = v4l2_dev->_buf_type;
    v4l2_buf.memory = v4l2_dev->_memory_type;
    if (v4l2_dev->_buf_sync) {
        v4l2_buf.flags = V4L2_BUF_FLAG_NO_CACHE_INVALIDATE | V4L2_BUF_FLAG_NO_CACHE_CLEAN;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
        v4l2_buf.m.planes = &v4l2_dev->_planes[index * v4l2_dev->_mplanes_count];
        v4l2_buf.length   = v4l2_dev->_mplanes_count;
    }

    switch (v4l2_dev->_memory_type) {
        case V4L2_MEMORY_DMABUF: {
            v4l2_buf.length = format->fmt.pix.sizeimage;
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
                V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
                v4l2_buf.length = v4l2_dev->_mplanes_count;
                for (i = 0; i < v4l2_dev->_mplanes_count; i++) {
                    v4l2_buf.m.planes[i].length    = format->fmt.pix.sizeimage;
                    v4l2_buf.m.planes[i].bytesused = format->fmt.pix.sizeimage;
                }
            }
        } break;
        case V4L2_MEMORY_MMAP: {
            void* pointer = MAP_FAILED;
            int map_flags = MAP_SHARED;
            int i         = 0;
#ifdef NEED_MAP_32BIT
            map_flags |= MAP_32BIT;
#endif
            if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_QUERYBUF, &v4l2_buf) < 0) {
                XCAM_LOG_ERROR("device(%s) query MMAP buf(%d) failed", v4l2_dev->_name,
                               index);
                return XCAM_RETURN_ERROR_MEM;
            }

            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
                V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
                for (i = 0; i < v4l2_dev->_mplanes_count; i++) {
                    XCAM_LOG_DEBUG("device(%s) get planar(%d) of buf(%d) length: %d",
                                   v4l2_dev->_name, i, index,
                                   v4l2_buf.m.planes[i].length);
                    pointer = mmap(0, v4l2_buf.m.planes[i].length, PROT_READ | PROT_WRITE,
                                   map_flags, v4l2_dev->_fd, v4l2_buf.m.planes[i].m.mem_offset);
                    v4l2_buf.m.planes[i].m.userptr = (uintptr_t)pointer;
                    if (pointer == MAP_FAILED) {
                        XCAM_LOG_ERROR("device(%s) mmap planar(%d) of buf(%d) failed",
                                       v4l2_dev->_name, i, index);
                        return XCAM_RETURN_ERROR_MEM;
                    }
                }
                expbuf_ptr = v4l2_buf.m.planes[0].m.userptr;
            } else {
                XCAM_LOG_DEBUG("device(%s) get buf(%d) length: %d", v4l2_dev->_name, index,
                               v4l2_buf.length);
                pointer = mmap(0, v4l2_buf.length, PROT_READ | PROT_WRITE, map_flags, v4l2_dev->_fd,
                               v4l2_buf.m.offset);
                if (pointer == MAP_FAILED) {
                    XCAM_LOG_ERROR("device(%s) mmap buf(%d) failed", v4l2_dev->_name,
                                   index);
                    return XCAM_RETURN_ERROR_MEM;
                }
                expbuf_ptr = v4l2_buf.m.userptr = (uintptr_t)pointer;
            }

            // export buf dma fd
            struct v4l2_exportbuffer expbuf;
            xcam_mem_clear(expbuf);
            expbuf.type  = v4l2_dev->_buf_type;
            expbuf.index = index;
            expbuf.flags = O_CLOEXEC;
            if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_EXPBUF, &expbuf) < 0) {
                XCAM_LOG_ERROR("device(%s) get dma buf(%d) failed", v4l2_dev->_name,
                               index);
                return XCAM_RETURN_ERROR_MEM;
            } else {
                XCAM_LOG_INFO("device(%s) get dma buf(%d)-fd: %d", v4l2_dev->_name, index,
                              expbuf.fd);
            }
            expbuf_fd = expbuf.fd;
        } break;
        case V4L2_MEMORY_USERPTR: {
            v4l2_buf.length = format->fmt.pix.sizeimage;
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
                V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
                v4l2_buf.length = v4l2_dev->_mplanes_count;
                for (i = 0; i < v4l2_dev->_mplanes_count; i++) {
                    v4l2_buf.m.planes[i].length    = format->fmt.pix.sizeimage;
                    v4l2_buf.m.planes[i].bytesused = format->fmt.pix.sizeimage;
                }
            }
        } break;
        default:
            XCAM_ASSERT(false);
            XCAM_LOG_WARNING("device(%s) allocated buffer mem_type(%d) doesn't support",
                             v4l2_dev->_name, v4l2_dev->_memory_type);
            return XCAM_RETURN_ERROR_MEM;
    }

    AiqV4l2Buffer_init(buf, &v4l2_buf, &v4l2_dev->_format, v4l2_dev);

    if (expbuf_fd != -1) AiqV4l2Buffer_setExpbufFd(buf, expbuf_fd);
    if (expbuf_ptr != 0) AiqV4l2Buffer_setExpbufUsrptr(buf, expbuf_ptr);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _v4l_release_buffer(AiqV4l2Device_t* v4l2_dev, AiqV4l2Buffer_t* buf) {
    int ret = 0, i = 0;
    switch (v4l2_dev->_memory_type) {
        case V4L2_MEMORY_DMABUF: {
        } break;
        case V4L2_MEMORY_MMAP: {
            if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
                V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
                for (i = 0; i < v4l2_dev->_mplanes_count; i++) {
                    XCAM_LOG_DEBUG("release multi planar(%d) of buffer length: %d", i,
                                   AiqV4l2Buffer_getBuf(buf)->m.planes[i].length);
                    ret = munmap((void*)AiqV4l2Buffer_getBuf(buf)->m.planes[i].m.userptr,
                                 AiqV4l2Buffer_getBuf(buf)->m.planes[i].length);
                }
                close(AiqV4l2Buffer_getExpbufFd(buf));
            } else {
                XCAM_LOG_DEBUG("release buffer length: %d", AiqV4l2Buffer_getBuf(buf)->length);
                ret = munmap((void*)AiqV4l2Buffer_getBuf(buf)->m.userptr,
                             AiqV4l2Buffer_getBuf(buf)->length);
                close(AiqV4l2Buffer_getExpbufFd(buf));
            }
            if (ret != 0) {
                XCAM_LOG_ERROR("release buffer: munmap failed");
            }
        } break;
        case V4L2_MEMORY_USERPTR:
            break;
        default:
            XCAM_ASSERT(false);
            XCAM_LOG_WARNING("device(%s) allocated buffer mem_type(%d) doesn't support",
                             v4l2_dev->_name, v4l2_dev->_memory_type);
            return XCAM_RETURN_ERROR_MEM;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _v4l_request_buffer(AiqV4l2Device_t* v4l2_dev) {
    struct v4l2_requestbuffers request_buf;

    XCAM_ASSERT(!AiqV4l2Device_isActivated(v4l2_dev));

    xcam_mem_clear(request_buf);
    request_buf.type   = v4l2_dev->_buf_type;
    request_buf.count  = v4l2_dev->_buf_count;
    request_buf.memory = v4l2_dev->_memory_type;

    XCAM_LOG_INFO("request buffers in device(%s): type: %d, count: %d, mem_type: %d",
                  v4l2_dev->_name, request_buf.type, request_buf.count,
                  request_buf.memory);

    if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_REQBUFS, &request_buf) < 0) {
        XCAM_LOG_INFO("device(%s) starts failed on VIDIOC_REQBUFS", v4l2_dev->_name);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    XCAM_LOG_INFO("device(%s) request buffer count: %d", v4l2_dev->_name,
                  request_buf.count);

    if (request_buf.count != v4l2_dev->_buf_count) {
        XCAM_LOG_INFO(
            "device(%s) request buffer count doesn't match user settings, reset buffer count to %d",
            v4l2_dev->_name, request_buf.count);
        v4l2_dev->_buf_count = request_buf.count;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type)
        v4l2_dev->_planes = (struct v4l2_plane*)aiq_mallocz(
            v4l2_dev->_buf_count * v4l2_dev->_mplanes_count * sizeof(struct v4l2_plane));

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _v4l_init_buffer_pool(AiqV4l2Device_t* v4l2_dev) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t i     = 0;

    if (v4l2_dev->_buf_pool) {
        aiq_free(v4l2_dev->_buf_pool);
        v4l2_dev->_buf_pool = NULL;
    }

    v4l2_dev->_buf_pool =
        (AiqV4l2Buffer_t*)aiq_mallocz(v4l2_dev->_buf_count * sizeof(AiqV4l2Buffer_t));

    for (; i < v4l2_dev->_buf_count; i++) {
        XCAM_LOG_DEBUG("allocate_buffer index: %d", i);
        ret = _v4l_allocate_buffer(v4l2_dev, &v4l2_dev->_buf_pool[i], &v4l2_dev->_format, i);
        if (ret != XCAM_RETURN_NO_ERROR) {
            break;
        }
    }

    if (i == 0) {
        XCAM_LOG_ERROR("No bufer allocated in device(%s)", v4l2_dev->_name);
        return XCAM_RETURN_ERROR_MEM;
    }

    if (i != v4l2_dev->_buf_count) {
        XCAM_LOG_WARNING("device(%s) allocate buffer count:%d failback to %d",
                         v4l2_dev->_name, v4l2_dev->_buf_count, i);
        v4l2_dev->_buf_count = i;
    }

    for (i = 0; i < v4l2_dev->_buf_count; i++) {
        struct v4l2_buffer* v4l2_buf = AiqV4l2Buffer_getBuf(&v4l2_dev->_buf_pool[i]);
        XCAM_LOG_DEBUG(
            "init_buffer_pool device(%s) index:%d, memory:%d, type:%d, length:%d, fd:%d, ptr:%p",
            v4l2_dev->_name, v4l2_buf->index, v4l2_buf->memory, v4l2_buf->type,
            v4l2_buf->length, AiqV4l2Buffer_getExpbufFd(&v4l2_dev->_buf_pool[i]),
            AiqV4l2Buffer_getExpbufUsrptr(&v4l2_dev->_buf_pool[i]));
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _v4l_fini_buffer_pool(AiqV4l2Device_t* v4l2_dev) {
    uint32_t i         = 0;
    int32_t tried_time = 0;

    if (v4l2_dev->_buf_pool) {
        for (; i < v4l2_dev->_buf_count; i++) {
            _v4l_release_buffer(v4l2_dev, &v4l2_dev->_buf_pool[i]);
        }
    }

    if (v4l2_dev->_memory_type == V4L2_MEMORY_MMAP) {
        struct v4l2_requestbuffers request_buf;
        xcam_mem_clear(request_buf);
        request_buf.type   = v4l2_dev->_buf_type;
        request_buf.count  = 0;
        request_buf.memory = v4l2_dev->_memory_type;
        do {
            if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_REQBUFS, &request_buf) < 0) {
                usleep(100);
                XCAM_LOG_ERROR("device(%s) failed on VIDIOC_REQBUFS, retry %d < 50times",
                               v4l2_dev->_name, tried_time);
                // return XCAM_RETURN_ERROR_IOCTL;
            } else {
                break;
            }
        } while (tried_time++ < 50);
    } else if (v4l2_dev->_memory_type == V4L2_MEMORY_DMABUF) {
        struct v4l2_requestbuffers request_buf;
        xcam_mem_clear(request_buf);
        request_buf.type   = v4l2_dev->_buf_type;
        request_buf.count  = 0;
        request_buf.memory = v4l2_dev->_memory_type;
        do {
            if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_REQBUFS, &request_buf) < 0) {
                usleep(100);
                XCAM_LOG_ERROR("device(%s) failed on VIDIOC_REQBUFS, retry %d < 50times",
                               v4l2_dev->_name, tried_time);
                // return XCAM_RETURN_ERROR_IOCTL;
            } else {
                break;
            }
        } while (tried_time++ < 50);
    }

    if (v4l2_dev->_buf_pool) {
        aiq_free(v4l2_dev->_buf_pool);
        v4l2_dev->_buf_pool = NULL;
    }

    if (v4l2_dev->_planes) {
        xcam_free(v4l2_dev->_planes);
        v4l2_dev->_planes = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _v4l_pre_set_format(AiqV4l2Device_t* v4l2_dev, struct v4l2_format* format) {
    XCAM_UNUSED(format);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _v4l_post_set_format(AiqV4l2Device_t* v4l2_dev, struct v4l2_format* format) {
    XCAM_UNUSED(format);
    return XCAM_RETURN_NO_ERROR;
}

int AiqV4l2Device_init(AiqV4l2Device_t* v4l2_dev, const char* name) {
    aiq_memset(v4l2_dev, 0, sizeof(AiqV4l2Device_t));
    v4l2_dev->_buf_type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_dev->_memory_type   = V4L2_MEMORY_MMAP;
    v4l2_dev->_buf_count     = XCAM_V4L2_DEFAULT_BUFFER_COUNT;
    v4l2_dev->_fd            = -1;
    v4l2_dev->_mplanes_count = FMT_NUM_PLANES;
    if (name) v4l2_dev->_name = strndup(name, XCAM_MAX_STR_SIZE);

    aiqMutex_init(&v4l2_dev->_buf_mutex);

    v4l2_dev->open            = AiqV4l2Device_open;
    v4l2_dev->close           = AiqV4l2Device_close;
    v4l2_dev->start           = AiqV4l2Device_start;
    v4l2_dev->stop            = AiqV4l2Device_stop;
    v4l2_dev->get_format      = AiqV4l2Device_getV4lFmt;
    v4l2_dev->poll_event      = AiqV4l2Device_pollEvt;
    v4l2_dev->_dequeue_buffer = AiqV4l2Device_dqbuf;
    v4l2_dev->io_control      = AiqV4l2Device_ioctl;
    v4l2_dev->prepare = AiqV4l2Device_prepare;

    return 0;
}

void AiqV4l2Device_deinit(AiqV4l2Device_t* v4l2_dev) {
    AiqV4l2Device_close(v4l2_dev);
    if (v4l2_dev->_name) {
        aiq_free(v4l2_dev->_name);
    }

    if (v4l2_dev->_planes) {
        xcam_free(v4l2_dev->_planes);
    }

    aiqMutex_deInit(&v4l2_dev->_buf_mutex);
}

bool AiqV4l2Device_setDeviceName(AiqV4l2Device_t* v4l2_dev, const char* name) {
    XCAM_ASSERT(name);

    if (AiqV4l2Device_isOpened(v4l2_dev)) {
        XCAM_LOG_WARNING("can't set device name since device opened");
        return false;
    }
    if (v4l2_dev->_name) xcam_free(v4l2_dev->_name);
    v4l2_dev->_name = strndup(name, XCAM_MAX_STR_SIZE);
    return true;
}

bool AiqV4l2Device_setSensorId(AiqV4l2Device_t* v4l2_dev, int id) {
    if (AiqV4l2Device_isOpened(v4l2_dev)) {
        XCAM_LOG_WARNING("can't set sensor id since device opened");
        return false;
    }
    v4l2_dev->_sensor_id = id;

    return true;
}

bool AiqV4l2Device_setCaptrueMode(AiqV4l2Device_t* v4l2_dev, uint32_t capture_mode) {
    if (AiqV4l2Device_isOpened(v4l2_dev)) {
        XCAM_LOG_WARNING("can't set sensor id since device opened");
        return false;
    }
    v4l2_dev->_capture_mode = capture_mode;
    return true;
}

bool AiqV4l2Device_setMplanesCount(AiqV4l2Device_t* v4l2_dev, uint32_t planes_count) {
    if (AiqV4l2Device_isOpened(v4l2_dev)) {
        XCAM_LOG_WARNING("device(%s) set mplanes count failed", v4l2_dev->_name);
        return false;
    }
    v4l2_dev->_mplanes_count = planes_count;
    return true;
}

bool AiqV4l2Device_setMemType(AiqV4l2Device_t* v4l2_dev, enum v4l2_memory type) {
    if (AiqV4l2Device_isActivated(v4l2_dev)) {
        XCAM_LOG_WARNING("device(%s) set mem type failed", v4l2_dev->_name);
        return false;
    }
    v4l2_dev->_memory_type = type;
    return true;
}

bool AiqV4l2Device_setBufType(AiqV4l2Device_t* v4l2_dev, enum v4l2_buf_type type) {
    if (AiqV4l2Device_isActivated(v4l2_dev)) {
        XCAM_LOG_WARNING("device(%s) set buf type failed", v4l2_dev->_name);
        return false;
    }
    v4l2_dev->_buf_type = type;
    return true;
}

bool AiqV4l2Device_setBufSync(AiqV4l2Device_t* v4l2_dev, bool sync) {
    if (AiqV4l2Device_isActivated(v4l2_dev)) {
        XCAM_LOG_WARNING("device(%s) set buf sync failed", v4l2_dev->_name);
        return false;
    }
    v4l2_dev->_buf_sync = sync;
    return true;
}

bool AiqV4l2Device_setBufCnt(AiqV4l2Device_t* v4l2_dev, uint32_t buf_count) {
    if (AiqV4l2Device_isActivated(v4l2_dev)) {
        XCAM_LOG_WARNING("device(%s) set buf sync failed", v4l2_dev->_name);
        return false;
    }
    v4l2_dev->_buf_count = buf_count;
    return true;
}

bool AiqV4l2Device_setFramerate(AiqV4l2Device_t* v4l2_dev, uint32_t n, uint32_t d) {
    if (v4l2_dev->_format.fmt.pix.pixelformat) {
        XCAM_LOG_WARNING("device(%s) set framerate failed since formatted was already set.",
                         v4l2_dev->_name);
        return false;
    }

    v4l2_dev->_fps_n = n;
    v4l2_dev->_fps_d = d;

    return true;
}

XCamReturn AiqV4l2Device_queryCap(AiqV4l2Device_t* v4l2_dev, struct v4l2_capability* cap) {
    int ret = 0;

    XCAM_FAIL_RETURN(ERROR, AiqV4l2Device_isOpened(v4l2_dev), XCAM_RETURN_ERROR_FILE,
                     "Cannot query cap from v4l2 device while it is closed.");

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_QUERYCAP, cap);

    if (ret < 0) {
        XCAM_LOG_ERROR("VIDIOC_QUERYCAP returned: %d (%s)", ret, strerror(errno));
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (cap->capabilities & V4L2_CAP_VIDEO_CAPTURE)
        v4l2_dev->_buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    else if (cap->capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
        v4l2_dev->_buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    else if (cap->capabilities & V4L2_CAP_VIDEO_OUTPUT)
        v4l2_dev->_buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    else if (cap->capabilities & V4L2_CAP_VIDEO_OUTPUT_MPLANE)
        v4l2_dev->_buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    else if (cap->capabilities & V4L2_CAP_META_CAPTURE)
        v4l2_dev->_buf_type = V4L2_BUF_TYPE_META_CAPTURE;
    else if (cap->capabilities & V4L2_CAP_META_OUTPUT)
        v4l2_dev->_buf_type = V4L2_BUF_TYPE_META_OUTPUT;
    else {
        XCAM_LOG_ERROR("@%s: unsupported buffer type.", __FUNCTION__);
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    XCAM_LOG_INFO("------------------------------");
    XCAM_LOG_INFO("driver:       '%s'", cap->driver);
    XCAM_LOG_INFO("card:         '%s'", cap->card);
    XCAM_LOG_INFO("bus_info:     '%s'", cap->bus_info);
    XCAM_LOG_INFO("version:      %x", cap->version);
    XCAM_LOG_INFO("capabilities: %x", cap->capabilities);
    XCAM_LOG_INFO("device caps:  %x", cap->device_caps);
    XCAM_LOG_INFO("buffer type   %d", v4l2_dev->_buf_type);
    XCAM_LOG_INFO("------------------------------");

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_open(AiqV4l2Device_t* v4l2_dev, bool nonblock) {
    struct v4l2_streamparm param;

    if (AiqV4l2Device_isOpened(v4l2_dev)) {
        XCAM_LOG_DEBUG("device(%s) was already opened", v4l2_dev->_name);
        return XCAM_RETURN_NO_ERROR;
    }

    if (!v4l2_dev->_name) {
        XCAM_LOG_DEBUG("v4l2 device open failed, there's no device name");
        return XCAM_RETURN_ERROR_PARAM;
    }
    if (nonblock)
        v4l2_dev->_fd = open(v4l2_dev->_name, O_RDWR | O_CLOEXEC | O_NONBLOCK);
    else
        v4l2_dev->_fd = open(v4l2_dev->_name, O_RDWR | O_CLOEXEC);
    if (v4l2_dev->_fd == -1) {
        XCAM_LOG_ERROR("open device(%s) failed", v4l2_dev->_name);
        return XCAM_RETURN_ERROR_IOCTL;
    } else {
        XCAM_LOG_DEBUG("open device(%s) successed, fd: %d", v4l2_dev->_name, v4l2_dev->_fd);
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
    if (v4l2_dev->_name && strstr(v4l2_dev->_name, "video")) {
        AiqV4l2Device_queryCap(v4l2_dev, &cap);
        // get default foramt
        AiqV4l2Device_getV4lFmt(v4l2_dev, &v4l2_dev->_format);
        if (v4l2_dev->_buf_type == V4L2_BUF_TYPE_META_CAPTURE) v4l2_dev->_buf_sync = true;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_close(AiqV4l2Device_t* v4l2_dev) {
    if (!AiqV4l2Device_isOpened(v4l2_dev)) return XCAM_RETURN_NO_ERROR;
    close(v4l2_dev->_fd);
    v4l2_dev->_fd = -1;

    XCAM_LOG_INFO("device(%s) closed", v4l2_dev->_name);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_getCrop(AiqV4l2Device_t* v4l2_dev, struct v4l2_crop* crop) {
    int ret = 0;
    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));
    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_G_CROP, crop);
    if (ret < 0) {
        XCAM_LOG_ERROR("subdev(%s) VIDIOC_G_CROP failed", v4l2_dev->_name);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_setCrop(AiqV4l2Device_t* v4l2_dev, struct v4l2_crop* crop) {
    int ret = 0;
    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));
    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_S_CROP, crop);
    if (ret < 0) {
        XCAM_LOG_ERROR("subdev(%s) VIDIOC_S_CROP failed", v4l2_dev->_name);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}
XCamReturn AiqV4l2Device_setSelection(AiqV4l2Device_t* v4l2_dev, struct v4l2_selection* select) {
    int ret = 0;
    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));
    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_S_SELECTION, select);
    if (ret < 0) {
        XCAM_LOG_ERROR("videodev (%s) VIDIOC_S_SELECTION failed", v4l2_dev->_name);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_getSelection(AiqV4l2Device_t* v4l2_dev, struct v4l2_selection* select) {
    int ret = 0;
    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));
    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_G_SELECTION, select);
    if (ret < 0) {
        XCAM_LOG_ERROR("videodev (%s) VIDIOC_G_SELECTION failed", v4l2_dev->_name);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_getV4lFmt(AiqV4l2Device_t* v4l2_dev, struct v4l2_format* format) {
    if (AiqV4l2Device_isActivated(v4l2_dev)) {
        *format = v4l2_dev->_format;
        return XCAM_RETURN_NO_ERROR;
    }

    if (!AiqV4l2Device_isOpened(v4l2_dev)) return XCAM_RETURN_ERROR_IOCTL;

    xcam_mem_clear(*format);
    format->type = v4l2_dev->_buf_type;

    if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_G_FMT, format) < 0) {
        // FIXME: also log the device name?
        XCAM_LOG_ERROR("Fail to get format via ioctl VIDVIO_G_FMT.");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_setV4lFmt(AiqV4l2Device_t* v4l2_dev, struct v4l2_format* format) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    XCAM_FAIL_RETURN(ERROR, !AiqV4l2Device_isActivated(v4l2_dev), XCAM_RETURN_ERROR_PARAM,
                     "Cannot set format to v4l2 device while it is active.");

    XCAM_FAIL_RETURN(ERROR, AiqV4l2Device_isOpened(v4l2_dev), XCAM_RETURN_ERROR_FILE,
                     "Cannot set format to v4l2 device while it is closed.");

    struct v4l2_format tmp_format = *format;

    ret = _v4l_pre_set_format(v4l2_dev, format);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING("device(%s) pre_set_format failed", v4l2_dev->_name);
        // RKTODO::rkisp no need to do set format op for subdev.
        // return ret;
    }

    if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_S_FMT, format) < 0) {
        if (errno == EBUSY) {
            // TODO log device name
            XCAM_LOG_ERROR("Video device is busy, fail to set format.");
        } else {
            // TODO log format details and errno
            XCAM_LOG_ERROR("Fail to set format: %s", strerror(errno));
        }

        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (tmp_format.fmt.pix.width != format->fmt.pix.width ||
        tmp_format.fmt.pix.height != format->fmt.pix.height) {
        XCAM_LOG_ERROR("device(%s) set v4l2 format failed, supported format: width:%d, height:%d",
                       v4l2_dev->_name, format->fmt.pix.width, format->fmt.pix.height);

        return XCAM_RETURN_ERROR_PARAM;
    }

    while (v4l2_dev->_fps_n && v4l2_dev->_fps_d) {
        struct v4l2_streamparm param;
        xcam_mem_clear(param);
        param.type = v4l2_dev->_buf_type;
        if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_G_PARM, &param) < 0) {
            XCAM_LOG_WARNING("device(%s) set framerate failed on VIDIOC_G_PARM but continue",
                             v4l2_dev->_name);
            break;
        }

        if (!(param.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)) break;

        param.parm.capture.timeperframe.numerator   = v4l2_dev->_fps_d;
        param.parm.capture.timeperframe.denominator = v4l2_dev->_fps_n;

        if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_S_PARM, &param) < 0) {
            XCAM_LOG_WARNING("device(%s) set framerate failed on VIDIOC_S_PARM but continue",
                             v4l2_dev->_name);
            break;
        }
        v4l2_dev->_fps_n = param.parm.capture.timeperframe.denominator;
        v4l2_dev->_fps_d = param.parm.capture.timeperframe.numerator;
        XCAM_LOG_INFO("device(%s) set framerate(%d/%d)", v4l2_dev->_name,
                      v4l2_dev->_fps_n, v4l2_dev->_fps_d);

        // exit here, otherwise it is an infinite loop
        break;
    }

    ret = _v4l_post_set_format(v4l2_dev, format);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING("device(%s) post_set_format failed", v4l2_dev->_name);
        return ret;
    }

    v4l2_dev->_format = *format;
    XCAM_LOG_INFO(
        "device(%s) set format(w:%d, h:%d, pixelformat:%s, bytesperline:%d,image_size:%d)",
        v4l2_dev->_name, format->fmt.pix.width, format->fmt.pix.height,
        xcam_fourcc_to_string(format->fmt.pix.pixelformat), format->fmt.pix.bytesperline,
        format->fmt.pix.sizeimage);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_setFmt(AiqV4l2Device_t* v4l2_dev, uint32_t width, uint32_t height,
                                uint32_t pixelformat, enum v4l2_field field,
                                uint32_t bytes_perline) {
    XCAM_LOG_INFO("device(%s) set format(w:%d, h:%d, pixelformat:%s, bytesperline:%d)",
                  (v4l2_dev->_name), width, height, xcam_fourcc_to_string(pixelformat),
                  bytes_perline);

    struct v4l2_format format;
    xcam_mem_clear(format);

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
       format.type                   = v4l2_dev->_buf_type;
       format.fmt.pix_mp.width       = width;
       format.fmt.pix_mp.height      = height;
       format.fmt.pix_mp.pixelformat = pixelformat;
       format.fmt.pix_mp.field       = field;
       format.fmt.pix_mp.num_planes  = v4l2_dev->_mplanes_count;

       if (bytes_perline != 0) {
           for (int i = 0; i < v4l2_dev->_mplanes_count; i++) {
               format.fmt.pix_mp.plane_fmt[i].bytesperline = bytes_perline;
           }
       }
    } else {
       format.type                = v4l2_dev->_buf_type;
       format.fmt.pix.width       = width;
       format.fmt.pix.height      = height;
       format.fmt.pix.pixelformat = pixelformat;
       format.fmt.pix.field       = field;

       if (bytes_perline != 0) format.fmt.pix.bytesperline = bytes_perline;
    }
    return AiqV4l2Device_setV4lFmt(v4l2_dev, &format);
}

int AiqV4l2Device_enumFmts(AiqV4l2Device_t* v4l2_dev, struct v4l2_fmtdesc* fmts, uint32_t max) {
    struct v4l2_fmtdesc format;
    uint32_t i = 0, cnts = 0;

    while (1) {
        xcam_mem_clear(format);
        format.index = i++;
        format.type  = v4l2_dev->_buf_type;
        if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_ENUM_FMT, &format) < 0) {
            if (errno == EINVAL)
                break;
            else {  // error
                XCAM_LOG_DEBUG("enum formats failed");
                return cnts;
            }
        }
        *fmts++ = format;
        cnts++;
        if (cnts >= max) break;
    }

    return cnts;
}

XCamReturn AiqV4l2Device_start(AiqV4l2Device_t* v4l2_dev, bool prepared) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t i     = 0;

    if (!prepared) {
        // request buffer first
        ret = _v4l_request_buffer(v4l2_dev);
        XCAM_FAIL_RETURN(ERROR, ret == XCAM_RETURN_NO_ERROR, ret, "device(%s) start failed",
                         (v4l2_dev->_name));
        v4l2_dev->_queued_bufcnt = 0;
        // alloc buffers
        ret = _v4l_init_buffer_pool(v4l2_dev);
        XCAM_FAIL_RETURN(ERROR, ret == XCAM_RETURN_NO_ERROR, ret, "device(%s) start failed",
                         (v4l2_dev->_name));
        if (!V4L2_TYPE_IS_OUTPUT(v4l2_dev->_buf_type) &&
            (v4l2_dev->_buf_type != V4L2_BUF_TYPE_META_OUTPUT)) {
            // queue all buffers
            for (i = 0; i < v4l2_dev->_buf_count; ++i) {
                AiqV4l2Buffer_t* buf = &v4l2_dev->_buf_pool[i];
                XCAM_ASSERT(AiqV4l2Buffer_getBuf(buf)->index == i);
                ret = AiqV4l2Device_qbuf(v4l2_dev, buf, false);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    XCAM_LOG_ERROR("device(%s) start failed on queue index:%d",
                                   (v4l2_dev->_name), i);
                    AiqV4l2Device_stop(v4l2_dev);
                    return ret;
                }
            }
        }
    }
    // stream on
    if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_STREAMON, &v4l2_dev->_buf_type) < 0) {
        XCAM_LOG_ERROR("device(%s) start failed on VIDIOC_STREAMON, fd=%d",
                       (v4l2_dev->_name), v4l2_dev->_fd);
        AiqV4l2Device_stop(v4l2_dev);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    v4l2_dev->_active = true;
    XCAM_LOG_INFO("device(%s) started successfully", (v4l2_dev->_name));
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_stop(AiqV4l2Device_t* v4l2_dev) {
    aiqMutex_lock(&v4l2_dev->_buf_mutex);
    XCAM_LOG_INFO("device(%s) stop, already start: %d", (v4l2_dev->_name),
                  v4l2_dev->_active);

    // stream off
    if (v4l2_dev->_active) {
        if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_STREAMOFF, &v4l2_dev->_buf_type) < 0) {
            XCAM_LOG_WARNING("device(%s) steamoff failed", (v4l2_dev->_name));
        }
        v4l2_dev->_active = false;
        /* while (_queued_bufcnt > 0) { */
        /*     struct v4l2_buffer v4l2_buf; */
        /*     struct v4l2_plane planes[_mplanes_count]; */

        /*     xcam_mem_clear (v4l2_buf); */
        /*     v4l2_buf.type = _buf_type; */
        /*     v4l2_buf.memory = _memory_type; */

        /*     if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == _buf_type || */
        /*             V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == _buf_type) { */
        /*         memset(planes, 0, sizeof(struct v4l2_plane) * _mplanes_count); */
        /*         v4l2_buf.m.planes = planes; */
        /*         v4l2_buf.length = _mplanes_count; */
        /*     } */

        /*     if (this->io_control (VIDIOC_DQBUF, &v4l2_buf) < 0) { */
        /*         XCAM_LOG_WARNING ("device(%s) fail to dequeue buffer.", (v4l2_dev->_name)); */
        /*     } */
        /*     _queued_bufcnt--; */
        /* } */
        /* fini_buffer_pool (); */
        /* release the shared buf between mipi tx and rx */
#if 0  // moved to fini_buffer_pool, buffer may be requested at prepare stage
        if (_memory_type == V4L2_MEMORY_DMABUF) {
            struct v4l2_requestbuffers request_buf;
            xcam_mem_clear (request_buf);
            request_buf.type = _buf_type;
            request_buf.count = 0;
            request_buf.memory = _memory_type;
            if (io_control (VIDIOC_REQBUFS, &request_buf) < 0) {
                XCAM_LOG_ERROR ("device(%s) starts failed on VIDIOC_REQBUFS", (v4l2_dev->_name));
                //return XCAM_RETURN_ERROR_IOCTL;
            }
        }
#endif
    }

    if (v4l2_dev->_buf_count > 0) _v4l_fini_buffer_pool(v4l2_dev);

    aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_stopStreamOff(AiqV4l2Device_t* v4l2_dev) {
    aiqMutex_lock(&v4l2_dev->_buf_mutex);
    XCAM_LOG_INFO("device(%s) stop stream off", (v4l2_dev->_name));
    // stream off
    if (v4l2_dev->_active) {
        if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_STREAMOFF, &v4l2_dev->_buf_type) < 0) {
            XCAM_LOG_WARNING("device(%s) steamoff failed", (v4l2_dev->_name));
        }
        v4l2_dev->_active = false;
    }

    aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_stopFreeBuf(AiqV4l2Device_t* v4l2_dev) {
    aiqMutex_lock(&v4l2_dev->_buf_mutex);

    XCAM_LOG_INFO("device(%s) stop free buffer", (v4l2_dev->_name));
    if (v4l2_dev->_buf_count > 0) _v4l_fini_buffer_pool(v4l2_dev);

    aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_prepare(AiqV4l2Device_t* v4l2_dev) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t i     = 0;
    //
    // request buffer first
    ret = _v4l_request_buffer(v4l2_dev);
    XCAM_FAIL_RETURN(ERROR, ret == XCAM_RETURN_NO_ERROR, ret, "device(%s) start failed",
                     (v4l2_dev->_name));
    v4l2_dev->_queued_bufcnt = 0;
    // alloc buffers
    ret = _v4l_init_buffer_pool(v4l2_dev);
    XCAM_FAIL_RETURN(ERROR, ret == XCAM_RETURN_NO_ERROR, ret, "device(%s) start failed",
                     (v4l2_dev->_name));
#ifndef USE_RAWSTREAM_LIB
    if (!V4L2_TYPE_IS_OUTPUT(v4l2_dev->_buf_type) &&
        (v4l2_dev->_buf_type != V4L2_BUF_TYPE_META_OUTPUT)) {
#else
    if (!V4L2_TYPE_IS_OUTPUT(v4l2_dev->_buf_type) &&
        (v4l2_dev->_memory_type != V4L2_MEMORY_DMABUF) &&
        (v4l2_dev->_buf_type != V4L2_BUF_TYPE_META_OUTPUT)) {
#endif
        // queue all buffers
        for (i = 0; i < v4l2_dev->_buf_count; ++i) {
            AiqV4l2Buffer_t* buf = &v4l2_dev->_buf_pool[i];
            XCAM_ASSERT(AiqV4l2Buffer_getBuf(buf)->index == i);
            ret = AiqV4l2Device_qbuf(v4l2_dev, buf, false);
            if (ret != XCAM_RETURN_NO_ERROR) {
                XCAM_LOG_ERROR("device(%s) start failed on queue index:%d",
                               (v4l2_dev->_name), i);
                AiqV4l2Device_stop(v4l2_dev);
                return ret;
            }
        }
    }
    return ret;
}

int AiqV4l2Device_pollEvt(AiqV4l2Device_t* v4l2_dev, int timeout_msec, int stop_fd) {
    int num_fds = stop_fd == -1 ? 1 : 2;
    struct pollfd poll_fds[num_fds];
    int ret = 0;

    XCAM_ASSERT(v4l2_dev->_fd > 0);

    memset(poll_fds, 0, sizeof(poll_fds));
    poll_fds[0].fd     = v4l2_dev->_fd;
    poll_fds[0].events = (POLLPRI | POLLIN | POLLOUT | POLLERR | POLLNVAL | POLLHUP);

    if (stop_fd != -1) {
        poll_fds[1].fd      = stop_fd;
        poll_fds[1].events  = POLLPRI | POLLIN | POLLOUT;
        poll_fds[1].revents = 0;
    }

    ret = poll(poll_fds, num_fds, timeout_msec);
    if (stop_fd != -1) {
        if ((poll_fds[1].revents & POLLIN) || (poll_fds[1].revents & POLLPRI)) {
            XCAM_LOG_DEBUG("%s: Poll returning from flush", __FUNCTION__);
            return POLL_STOP_RET;
        }
    }

    if (ret > 0 && (poll_fds[0].revents & (POLLERR | POLLNVAL | POLLHUP))) {
        XCAM_LOG_DEBUG("v4l2 subdev(%s) polled error", (v4l2_dev->_name));
        return -1;
    }

    return ret;
}
AiqV4l2Buffer_t* AiqV4l2Device_getBufByIndex(AiqV4l2Device_t* v4l2_dev, int index) {
    AiqV4l2Buffer_t* buf = NULL;

    aiqMutex_lock(&v4l2_dev->_buf_mutex);
    buf = &v4l2_dev->_buf_pool[index];
    aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    return buf;
}

AiqV4l2Buffer_t* AiqV4l2Device_dqbuf(AiqV4l2Device_t* v4l2_dev) {
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[v4l2_dev->_mplanes_count];
    AiqV4l2Buffer_t* buf = NULL;

    if (!AiqV4l2Device_isActivated(v4l2_dev)) {
        XCAM_LOG_DEBUG("device(%s) dequeue buffer failed since not activated",
                       (v4l2_dev->_name));
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

    if (v4l2_dev->_buf_sync)
        v4l2_buf.flags = V4L2_BUF_FLAG_NO_CACHE_INVALIDATE | V4L2_BUF_FLAG_NO_CACHE_CLEAN;

    if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_DQBUF, &v4l2_buf) < 0) {
        XCAM_LOG_ERROR("device(%s) fail to dequeue buffer.", (v4l2_dev->_name));
        return NULL;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
        XCAM_LOG_DEBUG("device(%s) multi planar dequeue buffer index:%d, length: %dn",
                       (v4l2_dev->_name), v4l2_buf.index, v4l2_buf.m.planes[0].length);
        if (V4L2_MEMORY_DMABUF == v4l2_dev->_memory_type) {
            XCAM_LOG_DEBUG("device(%s) multi planar index:%d, fd: %d", (v4l2_dev->_name),
                           v4l2_buf.index, v4l2_buf.m.planes[0].m.fd);
        }
    } else {
        XCAM_LOG_DEBUG("device(%s) dequeue buffer index:%d, length: %d", (v4l2_dev->_name),
                       v4l2_buf.index, v4l2_buf.length);
    }

    if (v4l2_buf.index > v4l2_dev->_buf_count) {
        XCAM_LOG_ERROR("device(%s) dequeue wrong buffer index:%d", (v4l2_dev->_name),
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
    } else {
        AiqV4l2Buffer_setLength(buf, v4l2_buf.length);
    }
    v4l2_dev->_queued_bufcnt--;

    aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    return buf;
}

XCamReturn AiqV4l2Device_qbuf(AiqV4l2Device_t* v4l2_dev, AiqV4l2Buffer_t* buf, bool locked) {
    struct v4l2_buffer v4l2_buf_s;
    struct v4l2_buffer* v4l2_buf = &v4l2_buf_s;
    struct v4l2_plane planes[v4l2_dev->_mplanes_count];
    uint32_t sequence = 0;

    if (!locked) aiqMutex_lock(&v4l2_dev->_buf_mutex);
    if (buf) sequence = AiqV4l2Buffer_getSequence(buf);

    XCAM_ASSERT(buf);
    AiqV4l2Buffer_reset(buf);

	// TODO: copied for params buf, or the contents of v4l2buf will
	// be modified by VIDIOC_QBUF.
    *v4l2_buf = *AiqV4l2Buffer_getBuf(buf);

    XCAM_ASSERT(v4l2_buf->index < v4l2_dev->_buf_count);

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == v4l2_dev->_buf_type ||
        V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE == v4l2_dev->_buf_type) {
        memset(planes, 0, sizeof(struct v4l2_plane) * v4l2_dev->_mplanes_count);
        v4l2_buf->m.planes = planes;
        planes[0]          = v4l2_buf->m.planes[0];
        XCAM_LOG_DEBUG(
            "device(%s) queue buffer index:%d, memory:%d, type:%d, multiply planar:%d, length:%d, "
            "fd:%d, ptr:%p",
            (v4l2_dev->_name), v4l2_buf->index, v4l2_buf->memory, v4l2_buf->type,
            v4l2_buf->length, v4l2_buf->m.planes[0].length, AiqV4l2Buffer_getExpbufFd(buf),
            AiqV4l2Buffer_getExpbufUsrptr(buf));
    } else {
        XCAM_LOG_DEBUG("device(%s) queue buffer index:%d, memory:%d, type:%d, length:%d, fd:%d",
                       (v4l2_dev->_name), v4l2_buf->index, v4l2_buf->memory, v4l2_buf->type,
                       v4l2_buf->length, AiqV4l2Buffer_getExpbufFd(buf));
    }

    if (v4l2_buf->type == V4L2_BUF_TYPE_META_OUTPUT) v4l2_buf->bytesused = v4l2_buf->length;
    if (v4l2_buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
        v4l2_buf->m.planes[0].bytesused = v4l2_buf->m.planes[0].length;
        if (V4L2_MEMORY_DMABUF == v4l2_dev->_memory_type)
            v4l2_buf->m.planes[0].m.fd = AiqV4l2Buffer_getExpbufFd(buf);
        else if (V4L2_MEMORY_USERPTR == v4l2_dev->_memory_type)
            v4l2_buf->m.planes[0].m.userptr = AiqV4l2Buffer_getExpbufUsrptr(buf);
    }

    v4l2_dev->_queued_bufcnt++;
    AiqV4l2Buffer_setQueued(buf, true);
    if (!locked) aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    if (v4l2_dev->_buf_sync)
        v4l2_buf->flags = V4L2_BUF_FLAG_NO_CACHE_INVALIDATE | V4L2_BUF_FLAG_NO_CACHE_CLEAN;

    if ((*v4l2_dev->io_control)(v4l2_dev, VIDIOC_QBUF, v4l2_buf) < 0) {
        XCAM_LOG_ERROR("%s fail to enqueue buffer index:%d.", (v4l2_dev->_name),
                       v4l2_buf->index);
        // restore buf status
        {
            if (!locked) aiqMutex_lock(&v4l2_dev->_buf_mutex);
            /* SmartLock auto_lock(_buf_mutex); */
            AiqV4l2Buffer_setQueued(buf, false);
            v4l2_dev->_queued_bufcnt--;
            if (!locked) aiqMutex_unlock(&v4l2_dev->_buf_mutex);
        }

        return XCAM_RETURN_ERROR_IOCTL;
    }

    AiqV4l2Buffer_setSequence(buf, sequence);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_returnBuf(AiqV4l2Device_t* v4l2_dev, AiqV4l2Buffer_t* buf) {
    aiqMutex_lock(&v4l2_dev->_buf_mutex);

    if (V4L2_TYPE_IS_OUTPUT(AiqV4l2Buffer_getBuf(buf)->type) ||
        (AiqV4l2Buffer_getBuf(buf)->type == V4L2_BUF_TYPE_META_OUTPUT)) {
        AiqV4l2Buffer_reset(buf);
        aiqMutex_unlock(&v4l2_dev->_buf_mutex);
        return XCAM_RETURN_NO_ERROR;
    } else {
        if (!v4l2_dev->_active) {
            AiqV4l2Buffer_reset(buf);
        } else {
            aiqMutex_unlock(&v4l2_dev->_buf_mutex);
            return AiqV4l2Device_qbuf(v4l2_dev, buf, true);
        }
    }

    aiqMutex_unlock(&v4l2_dev->_buf_mutex);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_returnBufToPool(AiqV4l2Device_t* v4l2_dev, AiqV4l2Buffer_t* buf) {
    aiqMutex_lock(&v4l2_dev->_buf_mutex);
    XCAM_ASSERT(buf);
    AiqV4l2Buffer_reset(buf);
    aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    return XCAM_RETURN_NO_ERROR;
}

AiqV4l2Buffer_t* AiqV4l2Device_getBuf(AiqV4l2Device_t* v4l2_dev, int index) {
    AiqV4l2Buffer_t* buf = NULL;
    uint32_t i           = 0;

    aiqMutex_lock(&v4l2_dev->_buf_mutex);

    if (v4l2_dev->_buf_count <= 0) {
        goto out;
    }

    if (index != -1 && !(AiqV4l2Buffer_getQueued(&v4l2_dev->_buf_pool[index]))) {
        buf = &v4l2_dev->_buf_pool[index];
        AiqV4l2Buffer_setQueued(buf, true);
        goto out;
    }

    for (i = 0; i < v4l2_dev->_buf_count; i++) {
        if (!(AiqV4l2Buffer_getQueued(&v4l2_dev->_buf_pool[i]))) {
            buf = &v4l2_dev->_buf_pool[i];
            AiqV4l2Buffer_setQueued(buf, true);
            break;
        }
    }

out:
    aiqMutex_unlock(&v4l2_dev->_buf_mutex);

    return buf;
}

int AiqV4l2Device_ioctl(AiqV4l2Device_t* v4l2_dev, unsigned long cmd, void* arg) {
    if (v4l2_dev->_fd <= 0) return -1;

    return xcam_device_ioctl(v4l2_dev->_fd, cmd, arg);
}

XCamReturn AiqV4l2Device_subscribeEvt(AiqV4l2Device_t* v4l2_dev, int event) {
    struct v4l2_event_subscription sub;
    int ret = 0;

    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    xcam_mem_clear(sub);
    sub.type = event;

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        XCAM_LOG_ERROR("subdev(%s) subscribe event(%d) failed", (v4l2_dev->_name), event);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_unsubscribeEvt(AiqV4l2Device_t* v4l2_dev, int event) {
    struct v4l2_event_subscription sub;
    int ret = 0;

    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    xcam_mem_clear(sub);
    sub.type = event;

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        XCAM_LOG_DEBUG("subdev(%s) unsubscribe event(%d) failed", (v4l2_dev->_name), event);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_subscribeEvt2(AiqV4l2Device_t* v4l2_dev, int event, int id) {
    struct v4l2_event_subscription sub;
    int ret = 0;

    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    xcam_mem_clear(sub);
    sub.type = event;
    sub.id = id;

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        XCAM_LOG_ERROR("subdev(%s) subscribe event(%d) failed", (v4l2_dev->_name), event);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_unsubscribeEvt2(AiqV4l2Device_t* v4l2_dev, int event, int id) {
    struct v4l2_event_subscription sub;
    int ret = 0;

    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    xcam_mem_clear(sub);
    sub.type = event;
    sub.id = id;

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        XCAM_LOG_DEBUG("subdev(%s) unsubscribe event(%d) failed", (v4l2_dev->_name), event);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2Device_dqEvt(AiqV4l2Device_t* v4l2_dev, struct v4l2_event* event) {
    int ret = 0;
    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_DQEVENT, event);
    if (ret < 0) {
        XCAM_LOG_DEBUG("subdev(%s) dequeue event failed", (v4l2_dev->_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2SubDevice_getSelection(AiqV4l2SubDevice_t* v4l2_subdev, int pad, uint32_t target,
                                         struct v4l2_subdev_selection* select) {
    int ret                   = 0;
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)v4l2_subdev;
    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    select->pad    = pad;
    select->which  = V4L2_SUBDEV_FORMAT_ACTIVE;
    select->target = target;

    ret = (*v4l2_dev->io_control)((AiqV4l2Device_t*)v4l2_subdev, VIDIOC_SUBDEV_G_SELECTION, select);
    if (ret < 0) {
        XCAM_LOG_DEBUG("subdev(%s) get selection failed", (v4l2_dev->_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2SubDevice_setSelection(AiqV4l2SubDevice_t* v4l2_subdev,
                                         struct v4l2_subdev_selection* aSelection) {
    int ret                   = 0;
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)v4l2_subdev;
    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    XCAM_LOG_DEBUG(
        "VIDIOC_SUBDEV_S_SELECTION: which: %d, pad: %d, target: 0x%x, "
        "flags: 0x%x, rect left: %d, rect top: %d, width: %d, height: %d",
        aSelection->which, aSelection->pad, aSelection->target, aSelection->flags,
        aSelection->r.left, aSelection->r.top, aSelection->r.width, aSelection->r.height);

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_SUBDEV_S_SELECTION, aSelection);
    if (ret < 0) {
        XCAM_LOG_ERROR("subdev(%s) VIDIOC_SUBDEV_S_SELECTION failed", (v4l2_dev->_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2SubDevice_setFormat(AiqV4l2SubDevice_t* v4l2_subdev,
                                      struct v4l2_subdev_format* aFormat) {
    int ret                   = 0;
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)v4l2_subdev;
    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    XCAM_LOG_DEBUG(
        "VIDIOC_SUBDEV_S_FMT: pad: %d, which: %d, width: %d, "
        "height: %d, format: 0x%x, field: %d, color space: %d",
        aFormat->pad, aFormat->which, aFormat->format.width, aFormat->format.height,
        aFormat->format.code, aFormat->format.field, aFormat->format.colorspace);

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_SUBDEV_S_FMT, aFormat);
    if (ret < 0) {
        XCAM_LOG_ERROR("subdev(%s) VIDIOC_SUBDEV_S_FMT failed: %s", (v4l2_dev->_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2SubDevice_getFormat(AiqV4l2SubDevice_t* v4l2_subdev,
                                      struct v4l2_subdev_format* aFormat) {
    int ret                   = 0;
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)v4l2_subdev;

    XCAM_ASSERT(AiqV4l2Device_isOpened(v4l2_dev));

    ret = (*v4l2_dev->io_control)(v4l2_dev, VIDIOC_SUBDEV_G_FMT, aFormat);
    if (ret < 0) {
        XCAM_LOG_ERROR("subdev(%s) VIDIOC_SUBDEV_G_FMT failed: %s", (v4l2_dev->_name));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    XCAM_LOG_DEBUG(
        "VIDIOC_SUBDEV_G_FMT: pad: %d, which: %d, width: %d, "
        "height: %d, format: 0x%x, field: %d, color space: %d",
        aFormat->pad, aFormat->which, aFormat->format.width, aFormat->format.height,
        aFormat->format.code, aFormat->format.field, aFormat->format.colorspace);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqV4l2SubDevice_start(AiqV4l2Device_t* v4l2_dev, bool prepared) {

    if (!AiqV4l2Device_isOpened(v4l2_dev))
        return XCAM_RETURN_ERROR_PARAM;
    v4l2_dev->_active = true;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqV4l2SubDevice_stop(AiqV4l2Device_t* v4l2_dev) {

    v4l2_dev->_active = false;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqV4l2SubDevice_init(AiqV4l2SubDevice_t* v4l2_subdev, const char* name) {
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)v4l2_subdev;

    aiq_memset(v4l2_subdev, 0, sizeof(AiqV4l2SubDevice_t));

    ret = AiqV4l2Device_init(v4l2_dev, name);

    if (ret) {
        XCAM_LOG_ERROR("subdev(%s) init failed", (v4l2_dev->_name));
        return ret;
    }

    v4l2_subdev->get_selection = AiqV4l2SubDevice_getSelection;
    v4l2_subdev->setFormat     = AiqV4l2SubDevice_setFormat;
    v4l2_subdev->getFormat     = AiqV4l2SubDevice_getFormat;
    v4l2_subdev->set_selection = AiqV4l2SubDevice_setSelection;
    v4l2_dev->start = AiqV4l2SubDevice_start;
    v4l2_dev->stop = AiqV4l2SubDevice_stop;

    return ret;
}

void AiqV4l2SubDevice_deinit(AiqV4l2SubDevice_t* v4l2_subdev) {
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)v4l2_subdev;
	AiqV4l2Device_deinit(v4l2_dev);
}

XCAM_END_DECLARE
