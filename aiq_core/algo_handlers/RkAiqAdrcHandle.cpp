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
#include "RkAiqAdrcHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAdrcHandleInt);

XCamReturn RkAiqAdrcHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adrc handle prepare failed");

    RkAiqAlgoConfigAdrc* adrc_config_int     = (RkAiqAlgoConfigAdrc*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    adrc_config_int->rawHeight    = sharedCom->snsDes.isp_acq_height;
    adrc_config_int->rawWidth     = sharedCom->snsDes.isp_acq_width;
    adrc_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adrc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqAdrcHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdrc());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdrc());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdrc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdrc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdrc());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdrc());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdrc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdrcHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_adrc_SetAttrib(mAlgoCtx, mCurAtt, true);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdrcHandleInt::setAttrib(drc_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAtt, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt   = att;
        updateAtt = true;
        waitSignal(att.sync.sync_mode);
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAdrcHandleInt::getAttrib(drc_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(updateAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdrcHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdrc* adrc_pre_int        = (RkAiqAlgoPreAdrc*)mPreInParam;
    RkAiqAlgoPreResAdrc* adrc_pre_res_int = (RkAiqAlgoPreResAdrc*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adrc handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrc algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdrcHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdrc* adrc_proc_int        = (RkAiqAlgoProcAdrc*)mProcInParam;
    RkAiqAlgoProcResAdrc* adrc_proc_res_int = (RkAiqAlgoProcResAdrc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adrc handle processing failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdrcHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAdrc* adrc_post_int        = (RkAiqAlgoPostAdrc*)mPostInParam;
    RkAiqAlgoPostResAdrc* adrc_post_res_int = (RkAiqAlgoPostResAdrc*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adrc handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adrc algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdrcHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAdrc* adrc_com = (RkAiqAlgoProcResAdrc*)mProcOutParam;

    if (!adrc_com) {
        LOGD_ANALYZER("no adrc result");
        return XCAM_RETURN_NO_ERROR;
    }

    RkAiqAlgoProcResAdrc* adrc_rk = (RkAiqAlgoProcResAdrc*)adrc_com;
    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAdrc* ahdr_rk = (RkAiqAlgoProcResAdrc*)adrc_com;

        rk_aiq_isp_drc_params_v21_t* drc_param = params->mDrcParams->data().ptr();
        if (sharedCom->init) {
            drc_param->frame_id = 0;
        } else {
            drc_param->frame_id = shared->frameId;
        }
        drc_param->result.DrcProcRes = ahdr_rk->AdrcProcRes.DrcProcRes;

        drc_param->result.CompressMode = ahdr_rk->AdrcProcRes.CompressMode;
        drc_param->result.update = ahdr_rk->AdrcProcRes.update;
        drc_param->result.LongFrameMode = ahdr_rk->AdrcProcRes.LongFrameMode;
        drc_param->result.isHdrGlobalTmo = ahdr_rk->AdrcProcRes.isHdrGlobalTmo;
        drc_param->result.bTmoEn = ahdr_rk->AdrcProcRes.bTmoEn;
        drc_param->result.isLinearTmo = ahdr_rk->AdrcProcRes.isLinearTmo;
    }

    cur_params->mDrcParams = params->mDrcParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;

}

};  // namespace RkCam
