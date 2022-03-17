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
#ifndef _RK_AIQ_AE_HANDLE_INT_H_
#define _RK_AIQ_AE_HANDLE_INT_H_

#include "RkAiqHandle.h"
#include "ae/rk_aiq_uapi_ae_int.h"
#include "rk_aiq_api_private.h"
#include "rk_aiq_pool.h"
#include "xcam_mutex.h"

namespace RkCam {

class RkAiqCustomAeHandle;

class RkAiqAeHandleInt : public RkAiqHandle {
    friend class RkAiqCustomAeHandle;

 public:
    explicit RkAiqAeHandleInt(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
        : RkAiqHandle(des, aiqCore), mPreResShared(nullptr), mProcResShared(nullptr) {
        updateExpSwAttrV2  = false;
        updateLinExpAttrV2 = false;
        updateHdrExpAttrV2 = false;

        updateLinAeRouteAttr = false;
        updateHdrAeRouteAttr = false;
        updateIrisAttr       = false;
        updateSyncTestAttr   = false;
        updateExpWinAttr     = false;

        memset(&mCurExpSwAttrV2, 0, sizeof(Uapi_ExpSwAttrV2_t));
        memset(&mNewExpSwAttrV2, 0, sizeof(Uapi_ExpSwAttrV2_t));
        memset(&mCurLinExpAttrV2, 0, sizeof(Uapi_LinExpAttrV2_t));
        memset(&mNewLinExpAttrV2, 0, sizeof(Uapi_LinExpAttrV2_t));
        memset(&mCurHdrExpAttrV2, 0, sizeof(Uapi_HdrExpAttrV2_t));
        memset(&mNewHdrExpAttrV2, 0, sizeof(Uapi_HdrExpAttrV2_t));

        memset(&mCurLinAeRouteAttr, 0, sizeof(Uapi_LinAeRouteAttr_t));
        memset(&mNewLinAeRouteAttr, 0, sizeof(Uapi_LinAeRouteAttr_t));
        memset(&mCurHdrAeRouteAttr, 0, sizeof(Uapi_HdrAeRouteAttr_t));
        memset(&mNewHdrAeRouteAttr, 0, sizeof(Uapi_HdrAeRouteAttr_t));
        memset(&mCurIrisAttr, 0, sizeof(Uapi_IrisAttrV2_t));
        memset(&mNewIrisAttr, 0, sizeof(Uapi_IrisAttrV2_t));
        memset(&mCurAecSyncTestAttr, 0, sizeof(Uapi_AecSyncTest_t));
        memset(&mNewAecSyncTestAttr, 0, sizeof(Uapi_AecSyncTest_t));
        memset(&mCurExpWinAttr, 0, sizeof(Uapi_ExpWin_t));
        memset(&mNewExpWinAttr, 0, sizeof(Uapi_ExpWin_t));
    };
    virtual ~RkAiqAeHandleInt() { RkAiqHandle::deInit(); };
    virtual XCamReturn updateConfig(bool needSync);
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();

    // TODO: calibv1
    virtual XCamReturn setExpSwAttr(Uapi_ExpSwAttr_t ExpSwAttr);
    virtual XCamReturn getExpSwAttr(Uapi_ExpSwAttr_t* pExpSwAttr);
    virtual XCamReturn setLinExpAttr(Uapi_LinExpAttr_t LinExpAttr);
    virtual XCamReturn getLinExpAttr(Uapi_LinExpAttr_t* pLinExpAttr);
    virtual XCamReturn setHdrExpAttr(Uapi_HdrExpAttr_t HdrExpAttr);
    virtual XCamReturn getHdrExpAttr(Uapi_HdrExpAttr_t* pHdrExpAttr);

    // TODO: calibv2
    virtual XCamReturn setExpSwAttr(Uapi_ExpSwAttrV2_t ExpSwAttr);
    virtual XCamReturn getExpSwAttr(Uapi_ExpSwAttrV2_t* pExpSwAttr);
    virtual XCamReturn setLinExpAttr(Uapi_LinExpAttrV2_t LinExpAttr);
    virtual XCamReturn getLinExpAttr(Uapi_LinExpAttrV2_t* pLinExpAttr);
    virtual XCamReturn setHdrExpAttr(Uapi_HdrExpAttrV2_t HdrExpAttr);
    virtual XCamReturn getHdrExpAttr(Uapi_HdrExpAttrV2_t* pHdrExpAttr);

    virtual XCamReturn setLinAeRouteAttr(Uapi_LinAeRouteAttr_t LinAeRouteAttr);
    virtual XCamReturn getLinAeRouteAttr(Uapi_LinAeRouteAttr_t* pLinAeRouteAttr);
    virtual XCamReturn setHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t HdrAeRouteAttr);
    virtual XCamReturn getHdrAeRouteAttr(Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr);

    virtual XCamReturn setIrisAttr(Uapi_IrisAttrV2_t IrisAttr);
    virtual XCamReturn getIrisAttr(Uapi_IrisAttrV2_t* pIrisAttr);
    virtual XCamReturn setSyncTestAttr(Uapi_AecSyncTest_t SyncTestAttr);
    virtual XCamReturn getSyncTestAttr(Uapi_AecSyncTest_t* pSyncTestAttr);
    virtual XCamReturn queryExpInfo(Uapi_ExpQueryInfo_t* pExpQueryInfo);
    virtual XCamReturn setLockAeForAf(bool lock_ae);
    virtual XCamReturn setExpWinAttr(Uapi_ExpWin_t ExpWinAttr);
    virtual XCamReturn getExpWinAttr(Uapi_ExpWin_t* pExpWinAttr);
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params);

 protected:
    virtual void init();
    virtual void deInit() { RkAiqHandle::deInit(); };
    SmartPtr<RkAiqAlgoPreResAeIntShared> mPreResShared;
    SmartPtr<RkAiqAlgoProcResAeIntShared> mProcResShared;

 private:
    // TODO: calibv1
    Uapi_ExpSwAttr_t mCurExpSwAttr;
    Uapi_ExpSwAttr_t mNewExpSwAttr;
    Uapi_LinExpAttr_t mCurLinExpAttr;
    Uapi_LinExpAttr_t mNewLinExpAttr;
    Uapi_HdrExpAttr_t mCurHdrExpAttr;
    Uapi_HdrExpAttr_t mNewHdrExpAttr;

    // TODO: calibv2
    Uapi_ExpSwAttrV2_t mCurExpSwAttrV2;
    Uapi_ExpSwAttrV2_t mNewExpSwAttrV2;
    Uapi_LinExpAttrV2_t mCurLinExpAttrV2;
    Uapi_LinExpAttrV2_t mNewLinExpAttrV2;
    Uapi_HdrExpAttrV2_t mCurHdrExpAttrV2;
    Uapi_HdrExpAttrV2_t mNewHdrExpAttrV2;

    Uapi_LinAeRouteAttr_t mCurLinAeRouteAttr;
    Uapi_LinAeRouteAttr_t mNewLinAeRouteAttr;
    Uapi_HdrAeRouteAttr_t mCurHdrAeRouteAttr;
    Uapi_HdrAeRouteAttr_t mNewHdrAeRouteAttr;
    Uapi_IrisAttrV2_t mCurIrisAttr;
    Uapi_IrisAttrV2_t mNewIrisAttr;
    Uapi_AecSyncTest_t mCurAecSyncTestAttr;
    Uapi_AecSyncTest_t mNewAecSyncTestAttr;
    Uapi_ExpWin_t mCurExpWinAttr;
    Uapi_ExpWin_t mNewExpWinAttr;

    bool updateExpSwAttr  = false;
    bool updateLinExpAttr = false;
    bool updateHdrExpAttr = false;

    mutable std::atomic<bool> updateExpSwAttrV2;
    mutable std::atomic<bool> updateLinExpAttrV2;
    mutable std::atomic<bool> updateHdrExpAttrV2;

    mutable std::atomic<bool> updateLinAeRouteAttr;
    mutable std::atomic<bool> updateHdrAeRouteAttr;
    mutable std::atomic<bool> updateIrisAttr;
    mutable std::atomic<bool> updateSyncTestAttr;
    mutable std::atomic<bool> updateExpWinAttr;

    uint16_t updateAttr = 0;

    XCam::Mutex mLockAebyAfMutex;
    bool lockaebyaf = false;

 private:
    DECLARE_HANDLE_REGISTER_TYPE(RkAiqAeHandleInt);
};

};  // namespace RkCam

#endif
