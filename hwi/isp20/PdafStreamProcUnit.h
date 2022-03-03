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
    XCamReturn prepare(CalibDbV2_Af_Pdaf_t *pdaf_calib, rk_sensor_pdaf_info_t *pdaf_inf);
protected:
    XCAM_DEAD_COPY (PdafStreamProcUnit);
protected:
    CamHwIsp20* mCamHw;
    SmartPtr<V4l2Device> mPdafDev;
    SmartPtr<RKPdafStream> mPdafStream;
    bool mStartFlag;
    rk_aiq_isp_pdaf_meas_t mPdafMeas;
    int mBufType;

};

}
#endif


