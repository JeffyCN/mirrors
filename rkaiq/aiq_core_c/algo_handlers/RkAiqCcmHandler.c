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
#include "RkAiqCcmHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "newStruct/algo_common.h"

static void _handlerCcm_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcCcm)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_CCM_PARAM;
    pHdl->mResultSize = sizeof(ccm_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerCcm_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "ccm handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "ccm algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerCcm_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoResCom* proc_res = pAlgoHandler->mProcOutParam;
    //RkAiqAlgoProcResCcm* ccm_proc_res_int = (RkAiqAlgoProcResCcm*)pAlgoHandler->mProcOutParam;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ccm handle processing failed");
    }

    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_CCM_PARAM];
    if (!pBase) {
        LOGW_ACCM("no ccm params buf !");
        return XCAM_RETURN_BYPASS;
    }

    proc_res->algoRes =  (rk_aiq_isp_ccm_params_t*)pBase->_data;

    RkAiqAlgoProcCcm* proc_int        = (RkAiqAlgoProcCcm*)pAlgoHandler->mProcInParam;
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    AlgoRstShared_t* awb_proc_res = shared->res_comb.awb_proc_res_c;
    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res->_data;
    }
    if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type))
        get_illu_estm_info(&proc_int->illu_info, awb_res, pCurExp, sharedCom->working_mode);

    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "ccm algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerCcm_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerCcm_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);

    pHdl->processing   = _handlerCcm_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerCcm_prepare;
    pHdl->init         = _handlerCcm_init;
	return pHdl;
}

XCamReturn AiqAlgoHandlerCcm_queryaccmStatus(AiqAlgoHandlerCcm_t* pHdlCcm, accm_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlCcm->mCfgMutex);  
    
    ret = algo_ccm_queryaccmStatus(pHdlCcm->mAlgoCtx, status);

    aiqMutex_unlock(&pHdlCcm->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerCcm_setCalib(AiqAlgoHandlerCcm_t* pHdlCcm, accm_ccmCalib_t* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlCcm->mCfgMutex);
    ret = algo_ccm_SetCalib(pHdlCcm->mAlgoCtx, calib);
    aiqMutex_unlock(&pHdlCcm->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerCcm_getCalib(AiqAlgoHandlerCcm_t* pHdlCcm, accm_ccmCalib_t* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlCcm->mCfgMutex);

    ret = algo_ccm_GetCalib(pHdlCcm->mAlgoCtx, calib);

    aiqMutex_unlock(&pHdlCcm->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
