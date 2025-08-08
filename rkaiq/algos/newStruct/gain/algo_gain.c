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

#include "gain_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

#include "interpolation.h"
#include "c_base/aiq_base.h"

XCamReturn GainSelectParam(GainContext_t *pGainCtx, gain_param_t* out, int iso);

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    GainContext_t* ctx = aiq_mallocz(sizeof(GainContext_t));

    if (ctx == NULL) {
        LOGE_ANR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->gain_attrib =
        (gain_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, gain));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ANR("%s: Gain (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    GainContext_t* pGainCtx = (GainContext_t*)context;
    aiq_free(pGainCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    GainContext_t* pGainCtx = (GainContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pGainCtx->gain_attrib =
                (gain_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, gain));
            pGainCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pGainCtx->gain_attrib =
        (gain_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, gain));
    pGainCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pGainCtx->prepare_params = &params->u.prepare;
    pGainCtx->isReCal_ = true;

    return result;
}

XCamReturn Again_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    GainContext_t* pGainCtx = (GainContext_t *)inparams->ctx;
    gain_api_attrib_t* gain_attrib = pGainCtx->gain_attrib;
    gain_param_t* pGainProcResParams = outparams->algoRes;

    LOGV_ANR("%s: Gain (enter)\n", __FUNCTION__ );

    if (!gain_attrib) {
        LOGE_ANR("gain_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    float blc_ob_predgain = 1.0;

    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pGainCtx->iso);

    outparams->cfg_update = false;

    if (gain_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", gain_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (init) {
        pGainCtx->isReCal_ = true;
    }

    if (inparams->u.proc.is_attrib_update)
        pGainCtx->isReCal_ = true;

    if (delta_iso > AGAINV2_RECALCULATE_DELTA_ISO)
        pGainCtx->isReCal_ = true;

    if (pGainCtx->isReCal_) {
        GainSelectParam(pGainCtx, pGainProcResParams, iso);
        outparams->cfg_update = true;
        outparams->en = gain_attrib->en;
        outparams->bypass = gain_attrib->bypass;
        LOGD_ANR("GAIN Process: iso %d, gain en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    pGainCtx->iso = iso;
    pGainCtx->isReCal_ = false;

    LOGV_ANR("%s: Gain (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = 50;
    if (!inparams->u.proc.init) {
        iso = inparams->u.proc.iso;
    }
    Again_processing(inparams, outparams, iso);
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_gain_SetAttrib(RkAiqAlgoContext *ctx,
                                    const gain_api_attrib_t *attr,
                                    bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    GainContext_t* pGainCtx = (GainContext_t*)ctx;
    gain_api_attrib_t* gain_attrib = pGainCtx->gain_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    gain_attrib->opMode = attr->opMode;
    gain_attrib->en = attr->en;
    gain_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        gain_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        gain_attrib->stMan = attr->stMan;
    else {
        LOGW_ANR("wrong mode: %d\n", attr->opMode);
    }

    pGainCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_gain_GetAttrib(const RkAiqAlgoContext *ctx,
                                    gain_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    GainContext_t* pGainCtx = (GainContext_t*)ctx;
    gain_api_attrib_t* gain_attrib = pGainCtx->gain_attrib;

    attr->opMode = gain_attrib->opMode;
    attr->en = gain_attrib->en;
    attr->bypass = gain_attrib->bypass;
    memcpy(&attr->stAuto, &gain_attrib->stAuto, sizeof(gain_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_GAIN_V2
XCamReturn GainSelectParam
(
    GainContext_t *pGainCtx,
    gain_param_t* out,
    int iso)
{
    gain_param_auto_t *paut = &pGainCtx->gain_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, pGainCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    out->dyn.hdrgain_ctrl_enable = interpolation_bool(paut->dyn[ilow].hdrgain_ctrl_enable,
        paut->dyn[ihigh].hdrgain_ctrl_enable, uratio);
    out->dyn.hdr_gain_scale_s = interpolation_f32(paut->dyn[ilow].hdr_gain_scale_s,
        paut->dyn[ihigh].hdr_gain_scale_s, ratio);
    out->dyn.hdr_gain_scale_m = interpolation_f32(paut->dyn[ilow].hdr_gain_scale_m,
        paut->dyn[ihigh].hdr_gain_scale_m, ratio);

    return XCAM_RETURN_NO_ERROR;
}

#endif

#define RKISP_ALGO_GAIN_VERSION     "v0.0.9"
#define RKISP_ALGO_GAIN_VENDOR      "Rockchip"
#define RKISP_ALGO_GAIN_DESCRIPTION "Rockchip gain algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescGain = {
    .common = {
        .version = RKISP_ALGO_GAIN_VERSION,
        .vendor  = RKISP_ALGO_GAIN_VENDOR,
        .description = RKISP_ALGO_GAIN_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGAIN,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

