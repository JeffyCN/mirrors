/*
 * Copyright (c) 2023 Rockchip Eletronics Co., Ltd.
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
 */

#ifndef _RK_AIQ_PARAM_MGE23_H_
#define _RK_AIQ_PARAM_MGE23_H_

#define MGE_OECURVE_LEN 17
#define MGE_MDCURVE_LEN 17

typedef enum mge_expRat_mode_e {
    mge_expRatSyncAE_mode = 0,
    mge_expRatFix_mode    = 1
} mge_expRat_mode_t;

typedef struct mge_expRat_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeCfg_hdrExpRat_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(mge_expRat_mode_t),
        M4_DEFAULT(mge_expRatSyncAE_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This parameter is used to select the configuration method of exposure ratio
       information among multiple frames.\n Reference enum types.\n Freq of use: low))  */
    mge_expRat_mode_t sw_mgeCfg_expRat_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeCfg_expRatFix_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1.0, 256),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(When expRat_mode=expRatFix_mode, the user configures a fixed exposure ratio through
       this parameter.\n expRatFix_val = Long frame exposure / Short frame exposure. Freq of use:
       low))  */
    // reg: hw_hdrmge_short_invGain, hw_hdrmge_short_gain,hw_hdrmge_long_gain,
    // hw_hdrmge_channelDetn_shortGain
    float sw_mgeCfg_expRatFix_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_hdrmge_msDiff_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 255),
        M4_DEFAULT(255),
        M4_DIGIT_EX(0f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(There is a deviation between the luma of the short frame after multiplying by
       expRat and the luma of the long frame. \n The lumaHdrL_scale and lumaHdrL_negOff parameters
       can be used to make corrections and adjustments to the luma of the long frame. \n Freq of
       use: low))  */
    // reg: hw_hdrmge_msDiff_scale
    float hw_mgeCfg_lumaHdrL_scale;
} mge_expRat_t;

typedef enum mge_curveCfg_mode_e {
    // @note: By configuring the curve, the software internally calculates and generates  curve
    // points based on the coefficients.
    mge_cfgByCoeff2Curve_mode = 0,
    // @note: Configure the curve points by directly configuring point value.
    mge_cfgByCurveDirectly_mode = 1
} mge_curveCfg_mode_t;

typedef struct mge_oeWgt_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_oeLut_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(mge_curveCfg_mode_t),
        M4_DEFAULT(mge_cfgByCoeff2Curve_mode),
        M4_GROUP_CTRL(oeLut_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This parameter is used to select the configuration method of overexposure LUT\n
        It is a mapping table of luma to hdr short frame fusion weights.\n
        Reference enum types.\n
        Freq of use: low))  */
    mge_curveCfg_mode_t sw_mgeT_oeLut_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(OE_Smooth),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(2),
        M4_GROUP(oeLut_mode_group:mge_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the overexposure LUT through oeLutCreate_xxx when oeLut_mode ==
       mge_cfgByCoeff2Curve_mode.\n Freq of use: high))  */
    // reg: hw_hdrmge_luma2wgt_val0~16
    // para: Smooth
    float sw_mgeT_lutCreate_slope;
    /* M4_GENERIC_DESC(
        M4_ALIAS(OE_Offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(108,300),
        M4_DEFAULT(260),
        M4_DIGIT_EX(0),
        M4_GROUP(oeLut_mode_group:mge_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the overexposure LUT through oeLutCreate_xxx when oeLut_mode ==
       mge_cfgByCoeff2Curve_mode.\n Freq of use: high))  */
    // reg: hw_hdrmge_luma2wgt_val0~16
    // para: Offset
    float sw_mgeT_lutCreate_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS( hw_hdrmge_luma2wgt_val),
        M4_TYPE(f32),
        M4_UI_MODULE(drc_curve),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
        M4_DATAX([512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024]),
        M4_DIGIT_EX(2),
        M4_GROUP(oeLut_mode_group:mge_cfgByCurveDirectly_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the overexposure LUT through oeLutCreate_xxx when oeLut_mode ==
       mge_cfgByCurveDirectly_mode.\n Freq of use: low))  */
    // reg: hw_hdrmge_luma2wgt_val0~16
    float sw_mgeT_luma2Wgt_val[MGE_OECURVE_LEN];
} mge_oeWgt_t;

typedef struct mge_mdWgt_baseHdrL_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeCfg_hdrExpRat_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(mge_curveCfg_mode_t),
        M4_DEFAULT(mge_cfgByCoeff2Curve_mode),
        M4_GROUP_CTRL(mdLut_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(This parameter is used to select the configuration method of motion detect LUT.\n
        It is a mapping table of motion luma diff between 2 frames  to hdr short frame fusion
       weights.\n Reference enum types.\n Freq of use: low))  */
    mge_curveCfg_mode_t sw_mgeT_mdLut_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(MD_Smooth),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(2),
        M4_GROUP(mdLut_mode_group:mge_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the luma motion detect LUT through lumaLutCreate_xxx when
       mdLut_mode == mge_cfgByCoeff2Curve_mode.\n Freq of use: high))  */
    // reg: hw_hdrmge_msLumaDiff2wgt_vall0~16
    // para: Smooth
    float sw_mgeT_lumaLutCreate_slope;
    /* M4_GENERIC_DESC(
        M4_ALIAS(MD_Offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.38),
        M4_DIGIT_EX(2),
        M4_GROUP(mdLut_mode_group:mge_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the luma motion detect LUT through lumaLutCreate_xxx when
       mdLut_mode == mge_cfgByCoeff2Curve_mode.\n Freq of use: high))  */
    // reg: hw_hdrmge_msLumaDiff2wgt_val0~16
    // para: Offset
    float hw_mgeT_lumaLutCreate_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rawCh_Smooth),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(2),
        M4_GROUP(baseHdrL_mode_group:mge_oeMdByLuma_rawCh_mode;mdLut_mode_group:mge_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the raw single channel overexposure detect LUT through
       rawChLutCreate_xxx when mdLut_mode == mge_cfgByCoeff2Curve_mode.\n Freq of use: low))  */
    // reg: hw_hdrmge_msRawDiff2wgt_val0~16
    // para: Smooth
    float sw_mgeT_rawChLutCreate_slope;
    /* M4_GENERIC_DESC(
        M4_ALIAS(rawCh_Offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.38),
        M4_DIGIT_EX(2),
        M4_GROUP(baseHdrL_mode_group:mge_oeMdByLuma_rawCh_mode;mdLut_mode_group:mge_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the raw single channel overexposure detect LUT through
       rawChLutCreate_xxx when mdLut_mode == mge_cfgByCoeff2Curve_mode.\n Freq of use: low))  */
    // reg: hw_hdrmge_msRawDiff2wgt_val0~16
    // para: Offset
    float sw_mgeT_rawChLutCreate_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS( hw_hdrmge_msLumaDiff2wgt_val),
        M4_TYPE(f32),
        M4_UI_MODULE(drc_curve),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
        M4_DATAX([0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256]),
        M4_DIGIT_EX(2),
        M4_GROUP(mdLut_mode_group:mge_cfgByCurveDirectly_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the luma motion detect LUT through lumaDiff2Wgt_val when mdLut_mode
       == mge_cfgByCurveDirectly_mode.\n Freq of use: low))  */
    // reg: hw_hdrmge_msLumaDiff2wgt_val0~16
    // para: Offset
    float sw_mgeT_lumaDiff2Wgt_val[MGE_MDCURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS( hw_hdrmge_msRawDiff2wgt_val),
        M4_TYPE(f32),
        M4_UI_MODULE(drc_curve),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
        M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
        M4_DIGIT_EX(2),
        M4_GROUP(baseHdrL_mode_group:mge_oeMdByLuma_rawCh_mode;mdLut_mode_group:mge_cfgByCurveDirectly_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Users configure the raw single channel overexposure detect LUT through
       rawChDiff2Wgt_val when mdLut_mode == mge_cfgByCurveDirectly_mode.\n Freq of use: low))  */
    // reg: hw_hdrmge_msRawDiff2wgt_val0~16
    // para: Offset
    float sw_mgeT_rawChDiff2Wgt_val[MGE_MDCURVE_LEN];
} mge_mdWgt_baseHdrL_t;

typedef struct mge_mdWgt_baseHdrS_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_diffThred_coef),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(mge threshold-high of ms for clipping diff.\n
        Freq of use: low))  */
    // reg: hw_hdrmge_msAbsDiffThred_maxLimit
    // para: coef
    float sw_mgeT_wgtMaxTh_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_hdrmge_msAbsDiffThred_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(mge threshold-low of ms for clipping diff.\n
        Freq of use: low))  */
    // reg: hw_hdrmge_msAbsDiffThred_minLimit
    float hw_mgeT_wgtZero_thred;
    /* M4_GENERIC_DESC(
    M4_ALIAS(sw_mgeT_lumaDiff_scale),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,64),
    M4_DEFAULT(1),
    M4_DIGIT_EX(2),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The scale value of calculating move detection. The higher the value, the easier it
   is to distinguish the moving zone. \n Freq of use: low))  */
    float sw_mgeT_lumaDiff_scale;
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_lumaThred_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.15),
        M4_DIGIT_EX(2),
        M4_GROUP(baseHdrS_mode_group:mge_motionDR_LPrior_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( Freq of use: low))  */
    float sw_mgeT_lumaThred_minLimit;
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_lumaThred_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(2),
        M4_GROUP(baseHdrS_mode_group:mge_motionDR_LPrior_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES( Freq of use: low))  */
    float sw_mgeT_lumaThred_maxLimit;
} mge_mdWgt_baseHdrS_t;

typedef enum mge_baseFrm_mode_e {
    /*
    hw_hdrmge_shortBase_en = 0
    */
    mge_baseHdrL_mode = 0,
    /*
    hw_hdrmge_shortBase_en = 1
    */
    mge_baseHdrS_mode = 1
} mge_baseFrm_mode_t;

typedef enum mge_baseHdrL_mode_e {
    mge_oeMdByLuma_mode       = 0,
    mge_oeMdByLuma_rawCh_mode = 1
} mge_baseHdrL_mode_t;

typedef enum mge_baseHdrS_mode_e {
    mge_motionDR_SPrior_mode = 0,
    mge_motionDR_LPrior_mode = 1
} mge_baseHdrS_mode_t;

typedef struct mge_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(expRat),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO.\n Freq of use: low))  */
    mge_expRat_t expRat;
} mge_params_static_t;

typedef struct mge_params_dyn_s {
    /* M4_GENERIC_DESC(
       M4_ALIAS(sw_mgeT_baseFrm_mode),
       M4_TYPE(enum),
       M4_ENUM_DEF(mge_baseFrm_mode_t),
       M4_DEFAULT(mge_baseHdrL_mode),
       M4_GROUP_CTRL(baseFrm_mode_group),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES(TODO\n
       Reference enum types.\n
       Freq of use: high))  */
    mge_baseFrm_mode_t sw_mgeT_baseFrm_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_baseHdrL_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(mge_baseHdrL_mode_t),
        M4_DEFAULT(mge_oeMdByLuma_mode),
        M4_GROUP_CTRL(baseHdrL_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Reference enum types.\n
        Freq of use: low))  */
    mge_baseHdrL_mode_t sw_mgeT_baseHdrL_mode;
	/* M4_GENERIC_DESC(
        M4_ALIAS(sw_mgeT_baseHdrS_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(mge_baseHdrS_mode_t),
        M4_DEFAULT(mge_motionDR_SPrior_mode),
        M4_GROUP_CTRL(baseHdrS_mode_group),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Reference enum types.\n
        Freq of use: low))  */
    mge_baseHdrS_mode_t sw_mgeT_baseHdrS_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(oeWgt),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Users configure the overexposure LUT through oeWgt. \n
       Freq of use: high))  */
    mge_oeWgt_t oeWgt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdWgt_baseHdrL),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_GROUP(baseFrm_mode_group:mge_baseHdrL_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Users configure the luma motion detect LUT and the raw single channel overexposure
       detect LUT through mdWgt_baseHdrL when sw_mgeT_baseFrm_mode == mge_baseHdrL_mode. \n
       Freq of use: high))  */
    mge_mdWgt_baseHdrL_t mdWgt_baseHdrL;
    /* M4_GENERIC_DESC(
        M4_ALIAS(mdWgt_baseHdrS),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_GROUP(baseFrm_mode_group:mge_baseHdrS_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Users configure the luma motion detect weight through mdWgt_baseHdrS when
       sw_mgeT_baseFrm_mode == mge_baseHdrS_mode. \n
       Freq of use: low))  */
    mge_mdWgt_baseHdrS_t mdWgt_baseHdrS;
} mge_params_dyn_t;

typedef struct mge_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of mge module))  */
    mge_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    mge_params_dyn_t dyn;
} mge_param_t;

#endif