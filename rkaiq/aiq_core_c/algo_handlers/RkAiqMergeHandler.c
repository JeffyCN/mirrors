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
#include "RkAiqMergeHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "newStruct/algo_common.h"

static void _handlerMerge_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcMerge)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_MERGE_PARAM;
    pHdl->mResultSize = sizeof(mge_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerMerge_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "mge handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "mge algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerMerge_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "mge handle processing failed");
    }

    RkAiqAlgoProcMerge* merge_proc_param = (RkAiqAlgoProcMerge*)pAlgoHandler->mProcInParam;
    merge_proc_param->LongFrmMode =
		((AiqAlgoHandlerMerge_t*)pAlgoHandler)->mAeProcRes.LongFrmMode;

    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerMerge_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerMerge_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);

    pHdl->processing   = _handlerMerge_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerMerge_prepare;
    pHdl->init         = _handlerMerge_init;
	return pHdl;
}

#if 0
XCamReturn AiqAlgoHandlerMerge_setAttrib(AiqAlgoHandlerMerge_t* pHdlMerge, mge_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlMerge->_base.mCfgMutex);
    ret = algo_merge_SetAttrib(pHdlMerge->_base.mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlMerge->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerMerge_getAttrib(AiqAlgoHandlerMerge_t* pHdlMerge, mge_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlMerge->_base.mCfgMutex);

    ret = algo_merge_GetAttrib(pHdlMerge->_base.mAlgoCtx, attr);

    aiqMutex_unlock(&pHdlMerge->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerMerge_queryStatus(AiqAlgoHandlerMerge_t* pHdlMerge, mge_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlMerge->_base.mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlMerge;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_MERGE_PARAM];

    if (pCurBase) {
        rk_aiq_isp_merge_params_t* mge_param = (rk_aiq_isp_merge_params_t*)pCurBase->_data;
        if (mge_param) {
            status->stMan  = *mge_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlMerge->_base.mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_AMERGE("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_AMERGE("have no status info !");
    }

    aiqMutex_unlock(&pHdlMerge->_base.mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
void AiqAlgoHandlerMerge_setAeProcRes(AiqAlgoHandlerMerge_t* pHdlMerge, RkAiqAlgoProcResAeShared_t* aeProcRes)
{
    pHdlMerge->mAeProcRes = *aeProcRes;
}
