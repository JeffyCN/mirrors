/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_TYPE_CCM22_H_
#define _RK_AIQ_TYPE_CCM22_H_

#define CCM_SATCURVE_DOT_NUM                    17
#define CCM_YCURVE_DOT_NUM                      18

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_facMax_minThred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,11),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The lower threshold for taking the max factor0 is 2^hw_ccmT_facMax_minThred.\n
        Freq of use: low))  */
    //reg:sw_ccm_bound_bit
    uint8_t hw_ccmT_facMax_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_facMax_maxThred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,11),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The upper threshold for taking the max factor0 is 4095 - 2^hw_ccmT_facMax_minThred.\n
        Freq of use: low))  */
    //reg:sw_ccm_right_bit
    uint8_t hw_ccmT_facMax_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_loY2Alpha_fac0),
        M4_TYPE(u16),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The factor curve related to y when y < 2^hw_ccmT_facMax_minThred. Ascending order.\n
        Freq of use: high))  */
    //reg:sw_ccm_alp_y0~17
    uint16_t hw_ccmT_loY2Alpha_fac0[CCM_YCURVE_DOT_NUM];    
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_hiY2Alpha_fac0),
        M4_TYPE(u16),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The factor curve related to (4095 - y) when (4095 - y) < 2^hw_ccmT_facMax_maxThred. Ascending order. \n
        Freq of use: high))  */
    //reg:sw_ccm_alp_y0~17
    uint16_t hw_ccmT_hiY2Alpha_fac0[CCM_YCURVE_DOT_NUM];
} ccm_ccmAlpha_yFac_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_satIdx_maxLimit),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The upper threshold of color diff for taking the min factor1 is -hw_ccmT_satIdx_maxLimit. \n
        Namely, sat2Alpha_fac1 = factor1[16] when color diff <= -hw_ccmT_satIdx_maxLimit.\n
        Freq of use: low))  */
    //reg:sw_ccm_hf_low
    uint8_t hw_ccmT_satIdx_maxLimit; 
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_facMax_thred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The lower threshold of color diff for taking the max factor1. \n
        Namely, sat2Alpha_fac1 = factor1[0] when color diff >= hw_ccmT_facMax_thred.\n
        Freq of use: low))  */
    //reg:sw_ccm_hf_up
    uint8_t hw_ccmT_facMax_thred;    
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_satIdx_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,63),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3f8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The scale factor of color diff used as the index for factor1 table. \n
        Only takes effect in [-hw_ccmT_satIdx_maxLimit, hw_ccmT_facMax_thred]. \n
        Freq of use: low))  */
    //reg:sw_ccm_hf_scale
    float hw_ccmT_satIdx_scale; 
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_sat2Alpha_fac1),
        M4_TYPE(u16),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT(1024),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The factor curve related to color diff when color diff in [-hw_ccmT_satIdx_maxLimit, hw_ccmT_facMax_thred]. \n
        Descending order.\n
        Freq of use: low)) */
    //reg:sw_ccm_hf_factor0~16
    uint16_t hw_ccmT_sat2Alpha_fac1[CCM_SATCURVE_DOT_NUM];
} ccm_ccmAlpha_satFac_t;

typedef struct {
   /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmC_matrix_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(3,3),
        M4_RANGE_EX(-8,7.992),
        M4_DEFAULT([1, 0, 0, 0, 1, 0, 0, 0, 1]),
        M4_DIGIT_EX(3f7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The ccm matrix coeff.\nFreq of use: high))  */
    //reg:sw_ccm_coeff0_r~sw_ccm_coeff2_b
    float hw_ccmC_matrix_coeff[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmC_matrix_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(-2048,2047),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The ccm offset.\nFreq of use: low))  */
    //reg:sw_ccm_offset_r~sw_ccm_offset_b
    float hw_ccmC_matrix_offset[3];
} ccm_matrix_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_enhance_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(enhCcm_en_group),
        M4_NOTES(The ccm color enhance adjust enable.\nFreq of use: low))  */
    //reg:sw_ccm_enh_adj_en
    bool hw_ccmT_enhance_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmT_enhanceRat_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15.9986),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4f10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(enhCcm_en_group),
        M4_NOTES(The maximum compression ratio limit for dynamic range in enhance mode.\n
        Freq of use: low))  */
    //reg:sw_ccm_color_enh_rat_max
    float hw_ccmT_enhanceRat_maxLimit;
} ccm_enhance_t;

typedef struct {
   /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ccmCfg_rgb2y_coeff),
        M4_TYPE(u16),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,128),
        M4_DEFAULT([38, 75, 15]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The coefficient values of RGB2Y calculation.\nFreq of use: low))  */
    //reg:sw_ccm_coeff0_y~sw_ccm_coeff2_y
    uint16_t hw_ccmCfg_rgb2y_coeff[3];
} ccm_param_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ccmAlpha_yFac),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The factor0 curve of alpha related to y.)) */
    ccm_ccmAlpha_yFac_t ccmAlpha_yFac;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ccmAlpha_satFac),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The factor1 curve of alpha related to color diff.)) */
    ccm_ccmAlpha_satFac_t ccmAlpha_satFac;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enhance),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The ccm enhance params.)) */
    ccm_enhance_t enhance;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ccMatrix),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The ccm matrix)) */
    ccm_matrix_t ccMatrix;
} ccm_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ccm_param_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ccm_param_dyn_t dyn;
} ccm_param_t;

#endif

