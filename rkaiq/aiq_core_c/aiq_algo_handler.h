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
#ifndef _AIQ_ALGO_HANDLE_H_
#define _AIQ_ALGO_HANDLE_H_

#include "xcore/base/xcam_common.h"
#include "c_base/aiq_mutex.h"
#include "algos/rk_aiq_algo_des.h"
#include "common/rk_aiq_types_priv_c.h"

//forward declare
typedef struct AiqCore_s AiqCore_t;

RKAIQ_BEGIN_DECLARE

enum {
    RKAIQ_CONFIG_COM_PREPARE,
    RKAIQ_CONFIG_COM_PRE,
    RKAIQ_CONFIG_COM_PROC,
    RKAIQ_CONFIG_COM_POST,
};

typedef struct AiqAlgoHandler_s AiqAlgoHandler_t;

struct AiqAlgoHandler_s {
    XCamReturn (*prepare)(AiqAlgoHandler_t* pAlgoHandler);
    XCamReturn (*preProcess)(AiqAlgoHandler_t* pAlgoHandler);
    XCamReturn (*processing)(AiqAlgoHandler_t* pAlgoHandler);
    XCamReturn (*postProcess)(AiqAlgoHandler_t* pAlgoHandler);
    XCamReturn (*genIspResult)(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
                               AiqFullParams_t* cur_params);
    XCamReturn (*queryInterpIso)(AiqAlgoHandler_t* pAlgoHandler, uint32_t* interpIso, uint32_t* isoH, uint32_t* isoL);
    // internal used
    XCamReturn (*configInparamsCom)(AiqAlgoHandler_t* pAlgoHandler, RkAiqAlgoCom* com, int type);
    void (*init)(AiqAlgoHandler_t* pAlgoHandler);
    void (*deinit)(AiqAlgoHandler_t* pAlgoHandler);

    RkAiqAlgoCom* mConfig;
    RkAiqAlgoCom* mPreInParam;
    RkAiqAlgoResCom* mPreOutParam;
    RkAiqAlgoCom* mProcInParam;
    RkAiqAlgoResCom* mProcOutParam;
    RkAiqAlgoCom* mPostInParam;
    RkAiqAlgoResCom* mPostOutParam;
    const RkAiqAlgoDesComm* mDes;
    RkAiqAlgoContext* mAlgoCtx;
    AiqCore_t* mAiqCore;
    bool mEnable;
    bool mReConfig;
    uint32_t mGroupId;
    void* mAlogsGroupSharedParams;
    AiqMutex_t mCfgMutex;
    AiqAlgoHandler_t* mNextHdl;
    AiqAlgoHandler_t* mParentHdl;
    bool mIsMulRun;
    bool mPostShared;
    uint32_t mSyncFlag;
    bool mIsUpdateGrpAttr;
    int mResultType;
    int mResultSize;
    rk_aiq_op_mode_t mOpMode;
};

// generic constructor/destructor
XCamReturn AiqAlgoHandler_constructor(AiqAlgoHandler_t* pAlgoHandler, RkAiqAlgoDesComm* des,
                                      AiqCore_t* aiqCore);
void AiqAlgoHandler_destructor(AiqAlgoHandler_t* pAlgoHandler);
void AiqAlgoHandler_deinit(AiqAlgoHandler_t* pAlgoHandler);
XCamReturn AiqAlgoHandler_prepare(AiqAlgoHandler_t* pAlgoHandler);
XCamReturn AiqAlgoHandler_preProcess(AiqAlgoHandler_t* pAlgoHandler);
XCamReturn AiqAlgoHandler_processing(AiqAlgoHandler_t* pAlgoHandler);
XCamReturn AiqAlgoHandler_postProcess(AiqAlgoHandler_t* pAlgoHandler);
XCamReturn AiqAlgoHandler_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
                                       AiqFullParams_t* cur_params);
XCamReturn AiqAlgoHandler_configInparamsCom(AiqAlgoHandler_t* pAlgoHandler, RkAiqAlgoCom* com,
                                            int type);

void AiqAlgoHandler_setEnable(AiqAlgoHandler_t* pAlgoHandler, bool enable);
void AiqAlgoHandler_setReConfig(AiqAlgoHandler_t* pAlgoHandler, bool reconfig);
bool AiqAlgoHandler_getEnable(AiqAlgoHandler_t* pAlgoHandler);
RkAiqAlgoContext* AiqAlgoHandler_getAlgoCtx(AiqAlgoHandler_t* pAlgoHandler);
int AiqAlgoHandler_getAlgoId(AiqAlgoHandler_t* pAlgoHandler);
int AiqAlgoHandler_getAlgoType(AiqAlgoHandler_t* pAlgoHandler);
void AiqAlgoHandler_setGroupId(AiqAlgoHandler_t* pAlgoHandler, int32_t gId);
int32_t AiqAlgoHandler_getGroupId(AiqAlgoHandler_t* pAlgoHandler);
void AiqAlgoHandler_setNextHdl(AiqAlgoHandler_t* pAlgoHandler, AiqAlgoHandler_t* next);
AiqAlgoHandler_t* AiqAlgoHandler_getNextHdl(AiqAlgoHandler_t* pAlgoHandler);
void AiqAlgoHandler_setParentHdl(AiqAlgoHandler_t* pAlgoHandler, AiqAlgoHandler_t* parent);
AiqAlgoHandler_t* AiqAlgoHandler_getParent(AiqAlgoHandler_t* pAlgoHandler);
void AiqAlgoHandler_setMulRun(AiqAlgoHandler_t* pAlgoHandler, bool isMulRun);
void AiqAlgoHandler_setGroupShared(AiqAlgoHandler_t* pAlgoHandler, void* grp_shared);
void* AiqAlgoHandler_getGroupShared(AiqAlgoHandler_t* pAlgoHandler);
RkAiqAlgoResCom* AiqAlgoHandler_getProcProcRes(AiqAlgoHandler_t* pAlgoHandler);
bool AiqAlgoHandler_isUpdateGrpAttr(AiqAlgoHandler_t* pAlgoHandler);
void AiqAlgoHandler_clearUpdateGrpAttr(AiqAlgoHandler_t* pAlgoHandler);
uint64_t AiqAlgoHandler_grpId2GrpMask(AiqAlgoHandler_t* pAlgoHandler, uint32_t grpId);

AiqAlgoHandler_t* createAlgoHandlerInstance(int type, RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
void destroyAlgoHandler(AiqAlgoHandler_t* pAlgoHandler);

XCamReturn AiqAlgoHandler_do_processing_common(AiqAlgoHandler_t* pAlgoHandler);
XCamReturn AiqAlgoHandler_genIspResult_common(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params, AiqFullParams_t* cur_params);
XCamReturn AiqAlgoHandler_genIspResult_byType(AiqAlgoHandler_t* pAlgoHandler,
        AiqFullParams_t* params, AiqFullParams_t* cur_params, int restype, int size);

XCamReturn AiqAlgoHandler_queryStatus_common(AiqAlgoHandler_t* pAlgoHandler,
                                             rk_aiq_op_mode_t* opMode, bool* en, bool* bypass, void* stMan);
XCamReturn AiqAlgoHandler_queryInterpIso_common(AiqAlgoHandler_t* pAlgoHandler,
                                             uint32_t* interpIso, uint32_t* isoH, uint32_t* isoL);

RKAIQ_END_DECLARE

#endif
