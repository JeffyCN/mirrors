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
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdegamma());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdegamma());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdegamma());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdegamma());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdegamma());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdegamma());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdegammaHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandleInt::setAttrib(rk_aiq_degamma_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
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

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adegamma handle processing failed");
    }

#ifdef RKAIQ_ENABLE_PARSER_V1
    adegamma_proc_int->calib = sharedCom->calib;
#endif

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adegamma algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdegammaHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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
    rk_aiq_isp_adegamma_params_v20_t* degamma_param = params->mAdegammaParams->data().ptr();

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

        degamma_param->result.degamma_en   = adegamma_rk->adegamma_proc_res.degamma_en;
        degamma_param->result.degamma_X_d0 = adegamma_rk->adegamma_proc_res.degamma_X_d0;
        degamma_param->result.degamma_X_d1 = adegamma_rk->adegamma_proc_res.degamma_X_d1;
        for (int i = 0; i < DEGAMMA_CRUVE_Y_KNOTS; i++) {
            degamma_param->result.degamma_tableR[i] =
                adegamma_rk->adegamma_proc_res.degamma_tableR[i];
            degamma_param->result.degamma_tableG[i] =
                adegamma_rk->adegamma_proc_res.degamma_tableG[i];
            degamma_param->result.degamma_tableB[i] =
                adegamma_rk->adegamma_proc_res.degamma_tableB[i];
        }
    }

    cur_params->mAdegammaParams = params->mAdegammaParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
