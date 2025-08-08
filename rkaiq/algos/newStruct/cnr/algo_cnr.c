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

#include "cnr_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"
#include "newStruct/algo_common.h"

#if RKAIQ_HAVE_CNR_V30
XCamReturn CnrSelectParam
(
    cnr_param_auto_t *pAuto,
    cnr_param_t* out,
    int iso)
{
    if(pAuto == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    cnr_param_auto_t *paut = pAuto;

    pre_interp(iso, NULL, 0, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_CNR_V31
XCamReturn CnrSelectParam
(
    CnrContext_t *pCnrCtx,
    cnr_param_t* out,
    int iso)
{
    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    cnr_param_auto_t *paut = &pCnrCtx->cnr_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pre_interp(iso, pCnrCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    out->dyn.locSgmStrg.hw_cnrT_glbSgmStrg_val = interpolation_f32(
                paut->dyn[ilow].locSgmStrg.hw_cnrT_glbSgmStrg_val, paut->dyn[ihigh].locSgmStrg.hw_cnrT_glbSgmStrg_val, ratio);
    out->dyn.locSgmStrg.hw_cnrT_glbSgmStrg_alpha = interpolation_f32(
                paut->dyn[ilow].locSgmStrg.hw_cnrT_glbSgmStrg_alpha, paut->dyn[ihigh].locSgmStrg.hw_cnrT_glbSgmStrg_alpha, ratio);
    out->dyn.locSgmStrg.hw_cnrT_locSgmStrg_scale = interpolation_f32(
                paut->dyn[ilow].locSgmStrg.hw_cnrT_locSgmStrg_scale, paut->dyn[ihigh].locSgmStrg.hw_cnrT_locSgmStrg_scale, ratio);
    out->dyn.loNrGuide_preProc.hw_cnrT_ds_mode = paut->dyn[inear].loNrGuide_preProc.hw_cnrT_ds_mode;
    out->dyn.loNrGuide_preProc.hw_cnrT_uvEdg_strg = interpolation_f32(
                paut->dyn[ilow].loNrGuide_preProc.hw_cnrT_uvEdg_strg, paut->dyn[ihigh].loNrGuide_preProc.hw_cnrT_uvEdg_strg, ratio);
    out->dyn.loNrGuide_bifilt.sw_cnrT_filtCfg_mode = paut->dyn[inear].loNrGuide_bifilt.sw_cnrT_filtCfg_mode;
    out->dyn.loNrGuide_bifilt.sw_cnrT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].loNrGuide_bifilt.sw_cnrT_filtSpatial_strg, paut->dyn[ihigh].loNrGuide_bifilt.sw_cnrT_filtSpatial_strg, ratio);
    for (i = 0; i < 4; i++) {
        out->dyn.loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[i] = paut->dyn[inear].loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[i];
    }
    out->dyn.loNrGuide_bifilt.sw_cnrT_rgeSgm_val = interpolation_f32(
                paut->dyn[ilow].loNrGuide_bifilt.sw_cnrT_rgeSgm_val, paut->dyn[ihigh].loNrGuide_bifilt.sw_cnrT_rgeSgm_val, ratio);
    out->dyn.loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha = interpolation_f32(
                paut->dyn[ilow].loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha, paut->dyn[ihigh].loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[i] = paut->dyn[inear].loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[i];
    }
    out->dyn.loNrGuide_iirFilt.sw_cnrT_rgeSgm_val = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.sw_cnrT_rgeSgm_val, paut->dyn[ihigh].loNrGuide_iirFilt.sw_cnrT_rgeSgm_val, ratio);
    //out->dyn.loNrGuide_iirFilt.sw_cnrT_rgeSgmRatio_mode = paut->dyn[inear].loNrGuide_iirFilt.sw_cnrT_rgeSgmRatio_mode;
    out->dyn.loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio, ratio);
    //out->dyn.loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha = interpolation_f32(
    //            paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_filtWgtZero_mode = paut->dyn[inear].hiNr_bifilt.hw_cnrT_filtWgtZero_mode;
    out->dyn.hiNr_preLpf.sw_cnrT_filtCfg_mode = paut->dyn[inear].hiNr_preLpf.sw_cnrT_filtCfg_mode;
    out->dyn.hiNr_preLpf.sw_cnrT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].hiNr_preLpf.sw_cnrT_filtSpatial_strg, paut->dyn[ihigh].hiNr_preLpf.sw_cnrT_filtSpatial_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i], paut->dyn[ihigh].hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.hiNr_preLpf.hw_cnrT_lpfOut_alpha = interpolation_f32(
                paut->dyn[ilow].hiNr_preLpf.hw_cnrT_lpfOut_alpha, paut->dyn[ihigh].hiNr_preLpf.hw_cnrT_lpfOut_alpha, ratio);
    for (i = 0; i < 8; i++) {
        out->dyn.hw_cnrC_luma2HiNrSgm_curve.idx[i] = paut->dyn[inear].hw_cnrC_luma2HiNrSgm_curve.idx[i];
    }
    for (i = 0; i < 8; i++) {
        out->dyn.hw_cnrC_luma2HiNrSgm_curve.val[i] = interpolation_f32(
                    paut->dyn[ilow].hw_cnrC_luma2HiNrSgm_curve.val[i], paut->dyn[ihigh].hw_cnrC_luma2HiNrSgm_curve.val[i], ratio);
    }
    out->dyn.hiNr_bifilt.hw_cnrT_uvEdg_strg = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_uvEdg_strg, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_uvEdg_strg, ratio);
    for (i = 0; i < 13; i++) {
        out->dyn.hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i], paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i], ratio);
    }
    for (i = 0; i < 13; i++) {
        out->dyn.hiNr_bifilt.hw_cnrT_locSgmStrg2CtrWgt_scale[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_bifilt.hw_cnrT_locSgmStrg2CtrWgt_scale[i], paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_locSgmStrg2CtrWgt_scale[i], ratio);
    }
    out->dyn.hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_satAdj_offset = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_satAdj_offset, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_satAdj_offset, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_satAdj_scale = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_satAdj_scale, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_satAdj_scale, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_bifiltOut_alpha = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_bifiltOut_alpha, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_bifiltOut_alpha, ratio);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn CnrApplyStrength
(
    CnrContext_t *pCnrCtx,
    cnr_param_t* out)
{
    int i = 0;

    if(pCnrCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pCnrCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pCnrCtx->fStrength);

        cnr_params_dyn_t* pdyn = &out->dyn;
        pdyn->loNrGuide_bifilt.sw_cnrT_rgeSgm_val *= fPercent ;
        pdyn->loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha *= fPercent ;
        pdyn->loNrGuide_iirFilt.sw_cnrT_rgeSgm_val *= fPercent ;

        for (i = 0; i < 8; i++) {
            pdyn->hw_cnrC_luma2HiNrSgm_curve.val[i] *= fPercent ;

        }
        for (i = 0; i < 13; i++) {
            out->dyn.hiNr_bifilt.hw_cnrT_locSgmStrg2CtrWgt_scale[i] /= fPercent;
        }
        pdyn->hiNr_bifilt.hw_cnrT_bifiltOut_alpha *= fPercent;
        LOGD_ANR("CnrApplyStrength: fStrength %f, fPercent %f\n", pCnrCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if defined(RKAIQ_HAVE_CNR_V35) || defined(RKAIQ_HAVE_CNR_V36)
XCamReturn CnrSelectParam
(
    CnrContext_t *pCnrCtx,
    cnr_param_t* out,
    int iso,
    bool is_aibnr_autorun,
    int aibnr_fixIndex)
{
    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    cnr_param_auto_t *paut = &pCnrCtx->cnr_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pre_interp(iso, pCnrCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    if (aibnr_fixIndex != -1 && is_aibnr_autorun) {
        ilow = aibnr_fixIndex;
        ihigh = aibnr_fixIndex;
        ratio = 0;
    }
    uratio = ratio * (1 << RATIO_FIXBIT);
    LOGD_ANR("%s ilow %d, ihigh %d, ratio %f, aibnr_fixIndex %d, is_aibnr_autorun %d",
        __func__, ilow, ihigh,ratio, aibnr_fixIndex, is_aibnr_autorun);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    out->dyn.locSgmStrg.hw_cnrT_glbSgmStrg_val = interpolation_f32(
                paut->dyn[ilow].locSgmStrg.hw_cnrT_glbSgmStrg_val, paut->dyn[ihigh].locSgmStrg.hw_cnrT_glbSgmStrg_val, ratio);
    out->dyn.locSgmStrg.hw_cnrT_glbSgmStrg_alpha = interpolation_f32(
                paut->dyn[ilow].locSgmStrg.hw_cnrT_glbSgmStrg_alpha, paut->dyn[ihigh].locSgmStrg.hw_cnrT_glbSgmStrg_alpha, ratio);
    out->dyn.locSgmStrg.hw_cnrT_locSgmStrg_scale = interpolation_f32(
                paut->dyn[ilow].locSgmStrg.hw_cnrT_locSgmStrg_scale, paut->dyn[ihigh].locSgmStrg.hw_cnrT_locSgmStrg_scale, ratio);
    out->dyn.loNrGuide_preProc.hw_cnrT_ds_mode = paut->dyn[inear].loNrGuide_preProc.hw_cnrT_ds_mode;
    out->dyn.loNrGuide_preProc.hw_cnrT_uvEdg_strg = interpolation_f32(
                paut->dyn[ilow].loNrGuide_preProc.hw_cnrT_uvEdg_strg, paut->dyn[ihigh].loNrGuide_preProc.hw_cnrT_uvEdg_strg, ratio);
    out->dyn.loNrGuide_bifilt.sw_cnrT_filtCfg_mode = paut->dyn[inear].loNrGuide_bifilt.sw_cnrT_filtCfg_mode;
    out->dyn.loNrGuide_bifilt.sw_cnrT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].loNrGuide_bifilt.sw_cnrT_filtSpatial_strg, paut->dyn[ihigh].loNrGuide_bifilt.sw_cnrT_filtSpatial_strg, ratio);
    for (i = 0; i < 4; i++) {
        out->dyn.loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[i], paut->dyn[ihigh].loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.loNrGuide_bifilt.sw_cnrT_rgeSgm_val = interpolation_f32(
                paut->dyn[ilow].loNrGuide_bifilt.sw_cnrT_rgeSgm_val, paut->dyn[ihigh].loNrGuide_bifilt.sw_cnrT_rgeSgm_val, ratio);
    out->dyn.loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha = interpolation_f32(
                paut->dyn[ilow].loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha, paut->dyn[ihigh].loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[i] = interpolation_u8(
                    paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[i], paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[i], uratio);
    }
    out->dyn.loNrGuide_iirFilt.sw_cnrT_rgeSgm_val = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.sw_cnrT_rgeSgm_val, paut->dyn[ihigh].loNrGuide_iirFilt.sw_cnrT_rgeSgm_val, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred, ratio);
    out->dyn.loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_iirFiltStrg_maxLimit, ratio);
    out->dyn.hiNr_preLpf.sw_cnrT_filtCfg_mode = paut->dyn[inear].hiNr_preLpf.sw_cnrT_filtCfg_mode;
    out->dyn.hiNr_preLpf.sw_cnrT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].hiNr_preLpf.sw_cnrT_filtSpatial_strg, paut->dyn[ihigh].hiNr_preLpf.sw_cnrT_filtSpatial_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i], paut->dyn[ihigh].hiNr_preLpf.hw_cnrT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.hiNr_preLpf.hw_cnrT_lpfOut_alpha = interpolation_f32(
                paut->dyn[ilow].hiNr_preLpf.hw_cnrT_lpfOut_alpha, paut->dyn[ihigh].hiNr_preLpf.hw_cnrT_lpfOut_alpha, ratio);
    for (i = 0; i < 8; i++) {
        out->dyn.hw_cnrC_luma2HiNrSgm_curve.idx[i] = interpolation_u16(
                    paut->dyn[ilow].hw_cnrC_luma2HiNrSgm_curve.idx[i], paut->dyn[ihigh].hw_cnrC_luma2HiNrSgm_curve.idx[i], uratio);
    }
    for (i = 0; i < 8; i++) {
        out->dyn.hw_cnrC_luma2HiNrSgm_curve.val[i] = interpolation_f32(
                    paut->dyn[ilow].hw_cnrC_luma2HiNrSgm_curve.val[i], paut->dyn[ihigh].hw_cnrC_luma2HiNrSgm_curve.val[i], ratio);
    }
    out->dyn.hiNr_bifilt.hw_cnrT_uvEdg_strg = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_uvEdg_strg, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_uvEdg_strg, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_filtWgtZero_mode = paut->dyn[inear].hiNr_bifilt.hw_cnrT_filtWgtZero_mode;
    for (i = 0; i < 13; i++) {
        out->dyn.hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i], paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i], ratio);
    }
    for (i = 0; i < 13; i++) {
        out->dyn.hiNr_bifilt.hw_cnrT_locSgmStrg2CtrWgt_scale[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_bifilt.hw_cnrT_locSgmStrg2CtrWgt_scale[i], paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_locSgmStrg2CtrWgt_scale[i], ratio);
    }
    out->dyn.hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_satAdj_offset = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_satAdj_offset, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_satAdj_offset, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_satAdj_scale = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_satAdj_scale, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_satAdj_scale, ratio);
    out->dyn.hiNr_bifilt.hw_cnrT_bifiltOut_alpha = interpolation_f32(
                paut->dyn[ilow].hiNr_bifilt.hw_cnrT_bifiltOut_alpha, paut->dyn[ihigh].hiNr_bifilt.hw_cnrT_bifiltOut_alpha, ratio);
    out->dyn.hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_en = paut->dyn[inear].hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_en;
    out->dyn.hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_maxLimit = interpolation_f32(
                paut->dyn[ilow].hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_maxLimit, paut->dyn[ihigh].hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_maxLimit, ratio);
    out->dyn.hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_minLimit = interpolation_f32(
                paut->dyn[ilow].hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_minLimit, paut->dyn[ihigh].hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_minLimit, ratio);
#if RKAIQ_HAVE_CNR_V36
    for(i = 0; i < 13; i++) {
        out->dyn.loNrGuide_iirFilt.hw_cnrT_locSgmStrg2SgmRat_val[i] = interpolation_f32(
                    paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_locSgmStrg2SgmRat_val[i],
                    paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_locSgmStrg2SgmRat_val[i], ratio);
    }
    out->dyn.loNrGuide_iirFilt.sw_cnrT_rgeSgmRatio_mode = paut->dyn[inear].loNrGuide_iirFilt.sw_cnrT_rgeSgmRatio_mode;
    out->dyn.loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha = interpolation_f32(
                paut->dyn[ilow].loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha, paut->dyn[ihigh].loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha, ratio);
    out->dyn.hiNr_locFiltAlpha.hw_cnrT_hueSatAdj_en = paut->dyn[inear].hiNr_locFiltAlpha.hw_cnrT_hueSatAdj_en;
    for (i = 0; i < 10; i++) {
        out->dyn.hiNr_locFiltAlpha.hw_cnrT_locSgmStrg2NrOutAlpha[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_locFiltAlpha.hw_cnrT_locSgmStrg2NrOutAlpha[i], paut->dyn[ihigh].hiNr_locFiltAlpha.hw_cnrT_locSgmStrg2NrOutAlpha[i], ratio);
        out->dyn.hiNr_locFiltAlpha.hw_cnrT_hue2NrOutAlpha[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_locFiltAlpha.hw_cnrT_hue2NrOutAlpha[i], paut->dyn[ihigh].hiNr_locFiltAlpha.hw_cnrT_hue2NrOutAlpha[i], ratio);
        out->dyn.hiNr_locFiltAlpha.hw_cnrT_sat2NrOutAlpha[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr_locFiltAlpha.hw_cnrT_sat2NrOutAlpha[i], paut->dyn[ihigh].hiNr_locFiltAlpha.hw_cnrT_sat2NrOutAlpha[i], ratio);
    }
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn CnrApplyStrength
(
    CnrContext_t *pCnrCtx,
    cnr_param_t* out)
{
    int i = 0;

    if(pCnrCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pCnrCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pCnrCtx->fStrength);

        cnr_params_dyn_t* pdyn = &out->dyn;
        pdyn->loNrGuide_bifilt.sw_cnrT_rgeSgm_val *= fPercent ;
        pdyn->loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha *= fPercent ;
        pdyn->loNrGuide_iirFilt.sw_cnrT_rgeSgm_val *= fPercent ;


        for (i = 0; i < 8; i++) {
            pdyn->hw_cnrC_luma2HiNrSgm_curve.val[i] *= fPercent ;

        }
        for (i = 0; i < 13; i++) {
            pdyn->hiNr_bifilt.hw_cnrT_locSgmStrg2CtrWgt_scale[i] /= fPercent;
        }
        pdyn->hiNr_bifilt.hw_cnrT_bifiltOut_alpha *= fPercent;

        pdyn->hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_maxLimit *= fPercent;
        pdyn->hiNr_locFiltAlpha.hw_cnrT_locFiltAlpha_minLimit *= fPercent;
        LOGD_ANR("CnrApplyStrength: fStrength %f, fPercent %f\n", pCnrCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}

#endif

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    CnrContext_t* ctx = aiq_mallocz(sizeof(CnrContext_t));

    if (ctx == NULL) {
        LOGE_ANR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->cnr_attrib =
        (cnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, cnr));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ANR("%s: Cnr (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CnrContext_t* pCnrCtx = (CnrContext_t*)context;
    aiq_free(pCnrCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CnrContext_t* pCnrCtx = (CnrContext_t *)params->ctx;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pCnrCtx->cnr_attrib =
                (cnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cnr));
            pCnrCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pCnrCtx->working_mode = params->u.prepare.working_mode;
    pCnrCtx->cnr_attrib =
        (cnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cnr));
    pCnrCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pCnrCtx->isReCal_ = true;

    return result;
}

XCamReturn Acnr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{

    CnrContext_t* pCnrCtx = (CnrContext_t *)inparams->ctx;
    cnr_api_attrib_t* cnr_attrib = pCnrCtx->cnr_attrib;
    cnr_param_t* cnr_res = outparams->algoRes;
    bool is_aibnr_autorun = inparams->u.proc.is_aibnr_autorun;
    int aibnr_fixIndex = inparams->u.proc.aibnr_fixIndex;
    bool is_aibnr_force_update = inparams->u.proc.is_aibnr_force_update;

    if (cnr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", cnr_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }
    if (cnr_attrib->en == false || cnr_attrib->bypass == true) {
        if (inparams->u.proc.is_attrib_update) {
            outparams->cfg_update = true;
            outparams->en = cnr_attrib->en;
            outparams->bypass = cnr_attrib->bypass;
        }
        LOGD_ANR("CNR not enable, skip!");
        return XCAM_RETURN_NO_ERROR;
    }

    bool need_recal = pCnrCtx->isReCal_;

    bool init = inparams->u.proc.init;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init || is_aibnr_force_update) {
        need_recal = true;
    }

    int delta_iso = abs(iso - pCnrCtx->pre_iso);
    if(delta_iso > 0.01 || init) {
        pCnrCtx->pre_iso = iso;
        need_recal = true;
    }

    outparams->cfg_update = false;
    if (need_recal) {
#if RKAIQ_HAVE_CNR_V31
        CnrSelectParam(pCnrCtx, cnr_res, iso);
        CnrApplyStrength(pCnrCtx, cnr_res);
#endif
#if defined(RKAIQ_HAVE_CNR_V35) || defined(RKAIQ_HAVE_CNR_V36)
        CnrSelectParam(pCnrCtx, cnr_res, iso, is_aibnr_autorun, aibnr_fixIndex);
        CnrApplyStrength(pCnrCtx, cnr_res);
#endif
        outparams->cfg_update = true;
        outparams->en = cnr_attrib->en;
        outparams->bypass = cnr_attrib->bypass;
        LOGD_ANR("CNR processing: iso %d, cnr en:%d, bypass:%d", iso, outparams->en, outparams->bypass);

        pCnrCtx->isReCal_ = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    int iso = inparams->u.proc.iso;
    Acnr_processing(inparams, outparams, iso);

    LOGV_ANR("%s: Cnr (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_cnr_SetAttrib(RkAiqAlgoContext *ctx,
                   const cnr_api_attrib_t *attr,
                   bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CnrContext_t* pCnrCtx = (CnrContext_t*)ctx;
    pCnrCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_cnr_GetAttrib(const RkAiqAlgoContext *ctx,
                   cnr_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CnrContext_t* pCnrCtx = (CnrContext_t*)ctx;
    cnr_api_attrib_t* cnr_attrib = pCnrCtx->cnr_attrib;

    attr->opMode = cnr_attrib->opMode;
    attr->en = cnr_attrib->en;
    attr->bypass = cnr_attrib->bypass;
    memcpy(&attr->stAuto, &cnr_attrib->stAuto, sizeof(cnr_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif
XCamReturn
algo_cnr_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CnrContext_t* pCnrCtx = (CnrContext_t*)ctx;

    pCnrCtx->fStrength = strg;
    pCnrCtx->strength_en = strg_en;
    pCnrCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_cnr_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CnrContext_t* pCnrCtx = (CnrContext_t*)ctx;

    *strg = pCnrCtx->fStrength;
    *strg_en = pCnrCtx->strength_en;

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_CNR_VERSION     "v0.0.9"
#define RKISP_ALGO_CNR_VENDOR      "Rockchip"
#define RKISP_ALGO_CNR_DESCRIPTION "Rockchip cnr algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescCnr = {
    .common = {
        .version = RKISP_ALGO_CNR_VERSION,
        .vendor  = RKISP_ALGO_CNR_VENDOR,
        .description = RKISP_ALGO_CNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACNR,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

