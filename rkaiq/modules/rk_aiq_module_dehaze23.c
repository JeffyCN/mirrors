/*
 * rk_aiq_module_dehaze23.c
 *
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */

#include "rk_aiq_isp39_modules.h"
#include "algo_types_priv.h"

RKAIQ_BEGIN_DECLARE

#define DEHAZE_GAUS_H0                              (2)
#define DEHAZE_GAUS_H1                              (4)
#define DEHAZE_GAUS_H2                              (2)
#define DEHAZE_GAUS_H3                              (4)
#define DEHAZE_GAUS_H4                              (8)
#define DEHAZE_GAUS_H5                              (4)
#define DEHAZE_GAUS_H6                              (2)
#define DEHAZE_GAUS_H7                              (4)
#define DEHAZE_GAUS_H8                              (2)
#define LIMIT_VALUE_UNSIGNED(value, max_value) (value > max_value ? max_value : value)

static int ClipFloatValue(float posx, int BitInt, int BitFloat) {
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    yOutInt = CLIP((int)(posx * pow(2, BitFloat)), yOutIntMin, yOutIntMax);

    return yOutInt;
}

unsigned int ClipIntValue(unsigned int posx, int BitInt, int BitFloat) {
    unsigned int yOutInt    = 0;
    unsigned int yOutIntMax = (1 << (BitFloat + BitInt)) - 1;

    posx <<= BitFloat;
    yOutInt = posx > yOutIntMax ? yOutIntMax : posx;
    return yOutInt;
}


void rk_aiq_dehaze23_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
    struct isp39_dhaz_cfg *pFix = &isp_params->isp_cfg->others.dhaz_cfg;
    dehaze_param_t* dehaze_param = (dehaze_param_t *) attr;
    dehaze_params_dyn_t *pdyn = &dehaze_param->dyn;

    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;

    pFix->mem_mode        = 0;
    pFix->mem_force       = 0;
    pFix->round_en        = 1;
    pFix->gaus_h0         = DEHAZE_GAUS_H4;
    pFix->gaus_h1         = DEHAZE_GAUS_H1;
    pFix->gaus_h2         = DEHAZE_GAUS_H0;
    pFix->dc_en           = cvtinfo->dehaze_en;

    if (pdyn->sw_dhazT_work_mode == dhaz_dehaze_mode) {
        pFix->enhance_en      = 0;
    } else {
        pFix->enhance_en      = 1;
    }

    pFix->air_lc_en = pdyn->dehaze_airLight.hw_dhazT_airBaseLimit_en;
    pFix->stab_fnum = pdyn->iir.hw_dhazT_iirFrm_maxLimit;

    if (pdyn->iir.hw_dhazT_iirWgtMaxSgm_val >= 0.0f)
        pFix->iir_wt_sigma = (unsigned short)(CLIP(
            (int)(1024.0f / (8.0f * pdyn->iir.hw_dhazT_iirWgtMaxSgm_val + 0.5f)), 0, 0x7ff));
    else
        pFix->iir_wt_sigma = 0x7ff;
    if (pdyn->iir.hw_dhazT_iirAirLightSgm_val)
        pFix->iir_air_sigma =
            (unsigned char)(CLIP((int)(1024.0f / pdyn->iir.hw_dhazT_iirAirLightSgm_val), 0, 255));
    else
        pFix->iir_air_sigma = 0x8;
    if (pdyn->iir.hw_dhazT_iirTransRatMinSgm_val)
        pFix->iir_tmax_sigma = (unsigned short)(CLIP(
            (int)(1.0f / pdyn->iir.hw_dhazT_iirTransRatMinSgm_val), 0, 0x7ff));
    else
        pFix->iir_tmax_sigma = 0x5f;

    pFix->enh_luma_en      = pdyn->enhance.hw_dhazT_luma2strg_en;
    pFix->color_deviate_en = pdyn->enhance.hw_dhazT_cProtect_en;

    pFix->yblk_th         = pdyn->dehaze_wgtMax.hw_dhazT_lumaCount_minRatio *
        ((rows + 15) / 16) * ((cols + 15) / 16);
    pFix->yhist_th        = pdyn->dehaze_wgtMax.hw_dhazT_lumaCount_maxThred;
    pFix->dark_th         = pdyn->dehaze_wgtMax.hw_dhazT_darkArea_thred;
    pFix->dc_max_th       = ClipIntValue(pdyn->dehaze_wgtMax.hw_dhazT_darkCh_maxThred, 8, 0);
    pFix->dc_min_th       = ClipIntValue(pdyn->dehaze_wgtMax.hw_dhazT_darkCh_minThred, 8, 0);
    pFix->wt_max          = ClipFloatValue(pdyn->dehaze_wgtMax.hw_dhazT_wgtMax_scale, 0, 8);
    pFix->cfg_wt          = ClipFloatValue(pdyn->dehaze_wgtMax.hw_dhazT_userWgtMax_val, 0, 8);
    pFix->bright_max      = pdyn->dehaze_airLight.hw_dhazT_bright_maxLimit;
    pFix->bright_min      = pdyn->dehaze_airLight.hw_dhazT_bright_minLimit;
    pFix->air_max         = pdyn->dehaze_airLight.hw_dhazT_airLight_maxLimit;
    pFix->air_min         = pdyn->dehaze_airLight.hw_dhazT_airLight_minLimit;
    pFix->cfg_air         = pdyn->dehaze_airLight.hw_dhazT_userAirLight_val;
    pFix->tmax_base       = pdyn->dehaze_transRatMin.hw_dhazT_transRatMin_scale;
    pFix->tmax_max        = ClipFloatValue(pdyn->dehaze_transRatMin.hw_dhazT_transRatMin_maxLimit, 0, 10);
    pFix->tmax_off        = ClipFloatValue(pdyn->dehaze_transRatMin.hw_dhazT_transRatMin_offset, 0, 10);
    pFix->cfg_tmax        = ClipFloatValue(pdyn->dehaze_transRatMin.hw_dhazT_userTransRatMin_val, 0, 10);
    pFix->cfg_alpha       = CLIP(pdyn->hw_dhazT_paramMerge_alpha * 255, 0, 255);

    pFix->range_sima      = ClipFloatValue(pdyn->bifilt.hw_dhazT_rgeSgm_val, 0, 8);
    pFix->space_sigma_cur = ClipFloatValue(pdyn->bifilt.hw_dhazT_curSpatialSgm_val, 0, 8);
    pFix->space_sigma_pre = ClipFloatValue(pdyn->bifilt.hw_dhazT_preSpatialSgm_val, 0, 8);
    pFix->dc_weitcur      = ClipFloatValue(pdyn->bifilt.hw_dhazT_bifilt_wgt, 0, 8);
    pFix->bf_weight       = ClipFloatValue(pdyn->bifilt.hw_dhazT_bifilt_alpha, 0, 8);
    pFix->iir_pre_wet     = ClipIntValue(pdyn->bifilt.hw_dhazT_iirFilt_wgt, 4, 0);

    pFix->enhance_value   = ClipFloatValue(pdyn->enhance.hw_dhazT_contrast_strg, 4, 10);
    pFix->enhance_chroma  = ClipFloatValue(pdyn->enhance.hw_dhazT_saturate_strg, 4, 10);

    for (int i = 0; i < ISP39_DHAZ_ENH_LUMA_NUM; i++)
        pFix->enh_luma[i] = ClipFloatValue(pdyn->enhance.hw_dhazT_luma2strg_val[i], 4, 6);
    for (int i = 0; i < ISP39_DHAZ_ENH_CURVE_NUM; i++)
        pFix->enh_curve[i] = (unsigned short)pdyn->hw_dhazT_loLumaConvert_val[i];

    //applyStrth(dehaze_attr, isp_cfg);
    return;
}

#define HSIT_BLOCK_HEIGHT_MIN                       (64)
#define HIST_BLOCK_HEIGHT_MAX                       (876)
#define HSIT_BLOCK_WIDTH_MIN                        (66)
#define HIST_BLOCK_WIDTH_MAX                        (1168)
#define YNR_CURVE_STEP             (16)

void rk_aiq_histeq23_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
    struct isp39_dhaz_cfg *pFix = &isp_params->isp_cfg->others.dhaz_cfg;
    histeq_param_t* histeq_attrib = (histeq_param_t*) attr;
    histeq_params_dyn_t *pdyn = &histeq_attrib->dyn;
    histeq_params_static_t *psta = &histeq_attrib->sta;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;

    // sw_hist_MapTflt_invSigma
    if (psta->sw_hist_MapTflt_invSigma)
        pFix->iir_sigma =
            LIMIT_VALUE((int)(256.0f / psta->sw_hist_MapTflt_invSigma), 255, 0);
    else
        pFix->iir_sigma = 0x1;

    // map_mode
    pFix->hist_en  = cvtinfo->histeq_en;
    pFix->map_mode = psta->hw_hist_imgMap_mode;
    // thumb_col
    pFix->thumb_col = psta->hw_histc_blocks_cols;
    // thumb_row
    pFix->thumb_row = psta->hw_histc_blocks_rows;
    // check thumb_col
    if (pFix->thumb_col % 2) {
        pFix->thumb_col = (uint8_t)(pFix->thumb_col / 2) * 2;
    }
    if (cols >= 4352 && pFix->thumb_col <= 4) {
        pFix->thumb_col = 6;
    } else if (pFix->thumb_col < 4) {
        pFix->thumb_col = 4;
    } else if (pFix->thumb_col > 10) {
        pFix->thumb_col = 10;
    }
    // check thumb_row
    if (pFix->thumb_row % 2) {
        pFix->thumb_row = (uint8_t)(pFix->thumb_row / 2) * 2;
    }
    if (rows > 6132) {
        pFix->thumb_row = 8;
    } else if (rows > 4088 && rows <= 6132 && pFix->thumb_row <= 4) {
        pFix->thumb_row = 6;
    } else if (pFix->thumb_row < 4) {
        pFix->thumb_row = 4;
    } else if (pFix->thumb_row > 8) {
        pFix->thumb_row = 8;
    }
    // blk_het
    pFix->blk_het = LIMIT_VALUE(rows / pFix->thumb_row, HIST_BLOCK_HEIGHT_MAX, HSIT_BLOCK_HEIGHT_MIN);
    // blk_wid
    pFix->blk_wid = LIMIT_VALUE(cols / pFix->thumb_col, HIST_BLOCK_WIDTH_MAX, HSIT_BLOCK_WIDTH_MIN);

    // hist_th_off
    pFix->hist_th_off = ClipIntValue(pdyn->hw_histc_noiseCount_offset, 8, 0);
    // hist_k
    pFix->hist_k = ClipFloatValue(pdyn->sw_histc_noiseCount_scale, 3, 2);
    // hist_min
    pFix->hist_min = ClipFloatValue(pdyn->sw_histc_countWgt_minLimit, 1, 8);
    pFix->hist_min = LIMIT_VALUE_UNSIGNED(pFix->hist_min, BIT_8_MAX + 1);
    // cfg_k
    pFix->cfg_k = ClipFloatValue(pdyn->sw_hist_mapUserSet, 1, 8);
    pFix->cfg_k = LIMIT_VALUE_UNSIGNED(pFix->cfg_k, BIT_8_MAX + 1);
    // cfg_k_alpha
    pFix->cfg_k_alpha = ClipFloatValue(pdyn->sw_hist_mapMerge_alpha, 1, 8);
    pFix->cfg_k_alpha = LIMIT_VALUE_UNSIGNED(pFix->cfg_k_alpha, BIT_8_MAX + 1);
    // k_gain
    pFix->k_gain = ClipFloatValue(pdyn->sw_hist_mapCount_scale, 1, 8);
    pFix->k_gain = LIMIT_VALUE_UNSIGNED(pFix->k_gain, 0x100);
    // blend_wet
    for (int j = 0; j < DHAZ_V14_BLEND_WET_NUM; j++) {
        int tmp_int = ClipFloatValue(pdyn->sw_dehaze_outputMerge_alpha[j], 1, 8);
        pFix->blend_wet[j] = LIMIT_VALUE_UNSIGNED(tmp_int, BIT_8_MAX + 1);
    }
}

void rk_aiq_dehazeHisteq23_sigma_params_cvt(isp_params_t* isp_params,
                                            common_cvt_info_t* cvtinfo) {
    struct isp39_dhaz_cfg* pFix = &isp_params->isp_cfg->others.dhaz_cfg;

    // get sigma_idx
    for (int i = 0; i < HIST_SIGMA_IDX_NUM; i++) pFix->sigma_idx[i] = (i + 1) * YNR_CURVE_STEP;
    // get sigma_lut
    float sigam_total = 0.0f;
    for (int i = 0; i < HIST_SIGMA_LUT_NUM; i++) sigam_total += cvtinfo->ynr_sigma[i];
    if (sigam_total < FLT_EPSILON) {
        for (int i = 0; i < HIST_SIGMA_LUT_NUM; i++) pFix->sigma_lut[i] = 0x200;
    } else {
        for (int i = 0; i < HIST_SIGMA_LUT_NUM; i++) {
            pFix->sigma_lut[i] = CLIP(cvtinfo->ynr_sigma[i] * 8, 0, 1023);
        }
    }
}

RKAIQ_END_DECLARE
