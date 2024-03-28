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
#include "RkAiqAgicHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAgicHandleInt);

void RkAiqAgicHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgic());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgic());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgic());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAgicHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAttV1) {
        mCurAttV1   = mNewAttV1;
        rk_aiq_uapi_agic_v1_SetAttrib(mAlgoCtx, &mCurAttV1, false);
        updateAttV1 = false;
        sendSignal(mCurAttV1.sync.sync_mode);
    }
    if (updateAttV2) {
        mCurAttV2   = mNewAttV2;
        rk_aiq_uapi_agic_v2_SetAttrib(mAlgoCtx, &mCurAttV2, false);
        updateAttV2 = false;
        sendSignal(mCurAttV2.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandleInt::setAttribV1(const rkaiq_gic_v1_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_agic_v1_SetAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAttV1, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV1, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV1   = *att;
        updateAttV1 = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandleInt::getAttribV1(rkaiq_gic_v1_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_agic_v1_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_agic_v1_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAttV1) {
            memcpy(att, &mNewAttV1, sizeof(mNewAttV1));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_agic_v1_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV1.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandleInt::setAttribV2(const rkaiq_gic_v2_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_agic_v2_SetAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAttV2, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV2, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV2   = *att;
        updateAttV2 = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandleInt::getAttribV2(rkaiq_gic_v2_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_agic_v2_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_agic_v2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAttV2) {
            memcpy(att, &mNewAttV2, sizeof(mNewAttV2));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_agic_v2_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV2.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "agic handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "agic algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAgicHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAgic* agic_pre_int        = (RkAiqAlgoPreAgic*)mPreInParam;
    RkAiqAlgoPreResAgic* agic_pre_res_int = (RkAiqAlgoPreResAgic*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "agic handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "agic algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAgicHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAgic* agic_proc_int        = (RkAiqAlgoProcAgic*)mProcInParam;
    RkAiqAlgoProcResAgic* agic_proc_res_int = (RkAiqAlgoProcResAgic*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (!shared->fullParams || !shared->fullParams->mGicParams.ptr()) {
        LOGE_ALSC("[%d]: no gic buf !", shared->frameId);
        return XCAM_RETURN_BYPASS;
    }

    agic_proc_res_int->gicRes = &shared->fullParams->mGicParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "agic handle processing failed");
    }

    agic_proc_int->hdr_mode = sharedCom->working_mode;
    switch (sharedCom->snsDes.sensor_pixelformat) {
        case V4L2_PIX_FMT_SBGGR14:
        case V4L2_PIX_FMT_SGBRG14:
        case V4L2_PIX_FMT_SGRBG14:
        case V4L2_PIX_FMT_SRGGB14:
            agic_proc_int->raw_bits = 14;
            break;
        case V4L2_PIX_FMT_SBGGR12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SRGGB12:
            agic_proc_int->raw_bits = 12;
            break;
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SRGGB10:
            agic_proc_int->raw_bits = 10;
            break;
        default:
            agic_proc_int->raw_bits = 8;
    }

    RKAiqAecExpInfo_t* aeCurExp = &shared->curExp;
    if (aeCurExp != NULL) {
        if((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            agic_proc_int->iso = aeCurExp->LinearExp.exp_real_params.analog_gain * aeCurExp->LinearExp.exp_real_params.isp_dgain * 50;
            LOGD_AGIC("%s:NORMAL:iso=%d,again=%f\n", __FUNCTION__, agic_proc_int->iso,
                      aeCurExp->LinearExp.exp_real_params.analog_gain);
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            agic_proc_int->iso = aeCurExp->HdrExp[1].exp_real_params.analog_gain * 50;
            LOGD_AGIC("%s:HDR2:iso=%d,again=%f\n", __FUNCTION__, agic_proc_int->iso,
                      aeCurExp->HdrExp[1].exp_real_params.analog_gain);
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_ISP_HDR3) {
            agic_proc_int->iso = aeCurExp->HdrExp[2].exp_real_params.analog_gain * 50;
            LOGD_AGIC("%s:HDR3:iso=%d,again=%f\n", __FUNCTION__, agic_proc_int->iso,
                      aeCurExp->HdrExp[2].exp_real_params.analog_gain);
        }
    } else {
        agic_proc_int->iso = 50;
        LOGE_AGIC("%s: pAEPreRes is NULL, so use default instead \n", __FUNCTION__);
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "agic algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgicHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAgic* agic_post_int        = (RkAiqAlgoPostAgic*)mPostInParam;
    RkAiqAlgoPostResAgic* agic_post_res_int = (RkAiqAlgoPostResAgic*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "agic handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "agic algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAgicHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAgic* agic_com              = (RkAiqAlgoProcResAgic*)mProcOutParam;
    rk_aiq_isp_gic_params_t* gic_param = params->mGicParams->data().ptr();

    if (!agic_com) {
        LOGD_ANALYZER("no agic result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAgic* agic_rk = (RkAiqAlgoProcResAgic*)agic_com;
        if (sharedCom->init) {
            gic_param->frame_id = 0;
        } else {
            gic_param->frame_id = shared->frameId;
        }

        if (agic_com->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            gic_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mGicParams = params->mGicParams;
            gic_param->is_update = true;
            LOGD_AGIC("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != gic_param->sync_flag) {
            gic_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mGicParams.ptr()) {
                gic_param->result = cur_params->mGicParams->data()->result;
                gic_param->is_update = true;
            } else {
                LOGE_AGIC("no latest params !");
                gic_param->is_update = false;
            }
            LOGD_AGIC("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            gic_param->is_update = false;
            LOGD_AGIC("[%d] params needn't update", shared->frameId);
        }
#if 0//moved to processing out params
        memcpy(&gic_param->result, &agic_rk->gicRes, sizeof(AgicProcResult_t));
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
