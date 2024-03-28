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
#include "RkAiqAdehazeHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAdehazeHandleInt);

void RkAiqAdehazeHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdhaz());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdhaz());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdhaz());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdehazeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adhaz handle prepare failed");

    RkAiqAlgoConfigAdhaz* adhaz_config_int      = (RkAiqAlgoConfigAdhaz*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

#ifdef RKAIQ_ENABLE_PARSER_V1
    adhaz_config_int->calib = sharedCom->calib;
#endif

    adhaz_config_int->working_mode      = sharedCom->working_mode;
    adhaz_config_int->is_multi_isp_mode = sharedCom->is_multi_isp_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdehazeHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAdhaz* adhaz_pre_int            = (RkAiqAlgoPreAdhaz*)mPreInParam;
    RkAiqAlgoPreResAdhaz* adhaz_pre_res_int     = (RkAiqAlgoPreResAdhaz*)mPreOutParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    adhaz_pre_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    adhaz_pre_int->rawWidth  = sharedCom->snsDes.isp_acq_width;

    RkAiqAdehazeStats* xDehazeStats = nullptr;
    if (shared->ispStats) {
        xDehazeStats = (RkAiqAdehazeStats*)shared->ispStats->map(shared->ispStats);
        if (!xDehazeStats) LOGE_ADEHAZE("isp stats is null");
    } else {
        LOGW_ADEHAZE("the xcamvideobuffer of isp stats is null");
    }

    if (!xDehazeStats || !xDehazeStats->adehaze_stats_valid || !sharedCom->init) {
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        LOGE("no adehaze stats, ignore!");
        return XCAM_RETURN_BYPASS;
#endif
    } else {
#if RKAIQ_HAVE_DEHAZE_V10
        memcpy(&adhaz_pre_int->stats.dehaze_stats_v10,
               &xDehazeStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v10,
               sizeof(dehaze_stats_v10_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11
        memcpy(&adhaz_pre_int->stats.dehaze_stats_v11,
               &xDehazeStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v11,
               sizeof(dehaze_stats_v11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
        memcpy(&adhaz_pre_int->stats.dehaze_stats_v11_duo,
               &xDehazeStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v11_duo,
               sizeof(dehaze_stats_v11_duo_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V12
        memcpy(&adhaz_pre_int->stats.dehaze_stats_v12,
               &xDehazeStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v12,
               sizeof(dehaze_stats_v12_t));
#endif
    }

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle preProcess failed");
    }

#ifdef RK_SIMULATOR_HW
    // nothing todo
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdehazeHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdhaz* adhaz_proc_int        = (RkAiqAlgoProcAdhaz*)mProcInParam;
    RkAiqAlgoProcResAdhaz* adhaz_proc_res_int = (RkAiqAlgoProcResAdhaz*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAdehazeStats* xDehazeStats = nullptr;
    if (shared->adehazeStatsBuf) {
        xDehazeStats = shared->adehazeStatsBuf;
        if (!xDehazeStats) LOGE_ADEHAZE("dehaze stats is null");
    } else {
        LOGW_ADEHAZE("the xcamvideobuffer of isp stats is null");
    }

    if (!xDehazeStats || !xDehazeStats->adehaze_stats_valid) {
        LOGW_ADEHAZE("no adehaze stats, ignore!");
        adhaz_proc_int->stats_true = false;
    } else {
        adhaz_proc_int->stats_true = true;

#if RKAIQ_HAVE_DEHAZE_V10
    adhaz_proc_int->dehaze_stats_v10 = &xDehazeStats->adehaze_stats.dehaze_stats_v10;
#endif
#if RKAIQ_HAVE_DEHAZE_V11
    adhaz_proc_int->dehaze_stats_v11 = &xDehazeStats->adehaze_stats.dehaze_stats_v11;
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
    adhaz_proc_int->dehaze_stats_v11_duo = &xDehazeStats->adehaze_stats.dehaze_stats_v11_duo;
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adhaz_proc_int->dehaze_stats_v12 = &xDehazeStats->adehaze_stats.dehaze_stats_v12;
#endif
#if RKAIQ_HAVE_DEHAZE_V14
    adhaz_proc_int->dehaze_stats_v14 = &xDehazeStats->adehaze_stats.dehaze_stats_v14;
#endif
    }
#if RKAIQ_HAVE_DEHAZE_V11_DUO
#if RKAIQ_HAVE_YNR_V3
    if (shared->res_comb.aynrV22_proc_res) {
        for (int i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++)
            adhaz_proc_int->sigma_v3[i] = shared->res_comb.aynrV3_proc_res->stSelect->sigma[i];
    }
#else
    for (int i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++) adhaz_proc_int->sigma_v3[i] = 0.0f;
#endif
#endif
#if RKAIQ_HAVE_DEHAZE_V12
#if RKAIQ_HAVE_YNR_V22
    if (shared->res_comb.aynrV22_proc_res) {
        for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++)
            adhaz_proc_int->sigma_v22[i] = shared->res_comb.aynrV22_proc_res->stSelect->sigma[i];
    }
#else
    for (int i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) adhaz_proc_int->sigma_v22[i] = 0.0f;
#endif
#if RKAIQ_HAVE_BLC_V32
    adhaz_proc_int->OBResV12.blc_ob_enable   = shared->res_comb.ablcV32_proc_res->blc_ob_enable;
    adhaz_proc_int->OBResV12.isp_ob_predgain = shared->res_comb.ablcV32_proc_res->isp_ob_predgain;
#else
    adhaz_proc_int->OBResV12.blc_ob_enable   = false;
    adhaz_proc_int->OBResV12.isp_ob_predgain = 1.0f;
#endif
#endif
#if RKAIQ_HAVE_DEHAZE_V14
#if RKAIQ_HAVE_YNR_V24
    if (shared->res_comb.aynrV24_proc_res) {
        for (int i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++)
            adhaz_proc_int->sigma_v24[i] = shared->res_comb.aynrV24_proc_res->stSelect->sigma[i];
    }
#else
    for (int i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++) adhaz_proc_int->sigma_v24[i] = 0.0f;
#endif
#if RKAIQ_HAVE_BLC_V32 && !USE_NEWSTRUCT
    adhaz_proc_int->OBResV12.blc_ob_enable   = shared->res_comb.ablcV32_proc_res->blc_ob_enable;
    adhaz_proc_int->OBResV12.isp_ob_predgain = shared->res_comb.ablcV32_proc_res->isp_ob_predgain;
#else
    adhaz_proc_int->OBResV12.blc_ob_enable   = false;
    adhaz_proc_int->OBResV12.isp_ob_predgain = 1.0f;
#endif
#endif

    adhaz_proc_res_int->AdehzeProcRes = &shared->fullParams->mDehazeParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle processing failed");
    }

#ifdef RKAIQ_ENABLE_PARSER_V1
    adhaz_proc_int->pCalibDehaze = sharedCom->calib;
#endif

#ifdef RK_SIMULATOR_HW
    // nothing todo
#endif
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "adhaz algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0

    RkAiqAlgoPostAdhaz* adhaz_post_int        = (RkAiqAlgoPostAdhaz*)mPostInParam;
    RkAiqAlgoPostResAdhaz* adhaz_post_res_int = (RkAiqAlgoPostResAdhaz*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_DEHAZE_V10
        mCurAttV10 = mNewAttV10;
        rk_aiq_uapi_adehaze_v10_SetAttrib(mAlgoCtx, &mCurAttV10, false);
        updateAtt = false;
        sendSignal(mCurAttV10.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
        mCurAttV11 = mNewAttV11;
        rk_aiq_uapi_adehaze_v11_SetAttrib(mAlgoCtx, &mCurAttV11, false);
        updateAtt = false;
        sendSignal(mCurAttV11.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DEHAZE_V12
        mCurAttV12 = mNewAttV12;
        rk_aiq_uapi_adehaze_v12_SetAttrib(mAlgoCtx, &mCurAttV12, false);
        updateAtt = false;
        sendSignal(mCurAttV12.sync.sync_mode);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
        mCurAttV14 = mNewAttV14;
        rk_aiq_uapi_adehaze_v14_SetAttrib(mAlgoCtx, &mCurAttV14, false);
        updateAtt = false;
        sendSignal(mCurAttV14.sync.sync_mode);
#endif
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if RKAIQ_HAVE_DEHAZE_V10
XCamReturn RkAiqAdehazeHandleInt::setSwAttribV10(const adehaze_sw_v10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adehaze_v10_SetAttrib(mAlgoCtx, const_cast<adehaze_sw_v10_t*>(att), false);
#else
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV10, att, sizeof(adehaze_sw_v10_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV10, att, sizeof(adehaze_sw_v10_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV10 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::getSwAttribV10(adehaze_sw_v10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
      mCfgMutex.lock();
      ret = rk_aiq_uapi_adehaze_v10_GetAttrib(mAlgoCtx, att);
      mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adehaze_v10_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV10, sizeof(adehaze_sw_v10_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adehaze_v10_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV10.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
XCamReturn RkAiqAdehazeHandleInt::setSwAttribV11(const adehaze_sw_v11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adehaze_v11_SetAttrib(mAlgoCtx, const_cast<adehaze_sw_v11_t*>(att), false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV11, att, sizeof(adehaze_sw_v11_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV11, att, sizeof(adehaze_sw_v11_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV11 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::getSwAttribV11(adehaze_sw_v11_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
      mCfgMutex.lock();
      ret = rk_aiq_uapi_adehaze_v11_GetAttrib(mAlgoCtx, att);
      mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adehaze_v11_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV11, sizeof(adehaze_sw_v11_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adehaze_v11_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV11.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V12
XCamReturn RkAiqAdehazeHandleInt::setSwAttribV12(const adehaze_sw_v12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
#ifndef USE_NEWSTRUCT
    ret = rk_aiq_uapi_adehaze_v12_SetAttrib(mAlgoCtx, const_cast<adehaze_sw_v12_t*>(att), false);
#endif
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV12, att, sizeof(adehaze_sw_v12_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV12, att, sizeof(adehaze_sw_v12_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV12 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::getSwAttribV12(adehaze_sw_v12_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
#ifndef USE_NEWSTRUCT
      mCfgMutex.lock();
      ret = rk_aiq_uapi_adehaze_v12_GetAttrib(mAlgoCtx, att);
      mCfgMutex.unlock();
#endif
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adehaze_v12_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV12, sizeof(adehaze_sw_v12_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adehaze_v12_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV12.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V14
XCamReturn RkAiqAdehazeHandleInt::setSwAttribV14(const adehaze_sw_v14_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_adehaze_v14_SetAttrib(mAlgoCtx, const_cast<adehaze_sw_v14_t*>(att), false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV14, att, sizeof(adehaze_sw_v14_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV14, att, sizeof(adehaze_sw_v14_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV14 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdehazeHandleInt::getSwAttribV14(adehaze_sw_v14_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    ret = rk_aiq_uapi_adehaze_v14_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adehaze_v14_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV14, sizeof(adehaze_sw_v14_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adehaze_v14_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV14.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
XCamReturn RkAiqAdehazeHandleInt::genIspResult(RkAiqFullParams* params,
                                               RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom  = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAdhaz* adhaz_com             = (RkAiqAlgoProcResAdhaz*)mProcOutParam;
    rk_aiq_isp_dehaze_params_t* dehaze_param = params->mDehazeParams->data().ptr();

    if (!adhaz_com) {
        LOGD_ANALYZER("no adhaz result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (sharedCom->init) {
        dehaze_param->frame_id = 0;
    } else {
        dehaze_param->frame_id = shared->frameId;
    }

    if (adhaz_com->res_com.cfg_update) {
        mSyncFlag = shared->frameId;
        dehaze_param->sync_flag = mSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mDehazeParams = params->mDehazeParams;
        dehaze_param->is_update = true;
        LOGD_ADEHAZE("[%d] params from algo", mSyncFlag);
    } else if (mSyncFlag != dehaze_param->sync_flag) {
        dehaze_param->sync_flag = mSyncFlag;
        // copy from latest result
        if (cur_params->mDehazeParams.ptr()) {
            dehaze_param->result = cur_params->mDehazeParams->data()->result;
            dehaze_param->is_update = true;
        } else {
            LOGE_ADEHAZE("no latest params !");
            dehaze_param->is_update = false;
        }
        LOGD_ADEHAZE("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        dehaze_param->is_update = false;
        LOGD_ADEHAZE("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
