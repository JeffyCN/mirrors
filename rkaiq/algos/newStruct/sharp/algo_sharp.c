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
#include "sharp_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"
#include "newStruct/algo_common.h"

#if RKAIQ_HAVE_SHARP_V41
XCamReturn SharpSelectParam
(
    SharpContext_t *pSharpCtx,
    sharp_param_t* out,
    int iso,
    bool is_aibnr_autorun,
    int aibnr_fixIndex)
{
    sharp_param_auto_t *paut = &pSharpCtx->sharp_attrib->stAuto;
    if(paut == NULL || out == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, pSharpCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    if (aibnr_fixIndex != -1 && is_aibnr_autorun) {
        ratio = 0;
        ilow = aibnr_fixIndex;
        ihigh = aibnr_fixIndex;
    }
    uratio = ratio * (1 << RATIO_FIXBIT);
    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    LOGD_ANR("%s ilow %d, ihigh %d, ratio %f, aibnr_fixIndex %d, is_aibnr_autorun %d",
        __func__, ilow, ihigh,ratio, aibnr_fixIndex, is_aibnr_autorun);

    out->dyn.locShpStrg.texRegion_clsfBaseTex.hw_shpT_flatRegion_maxThred = interpolation_u16(
                paut->dyn[ilow].locShpStrg.texRegion_clsfBaseTex.hw_shpT_flatRegion_maxThred, paut->dyn[ihigh].locShpStrg.texRegion_clsfBaseTex.hw_shpT_flatRegion_maxThred, uratio);
    out->dyn.locShpStrg.texRegion_clsfBaseTex.hw_shpT_edgeRegion_minThred = interpolation_u16(
                paut->dyn[ilow].locShpStrg.texRegion_clsfBaseTex.hw_shpT_edgeRegion_minThred, paut->dyn[ihigh].locShpStrg.texRegion_clsfBaseTex.hw_shpT_edgeRegion_minThred, uratio);
    out->dyn.locShpStrg.motionStrg.hw_shpT_locSgmStrg_mode = paut->dyn[inear].locShpStrg.motionStrg.hw_shpT_locSgmStrg_mode;
    out->dyn.locShpStrg.motionStrg.hw_shpT_localSgmStrg_scale = interpolation_f32(
                paut->dyn[ilow].locShpStrg.motionStrg.hw_shpT_localSgmStrg_scale, paut->dyn[ihigh].locShpStrg.motionStrg.hw_shpT_localSgmStrg_scale, ratio);
    out->dyn.locShpStrg.motionStrg.hw_shpT_glbSgmStrg_val = interpolation_f32(
                paut->dyn[ilow].locShpStrg.motionStrg.hw_shpT_glbSgmStrg_val, paut->dyn[ihigh].locShpStrg.motionStrg.hw_shpT_glbSgmStrg_val, ratio);
    out->dyn.locShpStrg.motionStrg.hw_shpT_glbSgmStrg_alpha = interpolation_f32(
                paut->dyn[ilow].locShpStrg.motionStrg.hw_shpT_glbSgmStrg_alpha, paut->dyn[ihigh].locShpStrg.motionStrg.hw_shpT_glbSgmStrg_alpha, ratio);
    out->dyn.locShpStrg.motionStrg.hw_shpT_locSgmStrg2Mot_mode = paut->dyn[inear].locShpStrg.motionStrg.hw_shpT_locSgmStrg2Mot_mode;
    out->dyn.locShpStrg.luma.sw_shpT_luma_en = paut->dyn[inear].locShpStrg.luma.sw_shpT_luma_en;
    for (i = 0; i < 8; i++) {
        out->dyn.locShpStrg.luma.hw_shpT_luma2ShpStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].locShpStrg.luma.hw_shpT_luma2ShpStrg_val[i], paut->dyn[ihigh].locShpStrg.luma.hw_shpT_luma2ShpStrg_val[i], ratio);
    }
    out->dyn.locShpStrg.radiDist.sw_shpT_radiDist_en = paut->dyn[inear].locShpStrg.radiDist.sw_shpT_radiDist_en;
    for (i = 0; i < 11; i++) {
        out->dyn.locShpStrg.radiDist.hw_shpT_radiDist2ShpStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].locShpStrg.radiDist.hw_shpT_radiDist2ShpStrg_val[i], paut->dyn[ihigh].locShpStrg.radiDist.hw_shpT_radiDist2ShpStrg_val[i], ratio);
    }
    out->dyn.locShpStrg.hue.sw_shpT_hue_en = paut->dyn[inear].locShpStrg.hue.sw_shpT_hue_en;
    for (i = 0; i < 9; i++) {
        out->dyn.locShpStrg.hue.hw_shpT_hue2ShpStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].locShpStrg.hue.hw_shpT_hue2ShpStrg_val[i], paut->dyn[ihigh].locShpStrg.hue.hw_shpT_hue2ShpStrg_val[i], ratio);
    }
    out->dyn.locShpStrg.sat.sw_shpT_sat_en = paut->dyn[inear].locShpStrg.sat.sw_shpT_sat_en;
    out->dyn.locShpStrg.sat.sw_shpT_loSatRegion_thred = paut->dyn[inear].locShpStrg.sat.sw_shpT_loSatRegion_thred;
    out->dyn.locShpStrg.sat.hw_shpT_loSatRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].locShpStrg.sat.hw_shpT_loSatRegionShp_strg, paut->dyn[ihigh].locShpStrg.sat.hw_shpT_loSatRegionShp_strg, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.locShpStrg.sat.hw_shpT_sat2ShpStrg_alpha[i] = interpolation_f32(
                    paut->dyn[ilow].locShpStrg.sat.hw_shpT_sat2ShpStrg_alpha[i], paut->dyn[ihigh].locShpStrg.sat.hw_shpT_sat2ShpStrg_alpha[i], ratio);
    }
    out->dyn.eHfDetailShp.detailExtra_hpf.sw_shpT_filtCfg_mode = paut->dyn[inear].eHfDetailShp.detailExtra_hpf.sw_shpT_filtCfg_mode;
    out->dyn.eHfDetailShp.detailExtra_hpf.sw_shpT_loFiltSpatial_strg = interpolation_f32(
                paut->dyn[ilow].eHfDetailShp.detailExtra_hpf.sw_shpT_loFiltSpatial_strg, paut->dyn[ihigh].eHfDetailShp.detailExtra_hpf.sw_shpT_loFiltSpatial_strg, ratio);
    out->dyn.eHfDetailShp.detailExtra_hpf.sw_shpT_upFiltSpatial_strg = interpolation_f32(
                paut->dyn[ilow].eHfDetailShp.detailExtra_hpf.sw_shpT_upFiltSpatial_strg, paut->dyn[ihigh].eHfDetailShp.detailExtra_hpf.sw_shpT_upFiltSpatial_strg, ratio);
    out->dyn.eHfDetailShp.detailExtra_hpf.sw_shpT_loFiltRadius_mode = paut->dyn[inear].eHfDetailShp.detailExtra_hpf.sw_shpT_loFiltRadius_mode;
    out->dyn.eHfDetailShp.detailExtra_hpf.sw_shpT_upFiltRadius_mode = paut->dyn[inear].eHfDetailShp.detailExtra_hpf.sw_shpT_upFiltRadius_mode;
    for (i = 0; i < 6; i++) {
        out->dyn.eHfDetailShp.detailExtra_hpf.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].eHfDetailShp.detailExtra_hpf.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].eHfDetailShp.detailExtra_hpf.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg = interpolation_f32(
                paut->dyn[ilow].eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg, paut->dyn[ihigh].eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.eHfDetailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].eHfDetailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i], paut->dyn[ihigh].eHfDetailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i], uratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.eHfDetailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].eHfDetailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i], paut->dyn[ihigh].eHfDetailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i], uratio);
    }
    out->dyn.eHfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].eHfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.dHfDetailShp.detailExtra.hw_shp_noiseThred_scale = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.detailExtra.hw_shp_noiseThred_scale, paut->dyn[ihigh].dHfDetailShp.detailExtra.hw_shp_noiseThred_scale, ratio);
    out->dyn.dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg, paut->dyn[ihigh].dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, paut->dyn[ihigh].dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, paut->dyn[ihigh].dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg2.sw_shpT_motionStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_motionStrg2.sw_shpT_motionStrg_mode;
    out->dyn.dHfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.dHfDetailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode;
    out->dyn.dHfDetailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode;
    out->dyn.dHfDetailShp.locShpStrg_sat.sw_shpT_locShpStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_sat.sw_shpT_locShpStrg_mode;
    out->dyn.mfDetailShp.detailExtra.hw_shpT_detailSrcMf_alpha = interpolation_f32(
                paut->dyn[ilow].mfDetailShp.detailExtra.hw_shpT_detailSrcMf_alpha, paut->dyn[ihigh].mfDetailShp.detailExtra.hw_shpT_detailSrcMf_alpha, ratio);
    out->dyn.mfDetailShp.glbShpStrg.hw_shpT_detailPos_strg = interpolation_f32(
                paut->dyn[ilow].mfDetailShp.glbShpStrg.hw_shpT_detailPos_strg, paut->dyn[ihigh].mfDetailShp.glbShpStrg.hw_shpT_detailPos_strg, ratio);
    out->dyn.mfDetailShp.glbShpStrg.hw_shpT_detailNeg_strg = interpolation_f32(
                paut->dyn[ilow].mfDetailShp.glbShpStrg.hw_shpT_detailNeg_strg, paut->dyn[ihigh].mfDetailShp.glbShpStrg.hw_shpT_detailNeg_strg, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.mfDetailShp.locShpStrg_tex.hw_shpT_tex2Shp_strg[i] = interpolation_f32(
                    paut->dyn[ilow].mfDetailShp.locShpStrg_tex.hw_shpT_tex2Shp_strg[i], paut->dyn[ihigh].mfDetailShp.locShpStrg_tex.hw_shpT_tex2Shp_strg[i], ratio);
    }
    out->dyn.mfDetailShp.shootReduction.hw_shpT_detailPosClip_val = interpolation_u16(
                paut->dyn[ilow].mfDetailShp.shootReduction.hw_shpT_detailPosClip_val, paut->dyn[ihigh].mfDetailShp.shootReduction.hw_shpT_detailPosClip_val, uratio);
    out->dyn.mfDetailShp.shootReduction.hw_shpT_detailNegClip_val = interpolation_u16(
                paut->dyn[ilow].mfDetailShp.shootReduction.hw_shpT_detailNegClip_val, paut->dyn[ihigh].mfDetailShp.shootReduction.hw_shpT_detailNegClip_val, uratio);
    out->dyn.mfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].mfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.mfDetailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode = paut->dyn[inear].mfDetailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode;
    out->dyn.mfDetailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode = paut->dyn[inear].mfDetailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode;
    out->dyn.mfDetailShp.locShpStrg_sat.sw_shpT_locShpStrg_mode = paut->dyn[inear].mfDetailShp.locShpStrg_sat.sw_shpT_locShpStrg_mode;
    for (i = 0; i < 8; i++) {
        out->dyn.detailShp.sgmEnv.sw_shpC_luma2Sigma_curve.val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.sgmEnv.sw_shpC_luma2Sigma_curve.val[i], paut->dyn[ihigh].detailShp.sgmEnv.sw_shpC_luma2Sigma_curve.val[i], uratio);
    }
    out->dyn.detailShp.detailExtra_preBifilt.sw_shpT_filtCfg_mode = paut->dyn[inear].detailShp.detailExtra_preBifilt.sw_shpT_filtCfg_mode;
    out->dyn.detailShp.detailExtra_preBifilt.sw_shpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_preBifilt.sw_shpT_filtSpatial_strg, paut->dyn[ihigh].detailShp.detailExtra_preBifilt.sw_shpT_filtSpatial_strg, ratio);
    for (i = 0; i < 3; i++) {
        out->dyn.detailShp.detailExtra_preBifilt.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.detailExtra_preBifilt.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].detailShp.detailExtra_preBifilt.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_scale = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_scale, paut->dyn[ihigh].detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_scale, ratio);
    out->dyn.detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_offset = interpolation_u8(
                paut->dyn[ilow].detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_offset, paut->dyn[ihigh].detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_offset, uratio);
    out->dyn.detailShp.detailExtra_preBifilt.hw_shpT_rgeWgt_slope = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_preBifilt.hw_shpT_rgeWgt_slope, paut->dyn[ihigh].detailShp.detailExtra_preBifilt.hw_shpT_rgeWgt_slope, ratio);
    out->dyn.detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha, paut->dyn[ihigh].detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha, ratio);
    out->dyn.detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha, paut->dyn[ihigh].detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha, ratio);
    out->dyn.detailShp.hiDetailExtra_lpf.sw_shpT_filtCfg_mode = paut->dyn[inear].detailShp.hiDetailExtra_lpf.sw_shpT_filtCfg_mode;
    out->dyn.detailShp.hiDetailExtra_lpf.sw_shpT_filtRadius_mode = paut->dyn[inear].detailShp.hiDetailExtra_lpf.sw_shpT_filtRadius_mode;
    out->dyn.detailShp.hiDetailExtra_lpf.sw_shpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.hiDetailExtra_lpf.sw_shpT_filtSpatial_strg, paut->dyn[ihigh].detailShp.hiDetailExtra_lpf.sw_shpT_filtSpatial_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.detailShp.hiDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.hiDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].detailShp.hiDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.detailShp.midDetailExtra_lpf.sw_shpT_filtCfg_mode = paut->dyn[inear].detailShp.midDetailExtra_lpf.sw_shpT_filtCfg_mode;
    out->dyn.detailShp.midDetailExtra_lpf.sw_shpT_filtRadius_mode = paut->dyn[inear].detailShp.midDetailExtra_lpf.sw_shpT_filtRadius_mode;
    out->dyn.detailShp.midDetailExtra_lpf.sw_shpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.midDetailExtra_lpf.sw_shpT_filtSpatial_strg, paut->dyn[ihigh].detailShp.midDetailExtra_lpf.sw_shpT_filtSpatial_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.detailShp.midDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.midDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].detailShp.midDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.detailShp.detailFusion.hw_shpT_detailAlpha_mode = paut->dyn[inear].detailShp.detailFusion.hw_shpT_detailAlpha_mode;
    out->dyn.detailShp.detailFusion.hw_shpT_hiDetailAlpha_minLimit = interpolation_u8(
                paut->dyn[ilow].detailShp.detailFusion.hw_shpT_hiDetailAlpha_minLimit, paut->dyn[ihigh].detailShp.detailFusion.hw_shpT_hiDetailAlpha_minLimit, uratio);
    out->dyn.detailShp.detailFusion.hw_shpT_hiDetailAlpha_maxLimit = interpolation_u8(
                paut->dyn[ilow].detailShp.detailFusion.hw_shpT_hiDetailAlpha_maxLimit, paut->dyn[ihigh].detailShp.detailFusion.hw_shpT_hiDetailAlpha_maxLimit, uratio);
    out->dyn.detailShp.glbShpStrg.hw_shpT_detailPos_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.glbShpStrg.hw_shpT_detailPos_strg, paut->dyn[ihigh].detailShp.glbShpStrg.hw_shpT_detailPos_strg, ratio);
    out->dyn.detailShp.glbShpStrg.hw_shpT_detailNeg_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.glbShpStrg.hw_shpT_detailNeg_strg, paut->dyn[ihigh].detailShp.glbShpStrg.hw_shpT_detailNeg_strg, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.locShpStrg_tex.hw_shpT_tex2Shp_strg[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.locShpStrg_tex.hw_shpT_tex2Shp_strg[i], paut->dyn[ihigh].detailShp.locShpStrg_tex.hw_shpT_tex2Shp_strg[i], ratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.locShpStrg_contrast.hw_shpT_contrast2posStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.locShpStrg_contrast.hw_shpT_contrast2posStrg_val[i], paut->dyn[ihigh].detailShp.locShpStrg_contrast.hw_shpT_contrast2posStrg_val[i], ratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.locShpStrg_contrast.hw_shpT_contrast2negStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.locShpStrg_contrast.hw_shpT_contrast2negStrg_val[i], paut->dyn[ihigh].detailShp.locShpStrg_contrast.hw_shpT_contrast2negStrg_val[i], ratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i], paut->dyn[ihigh].detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i], uratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i], paut->dyn[ihigh].detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i], uratio);
    }
    for (i = 0; i < 8; i++) {
        out->dyn.detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i], paut->dyn[ihigh].detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i], uratio);
    }
    for (i = 0; i < 8; i++) {
        out->dyn.detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i], paut->dyn[ihigh].detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i], uratio);
    }
    out->dyn.detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, ratio);
    out->dyn.detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, ratio);
    out->dyn.detailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, ratio);
    out->dyn.detailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, ratio);
    out->dyn.detailShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma, ratio);
    out->dyn.detailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].detailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.detailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode = paut->dyn[inear].detailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode;
    out->dyn.detailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode = paut->dyn[inear].detailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode;
    out->dyn.detailShp.locShpStrg_sat.sw_shpT_locShpStrg_mode = paut->dyn[inear].detailShp.locShpStrg_sat.sw_shpT_locShpStrg_mode;
    out->dyn.edgeShp.edgeExtra_bpf.sw_shpT_filtCfg_mode = paut->dyn[inear].edgeShp.edgeExtra_bpf.sw_shpT_filtCfg_mode;
    out->dyn.edgeShp.edgeExtra_bpf.sw_shpT_loFiltRadius_mode = paut->dyn[inear].edgeShp.edgeExtra_bpf.sw_shpT_loFiltRadius_mode;
    out->dyn.edgeShp.edgeExtra_bpf.sw_shpT_loFiltSpatial_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.edgeExtra_bpf.sw_shpT_loFiltSpatial_strg, paut->dyn[ihigh].edgeShp.edgeExtra_bpf.sw_shpT_loFiltSpatial_strg, ratio);
    out->dyn.edgeShp.edgeExtra_bpf.sw_shpT_upFiltRadius_mode = paut->dyn[inear].edgeShp.edgeExtra_bpf.sw_shpT_upFiltRadius_mode;
    out->dyn.edgeShp.edgeExtra_bpf.sw_shpT_upFiltSpatial_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.edgeExtra_bpf.sw_shpT_upFiltSpatial_strg, paut->dyn[ihigh].edgeShp.edgeExtra_bpf.sw_shpT_upFiltSpatial_strg, ratio);
    for (i = 0; i < 10; i++) {
        out->dyn.edgeShp.edgeExtra_bpf.hw_shpT_bpfSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].edgeShp.edgeExtra_bpf.hw_shpT_bpfSpatial_wgt[i], paut->dyn[ihigh].edgeShp.edgeExtra_bpf.hw_shpT_bpfSpatial_wgt[i], ratio);
    }
    out->dyn.edgeShp.glbShpStrg.hw_shpT_edgePos_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.glbShpStrg.hw_shpT_edgePos_strg, paut->dyn[ihigh].edgeShp.glbShpStrg.hw_shpT_edgePos_strg, ratio);
    out->dyn.edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg, paut->dyn[ihigh].edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg, ratio);
    out->dyn.edgeShp.locShpStrg_edge.sw_shpT_edgeWgtFltCfg_mode = paut->dyn[inear].edgeShp.locShpStrg_edge.sw_shpT_edgeWgtFltCfg_mode;
    out->dyn.edgeShp.locShpStrg_edge.sw_shpT_edgeWgtFlt_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.sw_shpT_edgeWgtFlt_strg, paut->dyn[ihigh].edgeShp.locShpStrg_edge.sw_shpT_edgeWgtFlt_strg, ratio);
    for (i = 0; i < 3; i++) {
        out->dyn.edgeShp.locShpStrg_edge.sw_shpT_edgeWgtFlt_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].edgeShp.locShpStrg_edge.sw_shpT_edgeWgtFlt_wgt[i], paut->dyn[ihigh].edgeShp.locShpStrg_edge.sw_shpT_edgeWgtFlt_wgt[i], ratio);
    }
    out->dyn.edgeShp.locShpStrg_edge.sw_shpT_edgeStrgCurve_mode = paut->dyn[inear].edgeShp.locShpStrg_edge.sw_shpT_edgeStrgCurve_mode;
    out->dyn.edgeShp.locShpStrg_edge.hw_shpT_edgePosIdx_scale = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.hw_shpT_edgePosIdx_scale, paut->dyn[ihigh].edgeShp.locShpStrg_edge.hw_shpT_edgePosIdx_scale, ratio);
    out->dyn.edgeShp.locShpStrg_edge.hw_shpT_edgeNegIdx_scale = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.hw_shpT_edgeNegIdx_scale, paut->dyn[ihigh].edgeShp.locShpStrg_edge.hw_shpT_edgeNegIdx_scale, ratio);
    out->dyn.edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_curvePower_val = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_curvePower_val, paut->dyn[ihigh].edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_curvePower_val, ratio);
    out->dyn.edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_edgeStrg_minLimit = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_edgeStrg_minLimit, paut->dyn[ihigh].edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_edgeStrg_minLimit, ratio);
    for (i = 0; i < 17; i++) {
        out->dyn.edgeShp.locShpStrg_edge.hw_shpT_edge2ShpStrg_val[i] = interpolation_u16(
                    paut->dyn[ilow].edgeShp.locShpStrg_edge.hw_shpT_edge2ShpStrg_val[i], paut->dyn[ihigh].edgeShp.locShpStrg_edge.hw_shpT_edge2ShpStrg_val[i], uratio);
    }
    out->dyn.edgeShp.shootReduction.sw_shpT_maxMinFlt_mode = paut->dyn[inear].edgeShp.shootReduction.sw_shpT_maxMinFlt_mode;
    out->dyn.edgeShp.shootReduction.hw_shpT_glbClip_thred = interpolation_u16(
                paut->dyn[ilow].edgeShp.shootReduction.hw_shpT_glbClip_thred, paut->dyn[ihigh].edgeShp.shootReduction.hw_shpT_glbClip_thred, uratio);
    out->dyn.edgeShp.shootReduction.hw_shpT_edgePosClip_val = interpolation_u16(
                paut->dyn[ilow].edgeShp.shootReduction.hw_shpT_edgePosClip_val, paut->dyn[ihigh].edgeShp.shootReduction.hw_shpT_edgePosClip_val, uratio);
    out->dyn.edgeShp.shootReduction.hw_shpT_edgeNegClip_val = interpolation_u16(
                paut->dyn[ilow].edgeShp.shootReduction.hw_shpT_edgeNegClip_val, paut->dyn[ihigh].edgeShp.shootReduction.hw_shpT_edgeNegClip_val, uratio);
    out->dyn.edgeShp.shootReduction.sw_shpT_overShoot_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.shootReduction.sw_shpT_overShoot_strg, paut->dyn[ihigh].edgeShp.shootReduction.sw_shpT_overShoot_strg, ratio);
    out->dyn.edgeShp.shootReduction.sw_shpT_underShoot_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.shootReduction.sw_shpT_underShoot_strg, paut->dyn[ihigh].edgeShp.shootReduction.sw_shpT_underShoot_strg, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma, ratio);
    out->dyn.edgeShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].edgeShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.edgeShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode = paut->dyn[inear].edgeShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode;
    out->dyn.edgeShp.locShpStrg_hue.sw_shpT_locShpStrg_mode = paut->dyn[inear].edgeShp.locShpStrg_hue.sw_shpT_locShpStrg_mode;
    out->dyn.edgeShp.locShpStrg_sat.sw_shpT_locShpStrg_mode = paut->dyn[inear].edgeShp.locShpStrg_sat.sw_shpT_locShpStrg_mode;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn SharpApplyStrength
(
    SharpContext_t *pSharpCtx,
    sharp_param_t* out)
{
    if(pSharpCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pSharpCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pSharpCtx->fStrength);

        out->dyn.eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg *= fPercent;
        out->dyn.dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg *= fPercent;

        out->dyn.detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha /= fPercent;
        out->dyn.detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha /= fPercent;
        out->dyn.detailShp.glbShpStrg.hw_shpT_detailPos_strg *= fPercent;
        out->dyn.detailShp.glbShpStrg.hw_shpT_detailNeg_strg *= fPercent;
        for (int i = 0; i < 9; i++) {
            out->dyn.detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i] *= fPercent;
            out->dyn.detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i] *= fPercent;
        }

        for (int i = 0; i < 8; i++) {
            out->dyn.detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i] *= fPercent;
            out->dyn.detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i] *= fPercent;
        }

        out->dyn.edgeShp.glbShpStrg.hw_shpT_edgePos_strg *= fPercent;
        out->dyn.edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg *= fPercent;

        LOGI_ANR("SharpApplyStrength: fStrength %f, fPercent %f\n", pSharpCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}

#endif

#if RKAIQ_HAVE_SHARP_V40
XCamReturn SharpSelectParam
(
    SharpContext_t *pSharpCtx,
    sharp_param_t* out,
    int iso,
    bool is_aibnr_autorun,
    int aibnr_fixIndex)
{
    sharp_param_auto_t *paut = &pSharpCtx->sharp_attrib->stAuto;
    if(paut == NULL || out == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, pSharpCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);
    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    out->dyn.locShpStrg.texRegion_clsfBaseTex.hw_shpT_flatRegion_maxThred = interpolation_u16(
                paut->dyn[ilow].locShpStrg.texRegion_clsfBaseTex.hw_shpT_flatRegion_maxThred, paut->dyn[ihigh].locShpStrg.texRegion_clsfBaseTex.hw_shpT_flatRegion_maxThred, uratio);
    out->dyn.locShpStrg.texRegion_clsfBaseTex.hw_shpT_edgeRegion_minThred = interpolation_u16(
                paut->dyn[ilow].locShpStrg.texRegion_clsfBaseTex.hw_shpT_edgeRegion_minThred, paut->dyn[ihigh].locShpStrg.texRegion_clsfBaseTex.hw_shpT_edgeRegion_minThred, uratio);
    out->dyn.locShpStrg.motionStrg.hw_shpT_locSgmStrg_mode = paut->dyn[inear].locShpStrg.motionStrg.hw_shpT_locSgmStrg_mode;
    out->dyn.locShpStrg.motionStrg.hw_shpT_localSgmStrg_scale = interpolation_f32(
                paut->dyn[ilow].locShpStrg.motionStrg.hw_shpT_localSgmStrg_scale, paut->dyn[ihigh].locShpStrg.motionStrg.hw_shpT_localSgmStrg_scale, ratio);
    out->dyn.locShpStrg.motionStrg.hw_shpT_glbSgmStrg_val = interpolation_f32(
                paut->dyn[ilow].locShpStrg.motionStrg.hw_shpT_glbSgmStrg_val, paut->dyn[ihigh].locShpStrg.motionStrg.hw_shpT_glbSgmStrg_val, ratio);
    out->dyn.locShpStrg.motionStrg.hw_shpT_glbSgmStrg_alpha = interpolation_f32(
                paut->dyn[ilow].locShpStrg.motionStrg.hw_shpT_glbSgmStrg_alpha, paut->dyn[ihigh].locShpStrg.motionStrg.hw_shpT_glbSgmStrg_alpha, ratio);
    out->dyn.locShpStrg.motionStrg.hw_shpT_locSgmStrg2Mot_mode = paut->dyn[inear].locShpStrg.motionStrg.hw_shpT_locSgmStrg2Mot_mode;
    out->dyn.locShpStrg.luma.sw_shpT_luma_en = paut->dyn[inear].locShpStrg.luma.sw_shpT_luma_en;
    for (i = 0; i < 8; i++) {
        out->dyn.locShpStrg.luma.hw_shpT_luma2ShpStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].locShpStrg.luma.hw_shpT_luma2ShpStrg_val[i], paut->dyn[ihigh].locShpStrg.luma.hw_shpT_luma2ShpStrg_val[i], ratio);
    }
    out->dyn.locShpStrg.radiDist.sw_shpT_radiDist_en = paut->dyn[inear].locShpStrg.radiDist.sw_shpT_radiDist_en;
    for (i = 0; i < 11; i++) {
        out->dyn.locShpStrg.radiDist.hw_shpT_radiDist2ShpStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].locShpStrg.radiDist.hw_shpT_radiDist2ShpStrg_val[i], paut->dyn[ihigh].locShpStrg.radiDist.hw_shpT_radiDist2ShpStrg_val[i], ratio);
    }
    out->dyn.locShpStrg.hue.sw_shpT_hue_en = paut->dyn[inear].locShpStrg.hue.sw_shpT_hue_en;
    for (i = 0; i < 9; i++) {
        out->dyn.locShpStrg.hue.hw_shpT_hue2ShpStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].locShpStrg.hue.hw_shpT_hue2ShpStrg_val[i], paut->dyn[ihigh].locShpStrg.hue.hw_shpT_hue2ShpStrg_val[i], ratio);
    }
    out->dyn.eHfDetailShp.detailExtra_hpf.sw_shpT_filtCfg_mode = paut->dyn[inear].eHfDetailShp.detailExtra_hpf.sw_shpT_filtCfg_mode;
    out->dyn.eHfDetailShp.detailExtra_hpf.sw_shpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].eHfDetailShp.detailExtra_hpf.sw_shpT_filtSpatial_strg, paut->dyn[ihigh].eHfDetailShp.detailExtra_hpf.sw_shpT_filtSpatial_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.eHfDetailShp.detailExtra_hpf.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].eHfDetailShp.detailExtra_hpf.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].eHfDetailShp.detailExtra_hpf.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg = interpolation_f32(
                paut->dyn[ilow].eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg, paut->dyn[ihigh].eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.eHfDetailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].eHfDetailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i], paut->dyn[ihigh].eHfDetailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i], uratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.eHfDetailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].eHfDetailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i], paut->dyn[ihigh].eHfDetailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i], uratio);
    }
    out->dyn.eHfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].eHfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.dHfDetailShp.detailExtra.hw_shp_noiseThred_scale = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.detailExtra.hw_shp_noiseThred_scale, paut->dyn[ihigh].dHfDetailShp.detailExtra.hw_shp_noiseThred_scale, ratio);
    out->dyn.dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg, paut->dyn[ihigh].dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, paut->dyn[ihigh].dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, paut->dyn[ihigh].dHfDetailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, paut->dyn[ihigh].dHfDetailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, ratio);
    out->dyn.dHfDetailShp.locShpStrg_motionStrg2.sw_shpT_motionStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_motionStrg2.sw_shpT_motionStrg_mode;
    out->dyn.dHfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.dHfDetailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode;
    out->dyn.dHfDetailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode = paut->dyn[inear].dHfDetailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode;
    for (i = 0; i < 8; i++) {
        out->dyn.detailShp.sgmEnv.sw_shpC_luma2Sigma_curve.val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.sgmEnv.sw_shpC_luma2Sigma_curve.val[i], paut->dyn[ihigh].detailShp.sgmEnv.sw_shpC_luma2Sigma_curve.val[i], uratio);
    }
    out->dyn.detailShp.detailExtra_preBifilt.sw_shpT_filtCfg_mode = paut->dyn[inear].detailShp.detailExtra_preBifilt.sw_shpT_filtCfg_mode;
    out->dyn.detailShp.detailExtra_preBifilt.sw_shpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_preBifilt.sw_shpT_filtSpatial_strg, paut->dyn[ihigh].detailShp.detailExtra_preBifilt.sw_shpT_filtSpatial_strg, ratio);
    for (i = 0; i < 3; i++) {
        out->dyn.detailShp.detailExtra_preBifilt.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.detailExtra_preBifilt.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].detailShp.detailExtra_preBifilt.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_scale = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_scale, paut->dyn[ihigh].detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_scale, ratio);
    out->dyn.detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_offset = interpolation_u8(
                paut->dyn[ilow].detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_offset, paut->dyn[ihigh].detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_offset, uratio);
    out->dyn.detailShp.detailExtra_preBifilt.hw_shpT_rgeWgt_slope = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_preBifilt.hw_shpT_rgeWgt_slope, paut->dyn[ihigh].detailShp.detailExtra_preBifilt.hw_shpT_rgeWgt_slope, ratio);
    out->dyn.detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha, paut->dyn[ihigh].detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha, ratio);
    out->dyn.detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha = interpolation_f32(
                paut->dyn[ilow].detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha, paut->dyn[ihigh].detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha, ratio);
    out->dyn.detailShp.hiDetailExtra_lpf.sw_shpT_filtCfg_mode = paut->dyn[inear].detailShp.hiDetailExtra_lpf.sw_shpT_filtCfg_mode;
    out->dyn.detailShp.hiDetailExtra_lpf.sw_shpT_filtRadius_mode = paut->dyn[inear].detailShp.hiDetailExtra_lpf.sw_shpT_filtRadius_mode;
    out->dyn.detailShp.hiDetailExtra_lpf.sw_shpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.hiDetailExtra_lpf.sw_shpT_filtSpatial_strg, paut->dyn[ihigh].detailShp.hiDetailExtra_lpf.sw_shpT_filtSpatial_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.detailShp.hiDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.hiDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].detailShp.hiDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.detailShp.midDetailExtra_lpf.sw_shpT_filtCfg_mode = paut->dyn[inear].detailShp.midDetailExtra_lpf.sw_shpT_filtCfg_mode;
    out->dyn.detailShp.midDetailExtra_lpf.sw_shpT_filtRadius_mode = paut->dyn[inear].detailShp.midDetailExtra_lpf.sw_shpT_filtRadius_mode;
    out->dyn.detailShp.midDetailExtra_lpf.sw_shpT_filt_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.midDetailExtra_lpf.sw_shpT_filt_strg, paut->dyn[ihigh].detailShp.midDetailExtra_lpf.sw_shpT_filt_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.detailShp.midDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.midDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].detailShp.midDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.detailShp.detailFusion.hw_shpT_detailAlpha_mode = paut->dyn[inear].detailShp.detailFusion.hw_shpT_detailAlpha_mode;
    out->dyn.detailShp.detailFusion.hw_shpT_hiDetailAlpha_minLimit = interpolation_u8(
                paut->dyn[ilow].detailShp.detailFusion.hw_shpT_hiDetailAlpha_minLimit, paut->dyn[ihigh].detailShp.detailFusion.hw_shpT_hiDetailAlpha_minLimit, uratio);
    out->dyn.detailShp.detailFusion.hw_shpT_hiDetailAlpha_maxLimit = interpolation_u8(
                paut->dyn[ilow].detailShp.detailFusion.hw_shpT_hiDetailAlpha_maxLimit, paut->dyn[ihigh].detailShp.detailFusion.hw_shpT_hiDetailAlpha_maxLimit, uratio);
    out->dyn.detailShp.glbShpStrg.hw_shpT_detailPos_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.glbShpStrg.hw_shpT_detailPos_strg, paut->dyn[ihigh].detailShp.glbShpStrg.hw_shpT_detailPos_strg, ratio);
    out->dyn.detailShp.glbShpStrg.hw_shpT_detailNeg_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.glbShpStrg.hw_shpT_detailNeg_strg, paut->dyn[ihigh].detailShp.glbShpStrg.hw_shpT_detailNeg_strg, ratio);
    out->dyn.detailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg, paut->dyn[ihigh].detailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg, ratio);
    out->dyn.detailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg, paut->dyn[ihigh].detailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg, ratio);
    out->dyn.detailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg, paut->dyn[ihigh].detailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg, ratio);
    out->dyn.detailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg, paut->dyn[ihigh].detailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.locShpStrg_contrast.hw_shpT_contrast2posStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.locShpStrg_contrast.hw_shpT_contrast2posStrg_val[i], paut->dyn[ihigh].detailShp.locShpStrg_contrast.hw_shpT_contrast2posStrg_val[i], ratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.locShpStrg_contrast.hw_shpT_contrast2negStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].detailShp.locShpStrg_contrast.hw_shpT_contrast2negStrg_val[i], paut->dyn[ihigh].detailShp.locShpStrg_contrast.hw_shpT_contrast2negStrg_val[i], ratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i], paut->dyn[ihigh].detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i], uratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i], paut->dyn[ihigh].detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i], uratio);
    }
    for (i = 0; i < 8; i++) {
        out->dyn.detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i], paut->dyn[ihigh].detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i], uratio);
    }
    for (i = 0; i < 8; i++) {
        out->dyn.detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i] = interpolation_u16(
                    paut->dyn[ilow].detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i], paut->dyn[ihigh].detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i], uratio);
    }
    out->dyn.detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, ratio);
    out->dyn.detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, ratio);
    out->dyn.detailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, ratio);
    out->dyn.detailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, ratio);
    out->dyn.detailShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma = interpolation_f32(
                paut->dyn[ilow].detailShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma, paut->dyn[ihigh].detailShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma, ratio);
    out->dyn.detailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].detailShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.detailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode = paut->dyn[inear].detailShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode;
    out->dyn.detailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode = paut->dyn[inear].detailShp.locShpStrg_hue.sw_shpT_locShpStrg_mode;
    out->dyn.edgeShp.edgeExtra.sw_shpT_filtRadius_mode = paut->dyn[inear].edgeShp.edgeExtra.sw_shpT_filtRadius_mode;
    out->dyn.edgeShp.edgeExtra.sw_shpT_filtCfg_mode = paut->dyn[inear].edgeShp.edgeExtra.sw_shpT_filtCfg_mode;
    out->dyn.edgeShp.edgeExtra.sw_shpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.edgeExtra.sw_shpT_filtSpatial_strg, paut->dyn[ihigh].edgeShp.edgeExtra.sw_shpT_filtSpatial_strg, ratio);
    for (i = 0; i < 10; i++) {
        out->dyn.edgeShp.edgeExtra.hw_shpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].edgeShp.edgeExtra.hw_shpT_filtSpatial_wgt[i], paut->dyn[ihigh].edgeShp.edgeExtra.hw_shpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.edgeShp.glbShpStrg.hw_shpT_edgePos_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.glbShpStrg.hw_shpT_edgePos_strg, paut->dyn[ihigh].edgeShp.glbShpStrg.hw_shpT_edgePos_strg, ratio);
    out->dyn.edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg, paut->dyn[ihigh].edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg, ratio);
    out->dyn.edgeShp.locShpStrg_edge.sw_shpT_edgeStrgCurve_mode = paut->dyn[inear].edgeShp.locShpStrg_edge.sw_shpT_edgeStrgCurve_mode;
    out->dyn.edgeShp.locShpStrg_edge.hw_shpT_edgePosIdx_scale = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.hw_shpT_edgePosIdx_scale, paut->dyn[ihigh].edgeShp.locShpStrg_edge.hw_shpT_edgePosIdx_scale, ratio);
    out->dyn.edgeShp.locShpStrg_edge.hw_shpT_edgeNegIdx_scale = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.hw_shpT_edgeNegIdx_scale, paut->dyn[ihigh].edgeShp.locShpStrg_edge.hw_shpT_edgeNegIdx_scale, ratio);
    out->dyn.edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_curvePower_val = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_curvePower_val, paut->dyn[ihigh].edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_curvePower_val, ratio);
    out->dyn.edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_edgeStrg_minLimit = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_edgeStrg_minLimit, paut->dyn[ihigh].edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_edgeStrg_minLimit, ratio);
    for (i = 0; i < 17; i++) {
        out->dyn.edgeShp.locShpStrg_edge.hw_shpT_edge2ShpStrg_val[i] = interpolation_u16(
                    paut->dyn[ilow].edgeShp.locShpStrg_edge.hw_shpT_edge2ShpStrg_val[i], paut->dyn[ihigh].edgeShp.locShpStrg_edge.hw_shpT_edge2ShpStrg_val[i], uratio);
    }
    out->dyn.edgeShp.shootReduction.sw_shpT_maxMinFlt_mode = paut->dyn[inear].edgeShp.shootReduction.sw_shpT_maxMinFlt_mode;
    out->dyn.edgeShp.shootReduction.sw_shpT_overShoot_alpha = interpolation_f32(
                paut->dyn[ilow].edgeShp.shootReduction.sw_shpT_overShoot_alpha, paut->dyn[ihigh].edgeShp.shootReduction.sw_shpT_overShoot_alpha, ratio);
    out->dyn.edgeShp.shootReduction.sw_shpT_underShoot_alpha = interpolation_f32(
                paut->dyn[ilow].edgeShp.shootReduction.sw_shpT_underShoot_alpha, paut->dyn[ihigh].edgeShp.shootReduction.sw_shpT_underShoot_alpha, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgStat_maxThred, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg1.sw_shpT_locSgmStrgMot_minThred, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg1.hw_shpT_motRegionShp_strg, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg1.hw_shpT_statRegionShp_strg, ratio);
    out->dyn.edgeShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma = interpolation_f32(
                paut->dyn[ilow].edgeShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma, paut->dyn[ihigh].edgeShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma, ratio);
    out->dyn.edgeShp.locShpStrg_luma.sw_shpT_locShpStrg_mode = paut->dyn[inear].edgeShp.locShpStrg_luma.sw_shpT_locShpStrg_mode;
    out->dyn.edgeShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode = paut->dyn[inear].edgeShp.locShpStrg_radiDist.sw_shpT_locShpStrg_mode;
    out->dyn.edgeShp.locShpStrg_hue.sw_shpT_locShpStrg_mode = paut->dyn[inear].edgeShp.locShpStrg_hue.sw_shpT_locShpStrg_mode;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn SharpApplyStrength
(
    SharpContext_t *pSharpCtx,
    sharp_param_t* out)
{
    if(pSharpCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pSharpCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pSharpCtx->fStrength);

        out->dyn.eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg *= fPercent;
        out->dyn.dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg *= fPercent;

        out->dyn.detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha /= fPercent;
        out->dyn.detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha /= fPercent;
        out->dyn.detailShp.glbShpStrg.hw_shpT_detailPos_strg *= fPercent;
        out->dyn.detailShp.glbShpStrg.hw_shpT_detailNeg_strg *= fPercent;
        for (int i = 0; i < 9; i++) {
            out->dyn.detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i] *= fPercent;
            out->dyn.detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i] *= fPercent;
        }

        for (int i = 0; i < 8; i++) {
            out->dyn.detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i] *= fPercent;
            out->dyn.detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i] *= fPercent;
        }

        out->dyn.edgeShp.glbShpStrg.hw_shpT_edgePos_strg *= fPercent;
        out->dyn.edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg *= fPercent;

        LOGI_ANR("SharpApplyStrength: fStrength %f, fPercent %f\n", pSharpCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}

#endif

#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
XCamReturn texEstSelectParam
(
    SharpContext_t *pSharpCtx,
    texEst_param_t* out,
    int iso,
    bool is_aibnr_autorun,
    int aibnr_fixIndex)
{
    texEst_param_auto_t *paut = &pSharpCtx->texEst_attrib->stAuto;
    if(paut == NULL || out == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, pSharpCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    if (aibnr_fixIndex != -1 && is_aibnr_autorun) {
        ratio = 0;
        ilow = aibnr_fixIndex;
        ihigh = aibnr_fixIndex;
    }
    uratio = ratio * (1 << RATIO_FIXBIT);
    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    LOGD_ANR("%s ilow %d, ihigh %d, ratio %f, aibnr_fixIndex %d, is_aibnr_autorun %d",
        __func__, ilow, ihigh,ratio, aibnr_fixIndex, is_aibnr_autorun);

    out->dyn.noiseEst.hw_texEstT_nsEstTexThd_mode = paut->dyn[inear].noiseEst.hw_texEstT_nsEstTexThd_mode;
    for (i = 0; i < 17; i++) {
        out->dyn.noiseEst.hw_texEstT_nsEstTexManual_thred[i] = interpolation_f32(
                    paut->dyn[ilow].noiseEst.hw_texEstT_nsEstTexManual_thred[i], paut->dyn[ihigh].noiseEst.hw_texEstT_nsEstTexManual_thred[i], ratio);
    }
    out->dyn.noiseEst.hw_texEstT_nsStatsCntThd_ratio = interpolation_f32(
                paut->dyn[ilow].noiseEst.hw_texEstT_nsStatsCntThd_ratio, paut->dyn[ihigh].noiseEst.hw_texEstT_nsStatsCntThd_ratio, ratio);
    out->dyn.noiseEst.hw_texEstT_noiseEst_mode = paut->dyn[inear].noiseEst.hw_texEstT_noiseEst_mode;
    out->dyn.noiseEst.hw_texEstT_nsEstMean_alpha = interpolation_f32(
                paut->dyn[ilow].noiseEst.hw_texEstT_nsEstMean_alpha, paut->dyn[ihigh].noiseEst.hw_texEstT_nsEstMean_alpha, ratio);
    out->dyn.noiseEst.hw_texEstT_nsEstTexThd_scale = interpolation_f32(
                paut->dyn[ilow].noiseEst.hw_texEstT_nsEstTexThd_scale, paut->dyn[ihigh].noiseEst.hw_texEstT_nsEstTexThd_scale, ratio);
    out->dyn.noiseEst.hw_texEstT_nsEstTexThd_minLimit = interpolation_f32(
                paut->dyn[ilow].noiseEst.hw_texEstT_nsEstTexThd_minLimit, paut->dyn[ihigh].noiseEst.hw_texEstT_nsEstTexThd_minLimit, ratio);
    out->dyn.noiseEst.hw_texEstT_nsEstTexThd_maxLimit = interpolation_f32(
                paut->dyn[ilow].noiseEst.hw_texEstT_nsEstTexThd_maxLimit, paut->dyn[ihigh].noiseEst.hw_texEstT_nsEstTexThd_maxLimit, ratio);
    out->dyn.texEst.hw_texEstT_texEst_mode = paut->dyn[inear].texEst.hw_texEstT_texEst_mode;
    out->dyn.texEst.hw_texEstT_nsEstDf1_scale = interpolation_f32(
                paut->dyn[ilow].texEst.hw_texEstT_nsEstDf1_scale, paut->dyn[ihigh].texEst.hw_texEstT_nsEstDf1_scale, ratio);
    out->dyn.texEst.hw_texEstT_sigmaDf1_scale = interpolation_f32(
                paut->dyn[ilow].texEst.hw_texEstT_sigmaDf1_scale, paut->dyn[ihigh].texEst.hw_texEstT_sigmaDf1_scale, ratio);
    out->dyn.texEst.hw_texEstT_sigmaDf1_offset = interpolation_f32(
                paut->dyn[ilow].texEst.hw_texEstT_sigmaDf1_offset, paut->dyn[ihigh].texEst.hw_texEstT_sigmaDf1_offset, ratio);
    out->dyn.texEst.hw_texEstT_texEstDf1_scale = interpolation_f32(
                paut->dyn[ilow].texEst.hw_texEstT_texEstDf1_scale, paut->dyn[ihigh].texEst.hw_texEstT_texEstDf1_scale, ratio);
    out->dyn.texEst.hw_texEstT_nsEstDf2_scale = interpolation_f32(
                paut->dyn[ilow].texEst.hw_texEstT_nsEstDf2_scale, paut->dyn[ihigh].texEst.hw_texEstT_nsEstDf2_scale, ratio);
    out->dyn.texEst.hw_texEstT_sigmaDf2_scale = interpolation_f32(
                paut->dyn[ilow].texEst.hw_texEstT_sigmaDf2_scale, paut->dyn[ihigh].texEst.hw_texEstT_sigmaDf2_scale, ratio);
    out->dyn.texEst.hw_texEstT_sigmaDf2_offset = interpolation_f32(
                paut->dyn[ilow].texEst.hw_texEstT_sigmaDf2_offset, paut->dyn[ihigh].texEst.hw_texEstT_sigmaDf2_offset, ratio);
    out->dyn.texEst.hw_texEstT_texEstDf2_scale = interpolation_f32(
                paut->dyn[ilow].texEst.hw_texEstT_texEstDf2_scale, paut->dyn[ihigh].texEst.hw_texEstT_texEstDf2_scale, ratio);
    out->dyn.texEst.hw_texEstT_wgtOpt_mode = paut->dyn[inear].texEst.hw_texEstT_wgtOpt_mode;
    out->dyn.texEstFlt.sw_texEstT_filtCfg_mode = paut->dyn[inear].texEstFlt.sw_texEstT_filtCfg_mode;
    out->dyn.texEstFlt.sw_texEstT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].texEstFlt.sw_texEstT_filtSpatial_strg, paut->dyn[ihigh].texEstFlt.sw_texEstT_filtSpatial_strg, ratio);
    for (i = 0; i < 3; i++) {
        out->dyn.texEstFlt.sw_texEstT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].texEstFlt.sw_texEstT_filtSpatial_wgt[i], paut->dyn[ihigh].texEstFlt.sw_texEstT_filtSpatial_wgt[i], ratio);
    }
    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_SHARP_V34
XCamReturn SharpSelectParam
(
    SharpContext_t *pSharpCtx,
    sharp_param_t* out,
    int iso)
{
    sharp_param_auto_t *paut = &pSharpCtx->sharp_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, pSharpCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    for (i = 0; i < 8; i++) {
        out->dyn.hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.idx[i] = paut->dyn[inear].hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.idx[i];
    }
    for (i = 0; i < 8; i++) {
        out->dyn.hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i] = interpolation_u16(
                    paut->dyn[ilow].hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i], paut->dyn[ihigh].hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i], uratio);
    }
    out->dyn.hfExtra_preBifilt.sw_sharpT_filtCfg_mode = paut->dyn[inear].hfExtra_preBifilt.sw_sharpT_filtCfg_mode;
    out->dyn.hfExtra_preBifilt.sw_sharpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].hfExtra_preBifilt.sw_sharpT_filtSpatial_strg, paut->dyn[ihigh].hfExtra_preBifilt.sw_sharpT_filtSpatial_strg, ratio);
    for (i = 0; i < 3; i++) {
        out->dyn.hfExtra_preBifilt.hw_sharpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].hfExtra_preBifilt.hw_sharpT_filtSpatial_wgt[i], paut->dyn[ihigh].hfExtra_preBifilt.hw_sharpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.hfExtra_preBifilt.sw_sharpT_rgeSgm_scale = interpolation_f32(
                paut->dyn[ilow].hfExtra_preBifilt.sw_sharpT_rgeSgm_scale, paut->dyn[ihigh].hfExtra_preBifilt.sw_sharpT_rgeSgm_scale, ratio);
    out->dyn.hfExtra_preBifilt.sw_sharpT_rgeSgm_offset = interpolation_u8(
                paut->dyn[ilow].hfExtra_preBifilt.sw_sharpT_rgeSgm_offset, paut->dyn[ihigh].hfExtra_preBifilt.sw_sharpT_rgeSgm_offset, uratio);
    out->dyn.hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha = interpolation_f32(
                paut->dyn[ilow].hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha, paut->dyn[ihigh].hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha, ratio);
    out->dyn.hfExtra_lpf.sw_sharpT_filtCfg_mode = paut->dyn[inear].hfExtra_lpf.sw_sharpT_filtCfg_mode;
    out->dyn.hfExtra_lpf.sw_sharpT_hfHi_strg = interpolation_f32(
                paut->dyn[ilow].hfExtra_lpf.sw_sharpT_hfHi_strg, paut->dyn[ihigh].hfExtra_lpf.sw_sharpT_hfHi_strg, ratio);
    out->dyn.hfExtra_lpf.sw_sharpT_hfMid_strg = interpolation_f32(
                paut->dyn[ilow].hfExtra_lpf.sw_sharpT_hfMid_strg, paut->dyn[ihigh].hfExtra_lpf.sw_sharpT_hfMid_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.hfExtra_lpf.hw_sharpT_lpf_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].hfExtra_lpf.hw_sharpT_lpf_wgt[i], paut->dyn[ihigh].hfExtra_lpf.hw_sharpT_lpf_wgt[i], ratio);
    }
    out->dyn.hfExtra_lpf.hw_sharpT_lpfOut_alpha = interpolation_f32(
                paut->dyn[ilow].hfExtra_lpf.hw_sharpT_lpfOut_alpha, paut->dyn[ihigh].hfExtra_lpf.hw_sharpT_lpfOut_alpha, ratio);
    out->dyn.hfExtra_hfBifilt.sw_sharpT_filtCfg_mode = paut->dyn[inear].hfExtra_hfBifilt.sw_sharpT_filtCfg_mode;
    out->dyn.hfExtra_hfBifilt.sw_sharpT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].hfExtra_hfBifilt.sw_sharpT_filtSpatial_strg, paut->dyn[ihigh].hfExtra_hfBifilt.sw_sharpT_filtSpatial_strg, ratio);
    for (i = 0; i < 3; i++) {
        out->dyn.hfExtra_hfBifilt.hw_sharpT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].hfExtra_hfBifilt.hw_sharpT_filtSpatial_wgt[i], paut->dyn[ihigh].hfExtra_hfBifilt.hw_sharpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.hfExtra_hfBifilt.sw_sharpT_rgeSgm_scale = interpolation_f32(
                paut->dyn[ilow].hfExtra_hfBifilt.sw_sharpT_rgeSgm_scale, paut->dyn[ihigh].hfExtra_hfBifilt.sw_sharpT_rgeSgm_scale, ratio);
    out->dyn.hfExtra_hfBifilt.sw_sharpT_rgeSgm_offset = interpolation_u8(
                paut->dyn[ilow].hfExtra_hfBifilt.sw_sharpT_rgeSgm_offset, paut->dyn[ihigh].hfExtra_hfBifilt.sw_sharpT_rgeSgm_offset, uratio);
    out->dyn.hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha = interpolation_f32(
                paut->dyn[ilow].hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha, paut->dyn[ihigh].hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha, ratio);
    for (i = 0; i < 8; i++) {
        out->dyn.shpScl_hf.hw_sharpT_luma2hfScl_val[i] = interpolation_f32(
                    paut->dyn[ilow].shpScl_hf.hw_sharpT_luma2hfScl_val[i], paut->dyn[ihigh].shpScl_hf.hw_sharpT_luma2hfScl_val[i], ratio);
    }
    for (i = 0; i < 17; i++) {
        out->dyn.sharpOpt.hw_sharpT_hfScl2ShpScl_val[i] = interpolation_u16(
                    paut->dyn[ilow].sharpOpt.hw_sharpT_hfScl2ShpScl_val[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_hfScl2ShpScl_val[i], uratio);
    }
    out->dyn.shpScl_locSgmStrg.hw_sharpT_locSgmStrg_mode = paut->dyn[inear].shpScl_locSgmStrg.hw_sharpT_locSgmStrg_mode;
    out->dyn.shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_val = interpolation_f32(
                paut->dyn[ilow].shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_val, paut->dyn[ihigh].shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_val, ratio);
    out->dyn.shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_alpha = interpolation_f32(
                paut->dyn[ilow].shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_alpha, paut->dyn[ihigh].shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_alpha, ratio);
    out->dyn.shpScl_locSgmStrg.hw_sharpT_localSgmStrg_scale = interpolation_f32(
                paut->dyn[ilow].shpScl_locSgmStrg.hw_sharpT_localSgmStrg_scale, paut->dyn[ihigh].shpScl_locSgmStrg.hw_sharpT_localSgmStrg_scale, ratio);
    out->dyn.shpScl_texDetect.hw_sharpT_estNsFilt_mode = paut->dyn[inear].shpScl_texDetect.hw_sharpT_estNsFilt_mode;
    out->dyn.shpScl_texDetect.hw_sharpT_estNsClip_mode = paut->dyn[inear].shpScl_texDetect.hw_sharpT_estNsClip_mode;
    out->dyn.shpScl_texDetect.hw_sharpT_estNsManual_maxLimit = interpolation_u16(
                paut->dyn[ilow].shpScl_texDetect.hw_sharpT_estNsManual_maxLimit, paut->dyn[ihigh].shpScl_texDetect.hw_sharpT_estNsManual_maxLimit, uratio);
    out->dyn.shpScl_texDetect.hw_sharpT_estNs_scale = interpolation_f32(
                paut->dyn[ilow].shpScl_texDetect.hw_sharpT_estNs_scale, paut->dyn[ihigh].shpScl_texDetect.hw_sharpT_estNs_scale, ratio);
    out->dyn.shpScl_texDetect.hw_sharpT_estNsNorize_shift = paut->dyn[inear].shpScl_texDetect.hw_sharpT_estNsNorize_shift;
    out->dyn.sharpOpt.hw_sharpT_shpSrc_mode = paut->dyn[inear].sharpOpt.hw_sharpT_shpSrc_mode;
    out->dyn.sharpOpt.hw_sharpT_shpOpt_mode = paut->dyn[inear].sharpOpt.hw_sharpT_shpOpt_mode;
    out->dyn.sharpOpt.hw_sharpT_hfHiGlbShpScl_val = interpolation_f32(
                paut->dyn[ilow].sharpOpt.hw_sharpT_hfHiGlbShpScl_val, paut->dyn[ihigh].sharpOpt.hw_sharpT_hfHiGlbShpScl_val, ratio);
    out->dyn.sharpOpt.hw_sharpT_hfMidGlbShpScl_val = interpolation_f32(
                paut->dyn[ilow].sharpOpt.hw_sharpT_hfMidGlbShpScl_val, paut->dyn[ihigh].sharpOpt.hw_sharpT_hfMidGlbShpScl_val, ratio);
    for (i = 0; i < 14; i++) {
        out->dyn.sharpOpt.hw_sharpT_locSgmStrg2ShpScl_val[i] = interpolation_f32(
                    paut->dyn[ilow].sharpOpt.hw_sharpT_locSgmStrg2ShpScl_val[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_locSgmStrg2ShpScl_val[i], ratio);
    }
    for (i = 0; i < 22; i++) {
        out->dyn.sharpOpt.hw_sharpT_radiDist2ShpScl_val[i] = interpolation_f32(
                    paut->dyn[ilow].sharpOpt.hw_sharpT_radiDist2ShpScl_val[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_radiDist2ShpScl_val[i], ratio);
    }
    out->dyn.sharpOpt.hw_sharpT_tex2ShpScl_scale = interpolation_f32(
                paut->dyn[ilow].sharpOpt.hw_sharpT_tex2ShpScl_scale, paut->dyn[ihigh].sharpOpt.hw_sharpT_tex2ShpScl_scale, ratio);
    out->dyn.sharpOpt.hw_sharpT_texShpSclRemap_en = paut->dyn[inear].sharpOpt.hw_sharpT_texShpSclRemap_en;
    for (i = 0; i < 17; i++) {
        out->dyn.sharpOpt.hw_sharpT_texShpSclRemap_val[i] = interpolation_u16(
                    paut->dyn[ilow].sharpOpt.hw_sharpT_texShpSclRemap_val[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_texShpSclRemap_val[i], uratio);
    }
    out->dyn.sharpOpt.sw_sharpT_bwEdgClipIdx_mode = paut->dyn[inear].sharpOpt.sw_sharpT_bwEdgClipIdx_mode;
    for (i = 0; i < 8; i++) {
        out->dyn.sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i] = interpolation_u16(
                    paut->dyn[ilow].sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i], uratio);
    }
    for (i = 0; i < 8; i++) {
        out->dyn.sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i] = interpolation_u16(
                    paut->dyn[ilow].sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i], uratio);
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn SharpApplyStrength
(
    SharpContext_t *pSharpCtx,
    sharp_param_t* out)
{
    if(pSharpCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pSharpCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pSharpCtx->fStrength);

        out->dyn.hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha /= fPercent;
        out->dyn.hfExtra_lpf.hw_sharpT_lpfOut_alpha /= fPercent;
        out->dyn.hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha /= fPercent;
        out->dyn.sharpOpt.hw_sharpT_hfHiGlbShpScl_val *= fPercent;
        out->dyn.sharpOpt.hw_sharpT_hfMidGlbShpScl_val *= fPercent;

        for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
            out->dyn.sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i] *= fPercent;
        }
        for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
            out->dyn.sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i] *= fPercent;
        }

        LOGD_ANR("SharpApplyStrength: fStrength %f, fPercent %f\n", pSharpCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    SharpContext_t* ctx = aiq_mallocz(sizeof(SharpContext_t));

    if (ctx == NULL) {
        LOGE_ASHARP("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->fStrength    = 0.5f;
    ctx->sharp_attrib =
        (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, sharp));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ASHARP("%s: Sharp (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    SharpContext_t* pSharpCtx = (SharpContext_t*)context;
    aiq_free(pSharpCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    SharpContext_t* pSharpCtx = (SharpContext_t *)params->ctx;

    pSharpCtx->working_mode = params->u.prepare.working_mode;
    pSharpCtx->sharp_attrib =
        (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, sharp));
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
    pSharpCtx->texEst_attrib =
        (texEst_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, texEst));
#endif
    pSharpCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pSharpCtx->isReCal_ = true;

    return result;
}

XCamReturn Asharp_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{

    SharpContext_t* pCtx = (SharpContext_t *)inparams->ctx;
    sharp_api_attrib_t* pAttrib = pCtx->sharp_attrib;
    sharp_param_t* sharp_res = outparams->algoRes;
    bool is_aibnr_autorun = inparams->u.proc.is_aibnr_autorun;
    int aibnr_fixIndex = inparams->u.proc.aibnr_fixIndex;
    bool is_aibnr_force_update = inparams->u.proc.is_aibnr_force_update;

    if (pAttrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", pAttrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }
    if (pAttrib->en == false || pAttrib->bypass == true) {
        if (inparams->u.proc.is_attrib_update) {
            outparams->cfg_update = true;
            outparams->en = pAttrib->en;
            outparams->bypass = pAttrib->bypass;
        }
        LOGD_ANR("SHARP not enable, skip!");
        return XCAM_RETURN_NO_ERROR;
    }

    bool need_recal = pCtx->isReCal_;
    bool init = inparams->u.proc.init;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init || is_aibnr_force_update) {
        need_recal = true;
    }

    int delta_iso = abs(iso - pCtx->pre_iso);
    if(delta_iso > 0.01 || init) {
        pCtx->pre_iso = iso;
        need_recal = true;
    }

    outparams->cfg_update = false;
    if (need_recal) {
        sharp_res->sta = pCtx->sharp_attrib->stAuto.sta;
#if RKAIQ_HAVE_SHARP_V34
        SharpSelectParam(pCtx, sharp_res, iso);
        SharpApplyStrength(pCtx, sharp_res);
#endif
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
        SharpSelectParam(pCtx, sharp_res, iso, is_aibnr_autorun, aibnr_fixIndex);
        SharpApplyStrength(pCtx, sharp_res);
#endif
        outparams->cfg_update = true;
        outparams->en = pAttrib->en;
        outparams->bypass = pAttrib->bypass;
        LOGD_ANR("SHARP processing: iso %d, en:%d, bypass:%d", iso, outparams->en, outparams->bypass);

        pCtx->isReCal_ = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AtexEst_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{

    SharpContext_t* pCtx = (SharpContext_t *)inparams->ctx;
    sharp_api_attrib_t* pAttrib = pCtx->sharp_attrib;
    void* algoRes = outparams->algoRes;
    bool is_aibnr_autorun = inparams->u.proc.is_aibnr_autorun;
    int aibnr_fixIndex = inparams->u.proc.aibnr_fixIndex;

#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
    texEstSelectParam(pCtx, algoRes, iso, is_aibnr_autorun, aibnr_fixIndex);
#endif
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = inparams->u.proc.iso;

    //Asharp_processing(inparams, outparams, iso);

    LOGV_ASHARP("%s: Sharp (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_sharp_SetAttrib(RkAiqAlgoContext *ctx, const sharp_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SharpContext_t* pSharpCtx = (SharpContext_t*)ctx;
    pSharpCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_sharp_GetAttrib(const RkAiqAlgoContext *ctx, sharp_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SharpContext_t* pSharpCtx = (SharpContext_t*)ctx;
    sharp_api_attrib_t* sharp_attrib = pSharpCtx->sharp_attrib;

    attr->opMode = sharp_attrib->opMode;
    attr->en = sharp_attrib->en;
    attr->bypass = sharp_attrib->bypass;
    memcpy(&attr->stAuto, &sharp_attrib->stAuto, sizeof(sharp_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif
XCamReturn
algo_sharp_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SharpContext_t* pSharpCtx = (SharpContext_t*)ctx;

    pSharpCtx->fStrength = strg;
    pSharpCtx->strength_en = strg_en;
    pSharpCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_sharp_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SharpContext_t* pSharpCtx = (SharpContext_t*)ctx;

    *strg = pSharpCtx->fStrength;
    *strg_en = pSharpCtx->strength_en;

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_SHARP_VERSION     "v0.0.9"
#define RKISP_ALGO_SHARP_VENDOR      "Rockchip"
#define RKISP_ALGO_SHARP_DESCRIPTION "Rockchip sharp algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescSharp = {
    .common = {
        .version = RKISP_ALGO_SHARP_VERSION,
        .vendor  = RKISP_ALGO_SHARP_VENDOR,
        .description = RKISP_ALGO_SHARP_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ASHARP,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

