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
#include "RkAiqAynrHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAynrHandleInt);

void RkAiqAynrHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAynr());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAynr());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAynr());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAynr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAynr());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAynr());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAynr());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAynrHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        updateAtt = false;
        // TODO
        rk_aiq_uapi_aynr_SetAttrib_v1(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if (updateIQpara) {
        mCurIQPara   = mNewIQPara;
        updateIQpara = false;
        // TODO
        rk_aiq_uapi_aynr_SetIQPara_v1(mAlgoCtx, &mCurIQPara, false);
        sendSignal();
    }

    if (updateJsonpara) {
        mCurJsonPara   = mNewJsonPara;
        updateJsonpara = false;
        // TODO
        rk_aiq_uapi_aynr_SetJsonPara_v1(mAlgoCtx, &mCurJsonPara, false);
        sendSignal();
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::setAttrib(rk_aiq_ynr_attrib_v1_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_ynr_attrib_v1_t))) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::getAttrib(rk_aiq_ynr_attrib_v1_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_GetAttrib_v1(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::setIQPara(rk_aiq_ynr_IQPara_V1_t* para) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_ynr_IQPara_V1_t))) {
        mNewIQPara   = *para;
        updateIQpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::getIQPara(rk_aiq_ynr_IQPara_V1_t* para) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_GetIQPara_v1(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::setJsonPara(rk_aiq_ynr_JsonPara_V1_t* para) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurIQPara, para, sizeof(rk_aiq_ynr_JsonPara_V1_t))) {
        mNewJsonPara   = *para;
        updateJsonpara = true;
        waitSignal();
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::getJsonPara(rk_aiq_ynr_JsonPara_V1_t* para) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_GetJsonPara_v1(mAlgoCtx, para);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::setStrength(float fPercent) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_SetLumaSFStrength_v1(mAlgoCtx, fPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::getStrength(float* pPercent) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_aynr_GetLumaSFStrength_v1(mAlgoCtx, pPercent);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aynr handle prepare failed");

    RkAiqAlgoConfigAynr* aynr_config_int = (RkAiqAlgoConfigAynr*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aynr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAynrHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAynr* aynr_pre_int        = (RkAiqAlgoPreAynr*)mPreInParam;
    RkAiqAlgoPreResAynr* aynr_pre_res_int = (RkAiqAlgoPreResAynr*)mPreOutParam;

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
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAynrHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAynr* aynr_proc_int        = (RkAiqAlgoProcAynr*)mProcInParam;
    RkAiqAlgoProcResAynr* aynr_proc_res_int = (RkAiqAlgoProcResAynr*)mProcOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aynr handle processing failed");
    }

    // TODO: fill procParam
    aynr_proc_int->iso      = sharedCom->iso;
    aynr_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAynrHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAynr* aynr_post_int        = (RkAiqAlgoPostAynr*)mPostInParam;
    RkAiqAlgoPostResAynr* aynr_post_res_int = (RkAiqAlgoPostResAynr*)mPostOutParam;

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
    return ret;
}

XCamReturn RkAiqAynrHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAynr* aynr_com              = (RkAiqAlgoProcResAynr*)mProcOutParam;

    if (!aynr_com) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAynr* aynr_rk = (RkAiqAlgoProcResAynr*)aynr_com;

        if (params->mYnrParams.ptr()) {
            rk_aiq_isp_ynr_params_v20_t* ynr_param = params->mYnrParams->data().ptr();
            LOGD_ANR("oyyf: %s:%d output ispp param start\n", __FUNCTION__, __LINE__);

            if (sharedCom->init) {
                ynr_param->frame_id = 0;
            } else {
                ynr_param->frame_id = shared->frameId;
            }
            memcpy(&ynr_param->result, &aynr_rk->stAynrProcResult.stFix, sizeof(RK_YNR_Fix_V1_t));
        }
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    cur_params->mYnrParams = params->mYnrParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
