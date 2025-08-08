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

#include "aiq_base.h"
#include "RkAiqDehazeHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

//static dehazeStrth strthCtrl;
//static bool updateStrth;

static void _handlerDehaze_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcDehaze)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_DEHAZE_PARAM;
    pHdl->mResultSize = sizeof(dehaze_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerDehaze_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "dehaze handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "dehaze algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerDehaze_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "dehaze handle processing failed");
    }

    RkAiqAlgoProcDehaze* dehaze_proc_param = (RkAiqAlgoProcDehaze*)pAlgoHandler->mProcInParam;
    aiq_stats_base_t* xDehazeStats = NULL;
    if (shared->adehazeStatsBuf) {
        xDehazeStats = shared->adehazeStatsBuf;
        if (!xDehazeStats) LOGE_ADEHAZE("dehaze stats is null");
    } else {
        LOGW_ADEHAZE("the xcamvideobuffer of isp stats is null");
    }
    if (!xDehazeStats || !xDehazeStats->bValid) {
        LOGW_ADEHAZE("no adehaze stats, ignore!");
        dehaze_proc_param->stats_true = false;
    }
    else {
        dehaze_proc_param->stats_true = true;
#if RKAIQ_HAVE_DEHAZE_V12
        dehaze_proc_param->dehaze_stats_v12 = &((rkisp_adehaze_stats_t*)xDehazeStats->_data)->dehaze_stats_v12;
#endif
#if RKAIQ_HAVE_DEHAZE_V14
        dehaze_proc_param->dehaze_stats_v14 = &((rkisp_adehaze_stats_t*)xDehazeStats->_data)->dehaze_stats_v14;
#endif
    }

    dehaze_proc_param->blc_ob_enable   = false;
    dehaze_proc_param->isp_ob_predgain = 1.0f;

    AiqAlgoHandler_do_processing_common(pAlgoHandler);

    /*
    ((rk_aiq_isp_dehaze_params_t*)pBase->_data)->MDehazeStrth = strthCtrl.MDehazeStrth;
    ((rk_aiq_isp_dehaze_params_t*)pBase->_data)->MEnhanceStrth = strthCtrl.MEnhanceStrth;
    ((rk_aiq_isp_dehaze_params_t*)pBase->_data)->MEnhanceChromeStrth = strthCtrl.MEnhanceChromeStrth;

    if (updateStrth) {
        dehaze_proc_res_int->res_com.cfg_update = true;
        updateStrth = false;
    }
    */

    RKAIQCORE_CHECK_RET(ret, "dehaze algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerDehaze_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerDehaze_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerDehaze_prepare;
    pHdl->init         = _handlerDehaze_init;
	return pHdl;
}

#if 0
XCamReturn AiqDehazeHandler_setAttrib(AiqDehazeHandler_t* pHdlDehaze, dehaze_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlDehaze->mCfgMutex);
    ret = algo_dehaze_SetAttrib(pHdlDehaze->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlDehaze->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqDehazeHandler_getAttrib(AiqDehazeHandler_t* pHdlDehaze, dehaze_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlDehaze->mCfgMutex);

    ret = algo_dehaze_GetAttrib(pHdlDehaze->mAlgoCtx, attr);

    aiqMutex_unlock(&pHdlDehaze->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqDehazeHandler_queryStatus(AiqDehazeHandler_t* pHdlDehaze, dehaze_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlDehaze->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlDehaze;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_DEHAZE_PARAM];

    if (pCurBase) {
        rk_aiq_isp_dehaze_params_t* dehaze_param = (rk_aiq_isp_dehaze_params_t*)pCurBase->_data;
        if (dehaze_param) {
            status->stMan  = *dehaze_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlDehaze->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ADEHAZE("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ADEHAZE("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;

}
#endif

XCamReturn AiqDehazeHandler_setStrength(AiqDehazeHandler_t* pHdlDehaze, adehaze_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlDehaze->mCfgMutex);
    ret = algo_dehaze_SetStrength(pHdlDehaze->mAlgoCtx, strg);
    aiqMutex_unlock(&pHdlDehaze->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqDehazeHandler_getStrength(AiqDehazeHandler_t* pHdlDehaze, adehaze_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlDehaze->mCfgMutex);
    ret = algo_dehaze_GetStrength(pHdlDehaze->mAlgoCtx, strg);
    aiqMutex_unlock(&pHdlDehaze->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
