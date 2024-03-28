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

#ifndef ISP20_STATS_BUFFER_H
#define ISP20_STATS_BUFFER_H

#include "rk_aiq_pool.h"
#include "v4l2_buffer_proxy.h"
#include "ICamHw.h"
#include "SensorHw.h"

using namespace XCam;

namespace RkCam {

class Isp20StatsBuffer
    : public V4l2BufferProxy {
public:
    explicit Isp20StatsBuffer(SmartPtr<V4l2Buffer> buf,
                              SmartPtr<V4l2Device> &device,
                              SmartPtr<BaseSensorHw> sensor,
                              ICamHw *camHw,
                              SmartPtr<RkAiqAfInfoProxy> AfParams,
                              SmartPtr<RkAiqIrisParamsProxy> IrisParams);
    virtual ~Isp20StatsBuffer() {};

    XCamReturn getEffectiveExpParams (uint32_t frameId, SmartPtr<RkAiqSensorExpParamsProxy>& expParams) {
        XCAM_ASSERT (mSensor.ptr ());
        return mSensor->getEffectiveExpParams(expParams, frameId);
    }

    XCamReturn getEffectiveIspParams(uint32_t frameId, rkisp_effect_params_v20& ispParams) {
        XCAM_ASSERT (mCamHw);
        return mCamHw->getEffectiveIspParams(ispParams, frameId);
    }

    SmartPtr<RkAiqAfInfoProxy>& get_af_params () {
        return _afParams;
    }

    SmartPtr<RkAiqIrisParamsProxy>& get_iris_params () {
        return _irisParams;
    }
private:
    XCAM_DEAD_COPY(Isp20StatsBuffer);
    SmartPtr<BaseSensorHw> mSensor;
    ICamHw *mCamHw;
    SmartPtr<RkAiqAfInfoProxy> _afParams;
    SmartPtr<RkAiqIrisParamsProxy> _irisParams;
};

class SofEventData : public BufferData
{
public:
    explicit SofEventData () {}
    ~SofEventData() {}
    virtual uint8_t *map () {return NULL;}
    virtual bool unmap () {return false;}
    int64_t _timestamp{0};
    uint32_t _frameid{0};
};

class SofEventBuffer
    : public BufferProxy {
public:
    explicit SofEventBuffer(SmartPtr<SofEventData> &buf,
                              SmartPtr<V4l2Device> &device);
    virtual ~SofEventBuffer() {};
    SmartPtr<SofEventData> get_data();
private:
    XCAM_DEAD_COPY(SofEventBuffer);
    //SmartPtr<RkAiqSofInfoProxy> _sofParams;
    //SmartPtr<RKAiqSofInfo_t> _sofInfo;
    //SmartPtr<SofEventData> _event_data;
};

class AiispEventData : public BufferData
{
public:
    explicit AiispEventData () {}
    ~AiispEventData() {}
    virtual uint8_t *map () {return NULL;}
    virtual bool unmap () {return false;}
    int32_t _height{0};
    uint32_t _frameid{0};
    rkisp_bay3dbuf_info_t bay3dbuf;
    void* iir_address;
    void* gain_address;
};

class AiispEventBuffer
    : public BufferProxy {
public:
    explicit AiispEventBuffer(SmartPtr<AiispEventData> &buf,
                              SmartPtr<V4l2Device> &device);
    virtual ~AiispEventBuffer() {};
    SmartPtr<AiispEventData> get_data();
private:
    XCAM_DEAD_COPY(AiispEventBuffer);
};

}

#endif
