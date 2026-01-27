#pragma once

#define DESTRIPE_ISO_STEP_MAX 8
typedef struct {
    bool en; // true
    int dynamic_sw0; // 100
    int dynamic_sw1; // 150
    float filt_ssigma; // 5
    float filt_rsigma;  // 10
    float strip_threld; // 0.8
} destripe_mfStripExtra_t;

typedef struct {
    bool en; // true
    float filt_ssigma; // 5
    float filt_rsigma;  // 25
    float strip_threld; // 0.8
} destripe_hfStripExtra_t;

typedef struct {
    destripe_mfStripExtra_t mfStripExtra;
    destripe_hfStripExtra_t hfStripExtra;
} destripe_vDestrip_t;

typedef struct {
    destripe_mfStripExtra_t mfStripExtra;
    destripe_hfStripExtra_t hfStripExtra;
} destripe_hDestrip_t;

typedef struct destripe_params_dyn_s {
    destripe_vDestrip_t vDestrip;
    destripe_hDestrip_t hDestrip;
}destripe_params_dyn_t;

typedef struct destripe_params_static_s {
    int segnum_v; // 4
    int segnum_h; // 4
}destripe_params_static_t;

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
    destripe_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    destripe_params_dyn_t dyn[DESTRIPE_ISO_STEP_MAX];
} rk_irfpa_sw_destripe_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    destripe_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    destripe_params_dyn_t dyn;
} destripe_params_t;