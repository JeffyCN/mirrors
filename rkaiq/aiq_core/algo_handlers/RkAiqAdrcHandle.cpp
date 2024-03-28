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
    adrc_config_int->compr_bit    = sharedCom->snsDes.compr_bit;
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
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdrc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdrc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdrcHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_DRC_V10
        mCurAttV10 = mNewAttV10;
        rk_aiq_uapi_adrc_v10_SetAttrib(mAlgoCtx, &mCurAttV10, true);
        updateAtt = false;
        sendSignal(mCurAttV10.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DRC_V11
        mCurAttV11 = mNewAttV11;
        rk_aiq_uapi_adrc_v11_SetAttrib(mAlgoCtx, &mCurAttV11, true);
        updateAtt = false;
        sendSignal(mCurAttV11.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DRC_V12
        mCurAttV12 = mNewAttV12;
        rk_aiq_uapi_adrc_v12_SetAttrib(mAlgoCtx, &mCurAttV12, true);
        updateAtt = false;
        sendSignal(mCurAttV12.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
        mCurAttV12Lite = mNewAttV12Lite;
        rk_aiq_uapi_adrc_v12_lite_SetAttrib(mAlgoCtx, &mCurAttV12Lite, true);
        updateAtt = false;
        sendSignal(mCurAttV12Lite.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DRC_V20
        mCurAttV20 = mNewAttV20;
        rk_aiq_uapi_adrc_v20_SetAttrib(mAlgoCtx, &mCurAttV20, true);
        updateAtt = false;
        sendSignal(mCurAttV20.sync.sync_mode);
#endif
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if RKAIQ_HAVE_DRC_V10
XCamReturn RkAiqAdrcHandleInt::setAttribV10(const drcAttrV10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adrc_v10_SetAttrib(mAlgoCtx, att, true);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV10, att, sizeof(drcAttrV10_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV10, att, sizeof(drcAttrV10_t)))
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
XCamReturn RkAiqAdrcHandleInt::getAttribV10(drcAttrV10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_adrc_v10_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v10_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV10, sizeof(drcAttrV10_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v10_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV10.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V11
XCamReturn RkAiqAdrcHandleInt::setAttribV11(const drcAttrV11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adrc_v11_SetAttrib(mAlgoCtx, att, true);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV11, att, sizeof(drcAttrV11_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV11, att, sizeof(drcAttrV11_t)))
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
XCamReturn RkAiqAdrcHandleInt::getAttribV11(drcAttrV11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_adrc_v11_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v11_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV11, sizeof(drcAttrV11_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v11_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV11.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V12
XCamReturn RkAiqAdrcHandleInt::setAttribV12(const drcAttrV12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
#ifndef USE_NEWSTRUCT
    ret = rk_aiq_uapi_adrc_v12_SetAttrib(mAlgoCtx, att, true);
#endif
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV12, att, sizeof(drcAttrV12_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV12, att, sizeof(drcAttrV12_t)))
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
XCamReturn RkAiqAdrcHandleInt::getAttribV12(drcAttrV12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
#ifndef USE_NEWSTRUCT
    mCfgMutex.lock();
    rk_aiq_uapi_adrc_v12_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#endif
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v12_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV12, sizeof(drcAttrV12_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v12_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV12.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
XCamReturn RkAiqAdrcHandleInt::setAttribV12Lite(const drcAttrV12Lite_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adrc_v12_lite_SetAttrib(mAlgoCtx, att, true);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV12Lite, att, sizeof(drcAttrV12Lite_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV12Lite, att, sizeof(drcAttrV12Lite_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV12Lite = *att;
        updateAtt      = true;
        waitSignal(att->sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAdrcHandleInt::getAttribV12Lite(drcAttrV12Lite_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_adrc_v12_lite_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v12_lite_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV12Lite, sizeof(drcAttrV12Lite_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v12_lite_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV12Lite.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DRC_V20
XCamReturn RkAiqAdrcHandleInt::setAttribV20(const drcAttrV20_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adrc_v20_SetAttrib(mAlgoCtx, att, true);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV20, att, sizeof(drcAttrV20_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV20, att, sizeof(drcAttrV20_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV20 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAdrcHandleInt::getAttribV20(drcAttrV20_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_adrc_v20_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adrc_v20_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV20, sizeof(drcAttrV20_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adrc_v20_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV20.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqAdrcHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
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
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdrcHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdrc* adrc_proc_int        = (RkAiqAlgoProcAdrc*)mProcInParam;
    RkAiqAlgoProcResAdrc* adrc_proc_res_int = (RkAiqAlgoProcResAdrc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    adrc_proc_res_int->AdrcProcRes = &shared->fullParams->mDrcParams->data()->result;
    adrc_proc_int->LongFrmMode = mAeProcRes.LongFrmMode;

#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || (RKAIQ_HAVE_DRC_V20 && !USE_NEWSTRUCT)
    adrc_proc_int->ablcV32_proc_res.blc_ob_enable = shared->res_comb.ablcV32_proc_res->blc_ob_enable;
    adrc_proc_int->ablcV32_proc_res.isp_ob_predgain =
        shared->res_comb.ablcV32_proc_res->isp_ob_predgain;
#endif

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adrc handle processing failed");
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "adrc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdrcHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
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
#endif
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

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAdrc* ahdr_rk = (RkAiqAlgoProcResAdrc*)adrc_com;

        rk_aiq_isp_drc_params_t* drc_param = params->mDrcParams->data().ptr();
        if (sharedCom->init) {
            drc_param->frame_id = 0;
        } else {
            drc_param->frame_id = shared->frameId;
        }

        if (adrc_com->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            drc_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mDrcParams = params->mDrcParams;
            drc_param->is_update = true;
            LOGD_ATMO("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != drc_param->sync_flag) {
            drc_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mDrcParams.ptr()) {
                drc_param->result = cur_params->mDrcParams->data()->result;
                drc_param->is_update = true;
            } else {
                LOGE_ATMO("no latest params !");
                drc_param->is_update = false;
            }
            LOGD_ATMO("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            drc_param->is_update = false;
            LOGD_ATMO("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;

}

}  // namespace RkCam
