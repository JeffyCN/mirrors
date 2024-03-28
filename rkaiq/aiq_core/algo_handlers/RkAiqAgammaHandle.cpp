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
#include "RkAiqAgammaHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAgammaHandleInt);

void RkAiqAgammaHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgamma());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgamma());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgamma());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAgammaHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_GAMMA_V10
        mCurAttV10 = mNewAttV10;
        rk_aiq_uapi_agamma_v10_SetAttrib(mAlgoCtx, &mCurAttV10, false);
        updateAtt = false;
        sendSignal(mCurAttV10.sync.sync_mode);
#endif
#if RKAIQ_HAVE_GAMMA_V11
        mCurAttV11 = mNewAttV11;
        rk_aiq_uapi_agamma_v11_SetAttrib(mAlgoCtx, &mCurAttV11, false);
        updateAtt = false;
        sendSignal(mCurAttV11.sync.sync_mode);
#endif
    }

    if (needSync) mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if RKAIQ_HAVE_GAMMA_V10
XCamReturn RkAiqAgammaHandleInt::setAttribV10(const rk_aiq_gamma_v10_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_agamma_v10_SetAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAttV10(sync)/mNewAttV10(async)
    // if something changed, set att to mNewAttV10, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV10, att, sizeof(rk_aiq_gamma_v10_attr_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV10, att, sizeof(rk_aiq_gamma_v10_attr_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV10 = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgammaHandleInt::getAttribV10(rk_aiq_gamma_v10_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_agamma_v10_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_agamma_v10_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV10, sizeof(rk_aiq_gamma_v10_attr_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_agamma_v10_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV10.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_GAMMA_V11
XCamReturn RkAiqAgammaHandleInt::setAttribV11(const rk_aiq_gamma_v11_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
#ifndef USE_NEWSTRUCT
    ret = rk_aiq_uapi_agamma_v11_SetAttrib(mAlgoCtx, att, false);
#endif
#else
    // check if there is different between att & mCurAttV11(sync)/mNewAttV11(async)
    // if something changed, set att to mNewAttV11, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV11, att, sizeof(rk_aiq_gamma_v11_attr_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV11, att, sizeof(rk_aiq_gamma_v11_attr_t)))
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

XCamReturn RkAiqAgammaHandleInt::getAttribV11(rk_aiq_gamma_v11_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
#ifndef USE_NEWSTRUCT
    mCfgMutex.lock();
    rk_aiq_uapi_agamma_v11_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#endif
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_agamma_v11_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV11, sizeof(rk_aiq_gamma_v11_attr_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_agamma_v11_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV11.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqAgammaHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "agamma handle prepare failed");

    RkAiqAlgoConfigAgamma* agamma_config_int = (RkAiqAlgoConfigAgamma*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

#ifdef RKAIQ_ENABLE_PARSER_V1
    agamma_config_int->calib = sharedCom->calib;
#endif

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "agamma algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAgammaHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAgamma* agamma_pre_int        = (RkAiqAlgoPreAgamma*)mPreInParam;
    RkAiqAlgoPreResAgamma* agamma_pre_res_int = (RkAiqAlgoPreResAgamma*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "agamma handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "agamma algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAgammaHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAgamma* agamma_proc_int        = (RkAiqAlgoProcAgamma*)mProcInParam;
    RkAiqAlgoProcResAgamma* agamma_proc_res_int = (RkAiqAlgoProcResAgamma*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    agamma_proc_res_int->GammaProcRes = &shared->fullParams->mAgammaParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "agamma handle processing failed");
    }

#ifdef RKAIQ_ENABLE_PARSER_V1
    agamma_proc_int->calib = sharedCom->calib;
#endif

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "agamma algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgammaHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0 
    RkAiqAlgoPostAgamma* agamma_post_int        = (RkAiqAlgoPostAgamma*)mPostInParam;
    RkAiqAlgoPostResAgamma* agamma_post_res_int = (RkAiqAlgoPostResAgamma*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "agamma handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "agamma algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAgammaHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAgamma* agamma_com = (RkAiqAlgoProcResAgamma*)mProcOutParam;
    rk_aiq_isp_agamma_params_t* agamma_param = params->mAgammaParams->data().ptr();

    if (!agamma_com) {
        LOGD_ANALYZER("no agamma result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAgamma* agamma_int = (RkAiqAlgoProcResAgamma*)agamma_com;
        if (sharedCom->init) {
            agamma_param->frame_id = 0;
        } else {
            agamma_param->frame_id = shared->frameId;
        }

        if (agamma_com->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            agamma_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mAgammaParams = params->mAgammaParams;
            agamma_param->is_update = true;
            LOGD_AGAMMA("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != agamma_param->sync_flag) {
            agamma_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mAgammaParams.ptr()) {
                agamma_param->result = cur_params->mAgammaParams->data()->result;
                agamma_param->is_update = true;
            } else {
                LOGE_AGAMMA("no latest params !");
                agamma_param->is_update = false;
            }
            LOGD_AGAMMA("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            agamma_param->is_update = false;
            LOGD_AGAMMA("[%d] params needn't update", shared->frameId);
        }

    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
