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
#include "RkAiqAgainV2Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAgainV2HandleInt);

void RkAiqAgainV2HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgainV2());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAgainV2());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAgainV2());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgainV2());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgainV2());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAgainV2());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAgainV2());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAgainV2HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_againV2_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::setAttrib(rk_aiq_gain_attrib_v2_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::getAttrib(rk_aiq_gain_attrib_v2_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_againV2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_againV2_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqAgainV2HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "again handle prepare failed");

    RkAiqAlgoConfigAgainV2* again_config_int = (RkAiqAlgoConfigAgainV2*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "again algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAgainV2HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAgainV2* again_pre_int        = (RkAiqAlgoPreAgainV2*)mPreInParam;
    RkAiqAlgoPreResAgainV2* again_pre_res_int = (RkAiqAlgoPreResAgainV2*)mPreOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "again handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "again algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAgainV2HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAgainV2* again_proc_int        = (RkAiqAlgoProcAgainV2*)mProcInParam;
    RkAiqAlgoProcResAgainV2* again_proc_res_int = (RkAiqAlgoProcResAgainV2*)mProcOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    static int auvnr_proc_framecnt = 0;
    auvnr_proc_framecnt++;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "again handle processing failed");
    }

    // TODO: fill procParam
    again_proc_int->iso      = sharedCom->iso;
    again_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "again algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAgainV2* again_post_int        = (RkAiqAlgoPostAgainV2*)mPostInParam;
    RkAiqAlgoPostResAgainV2* again_post_res_int = (RkAiqAlgoPostResAgainV2*)mPostOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "auvnr handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "auvnr algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAgainV2* again_rk = (RkAiqAlgoProcResAgainV2*)mProcOutParam;

    if (!again_rk) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (params->mGainV3xParams.ptr()) {
            rk_aiq_isp_gain_params_v3x_t* gain_param = params->mGainV3xParams->data().ptr();
            if (sharedCom->init) {
                gain_param->frame_id = 0;
            } else {
                gain_param->frame_id = shared->frameId;
            }
            LOGD_ANR("oyyf: %s:%d output gain  param start\n", __FUNCTION__, __LINE__);

            memcpy(&gain_param->result, &again_rk->stAgainProcResult.stFix,
                   sizeof(RK_GAIN_Fix_V2_t));
            LOGD_ANR("oyyf: %s:%d output gain param end \n", __FUNCTION__, __LINE__);
        }
    }

    cur_params->mGainV3xParams = params->mGainV3xParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
