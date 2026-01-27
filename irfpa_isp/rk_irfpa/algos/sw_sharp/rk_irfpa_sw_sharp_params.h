#pragma once

#define SHARP_ISO_STEP_MAX 8

typedef struct sw_sharp_params_dyn_s {
	int		luma_idx[8]; // [0, 1024,2048,4096,6144,10240,14336,16384]
	int 	vsigma[8]; //[8,12,16,16,16,16,16,16]
	float	preBifilt_scale; //1
	float	preBifilt_offset; //10
	float	preBifilt_alpha; //0.3
	float	guideFilt_alpha; //0.8

	float	detailBifilt_scale; //1
	float	detailBifilt_offset; //0
	float	detailBifilt_alpha; //0.8

	int 	filtCoeff_mode; //1

	float	preBifilt_rsigma; //0.5
	float	preBifilt_coeff[9]; //[0.2042, 0.1238, 0.0751,0,0,0,0,0,0]
	
	float	imgLpf0_radius; //1
	float	imgLpf1_radius; //2
	float	imgLpf0_rsigma; //1
	float	imgLpf1_rsigma; //2
	float	imgLpf0_strg; //6.5
	float	imgLpf1_strg; //3.5
	float	imgLpf0_coeff[25]; //[0.0632,0.0558,0.0492,0.0383,0.0338,0.0232]
	float	imgLpf1_coeff[25]; //[0.0632,0.0558,0.0492,0.0383,0.0338,0.0232]

	float	detailBifilt_rsigma; //0.6
	float	detailBifilt_coeff[9]; //[0.2042, 0.1238, 0.0751,0,0,0,0,0,0]

	int sharp_strg; // 5
}sw_sharp_params_dyn_t;

typedef struct sw_sharp_params_static_s {
    bool debug_hf_out_en; // false
}sw_sharp_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Enable bit.
        Freq of use: high))  */
    bool enable;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    sw_sharp_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    sw_sharp_params_dyn_t dyn[SHARP_ISO_STEP_MAX];
} rk_irfpa_sw_sharp_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    sw_sharp_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    sw_sharp_params_dyn_t dyn;
} sw_sharp_params_t;