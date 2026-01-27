/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#define DRC_CURVE_LEN 17

typedef struct drc_loBifiltLP_LP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_drcT_lp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Disable part of none-local means.
        Freq of use: low))  */
    // reg: sw_adrc_bf_lp_en
    bool hw_drcT_lp_en;
} drc_loBifiltLP_LP_t;

typedef struct drc_drcLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(loBifiltLP),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO. Freq of use: low))  */
    drc_loBifiltLP_LP_t loBifiltLP;
} drc_drcLP_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(lowPowerCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO. Freq of use: low))  */
    drc_drcLP_t lowPowerCfg;
} drc_params_static_t;

typedef enum drc_curveCfg_mode_e {
    drc_cfgCurveDirect_mode    = 0,
    drc_cfgCurveCtrlCoeff_mode = 1
} drc_curveCfg_mode_t;

typedef struct drc_toneCurveCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_toneCurveK_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can configure the tone curve through drcGain_maxLimit when toneCurve_mode ==
       drc_cfgCurveCtrlCoeff_mode. Freq of use: high))  */
    float sw_drcT_toneCurveK_coeff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_toneGain_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,8),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can configure the tone curve through drcGain_maxLimit when toneCurve_mode ==
       drc_cfgCurveCtrlCoeff_mode. Freq of use: high))  */
    float sw_drcT_toneGain_maxLimit;
} drc_toneCurveCtrl_t;

typedef struct drc_preProc_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_toneCurve_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_curveCfg_mode_t),
        M4_DEFAULT(drc_cfgCurveCtrlCoeff_mode),
        M4_GROUP_CTRL(toneCurve_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Reference enum types.\n
                Freq of use: low))  */
    drc_curveCfg_mode_t sw_drcT_toneCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_drcGainIdxLuma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale value for original luma in tone curve.\n
                Freq of use: low))  */
    // @reg: hw_adrc_drcGainIdxLuma_scale
    float hw_drcT_toneCurveIdx_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_luma2drcGain_val),
        M4_TYPE(f32),
        M4_UI_MODULE(drc_curve),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(1,8),
        M4_DEFAULT([1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0]),
        M4_DATAX([0, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4095]),
        M4_DIGIT_EX(2),
        M4_GROUP(toneCurve_mode_group:drc_cfgCurveDirect_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can
       directly configure the tone curve through luma2DrcGain_val when toneCurve_mode ==
       drc_cfgCurveDirect_mode. Freq of use: high))  */
    // @reg: hw_adrc_luma2drcGain_val0~16
    float hw_drcT_luma2ToneGain_val[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(toneCurveCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(drc_coeff_curve),
        M4_DATAX([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]),
        M4_GROUP(toneCurve_mode_group:drc_cfgCurveCtrlCoeff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO.\n Freq of use: high))  */
    drc_toneCurveCtrl_t toneCurveCtrl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_filtLuma_softThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
                Freq of use: low))  */
    // @reg: hw_adrc_filtLuma_softThred
    float hw_drcT_lpfSoftThd_thred;
} drc_preProc_dyn_t;

typedef struct drc_gdDiffMaxCurveCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_adrc_thumbThred_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the guide diff maxLimit LUT  through maxCurveCreate_xxx when guideDiffMaxLmt_mode == drc_cfgCurveCtrlCoeff_mode.
        Freq of use: low))  */
    float sw_drcT_maxLutCreate_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_adrc_thumbThred_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the guide diff maxLimit LUT  through maxCurveCreate_xxx when guideDiffMaxLmt_mode == drc_cfgCurveCtrlCoeff_mode.
        Freq of use: low))  */
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
        Freq of use: low))  */
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
        Freq of use: low))  */
    float sw_drcT_maxLutCreate_offset;
} drc_gdDiffMaxCurveCtrl_t;

typedef struct drc_bifilt_guideDiff_s {
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_adrc_thumbThred_en),
    M4_GROUP_CTRL(guideDiffLmt_en_group),
    M4_TYPE(bool),
    M4_DEFAULT(1),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The enable of thumb soft thread.\n
            Freq of use: low))  */
    // reg: hw_adrc_thumbThred_en;
    bool hw_drcT_guideDiffLmt_en;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_adrc_thumb_maxLimit),
    M4_GROUP(guideDiffLmt_en_group),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,16),
    M4_DEFAULT(4),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The max limit of thumb reliability.\n
    Freq of use: low))  */
    // @reg: hw_adrc_thumb_maxLimit
    float hw_drcT_guideLuma_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_thumbThred_neg),
        M4_GROUP(guideDiffLmt_en_group),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,2),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The negative thread of thumb.\n
        Freq of use: low))  */
    // @reg: hw_adrc_thumbThred_neg
    float hw_drcT_guideDiff_minLimit;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_adrc_thumb_scale),
    M4_GROUP(guideDiffLmt_en_group),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,16),
    M4_DEFAULT(1),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The scale value of thumb reliability.\n
    Freq of use: low))  */
    // @reg: hw_adrc_thumb_scale
    float hw_drcT_maxLutIdx_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_gdDiffMaxLut_mode),
        M4_GROUP(guideDiffLmt_en_group),
        M4_GROUP_CTRL(gdDiffMaxLut_mode_group),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_curveCfg_mode_t),
        M4_DEFAULT(drc_cfgCurveCtrlCoeff_mode),
        M4_GROUP_CTRL(gdDiffMaxLut_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( Reference enum types.\n
        Freq of use: low))  */
    drc_curveCfg_mode_t sw_drcT_gdDiffMaxLut_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_thumbDiff2thred_val),
        M4_GROUP(guideDiffLmt_en_group;gdDiffMaxLut_mode_group:drc_cfgCurveDirect_mode),
        M4_TYPE(f32),
        M4_UI_MODULE(drc_curve),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,2),
        M4_DEFAULT([1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]),
        M4_DATAX([0, 0.0625, 0.125, 0.1875, 0.25, 0.3125, 0.375, 0.4375, 0.5, 0.5625, 0.625, 0.6875, 0.75, 0.8125, 0.875, 0.9375, 1.0]),
        M4_GROUP(gdDiffMaxLut_mode_group:drc_cfgCurveDirect_mode),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users can directly configure the guide diff maxLimit LUT through
       guideLuma2DiffMax_curve when maxCurve_mode == drc_cfgCurveDirect_mode.\n Freq of use: low))
     */
    // @reg: hw_adrc_thumbDiff2thred_val0~16
    float hw_drcT_gdLuma2DiffMax_lut[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(gdDiffMaxCurveCtrl),
        M4_GROUP(guideDiffLmt_en_group;gdDiffMaxLut_mode_group:drc_cfgCurveCtrlCoeff_mode),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_GROUP(gdDiffMaxLut_mode_group:drc_cfgCurveCtrlCoeff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO.\n Freq of use: low))  */
    drc_gdDiffMaxCurveCtrl_t gdDiffMaxCurveCtrl;
} drc_bifilt_guideDiff_t;

typedef struct drc_bifilt_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_loRange_invSigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.25,4),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The range sigma for bilateral filtering using a low-frequency guidance image.
        The higher the value, the higher the strength of filter.\n
        Freq of use: high))  */
    // reg: hw_adrc_loRange_invSigma
    float hw_drcT_loRgeSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_hiRange_invSigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.25,4),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The range sigma for bilateral filtering using a mid-frequency guidance image.
        The higher the value, the higher the strength of filter.\n
        Freq of use: high))  */
    // reg: hw_adrc_hiRange_invSigma
    float hw_drcT_midRgeSgm_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_wgtOffset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The negative offset for range weight of bilateral filtering.\n
        Freq of use: low))  */
    // reg: hw_adrc_bifilt_wgtOffset
    float hw_drcT_rgeWgt_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_hiWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(In the fusion operation of intermediate and low frequencies, the fusion weight
       value of intermediate frequencies.\n Freq of use: high))  */
    // reg: hw_adrc_bifilt_hiWgt
    float hw_drcT_midWgt_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_curPixelWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 16),
        M4_DEFAULT(6),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The spatial weight of center pixel in filter.\n
        Freq of use: low))  */
    // reg: hw_adrc_bifilt_curPixelWgt
    float hw_drcT_centerPixel_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2f4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The wgt of bifilted pixel is used in the fusion operation between the pre bifilted
       pixel and the original pixel. Higher the value, the stronger denoise strength of pre
       bilateral filter.\n Freq of use: high))  */
    // @reg: hw_adrc_bifilt_wgt
    float hw_drcT_bifiltOut_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifiltSoftThred_en),
        M4_GROUP_CTRL(softThd_en_group),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit for soft threshold operation in bilateral filtering.\n
                Freq of use: low))  */
    // reg: hw_adrc_bifiltSoftThred_en;
    bool hw_drcT_softThd_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_bifilt_softThred),
        M4_GROUP(softThd_en_group),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(0),
        M4_DIGIT_EX(2f11),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The soft threshold parameter itself.\n
        Freq of use: low))  */
    // reg: hw_adrc_bifilt_softThred
    float hw_drcT_softThd_thred;
} drc_bifilt_t;

typedef enum drc_drcCurve_mode_e {
    drc_usrConfig_mode     = 0,
    drc_vendorDefault_mode = 1
} drc_drcCurve_mode_t;

typedef enum drc_drcGainLimit_mode_e {
    drc_drcGainLmt_auto_mode   = 0,
    drc_drcGainLmt_manual_mode = 1
} drc_drcGainLimit_mode_t;

typedef enum adrc_drcCurve_mode_e {
    adrc_usrConfig_mode     = 0,
    adrc_vendorDefault_mode = 1,
    adrc_auto_mode          = 2
} adrc_drcCurve_mode_t;

typedef struct drc_drcProc_s {
    /* M4_GENERIC_DESC(
   M4_ALIAS(hw_drcT_hfDarkRegion_strg),
   M4_TYPE(f32),
   M4_SIZE_EX(1,1),
   M4_RANGE_EX(0,1),
   M4_DEFAULT(0),
   M4_DIGIT_EX(2f11),
   M4_HIDE_EX(0),
   M4_RO(0),
   M4_ORDER(1),
   M4_NOTES(Adjust the local contrast of the DRC process through this parameter.
   The larger the parameter, the higher the low-light zone contrast..\n
   Freq of use: high))  */
    float hw_drcT_hfDarkRegion_strg;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_drcT_locDetail_strg),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0),
    M4_DIGIT_EX(2f11),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(Adjust the local contrast of the DRC process through this parameter.
    The larger the parameter, the higher the local contrast.\n
    Freq of use: high))  */
    // reg: hw_adrc_loDetail_ratio
    float hw_drcT_locDetail_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_drcCurve_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(adrc_drcCurve_mode_t),
        M4_DEFAULT(adrc_vendorDefault_mode),
        M4_GROUP_CTRL(drcCurve_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Through this parameter, users can select two configuration options for the dynamic
       range compression curve(hdr2sdr_curve): user config and vendor default. Reference enum types.
        Freq of use: high))  */
    adrc_drcCurve_mode_t sw_drcT_drcCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_compsIdxLuma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(1.78),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale value for original luma in compress curve.\n
                Freq of use: low))  */
    // @reg: hw_adrc_compsIdxLuma_scale
    float hw_drcT_drcCurveIdx_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_luma2compsLuma_mVal),
        M4_TYPE(u16),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1024),
        M4_UI_MODULE(drc_curve),
        M4_DEFAULT([0, 71, 139, 203, 263, 320, 374, 426, 475, 566, 649, 724, 794, 858, 918, 972, 1024]),
        M4_DATAX([0, 1024, 2048, 3072, 4096, 5120, 6144, 7168, 8192, 10240, 12288, 14336, 16384, 18432, 20480, 22528, 24576]),
        M4_GROUP(drcCurve_mode_group:adrc_usrConfig_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Users
       can directly configure the drc curve  through hdr2sdr_curve when drcCurve_mode ==
       adrc_usrConfig_mode.\n Freq of use: high))  */
    // reg: hw_adrc_luma2compsLuma_mVal0~16
    uint16_t hw_drcT_hdr2Sdr_curve[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_drcStrgLut_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_drcCurve_mode_t),
        M4_DEFAULT(drc_vendorDefault_mode),
        M4_GROUP_CTRL(drcStrgLut_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Usr can select two configuration options for the dynamic range compression strength
       curve(hw_adrc_luma2DrcStrg_val) Reference enum types.\n Freq of use: low))  */
    drc_drcCurve_mode_t sw_drcT_drcStrgLut_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_adjGainIdxLuma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(2f11),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The index scale value of drc strength gain.\n
        Freq of use: low))  */
    // reg: hw_adrc_adjGainIdxLuma_scale
    float hw_drcT_drcStrgLutLuma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_luma2scale_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1),
        M4_UI_MODULE(drc_curve),
        M4_DEFAULT([0,0.001,0.01,0.037,0.094,0.186,0.308,0.377,0.449,0.521,0.591,0.722,0.830,0.910,0.961,0.988,1.0]),
        M4_DATAX([0,256,384,512,640,768,896,960,1024,1088,1152,1280,1408,1536,1664,1792,2048]),
        M4_GROUP(drcStrgLut_mode_group:drc_usrConfig_mode),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The table of drc strength gain.\n.
        Freq of use: high))  */
    // reg: hw_adrc_luma2scale_val0~16
    float hw_drcT_luma2DrcStrg_val[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_drcGainLmt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_drcGainLimit_mode_t),
        M4_DEFAULT(drc_drcGainLmt_auto_mode),
        M4_GROUP_CTRL(drcGainLimit_mode_group),
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
        M4_GROUP(drcGainLimit_mode_group:drc_drcGainLmt_manual_mode),
        M4_DIGIT_EX(2f11),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Users can directly configure the drc gain limit when drcGainLmt_mode ==
       drc_drcGainLmt_manual_mode. Freq of use: low))  */
    // reg: hw_adrc_compsGain_minLimit
    float hw_drcT_drcGain_minLimit;
} drc_drcProc_t;

typedef struct drc_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(preProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All preProc params corresponded with iso array.\n Freq of use: high))  */
    drc_preProc_dyn_t preProc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt_guideDiff),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All bifilt_guideDiff params corresponded with iso array.\n Freq of use: low))  */
    drc_bifilt_guideDiff_t bifilt_guideDiff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All bifilt_filter params corresponded with iso array.\n Freq of use: high))  */
    drc_bifilt_t bifilt_filter;
    /* M4_GENERIC_DESC(
        M4_ALIAS(drcProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All drcProc params corresponded with iso array.\n Freq of use: high))  */
    drc_drcProc_t drcProc;
} drc_params_dyn_t;

typedef struct drc_param_s {
#if defined(ISP_HW_V33)
    /* M4_GENERIC_DESC(
    M4_ALIAS(static_param),
    M4_TYPE(struct),
    M4_UI_MODULE(static_ui),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(TODO))  */
    drc_params_static_t sta;
#endif
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    drc_params_dyn_t dyn;
} drc_param_t;

#endif
