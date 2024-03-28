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
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcgc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcgc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAcgcHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acgc handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acgc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcgcHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
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
#endif
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

    acgc_proc_res_int->acgc_res = &shared->fullParams->mCgcParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acgc handle processing failed");
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "acgc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcgcHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
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
#endif
    return ret;
}

XCamReturn RkAiqAcgcHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        rk_aiq_uapi_acgc_SetAttrib(mAlgoCtx, &mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();
#endif
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
    rk_aiq_isp_cgc_params_t* cgc_param      = params->mCgcParams->data().ptr();

    if (sharedCom->init) {
        cgc_param->frame_id = 0;
    } else {
        cgc_param->frame_id = shared->frameId;
    }

    if (!acgc_com) {
        LOGE_ACGC("no acgc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (acgc_com->res_com.cfg_update) {
        mSyncFlag = shared->frameId;
        cgc_param->sync_flag = mSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mCgcParams = params->mCgcParams;
        cgc_param->is_update = true;
        LOGD_ACGC("[%d] params from algo", mSyncFlag);
    } else if (mSyncFlag != cgc_param->sync_flag) {
        cgc_param->sync_flag = mSyncFlag;
        // copy from latest result
        if (cur_params->mCgcParams.ptr()) {
            cgc_param->result = cur_params->mCgcParams->data()->result;
            cgc_param->is_update = true;
        } else {
            LOGE_ACGC("no latest params !");
            cgc_param->is_update = false;
        }
        LOGD_ACGC("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        cgc_param->is_update = false;
        LOGD_ACGC("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn RkAiqAcgcHandleInt::setAttrib(const rk_aiq_uapi_acgc_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_acgc_SetAttrib(mAlgoCtx, att, false);
#else
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
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcgcHandleInt::getAttrib(rk_aiq_uapi_acgc_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_acgc_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_acgc_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_acgc_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
