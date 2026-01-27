#pragma once

#define SPNR_ISO_STEP_MAX 8

typedef struct {
    float filtSpatial_strg; // 3
    float rgeSgm_scale; // 15
    float filtOut_alpha; // 1.0
} spnr_bf_t;

typedef struct spnr_params_dyn_s {
	spnr_bf_t bf;
}spnr_params_dyn_t;

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
        M4_ALIAS(dyn),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    spnr_params_dyn_t dyn[SPNR_ISO_STEP_MAX];
} rk_irfpa_sw_spnr_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    spnr_params_dyn_t dyn;
} spnr_params_t;