/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef _RK_AIQ_CUSTOM_AE_HANDLE_H_
#define _RK_AIQ_CUSTOM_AE_HANDLE_H_

#include "algo_handlers/RkAiqAeHandle.h"

namespace RkCam {

// ae
class RkAiqCustomAeHandle:
    public RkAiqAeHandleInt {
public:
    explicit RkAiqCustomAeHandle(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqAeHandleInt (des, aiqCore) {};
    virtual ~RkAiqCustomAeHandle() {
        deInit();
    };
    virtual void init();
    virtual void deInit();
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn lock();
    virtual XCamReturn unlock();

    // TODO: calibv1
    virtual XCamReturn setExpSwAttr(Uapi_ExpSwAttr_t ExpSwAttr);
    virtual XCamReturn getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr);
    virtual XCamReturn setLinExpAttr(Uapi_LinExpAttr_t LinExpAttr);
    virtual XCamReturn getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr);
    virtual XCamReturn setHdrExpAttr(Uapi_HdrExpAttr_t HdrExpAttr);
    virtual XCamReturn getHdrExpAttr (Uapi_HdrExpAttr_t* pHdrExpAttr);

    // TODO: calibv2
    virtual XCamReturn setExpSwAttr(Uapi_ExpSwAttrV2_t ExpSwAttr);
    virtual XCamReturn getExpSwAttr(Uapi_ExpSwAttrV2_t* pExpSwAttr);
    virtual XCamReturn setLinExpAttr(Uapi_LinExpAttrV2_t LinExpAttr);
    virtual XCamReturn getLinExpAttr(Uapi_LinExpAttrV2_t* pLinExpAttr);
    virtual XCamReturn setHdrExpAttr(Uapi_HdrExpAttrV2_t HdrExpAttr);
    virtual XCamReturn getHdrExpAttr (Uapi_HdrExpAttrV2_t* pHdrExpAttr);

    virtual XCamReturn setLinAeRouteAttr(Uapi_LinAeRouteAttr_t LinAeRouteAttr);
    virtual XCamReturn getLinAeRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr);
    virtual XCamReturn setHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t HdrAeRouteAttr);
    virtual XCamReturn getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr);

    virtual XCamReturn setIrisAttr(Uapi_IrisAttrV2_t IrisAttr);
    virtual XCamReturn getIrisAttr (Uapi_IrisAttrV2_t* pIrisAttr);
    virtual XCamReturn setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr);
    virtual XCamReturn getSyncTestAttr (Uapi_AecSyncTest_t* pSyncTestAttr);
    virtual XCamReturn queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo);
    virtual XCamReturn setLockAeForAf(bool lock_ae);
    virtual XCamReturn setExpWinAttr(Uapi_ExpWin_t ExpWinAttr);
    virtual XCamReturn getExpWinAttr(Uapi_ExpWin_t* pExpWinAttr);
    virtual XCamReturn setAecStatsCfg(Uapi_AecStatsCfg_t AecStatsCfg);
    virtual XCamReturn getAecStatsCfg(Uapi_AecStatsCfg_t* pAecStatsCfg);
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);
};


} //namespace RkCam

#endif
