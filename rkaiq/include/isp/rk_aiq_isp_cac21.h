/*
 * rk_aiq_param_cac21.h
 *
 *  Copyright (c) 2023 Rockchip Corporation
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
#ifndef _RK_AIQ_PARAM_CAC21_H_
#define _RK_AIQ_PARAM_CAC21_H_

#define RKCAC_MAX_PATH_LEN       255
#define RKCAC_STRENGTH_TABLE_LEN 22

typedef struct cac_psfParam_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cacCfg_psf_path),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT("/etc/iqfiles/cac_map.bin"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    char sw_cacT_psfMap_path[RKCAC_MAX_PATH_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_psf_sft_bit),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,10),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_psf_sft_bit
    uint8_t hw_cacT_psfShift_bits;
} cac_psfParam_t;


typedef struct cac_strgCenter_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_center_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    // reg: sw_cac_center_en;
    bool hw_cacT_strgCenter_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_center_width),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_center_width
    uint16_t hw_cacT_strgCenter_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_center_height),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_center_height
    uint16_t hw_cacT_strgCenter_y;
} cac_strgCenter_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(psfParam),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(...\n
        Freq of use: low))  */
    cac_psfParam_t psfParam;
    /* M4_GENERIC_DESC(
        M4_ALIAS(strgCenter),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(...\n
        Freq of use: low))  */
    cac_strgCenter_t strgCenter;
} cac_params_static_t;

typedef struct cac_strgInterp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cacT_globalCorr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 16.00),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4f7)
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    float sw_cacT_globalCorr_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_strength),
        M4_TYPE(f32),
        M4_SIZE_EX(1,22),
        M4_RANGE_EX(0, 16.00),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4f7)
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_strength0 ~ sw_cac_strength21
    float hw_cacT_corrStrength_table[RKCAC_STRENGTH_TABLE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_edge_detect_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    // reg: sw_cac_edge_detect_en;
    bool hw_cacT_edgeDetect_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_flat_thed_b),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4f4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // reg: sw_cac_flat_thed_b;
    float hw_cacT_flatDctB_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_flat_thed_r),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_DIGIT_EX(4f4),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // reg: sw_cac_flat_thed_r;
    float hw_cacT_flatDctR_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_flat_thed_b),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_flat_thed_b
    uint32_t hw_cacT_flatDctB_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_flat_thed_r),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_flat_thed_r
    uint32_t hw_cacT_flatDctR_offset;
} cac_strgInterp_t;

typedef enum cac_clipGT_mode_e {
    cac_clipGNone_mode = 0,
    cac_clipGMinimal_mode,
    cac_clipGAverage_mode,
} cac_clipGT_mode_t;

typedef struct cac_chromaAberrCorr_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_clip_g_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(cac_clipGT_mode_t),
        M4_DEFAULT(cac_clipGNone_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Reference enum types.\n
		Freq of use: low))  */
    cac_clipGT_mode_t hw_cacT_clipG_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_neg_clip0_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    // reg: sw_cac_neg_clip0_en;
    bool hw_cacT_negClip0_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cacT_expoDctB_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    bool sw_cacT_expoDctB_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cacT_expoDctR_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    bool sw_cacT_expoDctR_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_over_expo_thed_b),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_over_expo_thed_b
    uint32_t hw_cacT_overExpoB_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_over_expo_thed_r),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_over_expo_thed_r
    uint32_t hw_cacT_overExpoR_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_over_expo_adj_b),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_over_expo_adj_b
    uint32_t hw_cacT_overExpoB_adj;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cac_over_expo_adj_r),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_cac_over_expo_adj_r
    uint32_t hw_cacT_overExpoR_adj;
} cac_chromaAberrCorr_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(strgInterp),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cac_strgInterp_t strgInterp;
    /* M4_GENERIC_DESC(
        M4_ALIAS(chromaAberrCorr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cac_chromaAberrCorr_t chromaAberrCorr;
} cac_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of demosaic module))  */
    cac_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    cac_params_dyn_t dyn;
} cac_param_t;

#endif