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

#ifndef _RK_AIQ_PARAM_BTNR41_H_
#define _RK_AIQ_PARAM_BTNR41_H_

#define BTNR_ISO_SEGMENT_MAX                     13
#define BTNR_SPNR_SIGMACURVE_SEGMENT_MAX         16
#define BTNR_MD_SIGMACURVE_SEGMENT_MAX           20

typedef enum {
    // @note: By configuring the strength coefficient, the software internally calculates and generates filter coefficients based on the strength coefficient to configure the filter.
    btnr_cfgByFiltStrg_mode = 0,
    // @note: Configure the filter by directly configuring the filter coefficients.
    btnr_cfgByFiltCoeff_mode = 1
} btnr_filtCfg_mode_t;

typedef enum btnr_pixDomain_mode_e {
    /*
    reg: (hw_btnr_transf_bypass_en== 0 && hw_btnr_transf_mode == 0)
    */
    btnr_pixLog2Domain_mode = 0,
    // /*
    // reg: (hw_btnr_transf_bypass_en== 0 && hw_btnr_transf_mode == 1)
    // */
    // btnr_pixSqrtDomain_mode,
    /*
    reg: (hw_btnr_transf_bypass_en== 1)
    */
    btnr_pixLinearDomain_mode,
} btnr_pixDomain_mode_t;

typedef enum btnr_trans_mode_e {
    /*
    reg: hw_btnr_transfMode_scale == 0
    */
    btnr_pixInBw20b_mode = 0,
    /*
    reg: hw_btnr_transfMode_scale == 1
    */
    btnr_pixInBw15b_mode = 1
} btnr_trans_mode_t;

typedef struct btnr_transCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_transfMode_scale),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_trans_mode_t),
        M4_DEFAULT(btnr_pixInBw20b_mode),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(pixDomain_mode_group:btnr_pixLog2Domain_mode),
        M4_NOTES(Scale used for transformation.\n
        Freq of use: low))  */
    // reg: hw_btnr_transfMode_scale
    btnr_trans_mode_t hw_btnrCfg_trans_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_transfMode_offset),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(256),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Offset used for transformation.\n
        Freq of use: low))  */
    // reg: hw_btnr_transfMode_offset
    int hw_btnrCfg_trans_offset;
} btnr_transCfg_t;


/*
Note:
1.  mdWgt (motion weight) : The weight represents the weight value of the motion. The larger the weight value, the greater the probability of motion.
  statWgt (static weight) : The weight represents the stationary weight value.

  mdWgt + statWgt = 1;
  mdWgt = 1 - statWgt;
*/
typedef struct btnr_subLoMd0_diffCh_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffVfilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of the vertical filter that filters the subLoMd0 diff data.\n
        Freq of use: low))  */
    // reg: hw_btnr_loDiffVfilt_en
    bool hw_btnrT_vFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffHfilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of the horizontal filter that filters the subLoMd0 diff data.\n
        Freq of use: low))  */
    // reg: hw_btnr_loDiffHfilt_en
    bool hw_btnrT_hFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffFirstLine_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,2.0),
        M4_DEFAULT(0.125),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial weight scaling factor of the first line of the IIR filter that filters the subLoMd0 diff data.\n
        The larger the value, the stronger the filtering strength\n
        Freq of use: low))  */
    // reg: hw_btnrT_vIIRFstLn_scale
    float hw_btnrT_vIIRFstLn_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffVfilt_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1.0,1024.0),
        M4_DEFAULT(8.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The Feedback weight of filtered pixel output results within the neighborhood of the IIR filter.\n
        The larger the value, the stronger the filtering strength\n
        Freq of use: low))  */
    // reg: hw_btnrT_loDiffVfilt_wgt
    // reg: hw_btnrT_vIIRFilt_strg = 1 / (1 - hw_btnrT_loDiffVfilt_wgt)
    float hw_btnrT_vIIRFilt_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffVfilt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial weight scaling factor of the first line of the IIR filter that filters the subLoMd0 diff data.\n
        The larger the value, the stronger the filtering strength\n
        Freq of use: low))  */
    // reg: hw_btnrT_loDiffVfilt_scale
    float hw_btnrT_vIIRWgt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffVfilt_offset),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4096),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial weight scaling factor of the first line of the IIR filter that filters the subLoMd0 diff data.\n
        The larger the value, the stronger the filtering strength\n
        Freq of use: low))  */
    // reg: hw_btnrT_loDiffVfilt_offset
    uint32_t hw_btnrT_vIIRWgt_offset;

} btnr_subLoMd0_diffCh_dyn_t;

typedef enum btnr_sigmaCurve_mode_e {
    // reg: hw_btnr_tnrSigmaCurveDoube_ens == 0
    // reg: hw_btnr_preSpnrsigmaCurveDouble_en == 0
    btnr_midSegmInterpOn_mode = 0,
    // reg: hw_btnr_tnrSigmaCurveDoube_ens == 1
    // reg: hw_btnr_preSpnrsigmaCurveDouble_en == 1
    btnr_midSegmInterpOff_mode = 1
} btnr_sigmaCurve_mode_t;

typedef struct btnr_mdSigma_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Sigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of sigma used by the MD.\n
        Freq of use: low))  */
    // reg: hw_btnr_Sigma_scale
    float hw_btnrT_sigma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_SigmaHdrSht_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of sigma used by the MD  for short frame fusion region.\n
        Freq of use: low))  */
    // reg: hw_btnr_SigmaHdrSht_scale
    float hw_btnrT_sigmaHdrS_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_tnrSigmaCurveDoube_ens),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_sigmaCurve_mode_t),
        M4_DEFAULT(btnr_midSegmInterpOn_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of sigma LUT used by the MD.\n
        Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_btnr_tnrSigmaCurveDoube_ens
    btnr_sigmaCurve_mode_t hw_btnrT_sigmaCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_sigmaIdxFltCft_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_filtCfg_mode_t),
        M4_DEFAULT(btnr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(sigmaIdxFlt_mode_group),
        M4_NOTES(The mode of sigma LUT used by the MD.\n
        Reference enum types.\n
        Freq of use: low))  */
    // @reg: sw
    btnr_filtCfg_mode_t sw_btnrT_sigmaIdxFltCft_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_sigmaIdxFilt_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
        M4_DEFAULT(25.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(sigmaIdxFlt_mode_group:btnr_cfgByFiltStrg_mode),
        M4_NOTES(The strength of sigma idx filter.\n
        Freq of use: low))  */
    // reg: hw_btnr_SigmaIdxFilt_coeff0-5
    float hw_btnrT_sigmaIdxFilt_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_SigmaIdxFilt_coeff),
        M4_TYPE(u8),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,255),
        M4_DEFAULT([56, 41, 41, 40, 40, 40]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(sigmaIdxFlt_mode_group:btnr_cfgByFiltCoeff_mode),
        M4_NOTES(The filter weight of sigma idx filter.\n
        Freq of use: low))  */
    // reg: hw_btnr_SigmaIdxFilt_coeff0-5
    uint8_t hw_btnrT_sigmaIdxFilt_coeff[6];
} btnr_mdSigma_t;

typedef struct btnr_subLoMd0_sgmCh_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_sigHfilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The enable bit of the horizontal filter that filters the subLoMd0 sigma data.\n
        Freq of use: low))  */
    // reg: hw_btnr_sigHfilt_en
    bool hw_btnrT_hFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_sigFirstLine_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,2.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial weight scaling factor of the first line of the IIR filter that filters the subLoMd0 sigma data.\n
        Freq of use: low))  */
    // reg: hw_btnr_sigFirstLine_scale
    float hw_btnrT_vIIRFstLn_scale;

} btnr_subLoMd0_sgmCh_dyn_t;

typedef struct btnr_subLoMd0_wgtOpt_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffWgtCal_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.125),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used to adjust the bias of the original motion weight of the subLoMd0.\n
        Freq of use: high))  */
    // reg: hw_btnrT_LoDiffWgtCal_Offset
    float hw_btnrT_mdWgt_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffWgtCal_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used as a scaling factor for the biased motion weight of the subLoMd0.\n
        Freq of use: high))  */
    // reg: hw_btnr_LoDiffWgtCal_scale
    float hw_btnrT_mdWgt_scale;
} btnr_subLoMd0_wgtOpt_dyn_t;

typedef struct btnr_subLoMd0_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd0_diffCh),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_subLoMd0_diffCh_dyn_t diffCh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd0_sgmCh),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_subLoMd0_sgmCh_dyn_t sgmCh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd0_wgtOpt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_subLoMd0_wgtOpt_dyn_t wgtOpt;
} btnr_subLoMd0_dyn_t;

typedef struct btnr_subLoMd1_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_lpfLo_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(subLoMd1_lpf_en_group),
        M4_NOTES(The enable bit of the LPF that processes the diff data to obtain low-frequency signals.\n
        Freq of use: low))  */
    // reg: hw_btnr_lpfLo_bypass
    bool hw_btnrT_lpf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_lpfCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_filtCfg_mode_t),
        M4_DEFAULT(btnr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(subLoMd1_lpfCfg_mode_group),
        M4_GROUP(subLoMd1_lpf_en_group),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    btnr_filtCfg_mode_t sw_btnrT_lpfCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_lpfLo_coeff),
        M4_TYPE(u16),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0, 1023),
        M4_DEFAULT([178, 99, 60, 22, 13, 3, 0, 1, 1]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(subLoMd1_lpf_en_group;subLoMd1_lpfCfg_mode_group:btnr_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial weight coeff of the LPF is directly configured through this para when lpfCfg_mode is btnr_cfgByFiltCoeff_mode.\n
        Freq of use: low))  */
    // reg: hw_btnr_lpfLo_coeff0~8
    uint16_t hw_btnrT_lpfSpatial_wgt[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_lpfLo_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(subLoMd1_lpf_en_group;subLoMd1_lpfCfg_mode_group:btnr_cfgByFiltStrg_mode),
        M4_NOTES(The spatial weight coeff of the LPF is configured through this para when lpfCfg_mode is btnr_cfgByFiltStrg_mode.\n
        Freq of use: low))  */
    // reg: hw_btnr_lpfLo_coeff0~8
    float sw_btnrT_lpf_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtCal_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(4.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit of the original motion weight of the subLoMd1.\n
        Freq of use: low))  */
    // reg: hw_btnrT_loWgtCal_maxLimit
    float hw_btnrT_mdWgt_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtCal_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.45),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used to adjust the bias of the original motion weight of the subLoMd1.\n
        Freq of use: high))  */
    // reg: hw_btnrT_loWgtCal_offset
    float hw_btnrT_mdWgt_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgt_first_off_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(subLoMd1_mdWgtFstLnNegOff_en_group),
        M4_NOTES(The first line uses a different offset value from other rows when the bit is enabled.\n
        Freq of use: low))   */
    // reg: hw_btnrT_loWgt_first_off_en
    float hw_btnrT_mdWgtFstLnNegOff_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgt_first_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.45),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(subLoMd1_mdWgtFstLnNegOff_en_group),
        M4_NOTES(The para is used to adjust the bias of the original motion weight of the first line when mdWgtFstLnNegOff_en==1.\n
        Freq of use: low))  */
    // reg: hw_btnrT_loWgt_first_offset
    float hw_btnrT_mdWgtFstLn_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtCal_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(3.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used as a scaling factor for the biased motion weight of the subLoMd1.\n
        Freq of use: high))  */
    // reg: hw_btnrT_loWgtCal_scale
    float hw_btnrT_mdWgt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtHfilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of the horizontal filter that filters the motion weight.\n
        Freq of use: low))  */
    // reg: hw_btnr_loWgtHfilt_en
    bool hw_btnrT_hFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtVfilt_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1.0,1024.0),
        M4_DEFAULT(8.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The Feedback weight of filter output results within the neighborhood of the IIR filter that filters the motion weight.\n
        Freq of use: low))  */
    // reg: hw_btnrT_loWgtVfilt_wgt
    // reg: hw_btnrT_vIIRFilt_strg = 1/ (1- 1hw_btnrT_loWgtVfilt_wgt)
    float hw_btnrT_vIIRFilt_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffVfilt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial weight scaling factor of the first line of the IIR filter that filters the subLoMd0 diff data.\n
        The larger the value, the stronger the filtering strength\n
        Freq of use: low))  */
    // reg: hw_btnrT_loWgtVfilt_scale
    float hw_btnrT_vIIRWgt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDiffVfilt_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The initial weight scaling factor of the first line of the IIR filter that filters the subLoMd0 diff data.\n
        The larger the value, the stronger the filtering strength\n
        Freq of use: low))  */
    // reg: hw_btnrT_loWgtVfilt_offset
    float hw_btnrT_vIIRWgt_offset;
} btnr_subLoMd1_dyn_t;

typedef enum btnr_dLoSrc_mode_e {
    // @note: extremely low motion detection source mode use low freq subLoMd1 wgt channel.
    btnr_subLoMd1_mode = 0,
    // @note: extremely low motion detection source mode use low freq subLoMd0 wgt channel.
    btnr_subLoMd0_mode = 1
} btnr_dLoSrc_mode_t;

typedef struct btnr_subDeepLoMd_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLo_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(dLoMd_en_group),
        M4_NOTES(The enable bit of the Large low motion detection.\n
        Freq of use: low))  */
    // reg: !en == hw_btnr_mdLargeLoMdWgt_bypss_en
    bool hw_btnrT_dLoMd_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLoUse_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_dLoSrc_mode_t),
        M4_DEFAULT(btnr_subLoMd1_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(dLoSrc_mode_group),
        M4_GROUP(dLoMd_en_group&&loMd_mode_group:btnr_allSubLoMdMix_mode|btnr_subDeepLoMdOnly_mode),
        M4_NOTES(which channel thumbs that the Large low motion detection use.\n
        Freq of use: low))  */
    btnr_dLoSrc_mode_t sw_btnrT_dLoSrc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLoMinFilter_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(dLoMd_en_group&&loMd_mode_group:btnr_allSubLoMdMix_mode|btnr_subDeepLoMdOnly_mode),
        M4_NOTES(The enable bit of min filter for Large low motion detection.\n
        Freq of use: low))  */
    // reg: !en == hw_btnr_mdLargeLoMinFilter_bypss_en
    bool hw_btnrT_minFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLoMinGauss_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(dLoMd_en_group&&loMd_mode_group:btnr_allSubLoMdMix_mode|btnr_subDeepLoMdOnly_mode),
        M4_NOTES(The enable bit of min gauss filter for Large low motion detection.\n
        Freq of use: low))  */
    // reg: !en == hw_btnr_mdLargeLoMinGauss_bypss_en
    bool hw_btnrT_lpf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLoWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(dLoMd_en_group&&loMd_mode_group:btnr_allSubLoMdMix_mode|btnr_subDeepLoMdOnly_mode),
        M4_NOTES(Large low motion detection use for next frame motion detection offset.\n
        Freq of use: low))  */
    // reg: hw_btnr_mdLargeLoMdWgt_scale
    float hw_btnrT_wgt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLoWgt_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(dLoMd_en_group&&loMd_mode_group:btnr_allSubLoMdMix_mode|btnr_subDeepLoMdOnly_mode),
        M4_NOTES(Large low motion detection use for next frame motion detection scale.\n
        Freq of use: low))  */
    // reg: hw_btnr_mdLargeLoMdWgt_offset
    float hw_btnrT_wgt_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLoCut_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(dLoMd_en_group),
        M4_NOTES(Large low motion detection weight cut offset.\n
        Freq of use: low))  */
    // reg: hw_btnr_mdLargeLoWgtCut_offset
    // para: mdBigLoCut_offset
    float sw_btnrT_wgt2FusionLmt_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLo_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(dLoMd_en_group),
        M4_NOTES(Large low motion detection weight scale.\n
        Freq of use: low))  */
    // reg: hw_btnr_mdLargeLoWgt_scale
    // para: mdBigLo_scale
    float sw_btnrT_wgt2FusionLmt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mdBigLoAdd_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.8),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(dLoMd_en_group),
        M4_NOTES(Large low motion detection weight add offset.\n
        Freq of use: low))  */
    // reg: hw_btnr_mdLargeLoWgtAdd_offset
    // para: mdBigLoAdd_offset
    float hw_btnrT_wgt2FussionLmt_offset;
} btnr_subDeepLoMd_dyn_t;

typedef enum btnr_loMd_mode_e {
    // @reg: hw_btnr_LoMgeWgt_mode == 0
    // @note: "low mdWgt = fun(loMd0Wgt, loMd1Wgt, extLoMdWgt)"
    btnr_allSubLoMdMix_mode = 0,
    // @reg: hw_btnr_LoMgeWgt_mode == 1
    // @note: "low mdWgt =loMd0Wgt"
    btnr_subLoMd0Only_mode = 1,
    // @reg: hw_btnr_LoMgeWgt_mode == 2
    // @note: "low mdWgt =loMd1Wgt"
    btnr_subLoMd1Only_mode = 2,
    // @reg: hw_btnr_LoMgeWgt_mode == 3
    // @note: "low mdWgt = eLoMdWgt"
    btnr_subDeepLoMdOnly_mode = 3
} btnr_loMd_mode_t;

typedef struct btnr_loMd_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDetection_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(loMd_en_group),
        M4_NOTES(The enable bit of the LPF that processes the diff data to obtain high-frequency signals.\n
        Freq of use: low))  */
    // reg: !en = hw_btnr_loDetection_bypass_en;
    bool hw_btnrT_loMd_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loDetection_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_loMd_mode_t),
        M4_DEFAULT(btnr_allSubLoMdMix_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(loMd_mode_group),
        M4_GROUP(loMd_en_group),
        M4_NOTES(The mode of low frequency motion detection. Reference enum types.\n
        Freq of use: high))  */
    // @reg: hw_btnr_LoMgeWgt_mode
    btnr_loMd_mode_t hw_btnrT_loMd_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_LoMgePreWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loMd_en_group),
        M4_NOTES(The scaling factor of the weight value of the previous frame before it is weighted with the cur weight.\n
        Freq of use: low))  */
    // reg: hw_btnr_LoMgePreWgt_scale
    float hw_btnrT_preWgtMge_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_LoMgePreWgt_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loMd_en_group),
        M4_NOTES(The offset parameter of the scaled weight value of the previous frame.\n
        Freq of use: low))  */
    // reg: hw_btnr_LoMgePreWgt_offset
    float hw_btnrT_preWgtMge_offset;
} btnr_loMd_dyn_t;

typedef enum btnr_md_mode_e {
    // @reg: hw_btnr_wgtCal_mode == 0
    btnr_loAsRatio_hi0_mode = 0,
    // @reg: hw_btnr_wgtCal_mode == 1
    btnr_loAsBias_hi1_mode = 1
} btnr_md_mode_t;

typedef struct btnr_loAsHiRatioMd_dyn_s {
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_btnr_LpfHi_bypass_en),
       M4_TYPE(bool),
       M4_DEFAULT(1),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_GROUP_CTRL(loAsHiRatioMd_hfLpf_en_group),
       M4_NOTES(The enable bit of the LPF that processes the diff data to obtain high-frequency signals.\n
       Freq of use: low))  */
    // reg: hw_btnr_LpfHi_bypass_en;
    bool hw_btnrT_hfLpf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_hfLpfCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_filtCfg_mode_t),
        M4_DEFAULT(btnr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(loAsHiRatioMd_hfLpfCfg_mode_group),
        M4_GROUP(loAsHiRatioMd_hfLpf_en_group),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    btnr_filtCfg_mode_t sw_btnrT_hfLpfCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_lpfHi_coeff),
        M4_TYPE(u16),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,1023.0),
        M4_DEFAULT([178, 99, 60, 22, 13, 3, 0, 1, 1]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loAsHiRatioMd_hfLpf_en_group;loAsHiRatioMd_hfLpfCfg_mode_group:btnr_cfgByFiltCoeff_mode),
        M4_NOTES(The spatial weight coeff of the LPF is directly configured through this para when hfLpfCfg_mode is btnr_cfgByFiltCoeff_mode.\n
        coeff[0] + 4* coeff[1] + 4*ceoff[2] + 4*coeff[3] + 8*coeff[4] + 4*coeff[5] + 4*coeff[6] + 4*coeff[7] + 2*coeff[8] = 1023.\n
        Freq of use: low))  */
    // reg: hw_btnr_lpfLo_coeff0~8
    uint16_t hw_btnrT_hfLpfSpatial_wgt[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_lpfHi_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loAsHiRatioMd_hfLpf_en_group;loAsHiRatioMd_hfLpfCfg_mode_group:btnr_cfgByFiltStrg_mode),
        M4_NOTES(The spatial weight coeff of the LPF is configured through this para when hfLpfCfg_mode is btnr_cfgByFiltStrg_mode.\n
        Freq of use: low))  */
    // reg: hw_btnr_LpfHi_coeff0~8
    float sw_btnrT_hfLpf_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode0LoWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(2.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of low-frequency stationary weights from loMd module.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode0LoWgt_scale
    float hw_btnrT_loWgtStat_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode0LoWgtHdrsht_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of low-frequency stationary weight in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode0LoWgtHdrsht_scale
    float hw_btnrT_loWgtStatHdrS_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode0Base_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(4.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of high-frequency motion weight.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode0Base_ratio
    float hw_btnrT_hiNsBase_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_mode0WgtOut_maxlimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of high-frequency motion weight.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode0WgtOut_maxLimit
    float hw_btnrT_mdWgt_maxLimit;
} btnr_loAsHiRatioMd_dyn_t;

typedef struct btnr_loAsHiBiasMd_dyn_s {
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_btnrT_hiMd_en),
       M4_TYPE(bool),
       M4_DEFAULT(1),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_GROUP_CTRL(loAsHiBiasMd_hiMd_en_group),
       M4_NOTES(The enable bit of the LPF that processes the diff data to obtain high-frequency signals.\n
       Freq of use: low))  */
    // reg: !hw_btnr_mdOnlyLo_en;
    bool hw_btnrT_hiMd_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1LoWgt_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used to adjust the bias of the low-frequency stationary weight from loMd module .\n
        Freq of use: high))  */
    // reg: hw_btnr_Mode1LoWgt_offset
    float hw_btnrT_loWgtStat_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1LoWgtHdrSht_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used to adjust the bias of the low-frequency stationary weight  in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode1LoWgtHdrSht_offset
    float hw_btnrT_loWgtStatHdrS_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1LoWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.414),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used as a scaling factor for the biased low-frequency stationary weight. \n
        Freq of use: high))  */
    // reg: hw_btnr_Mode1LoWgt_scale
    float hw_btnrT_loWgtStat_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode0LoWgtHdrsht_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.7),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used as a scaling factor for the biased low-frequency stationary weight in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode0LoWgtHdrsht_scale
    float hw_btnrT_loWgtStatHdrS_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1Wgt_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4.0),
        M4_DEFAULT(0.707),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loAsHiBiasMd_hiMd_en_group),
        M4_NOTES(The 2nd bias parameter of the low-frequency stationary weight value after scaling.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode1Wgt_offset
    float hw_btnrT_mdWgt_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1Wgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loAsHiBiasMd_hiMd_en_group),
        M4_NOTES(The minimum limit of low-frequency stationary weight values after 2 biases.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode1Wgt_minLimit
    float hw_btnrT_mdWgt_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1Wgt_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(15.9),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loAsHiBiasMd_hiMd_en_group),
        M4_NOTES(The minimum limit of low-frequency stationary weight values after 2 biases.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode1Wgt_maxLimit
    float hw_btnrT_mdWgt_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1Wgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(2.4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loAsHiBiasMd_hiMd_en_group),
        M4_NOTES(The scaling factor for final weight calculation.\n
        Freq of use: high))  */
    // reg: hw_btnr_Mode1Wgt_scale
    float hw_btnrT_mdWgt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1Wgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(loAsHiBiasMd_hiMd_en_group),
        M4_NOTES(The scaling factor for final weight calculation in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode1WgtHdrS_scale
    float hw_btnrT_mdWgtHdrS_scale;
} btnr_loAsHiBiasMd_dyn_t;

typedef struct btnr_mdWgtFilt_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(mode0WgtOut_Offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of high-frequency motion weight.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode0WgtOut_offset
    float hw_btnrT_mdWgt_offset;
} btnr_mdWgtFilt_dyn_t;

typedef struct btnr_memc_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_me_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(me_en_group),
        M4_NOTES( ...
        Freq of use: low))  */
    // reg: hw_btnr_motionEst_en
    bool hw_btnrT_memc_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_meUpMvxCost_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(me_en_group),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_motionEstUpMvxCost_scale;
    float hw_btnrT_upMvxCost_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_meUpMvxCost_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(me_en_group),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_motionEstUpMvxCost_offset;
    float hw_btnrT_upMvxCost_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_meUpLeftMvxCost_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(me_en_group),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_motionEstUpLeftMvxCost_scale;
    float hw_btnrT_upLeftMvxCost_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_meUpLeftMvxCost_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(me_en_group),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_motionEstUpLeftMvxCost_offset;
    float hw_btnrT_upLeftMvxCost_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_meUpRightMvxCost_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(me_en_group),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_motionEstUpRightMvxCost_scale;
    float hw_btnrT_upRightMvxCost_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_meUpRightMvxCost_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(me_en_group),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_motionEstUpRightMvxCost_offset;
    float hw_btnrT_upRightMvxCost_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_meSadLine_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT([1.0,1.0,1.0]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(me_en_group),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_motionEstSadLine_wgt;
    float hw_btnrT_sadFiltSpatial_wgt[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_meLoWgt_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.3),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(me_en_group),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_motionEstLoWgt_thred
    float hw_btnrT_mcLoWgt_thred;
} btnr_memc_dyn_t;

typedef struct btnr_curSpNr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnr_bypass_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(curSpNr_en_group),
        M4_NOTES(The enable bit of the spatial filter that processes the cur frame data.\n
        Freq of use: high))  */
    // reg: en = !hw_btnr_curSpnrFilt_bypass_en;
    bool hw_btnrT_spNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSigmaIdxFilt_bypass_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(curHiFlt_en_group),
        M4_GROUP(curSpNr_en_group),
        M4_NOTES(The enable bit for sigma LUT index filtering.\n
        Freq of use: low))  */
    // reg: en = !hw_btnr_curSpnrFilt_bypass_en;
    bool hw_btnrT_hiFilter_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_filtCfg_mode_t),
        M4_DEFAULT(btnr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(curSpNr_filtCfg_mode_group),
        M4_GROUP(curSpNr_en_group;curHiFlt_en_group),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    btnr_filtCfg_mode_t sw_btnrT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrHiSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(curSpNr_en_group;curHiFlt_en_group;curSpNr_filtCfg_mode_group:btnr_cfgByFiltStrg_mode),
        M4_NOTES(This strength is used to adjust the spatial weight of the bilateral filter.\n
        Freq of use: high))  */
    // reg: hw_btnr_curSpnrFilter_coeff0~5
    float sw_btnrT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS( hw_btnr_curSpnrFilter_coeff),
        M4_TYPE(u8),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,255),
        M4_DEFAULT([52, 25, 15, 5, 3, 0]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(curSpNr_en_group;curHiFlt_en_group;curSpNr_filtCfg_mode_group:btnr_cfgByFiltCoeff_mode),
        M4_NOTES(This strength is used to adjust the spatial weight of the bilateral filter.\n
        wgt[0] + 4*wgt[1] + 4*wgt[2] + 4*wgt[3] + 8*wgt[4] + 4*wgt[5] = 255.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrFilter_coeff0~5
    uint8_t sw_btnrT_filtSpatial_wgt[6];
} btnr_curSpNr_dyn_t;

typedef enum btnr_preSpNrSgm_mode_e {
    // @reg: hw_spnrPreSigmaUse_en == 0
    btnr_lutSgmOnly_mode = 0,
    // @reg: hw_spnrPreSigmaUse_en == 1
    btnr_preLutSgmMix_mode = 1,
} btnr_preSpNrSgm_mode_t;

typedef enum btnr_sgmIdxLpfStrg_mode_e {
    // @reg: hw_btnr_preSpnrSigmaIdxFilter_mode == 0   2 3 4 4
    btnr_lpfStrgH_mode = 0,
    // @reg: hw_btnr_preSpnrSigmaIdxFilter_mode == 1   1 4 6 8
    btnr_lpfStrgL_mode = 1,
} btnr_sgmIdxLpfStrg_mode_t;

typedef struct btnr_preSpNr_sigma_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigmaCurveDoube_ens),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_sigmaCurve_mode_t),
        M4_DEFAULT(btnr_midSegmInterpOff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of sigma LUT used by the iir SPNR.. Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_btnr_preSpnrSigmaCurveDoube_ens
    btnr_sigmaCurve_mode_t hw_btnrT_sigmaCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigmaIdxFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(sigmaIdxLpf_en_group);
        M4_NOTES(The enable bit for sigma LUT index filtering.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrSigmaIdxFilt_bypass_en;
    bool hw_btnrT_sigmaIdxLpf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigmaIdxFilt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_sgmIdxLpfStrg_mode_t),
        M4_DEFAULT(btnr_lpfStrgH_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(sigmaIdxLpf_en_group);
        M4_NOTES(The mode of sigma LUT used by the iir SPNR.. Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_btnr_preSpnrSigmaIdxFilt_mode
    btnr_sgmIdxLpfStrg_mode_t hw_btnrT_idxLpfStrg_mode;
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_btnr_preSpnrSigmaCtrl_scale),
       M4_TYPE(enum),
       M4_ENUM_DEF(btnr_preSpNrSgm_mode_t),
       M4_DEFAULT(btnr_lutSgmOnly_mode),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_GROUP_CTRL(preSpnrSigmaMode_group),
       M4_NOTES(TODO.\n
       Freq of use: low))  */
    // reg: hw_btnr_spnrPreSigmaUse_en
    btnr_preSpNrSgm_mode_t hw_btnrT_sigma_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigmaCtrl_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preSpnrSigmaMode_group:btnr_preLutSgmMix_mode),
        M4_NOTES(The scaling factor for the range sigma of the bilateral filter.\n
        Freq of use: low))  */
    // reg: sw_pre_sig_ctrl_scl
    float hw_btnrT_preSigma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor for the range sigma of the bilateral filter.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnrSigma_scale
    float hw_btnrT_sigma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigmaHdrSht_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor for the range sigma of the bilateral filter in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrSigmaHdrSht_scale
    float hw_btnrT_sigmaHdrS_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigma_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,255.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset para of the scaled sigma.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrSigma_offset
    float hw_btnrT_sigma_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigmaHdrSht_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,255.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset parameter of the scaled sigma in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrSigmaHdrSht_offset
    float hw_btnrT_sigmaHdrS_offset;
} btnr_preSpNr_sigma_dyn_t;


typedef enum btnr_rgeWgt_mode_e {
    // @reg: hw_btnr_preSpnrHiFilterWgt_Mode == 0
    // @reg: hw_btnr_preSpnrLoFilterWgt_Mode == 0
    btnr_bayerAvg_mode = 0,
    // @reg: hw_btnr_preSpnrHiFilterWgt_Mode == 1 && hw_btnr_preSpnrHiFilterRBWgt_Mode == 0
    // @reg: hw_btnr_preSpnrLoFilterWgt_Mode == 1 && hw_btnr_preSpnrLoFilterRBWgt_Mode == 0
    btnr_gIndividual_rbAvg_mode = 1,
    // @reg: hw_btnr_preSpnrHiFilterWgt_Mode == 1 && hw_btnr_preSpnrHiFilterRBWgt_Mode == 1
    // @reg: hw_btnr_preSpnrLoFilterWgt_Mode == 1 && hw_btnr_preSpnrLoFilterRBWgt_Mode == 1
    btnr_bayerIndividual_mode = 2,
} btnr_rgeWgt_mode_t;

typedef enum btnr_preSpnrGic_mode_e {
    // @reg: hw_btnr_preSpnrHiFilterGic_en == 0
    btnr_gicDisable_mode = 0,
    // @reg: (hw_btnr_preSpnrHiFilterGic_en == 1) && (hw_btnr_preSpnrHiFilterGicEnhance_en == 0)
    btnr_gicStrgL_mode = 1,
    // @reg: (hw_btnr_preSpnrHiFilterGic_en == 1) && (hw_btnr_preSpnrHiFilterGicEnhance_en == 1)
    btnr_gicStrgH_mode = 2
} btnr_preSpnrGic_mode_t;

typedef struct btnr_preSpNr_hiNr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(preHiFilt_en_group),
        M4_NOTES(The enable bit of the high freq filter that processes the iir frame data.\n
        Freq of use: low))  */
    // reg: !en = hw_btnr_preSpnrHifilter_bypass_en;
    bool hw_btnrT_hiNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiGuideFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(hiGuideFilt_en_group),
        M4_GROUP(preHiFilt_en_group)
        M4_NOTES(The enable bit of the high guide 3x3filter & 5x5 filter interpoloate that processes the iir frame data for iir high freq guide image.\n
        Freq of use: low))  */
    // reg: !en = hw_btnr_preSpnrHiGuidefilter_bypass_en;
    bool hw_btnrT_guideLpf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiGuideOut_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group;hiGuideFilt_en_group),
        M4_NOTES(The interpoloate weight of 3x3 gauss filter & 5x5 gauss filter for iir high freq guide output image.\n
        The bigger value, the larger weight of 3x3 guass filter.\n
        Freq of use: low))  */
    // reg: sw_prehi_guide_out_wgt
    float hw_btnrT_guideLpf3_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiNoiseCtrl_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(hiNoiseCtrl_en_group),
        M4_GROUP(preHiFilt_en_group)
        M4_NOTES(The enable bit of the high freq adjust guide filter by pk that processes the iir frame data.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrHiNoiseCtrl_en;
    bool hw_btnrT_guideNsCtrl_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiNoiseCtrl_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group;hiNoiseCtrl_en_group),
        M4_NOTES(IIR frame high freq adjust guide filter out offset.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrHiNoiseCtrl_offset
    float hw_btnrT_guideNsCtrl_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiNoiseCtrl_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,8.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group;hiNoiseCtrl_en_group),
        M4_NOTES(IIR frame high freq adjust guide filter out scale.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrHiNoiseCtrl_scale
    float hw_btnrT_guideNsCtrl_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_filtCfg_mode_t),
        M4_DEFAULT(btnr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(hifiltSpatialCfg_mode_group),
        M4_GROUP(preHiFilt_en_group),
        M4_NOTES(The way to configure the high freq filter weight in saptial. Reference enum types.\n
        Freq of use: low))  */
    btnr_filtCfg_mode_t sw_btnrT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiSpace_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
        M4_DEFAULT(8.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group;hifiltSpatialCfg_mode_group:btnr_cfgByFiltStrg_mode),
        M4_NOTES(This strength is used to adjust the spatial weight of the bilateral filter.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnrHifilter_coeff0~5
    float sw_btnrT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHifilter_coeff),
        M4_TYPE(u8),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,63),
        M4_DEFAULT([63, 63, 62, 60, 59, 56]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group;hifiltSpatialCfg_mode_group:btnr_cfgByFiltCoeff_mode),
        M4_NOTES(Adjust the spatial weight of the bilateral filter.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrHifilter_coeff0~5
    uint8_t sw_btnrT_filtSpatial_wgt[6];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group),
        M4_NOTES(This strength is used to adjust the range weight of the bilateral filter.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnrHiSigma_scale
    float hw_btnrT_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiWgtHiRbWgt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_rgeWgt_mode_t),
        M4_DEFAULT(btnr_bayerAvg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group),
        M4_NOTES(The way to configure the rggb 4 channel range weight calculate mode. Reference enum types.\n
        Freq of use: low))  */
    btnr_rgeWgt_mode_t sw_btnrT_rgeWgt_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiWgtCalc_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4.0),
        M4_DEFAULT(0.6),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group),
        M4_NOTES(IIR frame high freq filter weight calc scale.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrHiWgtCalc_scale
    float hw_btnrT_diffSgmRatio_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiFltWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.004,1.0),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preHiFilt_en_group),
        M4_NOTES(The scaled factor of the bias-adjusted neighborhood pixel diff in calculating the range weight of the bifilt.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrHiFilterWgt_minLimit
    float hw_btnrT_filtWgt_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(gicEn-gicEnhanceEn),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_preSpnrGic_mode_t),
        M4_DEFAULT(btnr_gicDisable_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preSpNr_en_group;preHiFilt_en_group;!gicLP_en_group),
        M4_NOTES(The way to configure the rggb 4 channel range weight calculate mode. Reference enum types.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrHifilterGic_en, hw_btnr_preSpnrHifilterGicEnhance_en;
    btnr_preSpnrGic_mode_t hw_btnrT_gic_mode;
} btnr_preSpNr_hiNr_dyn_t;

typedef struct btnr_preSpNr_loNr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrLoFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(preLoFilt_en_group),
        M4_NOTES(The enable bit of the high freq filter that processes the iir frame data.\n
        Freq of use: low))  */
    // reg: !en = hw_btnr_preSpnrLofilter_bypass_en;
    bool hw_btnrT_loNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrLoSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.45),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preLoFilt_en_group),
        M4_NOTES(This strength is used to adjust the range weight of the bilateral filter.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnrLoSigma_scale
    float hw_btnrT_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrLoWgtrLoRbWgt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_rgeWgt_mode_t),
        M4_DEFAULT(btnr_bayerAvg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preLoFilt_en_group;!avgRgeWgtLP_en_group),
        M4_NOTES(The way to configure the rggb 4 channel range weight calculate mode. Reference enum types.\n
        Freq of use: low))  */
    btnr_rgeWgt_mode_t sw_btnrT_rgeWgt_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrLoValWgtOut_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preLoFilt_en_group;!avgRgeWgtLP_en_group),
        M4_NOTES(4 channel avg(diff/sigma) * wgt  + avg(diff)/avg(sigma)(1 - wgt).\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrLoValWgtOut_wgt
    float hw_btnrT_ratAvgRgeWgt_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrLoWgtCalc_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4.0),
        M4_DEFAULT(0.6),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preLoFilt_en_group),
        M4_NOTES(IIR frame low freq filter weight calc scale.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrLoWgtCalc_scale
    float hw_btnrT_rgeWgt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrLoFltWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.004,1.0),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preLoFilt_en_group),
        M4_NOTES(The scaled factor of the bias-adjusted neighborhood pixel diff in calculating the range weight of the bifilt.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrLoFilterWgt_minLimit
    float hw_btnrT_filtWgt_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrLoFiltOut_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(preLoFilt_en_group),
        M4_NOTES(The alpha value of the output of the low-freq bifilter, when it is weighted with the high-filter pixel.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnrLoFilterOut_wgt
    float hw_btnrT_loNrOut_alpha;
} btnr_preSpNr_loNr_dyn_t;

typedef struct btnr_preSpNr_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnr_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(preSpNr_en_group),
        M4_NOTES(The enable bit of the spatial filter that processes the iir frame data.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnrHifilter_bypass_en && hw_btnr_preSpnrLofilter_bypass_en;
    bool sw_btnrT_spNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preFrmSpNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(preSpNr_en_group),
        M4_NOTES(TODO))  */
    btnr_preSpNr_sigma_dyn_t sigma;
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_btnr_preFrmSpNrHi),
       M4_TYPE(struct),
       M4_UI_MODULE(normal_ui_style),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(1),
       M4_GROUP(preSpNr_en_group),
       M4_NOTES(TODO))  */
    btnr_preSpNr_hiNr_dyn_t hiNr;
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_btnr_preFrmSpNrLo),
       M4_TYPE(struct),
       M4_UI_MODULE(normal_ui_style),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(1),
       M4_GROUP(preSpNr_en_group),
       M4_NOTES(TODO))  */
    btnr_preSpNr_loNr_dyn_t loNr;
} btnr_preSpNr_dyn_t;

typedef enum btnr_frmFusion_mode_e {
    //reg: prePixOut_mode = 0
    btnr_kalMdOnly_mode = 0,
    //reg: prePixOut_mode = 1
    btnr_kalMdMixKal_mode = 1
} btnr_frmFusion_mode_t;

typedef struct btnr_frmFusion_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_iirPixOut_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_frmFusion_mode_t),
        M4_DEFAULT(btnr_kalMdMixKal_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(frmFusion_mode_group),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_frmFusion_mode_t hw_btnrT_frmFusion_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_loMergeFrames_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of the low-frequency fusion weight of the IIR frame.\n
        Freq of use: high))  */
    // reg: hw_btnr_loWgtClip_minLimit
    float hw_btnrT_loFusion_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_loMergeFrames_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit of the low-frequency fusion weight of the IIR frame.\n
        Freq of use: high))  */
    // reg: hw_btnr_loWgtClip_maxLimit
    float hw_btnrT_loFusion_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_loMergeFramesHdrSht_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of the low-frequency fusion weight of the IIR frame in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_loWgtClipHdrSht_minLimit
    float hw_btnrT_loFusionHdrS_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_loMergeFramesHdrSht_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit of the low-frequency fusion weight of the IIR frame in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_loWgtClipHdrSht_maxLimit
    float hw_btnrT_loFusionHdrS_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_motionLoMergeFrames_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(4.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Pre frame lo freq filter weight max limit when motion detection.\n
        Freq of use: low))  */
    // reg: hw_btnr_loWgtMotionClip_maxLimit
    float hw_btnrT_loMotion_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loiirRbSoftThresh_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(frmFusion_mode_group:btnr_kalMdMixKal_mode),
        M4_NOTES(Motion Denoise out weight of noise balance.\n
        Freq of use: high))  */
    // reg: hw_btnr_loPreRbSoftThresh_scale high 11-21 bit
    float hw_btnrT_loRbSoftThd_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loiirGgSoftThresh_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(frmFusion_mode_group:btnr_kalMdMixKal_mode),
        M4_NOTES(Low frequency r and b compoment soft threshhold scale.\n
        Freq of use: low))  */
    // reg: hw_btnr_loPreGgSoftThresh_scale
    float hw_btnrT_loGgSoftThd_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loiirSoftThresh_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4096.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(frmFusion_mode_group:btnr_kalMdMixKal_mode),
        M4_NOTES(Low frequency soft threshhold minlimit.\n
        Freq of use: low))  */
    // reg: hw_btnr_loPreSoftThresh_minLimit
    float hw_btnrT_loSoftThd_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loiirSoftThresh_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4096.0),
        M4_DEFAULT(2000.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(frmFusion_mode_group:btnr_kalMdMixKal_mode),
        M4_NOTES(Low frequency soft threshhold minlimit.\n
        Freq of use: low))  */
    // reg: hw_btnr_loPreSoftThresh_maxLimit
    float hw_btnrT_loSoftThd_maxLimit;
} btnr_frmFusion_dyn_t;

typedef enum btnr_outFrmBase_mode_e {
    //@para: (outUsePre_en == 0)
    btnr_curBaseOut_mode = 0,
    //@para: (outUsePre_en == 1)
    btnr_preBaseOut_mode = 1
} btnr_outFrmBase_mode_t;

typedef enum btnr_nsBal_curBaseOut_mode_e {
    //@reg outUsePreMode == 0b101     @para: (outUseMidNoiseBalNr_en == 1)
    btnr_balPre3Freq_mode = 5,
    //@reg outUsePreMode == 0b110     @para: (outUseMidNoiseBalNr_en == 0)
    btnr_balPre2Freq_mode = 6,
    //@reg outUsePreMode == 0b000
    btnr_balCurPreHiOrg_mode = 0,
} btnr_nsBal_curBaseOut_mode_t;

typedef struct btnr_noiseBal_curBaseOut_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_outUseMidNoiseBalNr_en),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_nsBal_curBaseOut_mode_t),
        M4_DEFAULT(btnr_balPre3Freq_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(curNoiseBal_mode_group),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_nsBal_curBaseOut_mode_t sw_btnrT_noiseBal_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_outUseHiNoiseBalNr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1048576.0),
        M4_DEFAULT(16),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(curNoiseBal_mode_group:btnr_balPre3Freq_mode|btnr_balPre2Freq_mode),
        M4_NOTES(High freq out weight of noise balance.\n
        Freq of use: high))  */
    // reg: hw_btnr_outUseHiNoiseBalNr_strg low 0-10 bit
    float hw_btnrT_hiNoiseBal_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_outUseMdNoiseBalNr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1048576.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(curNoiseBal_mode_group:btnr_balPre3Freq_mode|btnr_balPre2Freq_mode),
        M4_NOTES(Motion Denoise out weight of noise balance.\n
        Freq of use: high))  */
    // reg: hw_btnr_outUseHiNoiseBalNr_strg high 11-21 bit
    float hw_btnrT_midNoiseBal_strg;
    /*
    * Note:It is only valid when outUsePreMode == 0b000. But the mode(outUsePreMode == 0b000) is under improvement and is not open to the public temporarily.
    */
#if 1
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrHiWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(curNoiseBal_mode_group:btnr_balCurPreHiOrg_mode),
        M4_NOTES(The alpha value of the high-frequency data separated by spatial filtering for cur frame.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrHiWgt_minLimit
    float hw_btnrT_curHiOrg_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(curNoiseBal_mode_group:btnr_balCurPreHiOrg_mode),
        M4_NOTES(The alpha value of the high-frequency data separated by spatial filtering for iir frame.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrHiWgt_minLimit
    float hw_btnrT_iirHiOrg_alpha;
#endif
} btnr_noiseBal_curBaseOut_t;

typedef enum btnr_nsBal_preBaseOut_mode_e {
    //@reg outUsePreMode == 0b010     @para: (useCurWgt_en == 0) && (outUseMidNoiseBalNr_en == 1)
    btnr_pre3FreqBal_mode = 2,
    //@reg outUsePreMode == 0b100     @para: (useCurWgt_en == 0) && (outUseMidNoiseBalNr_en == 0)
    btnr_pre2FreqBal_mode = 4,
    //@reg outUsePreMode == 0b001     @para: (useCurWgt_en == 1) && (outUseMidNoiseBalNr_en == 1)
    btnr_pre3FreqAndCurBal_mode = 1,
    //@reg outUsePreMode == 0b011     @para: (useCurWgt_en == 1) && (outUseMidNoiseBalNr_en == 0)
    btnr_pre2FreqAndCurBal_mode = 3
} btnr_nsBal_preBaseOut_mode_t;

typedef struct btnr_noiseBal_preBaseOut_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_outUseMidNoiseBalNr_en_and_useCurWgt_en),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_nsBal_preBaseOut_mode_t),
        M4_DEFAULT(btnr_pre3FreqAndCurBal_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    btnr_nsBal_preBaseOut_mode_t sw_btnrT_noiseBal_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_outUseHiNoiseBalNr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1048576.0),
        M4_DEFAULT(16),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(High freq out weight of noise balance.\n
        Freq of use: high))  */
    // reg: hw_btnr_outUseHiNoiseBalNr_strg low 0-10 bit
    float hw_btnrT_hiNoiseBal_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnr_outUseMdNoiseBalNr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1048576.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Motion Denoise out weight of noise balance.\n
        Freq of use: high))  */
    // reg: hw_btnr_outUseHiNoiseBalNr_strg high 11-21 bit
    float hw_btnrT_midNoiseBal_strg;
} btnr_noiseBal_preBaseOut_t;

typedef enum btnr_sigma_mode_e {
    btnr_autoSigma_mode = 0,
    btnr_manualSigma_mode = 1
} btnr_sigma_mode_t;

typedef struct btnr_mdSigmaCurve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,20),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    uint16_t idx[BTNR_MD_SIGMACURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
       M4_ALIAS(val),
       M4_TYPE(u16),
       M4_UI_PARAM(data_y),
       M4_SIZE_EX(1,20),
       M4_RANGE_EX(0,4095),
       M4_DEFAULT(256),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(1),
       M4_NOTES(TODO))  */
    uint16_t val[BTNR_MD_SIGMACURVE_SEGMENT_MAX];
} btnr_mdSigmaCurve_s;

typedef struct btnr_spNrSigmaCurve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,16),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    uint16_t idx[BTNR_SPNR_SIGMACURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(val),
        M4_TYPE(u16),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,16),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(128),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    uint16_t val[BTNR_SPNR_SIGMACURVE_SEGMENT_MAX];
} btnr_spNrSigmaCurve_t;

typedef struct btnr_sigma_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrCfg_sigma_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_sigma_mode_t),
        M4_DEFAULT(btnr_autoSigma_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(autoSigma_mode_group),
        M4_NOTES(Configure the mode of sigma data used by the filter\n
        Freq of use: low))  */
    btnr_sigma_mode_t sw_btnrCfg_sigma_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_tnrAutoSigmaCountTh),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1048575),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(autoSigma_mode_group:btnr_autoSigma_mode),
        M4_NOTES(The sigma statistics is valid only when the number of pixels counted exceeds the threshold.\n
        Freq of use: low))  */
    // reg: hw_btnr_tnrAutoSigmaCountTh
    uint32_t hw_btnrCfg_statsPixCnt_thred;
} btnr_sigma_static_t;

typedef struct btnr_sigmaAttrib_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrCfg_pixDomain_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_pixDomain_mode_t),
        M4_DEFAULT(btnr_pixLog2Domain_mode),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(The pixDomain mode used on isp calibration tuning, read only.\n
        Freq of use: low))  */
    btnr_pixDomain_mode_t hw_btnrCfg_pixDomain_mode;
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_btnr_transfMode_scale),
       M4_TYPE(enum),
       M4_ENUM_DEF(btnr_trans_mode_t),
       M4_DEFAULT(btnr_pixInBw15b_mode),
       M4_DIGIT_EX(0),
       M4_HIDE_EX(0),
       M4_RO(1),
       M4_ORDER(0),
       M4_NOTES(Scale used for transformation.\n
       Freq of use: low))  */
    // reg: hw_btnr_transfMode_scale
    btnr_trans_mode_t hw_btnrCfg_trans_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_transfMode_offset),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8191),
        M4_DEFAULT(256),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(Offset used for transformation.\n
        Freq of use: low))  */
    // reg: hw_btnr_transfMode_offset
    int hw_btnrCfg_trans_offset;
    /* M4_GENERIC_DESC(
       M4_ALIAS(ob_offset),
       M4_TYPE(u16),
       M4_SIZE_EX(1,1),
       M4_RANGE_EX(0,511),
       M4_DEFAULT(0),
       M4_HIDE_EX(0),
       M4_RO(1),
       M4_ORDER(0),
       M4_NOTES(The ob_offset value used on isp calibration tuning, read only.\n
       Freq of use: low))  */
    uint16_t ob_offset;
    /* M4_GENERIC_DESC(
       M4_ALIAS(predgain),
       M4_TYPE(f32),
       M4_SIZE_EX(1,1),
       M4_RANGE_EX(0,16.0),
       M4_DEFAULT(1.0),
       M4_DIGIT_EX(3),
       M4_HIDE_EX(0),
       M4_RO(1),
       M4_ORDER(0),
       M4_NOTES(The predgain value used on isp calibration tuning, read only.\n
       Freq of use: low))  */
    float predgain;
} btnr_sigmaAttrib_t;

typedef struct btnr_sigmaEnv_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_autoSigmaCountWgt_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.8),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(autoSigma_mode_group:btnr_autoSigma_mode),
        M4_NOTES(Only pixels with a static weight value exceeding the threshold are subjected to sigma statistics.\n
        Freq of use: low))  */
    // reg: hw_btnr_autoSigmaCountWgt_thred
    float hw_btnrT_statsPixAlpha_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_autoSgmIIR_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0.7),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(autoSigma_mode_group:btnr_autoSigma_mode),
        M4_NOTES(The weight of previous sigma value with current sigma value on auto sigma count mode.\n
        Freq of use: low))  */
    float sw_btnrT_autoSgmIIR_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sigmaAttrib),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_sigmaAttrib_t hw_btnrC_sigmaAttrib;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrLuma2sigma),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The sigma calibration curve of the spatial filter used to process the iir frame data. \n
        The filter only use the sigma data of the curve when sw_btnrCfg_sigma_mode == btnr_manualSigma_mode.\n
        The filter will use the sigma data of the curve and sigma statistics when sw_btnrCfg_sigma_mode == btnr_autoSigma_mode.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrLuma2sigma_idx0 ~ 15, hw_btnr_preSpnrLuma2sigma_val0 ~ 15
    btnr_spNrSigmaCurve_t hw_btnrC_preSpNrSgm_curve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_tnrLuma2Sigma),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The sigma calibration curve of the MD module. \n
        The MD only use the sigma data of the curve when sw_btnrCfg_sigma_mode == btnr_manualSigma_mode.\n
        The MD will use the sigma data of the curve and sigma statistics when sw_btnrCfg_sigma_mode == btnr_autoSigma_mode.\n
        Freq of use: low))  */
    // @reg:hw_btnr_tnrLuma2Sigma_idx0~19, hw_btnr_tnrLuma2Sigma_val0~19
    btnr_mdSigmaCurve_s hw_btnrC_mdSigma_curve;
} btnr_sigmaEnv_dyn_t;

typedef struct btnr_locSgmStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_gain_max),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( TODO.\n
        Freq of use: low))  */
    // @reg:hw_btnr_gain_max
    float hw_bnrT_locSgmStrg_maxLimit;
} btnr_locSgmStrg_dyn_t;

typedef enum btnr_dbgOutMux_mode_e {
    // reg: hw_btnr_iirspnr_out_en == 1
    btnr_dbgOut_preSpNr_mode = 0,
    // reg: hw_btnr_curspnr_out_en == 1
    btnr_dbgOut_curSpNr_mode = 1,
    // reg: hw_btnr_mdWgtOut_en == 1
    btnr_dbgOut_mdWgt_mode = 2
} btnr_dbgOutMux_mode_t;

typedef enum btnr_dbgPredgain_mode_e {
    // Dynamic predgain
    btnr_vendorDefault_mode = 0,
    // Pregain is configured by the user
    btnr_usrConfig_mode = 1
} btnr_dbgPredgain_mode_t;

typedef struct btnr_predgainWkArd_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_predgainWkArd_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(predgainWkArd_en_group),
        M4_NOTES(The enable bit for using dynamic/usrConfig pregain\n
        Freq of use: low))  */
    bool sw_btnrT_predgainWkArd_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_predgain_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_dbgPredgain_mode_t),
        M4_DEFAULT(btnr_vendorDefault_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(predgainWkArd_en_group),
        M4_GROUP_CTRL(dbgPredgain_mode_group),
        M4_NOTES(Reference enum types.\n
        Freq of use: low))  */
    btnr_dbgPredgain_mode_t sw_btnrT_predgain_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_predgain_curve),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,8),
        M4_DEFAULT([1,1,1,1,1,1,1,1,1,1,1,1,1]),
        M4_GROUP(predgainWkArd_en_group; dbgPredgain_mode_group:btnr_usrConfig_mode),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Users can directly configure the predgain curve through dbgPregian_val when dbgPredgain_mode ==
        btnr_usrConfig_mode.\n Freq of use: low))  */
    float hw_btnrT_predgain_curve[BTNR_ISO_SEGMENT_MAX];
} btnr_predgainWkArd_t;

typedef struct btnr_debug_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_dbgOut_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(dbgOut_en_group),
        M4_NOTES(The enable bit for debugging data to replace pixel data output\n
        Freq of use: high))  */
    // reg: (hw_btnr_iirspnr_out_en == 0) && (hw_btnr_curspnr_out_en == 0) && (hw_btnr_mdWgtOut_en == 0)
    bool sw_btnrT_dbgOut_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_dbgOut_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_dbgOutMux_mode_t),
        M4_DEFAULT(btnr_dbgOut_mdWgt_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(dbgOut_en_group),
        M4_NOTES( Reference enum types.\n
        Freq of use: high))  */
    btnr_dbgOutMux_mode_t hw_btnrT_dbgOut_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(predgainWkArd),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(pixDomain_mode_group:btnr_pixLog2Domain_mode),
        M4_NOTES(TODO))  */
    btnr_predgainWkArd_t predgainWkArd;
} btnr_debug_t;

typedef struct btnr_preSpnr_hiNrLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_preHi_bf_lp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(IIR frame high freq bilafilter low power mode enable.\n
        Freq of use: low))  */
    // reg: (hw_preHi_bf_lp_en)
    bool hw_btnrCfg_bfLP_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_preHi_gic_lp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(gicLP_en_group),
        M4_NOTES(IIR frame high freq green imbanlace correction low power mode enable.\n
        Freq of use: low))  */
    // reg: (hw_preHi_gic_lp_en)
    bool hw_btnrCfg_gicLP_en;
} btnr_preSpnr_hiNrLP_t;

typedef struct btnr_preSpnr_loNrLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_preLo_avg_lp_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(avgRgeWgtLP_en_group),
        M4_NOTES(IIR frame low freq channel averge low power mode enable.\n
        Freq of use: low))  */
    // reg: (hw_preLo_avg_lp_en)
    bool hw_btnrCfg_avgRgeWgtLP_en;
} btnr_preSpnr_loNrLP_t;

typedef struct btnr_preSpnrLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiNrLP),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_preSpnr_hiNrLP_t hiNrLP;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNrLP),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_preSpnr_loNrLP_t loNrLP;
} btnr_preSpnrLP_t;

typedef struct btnr_cfgLP_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(preSpnrLP),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_preSpnrLP_t preSpnrLP;
} btnr_cfgLP_t;

typedef enum btnr_megWgtDs_mode_e {
    // reg: hw_btnr_megWgtDs_mode == 0
    btnr_megWgtDs_avg_mode = 0,
    // reg: hw_btnr_megWgtDs_mode == 1
    btnr_megWgtDs_max_mode = 1,
} btnr_megWgtDs_mode_t;

typedef enum btnr_kalWgtDs_mode_e {
    // reg: hw_btnr_kalmanWgtDs_mode == 0
    btnr_kalWgtDs_avg_mode = 0,
    // reg: hw_btnr_kalmanWgtDs_mode == 1
    btnr_kalWgtDs_max_mode = 1,
    // reg: hw_btnr_kalmanWgtDs_mode == 2
    btnr_kalWgtDs_min_mode = 2
} btnr_kalWgtDs_mode_t;

typedef enum btnr_fusionIsoSw_mode_e {
    btnr_limitFixed_mode = 0,
    btnr_limitAdj_mode = 1,
} btnr_fusionIsoSw_mode_t;

typedef struct btnr_fusionIsoSw_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_fusionIso_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_fusionIsoSw_mode_t),
        M4_DEFAULT(btnr_limitFixed_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(btnr_fusionIso_mode_group),
        M4_NOTES( Reference enum types.\n
        Freq of use: low))  */
    btnr_fusionIsoSw_mode_t sw_btnrT_fusionIso_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_limitAdj_deltaOB),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,511),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(btnr_fusionIso_mode_group:btnr_limitAdj_mode),
        M4_NOTES(The delta ob_offset of current frame and last frame is bigger the this thred, the fusion limitAdj will have effect.\n
        Freq of use: low))  */
    int sw_btnrT_limitAdj_deltaOB;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_limitAdj_strg),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(btnr_fusionIso_mode_group:btnr_limitAdj_mode),
        M4_NOTES(The tnr fusion weight will change with ob offset.This param represent the max fusion frame.\n
        Freq of use: low))  */
    int sw_btnrT_limitAdj_strg;

} btnr_fusionIsoSw_t;

typedef struct btnr_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrCfg_pixDomain_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_pixDomain_mode_t),
        M4_DEFAULT(btnr_pixLog2Domain_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(pixDomain_mode_group),
        M4_NOTES( Reference enum types.\n
        Freq of use: low))  */
    btnr_pixDomain_mode_t hw_btnrCfg_pixDomain_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(transCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_transCfg_t transCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sigmaEnv),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_sigma_static_t sigmaEnv;
    /* M4_GENERIC_DESC(
        M4_ALIAS(debug),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_debug_t debug;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lowPower),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_cfgLP_t lowPowerCfg;
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_btnrCfg_megWgtDs_mode),
       M4_TYPE(enum),
       M4_ENUM_DEF(btnr_megWgtDs_mode_t),
       M4_DEFAULT(btnr_megWgtDs_avg_mode),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES( Reference enum types.\n
       Freq of use: low))  */
    btnr_megWgtDs_mode_t hw_btnrCfg_megWgtDs_mode;
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_btnrCfg_pixDomain_mode),
       M4_TYPE(enum),
       M4_ENUM_DEF(btnr_kalWgtDs_mode_t),
       M4_DEFAULT(btnr_kalWgtDs_avg_mode),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES( Reference enum types.\n
       Freq of use: low))  */
    btnr_kalWgtDs_mode_t hw_btnrCfg_kalWgtDs_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_fusionIso),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_fusionIsoSw_t sw_fusionIso;
} btnr_params_static_t;

typedef struct btnr_md_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_md_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(md_en_group),
        M4_NOTES( ...
        Freq of use: high))  */
    // reg: en = !hw_btnr_MotionDetect_bypass_en
    bool hw_btnrT_md_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdSigma),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group),
        M4_NOTES(TODO))  */
    btnr_mdSigma_t mdSigma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd0),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group&&loMd_en_group&&((loMd_mode_group:btnr_allSubLoMdMix_mode)||(loMd_mode_group:btnr_subDeepLoMdOnly_mode&&dLoMd_en_group&&dLoSrc_mode_group:btnr_subLoMd0_mode)||(loMd_mode_group:btnr_subLoMd0Only_mode))),
        M4_NOTES(TODO))  */
    btnr_subLoMd0_dyn_t subLoMd0;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd1),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group&&loMd_en_group&&((loMd_mode_group:btnr_allSubLoMdMix_mode|btnr_subLoMd1Only_mode)||(loMd_mode_group:btnr_subDeepLoMdOnly_mode&&dLoMd_en_group&&dLoSrc_mode_group:btnr_subLoMd1_mode))),
        M4_NOTES(TODO))  */
    btnr_subLoMd1_dyn_t subLoMd1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subDeepLoMd),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group),
        M4_NOTES(TODO))  */
    btnr_subDeepLoMd_dyn_t subDeepLoMd;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loMd),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group),
        M4_NOTES(TODO))  */
    btnr_loMd_dyn_t loMd;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_WgtCal_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_md_mode_t),
        M4_DEFAULT(btnr_loAsBias_hi1_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(md_mode_group),
        M4_GROUP(md_en_group),
        M4_NOTES(Mode of low-frequency motion detection. Reference enum types.\n
        Freq of use: low))  */
    //@reg: hw_btnr_WgtCal_mode
    btnr_md_mode_t hw_btnrT_md_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(WgtCal_Mode0),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group;md_mode_group:btnr_loAsRatio_hi0_mode),
        M4_NOTES(TODO))  */
    btnr_loAsHiRatioMd_dyn_t loAsRatio_hiMd0;
    /* M4_GENERIC_DESC(
        M4_ALIAS(WgtCal_Mode1),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group;md_mode_group:btnr_loAsBias_hi1_mode),
        M4_NOTES(TODO))   */
    btnr_loAsHiBiasMd_dyn_t loAsBias_hiMd1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdWgtFilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group),
        M4_NOTES(TODO))   */
    btnr_mdWgtFilt_dyn_t mdWgtFilt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(me),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(md_en_group),
        M4_NOTES(TODO))   */
    btnr_memc_dyn_t memc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(frmAlpha),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(md_en_group),
        M4_NOTES(TODO))  */
    btnr_frmFusion_dyn_t frmFusion;
} btnr_mdMe_dyn_t;

typedef struct btnr_other_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sigmaEnv),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    btnr_sigmaEnv_dyn_t sigmaEnv;
    /* M4_GENERIC_DESC(
        M4_ALIAS(curSpNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_curSpNr_dyn_t curSpNr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(preSpNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_preSpNr_dyn_t preSpNr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(outUsePre_en),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_outFrmBase_mode_t),
        M4_DEFAULT(btnr_preBaseOut_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(outFrmBase_mode_group),
        M4_NOTES(Mode of low-frequency motion detection. Reference enum types.\n
        Freq of use: low))  */
    //@reg: hw_btnr_outUsePre_mode
    //@para: outUsePre_en
    btnr_outFrmBase_mode_t sw_btnrT_outFrmBase_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseBal_preBaseOut),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(outFrmBase_mode_group:btnr_preBaseOut_mode),
        M4_NOTES(TODO))  */
    btnr_noiseBal_preBaseOut_t noiseBal_preBaseOut;
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseBal_curBaseOut),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(outFrmBase_mode_group:btnr_curBaseOut_mode),
        M4_NOTES(TODO))  */
    btnr_noiseBal_curBaseOut_t noiseBal_curBaseOut;
    /* M4_GENERIC_DESC(
        M4_ALIAS(locSgmStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_locSgmStrg_dyn_t locSgmStrg;
} btnr_other_dyn_t;

typedef struct btnr_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdDyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    btnr_mdMe_dyn_t mdMeDyn;
    /* M4_GENERIC_DESC(
        M4_ALIAS(spNrDyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    btnr_other_dyn_t spNrDyn;
} btnr_param_t;

#endif

