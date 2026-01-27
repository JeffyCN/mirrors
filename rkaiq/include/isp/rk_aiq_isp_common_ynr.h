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

#ifndef _RK_AIQ_ISP_COMMON_YNR_H_
#define _RK_AIQ_ISP_COMMON_YNR_H_

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

typedef enum ynr_sigmaCurveCfg_mode_e {
    // @note: By configuring the sigma curve, the software internally calculates and generates sigma curve points based on the sigma coefficients and freq coeff to configure the sigma curve.
    ynr_cfgByCoeff2Curve_mode = 0,
    // @note: Configure the sigma curve points by directly configuring point value.
    ynr_cfgByCurveDirectly_mode = 1
} ynr_sigmaCurveCfg_mode_t;

typedef struct ynr_ynrScl_radi_static_s {
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
        M4_HIDE_EX(0),
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
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The pixels at different radial distances have different scaling strengths for pixSigma.\n
        The sgmScl is larger, the loNr and hiNr strength is stronger.
        Freq of use: high))  */
    //reg: hw_ynr_radius2strg_val0 ~16
    float hw_ynrT_radiDist2YnrScl_val[YNR_RNR_STRENGTH_SEGMENT_MAX];
} ynr_ynrScl_radi_dyn_t;

typedef struct ynr_sigmaCurve_dyn_s  {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1023]),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: high))  */
    uint16_t idx[YNR_SIGMACURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(val),
        M4_TYPE(f32),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(64),
        M4_HIDE_EX(0),
        M4_DIGIT_EX(3f9b),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.\n
        Freq of use: high))  */
    float val[YNR_SIGMACURVE_SEGMENT_MAX];
} ynr_sigmaCurve_dyn_t;

typedef struct ynr_coeff2SgmCurve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sigma_coeff),
        M4_TYPE(f64),
        M4_SIZE_EX(1,5),
        M4_RANGE_EX(-4095.0, 4095.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(20),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Freq of use: high))  */
    float sigma_coeff[YNR_SIGMACURVE_COEFF_MAX];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_lowFreqCoeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,16.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Software internally caulculate low frequency sigma curve points by sigma coeff and low frequency coeff.\n
        Freq of use: high))  */
    float lowFreqCoeff;
} ynr_coeff2SgmCurve_t;

#endif

