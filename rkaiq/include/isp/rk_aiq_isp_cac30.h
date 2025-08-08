/*
 * rk_aiq_param_cac30.h
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
#ifndef _RK_AIQ_PARAM_CAC30_H_
#define _RK_AIQ_PARAM_CAC30_H_

typedef struct cac_params_static_s{
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_psfTableFix_bit),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,10),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_cacT_psfTableFix_bit
    uint8_t hw_cacT_psfTableFix_bit;
} cac_params_static_t;

typedef struct cac_psfTable_s {
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_bluePsf_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(2),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_cacT_psf_b_ker0/1
	float hw_cacT_bluePsf_sigma;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_redPsf_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(2),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_cacT_psf_r_ker0/1
	float hw_cacT_redPsf_sigma;
} cac_psfTable_t;

typedef struct cac_g_interp_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_hiDrct_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3.999),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // @reg: hw_cacT_hiDrct_ratio
    float hw_cacT_hiDrct_ratio;
} cac_g_interp_t;

typedef struct cac_edge_detect_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_edgeDetect_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: high))  */
    // reg: hw_cacT_edgeDetect_en;
    bool hw_cacT_edgeDetect_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_flat_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_flat_thred;
    float hw_cacT_flat_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_flat_offset),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535]),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // @reg: sw_cac_flat_thed_b
    unsigned int hw_cacT_flat_offset;
} cac_edge_detect_t;

typedef struct cac_chromaAberrCorr_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_chromaLoFlt_coeff0),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4]),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_cacT_chromaLoFlt_coeff0
    int hw_cacT_chromaLoFlt_coeff0;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_chromaLoFlt_coeff1),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4]),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_cacT_chromaLoFlt_coeff1
    //Note: ((hw_cacT_chromaLoFlt_coeff0+2*hw_cacT_chromaLoFlt_coeff1)<=7) && ((hw_cacT_chromaLoFlt_coeff0 + 2*hw_cacT_chromaLoFlt_coeff1)>=1).
    int hw_cacT_chromaLoFlt_coeff1;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_searchRange_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // reg: hw_cacT_searchRange_ratio;
	float hw_cacT_searchRange_ratio;
       /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColorB_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtColorB_minThred;
    // check fixed 20bit or 22bit
	float hw_cacT_wgtColorB_minThred;
       /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColorB_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 16),
        M4_DEFAULT(0.6),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtColorB_slope;
	float hw_cacT_wgtColorB_maxThred;
       /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColorR_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2),
        M4_DEFAULT(0),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtColorR_minThred;
	float hw_cacT_wgtColorR_minThred;
        /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColorR_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 16),
        M4_DEFAULT(0.1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtColorR_slope;
	float hw_cacT_wgtColorR_maxThred;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColor_minLuma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtColor_minLuma;
	float hw_cacT_wgtColor_minLuma;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColor_maxLuma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtColorLuma_slope;
	float hw_cacT_wgtColor_maxLuma;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_residualChroma_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_residualChroma_ratio;
	float hw_cacT_residualChroma_ratio;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColorLoFlt_coeff0),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // reg: hw_cacT_colorLoFlt_coeff0;
    int hw_cacT_wgtColorLoFlt_coeff0;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColorLoFlt_coeff1),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 4),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // reg: hw_cacT_colorLoFlt_coeff1;
    //Note: ((hw_cacT_colorLoFlt_coeff0+2*hw_cacT_colorLoFlt_coeff1)<=7) && ((hw_cacT_colorLoFlt_coeff0+2*hw_cacT_colorLoFlt_coeff1)>=1).
    int hw_cacT_wgtColorLoFlt_coeff1;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtOverExpo_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(0.8),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtOverExpo_minThred;
	float hw_cacT_wgtOverExpo_minThred;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtOverExpo_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1.00),
        M4_DEFAULT(0.9),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtOverExpo_slope;
	float hw_cacT_wgtOverExpo_maxThred;
     /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_contrast_offset),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1048575),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_contrast_offset;
	unsigned int hw_cacT_contrast_offset;
      /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtContrast_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.6),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_contrast_offset;
	float hw_cacT_wgtContrast_minThred;
      /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtContrast_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.9),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgtContrast_slope;
	float hw_cacT_wgtContrast_maxThred;
      /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtDarkArea_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgt_dark_thed;
	float hw_cacT_wgtDarkArea_minThred;
      /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtDarkArea_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_wgt_dark_slope;
	float hw_cacT_wgtDarkArea_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_wgtColor_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: high))  */
    // reg: sw_cacT_wgt_color_en;
	bool hw_cacT_wgtColor_en;
}cac_chromaAberrCorr_t;

typedef struct cac_hfCalc_s{
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_negClip0_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    // reg: hw_cacT_negClip0_en;
    bool  hw_cacT_negClip0_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_overExpoDetect_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: high))  */
    // reg: no reg;
    bool hw_cacT_overExpoDetect_en;
      /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_overExpo_thred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_overExpo_thred;
	float hw_cacT_overExpo_thred;
      /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cacT_overExpo_adj),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: high))  */
    // reg: hw_cacT_overExpo_adj;
	float hw_cacT_overExpo_adj;
}cac_hfCalc_t;

typedef struct {
      /* M4_GENERIC_DESC(
        M4_ALIAS(psfTable),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cac_psfTable_t psfTable;
       /* M4_GENERIC_DESC(
        M4_ALIAS(gInterp),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cac_g_interp_t gInterp;
     /* M4_GENERIC_DESC(
        M4_ALIAS(chromaAberrCorr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cac_chromaAberrCorr_t chromaAberrCorr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(edgeDetect),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cac_edge_detect_t edgeDetect;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hfCalc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    cac_hfCalc_t hfCalc;
} cac_params_dyn_t;

typedef struct cac_param_s{
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of demosaic module))  */
    cac_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    cac_params_dyn_t dyn;
} cac_param_t;

#endif
