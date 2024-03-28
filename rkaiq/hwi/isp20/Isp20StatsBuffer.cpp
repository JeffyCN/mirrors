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

#include "Isp20StatsBuffer.h"

namespace RkCam {

Isp20StatsBuffer::Isp20StatsBuffer(SmartPtr<V4l2Buffer> buf,
                                   SmartPtr<V4l2Device> &device,
                                   SmartPtr<BaseSensorHw> sensor,
                                   ICamHw *camHw,
                                   SmartPtr<RkAiqAfInfoProxy> AfParams,
                                   SmartPtr<RkAiqIrisParamsProxy> IrisParams)
    : V4l2BufferProxy(buf, device)
{
    ENTER_CAMHW_FUNCTION();
    mSensor = sensor;
    mCamHw = camHw;
    _afParams = AfParams;
    _irisParams = IrisParams;
    EXIT_CAMHW_FUNCTION();
}

SofEventBuffer::SofEventBuffer(SmartPtr<SofEventData> &buf,
                                   SmartPtr<V4l2Device> &device)
    : BufferProxy (buf)
{
    ENTER_CAMHW_FUNCTION();
    //to do
    //_sofParams = new
    //_sofInfo = new RKAiqSofInfo_t();
    //_sofInfo->timestamp = _timestamp = timestamp;
    //_sofInfo->frameid = _frameid = frameid;
    EXIT_CAMHW_FUNCTION();
}

SmartPtr<SofEventData>
SofEventBuffer::get_data()
{
    SmartPtr<BufferData> buf = get_buffer_data ();
    return buf.dynamic_cast_ptr<SofEventData>();
}

AiispEventBuffer::AiispEventBuffer(SmartPtr<AiispEventData> &buf,
                                   SmartPtr<V4l2Device> &device)
    : BufferProxy (buf)
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

SmartPtr<AiispEventData>
AiispEventBuffer::get_data()
{
    SmartPtr<BufferData> buf = get_buffer_data ();
    return buf.dynamic_cast_ptr<AiispEventData>();
}

} //namspace RkCam
