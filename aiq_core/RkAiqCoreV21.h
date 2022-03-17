/*
 * RkAiqCoreV21.h
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

#ifndef _RK_AIQ_CORE_V21_H_
#define _RK_AIQ_CORE_V21_H_

#include "RkAiqCore.h"

using namespace XCam;
namespace RkCam {

class RkAiqCoreV21
    : public RkAiqCore {
public:
    RkAiqCoreV21();
    virtual ~RkAiqCoreV21();
protected:
    SmartPtr<RkAiqHandle> newAlgoHandle(RkAiqAlgoDesComm* algo, bool generic, int hw_ver);
    void copyIspStats(SmartPtr<RkAiqAecStatsProxy>& aecStat,
                      SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                      SmartPtr<RkAiqAfStatsProxy>& afStat,
                      rk_aiq_isp_stats_t* to);
    XCamReturn genIspParamsResult(RkAiqFullParams *aiqParams, enum rk_aiq_core_analyze_type_e type);
    XCamReturn genIspArawnrResult(RkAiqFullParams* params);
    XCamReturn genIspAmfnrResult(RkAiqFullParams* params);
    XCamReturn genIspAynrResult(RkAiqFullParams* params);
    XCamReturn genIspAcnrResult(RkAiqFullParams* params);
    XCamReturn genIspAdrcResult(RkAiqFullParams* params);
    XCamReturn genIspAwbResult(RkAiqFullParams* params);
    XCamReturn genIspAblcResult(RkAiqFullParams* params);
    XCamReturn genIspAgicResult(RkAiqFullParams* params);
    XCamReturn genIspAccmResult(RkAiqFullParams* params);
    XCamReturn genIspAdhazResult(RkAiqFullParams* params);
    XCamReturn genIspAsharpResult(RkAiqFullParams* params);
    void newAiqParamsPool();
    XCamReturn getAiqParamsBuffer(RkAiqFullParams* aiqParams, enum rk_aiq_core_analyze_type_e type);

private:
};

};

#endif //_RK_AIQ_CORE_V21_H_
