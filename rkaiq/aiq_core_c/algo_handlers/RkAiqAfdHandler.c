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
#include "RkAiqAfdHandler.h"
#include "RkAiqAeHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "afd/rk_aiq_uapi_afd_int.h"

static void _handlerAfd_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoConfigAfd)));
    pHdl->mPreInParam   = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoPreAfd)));
    pHdl->mPreOutParam  = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoPreResAfd)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcAfd)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResAfd)));

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerAfd_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "afd handle prepare failed");

    RkAiqAlgoConfigAfd* afd_config = (RkAiqAlgoConfigAfd*)pAlgoHandler->mConfig;
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    /*****************AfdConfig Sensor Exp related info*****************/
    afd_config->LinePeriodsPerField = (float)sharedCom->snsDes.frame_length_lines;
    afd_config->PixelPeriodsPerLine = (float)sharedCom->snsDes.line_length_pck;
    afd_config->PixelClockFreqMHZ = (float) sharedCom->snsDes.pixel_clock_freq_mhz;

    /*****************AecConfig pic-info params*****************/
    afd_config->RawWidth  = sharedCom->snsDes.isp_acq_width;
    afd_config->RawHeight = sharedCom->snsDes.isp_acq_height;

    GlobalParamsManager_lockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    GlobalParamsManager_unlockAlgoParam(pAlgoHandler->mAiqCore->mGlobalParamsManger, pAlgoHandler->mResultType);
    RKAIQCORE_CHECK_RET(ret, "afd algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerAfd_preProcess(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAfdHandler_t* pAfdHdl = (AiqAfdHandler_t*)pAlgoHandler;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoPreAfd* afd_pre_int        = (RkAiqAlgoPreAfd*)pAlgoHandler->mPreInParam;
    afd_pre_int->aeProcRes = &pAfdHdl->mAeProcRes;

    ret = AiqAlgoHandler_preProcess(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "afd handle preprocess failed");
    }

    if(sharedCom->init)
    {
        LOGV("init: no thumb data, ignore!");
        return XCAM_RETURN_NO_ERROR;
    }
    else
    {
        if(shared->scaleRawInfo.raw_s) {
            afd_pre_int->thumbStatsS = &(shared->scaleRawInfo.raw_s)->_xcam_vb;
            afd_pre_int->thumbH = shared->scaleRawInfo.raw_s->_xcam_vb.info.height;
            afd_pre_int->thumbW = shared->scaleRawInfo.raw_s->_xcam_vb.info.width;
        } else {
            afd_pre_int->thumbStatsS = NULL;
        }

        if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2) {
            if(shared->scaleRawInfo.raw_l) {
                afd_pre_int->thumbStatsL = &(shared->scaleRawInfo.raw_l)->_xcam_vb;
            } else {
                afd_pre_int->thumbStatsL = NULL;
            }
        }
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->pre_process(pAlgoHandler->mPreInParam, pAlgoHandler->mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "afd algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerAfd_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "afd handle processing failed");
    }

	// skip processing if is group algo
	if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
		RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
		ret                       = des->processing(pAlgoHandler->mProcInParam, pAlgoHandler->mProcOutParam);
		RKAIQCORE_CHECK_RET(ret, "afd algo processing failed");
	}

#if RKAIQ_HAVE_AFD_V2
    RkAiqAlgoProcResAfd* afd_proc_res_int = (RkAiqAlgoProcResAfd*)pAlgoHandler->mProcOutParam;
    AiqAlgoHandler_t* ae_handle = pAlgoHandler->mAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];
    if (ae_handle) {
        int algo_id = ae_handle->mDes->id;
        if (algo_id == 0) {
            AiqAlgoHandlerAe_getAfdResForAE((AiqAlgoHandlerAe_t*)ae_handle, afd_proc_res_int->afdRes.PeakRes);
        }
    }
#endif

    RKAIQCORE_CHECK_RET(ret, "afd algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


static XCamReturn _handlerAfd_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
                                           AiqFullParams_t* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAfdHandler_t* pAfdHdl = (AiqAfdHandler_t*)pAlgoHandler;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResAfd* afd_res = (RkAiqAlgoProcResAfd*)pAlgoHandler->mProcOutParam;

    aiq_params_base_t* pBase = params->pParamsArray[RESULT_TYPE_AFD_PARAM];
    if (!pBase) {
        LOGW_ATMO("no afd params buf !");
        return XCAM_RETURN_BYPASS;
    }

    if (!afd_res) {
        LOGE_ATMO("no afd result");
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_isp_afd_params_t* afd_param = (rk_aiq_isp_afd_params_t*)pBase->_data;
    afd_param->enable = pAfdHdl->mAeProcRes.IsAutoAfd;
    AiqAfdHandler_getScaleRatio(pAfdHdl, &afd_param->ratio);
	cur_params->pParamsArray[RESULT_TYPE_AFD_PARAM] =
		params->pParamsArray[RESULT_TYPE_AFD_PARAM];

    if (pAfdHdl->mAfdSyncAly != afd_param->enable) {
        pAfdHdl->mAfdSyncAly = afd_param->enable;
        AiqCore_setVicapScaleFlag(pAlgoHandler->mAiqCore, pAfdHdl->mAfdSyncAly);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerAfd_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAfdHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->preProcess   = _handlerAfd_preProcess;
    pHdl->processing   = _handlerAfd_processing;
    pHdl->genIspResult = _handlerAfd_genIspResult;
    pHdl->prepare      = _handlerAfd_prepare;
    pHdl->init         = _handlerAfd_init;
    ((AiqAfdHandler_t*)pHdl)->mAfdSyncAly  = true;
	return pHdl;
}

XCamReturn AiqAfdHandler_getScaleRatio(AiqAfdHandler_t* pHdlAfd, int* scale_ratio)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlAfd;
    aiqMutex_lock(&pHdl->mCfgMutex);

    Uapi_AfdAttr_t AfdAttr;
    rk_aiq_uapi_afd_v2_GetAttr(pHdl->mAlgoCtx, &AfdAttr);
    *scale_ratio = AfdAttr.scale_ratio;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAfdHandler_getAfdEn(AiqAfdHandler_t* pHdlAfd, bool* en)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlAfd;
    aiqMutex_lock(&pHdl->mCfgMutex);

    Uapi_AfdAttr_t AfdAttr;
    rk_aiq_uapi_afd_v2_GetAttr(pHdl->mAlgoCtx, &AfdAttr);
    *en = AfdAttr.enable;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
