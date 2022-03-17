/*
 * RkAiqHandle.h
 *
 *  Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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

#include "RkAiqHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

RkAiqHandle::RkAiqHandle(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)
    : mDes(des), mAiqCore(aiqCore), mEnable(true), mReConfig(false) {
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    mDes->create_context(&mAlgoCtx, (const _AlgoCtxInstanceCfg*)(&sharedCom->ctxCfigs[des->type]));
    mConfig       = NULL;
    mPreInParam   = NULL;
    mPreOutParam  = NULL;
    mProcInParam  = NULL;
    mProcOutParam = NULL;
    mPostInParam  = NULL;
    mPostOutParam = NULL;
}

RkAiqHandle::~RkAiqHandle() {
    if (mDes) mDes->destroy_context(mAlgoCtx);
}

XCamReturn RkAiqHandle::configInparamsCom(RkAiqAlgoCom* com, int type) {
    ENTER_ANALYZER_FUNCTION();

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    xcam_mem_clear(*com);

    if (type == RKAIQ_CONFIG_COM_PREPARE) {
        com->ctx                     = mAlgoCtx;
        com->frame_id                = shared->frameId;
        com->u.prepare.working_mode  = sharedCom->working_mode;
        com->u.prepare.sns_op_width  = sharedCom->snsDes.isp_acq_width;
        com->u.prepare.sns_op_height = sharedCom->snsDes.isp_acq_height;
        com->u.prepare.conf_type     = sharedCom->conf_type;
    } else {
        com->ctx         = mAlgoCtx;
        com->frame_id    = shared->frameId;
        com->u.proc.init = sharedCom->init;
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mConfig == NULL) init();
    // build common configs
    RkAiqAlgoCom* cfgParam = mConfig;
    configInparamsCom(cfgParam, RKAIQ_CONFIG_COM_PREPARE);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret         = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCom* preParam = mPreInParam;

    configInparamsCom(preParam, RKAIQ_CONFIG_COM_PRE);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqHandle::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCom* procParam = mProcInParam;

    configInparamsCom(procParam, RKAIQ_CONFIG_COM_PROC);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCom* postParam = mPostInParam;

    configInparamsCom(postParam, RKAIQ_CONFIG_COM_POST);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

void RkAiqHandle::deInit() {
    ENTER_ANALYZER_FUNCTION();

#define RKAIQ_DELLET(a) \
    if (a) {            \
        delete a;       \
        a = NULL;       \
    }

    RKAIQ_DELLET(mConfig);
    RKAIQ_DELLET(mPreInParam);
    RKAIQ_DELLET(mPreOutParam);
    RKAIQ_DELLET(mProcInParam);
    RKAIQ_DELLET(mProcOutParam);
    RKAIQ_DELLET(mPostInParam);
    RKAIQ_DELLET(mPostOutParam);

    EXIT_ANALYZER_FUNCTION();
}


};  // namespace RkCam
