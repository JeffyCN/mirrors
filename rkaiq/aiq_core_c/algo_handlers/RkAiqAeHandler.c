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
#include "RkAiqAeHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "RkAiqMergeHandler.h"
#include "RkAiqDrcHandler.h"
#include "RkAiqBlcHandler.h"
#include "RkAiqAfdHandler.h"
#include "RkAiqAfHandler.h"
#include "ae/rk_aiq_uapi_ae_int.h"

static void _handlerAe_deinit(AiqAlgoHandler_t* pHdl) {
    AiqAlgoHandler_deinit(pHdl);
    AiqAlgoHandlerAe_t* pAeHdl = (AiqAlgoHandlerAe_t*)pHdl;
    aiqMutex_deInit(&pAeHdl->mLockAebyAfMutex);
    aiqMutex_deInit(&pAeHdl->mGetAfdResMutex);
}

static void _handlerAe_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoConfigAe)));
    pHdl->mPreInParam   = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoPreAe)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcAe)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResAe)));
    pHdl->mPostInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoPostAe)));
    pHdl->mPostOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoPostResAe)));

    pHdl->mResultType = RESULT_TYPE_AEC_PARAM;
    //pHdl->mResultSize = sizeof(ae_param_t);

    AiqAlgoHandlerAe_t* pAeHdl = (AiqAlgoHandlerAe_t*)pHdl;
    pAeHdl->mMeasSyncFlag = -1;
    pAeHdl->mHistSyncFlag = -1;
    aiqMutex_init(&pAeHdl->mLockAebyAfMutex);
    aiqMutex_init(&pAeHdl->mGetAfdResMutex);

#if RKAIQ_HAVE_AF
    pAeHdl->mAf_handle = pHdl->mAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AF];
#endif
#if RKAIQ_HAVE_AFD_V1 || RKAIQ_HAVE_AFD_V2
    pAeHdl->mAfd_handle = pHdl->mAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AFD];
#endif

    pAeHdl->mAmerge_handle = pHdl->mAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AMERGE];
    pAeHdl->mAdrc_handle = pHdl->mAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_ADRC];
    pAeHdl->mAblc_handle = pHdl->mAiqCore->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_ABLC];

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerAe_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "ae handle prepare failed");

    RkAiqAlgoConfigAe* ae_config_int = (RkAiqAlgoConfigAe*)pAlgoHandler->mConfig;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    // TODO config ae common params:

    /*****************AecConfig Sensor Exp related info*****************/
    ae_config_int->LinePeriodsPerField = (float)sharedCom->snsDes.frame_length_lines;
    ae_config_int->PixelPeriodsPerLine = (float)sharedCom->snsDes.line_length_pck;
    ae_config_int->PixelClockFreqMHZ   = (float)sharedCom->snsDes.pixel_clock_freq_mhz;

    /*****************AecConfig pic-info params*****************/
    ae_config_int->RawWidth  = sharedCom->snsDes.isp_acq_width;
    ae_config_int->RawHeight = sharedCom->snsDes.isp_acq_height;
    ae_config_int->nr_switch = sharedCom->snsDes.nr_switch;
    ae_config_int->compr_bit = sharedCom->snsDes.compr_bit;
    ae_config_int->dcg_ratio = sharedCom->snsDes.dcg_ratio;

    aiqMutex_lock(&pAlgoHandler->mCfgMutex);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
    RKAIQCORE_CHECK_RET(ret, "ae algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerAe_preProcess(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandlerAe_t* pAeHdl = (AiqAlgoHandlerAe_t*)pAlgoHandler;

    RkAiqAlgoPreAe* ae_pre_int = (RkAiqAlgoPreAe*)pAlgoHandler->mPreInParam;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    aiq_stats_base_t* xAecStats = NULL;
    if (shared->aecStatsBuf) {
        xAecStats = shared->aecStatsBuf;
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGW_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->bValid) && !sharedCom->init) {
        LOGW("fid:%d no aec stats, ignore!", shared->frameId);
        return XCAM_RETURN_BYPASS;
    }

    if (pAeHdl->mPreResShared) {
        /* dec ref from aiqPool_getFree */
        AIQ_REF_BASE_UNREF(&pAeHdl->mPreResShared->_ref_base);
        pAeHdl->mPreResShared = NULL;
    }

    int algoId = pAlgoHandler->mDes->id;
    if (algoId == 0) {
        AiqPoolItem_t* pItem =
            aiqPool_getFree(pAlgoHandler->mAiqCore->mPreResAeSharedPool);
        if (pItem) {
            pAeHdl->mPreResShared = (AlgoRstShared_t*)pItem->_pData;
        } else {
#if RKAIQ_HAVE_DUMPSYS
            pAlgoHandler->mAiqCore->mNoFreeBufCnt.aePreRes++;
#endif
        }
    }

    if (!pAeHdl->mPreResShared) {
        LOGE("new ae mPreOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }

    ret = AiqAlgoHandler_preProcess(pAlgoHandler);
    if (ret < 0) {
        LOGE_ANALYZER("ae handle preProcess failed ret %d", ret);
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        return ret;
    }

    aiq_ae_stats_wrapper_t* pAeStatsWrap = NULL;
    if (xAecStats)
        pAeStatsWrap = (aiq_ae_stats_wrapper_t*)xAecStats->_data;
#ifdef USE_NEWSTRUCT
    if (pAeStatsWrap) {
        ae_pre_int->aecStatsV25Buf = &pAeStatsWrap->aec_stats_v25;
        ae_pre_int->af_prior = pAeStatsWrap->af_prior;
    } else {
        ae_pre_int->aecStatsBuf = NULL;
        ae_pre_int->af_prior = false;
    }
#else
    if (pAeStatsWrap) {
        ae_pre_int->aecStatsBuf = &pAeStatsWrap->aec_stats;
        ae_pre_int->af_prior = pAeStatsWrap->af_prior;
    } else {
        ae_pre_int->aecStatsBuf = NULL;
        ae_pre_int->af_prior = false;
    }
#endif

    if (algoId == 0) {
        AeInstanceConfig_t* pAeInstConfig           = (AeInstanceConfig_t*)pAlgoHandler->mAlgoCtx;
        if(pAeInstConfig->cbs == NULL) {
            aiqMutex_lock(&pAeHdl->mGetAfdResMutex);
            pAeInstConfig->aecCfg->AfdRes = pAeHdl->mAfdRes;
            aiqMutex_unlock(&pAeHdl->mGetAfdResMutex);
        }
    }
    ae_pre_int->amtdRes = shared->amtdRes;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
        if (des->pre_process) {
            aiqMutex_lock(&pAlgoHandler->mCfgMutex);
            ret = des->pre_process(pAlgoHandler->mPreInParam,
                                   (RkAiqAlgoResCom*)(pAeHdl->mPreResShared->_data));
            aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
            if (ret < 0) {
                LOGE_ANALYZER("ae handle pre_process failed ret %d", ret);
                return ret;
            } else if (ret == XCAM_RETURN_BYPASS) {
                LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
                return ret;
            }
        }
    }

    if (pAlgoHandler->mPostShared) {
        if (pAlgoHandler->mAiqCore->mAlogsComSharedParams.init) {
            RkAiqAlgosGroupShared_t* grpShared = NULL;
            for (uint32_t i = 0; i < RK_AIQ_CORE_ANALYZE_MAX; i++) {
                grpShared = pAlgoHandler->mAiqCore->mAlogsGroupSharedParamsMap[i];
                if (grpShared) {
                    grpShared->res_comb.ae_pre_res_c = pAeHdl->mPreResShared;
                }
            }
        } else {
            pAeHdl->mPreResShared->frame_id = shared->frameId;
            pAeHdl->mPreResShared->type = XCAM_MESSAGE_AE_PRE_RES_OK;
            AiqCoreMsg_t msg;
            msg.msg_id                    = XCAM_MESSAGE_AE_PRE_RES_OK;
            msg.frame_id                  = shared->frameId;
            *(AlgoRstShared_t**)(msg.buf) = pAeHdl->mPreResShared;
            AiqCore_post_message(pAlgoHandler->mAiqCore, &msg);
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerAe_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandlerAe_t* pAeHdl = (AiqAlgoHandlerAe_t*)pAlgoHandler;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcAe* ae_proc_int        = (RkAiqAlgoProcAe*)pAlgoHandler->mProcInParam;
    RkAiqAlgoProcResAe* ae_proc_res_int        = (RkAiqAlgoProcResAe*)pAlgoHandler->mProcOutParam;

    aiq_stats_base_t* xAecStats = NULL;
    if (shared->aecStatsBuf) {
        xAecStats = shared->aecStatsBuf;
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGW_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->bValid) && !sharedCom->init) {
        LOGW("fid:%d no aec stats, ignore!", shared->frameId);
        return XCAM_RETURN_BYPASS;
    }

    int algoId = pAlgoHandler->mDes->id;
    if (algoId == 0) {
        AeInstanceConfig_t* pAeInstConfig           = (AeInstanceConfig_t*)pAlgoHandler->mAlgoCtx;
        if(pAeInstConfig->cbs == NULL) {
            aiqMutex_lock(&pAeHdl->mLockAebyAfMutex);
            pAeInstConfig->lockaebyaf = pAeHdl->lockaebyaf;
            aiqMutex_unlock(&pAeHdl->mLockAebyAfMutex);
        }
    }

    AiqAecExpInfoWrapper_t* pExpWrap =
        (AiqAecExpInfoWrapper_t*)(shared->fullParams->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->_data);
    ae_proc_res_int->new_ae_exp = &pExpWrap->new_ae_exp;
    ae_proc_res_int->ae_proc_res_rk = &pExpWrap->ae_proc_res_rk;
    ae_proc_res_int->exp_i2c_params = &pExpWrap->exp_i2c_params;

    ae_proc_res_int->ae_stats_cfg =
        (aeStats_cfg_t*)(shared->fullParams->pParamsArray[RESULT_TYPE_AESTATS_PARAM]->_data);

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret < 0) {
        LOGE_ANALYZER("ae handle processing failed ret %d", ret);
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        return ret;
    }

    // TODO config common ae processing params
    aiq_ae_stats_wrapper_t* pAeStatsWrap = NULL;
    if (xAecStats)
        pAeStatsWrap = (aiq_ae_stats_wrapper_t*)xAecStats->_data;
#if USE_NEWSTRUCT
    if (pAeStatsWrap) {
        ae_proc_int->aecStatsV25Buf = &pAeStatsWrap->aec_stats_v25;
    } else {
        ae_proc_int->aecStatsV25Buf = NULL;
    }
#else
    if (pAeStatsWrap) {
        ae_proc_int->aecStatsBuf = &pAeStatsWrap->aec_stats;
    } else {
        ae_proc_int->aecStatsBuf = NULL;
    }
#endif

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
        if (des->processing) {
            aiqMutex_lock(&pAlgoHandler->mCfgMutex);
            ret = des->processing(pAlgoHandler->mProcInParam, (RkAiqAlgoResCom*)(pAlgoHandler->mProcOutParam));
            aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
            if (ret < 0) {
                LOGE_ANALYZER("ae algo processing failed ret %d", ret);
                return ret;
            } else if (ret == XCAM_RETURN_BYPASS) {
                LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
                return ret;
            }
        }
    }

    if (pAlgoHandler->mAiqCore->mAlogsComSharedParams.init) {
        RkAiqAlgosGroupShared_t* measGroupshared =
            pAlgoHandler->mAiqCore->mAlogsGroupSharedParamsMap[RK_AIQ_CORE_ANALYZE_MEAS];
        if (measGroupshared) {
            measGroupshared->frameId                 = shared->frameId;
        }

        int hdr_iso[3] = {0};
        if (pAlgoHandler->mAiqCore->mAlogsComSharedParams.hdr_mode == 0) {
            hdr_iso[0] = 50 *
                         ae_proc_res_int->new_ae_exp->LinearExp.exp_real_params.analog_gain *
                         ae_proc_res_int->new_ae_exp->LinearExp.exp_real_params.digital_gain *
                         ae_proc_res_int->new_ae_exp->LinearExp.exp_real_params.isp_dgain;
        } else {
            for(int i = 0; i < 3; i++) {
                hdr_iso[i] = 50 *
                             ae_proc_res_int->new_ae_exp->HdrExp[i].exp_real_params.analog_gain *
                             ae_proc_res_int->new_ae_exp->HdrExp[i].exp_real_params.digital_gain *
                             ae_proc_res_int->new_ae_exp->HdrExp[i].exp_real_params.isp_dgain;
            }
        }

        /* Transfer the initial exposure to other algorithm modules */
        RkAiqAlgosGroupShared_t* grpShared = NULL;
        for (int type = RK_AIQ_CORE_ANALYZE_MEAS; type < RK_AIQ_CORE_ANALYZE_MAX; \
                type++) {
            grpShared = pAlgoHandler->mAiqCore->mAlogsGroupSharedParamsMap[type];
            if (grpShared) {
                grpShared->preExp = *ae_proc_res_int->new_ae_exp;
                grpShared->curExp = *ae_proc_res_int->new_ae_exp;
                grpShared->nxtExp = *ae_proc_res_int->new_ae_exp;
                grpShared->iso    = hdr_iso[0];
            }

        }
    }

#if RKAIQ_HAVE_AF
    if (pAeHdl->mAf_handle) {
        AiqAlgoHandlerAf_t* af_algo = (AiqAlgoHandlerAf_t*)(pAeHdl->mAf_handle);
        AiqAlgoHandlerAf_setAeStable(af_algo, ae_proc_res_int->ae_proc_res_rk->IsConverged);
    }
#endif

    RkAiqAlgoProcResAeShared_t aeProcResShared;

    aeProcResShared.IsConverged = ae_proc_res_int->ae_proc_res_rk->IsConverged;
    aeProcResShared.IsEnvChanged = ae_proc_res_int->ae_proc_res_rk->IsEnvChanged;
    aeProcResShared.IsAutoAfd =  ae_proc_res_int->ae_proc_res_rk->IsAutoAfd;
    aeProcResShared.LongFrmMode =  ae_proc_res_int->ae_proc_res_rk->LongFrmMode;

#if RKAIQ_HAVE_AFD_V1 || RKAIQ_HAVE_AFD_V2
    if (pAeHdl->mAfd_handle) {
        AiqAfdHandler_t* afd_algo = (AiqAfdHandler_t*)(pAeHdl->mAfd_handle);

        AiqAfdHandler_setAeProcRes((AiqAfdHandler_t*)afd_algo, &aeProcResShared);
    }
#endif

    if (pAeHdl->mAmerge_handle) {
        AiqAlgoHandlerMerge_t* Merge_algo = (AiqAlgoHandlerMerge_t*)(pAeHdl->mAmerge_handle);
        AiqAlgoHandlerMerge_setAeProcRes(Merge_algo, &aeProcResShared);
    }

    if (pAeHdl->mAdrc_handle) {
        AiqDrcHandler_t* Drc_algo = (AiqDrcHandler_t*)(pAeHdl->mAdrc_handle);
        AiqDrcHandler_setAeProcRes(Drc_algo, &aeProcResShared);
    }

    if (pAeHdl->mAblc_handle) {
        AiqAlgoHandlerBlc_t* Blc_algo = (AiqAlgoHandlerBlc_t*)(pAeHdl->mAblc_handle);
        AiqBlcHandler_setAeProcRes(Blc_algo, &aeProcResShared);
    }

    AiqStatsTranslator_t* translator = pAlgoHandler->mAiqCore->mTranslator;
    if (ae_proc_res_int->stats_cfg_to_trans.isUpdate) {
        AiqStatsTranslator_setAeAlgoStatsCfg(translator, &ae_proc_res_int->stats_cfg_to_trans);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

static XCamReturn _handlerAe_postProcess(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqAlgoHandler_postProcess(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ae handle postProcess failed");
        return ret;
    }
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    if (des->post_process)
        ret                       = des->post_process(pAlgoHandler->mPostInParam, pAlgoHandler->mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "ae algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

static XCamReturn _handlerAe_genIspResult(AiqAlgoHandler_t* pAlgoHandler, AiqFullParams_t* params,
        AiqFullParams_t* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    AiqAlgoHandlerAe_t* pAeHdl = (AiqAlgoHandlerAe_t*)pAlgoHandler;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResAe* ae_proc                 = (RkAiqAlgoProcResAe*)pAlgoHandler->mProcOutParam;
    RkAiqAlgoPostResAe* ae_post                 = (RkAiqAlgoPostResAe*)pAlgoHandler->mPostOutParam;

    int algo_id = pAlgoHandler->mDes->id;

    if (pAlgoHandler->mIsMulRun && (algo_id == 0)) {
        // do nothing for rkawb if custom algo running with rk algo
        return ret;
    }

    aiq_params_base_t* pBase = params->pParamsArray[RESULT_TYPE_AESTATS_PARAM];
    if (!pBase) {
        LOGW_AEC("no ae params buf !");
        return XCAM_RETURN_BYPASS;
    }

    rk_aiq_isp_ae_stats_cfg_t* ae_stats_cfg   = (rk_aiq_isp_ae_stats_cfg_t*)pBase->_data;
    // TODO: Why need to copy again?
    //ae_stats_cfg->result = *ae_proc->ae_stats_cfg;
    // memcpy(ae_stats_cfg, ae_proc->ae_stats_cfg, sizeof(*ae_stats_cfg));
    if (sharedCom->init) {
        pBase->frame_id  = 0;
    } else {
        pBase->frame_id  = shared->frameId;
    }

    aiq_params_base_t* pCurBase            = cur_params->pParamsArray[RESULT_TYPE_AESTATS_PARAM];
    rk_aiq_isp_ae_stats_cfg_t* cur_ae_stats_cfg  = NULL;
    if (pCurBase)
        cur_ae_stats_cfg  = (rk_aiq_isp_ae_stats_cfg_t*)pCurBase->_data;

    if (sharedCom->init) {
        pBase->frame_id = 0;
    } else {
        pBase->frame_id = shared->frameId;
    }

    AiqAecExpInfoWrapper_t* exp_param =
        (AiqAecExpInfoWrapper_t*)(params->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->_data);

    AiqIrisInfoWrapper_t* iris_param    =
        (AiqIrisInfoWrapper_t*)(params->pParamsArray[RESULT_TYPE_IRIS_PARAM]->_data);

    if (sharedCom->init) {
        params->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->frame_id = 0;
    } else {
        params->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]->frame_id = shared->frameId;
    }

    if (AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
        cur_params->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM] = params->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM];
        cur_params->pParamsArray[RESULT_TYPE_IRIS_PARAM] = params->pParamsArray[RESULT_TYPE_IRIS_PARAM];
        cur_params->pParamsArray[RESULT_TYPE_AESTATS_PARAM] = params->pParamsArray[RESULT_TYPE_AESTATS_PARAM];
        return XCAM_RETURN_NO_ERROR;
    }

    // exposure
    exp_param->algo_id      = 0;//algo_id;
    cur_params->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM] = params->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM];
    // iris
    if (algo_id == 0) {
        RkAiqAlgoPostResAe* ae_post_rk = (RkAiqAlgoPostResAe*)ae_post;
        // HDC iris control
        iris_param->DCIris.update = ae_post_rk->ae_post_res_rk.DCIris.update;
        iris_param->DCIris.pwmDuty = ae_post_rk->ae_post_res_rk.DCIris.pwmDuty;
        // HDC iris control
        iris_param->HDCIris.update = ae_post_rk->ae_post_res_rk.HDCIris.update;
        iris_param->HDCIris.target = ae_post_rk->ae_post_res_rk.HDCIris.target;
        ae_post_rk->ae_post_res_rk.HDCIris.adc = iris_param->HDCIris.adc;
        ae_post_rk->ae_post_res_rk.HDCIris.zoomPos = iris_param->HDCIris.zoomPos;
        cur_params->pParamsArray[RESULT_TYPE_IRIS_PARAM] = params->pParamsArray[RESULT_TYPE_IRIS_PARAM];
    }

    // meas
    if (pAeHdl->useStatsApiCfg) {
        if (pAeHdl->updateStatsApiCfg) {
            pAeHdl->mMeasSyncFlag = shared->frameId;
            *ae_stats_cfg = pAeHdl->mStatsApiCfg;
            pBase->sync_flag = pAeHdl->mMeasSyncFlag;

            cur_params->pParamsArray[RESULT_TYPE_AESTATS_PARAM] = params->pParamsArray[RESULT_TYPE_AESTATS_PARAM];
            pBase->is_update = true;
            LOGK_AEC("[%d] update stats cfg from api", pAeHdl->mMeasSyncFlag);
        }
    } else {
        if (ae_proc->ae_stats_cfg->sw_aeCfg_update) {
            pAeHdl->mMeasSyncFlag = shared->frameId;
            pBase->sync_flag = pAeHdl->mMeasSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->pParamsArray[RESULT_TYPE_AESTATS_PARAM] = params->pParamsArray[RESULT_TYPE_AESTATS_PARAM];
            pBase->is_update = true;
            ae_proc->ae_stats_cfg->sw_aeCfg_update = false;

            LOGD_AEC("[%d] new aecfg params from algo", pAeHdl->mMeasSyncFlag);
        } else if (pAeHdl->mMeasSyncFlag != pBase->sync_flag) {
            pBase->sync_flag = pAeHdl->mMeasSyncFlag;
            // copy from latest result
            if (cur_params->pParamsArray[RESULT_TYPE_AESTATS_PARAM]) {
                *ae_stats_cfg = *cur_ae_stats_cfg;
                pBase->is_update = true;
            } else {
                LOGE_AEC("no new aecfg meas params !");
                pBase->is_update = false;
            }
            LOGV_AEC("[%d] meas new aecfg from latest [%d]", shared->frameId, pAeHdl->mMeasSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            pBase->is_update = false;
            LOGV_AEC("[%d] meas new aecfg needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerAe_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerAe_t));
    if (!pHdl)
        return NULL;
    AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->preProcess   = _handlerAe_preProcess;
    pHdl->processing   = _handlerAe_processing;
    pHdl->postProcess  = _handlerAe_postProcess;
    pHdl->genIspResult = _handlerAe_genIspResult;
    pHdl->prepare      = _handlerAe_prepare;
    pHdl->init         = _handlerAe_init;
    pHdl->deinit       = _handlerAe_deinit;
    return pHdl;
}

#ifndef USE_NEWSTRUCT
XCamReturn AiqAlgoHandlerAe_setExpSwAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpSwAttrV2_t ExpSwAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_setExpSwAttr(pHdl->mAlgoCtx, &ExpSwAttrV2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_EXPSWATTR;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getExpSwAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpSwAttrV2_t* pExpSwAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getExpSwAttr(pHdl->mAlgoCtx, pExpSwAttrV2, false);
    pExpSwAttrV2->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setLinExpAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_LinExpAttrV2_t LinExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_setLinExpAttr(pHdl->mAlgoCtx, &LinExpAttrV2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_LINEXPATTR;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getLinExpAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_LinExpAttrV2_t* pLinExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getLinExpAttr(pHdl->mAlgoCtx, pLinExpAttrV2, false);
    pLinExpAttrV2->sync.done = true;

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setHdrExpAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_HdrExpAttrV2_t HdrExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setHdrExpAttr(pHdl->mAlgoCtx, &HdrExpAttrV2, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_HDREXPATTR;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getHdrExpAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_HdrExpAttrV2_t* pHdrExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getHdrExpAttr(pHdl->mAlgoCtx, pHdrExpAttrV2, false);
    pHdrExpAttrV2->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setLinAeRouteAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_LinAeRouteAttr_t LinAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setLinAeRouteAttr(pHdl->mAlgoCtx, &LinAeRouteAttr, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_LINAEROUTEATTR;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getLinAeRouteAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_LinAeRouteAttr_t* pLinAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getLinAeRouteAttr(pHdl->mAlgoCtx, pLinAeRouteAttr, false);
    pLinAeRouteAttr->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setHdrAeRouteAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_HdrAeRouteAttr_t HdrAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setHdrAeRouteAttr(pHdl->mAlgoCtx, &HdrAeRouteAttr, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_HDRAEROUTEATTR;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getHdrAeRouteAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getHdrAeRouteAttr(pHdl->mAlgoCtx, pHdrAeRouteAttr, false);
    pHdrAeRouteAttr->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setIrisAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_IrisAttrV2_t IrisAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setIrisAttr(pHdl->mAlgoCtx, &IrisAttr, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_IRISATTR;

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getIrisAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_IrisAttrV2_t* pIrisAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getIrisAttr(pHdl->mAlgoCtx, pIrisAttr);
    pIrisAttr->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setSyncTestAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_AecSyncTest_t SyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setSyncTest(pHdl->mAlgoCtx, &SyncTestAttr, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_SYNCTESTATTR;

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getSyncTestAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_AecSyncTest_t* pSyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getSyncTest(pHdl->mAlgoCtx, pSyncTestAttr, false);
    pSyncTestAttr->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_queryExpInfo(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpQueryInfo_t* pExpQueryInfo)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;

    rk_aiq_uapi_ae_queryExpInfo(pHdl->mAlgoCtx, pExpQueryInfo, false);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#else
XCamReturn AiqAlgoHandlerAe_setExpSwAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_expSwAttr_t ExpSwAttrV2)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_setExpSwAttr(pHdl->mAlgoCtx, &ExpSwAttrV2, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getExpSwAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_expSwAttr_t* pExpSwAttrV2)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_getExpSwAttr(pHdl->mAlgoCtx, pExpSwAttrV2, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setLinExpAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_linExpAttr_t LinExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_setLinExpAttr(pHdl->mAlgoCtx, &LinExpAttrV2, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getLinExpAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_linExpAttr_t* pLinExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_getLinExpAttr(pHdl->mAlgoCtx, pLinExpAttrV2, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setHdrExpAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_hdrExpAttr_t HdrExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_setHdrExpAttr(pHdl->mAlgoCtx, &HdrExpAttrV2, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getHdrExpAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_hdrExpAttr_t* pHdrExpAttrV2)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_getHdrExpAttr(pHdl->mAlgoCtx, pHdrExpAttrV2, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setIrisAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_irisAttr_t IrisAttr)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_setIrisAttr(pHdl->mAlgoCtx, &IrisAttr);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getIrisAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_irisAttr_t* pIrisAttr)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_getIrisAttr(pHdl->mAlgoCtx, pIrisAttr);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setSyncTestAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_syncTestAttr_t SyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_setSyncTest(pHdl->mAlgoCtx, &SyncTestAttr, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getSyncTestAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_syncTestAttr_t* pSyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_getSyncTest(pHdl->mAlgoCtx, pSyncTestAttr, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_queryExpInfo(AiqAlgoHandlerAe_t* pAeHdl, ae_api_queryInfo_t* pExpQueryInfo)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    rk_aiq_uapi_ae_queryExpInfo(pHdl->mAlgoCtx, pExpQueryInfo, false);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getRkAeStats(AiqAlgoHandlerAe_t* pAeHdl, Uapi_RkAeStats_t* pRkAeStats)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    rk_aiq_uapi_ae_getRKAeStats(pHdl->mAlgoCtx, pRkAeStats, false);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn AiqAlgoHandlerAe_setLockAeForAf(AiqAlgoHandlerAe_t* pAeHdl, bool lock_ae)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pAeHdl->mLockAebyAfMutex);
    pAeHdl->lockaebyaf = lock_ae;
    aiqMutex_unlock(&pAeHdl->mLockAebyAfMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getAfdResForAE(AiqAlgoHandlerAe_t* pAeHdl, AfdPeakRes_t AfdRes)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pAeHdl->mGetAfdResMutex);
    pAeHdl->mAfdRes = AfdRes;
    aiqMutex_unlock(&pAeHdl->mGetAfdResMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setExpWinAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpWin_t ExpWinAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setExpWinAttr(pHdl->mAlgoCtx, &ExpWinAttr, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_EXPWINATTR;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn AiqAlgoHandlerAe_getExpWinAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpWin_t* pExpWinAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getExpWinAttr(pHdl->mAlgoCtx, pExpWinAttr);
    pExpWinAttr->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn AiqAlgoHandlerAe_setExpSubWinAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpSubWin_t ExpSubWinAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setExpSubWinAttr(pHdl->mAlgoCtx, &ExpSubWinAttr, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_EXPSUBWINATTR;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn AiqAlgoHandlerAe_getExpSubWinAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpSubWin_t* pExpSubWinAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getExpSubWinAttr(pHdl->mAlgoCtx, pExpSubWinAttr);
    pExpSubWinAttr->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setFrameHdrAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_FrameHdrAttr_t FrameHdrAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setFrameHdrAttr(pHdl->mAlgoCtx, &FrameHdrAttr, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_FRAMEHDRATTR;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn AiqAlgoHandlerAe_getFrameHdrAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_FrameHdrAttr_t* pFrameHdrAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getFrameHdrAttr(pHdl->mAlgoCtx, pFrameHdrAttr, false);
    pFrameHdrAttr->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setAecStatsCfg(AiqAlgoHandlerAe_t* pAeHdl, Uapi_AecStatsCfg_t AecStatsCfg)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_setAecStatsCfg(pHdl->mAlgoCtx, &AecStatsCfg, false, false);
    AeInstanceConfig_t* pAeInstConfig = (AeInstanceConfig_t*)pHdl->mAlgoCtx;
    AeConfig_t pAecCfg                = pAeInstConfig->aecCfg;
    pAecCfg->IsReconfig |= UPDATE_AECSTATSCFG;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_getAecStatsCfg(AiqAlgoHandlerAe_t* pAeHdl, Uapi_AecStatsCfg_t* pAecStatsCfg)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapi_ae_getAecStatsCfg(pHdl->mAlgoCtx, pAecStatsCfg, false);
    pAecStatsCfg->sync.done = true;
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setAOVForAE(AiqAlgoHandlerAe_t* pAeHdl, bool en)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    rk_aiq_uapi_ae_setAOV(pHdl->mAlgoCtx, en);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerAe_setStatsApiCfg(AiqAlgoHandlerAe_t* pAeHdl, rk_aiq_op_mode_t mode, aeStats_cfg_t *cfg)
{
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    if (mode == RK_AIQ_OP_MODE_AUTO) {
        LOGK("setStatsApiCfg use algo stats.");
        pAeHdl->useStatsApiCfg = false;
    }
    if (mode == RK_AIQ_OP_MODE_MANUAL) {
        LOGK("setStatsApiCfg use api stats.");
        pAeHdl->useStatsApiCfg = true;
        pAeHdl->updateStatsApiCfg = true;
        pAeHdl->mStatsApiCfg = *cfg;
    }
    aiqMutex_unlock(&pHdl->mCfgMutex);
    return XCAM_RETURN_NO_ERROR;
}
