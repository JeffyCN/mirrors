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
#include "RkAiqAcsmHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAcsmHandleInt);

void RkAiqAcsmHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcsm());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcsm());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcsm());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAcsmHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acsm handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acsm algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcsmHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAcsm* acsm_pre_int        = (RkAiqAlgoPreAcsm*)mPreInParam;
    RkAiqAlgoPreResAcsm* acsm_pre_res_int = (RkAiqAlgoPreResAcsm*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acsm handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "acsm algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcsmHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcsm* acsm_proc_int        = (RkAiqAlgoProcAcsm*)mProcInParam;
    RkAiqAlgoProcResAcsm* acsm_proc_res_int = (RkAiqAlgoProcResAcsm*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    acsm_proc_res_int->acsm_res = &shared->fullParams->mCsmParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acsm handle processing failed");
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "acsm algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcsmHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAcsm* acsm_post_int        = (RkAiqAlgoPostAcsm*)mPostInParam;
    RkAiqAlgoPostResAcsm* acsm_post_res_int = (RkAiqAlgoPostResAcsm*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acsm handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "acsm algo post_process failed");
    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAcsmHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_acsm_SetAttrib(mAlgoCtx, &mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcsmHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAcsm* acsm_com = (RkAiqAlgoProcResAcsm*)mProcOutParam;
    rk_aiq_isp_csm_params_t* csm_param = params->mCsmParams->data().ptr();

    if (!acsm_com) {
        LOGD_ANALYZER("no acsm result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (sharedCom->init) {
        csm_param->frame_id = 0;
    } else {
        csm_param->frame_id = shared->frameId;
    }

    if (acsm_com->res_com.cfg_update) {
        mSyncFlag = shared->frameId;
        csm_param->sync_flag = mSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mCsmParams = params->mCsmParams;
        csm_param->is_update = true;
        LOGD_ACSM("[%d] params from algo", mSyncFlag);
    } else if (mSyncFlag != csm_param->sync_flag) {
        csm_param->sync_flag = mSyncFlag;
        // copy from latest result
        if (cur_params->mCsmParams.ptr()) {
            csm_param->result = cur_params->mCsmParams->data()->result;
            csm_param->is_update = true;
        } else {
            LOGE_ACSM("no latest params !");
            csm_param->is_update = false;
        }
        LOGD_ACSM("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        csm_param->is_update = false;
        LOGD_ACSM("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcsmHandleInt::setAttrib(const rk_aiq_uapi_acsm_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_acsm_SetAttrib(mAlgoCtx, att, false);
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
        mNewAtt = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcsmHandleInt::getAttrib(rk_aiq_uapi_acsm_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_acsm_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_acsm_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_acsm_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam
