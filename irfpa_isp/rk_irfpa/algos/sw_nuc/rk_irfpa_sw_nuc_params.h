#pragma once

typedef struct {
    uint8_t tnrFrm_num; // 10
} nuc_nuExtra_t;

typedef struct nuc_params_static_s {
    nuc_nuExtra_t nuExtra;
}nuc_params_static_t;

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
    nuc_params_static_t sta;
} rk_irfpa_sw_nuc_params_t;
