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

#include "algo_types_priv.h"
#include "yme_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
//#include "RkAiqHandle.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    YmeContext_t* ctx = aiq_mallocz(sizeof(YmeContext_t));

    if (ctx == NULL) {
        LOGE_ANR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->yme_attrib =
        (yme_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, yme));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ANR("%s: Yme (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    YmeContext_t* pYmeCtx = (YmeContext_t*)context;
    aiq_free(pYmeCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    YmeContext_t* pYmeCtx = (YmeContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pYmeCtx->yme_attrib =
                (yme_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, yme));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pYmeCtx->working_mode = params->u.prepare.working_mode;
    pYmeCtx->yme_attrib =
        (yme_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, yme));
    pYmeCtx->prepare_params = &params->u.prepare;
    pYmeCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    YmeContext_t* pYmeCtx = (YmeContext_t *)inparams->ctx;
    yme_api_attrib_t* yme_attrib = pYmeCtx->yme_attrib;
    yme_param_t* pYmeProcResParams = outparams->algoRes;
    RkAiqAlgoProcYme* yme_proc_param = (RkAiqAlgoProcYme*)inparams;

    LOGV_ANR("%s: Yme (enter)\n", __FUNCTION__ );

    if (!yme_attrib) {
        LOGE_ANR("yme_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    float blc_ob_predgain = 1.0;
    int iso = inparams->u.proc.iso;

    if (pYmeCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        iso = iso * blc_ob_predgain;

    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pYmeCtx->iso);

    outparams->cfg_update = false;

    if (yme_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", yme_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (init) {
        pYmeCtx->isReCal_ = true;
    }

    if (inparams->u.proc.is_attrib_update)
        pYmeCtx->isReCal_ = true;

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pYmeCtx->isReCal_ = true;

    if (pYmeCtx->isReCal_) {
        pYmeProcResParams->sta = pYmeCtx->yme_attrib->stAuto.sta;
        YmeSelectParam(&pYmeCtx->yme_attrib->stAuto, pYmeProcResParams, iso);
        outparams->cfg_update = true;
        outparams->en = yme_attrib->en;
        outparams->bypass = yme_attrib->bypass;
        LOGD_ANR("YME Process: iso %d, yme en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    pYmeCtx->iso = iso;
    pYmeCtx->isReCal_ = false;

    LOGV_ANR("%s: Yme (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_yme_SetAttrib(RkAiqAlgoContext *ctx,
                                    const yme_api_attrib_t *attr,
                                    bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    YmeContext_t* pYmeCtx = (YmeContext_t*)ctx;
    yme_api_attrib_t* yme_attrib = pYmeCtx->yme_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    yme_attrib->opMode = attr->opMode;
    yme_attrib->en = attr->en;
    yme_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        yme_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        yme_attrib->stMan = attr->stMan;
    else {
        LOGW_ANR("wrong mode: %d\n", attr->opMode);
    }

    pYmeCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_yme_GetAttrib(const RkAiqAlgoContext *ctx,
                                    yme_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    YmeContext_t* pYmeCtx = (YmeContext_t*)ctx;
    yme_api_attrib_t* yme_attrib = pYmeCtx->yme_attrib;

    attr->opMode = yme_attrib->opMode;
    attr->en = yme_attrib->en;
    attr->bypass = yme_attrib->bypass;
    memcpy(&attr->stAuto, &yme_attrib->stAuto, sizeof(yme_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_YUVME_V1
XCamReturn YmeSelectParam
(
    yme_param_auto_t *pAuto,
    yme_param_t* out,
    int iso)
{
    if(pAuto == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    yme_param_auto_t *paut = pAuto;

    pre_interp(iso, NULL, 0, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    yme_params_dyn_t *plow = &paut->dyn[ilow];
    yme_params_dyn_t *phigh = &paut->dyn[ihigh];

    out->dyn.hw_yuvme_searchRange_mode = ROUND_I32(INTERP1(plow->hw_yuvme_searchRange_mode, phigh->hw_yuvme_searchRange_mode, ratio));
    out->dyn.hw_yuvme_timeRelevance_offset = ROUND_I32(INTERP1(plow->hw_yuvme_timeRelevance_offset, phigh->hw_yuvme_timeRelevance_offset, ratio));
    out->dyn.hw_yuvme_spaceRelevance_offset = ROUND_I32(INTERP1(plow->hw_yuvme_spaceRelevance_offset, phigh->hw_yuvme_spaceRelevance_offset, ratio));
    out->dyn.hw_yuvme_staticDetect_thred = ROUND_I32(INTERP1(plow->hw_yuvme_staticDetect_thred, phigh->hw_yuvme_staticDetect_thred, ratio));

    out->dyn.sw_yuvme_globalNr_strg = INTERP1(plow->sw_yuvme_globalNr_strg, phigh->sw_yuvme_globalNr_strg, ratio);
    out->dyn.sw_yuvme_nrDiff_scale = INTERP1(plow->sw_yuvme_nrDiff_scale, phigh->sw_yuvme_nrDiff_scale, ratio);
    out->dyn.sw_yuvme_nrStatic_scale = INTERP1(plow->sw_yuvme_nrStatic_scale, phigh->sw_yuvme_nrStatic_scale, ratio);

    for (int i = 0; i < 16; i++)
    {
        out->dyn.hw_yuvme_nrLuma2Sigma_val[i] = ROUND_I32(INTERP1(plow->hw_yuvme_nrLuma2Sigma_val[i], phigh->hw_yuvme_nrLuma2Sigma_val[i], ratio));
    }

    out->dyn.sw_yuvme_nrFusion_limit = INTERP1(plow->sw_yuvme_nrFusion_limit, phigh->sw_yuvme_nrFusion_limit, ratio);
    out->dyn.sw_yuvme_nrMotion_scale = INTERP1(plow->sw_yuvme_nrMotion_scale, phigh->sw_yuvme_nrMotion_scale, ratio);

    out->dyn.sw_yuvme_curWeight_limit = INTERP1(plow->sw_yuvme_curWeight_limit, phigh->sw_yuvme_curWeight_limit, ratio);
    out->dyn.hw_yuvme_nrFusion_mode = ROUND_I32(INTERP1(plow->hw_yuvme_nrFusion_mode, phigh->hw_yuvme_nrFusion_mode, ratio));


    return XCAM_RETURN_NO_ERROR;
}

#endif

#define RKISP_ALGO_YME_VERSION     "v0.0.9"
#define RKISP_ALGO_YME_VENDOR      "Rockchip"
#define RKISP_ALGO_YME_DESCRIPTION "Rockchip yme algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescYme = {
    .common = {
        .version = RKISP_ALGO_YME_VERSION,
        .vendor  = RKISP_ALGO_YME_VENDOR,
        .description = RKISP_ALGO_YME_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMD,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

