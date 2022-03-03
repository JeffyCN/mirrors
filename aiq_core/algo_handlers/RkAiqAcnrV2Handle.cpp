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
#include "RkAiqAcnrV2Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAcnrV2HandleInt);

void RkAiqAcnrV2HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcnrV2());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAcnrV2());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAcnrV2());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcnrV2());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcnrV2());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAcnrV2());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAcnrV2());


    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAcnrV2HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_acnrV2_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateStrength) {
        mCurStrength   = mNewStrength;
        rk_aiq_uapi_acnrV2_SetChromaSFStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcnrV2HandleInt::setAttrib(rk_aiq_cnr_attrib_v2_t* att) {
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

XCamReturn RkAiqAcnrV2HandleInt::getAttrib(rk_aiq_cnr_attrib_v2_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_acnrV2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_acnrV2_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcnrV2HandleInt::setStrength(rk_aiq_cnr_strength_v2_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    bool isChanged = false;
    if (pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewStrength, pStrength, sizeof(*pStrength)))
        isChanged = true;
    else if (pStrength->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurStrength, pStrength, sizeof(*pStrength)))
        isChanged = true;

    if (isChanged) {
        mNewStrength   = *pStrength;
        updateStrength = true;
        waitSignal(pStrength->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn RkAiqAcnrV2HandleInt::getStrength(rk_aiq_cnr_strength_v2_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;


    if(pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_acnrV2_GetChromaSFStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateStrength) {
            *pStrength = mNewStrength;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_acnrV2_GetChromaSFStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcnrV2HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acnr handle prepare failed");

    RkAiqAlgoConfigAcnrV1* acnr_config_int = (RkAiqAlgoConfigAcnrV1*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcnrV2HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAcnrV2* acnr_pre_int        = (RkAiqAlgoPreAcnrV2*)mPreInParam;
    RkAiqAlgoPreResAcnrV2* acnr_pre_res_int = (RkAiqAlgoPreResAcnrV2*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acnr handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "acnr algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcnrV2HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcnrV2* acnr_proc_int        = (RkAiqAlgoProcAcnrV2*)mProcInParam;
    RkAiqAlgoProcResAcnrV2* acnr_proc_res_int = (RkAiqAlgoProcResAcnrV2*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acnr handle processing failed");
    }

    // TODO: fill procParam
    acnr_proc_int->iso      = sharedCom->iso;
    acnr_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcnrV2HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAcnrV2* acnr_post_int        = (RkAiqAlgoPostAcnrV2*)mPostInParam;
    RkAiqAlgoPostResAcnrV2* acnr_post_res_int = (RkAiqAlgoPostResAcnrV2*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acnr handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "acnr algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcnrV2HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAcnrV2* acnr_rk = (RkAiqAlgoProcResAcnrV2*)mProcOutParam;

    if (!acnr_rk) {
        LOGD_ANALYZER("no aynr result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_cnr_params_v3x_t* cnr_param = params->mCnrV3xParams->data().ptr();
        if (sharedCom->init) {
            cnr_param->frame_id = 0;
        } else {
            cnr_param->frame_id = shared->frameId;
        }

        memcpy(&cnr_param->result, &acnr_rk->stAcnrProcResult.stFix, sizeof(RK_CNR_Fix_V2_t));
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    cur_params->mCnrV3xParams = params->mCnrV3xParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
