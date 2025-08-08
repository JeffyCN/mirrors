/*
 * v4l2_device.h - v4l2 device
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

#ifndef XCAM_FAKE_V4L2_DEVICE_H
#define XCAM_FAKE_V4L2_DEVICE_H

#include <v4l2_device.h>
#include "rk_aiq_types_priv.h"

namespace XCam {
class FakeV4l2Device : public V4l2Device
{
public:
    FakeV4l2Device () : V4l2Device("/dev/zero")
    {
        _pipe_fd[0] = -1;
        _pipe_fd[1] = -1;
        _use_type = 0;
    }
    virtual ~FakeV4l2Device () {
    }
    virtual XCamReturn open (bool nonblock = false) override;
    virtual XCamReturn close () override;
    int create_notify_pipe ();
    void destroy_notify_pipe ();
    virtual XCamReturn start(bool prepared = false) override;
    virtual XCamReturn stop() override;
    virtual XCamReturn get_format (struct v4l2_format &format) override;
    virtual int poll_event (int timeout_msec, int stop_fd) override;
    virtual XCamReturn dequeue_buffer (SmartPtr<V4l2Buffer> &buf) override;
    // use as less as possible
    virtual int io_control (unsigned long cmd, void *arg) override;
    void on_timer_proc();
    void enqueue_rawbuffer(struct rk_aiq_vbuf_info *vbinfo);
    virtual int get_use_type() override {return _use_type;}
    virtual void set_use_type(int type) override {_use_type = type;}
protected:
    Mutex _mutex;
    int _pipe_fd[2];
    std::list<struct rk_aiq_vbuf_info> _buf_list;
    XCAM_DEAD_COPY (FakeV4l2Device);
    uint32_t get_available_buffer_index ();
    int _use_type;
};

}
#endif // XCAM_FAKE_V4L2_DEVICE_H

