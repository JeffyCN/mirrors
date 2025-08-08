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
#include "ynr_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"
#include "newStruct/algo_common.h"

#define YNR_V22_ISO_CURVE_POINT_BIT          4
#define YNR_V22_ISO_CURVE_POINT_NUM          ((1 << YNR_V22_ISO_CURVE_POINT_BIT)+1)

#if RKAIQ_HAVE_YNR_V22
XCamReturn YnrSelectParam
(
    ynr_param_auto_t *pAuto,
    ynr_param_t* out,
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
    ynr_param_auto_t *paut = pAuto;

    pre_interp(iso, NULL, 0, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);
    {
        out->dyn.loNrBifilt3.hw_ynrT_bifilt3_en = interpolation_bool(
                    paut->dyn[ilow].loNrBifilt3.hw_ynrT_bifilt3_en, paut->dyn[ihigh].loNrBifilt3.hw_ynrT_bifilt3_en, uratio);
        out->dyn.loNrBifilt3.sw_ynrT_guideImgLpf_strg = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt3.sw_ynrT_guideImgLpf_strg, paut->dyn[ihigh].loNrBifilt3.sw_ynrT_guideImgLpf_strg, ratio);
        out->dyn.loNrBifilt3.hw_ynrT_guideImgLpf_alpha = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt3.hw_ynrT_guideImgLpf_alpha, paut->dyn[ihigh].loNrBifilt3.hw_ynrT_guideImgLpf_alpha, ratio);
        out->dyn.loNrBifilt3.hw_ynrT_rgeSgm_div = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt3.hw_ynrT_rgeSgm_div, paut->dyn[ihigh].loNrBifilt3.hw_ynrT_rgeSgm_div, ratio);
        out->dyn.loNrBifilt3.hw_ynrT_bifiltOut_alpha = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt3.hw_ynrT_bifiltOut_alpha, paut->dyn[ihigh].loNrBifilt3.hw_ynrT_bifiltOut_alpha, ratio);
        out->dyn.loNrBifilt5.hw_ynrT_bifilt5_en = interpolation_bool(
                    paut->dyn[ilow].loNrBifilt5.hw_ynrT_bifilt5_en, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_bifilt5_en, uratio);
        out->dyn.loNrBifilt5.hw_ynrT_guideImg_mode = ratio > 0.5 ?
                paut->dyn[ihigh].loNrBifilt5.hw_ynrT_guideImg_mode : paut->dyn[ilow].loNrBifilt5.hw_ynrT_guideImg_mode;
        out->dyn.loNrBifilt5.hw_ynrT_guideImgText_maxLimit = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt5.hw_ynrT_guideImgText_maxLimit, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_guideImgText_maxLimit, ratio);
        out->dyn.loNrBifilt5.hw_ynrT_rgeSgm_scale = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt5.hw_ynrT_rgeSgm_scale, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_rgeSgm_scale, ratio);
        out->dyn.loNrBifilt5.hw_ynrT_spatialSgm_scale = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt5.hw_ynrT_spatialSgm_scale, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_spatialSgm_scale, ratio);
        out->dyn.loNrBifilt5.hw_ynrT_centerPix_wgt = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt5.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_centerPix_wgt, ratio);
        out->dyn.loNrBifilt5.hw_ynrT_nhoodPixWgt_thred = interpolation_f32(
                    paut->dyn[ilow].loNrBifilt5.hw_ynrT_nhoodPixWgt_thred, paut->dyn[ihigh].loNrBifilt5.hw_ynrT_nhoodPixWgt_thred, ratio);
        out->dyn.loNrPost.hw_ynrT_loNROutSel_mode = ratio > 0.5 ?
                paut->dyn[ihigh].loNrPost.hw_ynrT_loNROutSel_mode : paut->dyn[ilow].loNrPost.hw_ynrT_loNROutSel_mode;
        out->dyn.loNrPost.hw_ynrT_softThd_val = interpolation_f32(
                paut->dyn[ilow].loNrPost.hw_ynrT_softThd_val, paut->dyn[ihigh].loNrPost.hw_ynrT_softThd_val, ratio);
        out->dyn.loNrPost.hw_ynrT_loNr_alpha = interpolation_f32(
                paut->dyn[ilow].loNrPost.hw_ynrT_loNr_alpha, paut->dyn[ihigh].loNrPost.hw_ynrT_loNr_alpha, ratio);
        for (i = 0; i < 17; i++) {
            out->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.idx[i] = interpolation_u16(
                        paut->dyn[ilow].loNrPost.hw_ynrC_luma2LoSgm_curve.idx[i], paut->dyn[ihigh].loNrPost.hw_ynrC_luma2LoSgm_curve.idx[i], uratio);
        }
        for (i = 0; i < 17; i++) {
            out->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[i] = interpolation_u16(
                        paut->dyn[ilow].loNrPost.hw_ynrC_luma2LoSgm_curve.val[i], paut->dyn[ihigh].loNrPost.hw_ynrC_luma2LoSgm_curve.val[i], uratio);
        }
        out->dyn.loNrGaus.hw_ynrT_gausFilt_en = interpolation_bool(
                paut->dyn[ilow].loNrGaus.hw_ynrT_gausFilt_en, paut->dyn[ihigh].loNrGaus.hw_ynrT_gausFilt_en, uratio);
        out->dyn.loNrGaus.sw_ynrT_gausFilt_strg = interpolation_f32(
                    paut->dyn[ilow].loNrGaus.sw_ynrT_gausFilt_strg, paut->dyn[ihigh].loNrGaus.sw_ynrT_gausFilt_strg, ratio);
        out->dyn.inPixSgm.hw_ynrT_localPixSgm_scale = interpolation_f32(
                    paut->dyn[ilow].inPixSgm.hw_ynrT_localPixSgm_scale, paut->dyn[ihigh].inPixSgm.hw_ynrT_localPixSgm_scale, ratio);
        out->dyn.inPixSgm.hw_ynrT_localPixSgmScl2_thred = interpolation_f32(
                    paut->dyn[ilow].inPixSgm.hw_ynrT_localPixSgmScl2_thred, paut->dyn[ihigh].inPixSgm.hw_ynrT_localPixSgmScl2_thred, ratio);
        out->dyn.inPixSgm.hw_ynrT_localPixSgm_scale2 = interpolation_f32(
                    paut->dyn[ilow].inPixSgm.hw_ynrT_localPixSgm_scale2, paut->dyn[ihigh].inPixSgm.hw_ynrT_localPixSgm_scale2, ratio);
        out->dyn.inPixSgm.hw_ynrT_glbPixSgm_val = interpolation_f32(
                    paut->dyn[ilow].inPixSgm.hw_ynrT_glbPixSgm_val, paut->dyn[ihigh].inPixSgm.hw_ynrT_glbPixSgm_val, ratio);
        out->dyn.inPixSgm.hw_ynrT_glbPixSgm_alpha = interpolation_f32(
                    paut->dyn[ilow].inPixSgm.hw_ynrT_glbPixSgm_alpha, paut->dyn[ihigh].inPixSgm.hw_ynrT_glbPixSgm_alpha, ratio);
        out->dyn.hiNr.hw_ynrT_nlmFilt_en = interpolation_bool(
                                               paut->dyn[ilow].hiNr.hw_ynrT_nlmFilt_en, paut->dyn[ihigh].hiNr.hw_ynrT_nlmFilt_en, uratio);
        for (i = 0; i < 17; i++) {
            out->dyn.hiNr.hw_ynrT_radiDist2Wgt_val[i] = interpolation_f32(
                        paut->dyn[ilow].hiNr.hw_ynrT_radiDist2Wgt_val[i], paut->dyn[ihigh].hiNr.hw_ynrT_radiDist2Wgt_val[i], ratio);
        }
        out->dyn.hiNr.hw_ynrT_pixSgmRadiDistWgt_alpha = interpolation_f32(
                    paut->dyn[ilow].hiNr.hw_ynrT_pixSgmRadiDistWgt_alpha, paut->dyn[ihigh].hiNr.hw_ynrT_pixSgmRadiDistWgt_alpha, ratio);
        out->dyn.hiNr.hw_ynrT_nlmSgm_minLimit = interpolation_f32(
                paut->dyn[ilow].hiNr.hw_ynrT_nlmSgm_minLimit, paut->dyn[ihigh].hiNr.hw_ynrT_nlmSgm_minLimit, ratio);
        out->dyn.hiNr.hw_ynrT_nlmSgm_scale = interpolation_f32(
                paut->dyn[ilow].hiNr.hw_ynrT_nlmSgm_scale, paut->dyn[ihigh].hiNr.hw_ynrT_nlmSgm_scale, ratio);
        out->dyn.hiNr.hw_ynrT_nlmRgeWgt_negOffset = interpolation_f32(
                    paut->dyn[ilow].hiNr.hw_ynrT_nlmRgeWgt_negOffset, paut->dyn[ihigh].hiNr.hw_ynrT_nlmRgeWgt_negOffset, ratio);
        for (i = 0; i < 7; i++) {
            out->dyn.hiNr.hw_ynrT_nlmSpatial_wgt[i] = interpolation_u8(
                        paut->dyn[ilow].hiNr.hw_ynrT_nlmSpatial_wgt[i], paut->dyn[ihigh].hiNr.hw_ynrT_nlmSpatial_wgt[i], uratio);
        }
        out->dyn.hiNr.hw_ynrT_nlmOut_alpha = interpolation_f32(
                paut->dyn[ilow].hiNr.hw_ynrT_nlmOut_alpha, paut->dyn[ihigh].hiNr.hw_ynrT_nlmOut_alpha, ratio);
    }
    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_YNR_V24
XCamReturn YnrSelectParam
(
    YnrContext_t *pYnrCtx,
    ynr_param_t* out,
    int iso)
{
    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    ynr_param_auto_t *paut = &pYnrCtx->ynr_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pre_interp(iso, pYnrCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    for (i = 0; i < 17; i++) {
        out->dyn.ynrScl_radi.hw_ynrT_radiDist2YnrScl_val[i] = interpolation_f32(
                    paut->dyn[ilow].ynrScl_radi.hw_ynrT_radiDist2YnrScl_val[i], paut->dyn[ihigh].ynrScl_radi.hw_ynrT_radiDist2YnrScl_val[i], ratio);
    }
    out->dyn.ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_val = interpolation_f32(
                paut->dyn[ilow].ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_val, paut->dyn[ihigh].ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_val, ratio);
    out->dyn.ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_alpha = interpolation_f32(
                paut->dyn[ilow].ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_alpha, paut->dyn[ihigh].ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_alpha, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrScl_val[i] = interpolation_f32(
                    paut->dyn[ilow].ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrScl_val[i], paut->dyn[ihigh].ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrScl_val[i], ratio);
    }
    for (i = 0; i < 5; i++) {
        out->dyn.coeff2SgmCurve.sigma_coeff[i] = interpolation_f32(
                    paut->dyn[ilow].coeff2SgmCurve.sigma_coeff[i], paut->dyn[ihigh].coeff2SgmCurve.sigma_coeff[i], ratio);
    }
    out->dyn.coeff2SgmCurve.lowFreqCoeff = interpolation_f32(
            paut->dyn[ilow].coeff2SgmCurve.lowFreqCoeff, paut->dyn[ihigh].coeff2SgmCurve.lowFreqCoeff, ratio);
    for (i = 0; i < 17; i++) {
        out->dyn.hw_ynrC_luma2Sigma_curve.idx[i] = interpolation_u16(
                    paut->dyn[ilow].hw_ynrC_luma2Sigma_curve.idx[i], paut->dyn[ihigh].hw_ynrC_luma2Sigma_curve.idx[i], uratio);
    }
    for (i = 0; i < 17; i++) {
        out->dyn.hw_ynrC_luma2Sigma_curve.val[i] = interpolation_f32(
                    paut->dyn[ilow].hw_ynrC_luma2Sigma_curve.val[i], paut->dyn[ihigh].hw_ynrC_luma2Sigma_curve.val[i], ratio);
    }
    out->dyn.hw_ynrT_loNr_en = paut->dyn[inear].hw_ynrT_loNr_en;
    out->dyn.loNr_preProc.sw_ynrT_preLpfCfg_mode = paut->dyn[inear].loNr_preProc.sw_ynrT_preLpfCfg_mode;
    for (i = 0; i < 3; i++) {
        out->dyn.loNr_preProc.hw_ynrT_preLpfSpatial_wgt[i] = interpolation_u8(
                    paut->dyn[ilow].loNr_preProc.hw_ynrT_preLpfSpatial_wgt[i], paut->dyn[ihigh].loNr_preProc.hw_ynrT_preLpfSpatial_wgt[i], uratio);
    }
    out->dyn.loNr_preProc.sw_ynrT_preLpf_strg = interpolation_f32(
                paut->dyn[ilow].loNr_preProc.sw_ynrT_preLpf_strg, paut->dyn[ihigh].loNr_preProc.sw_ynrT_preLpf_strg, ratio);
    out->dyn.loNr_preProc.sw_ynrT_edgeDctConf_scale = interpolation_f32(
                paut->dyn[ilow].loNr_preProc.sw_ynrT_edgeDctConf_scale, paut->dyn[ihigh].loNr_preProc.sw_ynrT_edgeDctConf_scale, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_localYnrScl_alpha = interpolation_f32(
                paut->dyn[ilow].loNr_iirGuide.hw_ynrT_localYnrScl_alpha, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_localYnrScl_alpha, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_iiFilt_strg = interpolation_f32(
                paut->dyn[ilow].loNr_iirGuide.hw_ynrT_iiFilt_strg, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_iiFilt_strg, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_pixDiffEge_thred = interpolation_f32(
                paut->dyn[ilow].loNr_iirGuide.hw_ynrT_pixDiffEge_thred, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_pixDiffEge_thred, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_centerPix_wgt = interpolation_f32(
                paut->dyn[ilow].loNr_iirGuide.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_centerPix_wgt, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_iirInitWgt_scale = interpolation_f32(
                paut->dyn[ilow].loNr_iirGuide.hw_ynrT_iirInitWgt_scale, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_iirInitWgt_scale, ratio);
    out->dyn.loNr_iirGuide.hw_ynrT_softThd_scale = interpolation_f32(
                paut->dyn[ilow].loNr_iirGuide.hw_ynrT_softThd_scale, paut->dyn[ihigh].loNr_iirGuide.hw_ynrT_softThd_scale, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_rgeSgm_scale = interpolation_f32(
                paut->dyn[ilow].loNr_bifilt.hw_ynrT_rgeSgm_scale, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_rgeSgm_scale, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_filtSpatialV_strg = interpolation_f32(
                paut->dyn[ilow].loNr_bifilt.hw_ynrT_filtSpatialV_strg, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_filtSpatialV_strg, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_filtSpatialH_strg = interpolation_f32(
                paut->dyn[ilow].loNr_bifilt.hw_ynrT_filtSpatialH_strg, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_filtSpatialH_strg, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_centerPix_wgt = interpolation_f32(
                paut->dyn[ilow].loNr_bifilt.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_centerPix_wgt, ratio);
    out->dyn.loNr_bifilt.hw_ynrT_bifiltOut_alpha = interpolation_f32(
                paut->dyn[ilow].loNr_bifilt.hw_ynrT_bifiltOut_alpha, paut->dyn[ihigh].loNr_bifilt.hw_ynrT_bifiltOut_alpha, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_nlmFilt_en = paut->dyn[inear].hiNr_filtProc.hw_ynrT_nlmFilt_en;
    out->dyn.hiNr_filtProc.hw_ynrT_localYnrScl_alpha = interpolation_f32(
                paut->dyn[ilow].hiNr_filtProc.hw_ynrT_localYnrScl_alpha, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_localYnrScl_alpha, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_nlmSgm_minLimit = interpolation_f32(
                paut->dyn[ilow].hiNr_filtProc.hw_ynrT_nlmSgm_minLimit, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_nlmSgm_minLimit, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_nlmSgm_scale = interpolation_f32(
                paut->dyn[ilow].hiNr_filtProc.hw_ynrT_nlmSgm_scale, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_nlmSgm_scale, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_nlmRgeWgt_negOff = interpolation_f32(
                paut->dyn[ilow].hiNr_filtProc.hw_ynrT_nlmRgeWgt_negOff, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_nlmRgeWgt_negOff, ratio);
    out->dyn.hiNr_filtProc.hw_ynrT_centerPix_wgt = interpolation_f32(
                paut->dyn[ilow].hiNr_filtProc.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].hiNr_filtProc.hw_ynrT_centerPix_wgt, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[i] = paut->dyn[inear].hiNr_filtProc.hw_ynrT_nlmSpatial_wgt[i];
    }
    out->dyn.hiNr_alphaProc.hw_ynrT_nlmOut_alpha = interpolation_f32(
                paut->dyn[ilow].hiNr_alphaProc.hw_ynrT_nlmOut_alpha, paut->dyn[ihigh].hiNr_alphaProc.hw_ynrT_nlmOut_alpha, ratio);
    out->dyn.hiNr_alphaProc.hw_ynrT_edgAlphaUp_thred = interpolation_f32(
                paut->dyn[ilow].hiNr_alphaProc.hw_ynrT_edgAlphaUp_thred, paut->dyn[ihigh].hiNr_alphaProc.hw_ynrT_edgAlphaUp_thred, ratio);
    out->dyn.hiNr_alphaProc.hw_ynrT_locSgmStrgAlphaUp_thred = interpolation_f32(
                paut->dyn[ilow].hiNr_alphaProc.hw_ynrT_locSgmStrgAlphaUp_thred, paut->dyn[ihigh].hiNr_alphaProc.hw_ynrT_locSgmStrgAlphaUp_thred, ratio);

    return XCAM_RETURN_NO_ERROR;
}

static void ynr_init_params_json_V24(ynr_api_attrib_t *attrib) {
    int i = 0;
    int j = 0;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;

    bit_proc = 10; // for V22, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue = (1 << (bit_calib - 4));
    isoCurveSectValue1 = (1 << bit_calib);

    for(j = 0; j < 13; j++) {
        ynr_params_dyn_t* pdyn = &(attrib->stAuto.dyn[j]);
        if(pdyn->sw_ynrCfg_sgmCurve_mode == ynr_cfgByCoeff2Curve_mode) {
            // get noise sigma sample data at [0, 64, 128, ... , 1024]
            for (i = 0; i < YNR_V22_ISO_CURVE_POINT_NUM; i++) {
                if (i == (YNR_V22_ISO_CURVE_POINT_NUM - 1)) {
                    ave1 = (float)isoCurveSectValue1;
                } else {
                    ave1 = (float)(i * isoCurveSectValue);
                }
                pdyn->hw_ynrC_luma2Sigma_curve.idx[i] = (short)ave1;
                ave2 = ave1 * ave1;
                ave3 = ave2 * ave1;
                ave4 = ave3 * ave1;
                pdyn->hw_ynrC_luma2Sigma_curve.val[i] = pdyn->coeff2SgmCurve.sigma_coeff[0] * ave4
                                                        + pdyn->coeff2SgmCurve.sigma_coeff[1] * ave3
                                                        + pdyn->coeff2SgmCurve.sigma_coeff[2] * ave2
                                                        + pdyn->coeff2SgmCurve.sigma_coeff[3] * ave1
                                                        + pdyn->coeff2SgmCurve.sigma_coeff[4];

                if (bit_shift > 0) {
                    pdyn->hw_ynrC_luma2Sigma_curve.idx[i] >>= bit_shift;
                } else {
                    pdyn->hw_ynrC_luma2Sigma_curve.idx[i] <<= ABS(bit_shift);
                }

                pdyn->hw_ynrC_luma2Sigma_curve.val[i] *= pdyn->coeff2SgmCurve.lowFreqCoeff;
                if (pdyn->hw_ynrC_luma2Sigma_curve.val[i] < 0.0) {
                    pdyn->hw_ynrC_luma2Sigma_curve.val[i] = 0.0;
                }
            }
        }
    }
}

XCamReturn YnrApplyStrength
(
    YnrContext_t *pYnrCtx,
    ynr_param_t* out)
{
    if(pYnrCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pYnrCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pYnrCtx->fStrength);

        ynr_params_dyn_t* pdyn = &out->dyn;
        pdyn->hiNr_filtProc.hw_ynrT_nlmSgm_scale *= fPercent;
        pdyn->hiNr_filtProc.hw_ynrT_centerPix_wgt /= fPercent;
        pdyn->hiNr_alphaProc.hw_ynrT_nlmOut_alpha *= fPercent;
        pdyn->loNr_bifilt.hw_ynrT_bifiltOut_alpha *= fPercent;
        pdyn->loNr_bifilt.hw_ynrT_rgeSgm_scale *= fPercent;
        LOGD_ANR("YnrApplyStrength: fStrength %f, fPercent %f\n", pYnrCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if defined(RKAIQ_HAVE_YNR_V40) || defined(RKAIQ_HAVE_YNR_V41)
XCamReturn YnrSelectParam
(
    YnrContext_t *pYnrCtx,
    ynr_param_t* out,
    int iso,
    bool is_aibnr_autorun,
    int aibnr_fixIndex)
{
    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    ynr_param_auto_t *paut = &pYnrCtx->ynr_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pre_interp(iso, pYnrCtx->iso_list, 13, &ilow, &ihigh, &ratio);
    if (aibnr_fixIndex != -1 && is_aibnr_autorun) {
        ratio = 0;
        ilow = aibnr_fixIndex;
        ihigh = aibnr_fixIndex;
    }
    uratio = ratio * (1 << RATIO_FIXBIT);
    LOGD_ANR("%s ilow %d, ihigh %d, ratio %f, aibnr_fixIndex %d, is_aibnr_autorun %d",
        __func__, ilow, ihigh,ratio, aibnr_fixIndex, is_aibnr_autorun);

    if (ratio > 0.5) {
        inear = ihigh;
    } else {
        inear = ilow;
    }

    out->dyn.locYnrStrg.radiDist.sw_ynrT_radiDist_en = paut->dyn[inear].locYnrStrg.radiDist.sw_ynrT_radiDist_en;
    for (i = 0; i < 17; i++) {
        out->dyn.locYnrStrg.radiDist.hw_ynrT_radiDist2YnrStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].locYnrStrg.radiDist.hw_ynrT_radiDist2YnrStrg_val[i], paut->dyn[ihigh].locYnrStrg.radiDist.hw_ynrT_radiDist2YnrStrg_val[i], ratio);
    }
    out->dyn.locYnrStrg.locSgmStrg.hw_ynrT_glbSgmStrg_val = interpolation_f32(
                paut->dyn[ilow].locYnrStrg.locSgmStrg.hw_ynrT_glbSgmStrg_val, paut->dyn[ihigh].locYnrStrg.locSgmStrg.hw_ynrT_glbSgmStrg_val, ratio);
    out->dyn.locYnrStrg.locSgmStrg.hw_ynrT_glbSgmStrg_alpha = interpolation_f32(
                paut->dyn[ilow].locYnrStrg.locSgmStrg.hw_ynrT_glbSgmStrg_alpha, paut->dyn[ihigh].locYnrStrg.locSgmStrg.hw_ynrT_glbSgmStrg_alpha, ratio);
    out->dyn.sigmaEnv.sw_ynrCfg_sgmCurve_mode = paut->dyn[inear].sigmaEnv.sw_ynrCfg_sgmCurve_mode;
    for (i = 0; i < 5; i++) {
        out->dyn.sigmaEnv.coeff2SgmCurve.sigma_coeff[i] = interpolation_f32(
                    paut->dyn[ilow].sigmaEnv.coeff2SgmCurve.sigma_coeff[i], paut->dyn[ihigh].sigmaEnv.coeff2SgmCurve.sigma_coeff[i], ratio);
    }
    out->dyn.sigmaEnv.coeff2SgmCurve.lowFreqCoeff = interpolation_f32(
                paut->dyn[ilow].sigmaEnv.coeff2SgmCurve.lowFreqCoeff, paut->dyn[ihigh].sigmaEnv.coeff2SgmCurve.lowFreqCoeff, ratio);
    for (i = 0; i < 17; i++) {
        out->dyn.sigmaEnv.hw_ynrC_luma2Sigma_curve.idx[i] = interpolation_u16(
                    paut->dyn[ilow].sigmaEnv.hw_ynrC_luma2Sigma_curve.idx[i], paut->dyn[ihigh].sigmaEnv.hw_ynrC_luma2Sigma_curve.idx[i], uratio);
    }
    for (i = 0; i < 17; i++) {
        out->dyn.sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i] = interpolation_f32(
                    paut->dyn[ilow].sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i], paut->dyn[ihigh].sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i], ratio);
    }
    out->dyn.hiNr.hw_ynrT_hiNr_en = paut->dyn[inear].hiNr.hw_ynrT_hiNr_en;
    out->dyn.hiNr.epf.sw_ynrT_filtCfg_mode = paut->dyn[inear].hiNr.epf.sw_ynrT_filtCfg_mode;
    out->dyn.hiNr.epf.sw_ynrT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].hiNr.epf.sw_ynrT_filtSpatial_strg, paut->dyn[ihigh].hiNr.epf.sw_ynrT_filtSpatial_strg, ratio);
    for (i = 0; i < 5; i++) {
        out->dyn.hiNr.epf.hw_ynrT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr.epf.hw_ynrT_filtSpatial_wgt[i], paut->dyn[ihigh].hiNr.epf.hw_ynrT_filtSpatial_wgt[i], uratio);
    }
    out->dyn.hiNr.epf.hw_ynrT_locYnrStrg_alpha = interpolation_f32(
                paut->dyn[ilow].hiNr.epf.hw_ynrT_locYnrStrg_alpha, paut->dyn[ihigh].hiNr.epf.hw_ynrT_locYnrStrg_alpha, ratio);
    out->dyn.hiNr.epf.hw_ynrT_rgeSgm_minLimit = interpolation_f32(
                paut->dyn[ilow].hiNr.epf.hw_ynrT_rgeSgm_minLimit, paut->dyn[ihigh].hiNr.epf.hw_ynrT_rgeSgm_minLimit, ratio);
    out->dyn.hiNr.epf.hw_ynrT_rgeSgm_scale = interpolation_f32(
                paut->dyn[ilow].hiNr.epf.hw_ynrT_rgeSgm_scale, paut->dyn[ihigh].hiNr.epf.hw_ynrT_rgeSgm_scale, ratio);
    out->dyn.hiNr.epf.hw_ynrT_rgeWgt_negOff = interpolation_f32(
                paut->dyn[ilow].hiNr.epf.hw_ynrT_rgeWgt_negOff, paut->dyn[ihigh].hiNr.epf.hw_ynrT_rgeWgt_negOff, ratio);
    out->dyn.hiNr.epf.hw_ynrT_centerPix_wgt = interpolation_f32(
                paut->dyn[ilow].hiNr.epf.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].hiNr.epf.hw_ynrT_centerPix_wgt, ratio);
    out->dyn.hiNr.sf.sw_ynrT_filtCfg_mode = paut->dyn[inear].hiNr.sf.sw_ynrT_filtCfg_mode;
    out->dyn.hiNr.sf.sw_ynrT_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].hiNr.sf.sw_ynrT_filtSpatial_strg, paut->dyn[ihigh].hiNr.sf.sw_ynrT_filtSpatial_strg, ratio);
    for (i = 0; i < 6; i++) {
        out->dyn.hiNr.sf.hw_ynrT_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].hiNr.sf.hw_ynrT_filtSpatial_wgt[i], paut->dyn[ihigh].hiNr.sf.hw_ynrT_filtSpatial_wgt[i], uratio);
    }
    out->dyn.hiNr.sfAlphaEpf_baseTex.hw_ynrT_maxAlphaTex_maxThred = interpolation_f32(
                paut->dyn[ilow].hiNr.sfAlphaEpf_baseTex.hw_ynrT_maxAlphaTex_maxThred, paut->dyn[ihigh].hiNr.sfAlphaEpf_baseTex.hw_ynrT_maxAlphaTex_maxThred, uratio);
    out->dyn.hiNr.sfAlphaEpf_baseTex.sw_ynr_texIdx_scale = interpolation_f32(
                paut->dyn[ilow].hiNr.sfAlphaEpf_baseTex.sw_ynr_texIdx_scale, paut->dyn[ihigh].hiNr.sfAlphaEpf_baseTex.sw_ynr_texIdx_scale, ratio);
    out->dyn.hiNr.sfAlphaEpf_baseTex.sw_ynr_sfAlpha_scale = interpolation_f32(
                paut->dyn[ilow].hiNr.sfAlphaEpf_baseTex.sw_ynr_sfAlpha_scale, paut->dyn[ihigh].hiNr.sfAlphaEpf_baseTex.sw_ynr_sfAlpha_scale, ratio);
    out->dyn.midLoNr.mf.hw_ynrT_midNr_en = paut->dyn[inear].midLoNr.mf.hw_ynrT_midNr_en;
    out->dyn.midLoNr.mf.sw_ynrT_filtCfg_mode = paut->dyn[inear].midLoNr.mf.sw_ynrT_filtCfg_mode;
    out->dyn.midLoNr.mf.sw_ynr_filtSpatial_strg = interpolation_f32(
                paut->dyn[ilow].midLoNr.mf.sw_ynr_filtSpatial_strg, paut->dyn[ihigh].midLoNr.mf.sw_ynr_filtSpatial_strg, ratio);
#if defined(RKAIQ_HAVE_YNR_V40)
    for (i = 0; i < 3; i++) {
        out->dyn.midLoNr.mf.sw_ynr_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].midLoNr.mf.sw_ynr_filtSpatial_wgt[i], paut->dyn[ihigh].midLoNr.mf.sw_ynr_filtSpatial_wgt[i], ratio);
    }
#endif
#if defined(RKAIQ_HAVE_YNR_V41)
    for (i = 0; i < 5; i++) {
        out->dyn.midLoNr.mf.sw_ynr_filtSpatial_wgt[i] = interpolation_f32(
                    paut->dyn[ilow].midLoNr.mf.sw_ynr_filtSpatial_wgt[i], paut->dyn[ihigh].midLoNr.mf.sw_ynr_filtSpatial_wgt[i], ratio);
    }
#endif
    out->dyn.midLoNr.mf.sw_ynr_rgeSgm_scale = interpolation_f32(
                paut->dyn[ilow].midLoNr.mf.sw_ynr_rgeSgm_scale, paut->dyn[ihigh].midLoNr.mf.sw_ynr_rgeSgm_scale, ratio);
    out->dyn.midLoNr.mf.sw_ynr_centerPix_wgt = interpolation_f32(
                paut->dyn[ilow].midLoNr.mf.sw_ynr_centerPix_wgt, paut->dyn[ihigh].midLoNr.mf.sw_ynr_centerPix_wgt, ratio);
    out->dyn.midLoNr.mf.hw_ynrT_softThd_scale = interpolation_f32(
                paut->dyn[ilow].midLoNr.mf.hw_ynrT_softThd_scale, paut->dyn[ihigh].midLoNr.mf.hw_ynrT_softThd_scale, ratio);
    out->dyn.midLoNr.mf.hw_ynrT_alphaMfTex_scale = interpolation_f32(
                paut->dyn[ilow].midLoNr.mf.hw_ynrT_alphaMfTex_scale, paut->dyn[ihigh].midLoNr.mf.hw_ynrT_alphaMfTex_scale, ratio);
    out->dyn.midLoNr.mf.hw_ynrT_midNrOut_alpha = interpolation_f32(
                paut->dyn[ilow].midLoNr.mf.hw_ynrT_midNrOut_alpha, paut->dyn[ihigh].midLoNr.mf.hw_ynrT_midNrOut_alpha, ratio);
    out->dyn.midLoNr.lf.hw_ynrT_loNr_en = paut->dyn[inear].midLoNr.lf.hw_ynrT_loNr_en;
    out->dyn.midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_tex2NrStrg_en = paut->dyn[inear].midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_tex2NrStrg_en;
    out->dyn.midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_flatRegion_maxThred = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_flatRegion_maxThred, paut->dyn[ihigh].midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_flatRegion_maxThred, ratio);
    out->dyn.midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_edgeRegion_minThred = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_edgeRegion_minThred, paut->dyn[ihigh].midLoNr.lf.locYnrStrg_texRegion.hw_ynrT_edgeRegion_minThred, ratio);
    out->dyn.midLoNr.lf.locYnrStrg_texRegion.sw_ynrT_edgeRegionNr_strg = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.locYnrStrg_texRegion.sw_ynrT_edgeRegionNr_strg, paut->dyn[ihigh].midLoNr.lf.locYnrStrg_texRegion.sw_ynrT_edgeRegionNr_strg, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrStrg_val[i] = interpolation_f32(
                    paut->dyn[ilow].midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrStrg_val[i], paut->dyn[ihigh].midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrStrg_val[i], ratio);
    }
    for (i = 0; i < 6; i++) {
        out->dyn.midLoNr.locNrStrg.hw_ynrT_luma2RgeSgm_scale[i] = interpolation_f32(
                    paut->dyn[ilow].midLoNr.locNrStrg.hw_ynrT_luma2RgeSgm_scale[i], paut->dyn[ihigh].midLoNr.locNrStrg.hw_ynrT_luma2RgeSgm_scale[i], ratio);
    }
    out->dyn.midLoNr.lf.epf.hw_ynrT_rgeSgm_scale = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.epf.hw_ynrT_rgeSgm_scale, paut->dyn[ihigh].midLoNr.lf.epf.hw_ynrT_rgeSgm_scale, ratio);
    out->dyn.midLoNr.lf.epf.hw_ynrT_guideSoftThd_scale = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.epf.hw_ynrT_guideSoftThd_scale, paut->dyn[ihigh].midLoNr.lf.epf.hw_ynrT_guideSoftThd_scale, ratio);
    out->dyn.midLoNr.lf.epf.hw_ynrT_centerPix_wgt = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.epf.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].midLoNr.lf.epf.hw_ynrT_centerPix_wgt, ratio);
    out->dyn.midLoNr.lf.epf.hw_ynrT_softThd_scale = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.epf.hw_ynrT_softThd_scale, paut->dyn[ihigh].midLoNr.lf.epf.hw_ynrT_softThd_scale, ratio);
    out->dyn.midLoNr.lf.epf.hw_ynrT_loNrOut_alpha = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.epf.hw_ynrT_loNrOut_alpha, paut->dyn[ihigh].midLoNr.lf.epf.hw_ynrT_loNrOut_alpha, ratio);
    for (i = 0; i < 9; i++) {
        out->dyn.midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrOut_alpha[i] = interpolation_f32(
                    paut->dyn[ilow].midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrOut_alpha[i], paut->dyn[ihigh].midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrOut_alpha[i], ratio);
    }

#if defined(RKAIQ_HAVE_YNR_V41)
    out->dyn.midLoNr.hw_ynrT_tex2NrOutAlpha_en = paut->dyn[inear].midLoNr.hw_ynrT_tex2NrOutAlpha_en;
    for (i = 0; i < 9; i++) {
        out->dyn.midLoNr.mf.tex2NrOutAlpha[i] = interpolation_f32(
                    paut->dyn[ilow].midLoNr.mf.tex2NrOutAlpha[i], paut->dyn[ihigh].midLoNr.mf.tex2NrOutAlpha[i], ratio);
    }
    for (i = 0; i < 9; i++) {
        out->dyn.midLoNr.lf.epf.tex2NrOutAlpha[i] = interpolation_f32(
                    paut->dyn[ilow].midLoNr.lf.epf.tex2NrOutAlpha[i], paut->dyn[ihigh].midLoNr.lf.epf.tex2NrOutAlpha[i], ratio);
    }
    out->dyn.midLoNr.lf.epf.hw_ynrT_alphaLfTex_scale = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.epf.hw_ynrT_alphaLfTex_scale, paut->dyn[ihigh].midLoNr.lf.epf.hw_ynrT_alphaLfTex_scale, ratio);

    out->dyn.midLoNr.lf.dsFlt.hw_ynrT_dsFlt_en = paut->dyn[inear].midLoNr.lf.dsFlt.hw_ynrT_dsFlt_en;
    out->dyn.midLoNr.lf.dsFlt.hw_ynrT_dsFlt_strg = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.dsFlt.hw_ynrT_dsFlt_strg, paut->dyn[ihigh].midLoNr.lf.dsFlt.hw_ynrT_dsFlt_strg, ratio);
    out->dyn.midLoNr.lf.dsFlt.hw_ynrT_diff_offset = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.dsFlt.hw_ynrT_diff_offset, paut->dyn[ihigh].midLoNr.lf.dsFlt.hw_ynrT_diff_offset, ratio);
    out->dyn.midLoNr.lf.dsFlt.hw_ynrT_centerPix_wgt = interpolation_f32(
                paut->dyn[ilow].midLoNr.lf.dsFlt.hw_ynrT_centerPix_wgt, paut->dyn[ihigh].midLoNr.lf.dsFlt.hw_ynrT_centerPix_wgt, ratio);

#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn YnrApplyStrength
(
    YnrContext_t *pYnrCtx,
    ynr_param_t* out)
{
    int i = 0;

    if(pYnrCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pYnrCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pYnrCtx->fStrength);

        ynr_params_dyn_t* pdyn = &out->dyn;
        pdyn->hiNr.epf.hw_ynrT_rgeSgm_scale *= fPercent;
        pdyn->hiNr.epf.hw_ynrT_centerPix_wgt /= fPercent;

        pdyn->midLoNr.mf.sw_ynr_rgeSgm_scale *= fPercent;
        pdyn->midLoNr.mf.sw_ynr_centerPix_wgt /= fPercent;
        pdyn->midLoNr.mf.hw_ynrT_midNrOut_alpha *= fPercent;


        pdyn->midLoNr.lf.epf.hw_ynrT_rgeSgm_scale *= fPercent;
        for (i = 0; i < 6; i++) {
            pdyn->midLoNr.locNrStrg.hw_ynrT_luma2RgeSgm_scale[i] *= fPercent;
        }
        pdyn->midLoNr.lf.epf.hw_ynrT_loNrOut_alpha *= fPercent;
        pdyn->midLoNr.lf.epf.hw_ynrT_centerPix_wgt /= fPercent;
        for (i = 0; i < 9; i++) {
            pdyn->midLoNr.locNrStrg.hw_ynrT_locSgmStrg2NrOut_alpha[i] *= fPercent;
        }
        LOGI_ANR("YnrApplyStrength: fStrength %f, fPercent %f\n", pYnrCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}

#define YNR_V30_ISO_CURVE_POINT_BIT          4
#define YNR_V30_ISO_CURVE_POINT_NUM          ((1 << YNR_V30_ISO_CURVE_POINT_BIT)+1)

static void ynr_init_params_json_V40(ynr_api_attrib_t *attrib) {
    int i, j, k;
    short isoCurveSectValue;
    short isoCurveSectValue1;
    float ave1, ave2, ave3, ave4;
    int bit_calib = 12;
    int bit_proc;
    int bit_shift;
    float tmpYnrCurve[5];
    float* pYnrCurve;

    bit_proc = 10; // for V22, YNR_SIGMA_BITS = 10
    bit_shift = bit_calib - bit_proc;

    isoCurveSectValue = (1 << (bit_calib - 4));
    isoCurveSectValue1 = (1 << bit_calib);

    // read noise sigma curve data and ci
    for (j = 0; j < 13; j++) {
        ynr_params_dyn_t* pdyn = &(attrib->stAuto.dyn[j]);
        if(pdyn->sigmaEnv.sw_ynrCfg_sgmCurve_mode == ynr_cfgByCoeff2Curve_mode) {
            for (k = 0; k < 5; k++) {
                tmpYnrCurve[k] = (float)pdyn->sigmaEnv.coeff2SgmCurve.sigma_coeff[k];
            }

            pYnrCurve = tmpYnrCurve;

            // get noise sigma sample data at [0, 64, 128, ... , 1024]
            for (i = 0; i < YNR_V30_ISO_CURVE_POINT_NUM; i++) {
                if (i == (YNR_V30_ISO_CURVE_POINT_NUM - 1)) {
                    ave1 = (float)isoCurveSectValue1;
                }
                else {
                    ave1 = (float)(i * isoCurveSectValue);
                }
                pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.idx[i] = (short)ave1;
                ave2 = ave1 * ave1;
                ave3 = ave2 * ave1;
                ave4 = ave3 * ave1;
                pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i] = pYnrCurve[0] * ave4 + pYnrCurve[1] * ave3 + pYnrCurve[2] * ave2 + pYnrCurve[3] * ave1 + pYnrCurve[4];

                pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i] *= pdyn->sigmaEnv.coeff2SgmCurve.lowFreqCoeff;
                if(pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i]  < 0.0) {
                    pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i]  = 0.0;
                }

                if (bit_shift > 0) {
                    pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.idx[i] >>= bit_shift;
                } else {
                    pdyn->sigmaEnv.hw_ynrC_luma2Sigma_curve.idx[i] <<= ABS(bit_shift);
                }
            }
        }
    } //  read noise sigma curve data and ci

}

#endif

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    YnrContext_t* ctx = aiq_mallocz(sizeof(YnrContext_t));

    if (ctx == NULL) {
        LOGE_ANR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->init_json = true;
    ctx->ynr_attrib =
        (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, ynr));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ANR("%s: Ynr (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    YnrContext_t* pYnrCtx = (YnrContext_t*)context;
    aiq_free(pYnrCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    YnrContext_t* pYnrCtx = (YnrContext_t *)params->ctx;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pYnrCtx->ynr_attrib =
                (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ynr));
            pYnrCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pYnrCtx->working_mode = params->u.prepare.working_mode;
    pYnrCtx->ynr_attrib =
        (ynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ynr));
    pYnrCtx->iso_list = params->u.prepare.calibv2->sensor_info->iso_list;
    pYnrCtx->isReCal_ = true;
    pYnrCtx->init_json = true;

    return result;
}

XCamReturn Aynr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    YnrContext_t* pYnrCtx = (YnrContext_t *)inparams->ctx;
    ynr_api_attrib_t* ynr_attrib = pYnrCtx->ynr_attrib;
    ynr_param_t* ynr_res = outparams->algoRes;
    bool is_aibnr_autorun = inparams->u.proc.is_aibnr_autorun;
    int aibnr_fixIndex = inparams->u.proc.aibnr_fixIndex;
    bool is_aibnr_force_update = inparams->u.proc.is_aibnr_force_update;

    if (ynr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", ynr_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }
    if (ynr_attrib->en == false || ynr_attrib->bypass == true) {
        if (inparams->u.proc.is_attrib_update) {
            outparams->cfg_update = true;
            outparams->en = ynr_attrib->en;
            outparams->bypass = ynr_attrib->bypass;
        }
        LOGD_ANR("YNR not enable, skip!");
        return XCAM_RETURN_NO_ERROR;
    }
    bool need_recal = pYnrCtx->isReCal_;

    bool init = inparams->u.proc.init;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init || is_aibnr_force_update) {
        need_recal = true;
        pYnrCtx->init_json = true;
    }

    if (pYnrCtx->init_json) {
#if RKAIQ_HAVE_YNR_V24
        ynr_init_params_json_V24(pYnrCtx->ynr_attrib);
#endif
#if defined(RKAIQ_HAVE_YNR_V40) || defined(RKAIQ_HAVE_YNR_V41)
        ynr_init_params_json_V40(pYnrCtx->ynr_attrib);
#endif
        pYnrCtx->init_json = false;
    }

    int delta_iso = abs(iso - pYnrCtx->pre_iso);
    if(delta_iso > 0.01 || init) {
        pYnrCtx->pre_iso = iso;
        need_recal = true;
    }

    outparams->cfg_update = false;
    if (need_recal) {
        ynr_res->sta = pYnrCtx->ynr_attrib->stAuto.sta;
#if RKAIQ_HAVE_YNR_V24
        YnrSelectParam(pYnrCtx, ynr_res, iso);
        YnrApplyStrength(pYnrCtx, ynr_res);
#endif
#if defined(RKAIQ_HAVE_YNR_V40) || defined(RKAIQ_HAVE_YNR_V41)
        YnrSelectParam(pYnrCtx, ynr_res, iso, is_aibnr_autorun, aibnr_fixIndex);
        YnrApplyStrength(pYnrCtx, ynr_res);
#endif
        outparams->cfg_update = true;
        outparams->en = ynr_attrib->en;
        outparams->bypass = ynr_attrib->bypass;
        LOGD_ANR("YNR processing: iso %d, ynr en:%d, bypass:%d", iso, outparams->en, outparams->bypass);

        pYnrCtx->isReCal_ = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = inparams->u.proc.iso;
    Aynr_processing(inparams, outparams, iso);

    LOGV_ANR("%s: Ynr (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_ynr_SetAttrib(RkAiqAlgoContext *ctx, const ynr_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    YnrContext_t* pYnrCtx = (YnrContext_t*)ctx;
    pYnrCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ynr_GetAttrib(const RkAiqAlgoContext *ctx, ynr_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    YnrContext_t* pYnrCtx = (YnrContext_t*)ctx;
    ynr_api_attrib_t* ynr_attrib = pYnrCtx->ynr_attrib;

    attr->opMode = ynr_attrib->opMode;
    attr->en = ynr_attrib->en;
    attr->bypass = ynr_attrib->bypass;
    memcpy(&attr->stAuto, &ynr_attrib->stAuto, sizeof(ynr_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn
algo_ynr_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    YnrContext_t* pYnrCtx = (YnrContext_t*)ctx;

    pYnrCtx->fStrength = strg;
    pYnrCtx->strength_en = strg_en;
    pYnrCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ynr_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    YnrContext_t* pYnrCtx = (YnrContext_t*)ctx;

    *strg = pYnrCtx->fStrength;
    *strg_en = pYnrCtx->strength_en;

    return XCAM_RETURN_NO_ERROR;
}


#define RKISP_ALGO_YNR_VERSION     "v0.0.9"
#define RKISP_ALGO_YNR_VENDOR      "Rockchip"
#define RKISP_ALGO_YNR_DESCRIPTION "Rockchip ynr algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescYnr = {
    .common = {
        .version = RKISP_ALGO_YNR_VERSION,
        .vendor  = RKISP_ALGO_YNR_VENDOR,
        .description = RKISP_ALGO_YNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AYNR,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

