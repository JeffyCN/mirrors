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
#include "RkAiqCgcHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerCgc_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcCgc)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_CGC_PARAM;
    pHdl->mResultSize = sizeof(cgc_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerCgc_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "cgc handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "cgc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerCgc_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "cgc handle processing failed");
    }

    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "cgc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerCgc_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerCgc_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerCgc_prepare;
    pHdl->init         = _handlerCgc_init;
	return pHdl;
}

#if 0
XCamReturn AiqAlgoHandlerCgc_setAttrib(AiqAlgoHandlerCgc_t* pHdlCgc, cgc_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlCgc->mCfgMutex);
    ret = algo_cgc_SetAttrib(pHdlCgc->mAlgoCtx, attr, false);
    aiqMutex_unlock(&pHdlCgc->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqAlgoHandlerCgc_getAttrib(AiqAlgoHandlerCgc_t* pHdlCgc, cgc_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlCgc->mCfgMutex);

    ret = algo_cgc_GetAttrib(pHdlCgc->mAlgoCtx, attr);

    aiqMutex_unlock(&pHdlCgc->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerCgc_queryStatus(AiqAlgoHandlerCgc_t* pHdlCgc, cgc_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlCgc->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlCgc;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_CGC_PARAM];

    if (pCurBase) {
        rk_aiq_isp_cgc_params_t* cgc_param = (rk_aiq_isp_cgc_params_t*)pCurBase->_data;
        if (cgc_param) {
            status->stMan  = *cgc_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlCgc->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACGC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACGC("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;

}
#endif
