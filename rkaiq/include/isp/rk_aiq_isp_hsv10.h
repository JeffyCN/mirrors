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

#ifndef _RK_AIQ_PARAM_HSV10_H_
#define _RK_AIQ_PARAM_HSV10_H_

#ifndef HSV_1DLUT_NUM
#define HSV_1DLUT_NUM 65
#endif
#ifndef HSV_2DLUT_ROW
#define HSV_2DLUT_ROW 17
#endif
#ifndef HSV_2DLUT_COL
#define HSV_2DLUT_COL 17
#endif
#ifndef HSV_2DLUT_NUM
#define HSV_2DLUT_NUM  (HSV_2DLUT_ROW*HSV_2DLUT_COL)
#endif
/*****************************************************************************/
/**
 * @brief   Global HSV calibration structure of isp2.0
 */
/*****************************************************************************/
typedef enum hsv_lut1d_mode_e {
    // @reg: sw_hsv_1dlut0_idx_mode == 0 && sw_hsv_1dlut0_item_mode == 0
    // @reg: sw_hsv_1dlut1_idx_mode == 0 && sw_hsv_1dlut1_item_mode == 0
    hsv_lut1d_h2hDiff_mode = 0,
    // @reg: sw_hsv_1dlut0_idx_mode == 0 && sw_hsv_1dlut0_item_mode == 1
    // @reg: sw_hsv_1dlut1_idx_mode == 0 && sw_hsv_1dlut1_item_mode == 1
    hsv_lut1d_h2sDiff_mode = 1,
    // @reg: sw_hsv_1dlut0_idx_mode == 0 && sw_hsv_1dlut0_item_mode == 2
    // @reg: sw_hsv_1dlut1_idx_mode == 0 && sw_hsv_1dlut1_item_mode == 2
    hsv_lut1d_h2vDiff_mode = 2,
    // @reg: sw_hsv_1dlut0_idx_mode == 1 && sw_hsv_1dlut0_item_mode == 1
    // @reg: sw_hsv_1dlut1_idx_mode == 1 && sw_hsv_1dlut1_item_mode == 1
    hsv_lut1d_s2sDiff_mode = 3,
} hsv_lut1d_mode_t;

typedef enum hsv_lut2d_mode_e {
    // @reg: sw_hsv_2dlut_idx_mode == 0 && sw_hsv_2dlut_item_mode == 0
    hsv_lut2d_hs2h_mode = 0,
    // @reg: sw_hsv_2dlut_idx_mode == 1 && sw_hsv_2dlut_item_mode == 0
    hsv_lut2d_hv2h_mode = 1,
    // @reg: sw_hsv_2dlut_idx_mode == 2 && sw_hsv_2dlut_item_mode == 1
    hsv_lut2d_sv2s_mode = 2,
    // @reg: sw_hsv_2dlut_idx_mode == 0 && sw_hsv_2dlut_item_mode == 1
    hsv_lut2d_hs2s_mode = 3,
    // @reg: sw_hsv_2dlut_idx_mode == 1 && sw_hsv_2dlut_item_mode == 2
    hsv_lut2d_hv2v_mode = 4,
    // @reg: sw_hsv_2dlut_idx_mode == 2 && sw_hsv_2dlut_item_mode == 2
    hsv_lut2d_sv2v_mode = 5,
} hsv_lut2d_mode_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_hsvT_lut1d_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(hsv_lut1d_mode_t),
        M4_DEFAULT(hsv_lut1d_h2hDiff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Input and output channels of lut1d.\n
        Freq of use: high))  */
    hsv_lut1d_mode_t hw_hsvT_lut1d_mode;
   /* M4_GENERIC_DESC(
       M4_ALIAS(LutValue),
       M4_TYPE(s16),
       M4_SIZE_EX(1,65),
       M4_RANGE_EX(-1024,1023),
       M4_DEFAULT(0),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES(The values of lut1d.\n Freq of use: high))  */
    // @reg: lut0_1d/lut1_1d
    int16_t hw_hsvT_lut1d_val[HSV_1DLUT_NUM];
} hsv_lut1d_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_hsvT_lut2d_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(hsv_lut2d_mode_t),
        M4_DEFAULT(hsv_lut2d_sv2s_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Input and output channels of lut2d.\n
        Freq of use: high))  */
    hsv_lut2d_mode_t hw_hsvT_lut2d_mode;
   /* M4_GENERIC_DESC(
       M4_ALIAS(LutValue),
       M4_TYPE(s16),
       M4_SIZE_EX(17,17),
       M4_RANGE_EX(0,1023),
       M4_DEFAULT(640),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES(The values of lut2d.\n Freq of use: high))  */
    // @reg: lut_2d
    int16_t hw_hsvT_lut2d_val[HSV_2DLUT_NUM];
} hsv_lut2d_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(lut1d0),
        M4_TYPE(struct),
        M4_UI_MODULE(HSV_1DLut0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO)) */
    hsv_lut1d_dyn_t lut1d0;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lut1d1),
        M4_TYPE(struct),
        M4_UI_MODULE(HSV_1DLut1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO)) */
    hsv_lut1d_dyn_t lut1d1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lut2d),
        M4_TYPE(struct),
        M4_UI_MODULE(HSV_2DLut),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO)) */
    hsv_lut2d_dyn_t lut2d;
} hsv_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_hsvT_lut1d0_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable lut0.\nFreq of use: high))  */
    // @reg: sw_hsv_1dlut0_en
    bool hw_hsvT_lut1d0_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_hsvT_lut1d1_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable lut1.\nFreq of use: high))  */
    // @reg: sw_hsv_1dlut1_en
    bool hw_hsvT_lut1d1_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_hsvT_lut2d_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable lut2.\nFreq of use: high))  */
    // @reg: sw_hsv_2dlut_en
    bool hw_hsvT_lut2d_en;
} hsv_param_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    hsv_param_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    hsv_param_dyn_t dyn;
} hsv_param_t;

#endif
