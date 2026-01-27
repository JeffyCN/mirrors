#pragma once

#define TNR_ISO_STEP_MAX 8

typedef struct tnr_params_dyn_s {
	bool hiMd_en; // true
    int wgtcal_mode; // 1
    bool loMd_en; // true

    float sigmaIdxFilt_strg; //2
    float sigma_scale; //2.8

    bool lpfhi_en; // true
    bool lpflo_en; // true
    float hfLpf_strg; // 1
    float Lpf_strg; // 3

    float lowgt_calc_firstline_sigma_scale; // 1
    float lowgt_vfilt_wgt; // 1
    float lowgt_calc_offset; // 0.2
    float lowgt_calc_scale; // 2
    float lowgt_calc_maxv; // 4
    float lowgt_vfilt_offset; // 0.1
    float lowgt_vfilt_scale; // 1

    bool lowgt_hfilt_en; // true
    float lowgt_hfilt_strg; // 1

    float lomge_prewgt_offset; // 1
    float lomge_prewgt_scale; // 2

    float mode0_base_ratio; // 1
    float mode0_lowgt_scale; // 0.5
    float mode0_wgtout_maxv; // 1
    float mode0_wgtout_neg_offset; // 0.1

	float mode1_lowgt_scale; // 4
	float mode1_lowgt_offset; // 1
	float mode1_wgt_scale; // 0.5
	float mode1_wgt_offset; // 0.5
	float mode1_wgt_minv; // 0
    float mode1_wgt_maxv; // 2

    int tnr_merge_frames_min; // 0
    int tnr_merge_frames_max; // 32
    int luma2sigma_x[20]; // [6000, 6200, 6400, 6600, 6800,7028, 7273, 7473, 7532, 7738, 7897, 7981, 7989, 8112, 8312, 8452, 8509, 9120, 9509, 9763]
    int luma2sigma_y[20]; // [10,    10,   10,   10,   10,  10,   10,    10,   9,   8,    7,   6,     5,     5,   4,     4,   4,      4,   4,   4]
}tnr_params_dyn_t;

typedef struct tnr_params_static_s {
    bool debug_md_wgt_out_en;// false
}tnr_params_static_t;

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
    tnr_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    tnr_params_dyn_t dyn[TNR_ISO_STEP_MAX];
} rk_irfpa_sw_tnr_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    tnr_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    tnr_params_dyn_t dyn;
} sw_tnr_params_t;