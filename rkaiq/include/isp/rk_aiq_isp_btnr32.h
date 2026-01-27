/*
 * rk_aiq_param_btnr32.h
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

#ifndef _RK_AIQ_PARAM_BTNR32_H_
#define _RK_AIQ_PARAM_BTNR32_H_


#define BTNR_ISO_SEGMENT_MAX                13
#define BTNR_SIGMACURVE_SEGMENT_MAX         16

/********************************************************
The following registers are used for internal mode control and are only implemented in the driver code：
  sw_bay3d_curds_high_en
  sw_bay3d_bwopt_gain_dis
  sw_bay3d_iirwr_rnd_en

 *******************************************************/

typedef enum btnr_hiFiltAbs_mode_e {
    filt_abs_mode   =0,
    abs_filt_mode   =1
} btnr_hiFiltAbs_mode_t;

typedef enum btnr_bw_mode_e {
    // @reg: sw_bay3d_bwsaving_en == 1
    bw_compress_mode   = 0,
    // @reg: sw_bay3d_bwsaving_en == 0
    bw_original_mode   = 1
} btnr_bw_mode_t;

typedef enum btnr_hiMdFilt_mode_e {
    // @reg: (sw_bay3d_hichncor_en != 1) && (sw_bayer_higaus3_mode == 0) && (sw_hichnsplit_en == 1)
    bayerFilt_gaus_mode     = 0,
    // @reg: sw_bay3d_hichncor_en == 1
    bayerFilt_gausMean_mode     = 1,
    // @reg: (sw_bay3d_hichncor_en != 1) && (sw_bayer_higaus3_mode != 0) && (sw_bayer_higaus3_mode == 2)
    lumaFilt_filt3StrgLo_mode    =2,
    // @reg: (sw_bay3d_hichncor_en != 1) && (sw_bayer_higaus3_mode != 0) && (sw_bayer_higaus3_mode == 1)
    lumaFilt_filt3StrgHi_mode    =3,
    // @reg: (sw_bay3d_hichncor_en != 1) && (sw_bayer_higaus3_mode == 0) && (sw_hichlsplit_en == 0) && (sw_higaus5x5_en == 0)
    lumaFilt_filt5StrgLo_mode    =4,
    // @reg: (sw_bay3d_hichncor_en != 1) && (sw_bayer_higaus3_mode == 0) && (sw_hichlsplit_en == 0)  && (sw_higaus5x5_en == 1)
    lumaFilt_filt7StrgLo_mode    =5
} btnr_hiMdFilt_mode_t;

typedef enum btnr_md_mode_e {
    // @reg: (sw_bayer3d_lobypass_en == 0) && (sw_bayer3d_hibypass_en == 0)
    loHiMdMix_mode  = 0,
    /* @reg: 1. (sw_bayer3d_lobypass_en == 1) && (sw_bayer3d_hibypass_en == 0)
            2. (sw_bayer3d_lobypass_en == 1) && (sw_bayer3d_hibypass_en == 1)
    */
    hiMdOnly_mode   = 1,
    // @reg: (sw_bayer3d_lobypass_en == 0) && (sw_bayer3d_hibypass_en == 1)
    loMdOnly_mode  = 2
} btnr_md_mode_t;

typedef enum btnr_wgtMdOpt_mode_e {
    // @reg: sw_hiwgt_opt_en == 0
    loHi_loHiDiff_mode   = 0,
    // @reg: (sw_hiwgt_opt_en == 1) && (sw_wgtmix_opt_en == 1)
    loHi0_hi1_mode  = 1,
    // @reg: (sw_hiwgt_opt_en == 1) && (sw_wgtmix_opt_en == 0)
    loHi0_negHi1_mode  = 2
} btnr_wgtMdOpt_mode_t;

typedef enum btnr_loDs_mode_e {
    // @reg: sw_bayer3d_lo4x4_en == 1
    loDs_4x4_mode   = 0,
    // @reg: (sw_bayer3d_lo4x4_en == 0) && (sw_bayer3d_lo4x8_en == 1)
    loDs_8x4_mode  = 1,
    // @reg: (sw_bayer3d_lo4x4_en == 0) && (sw_bayer3d_lo4x8_en == 0)
    loDs_8x8_mode  = 2
} btnr_loDs_mode_t;

typedef enum btnr_filtPixGain_mode_e {
    // @reg: sw_bay3d_glbpk_en == 0
    btnr_pkSigma_local_mode = 0,
    // @reg: sw_bay3d_glbpk_en == 1
    btnr_pkSigma_glb_mode = 1
} btnr_pkSigma_mode_t;

typedef struct {
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
    uint16_t idx[BTNR_SIGMACURVE_SEGMENT_MAX];
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
    uint16_t val[BTNR_SIGMACURVE_SEGMENT_MAX];
} btnr_sigmaCurve_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_pkSigma_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_pkSigma_mode_t),
        M4_DEFAULT(btnr_pkSigma_local_mode ),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of pksigma used for btnr.Reference enum types.\nFreq of use: low))  */
    btnr_pkSigma_mode_t hw_btnrT_pkSigma_mode;
} btnr_pkSigma_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_bw_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_bw_mode_t),
        M4_DEFAULT(bw_compress_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The mode of btnr bandwidth.Reference enum types.\nFreq of use: Low))  */
    // @reg: sw_bay3d_bwsaving_en
    btnr_bw_mode_t hw_btnrCfg_bw_mode;
} btnr_bwCfg_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_md_mode_t),
        M4_DEFAULT(loHiMdMix_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_md_mode_t hw_btnrT_md_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_loDs_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_loDs_mode_t),
        M4_DEFAULT(loDs_8x4_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The mode of downscale for lo freq motion detection. It invalid onhiMdOnly_mode. Reference enum types.\nFreq of use: low))  */
    // @reg: sw_bayer3d_lo4x8_en, sw_bayer3d_lo4x4_en
    btnr_loDs_mode_t hw_btnrT_loDs_mode;
} btnr_mdCfg_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_loMedfilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable median filter for lo freq motion detection.Turn on by setting this bit to 1.\nFreq of use: low))  */
    // @reg: sw_bay3d_lomed_bypass_en
    bool hw_btnrT_loMedfilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_loBayerFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Enable bayer filter for lo freq motion detection.Turn on by setting this bit to 1.\nFreq of use: low))  */
    // @reg: sw_bay3d_logaus5_bypass_en
    bool hw_btnrT_loBayerFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_loLumaFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Enable luma filter for lo freq motion detection.Turn on by setting this bit to 1.\nFreq of use: low))  */
    // @reg: sw_bay3d_logaus3_bypass_en
    bool hw_btnrT_loLumaFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15.9961),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(8),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The scaling factor of lo sigma that it affect the lo freq motion weight.\nThe val higher, the lo wgt is higher.\nFreq of use: high))  */
    // @reg: sw_bay3d_losigrat
    float hw_btnrT_loSigma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_loSigma_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3.9991),
        M4_DEFAULT(0),
        M4_DIGIT_EX(10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The offset of lo sigma that it affect the lo freq motion weight.The val higher, the lo wgt is higher.\nFreq of use: high))  */
    // @reg: sw_bay3d_losigoff
    float hw_btnrT_loSigma_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrC_luma2LoSigma),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(TODO))  */
    // @reg: sw_baynr_sig2_x0 ~ sw_baynr_sig2_x15, sw_baynr_sig2_y0 ~ sw_baynr_sig2_y15
    btnr_sigmaCurve_t hw_btnrC_luma2LoSigma_Curve;
} btnr_loMd_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiMedfilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable median pre-filter for hi motion detection.Turn on by setting this bit to 1.\nFreq of use: low))  */
    // @reg: sw_bay3d_himed_bypass_en
    bool hw_btnrT_hiMedfilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiFiltAbs_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_hiFiltAbs_mode_t),
        M4_DEFAULT(filt_abs_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Select the sequence of filtering and abs  operation by this mode.Reference enum types.\nFreq of use: low))  */
    // @reg: sw_bay3d_hiabs_possel
    btnr_hiFiltAbs_mode_t hw_btnrT_hiFiltAbs_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiFIlt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Enable the pre-filter for hi motion detection input data.Turn on by setting this bit to 1.\nFreq of use: high))  */
    // @reg: sw_bay3d_higaus_bypass_en
    bool hw_btnrT_hiFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiFilt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_hiMdFilt_mode_t),
        M4_DEFAULT(lumaFilt_filt5StrgLo_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The mode of pre-filter for hi motion detection input data.Reference enum types.\nFreq of use: high))  */
    // @reg: sw_bay3d_hichncor_en, sw_bayer_higaus3_mode, sw_bayer_higaus3_mode, sw_hichlsplit_en
    btnr_hiMdFilt_mode_t hw_btnrT_hiFilt_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiSigmaIdxPreFilt_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Enable pre filter for pixels as hisigma curve idx.Turn on by setting this bit to 1.\nFreq of use: low))  */
    // @reg: sw_bay3d_hisig_ind_sel
    bool hw_btnrT_hiSigmaIdxPreFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_SigmaIdxPreFilt_coeff),
        M4_TYPE(u8),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0,63),
        M4_DEFAULT([16,8,16,8]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The operator coeff of pre filter for pixels as pksigma curve idx.\ncoeff[0] + 2*coeff[1] + coeff[2] + 2*coeff[3]== 64.\nFreq of use: low))  */
    // @reg: sw_bay3d_siggaus0~3
    uint8_t hw_btnrT_SigmaIdxPreFilt_coeff[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiSigma0_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15.9961),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The scaling factor of hi0 sigma that it affect the hi freq motion weight.The val higher, the hi0 wgt is higher.\nFreq of use: high))  */
    // @reg: sw_bay3d_hisigrat0
    float hw_btnrT_hi0Sigma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiSigma0_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3.9991),
        M4_DEFAULT(0),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(The offset of hi1 sigma that it affect the hi freq motion weight.Higher the value, lower the weight.\nFreq of use: high))  */
    // @reg: sw_bay3d_hisigoff0
    float hw_btnrT_hi0Sigma_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiSigma1_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15.9961),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(The scaling factor of hisigma1 that it affect the hi freq motion weight.Higher the value, lower the weight.\nFreq of use: high))  */
    // @reg: sw_bay3d_hisigrat1
    float hw_btnrT_hi1Sigma_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiSigma0_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3.9991),
        M4_DEFAULT(0),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(The offset of hisigma1 that it affect the hi freq motion weight.Higher the value, lower the weight.\nFreq of use: high))  */
    // @reg: sw_bay3d_hisigoff1
    float hw_btnrT_hi1Sigma_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrC_luma2HiSigma),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(TODO))  */
    // @reg: sw_bay3d_sig1_x0 ~ sw_bay3d_sig1_x15, sw_bay3d_sig1_y0 ~ sw_bay3d_sig1_y15
    btnr_sigmaCurve_t hw_btnrC_luma2HiSigma_Curve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_hiDiffStatics_maxLimit),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(4095),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(The min limit for the hi-freq diff between the cur and iir frm. The hi-feq diff is only used as statistics and not affect btnr.  It is in the linear domain.\nHigher the value, lower the weight.\nFreq of use: low))  */
    // @reg: sw_bay3d_hidiff_th
    /* @note:
           bnr: hw_bnrCfg_logTrans_en== 1:  It must be trans to log by AIQ before set to sw_bay3d_hidiff_th.
           bnr: hw_bnrCfg_logTrans_en== 0:  It is set to sw_bay3d_hidiff_th directly.
    */
    uint16_t hw_btnrT_hiDiffStatics_minLimit;
} btnr_hiMd_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(filtPixGain),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_pkSigma_static_t pkSigmaMode;
} btnr_wgtPk_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_glbPkSigma_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3.992),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale of global pksigma. Only valid on btnr_pkSigma_glb_mode.\nFreq of use: low))  */
    float sw_btnrT_glbPkSigma_scale;
    /* M4_GENERIC_DESC(
            M4_ALIAS(hw_btnrT_pkSigmaIdxPreFilt_en),
            M4_TYPE(bool),
            M4_DEFAULT(1),
            M4_HIDE_EX(0),
            M4_RO(0),
            M4_ORDER(5),
            M4_NOTES(Enable pre filter for pixels as pksigma curve idx. Only valid on btnr_pkSigma_local_mode. Turn on by setting to 1.\nFreq of use: low))  */
    // @reg: sw_bay3d_pksig_ind_sel
    bool hw_btnrT_pkSigmaIdxPreFilt_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_wgtPk_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0.999),
        M4_DEFAULT(0.03215),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The min limit of wgt_pk value. Higher the value, the fewer frms be fused.\nFreq of use: high))  */
    // @reg: sw_bay3d_wgtlmt
    float hw_btnrT_wgtPk_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrC_luma2PkSigma),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(TODO))  */
    // @reg: sw_bay3d_sig0_x0 ~ sw_bay3d_sig0_x15, sw_bay3d_sig0_y0 ~ sw_bay3d_sig0_y15
    btnr_sigmaCurve_t hw_btnrC_luma2PkSigma_Curve;
} btnr_wgtPk_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_btnrT_wgtMdOpt_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(btnr_wgtMdOpt_mode_t),
        M4_DEFAULT(loHi0_hi1_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The operation mode of IIR frm alpha value depends on lo/hi0/hi1 wgt.\nOnly valid on loHiMdMix_mode.\nFreq of use: high))  */
    btnr_wgtMdOpt_mode_t sw_btnrT_wgtMdOpt_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnrT_HiLoWgtDiff_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0.999),
        M4_DEFAULT(0.999),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The scaling factor of diff between hi and lo wgt that it affects IIR frm alpha value.\nOnly valid on loHi0_loHi0Diff_Mode.\nFreq of use: high))  */
    // @reg: sw_bay3d_wgtratio
    float hw_btnrT_loHi0Diff_scale;
} btnr_wgtMd_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(bool hw_btnrT_frmAlpha_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable fusion between cur and IIR frm.Turn on by setting this bit to 1.\nFreq of use: low))  */
    // @reg: sw_bay3d_bypass_en
    bool hw_btnrT_frmAlpha_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_bnrT_softThHold_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0.999),
        M4_DEFAULT(0.03215),
        M4_DIGIT_EX(5),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The scaling factor of sigma that is used in soft threshold operation.Higher the value, the higher noise be retained.\nFreq of use: high))  */
    // @reg: sw_bay3d_softwgt
    float hw_bnrT_softThred_scale;
} btnr_frmAlpha_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(btnr_bwCfg_static_t),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    btnr_bwCfg_static_t bwCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(btnr_mdCfg_static_t),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_mdCfg_static_t mdCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(btnr_wgtPk_static_t),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    btnr_wgtPk_static_t pkCfg;
} btnr_params_static_t;

typedef struct {
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
        M4_ALIAS(hiMd),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    btnr_hiMd_dyn_t hiMd;
    /* M4_GENERIC_DESC(
        M4_ALIAS(frmAlphaWgtMd),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    btnr_wgtMd_dyn_t frmAlphaWgtMd;
    /* M4_GENERIC_DESC(
        M4_ALIAS(frmAlphaWgtPk),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    btnr_wgtPk_dyn_t frmAlphaWgtPk;
    /* M4_GENERIC_DESC(
        M4_ALIAS(iirFrmAlpha),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(TODO))  */
    btnr_frmAlpha_dyn_t frmAlpha;
} btnr_params_dyn_t;

typedef struct {
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
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    btnr_params_dyn_t dyn;
} btnr_param_t;

#endif
