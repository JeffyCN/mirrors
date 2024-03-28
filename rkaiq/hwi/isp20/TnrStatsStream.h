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
#ifndef _TNR_STATS_STREAM_H_
#define _TNR_STATS_STREAM_H_
#include <map>
#include "Stream.h"
#include "xcam_thread.h"
#include "xcam_mutex.h"
#include "NrStatsStream.h"

using namespace XCam;

namespace RkCam {

class TnrProcThread;
class TnrStatsStream : public RKStream, public PollCallback
{
public:
    explicit TnrStatsStream (SmartPtr<V4l2Device> dev, int type);
    virtual ~TnrStatsStream      ();
    void start                  ();
    void stop                   ();
    void set_device             (CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> dev);
    //bool thread_proc            ();
    virtual SmartPtr<VideoBuffer> new_video_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
    //void cache_sp_buffer        (SmartPtr<SPStatsBufferProxy> buf);
    //void return_sp_buffer       ();
    //void notify_motion_param(rk_aiq_amd_params_t amd_param);
    //void connect_nr_stream (SmartPtr<NrStatsStream> stream);
    // from PollCallback
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf);
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
protected:
    bool init_tnrbuf            ();
    void deinit_tnrbuf          ();
    int get_fd_by_index         (uint32_t index);
private:
    CamHwIsp20* _camHw;
    SmartPtr<V4l2SubDevice>  _ispp_dev;
    uint32_t _idx_array[64];
    int _fd_array[64];
    int _buf_num;
    bool _fd_init_flag;
    std::map<uint32_t, int> _idx_fd_map;
    //SmartPtr<TnrProcThread> _proc_thread;
    //SmartPtr<NrStatsStream> _nr_stream_unit;
    Mutex _list_mutex;
};

class TnrProcThread
    : public Thread
{
public:
    TnrProcThread (TnrStatsStream *handle)
        : Thread ("TnrProcThread")
        , _handle (handle)
    {
        (void)(_handle);
    }

protected:
    virtual bool loop () {
        return  false;//_handle->thread_proc ();
    }

private:
    TnrStatsStream *_handle;
};
}
#endif
