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

#include "RkAiqAdebayerHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAdebayerHandleInt);

void RkAiqAdebayerHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdebayer());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdebayer());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdebayer());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdebayer());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdebayer());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdebayer());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdebayer());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdebayerHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_adebayer_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }
    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::setAttrib(adebayer_attrib_t att) {
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

XCamReturn RkAiqAdebayerHandleInt::getAttrib(adebayer_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adebayer_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adebayer_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adebayer handle prepare failed");

    RkAiqAlgoConfigAdebayer* adebayer_config_int = (RkAiqAlgoConfigAdebayer*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdebayerHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdebayer* adebayer_pre_int        = (RkAiqAlgoPreAdebayer*)mPreInParam;
    RkAiqAlgoPreResAdebayer* adebayer_pre_res_int = (RkAiqAlgoPreResAdebayer*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adebayer handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdebayerHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdebayer* adebayer_proc_int = (RkAiqAlgoProcAdebayer*)mProcInParam;
    RkAiqAlgoProcResAdebayer* adebayer_proc_res_int =
        (RkAiqAlgoProcResAdebayer*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adebayer handle processing failed");
    }

    // TODO: fill procParam
    adebayer_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAdebayer* adebayer_post_int = (RkAiqAlgoPostAdebayer*)mPostInParam;
    RkAiqAlgoPostResAdebayer* adebayer_post_res_int =
        (RkAiqAlgoPostResAdebayer*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adebayer handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::genIspResult(RkAiqFullParams* params,
                                                RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAdebayer* adebayer_com = (RkAiqAlgoProcResAdebayer*)mProcOutParam;
    rk_aiq_isp_debayer_params_v20_t* debayer_param = params->mDebayerParams->data().ptr();

    if (!adebayer_com) {
        LOGD_ANALYZER("no adebayer result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAdebayer* adebayer_rk = (RkAiqAlgoProcResAdebayer*)adebayer_com;
        if (sharedCom->init) {
            debayer_param->frame_id = 0;
        } else {
            debayer_param->frame_id = shared->frameId;
        }
        memcpy(&debayer_param->result, &adebayer_rk->debayerRes.config, sizeof(AdebayerConfig_t));
    }

    cur_params->mDebayerParams = params->mDebayerParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
