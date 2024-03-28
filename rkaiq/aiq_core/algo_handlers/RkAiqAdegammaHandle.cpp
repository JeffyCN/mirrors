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
#include "RkAiqAdegammaHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAdegammaHandleInt);

void RkAiqAdegammaHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdegamma());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdegamma());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdegamma());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdegammaHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_adegamma_SetAttrib(mAlgoCtx, mCurAtt, false);
        waitSignal();
    }

    if (needSync) mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandleInt::setAttrib(rk_aiq_degamma_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adegamma_SetAttrib(mAlgoCtx, att, false);
#else
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_degamma_attrib_t))) {
        mNewAtt   = att;
        updateAtt = true;
        sendSignal();
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandleInt::getAttrib(rk_aiq_degamma_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_adegamma_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adegamma handle prepare failed");

    RkAiqAlgoConfigAdegamma* adegamma_config_int = (RkAiqAlgoConfigAdegamma*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom  = &mAiqCore->mAlogsComSharedParams;

#ifdef RKAIQ_ENABLE_PARSER_V1
    adegamma_config_int->calib = sharedCom->calib;
#endif

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adegamma algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdegammaHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAdegamma* adegamma_pre_int        = (RkAiqAlgoPreAdegamma*)mPreInParam;
    RkAiqAlgoPreResAdegamma* adegamma_pre_res_int = (RkAiqAlgoPreResAdegamma*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adegamma handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adegamma algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdegammaHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdegamma* adegamma_proc_int        = (RkAiqAlgoProcAdegamma*)mProcInParam;
    RkAiqAlgoProcResAdegamma* adegamma_proc_res_int = (RkAiqAlgoProcResAdegamma*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    adegamma_proc_res_int->adegamma_proc_res = &shared->fullParams->mAdegammaParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adegamma handle processing failed");
    }

#ifdef RKAIQ_ENABLE_PARSER_V1
    adegamma_proc_int->calib = sharedCom->calib;
#endif

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "adegamma algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAdegamma* adegamma_post_int        = (RkAiqAlgoPostAdegamma*)mPostInParam;
    RkAiqAlgoPostResAdegamma* adegamma_post_res_int = (RkAiqAlgoPostResAdegamma*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adegamma handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "agamma algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAdegammaHandleInt::genIspResult(RkAiqFullParams* params,
                                                RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom     = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAdegamma* adegamma_com          = (RkAiqAlgoProcResAdegamma*)mProcOutParam;
    rk_aiq_isp_adegamma_params_t* degamma_param = params->mAdegammaParams->data().ptr();

    if (!adegamma_com) {
        LOGD_ANALYZER("no adegamma result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAdegamma* adegamma_rk = (RkAiqAlgoProcResAdegamma*)adegamma_com;
        if (sharedCom->init) {
            degamma_param->frame_id = 0;
        } else {
            degamma_param->frame_id = shared->frameId;
        }

        if (adegamma_com->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            degamma_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mAdegammaParams= params->mAdegammaParams;
            degamma_param->is_update = true;
            LOGD_ADEGAMMA("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != degamma_param->sync_flag) {
            degamma_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mAdegammaParams.ptr()) {
                degamma_param->result = cur_params->mAdegammaParams->data()->result;
                degamma_param->is_update = true;
            } else {
                LOGE_ADEGAMMA("no latest params !");
                degamma_param->is_update = false;
            }
            LOGD_ADEGAMMA("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            degamma_param->is_update = false;
            LOGD_ADEGAMMA("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
