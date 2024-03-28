/*
 *  Copyright (c) 2019 Rockchip Corporation
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
#ifndef _STREAM_H_
#define _STREAM_H_
#include <sys/mman.h>
#include <v4l2_device.h>
#include "poll_thread.h"
#include "xcam_mutex.h"

using namespace XCam;

namespace RkRawStream {

#define LOGD_RKSTREAM(...) XCAM_MODULE_LOG_DEBUG(XCORE_LOG_MODULE_RKRAWSTREAM, 0xff, ##__VA_ARGS__)
#define LOGI_RKSTREAM(...) XCAM_MODULE_LOG_INFO(XCORE_LOG_MODULE_RKRAWSTREAM, 0xff, ##__VA_ARGS__)
#define LOGW_RKSTREAM(...) XCAM_MODULE_LOG_WARNING(XCORE_LOG_MODULE_RKRAWSTREAM, 0xff, ##__VA_ARGS__)
#define LOGE_RKSTREAM(...) XCAM_MODULE_LOG_ERROR(XCORE_LOG_MODULE_RKRAWSTREAM, 0xff, ##__VA_ARGS__)

class RkPollThread;

typedef enum {
	RKRAWSTREAM_POLL_VICAP = 0,
	RKRAWSTREAM_POLL_READBACK,
	RKRAWSTREAM_POLL_ISP,
	RKRAWSTREAM_POLL_TYPE_MAX,
} rkrawstream_poll_type_t;

class RKStream
{
public:
    RKStream               (SmartPtr<V4l2Device> dev, int type);
    RKStream               (SmartPtr<V4l2SubDevice> dev, int type);
    RKStream               (const char *path, int type);
    virtual ~RKStream      ();
    virtual XCamReturn start            ();
    virtual void startThreadOnly        ();
    virtual void startDeviceOnly        ();
    virtual void stop                   ();
    virtual void stopThreadOnly         ();
    virtual void stopDeviceOnly         ();
    virtual void stopDeviceStreamoff    ();
    virtual void stopDeviceFreebuffer   ();
    virtual void pause                  ();
    virtual void resume                 ();
    virtual bool setPollCallback (PollCallback *callback);
    virtual SmartPtr<VideoBuffer>
    new_video_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
    virtual SmartPtr<VideoBuffer>
    new_video_buffer(struct v4l2_event &event, SmartPtr<V4l2Device> dev) { return NULL; }
    virtual SmartPtr<V4l2BufferProxy>
    new_v4l2proxy_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev) { return NULL; }
    void set_device_prepared(bool prepare);
    XCamReturn virtual getFormat(struct v4l2_format &format);
    XCamReturn virtual getFormat(struct v4l2_subdev_format &format);
protected:
    XCAM_DEAD_COPY (RKStream);
protected:
    static const char* poll_type_to_str[RKRAWSTREAM_POLL_TYPE_MAX];
    SmartPtr<V4l2Device>  _dev;
    SmartPtr<V4l2SubDevice>  _subdev;
    int _dev_type;
    SmartPtr<RkPollThread> _poll_thread;
    bool _dev_prepared;
};

class RkPollThread : public Thread
{
public:
    explicit RkPollThread (const char* thName, int type, SmartPtr<V4l2Device> dev, RKStream *stream);
    explicit RkPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> dev, RKStream *stream);
    virtual ~RkPollThread ();
    virtual bool setPollCallback (PollCallback *callback);
    virtual XCamReturn start();
    virtual XCamReturn stop ();
protected:
    XCAM_DEAD_COPY (RkPollThread);
    XCamReturn poll_buffer_loop ();
    virtual bool loop () {
        XCamReturn ret = poll_buffer_loop ();
        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT ||
                ret == XCAM_RETURN_BYPASS)
            return true;
        return false;
    }
    XCamReturn create_stop_fds ();
    void destroy_stop_fds ();
protected:
    static const int default_poll_timeout;
    SmartPtr<V4l2Device> _dev;
    SmartPtr<V4l2SubDevice> _subdev;
    PollCallback *_poll_callback;
    RKStream *_stream;
    //frame syncronization
    int frameid;
    int _dev_type;
    int _poll_stop_fd[2];
};

class RkEventPollThread : public RkPollThread
{
public:
    explicit RkEventPollThread (const char* thName, int type, SmartPtr<V4l2Device> dev, RKStream *stream);
    explicit RkEventPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> subdev, RKStream *stream);
    virtual ~RkEventPollThread ();
protected:
    XCAM_DEAD_COPY (RkEventPollThread);
    virtual XCamReturn poll_event_loop ();
    virtual bool loop () {
        XCamReturn ret = poll_event_loop ();
        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT ||
                XCAM_RETURN_BYPASS)
            return true;
        return false;
    }
    //SmartPtr<V4l2SubDevice> _subdev;
    struct v4l2_event _event;
};

class RKRawStream : public RKStream
{
public:
    RKRawStream               (SmartPtr<V4l2Device> dev, int index, int type);
    virtual ~RKRawStream      ();
    virtual SmartPtr<V4l2BufferProxy>
    new_v4l2proxy_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
public:
    int _dev_index;
protected:
    XCAM_DEAD_COPY (RKRawStream);
};

}
#endif
