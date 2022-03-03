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
#include "RkAiqAmergeHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAmergeHandleInt);

void RkAiqAmergeHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAmerge());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAmerge());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAmerge());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAmerge());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAmerge());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAmerge());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAmerge());

    EXIT_ANALYZER_FUNCTION();
}
XCamReturn RkAiqAmergeHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_amerge_SetAttrib(mAlgoCtx, mCurAtt, true);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmergeHandleInt::setAttrib(amerge_attrib_t att) {
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
XCamReturn RkAiqAmergeHandleInt::getAttrib(amerge_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_amerge_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(updateAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_amerge_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmergeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "amerge handle prepare failed");

    RkAiqAlgoConfigAmerge* amerge_config_int = (RkAiqAlgoConfigAmerge*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    // TODO
    amerge_config_int->rawHeight    = sharedCom->snsDes.isp_acq_height;
    amerge_config_int->rawWidth     = sharedCom->snsDes.isp_acq_width;
    amerge_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "amerge algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAmergeHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAmerge* amerge_pre_int        = (RkAiqAlgoPreAmerge*)mPreInParam;
    RkAiqAlgoPreResAmerge* amerge_pre_res_int = (RkAiqAlgoPreResAmerge*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amerge handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "amerge algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAmergeHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAmerge* amerge_proc_int        = (RkAiqAlgoProcAmerge*)mProcInParam;
    RkAiqAlgoProcResAmerge* amerge_proc_res_int = (RkAiqAlgoProcResAmerge*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amerge handle processing failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "amerge algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmergeHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAmerge* amerge_post_int        = (RkAiqAlgoPostAmerge*)mPostInParam;
    RkAiqAlgoPostResAmerge* amerge_post_res_int = (RkAiqAlgoPostResAmerge*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amerge handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "amerge algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmergeHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAmerge* amerge_com          = (RkAiqAlgoProcResAmerge*)mProcOutParam;

    rk_aiq_isp_merge_params_v20_t* merge_param = params->mMergeParams->data().ptr();

    if (!amerge_com) {
        LOGD_ANALYZER("no amerge result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAmerge* amerge_rk = (RkAiqAlgoProcResAmerge*)amerge_com;

        if (sharedCom->init) {
            merge_param->frame_id = 0;
        } else {
            merge_param->frame_id = shared->frameId;
        }

        merge_param->result = amerge_rk->AmergeProcRes;
    }

    cur_params->mMergeParams = params->mMergeParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
