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
#include "RkAiqAsharpV4Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAsharpV4HandleInt);

void RkAiqAsharpV4HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAsharpV4());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAsharpV4());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAsharpV4());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAsharpV4());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAsharpV4());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAsharpV4());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAsharpV4());


    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAsharpV4HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_asharpV4_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateStrength) {
        mCurStrength   = mNewStrength;
        rk_aiq_uapi_asharpV4_SetStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV4HandleInt::setAttrib(rk_aiq_sharp_attrib_v4_t* att) {
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

XCamReturn RkAiqAsharpV4HandleInt::getAttrib(rk_aiq_sharp_attrib_v4_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_asharpV4_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_asharpV4_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV4HandleInt::setStrength(rk_aiq_sharp_strength_v4_t *pStrength) {
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

XCamReturn RkAiqAsharpV4HandleInt::getStrength(rk_aiq_sharp_strength_v4_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();


    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_asharpV4_GetStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateStrength) {
            *pStrength = mNewStrength;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_asharpV4_GetStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV4HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "asharp handle prepare failed");

    RkAiqAlgoConfigAsharpV4* asharp_config_int = (RkAiqAlgoConfigAsharpV4*)mConfig;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "asharp algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAsharpV4HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAsharpV4* asharp_pre_int        = (RkAiqAlgoPreAsharpV4*)mPreInParam;
    RkAiqAlgoPreResAsharpV4* asharp_pre_res_int = (RkAiqAlgoPreResAsharpV4*)mPreOutParam;
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
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAsharpV4HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAsharpV4* asharp_proc_int        = (RkAiqAlgoProcAsharpV4*)mProcInParam;
    RkAiqAlgoProcResAsharpV4* asharp_proc_res_int = (RkAiqAlgoProcResAsharpV4*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    static int asharp_proc_framecnt             = 0;
    asharp_proc_framecnt++;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "asharp handle processing failed");
    }

    // TODO: fill procParam
    asharp_proc_int->iso      = sharedCom->iso;
    asharp_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "asharp algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAsharpV4HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAsharpV4* asharp_post_int        = (RkAiqAlgoPostAsharpV4*)mPostInParam;
    RkAiqAlgoPostResAsharpV4* asharp_post_res_int = (RkAiqAlgoPostResAsharpV4*)mPostOutParam;
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
    return ret;
}

XCamReturn RkAiqAsharpV4HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAsharpV4* asharp_rk = (RkAiqAlgoProcResAsharpV4*)mProcOutParam;

    if (!asharp_rk) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_sharpen_params_v3x_t* sharp_param = params->mSharpenV3xParams->data().ptr();
        if (sharedCom->init) {
            sharp_param->frame_id = 0;
        } else {
            sharp_param->frame_id = shared->frameId;
        }
        memcpy(&sharp_param->result, &asharp_rk->stAsharpProcResult.stFix,
               sizeof(RK_SHARP_Fix_V4_t));
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    cur_params->mSharpenV3xParams = params->mSharpenV3xParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
