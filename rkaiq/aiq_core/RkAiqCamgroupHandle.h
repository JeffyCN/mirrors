/*
 * RkAiqCamgroupHandle.h
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

#ifndef _RK_AIQ_CAMGROUP_HANDLE_H_
#define _RK_AIQ_CAMGROUP_HANDLE_H_

#include "algos/rk_aiq_algo_des.h"
#include "rk_aiq_algo_camgroup_types.h"
#include "xcore/xcam_mutex.h"
#include "RkAiqHandle.h"

namespace RkCam {

class RkAiqCamGroupManager;
class RkAiqCore;

class RkAiqCamgroupHandle {
 public:
    explicit RkAiqCamgroupHandle(RkAiqAlgoDesComm* des,
                                 RkAiqCamGroupManager* camGroupMg);
    virtual ~RkAiqCamgroupHandle ();
    virtual XCamReturn prepare(RkAiqCore* aiqCore);
    virtual XCamReturn processing(rk_aiq_singlecam_3a_result_t** resArray);
    void setEnable(bool enable) { mEnable = enable; };
    bool getEnable() { return mEnable; };
    RkAiqAlgoContext* getAlgoCtx() { return mAlgoCtx; }
    int getAlgoId() { return mDes->id; }
    int getAlgoType() { return mDes->type; }
    void setNextHdl(RkAiqCamgroupHandle* next) { mNextHdl = next; }
    void setParentHdl(RkAiqCamgroupHandle* parent) { mParentHdl = parent; }
    RkAiqCamgroupHandle* getNextHdl() { return mNextHdl; }
    RkAiqCamgroupHandle* getParent() { return mParentHdl; }
    virtual XCamReturn updateConfig(bool /* needSync */) { return XCAM_RETURN_NO_ERROR; };
    void setAlgoHandle(RkAiqHandle* handle) { mSingleHdl = handle; };
 protected:
    virtual void init();
    virtual void deInit();
    void waitSignal(rk_aiq_uapi_mode_sync_e sync = RK_AIQ_UAPI_MODE_DEFAULT);
    void sendSignal(rk_aiq_uapi_mode_sync_e sync = RK_AIQ_UAPI_MODE_DEFAULT);
    enum {
        RKAIQ_CONFIG_COM_PREPARE,
        RKAIQ_CONFIG_COM_PROC,
    };
    RkAiqAlgoCom* mConfig;
    RkAiqAlgoCom* mProcInParam;
    RkAiqAlgoResCom* mProcOutParam;
    RkAiqAlgoDesComm* mDes;
    RkAiqAlgoContext* mAlgoCtx;
    RkAiqCamGroupManager* mGroupMg;
    RkAiqCore* mAiqCore;
    RkAiqHandle* mSingleHdl;
    bool mEnable;
    XCam::Mutex mCfgMutex;
    bool updateAtt;
    XCam::Cond mUpdateCond;
    RkAiqCamgroupHandle* mNextHdl;
    RkAiqCamgroupHandle* mParentHdl;
};

}  // namespace RkCam

#endif
