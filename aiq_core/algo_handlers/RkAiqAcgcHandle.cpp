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
#include "RkAiqAcgcHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAcgcHandleInt);

void RkAiqAcgcHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcgc());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAcgc());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAcgc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcgc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcgc());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAcgc());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAcgc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAcgcHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acgc handle prepare failed");

    RkAiqAlgoConfigAcgc* acgc_config_int = (RkAiqAlgoConfigAcgc*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acgc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcgcHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAcgc* acgc_pre_int        = (RkAiqAlgoPreAcgc*)mPreInParam;
    RkAiqAlgoPreResAcgc* acgc_pre_res_int = (RkAiqAlgoPreResAcgc*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acgc handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "acgc algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcgcHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcgc* acgc_proc_int        = (RkAiqAlgoProcAcgc*)mProcInParam;
    RkAiqAlgoProcResAcgc* acgc_proc_res_int = (RkAiqAlgoProcResAcgc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acgc handle processing failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "acgc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcgcHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAcgc* acgc_post_int        = (RkAiqAlgoPostAcgc*)mPostInParam;
    RkAiqAlgoPostResAcgc* acgc_post_res_int = (RkAiqAlgoPostResAcgc*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acgc handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "acgc algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcgcHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAcgc* acgc_com              = (RkAiqAlgoProcResAcgc*)mProcOutParam;
    rk_aiq_isp_cgc_params_v20_t* cgc_param      = params->mCgcParams->data().ptr();

    if (sharedCom->init) {
        cgc_param->frame_id = 0;
    } else {
        cgc_param->frame_id = shared->frameId;
    }

    if (!acgc_com) {
        LOGD_ANALYZER("no acgc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAcgc* acgc_rk = (RkAiqAlgoProcResAcgc*)acgc_com;
    }

    cur_params->mCgcParams = params->mCgcParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
