/*
 *rk_aiq_types_asharp_hw_v34.h
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
#ifndef _RK_AIQ_TYPES_ASHARP_HW_V34_H_
#define _RK_AIQ_TYPES_ASHARP_HW_V34_H_
#include "rk_aiq_comm.h"

typedef struct RK_SHARP_Fix_V34_s {
    /* SHARP_EN */
    uint8_t bypass;
    uint8_t center_mode;
    uint8_t local_gain_bypass;
    uint8_t radius_step_mode;
    uint8_t noise_clip_mode;
    uint8_t clipldx_sel;
    uint8_t baselmg_sel;
    uint8_t noise_filt_sel;
    uint8_t tex2wgt_en;

    /* SHARP_RATIO */
    uint8_t pre_bifilt_alpha;
    uint8_t guide_filt_alpha;
    uint8_t detail_bifilt_alpha;
    uint8_t global_sharp_strg;

    /* SHARP_LUMA_DX */
    uint8_t luma2table_idx[7];

    /* SHARP_PBF_SIGMA_INV */
    uint16_t pbf_sigma_inv[8];

    /* SHARP_BF_SIGMA_INV */
    uint16_t bf_sigma_inv[8];

    /* SHARP_SIGMA_SHIFT */
    uint8_t pbf_sigma_shift;
    uint8_t bf_sigma_shift;

    /* EHF_TH */
    uint16_t luma2strg_val[8];

    /* SHARP_CLIP_HF */
    uint16_t luma2posclip_val[8];

    /* SHARP_PBF_COEF */
    uint8_t pbf_coef0;
    uint8_t pbf_coef1;
    uint8_t pbf_coef2;

    /* SHARP_BF_COEF */
    uint8_t bf_coef0;
    uint8_t bf_coef1;
    uint8_t bf_coef2;

    /* SHARP_GAUS_COEF */
    uint8_t img_lpf_coeff[6];

    /* SHARP_GAIN */
    uint16_t global_gain;
    uint8_t gain_merge_alpha;
    uint8_t local_gain_scale;

    /* SHARP_GAIN_ADJUST */
    uint16_t gain2strg_val[14];

    /* SHARP_CENTER */
    uint16_t center_x;
    uint16_t center_y;

    /* SHARP_GAIN_DIS_STRENGTH */
    uint8_t distance2strg_val[22];

    /* CLIP_NEG */
    uint16_t luma2neg_clip_val[8];

    /* TEXTURE0 */
    uint16_t noise_max_limit;
    uint8_t tex_reserve_level;
    /* TEXTURE1 */
    uint16_t tex_wet_scale;
    uint8_t noise_norm_bit;
    uint8_t tex_wgt_mode;
    /* TEXTURE_LUT */
    uint16_t tex2wgt_val[17];
    /* TEXTURE2 */
    uint32_t noise_strg;
    /* DETAIL_STRG_LUT */
    uint16_t detail2strg_val[17];
} RK_SHARP_Fix_V34_t;

#endif
