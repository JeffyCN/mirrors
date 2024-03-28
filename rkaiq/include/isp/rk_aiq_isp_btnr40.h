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

#ifndef _RK_AIQ_PARAM_BTNR40_H_
#define _RK_AIQ_PARAM_BTNR40_H_

#define BTNR_ISO_SEGMENT_MAX                     13
#define BTNR_SPNR_SIGMACURVE_SEGMENT_MAX         16
#define BTNR_MD_SIGMACURVE_SEGMENT_MAX           20

typedef enum {
    // @note: By configuring the strength coefficient, the software internally calculates and generates filter coefficients based on the strength coefficient to configure the filter.
    btnr_cfgByFiltStrg_mode = 0,
    // @note: Configure the filter by directly configuring the filter coefficients.
    btnr_cfgByFiltCoeff_mode = 1
} btnr_filtCfg_mode_t;


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
        M4_ALIAS(hw_btnrT_vIIRFstLn_scale),
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
        M4_ALIAS(hw_btnrT_loDiffVfilt_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.875),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The Feedback weight of filtered pixel output results within the neighborhood of the IIR filter.\n
        The larger the value, the stronger the filtering strength\n
        Freq of use: high))  */
    // reg: hw_btnrT_loDiffVfilt_wgt
    float hw_btnrT_vIIRFilt_strg;

} btnr_subLoMd0_diffCh_dyn_t;

typedef enum btnr_sigmaCurve_mode_e {
    // reg: hw_btnr_tnrSigmaCurveDoube_ens == 0
    // reg: hw_btnr_curSpnrsigmaCurveDouble_en == 0
    // reg: hw_btnr_preSpnrsigmaCurveDouble_en == 0
    btnr_midSegmInterpOn_mode = 0,
    // reg: hw_btnr_tnrSigmaCurveDoube_ens == 1
    // reg: hw_btnr_curSpnrsigmaCurveDouble_en == 1
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
        Freq of use: high))  */
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
} btnr_mdSigma_t;

 typedef struct btnr_subLoMd0_sgmCh_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_sigHfilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
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
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_sigVfilt_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.875),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The Feedback weight of filtered pixel output results within the neighborhood of the IIR filter.\n
        The larger the value, the stronger the filtering strength\n
        Freq of use: high))  */
    // reg: hw_btnr_sigVfilt_wgt
    float hw_btnrT_vIIRFilt_strg;
} btnr_subLoMd0_sgmCh_dyn_t;

 typedef struct btnr_subLoMd0_wgtOpt_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_LoDiffWgtCal_Offset),
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
        M4_ALIAS(hw_btnr_LoDiffWgtCal_scale),
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

typedef struct btnr_subLoMd1_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_lpfLo_bypass),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The enable bit of the LPF that processes the diff data to obtain low-frequency signals.
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
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    btnr_filtCfg_mode_t sw_btnrT_lpfCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_lpfLo_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0.0,100.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The spatial weight coeff of the LPF is directly configured through this para when lpfCfg_mode is btnr_cfgByFiltCoeff_mode.\n
        Freq of use: low))  */
    // reg: hw_btnr_lpfLo_coeff0~8
    float hw_btnrT_lpfSpatial_wgt[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_lpfLo_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,100.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The spatial weight coeff of the LPF is configured through this para when lpfCfg_mode is btnr_cfgByFiltStrg_mode.\n
        Freq of use: high))  */
    // reg: hw_btnr_lpfLo_coeff0~8
    float sw_btnrT_lpf_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_loWgtCal_maxLimit),
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
        M4_ALIAS(hw_btnrT_loWgtCal_offset),
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
        M4_ALIAS(hw_btnrT_loWgt_first_off_en),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.45),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The first line uses a different offset value from other rows when the bit is enabled.\n
        Freq of use: low))  */
    // reg: hw_btnrT_loWgt_first_off_en
    float hw_btnrT_mdWgtFstLnNegOff_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_loWgt_first_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.45),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The para is used to adjust the bias of the original motion weight of the first line when mdWgtFstLnNegOff_en==1\n
        Freq of use: low))  */
    // reg: hw_btnrT_loWgt_first_offset
    float hw_btnrT_mdWgtFstLn_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_loWgtCal_scale),
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
        M4_ALIAS(hw_btnrT_loWgtVfilt_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.875),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The Feedback weight of filter output results within the neighborhood of the IIR filter that filters the motion weight.\n
        Freq of use: high))  */
    // reg: hw_btnrT_loWgtVfilt_wgt
    float hw_btnrT_vIIRFilt_strg;
} btnr_subLoMd1_dyn_t;

typedef enum btnr_loMd_mode_e {
    // @reg: hw_btnr_LoMgeWgt_mode == 0
    // @note: "mdWgt = fun(loMd0Wgt, loMd1Wgt)"
    btnr_subLoMd01Mix_mode = 0,
    // @reg: hw_btnr_LoMgeWgt_mode == 1
    // @note: "mdWgt =loMd0Wgt"
    btnr_subLoMd0Only_mode = 1,
    // @reg: hw_btnr_LoMgeWgt_mode == 2
    // @note: "mdWgt =loMd1Wgt"
    btnr_subLoMd1Only_mode = 2
} btnr_loMd_mode_t;

typedef struct btnr_loMd_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_LoMgeWgt_mode),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable the low frequency motion detection.\n
        Freq of use: high))  */
    // reg: hw_btnr_LoMgeWgt_mode==3;
    bool hw_btnrT_loMd_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_WgtCal_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_mdWgtMeg_mode_t),
        M4_DEFAULT(btnr_subLoMd01Mix_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of low frequency motion detection. Reference enum types.\n
        Freq of use: high))  */
    // @reg: hw_btnr_WgtCal_mode
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
        M4_NOTES(The scaling factor of the weight value of the previous frame before it is weighted with the cur weight.\n
        Freq of use: low))  */
    // reg: hw_btnr_LoMgePreWgt_scale
    float hw_btnrT_preWgtMge_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_LoMgePreWgt_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset parameter of the scaled weight value of the previous frame.\n
        Freq of use: low))  */
    // reg: hw_btnr_LoMgePreWgt_offset
    float hw_btnrT_preWgtMge_offset;
}btnr_loMd_dyn_t;

typedef enum btnr_md_mode_e {
    // @reg: hw_btnr_wgtCal_mode == 0
    btnr_loAsRatioForHi_mode = 0,
    // @reg: hw_btnr_wgtCal_mode == 1
    btnr_loAsBiasForHi_mode = 1,
    // @reg: hw_btnr_lomdWgt_dbg_en == 1
    btnr_loMdOnly_mode = 2
} btnr_md_mode_t;

typedef struct btnr_loAsHiRatioMd_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_LpfHi_bypass_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
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
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    btnr_filtCfg_mode_t sw_btnrT_hfLpfCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_hflpfLo_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0.0,100.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The spatial weight coeff of the LPF is directly configured through this para when hfLpfCfg_mode is btnr_cfgByFiltCoeff_mode.\n
        Freq of use: low))  */
    // reg: hw_btnr_lpfLo_coeff0~8
    float hw_btnrT_hfLpfSpatial_wgt[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_hflpfLo_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,100.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The spatial weight coeff of the LPF is configured through this para when hfLpfCfg_mode is btnr_cfgByFiltStrg_mode.\n
        Freq of use: low))  */
    // reg: hw_btnr_LpfHi_coeff0~8
    float sw_btnrT_hfLpf_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode0LoWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
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
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of high-frequency motion weight\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode0Base_ratio
    float hw_btnrT_hiMdWgt_scale;
} btnr_loAsHiRatioMd_dyn_t;

typedef struct btnr_loAsHiBiasMd_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1LoWgt_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
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
        M4_DEFAULT(1.0),
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
        M4_DEFAULT(1.0),
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
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The 2nd bias parameter of the low-frequency stationary weight value after scaling.\n
        Freq of use: high))  */
    // reg: hw_btnr_Mode1Wgt_offset
    float hw_btnrT_loWgtStat_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1Wgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The minimum limit of low-frequency stationary weight values after 2 biases.\n
        Freq of use: high))  */
    // reg: hw_btnr_Mode1Wgt_minLimit
    float hw_btnrT_loWgtStat_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1Wgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor for final weight calculation.\n
        Freq of use: high))  */
    // reg: hw_btnr_Mode1Wgt_scale
    float hw_btnrT_mdWgt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_Mode1Wgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor for final weight calculation in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_Mode1WgtHdrS_scale
    float hw_btnrT_mdWgtHdrS_scale;
}btnr_loAsHiBiasMd_dyn_t;

typedef struct btnr_mdWgtPost_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_WgtFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The enable bit of the LPF that processes the mdWgt data from MD module\n
        Freq of use: high))  */
    // reg: hw_btnr_WgtFilt_en
    float hw_btnrT_lpf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_WgtFilt_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,4.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES( Bay3d weight filter coeff.\n
        Freq of use: low))  */
    // @reg: hw_btnr_WgtFilt_coeff0~2
    float hw_btnr_lpfSpatial_wgt[3];
}btnr_mdWgtPost_dyn_t;

typedef struct btnr_curSpNr_dyn_s{
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnr_bypass_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of the spatial filter that processes the cur frame data.\n
        Freq of use: high))  */
    // reg: en = !hw_btnr_curSpnr_bypass_en;
    bool hw_btnrT_spNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSigmaIdxFilt_bypass_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit for sigma LUT index filtering.\n
        Freq of use: low))  */
    // reg: en = !hw_btnr_curSpnrSigmaIdxFilt_bypass_en;
    bool hw_btnrT_sigmaIdxLpf_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSigmaCurveDoube_ens),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_sigmaCurve_mode_t),
        M4_DEFAULT(btnr_midSegmInterpOff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of sigma LUT used by the cur SPNR. Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_btnr_curSpnrSigmaCurveDoube_ens
    btnr_sigmaCurve_mode_t hw_btnrT_sigmaCurve_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor for the range sigma of the bilateral filter.\n
        Freq of use: high))  */
    // reg: hw_btnr_curSpnrSigma_scale
    float hw_btnrT_sigma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSigmaHdrSht_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor for the range sigma of the bilateral filter in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrSigmaHdrSht_scale
    float hw_btnrT_sigmaHdrS_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSigma_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset para of the scaled sigma.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrSigma_offset
    float hw_btnrT_sigma_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSigmaHdrSht_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset parameter of the scaled sigma in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrSigmaHdrSht_offset
    float hw_btnrT_sigmaHdrS_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_filtCfg_mode_t),
        M4_DEFAULT(btnr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    btnr_filtCfg_mode_t sw_btnrT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSpaceRb_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,100.0),
        M4_DEFAULT(25),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This strength is used to adjust the spatial weight of the bilateral filter.\n
        Freq of use: high))  */
    // reg: hw_btnr_curSpnrSpaceRb_wgt0~8, hw_btnr_curSpnrSpaceGg_wgt0~8
    float sw_btnrT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrSpaceRb_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0.0,100.0),
        M4_DEFAULT(25),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This strength is used to adjust the spatial weight of the bilateral filter.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrSpaceRb_wgt0~8, hw_btnr_curSpnrSpaceGg_wgt0~8
    float sw_btnrT_filtSpatial_wgt[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrPixDiff_maxLimit),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4095),
        M4_DEFAULT(4095),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The maximum limit of the diff value of neighboring pixels of bilateral filter.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrPixDiff_maxLimit
    uint16_t hw_btnrT_pixDiff_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrWgtCal_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset para of the neighborhood pixel diff when calculating the range weight of the bifilt.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrWgtCal_offset
    float hw_btnrT_pixDiff_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrWgtCal_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.15),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaled factor of the bias-adjusted neighborhood pixel diff in calculating the range weight of the bifilt.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrWgtCal_scale
    float hw_btnrT_pixDiff_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnr_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The alpha value of the output of the bifilter, when it is weighted with the input pixel.\n
        Freq of use: high))  */
    // reg: hw_btnr_curSpnr_wgt
    float hw_btnrT_spNrOut_alpha;
}btnr_curSpNr_dyn_t;

typedef enum btnr_preSpNrSgm_mode_e {
    // @reg: hw_btnr_wgtCal_mode == 0
    btnr_lutSgmOnly_mode = 0,
    // @reg: hw_btnr_wgtCal_mode == 1
    btnr_kalPkSgm_mode = 1,
} btnr_preSpNrSgm_mode_t;

typedef struct btnr_preSpNr_dyn_s{
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnr_bypass_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit of the spatial filter that processes the iir frame data.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnr_bypass_en;
    bool hw_btnrT_spNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSigmaIdxFilt_bypass_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The enable bit for sigma LUT index filtering.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrSigmaIdxFilt_bypass_en;
    bool hw_btnrT_sigmaIdxLpf_en;
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
        M4_ALIAS(hw_btnr_preSpnrSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,256.0),
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
        M4_RANGE_EX(0.0,256.0),
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
        M4_RANGE_EX(0.0,16.0),
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
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset parameter of the scaled sigma in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrSigmaHdrSht_offset
    float hw_btnrT_sigmaHdrS_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_spnrPreSigmaUse_en),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_iirSpNrSgm_mode_t),
        M4_DEFAULT(btnr_lutSgmOnly_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO,
        Freq of use: low))  */
    // reg: hw_btnr_spnrPreSigmaUse_en
    btnr_preSpNrSgm_mode_t hw_btnrT_sigma_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_filtCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_filtCfg_mode_t),
        M4_DEFAULT(btnr_cfgByFiltStrg_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
        Freq of use: low))  */
    btnr_filtCfg_mode_t sw_btnrT_filtCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrSpaceRb_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,100.0),
        M4_DEFAULT(25),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This strength is used to adjust the spatial weight of the bilateral filter.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnrSpaceRb_wgt0~8, hw_btnr_preSpnrSpaceGg_wgt0~8
    float sw_btnrT_filtSpatial_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS( hw_btnr_preSpnrSpaceRb_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0.0,100.0),
        M4_DEFAULT(25),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This strength is used to adjust the spatial weight of the bilateral filter.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrSpaceRb_wgt0~8, hw_btnr_preSpnrSpaceGg_wgt0~8
    float sw_btnrT_filtSpatial_wgt[9];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrPixDiff_maxLimit),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4095),
        M4_DEFAULT(4095),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The maximum limit of the diff value of neighboring pixels of bilateral filter.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrPixDiff_maxLimit
    uint16_t hw_btnrT_pixDiff_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrWgtCal_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaled factor of the bias-adjusted neighborhood pixel diff in calculating the range weight of the bifilt.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrWgtCal_offset
    float hw_btnrT_pixDiff_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrWgtCal_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.15),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaled factor of the bias-adjusted neighborhood pixel diff in calculating the range weight of the bifilt.\n
        Freq of use: low))  */
    // reg: hw_btnr_preSpnrWgtCal_scale
    float hw_btnrT_pixDiff_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnr_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The alpha value of the output of the bifilter, when it is weighted with the input pixel..\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnr_wgt
    float hw_btnrT_spNrOut_alpha;
} btnr_preSpNr_dyn_t;

typedef enum btnr_frmAlpha_mode_e {
    // reg: hw_btnrT_noisebal_mode = 0
    btnr_hiAlphaByHi_mode = 0,
    // reg: hw_btnrT_noisebal_mode = 1
    btnr_hiAlphaByLo_mode = 1
} btnr_frmAlpha_mode_t;

typedef struct btnr_frmAlphaLo_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtClip_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of the low-frequency fusion weight of the IIR frame.\n
        Freq of use: low))  */
    // reg: hw_btnr_loWgtClip_minLimit
    float hw_btnrT_loAlpha_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtClipHdrSht_minLimit),
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
    float hw_btnrT_loAlphaHdrS_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtClip_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit of the low-frequency fusion weight of the IIR frame.\n
        Freq of use: low))  */
    // reg: hw_btnr_loWgtClip_maxLimit
    float hw_btnrT_loAlpha_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loWgtClipHdrSht_maxLimit),
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
    float hw_btnrT_loAlphaHdrS_maxLimit;
} btnr_frmAlphaLo_dyn_t;

typedef struct btnr_frmAlpha_hiByHi_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_hiWgtClip_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of the high-frequency fusion weight of the IIR frame.\n
        Freq of use: low))  */
    // reg: hw_btnr_hiWgtClip_minLimit
    float hw_btnrT_hiAlpha_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_hiWgtClipHdrSht_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of the high-frequency fusion weight of the IIR frame in short-frame fusion area.\n
        Freq of use: low))  */
    // reg: hw_btnr_hiWgtClipHdrSht_minLimit
    float hw_btnrT_hiAlphaHdrS_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_hiWgtClip_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The max limit of the high-frequency fusion weight of the IIR frame.\n
        Freq of use: low))  */
    // reg: hw_btnr_hiWgtClip_maxLimit
    float hw_btnrT_hiAlpha_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_hiWgtClipHdrSht_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of the high-frequency fusion weight of the IIR frame in short-frame fusion area..\n
        Freq of use: low))  */
    // reg: hw_btnr_hiWgtClipHdrSht_maxLimit
    float hw_btnrT_hiAlphaHdrS_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrHiWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The alpha value of the high-frequency data separated by spatial filtering for cur frame.\n
        Freq of use: high))  */
    // reg: hw_btnr_curSpnrHiWgt_minLimit
    float hw_btnrT_curHiOrg_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_preSpnrHiWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(32),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The alpha value of the high-frequency data separated by spatial filtering for iir frame.\n
        Freq of use: high))  */
    // reg: hw_btnr_preSpnrHiWgt_minLimit
    float hw_btnrT_iirHiOrg_alpha;
}btnr_frmAlpha_hiByHi_dyn_t;

typedef struct btnr_frmAlpha_hiByLo_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_motion_nr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,4095.0),
        M4_DEFAULT(4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The filter strg of the high-frequency data separated by spatial filtering.\n
        Freq of use: high))  */
    // reg: hw_btnr_motion_nr_strg
    float hw_btnrT_hiMotionNr_strg;
} btnr_frmAlpha_hiByLo_dyn_t;

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
       M4_DEFAULT(32),
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
        M4_DEFAULT(0),
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
        M4_NOTES(The sigma statistics is valid only when the number of pixels counted exceeds the threshold.\n
        Freq of use: low))  */
    // reg: hw_btnr_tnrAutoSigmaCountTh
    uint32_t hw_btnrCfg_statsPixCnt_thred;
} btnr_sigma_static_t;

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
        M4_NOTES(The weight of previous sigma value with current sigma value on auto sigma count mode.\n
        Freq of use: high))  */
    float sw_btnrT_autoSgmIIR_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_curSpnrLuma2sigma),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The sigma calibration curve of the spatial filter used to process the current frame data. \n
        The filter only use the sigma data of the curve when sw_btnrCfg_sigma_mode == btnr_manualSigma_mode.\n
        The filter will use the sigma data of the curve and sigma statistics when sw_btnrCfg_sigma_mode == btnr_autoSigma_mode.\n
        Freq of use: low))  */
    // reg: hw_btnr_curSpnrLuma2sigma_idx0 ~ 15, hw_btnr_curSpnrLuma2sigma_val0 ~ 15
    btnr_spNrSigmaCurve_t hw_btnrC_curSpNrSgm_curve;
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
    btnr_spNrSigmaCurve_t hw_btnrC_iirSpNrSgm_curve;
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
        M4_DEFAULT(0.7),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( .Freq of use: high))  */
    // @reg:hw_btnr_gain_max
    float hw_bnrT_locSgmStrg_maxLimit;
} btnr_locSgmStrg_dyn_t;


typedef enum btnr_pixDomain_mode_e {
    /*
    reg: (hw_btnr_transf_bypass_en== 0 && hw_btnr_transf_mode == 0)
    */
    btnr_pixLog2Domain_mode = 0,
    /*
    reg: (hw_btnr_transf_bypass_en== 0 && hw_btnr_transf_mode == 1)
    */
    btnr_pixSqrtDomain_mode,
    /*
    reg: (hw_btnr_transf_bypass_en== 1)
    */
    btnr_pixLinearDomain_mode,
} btnr_pixDomain_mode_t;

/**************************************************
=============== 以上为模块硬件参数定义 =============
**************************************************/

typedef struct btnr_transCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_transfMode_scale),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Scale used for transformation.\n
        Freq of use: low))  */
    // reg: hw_btnr_transfMode_scale
    int hw_btnr_trans_scale;
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
    int hw_btnr_trans_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_transfData_maxLimit),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1048575),
        M4_DEFAULT(1048575),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The maximum value limit for the transformation.\n
        Freq of use: low))  */
    // reg: hw_btnr_transfData_maxLimit
    int hw_btnr_transData_maxLimit;
}btnr_transCfg_t;


typedef enum btnr_dbgOutMux_mode_e {
    // reg: hw_btnr_iirspnr_out_en == 1
    btnr_dbgOut_iirSpNr_mode = 0,
    // reg: hw_btnr_curspnr_out_en == 1
    btnr_dbgOut_curSpNr_mode = 1,
    // reg: hw_btnr_mdWgtOut_en == 1
    btnr_dbgOut_mdWgt_mode = 2
} btnr_dbgOutMux_mode_t;

typedef struct btnr_debug_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_dbgOut_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
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
        M4_NOTES( Reference enum types.\n
        Freq of use: high))  */
    btnr_dbgOutMux_mode_t hw_btnrT_dbgOut_mode;
} btnr_debug_t;

typedef struct btnr_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrCfg_pixDomain_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_pixDomain_mode_t),
        M4_DEFAULT(btnr_pixLog2Domain_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
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
        M4_NOTES(...\n
        Freq of use: low))  */
    btnr_transCfg_t transCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sigmaEnv),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
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
} btnr_params_static_t;

typedef struct btnr_md_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_md_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
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
        M4_NOTES(TODO))  */
    btnr_mdSigma_t mdSigma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd0_diffCh),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_subLoMd0_diffCh_dyn_t subLoMd0_diffCh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd0_sgmCh),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_subLoMd0_sgmCh_dyn_t subLoMd0_sgmCh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd0_wgtOpt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_subLoMd0_wgtOpt_dyn_t subLoMd0_wgtOpt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subLoMd1),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_subLoMd1_dyn_t subLoMd1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loMd),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_loMd_dyn_t loMd;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_md_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_mdWgtOpt_mode_t),
        M4_DEFAULT(btnr_loAsBiasForHi_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Mode of low-frequency motion detection. Reference enum types.\n
        Freq of use: low))  */
    //@reg: hw_btnr_WgtCal_mode
    btnr_md_mode_t hw_btnrT_md_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loAsHiRatioMd),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_loAsHiRatioMd_dyn_t loAsHiRatioMd;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loAsHiBiasMd),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    btnr_loAsHiBiasMd_dyn_t loAsHiBiasMd;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdWgtPost),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    btnr_mdWgtPost_dyn_t mdWgtPost;
} btnr_md_dyn_t;

typedef struct btnr_other_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sigmaEnv),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_sigmaEnv_dyn_t sigmaEnv;
    /* M4_GENERIC_DESC(
        M4_ALIAS(curFrmSpNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_curSpNr_dyn_t curFrmSpNr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(preFrmSpNr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_preSpNr_dyn_t preFrmSpNr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(frmAlpha_lo),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_frmAlphaLo_dyn_t frmAlpha_lo;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_frmAlpha_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_frmAlpha_mode_t),
        M4_DEFAULT(btnr_hiAlphaByHi_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Mode of low-frequency motion detection. Reference enum types.\n
        Freq of use: high))  */
    //@reg: hw_btnr_noisebal_mode
    btnr_frmAlpha_mode_t hw_btnrT_frmAlpha_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(frmAlpha_hiByLo),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_frmAlpha_hiByLo_dyn_t frmAlpha_hiByLo;
    /* M4_GENERIC_DESC(
        M4_ALIAS(frmAlpha_hiByHi),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_frmAlpha_hiByHi_dyn_t frmAlpha_hiByHi;
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
    btnr_md_dyn_t mdDyn;
    /* M4_GENERIC_DESC(
        M4_ALIAS(spNrDyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    btnr_other_dyn_t dyn;
} btnr_param_t;

#endif
