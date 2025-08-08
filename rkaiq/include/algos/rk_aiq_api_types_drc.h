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
#ifndef _RK_AIQ_API_TYPES_DRC_H_
#define _RK_AIQ_API_TYPES_DRC_H_

#include "common/rk_aiq_comm.h"

#define DRC_ISO_STEP_MAX 13
#if ISP_HW_V35
#include "isp/rk_aiq_isp_drc41.h"
#else
#include "isp/rk_aiq_isp_drc40.h"
#endif

RKAIQ_BEGIN_DECLARE

typedef struct {
    bool darkAreaBoostEn;
    unsigned int darkAreaBoostStrength;
    bool hdrStrengthEn;
    unsigned int hdrStrength;
} adrc_strength_t;

typedef struct drcCurve_auto_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_drcT_wp_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.95),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
                Freq of use: low))  */
    float sw_drcT_wp_val;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_drcT_anchorPoint_x1),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.05),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.
            Freq of use: low))  */
    float sw_drcT_anchorPoint_x1;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_drcT_anchorPoint_x2),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.
            Freq of use: low))  */
    float sw_drcT_anchorPoint_x2;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_drcT_anchorPoint_x3),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.8),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.
            Freq of use: low))  */
    float sw_drcT_anchorPoint_x3;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_drcT_anchorPoint_y1),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.05),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.
            Freq of use: low))  */
    float sw_drcT_anchorPoint_y1;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_drcT_anchorPoint_y2),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.2),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.
            Freq of use: low))  */
    float sw_drcT_anchorPoint_y2;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_drcT_anchorPoint_y3),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.8),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.
            Freq of use: low))  */
    float sw_drcT_anchorPoint_y3;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_drcT_iirFrm_maxLimit),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(1,256),
    M4_DEFAULT(16),
    M4_DIGIT_EX(0),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The current weight of IIR filter.
        Freq of use: low))  */
    float sw_drcT_iirFrm_maxLimit;
} drcCurve_auto_t;

typedef struct adrc_drcProc_s {
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
        M4_GROUP_CTRL(adrcCurve_mode_group),
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
        M4_GROUP(adrcCurve_mode_group:adrc_usrConfig_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Users
       can directly configure the drc curve  through hdr2sdr_curve when drcCurve_mode ==
       drc_usrConfig_mode.\n Freq of use: high))  */
    // reg: hw_adrc_luma2compsLuma_mVal0~16
    uint16_t hw_drcT_hdr2Sdr_curve[DRC_CURVE_LEN];
    /* M4_GENERIC_DESC(
    M4_ALIAS(preProc),
    M4_GROUP(adrcCurve_mode_group:adrc_auto_mode),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(Parameters when sw_drcT_drcCurve_mode == adrc_auto_mode.\n Freq of use: high))  */
    drcCurve_auto_t sw_drcT_drcCurve_auto;
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
} adrc_drcProc_t;

typedef struct adrc_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(preProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All preProc params corresponded with iso array. Freq of use: high))  */
    drc_preProc_dyn_t preProc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt_guideDiff),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All bifilt_guideDiff params corresponded with iso array. Freq of use: low))  */
    drc_bifilt_guideDiff_t bifilt_guideDiff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All bifilt_filter params corresponded with iso array. Freq of use: high))  */
    drc_bifilt_t bifilt_filter;
    /* M4_GENERIC_DESC(
        M4_ALIAS(drcProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All drcProc params corresponded with iso array. Freq of use: high))  */
    adrc_drcProc_t drcProc;
} adrc_params_dyn_t;

typedef struct drc_param_auto_s {
#if defined(ISP_HW_V33)
    /* M4_GENERIC_DESC(
    M4_ALIAS(sta),
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
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,13),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    adrc_params_dyn_t dyn[DRC_ISO_STEP_MAX];
} drc_param_auto_t;

typedef struct drc_api_attrib_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(opMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current operation mode))  */
    rk_aiq_op_mode_t opMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stAuto),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The params of auto mode, be only valid for auto mode))  */
    drc_param_auto_t stAuto;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The current params of module))  */
    drc_param_t stMan;
} drc_api_attrib_t;

typedef struct drc_status_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(opMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current operation mode))  */
    rk_aiq_op_mode_t opMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The hardware enable flag))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(stMan),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The current params of module))  */
    drc_param_t stMan;
} drc_status_t;

RKAIQ_END_DECLARE

#endif
