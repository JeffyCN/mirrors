/*
 * rk_aiq_algo_debayer_itf.c
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

#include "algo_types_priv.h"
#include "cac_types_prvt.h"

#include "c_base/aiq_base.h"
#include "interpolation.h"

//RKAIQ_BEGIN_DECLARE

XCamReturn CacSelectParam(CacContext_t* pCacCtx, cac_param_t* out, int iso);

static XCamReturn
create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    CacContext_t *ctx = aiq_mallocz(sizeof(CacContext_t));
    if (ctx == NULL) {
        LOGE_ACAC( "%s: create Cac context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->cac_attrib =
        (cac_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, cac));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ACAC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext* context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    CacContext_t* pCacCtx = (CacContext_t*)context;
    aiq_free(pCacCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CacContext_t* pCacCtx = (CacContext_t *)params->ctx;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pCacCtx->cac_attrib =
                (cac_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cac));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pCacCtx->cac_attrib =
        (cac_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cac));
    pCacCtx->prepare_params = &params->u.prepare;
    pCacCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    CacContext_t* pCacCtx = (CacContext_t *)inparams->ctx;
    cac_api_attrib_t* cac_attrib = pCacCtx->cac_attrib;
    RkAiqAlgoProcResCac* pCacProcResParams = (RkAiqAlgoProcResCac*)outparams;
    RkAiqAlgoProcCac* cac_proc_param = (RkAiqAlgoProcCac*)inparams;

    LOGV_ACAC("%s: Cac (enter)\n", __FUNCTION__ );

    if (!cac_attrib) {
        LOGE_ACAC("cac_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    int iso = cac_proc_param->iso;
    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pCacCtx->iso);
    pCacCtx->iso = iso;

    outparams->cfg_update = false;

    if (cac_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACAC("mode is %d, not auto mode, ignore", cac_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pCacCtx->isReCal_ = true;
    }

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO) {
        pCacCtx->isReCal_ = true;
    }
    if (!pCacCtx->cac_attrib->en) {
        outparams->en = false;
        if (pCacCtx->isReCal_) {
            outparams->cfg_update = true;
            pCacCtx->isReCal_ = false;
        }
        return XCAM_RETURN_NO_ERROR;
    }

    if (pCacCtx->isReCal_) {
        CacSelectParam(pCacCtx, pCacProcResParams->cacRes, iso);
        outparams->cfg_update = true;
        outparams->en = cac_attrib->en;
        outparams->bypass = cac_attrib->bypass;
        LOGD_ACAC("cac en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pCacCtx->isReCal_ = false;

    LOGV_ACAC("%s: Cac (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}
#if RKAIQ_HAVE_CAC_V11
XCamReturn CacSelectParam(CacContext_t* pCacCtx, cac_param_t* out, int iso)
{
    LOGD_ACAC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pCacCtx == NULL) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    cac_param_auto_t* paut = &pCacCtx->cac_attrib->stAuto;
    int i;
    int iso_low  = 50;
    int iso_high = 50;
    int gain_high, gain_low;
    float ratio      = 1.0;
    int iso_div      = 50;
    int max_iso_step = CAC_ISO_STEP_MAX;

    for (i = max_iso_step - 1; i >= 0; i--) {
        if (iso < iso_div * (2 << i)) {
            iso_low  = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    uint16_t uratio = ratio * (1 << RATIO_FIXBIT);
    if (iso_low == iso) {
        iso_high = iso;
        ratio    = 0;
    }
    if (iso_high == iso) {
        iso_low = iso;
        ratio   = 1;
    }
    gain_high = (int)(log((float)(iso_high) / 50) / log(2.0));
    gain_low  = (int)(log((float)(iso_low) / 50) / log(2.0));

    gain_low  = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high = MIN(MAX(gain_high, 0), max_iso_step - 1);

    XCAM_ASSERT(gain_low >= 0 && gain_low < max_iso_step);
    XCAM_ASSERT(gain_high >= 0 && gain_high < max_iso_step);

    out->sta = paut->sta;

    out->dyn.strgInterp.sw_cacT_globalCorr_strg =
            interpolation_f32(paut->dyn[gain_low].strgInterp.sw_cacT_globalCorr_strg, paut->dyn[gain_high].strgInterp.sw_cacT_globalCorr_strg, ratio);
    float strenth_low = 0.0;
    float strenth_high = 0.0;
    for (i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        strenth_low = paut->dyn[gain_low].strgInterp.sw_cacT_globalCorr_strg > 0
            ? paut->dyn[gain_low].strgInterp.sw_cacT_globalCorr_strg
            : paut->dyn[gain_low].strgInterp.hw_cacT_corrStrength_table[i];
        strenth_high = paut->dyn[gain_high].strgInterp.sw_cacT_globalCorr_strg > 0
            ? paut->dyn[gain_high].strgInterp.sw_cacT_globalCorr_strg
            : paut->dyn[gain_high].strgInterp.hw_cacT_corrStrength_table[i];
        out->dyn.strgInterp.hw_cacT_corrStrength_table[i] =
            interpolation_f32(strenth_low, strenth_high, ratio);
    }

    int clip_g_mode1 = paut->dyn[gain_low].chromaAberrCorr.hw_cacT_clipG_mode;
    int clip_g_mode2 = paut->dyn[gain_high].chromaAberrCorr.hw_cacT_clipG_mode;
    out->dyn.chromaAberrCorr.hw_cacT_clipG_mode =
        (cac_clipGT_mode_t) interpolation_u32(clip_g_mode1, clip_g_mode1, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_negClip0_en =
        interpolation_bool(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_negClip0_en,
                    paut->dyn[gain_high].chromaAberrCorr.hw_cacT_negClip0_en, uratio);
    out->dyn.strgInterp.hw_cacT_edgeDetect_en =
        interpolation_bool(paut->dyn[gain_low].strgInterp.hw_cacT_edgeDetect_en,
                    paut->dyn[gain_high].strgInterp.hw_cacT_edgeDetect_en, uratio);
    out->dyn.strgInterp.hw_cacT_flatDctB_thred =
        interpolation_f32(paut->dyn[gain_low].strgInterp.hw_cacT_flatDctB_thred,
            paut->dyn[gain_high].strgInterp.hw_cacT_flatDctB_thred, ratio);
    out->dyn.strgInterp.hw_cacT_flatDctR_thred =
        interpolation_f32(paut->dyn[gain_low].strgInterp.hw_cacT_flatDctR_thred,
            paut->dyn[gain_high].strgInterp.hw_cacT_flatDctR_thred, ratio);
    out->dyn.strgInterp.hw_cacT_flatDctB_offset =
        interpolation_u32(paut->dyn[gain_low].strgInterp.hw_cacT_flatDctB_offset,
            paut->dyn[gain_high].strgInterp.hw_cacT_flatDctB_offset, ratio);
    out->dyn.strgInterp.hw_cacT_flatDctR_offset =
        interpolation_u32(paut->dyn[gain_low].strgInterp.hw_cacT_flatDctR_offset,
            paut->dyn[gain_high].strgInterp.hw_cacT_flatDctR_offset, ratio);
    out->dyn.chromaAberrCorr.sw_cacT_expoDctB_en =
        interpolation_bool(paut->dyn[gain_low].chromaAberrCorr.sw_cacT_expoDctB_en,
            paut->dyn[gain_high].chromaAberrCorr.sw_cacT_expoDctB_en, uratio);
    out->dyn.chromaAberrCorr.sw_cacT_expoDctR_en =
        interpolation_bool(paut->dyn[gain_low].chromaAberrCorr.sw_cacT_expoDctR_en,
            paut->dyn[gain_high].chromaAberrCorr.sw_cacT_expoDctR_en, uratio);

    out->dyn.chromaAberrCorr.hw_cacT_overExpoB_thred =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_overExpoB_thred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_overExpoB_thred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_overExpoR_thred =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_overExpoR_thred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_overExpoR_thred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_overExpoB_adj =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_overExpoB_adj,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_overExpoB_adj, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_overExpoR_adj =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_overExpoR_adj,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_overExpoR_adj, ratio);

    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_CAC_V30
XCamReturn CacSelectParam(CacContext_t *pCacCtx, cac_param_t* out, int iso)
{
      if(pCacCtx == NULL) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    cac_param_auto_t* paut = &pCacCtx->cac_attrib->stAuto;
    int i;
    int iso_low  = 50;
    int iso_high = 50;
    int gain_high, gain_low;
    float ratio      = 1.0;
    int iso_div      = 50;
    int max_iso_step = CAC_ISO_STEP_MAX;
    for (i = max_iso_step - 1; i >= 0; i--) {
        if (iso < iso_div * (2 << i)) {
            iso_low  = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    uint16_t uratio = ratio * (1 << RATIO_FIXBIT);
    if (iso_low == iso) {
        iso_high = iso;
        ratio    = 0;
    }
    if (iso_high == iso) {
        iso_low = iso;
        ratio   = 1;
    }
    LOGD_ACAC("%s:iso=%d,iso_high=%d,iso_low=%d,ratio=%f\n", __FUNCTION__, iso,iso_high,iso_low,ratio);
    gain_high = (int)(log((float)(iso_high) / 50) / log(2.0));
    gain_low  = (int)(log((float)(iso_low) / 50) / log(2.0));

    gain_low  = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high = MIN(MAX(gain_high, 0), max_iso_step - 1);

    XCAM_ASSERT(gain_low >= 0 && gain_low < max_iso_step);
    XCAM_ASSERT(gain_high >= 0 && gain_high < max_iso_step);

    out->sta.hw_cacT_psfTableFix_bit
        = paut->sta.hw_cacT_psfTableFix_bit;
    out->dyn.psfTable.hw_cacT_bluePsf_sigma = interpolation_f32(paut->dyn[gain_low].psfTable.hw_cacT_bluePsf_sigma,
            paut->dyn[gain_high].psfTable.hw_cacT_bluePsf_sigma, ratio);
    out->dyn.psfTable.hw_cacT_redPsf_sigma = interpolation_f32(paut->dyn[gain_low].psfTable.hw_cacT_redPsf_sigma,
            paut->dyn[gain_high].psfTable.hw_cacT_redPsf_sigma, ratio);
    out->dyn.gInterp.hw_cacT_hiDrct_ratio = interpolation_f32(paut->dyn[gain_low].gInterp.hw_cacT_hiDrct_ratio,
            paut->dyn[gain_high].gInterp.hw_cacT_hiDrct_ratio, ratio);


    out->dyn.chromaAberrCorr.hw_cacT_chromaLoFlt_coeff0 =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_chromaLoFlt_coeff0,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_chromaLoFlt_coeff0, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_chromaLoFlt_coeff1 =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_chromaLoFlt_coeff1,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_chromaLoFlt_coeff1, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_searchRange_ratio =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_searchRange_ratio,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_searchRange_ratio, ratio);


    out->dyn.chromaAberrCorr.hw_cacT_wgtColorB_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorB_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorB_minThred, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorB_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorB_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorB_maxThred, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorR_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorR_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorR_minThred, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorR_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorR_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorR_maxThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtColor_minLuma =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColor_minLuma,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColor_minLuma, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColor_maxLuma =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColor_maxLuma,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColor_maxLuma, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_residualChroma_ratio =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_residualChroma_ratio,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_residualChroma_ratio, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff0 =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff0,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff0, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff1 =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff1,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff1, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtOverExpo_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtOverExpo_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtOverExpo_minThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtOverExpo_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtOverExpo_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtOverExpo_maxThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_contrast_offset =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_contrast_offset,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_contrast_offset, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtContrast_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtContrast_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtContrast_minThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtContrast_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtContrast_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtContrast_maxThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtDarkArea_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtDarkArea_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtDarkArea_minThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtDarkArea_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtDarkArea_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtDarkArea_maxThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtColor_en =
        interpolation_bool(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColor_en,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColor_en, ratio);


     out->dyn.edgeDetect.hw_cacT_edgeDetect_en =
        interpolation_bool(paut->dyn[gain_low].edgeDetect.hw_cacT_edgeDetect_en,
                    paut->dyn[gain_high].edgeDetect.hw_cacT_edgeDetect_en, ratio);
    out->dyn.edgeDetect.hw_cacT_flat_thred =
        interpolation_f32(paut->dyn[gain_low].edgeDetect.hw_cacT_flat_thred,
            paut->dyn[gain_high].edgeDetect.hw_cacT_flat_thred, ratio);
    out->dyn.edgeDetect.hw_cacT_flat_offset =
        interpolation_u32(paut->dyn[gain_low].edgeDetect.hw_cacT_flat_offset,
            paut->dyn[gain_high].edgeDetect.hw_cacT_flat_offset, ratio);
     out->dyn.hfCalc.hw_cacT_negClip0_en =
        interpolation_bool(paut->dyn[gain_low].hfCalc.hw_cacT_negClip0_en,
                    paut->dyn[gain_high].hfCalc.hw_cacT_negClip0_en, ratio);
     out->dyn.hfCalc.hw_cacT_overExpoDetect_en =
        interpolation_bool(paut->dyn[gain_low].hfCalc.hw_cacT_overExpoDetect_en,
                    paut->dyn[gain_high].hfCalc.hw_cacT_overExpoDetect_en, ratio);
     out->dyn.hfCalc.hw_cacT_overExpo_thred =
        interpolation_f32(paut->dyn[gain_low].hfCalc.hw_cacT_overExpo_thred,
                    paut->dyn[gain_high].hfCalc.hw_cacT_overExpo_thred, ratio);
     out->dyn.hfCalc.hw_cacT_overExpo_adj =
        interpolation_f32(paut->dyn[gain_low].hfCalc.hw_cacT_overExpo_adj,
                    paut->dyn[gain_high].hfCalc.hw_cacT_overExpo_adj, ratio);
    return XCAM_RETURN_NO_ERROR;
}
#endif
#if 0
XCamReturn
algo_cac_SetAttrib(RkAiqAlgoContext* ctx, cac_api_attrib_t* attr) {
    if(ctx == NULL || attr == NULL) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    CacContext_t* pCacCtx = (CacContext_t*)ctx;
    cac_api_attrib_t* cac_attrib = pCacCtx->cac_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACAC("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    cac_attrib->opMode = attr->opMode;
    cac_attrib->en = attr->en;
    cac_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        cac_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        cac_attrib->stMan = attr->stMan;
    else {
        LOGW_ACAC("wrong mode: %d\n", attr->opMode);
    }

    pCacCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
algo_cac_GetAttrib(RkAiqAlgoContext* ctx, cac_api_attrib_t* attr)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CacContext_t* pCacCtx = (CacContext_t*)ctx;
    cac_api_attrib_t* cac_attrib = pCacCtx->cac_attrib;

#if 0
    if (cac_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACAC("not auto mode: %d", cac_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = cac_attrib->opMode;
    attr->en = cac_attrib->en;
    attr->bypass = cac_attrib->bypass;
    memcpy(&attr->stAuto, &cac_attrib->stAuto, sizeof(cac_param_auto_t));
    return XCAM_RETURN_NO_ERROR;
}
#endif
#define RKISP_ALGO_CAC_VERSION     "v0.0.1"
#define RKISP_ALGO_CAC_VENDOR      "Rockchip"
#define RKISP_ALGO_CAC_DESCRIPTION "Rockchip cac algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCac = {
    .common = {
        .version = RKISP_ALGO_CAC_VERSION,
        .vendor  = RKISP_ALGO_CAC_VENDOR,
        .description = RKISP_ALGO_CAC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACAC,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

//RKAIQ_END_DECLARE
