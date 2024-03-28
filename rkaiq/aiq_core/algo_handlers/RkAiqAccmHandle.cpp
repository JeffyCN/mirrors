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
#include "RkAiqAccmHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAccmHandleInt);

void RkAiqAccmHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAccm());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAccm());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAccm());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAccmHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_CCM_V1
        mCurAtt   = mNewAtt;
        // TODO
        rk_aiq_uapi_accm_SetAttrib(mAlgoCtx, &mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
#endif
#if RKAIQ_HAVE_CCM_V2
        mCurAttV2   = mNewAttV2;
        // TODO
        rk_aiq_uapi_accm_v2_SetAttrib(mAlgoCtx, &mCurAttV2, false);
        updateAtt = false;
        sendSignal(mCurAttV2.sync.sync_mode);
#endif
#if RKAIQ_HAVE_CCM_V3
        mCurAttV3   = mNewAttV3;
        // TODO
        rk_aiq_uapi_accm_v3_SetAttrib(mAlgoCtx, &mCurAttV3, false);
        updateAtt = false;
        sendSignal(mCurAttV3.sync.sync_mode);
#endif
    }
    if (updateCalibAttr) {
#if RKAIQ_HAVE_CCM_V1
        mCurCalibAtt   = mNewCalibAtt;
        // TODO
        rk_aiq_uapi_accm_SetIqParam(mAlgoCtx, &mCurCalibAtt, false);
        updateCalibAttr = false;
        sendSignal(mCurCalibAtt.sync.sync_mode);
#endif
#if RKAIQ_HAVE_CCM_V2
        mCurCalibAttV2   = mNewCalibAttV2;
        // TODO
        rk_aiq_uapi_accm_SetIqParam(mAlgoCtx, &mCurCalibAttV2, false);
        updateCalibAttr = false;
        sendSignal(mCurCalibAttV2.sync.sync_mode);
#endif
#if RKAIQ_HAVE_CCM_V3
        mCurCalibAttV3   = mNewCalibAttV3;
        // TODO
        rk_aiq_uapi_accm_SetIqParam(mAlgoCtx, &mCurCalibAttV3, false);
        updateCalibAttr = false;
        sendSignal(mCurCalibAttV3.sync.sync_mode);
#endif
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if RKAIQ_HAVE_CCM_V1
XCamReturn RkAiqAccmHandleInt::setAttrib(const rk_aiq_ccm_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_accm_SetAttrib(mAlgoCtx, att, false);
#else

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
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::getAttrib(rk_aiq_ccm_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_accm_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::setIqParam(const rk_aiq_ccm_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_accm_SetIqParam(mAlgoCtx, att, false);
#else

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewCalibAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurCalibAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewCalibAtt   = *att;
        updateCalibAttr = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::getIqParam(rk_aiq_ccm_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateCalibAttr) {
            memcpy(att, &mNewCalibAtt, sizeof(mNewCalibAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
            att->sync.sync_mode = mNewCalibAtt.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

#if RKAIQ_HAVE_CCM_V2
XCamReturn RkAiqAccmHandleInt::setAttribV2(const rk_aiq_ccm_v2_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_accm_v2_SetAttrib(mAlgoCtx, att, false);
#else
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
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::getAttribV2(rk_aiq_ccm_v2_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_accm_v2_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_v2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV2, sizeof(mNewAttV2));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_v2_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV2.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::setIqParamV2(const rk_aiq_ccm_v2_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_accm_SetIqParam(mAlgoCtx, att, false);
#else

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewCalibAttV2, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurCalibAttV2, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewCalibAttV2   = *att;
        updateCalibAttr = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::getIqParamV2(rk_aiq_ccm_v2_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
    mCfgMutex.unlock();
#else

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (mNewCalibAttV2) {
            memcpy(att, &mNewCalibAttV2, sizeof(mNewCalibAttV2));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
            att->sync.sync_mode = mNewCalibAttV2.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_CCM_V3
XCamReturn RkAiqAccmHandleInt::setAttribV3(const rk_aiq_ccm_v3_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_accm_v3_SetAttrib(mAlgoCtx, att, false);
#else
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAttV3, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAttV3, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV3   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::getAttribV3(rk_aiq_ccm_v3_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_accm_v3_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_v3_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV3, sizeof(mNewAttV3));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_v3_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV3.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::setIqParamV3(const rk_aiq_ccm_v3_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_accm_SetIqParam(mAlgoCtx, att, false);
#else

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewCalibAttV3, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurCalibAttV3, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewCalibAttV3   = *att;
        updateCalibAttr = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::getIqParamV3(rk_aiq_ccm_v3_calib_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
    mCfgMutex.unlock();
#else

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (mNewCalibAttV3) {
            memcpy(att, &mNewCalibAttV3, sizeof(mNewCalibAttV3));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_GetIqParam(mAlgoCtx, att);
            att->sync.sync_mode = mNewCalibAttV3.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqAccmHandleInt::queryCcmInfo(rk_aiq_ccm_querry_info_t* ccm_querry_info) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(ccm_querry_info != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_accm_QueryCcmInfo(mAlgoCtx, ccm_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAccmHandleInt::getAcolorSwInfo(rk_aiq_color_info_t* aColor_sw_info) {
    ENTER_ANALYZER_FUNCTION();
    LOGV_ACCM("%s enter",__FUNCTION__);
    XCAM_ASSERT(aColor_sw_info != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAccm* accm_proc_int        = (RkAiqAlgoProcAccm*)mProcInParam;
    memcpy(aColor_sw_info->awbGain,accm_proc_int->accm_sw_info.awbGain,sizeof(aColor_sw_info->awbGain));
    aColor_sw_info->sensorGain = accm_proc_int->accm_sw_info.sensorGain;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::setAcolorSwInfo(rk_aiq_color_info_t aColor_sw_info) {
    ENTER_ANALYZER_FUNCTION();


    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGV_ACCM("%s sensor gain = %f, wbgain=[%f,%f] ",__FUNCTION__,aColor_sw_info.sensorGain,
      aColor_sw_info.awbGain[0],aColor_sw_info.awbGain[1]);
    colorSwInfo = aColor_sw_info;
    colorConstFlag=true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "accm handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "accm algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAccmHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAccm* accm_pre_int        = (RkAiqAlgoPreAccm*)mPreInParam;
    RkAiqAlgoPreResAccm* accm_pre_res_int = (RkAiqAlgoPreResAccm*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "accm handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "accm algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAccmHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAccm* accm_proc_int        = (RkAiqAlgoProcAccm*)mProcInParam;
    RkAiqAlgoProcResAccm* accm_proc_res_int = (RkAiqAlgoProcResAccm*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (!shared->fullParams || !shared->fullParams->mCcmParams.ptr()) {
        LOGE_ACCM("[%d]: no ccm buf !", shared->frameId);
        return XCAM_RETURN_BYPASS;
    }

#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    accm_proc_res_int->accm_hw_conf_v2 = &shared->fullParams->mCcmParams->data()->result;
#else
    accm_proc_res_int->accm_hw_conf = &shared->fullParams->mCcmParams->data()->result;
#endif

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "accm handle processing failed");
    }

    // TODO should check if the rk awb algo used
    XCamVideoBuffer* xCamAwbProcRes = shared->res_comb.awb_proc_res;
    if (xCamAwbProcRes) {
        RkAiqAlgoProcResAwbShared_t* awb_res =
            (RkAiqAlgoProcResAwbShared_t*)xCamAwbProcRes->map(xCamAwbProcRes);
        if (awb_res) {
            if (awb_res->awb_gain_algo.grgain < DIVMIN || awb_res->awb_gain_algo.gbgain < DIVMIN) {
                LOGW("get wrong awb gain from AWB module ,use default value ");
            } else {
                accm_proc_int->accm_sw_info.awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                accm_proc_int->accm_sw_info.awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            accm_proc_int->accm_sw_info.awbIIRDampCoef = awb_res->awb_smooth_factor;
            accm_proc_int->accm_sw_info.varianceLuma   = awb_res->varianceLuma;
            accm_proc_int->accm_sw_info.awbConverged   = awb_res->awbConverged;
        } else {
            LOGW("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGW("fail to get awb gain form AWB module,use default value ");
    }
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    if (pCurExp) {
        if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            accm_proc_int->accm_sw_info.sensorGain =
                pCurExp->LinearExp.exp_real_params.analog_gain *
                pCurExp->LinearExp.exp_real_params.digital_gain *
                pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from second hdr frame for accm");
            accm_proc_int->accm_sw_info.sensorGain =
                pCurExp->HdrExp[1].exp_real_params.analog_gain *
                pCurExp->HdrExp[1].exp_real_params.digital_gain *
                pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from third hdr frame for accm");
            accm_proc_int->accm_sw_info.sensorGain =
                pCurExp->HdrExp[2].exp_real_params.analog_gain *
                pCurExp->HdrExp[2].exp_real_params.digital_gain *
                pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGE(
                "working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default "
                "value ",
                sharedCom->working_mode);
        }
    } else {
        LOGE("fail to get sensor gain form AE module,use default value ");
    }

#if RKAIQ_HAVE_BLC_V32 && !USE_NEWSTRUCT
    if (shared->res_comb.ablcV32_proc_res->blc_ob_enable) {
        if (shared->res_comb.ablcV32_proc_res->isp_ob_predgain >= 1.0f) {
            accm_proc_int->accm_sw_info.sensorGain *=  shared->res_comb.ablcV32_proc_res->isp_ob_predgain;
        }
    }
#endif
    if(colorConstFlag==true){
        memcpy(accm_proc_int->accm_sw_info.awbGain,colorSwInfo.awbGain,sizeof(colorSwInfo.awbGain));
        accm_proc_int->accm_sw_info.sensorGain = colorSwInfo.sensorGain;
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "accm algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if 0
    RkAiqAlgoPostAccm* accm_post_int        = (RkAiqAlgoPostAccm*)mPostInParam;
    RkAiqAlgoPostResAccm* accm_post_res_int = (RkAiqAlgoPostResAccm*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "accm handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "accm algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAccmHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAccm* accm_com = (RkAiqAlgoProcResAccm*)mProcOutParam;
    rk_aiq_isp_ccm_params_t* ccm_param = params->mCcmParams->data().ptr();

    if (!accm_com) {
        LOGD_ANALYZER("no accm result");
        return XCAM_RETURN_NO_ERROR;
    }

    RkAiqAlgoProcResAccm* accm_rk = (RkAiqAlgoProcResAccm*)accm_com;

    if (sharedCom->init) {
        ccm_param->frame_id = 0;
    } else {
        ccm_param->frame_id = shared->frameId;
    }

#if 0//moved to processing out params
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    ccm_param->result = accm_rk->accm_hw_conf_v2;
#else
    ccm_param->result = accm_rk->accm_hw_conf;
#endif
#endif

    if (accm_com->res_com.cfg_update) {
        mSyncFlag = shared->frameId;
        ccm_param->sync_flag = mSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mCcmParams = params->mCcmParams;
        ccm_param->is_update = true;
        LOGD_ACCM("[%d] params from algo", mSyncFlag);
    } else if (mSyncFlag != ccm_param->sync_flag) {
        ccm_param->sync_flag = mSyncFlag;
        // copy from latest result
        if (cur_params->mCcmParams.ptr()) {
            ccm_param->result = cur_params->mCcmParams->data()->result;
            ccm_param->is_update = true;
        } else {
            LOGE_ACCM("no latest params !");
            ccm_param->is_update = false;
        }
        LOGD_ACCM("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        ccm_param->is_update = false;
        LOGD_ACCM("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
