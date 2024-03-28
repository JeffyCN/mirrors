/*
 * rk_aiq_algo_acp_itf.c
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

#include "acp/rk_aiq_algo_acp_itf.h"
#include "xcam_log.h"
#include "acp/rk_aiq_types_algo_acp_prvt.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ACP( "%s: create acp context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->acpCtx.calib = cfg->calib;
    ctx->acpCtx.calibv2 = cfg->calibv2;
    rk_aiq_acp_params_t* params = &ctx->acpCtx.params;
#if RKAIQ_HAVE_ACP_V10
    if (ctx->acpCtx.calib) {
        CalibDb_cProc_t *cproc =
            (CalibDb_cProc_t*)(CALIBDB_GET_MODULE_PTR(ctx->acpCtx.calib, cProc));
        params->enable = cproc->enable;
        params->brightness = cproc->brightness;
        params->hue = cproc->hue;
        params->saturation = cproc->saturation;
        params->contrast = cproc->contrast;
    } else if (ctx->acpCtx.calibv2) {
        Cproc_Param_t *cproc =
            (Cproc_Param_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->acpCtx.calibv2, cproc));
        params->enable = cproc->enable;
        params->brightness = cproc->brightness;
        params->hue = cproc->hue;
        params->saturation = cproc->saturation;
        params->contrast = cproc->contrast;
    }
#endif
    *context = ctx;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    delete context;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    rk_aiq_acp_params_t* acp_params = &params->ctx->acpCtx.params;
    RkAiqAlgoConfigAcp* pCfgParam = (RkAiqAlgoConfigAcp*)params;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_ACP_V10
        if (pCfgParam->com.u.prepare.calib) {
            CalibDb_cProc_t *cproc =
                (CalibDb_cProc_t*)(CALIBDB_GET_MODULE_PTR(pCfgParam->com.u.prepare.calib, cProc));
            acp_params->enable = cproc->enable;
            acp_params->brightness = cproc->brightness;
            acp_params->hue = cproc->hue;
            acp_params->saturation = cproc->saturation;
            acp_params->contrast = cproc->contrast;
        } else if (pCfgParam->com.u.prepare.calibv2) {
            CalibDbV2_Cproc_t* cproc =
                (CalibDbV2_Cproc_t*)(CALIBDBV2_GET_MODULE_PTR(pCfgParam->com.u.prepare.calibv2, cproc));

            acp_params->enable = cproc->param.enable;
            acp_params->brightness = cproc->param.brightness;
            acp_params->hue = cproc->param.hue;
            acp_params->saturation = cproc->param.saturation;
            acp_params->contrast = cproc->param.contrast;
        }
#endif
    }

    params->ctx->acpCtx.isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RkAiqAlgoProcResAcp* res_com = (RkAiqAlgoProcResAcp*)outparams;
    RkAiqAlgoContext* ctx = inparams->ctx;

    if (ctx->acpCtx.isReCal_) {
        *res_com->acp_res = ctx->acpCtx.params;
        outparams->cfg_update = true;
        ctx->acpCtx.isReCal_ = false;
    } else {
        outparams->cfg_update = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAcp = {
    .common = {
        .version = RKISP_ALGO_ACP_VERSION,
        .vendor  = RKISP_ALGO_ACP_VENDOR,
        .description = RKISP_ALGO_ACP_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACP,
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
