/*
 * rk_aiq_algo_acgc_itf.c
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

#include "acgc/rk_aiq_algo_acgc_itf.h"

#include "rk_aiq_algo_types.h"
#include "rk_aiq_types_algo_acgc_prvt.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static rk_aiq_acgc_params_t g_cgc_def = {
    .op_mode       = RK_AIQ_OP_MODE_AUTO,
    .cgc_ratio_en  = false,  // true: 219/224 false: 256/256
    .cgc_yuv_limit = false   // true: limit range y 16-235/ c 16-240; false: full range
};

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    AcgcContext_t *ctx = new AcgcContext_t();
    if (ctx == NULL) {
        LOGE_ACGC( "%s: create acgc context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->calibv2                 = cfg->calibv2;
    rk_aiq_acgc_params_t* params = &ctx->params;
    memset(params, 0, sizeof(*params));
    if (ctx->calibv2) {
        Cgc_Param_t* cgc =
            (Cgc_Param_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->calibv2, cgc));
        if (cgc) {
            *params = *cgc;
        } else {
            *params = g_cgc_def;
        }

    } else {
        // auto means using chip reset valuse
        *params = g_cgc_def;
    }

    *context = (RkAiqAlgoContext *)ctx;
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
    rk_aiq_acgc_params_t* acgc_params = &params->ctx->acgcCtx.params;
    RkAiqAlgoConfigAcgc* pCfgParam    = (RkAiqAlgoConfigAcgc*)params;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;

        if (pCfgParam->com.u.prepare.calibv2) {
#if RKAIQ_HAVE_CGC_V1
            Cgc_Param_t* cgc =
                (Cgc_Param_t*)(CALIBDBV2_GET_MODULE_PTR(pCfgParam->com.u.prepare.calibv2, cgc));
            if (cgc) *acgc_params = *cgc;
#endif
        }
    }

    params->ctx->acgcCtx.isReCal_ = true;

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
    RkAiqAlgoProcResAcgc* res_com = (RkAiqAlgoProcResAcgc*)outparams;
    RkAiqAlgoContext* ctx         = inparams->ctx;

    if (ctx->acgcCtx.params.op_mode == RK_AIQ_OP_MODE_AUTO) {
        ctx->acgcCtx.params = g_cgc_def;
    }

    if (ctx->acgcCtx.isReCal_) {
        *res_com->acgc_res = ctx->acgcCtx.params;
        outparams->cfg_update = true;
        ctx->acgcCtx.isReCal_ = false;
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

RkAiqAlgoDescription g_RkIspAlgoDescAcgc = {
    .common = {
        .version = RKISP_ALGO_ACGC_VERSION,
        .vendor  = RKISP_ALGO_ACGC_VENDOR,
        .description = RKISP_ALGO_ACGC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACGC,
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
