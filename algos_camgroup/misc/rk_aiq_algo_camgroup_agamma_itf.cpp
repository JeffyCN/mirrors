/*
 * rk_aiq_algo_agamma_itf.c
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
#include "algos/agamma/rk_aiq_algo_agamma_itf.h"
#include "algos/agamma/rk_aiq_agamma_algo.h"

RKAIQ_BEGIN_DECLARE


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* pAgammaGrpCtx = NULL;
    AlgoCtxInstanceCfgCamGroup* instanc_int = (AlgoCtxInstanceCfgCamGroup*)cfg;

    ret = AgammaInit(&pAgammaGrpCtx, (CamCalibDbV2Context_t*)(instanc_int->s_calibv2));

    *context = (RkAiqAlgoContext *)(pAgammaGrpCtx);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    AgammaHandle_t* pAgammaGrpCtx = (AgammaHandle_t*)context;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AgammaRelease(pAgammaGrpCtx);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t * pAgammaGrpCtx = (AgammaHandle_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* pCfgParam = (RkAiqAlgoCamGroupPrepare*)params;
    rk_aiq_gamma_cfg_t *agamma_config = &pAgammaGrpCtx->agamma_config;
    pAgammaGrpCtx->working_mode = pCfgParam->gcom.com.u.prepare.working_mode;
    pAgammaGrpCtx->prepare_type = pCfgParam->gcom.com.u.prepare.conf_type;

    if(!!(pAgammaGrpCtx->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {

        if(CHECK_ISP_HW_V21()) {
            CalibDbV2_gamma_t* calibv2_agamma_calib =
                (CalibDbV2_gamma_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCfgParam->s_calibv2), agamma_calib));
            memcpy(&pAgammaGrpCtx->CalibDb.Gamma_v20, calibv2_agamma_calib, sizeof(CalibDbV2_gamma_t));//reload iq
        }
        else if(CHECK_ISP_HW_V30()) {
            CalibDbV2_gamma_V30_t* calibv2_agamma_calib =
                (CalibDbV2_gamma_V30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCfgParam->s_calibv2), agamma_calib));
            memcpy(&pAgammaGrpCtx->CalibDb.Gamma_v30, calibv2_agamma_calib, sizeof(CalibDbV2_gamma_V30_t));//reload iq
        }
        LOGI_AGAMMA("%s: Agamma Reload Para!!!\n", __FUNCTION__);
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* pAgammaGrpCtx = (AgammaHandle_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcOut* pAgammaGrpProcRes = (RkAiqAlgoCamGroupProcOut*)outparams;

    AgammaProcessing(pAgammaGrpCtx);

    //set proc res
    AgammaSetProcRes(pAgammaGrpProcRes->camgroupParmasArray[0]->_agammaConfig, &pAgammaGrpCtx->agamma_config);
    for(int i = 0; i < pAgammaGrpProcRes->arraySize; i++)
        memcpy(pAgammaGrpProcRes->camgroupParmasArray[i]->_agammaConfig, pAgammaGrpProcRes->camgroupParmasArray[0]->_agammaConfig, sizeof(AgammaProcRes_t));

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescamgroupAgamma = {
    .common = {
        .version = RKISP_ALGO_AGAMMA_VERSION,
        .vendor  = RKISP_ALGO_AGAMMA_VENDOR,
        .description = RKISP_ALGO_AGAMMA_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGAMMA,
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
