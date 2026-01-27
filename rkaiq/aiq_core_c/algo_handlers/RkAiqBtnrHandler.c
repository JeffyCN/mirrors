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
#include "RkAiqBtnrHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerBtnr_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcBtnr)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_TNR_PARAM;
    pHdl->mResultSize = sizeof(btnr_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerBtnr_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "btnr handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "btnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerBtnr_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "btnr handle processing failed");
    }

#if defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)
    RkAiqAlgoProcBtnr* btnr_proc_param = (RkAiqAlgoProcBtnr*)pAlgoHandler->mProcInParam;
    btnr_proc_param->blc_ob_predgain = 1.0;
#else
    AblcProc_V32_t* blc_res = shared->res_comb.ablcV32_proc_res;
    float ob_predgain = blc_res->isp_ob_predgain;
    RkAiqAlgoProcBtnr* btnr_proc_param = (RkAiqAlgoProcBtnr*)pAlgoHandler->mProcInParam;
    btnr_proc_param->blc_ob_predgain = ob_predgain;
#endif

    AiqAlgoHandler_do_processing_common(pAlgoHandler);

    if (pAlgoHandler->mProcOutParam->cfg_update) {
        shared->res_comb.bayernr3d_en = pAlgoHandler->mProcOutParam->en;
    }

    RKAIQCORE_CHECK_RET(ret, "btnr algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerBtnr_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
        return NULL;
    AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerBtnr_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerBtnr_prepare;
    pHdl->init         = _handlerBtnr_init;
    return pHdl;
}

#if 0
XCamReturn AiqBtnrHandler_setAttrib(AiqBtnrHandler_t* pHdlBtnr, btnr_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlBtnr->mCfgMutex);
    ret = algo_bayertnr_SetAttrib(pHdlBtnr->mAlgoCtx, attr, false);
    aiqMutex_unlock(&pHdlBtnr->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqBtnrHandler_getAttrib(AiqBtnrHandler_t* pHdlBtnr, btnr_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlBtnr->mCfgMutex);

    ret = algo_bayertnr_GetAttrib(pHdlBtnr->mAlgoCtx, attr);

    aiqMutex_unlock(&pHdlBtnr->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqBtnrHandler_queryStatus(AiqBtnrHandler_t* pHdlBtnr, btnr_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlBtnr->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlBtnr;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_TNR_PARAM];

    if (pCurBase) {
        rk_aiq_isp_btnr_params_t* btnr_param = (rk_aiq_isp_btnr_params_t*)pCurBase->_data;
        if (btnr_param) {
            status->stMan  = *btnr_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlBtnr->mOpMode;
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

XCamReturn AiqBtnrHandler_setStrength(AiqBtnrHandler_t* pHdlBtnr, abtnr_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlBtnr->mCfgMutex);
    ret = algo_bayertnr_SetStrength(pHdlBtnr->mAlgoCtx, strg->percent, strg->en);
    aiqMutex_unlock(&pHdlBtnr->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqBtnrHandler_getStrength(AiqBtnrHandler_t* pHdlBtnr, abtnr_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlBtnr->mCfgMutex);
    ret = algo_bayertnr_GetStrength(pHdlBtnr->mAlgoCtx, &strg->percent, &strg->en);
    aiqMutex_unlock(&pHdlBtnr->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
