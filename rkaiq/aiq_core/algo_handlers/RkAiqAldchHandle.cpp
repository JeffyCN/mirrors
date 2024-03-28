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
#include "RkAiqAldchHandle.h"

#include "RkAiqCore.h"
#if (RKAIQ_HAVE_LDCH_V21)
#include "aldch/rk_aiq_types_aldch_algo_prvt.h"
#endif

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAldchHandleInt);

void RkAiqAldchHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAldch());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAldch());
#if (RKAIQ_HAVE_LDCH_V21)
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAldchV21());
#else
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAldch());
#endif

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAldchHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aldch handle prepare failed");

    RkAiqAlgoConfigAldch* aldch_config_int   = (RkAiqAlgoConfigAldch*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    // memcpy(&aldch_config_int->aldch_calib_cfg, &shared->calib->aldch, sizeof(CalibDb_LDCH_t));
    aldch_config_int->resource_path = sharedCom->resourcePath;
    aldch_config_int->mem_ops_ptr   = mAiqCore->mShareMemOps;
    aldch_config_int->is_multi_isp = sharedCom->is_multi_isp_mode;
    aldch_config_int->multi_isp_extended_pixel = sharedCom->multi_isp_extended_pixels;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    ret                             = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aldch algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAldchHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAldch* aldch_pre_int        = (RkAiqAlgoPreAldch*)mPreInParam;
    RkAiqAlgoPreResAldch* aldch_pre_res_int = (RkAiqAlgoPreResAldch*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aldch handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "aldch algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAldchHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAldch* aldch_proc_int        = (RkAiqAlgoProcAldch*)mProcInParam;
#if (RKAIQ_HAVE_LDCH_V21)
    RkAiqAlgoProcResAldchV21* aldch_proc_res_int = (RkAiqAlgoProcResAldchV21*)mProcOutParam;
#else
    RkAiqAlgoProcResAldch* aldch_proc_res_int = (RkAiqAlgoProcResAldch*)mProcOutParam;
#endif
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    aldch_proc_res_int->ldch_result = &shared->fullParams->mLdchParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aldch handle processing failed");
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "aldch algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAldch* aldch_post_int        = (RkAiqAlgoPostAldch*)mPostInParam;
    RkAiqAlgoPostResAldch* aldch_post_res_int = (RkAiqAlgoPostResAldch*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aldch handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "aldch algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAldchHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
#if (RKAIQ_HAVE_LDCH_V21)
        rk_aiq_uapi_aldch_v21_SetAttrib(mAlgoCtx, mCurAtt, false);
        mCurAtt.lut.update_flag = false;
#else
        rk_aiq_uapi_aldch_SetAttrib(mAlgoCtx, mCurAtt, false);
#endif
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if (RKAIQ_HAVE_LDCH_V21)
XCamReturn RkAiqAldchHandleInt::setAttrib(const rk_aiq_ldch_v21_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_aldch_v21_SetAttrib(mAlgoCtx, *att, false);
#else
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
        ret       = copyLutFromExtBuffer(att);
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandleInt::getAttrib(rk_aiq_ldch_v21_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_aldch_v21_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_aldch_v21_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_aldch_v21_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandleInt::copyLutFromExtBuffer(const rk_aiq_ldch_v21_attrib_t* att) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!mAlgoCtx->hLDCH) {
        LOGE_ALDCH("Ldch contex is NULL!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (att->update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_BUFFER && \
        att->lut.update_flag) {
        LDCHHandle_t ldchCtx = (LDCHHandle_t)mAlgoCtx->hLDCH;
        if (!ldchCtx->_lutCache.ptr()) {
            ldchCtx->_lutCache = new LutCache(att->lut.u.buffer.size);
        } else if (att->lut.u.buffer.size != ldchCtx->_lutCache->GetSize()) {
            ldchCtx->_lutCache.release();
            ldchCtx->_lutCache = new LutCache(att->lut.u.buffer.size);
        }

        if (ldchCtx->_lutCache.ptr()) {
            if (ldchCtx->_lutCache->GetBuffer()) {
                memcpy(ldchCtx->_lutCache->GetBuffer(), att->lut.u.buffer.addr, att->lut.u.buffer.size);
            }
        } else {
            LOGE_ALDCH("Failed to malloc ldch cache!");
            return XCAM_RETURN_ERROR_MEM;
        }
    }

    return ret;
}

#else
XCamReturn RkAiqAldchHandleInt::setAttrib(const rk_aiq_ldch_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_aldch_SetAttrib(mAlgoCtx, *att, false);
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

XCamReturn RkAiqAldchHandleInt::getAttrib(rk_aiq_ldch_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_aldch_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_aldch_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_aldch_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqAldchHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
#if (RKAIQ_HAVE_LDCH_V21)
    RkAiqAlgoProcResAldchV21* aldch_com         = (RkAiqAlgoProcResAldchV21*)mProcOutParam;
    rk_aiq_isp_ldch_params_t* ldch_param    = params->mLdchParams->data().ptr();
#else
    RkAiqAlgoProcResAldch* aldch_com         = (RkAiqAlgoProcResAldch*)mProcOutParam;
    rk_aiq_isp_ldch_params_t* ldch_param = params->mLdchParams->data().ptr();
#endif

    if (!aldch_com) {
        LOGD_ANALYZER("no aldch result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (sharedCom->init) {
        ldch_param->frame_id = 0;
        shared->frameId = 0;
    } else {
        ldch_param->frame_id = shared->frameId;
    }

    if (aldch_com->res_com.cfg_update) {
        mSyncFlag = shared->frameId;
        ldch_param->sync_flag = mSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mLdchParams = params->mLdchParams;
        ldch_param->is_update = true;
        LOGD_ALDCH("[%d] params from algo", mSyncFlag);
    } else if (mSyncFlag != ldch_param->sync_flag) {
        ldch_param->sync_flag = mSyncFlag;
        // copy from latest result
        if (cur_params->mLdchParams.ptr()) {
            ldch_param->is_update = true;
            ldch_param->result = cur_params->mLdchParams->data()->result;
        } else {
            LOGE_ALDCH("no latest params !");
            ldch_param->is_update = false;
        }
        LOGD_ALDCH("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        ldch_param->is_update = false;
        LOG1_ALDCH("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
