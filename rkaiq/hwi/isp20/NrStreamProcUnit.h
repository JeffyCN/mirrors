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
#ifndef _NR_STREAM_PROC_UNIT_H_
#define _NR_STREAM_PROC_UNIT_H_
#include "NrStatsStream.h"
#include "rk_aiq_pool.h"
#include "Isp20Params.h"

using namespace XCam;
namespace RkCam {

class CamHwIsp20;
class NrStreamProcUnit
{
public:
    NrStreamProcUnit               (const rk_sensor_full_info_t *s_info);
    virtual ~NrStreamProcUnit      ();
    void set_devices				(CamHwIsp20 *camHw, SmartPtr<V4l2SubDevice> isppdev);
    void start                  ();
    void stop                   ();
    void pause                  ();
    void resume                 ();
    XCamReturn config_params(uint32_t frameId, SmartPtr<cam3aResult>& result);
protected:
    XCamReturn configToDrv(uint32_t frameId);

private:
    XCAM_DEAD_COPY (NrStreamProcUnit);
    SmartPtr<V4l2Device> mNrStatsDev;
    SmartPtr<V4l2Device> mNrParamsDev;
    SmartPtr<RKStream> mNrParamStream;
    SmartPtr<NrStatsStream> mNrStatsStream;
    CamHwIsp20 *mCamHw;
    SmartPtr<V4l2SubDevice> mIsppSubDev;
    SmartPtr<IspParamsAssembler> mParamsAssembler;
    struct rkispp_params_nrcfg last_ispp_nr_params;
};


}
#endif
