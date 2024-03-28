/*
 * af_statics.h
 *
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef __AF_HW_STATICS_HEADER_H__
#define __AF_HW_STATICS_HEADER_H__

#include "rk_aiq_comm.h"


#define AFSTATS_ZONE_ROW            15
#define AFSTATS_ZONE_COLUMN         15
#define AFSTATS_ZONE_NUM            225

typedef enum afStats_src_mode_e {
    /*
    reg: (sw_rawaf_sel == 0)
    */
    afStats_chl0Wb0Out_mode = 0,
    /*
    reg: (sw_rawaf_sel == 1)
    */
    afStats_chl1Wb0Out_mode = 1,
    /*
    reg: (sw_rawaf_sel == 2)
    */
    afStats_chl2Wb0Out_mode = 2,
    /*
    reg: (sw_rawaf_sel == 3)
    */
    afStats_dmIn_mode = 3,
    /*
    reg: (sw_bnr2af_sel_en == 1)
    */
    afStats_btnrOut_mode = 4,
    /*
    reg: (sw_rawaf_from_ynr == 1)
    */
    afStats_ynrOut_mode = 5,
    /*
    reg: (sw_vi3a_af_from_vicap == 1)
    */
    afStats_vicapOut_mode = 6
} afStats_src_mode_t;
    
typedef enum afStats_ds_mode_e {
    /*
    reg: (sw_rawaf_avg_ds_en == 0 && sw_rawaf_v_dnscl_mode == 1)
    */
    afStats_ds_disable_mode = 0,
    /*
    reg: (sw_rawaf_avg_ds_en == 0 && sw_rawaf_v_dnscl_mode == 2)
    */
    afStats_ds_2x2_mode = 1,
    /*
    reg: (sw_rawaf_avg_ds_en == 1 && sw_rawaf_avg_ds_mode == 0)
    */
    afStats_ds_4x4_mode = 2,
    /*
    reg: (sw_rawaf_avg_ds_en == 1 && sw_rawaf_avg_ds_mode == 1)
    */
    afStats_ds_8x8_mode = 3
} afStats_ds_mode_t;


typedef enum afStats_hFiltLnBnd_mode_e {
    afStats_hFiltLnBnd_preLnPix_mode = 0,
    afStats_hFiltLnBnd_curLnPix_mode = 1
} afStats_hFiltLnBnd_mode_t;
typedef struct afStats_coring_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_coring_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(10f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Threshold value for coring.\n
        Freq of use: high))  */
    //reg: sw_rawaf_h_fv_thresh, sw_rawaf_v_fv_thresh
    float hw_afCfg_coring_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_slope_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0039, 1.996),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f9b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Slope value for coring.\n
        Freq of use: high))  */
    //reg: sw_rawaf_h_fv_slope, sw_rawaf_v_fv_slope
    float hw_afCfg_slope_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_coring_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(10f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Max limit value for coring.\n
        Freq of use: high))  */
    //reg: sw_rawaf_h_fv_limit, sw_rawaf_v_fv_limit
    float hw_afCfg_coring_maxLimit;
} afStats_coring_t;

typedef struct afStats_hLdg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_lumaL_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Threshold value at dark side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_lumth0, sw_rawaf_h_ldg_lumth0
    float hw_afCfg_lumaL_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_gainL_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Gain value at dark side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_gain0, sw_rawaf_h_ldg_gain0
    float hw_afCfg_gainL_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_slopeL_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 31.996),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f13b),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Slope value at dark side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_gslp0, sw_rawaf_h_ldg_gslp0
    float hw_afCfg_slopeL_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_lumaR_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Threshold value at bright side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_lumth1, sw_rawaf_h_ldg_lumth1
    float hw_afCfg_lumaR_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_gainR_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Gain value at bright side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_gain1, sw_rawaf_h_ldg_gain1
    float hw_afCfg_gainR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_slopeR_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 31.996),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f13b),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Slope value at bright side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_gslp1, sw_rawaf_h_ldg_gslp1
    float hw_afCfg_slopeR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_maxOptBndL_len),
        M4_TYPE(u8),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(17),
        M4_NOTES(Enable use max luma of left hw_afCfg_hLdgDilate_num pixel to do ldg function.\nFreq of use: low))  */
    //reg: sw_rawaf_hldg_dilate_num
    uint8_t hw_afCfg_maxOptBndL_len;
} afStats_hLdg_t;

typedef struct afStats_vLdg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_lumaL_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Threshold value at dark side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_lumth0, sw_rawaf_h_ldg_lumth0
    float hw_afCfg_lumaL_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_gainL_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Gain value at dark side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_gain0, sw_rawaf_h_ldg_gain0
    float hw_afCfg_gainL_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_slopeL_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 31.996),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f13b),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Slope value at dark side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_gslp0, sw_rawaf_h_ldg_gslp0
    float hw_afCfg_slopeL_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_lumaR_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Threshold value at bright side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_lumth1, sw_rawaf_h_ldg_lumth1
    float hw_afCfg_lumaR_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_gainR_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Gain value at bright side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_gain1, sw_rawaf_h_ldg_gain1
    float hw_afCfg_gainR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_slopeR_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 31.996),
        M4_DEFAULT(0),
        M4_DIGIT_EX(8f13b),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Slope value at bright side of ldg function.\n
        Freq of use: low))  */
    //reg: sw_rawaf_v_ldg_gslp1, sw_rawaf_h_ldg_gslp1
    float hw_afCfg_slopeR_val;
} afStats_vLdg_t;


typedef enum afStats_fvFmt_mode_e {
    /*
    reg: (sw_rawaf_XX_fv_outmode== 1 && sw_rawaf_XX_acc_mode == 1)
    */
    afStats_outPeak_sumLinear_mode = 0,
    /*
    reg: (sw_rawaf_XX_fv_outmode== 1 && sw_rawaf_XX_acc_mode == 0)
    */
    afStats_outNorm_sumLinear_mode = 1,
    /*
    reg: (sw_rawaf_XX_fv_outmode== 0 && sw_rawaf_XX_acc_mode == 1)
    */
    afStats_outPeak_sumSqu_mode = 2,
    /*
    reg: (sw_rawaf_XX_fv_outmode== 0 && sw_rawaf_XX_acc_mode == 0)
    */
    afStats_outNorm_sumSqu_mode = 3
} afStats_fvFmt_mode_t;

typedef struct afStats_hFilt_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_iirStep1_coeff),
        M4_TYPE(s16),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(-2047,2047),
        M4_DEFAULT([512, 557, -276, 460, 0, -460]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(H1 iir coefficient in af statics.\nFreq of use: high))  */
    //reg: sw_rawaf_h1iir1_coe0 ~ sw_rawaf_h1iir1_coe5
    //reg: sw_rawaf_h2iir1_coe0 ~ sw_rawaf_h2iir1_coe5
    int16_t hw_afCfg_iirStep1_coeff[6];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_iirStep2_coeff),
        M4_TYPE(s16),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(-2047,2047),
        M4_DEFAULT([100, 870, -399, 191, 0, -191]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(H1 iir coefficient in af statics.\nFreq of use: high))  */
    //reg: sw_rawaf_h1iir2_coe0 ~ sw_rawaf_h1iir2_coe5
    //reg: sw_rawaf_h2iir2_coe0 ~ sw_rawaf_h2iir2_coe5
    int16_t hw_afCfg_iirStep2_coeff[6];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_fvFmt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(afStats_fvFmt_mode_t),
        M4_DEFAULT(afStats_outNorm_sumSqu_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(... Reference enum types.\nFreq of use: high))  */
    // reg: sw_rawaf_h1_fv_outmode, sw_rawaf_h1_acc_mode, sw_rawaf_h2_fv_outmode, sw_rawaf_h2_acc_mode,
    afStats_fvFmt_mode_t hw_afCfg_fvFmt_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_accMainWin_shift),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 7),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(...\n
        Freq of use: low))  */
    // reg: sw_rawaf_h1iir_shift_wina, sw_rawaf_h2iir_shift_wina 
    uint8_t hw_afCfg_accMainWin_shift;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_accSubWin_shift),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 15),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(...\n
        Freq of use: low))  */
    // reg: sw_rawaf_h1iir_shift_wina, sw_rawaf_h2iir_shift_wina 
    uint8_t hw_afCfg_accSubWin_shift;
} afStats_hFilt_t;

typedef struct afStats_vFilt_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_iirStep1_coeff),
        M4_TYPE(s16),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(-2047,2047),
        M4_DEFAULT([-265, 686, 512]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(H1 iir coefficient in af statics.\nFreq of use: high))  */
    //reg: sw_rawaf_v1iir_coe0 ~ sw_rawaf_v1iir_coe3
    //reg: sw_rawaf_v2iir_coe0 ~ sw_rawaf_v2iir_coe3
    int16_t hw_afCfg_iirStep1_coeff[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afT_firStep2_coeff),
        M4_TYPE(s16),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(-2047,2047),
        M4_DEFAULT([-124, 0, 124]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(H1 iir coefficient in af statics.\nFreq of use: high))  */
    //reg: sw_rawaf_v1fir_coe0 ~ sw_rawaf_v1fir_coe3
    //reg: sw_rawaf_v2fir_coe0 ~ sw_rawaf_v2fir_coe3
    int16_t hw_afCfg_firStep2_coeff[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_fvFmt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(afStats_fvFmt_mode_t),
        M4_DEFAULT(afStats_outNorm_sumSqu_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(... Reference enum types.\nFreq of use: high))  */
    // reg: sw_rawaf_v1_fv_outmode, sw_rawaf_v1_acc_mode, sw_rawaf_v2_fv_outmode, sw_rawaf_v2_acc_mode,
    afStats_fvFmt_mode_t hw_afCfg_fvFmt_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_accMainWin_shift),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 7),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(...\n
        Freq of use: low))  */
    // reg: sw_rawaf_v1iir_shift_wina, sw_rawaf_v2iir_shift_wina 
    uint8_t hw_afCfg_accMainWin_shift;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_accSubWin_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 15),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(...\n
        Freq of use: low))  */
    // reg: sw_rawaf_v1iir_shift_winb, sw_rawaf_v2iir_shift_winb 
    uint8_t hw_afCfg_accSubWin_shift;
} afStats_vFilt_t;

typedef struct afStats_gamma_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_gamma_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Whether to enable gamma function.\nFreq of use: low))  */
    //reg: sw_gamma_en
    bool hw_afCfg_gamma_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_gamma_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([0, 0.0440, 0.1056, 0.1750, 0.2395, 0.3363, 0.3998, 0.4487, 0.4888, 0.5543, 0.6080, 0.6608, 0.7419, 0.8143, 0.8759, 0.9404, 1.0000]),
        M4_HIDE_EX(0),
        M4_DIGIT_EX(8f8b),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Gamma curve in af statics.\nFreq of use: high))  */
    //reg: sw_y0 ~ sw_y16
    float hw_afCfg_gamma_val[17]; 
} afStats_gamma_t;

typedef struct afStats_gaus_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_gaus_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Whether to enable gause filter.\nFreq of use: low))  */
    //reg: sw_gaus_en
    bool hw_afCfg_gaus_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_gaus_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(3,3),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0,0.5,0,0,0.5,0,0,0,0]),
        M4_HIDE_EX(0),
        M4_DIGIT_EX(8f8b),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Gause coefficient in af statics.\nFreq of use: high))  */
    //reg: sw_rawaf_gaus_coe0 ~ sw_rawaf_gaus_coe8
    float hw_afCfg_gaus_coeff[9];
} afStats_gaus_t;

typedef struct afStats_mainWin_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_win_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(2,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal offset of af statics roi.\nFreq of use: high))  */
    //reg: sw_a_h_l
    uint16_t hw_afCfg_win_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_win_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Vertical offset of af statics roi.\nFreq of use: high))  */
    //reg: sw_a_v_t
    uint16_t hw_afCfg_win_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_win_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Horizontal size of af statics roi.\nFreq of use: high))  */
    //reg: sw_a_h_size
    uint16_t hw_afCfg_win_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_win_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Vertical size of af statics roi.\nFreq of use: high))  */
    //reg: sw_a_v_size
    uint16_t hw_afCfg_win_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_lumaStats_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Enable luma and highlight statics.\n))  */
    //reg: sw_rawaf_aehgl_en
    bool hw_afCfg_lumaStats_en;
} afStats_mainWin_t;

typedef struct afStats_subWin_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_win_x),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(2,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Horizontal offset of af statics roi.\nFreq of use: high))  */
    //reg: sw_b_h_l
    uint16_t hw_afCfg_win_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_win_y),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,8191),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Vertical offset of af statics roi.\nFreq of use: high))  */
    //reg: sw_b_v_t
    uint16_t hw_afCfg_win_y;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_win_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8192),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Horizontal size of af statics roi.\nFreq of use: high))  */
    //reg: sw_b_h_size
    uint16_t hw_afCfg_win_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_win_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8192),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Vertical size of af statics roi.\nFreq of use: high))  */
    //reg: sw_b_v_size
    uint16_t hw_afCfg_win_height;
} afStats_subWin_t;

typedef struct afStats_blc_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_blc_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable blc function\nFreq of use: low))  */
    //reg: sw_rawaf_bls_en
    bool hw_afCfg_blc_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_blc_offset),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-255,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Offset value of black level\nFreq of use: low))  */
    //reg: sw_rawaf_bls_offset
    int16_t hw_afCfg_ob_val;
} afStats_blc_t;

typedef struct afStats_cfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_stats_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable af statics\nFreq of use: low))  */
    //reg: sw_rawaf_en
    bool hw_afCfg_stats_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_statsSrc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(afStats_src_mode_t),
        M4_DEFAULT(afStats_chl0Wb0Out_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Input source selection. Reference enum types.\nFreq of use: high))  */
    afStats_src_mode_t hw_afCfg_statsSrc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_statsBtnrOut_shift),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Right shift bit number when use af_statsBtnrOut_mode.\nFreq of use: high))  */
    //reg: sw_rawaf_tnrin_shift
    uint8_t hw_afCfg_statsBtnrOut_shift;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mainWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Roi of window A))  */
    afStats_mainWin_t mainWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Roi of window B))  */
    afStats_subWin_t subWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(blc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Black level setting))  */
    afStats_blc_t blc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_ds_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(afStats_ds_mode_t),
        M4_DEFAULT(AFSEL_DPCCOUT_0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Down scale mode. Reference enum types.\nFreq of use: high))  */
    afStats_ds_mode_t hw_afCfg_ds_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gamma),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Gamma setting))  */
    afStats_gamma_t gamma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(preFilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Gause setting))  */
    afStats_gaus_t preFilt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_hLumaCnt_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.8915),
        M4_HIDE_EX(0),
        M4_DIGIT_EX(10f10b),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Hightlight threshold value.\nFreq of use: high))  */
    //reg: sw_rawaf_highlit_thresh
    float hw_afCfg_hLumaCnt_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_ldg_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Enable ldg function.\nFreq of use: high))  */
    //reg: sw_rawaf_ldg_en
    bool hw_afCfg_ldg_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_hFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Whether to enable horizontal filter.\n
        Freq of use: low))  */
    //reg: sw_rawaf_hiir_en
    bool hw_afCfg_hFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_hFiltLnBnd_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(afStats_hFiltLnBnd_mode_t),
        M4_DEFAULT(afStats_hFiltLnBnd_curLnPix_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Weather left border of horizontal filter use above line right border out.\nFreq of use: low))  */
    //reg: sw_rawaf_hiir_left_border_mode
    afStats_hFiltLnBnd_mode_t hw_afCfg_hFiltLnBnd_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hFilt1),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(Horizontal filter 1 setting))  */
    afStats_hFilt_t hFilt1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hFilt2),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Horizontal filter 2 setting))  */
    afStats_hFilt_t hFilt2;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hFiltCoring),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Horizontal filter coring setting))  */
    afStats_coring_t hFilt_coring;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hFiltLdg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Horizontal filter ldg setting))  */
    afStats_hLdg_t hFilt_ldg;    
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_afCfg_vFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(18),
        M4_NOTES(Whether to enable vertical filter.\n
        Freq of use: low))  */
    //reg: sw_rawaf_viir_en
    bool hw_afCfg_vFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(vFilt1),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(19),
        M4_NOTES(Vertical filter 1 setting))  */
    afStats_vFilt_t vFilt1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(vFilt2),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(20),
        M4_NOTES(Vertical filter 2 setting))  */
    afStats_vFilt_t vFilt2;
    /* M4_GENERIC_DESC(
        M4_ALIAS(vFiltCoring),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(21),
        M4_NOTES(Vertical filter coring setting))  */
    afStats_coring_t vFilt_coring;
    /* M4_GENERIC_DESC(
        M4_ALIAS(vFiltLdg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(Vertical filter ldg setting))  */
    afStats_vLdg_t vFilt_ldg;
} afStats_cfg_t;

typedef struct afStats_mainWinStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_hFilt1Fv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(H1 fv value\nFreq of use: high))  */
    uint32_t hw_af_hFilt1Fv_val[AFSTATS_ZONE_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_hFilt2Fv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(H2 fv value\nFreq of use: high))  */
    uint32_t hw_af_hFilt2Fv_val[AFSTATS_ZONE_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_vFilt1Fv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(V1 fv value\nFreq of use: high))  */
    uint32_t hw_af_vFilt1Fv_val[AFSTATS_ZONE_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_vFilt2Fv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(V2 fv value\nFreq of use: high))  */
    uint32_t hw_af_vFilt2Fv_val[AFSTATS_ZONE_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_luma_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Luma value.\nFreq of use: high))  */
    uint32_t hw_af_luma_val[AFSTATS_ZONE_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_hLumaCnt_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(High light value.\nFreq of use: high))  */
    uint32_t hw_af_hLumaCnt_val[AFSTATS_ZONE_NUM];
} afStats_mainWinStats_t;

typedef struct afStats_subWinStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_hFilt1Fv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(H1 fv value\nFreq of use: high))  */
    uint32_t hw_af_hFilt1Fv_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_hFilt2Fv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(H2 fv value\nFreq of use: high))  */
    uint32_t hw_af_hFilt2Fv_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_vFilt1Fv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(V1 fv value\nFreq of use: high))  */
    uint32_t hw_af_vFilt1Fv_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_vFilt2Fv_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(V2 fv value\nFreq of use: high))  */
    uint32_t hw_af_vFilt2Fv_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_luma_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Luma value.\nFreq of use: high))  */
    uint32_t hw_af_luma_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_af_hLumaCnt_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 0xFFFFFF),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(High light value.\nFreq of use: high))  */
    uint32_t hw_af_hLumaCnt_val;
} afStats_subWinStats_t;

typedef struct afStats_stats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(statsWinA),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Window A stats))  */
    afStats_mainWinStats_t mainWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(statsWinB),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Window B stats))  */
    afStats_subWinStats_t subWin;
} afStats_stats_t;

#endif
