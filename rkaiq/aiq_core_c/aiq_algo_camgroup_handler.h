/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
#ifndef _AIQ_ALGO_CAMGROUP_HANDLE_H_
#define _AIQ_ALGO_CAMGROUP_HANDLE_H_

#include "xcore/base/xcam_common.h"
#include "c_base/aiq_mutex.h"
#include "algos/rk_aiq_algo_des.h"
#include "common/rk_aiq_types_priv_c.h"
#include "rk_aiq_algo_camgroup_types.h"

//forward declare
typedef struct AiqCore_s AiqCore_t;
typedef struct AiqCamGroupManager_s AiqCamGroupManager_t;
typedef struct AiqAlgoHandler_s AiqAlgoHandler_t;

RKAIQ_BEGIN_DECLARE

typedef struct AiqAlgoCamGroupHandler_s AiqAlgoCamGroupHandler_t;

struct AiqAlgoCamGroupHandler_s {
    XCamReturn (*prepare)(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler, AiqCore_t* aiqCore);
    XCamReturn (*processing)(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler, rk_aiq_singlecam_3a_result_t** resArray);
    // internal used
    void (*init)(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler);
    void (*deinit)(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler);

    RkAiqAlgoCom* mConfig;
    RkAiqAlgoCom* mProcInParam;
    RkAiqAlgoResCom* mProcOutParam;
    const RkAiqAlgoDesComm* mDes;
    RkAiqAlgoContext* mAlgoCtx;
    AiqCamGroupManager_t* mGroupMg;
    AiqCore_t* mAiqCore;
    bool mEnable;
    bool mReConfig;
    AiqMutex_t mCfgMutex;
    AiqAlgoCamGroupHandler_t* mNextHdl;
    AiqAlgoCamGroupHandler_t* mParentHdl;
    AiqAlgoHandler_t* mSingleHdl;
};

// generic constructor/destructor
AiqAlgoCamGroupHandler_t* AiqAlgoCamGroupHandler_constructor(RkAiqAlgoDesComm* des,
                                      AiqCamGroupManager_t* camGroupMg, AiqCore_t* aiqcore);
void AiqAlgoCamGroupHandler_destructor(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler);
void AiqAlgoCamGroupHandler_deinit(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler);

XCamReturn AiqAlgoCamGroupHandler_prepare(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler, AiqCore_t* aiqCore);
XCamReturn AiqAlgoCamGroupHandler_processing(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler, rk_aiq_singlecam_3a_result_t** resArray);

#define AiqAlgoCamGroupHandler_setEnable(pAlgoCamGroupHandler, enable) \
    (pAlgoCamGroupHandler)->mEnable = enable

#define AiqAlgoCamGroupHandler_getEnable(pAlgoCamGroupHandler) \
    (pAlgoCamGroupHandler)->mEnable 

#define AiqAlgoCamGroupHandler_getAlgoCtx(pAlgoCamGroupHandler) \
    (pAlgoCamGroupHandler)->mAlgoCtx

#define AiqAlgoCamGroupHandler_getAlgoId(pAlgoCamGroupHandler) \
    (pAlgoCamGroupHandler)->mDes->id

#define AiqAlgoCamGroupHandler_getAlgoType(pAlgoCamGroupHandler) \
    (pAlgoCamGroupHandler)->mDes->type

#define AiqAlgoCamGroupHandler_setNextHdl(pAlgoCamGroupHandler, next) \
    (pAlgoCamGroupHandler)->mNextHdl = next 

#define AiqAlgoCamGroupHandler_getNextHdl(pAlgoCamGroupHandler) \
    (pAlgoCamGroupHandler)->mNextHdl

#define AiqAlgoCamGroupHandler_setParentHdl(pAlgoCamGroupHandler, parent) \
    (pAlgoCamGroupHandler)->mParentHdl = parent 

#define AiqAlgoCamGroupHandler_setParentHdl(pAlgoCamGroupHandler, parent) \
    (pAlgoCamGroupHandler)->mParentHdl = parent 

#define AiqAlgoCamGroupHandler_setAlgoHandle(pAlgoCamGroupHandler, single) \
    (pAlgoCamGroupHandler)->mSingleHdl = single

AiqAlgoCamGroupHandler_t* createAlgoCamGroupHandlerInstance(int type, RkAiqAlgoDesComm* des, AiqCamGroupManager_t* camGroupMg, AiqCore_t* aiqcore);
void destroyAlgoCamGroupHandler(AiqAlgoCamGroupHandler_t* pAlgoCamGroupHandler);

RKAIQ_END_DECLARE

#endif
