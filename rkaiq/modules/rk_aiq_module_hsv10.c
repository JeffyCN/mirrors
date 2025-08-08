/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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

#include "rk_aiq_isp33_modules.h"

RKAIQ_BEGIN_DECLARE
    
void rk_aiq_hsv10_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo)
{
    struct isp33_hsv_cfg *cfg = &isp_params->isp_cfg->others.hsv_cfg;
    hsv_param_t* hsv_param = (hsv_param_t*) attr;
    hsv_param_static_t* psta = &hsv_param->sta;
    hsv_param_dyn_t* pdyn = &hsv_param->dyn;

    cfg->hsv_1dlut0_en        = psta->hw_hsvT_lut1d0_en;
    cfg->hsv_1dlut1_en        = psta->hw_hsvT_lut1d1_en;
    cfg->hsv_2dlut_en         = psta->hw_hsvT_lut2d_en;

    switch ((uint8_t)pdyn->lut1d0.hw_hsvT_lut1d_mode) {
        case hsv_lut1d_h2hDiff_mode: {
            cfg->hsv_1dlut0_idx_mode  = 0;
            cfg->hsv_1dlut0_item_mode = 0;
        } break;
        case hsv_lut1d_h2sDiff_mode: {
            cfg->hsv_1dlut0_idx_mode  = 0;
            cfg->hsv_1dlut0_item_mode = 1;
        } break;
        case hsv_lut1d_h2vDiff_mode: {
            cfg->hsv_1dlut0_idx_mode  = 0;
            cfg->hsv_1dlut0_item_mode = 2;
        } break;
        case hsv_lut1d_s2sDiff_mode: {
            cfg->hsv_1dlut0_idx_mode  = 1;
            cfg->hsv_1dlut0_item_mode = 1;
        } break;
        default:
            break;
    }

    switch ((uint8_t)pdyn->lut1d1.hw_hsvT_lut1d_mode) {
        case hsv_lut1d_h2hDiff_mode: {
            cfg->hsv_1dlut1_idx_mode  = 0;
            cfg->hsv_1dlut1_item_mode = 0;
        } break;
        case hsv_lut1d_h2sDiff_mode: {
            cfg->hsv_1dlut1_idx_mode  = 0;
            cfg->hsv_1dlut1_item_mode = 1;
        } break;
        case hsv_lut1d_h2vDiff_mode: {
            cfg->hsv_1dlut1_idx_mode  = 0;
            cfg->hsv_1dlut1_item_mode = 2;
        } break;
        case hsv_lut1d_s2sDiff_mode: {
            cfg->hsv_1dlut1_idx_mode  = 1;
            cfg->hsv_1dlut1_item_mode = 1;
        } break;
        default:
            break;
    }
    
    switch (pdyn->lut2d.hw_hsvT_lut2d_mode) {
        case hsv_lut2d_hs2h_mode: {
            cfg->hsv_2dlut_idx_mode  = 0;
            cfg->hsv_2dlut_item_mode = 0;
        } break;
        case hsv_lut2d_hv2h_mode: {
            cfg->hsv_2dlut_idx_mode  = 1;
            cfg->hsv_2dlut_item_mode = 0;
        } break;
        case hsv_lut2d_sv2s_mode: {
            cfg->hsv_2dlut_idx_mode  = 2;
            cfg->hsv_2dlut_item_mode = 1;
        } break;
        case hsv_lut2d_hs2s_mode: {
            cfg->hsv_2dlut_idx_mode  = 0;
            cfg->hsv_2dlut_item_mode = 1;
        } break;
        case hsv_lut2d_hv2v_mode: {
            cfg->hsv_2dlut_idx_mode  = 1;
            cfg->hsv_2dlut_item_mode = 2;
        } break;
        case hsv_lut2d_sv2v_mode: {
            cfg->hsv_2dlut_idx_mode  = 2;
            cfg->hsv_2dlut_item_mode = 2;
        } break;
        default:
            break;
    }
    
    for (int i = 0; i < ISP33_HSV_1DLUT_NUM; i++)
        cfg->lut0_1d[i] = ((uint16_t)pdyn->lut1d0.hw_hsvT_lut1d_val[i]) << 2;

    for (int i = 0; i < ISP33_HSV_1DLUT_NUM; i++)
        cfg->lut1_1d[i] = ((uint16_t)pdyn->lut1d1.hw_hsvT_lut1d_val[i]) << 2;
    
    int count = 0;
    for (int i = 0; i < ISP33_HSV_2DLUT_ROW; i++){
        for (int j = 0; j < ISP33_HSV_2DLUT_COL; j++) {
            cfg->lut_2d[i][j] = ((uint16_t)pdyn->lut2d.hw_hsvT_lut2d_val[count]) << 2;
            count ++;
        }
    }

}

RKAIQ_END_DECLARE
