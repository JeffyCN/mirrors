/*
 * rk_aiq_param_ynr34.h
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

#ifndef _RK_AIQ_PARAM_DRC40_H_
#define _RK_AIQ_PARAM_DRC40_H_

#define DRC_CURVE_LEN       17

typedef enum drc_curveCfg_mode_e {
    drc_cfgCurveDirect_mode = 0,
    drc_cfgCurveCtrlCoeff_mode = 1
} drc_curveCfg_mode_t;

typedef struct drc_preProc_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_toneCurve_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_curveCfg_mode_t),
        M4_DEFAULT(drc_cfgCurveCtrlCoeff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Reference enum types.\n
		Freq of use: low))  */
    drc_curveCfg_mode_t sw_drcT_toneCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_drcGainIdxLuma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(0,1),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
		Freq of use: low))  */
        // @reg: hw_adrc_drcGainIdxLuma_scale
    float hw_drcT_toneCurveIdx_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_compsIdxLuma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,24576),
        M4_DEFAULT(8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
        // @reg: hw_adrc_compsIdxLuma_scale
    float hw_drcT_drcCurveLuma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_luma2drcGain_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(1,8),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the tone curve through luma2DrcGain_val when toneCurve_mode == drc_cfgCurveDirect_mode.
		Freq of use: low))  */
    // @reg: hw_adrc_luma2drcGain_val0~16
    float hw_drcT_luma2ToneGain_val[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_toneCurveK_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,8),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can configure the tone curve through drcGain_maxLimit when toneCurve_mode == drc_cfgCurveCtrlCoeff_mode.
		Freq of use: high))  */
    float sw_drcT_toneCurveK_coeff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_toneGain_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,8),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can configure the tone curve through drcGain_maxLimit when toneCurve_mode == drc_cfgCurveCtrlCoeff_mode.
		Freq of use: high))  */
    float sw_drcT_toneGain_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_filtLuma_softThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,8),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    // @reg: hw_adrc_filtLuma_softThred
    float hw_drcT_lpfSoftThd_thred;
} drc_preProc_dyn_t;

typedef struct drc_bifilt_guideDiff_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_thumb_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_adrc_thumb_maxLimit
    float hw_drcT_guideLuma_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_thumbThred_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    // reg: hw_adrc_thumbThred_en;
    bool hw_drcT_guideDiffLmt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_thumbThred_neg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // @reg: hw_adrc_thumbThred_neg
    float hw_drcT_guideDiff_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_gdDiffMaxLut_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_curveCfg_mode_t),
        M4_DEFAULT(drc_cfgCurveCtrlCoeff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Reference enum types.
        Freq of use: low))  */
    drc_curveCfg_mode_t sw_drcT_gdDiffMaxLut_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_thumb_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,16),
        M4_DEFAULT(1.0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_adrc_thumb_scale
    float hw_drcT_maxLutIdx_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_thumbDiff2thred_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,2.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the guide diff maxLimit LUT through guideLuma2DiffMax_curve when maxCurve_mode == drc_cfgCurveDirect_mode.
        Freq of use: low))  */
    // @reg: hw_adrc_thumbDiff2thred_val0~16
    float hw_drcT_gdLuma2DiffMax_lut[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_adrc_thumbThred_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,2),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the guide diff maxLimit LUT  through maxCurveCreate_xxx when guideDiffMaxLmt_mode == drc_cfgCurveCtrlCoeff_mode.
        Freq of use: high))  */
    float sw_drcT_maxLutCreate_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_adrc_thumbThred_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,2),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the guide diff maxLimit LUT  through maxCurveCreate_xxx when guideDiffMaxLmt_mode == drc_cfgCurveCtrlCoeff_mode.
        Freq of use: high))  */
    float sw_drcT_maxLutCreate_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_adrc_thumbThred_slope),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,50),
        M4_DIGIT_EX(2f8),
        M4_DEFAULT(10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the guide diff maxLimit LUT  through maxCurveCreate_xxx when guideDiffMaxLmt_mode == drc_cfgCurveCtrlCoeff_mode.
        Freq of use: high))  */
    float sw_drcT_maxLutCreate_slope;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_adrc_thumbThred_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.35),
        M4_DIGIT_EX(2f8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the guide diff maxLimit LUT  through maxCurveCreate_xxx when guideDiffMaxLmt_mode == drc_cfgCurveCtrlCoeff_mode.
        Freq of use: high))  */
    float sw_drcT_maxLutCreate_offset;
} drc_bifilt_guideDiff_t;

typedef struct drc_bifilt_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_loRange_invSigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.0078),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The range sigma for bilateral filtering using a low-frequency guidance image.
        The higher the value, the higher the strength of filter.
        Freq of use: high))  */
    //reg: hw_adrc_loRange_invSigma
    float hw_drcT_loRgeSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_hiRange_invSigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.0078),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The range sigma for bilateral filtering using a mid-frequency guidance image.
        The higher the value, the higher the strength of filter.
        Freq of use: high))  */
    //reg: hw_adrc_hiRange_invSigma
    float hw_drcT_midRgeSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_wgtOffset),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.0078),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The negative offset for range weight of bilateral filtering
        Freq of use: low))  */
    //reg: hw_adrc_bifilt_wgtOffset
    uint8_t hw_drcT_rgeWgt_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_hiWgt),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(In the fusion operation of intermediate and low frequencies, the fusion weight value of intermediate frequencies. 
        TODO.
        Freq of use: high))  */
    //reg: hw_adrc_bifilt_hiWgt
    uint8_t hw_drcT_midWgt_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_curPixelWgt),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The spatial weight of center pixel in filter. 
        TODO.
        Freq of use: high))  */
    //reg: hw_adrc_bifilt_curPixelWgt
    uint8_t hw_drcT_centerPixel_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_wgt),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(1f4),,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The wgt of bifilted pixel is used in the fusion operation between the pre bifilted pixel and the original pixel.
        Higher the value, the stronger denoise strength of pre bilateral filter.
        Freq of use: high))  */
    // @reg: hw_adrc_bifilt_wgt
    uint8_t hw_drcT_bifiltOut_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifiltSoftThred_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit for soft threshold operation in bilateral filtering.
		Freq of use: low))  */
    // reg: hw_adrc_bifiltSoftThred_en;
    bool hw_drcT_softThd_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_softThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f11),,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The soft threshold parameter itself.
        Freq of use: low))  */
    //reg: hw_adrc_bifilt_softThred
    float hw_drcT_softThd_thred;
} drc_bifilt_t;

typedef enum drc_drcCurve_mode_e {
    drc_usrConfig_mode = 0,
    drc_vendorDefault_mode = 1
} drc_drcCurve_mode_t;

typedef enum drc_drcGainLimit_mode_e {
    drc_drcGainLmt_auto_mode = 0,
    drc_drcGainLmt_manual_mode = 1
} drc_drcGainLimit_mode_t;

typedef struct drc_drcProc_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_drcCurve_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_drcCurve_mode_t),
        M4_DEFAULT(drc_vendorDefault_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Through this parameter, users can select two configuration options for the dynamic range compression curve(hdr2sdr_curve): user config and vendor default.
        Reference enum types.
        Freq of use: high))  */
    drc_drcCurve_mode_t sw_drcT_drcCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_luma2compsLuma_mVal),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f8),,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Users can directly configure the drc curve  through hdr2sdr_curve when drcCurve_mode == drc_usrConfig_mode.
        Freq of use: high))  */
    //reg: hw_adrc_luma2compsLuma_mVal0~16
    float hw_drcT_hdr2Sdr_curve[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_loDetail_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(1f11),,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Adjust the local contrast of the DRC process through this parameter.
        The larger the parameter, the higher the local contrast.
        Freq of use: high))  */
    //reg: hw_adrc_loDetail_ratio
    float hw_drcT_loDetail_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_drcStrgLut_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_drcCurve_mode_t),
        M4_DEFAULT(drc_vendorDefault_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Usr can select two configuration options for the dynamic range compression strength curve(hw_adrc_luma2DrcStrg_val)
        Reference enum types.
        Freq of use: low))  */
    drc_drcCurve_mode_t sw_drcT_drcStrgLut_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_adjGainIdxLuma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(1f11),,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.
        Freq of use: low))  */
    //reg: hw_adrc_adjGainIdxLuma_scale
    float hw_drcT_drcStrgLutLuma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_luma2scale_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(1f7),,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.
        Freq of use: high))  */
    //reg: hw_adrc_luma2scale_val0~16
    float hw_drcT_luma2DrcStrg_val[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_hiDetail_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(1f11),,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.
        Freq of use: high))  */
    //reg: hw_adrc_hiDetail_ratio = 1 - hw_drcT_drcStrg_alpha
    float hw_drcT_drcStrg_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_drcGainLmt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_drcGainLimit_mode_t),
        M4_DEFAULT(drc_drcGainLmt_auto_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Reference enum types.
        Freq of use: low))  */
    drc_drcGainLimit_mode_t sw_drcT_drcGainLimit_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_compsGain_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(1f11),,
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Users can directly configure the drc gain limit when drcGainLmt_mode == drc_drcGainLmt_manual_mode.
        Freq of use: low))  */
    //reg: hw_adrc_compsGain_minLimit
    float hw_drcT_drcGain_minLimit;
} drc_drcProc_t;

typedef struct drc_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(preProc),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    drc_preProc_dyn_t preProc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt_guideDiff),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    drc_bifilt_guideDiff_t bifilt_guideDiff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    drc_bifilt_t bifilt_filter;
    /* M4_GENERIC_DESC(
        M4_ALIAS(drcProc),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    drc_drcProc_t drcProc;
} drc_params_dyn_t;

typedef struct drc_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    drc_params_dyn_t dyn;
} drc_param_t;

#endif
