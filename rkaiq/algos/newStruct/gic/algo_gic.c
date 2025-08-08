/*
 *  Copyright (c) 2023 Rockchip Corporation
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

#include "gic_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"

XCamReturn GicSelectParam(GicContext_t *pGicCtx, gic_param_t* out, int iso);

#define GIC_V22_ISO_CURVE_POINT_BIT          4
#define GIC_V22_ISO_CURVE_POINT_NUM          ((1 << GIC_V22_ISO_CURVE_POINT_BIT)+1)

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    GicContext_t* ctx = aiq_mallocz(sizeof(GicContext_t));

    if (ctx == NULL) {
        LOGE_ANR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->gic_attrib =
        (gic_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, gic));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ANR("%s: Gic (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    GicContext_t* pGicCtx = (GicContext_t*)context;
    aiq_free(pGicCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    GicContext_t* pGicCtx = (GicContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pGicCtx->gic_attrib =
                (gic_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, gic));
            pGicCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pGicCtx->working_mode = params->u.prepare.working_mode;
    pGicCtx->gic_attrib =
        (gic_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, gic));
    pGicCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pGicCtx->prepare_params = &params->u.prepare;
    pGicCtx->isReCal_ = true;

    return result;
}

XCamReturn Agic_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    GicContext_t* pGicCtx = (GicContext_t *)inparams->ctx;
    gic_api_attrib_t* gic_attrib = pGicCtx->gic_attrib;

    LOGV_ANR("%s: Gic (enter)\n", __FUNCTION__ );

    if (!gic_attrib) {
        LOGE_ANR("gic_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }
    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pGicCtx->iso);

    outparams->cfg_update = false;

    if (gic_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", gic_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (init) {
        pGicCtx->isReCal_ = true;
    }

    if (inparams->u.proc.is_attrib_update)
        pGicCtx->isReCal_ = true;

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pGicCtx->isReCal_ = true;

    if (pGicCtx->isReCal_) {
        gic_param_t* gic_res = outparams->algoRes;
        gic_res->sta = pGicCtx->gic_attrib->stAuto.sta;
        GicSelectParam(pGicCtx, outparams->algoRes, iso);
        outparams->cfg_update = true;
        outparams->en = gic_attrib->en;
        outparams->bypass = gic_attrib->bypass;
        LOGD_ANR("GIC Process: iso %d, gic en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    pGicCtx->iso = iso;
    pGicCtx->isReCal_ = false;

    LOGV_ANR("%s: Gic (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = inparams->u.proc.iso;
    Agic_processing(inparams, outparams, iso);
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_gic_GetAttrib(const RkAiqAlgoContext *ctx,
                                    gic_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    GicContext_t* pGicCtx = (GicContext_t*)ctx;
    gic_api_attrib_t* gic_attrib = pGicCtx->gic_attrib;

    attr->opMode = gic_attrib->opMode;
    attr->en = gic_attrib->en;
    attr->bypass = gic_attrib->bypass;
    memcpy(&attr->stAuto, &gic_attrib->stAuto, sizeof(gic_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif
XCamReturn GicSelectParam
(
    GicContext_t *pGicCtx,
    gic_param_t* out,
    int iso)
{
    gic_param_auto_t *paut = &pGicCtx->gic_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;

    pre_interp(iso, pGicCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;
#if RKAIQ_HAVE_GIC_V2
    out->dyn.strongEdg_darkArea.hw_gicT_darkArea_minThred = interpolation_u16(
        paut->dyn[ilow].strongEdg_darkArea.hw_gicT_darkArea_minThred, paut->dyn[ihigh].strongEdg_darkArea.hw_gicT_darkArea_minThred, uratio);
    out->dyn.strongEdg_darkArea.hw_gicT_darkArea_maxThred = interpolation_u16(
        paut->dyn[ilow].strongEdg_darkArea.hw_gicT_darkArea_maxThred, paut->dyn[ihigh].strongEdg_darkArea.hw_gicT_darkArea_maxThred, uratio);
    out->dyn.strongEdg_darkArea.hw_gicT_hiGrad2EdgTh_ratio = interpolation_f32(
        paut->dyn[ilow].strongEdg_darkArea.hw_gicT_hiGrad2EdgTh_ratio, paut->dyn[ihigh].strongEdg_darkArea.hw_gicT_hiGrad2EdgTh_ratio, ratio);
    out->dyn.strongEdg_darkArea.hw_gicT_loGrad2EdgTh_ratio = interpolation_f32(
        paut->dyn[ilow].strongEdg_darkArea.hw_gicT_loGrad2EdgTh_ratio, paut->dyn[ihigh].strongEdg_darkArea.hw_gicT_loGrad2EdgTh_ratio, ratio);
    out->dyn.strongEdg_darkArea.hw_gicT_hiEdgTh_minLimit = interpolation_u16(
        paut->dyn[ilow].strongEdg_darkArea.hw_gicT_hiEdgTh_minLimit, paut->dyn[ihigh].strongEdg_darkArea.hw_gicT_hiEdgTh_minLimit, uratio);
    out->dyn.strongEdg_darkArea.hw_gicT_loEdgTh_minLimit = interpolation_u16(
        paut->dyn[ilow].strongEdg_darkArea.hw_gicT_loEdgTh_minLimit, paut->dyn[ihigh].strongEdg_darkArea.hw_gicT_loEdgTh_minLimit, uratio);
    out->dyn.strongEdg_lightArea.hw_gicT_hiGrad2EdgTh_ratio = interpolation_f32(
        paut->dyn[ilow].strongEdg_lightArea.hw_gicT_hiGrad2EdgTh_ratio, paut->dyn[ihigh].strongEdg_lightArea.hw_gicT_hiGrad2EdgTh_ratio, ratio);
    out->dyn.strongEdg_lightArea.hw_gicT_loGrad2EdgTh_ratio = interpolation_f32(
        paut->dyn[ilow].strongEdg_lightArea.hw_gicT_loGrad2EdgTh_ratio, paut->dyn[ihigh].strongEdg_lightArea.hw_gicT_loGrad2EdgTh_ratio, ratio);
    out->dyn.strongEdg_lightArea.hw_gicT_hiEdgTh_minLimit = interpolation_u16(
        paut->dyn[ilow].strongEdg_lightArea.hw_gicT_hiEdgTh_minLimit, paut->dyn[ihigh].strongEdg_lightArea.hw_gicT_hiEdgTh_minLimit, uratio);
    out->dyn.strongEdg_lightArea.hw_gicT_loEdgTh_minLimit = interpolation_u16(
        paut->dyn[ilow].strongEdg_lightArea.hw_gicT_loEdgTh_minLimit, paut->dyn[ihigh].strongEdg_lightArea.hw_gicT_loEdgTh_minLimit, uratio);
    out->dyn.weakEdg.hw_gicT_estNs_offset = interpolation_u16(
        paut->dyn[ilow].weakEdg.hw_gicT_estNs_offset, paut->dyn[ihigh].weakEdg.hw_gicT_estNs_offset, uratio);
    out->dyn.textDct.hw_gicT_textTh_minLimit = interpolation_u16(
        paut->dyn[ilow].textDct.hw_gicT_textTh_minLimit, paut->dyn[ihigh].textDct.hw_gicT_textTh_minLimit, uratio);
    out->dyn.gicProc.hw_gicT_sigma_scale = interpolation_f32(
        paut->dyn[ilow].gicProc.hw_gicT_sigma_scale, paut->dyn[ihigh].gicProc.hw_gicT_sigma_scale, ratio);
    out->dyn.gicProc.hw_gicT_sigma_offset = interpolation_f32(
        paut->dyn[ilow].gicProc.hw_gicT_sigma_offset, paut->dyn[ihigh].gicProc.hw_gicT_sigma_offset, ratio);
    out->dyn.gicProc.hw_gicT_gDiffSoftThd_scale = interpolation_u8(
        paut->dyn[ilow].gicProc.hw_gicT_gDiffSoftThd_scale, paut->dyn[ihigh].gicProc.hw_gicT_gDiffSoftThd_scale, uratio);
    out->dyn.gicProc.hw_gicT_gDiff_maxLimit = interpolation_u16(
        paut->dyn[ilow].gicProc.hw_gicT_gDiff_maxLimit, paut->dyn[ihigh].gicProc.hw_gicT_gDiff_maxLimit, uratio);
    out->dyn.gicProc.hw_gicT_correctGb_strg = interpolation_u8(
        paut->dyn[ilow].gicProc.hw_gicT_correctGb_strg, paut->dyn[ihigh].gicProc.hw_gicT_correctGb_strg, uratio);
    out->dyn.gicProc.hw_gicT_correctGb2Gr_ratio = interpolation_u8(
        paut->dyn[ilow].gicProc.hw_gicT_correctGb2Gr_ratio, paut->dyn[ihigh].gicProc.hw_gicT_correctGb2Gr_ratio, uratio);
#endif

#if RKAIQ_HAVE_GIC_V3
    out->dyn.locGicStrg.locSgmStrg.hw_gicT_locSgmStrg_mode = paut->dyn[inear].locGicStrg.locSgmStrg.hw_gicT_locSgmStrg_mode;
    out->dyn.locGicStrg.locSgmStrg.hw_gicT_locSgmStrg_scale = interpolation_f32(
        paut->dyn[ilow].locGicStrg.locSgmStrg.hw_gicT_locSgmStrg_scale, paut->dyn[ihigh].locGicStrg.locSgmStrg.hw_gicT_locSgmStrg_scale, ratio);
    out->dyn.locGicStrg.locSgmStrg.hw_gicT_glbSgmStrg_val = interpolation_f32(
        paut->dyn[ilow].locGicStrg.locSgmStrg.hw_gicT_glbSgmStrg_val, paut->dyn[ihigh].locGicStrg.locSgmStrg.hw_gicT_glbSgmStrg_val, ratio);
    out->dyn.locGicStrg.locSgmStrg.hw_gicT_glbSgmStrg_alpha = interpolation_f32(
        paut->dyn[ilow].locGicStrg.locSgmStrg.hw_gicT_glbSgmStrg_alpha, paut->dyn[ihigh].locGicStrg.locSgmStrg.hw_gicT_glbSgmStrg_alpha, ratio);
    out->dyn.locGicStrg.locSgmStrg2GicStrg.sw_gicT_locSgmStrgStat_maxThred = interpolation_f32(
        paut->dyn[ilow].locGicStrg.locSgmStrg2GicStrg.sw_gicT_locSgmStrgStat_maxThred, paut->dyn[ihigh].locGicStrg.locSgmStrg2GicStrg.sw_gicT_locSgmStrgStat_maxThred, ratio);
    out->dyn.locGicStrg.locSgmStrg2GicStrg.sw_gicT_locSgmStrgMot_minThred = interpolation_f32(
        paut->dyn[ilow].locGicStrg.locSgmStrg2GicStrg.sw_gicT_locSgmStrgMot_minThred, paut->dyn[ihigh].locGicStrg.locSgmStrg2GicStrg.sw_gicT_locSgmStrgMot_minThred, ratio);
    out->dyn.locGicStrg.locSgmStrg2GicStrg.hw_shpT_statRegionGic_strg = interpolation_f32(
        paut->dyn[ilow].locGicStrg.locSgmStrg2GicStrg.hw_shpT_statRegionGic_strg, paut->dyn[ihigh].locGicStrg.locSgmStrg2GicStrg.hw_shpT_statRegionGic_strg, ratio);
    out->dyn.locGicStrg.locSgmStrg2GicStrg.hw_shpT_motRegionGic_strg = interpolation_f32(
        paut->dyn[ilow].locGicStrg.locSgmStrg2GicStrg.hw_shpT_motRegionGic_strg, paut->dyn[ihigh].locGicStrg.locSgmStrg2GicStrg.hw_shpT_motRegionGic_strg, ratio);
    out->dyn.gicPre_medAndEpf.medFilt.hw_gicT_yFiltClipMin_idx = interpolation_u8(
        paut->dyn[ilow].gicPre_medAndEpf.medFilt.hw_gicT_yFiltClipMin_idx, paut->dyn[ihigh].gicPre_medAndEpf.medFilt.hw_gicT_yFiltClipMin_idx, uratio);
    out->dyn.gicPre_medAndEpf.medFilt.hw_gicT_yFiltClipMax_idx = interpolation_u8(
        paut->dyn[ilow].gicPre_medAndEpf.medFilt.hw_gicT_yFiltClipMax_idx, paut->dyn[ihigh].gicPre_medAndEpf.medFilt.hw_gicT_yFiltClipMax_idx, uratio);
    out->dyn.gicPre_medAndEpf.medFilt.hw_gicT_yFiltOut_alpha = interpolation_f32(
        paut->dyn[ilow].gicPre_medAndEpf.medFilt.hw_gicT_yFiltOut_alpha, paut->dyn[ihigh].gicPre_medAndEpf.medFilt.hw_gicT_yFiltOut_alpha, ratio);
    out->dyn.gicPre_medAndEpf.medFilt.hw_gicT_uvFiltClipMin_idx = interpolation_u8(
        paut->dyn[ilow].gicPre_medAndEpf.medFilt.hw_gicT_uvFiltClipMin_idx, paut->dyn[ihigh].gicPre_medAndEpf.medFilt.hw_gicT_uvFiltClipMin_idx, uratio);
    out->dyn.gicPre_medAndEpf.medFilt.hw_gicT_uvFiltClipMax_idx = interpolation_u8(
        paut->dyn[ilow].gicPre_medAndEpf.medFilt.hw_gicT_uvFiltClipMax_idx, paut->dyn[ihigh].gicPre_medAndEpf.medFilt.hw_gicT_uvFiltClipMax_idx, uratio);
    out->dyn.gicPre_medAndEpf.medFilt.hw_gicT_uvFiltOut_alpha = interpolation_f32(
        paut->dyn[ilow].gicPre_medAndEpf.medFilt.hw_gicT_uvFiltOut_alpha, paut->dyn[ihigh].gicPre_medAndEpf.medFilt.hw_gicT_uvFiltOut_alpha, ratio);
    for (i=0; i<8; i++) {
        out->dyn.lumaLutCfg.hw_gicT_lumaLutIdx_val[i] = interpolation_u16(
            paut->dyn[ilow].lumaLutCfg.hw_gicT_lumaLutIdx_val[i], paut->dyn[ihigh].lumaLutCfg.hw_gicT_lumaLutIdx_val[i], uratio);
    }
    for (i=0; i<17; i++) {
        out->dyn.epf.hw_gicT_luma2Manual_rgeSgm[i] = interpolation_u16(
            paut->dyn[ilow].epf.hw_gicT_luma2Manual_rgeSgm[i], paut->dyn[ihigh].epf.hw_gicT_luma2Manual_rgeSgm[i], uratio);
    }
    out->dyn.epf.sw_gicT_rgeSgm_scale = interpolation_f32(
        paut->dyn[ilow].epf.sw_gicT_rgeSgm_scale, paut->dyn[ihigh].epf.sw_gicT_rgeSgm_scale, ratio);
    out->dyn.epf.diffSgmRat2RgeWgt.sw_gicT_rat2MaxWgt_maxThred = interpolation_f32(
        paut->dyn[ilow].epf.diffSgmRat2RgeWgt.sw_gicT_rat2MaxWgt_maxThred, paut->dyn[ihigh].epf.diffSgmRat2RgeWgt.sw_gicT_rat2MaxWgt_maxThred, ratio);
    out->dyn.epf.diffSgmRat2RgeWgt.sw_gicT_rat2MinWgt_minThred = interpolation_f32(
        paut->dyn[ilow].epf.diffSgmRat2RgeWgt.sw_gicT_rat2MinWgt_minThred, paut->dyn[ihigh].epf.diffSgmRat2RgeWgt.sw_gicT_rat2MinWgt_minThred, ratio);
    out->dyn.epf.sw_gicT_filtSpatial_strg = interpolation_f32(
        paut->dyn[ilow].epf.sw_gicT_filtSpatial_strg, paut->dyn[ihigh].epf.sw_gicT_filtSpatial_strg, ratio);
    out->dyn.epf.sw_gicT_filtOut_alpha = interpolation_f32(
        paut->dyn[ilow].epf.sw_gicT_filtOut_alpha, paut->dyn[ihigh].epf.sw_gicT_filtOut_alpha, ratio);
    for (i=0; i<8; i++) {
        out->dyn.manualSoftThd.hw_gicT_luma2SofThd_thred[i] = interpolation_u16(
            paut->dyn[ilow].manualSoftThd.hw_gicT_luma2SofThd_thred[i], paut->dyn[ihigh].manualSoftThd.hw_gicT_luma2SofThd_thred[i], uratio);
    }
    for (i=0; i<4; i++) {
        out->dyn.gicPost_guideEpf.lpf.hw_gicT_grFiltSpatial_wgt[i] = interpolation_u16(
            paut->dyn[ilow].gicPost_guideEpf.lpf.hw_gicT_grFiltSpatial_wgt[i], paut->dyn[ihigh].gicPost_guideEpf.lpf.hw_gicT_grFiltSpatial_wgt[i], uratio);
    }
    for (i=0; i<2; i++) {
        out->dyn.gicPost_guideEpf.lpf.hw_gicT_gbFiltSpatial_wgt[i] = interpolation_u16(
            paut->dyn[ilow].gicPost_guideEpf.lpf.hw_gicT_gbFiltSpatial_wgt[i], paut->dyn[ihigh].gicPost_guideEpf.lpf.hw_gicT_gbFiltSpatial_wgt[i], uratio);
    }
    out->dyn.gicPost_guideEpf.autoSoftThd.hw_gicT_softThd_scale = interpolation_f32(
        paut->dyn[ilow].gicPost_guideEpf.autoSoftThd.hw_gicT_softThd_scale, paut->dyn[ihigh].gicPost_guideEpf.autoSoftThd.hw_gicT_softThd_scale, ratio);
    for (i=0; i<8; i++) {
        out->dyn.gicPost_guideEpf.autoSoftThd.hw_gicT_luma2Thred_maxLimit[i] = interpolation_u16(
            paut->dyn[ilow].gicPost_guideEpf.autoSoftThd.hw_gicT_luma2Thred_maxLimit[i], paut->dyn[ihigh].gicPost_guideEpf.autoSoftThd.hw_gicT_luma2Thred_maxLimit[i], uratio);
    }
    for (i=0; i<8; i++) {
        out->dyn.gicPost_guideEpf.autoSoftThd.hw_gicT_luma2Thred_minLimit[i] = interpolation_u16(
            paut->dyn[ilow].gicPost_guideEpf.autoSoftThd.hw_gicT_luma2Thred_minLimit[i], paut->dyn[ihigh].gicPost_guideEpf.autoSoftThd.hw_gicT_luma2Thred_minLimit[i], uratio);
    }
    for (i=0; i<2; i++) {
        out->dyn.gicPost_guideEpf.autoSoftThd.hw_gicT_thredFiltSpatial_wgt[i] = interpolation_u16(
            paut->dyn[ilow].gicPost_guideEpf.autoSoftThd.hw_gicT_thredFiltSpatial_wgt[i], paut->dyn[ihigh].gicPost_guideEpf.autoSoftThd.hw_gicT_thredFiltSpatial_wgt[i], uratio);
    }
#endif
    return XCAM_RETURN_NO_ERROR;
}


#define RKISP_ALGO_GIC_VERSION     "v0.0.9"
#define RKISP_ALGO_GIC_VENDOR      "Rockchip"
#define RKISP_ALGO_GIC_DESCRIPTION "Rockchip gic algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescGic = {
    .common = {
        .version = RKISP_ALGO_GIC_VERSION,
        .vendor  = RKISP_ALGO_GIC_VENDOR,
        .description = RKISP_ALGO_GIC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGIC,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

