/*
 *  Copyright (c) 2021 Rockchip Corporation
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
#ifndef _PDAF_STREAM_PROC_UNIT_H_
#define _PDAF_STREAM_PROC_UNIT_H_
#include <v4l2_device.h>
#include "poll_thread.h"
#include "xcam_mutex.h"
#include "Stream.h"
#include "af_head.h"

using namespace XCam;

namespace RkCam {

class PdafBufferProxy : public V4l2BufferProxy
{
    public:
    explicit PdafBufferProxy(SmartPtr<V4l2Buffer> &buf, SmartPtr<V4l2Device> &device)
             : V4l2BufferProxy(buf, device)
    {
        memset(&pdaf_meas, 0, sizeof(pdaf_meas));
    }
    virtual ~PdafBufferProxy() {}
    rk_aiq_isp_pdaf_meas_t pdaf_meas;
protected:
    XCAM_DEAD_COPY (PdafBufferProxy);
};

class CamHwIsp20;
class RKStream;
class PdafStreamHelperThd;
class PdafStreamProcUnit       : public PollCallback
{
public:
    explicit PdafStreamProcUnit (int type);
    virtual ~PdafStreamProcUnit ();
    virtual void start ();
    virtual void stop ();

    // from PollCallback
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<VideoBuffer> &buf) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index);
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) { return XCAM_RETURN_ERROR_FAILED; }
    virtual XCamReturn poll_event_failed (int64_t timestamp, const char *msg) { return XCAM_RETURN_ERROR_FAILED; }

    void set_devices(CamHwIsp20* camHw);
    int64_t get_systime_us();
    XCamReturn prepare(rk_sensor_pdaf_info_t *pdaf_inf);
    XCamReturn start_stream(bool lock);
    XCamReturn stop_stream(bool lock);
    XCamReturn deinit();
protected:
    XCAM_DEAD_COPY (PdafStreamProcUnit);
protected:
    CamHwIsp20* mCamHw;
    SmartPtr<V4l2Device> mPdafDev;
    SmartPtr<RKPdafStream> mPdafStream;
    bool mStartFlag;
    bool mStartStreamFlag;
    rk_aiq_isp_pdaf_meas_t mPdafMeas;
    int mBufType;
    SmartPtr<PdafStreamHelperThd> mHelperThd;
    XCam::Mutex mStreamMutex;
    rk_sensor_pdaf_info_t mPdafInf;
};

typedef struct _PdafStreamParam {
    bool valid;
    bool stream_flag;
} PdafStreamParam;

class PdafStreamHelperThd
    : public XCam::Thread {
public:
    PdafStreamHelperThd(PdafStreamProcUnit *pdafstreamproc)
        : Thread("PdafStreamHelperThd"), mPdafStreamProc(pdafstreamproc) {};
    ~PdafStreamHelperThd() {
        mAttrQueue.clear ();
    };

    void triger_stop() {
        mAttrQueue.pause_pop ();
    };

    void triger_start() {
        mAttrQueue.resume_pop ();
    };

    bool push_attr (const XCam::SmartPtr<PdafStreamParam> buffer) {
        mAttrQueue.push (buffer);
        return true;
    };

    bool is_empty () {
        return mAttrQueue.is_empty();
    };

    void clear_attr () {
        mAttrQueue.clear ();
    };

protected:
    //virtual bool started ();
    virtual void stopped () {
        mAttrQueue.clear ();
    };
    virtual bool loop ();
private:
    PdafStreamProcUnit *mPdafStreamProc;
    XCam::SafeList<PdafStreamParam> mAttrQueue;
};

}
#endif


