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
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAmerge());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAmerge());

    EXIT_ANALYZER_FUNCTION();
}
XCamReturn RkAiqAmergeHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_MERGE_V10
        mCurAttV10 = mNewAttV10;
        rk_aiq_uapi_amerge_v10_SetAttrib(mAlgoCtx, &mCurAttV10, true);
        updateAtt = false;
        sendSignal(mCurAttV10.sync.sync_mode);
#endif
#if RKAIQ_HAVE_MERGE_V11
        mCurAttV11 = mNewAttV11;
        rk_aiq_uapi_amerge_v11_SetAttrib(mAlgoCtx, &mCurAttV11, true);
        updateAtt = false;
        sendSignal(mCurAttV11.sync.sync_mode);
#endif
#if RKAIQ_HAVE_MERGE_V12
        mCurAttV12 = mNewAttV12;
        rk_aiq_uapi_amerge_v12_SetAttrib(mAlgoCtx, &mCurAttV12, true);
        updateAtt = false;
        sendSignal(mCurAttV12.sync.sync_mode);
#endif
    }
    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if RKAIQ_HAVE_MERGE_V10
XCamReturn RkAiqAmergeHandleInt::setAttribV10(const mergeAttrV10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_amerge_v10_SetAttrib(mAlgoCtx, att, false);
#else

    // check if there is different between att & mCurAttV10(sync)/mNewAttV10(async)
    // if something changed, set att to mNewAttV10, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV10, att, sizeof(mergeAttrV10_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV10, att, sizeof(mergeAttrV10_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV10 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAmergeHandleInt::getAttribV10(mergeAttrV10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_amerge_v10_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_amerge_v10_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV10, sizeof(mergeAttrV10_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_amerge_v10_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV10.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

#if RKAIQ_HAVE_MERGE_V11
XCamReturn RkAiqAmergeHandleInt::setAttribV11(const mergeAttrV11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_amerge_v11_SetAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAttV11(sync)/mNewAttV11(async)
    // if something changed, set att to mNewAttV11, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV11, att, sizeof(mergeAttrV11_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV11, att, sizeof(mergeAttrV11_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV11 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAmergeHandleInt::getAttribV11(mergeAttrV11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_amerge_v11_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_amerge_v11_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV11, sizeof(mergeAttrV11_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_amerge_v11_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV11.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

#if RKAIQ_HAVE_MERGE_V12
XCamReturn RkAiqAmergeHandleInt::setAttribV12(const mergeAttrV12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_amerge_v12_SetAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAttV12(sync)/mNewAttV12(async)
    // if something changed, set att to mNewAttV12, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV12, att, sizeof(mergeAttrV12_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV12, att, sizeof(mergeAttrV12_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV12 = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAmergeHandleInt::getAttribV12(mergeAttrV12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_amerge_v12_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_amerge_v12_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV12, sizeof(mergeAttrV12_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_amerge_v12_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV12.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqAmergeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "amerge handle prepare failed");

    RkAiqAlgoConfigAmerge* amerge_config_int = (RkAiqAlgoConfigAmerge*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

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
#if 0
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
#endif
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

    amerge_proc_res_int->AmergeProcRes = &shared->fullParams->mMergeParams->data()->result;
    amerge_proc_int->LongFrmMode = mAeProcRes.LongFrmMode;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amerge handle processing failed");
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "amerge algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmergeHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0 
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
#endif
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

    rk_aiq_isp_merge_params_t* merge_param = params->mMergeParams->data().ptr();

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

        if (amerge_com->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            merge_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mMergeParams = params->mMergeParams;
            merge_param->is_update = true;
            LOGD_AMERGE("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != merge_param->sync_flag) {
            merge_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mMergeParams.ptr()) {
                merge_param->result = cur_params->mMergeParams->data()->result;
                merge_param->is_update = true;
            } else {
                LOGE_AMERGE("no latest params !");
                merge_param->is_update = false;
            }
            LOGD_AMERGE("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            merge_param->is_update = false;
            LOGD_AMERGE("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
