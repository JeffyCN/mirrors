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
#if RKAIQ_HAVE_AIE_V10
    if (ctx->calib) {
        CalibDb_IE_t *calib_ie =
            (CalibDb_IE_t*)(CALIBDB_GET_MODULE_PTR(ctx->calib, ie));
        if (calib_ie->enable) {
            ctx->params.mode = (rk_aiq_ie_effect_t)calib_ie->mode;
        } else {
            ctx->params.mode = RK_AIQ_IE_EFFECT_NONE;
        }

        ctx->last_params.skip_frame = 10;

    } else if (ctx->calibv2) {
        CalibDbV2_IE_t* calibv2_ie =
                (CalibDbV2_IE_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->calibv2, ie));
        if (calibv2_ie->param.enable) {
            ctx->params.mode = (rk_aiq_ie_effect_t)calibv2_ie->param.mode;
        } else {
            ctx->params.mode = RK_AIQ_IE_EFFECT_NONE;
        }

        CalibDbV2_ColorAsGrey_t *colorAsGrey =
                (CalibDbV2_ColorAsGrey_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->calibv2, colorAsGrey));

        if (colorAsGrey->param.enable) {
            ctx->last_params.skip_frame = colorAsGrey->param.skip_frame;
        } else {
            ctx->last_params.skip_frame = 10;
        }
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
#endif
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
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            LOGK_AIE("only update calib ptr, skip reconfig \n");
            return XCAM_RETURN_NO_ERROR;
        }
#if RKAIQ_HAVE_AIE_V10
        if (ctx->calib) {
            CalibDb_IE_t *calib_ie =
                (CalibDb_IE_t*)(CALIBDB_GET_MODULE_PTR(ctx->calib, ie));
            if (ctx->skip_frame > 0) {
                if (!calib_ie->enable ||
                        (calib_ie->enable &&
                        ((rk_aiq_ie_effect_t)calib_ie->mode == ctx->last_params.mode))) {
                    return XCAM_RETURN_NO_ERROR;
                } else {
                    ctx->params.mode = (rk_aiq_ie_effect_t)calib_ie->mode;
                    ctx->last_params.mode = ctx->params.mode;
                }
            } else {
                if (calib_ie->enable){
                    ctx->params.mode = (rk_aiq_ie_effect_t)calib_ie->mode;
                    ctx->last_params.mode = ctx->params.mode;
                } else {
                    ctx->params.mode = RK_AIQ_IE_EFFECT_NONE;
                }
            }
        } else if (ctx->calibv2) {
            CalibDbV2_IE_t* calibv2_ie =
                (CalibDbV2_IE_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->calibv2, ie));
            if (ctx->skip_frame > 0) {
                if (!calibv2_ie->param.enable ||
                        (calibv2_ie->param.enable &&
                        ((rk_aiq_ie_effect_t)calibv2_ie->param.mode == ctx->last_params.mode))) {
                    return XCAM_RETURN_NO_ERROR;
                } else {
                    ctx->params.mode = (rk_aiq_ie_effect_t)calibv2_ie->param.mode;
                    ctx->last_params.mode = ctx->params.mode;
                }
            } else {
                if (calibv2_ie->param.enable){
                    ctx->params.mode = (rk_aiq_ie_effect_t)calibv2_ie->param.mode;
                    ctx->last_params.mode = ctx->params.mode;
                } else {
                    ctx->params.mode = RK_AIQ_IE_EFFECT_NONE;
                }
            }

            CalibDbV2_ColorAsGrey_t *colorAsGrey =
                (CalibDbV2_ColorAsGrey_t*)(CALIBDBV2_GET_MODULE_PTR(ctx->calibv2, colorAsGrey));

            if (colorAsGrey->param.enable) {
                ctx->last_params.skip_frame = colorAsGrey->param.skip_frame;
            }
        }
#endif
    }

    params->ctx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}
#if 0
static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RkAiqAlgoContext *ctx = inparams->ctx;
    RkAiqAlgoPreAie* pAiePreParams = (RkAiqAlgoPreAie*)inparams;
    // force gray_mode by aiq framework
#if RKAIQ_HAVE_AIE_V10
    if (pAiePreParams->com.u.proc.gray_mode &&
        ctx->params.mode !=  RK_AIQ_IE_EFFECT_BW) {
        ctx->last_params.mode = ctx->params.mode;
        ctx->params.mode = RK_AIQ_IE_EFFECT_BW;
        ctx->skip_frame = ctx->last_params.skip_frame;
    } else if (!pAiePreParams->com.u.proc.gray_mode &&
               ctx->params.mode == RK_AIQ_IE_EFFECT_BW) {
        // force non gray_mode by aiq framework
        if (ctx->skip_frame && --ctx->skip_frame == 0)
            ctx->params.mode = ctx->last_params.mode;
    }
#endif
    return XCAM_RETURN_NO_ERROR;
}
#endif
static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    RkAiqAlgoContext *ctx = inparams->ctx;

    RkAiqAlgoProcResAie* res = (RkAiqAlgoProcResAie*)outparams;

    rk_aiq_aie_params_int_t* int_params = NULL;
#if RKAIQ_HAVE_AIE_V10
    if (inparams->u.proc.gray_mode &&
        ctx->params.mode !=  RK_AIQ_IE_EFFECT_BW) {
        ctx->last_params.mode = ctx->params.mode;
        ctx->params.mode = RK_AIQ_IE_EFFECT_BW;
        ctx->skip_frame = ctx->last_params.skip_frame;
        ctx->isReCal_ = true;
    } else if (!inparams->u.proc.gray_mode &&
               ctx->params.mode == RK_AIQ_IE_EFFECT_BW) {
        // force non gray_mode by aiq framework
        if (ctx->skip_frame && --ctx->skip_frame == 0) {
            ctx->params.mode = ctx->last_params.mode;
            ctx->isReCal_ = true;
        }
        if (ctx->skip_frame) {
            LOGE_AIE("still need skip %d frame!!! \n", ctx->skip_frame);
        }
    }

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
#endif

    if (ctx->isReCal_) {
        res->ieRes->base = ctx->params;
        if (int_params)
            res->ieRes->extra = *int_params;
        outparams->cfg_update = true;
        ctx->isReCal_ = false;
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
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
