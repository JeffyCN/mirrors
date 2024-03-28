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
#include "RkAiqAyuvmeV1Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAyuvmeV1HandleInt);

void RkAiqAyuvmeV1HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAyuvmeV1());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAyuvmeV1());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAyuvmeV1());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAyuvmeV1HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_ayuvmeV1_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateStrength) {
        mCurStrength   = mNewStrength;
        rk_aiq_uapi_ayuvmeV1_SetLumaSFStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAyuvmeV1HandleInt::setAttrib(const rk_aiq_yuvme_attrib_v1_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_ayuvmeV1_SetAttrib(mAlgoCtx, att, false);
#else
    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_yuvme_attrib_v1_t))) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);

    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAyuvmeV1HandleInt::getAttrib(rk_aiq_yuvme_attrib_v1_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_ayuvmeV1_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ayuvmeV1_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_ayuvmeV1_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqAyuvmeV1HandleInt::setStrength(const rk_aiq_yuvme_strength_v1_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_ayuvmeV1_SetLumaSFStrength(mAlgoCtx, pStrength);
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

XCamReturn RkAiqAyuvmeV1HandleInt::getStrength(rk_aiq_yuvme_strength_v1_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_ayuvmeV1_GetLumaSFStrength(mAlgoCtx, pStrength);
    mCfgMutex.unlock();
#else
    if(pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.unlock();
        rk_aiq_uapi_ayuvmeV1_GetLumaSFStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateStrength) {
            *pStrength = mNewStrength;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_ayuvmeV1_GetLumaSFStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqAyuvmeV1HandleInt::getInfo(rk_aiq_yuvme_info_v1_t *pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.unlock();
        rk_aiq_uapi_ayuvmeV1_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_ayuvmeV1_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAyuvmeV1HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ayuvme handle prepare failed");

    RkAiqAlgoConfigAyuvmeV1* ayuvme_config_int   = (RkAiqAlgoConfigAyuvmeV1*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ayuvme_config_int->stAyuvmeConfig.rawWidth  = sharedCom->snsDes.isp_acq_width;
    ayuvme_config_int->stAyuvmeConfig.rawHeight = sharedCom->snsDes.isp_acq_height;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ayuvme algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAyuvmeV1HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAyuvmeV1* ayuvme_pre_int        = (RkAiqAlgoPreAyuvmeV1*)mPreInParam;
    RkAiqAlgoPreResAyuvmeV1* ayuvme_pre_res_int = (RkAiqAlgoPreResAyuvmeV1*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ayuvme handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "ayuvme algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAyuvmeV1HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAyuvmeV1* ayuvme_proc_int        = (RkAiqAlgoProcAyuvmeV1*)mProcInParam;
    RkAiqAlgoProcResAyuvmeV1* ayuvme_proc_res_int = (RkAiqAlgoProcResAyuvmeV1*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ayuvme handle processing failed");
    }

    // TODO: fill procParam
    ayuvme_proc_int->iso      = sharedCom->iso;
    ayuvme_proc_int->hdr_mode = sharedCom->working_mode;
#ifndef USE_NEWSTRUCT
    ayuvme_proc_int->stAblcV32_proc_res = shared->res_comb.ablcV32_proc_res;
#else
    ayuvme_proc_int->stAblcV32_proc_res = NULL;
#endif
	ayuvme_proc_int->bayertnr_en = shared->res_comb.bayernr3d_en;

    ayuvme_proc_res_int->stAyuvmeProcResult.stFix = &shared->fullParams->mYuvmeParams->data()->result;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "ayuvme algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAyuvmeV1HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAyuvmeV1* ayuvme_post_int        = (RkAiqAlgoPostAyuvmeV1*)mPostInParam;
    RkAiqAlgoPostResAyuvmeV1* ayuvme_post_res_int = (RkAiqAlgoPostResAyuvmeV1*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ayuvme handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "ayuvme algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAyuvmeV1HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAyuvmeV1* ayuvme_rk = (RkAiqAlgoProcResAyuvmeV1*)mProcOutParam;

    if (!ayuvme_rk) {
        LOGD_ANALYZER("no ayuvme result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_yuvme_params_t* yuvme_param = params->mYuvmeParams->data().ptr();
        if (sharedCom->init) {
            yuvme_param->frame_id = 0;
        } else {
            yuvme_param->frame_id = shared->frameId;
        }
        if (ayuvme_rk->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            yuvme_param->sync_flag = mSyncFlag;
            // copy from algo result
            cur_params->mYuvmeParams = params->mYuvmeParams;
            yuvme_param->is_update = true;
            LOGD_ANR("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != yuvme_param->sync_flag) {
            yuvme_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mYuvmeParams.ptr()) {
                yuvme_param->result = cur_params->mYuvmeParams->data()->result;
                yuvme_param->is_update = true;
            } else {
                LOGE_ANR("no latest params !");
                yuvme_param->is_update = false;
            }
            LOGD_ANR("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            yuvme_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
