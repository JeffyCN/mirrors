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
#include "RkAiqBlcHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerBlc_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcBlc)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResBlc)));

    pHdl->mResultType = RESULT_TYPE_BLC_PARAM;
    pHdl->mResultSize = sizeof(blc_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerBlc_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "blc handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "blc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerBlc_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResBlc* blc_proc_res_int = (RkAiqAlgoProcResBlc*)pAlgoHandler->mProcOutParam;
    RkAiqAlgoProcBlc* blc_proc_param = (RkAiqAlgoProcBlc*)pAlgoHandler->mProcInParam;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    if (sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        blc_proc_param->ishdr = 0;
    else
        blc_proc_param->ishdr = 1;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "blc handle processing failed");
    }

    AiqAlgoHandlerBlc_t* pblcHandler     = (AiqAlgoHandlerBlc_t*)pAlgoHandler;
    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_BLC_PARAM];
    if (!pBase) {
        LOGW_ABLC("no blc params buf !");
        return XCAM_RETURN_BYPASS;
    }

    blc_proc_param->aeIsConverged = pblcHandler->mAeProcRes.IsConverged;
    rk_aiq_isp_blc_v33_t* blcRes = (rk_aiq_isp_blc_params_t*)pBase->_data;
    blcRes->aeIsConverged = &pblcHandler->aeIsConverged;
    blcRes->damping = &pblcHandler->damping;
    pblcHandler->aeIsConverged = pblcHandler->mAeProcRes.IsConverged;
    if (pblcHandler->damping && !blc_proc_param->ishdr) {
        blc_proc_param->damping = true;
    }
    else {
        blc_proc_param->damping = false;
    }

    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);

    blc_proc_res_int->blcRes = (blc_param_t*)pBase->_data;
    if (blc_proc_res_int->res_com.cfg_update) {
        if (!blc_proc_res_int->res_com.en) {
            blc_proc_res_int->blcRes->dyn.obcPostTnr.sw_blcT_obcPostTnr_en = false;
        }
        shared->res_comb.blc_en       = blc_proc_res_int->res_com.en;
        shared->res_comb.blc_proc_res = blc_proc_res_int->blcRes;
    } else {
        shared->res_comb.blc_en       = ((AiqAlgoHandlerBlc_t*)pAlgoHandler)->mLatesten;
        shared->res_comb.blc_proc_res = ((AiqAlgoHandlerBlc_t*)pAlgoHandler)->mLatestparam;
    }

    RKAIQCORE_CHECK_RET(ret, "blc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

static XCamReturn _handlerBlc_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
                                           AiqFullParams_t* cur_params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandlerBlc_t* pHdlBlc     = (AiqAlgoHandlerBlc_t*)pAlgoHandler;
    aiq_params_base_t* pBase = params->pParamsArray[RESULT_TYPE_BLC_PARAM];
    if (!pBase) {
        LOGW_ABLC("no blc params buf !");
        return XCAM_RETURN_BYPASS;
    }
    blc_param_t* blc_param     = (blc_param_t*)pBase->_data;
    if (pAlgoHandler->mProcOutParam->cfg_update) {
		pHdlBlc->mLatestparam                           = blc_param;
		pHdlBlc->mLatesten                              = pBase->en;
	}
    ret = AiqAlgoHandler_genIspResult_common(pAlgoHandler, params, cur_params);
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerBlc_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerBlc_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);

    ((AiqAlgoHandlerBlc_t*)pHdl)->mLatestparam               = NULL;
    ((AiqAlgoHandlerBlc_t*)pHdl)->mLatesten                  = false;
    pHdl->processing   = _handlerBlc_processing;
    pHdl->genIspResult = _handlerBlc_genIspResult;
    pHdl->prepare      = _handlerBlc_prepare;
    pHdl->init         = _handlerBlc_init;
	return pHdl;
}

#if 0
XCamReturn AiqAlgoHandlerBlc_setAttrib(AiqAlgoHandlerBlc_t* pHdlBlc, blc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlBlc->_base.mCfgMutex);
    ret = algo_blc_SetAttrib(pHdlBlc->_base.mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlBlc->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerBlc_getAttrib(AiqAlgoHandlerBlc_t* pHdlBlc, blc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlBlc->_base.mCfgMutex);

    ret = algo_blc_GetAttrib(pHdlBlc->_base.mAlgoCtx, attr);

    aiqMutex_unlock(&pHdlBlc->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerBlc_queryStatus(AiqAlgoHandlerBlc_t* pHdlBlc, blc_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlBlc->_base.mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlBlc;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_BLC_PARAM];

    if (pCurBase) {
        rk_aiq_isp_blc_params_t* blc_param = (rk_aiq_isp_blc_params_t*)pCurBase->_data;
        if (blc_param) {
            status->stMan  = *blc_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlBlc->_base.mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ABLC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ABLC("have no status info !");
    }

    aiqMutex_unlock(&pHdlBlc->_base.mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
