/*
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

RKAIQ_BEGIN_DECLARE

static int ClipValue(float posx, int BitInt, int BitFloat) {
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

void rk_aiq_dehaze23_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg)
{
    struct isp39_dhaz_cfg *phwcfg = &isp_cfg->others.dhaz_cfg;
    dehaze_param_t* dehaze_attrib = (dehaze_param_t*) attr;
    dehaze_params_dyn_t *pdyn = &dehaze_attrib->dyn;
    dehaze_params_static_t *psta = &dehaze_attrib->sta;

    phwcfg->mem_mode        = psta->mem_mode;
    phwcfg->mem_force       = psta->mem_force;
    phwcfg->round_en        = psta->round_en;
    phwcfg->soft_wr_en      = psta->soft_wr_en;
    phwcfg->enhance_en      = psta->enhance_en;
    phwcfg->hist_en         = psta->hist_en;
    phwcfg->dc_en           = psta->dc_en;

    phwcfg->air_lc_en = psta->hw_dehaze_setting.hw_dehaze_luma_mode;
    if (psta->hw_hist_setting.sw_hist_MapTflt_invSigma)
        phwcfg->iir_sigma = (unsigned char)(CLIP(
            int(256.0f / psta->hw_hist_setting.sw_hist_MapTflt_invSigma), 0, 255));
    else
        phwcfg->iir_sigma = 0x1;
    if (psta->hw_dehaze_setting.sw_dhaz_invContrastTflt_invSgm >= 0.0f)
        phwcfg->iir_wt_sigma = (unsigned short)(CLIP(
            int(1024.0f / (8.0f * psta->hw_dehaze_setting.sw_dhaz_invContrastTflt_invSgm + 0.5f)), 0, 0x7ff));
    else
        phwcfg->iir_wt_sigma = 0x7ff;
    if (psta->hw_dehaze_setting.sw_dhaz_airLightTflt_invSgm)
        phwcfg->iir_air_sigma = (unsigned char)(CLIP(
            int(1024.0f / psta->hw_dehaze_setting.sw_dhaz_airLightTflt_invSgm), 0, 255));
    else
        phwcfg->iir_air_sigma = 0x8;
    if (psta->hw_dehaze_setting.sw_dhaz_transRatioTflt_invSgm)
        phwcfg->iir_tmax_sigma = (unsigned short)(CLIP(
            int(1.0f / psta->hw_dehaze_setting.sw_dhaz_transRatioTflt_invSgm), 0, 0x7ff));
    else
        phwcfg->iir_tmax_sigma = 0x5f;

    phwcfg->enh_luma_en = psta->hw_enhance_setting.hw_enhance_luma2strg_en;
    phwcfg->color_deviate_en = psta->hw_enhance_setting.hw_enhance_cProtect_en;

    phwcfg->yblk_th         = pdyn->hw_dehaze_params.sw_dehaze_lumaCount_minRatio;
    phwcfg->yhist_th        = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_lumaCount_maxThed, 8, 0);
    phwcfg->dc_max_th       = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_darkCh_maxThed, 8, 0);
    phwcfg->dc_min_th       = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_darkCh_minThed, 8, 0);
    phwcfg->wt_max          = ClipValue(pdyn->hw_dehaze_params.sw_dehaze_invContrast_scale, 0, 8);
    phwcfg->bright_max      = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_bright_maxLimit, 8, 0);
    phwcfg->bright_min      = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_bright_minLimit, 8, 0);
    phwcfg->tmax_base       = ClipValue(pdyn->hw_dehaze_params.sw_dehaze_airLight_scale, 8, 0);
    phwcfg->dark_th         = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_darkArea_thed, 8, 0);
    phwcfg->air_max         = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_airLight_maxLimit, 8, 0);
    phwcfg->air_min         = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_airLight_minLimit, 8, 0);
    phwcfg->tmax_max        = ClipValue(pdyn->hw_dehaze_params.sw_dehaze_transRatio_maxLimit, 0, 10);
    phwcfg->tmax_off        = ClipValue(pdyn->hw_dehaze_params.sw_dehaze_transRatio_offset, 0, 10);
    phwcfg->cfg_wt          = ClipValue(pdyn->hw_dehaze_params.sw_dehaze_userInvContrast, 0, 8);
    phwcfg->cfg_air         = ClipValue(pdyn->hw_dehaze_params.hw_dehaze_userAirLight, 8, 0);
    phwcfg->cfg_tmax        = ClipValue(pdyn->hw_dehaze_params.sw_dehaze_userTransRatio, 0, 10);
    phwcfg->cfg_alpha       = CLIP(pdyn->hw_dehaze_params.sw_dehaze_paramMerge_alpha, 0, 255);

    phwcfg->range_sima      = ClipValue(pdyn->hw_commom_setting.hw_contrast_ThumbFlt_invVsigma, 0, 8);
    phwcfg->space_sigma_cur = ClipValue(pdyn->hw_commom_setting.hw_contrast_curThumbFlt_invRsgm, 0, 8);
    phwcfg->space_sigma_pre = ClipValue(pdyn->hw_commom_setting.hw_contrast_preThumbFlt_invRsgm, 0, 8);
    phwcfg->dc_weitcur      = ClipValue(pdyn->hw_commom_setting.hw_contrast_thumbFlt_curWgt, 0, 8);
    phwcfg->bf_weight       = ClipValue(pdyn->hw_commom_setting.hw_contrast_thumbMerge_wgt, 0, 8);
    float tmp = pdyn->hw_commom_setting.hw_dehaze_user_gainFuse * 256;
    phwcfg->gain_fuse_alpha = tmp > 256 ? 256 : tmp;
    phwcfg->iir_pre_wet =
        ClipIntValue(pdyn->hw_commom_setting.sw_contrast_thumbTflt_curWgt, 4, 0);
    phwcfg->stab_fnum = ClipIntValue(pdyn->hw_commom_setting.sw_contrast_paramTflt_curWgt, 5, 0);
    phwcfg->gaus_h0         = psta->gaus_h0;
    phwcfg->gaus_h1         = psta->gaus_h1;
    phwcfg->gaus_h2         = psta->gaus_h2;

    phwcfg->enhance_value   = ClipValue(pdyn->hw_enhance_params.sw_enhance_contrast_strg, 4, 10);
    phwcfg->enhance_chroma  = ClipValue(pdyn->hw_enhance_params.sw_enhance_saturate_strg, 4, 10);
    for (int i = 0; i < ISP39_DHAZ_ENH_LUMA_NUM; i++)
        phwcfg->enh_luma[i] = ClipValue(pdyn->hw_enhance_params.sw_enhance_luma2strg_val[i], 4, 6);
    for (int i = 0; i < ISP39_DHAZ_ENH_CURVE_NUM; i++)
        phwcfg->enh_curve[i] = (unsigned short)pdyn->hw_enhance_params.hw_enhance_loLumaConvert_val[i];

    phwcfg->map_mode        = psta->hw_hist_setting.hw_hist_imgMap_mode;
    phwcfg->blk_het         = psta->hw_hist_setting.blk_het;
    phwcfg->blk_wid         = psta->hw_hist_setting.blk_wid;
    phwcfg->thumb_row       = psta->hw_hist_setting.hw_histc_blocks_rows;
    phwcfg->thumb_col       = psta->hw_hist_setting.hw_histc_blocks_cols;
    phwcfg->hist_k          = ClipValue(pdyn->hw_hist_params.sw_histc_noiseCount_scale, 3, 2);
    phwcfg->hist_th_off     = ClipIntValue(pdyn->hw_hist_params.hw_histc_noiseCount_offset, 8, 0);
    phwcfg->hist_min        = ClipValue(pdyn->hw_hist_params.sw_histc_countWgt_minLimit, 1, 8);
    phwcfg->cfg_k_alpha     = ClipValue(pdyn->hw_hist_params.sw_hist_mapMerge_alpha, 1, 8);
    phwcfg->cfg_k      = ClipValue(pdyn->hw_hist_params.sw_hist_mapUserSet, 1, 8);
    phwcfg->k_gain      = ClipValue(pdyn->hw_hist_params.sw_hist_mapCount_scale, 3, 8);

    for (int i = 0; i < ISP39_DHAZ_SIGMA_IDX_NUM; i++)
        phwcfg->sigma_idx[i] = psta->sigma_idx[i];
    for (int i = 0; i < ISP39_DHAZ_SIGMA_LUT_NUM; i++)
        phwcfg->sigma_lut[i] = psta->sigma_lut[i];
    for (int i = 0; i < ISP39_DHAZ_BLEND_WET_NUM; i++) {
        int tmp_int = ClipValue(pdyn->hw_hist_params.sw_dehaze_outputMerge_alpha[i], 1, 8);
        phwcfg->blend_wet[i] = tmp_int > 256 ? 256 : tmp_int;
    }
        
    return;
}

RKAIQ_END_DECLARE
