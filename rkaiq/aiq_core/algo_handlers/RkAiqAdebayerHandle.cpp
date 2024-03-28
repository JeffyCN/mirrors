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
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdebayer());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdebayer());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdebayerHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_DEBAYER_V1
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_adebayer_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
#endif

#if RKAIQ_HAVE_DEBAYER_V2
        mCurAttV2 = mNewAttV2;
        rk_aiq_uapi_adebayer_v2_SetAttrib(mAlgoCtx, mCurAttV2, false);
        sendSignal(mCurAttV2.sync.sync_mode);
        updateAtt = false;
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
        mCurAttV2Lite = mNewAttV2Lite;
        rk_aiq_uapi_adebayer_v2lite_SetAttrib(mAlgoCtx, mCurAttV2Lite, false);
        sendSignal(mCurAttV2Lite.sync.sync_mode);
        updateAtt = false;
#endif

#if RKAIQ_HAVE_DEBAYER_V3
        mCurAttV3 = mNewAttV3;
        rk_aiq_uapi_adebayer_v3_SetAttrib(mAlgoCtx, mCurAttV3, false);
        sendSignal(mCurAttV3.sync.sync_mode);
        updateAtt = false;
#endif

    }

    if (needSync) mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#if RKAIQ_HAVE_DEBAYER_V1
XCamReturn RkAiqAdebayerHandleInt::setAttrib(adebayer_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adebayer_SetAttrib(mAlgoCtx, att, false);
#else
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
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::getAttrib(adebayer_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_adebayer_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
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
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

#if RKAIQ_HAVE_DEBAYER_V2
XCamReturn RkAiqAdebayerHandleInt::setAttribV2(adebayer_v2_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
#ifndef USE_NEWSTRUCT
    ret = rk_aiq_uapi_adebayer_v2_SetAttrib(mAlgoCtx, att, false);
#endif
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewAttV2, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV2, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV2   = att;
        updateAtt = true;
        waitSignal(att.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::getAttribV2(adebayer_v2_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
#ifndef USE_NEWSTRUCT
    mCfgMutex.lock();
    rk_aiq_uapi_adebayer_v2_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#endif
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adebayer_v2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV2, sizeof(mNewAttV2));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adebayer_v2_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV2.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
XCamReturn RkAiqAdebayerHandleInt::setAttribV2(adebayer_v2lite_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adebayer_v2lite_SetAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewAttV2Lite, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV2Lite, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV2Lite = att;
        updateAtt = true;
        waitSignal(att.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::getAttribV2(adebayer_v2lite_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_adebayer_v2lite_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adebayer_v2lite_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV2Lite, sizeof(mNewAttV2Lite));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adebayer_v2lite_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV2Lite.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

#if RKAIQ_HAVE_DEBAYER_V3
XCamReturn RkAiqAdebayerHandleInt::setAttribV3(adebayer_v3_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adebayer_v3_SetAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewAttV3, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV3, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV3   = att;
        updateAtt = true;
        waitSignal(att.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::getAttribV3(adebayer_v3_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_adebayer_v3_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adebayer_v3_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV3, sizeof(mNewAttV3));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adebayer_v3_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV3.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqAdebayerHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adebayer handle prepare failed");

    RkAiqAlgoConfigAdebayer* adebayer_config_int = (RkAiqAlgoConfigAdebayer*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    adebayer_config_int->compr_bit = sharedCom->snsDes.compr_bit;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdebayerHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
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
#endif
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

#if RKAIQ_HAVE_DEBAYER_V1
    adebayer_proc_res_int->debayerResV1.config = &shared->fullParams->mDebayerParams->data()->result;
#endif
#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE
    adebayer_proc_res_int->debayerResV2.config = &shared->fullParams->mDebayerParams->data()->result;
#endif
#if RKAIQ_HAVE_DEBAYER_V3
    adebayer_proc_res_int->debayerResV3.config = &shared->fullParams->mDebayerParams->data()->result;
#endif

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adebayer handle processing failed");
    }

    // TODO: fill procParam
    adebayer_proc_int->hdr_mode = sharedCom->working_mode;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdebayerHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
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
#endif
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

    rk_aiq_isp_debayer_params_t* debayer_param = params->mDebayerParams->data().ptr();
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

        if (adebayer_com->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            debayer_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mDebayerParams = params->mDebayerParams;
            debayer_param->is_update = true;
            LOGD_ADEBAYER("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != debayer_param->sync_flag) {
            debayer_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mDebayerParams.ptr()) {
                debayer_param->result = cur_params->mDebayerParams->data()->result;
                debayer_param->is_update = true;
            } else {
                LOGE_ADEBAYER("no latest params !");
                debayer_param->is_update = false;
            }
            LOGD_ADEBAYER("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            debayer_param->is_update = false;
            LOGD_ADEBAYER("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
