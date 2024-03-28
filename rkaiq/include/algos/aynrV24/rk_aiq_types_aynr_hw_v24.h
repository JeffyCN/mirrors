/*
 *rk_aiq_types_aynr_hw_v24.h
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

#ifndef _RK_AIQ_TYPE_AYNR_HW_V24_H_
#define _RK_AIQ_TYPE_AYNR_HW_V24_H_
#include "rk_aiq_comm.h"

typedef struct RK_YNR_Fix_V24_s {
    bool ynr_en;
    /* GLOBAL_CTRL */
    uint8_t lospnr_bypass;
    uint8_t hispnr_bypass;
    uint8_t exgain_bypass;
    uint16_t global_set_gain;
    uint8_t gain_merge_alpha;
    uint8_t rnr_en;
    /* RNR_MAX_R */
    uint16_t rnr_max_radius;
    uint16_t local_gain_scale;
    /* RNR_CENTER_COOR */
    uint16_t rnr_center_coorh;
    uint16_t rnr_center_coorv;
    /* LOWNR_CTRL0 */
    uint16_t ds_filt_soft_thred_scale;
    uint8_t ds_img_edge_scale;
    uint16_t ds_filt_wgt_thred_scale;
    /* LOWNR_CTRL1 */
    uint8_t ds_filt_local_gain_alpha;
    uint8_t ds_iir_init_wgt_scale;
    uint16_t ds_filt_center_wgt;
    /* LOWNR_CTRL2 */
    uint16_t ds_filt_inv_strg;
    uint16_t lospnr_wgt;
    /* LOWNR_CTRL3 */
    uint16_t lospnr_center_wgt;
    uint16_t lospnr_strg;
    /* LOWNR_CTRL4 */
    uint16_t lospnr_dist_vstrg_scale;
    uint16_t lospnr_dist_hstrg_scale;
    /* GAUSS_COEFF */
    uint8_t pre_filt_coeff0;
    uint8_t pre_filt_coeff1;
    uint8_t pre_filt_coeff2;
    /* LOW_GAIN_ADJ */
    uint8_t lospnr_gain2strg_val[9];
    /* SGM_DX */
    uint16_t luma2sima_idx[17];
    /* LSGM_Y */
    uint16_t luma2sima_val[17];
    /* RNR_STRENGTH */
    uint8_t radius2strg_val[17];
    /* NLM_STRONG_EDGE */
    uint8_t hispnr_strong_edge;
    /* NLM_SIGMA_GAIN */
    uint16_t hispnr_sigma_min_limit;
    uint8_t hispnr_local_gain_alpha;
    uint16_t hispnr_strg;
    /* NLM_COE */
    uint8_t hispnr_filt_coeff[6];
    /* NLM_WEIGHT */
    uint16_t hispnr_filt_wgt_offset;
    uint32_t hispnr_filt_center_wgt;
    /* NLM_NR_WEIGHT */
    uint16_t hispnr_filt_wgt;
    uint16_t hispnr_gain_thred;
} RK_YNR_Fix_V24_t;

#endif
