/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqAsharpV34Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAsharpV34HandleInt);

void RkAiqAsharpV34HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAsharpV34());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAsharpV34());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAsharpV34());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAsharpV34HandleInt::updateConfig(bool needsync) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifndef DISABLE_HANDLE_ATTRIB
    if (needsync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        rk_aiq_uapi_asharpV34_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateStrength) {
        mCurStrength = mNewStrength;
        rk_aiq_uapi_asharpV34_SetStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needsync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::setAttrib(const rk_aiq_sharp_attrib_v34_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_asharpV34_SetAttrib(mAlgoCtx, att, false);
#else
    // if something changed
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && memcmp(&mCurAtt, att, sizeof(*att)))
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

XCamReturn RkAiqAsharpV34HandleInt::getAttrib(rk_aiq_sharp_attrib_v34_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_asharpV34_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_asharpV34_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_asharpV34_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::setStrength(const rk_aiq_sharp_strength_v34_t* pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_asharpV34_SetStrength(mAlgoCtx, pStrength);
#else
    bool isChanged = false;
    if (pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewStrength, pStrength, sizeof(*pStrength)))
        isChanged = true;
    else if (pStrength->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurStrength, pStrength, sizeof(*pStrength)))
        isChanged = true;

    if (isChanged) {
        mNewStrength   = *pStrength;
        updateStrength = true;
        waitSignal(pStrength->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::getStrength(rk_aiq_sharp_strength_v34_t* pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_asharpV34_GetStrength(mAlgoCtx, pStrength);
    mCfgMutex.unlock();
#else
    if (pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_asharpV34_GetStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateStrength) {
            *pStrength           = mNewStrength;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_asharpV34_GetStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::getInfo(rk_aiq_sharp_info_v34_t* pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_asharpV34_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_asharpV34_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret            = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "asharp handle prepare failed");

    RkAiqAlgoConfigAsharpV34* asharp_config_int = (RkAiqAlgoConfigAsharpV34*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    asharp_config_int->stAsharpConfig.rawWidth  = sharedCom->snsDes.isp_acq_width;
    asharp_config_int->stAsharpConfig.rawHeight = sharedCom->snsDes.isp_acq_height;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "asharp algo prepare failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAsharpV34* asharp_pre_int        = (RkAiqAlgoPreAsharpV34*)mPreInParam;
    RkAiqAlgoPreResAsharpV34* asharp_pre_res_int = (RkAiqAlgoPreResAsharpV34*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "asharp handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "asharp algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                                 = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcAsharpV34* asharp_proc_int        = (RkAiqAlgoProcAsharpV34*)mProcInParam;
    RkAiqAlgoProcResAsharpV34* asharp_proc_res_int = (RkAiqAlgoProcResAsharpV34*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    asharp_proc_res_int->stAsharpProcResult.stFix = &shared->fullParams->mSharpenParams->data()->result;
    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "asharp handle processing failed");
    }

    // TODO: fill procParam
    asharp_proc_int->iso                = sharedCom->iso;
    asharp_proc_int->hdr_mode           = sharedCom->working_mode;
    asharp_proc_int->stAblcV32_proc_res = shared->res_comb.ablcV32_proc_res;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "asharp algo processing failed");

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAsharpV34* asharp_post_int        = (RkAiqAlgoPostAsharpV34*)mPostInParam;
    RkAiqAlgoPostResAsharpV34* asharp_post_res_int = (RkAiqAlgoPostResAsharpV34*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "asharp handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "asharp algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAsharpV34HandleInt::genIspResult(RkAiqFullParams* params,
                                                 RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAsharpV34* asharp_rk        = (RkAiqAlgoProcResAsharpV34*)mProcOutParam;

    if (!asharp_rk) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_sharpen_params_t* sharp_param = params->mSharpenParams->data().ptr();
        if (sharedCom->init) {
            sharp_param->frame_id = 0;
        } else {
            sharp_param->frame_id = shared->frameId;
        }
        if (asharp_rk->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            sharp_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mSharpenParams = params->mSharpenParams;
            sharp_param->is_update = true;
            LOGD_ASHARP("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != sharp_param->sync_flag) {
            sharp_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mSharpenParams.ptr()) {
                sharp_param->result = cur_params->mSharpenParams->data()->result;
                sharp_param->is_update = true;
            } else {
                LOGE_ASHARP("no latest params !");
                sharp_param->is_update = false;
            }
            LOGD_ASHARP("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            sharp_param->is_update = false;
            LOGD_ASHARP("[%d] params needn't update", shared->frameId);
        }
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }


    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
