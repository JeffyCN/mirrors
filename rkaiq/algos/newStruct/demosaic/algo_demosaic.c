/*
 * rk_aiq_algo_debayer_itf.c
 *
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
#include "demosaic_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

#include "interpolation.h"
#include "c_base/aiq_base.h"

//RKAIQ_BEGIN_DECLARE

#if RKAIQ_HAVE_DEBAYER_V2
static XCamReturn DmSelectParam
(
    DmContext_t *pDmCtx,
    dm_param_t* out,
    int iso)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pDmCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    dm_param_auto_t *paut = &pDmCtx->dm_attrib->stAuto;
    // test use default iso list;
    //int *iso_list = (int *)pDmCtx->p_auto_param->iso;

    pre_interp(iso, pDmCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    //iso_low = iso_list[ilow];
    //iso_high = iso_list[ihigh];
    //printf("pre_interp %d, [%d,%d], ratio %f\n", iso, iso_low,iso_high,ratio);

    out->dyn.gInterp.hw_dmT_gInterpClip_en = interpolation_bool(
                paut->dyn[ilow].gInterp.hw_dmT_gInterpClip_en, paut->dyn[ihigh].gInterp.hw_dmT_gInterpClip_en, uratio);
    out->dyn.gInterp.hw_dmT_gInterpSharpStrg_offset = interpolation_u16(
                paut->dyn[ilow].gInterp.hw_dmT_gInterpSharpStrg_offset, paut->dyn[ihigh].gInterp.hw_dmT_gInterpSharpStrg_offset, uratio);
    out->dyn.gInterp.hw_dmT_gInterpSharpStrg_maxLim = interpolation_u8(
                paut->dyn[ilow].gInterp.hw_dmT_gInterpSharpStrg_maxLim, paut->dyn[ihigh].gInterp.hw_dmT_gInterpSharpStrg_maxLim, uratio);
    out->dyn.gDrctAlpha.hw_dmT_drct_offset = interpolation_u16(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_drct_offset, paut->dyn[ihigh].gDrctAlpha.hw_dmT_drct_offset, uratio);
    out->dyn.gDrctAlpha.hw_dmT_drctMethod_thred = interpolation_u8(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_drctMethod_thred, paut->dyn[ihigh].gDrctAlpha.hw_dmT_drctMethod_thred, uratio);
    out->dyn.gDrctAlpha.hw_dmT_hiDrct_thred = interpolation_u8(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_hiDrct_thred, paut->dyn[ihigh].gDrctAlpha.hw_dmT_hiDrct_thred, uratio);
    out->dyn.gDrctAlpha.hw_dmT_loDrct_thred = interpolation_u8(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_loDrct_thred, paut->dyn[ihigh].gDrctAlpha.hw_dmT_loDrct_thred, uratio);
    out->dyn.gDrctAlpha.hw_dmT_hiTexture_thred = interpolation_u8(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_hiTexture_thred, paut->dyn[ihigh].gDrctAlpha.hw_dmT_hiTexture_thred, uratio);
    out->dyn.gOutlsFlt.hw_dmT_gOutlsFlt_en = interpolation_u8(
                paut->dyn[ilow].gOutlsFlt.hw_dmT_gOutlsFlt_en, paut->dyn[ihigh].gOutlsFlt.hw_dmT_gOutlsFlt_en, uratio);
    out->dyn.gOutlsFlt.hw_dmT_gOutlsFltRange_offset = interpolation_u16(
                paut->dyn[ilow].gOutlsFlt.hw_dmT_gOutlsFltRange_offset, paut->dyn[ihigh].gOutlsFlt.hw_dmT_gOutlsFltRange_offset, uratio);
    out->dyn.cFlt.hw_dmT_cnrFlt_en = interpolation_bool(
                                         paut->dyn[ilow].cFlt.hw_dmT_cnrFlt_en, paut->dyn[ihigh].cFlt.hw_dmT_cnrFlt_en, uratio);
    out->dyn.cFlt.hw_dmT_cnrLogGuide_offset = interpolation_u16(
                paut->dyn[ilow].cFlt.hw_dmT_cnrLogGuide_offset, paut->dyn[ihigh].cFlt.hw_dmT_cnrLogGuide_offset, uratio);
    out->dyn.cFlt.sw_dmT_cnrLoFlt_vsigma = interpolation_f32(
            paut->dyn[ilow].cFlt.sw_dmT_cnrLoFlt_vsigma, paut->dyn[ihigh].cFlt.sw_dmT_cnrLoFlt_vsigma, ratio);
    out->dyn.cFlt.sw_dmT_cnrLoFltWgt_maxLimit = interpolation_f32(
                paut->dyn[ilow].cFlt.sw_dmT_cnrLoFltWgt_maxLimit, paut->dyn[ihigh].cFlt.sw_dmT_cnrLoFltWgt_maxLimit, ratio);
    out->dyn.cFlt.sw_dmT_cnrLoFltWgt_minThred = interpolation_f32(
                paut->dyn[ilow].cFlt.sw_dmT_cnrLoFltWgt_minThred, paut->dyn[ihigh].cFlt.sw_dmT_cnrLoFltWgt_minThred, ratio);
    out->dyn.cFlt.sw_dmT_cnrLoFltWgt_slope = interpolation_f32(
                paut->dyn[ilow].cFlt.sw_dmT_cnrLoFltWgt_slope, paut->dyn[ihigh].cFlt.sw_dmT_cnrLoFltWgt_slope, ratio);
    out->dyn.cFlt.sw_dmT_cnrHiFlt_vsigma = interpolation_f32(
            paut->dyn[ilow].cFlt.sw_dmT_cnrHiFlt_vsigma, paut->dyn[ihigh].cFlt.sw_dmT_cnrHiFlt_vsigma, ratio);
    out->dyn.cFlt.hw_dmT_cnrHiFltWgt_minLimit = interpolation_u8(
                paut->dyn[ilow].cFlt.hw_dmT_cnrHiFltWgt_minLimit, paut->dyn[ihigh].cFlt.hw_dmT_cnrHiFltWgt_minLimit, uratio);
    out->dyn.cFlt.hw_dmT_cnrHiFltCur_wgt = interpolation_u8(
            paut->dyn[ilow].cFlt.hw_dmT_cnrHiFltCur_wgt, paut->dyn[ihigh].cFlt.hw_dmT_cnrHiFltCur_wgt, uratio);
    out->dyn.cFlt.hw_dmT_cnrLogGrad_offset = interpolation_u16(
                paut->dyn[ilow].cFlt.hw_dmT_cnrLogGrad_offset, paut->dyn[ihigh].cFlt.hw_dmT_cnrLogGrad_offset, uratio);
    out->dyn.cFlt.hw_dmT_cnrMoireAlpha_offset = interpolation_u16(
                paut->dyn[ilow].cFlt.hw_dmT_cnrMoireAlpha_offset, paut->dyn[ihigh].cFlt.hw_dmT_cnrMoireAlpha_offset, uratio);
    out->dyn.cFlt.sw_dmT_cnrMoireAlpha_scale = interpolation_f32(
                paut->dyn[ilow].cFlt.sw_dmT_cnrMoireAlpha_scale, paut->dyn[ihigh].cFlt.sw_dmT_cnrMoireAlpha_scale, ratio);
    out->dyn.cFlt.hw_dmT_cnrEdgeAlpha_offset = interpolation_u16(
                paut->dyn[ilow].cFlt.hw_dmT_cnrEdgeAlpha_offset, paut->dyn[ihigh].cFlt.hw_dmT_cnrEdgeAlpha_offset, uratio);
    out->dyn.cFlt.sw_dmT_cnrEdgeAlpha_scale = interpolation_f32(
                paut->dyn[ilow].cFlt.sw_dmT_cnrEdgeAlpha_scale, paut->dyn[ihigh].cFlt.sw_dmT_cnrEdgeAlpha_scale, ratio);

    // static
    out->sta = paut->sta;

    //debug_print_dm_params_dyn_t(&out->dyn);
    return XCAM_RETURN_NO_ERROR;
}
#elif RKAIQ_HAVE_DEBAYER_V3 || RKAIQ_HAVE_DEBAYER_V4
static XCamReturn DmSelectParam
(
    DmContext_t *pDmCtx,
    dm_param_t* out,
    int iso)
{
    LOGI_ADEBAYER("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pDmCtx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    dm_param_auto_t *paut = &pDmCtx->dm_attrib->stAuto;
    // test use default iso list;
    //int *iso_list = (int *)pDmCtx->p_auto_param->iso;

    pre_interp(iso, pDmCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    out->dyn.gInterp.hw_dmT_gInterpClip_en = paut->dyn[inear].gInterp.hw_dmT_gInterpClip_en;
    out->dyn.gInterp.hw_dmT_gInterpSharpStrg_offset = interpolation_u16(
                paut->dyn[ilow].gInterp.hw_dmT_gInterpSharpStrg_offset, paut->dyn[ihigh].gInterp.hw_dmT_gInterpSharpStrg_offset, uratio);
    out->dyn.gInterp.hw_dmT_gInterpSharpStrg_maxLim = interpolation_u8(
                paut->dyn[ilow].gInterp.hw_dmT_gInterpSharpStrg_maxLim, paut->dyn[ihigh].gInterp.hw_dmT_gInterpSharpStrg_maxLim, uratio);
#if ISP_HW_V39
    out->dyn.gInterp.hw_dmT_gInterpWgtFlt_alpha = interpolation_f32(
                paut->dyn[ilow].gInterp.hw_dmT_gInterpWgtFlt_alpha, paut->dyn[ihigh].gInterp.hw_dmT_gInterpWgtFlt_alpha, ratio);
#endif
    for (i = 0; i < 8; i++) {
        out->dyn.gDrctAlpha.hw_dmT_luma2Drct_offset[i] = interpolation_u16(
                    paut->dyn[ilow].gDrctAlpha.hw_dmT_luma2Drct_offset[i], paut->dyn[ihigh].gDrctAlpha.hw_dmT_luma2Drct_offset[i], uratio);
    }
    out->dyn.gDrctAlpha.hw_dmT_drctMethod_thred = interpolation_u8(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_drctMethod_thred, paut->dyn[ihigh].gDrctAlpha.hw_dmT_drctMethod_thred, uratio);
    out->dyn.gDrctAlpha.hw_dmT_hiDrct_thred = interpolation_u8(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_hiDrct_thred, paut->dyn[ihigh].gDrctAlpha.hw_dmT_hiDrct_thred, uratio);
    out->dyn.gDrctAlpha.hw_dmT_loDrct_thred = interpolation_u8(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_loDrct_thred, paut->dyn[ihigh].gDrctAlpha.hw_dmT_loDrct_thred, uratio);
    out->dyn.gDrctAlpha.hw_dmT_hiTexture_thred = interpolation_u8(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_hiTexture_thred, paut->dyn[ihigh].gDrctAlpha.hw_dmT_hiTexture_thred, uratio);
    out->dyn.gDrctAlpha.hw_dmT_gradLoFlt_alpha = interpolation_f32(
                paut->dyn[ilow].gDrctAlpha.hw_dmT_gradLoFlt_alpha, paut->dyn[ihigh].gDrctAlpha.hw_dmT_gradLoFlt_alpha, ratio);
    out->dyn.hw_dmT_gOutlsFlt_en = paut->dyn[inear].hw_dmT_gOutlsFlt_en;
    out->dyn.hw_dmT_gOutlsFlt_mode = paut->dyn[inear].hw_dmT_gOutlsFlt_mode;
    out->dyn.gOutlsFlt_maxMin.hw_dmT_gOutlsFltRange_offset = interpolation_u16(
                paut->dyn[ilow].gOutlsFlt_maxMin.hw_dmT_gOutlsFltRange_offset, paut->dyn[ihigh].gOutlsFlt_maxMin.hw_dmT_gOutlsFltRange_offset, uratio);
    out->dyn.gOutlsFlt_bifilt.sw_dmT_filtCfg_mode = paut->dyn[inear].gOutlsFlt_bifilt.sw_dmT_filtCfg_mode;
    out->dyn.gOutlsFlt_bifilt.sw_dmT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].gOutlsFlt_bifilt.sw_dmT_filtSpatial_strg, paut->dyn[ihigh].gOutlsFlt_bifilt.sw_dmT_filtSpatial_strg, ratio);
    for (i = 0; i < 3; i++) {
        out->dyn.gOutlsFlt_bifilt.hw_dmT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].gOutlsFlt_bifilt.hw_dmT_filtSpatial_wgt[i], paut->dyn[ihigh].gOutlsFlt_bifilt.hw_dmT_filtSpatial_wgt[i], ratio);
    }
    for (i = 0; i < 8; i++) {
        out->dyn.gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[i] = interpolation_u16(
                    paut->dyn[ilow].gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[i], paut->dyn[ihigh].gOutlsFlt_bifilt.hw_dmT_luma2RgeSgm_val[i], uratio);
    }
    out->dyn.gOutlsFlt_bifilt.hw_dmT_bifilt_alpha = interpolation_f32(
                paut->dyn[ilow].gOutlsFlt_bifilt.hw_dmT_bifilt_alpha, paut->dyn[ihigh].gOutlsFlt_bifilt.hw_dmT_bifilt_alpha, ratio);
#if RKAIQ_HAVE_DEBAYER_V3
    out->dyn.cFlt.hw_dmT_cnrFlt_en = paut->dyn[inear].cFlt.hw_dmT_cnrFlt_en;
    out->dyn.cFlt.hw_dmT_cnrLogGuide_offset = interpolation_u16(
                paut->dyn[ilow].cFlt.hw_dmT_cnrLogGuide_offset, paut->dyn[ihigh].cFlt.hw_dmT_cnrLogGuide_offset, uratio);
    out->dyn.cFlt.sw_dmT_cnrLoFlt_rgeSgm = interpolation_f32(
            paut->dyn[ilow].cFlt.sw_dmT_cnrLoFlt_rgeSgm, paut->dyn[ihigh].cFlt.sw_dmT_cnrLoFlt_rgeSgm, ratio);
    out->dyn.cFlt.hw_dmT_cnrLoFltWgt_maxLimit = interpolation_f32(
                paut->dyn[ilow].cFlt.hw_dmT_cnrLoFltWgt_maxLimit, paut->dyn[ihigh].cFlt.hw_dmT_cnrLoFltWgt_maxLimit, ratio);
    out->dyn.cFlt.hw_dmT_cnrLoFltWgt_minThred = interpolation_f32(
                paut->dyn[ilow].cFlt.hw_dmT_cnrLoFltWgt_minThred, paut->dyn[ihigh].cFlt.hw_dmT_cnrLoFltWgt_minThred, ratio);
    out->dyn.cFlt.hw_dmT_cnrLoFltWgt_slope = interpolation_f32(
                paut->dyn[ilow].cFlt.hw_dmT_cnrLoFltWgt_slope, paut->dyn[ihigh].cFlt.hw_dmT_cnrLoFltWgt_slope, ratio);
    out->dyn.cFlt.sw_dmT_cnrHiFlt_rgeSgm = interpolation_f32(
            paut->dyn[ilow].cFlt.sw_dmT_cnrHiFlt_rgeSgm, paut->dyn[ihigh].cFlt.sw_dmT_cnrHiFlt_rgeSgm, ratio);
    out->dyn.cFlt.hw_dmT_cnrHiFltWgt_minLimit = interpolation_u8(
                paut->dyn[ilow].cFlt.hw_dmT_cnrHiFltWgt_minLimit, paut->dyn[ihigh].cFlt.hw_dmT_cnrHiFltWgt_minLimit, uratio);
    out->dyn.cFlt.hw_dmT_cnrHiFltCur_wgt = interpolation_u8(
            paut->dyn[ilow].cFlt.hw_dmT_cnrHiFltCur_wgt, paut->dyn[ihigh].cFlt.hw_dmT_cnrHiFltCur_wgt, uratio);
    out->dyn.cFlt.hw_dmT_cnrLogGrad_offset = interpolation_u16(
                paut->dyn[ilow].cFlt.hw_dmT_cnrLogGrad_offset, paut->dyn[ihigh].cFlt.hw_dmT_cnrLogGrad_offset, uratio);
    out->dyn.cFlt.hw_dmT_cnrMoireAlpha_offset = interpolation_u16(
                paut->dyn[ilow].cFlt.hw_dmT_cnrMoireAlpha_offset, paut->dyn[ihigh].cFlt.hw_dmT_cnrMoireAlpha_offset, uratio);
    out->dyn.cFlt.hw_dmT_cnrMoireAlpha_scale = interpolation_f32(
                paut->dyn[ilow].cFlt.hw_dmT_cnrMoireAlpha_scale, paut->dyn[ihigh].cFlt.hw_dmT_cnrMoireAlpha_scale, ratio);
    out->dyn.cFlt.hw_dmT_cnrEdgeAlpha_offset = interpolation_u16(
                paut->dyn[ilow].cFlt.hw_dmT_cnrEdgeAlpha_offset, paut->dyn[ihigh].cFlt.hw_dmT_cnrEdgeAlpha_offset, uratio);
    out->dyn.cFlt.hw_dmT_cnrEdgeAlpha_scale = interpolation_f32(
                paut->dyn[ilow].cFlt.hw_dmT_cnrEdgeAlpha_scale, paut->dyn[ihigh].cFlt.hw_dmT_cnrEdgeAlpha_scale, ratio);
#endif
    return XCAM_RETURN_NO_ERROR;
}
#endif


static XCamReturn
create_context
(
    RkAiqAlgoContext** context,
    const AlgoCtxInstanceCfg* cfg
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    DmContext_t *ctx = aiq_mallocz(sizeof(DmContext_t));
    if (ctx == NULL) {
        LOGE_ADEBAYER( "%s: create Dm context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->dm_attrib =
        (dm_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, demosaic));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ADEBAYER("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context
(
    RkAiqAlgoContext* context
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DmContext_t* pDmCtx = (DmContext_t*)context;
    aiq_free(pDmCtx);
    return result;
}

static XCamReturn
prepare
(
    RkAiqAlgoCom* params
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    DmContext_t* pDmCtx = (DmContext_t *)params->ctx;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pDmCtx->dm_attrib =
                (dm_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, demosaic));
            pDmCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pDmCtx->dm_attrib =
        (dm_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, demosaic));
    pDmCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pDmCtx->prepare_params = &params->u.prepare;
    pDmCtx->isReCal_ = true;

    return result;
}

XCamReturn Adm_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    DmContext_t* pDmCtx = (DmContext_t *)inparams->ctx;
    dm_api_attrib_t* dm_attrib = pDmCtx->dm_attrib;
    dm_param_t* dm_res = outparams->algoRes;

    if (!dm_attrib) {
        LOGE_ADEBAYER("dm_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pDmCtx->iso);

    outparams->cfg_update = false;

    if (inparams->u.proc.is_bw_sensor) {
#if RKAIQ_HAVE_DEBAYER_V4
        dm_attrib->en = true; //drc relay on dm
        dm_attrib->bypass = true;
#else
        dm_attrib->en = false;
#endif
        outparams->cfg_update = init ? true : false;
        outparams->en = dm_attrib->en;
        outparams->bypass = dm_attrib->bypass;
        LOGI_ADEBAYER("delta_iso:%d, dm en:%d, bypass:%d", delta_iso, outparams->en, outparams->bypass);
        return XCAM_RETURN_NO_ERROR;
    }

    if (dm_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ADEBAYER("mode is %d, not auto mode, ignore", dm_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pDmCtx->isReCal_ = true;
    }

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pDmCtx->isReCal_ = true;

    if (pDmCtx->isReCal_) {
        dm_res->sta = pDmCtx->dm_attrib->stAuto.sta;
        DmSelectParam(pDmCtx, dm_res, iso);

        outparams->cfg_update = true;
        outparams->en = dm_attrib->en;
        outparams->bypass = dm_attrib->bypass;
        LOGI_ADEBAYER("delta_iso:%d, dm en:%d, bypass:%d", delta_iso, outparams->en, outparams->bypass);
    }

    pDmCtx->iso = iso;
    pDmCtx->isReCal_ = false;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    int iso = inparams->u.proc.iso;
    Adm_processing(inparams, outparams, iso);

    LOGV_ADEBAYER("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_demosaic_SetAttrib
(
    RkAiqAlgoContext* ctx,
    dm_api_attrib_t *attr
) {
    if(ctx == NULL || attr == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DmContext_t* pDmCtx = (DmContext_t*)ctx;
    pDmCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_demosaic_GetAttrib
(
    RkAiqAlgoContext* ctx,
    dm_api_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DmContext_t* pDmCtx = (DmContext_t*)ctx;
    dm_api_attrib_t* dm_attrib = pDmCtx->dm_attrib;

    attr->opMode = dm_attrib->opMode;
    attr->en = dm_attrib->en;
    attr->bypass = dm_attrib->bypass;
    memcpy(&attr->stAuto, &dm_attrib->stAuto, sizeof(dm_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#define RKISP_ALGO_DM_VERSION     "v0.0.1"
#define RKISP_ALGO_DM_VENDOR      "Rockchip"
#define RKISP_ALGO_DM_DESCRIPTION "Rockchip demosaic algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescDemosaic = {
    .common = {
        .version = RKISP_ALGO_DM_VERSION,
        .vendor  = RKISP_ALGO_DM_VENDOR,
        .description = RKISP_ALGO_DM_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADEBAYER,
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
