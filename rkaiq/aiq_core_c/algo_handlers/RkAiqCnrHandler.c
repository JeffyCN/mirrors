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
#include "RkAiqCnrHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerCnr_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcCnr)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_UVNR_PARAM;
    pHdl->mResultSize = sizeof(cnr_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerCnr_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "cnr handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "cnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerCnr_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "cnr handle processing failed");
    }

#if defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)
    RkAiqAlgoProcCnr* cnr_proc_param = (RkAiqAlgoProcCnr*)pAlgoHandler->mProcInParam;
    cnr_proc_param->blc_ob_predgain = 1.0;
#else
    AblcProc_V32_t* blc_res = shared->res_comb.ablcV32_proc_res;
    float ob_predgain = blc_res->isp_ob_predgain;
    RkAiqAlgoProcCnr* cnr_proc_param = (RkAiqAlgoProcCnr*)pAlgoHandler->mProcInParam;
    cnr_proc_param->blc_ob_predgain = ob_predgain;
#endif

    AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "cnr algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerCnr_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerCnr_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerCnr_prepare;
    pHdl->init         = _handlerCnr_init;
	return pHdl;
}

#if 0
XCamReturn AiqCnrHandler_setAttrib(AiqCnrHandler_t* pHdlCnr, cnr_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlCnr->mCfgMutex);
    ret = algo_cnr_SetAttrib(pHdlCnr->mAlgoCtx, attr, false);
    aiqMutex_unlock(&pHdlCnr->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqCnrHandler_getAttrib(AiqCnrHandler_t* pHdlCnr, cnr_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlCnr->mCfgMutex);
    ret = algo_cnr_GetAttrib(pHdlCnr->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlCnr->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqCnrHandler_queryStatus(AiqCnrHandler_t* pHdlCnr, cnr_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlCnr->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlCnr;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_UVNR_PARAM];

    if (pCurBase) {
        rk_aiq_isp_cnr_params_t* cnr_param = (rk_aiq_isp_cnr_params_t*)pCurBase->_data;
        if (cnr_param) {
            status->stMan  = *cnr_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlCnr->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;

}
#endif

XCamReturn AiqCnrHandler_setStrength(AiqCnrHandler_t* pHdlCnr, acnr_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlCnr->mCfgMutex);
    ret = algo_cnr_SetStrength(pHdlCnr->mAlgoCtx, strg->percent, strg->en);
    aiqMutex_unlock(&pHdlCnr->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqCnrHandler_getStrength(AiqCnrHandler_t* pHdlCnr, acnr_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlCnr->mCfgMutex);
    ret = algo_cnr_GetStrength(pHdlCnr->mAlgoCtx, &strg->percent, &strg->en);
    aiqMutex_unlock(&pHdlCnr->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
