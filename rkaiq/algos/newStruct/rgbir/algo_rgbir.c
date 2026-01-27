/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#include "rgbir_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"

XCamReturn RgbirSelectParam(RgbirContext_t *pRgbirCtx, rgbir_param_t* out, int iso);

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    RgbirContext_t* ctx = aiq_mallocz(sizeof(RgbirContext_t));

    if (ctx == NULL) {
        LOGE_ARGBIR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->rgbir_attrib =
        (rgbir_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, rgbir));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ARGBIR("%s: Rgbir (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    RgbirContext_t* pRgbirCtx = (RgbirContext_t*)context;
    aiq_free(pRgbirCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    RgbirContext_t* pRgbirCtx = (RgbirContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pRgbirCtx->rgbir_attrib =
                (rgbir_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, rgbir));
            pRgbirCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pRgbirCtx->working_mode = params->u.prepare.working_mode;
    pRgbirCtx->rgbir_attrib =
        (rgbir_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, rgbir));
    pRgbirCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pRgbirCtx->prepare_params = &params->u.prepare;
    pRgbirCtx->isReCal_ = true;

    return result;
}

XCamReturn Argbir_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    RgbirContext_t* pRgbirCtx = (RgbirContext_t *)inparams->ctx;
    rgbir_api_attrib_t* rgbir_attrib = pRgbirCtx->rgbir_attrib;
    rgbir_param_t* pRgbirProcResParams = outparams->algoRes;
    RkAiqAlgoProcRgbir* rgbir_proc_param = (RkAiqAlgoProcRgbir*)inparams;

    LOGV_ARGBIR("%s: Rgbir (enter)\n", __FUNCTION__ );

    if (!rgbir_attrib) {
        LOGE_ARGBIR("rgbir_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    float blc_ob_predgain = 1.0;

    if (pRgbirCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        iso = iso * blc_ob_predgain;

    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pRgbirCtx->iso);

    outparams->cfg_update = false;

    if (rgbir_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ARGBIR("mode is %d, not auto mode, ignore", rgbir_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (init) {
        pRgbirCtx->isReCal_ = true;
    }

    if (inparams->u.proc.is_attrib_update)
        pRgbirCtx->isReCal_ = true;

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pRgbirCtx->isReCal_ = true;

    if (pRgbirCtx->isReCal_) {
        RgbirSelectParam(pRgbirCtx, pRgbirProcResParams, iso);
        outparams->cfg_update = true;
        outparams->en = rgbir_attrib->en;
        outparams->bypass = rgbir_attrib->bypass;
        LOGD_ARGBIR("RGBIR Process: iso %d, rgbir en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    pRgbirCtx->iso = iso;
    pRgbirCtx->isReCal_ = false;

    LOGV_ARGBIR("%s: Rgbir (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}
static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = inparams->u.proc.iso;
    Argbir_processing(inparams, outparams, iso);
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_rgbir_SetAttrib(RkAiqAlgoContext *ctx,
                                    const rgbir_api_attrib_t *attr,
                                    bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ARGBIR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    RgbirContext_t* pRgbirCtx = (RgbirContext_t*)ctx;
    rgbir_api_attrib_t* rgbir_attrib = pRgbirCtx->rgbir_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ARGBIR("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    rgbir_attrib->opMode = attr->opMode;
    rgbir_attrib->en = attr->en;
    rgbir_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        rgbir_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        rgbir_attrib->stMan = attr->stMan;
    else {
        LOGW_ARGBIR("wrong mode: %d\n", attr->opMode);
    }

    pRgbirCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_rgbir_GetAttrib(const RkAiqAlgoContext *ctx,
                                    rgbir_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ARGBIR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    RgbirContext_t* pRgbirCtx = (RgbirContext_t*)ctx;
    rgbir_api_attrib_t* rgbir_attrib = pRgbirCtx->rgbir_attrib;

    attr->opMode = rgbir_attrib->opMode;
    attr->en = rgbir_attrib->en;
    attr->bypass = rgbir_attrib->bypass;
    memcpy(&attr->stAuto, &rgbir_attrib->stAuto, sizeof(rgbir_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_RGBIR_REMOSAIC
XCamReturn RgbirSelectParam
(
    RgbirContext_t *pRgbirCtx,
    rgbir_param_t* out,
    int iso)
{
    rgbir_param_auto_t *paut = &pRgbirCtx->rgbir_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ARGBIR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;

    pre_interp(iso, pRgbirCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    out->dyn.hw_rgbir_remosaic_edge_coef = interpolation_u16(
        paut->dyn[ilow].hw_rgbir_remosaic_edge_coef,
        paut->dyn[ihigh].hw_rgbir_remosaic_edge_coef, uratio);
    out->dyn.hw_rgbir_remosaic_ir_blk_level = interpolation_f32(
        paut->dyn[ilow].hw_rgbir_remosaic_ir_blk_level,
        paut->dyn[ihigh].hw_rgbir_remosaic_ir_blk_level, ratio);
    out->dyn.sw_rgbir_remosaic_r_scale_coef = interpolation_f32(
        paut->dyn[ilow].sw_rgbir_remosaic_r_scale_coef,
        paut->dyn[ihigh].sw_rgbir_remosaic_r_scale_coef, ratio);
    out->dyn.sw_rgbir_remosaic_gr_scale_coef = interpolation_f32(
        paut->dyn[ilow].sw_rgbir_remosaic_gr_scale_coef,
        paut->dyn[ihigh].sw_rgbir_remosaic_gr_scale_coef, ratio);
    out->dyn.sw_rgbir_remosaic_gb_scale_coef = interpolation_f32(
        paut->dyn[ilow].sw_rgbir_remosaic_gb_scale_coef,
        paut->dyn[ihigh].sw_rgbir_remosaic_gb_scale_coef, ratio);
    out->dyn.sw_rgbir_remosaic_b_scale_coef = interpolation_f32(
        paut->dyn[ilow].sw_rgbir_remosaic_b_scale_coef,
        paut->dyn[ihigh].sw_rgbir_remosaic_b_scale_coef, ratio);
    for (i = 0;i < RGBIR_LUM2SCALE_TABLE_LEN;i++) {
        out->dyn.hw_rgbir_remosaic_lum2scale_idx[i] = interpolation_u16(
            paut->dyn[ilow].hw_rgbir_remosaic_lum2scale_idx[i],
            paut->dyn[ihigh].hw_rgbir_remosaic_lum2scale_idx[i], uratio);
        out->dyn.hw_rgbir_remosaic_lum2scale_val[i] = interpolation_u16(
            paut->dyn[ilow].hw_rgbir_remosaic_lum2scale_val[i],
            paut->dyn[ihigh].hw_rgbir_remosaic_lum2scale_val[i], uratio);
    }
    return XCAM_RETURN_NO_ERROR;
}

#endif

#define RKISP_ALGO_RGBIR_VERSION     "v0.0.9"
#define RKISP_ALGO_RGBIR_VENDOR      "Rockchip"
#define RKISP_ALGO_RGBIR_DESCRIPTION "Rockchip rgbir algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescRgbir = {
    .common = {
        .version = RKISP_ALGO_RGBIR_VERSION,
        .vendor  = RKISP_ALGO_RGBIR_VENDOR,
        .description = RKISP_ALGO_RGBIR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ARGBIR,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

