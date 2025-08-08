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
#include "RkAiqHisteqHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerHisteq_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_HISTEQ_PARAM;
    pHdl->mResultSize = sizeof(histeq_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerHisteq_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "histeq handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "histeq algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerHisteq_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "histeq handle processing failed");
    }

    AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "histeq algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerHisteq_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerHisteq_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerHisteq_prepare;
    pHdl->init         = _handlerHisteq_init;
	return pHdl;
}

#if 0
XCamReturn AiqHisteqHandler_setAttrib(AiqHisteqHandler_t* pHdlHisteq, histeq_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlHisteq->mCfgMutex);
    ret = algo_histeq_SetAttrib(pHdlHisteq->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlHisteq->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqHisteqHandler_getAttrib(AiqHisteqHandler_t* pHdlHisteq, histeq_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlHisteq->mCfgMutex);
    ret = algo_histeq_GetAttrib(pHdlHisteq->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlHisteq->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqHisteqHandler_queryStatus(AiqHisteqHandler_t* pHdlHisteq, histeq_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlHisteq->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlHisteq;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_HISTEQ_PARAM];

    if (pCurBase) {
        rk_aiq_isp_histeq_params_t* histeq_param = (rk_aiq_isp_histeq_params_t*)pCurBase->_data;
        if (histeq_param) {
            status->stMan  = *histeq_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlHisteq->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_AHISTEQ("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_AHISTEQ("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;

}
#endif
