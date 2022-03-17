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

#include "rk_aiq_algo_types_int.h"
#include "agamma/rk_aiq_algo_agamma_itf.h"
#include "agamma/rk_aiq_agamma_algo.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    void* place_holder[0];
} RkAiqAlgoContext;



static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t*AgammaHandle = NULL;
    AlgoCtxInstanceCfgInt* instanc_int = (AlgoCtxInstanceCfgInt*)cfg;
    CamCalibDbV2Context_t* calibV2 = instanc_int->calibv2;

    ret = AgammaInit(&AgammaHandle, calibV2);

    AgammaHandle->HWversion = instanc_int->cfg_com.isp_hw_version;//get hadrware version

    *context = (RkAiqAlgoContext *)(AgammaHandle);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    AgammaHandle_t*AgammaHandle = (AgammaHandle_t*)context;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AgammaRelease(AgammaHandle);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t * AgammaHandle = (AgammaHandle_t *)params->ctx;
    RkAiqAlgoConfigAgammaInt* pCfgParam = (RkAiqAlgoConfigAgammaInt*)params;
    rk_aiq_gamma_cfg_t *agamma_config = &AgammaHandle->agamma_config;
    AgammaHandle->working_mode = pCfgParam->agamma_config_com.com.u.prepare.working_mode;
    AgammaHandle->prepare_type = pCfgParam->agamma_config_com.com.u.prepare.conf_type;

    if(!!(AgammaHandle->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        CalibDbV2_gamma_t* calibv2_agamma_calib =
            (CalibDbV2_gamma_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCfgParam->rk_com.u.prepare.calibv2), agamma_calib));
        AgammaHandle->pCalibDb = calibv2_agamma_calib;//reload iq
        LOGD_AGAMMA("%s: Agamma Reload Para!!!\n", __FUNCTION__);
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    RkAiqAlgoPreAgammaInt* pAgammaPreParams = (RkAiqAlgoPreAgammaInt*)inparams;
    AgammaHandle_t * AgammaHandle = (AgammaHandle_t *)inparams->ctx;
    rk_aiq_gamma_cfg_t *agamma_config = &AgammaHandle->agamma_config;


    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t * AgammaHandle = (AgammaHandle_t *)inparams->ctx;
    RkAiqAlgoProcResAgamma* procResPara = (RkAiqAlgoProcResAgamma*)outparams;
    AgammaProcRes_t* AgammaProcRes = (AgammaProcRes_t*)&procResPara->agamma_proc_res;

    AgammaProcessing(AgammaHandle);

    //set proc res
    AgammaSetProcRes(AgammaProcRes, &AgammaHandle->agamma_config);
    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAgamma = {
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
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE
