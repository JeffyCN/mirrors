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

#ifndef _RK_AIQ_PARAM_YNR34_H_
#define _RK_AIQ_PARAM_YNR34_H_

#define YNR_ISO_STEP_MAX                13
#define YNR_SIGMACURVE_SEGMENT_MAX         17
#define YNR_NLM_COEF_MAX                   6
#define YNR_RNR_STRENGTH_SEGMENT_MAX       17
#define YNR_PIXSGMSCL_SEGMENT_MAX       9
#define YNR_SIGMACURVE_COEFF_MAX       5
#define YNR_LUMA2SIGMASTRENGTH_SEGMENT_MAX       6

typedef enum ynr_filtCfg_mode_e {
    // @note: By configuring the strength coefficient, the software internally calculates and generates filter coefficients based on the strength coefficient to configure the filter.
    ynr_cfgByFiltStrg_mode = 0,
    // @note: Configure the filter by directly configuring the filter coefficients.
    ynr_cfgByFiltCoeff_mode = 1
} ynr_filtCfg_mode_t;

typedef struct ynr_loNr_preProc_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_preLpfCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ynr_filtCfg_mode_t),
        M4_DEFAULT(ynr_cfgByFiltStrg_mode),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The way to configure the LPF. Reference enum types.\n
		Freq of use: low))  */
    ynr_filtCfg_mode_t sw_ynrT_preLpfCfg_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_preFilts_coeff),
        M4_TYPE(u8),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The spatial weight coeff of the LPF is directly configured through this para when lpfCfg_mode is ynr_cfgByFiltCoeff_mode.\n
		Freq of use: low))  */
    // reg: hw_ynr_preFilts_coeff0~2
    uint8_t hw_ynrT_preLpfSpatial_wgt[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_preFilts_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The spatial weight coeff of the pre LPF is configured through this para when lpfCfg_mode is ynr_cfgByFiltStrg_mode.\n
		Freq of use: high))  */
    // reg: hw_btnr_lpfLo_coeff0~2
    float sw_ynrT_preLpf_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_btnr_dsImg_edgeScale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor for the confidence level of the texture detection module.\n
		Freq of use: high))  */
    // reg: hw_btnr_dsImg_edgeScale
    float sw_ynrT_edgeDctConf_scale;
} ynr_loNr_preProc_t;

typedef struct ynr_loNr_iirGuide_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_dsFiltLocalGain_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The alpha value of local sigmaScl in alpha operation between local and global sigmaScl.\n
        The local sigmaScl is composed of inPixSgmScl and radiSgmScl.\n
        The larger , the greater its impact on the local strength of the filter.\n
		Freq of use: high))  */
    // reg: hw_ynr_dsFiltLocalGain_alpha
    float hw_ynrT_localYnrScl_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_dsFilt_invStrg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f9b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( This parameter directly controls the global strength of the loNr\n
        The smaller, the global strength of the loNr is stronger \n
		Freq of use: high))  */
    // reg: hw_ynr_dsFilt_invStrg
    float hw_ynrT_iiFilt_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_dsFiltWgtThred_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,0.985),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3f9b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Neighboring pixels with pixel difference exceeding the threshold are considered to be edge regions.\n
        The filter applies higher frequency filtering to edge areas.\n
        Freq of use: low))  */
    // reg: sw_ynr_dsFiltWgtThred_scale
    float hw_ynrT_pixDiffEge_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_dsFilt_centerWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,0.999),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3f11b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The spatial weight of center pixel in filter.The higher the value, the smaller the strength of reducing noise.\n
        Freq of use: low))  */
    // reg: sw_ynr_dsFilt_centerWgt
    float hw_ynrT_centerPix_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_dsIIRinitWgt_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,0.985),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The scaling factor of initial weight of IIR filter of loNr.\n
        The smaller, the stronger the strength of loNr.\n
        Freq of use: low))  */
    // reg: hw_ynr_dsIIRinitWgt_scale
    float hw_ynrT_iirInitWgt_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_dsFiltSoftThred_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 0.998),
        M4_DEFAULT(0.1),
        M4_DIGIT_EX(3f9b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The scaling factor of soft threshold\n
        The larger, more original noise and texture are preserved.\n
        Freq of use: high))  */
    // reg: hw_ynr_dsFiltSoftThred_scale
    float hw_ynrT_softThd_scale;
} ynr_loNr_iirGuide_t;

typedef struct ynr_loNr_bifilt_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_loSpnr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 31.993),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f7b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of the range sigma of the bilateral filter\n
        Freq of use: high))  */
    // reg: hw_ynr_loSpnr_strg
    float hw_ynrT_rgeSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_loSpnrDistVstrg_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 7.985),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of the distance of the neighboring pixels in the vertical direction on the effect of the spatial sigma\n
        Freq of use: low))  */
    // reg: hw_ynr_loSpnrDistVstrg_scale
    float hw_ynrT_filtSpatialV_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_loSpnrDistHstrg_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 7.985),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scaling factor of the distance of the neighboring pixels in the horizontal direction on the effect of the spatial sigma\n
        Freq of use: low))  */
    // reg: hw_ynr_loSpnrDistHstrg_scale
    float hw_ynrT_filtSpatialH_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_loSpnr_centerWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 4.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The spatial weight of center pixel in filter.The higher the value, the smaller the strength of reducing noise.\n
        Freq of use: high))  */
    // reg: sw_ynr_loSpnr_centerWgt
    float hw_ynrT_centerPix_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_loSpnr_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The alpha value of the filted pixel in alpha operation between filted and original pixel.\n
        Freq of use: high))  */
    // reg: sw_ynr_loSpnr_wgt
    float hw_ynrT_bifiltOut_alpha;
} ynr_loNr_bifilt_t;

typedef struct ynr_ynrScl_locSgmStrg_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_globalSet_gain),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 32.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(1),
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
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_loSpnrGain2strg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,9),
        M4_RANGE_EX(0,32.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_FP_EX(0,6,9),
        M4_HIDE_EX(0),
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
    float hw_ynrT_locSgmStrg2YnrScl_val[YNR_PIXSGMSCL_SEGMENT_MAX];
} ynr_ynrScl_locSgmStrg_dyn_t;

typedef struct ynr_ynrScl_radi_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_rnr_center_cooh),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1000,1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES( The x-coordinates of the optical center in the image\n
        (0,0) is the img center. (-1000,-1000) is the img left top corner. (1000,1000) is the img right bottom corner\n
        Freq of use: low))  */
    // @reg: sw_ynr_rnr_center_cooh
    // @para: Center_Mode
    int16_t hw_ynrCfg_opticCenter_x;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_rnr_center_coov),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1000,1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(1),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES( The y-coordinates of the optical center in the image\n
        (0,0) is the img center. (-1000,-1000) is the img left top corner. (1000,1000) is the img right bottom corner\n
        Freq of use: low))  */
    // @reg: sw_ynr_rnr_center_coov
    // @para: Center_Mode
    int16_t hw_ynrCfg_opticCenter_y;
} ynr_ynrScl_radi_static_t;

typedef struct ynr_ynrScl_radi_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_radius2strg_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0.0, 15.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The pixels at different radial distances have different scaling strengths for pixSigma.\n
        The sgmScl is larger, the loNr and hiNr strength is stronger.
        Freq of use: high))  */
    //reg: hw_ynr_radius2strg_val0 ~16
    float hw_ynrT_radiDist2YnrScl_val[YNR_RNR_STRENGTH_SEGMENT_MAX];

} ynr_ynrScl_radi_dyn_t;

typedef enum ynr_sigmaCurveCfg_mode_e {
    // @note: By configuring the sigma curve, the software internally calculates and generates sigma curve points based on the sigma coefficients and freq coeff to configure the sigma curve.
    ynr_cfgByCoeff2Curve_mode = 0,
    // @note: Configure the sigma curve points by directly configuring point value.
    ynr_cfgByCurveDirectly_mode = 1
} ynr_sigmaCurveCfg_mode_t;

typedef struct ynr_coeff2SgmCurve_s {
	/* M4_GENERIC_DESC(
        M4_ALIAS(sigma_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,5),
        M4_RANGE_EX(-4095.0, 4095.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(6),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    float sigma_coeff[YNR_SIGMACURVE_COEFF_MAX];
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_lowFreqCoeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Software internally caulculate low frequency sigma curve points by sigma coeff and low frequency coeff.\n
		Freq of use: low))  */
    float lowFreqCoeff;
} ynr_coeff2SgmCurve_t;

typedef struct ynr_sigmaCurve_dyn_s  {
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
} ynr_sigmaCurve_dyn_t;

typedef struct ynr_hiNr_filtProc_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynr_hiSpnr_bypass),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Enable the NLM filter for hi freq noise.Turn on by setting this bit to 1.
        Freq of use: high))  */
    // reg: hw_ynrT_nlmFilt_en = !sw_ynr_hiSpnr_bypass
    bool hw_ynrT_nlmFilt_en;
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
    float hw_ynrT_localYnrScl_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrSigma_minLimt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.0068),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Mininum noise sigma for high freq noise.The higher the value, the bigger the strength of flat area denoise.\n
        Freq of use: low))  */
    // reg: hw_ynr_hiSpnrSigma_minLimt
    float hw_ynrT_nlmSgm_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnr_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The scale of nlm sigma.The higher the value, the strength of filter is higher.\n
        Freq of use: high))  */
    // reg: hw_ynr_hiSpnr_strg
    float hw_ynrT_nlmSgm_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrFilt_wgtOffset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The negative offset of nlm filt range weight.The higher the value, the smaller the strength of high freq reducing noise.\n
        Freq of use: low))  */
    // reg: hw_ynr_hiSpnrFilt_wgtOffset
    float hw_ynrT_nlmRgeWgt_negOff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrFilt_centerWgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 128.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3f10b),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The spatial weight of center pixel in filter.The higher the value, the smaller the strength of reducing noise.\n
        Freq of use: low))  */
    // reg: hw_ynr_hiSpnrFilt_centerWgt
    float hw_ynrT_centerPix_wgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_nlm_coe),
        M4_TYPE(u8),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,6),
        M4_RANGE_EX(0,15),
        M4_DEFAULT(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The spatial weight of adjacent pixels in the nlm filter\n
        Freq of use: low))  */
    // reg: hw_ynr_nlm_coe_0~5
    uint8_t hw_ynrT_nlmSpatial_wgt[YNR_NLM_COEF_MAX];
} ynr_hiNr_filtProc_dyn_t;

typedef struct ynr_hiNr_alphaProc_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrFilt_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The wgt of nlm filted pixel is used in the fusion operation between the  nlm filted pixel and the original pixel.\n
        Higher the value, the stronger denoise strength of  filter.\n
        Freq of use: high))  */
    //reg: hw_ynr_hiSpnrFilt_wgt
    float hw_ynrT_nlmOut_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnrStrongEdge_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The threshold for determining the edge region, where the filted pixels in this region have an increased weight in the alpha operation with the non-filted pixels.\n
        The larger the threshold, the stronger the texture can trigger the alpha increase mechanism.\n
        Freq of use: high))  */
    //reg: hw_ynr_hiSpnrStrongEdge_scale
    float hw_ynrT_edgAlphaUp_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynr_hiSpnr_gainThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f10),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The inPixSgm threshold for determining the region, where the filted pixels in this region have an increased weight in the alpha operation with the non-filted pixels.\n
        The larger the threshold, the stronger the inPixSgm can trigger the alpha increase mechanism.\n
        Freq of use: high))  */
    //reg: hw_ynr_hiSpnr_gainThred
    float hw_ynrT_locSgmStrgAlphaUp_thred;
} ynr_hiNr_alphaProc_dyn_t;

typedef struct ynr_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ynrScl_radi),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_ynrScl_radi_static_t ynrScl_radi;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_sigmaCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ynr_sigmaCurveCfg_mode_t),
        M4_DEFAULT(ynr_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The way to configure the sigma curve point. Reference enum types.\n
		Freq of use: low))  */
    ynr_sigmaCurveCfg_mode_t sw_ynrCfg_sgmCurve_mode;
} ynr_params_static_t;

typedef struct ynr_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ynrScl_radi),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_ynrScl_radi_dyn_t ynrScl_radi;
    /* M4_GENERIC_DESC(
        M4_ALIAS(localSgmScl_inPixSgm),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ynr_ynrScl_locSgmStrg_dyn_t ynrScl_locSgmStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_sigamaCurve_Coeff),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The sigma calibration curve of the ynr module. \n
        The ynr only use the sigma coeffs of the curve when sw_ynrCfg_sgmCurve_mode == ynr_cfgByCoeff2Curve_mode.\n
        ))  */
	ynr_coeff2SgmCurve_t coeff2SgmCurve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrC_luma2Sigma_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(...\n
        The ynr will use the sigma point data of the curve when sw_ynrCfg_sigmaCfg_mode == ynr_cfgByCurveDirectly_mode.))  */
   // reg: hw_ynr_luma2sigma_idx0 ~ 16, hw_ynr_luma2sigma_val0 ~ 16
    ynr_sigmaCurve_dyn_t hw_ynrC_luma2Sigma_curve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrT_loNr_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Enable the loNr filter for lo freq noise.Turn on by setting this bit to 1.
        Freq of use: high))  */
    // reg: hw_ynrT_loNr_en = !sw_ynr_loSpnr_bypass
    bool hw_ynrT_loNr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNr_preProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ynr_loNr_preProc_t loNr_preProc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNr_lpfForGuideImg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_loNr_iirGuide_t loNr_iirGuide;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNr_bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_loNr_bifilt_t loNr_bifilt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiNr_filtProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    ynr_hiNr_filtProc_dyn_t hiNr_filtProc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiNr_alphaProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(TODO))  */
    ynr_hiNr_alphaProc_dyn_t hiNr_alphaProc;
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
