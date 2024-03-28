/*
 * rk_aiq_algo_camgroup_adpcc_itf.c
 *
 *  Copyright (c) 2021 Rockchip Corporation
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
 *
 */

#include "rk_aiq_algo_camgroup_types.h"
#include "misc/rk_aiq_algo_camgroup_misc_itf.h"
#include "adpcc/rk_aiq_algo_adpcc_itf.h"
#include "adpcc/rk_aiq_adpcc_algo.h"
#include "adpcc/rk_aiq_types_adpcc_algo_prvt.h"


RKAIQ_BEGIN_DECLARE


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ADPCC("%s: (enter)", __FUNCTION__ );
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;

    AdpccContext_t* pAdpccCtx = NULL;
    AdpccResult_t ret = AdpccInit(&pAdpccCtx, (CamCalibDbV2Context_t*)(cfgInt->s_calibv2));//load json paras
    if(ret != ADPCC_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ADPCC("%s: Initializaion Adpcc failed (%d)", __FUNCTION__, ret);
    } else {
        *context = (RkAiqAlgoContext *)(pAdpccCtx);
    }

    LOG1_ADPCC("%s: (exit)", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ADPCC("%s: (enter)", __FUNCTION__ );

#if 1
    AdpccContext_t* pAdpccCtx = (AdpccContext_t*)context;
    AdpccResult_t ret = AdpccRelease(pAdpccCtx);
    if(ret != ADPCC_RET_SUCCESS) {
        result = XCAM_RETURN_ERROR_FAILED;
        LOGE_ADPCC("%s: release Adpcc failed (%d)", __FUNCTION__, ret);
    }
#endif

    LOG1_ADPCC("%s: (exit)", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ADPCC("%s: (enter)", __FUNCTION__ );

    AdpccContext_t* pAdpccCtx = (AdpccContext_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* pCfgParam = (RkAiqAlgoCamGroupPrepare*)params;
    pAdpccCtx->prepare_type = params->u.prepare.conf_type;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        AdpccResult_t ret = AdpccReloadPara(pAdpccCtx, (CamCalibDbV2Context_t*)(pCfgParam->s_calibv2));
        if(ret != ADPCC_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ADPCC("%s: Adpcc Reload Para failed (%d)", __FUNCTION__, ret);
        }
    }

    pAdpccCtx->isReCal_ = true;

    LOG1_ADPCC("%s: (exit)", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ADPCC("%s: (enter)", __FUNCTION__ );

#if 1
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    AdpccContext_t* pAdpccCtx = (AdpccContext_t *)inparams->ctx;
    AdpccExpInfo_t stExpInfo;
    memset(&stExpInfo, 0x00, sizeof(AdpccExpInfo_t));

    LOGD_ADPCC("%s:%d init:%d hdr mode:%d",
               __FUNCTION__, __LINE__,
               inparams->u.proc.init,
               procParaGroup->working_mode);

    stExpInfo.hdr_mode = 0; //pAnrProcParams->hdr_mode;
    for(int i = 0; i < 3; i++) {
        stExpInfo.arPreResIso[i] = 50;
        stExpInfo.arPreResAGain[i] = 1.0;
        stExpInfo.arPreResDGain[i] = 1.0;
        stExpInfo.arPreResTime[i] = 0.01;

        stExpInfo.arProcResIso[i] = 50;
        stExpInfo.arProcResAGain[i] = 1.0;
        stExpInfo.arProcResDGain[i] = 1.0;
        stExpInfo.arProcResTime[i] = 0.01;
    }

    pAdpccCtx->isBlackSensor = procParaGroup->_is_bw_sensor;

    if(procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        stExpInfo.hdr_mode = 0;
    } else if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR
              || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR ) {
        stExpInfo.hdr_mode = 1;
    } else if(procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR
              || procParaGroup->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR ) {
        stExpInfo.hdr_mode = 2;
    }

    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];

    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pAERes = &scam_3a_res->aec._effAecExpInfo;
        if(procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.arPreResAGain[0] = pAERes->LinearExp.exp_real_params.analog_gain;
            stExpInfo.arPreResDGain[0] = pAERes->LinearExp.exp_real_params.digital_gain * pAERes->LinearExp.exp_real_params.isp_dgain;
            stExpInfo.arPreResTime[0] = pAERes->LinearExp.exp_real_params.integration_time;
            stExpInfo.arPreResIso[0] = stExpInfo.arPreResAGain[0] * stExpInfo.arPreResDGain[0] * 50;

            LOGD_ADPCC("%s:%d index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d",
                       __FUNCTION__, __LINE__,
                       0,
                       stExpInfo.arPreResAGain[0],
                       stExpInfo.arPreResDGain[0],
                       stExpInfo.arPreResTime[0],
                       stExpInfo.arPreResIso[0],
                       stExpInfo.hdr_mode);
        } else {
            for(int i = 0; i < 3; i++) {
                stExpInfo.arPreResAGain[i] = pAERes->HdrExp[i].exp_real_params.analog_gain;
                stExpInfo.arPreResDGain[i] = pAERes->HdrExp[i].exp_real_params.digital_gain *
                    pAERes->HdrExp[i].exp_real_params.isp_dgain;
                stExpInfo.arPreResTime[i] = pAERes->HdrExp[i].exp_real_params.integration_time;
                stExpInfo.arPreResIso[i] = stExpInfo.arPreResAGain[i] * stExpInfo.arPreResDGain[i] * 50;

                LOGD_ADPCC("%s:%d index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d",
                           __FUNCTION__, __LINE__,
                           i,
                           stExpInfo.arPreResAGain[i],
                           stExpInfo.arPreResDGain[i],
                           stExpInfo.arPreResTime[i],
                           stExpInfo.arPreResIso[i],
                           stExpInfo.hdr_mode);
            }
        }
    } else {
        LOGE_ADPCC("%s:%d pAERes is NULL, so use default instead", __FUNCTION__, __LINE__);
    }

    if (pAdpccCtx->stExpInfo.arPreResIso[pAdpccCtx->stExpInfo.hdr_mode] !=
        stExpInfo.arPreResIso[pAdpccCtx->stExpInfo.hdr_mode])
        pAdpccCtx->isReCal_ = true;


    if (pAdpccCtx->isReCal_) {
        AdpccResult_t ret = AdpccProcess(pAdpccCtx, &stExpInfo, procResParaGroup->camgroupParmasArray[0]->_dpccConfig);
        if(ret != ADPCC_RET_SUCCESS) {
            result = XCAM_RETURN_ERROR_FAILED;
            LOGE_ADPCC("%s: processing Adpcc failed (%d)", __FUNCTION__, ret);
        }
        IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[0]->_adehazeConfig), procParaGroup->_offset_is_update) =
            true;
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->_dpccConfig = procResParaGroup->camgroupParmasArray[0]->_dpccConfig;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->_adehazeConfig), procParaGroup->_offset_is_update) =
                true;
        }

        outparams->cfg_update = true;
        pAdpccCtx->isReCal_ = false;
    } else {
        IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[0]->_adehazeConfig), procParaGroup->_offset_is_update) =
            false;
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->_dpccConfig = procResParaGroup->camgroupParmasArray[0]->_dpccConfig;
            IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->_adehazeConfig), procParaGroup->_offset_is_update) =
                false;
        }
        outparams->cfg_update = false;
    }
#if 0 //TODO: Is this necessary for group algo ?
    //sensor dpcc setting
    pAdpccProcResParams->adpcc_proc_res_com.SenDpccRes.enable = pAdpccCtx->SenDpccRes.enable;
    pAdpccProcResParams->adpcc_proc_res_com.SenDpccRes.total_dpcc = pAdpccCtx->SenDpccRes.total_dpcc;
    pAdpccProcResParams->adpcc_proc_res_com.SenDpccRes.cur_single_dpcc = pAdpccCtx->SenDpccRes.cur_single_dpcc;
    pAdpccProcResParams->adpcc_proc_res_com.SenDpccRes.cur_multiple_dpcc = pAdpccCtx->SenDpccRes.cur_multiple_dpcc;
#endif

#endif

    LOG1_ADPCC("%s: (exit)", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAdpcc = {
    .common = {
        .version = RKISP_ALGO_ADPCC_VERSION,
        .vendor  = RKISP_ALGO_ADPCC_VENDOR,
        .description = RKISP_ALGO_ADPCC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADPCC,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
