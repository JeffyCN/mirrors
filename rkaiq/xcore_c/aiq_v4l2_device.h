/*
 * aiq_v4l2_device.h
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

#ifndef C_AIQ_V4L2_DEVICE_H
#define C_AIQ_V4L2_DEVICE_H

#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>

#include "aiq_v4l2_buffer.h"
#include "c_base/aiq_mutex.h"

XCAM_BEGIN_DECLARE

struct v4l2_event;
struct v4l2_format;
struct v4l2_fmtdesc;
struct v4l2_frmsizeenum;

#define FMT_NUM_PLANES 1
#define POLL_STOP_RET  3

typedef struct AiqV4l2Device_s AiqV4l2Device_t;
struct AiqV4l2Device_s {
    char* _name;
    int _fd;
    bool _active;
    int32_t _sensor_id;
    uint32_t _capture_mode;
    enum v4l2_buf_type _buf_type;
    bool _buf_sync;
    enum v4l2_memory _memory_type;
    struct v4l2_plane* _planes;
    struct v4l2_format _format;
    uint32_t _fps_n;
    uint32_t _fps_d;
    AiqV4l2Buffer_t* _buf_pool;
    uint32_t _buf_count;
    uint32_t _queued_bufcnt;
    AiqMutex_t _buf_mutex;
    int32_t _mplanes_count;
    int _use_type;
    XCamReturn (*open)(AiqV4l2Device_t* v4l2_dev, bool nonblock);
    XCamReturn (*close)(AiqV4l2Device_t* v4l2_dev);
    XCamReturn (*prepare)(AiqV4l2Device_t* v4l2_dev);
    XCamReturn (*start)(AiqV4l2Device_t* v4l2_dev, bool prepared);
    XCamReturn (*stop)(AiqV4l2Device_t* v4l2_dev);
    XCamReturn (*get_format)(AiqV4l2Device_t* v4l2_dev, struct v4l2_format* format);
    int (*poll_event)(AiqV4l2Device_t* v4l2_dev, int timeout_msec, int stop_fd);
    AiqV4l2Buffer_t* (*_dequeue_buffer)(AiqV4l2Device_t* v4l2_dev);
    int (*io_control)(AiqV4l2Device_t* v4l2_dev, unsigned long cmd, void* arg);
};

/* start: following functions are used for both v4l2Device and v4l2SubDevice */
XCamReturn AiqV4l2Device_open(AiqV4l2Device_t* v4l2_dev, bool nonblock);
XCamReturn AiqV4l2Device_close(AiqV4l2Device_t* v4l2_dev);
int AiqV4l2Device_ioctl(AiqV4l2Device_t* v4l2_dev, unsigned long cmd, void* arg);
XCamReturn AiqV4l2Device_subscribeEvt(AiqV4l2Device_t* v4l2_dev, int event);
XCamReturn AiqV4l2Device_unsubscribeEvt(AiqV4l2Device_t* v4l2_dev, int event);
XCamReturn AiqV4l2Device_subscribeEvt2(AiqV4l2Device_t* v4l2_dev, int event, int id);
XCamReturn AiqV4l2Device_unsubscribeEvt2(AiqV4l2Device_t* v4l2_dev, int event, int id);
XCamReturn AiqV4l2Device_dqEvt(AiqV4l2Device_t* v4l2_dev, struct v4l2_event* event);
/* end: following functions are used for both v4l2Device and v4l2SubDevice */

int AiqV4l2Device_init(AiqV4l2Device_t* v4l2_dev, const char* name);
void AiqV4l2Device_deinit(AiqV4l2Device_t* v4l2_dev);
bool AiqV4l2Device_setDeviceName(AiqV4l2Device_t* v4l2_dev, const char* name);
bool AiqV4l2Device_setSensorId(AiqV4l2Device_t* v4l2_dev, int id);
bool AiqV4l2Device_setCaptrueMode(AiqV4l2Device_t* v4l2_dev, uint32_t capture_mode);
bool AiqV4l2Device_setMplanesCount(AiqV4l2Device_t* v4l2_dev, uint32_t planes_count);
XCamReturn AiqV4l2Device_queryCap(AiqV4l2Device_t* v4l2_dev, struct v4l2_capability* cap);
bool AiqV4l2Device_setBufCnt(AiqV4l2Device_t* v4l2_dev, uint32_t buf_count);

#define AiqV4l2Device_getFd(v4l2_dev) ((v4l2_dev)->_fd)

#define AiqV4l2Device_getDevName(v4l2_dev) ((v4l2_dev)->_name)

#define AiqV4l2Device_isOpened(v4l2_dev) ((v4l2_dev)->_fd != -1)

#define AiqV4l2Device_isActivated(v4l2_dev) ((v4l2_dev)->_active)

bool AiqV4l2Device_setMemType(AiqV4l2Device_t* v4l2_dev, enum v4l2_memory type);

#define AiqV4l2Device_getMemType(v4l2_dev) ((v4l2_dev)->_memory_type)

bool AiqV4l2Device_setBufType(AiqV4l2Device_t* v4l2_dev, enum v4l2_buf_type type);

#define AiqV4l2Device_getBufType(v4l2_dev) ((v4l2_dev)->_buf_type)

bool AiqV4l2Device_setBufSync(AiqV4l2Device_t* v4l2_dev, bool sync);

#define AiqV4l2Device_getSize(v4l2_dev, w, h) \
    *w = v4l2_dev->_format.fmt.pix.width;     \
    *h = v4l2_dev->_format.fmt.pix.height

#define AiqV4l2Device_getPixelFmt(v4l2_dev) ((v4l2_dev)->_format.fmt.pix.pixelformat)

#define AiqV4l2Device_getBufCnt(v4l2_dev) ((v4l2_dev)->_buf_count)

bool AiqV4l2Device_setBufCnt(AiqV4l2Device_t* v4l2_dev, uint32_t buf_count);

#define AiqV4l2Device_getQueuedBufCnt(v4l2_dev) ((v4l2_dev)->_queued_bufcnt)

bool AiqV4l2Device_setFramerate(AiqV4l2Device_t* v4l2_dev, uint32_t n, uint32_t d);

#define AiqV4l2Device_getFramerate(v4l2_dev, n, d) \
    *n = _fps_n;                                   \
    *d = _fps_d

XCamReturn AiqV4l2Device_getCrop(AiqV4l2Device_t* v4l2_dev, struct v4l2_crop* crop);
XCamReturn AiqV4l2Device_setCrop(AiqV4l2Device_t* v4l2_dev, struct v4l2_crop* crop);
XCamReturn AiqV4l2Device_setSelection(AiqV4l2Device_t* v4l2_dev, struct v4l2_selection* select);
XCamReturn AiqV4l2Device_getSelection(AiqV4l2Device_t* v4l2_dev, struct v4l2_selection* select);
XCamReturn AiqV4l2Device_getV4lFmt(AiqV4l2Device_t* v4l2_dev, struct v4l2_format* format);
XCamReturn AiqV4l2Device_setV4lFmt(AiqV4l2Device_t* v4l2_dev, struct v4l2_format* format);
XCamReturn AiqV4l2Device_setFmt(AiqV4l2Device_t* v4l2_dev, uint32_t width, uint32_t height,
                                uint32_t pixelformat, enum v4l2_field field,
                                uint32_t bytes_perline);
int AiqV4l2Device_enumFmts(AiqV4l2Device_t* v4l2_dev, struct v4l2_fmtdesc* fmts, uint32_t max);
XCamReturn AiqV4l2Device_start(AiqV4l2Device_t* v4l2_dev, bool prepared);
XCamReturn AiqV4l2Device_stop(AiqV4l2Device_t* v4l2_dev);
XCamReturn AiqV4l2Device_stopStreamOff(AiqV4l2Device_t* v4l2_dev);
XCamReturn AiqV4l2Device_stopFreeBuf(AiqV4l2Device_t* v4l2_dev);
XCamReturn AiqV4l2Device_prepare(AiqV4l2Device_t* v4l2_dev);
int AiqV4l2Device_pollEvt(AiqV4l2Device_t* v4l2_dev, int timeout_msec, int stop_fd);
AiqV4l2Buffer_t* AiqV4l2Device_getBufByIndex(AiqV4l2Device_t* v4l2_dev, int index);
AiqV4l2Buffer_t* AiqV4l2Device_dqbuf(AiqV4l2Device_t* v4l2_dev);
XCamReturn AiqV4l2Device_qbuf(AiqV4l2Device_t* v4l2_dev, AiqV4l2Buffer_t* buf, bool locked);
XCamReturn AiqV4l2Device_returnBuf(AiqV4l2Device_t* v4l2_dev, AiqV4l2Buffer_t* buf);
XCamReturn AiqV4l2Device_returnBufToPool(AiqV4l2Device_t* v4l2_dev, AiqV4l2Buffer_t* buf);
AiqV4l2Buffer_t* AiqV4l2Device_getBuf(AiqV4l2Device_t* v4l2_dev, int index);

typedef struct AiqV4l2SubDevice_s AiqV4l2SubDevice_t;
struct AiqV4l2SubDevice_s {
    AiqV4l2Device_t _v4l_base;
    XCamReturn (*get_selection)(AiqV4l2SubDevice_t* v4l2_subdev, int pad, uint32_t target,
                                struct v4l2_subdev_selection* select);
    XCamReturn (*setFormat)(AiqV4l2SubDevice_t* v4l2_subdev, struct v4l2_subdev_format* aFormat);
    XCamReturn (*getFormat)(AiqV4l2SubDevice_t* v4l2_subdev, struct v4l2_subdev_format* aFormat);
    XCamReturn (*set_selection)(AiqV4l2SubDevice_t* v4l2_subdev,
                                struct v4l2_subdev_selection* select);
};

XCamReturn AiqV4l2SubDevice_init(AiqV4l2SubDevice_t* v4l2_subdev, const char* name);
void AiqV4l2SubDevice_deinit(AiqV4l2SubDevice_t* v4l2_subdev);
XCamReturn AiqV4l2SubDevice_getSelection(AiqV4l2SubDevice_t* v4l2_subdev, int pad, uint32_t target,
                                         struct v4l2_subdev_selection* select);
XCamReturn AiqV4l2SubDevice_setSelection(AiqV4l2SubDevice_t* v4l2_subdev,
                                         struct v4l2_subdev_selection* aSelection);
XCamReturn AiqV4l2SubDevice_setFormat(AiqV4l2SubDevice_t* v4l2_subdev,
                                      struct v4l2_subdev_format* aFormat);
XCamReturn AiqV4l2SubDevice_getFormat(AiqV4l2SubDevice_t* v4l2_subdev,
                                      struct v4l2_subdev_format* aFormat);

#define AiqV4l2SubDevice_open(v4l2_subdev, nonblocked) \
    AiqV4l2Device_open((AiqV4l2Device_t*)(v4l2_subdev), nonblocked)

#define AiqV4l2SubDevice_close(v4l2_subdev) AiqV4l2Device_close((AiqV4l2Device_t*)(v4l2_subdev))

#define AiqV4l2SubDevice_ioctl(v4l2_subdev, cmd, arg) \
    AiqV4l2Device_ioctl((AiqV4l2Device_t*)(v4l2_subdev), cmd, arg)

#define AiqV4l2SubDevice_subscribeEvt(v4l2_subdev, event) \
    AiqV4l2Device_subscribeEvt((AiqV4l2Device_t*)(v4l2_subdev), event)

#define AiqV4l2SubDevice_unsubscribeEvt(v4l2_subdev, event) \
    AiqV4l2Device_unsubscribeEvt((AiqV4l2Device_t*)(v4l2_subdev), event)

#define AiqV4l2SubDevice_dqEvt(v4l2_subdev, event) \
    AiqV4l2Device_dqEvt((AiqV4l2Device_t*)(v4l2_subdev), event)

XCAM_END_DECLARE

#endif
