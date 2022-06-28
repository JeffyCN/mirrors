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
#include "RkAiqAdpccHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAdpccHandleInt);

void RkAiqAdpccHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdpcc());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdpcc());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdpcc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdpcc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdpcc());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdpcc());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdpcc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdpccHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_adpcc_SetAttrib(mAlgoCtx, &mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdpccHandleInt::setAttrib(rk_aiq_dpcc_attrib_V20_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdpccHandleInt::getAttrib(rk_aiq_dpcc_attrib_V20_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
      mCfgMutex.lock();
      rk_aiq_uapi_adpcc_GetAttrib(mAlgoCtx, att);
      att->sync.done = true;
      mCfgMutex.unlock();
    } else {
      if (updateAtt) {
        memcpy(att, &mNewAtt, sizeof(updateAtt));
        att->sync.done = false;
      } else {
        rk_aiq_uapi_adpcc_GetAttrib(mAlgoCtx, att);
        att->sync.sync_mode = mNewAtt.sync.sync_mode;
        att->sync.done      = true;
      }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdpccHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adpcc handle prepare failed");

    RkAiqAlgoConfigAdpcc* adpcc_config_int = (RkAiqAlgoConfigAdpcc*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adpcc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdpccHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdpcc* adpcc_pre_int        = (RkAiqAlgoPreAdpcc*)mPreInParam;
    RkAiqAlgoPreResAdpcc* adpcc_pre_res_int = (RkAiqAlgoPreResAdpcc*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adpcc handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adpcc algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdpccHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdpcc* adpcc_proc_int        = (RkAiqAlgoProcAdpcc*)mProcInParam;
    RkAiqAlgoProcResAdpcc* adpcc_proc_res_int = (RkAiqAlgoProcResAdpcc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adpcc handle processing failed");
    }

    // TODO: fill procParam
    adpcc_proc_int->iso      = sharedCom->iso;
    adpcc_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adpcc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdpccHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAsd* asd_post_int        = (RkAiqAlgoPostAsd*)mPostInParam;
    RkAiqAlgoPostResAsd* asd_post_res_int = (RkAiqAlgoPostResAsd*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "asd handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "asd algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdpccHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAdpcc* adpcc_com            = (RkAiqAlgoProcResAdpcc*)mProcOutParam;
    rk_aiq_isp_dpcc_params_v20_t* dpcc_param    = params->mDpccParams->data().ptr();

    if (!adpcc_com) {
        LOGD_ANALYZER("no adpcc result");
        return XCAM_RETURN_NO_ERROR;
    }

#if 0
    // gen rk adpcc result
    exp_param->SensorDpccInfo.enable = adpcc_com->SenDpccRes.enable;
    exp_param->SensorDpccInfo.cur_single_dpcc = adpcc_com->SenDpccRes.cur_single_dpcc;
    exp_param->SensorDpccInfo.cur_multiple_dpcc = adpcc_com->SenDpccRes.cur_multiple_dpcc;
    exp_param->SensorDpccInfo.total_dpcc = adpcc_com->SenDpccRes.total_dpcc;
#endif

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAdpcc* adpcc_rk = (RkAiqAlgoProcResAdpcc*)adpcc_com;

        if (sharedCom->init) {
            dpcc_param->frame_id = 0;
        } else {
            dpcc_param->frame_id = shared->frameId;
        }
        memcpy(&dpcc_param->result, &adpcc_rk->stAdpccProcResult, sizeof(rk_aiq_isp_dpcc_t));
    }

    cur_params->mDpccParams = params->mDpccParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
