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
#include "RkAiqAldcHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAldcHandleInt);

void RkAiqAldcHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAldc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAldc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAldc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAldcHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aldch handle prepare failed");

    RkAiqAlgoConfigAldc* config                 = (RkAiqAlgoConfigAldc*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (sharedCom->resourcePath)
        strcpy(config->iqpath, sharedCom->resourcePath);
    else
        strcpy(config->iqpath, "/etc/iqfiles");

    config->mem_ops           = mAiqCore->mShareMemOps;
    config->is_multi_isp      = sharedCom->is_multi_isp_mode;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "aldch algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAldcHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAldcHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAldc* aldch_proc_int       = (RkAiqAlgoProcAldc*)mProcInParam;
    RkAiqAlgoProcResAldc* aldc_proc_res_int = (RkAiqAlgoProcResAldc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    aldc_proc_res_int->config = &shared->fullParams->mLdcParams->data()->result.cfg;

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

XCamReturn RkAiqAldcHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    return ret;
}

XCamReturn RkAiqAldcHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        rk_aiq_uapi_aldc_SetAttrib(mAlgoCtx, mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldcHandleInt::setAttrib(const rk_aiq_ldc_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_aldc_SetAttrib(mAlgoCtx, att, false);
#else
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && memcmp(&mCurAtt, att, sizeof(*att)))
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

XCamReturn RkAiqAldcHandleInt::getAttrib(rk_aiq_ldc_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_aldc_GetAttrib(mAlgoCtx, att);
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

XCamReturn RkAiqAldcHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAldc* aldc_com              = (RkAiqAlgoProcResAldc*)mProcOutParam;
    rk_aiq_isp_ldc_params_t* ldc_param          = params->mLdcParams->data().ptr();

    if (!aldc_com) {
        LOGD_ANALYZER("no aldch result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (sharedCom->init) {
        ldc_param->frame_id = 0;
        shared->frameId     = 0;
    } else {
        ldc_param->frame_id = shared->frameId;
    }
    ldc_param->result.ldch_en = aldc_com->ldch_en;
    ldc_param->result.ldcv_en = aldc_com->ldcv_en;

    if (aldc_com->res_com.cfg_update) {
        mSyncFlag            = shared->frameId;
        ldc_param->sync_flag = mSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mLdcParams = params->mLdcParams;
        ldc_param->is_update   = true;
        LOGD_ALDC("[%d] params from algo", mSyncFlag);
    } else if (mSyncFlag != ldc_param->sync_flag) {
        ldc_param->sync_flag = mSyncFlag;
        // copy from latest result
        if (cur_params->mLdcParams.ptr()) {
            ldc_param->is_update = true;
            ldc_param->result    = cur_params->mLdcParams->data()->result;
        } else {
            LOGE_ALDC("no latest params !");
            ldc_param->is_update = false;
        }
        LOGD_ALDC("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        ldc_param->is_update = false;
        LOG1_ALDC("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam
