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
#ifndef _NR_STATS_STREAM_H_
#define _NR_STATS_STREAM_H_
#include <map>
#include "xcam_thread.h"
#include "xcam_mutex.h"
#include "Stream.h"
#include "smart_buffer_priv.h"
using namespace XCam;

namespace RkCam {

class NrParamProcThread;
class NrStatsStream  : public RKStream, public PollCallback
{
public:
    explicit NrStatsStream      (SmartPtr<V4l2Device> dev, int type);
    virtual ~NrStatsStream      ();
    void start                  ();
    void stop                   ();
    void set_device             (CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> dev);
    virtual SmartPtr<VideoBuffer> new_video_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
    int queue_NRImg_fd(int fd, uint32_t frameid);
    int get_NRImg_fd(uint32_t frameid);
    // from PollCallback
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf);
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
protected:
    bool init_nrbuf_fd          ();
    bool deinit_nrbuf_fd        ();
    int get_NRimg_fd_by_index(int index);
private:
    SmartPtr<V4l2SubDevice> _ispp_dev;
    struct v4l2_subdev_format _ispp_fmt;
    int _fd_array[16];
    int _idx_array[16];
    int _buf_num;
    bool _fd_init_flag;
    //SmartPtr<NrParamProcThread> _proc_thread;
    Mutex _list_mutex;
    CamHwIsp20* _camHw;
    SmartPtr<VideoBuffer> _NrImage;
    std::map<uint32_t, int> _NrImg_ready_map;
};

class NrParamProcThread
    : public Thread
{
public:
    NrParamProcThread (NrStatsStream *handle)
        : Thread ("NrParamProcThread")
        , _handle (handle)
    {
        (void)(_handle);
    }

protected:
    virtual bool loop () {
        return  false;//_handle->thread_proc ();
    }

private:
    NrStatsStream *_handle;
};
}
#endif
