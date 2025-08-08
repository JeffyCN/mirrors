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
#include "RkAiqLscHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "newStruct/algo_common.h"

static void _handlerLsc_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcLsc)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_LSC_PARAM;
    pHdl->mResultSize = sizeof(lsc_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerLsc_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "lsc handle prepare failed");

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "lsc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerLsc_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //RkAiqAlgoProcResLsc* lsc_proc_res_int = (RkAiqAlgoProcResLsc*)pAlgoHandler->mProcOutParam;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "lsc handle processing failed");
    }

    RkAiqAlgoProcLsc* proc_int        = (RkAiqAlgoProcLsc*)pAlgoHandler->mProcInParam;
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    AlgoRstShared_t* awb_proc_res = shared->res_comb.awb_proc_res_c;
    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res->_data;
    }
    if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type))
        get_illu_estm_info(&proc_int->illu_info, awb_res, pCurExp, sharedCom->working_mode);
    /*
    if(colorConstFlag==true){
        memcpy(lsc_proc_int->illu_info.awbGain,colorSwInfo.awbGain,sizeof(colorSwInfo.awbGain));
        lsc_proc_int->illu_info.sensorGain = colorSwInfo.sensorGain;
    }
    */

    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerLsc_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerLsc_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);

    pHdl->processing   = _handlerLsc_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerLsc_prepare;
    pHdl->init         = _handlerLsc_init;
	return pHdl;
}

XCamReturn AiqAlgoHandlerLsc_queryalscStatus(AiqAlgoHandlerLsc_t* pHdlLsc, alsc_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlLsc->_base.mCfgMutex);

    ret = algo_lsc_queryalscStatus(pHdlLsc->_base.mAlgoCtx, status);

    aiqMutex_unlock(&pHdlLsc->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerLsc_setCalib(AiqAlgoHandlerLsc_t* pHdlLsc, alsc_lscCalib_t* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlLsc->_base.mCfgMutex);
    ret = algo_lsc_SetCalib(pHdlLsc->_base.mAlgoCtx, calib);
    aiqMutex_unlock(&pHdlLsc->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerLsc_getCalib(AiqAlgoHandlerLsc_t* pHdlLsc, alsc_lscCalib_t* calib) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlLsc->_base.mCfgMutex);

    ret = algo_lsc_GetCalib(pHdlLsc->_base.mAlgoCtx, calib);

    aiqMutex_unlock(&pHdlLsc->_base.mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if 0
XCamReturn AiqAlgoHandlerLsc_queryStatus(AiqAlgoHandlerLsc_t* pHdlLsc, lsc_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlLsc->_base.mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlLsc;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_LSC_PARAM];

    if (pCurBase) {
        rk_aiq_isp_lsc_params_t* lsc_param = (rk_aiq_isp_lsc_params_t*)pCurBase->_data;
        if (lsc_param) {
            status->stMan = *lsc_param; 
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlLsc->_base.mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ALSC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ALSC("have no status info !");
    }

    aiqMutex_unlock(&pHdlLsc->_base.mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
XCamReturn AiqAlgoHandlerLsc_setAcolorSwInfo(AiqAlgoHandlerLsc_t* pHdlLsc, rk_aiq_color_info_t* aColor_sw_info)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOGV_ALSC("%s sensor gain = %f, wbgain=[%f,%f] ",__FUNCTION__,aColor_sw_info->sensorGain,
      aColor_sw_info->awbGain[0], aColor_sw_info->awbGain[1]);
    pHdlLsc->colorSwInfo = *aColor_sw_info;
    pHdlLsc->colorConstFlag=true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
