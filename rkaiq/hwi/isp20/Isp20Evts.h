/*
 * Isp20StatsBuffer.h - isp20 stats buffer
 *
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

#ifndef ISP20_EVTS_H
#define ISP20_EVTS_H

#include "rk_aiq_pool.h"
#include "ICamHw.h"
#include "SensorHw.h"

using namespace XCam;

namespace RkCam {

class Isp20Evt
    : public ispHwEvt_t, public BufferData {
public:
    Isp20Evt(ICamHw *camHw, SmartPtr<SensorHw> sensor)
        : mSensor(sensor), mCamHw(camHw), mTimestamp(-1) {
        (void)(mCamHw);
    }
    virtual ~Isp20Evt() {}

    XCamReturn getExpInfoParams(SmartPtr<RkAiqSensorExpParamsProxy>& expInfo, uint32_t frameId);

    void setSofTimeStamp(int64_t timestamp) {
        mTimestamp = timestamp;
    }

    int64_t getSofTimeStamp() {
        return mTimestamp;
    }

    virtual uint8_t *map () {return NULL;}
    virtual bool unmap () {return false;}

    uint32_t sequence = (uint32_t)(-1);
    uint32_t expDelay = 2;

private:
    // XCAM_DEAD_COPY(Isp20Evt);
    Mutex _mutex;
    SmartPtr<SensorHw> mSensor;
    ICamHw *mCamHw;
    int64_t mTimestamp;
};

class Isp20EvtBuffer
    : public BufferProxy {
public:
    explicit Isp20EvtBuffer(SmartPtr<Isp20Evt> &buf,
                            SmartPtr<V4l2Device> &device)
        : BufferProxy (buf) {}
    virtual ~Isp20EvtBuffer() {};
    SmartPtr<Isp20Evt> get_data() {
        SmartPtr<BufferData> buf = get_buffer_data ();
        return buf.dynamic_cast_ptr<Isp20Evt>();
    }
private:
    XCAM_DEAD_COPY(Isp20EvtBuffer);
    //SmartPtr<RkAiqSofInfoProxy> _sofParams;
    //SmartPtr<RKAiqSofInfo_t> _sofInfo;
    //SmartPtr<SofEventData> _event_data;
};

}

#endif
