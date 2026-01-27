/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _RK_AIQ_PARAM_AIYNR_H_
#define _RK_AIQ_PARAM_AIYNR_H_

#define AIYNR_MODEL_DIRNAME_LEN     128
#define AIYNR_MODEL_FILENAME_LEN    64
#define AIYNR_SIGMA_LUT_LEN         81
#define AIYNR_DECOMP_CURVE_LEN      33

typedef struct aiynr_param_calib_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrC_sigma_lut),
        M4_TYPE(f32),
        M4_SIZE_EX(1,81),
        M4_RANGE_EX(0, 255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Noise Intensity Lookup Table.\n
        Freq of use: High))  */
    float sw_aiYnrC_sigma_lut[AIYNR_SIGMA_LUT_LEN];
} aiynr_param_calib_t;

typedef struct aiynr_param_tunning_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_gainMerge_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The value of aipre global gain alpha.\n
        Freq of use: High))  */
    float sw_aiYnrT_gainMerge_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_globalGain_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 16.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The value of aipre global gain.\n
        Freq of use: High))  */
    float sw_aiYnrT_globalGain_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_noiseAdd_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The value of noise add ratio.\n
        Freq of use: High))  */
    float sw_aiYnrT_noiseAdd_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_noiseAddMot_offset),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1.0, 32.0),
        M4_DEFAULT(10.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The value of motion offset for adding noise.\n
        Freq of use: High))  */
    float sw_aiYnrT_noiseAddMot_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_noiseAddMot_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-8.0, 8.0),
        M4_DEFAULT(4.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(The value of motion coefficient for adding noise.\n
        Freq of use: High))  */
    float sw_aiYnrT_noiseAddMot_coeff;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_narManual_strg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(The value of global narmap strength.\n
        Freq of use: High))  */
    float sw_aiYnrT_narManual_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_narManual_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(The value of global narmap alpha.\n
        Freq of use: High))  */
    float sw_aiYnrT_narManual_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_narInverse_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(The enable bit of the narmap inverse mode.\n
        Freq of use: Low))  */
    bool sw_aiYnrT_narInverse_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_gain_leftshift),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,7),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Pyramid gain left-shift bits.\n
        Freq of use: high))  */
    uint8_t sw_aiYnrT_gain_leftshift;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_sigmaLut_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 16.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(The scale value of sigmaLut.\n
        Freq of use: High))  */
    float sw_aiYnrT_sigmaLut_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_noiseAdd_limit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(The value of noise add limit.\n
        Freq of use: High))  */
    float sw_aiYnrT_noiseAdd_limit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_decomp_curve),
        M4_TYPE(f32),
        M4_SIZE_EX(1,33),
        M4_RANGE_EX(0.0, 255.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(The curve value of sigma table.\n
        Freq of use: High))  */
    float sw_aiYnrT_decomp_curve[AIYNR_DECOMP_CURVE_LEN];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_model_file),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 64),
        M4_DEFAULT("iso50.bin"),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(12),
        M4_NOTES(Model info file.\n
        Freq of use: High))  */
    char sw_aiYnrT_model_file[AIYNR_MODEL_FILENAME_LEN];
} aiynr_param_tunning_t;

typedef struct aiynr_param_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(calib),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(calib parameters))  */
    aiynr_param_calib_t calib;
    /* M4_GENERIC_DESC(
        M4_ALIAS(tunning),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(tunning parameters))  */
    aiynr_param_tunning_t tunning;
} aiynr_param_dyn_t;

typedef enum aiynr_debug_mode_e {
    aiynr_debugOff_mode = 0,
    aiynr_debugYsrc_mode = 1,
    aiynr_debugYdest_mode = 2
} aiynr_debug_mode_t;

typedef struct aiynr_debug_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_debug_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(aiynr_debug_mode_t),
        M4_DEFAULT(aiynr_debugOff_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Debug mode selection. Reference enum types.\nFreq of use: high))  */
    aiynr_debug_mode_t sw_aiYnrT_debug_mode;
} aiynr_debug_static_t;

typedef struct aiynr_model_dir_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiYnrT_model_file),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 128),
        M4_DEFAULT("/etc/iqfiles/"),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(20),
        M4_NOTES(Model info file.\n
        Freq of use: High))  */
    char sw_aiYnrCfg_model_dir[AIYNR_MODEL_DIRNAME_LEN];
} aiynr_model_dir_t;

typedef struct aiynr_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(debug),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Debug mode))  */
    aiynr_debug_static_t debug;
    /* M4_GENERIC_DESC(
        M4_ALIAS(model_dir),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The path to the model))  */
    aiynr_model_dir_t model_dir;
} aiynr_params_static_t;

typedef struct aiynr_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The static params of aiynr module))  */
    aiynr_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dynamic params of aiynr module))  */
    aiynr_param_dyn_t dyn;
} aiynr_param_t;

#endif
