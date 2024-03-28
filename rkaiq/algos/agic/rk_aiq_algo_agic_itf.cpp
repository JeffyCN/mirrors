/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#include "algos/agic/rk_aiq_algo_agic_itf.h"

#include "algos/agic/rk_aiq_types_algo_agic_prvt.h"
#include "algos/rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    LOG1_AGIC("enter!");

    RkAiqAlgoContext* ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_AGIC("Create gic context fail!");
        return XCAM_RETURN_ERROR_MEM;
    }
    CamCalibDbV2Context_t* calibv2 = cfg->calibv2;
    AgicInit(&ctx->agicCtx, calibv2);

    *context = ctx;
    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn destroy_context(RkAiqAlgoContext* context) {
    LOG1_AGIC("enter!");

    AgicContext_t* pAgicCtx = (AgicContext_t*)&context->agicCtx;
    AgicRelease(pAgicCtx);
    delete context;

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn prepare(RkAiqAlgoCom* params) {
    LOG1_AGIC("enter!");

    AgicContext_t* pAgicCtx        = (AgicContext_t*)&params->ctx->agicCtx;
    RkAiqAlgoConfigAgic* pCfgParam = (RkAiqAlgoConfigAgic*)params;
    CamCalibDbV2Context_t* calibv2 = pCfgParam->com.u.prepare.calibv2;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        LOGD_AGIC("%s: Agic Reload Para!\n", __FUNCTION__);

#if RKAIQ_HAVE_GIC_V1
        CalibDbV2_Gic_V20_t* calibv2_agic_calib_V20 =
            (CalibDbV2_Gic_V20_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, agic_calib_v20));
        pAgicCtx->full_param.gic_v20 = calibv2_agic_calib_V20;
        GicV1CalibToAttr(calibv2_agic_calib_V20, &pAgicCtx->attr.v1);
#endif
#if RKAIQ_HAVE_GIC_V2
        CalibDbV2_Gic_V21_t* calibv2_agic_calib_V21 =
            (CalibDbV2_Gic_V21_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, agic_calib_v21));
        pAgicCtx->full_param.gic_v21 = calibv2_agic_calib_V21;
        GicV2CalibToAttr(calibv2_agic_calib_V21, &pAgicCtx->attr.v2);
#endif
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;

        pAgicCtx->calib_changed = true;
    }

    pAgicCtx->working_mode = pCfgParam->com.u.prepare.working_mode;

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}
#if 0
static XCamReturn pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    LOG1_AGIC("enter!");

    RkAiqAlgoConfigAgic* config            = (RkAiqAlgoConfigAgic*)inparams;
    RkAiqAlgoPreResAgic* pAgicPreResParams = (RkAiqAlgoPreResAgic*)outparams;
    AgicContext_t* pAgicCtx                = (AgicContext_t*)&inparams->ctx->agicCtx;

    if (config->com.u.proc.gray_mode)
        pAgicCtx->Gic_Scene_mode = GIC_NIGHT;
    else if (GIC_NORMAL == pAgicCtx->working_mode)
        pAgicCtx->Gic_Scene_mode = GIC_NORMAL;
    else
        pAgicCtx->Gic_Scene_mode = GIC_HDR;

    LOG1_AGIC("exit!");
    return XCAM_RETURN_NO_ERROR;
}
#endif
static XCamReturn processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    LOG1_AGIC("enter!");
    RkAiqAlgoProcAgic* pAgicProcParams       = (RkAiqAlgoProcAgic*)inparams;
    RkAiqAlgoProcResAgic* pAgicProcResParams = (RkAiqAlgoProcResAgic*)outparams;
    AgicContext_t* pAgicCtx                  = (AgicContext_t*)&inparams->ctx->agicCtx;
    int iso                                  = pAgicProcParams->iso;

    if (inparams->u.proc.gray_mode)
        pAgicCtx->Gic_Scene_mode = GIC_NIGHT;
    else if (GIC_NORMAL == pAgicCtx->working_mode)
        pAgicCtx->Gic_Scene_mode = GIC_NORMAL;
    else
        pAgicCtx->Gic_Scene_mode = GIC_HDR;

    pAgicCtx->raw_bits       = pAgicProcParams->raw_bits;
    pAgicCtx->Gic_Scene_mode = 0;
    if (pAgicCtx->last_iso != iso || pAgicCtx->calib_changed) {
        AgicProcess(pAgicCtx, iso, pAgicCtx->Gic_Scene_mode);
        pAgicCtx->calib_changed          = false;
        AgicGetProcResult(pAgicCtx, pAgicProcResParams->gicRes);
        outparams->cfg_update = true;
        pAgicCtx->last_iso = iso;
    } else {
        outparams->cfg_update = false;
    }

    LOG1_AGIC("enter!");
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    (void)(inparams);
    (void)(outparams);

    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAgic = {
    .common =
    {
        .version         = RKISP_ALGO_AGIC_VERSION,
        .vendor          = RKISP_ALGO_AGIC_VENDOR,
        .description     = RKISP_ALGO_AGIC_DESCRIPTION,
        .type            = RK_AIQ_ALGO_TYPE_AGIC,
        .id              = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare      = prepare,
    .pre_process  = NULL,
    .processing   = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
