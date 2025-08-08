/*
 * rk_aiq_param_cnr32.h
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

#ifndef _RK_AIQ_PARAM_CNR32_H_
#define _RK_AIQ_PARAM_CNR32_H_

#define CNR_ISO_STEP_MAX                  13
#define CNR_NRPIXGAINMAP_SEGMENT_MAX      13

typedef enum cnr_nrpixgain_mode_e {
    /*
    reg:sw_cnr_exgain_bypass == 0
    */
    CnrPG_Local_Mode = 0,
    /*
    reg:sw_cnr_exgain_bypass == 1
    */
    CnrPG_Global_Mode = 1
} cnr_nrpixgain_mode_t;

typedef enum cnr_loDs_mode_e {
    /*
    sw_cnr_thumb_mode == 0
    */
    LoDs_2x2_Mode   = 0,
    /*
    sw_cnr_thumb_mode == 1
    */
    LoDs_4x4_Mode  = 1,
    /*
    sw_cnr_thumb_mode == 2
    */
    LoDs_8x6_Mode  = 2
} cnr_loDs_mode_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_nrPG_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(cnr_nrpixgain_mode_t),
        M4_DEFAULT(CnrPG_Local_Mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of pix gain used for cnr. Reference enum types.Freq of use: low))  */
    // reg: sw_cnr_exgain_bypass
    cnr_nrpixgain_mode_t hw_cnrT_nrPG_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_nrPG_globalGainScale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Enlarge the gloabl gain.Gain = (sw_cnr_global_gain_alpha * sw_cnr_global_gain + (8 - sw_cnr_global_gain_alpha) * local_gain) >> 3Higher the value, the higher global gain value.Freq of use: low))  */
    // reg: sw_cnr_global_gain
    float hw_cnrT_nrPG_globalGainScale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_nrPG_globalGainAlpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Control the ratio of global gain and local gain effects.Gain = (sw_cnr_global_gain_alpha * sw_cnr_global_gain + (8 - sw_cnr_global_gain_alpha) * local_gain) >> 3Higher the value, the higher global gain effects.Freq of use: low))  */
    // reg: sw_cnr_global_gain_alpha
    float hw_cnrT_nrPG_globalGainAlpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_nrPG_localGainScale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(To enlarge the extra gain, range from 8 to 128.Higher the value, the higher local gain effects.Freq of use: low))  */
    // reg: sw_cnr_gain_iso
    float hw_cnrT_nrPG_localGainScale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_nrPG_iirGlobalGainScale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Enlarge the iir gloabl gain.Higher the value, the higher iir global gain effects.Freq of use: low))  */
    // reg: sw_cnr_iir_global_gain
    float hw_cnrT_nrPG_iirGlobalGainScale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_nrPG_iirGlobalGainAlpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(8.0,8.0),
        M4_DEFAULT(8.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(IIR gain alpha control. This value can only be 8 in current version.Freq of use: low))  */
    // reg: sw_cnr_iir_gain_alpha
    float hw_cnrT_nrPG_iirGlobalGainAlpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_nrPG_gainAdj),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,4.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(cnr gain adjust.Freq of use: low))  */
    // reg: sw_cnr_sigma_y0 ~ sw_cnr_sigma_y16
    float hw_cnrT_nrPG_gainAdj[CNR_NRPIXGAINMAP_SEGMENT_MAX];
} cnr_nrpixgain_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cnrT_loDs_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(cnr_loDs_mode_t),
        M4_DEFAULT(LoDs_2x2_Mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cnr_loDs_mode_t sw_cnrT_loDs_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_loThumbBiFilt_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.005),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Control the sigma Y used in lbf1x7 module.Freq of use: high))  */
    // reg: sw_cnr_thumb_sigma_y
    float hw_cnrT_loThumbBiFilt_sigma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_loThumbBiFilt_spatWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([1.0,1.0,1.0,1.0]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The distance weight in lbf1x7 module.Freq of use: low))  */
    // reg: sw_lbf1x7_weit_d0-d3
    float hw_cnrT_loThumbBiFilt_spatWgt[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_thumbBiFilt_Alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Control the weight of in_data in lbf1x7 module out_data..Freq of use: high))  */
    // reg: sw_cnr_thumb_bf_ratio
    float hw_cnrT_loThumbBiFilt_Alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_cnrT_loChromaFilt_uvgain),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.333),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Control the sigma uv used in lbf1x7 module and IIR filter.Freq of use: high))  */
    // reg: sw_cnr_thumb_sigma_c  sw_cnr_iir_uvgain
    float sw_cnrT_loChromaFilt_uvgain;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_loIIRFilt_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0050),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The strength of IIR filter.Freq of use: high))  */
    // reg: sw_cnr_iir_strength  sw_cnr_exp_shift
    float hw_cnrT_loIIRFilt_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_loIIRFilt_uvClip),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Clip out weight sum in IIR filter.Freq of use: low))  */
    // reg: sw_cnr_iir_uv_clip
    float hw_cnrT_loIIRFilt_wgtClip;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_loIIRFilt_minWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0313),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(IIR filter min wei_i to anti the chroma ghost.Freq of use: low))  */
    // reg: sw_cnr_chroma_ghost
    float hw_cnrT_loIIRFilt_minWgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_lo_wgtSlope),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8.0),
        M4_DEFAULT(0.7),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The delete number of weit_i in IIR filter.Freq of use: low))  */
    // reg: sw_cnr_wgt_slope
    float hw_cnrT_lo_wgtSlope;
} cnr_lo_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_loThumbBiFilt_spatialWgt),
        M4_TYPE(u8),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,127),
        M4_DEFAULT([36,24,16,6,4,1]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Gaussian filter coefficient.Freq of use: low))  */
    // reg: sw_cnr_gaus_coe0-coe5
    uint8_t hw_cnrT_hiGausFilt_coef[6];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_hiGausFilt_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0313),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The src_data weight of gaus_out_data.The higher value, the bigger ratio of gaus_out data.Freq of use: high))  */
    // reg: sw_cnr_gaus_ratio
    float hw_cnrT_hiGausFilt_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_hiBiFilt_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0038),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The sigma of bf3x3 module.The higher value, the bigger strength of bf3x3 reduce noise.Freq of use: high))  */
    // reg: sw_cnr_sigma_r
    float hw_cnrT_hiBiFilt_sigma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_hiBiFilt_uvGain),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8.0),
        M4_DEFAULT(3.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(UV gain of bf3x3 module.The higher value, the bigger uv gain of bf3x3 module.Freq of use: high))  */
    // reg: sw_cnr_uv_gain
    float hw_cnrT_hiBiFilt_uvGain;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_hiBiFilt_centreWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0625),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The in_data ratio in bf3x3 bilateral filter.The higher value, the bigger ratio of in_data.Freq of use: high))  */
    // reg: sw_cnr_bf_ratio
    float hw_cnrT_hiBiFilt_centreWgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_hiBiFilt_minWgtClip),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0078),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The min weight in bf3x3 module.The higher value, the bigger ratio of bf3x3 out_data.Freq of use: high))  */
    // reg: sw_cnr_bf_wgt_clip
    float hw_cnrT_hiBiFilt_minWgtClip;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_hiBiFilt_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The in_data weight of bf3x3 out_data.The higher value, the bigger ratio of bf3x3 out_data.Freq of use: high))  */
    // reg: sw_cnr_global_alpha
    float hw_cnrT_hiBiFilt_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_saturation_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(The adjust saturation ratio in bf3x3 module.The higher value, the higger saturation retain in low saturation part..Freq of use: low))  */
    // reg: sw_cnr_adj_ratio
    float hw_cnrT_saturation_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_saturation_offset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,511),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The adjust saturation offset in bf3x3 module.The higher value, the lower saturation retain in low saturation part.Freq of use: low))  */
    // reg: sw_cnr_adj_offset
    uint16_t hw_cnrT_saturation_offset;
} cnr_hi_param_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(nrpixgainParam),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cnr_nrpixgain_param_dyn_t nrpixgainParam;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loParam),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    cnr_lo_param_dyn_t loParam;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiParam),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_hi_param_dyn_t hiParam;
} cnr_params_dyn_t;

typedef struct {
    int a;
} cnr_params_static_t;


typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    cnr_params_dyn_t dyn;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    cnr_params_static_t sta;
} cnr_param_t;

#endif
