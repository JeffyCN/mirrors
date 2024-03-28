/*
 * poll_thread.h - poll thread for event and buffer
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

#ifndef XCAM_POLL_THREAD_H
#define XCAM_POLL_THREAD_H

#include <xcam_std.h>
#include <xcam_mutex.h>
#include <v4l2_buffer_proxy.h>
#include <v4l2_device.h>
#include "xcam_thread.h"
#include "rk_aiq_types_priv.h"

namespace XCam {

class PollCallback
{
public:
    PollCallback () {}
    virtual ~PollCallback() {}
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type) = 0;
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg) = 0;
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) = 0;
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) = 0;
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf) = 0;
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) = 0;

private:
    XCAM_DEAD_COPY (PollCallback);

};

class V4l2Device;
class V4l2SubDevice;
class IspPollThread;
class EventPollThread;
class Thread;

class PollThread
{
    friend class IspPollThread;
    friend class EventPollThread;
    friend class FakePollThread;
public:
    explicit PollThread ();
    virtual ~PollThread ();

    bool set_isp_params_devices (SmartPtr<V4l2Device> &params_dev,
                                 SmartPtr<V4l2Device> &post_params_dev);
    bool set_event_device (SmartPtr<V4l2SubDevice> &sub_dev);
    bool set_isp_luma_device (SmartPtr<V4l2Device> &dev);
    bool set_ispp_stats_device (SmartPtr<V4l2Device> &dev);
    bool set_isp_stats_device (SmartPtr<V4l2Device> &dev);
    bool set_poll_callback (PollCallback *callback);

    virtual XCamReturn start();
    virtual XCamReturn stop ();

    static const char* isp_poll_type_to_str[ISP_POLL_POST_MAX];
protected:
    XCamReturn poll_subdev_event_loop ();
    virtual XCamReturn poll_buffer_loop (int type);

    virtual XCamReturn handle_events (struct v4l2_event &event);
    XCamReturn handle_frame_sync_event (struct v4l2_event &event);
    virtual SmartPtr<VideoBuffer> new_video_buffer(SmartPtr<V4l2Buffer> buf,
            SmartPtr<V4l2Device> dev,
            int type);
    virtual XCamReturn notify_sof (uint64_t time, uint32_t frameid);

private:
    XCamReturn create_stop_fds ();
    void destroy_stop_fds ();

private:
    XCAM_DEAD_COPY (PollThread);

protected:
    static const int default_poll_timeout;

    SmartPtr<Thread>      _ispp_stats_loop;
    SmartPtr<Thread>      _isp_luma_loop;
    SmartPtr<Thread>      _isp_stats_loop;
    SmartPtr<Thread>      _event_loop;
    SmartPtr<Thread>      _isp_params_loop;
    SmartPtr<Thread>      _isp_pparams_loop;

    SmartPtr<V4l2SubDevice>          _event_dev;
    SmartPtr<V4l2Device>             _isp_params_dev;
    SmartPtr<V4l2Device>             _isp_pparams_dev;
    SmartPtr<V4l2Device>             _isp_stats_dev;
    SmartPtr<V4l2Device>             _isp_luma_dev;
    SmartPtr<V4l2Device>             _ispp_stats_dev;

    PollCallback                    *_poll_callback;

    //frame syncronization
    uint32_t frameid;

    int _ispp_poll_stop_fd[2];
    int _luma_poll_stop_fd[2];
    int _3a_stats_poll_stop_fd[2];
    int _event_poll_stop_fd[2];
    int _isp_params_poll_stop_fd[2];
    int _isp_pparams_poll_stop_fd[2];
};

}

#endif //XCAM_POLL_THREAD_H
