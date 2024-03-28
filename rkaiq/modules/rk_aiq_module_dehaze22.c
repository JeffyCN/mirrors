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

#include "rk_aiq_isp32_modules.h"

RKAIQ_BEGIN_DECLARE

static int ClipValue(float posx, int BitInt, int BitFloat) {
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    yOutInt = CLIP((int)(posx * pow(2, BitFloat)), yOutIntMin, yOutIntMax);

    return yOutInt;
}

void rk_aiq_dehaze22_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg)
{
    int tmp;
    struct isp32_dhaz_cfg* phwcfg = &isp_cfg->others.dhaz_cfg;
    dehaze_param_t* dehaze_attrib = (dehaze_param_t*) attr;
    dehaze_params_static_t* psta = &dehaze_attrib->sta;
    dehaze_params_dyn_t* pdyn = &dehaze_attrib->dyn;

    // printf("pdyn->cfg_alpha:%f\n", pdyn->cfg_alpha);
    phwcfg->round_en        = psta->round_en;
    phwcfg->soft_wr_en      = psta->soft_wr_en;
    phwcfg->enhance_en      = psta->enhance_en;
    phwcfg->hist_en         = psta->hist_en;
    phwcfg->dc_en           = psta->dc_en;
    phwcfg->cfg_alpha = CLIP((int)(psta->cfg_alpha * (1 << 8)), 0, 255);
    phwcfg->air_lc_en = psta->dehaze_setting.air_lc_en;
    if (psta->dehaze_setting.iir_sigma)
        phwcfg->iir_sigma = (unsigned char)(CLIP(
            int(256.0f / psta->dehaze_setting.iir_sigma), 0, 255));
    else
        phwcfg->iir_sigma = 0x1;
    if (psta->dehaze_setting.iir_wt_sigma >= 0.0f)
        phwcfg->iir_wt_sigma = (unsigned short)(CLIP(
            int(1024.0f / (8.0f * psta->dehaze_setting.iir_wt_sigma + 0.5f)), 0, 0x7ff));
    else
        phwcfg->iir_wt_sigma = 0x7ff;
    if (psta->dehaze_setting.iir_air_sigma)
        phwcfg->iir_air_sigma = (unsigned char)(CLIP(
            int(1024.0f / psta->dehaze_setting.iir_air_sigma), 0, 255));
    else
        phwcfg->iir_air_sigma = 0x8;
    if (psta->dehaze_setting.iir_tmax_sigma)
        phwcfg->iir_tmax_sigma = (unsigned short)(CLIP(
            int(1.0f / psta->dehaze_setting.iir_tmax_sigma), 0, 0x7ff));
    else
        phwcfg->iir_tmax_sigma = 0x5f;
    phwcfg->iir_pre_wet =
        (unsigned char)(CLIP(int(psta->dehaze_setting.iir_pre_wet - 1.0f), 0, 15));
    phwcfg->stab_fnum = ClipValue(psta->dehaze_setting.stab_fnum, 5, 0);
    phwcfg->gaus_h0         = psta->dehaze_setting.gaus_h0;
    phwcfg->gaus_h1         = psta->dehaze_setting.gaus_h1;
    phwcfg->gaus_h2         = psta->dehaze_setting.gaus_h2;

    phwcfg->enh_luma_en = psta->enhance_setting.enh_luma_en;
    phwcfg->color_deviate_en = psta->enhance_setting.color_deviate_en;
    
    phwcfg->hpara_en        = psta->hist_setting.hist_para_en;
    
    phwcfg->yblk_th         = pdyn->DehazeData.yblk_th;
    phwcfg->yhist_th        = ClipValue(pdyn->DehazeData.yhist_th, 8, 0);
    phwcfg->dc_max_th       = ClipValue(pdyn->DehazeData.dc_max_th, 8, 0);
    phwcfg->dc_min_th       = ClipValue(pdyn->DehazeData.dc_min_th, 8, 0);
    phwcfg->wt_max          = ClipValue(pdyn->DehazeData.wt_max, 0, 8);
    phwcfg->bright_max      = ClipValue(pdyn->DehazeData.bright_max, 8, 0);
    phwcfg->bright_min      = ClipValue(pdyn->DehazeData.bright_min, 8, 0);
    phwcfg->tmax_base       = ClipValue(pdyn->DehazeData.tmax_base, 8, 0);
    phwcfg->dark_th         = ClipValue(pdyn->DehazeData.dark_th, 8, 0);
    phwcfg->air_max         = ClipValue(pdyn->DehazeData.air_max, 8, 0);
    phwcfg->air_min         = ClipValue(pdyn->DehazeData.air_min, 8, 0);
    phwcfg->tmax_max        = ClipValue(pdyn->DehazeData.tmax_max, 0, 10);
    phwcfg->tmax_off        = ClipValue(pdyn->DehazeData.tmax_off, 0, 10);
    phwcfg->cfg_wt          = ClipValue(pdyn->DehazeData.cfg_wt, 0, 8);
    phwcfg->cfg_air         = ClipValue(pdyn->DehazeData.cfg_air, 8, 0);
    phwcfg->cfg_tmax        = ClipValue(pdyn->DehazeData.cfg_tmax, 0, 10);
    phwcfg->range_sima      = ClipValue(pdyn->DehazeData.range_sigma, 0, 8);
    phwcfg->space_sigma_cur = ClipValue(pdyn->DehazeData.space_sigma_cur, 0, 8);
    phwcfg->space_sigma_pre = ClipValue(pdyn->DehazeData.space_sigma_pre, 0, 8);
    phwcfg->dc_weitcur      = ClipValue(pdyn->DehazeData.dc_weitcur, 0, 8);
    phwcfg->bf_weight       = ClipValue(pdyn->DehazeData.bf_weight, 0, 8);
    
    phwcfg->enhance_value   = ClipValue(pdyn->EnhanceData.enhance_value, 4, 10);
    phwcfg->enhance_chroma  = ClipValue(pdyn->EnhanceData.enhance_chroma, 4, 10);
    for (int i = 0; i < ISP32_DHAZ_ENH_LUMA_NUM; i++)
        phwcfg->enh_luma[i] = ClipValue(pdyn->EnhanceData.enh_luma[i], 4, 6);
    for (int i = 0; i < ISP32_DHAZ_ENH_CURVE_NUM; i++)
        phwcfg->enh_curve[i] = (unsigned short)pdyn->EnhanceData.enhance_curve[i];

    phwcfg->hist_k          = ClipValue(pdyn->HistData.hist_k, 3, 2);
    phwcfg->hist_th_off     = ClipValue(pdyn->HistData.hist_th_off, 8, 0);
    phwcfg->hist_min        = ClipValue(pdyn->HistData.hist_min, 1, 8);
    phwcfg->hist_gratio     = ClipValue(pdyn->HistData.hist_gratio, 5, 3);
    phwcfg->hist_scale      = ClipValue(pdyn->HistData.hist_scale, 5, 8);
    phwcfg->cfg_gratio      = ClipValue(pdyn->HistData.cfg_gratio, 5, 8);
    for (int i = 0; i < ISP32_DHAZ_HIST_WR_NUM; i++)
        phwcfg->hist_wr[i] = psta->hist_setting.hist_sta_wr.hist_wr[i];

    for (int i = 0; i < ISP32_DHAZ_SIGMA_IDX_NUM; i++)
        phwcfg->sigma_idx[i] = psta->sigma_idx[i];
    for (int i = 0; i < ISP32_DHAZ_SIGMA_LUT_NUM; i++)
        phwcfg->sigma_lut[i] = psta->sigma_lut[i];

    return;
}

RKAIQ_END_DECLARE
