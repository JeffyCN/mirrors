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
#ifndef _FEC_PARAM_STREAM_H_
#define _FEC_PARAM_STREAM_H_
#include <map>
#include "Stream.h"
#include "rk_aiq_pool.h"
#include "Isp20Params.h"

using namespace XCam;

namespace RkCam {

class CamHwIsp20;
class FecParamStream  : public RKStream
{
public:
    explicit FecParamStream      (SmartPtr<V4l2Device> dev, int type);
    explicit FecParamStream      (const rk_sensor_full_info_t *s_info);
    virtual ~FecParamStream      ();
    XCamReturn config_params (uint32_t frameId, SmartPtr<cam3aResult>& result);
    void set_devices             (CamHwIsp20* camHw, SmartPtr<V4l2SubDevice> isppdev);
    virtual void start             ();
    virtual void stop              ();
protected:
    XCamReturn configToDrv(uint32_t frameId);

private:
    CamHwIsp20 *mCamHw;
    SmartPtr<V4l2SubDevice> mIsppSubDev;
    SmartPtr<IspParamsAssembler> mParamsAssembler;
    struct rkispp_params_feccfg last_ispp_fec_params;

};

}
#endif
