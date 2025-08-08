/*
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

#ifndef _RK_AIQ_PARAM_GIC21_H_
#define _RK_AIQ_PARAM_GIC21_H_

#define GIC_SIGMACURVE_SEGMENT_MAX         15

typedef struct gic_strongEdg_darkArea_s {
    /* M4_GENERIC_DESC(
       M4_ALIAS(sw_regdarkthre),
       M4_TYPE(u16),
       M4_SIZE_EX(1,1),
       M4_RANGE_EX(0,511),
       M4_DEFAULT(120),
       M4_DIGIT_EX(0),
       M4_FP_EX(0,11,0),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES(The lower threshold of luma in the dark areas of the image\n
       Freq of use: low))  */
    //@reg: sw_regdarkthre
    //@para: dark_thre
    uint16_t hw_gicT_darkArea_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regdarktthrehi),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,511),
        M4_DEFAULT(240),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The upper threshold of luma in the dark areas of the image\n
        Freq of use: low))  */
    //@reg: sw_regdarktthrehi
    //@para: dark_threHi
    uint16_t hw_gicT_darkArea_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regkgrad1dark),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32768.0),
        M4_DEFAULT(64.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min vaule of kGrad1.\n
        Freq of use: high))  */
    //@reg: sw_regkgrad1dark/32
    //@para: k_grad1_dark
    float hw_gicT_hiGrad2EdgTh_ratio;
    /* M4_GENERIC_DESC(
         M4_ALIAS(k_grad2_dark),
         M4_TYPE(f32),
         M4_SIZE_EX(1,1),
         M4_RANGE_EX(0,32768.0),
         M4_DEFAULT(2.0),
         M4_DIGIT_EX(2f4b),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(0),
         M4_NOTES(The min vaule of kGrad2.\n
         Freq of use: high))  */
    //@reg: sw_regkgrad2dark / 32
    //@para: k_grad2_dark
    float hw_gicT_loGrad2EdgTh_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regmingradthrdark1),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min vaule of MinGrad1.\n
        Freq of use: low))  */
    //@reg: sw_regmingradthrdark1
    //@para: min_grad_thr_dark1
    uint16_t hw_gicT_hiEdgTh_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regmingradthrdark2),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min vaule of MinGrad2.\n
        Freq of use: low))  */
    //@reg: sw_regmingradthrdark2
    //@para: min_grad_thr_dark2
    uint16_t hw_gicT_loEdgTh_minLimit;
} gic_strongEdg_darkArea_t ;

typedef struct gic_strongEdg_lightArea_s {
    /* M4_GENERIC_DESC(
         M4_ALIAS(sw_regkgrad1),
         M4_TYPE(f32),
         M4_SIZE_EX(1,1),
         M4_RANGE_EX(0,32768.0),
         M4_DEFAULT(64.0),
         M4_DIGIT_EX(2f4b),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(0),
         M4_NOTES(The max vaule of kGrad1.\n
         Freq of use: high))  */
    //@reg: sw_regkgrad1 / 32
    //@para: k_grad1
    float hw_gicT_hiGrad2EdgTh_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regkgrad2),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32768.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min vaule of kGrad2.\n
        Freq of use: high))  */
    //@reg: sw_regkgrad2 /32
    //@para: k_grad2_dark
    float hw_gicT_loGrad2EdgTh_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regmingradthr1),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min vaule of MinGrad1.\n
        Freq of use: low))  */
    //@reg: sw_regmingradthr1
    //@para: min_grad_thr1
    uint16_t hw_gicT_hiEdgTh_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regmingradthr2),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min vaule of MinGrad2.\n
        Freq of use: low))  */
    //@reg: sw_regmingradthr2
    //@para: min_grad_thr2
    uint16_t hw_gicT_loEdgTh_minLimit;
} gic_strongEdg_lightArea_t ;

typedef struct gic_weakEdg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_noise_base),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The lower limit of the gradient threshold.\n
        Freq of use: High))  */
    //@reg: sw_noise_base
    //@para: min_busy_thre
    uint16_t hw_gicT_estNs_offset;
} gic_weakEdg_t;

typedef struct gic_textDct_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regminbusythre),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(40),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The lower limit of the gradient threshold.\n
        Freq of use: High))  */
    //@reg: sw_regminbusythre
    //@para: min_busy_thre
    uint16_t hw_gicT_textTh_minLimit;
} gic_textDct_t;

typedef struct gic_gicProc_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseCurve_0),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1023.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_FP_EX(0,0,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The coeff0 of sigma fomula.\n
        Freq of use: low))  */
    //@reg: sw_sigma_y0 ~ sw_sigma_y14
    //@para: noiseCurve_0
    float hw_gicT_sigma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseCurve_1),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1023.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_FP_EX(0,0,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The coeff0 of sigma fomula.\n
        Freq of use: low))  */
    //@reg: sw_sigma_y0 ~ sw_sigma_y14
    //@para: noiseCurve_1
    float hw_gicT_sigma_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_noise_scale),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,127),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,7,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale of sigma value for max diff calculation.\n
        Freq of use: low))  */
    //@reg: sw_noise_scale
    //@para: NoiseScale / 32
    uint8_t hw_gicT_gDiffSoftThd_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_diff_clip),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32767),
        M4_DEFAULT(32767),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,15,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit value for diff_gb.\n
        Freq of use: high))  */
    //@reg: sw_diff_clip
    //@para: diff_clip
    uint16_t hw_gicT_gDiff_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_regstrengthglobal_fix),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,8,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(adjust weight of the diff_gb value for compensation.\n
        Freq of use: high))  */
    //@reg: sw_regstrengthglobal_fix
    //@para: globalStrength
    float hw_gicT_correctGb_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gr_ratio),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,2,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(control the weight of diff_gb compensation of gr pixel on bayer position.\n
        Freq of use: high))  */
    //@reg: sw_gr_ratio
    //@para: gr_ration
    uint8_t hw_gicT_correctGb2Gr_ratio;
} gic_gicProc_t ;

typedef struct gic_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(strongEdg_darkArea),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_strongEdg_darkArea_t strongEdg_darkArea;
    /* M4_GENERIC_DESC(
        M4_ALIAS(strongEdg_lightArea),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_strongEdg_lightArea_t strongEdg_lightArea;
    /* M4_GENERIC_DESC(
        M4_ALIAS(weakEdg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_weakEdg_t weakEdg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(textDct),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_textDct_t textDct;
    /* M4_GENERIC_DESC(
        M4_ALIAS(compensation),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_gicProc_t gicProc;
} gic_params_dyn_t;

typedef struct {
    int a;
} gic_params_static_t;

typedef struct gic_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_params_dyn_t dyn;
} gic_param_t;

#endif

