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

#include "adegamma/rk_aiq_algo_adegamma_itf.h"
#include "adegamma/rk_aiq_adegamma_algo.h"
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    AdegammaHandle_t AdegammaHandle;
} RkAiqAlgoContext;


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdegammaHandle_t*AdegammaHandle = NULL;
    CamCalibDbV2Context_t* calib = cfg->calibv2;
    ret = AdegammaInit(&AdegammaHandle, calib);
    *context = (RkAiqAlgoContext *)(AdegammaHandle);

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);
    AdegammaHandle_t*AdegammaHandle = (AdegammaHandle_t*)context;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AdegammaRelease(AdegammaHandle);

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdegammaHandle_t * AdegammaHandle = (AdegammaHandle_t *)params->ctx;
    RkAiqAlgoConfigAdegamma* pCfgParam = (RkAiqAlgoConfigAdegamma*)params;
    AdegammaHandle->working_mode = pCfgParam->com.u.prepare.working_mode;
    AdegammaHandle->prepare_type = pCfgParam->com.u.prepare.conf_type;

    if(!!(AdegammaHandle->prepare_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        CalibDbV2_Adegmma_t* adegamma_calib =
            (CalibDbV2_Adegmma_t*)(CALIBDBV2_GET_MODULE_PTR(pCfgParam->com.u.prepare.calibv2, adegamma_calib));
        AdegammaHandle->pCalibDb = adegamma_calib;//reload iq
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;
        LOGD_ADEGAMMA("%s: Adegamma Reload Para!!!\n", __FUNCTION__);
    }

    if (pCfgParam->com.u.proc.gray_mode)
        AdegammaHandle->Scene_mode = DEGAMMA_OUT_NIGHT;
    else if (DEGAMMA_OUT_NORMAL == AdegammaHandle->working_mode)
        AdegammaHandle->Scene_mode = DEGAMMA_OUT_NORMAL;
    else
        AdegammaHandle->Scene_mode = DEGAMMA_OUT_HDR;

    AdegammaHandle->isReCal_ = true;

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdegammaHandle_t * AdegammaHandle = (AdegammaHandle_t *)inparams->ctx;
    RkAiqAlgoProcResAdegamma* procResPara = (RkAiqAlgoProcResAdegamma*)outparams;
    AdegammaProcRes_t* AdegammaProcRes = procResPara->adegamma_proc_res;

    if (inparams->u.proc.gray_mode)
        AdegammaHandle->Scene_mode = DEGAMMA_OUT_NIGHT;
    else if (DEGAMMA_OUT_NORMAL == AdegammaHandle->working_mode)
        AdegammaHandle->Scene_mode = DEGAMMA_OUT_NORMAL;
    else
        AdegammaHandle->Scene_mode = DEGAMMA_OUT_HDR;

    if (AdegammaHandle->isReCal_) {
        AdegammaProcessing(AdegammaHandle, AdegammaProcRes);
        outparams->cfg_update = true;
        AdegammaHandle->isReCal_ = false;
    } else {
        outparams->cfg_update = false;
    }

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ADEGAMMA("ENTER: %s \n", __func__);

    LOG1_ADEGAMMA("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAdegamma = {
    .common = {
        .version = RKISP_ALGO_ADEGAMMA_VERSION,
        .vendor  = RKISP_ALGO_ADEGAMMA_VENDOR,
        .description = RKISP_ALGO_ADEGAMMA_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADEGAMMA,
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
