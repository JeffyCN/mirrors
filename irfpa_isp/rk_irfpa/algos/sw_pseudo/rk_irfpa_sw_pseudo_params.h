#pragma once

typedef enum {
    RK_IRFPA_PSEUDO_COLOR_WHITE_HOT,         ///< 白热模式
    RK_IRFPA_PSEUDO_COLOR_BLACK_HOT,         ///< 黑热模式
    RK_IRFPA_PSEUDO_COLOR_RED_HOT,           ///< 红热模式
    RK_IRFPA_PSEUDO_COLOR_LAVA,              ///< 熔岩模式
    RK_IRFPA_PSEUDO_COLOR_IRON_RED,          ///< 铁红模式
    RK_IRFPA_PSEUDO_COLOR_HOT_IRON,          ///< 热铁模式
    RK_IRFPA_PSEUDO_COLOR_MEDICAL,           ///< 医疗模式
    RK_IRFPA_PSEUDO_COLOR_MAX                ///< 最大模式数量
} sw_pseudo_color_e;

typedef enum {
    RK_IRFPA_PSEUDO_MODE_RGB,
    RK_IRFPA_PSEUDO_MODE_NV12_BT601,
    RK_IRFPA_PSEUDO_MODE_NV12_BT709
} sw_pseudo_mode_e;

typedef struct pseudo_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(nv12_mode),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO\n
        Freq of use: high))  */
    bool nv12_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pseudo_color_e),
        M4_TYPE(enum),
        M4_ENUM_DEF(sw_pseudo_color_e),
        M4_DEFAULT(RK_IRFPA_PSEUDO_COLOR_LAVA),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: high))  */
    sw_pseudo_color_e color;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_pseudo_mode_e),
        M4_TYPE(enum),
        M4_ENUM_DEF(sw_pseudo_mode_e),
        M4_DEFAULT(RK_IRFPA_PSEUDO_MODE_NV12_BT601),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: high))  */
    sw_pseudo_mode_e pseudo_mode;
}pseudo_params_static_t;

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
    pseudo_params_static_t sta;
} rk_irfpa_sw_pseudo_params_t;
