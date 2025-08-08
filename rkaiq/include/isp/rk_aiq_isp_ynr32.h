/*
 * rk_aiq_param_ynr32.h
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

#ifndef _RK_AIQ_ISP_YNR32_H_
#define _RK_AIQ_ISP_YNR32_H_

#define YNR_ISO_STEP_MAX                13
#define YNR_SIGMACURVE_SEGMENT_MAX         17
#define YNR_NLM_COEF_MAX                   7
#define YNR_RNR_STRENGTH_SEGMENT_MAX       17

typedef struct ynr_sigmacurve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    uint16_t idx[YNR_SIGMACURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(val),
        M4_TYPE(u16),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    uint16_t val[YNR_SIGMACURVE_SEGMENT_MAX];
} ynr_sigmacurve_t;

typedef enum {
    /*
    reg: (sw_ynr_flt1x1_bypass == 1) && (sw_ynr_flt1x1_bypss_sel == 0)
    */
    loNrGausFilt_mode = 0,
    /*
    reg: (sw_ynr_flt1x1_bypass == 1) && (sw_ynr_flt1x1_bypss_sel == 1)    */
    loNrBifilt3_mode = 1,
	/*
    reg: (sw_ynr_flt1x1_bypass == 1) && (sw_ynr_flt1x1_bypss_sel == 2)
    */
    loNrBifilt3GuideImg_mode = 2,
    /*
    reg: (sw_ynr_flt1x1_bypass == 0)
    */
    loNrEnd_mode = 3,
} ynr_loNROutSel_mode_t;

typedef enum ynr_bifilt5GuideImg_mode_e {
    // @reg: sw_ynr_thumb_mix_cur_en == 0
    ynr_guideImg_preFrmOnly_mode = 0,
    // @reg: sw_ynr_thumb_mix_cur_en == 1
    ynr_guideImg_preCurMix_mode = 1
} ynr_bifilt5GuideImg_mode_t;




typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_bifilt3x3_bypass),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable the bifilt3 for lo freq noise remove.Turn on by setting this bit to 1. Freq of use: low))  */
    // reg: sw_ynr_bft3x3_bypass;
    bool hw_ynrT_bifilt3_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_bifilt3x3_spatialStrg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The strength of low pass filter for generate guide image for bifilter3.The value higher, the strength of lpf is higher. Freq of use: high))  */
    // reg: sw_ynr_low_gauss1_coeff0~2
    float sw_ynrT_guideImgLpf_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_loPeakSupress_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The alpha value of lpf output pix in operation with input pix. The higher the value, the strength of lpf is higher. Freq of use: high))  */
    // reg: sw_ynr_low_peak_supress
    float hw_ynrT_guideImgLpf_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_bifilt3x3_inv),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,32.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The divisor of bifilter range sigma that will affect the strength of bifilter.The higher the value, the lower the strength of filter.Freq of use: high))  */
    // reg: sw_ynr_low_bf_inv0
    float hw_ynrT_rgeSgm_div;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_bifilt3x3_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Use for original Ydata merge with bft3x3 output data for soft threshhold filter.The higher the value, the bigger the strength of reducing noise.Freq of use: high))  */
    // reg: sw_ynr_low_bi_weight
    float hw_ynrT_bifiltOut_alpha;
} ynr_loNrBifilt3_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_biifilt5_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Enable the bifilt5 for lo freq noise remove. Turn on by setting this bit to 1. Freq of use: low))  */
    // reg: sw_ynr_lbft5x5_bypass
    bool hw_ynrT_bifilt5_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_guideImg_mode),
        M4_TYPE(ynr_bifilt5GuideImg_mode_t),
        M4_DEFAULT(ynr_guideImg_preCurMix_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of guide img generation for bifilt5. Freq of use: low))  */
    // reg: sw_ynr_thumb_mix_cur_en;
    ynr_bifilt5GuideImg_mode_t hw_ynrT_guideImg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_loBiFilt5x5_edgeWgtAdj),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1023.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Adjust the edge weight of LBFT5x5 bilateral filter.The higher the value, the smaller the strength of reducing noise.Freq of use: high))  */
    // reg: sw_ynr_low_edge_adj_thresh
    float hw_ynrT_guideImgText_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_rgeSgm_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,32.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(This scale factor of bifilter range sigma that will affect the strength of bifilte.The higher the value, the higher the strength of bifilter. Freq of use: high))  */
    // reg: sw_ynr_low_bf_inv1
    float hw_ynrT_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_spatialSgm_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,128.0),
        M4_DEFAULT(8),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Adjust the edge weight of LBFT5x5 bilateral filter.The higher the value, the smaller the strength of reducing noise.Freq of use: high))  */
    // reg: sw_ynr_low_dist_adj
    float hw_ynrT_spatialSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_centerPix_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The spatial weight of center pixel in filter.The higher the value, the smaller the strength of reducing noise.Freq of use: high))  */
    // reg: sw_ynr_low_center_weight
    float hw_ynrT_centerPix_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_nhoodPixWgt_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The pix which range wgt is less than threshold is not considered as neighborhood pixels. The higher the value, the smaller the strength of reducing noise.Freq of use: high))  */
    // reg: sw_ynr_lbf_weight_thresh
    float hw_ynrT_nhoodPixWgt_thred;
} ynr_loNrBifilt5_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_gausFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Enable gauss filter after the bifilt5 for lo freq module.Turn on by setting this bit to 1.Freq of use: low))  */
    // reg: sw_ynr_lgft3x3_bypass
    bool hw_ynrT_gausFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_gausFilt_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Adjust the gauss filter coeff for lgft3x3.The higher the value, the stronger the strength of reducing noise.Freq of use: high))  */
    // reg: sw_ynr_low_gauss2_coeff0~2
    float sw_ynrT_gausFilt_strg;
} ynr_loNrGausFilt_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_loFilt1x1_bypass),
        M4_TYPE(ynr_loNROutSel_mode_t),
        M4_DEFAULT(loNrEnd_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Bypass soft threshhold filter for lo freq module.Turn on by setting this bit to 1.Freq of use: low))  */
    // reg: sw_ynr_flt1x1_bypass, sw_ynr_flt1x1_bypass_sel
    ynr_loNROutSel_mode_t hw_ynrT_loNROutSel_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_loFilt1x1_softThHold),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,31.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Adjust the threshhold of low filter filter.The higher the value, the higher the strength of reducing noise.Freq of use: high))  */
    // reg: sw_ynr_low_thred_adj
    float hw_ynrT_softThd_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_loBiFilt5x5_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Use for undenoise data merge with lbft5x5 denoise data.The higher the value, the bigger the strength of reducing noise.Freq of use: high))  */
    // reg: sw_ynr_low_weight
    float hw_ynrT_loNr_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrC_luma2LoSigma),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(TODO))  */
    // reg: sw_ynr_luma_points_x0 ~ x16, sw_ynr_lsigma_y0 ~ y16
    ynr_sigmacurve_t hw_ynrC_luma2LoSgm_curve;
} ynr_loNrPost_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_localPixSgm_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.3),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The thresh of local gain.Freq of use: high))  */
    // reg: sw_ynr_local_gainscale
    float hw_ynrT_localPixSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_pixSgmScl_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.3),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The thresh of local gain.Freq of use: high))  */
    // reg: sw_ynr_localgain_adj_thresh
    float hw_ynrT_localPixSgmScl2_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_pixSgm_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The adj strength of local gain.Freq of use: high))  */
    // reg: sw_ynr_localgain_adj
    float hw_ynrT_localPixSgm_scale2;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_glbPixSgm_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The value of global input pix sigma.Freq of use: low))  */
    // reg: sw_ynr_globalgain
    float hw_ynrT_glbPixSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_glbPixSgmAlpha_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The weight of global gain and local gain.The higer the value, the higher weight for global gain.Freq of use: low))  */
    // reg: sw_ynr_globalgain_alpha
    float hw_ynrT_glbPixSgm_alpha;

} ynr_inPixSgm_dyn_t;


typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_nlmFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Enable the NLM filter for hi freq noise.Turn on by setting this bit to 1.Freq of use: low))  */
    // reg: sw_ynr_nlm11x11_bypass
    bool hw_ynrT_nlmFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_radiDist2Wgt_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The nlm filter strength is positively correlated with radiDist2Wgt, pixSgmWgt. Freq of use: low))  */
    float hw_ynrT_radiDist2Wgt_val[YNR_RNR_STRENGTH_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_pixSgmRadiDistWgt_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.61),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The alpha value of the local wgt that consist of pixSgm2Wgt, radiDist2Wgt. The higher the value, nlm sigma is higher and filter strength is higher. Freq of use: low))  */
    // reg: sw_ynr_nlm_hi_gain_alpha
    float hw_ynrT_pixSgmRadiDistWgt_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_nlmSgm_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.0068),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Mininum noise sigma for high freq noise.The higher the value, the bigger the strength of flat area denoise.Freq of use: high))  */
    float hw_ynrT_nlmSgm_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_hiNlmFilt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The scale of nlm sigma.The higher the value, the strength of filter is higher.Freq of use: high))  */
    // reg: sw_ynr_nlm_hi_bf_scale
    float hw_ynrT_nlmSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_hiNlmFilt_wgtOffset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The negative offset of nlm filt range weight.The higher the value, the smaller the strength of high freq reducing noise.Freq of use: high))  */
    // reg: sw_ynr_nlm_weight_offset
    float hw_ynrT_nlmRgeWgt_negOffset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_nlmSpatial_wgt),
        M4_TYPE(u8),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Wgt[0] is center pix weight. Other is neigborhood pix weight))  */
    // reg: sw_ynr_nlm_coe_0~5, sw_ynr_nlm_center_weight
    uint8_t hw_ynrT_nlmSpatial_wgt[YNR_NLM_COEF_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_hiNlmFilt_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The wgt of nlm filted pixel is used in the fusion operation between the  nlm filted pixel and the original pixel. Higher the value, the stronger denoise strength of  filter.Freq of use: high))  */
    float hw_ynrT_nlmOut_alpha;
} ynr_hiNr_dyn_t;



typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNrBifilt3),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ynr_loNrBifilt3_dyn_t loNrBifilt3;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loParam),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_loNrBifilt5_dyn_t loNrBifilt5;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNrPost),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_loNrPost_dyn_t loNrPost;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNrGaus),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_loNrGausFilt_dyn_t loNrGaus;
    /* M4_GENERIC_DESC(
        M4_ALIAS(inPixSgm),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ynr_inPixSgm_dyn_t inPixSgm;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiParam),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    ynr_hiNr_dyn_t hiNr;
} ynr_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrCfg_opticCenterX_coord),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1000,1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES( The x-coordinates of the optical center in the image\n
        (0,0) is the img center. (-1000,-1000) is the img left top corner. (1000,1000) is the img right bottom corner\n
        Freq of use: low))  */
    // @reg: sw_ynr_rnr_center_cooh
    // @para: Center_Mode
    int16_t hw_ynrCfg_opticCenterX_coord;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrCfg_opticCenterY_coord),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1000,1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES( The y-coordinates of the optical center in the image\n
        (0,0) is the img center. (-1000,-1000) is the img left top corner. (1000,1000) is the img right bottom corner\n
        Freq of use: low))  */
    // @reg: sw_ynr_rnr_center_coov
    // @para: Center_Mode
    int16_t hw_ynrCfg_opticCenterY_coord;
} ynr_opticCenterCfg_t;


typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(opticCenter),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ynr_opticCenterCfg_t opticCenter;
} ynr_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ynr_params_dyn_t dyn;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ynr_params_static_t sta;
} ynr_param_t;

#endif
