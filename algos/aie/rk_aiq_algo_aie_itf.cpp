/*
 * rk_aiq_algo_aie_itf.c
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

#include "aie/rk_aiq_algo_aie_itf.h"
#include "rk_aiq_types_algo_aie_prvt.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_AIE( "%s: create aie context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset(ctx, 0, sizeof(RkAiqAlgoContext));

    ctx->calib = cfg->calib;
    ctx->calibv2 = cfg->calibv2;
    if (ctx->calib) {
        CalibDb_IE_t *calib_ie =
            (CalibDb_IE_t*)(CALIBDB_GET_MODULE_PTR(ctx->calib, ie));
        ctx->params.mode = (rk_aiq_ie_effect_t)calib_ie->mode;
    } else if (ctx->calibv2) {
        CalibDbV2_IE_t* calibv2_ie =
                (CalibDbV2_IE_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->calibv2, ie));
        ctx->params.mode = (rk_aiq_ie_effect_t)calibv2_ie->param.mode;
    }

    // default value
    ctx->emboss_params.mode_coeffs[0] = 0x9;//  2
    ctx->emboss_params.mode_coeffs[1] = 0x0;// 0
    ctx->emboss_params.mode_coeffs[2] = 0x0;// 0
    ctx->emboss_params.mode_coeffs[3] = 0x8;// 1
    ctx->emboss_params.mode_coeffs[4] = 0x0;// 0
    ctx->emboss_params.mode_coeffs[5] = 0xc;// -1
    ctx->emboss_params.mode_coeffs[6] = 0x0;// 0x0
    ctx->emboss_params.mode_coeffs[7] = 0xc;// -1
    ctx->emboss_params.mode_coeffs[8] = 0x9;// 2

    ctx->sketch_params.mode_coeffs[0] = 0xc;//-1
    ctx->sketch_params.mode_coeffs[1] = 0xc;//-1
    ctx->sketch_params.mode_coeffs[2] = 0xc;//-1
    ctx->sketch_params.mode_coeffs[3] = 0xc;//-1
    ctx->sketch_params.mode_coeffs[4] = 0xb;// 0x8
    ctx->sketch_params.mode_coeffs[5] = 0xc;//-1
    ctx->sketch_params.mode_coeffs[6] = 0xc;//-1
    ctx->sketch_params.mode_coeffs[7] = 0xc;//-1
    ctx->sketch_params.mode_coeffs[8] = 0xc;//-1

    ctx->sharp_params.mode_coeffs[0] = 0xc;//-1
    ctx->sharp_params.mode_coeffs[1] = 0xc;//-1
    ctx->sharp_params.mode_coeffs[2] = 0xc;//-1
    ctx->sharp_params.mode_coeffs[3] = 0xc;//-1
    ctx->sharp_params.mode_coeffs[4] = 0xb;// 0x8
    ctx->sharp_params.mode_coeffs[5] = 0xc;//-1
    ctx->sharp_params.mode_coeffs[6] = 0xc;//-1
    ctx->sharp_params.mode_coeffs[7] = 0xc;//-1
    ctx->sharp_params.mode_coeffs[8] = 0xc;//-1
    ctx->sharp_params.sharp_factor = 8.0;
    ctx->sharp_params.sharp_thres = 128;

    *context = ctx;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    delete context;
    context = NULL;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)){
        RkAiqAlgoConfigAie* confPara = (RkAiqAlgoConfigAie*)params;
        RkAiqAlgoContext *ctx = params->ctx;
        ctx->calib = confPara->com.u.prepare.calib;
        ctx->calibv2 = confPara->com.u.prepare.calibv2;
        if (ctx->calib) {
            CalibDb_IE_t *calib_ie =
                (CalibDb_IE_t*)(CALIBDB_GET_MODULE_PTR(ctx->calib, ie));
            ctx->params.mode = (rk_aiq_ie_effect_t)calib_ie->mode;
        } else if (ctx->calibv2) {
            CalibDbV2_IE_t* calibv2_ie =
                (CalibDbV2_IE_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->calibv2, ie));
            ctx->params.mode = (rk_aiq_ie_effect_t)calibv2_ie->param.mode;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RkAiqAlgoContext *ctx = inparams->ctx;
    RkAiqAlgoPreAie* pAiePreParams = (RkAiqAlgoPreAie*)inparams;
    // force gray_mode by aiq framework
    if (pAiePreParams->com.u.proc.gray_mode &&
        ctx->params.mode !=  RK_AIQ_IE_EFFECT_BW) {
        ctx->last_params = ctx->params;
        ctx->params.mode = RK_AIQ_IE_EFFECT_BW;
        ctx->skip_frame = 10;
    } else if (!pAiePreParams->com.u.proc.gray_mode &&
               ctx->params.mode == RK_AIQ_IE_EFFECT_BW) {
        // force non gray_mode by aiq framework
        if (ctx->skip_frame && --ctx->skip_frame == 0)
            ctx->params = ctx->last_params;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RkAiqAlgoContext *ctx = inparams->ctx;

    RkAiqAlgoProcResAie* res = (RkAiqAlgoProcResAie*)outparams;

    res->params_com = ctx->params;

    rk_aiq_aie_params_int_t* int_params = NULL;
    switch (ctx->params.mode)
    {
    case RK_AIQ_IE_EFFECT_EMBOSS :
        int_params = &ctx->emboss_params;
        break;
    case RK_AIQ_IE_EFFECT_SKETCH :
        int_params = &ctx->sketch_params;
        break;
    case RK_AIQ_IE_EFFECT_SHARPEN : /*!< deprecated */
        int_params = &ctx->sharp_params;
        break;
    default:
        break;
    }

    if (int_params)
        res->params = *int_params;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAie = {
    .common = {
        .version = RKISP_ALGO_AIE_VERSION,
        .vendor  = RKISP_ALGO_AIE_VENDOR,
        .description = RKISP_ALGO_AIE_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AIE,
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
