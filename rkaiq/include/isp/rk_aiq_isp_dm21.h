/*
 * Copyright (c) 2023 Rockchip Eletronics Co., Ltd.
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

#ifndef _RK_AIQ_PARAM_DM21_H_
#define _RK_AIQ_PARAM_DM21_H_

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_loDrctFlt_coeff),
        M4_TYPE(s8),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(-16,15),
        M4_DEFAULT([-2,2,-4,4]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hp filter kernel to get high-freq gradient in h/v direction. Fixed. Freq of use: low))  */
    int8_t hw_dmT_loDrctFlt_coeff[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_hiDrctFlt_coeff),
        M4_TYPE(s8),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(-16,15),
        M4_DEFAULT([-2,2,-4,4]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The hp filter kernel to get low-freq gradient in h/v direction. Fixed. Freq of use: low))  */
    int8_t hw_dmT_hiDrctFlt_coeff[4];
} dm_ginterp_param_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_gInterpClip_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable g-channel clip using its neighboring pixels. Freq of use: high))  */
    bool hw_dmT_gInterpClip_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_gInterpSharpStrg_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The offset to contrl fusion weight of color-channel gradient for g interpolation, related to sharp strength. Freq of use: high))  */
    uint16_t hw_dmT_gInterpSharpStrg_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_gInterpSharpStrg_maxLimit),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,63),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The max fusion weight of color-channel gradient for g interpolation, related to sharp strength. Freq of use: high))  */
    uint8_t hw_dmT_gInterpSharpStrg_maxLim;
} dm_ginterp_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_drct_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(13),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset to control direction-detect sensitivity for g interpolation. Higher the value, lower the sensitivity. Freq of use: high))  */
    uint16_t hw_dmT_drct_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_drctMethod_thred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(13),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The thredhold to judge whether there is obvious directionality between hi or lo-freq gradient of g-channel . Freq of use: high))  */
    uint8_t hw_dmT_drctMethod_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_hiDrct_thred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15),
        M4_DEFAULT(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The thredhold to judge whether high-freq gradient of g-channel has directionality. Higher the value, the harder to be directional. Freq of use: high))  */
    uint8_t hw_dmT_hiDrct_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_loDrct_thred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15),
        M4_DEFAULT(6),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The thredhold to judge whether low-freq gradient of g-channel has not directionality. Higher the value, the harder to be directional. Freq of use: high))  */
    uint8_t hw_dmT_loDrct_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_hiTexture_thred),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15),
        M4_DEFAULT(8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The thredhold to control sensitivity of high-freq texture detection. Higher the value, lower the sensitivity. Freq of use: high))  */
    uint8_t hw_dmT_hiTexture_thred;
} dm_gdrctalpha_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_gOutlsFlt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable filtering on g-channel to eliminate noise pixels. Freq of use: high))  */
    bool hw_dmT_gOutlsFlt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_gOutlsFltRange_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2047),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The offset to control the value range of g-channel, used in filtering . Freq of use: high))  */
    uint16_t hw_dmT_gOutlsFltRange_offset;
} dm_gflt_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrLoGuideLpf_coeff),
        M4_TYPE(s8),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,127),
        M4_DEFAULT([64,32,16]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The lp filter kernel that used to generate guide plane for color-channel diff plane. Fixed. Freq of use: low))  */
    int8_t hw_dmT_cnrLoGuideLpf_coeff[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrAlphaLpf_coeff),
        M4_TYPE(s8),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,127),
        M4_DEFAULT([64,32,16]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The lp filter kernel that used to filt the fusion weight map of color-channel diff plane. Fixed. Freq of use: low))  */
    int8_t hw_dmT_cnrAlphaLpf_coeff[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrPreFlt_coeff),
        M4_TYPE(s8),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,127),
        M4_DEFAULT([64,32,16]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The lp filter, as a prefilter, used on color-channel diff plane. Fixed. Freq of use: low))  */
    int8_t hw_dmT_cnrPreFlt_coeff[3];
} dm_cflt_param_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrFlt_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable flitering on color-channel diff planes for eliminating demosaic artifacts. Freq of use: high))  */
    bool hw_dmT_cnrFlt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrLogGuide_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The offset to control log-domain transform. Freq of use: low))  */
    uint16_t hw_dmT_cnrLogGuide_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dmT_cnrLoFlt_vsigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0313),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The sigma of color diff plane to guide pixel lp filter,related to the lpf strength.Higher the value, higher the lpf strength.  Freq of use: high))  */
    float sw_dmT_cnrLoFlt_vsigma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dmT_cnrLoFltWgt_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15.875),
        M4_DEFAULT(7.25),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The max limit of current pixel weight,related to the lpf strength. Higher the value, higher the strength. Freq of use: low))  */
    float sw_dmT_cnrLoFltWgt_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dmT_cnrLoFltWgt_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0.98),
        M4_DEFAULT(0.03),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The min limit of neighbor pixel weight,related to the lpf strength. Higher the value, higher the strength. Freq of use: low))  */
    float sw_dmT_cnrLoFltWgt_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dmT_cnrLoFltWgt_slope),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,31.992),
        M4_DEFAULT(0.7),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The slope to control the scale coefficient of pixel weight. Higher the value, lower the lpf strength. Freq of use: low))  */
    float sw_dmT_cnrLoFltWgt_slope;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dmT_cnrHiFlt_vsigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0266),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The sigma of color-channel diff plane, related to hp filter strength. Higher the value, higher the hpf strength. Freq of use: high))  */
    float sw_dmT_cnrHiFlt_vsigma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrHiFltWgt_minLimit),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,127),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(The min limit of guide pixel weight for hp filter. Higher the value, higher the hpf strength. Freq of use: high))  */
    uint8_t hw_dmT_cnrHiFltWgt_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrHiFltCur_wgt),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,127),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The weight of current pixel for hp filter. Higher the value, lower the hpf strength. Freq of use: high))  */
    uint8_t hw_dmT_cnrHiFltCur_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrLogGrad_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(128),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(The offset to control sensitivity of moire and edge detection. Higher the value, lower the sensitivity. Freq of use: high))  */
    uint16_t hw_dmT_cnrLogGrad_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrMoireAlpha_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(The offset to control the fusion weight of Moire on filted color-channel diff plane. Higher the value, lower the weight. Freq of use: high))  */
    uint16_t hw_dmT_cnrMoireAlpha_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dmT_cnrMoireAlpha_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023.999),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(The scaling factor to control the fusion weight of Moire on filted color-channel diff plane. Higher the value, higher the weight. Freq of use: high))  */
    float sw_dmT_cnrMoireAlpha_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_dmT_cnrEdgeAlpha_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(The offset to control the fusion weight of edge on filted color-channel diff plane. Higher the value, lower the weight. Freq of use: high))  */
    uint16_t hw_dmT_cnrEdgeAlpha_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_dmT_cnrEdgeAlpha_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023.999),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(The scaling factor to control the fusion weight of edge on filted color-channel diff plane. Higher the value, higher the weight. Freq of use: high))  */
    float sw_dmT_cnrEdgeAlpha_scale;
} dm_cflt_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(g_interp_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The dynamic params of g-channel interplotion))  */
    dm_ginterp_param_static_t gInterp;
    /* M4_GENERIC_DESC(
        M4_ALIAS(c_filter_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dynamic params of color-channel filter))  */
    dm_cflt_param_static_t cFlt;
} dm_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(g_interp_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The dynamic params of g-channel interplotion))  */
    dm_ginterp_param_dyn_t gInterp;
    /* M4_GENERIC_DESC(
        M4_ALIAS(g_drctAlpha_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dynamic params of g-channel direct alpha))  */
    dm_gdrctalpha_param_dyn_t gDrctAlpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(g_outlsFlt_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The dynamic params of g-channel filter))  */
    dm_gflt_param_dyn_t gOutlsFlt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(c_filter_dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The dynamic params of color-channel filter))  */
    dm_cflt_param_dyn_t cFlt;
} dm_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of demosaic module))  */
    dm_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    dm_params_dyn_t dyn;
} dm_param_t;

#define RK_DM21_FIX_BIT_INT_TO_FLOAT      7
#define RK_DM21_FIX_BIT_LOG2              12

#define RK_DM21_FIX_BIT_IIR_WGT_CLIP      3
#define RK_DM21_FIX_BIT_IIR_GHOST         6

#define RK_DM21_FIX_BIT_ALPHA_SCALE       10
#define RK_DM21_FIX_BIT_EDGE_SCALE        10

#define RK_DM21_FIX_BIT_INV_SIGMA         16
#define RK_DM21_FIX_BIT_INV_BF_SIGMA      16

#define RK_DM21_FIX_BIT_WGT_SLOPE         7

#endif
