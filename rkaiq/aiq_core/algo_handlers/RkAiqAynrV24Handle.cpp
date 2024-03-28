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
#include "RkAiqAynrV24Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAynrV24HandleInt);

void RkAiqAynrV24HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAynrV24());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAynrV24());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAynrV24());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAynrV24HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        rk_aiq_uapi_aynrV24_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateStrength) {
        mCurStrength = mNewStrength;
        rk_aiq_uapi_aynrV24_SetLumaSFStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrV24HandleInt::setAttrib(const rk_aiq_ynr_attrib_v24_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_aynrV24_SetAttrib(mAlgoCtx, att, false);
#else
    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_ynr_attrib_v24_t))) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrV24HandleInt::getAttrib(rk_aiq_ynr_attrib_v24_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_aynrV24_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_aynrV24_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_aynrV24_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrV24HandleInt::setStrength(const rk_aiq_ynr_strength_v24_t* pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_aynrV24_SetLumaSFStrength(mAlgoCtx, pStrength);
#else
    if (0 != memcmp(&mCurStrength, pStrength, sizeof(mCurStrength))) {
        mNewStrength   = *pStrength;
        updateStrength = true;
        waitSignal(pStrength->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrV24HandleInt::getStrength(rk_aiq_ynr_strength_v24_t* pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_aynrV24_GetLumaSFStrength(mAlgoCtx, pStrength);
    mCfgMutex.unlock();
#else
    if (pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.unlock();
        rk_aiq_uapi_aynrV24_GetLumaSFStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateStrength) {
            *pStrength           = mNewStrength;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_aynrV24_GetLumaSFStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrV24HandleInt::getInfo(rk_aiq_ynr_info_v24_t* pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.unlock();
        rk_aiq_uapi_aynrV24_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_aynrV24_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrV24HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aynr handle prepare failed");

    RkAiqAlgoConfigAynrV24* aynr_config_int     = (RkAiqAlgoConfigAynrV24*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    aynr_config_int->stAynrConfig.rawWidth  = sharedCom->snsDes.isp_acq_width;
    aynr_config_int->stAynrConfig.rawHeight = sharedCom->snsDes.isp_acq_height;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aynr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAynrV24HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAynrV24* aynr_pre_int        = (RkAiqAlgoPreAynrV24*)mPreInParam;
    RkAiqAlgoPreResAynrV24* aynr_pre_res_int = (RkAiqAlgoPreResAynrV24*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aynr handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAynrV24HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAynrV24* aynr_proc_int = (RkAiqAlgoProcAynrV24*)mProcInParam;
    if (mDes->id == 0) {
        mProcResShared = new RkAiqAlgoProcResAynrV24IntShared();
        if (!mProcResShared.ptr()) {
            LOGE("new YNR(v24) mProcResShared failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aynr handle processing failed");
    }

    // TODO: fill procParam
    aynr_proc_int->iso                = sharedCom->iso;
    aynr_proc_int->hdr_mode           = sharedCom->working_mode;
#ifndef USE_NEWSTRUCT
    aynr_proc_int->stAblcV32_proc_res = shared->res_comb.ablcV32_proc_res;
#endif

    mProcResShared->result.stAynrProcResult.stFix = &shared->fullParams->mYnrParams->data()->result;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)(&mProcResShared->result));
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    if (!mProcResShared->result.res_com.cfg_update) {
        mProcResShared->result.stAynrProcResult = mLatestparam;
        LOGD_ANR("[%d] copy results from latest !", shared->frameId);
    }
    if (!mAiqCore->mAlogsComSharedParams.init && mPostShared) {
        mProcResShared->set_sequence(shared->frameId);
        RkAiqCoreVdBufMsg msg(XCAM_MESSAGE_YNR_V24_PROC_RES_OK, shared->frameId, mProcResShared);
        mAiqCore->post_message(msg);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrV24HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAynrV24* aynr_post_int        = (RkAiqAlgoPostAynrV24*)mPostInParam;
    RkAiqAlgoPostResAynrV24* aynr_post_res_int = (RkAiqAlgoPostResAynrV24*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aynr handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAynrV24HandleInt::genIspResult(RkAiqFullParams* params,
                                               RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    if (!mProcResShared.ptr()) return XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAynrV24* aynr_rk = (RkAiqAlgoProcResAynrV24*)&mProcResShared->result;

    if (!aynr_rk) {
        LOGD_ANALYZER("no aynr result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_ynr_params_t* ynr_param = params->mYnrParams->data().ptr();
        if (sharedCom->init) {
            ynr_param->frame_id = 0;
        } else {
            ynr_param->frame_id = shared->frameId;
        }
        if (aynr_rk->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            ynr_param->sync_flag = mSyncFlag;
            // copy from algo result
            cur_params->mYnrParams = params->mYnrParams;
            mLatestparam = aynr_rk->stAynrProcResult;
            ynr_param->is_update = true;
            LOGD_ANR("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != ynr_param->sync_flag) {
            ynr_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mYnrParams.ptr()) {
                ynr_param->result = cur_params->mYnrParams->data()->result;
                ynr_param->is_update = true;
            } else {
                LOGE_ANR("no latest params !");
                ynr_param->is_update = false;
            }
            LOGD_ANR("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            ynr_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }


    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
