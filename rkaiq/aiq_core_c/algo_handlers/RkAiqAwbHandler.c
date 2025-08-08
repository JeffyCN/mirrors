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
#include "RkAiqAwbHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

#include "awb/rk_aiq_uapiv3_awb_int.h"

static void _handlerAwb_deinit(AiqAlgoHandler_t* pHdl) {
    AiqAlgoHandler_deinit(pHdl);
}

static void _handlerAwb_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoConfigAwb)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcAwb)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResAwb)));

    pHdl->mResultType = RESULT_TYPE_AWB_PARAM;
    //pHdl->mResultSize = sizeof(ae_param_t);

	AiqAlgoHandlerAwb_t* pAwbHdl = (AiqAlgoHandlerAwb_t*)pHdl;
    pAwbHdl->mWbGainSyncFlag = -1;
    pAwbHdl->mWbParamSyncFlag = -1;

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerAwb_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "ae handle prepare failed");

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    pAlgoHandler->mConfig->reserverd = (void*)(sharedCom->gray_mode);

    RkAiqAlgoConfigAwb* awb_config_int = (RkAiqAlgoConfigAwb*)pAlgoHandler->mConfig;

    awb_config_int->mem_ops_ptr   = pAlgoHandler->mAiqCore->mShareMemOps;

    aiqMutex_lock(&pAlgoHandler->mCfgMutex);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
    RKAIQCORE_CHECK_RET(ret, "ae algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerAwb_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

	AiqAlgoHandlerAwb_t* pAwbHdl = (AiqAlgoHandlerAwb_t*)pAlgoHandler;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcAwb* awb_proc_int = (RkAiqAlgoProcAwb*)pAlgoHandler->mProcInParam;
    RkAiqAlgoProcResAwb* awb_proc_res_int = (RkAiqAlgoProcResAwb*)pAlgoHandler->mProcOutParam;

    if ((!shared->awbStatsBuf || !shared->awbStatsBuf->bValid) && !sharedCom->init) {
		LOGE("fid:%d no awb stats, ignore!", shared->frameId);
        return XCAM_RETURN_BYPASS;
    }

	aiq_awb_stats_wrapper_t* pAwbStatsWrap = NULL;
	if (shared->awbStatsBuf)
		pAwbStatsWrap = (aiq_awb_stats_wrapper_t*)shared->awbStatsBuf->_data;
#if USE_NEWSTRUCT
    if (pAwbStatsWrap) {
        awb_proc_int->awb_statsBuf_v39 = &pAwbStatsWrap ->awb_stats_v39;
	} else
        awb_proc_int->awb_statsBuf_v39 = NULL;
#else
	awb_proc_int->awb_statsBuf_v39 = NULL;
#endif

#if !defined(ISP_HW_V39) || !defined(ISP_HW_V33) || !defined(ISP_HW_V35)
    if (shared->aecStatsBuf) {
		aiq_ae_stats_wrapper_t* pAeStatsWrap = (aiq_ae_stats_wrapper_t*)shared->aecStatsBuf->_data;
        awb_proc_int->aecStatsBuf = &pAeStatsWrap->aec_stats;
	} else
        awb_proc_int->aecStatsBuf = NULL;
#else
        awb_proc_int->aecStatsBuf = NULL;
#endif

    awb_proc_int->ablcProcResVaid = false;

#if RKAIQ_HAVE_BLC_V1
    awb_proc_int->ablcProcRes = shared->res_comb.ablc_proc_res;
    awb_proc_int->ablcProcResVaid = true;
#endif
#if RKAIQ_HAVE_BLC_V32
    blc_param_t* blc_res;
    AblcProc_V32_t ablcProcResV32;
    int isp_ob_max;
    ablcProcResV32.enable = shared->res_comb.blc_en;
    if (shared->res_comb.blc_proc_res) {
        blc_res = shared->res_comb.blc_proc_res;
        ablcProcResV32.blc_r  = blc_res->dyn.obcPreTnr.hw_blcC_obR_val;
        ablcProcResV32.blc_gr = blc_res->dyn.obcPreTnr.hw_blcC_obGr_val;
        ablcProcResV32.blc_gb = blc_res->dyn.obcPreTnr.hw_blcC_obGb_val;
        ablcProcResV32.blc_b  = blc_res->dyn.obcPreTnr.hw_blcC_obB_val;
        if (blc_res->dyn.obcPostTnr.sw_blcT_obcPostTnr_en) {
            ablcProcResV32.blc1_enable = true;
            ablcProcResV32.blc1_r  = 0;
            ablcProcResV32.blc1_gr = 0;
            ablcProcResV32.blc1_gb = 0;
            ablcProcResV32.blc1_b  = 0;

            ablcProcResV32.blc_ob_enable  = true;
            ablcProcResV32.isp_ob_offset  = blc_res->dyn.obcPostTnr.sw_blcT_autoOB_offset;
            ablcProcResV32.isp_ob_predgain = 1.0;
            isp_ob_max = (int)(4096 * ablcProcResV32.isp_ob_predgain) - ablcProcResV32.isp_ob_offset;
            if (isp_ob_max > 0)
                ablcProcResV32.isp_ob_max = isp_ob_max < 1048575 ? isp_ob_max : 1048575;
        } else {
            ablcProcResV32.blc1_enable = false;
            ablcProcResV32.blc_ob_enable = false;
            ablcProcResV32.blc1_r  = 0;
            ablcProcResV32.blc1_gr = 0;
            ablcProcResV32.blc1_gb = 0;
            ablcProcResV32.blc1_b  = 0;

            ablcProcResV32.blc_ob_enable  = false;
            ablcProcResV32.isp_ob_offset  = 0;
            ablcProcResV32.isp_ob_predgain = 1.0;
            ablcProcResV32.isp_ob_max = 0xfff;
        }
    }
    awb_proc_int->ablcProcResV32 = &ablcProcResV32;
#endif
	//
    // for otp awb
    awb_proc_int->awb_otp = &sharedCom->snsDes.otp_awb;

#if defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)
	aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_AWB_PARAM];
	if (pBase)
		awb_proc_res_int->awb_hw_cfg_priv = (rk_aiq_isp_awb_params_t*)pBase->_data;
	else
		awb_proc_res_int->awb_hw_cfg_priv = NULL;
#else
    awb_proc_res_int->awb_hw_cfg_priv = NULL;
#endif

	aiq_params_base_t* pGainBase = shared->fullParams->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM];
	if (pGainBase)
		awb_proc_res_int->awb_gain_algo = (rk_aiq_isp_awb_gain_params_t*)pGainBase->_data;
	else
		awb_proc_res_int->awb_gain_algo = NULL;

	ret = AiqAlgoHandler_processing(pAlgoHandler);
	if (ret < 0) {
		LOGE_ANALYZER("awb handle processing failed ret %d", ret);
		return ret;
	} else if (ret == XCAM_RETURN_BYPASS) {
		LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
		return ret;
	}

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
		if (des->processing) {
			aiqMutex_lock(&pAlgoHandler->mCfgMutex);
			ret = des->processing(pAlgoHandler->mProcInParam, (RkAiqAlgoResCom*)(pAlgoHandler->mProcOutParam));
			aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
			if (ret < 0) {
				LOGE_ANALYZER("awb algo processing failed ret %d", ret);
				return ret;
			} else if (ret == XCAM_RETURN_BYPASS) {
				LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
				return ret;
			}
		}
    }

    if (awb_proc_res_int->awb_cfg_update || awb_proc_res_int->awb_gain_update) {
		if (pAwbHdl->mProcResShared) {
			/* dec ref from aiqPool_getFree */
			AIQ_REF_BASE_UNREF(&pAwbHdl->mProcResShared->_ref_base);
			pAwbHdl->mProcResShared = NULL;
		}
        if (pAlgoHandler->mDes->id == 0) {
			AiqPoolItem_t* pItem =
				aiqPool_getFree(pAlgoHandler->mAiqCore->mProcResAwbSharedPool);
			if (pItem) {
				pAwbHdl->mProcResShared = (AlgoRstShared_t*)pItem->_pData;
			} else {
				LOGW_AWB("no awb_proc_res buf !");
#if RKAIQ_HAVE_DUMPSYS
                                pAlgoHandler->mAiqCore->mNoFreeBufCnt.awbProcRes++;
#endif
                        }
        }
		RkAiqAlgoProcResAwbShared_t* pAwbShared = NULL;
		if (pAwbHdl->mProcResShared) {
			pAwbShared = (RkAiqAlgoProcResAwbShared_t*)pAwbHdl->mProcResShared->_data;
			memcpy(&pAwbShared->awb_gain_algo, awb_proc_res_int->awb_gain_algo, sizeof(rk_aiq_wb_gain_t));
			pAwbShared->awb_smooth_factor = awb_proc_res_int->awb_smooth_factor;
			pAwbShared->varianceLuma = awb_proc_res_int->varianceLuma;
			pAwbShared->awbConverged = awb_proc_res_int->awbConverged;
		}
    } else {
        LOGD_AWB("awb results not updated");
    }

    if (pAlgoHandler->mPostShared && pAwbHdl->mProcResShared) {
        if (pAlgoHandler->mAiqCore->mAlogsComSharedParams.init) {
            RkAiqAlgosGroupShared_t* grpShared =
				pAlgoHandler->mAiqCore->mAlogsGroupSharedParamsMap[RK_AIQ_CORE_ANALYZE_GRP1];
			if (grpShared) {
				pAwbHdl->mProcResShared->frame_id = shared->frameId;
				grpShared->res_comb.awb_proc_res_c = pAwbHdl->mProcResShared;
            }
        } else {
			pAwbHdl->mProcResShared->frame_id = shared->frameId;
			AiqCoreMsg_t msg;
			msg.msg_id                    = XCAM_MESSAGE_AWB_PROC_RES_OK;
			msg.frame_id                  = shared->frameId;
			*(AlgoRstShared_t**)(msg.buf) = pAwbHdl->mProcResShared;
			AiqCore_post_message(pAlgoHandler->mAiqCore, &msg);
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

static XCamReturn _handlerAwb_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
                                           AiqFullParams_t* cur_params) {
    ENTER_ANALYZER_FUNCTION();
	AiqAlgoHandlerAwb_t* pAwbHdl = (AiqAlgoHandlerAwb_t*)pAlgoHandler;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResAwb* awb_com                = (RkAiqAlgoProcResAwb*)pAlgoHandler->mProcOutParam;

    int algo_id = pAlgoHandler->mDes->id;

    if (pAlgoHandler->mIsMulRun && (algo_id == 0)) {
        // do nothing for rkawb if custom algo running with rk algo
        return ret;
    }

    aiq_params_base_t* pAwbBase = params->pParamsArray[RESULT_TYPE_AWB_PARAM];
    if (!pAwbBase) {
        LOGW_AWB("no awb params buf !");
        return XCAM_RETURN_BYPASS;
    }

    aiq_params_base_t* pGainBase = params->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM];
    if (!pGainBase) {
        LOGW_AWB("no awb gain buf !");
        return XCAM_RETURN_BYPASS;
    }

    if (sharedCom->init) {
        pAwbBase->frame_id = 0;
        pGainBase->frame_id      = 0;
    } else {
        pAwbBase->frame_id = shared->frameId;
        pGainBase->frame_id      = shared->frameId;
    }

    if (AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
		cur_params->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM] = params->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM];
		cur_params->pParamsArray[RESULT_TYPE_AWB_PARAM] = params->pParamsArray[RESULT_TYPE_AWB_PARAM];
        return XCAM_RETURN_NO_ERROR;
    }

    if (awb_com->awb_gain_update) {
        pAwbHdl->mWbGainSyncFlag = shared->frameId;
        pGainBase->sync_flag = pAwbHdl->mWbGainSyncFlag;
		cur_params->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM] = params->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM];
        pGainBase->is_update = true;
        awb_com->awb_gain_update = false;
        LOGD_AWB("[%d] wbgain params from algo", pAwbHdl->mWbGainSyncFlag);
    } else if (pAwbHdl->mWbGainSyncFlag != pGainBase->sync_flag) {
        pGainBase->sync_flag = pAwbHdl->mWbGainSyncFlag;
        if (cur_params->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM]) {
            pGainBase->is_update = true;
			*(rk_aiq_isp_awb_gain_params_t*)(pGainBase->_data) =
				*(rk_aiq_isp_awb_gain_params_t*)(cur_params->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM]->_data);
        } else {
            LOGE_AWB("no latest params !");
            pGainBase->is_update = false;
        }
        LOGD_AWB("[%d] wbgain from latest [%d]", shared->frameId, pAwbHdl->mWbGainSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        pGainBase->is_update = false;
        LOGD_AWB("[%d] wbgain params needn't update", shared->frameId);
    }

    if (awb_com->awb_cfg_update) {
        pAwbHdl->mWbParamSyncFlag = shared->frameId;
        pAwbBase->sync_flag = pAwbHdl->mWbParamSyncFlag;
		cur_params->pParamsArray[RESULT_TYPE_AWB_PARAM] = params->pParamsArray[RESULT_TYPE_AWB_PARAM];
        pAwbBase->is_update = true;
        awb_com->awb_cfg_update = false;
        LOGD_AWB("[%d] params from algo", pAwbHdl->mWbParamSyncFlag);
    } else if (pAwbHdl->mWbParamSyncFlag != pAwbBase->sync_flag) {
        pAwbBase->sync_flag = pAwbHdl->mWbParamSyncFlag;
        // copy from latest result
        if (cur_params->pParamsArray[RESULT_TYPE_AWB_PARAM]) {
            pAwbBase->is_update = true;
			*(rk_aiq_isp_awb_params_t*)(pAwbBase->_data) =
				*(rk_aiq_isp_awb_params_t*)(cur_params->pParamsArray[RESULT_TYPE_AWB_PARAM]->_data);
        } else {
            LOGE_AWB("no latest params !");
            pAwbBase->is_update = false;
        }
        LOGD_AWB("[%d] params from latest [%d]", shared->frameId, pAwbHdl->mWbParamSyncFlag);
    } else {
        pAwbBase->is_update = false;
        // do nothing, result in buf needn't update
        LOGD_AWB("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerAwb_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerAwb_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerAwb_processing;
    pHdl->genIspResult = _handlerAwb_genIspResult;
    pHdl->prepare      = _handlerAwb_prepare;
    pHdl->init         = _handlerAwb_init;
    pHdl->deinit       = _handlerAwb_deinit;
	return pHdl;
}

XCamReturn AiqAlgoHandlerAwb_setAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_api_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_getAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_api_attrib_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_setWbGainCtrlAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_gainCtrl_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetWbGainCtrlAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_getWbGainCtrlAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_gainCtrl_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetWbGainCtrlAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_setAwbStatsAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_Stats_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetAwbStatsAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_getAwbStatsAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_Stats_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAwbStatsAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_setAwbGnCalcStepAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_gainCalcStep_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetAwbGnCalcStepAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_getAwbGnCalcStepAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_gainCalcStep_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAwbGnCalcStepAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_setAwbGnCalcOthAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_gainCalcOth_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetAwbGnCalcOthAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_getAwbGnCalcOthAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, awb_gainCalcOth_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAwbGnCalcOthAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn AiqAlgoHandlerAwb_writeAwbIn(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_awb_wrtIn_attr_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapiV3_awb_WriteInput(pHdl->mAlgoCtx, att, false);

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn AiqAlgoHandlerAwb_awbIqMap2Main(AiqAlgoHandlerAwb_t* pAwbHdl,  rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapiV3_awb_IqMap2Main(pHdl->mAlgoCtx, att, false);

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_setAwbPreWbgain(AiqAlgoHandlerAwb_t* pAwbHdl, const float att[4])
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapiV3_awb_SetPreWbgain(pHdl->mAlgoCtx, att, false);

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn AiqAlgoHandlerAwb_queryWBInfo(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_wb_querry_info_t* wb_querry_info)
{
    ENTER_ANALYZER_FUNCTION();

	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV3_awb_QueryWBInfo(pHdl->mAlgoCtx, wb_querry_info);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}

XCamReturn AiqAlgoHandlerAwb_lock(AiqAlgoHandlerAwb_t* pAwbHdl)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;

    rk_aiq_uapiV3_awb_Lock(pHdl->mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_unlock(AiqAlgoHandlerAwb_t* pAwbHdl)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;

    rk_aiq_uapiV3_awb_Unlock(pHdl->mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}




XCamReturn AiqAlgoHandlerAwb_setFFWbgainAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_awb_ffwbgain_attr_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetFstFrWbgain(pHdl->mAlgoCtx, att.wggain, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_getAlgoStat(AiqAlgoHandlerAwb_t* pAwbHdl, rk_tool_awb_stat_res_full_t *awb_stat_algo)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAlgoStat(pHdl->mAlgoCtx, awb_stat_algo);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}

XCamReturn AiqAlgoHandlerAwb_getStrategyResult(AiqAlgoHandlerAwb_t* pAwbHdl, rk_tool_awb_strategy_result_t *awb_strategy_result)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetStrategyResult(pHdl->mAlgoCtx, awb_strategy_result);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAwb_SetNNres(AiqAlgoHandlerAwb_t* pAwbHdl, const awb_ai_res_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapiV3_awb_SetNNres(pHdl->mAlgoCtx, att, false);

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
