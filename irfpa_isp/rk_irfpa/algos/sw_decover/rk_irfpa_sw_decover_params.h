#pragma once
#define DECOVER_MAX_PATH_LEN 255

typedef struct decover_params_static_s {
    float strg; // 45.0
    float sigma; //0.7

    bool fix_center_en; // false
    int center_x; // 200
    int center_y; // 150

    /* M4_GENERIC_DESC(
        M4_ALIAS(corrFile_path),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT("data/correct_val"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    char corrFile_path[DECOVER_MAX_PATH_LEN];
    int temp_min;   // 29
    int temp_mid;   // 41
    int temp_max;   // 53
}decover_params_static_t;

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
    decover_params_static_t sta;
} rk_irfpa_sw_decover_params_t;
