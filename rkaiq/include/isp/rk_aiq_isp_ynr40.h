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

#ifndef _RK_AIQ_PARAM_YNR40_H_
#define _RK_AIQ_PARAM_YNR40_H_

#include "rk_aiq_isp_common_ynr.h"

typedef struct ynr_radiDistYnrStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_radiDist_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(rnr_en_group),
        M4_NOTES(Enable the miNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: low))  */
    //reg: hw_ynr_rnr_en
    bool sw_ynrT_radiDist_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_radius2strg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0.0, 15.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(rnr_en_group),
        M4_NOTES(The pixels at different radial distances have different scaling strengths for pixSigma.\n
        The sgmScl is larger, the loNr and hiNr strength is stronger.
        Freq of use: high))  */
    //reg: hw_ynr_radius2strg_val0 ~16
    float hw_ynrT_radiDist2YnrStrg_val[YNR_RNR_STRENGTH_SEGMENT_MAX];
} ynr_radiDistYnrStrg_dyn_t;

typedef struct ynr_locSgmStrgYnrStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_globalSet_gain),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The value of global input pix sigma.\n
        Freq of use: low))  */
    // reg: sw_ynr_globalSet_gain
    float hw_ynrT_glbSgmStrg_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_gainMerge_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(2f3b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The weight of global gain and local gain.The higer the value, the higher weight for global gain.\n
        Freq of use: low))  */
    // reg: sw_ynr_gainMerge_alpha
    float hw_ynrT_glbSgmStrg_alpha;
} ynr_locSgmStrgYnrStrg_dyn_t;

typedef struct ynr_locYnrStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(radiDist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_radiDistYnrStrg_dyn_t radiDist;
    /* M4_GENERIC_DESC(
        M4_ALIAS(radiDist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_locSgmStrgYnrStrg_dyn_t locSgmStrg;
} ynr_locYnrStrg_dyn_t;

typedef struct ynr_hiNrEPF_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_nlmFltCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ynr_filtCfg_mode_t),
        M4_DEFAULT(ynr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(nlmFltCft_mode_group),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    ynr_filtCfg_mode_t sw_ynrT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_nlmSpatial_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(nlmFltCft_mode_group:ynr_cfgByFiltStrg_mode),
        M4_NOTES(The spatial weight coeff of the nlm filter is directly configured through this para when sw_ynrT_nlmFltCfg_mode is ynr_cfgByFiltCoeff_mode.\n
        Freq of use: low))  */
    // reg:  hw_ynr_hiSpnrFilt_coeff0~4
    // para: hiSpnrFilt_distSigma
    float sw_ynrT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_nlm_coe),
        M4_TYPE(f32),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,5),
        M4_RANGE_EX(0,0.25),
        M4_DIGIT_EX(8f6b),
        M4_DEFAULT([0.0546875, 0.0390625, 0.05078125, 0.03515625, 0.0234375]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(nlmFltCft_mode_group:ynr_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial weight of adjacent pixels in the nlm filter.\n
        The nlm filter totally have 6 coeffs, only centert pix is not control by theis params.\n
        centerWgt = 1.0 - 4*wgt[0] + 4*wgt[1] + 4*wgt[2] + 8*wgt[3] + 4*wgt[4].\n
        Freq of use: low))  */
    // reg:  hw_ynr_hiSpnrFilt_coeff0~4
    float hw_ynrT_filtSpatial_wgt[5];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrLocalGain_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The alpha value of local sigmaScl in alpha operation between local and global sigmaScl.\n
        The local sigmaScl is composed of inPixSgmScl and radiSgmScl.\n
        The larger , the greater its impact on the local strength of the filter.\n\n
        Freq of use: high))  */
    // reg: hw_ynr_hiSpnrLocalGain_alpha
    float hw_ynrT_locYnrStrg_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrSigma_minLimt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.0068),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Mininum noise sigma for high freq noise.The higher the value, the bigger the strength of flat area denoise.\n
        Freq of use: low))  */
    // reg: hw_ynr_hiSpnrSigma_minLimt
    float hw_ynrT_rgeSgm_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The scale of nlm sigma.The higher the value, the strength of filter is higher.\n
        Freq of use: high))  */
    // reg: hw_ynr_hiSpnr_strg
    float hw_ynrT_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrFilt_wgtOffset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.01),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The negative offset of nlm filt range weight.The higher the value, the smaller the strength of high freq reducing noise.\n
        Freq of use: low))  */
    // reg: hw_ynr_hiSpnrFilt_wgtOffset
    float hw_ynrT_rgeWgt_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrFilt_centerWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(4f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The spatial weight of center pixel in filter.The higher the value, the smaller the strength of reducing noise.\n
        Freq of use: low))  */
    // reg: hw_ynr_hiSpnrFilt_centerWgt
    float hw_ynrT_centerPix_wgt;
} ynr_hiNrEPF_dyn_t;

typedef struct ynr_hiNrSF_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_hiSpnrFlt1Cfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ynr_filtCfg_mode_t),
        M4_DEFAULT(ynr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(hiSpnrFlt1_mode_group),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    ynr_filtCfg_mode_t sw_ynrT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_hiSpnrFilt1_distSigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 256.0),
        M4_DEFAULT(1.5),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(hiSpnrFlt1_mode_group:ynr_cfgByFiltStrg_mode),
        M4_NOTES(The spatial weight coeff of the high freq guass filter is directly configured through this para when sw_ynrT_hiSpnrFlt1Cfg_mode is ynr_cfgByFiltCoeff_mode.\n
        Freq of use: low))  */
    // reg: reg: hw_ynr_hiSpnrFilt_coeff0~5
    float sw_ynrT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_hiSpnrFilt1_coeff),
        M4_TYPE(f32),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([0.25, 0.06640625, 0.03125, 0.0546875, 0.02734375, 0.01171875]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(hiSpnrFlt1_mode_group:ynr_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial weight of high smoothimg filter.\n
        wgt[0] + 4*wgt[1] + 4*wgt[1] + 4*wgt[2] + 4*wgt[3] + 8*wgt[4] + 4*wgt[5] == 1.\n
        Freq of use: low))  */
    // reg: hw_ynr_hiSpnrFilt_coeff0~5
    float hw_ynrT_filtSpatial_wgt[6];

} ynr_hiNrSF_dyn_t;

typedef struct ynr_hiNr_tex2SFAlpha_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiSpnrFilt1_texThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,128),
        M4_DEFAULT(12),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,10,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(the upper limit value of flat area.\n
        Freq of use: low))  */
    // @reg: hw_ynr_hiSpnrFilt1_texThred
    float hw_ynrT_maxAlphaTex_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_hiSpnrFilt1_texScale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,128.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Scale factor of texture value in hi-NR.
        Freq of use: high))  */
    //reg: hw_ynr_hiSpnrFilt1_texScale
    float sw_ynr_texIdx_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_hiSpnrFilt1_wgtAlpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The global weight of gauss filter result.\n
        Higher the value, the stronger denoise strength of filter.\n
        Freq of use: high))  */
    //reg: hw_ynr_hiSpnrFilt1_wgtAlpha
    float sw_ynr_sfAlpha_scale;
} ynr_hiNr_tex2SFAlpha_t;

typedef struct ynr_hiNr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiSpnr_bypass),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(hiNr_en_group),
        M4_NOTES(Enable the miNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: low))  */
    // reg: hw_ynrT_hiNr_en = !sw_ynr_hiSpnr_bypass
    bool hw_ynrT_hiNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgePresvFilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(hiNr_en_group),
        M4_NOTES(TODO))  */
    ynr_hiNrEPF_dyn_t epf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(smoothFilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(hiNr_en_group),
        M4_NOTES(TODO))  */
    ynr_hiNrSF_dyn_t sf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(texRegion_clsfBaseTex),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(hiNr_en_group),
        M4_NOTES(TODO))  */
    ynr_hiNr_tex2SFAlpha_t sfAlphaEpf_baseTex;
} ynr_hiNr_dyn_t;

typedef struct ynr_midNr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_miNr_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(miNr_en_group),
        M4_NOTES(Enable the miNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: low))  */
    // reg: hw_ynrT_midNr_en = !sw_ynr_miSpnr_bypass
    bool hw_ynrT_midNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_miNrFltCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ynr_filtCfg_mode_t),
        M4_DEFAULT(ynr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(miNrFltCfg_mode_group),
        M4_GROUP(miNr_en_group),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    ynr_filtCfg_mode_t sw_ynrT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_miSpnrFilt_distSigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 256.0),
        M4_DEFAULT(3.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(miNr_en_group;miNrFltCfg_mode_group:ynr_cfgByFiltStrg_mode),
        M4_NOTES(The spatial weight coeff of the mi spnr filter is directly configured through this para when sw_ynrT_miSpnrFltCfg_mode is ynr_cfgByFiltCoeff_mode.\n
        Freq of use: low))  */
    // reg: hw_ynr_miSpnrFilt_coeff0~2
    // para: miSpnrFilt_distSigma
    float sw_ynr_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_miNrSpatial_coeff),
        M4_TYPE(f32),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1),
        M4_DIGIT_EX(4),
        M4_DEFAULT([0.2, 0.15, 0.05]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(miNr_en_group;miNrFltCfg_mode_group:ynr_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial weight of adjacent pixels in the mi spnr filter.\n
       coef[0] + 4*coeff[1] + 4*coeff[2] = 1.\n
        Freq of use: low))  */
    // reg: hw_ynr_miSpnrFilt_coeff0~2
    float sw_ynr_filtSpatial_wgt[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_miSpnr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.6),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(miNr_en_group),
        M4_NOTES(Scale factor of texture value in hi-NR.
        Freq of use: high))  */
    //reg: hw_ynr_miSpnrFilt_strg
    float sw_ynr_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_miSpnrFilt_centerWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.3),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(miNr_en_group),
        M4_NOTES(Extra weight of original centre pixel for mi-NR.
        Freq of use: high))  */
    //reg: hw_ynr_miSpnrFilt_centerWgt
    float sw_ynr_centerPix_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_miSpnrSoftThred_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4.0),
        M4_DEFAULT(0.0783),
        M4_DIGIT_EX(3f7),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(miNr_en_group),
        M4_NOTES(The scale factor of soft-threshold operation for mi-NR when blending input.
        Freq of use: high))  */
    //reg: hw_ynr_miSpnrSoftThred_scale
    float hw_ynrT_softThd_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_miEnhance_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,32.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(miNr_en_group),
        M4_NOTES(The scale factor of blending mi-frequency detail to NR result.
        Freq of use: high))  */
    //reg: hw_ynrT_miEnhance_scale = alphaMfTex_scale, hw_ynrT_miEnhance_en =  !(alphaMfTex_scale == 1.0)
    float hw_ynrT_alphaMfTex_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_miSpnr_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(miNr_en_group),
        M4_NOTES(Blending weight of input for mi-NR.
        Freq of use: high))  */
    //reg: hw_ynr_miSpnr_wgt
    float hw_ynrT_midNrOut_alpha;
} ynr_midNr_dyn_t;

typedef struct ynr_loNr_tex2NrStrg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_tex2loStrg_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(tex2loStrg_en_group),
        M4_NOTES(Enable the loNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: low))  */
    // reg: hw_ynrT_tex2LoStrg_en
    bool hw_ynrT_tex2NrStrg_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_tex2loStrg_lowerThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(tex2loStrg_en_group),
        M4_NOTES(The scaling factor of the range sigma of the bilateral filter\n
        Freq of use: high))  */
    // reg: hw_ynr_tex2loStrg_lowerThred
    float hw_ynrT_flatRegion_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_tex2loStrg_upperThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.6),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(tex2loStrg_en_group),
        M4_NOTES(The scaling factor of the range sigma of the bilateral filter\n
        Freq of use: high))  */
    // reg: hw_ynr_tex2loStrg_upperThred
    float hw_ynrT_edgeRegion_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_tex2loStrg_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(tex2loStrg_en_group),
        M4_NOTES(The scaling factor of the range sigma of the bilateral filter\n
        Freq of use: high))  */
    // reg: sw_ynr_tex2loStrg_mantissa & sw_ynr_tex2loStrg_exponent
    // para: tex2loStrg_minLimit
    float sw_ynrT_edgeRegionNr_strg;
} ynr_loNr_tex2NrStrg_t;

typedef struct ynr_loNrEPF_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_loSpnr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The scale of lowFreq filter sigma.The higher the value, the strength of filter is higher.\n
        Freq of use: high))  */
    // reg: hw_ynr_loSpnr_strg
    float hw_ynrT_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_loSpnrThumbThred_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Scale factior of soft-threshold operation between reference pixel and down-scale pixel.\n
        Freq of use: low))  */
    // reg: sw_ynr_loSpnrThumbThred_scale
    float hw_ynrT_guideSoftThd_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_loSpnrFilt_centerWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 4.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The spatial weight of center pixel in filter.The higher the value, the smaller the strength of reducing noise.\n
        Freq of use: low))  */
    // reg: sw_ynr_loSpnr_centerWgt
    float hw_ynrT_centerPix_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_loSpnrSoftThred_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 4.0),
        M4_DEFAULT(0.0783),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Scale factor of soft-threshold operation for mi-NR when blending input .\n
        Freq of use: low))  */
    // reg: sw_ynr_loSpnrSoftThred_scale
    float hw_ynrT_softThd_scale;
    /* M4_GENERIC_DESC(
       M4_ALIAS(sw_ynr_loSpnr_wgt),
       M4_TYPE(f32),
       M4_SIZE_EX(1,1),
       M4_RANGE_EX(0.0,1.0),
       M4_DEFAULT(1.0),
       M4_DIGIT_EX(3),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(6),
       M4_NOTES(Blending weight of input for lo-NR.
       Freq of use: high))  */
    //reg: hw_ynr_loSpnr_wgt
    float hw_ynrT_loNrOut_alpha;

} ynr_loNrEPF_t;

typedef struct ynr_loNr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_loNr_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(loNr_en_group),
        M4_NOTES(Enable the loNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: high))  */
    // reg: hw_ynrT_loNr_en = !sw_ynr_loSpnr_bypass
    bool hw_ynrT_loNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(locYnrStrg_texRegion),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(loNr_en_group),
        M4_NOTES(TODO))  */
    ynr_loNr_tex2NrStrg_t locYnrStrg_texRegion;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(loNr_en_group),
        M4_NOTES(TODO))  */
    ynr_loNrEPF_t epf;
} ynr_loNr_dyn_t;

typedef struct ynr_sigmaEnv_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_sigmaCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ynr_sigmaCurveCfg_mode_t),
        M4_DEFAULT(ynr_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(sgmCurve_mode_group),
        M4_NOTES(The way to configure the sigma curve point. Reference enum types.\n
        Freq of use: low))  */
    ynr_sigmaCurveCfg_mode_t sw_ynrCfg_sgmCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_sigamaCurve_Coeff),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(sgmCurve_mode_group:ynr_cfgByCoeff2Curve_mode),
        M4_NOTES(The sigma calibration curve of the ynr module. \n
        The ynr only use the sigma coeffs of the curve when sw_ynrCfg_sgmCurve_mode == ynr_cfgByCoeff2Curve_mode.))  */
    ynr_coeff2SgmCurve_t coeff2SgmCurve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrC_luma2Sigma_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(sgmCurve_mode_group:ynr_cfgByCurveDirectly_mode),
        M4_NOTES(...\n
        The ynr will use the sigma point data of the curve when sw_ynrCfg_sigmaCfg_mode == ynr_cfgByCurveDirectly_mode.))  */
    // reg: hw_ynr_luma2sigma_idx0 ~ 16, hw_ynr_luma2sigma_val0 ~ 16
    ynr_sigmaCurve_dyn_t hw_ynrC_luma2Sigma_curve;
} ynr_sigmaEnv_t;

typedef struct ynr_hiNrEPF_LP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_hiNlmFltLp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Disable part of none-local means.
        Freq of use: high))  */
    // reg: sw_ynr_hi_lp_en
    bool hw_ynrT_lp_en;
} ynr_hiNrEPF_LP_t;

typedef struct ynr_hiNrLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(epfLP),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_hiNrEPF_LP_t epfLP;
} ynr_hiNrLP_t;

typedef struct ynr_ynrLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiNrLP),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_hiNrLP_t hiNrLP;
} ynr_ynrLP_t;

typedef struct ynr_radiDistYnrStrg_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_rnr_center_cooh),
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
    // @reg: sw_ynr_rnr_vcenter
    // @para: Center_Mode
    int16_t hw_ynrCfg_opticCenter_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_rnr_center_coov),
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
    // @reg: sw_ynr_rnr_vcenter
    // @para: Center_Mode
    int16_t hw_ynrCfg_opticCenter_y;
} ynr_radiDistYnrStrg_static_t;

typedef struct ynr_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(locYnrStrg_radiDist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_radiDistYnrStrg_static_t locYnrStrg_radiDist;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lowPowerCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ynr_ynrLP_t lowPowerCfg;
} ynr_params_static_t;

typedef struct ynr_locMidLoNrStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_loSpnr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0.016, 16),
        M4_DEFAULT([1.0, 1.0, 1.0, 1.0]),
        M4_DIGIT_EX(3f7b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 32, 64, 128, 192, 256]),
        M4_NOTES(The scaling factor of the range sigma of the bilateral filter\n
        Freq of use: high))  */
    // reg: hw_ynr_loSpnr_strg
    // para: luma2loStrg, loSpnr_strg
    float hw_ynrT_luma2RgeSgm_scale[6];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_loSpnrGain2strg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(4),
        M4_FP_EX(0,6,9),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_GROUP(loNr_en_group),
        M4_DATAX([1, 2, 4, 8, 16, 32, 64, 128, 256]),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(....\n
        ....
        Freq of use: high))  */
    // @reg: hw_ynr_loSpnrGain2strg_val0~8
    /*
      The pixSgm values of the 8 nodes on the x-axis are as follows:
      [2, 4, 8, 16, 32, 64, 128, 256]
      */
    float hw_ynrT_locSgmStrg2NrStrg_val[YNR_PIXSGMSCL_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_loSpnrGain2strg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_FP_EX(0,1,7),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([1, 2, 4, 8, 16, 32, 64, 128, 256]),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Scale factor of blending weight for nr-result in low-NR.\n
        Freq of use: high))  */
    // @reg: hw_ynr_loSpnr_wgt = 1.0, hw_ynr_loGain2wgt_val0~8
    float hw_ynrT_locSgmStrg2NrOut_alpha[YNR_PIXSGMSCL_SEGMENT_MAX];
} ynr_locMidLoNrStrg_dyn_t;

typedef struct ynr_midLoNr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(midNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(miNr_en_group||loNr_en_group),
        M4_NOTES(TODO))  */
    ynr_locMidLoNrStrg_dyn_t locNrStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(midNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    ynr_midNr_dyn_t mf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_loNr_dyn_t lf;
} ynr_midLoNr_dyn_t;

typedef struct ynr_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(locYnrStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ynr_locYnrStrg_dyn_t locYnrStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sigma),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ynr_sigmaEnv_t sigmaEnv;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ynr_hiNr_dyn_t hiNr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(midLoNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    ynr_midLoNr_dyn_t midLoNr;
} ynr_params_dyn_t;

typedef struct ynr_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ynr_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_params_dyn_t dyn;
} ynr_param_t;


#endif
