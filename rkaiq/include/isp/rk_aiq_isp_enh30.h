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

#ifndef _RK_AIQ_PARAM_ENH30_H_
#define _RK_AIQ_PARAM_ENH30_H_

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_enhT_sigma_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The inverse sigma of iir filter.\n Freq of use: low))  */
    float hw_enhT_sigma_val;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_softThd_val),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,31),
    M4_DEFAULT(5),
    M4_DIGIT_EX(0f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The soft thread sigma of iir filter.\n Freq of use: low))  */
    float hw_enhT_softThd_val;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_centerPix_wgt),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The current pixel weight of iir filter.\n Freq of use: low))  */
    float hw_enhT_centerPix_wgt;
} enh_iir_spatial_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_iirFrm_maxLimit),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(1,15),
    M4_DEFAULT(4),
    M4_DIGIT_EX(0f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The current thumb weight of low frequency bilateral filter.\n Freq of use: low))  */
    float hw_enhT_iirFrm_maxLimit;
} enh_iir_temporal_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(spatial),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All spatial params corresponded with iso array.\n Freq of use: low))  */
    enh_iir_spatial_params_t spatial;
    /* M4_GENERIC_DESC(
    M4_ALIAS(temporal),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All temporal params corresponded with iso array.\n Freq of use: low))  */
    enh_iir_temporal_params_t temporal;
} enh_guideImg_iir_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_loBlf_en),
    M4_GROUP_CTRL(loBlf_en_group),
    M4_TYPE(bool),
    M4_DEFAULT(0),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The enable of low frequency bilateral filter.\n
    Freq of use: high))  */
    bool hw_enhT_loBlf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_enhT_sigma_val),
        M4_GROUP(loBlf_en_group),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The inverse sigma of low frequency bilateral filter.\n Freq of use: high))  */
    float hw_enhT_sigma_val;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_centerPix_wgt),
    M4_GROUP(loBlf_en_group),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The current pixel weight of low frequency bilateral filter.\n Freq of use: low))  */
    float hw_enhT_centerPix_wgt;
} enh_loBifilt_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_enhT_sigma_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The inverse sigma of mid frequency bilateral filter.\n Freq of use: high))  */
    float hw_enhT_sigma_val;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_centerPix_wgt),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.01),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The current pixel weight of mid frequency bilateral filter.\n Freq of use: low))  */
    float hw_enhT_centerPix_wgt;
} enh_midBifilt_params_t;

typedef struct enh_detail2strg_curve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(detail2strg_idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT([16,32,64,128,256,512,640,1024]),
        M4_DIGIT_EX(0f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
    Freq of use: high))  */
    float idx[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(detail2strg_val),
        M4_TYPE(f32),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([1, 1, 1, 1, 1, 1, 1, 1]),
        M4_DIGIT_EX(2f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
    Freq of use: high))  */
    float val[8];
} enh_detail2strg_curve_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(global_strg),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,16),
    M4_DEFAULT(1),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The global strength of enhance.\n Freq of use: high))  */
    float hw_enhT_global_strg;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_detail2Strg_en),
    M4_GROUP_CTRL(detail2strg_en_group),
    M4_TYPE(bool),
    M4_DEFAULT(0),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The enable of using luma as index to control the enhance strength.\n
    Freq of use: high))  */
    bool hw_enhT_detail2Strg_en;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_detail2Strg_curve),
    M4_GROUP(detail2strg_en_group),
    M4_TYPE(struct),
    M4_UI_MODULE(curve_ui),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
    Freq of use: high))  */
    enh_detail2strg_curve_t hw_enhT_detail2Strg_curve;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_luma2Strg_en),
    M4_GROUP_CTRL(luma2strg_en_group),
    M4_TYPE(bool),
    M4_DEFAULT(0),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The enable of using luma as index to control the enhance strength.\n
    Freq of use: high))  */
    bool hw_enhT_luma2Strg_en;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_luma2Strg_val),
    M4_GROUP(luma2strg_en_group),
    M4_TYPE(f32),
    M4_UI_MODULE(drc_curve),
    M4_SIZE_EX(1,17),
    M4_RANGE_EX(0,1),
    M4_DEFAULT([1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]),
    M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
    M4_DIGIT_EX(2f6b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
    Freq of use: high))  */
    float hw_enhT_luma2Strg_val[17];
} enh_strg_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(guideImg_iir),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All iir filter params corresponded with iso array.\n
    Freq of use: low))  */
    enh_guideImg_iir_params_t guideImg_iir;
    /* M4_GENERIC_DESC(
    M4_ALIAS(loBifilt),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All low frequency bilateral filter params corresponded with iso array.\n
    Freq of use: high))  */
    enh_loBifilt_params_t loBifilt;
    /* M4_GENERIC_DESC(
    M4_ALIAS(midBifilt),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All mid frequency bilateral filter params corresponded with iso array.\n
    Freq of use: high))  */
    enh_midBifilt_params_t midBifilt;
    /* M4_GENERIC_DESC(
    M4_ALIAS(strg),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All strength params corresponded with iso array.\n
    Freq of use: high))  */
    enh_strg_params_t strg;
} enh_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    enh_params_dyn_t dyn;
} enh_param_t;

#endif
