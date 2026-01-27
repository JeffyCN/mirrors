#pragma once

typedef enum {
    drc_linDrc_mode,
} drc_mode_e;

typedef struct {
    bool enable_smooth; // false
    float validHist_maxThred; // 99.0
    float validHist_minThred; // 1.0
    float smooth_strg; // 0.3
    uint16_t diff_limit; // 256
} drc_linDrc_t;

typedef struct drc_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(drc_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_mode),
        M4_DEFAULT(drc_linDrc_mode),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: high))  */
    drc_mode_e drc_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(linDrc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    drc_linDrc_t linDrc;
}drc_params_static_t;

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
    drc_params_static_t sta;
} rk_irfpa_sw_drc_params_t;
