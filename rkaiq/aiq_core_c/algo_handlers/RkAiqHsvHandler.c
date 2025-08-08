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
#include "RkAiqHsvHandler.h"
#include "aiq_core.h"
#include "newStruct/algo_common.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerHsv_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcHsv)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_HSV_PARAM;
    pHdl->mResultSize = sizeof(hsv_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerHsv_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "hsv handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "hsv algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerHsv_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoResCom* proc_res = pAlgoHandler->mProcOutParam;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "hsv handle processing failed");
    }

    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_HSV_PARAM];
    if (!pBase) {
        LOGW_AHSV("no hsv params buf !");
        return XCAM_RETURN_BYPASS;
    }

    proc_res->algoRes =  (rk_aiq_isp_hsv_params_t*)pBase->_data;

    RkAiqAlgoProcHsv* proc_int        = (RkAiqAlgoProcHsv*)pAlgoHandler->mProcInParam;
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    AlgoRstShared_t* awb_proc_res = shared->res_comb.awb_proc_res_c;
    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res->_data;
    }
    if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type))
        get_illu_estm_info(&proc_int->illu_info, awb_res, pCurExp, sharedCom->working_mode);

    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "hsv algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerHsv_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerHsv_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerHsv_prepare;
    pHdl->init         = _handlerHsv_init;
	return pHdl;
}

XCamReturn AiqHsvHandler_queryahsvStatus(AiqHsvHandler_t* pHdlHsv, ahsv_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlHsv->mCfgMutex);
    ret = algo_hsv_queryahsvStatus(pHdlHsv->mAlgoCtx, status);
    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHsvHandler_setCalib(AiqHsvHandler_t* pHdlHsv, ahsv_hsvCalib_t* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlHsv->mCfgMutex);
    ret = algo_hsv_SetCalib(pHdlHsv->mAlgoCtx, calib);
    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHsvHandler_getCalib(AiqHsvHandler_t* pHdlHsv, ahsv_hsvCalib_t* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlHsv->mCfgMutex);

    ret = algo_hsv_GetCalib(pHdlHsv->mAlgoCtx, calib);

    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHsvHandler_setSatStrth(AiqHsvHandler_t* pHdlHsv, ahsv_satStrg_t* strg) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlHsv->mCfgMutex);
    ret = algo_hsv_SetSatStrth(pHdlHsv->mAlgoCtx, strg);
    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHsvHandler_getSatStrth(AiqHsvHandler_t* pHdlHsv, ahsv_satStrg_t* strg) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlHsv->mCfgMutex);

    ret = algo_hsv_GetSatStrth(pHdlHsv->mAlgoCtx, strg);

    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHsvHandler_setHueOffset(AiqHsvHandler_t* pHdlHsv, ahsv_hueOffset_t* offset) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlHsv->mCfgMutex);
    ret = algo_hsv_SetHueOffset(pHdlHsv->mAlgoCtx, offset);
    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHsvHandler_getHueOffset(AiqHsvHandler_t* pHdlHsv, ahsv_hueOffset_t* offset) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlHsv->mCfgMutex);

    ret = algo_hsv_getHueOffset(pHdlHsv->mAlgoCtx, offset);

    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHsvHandler_setValOffset(AiqHsvHandler_t* pHdlHsv, ahsv_valOffset_t* offset) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlHsv->mCfgMutex);
    ret = algo_hsv_SetValOffset(pHdlHsv->mAlgoCtx, offset);
    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHsvHandler_getValOffset(AiqHsvHandler_t* pHdlHsv, ahsv_valOffset_t* offset) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlHsv->mCfgMutex);

    ret = algo_hsv_getValOffset(pHdlHsv->mAlgoCtx, offset);

    aiqMutex_unlock(&pHdlHsv->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}