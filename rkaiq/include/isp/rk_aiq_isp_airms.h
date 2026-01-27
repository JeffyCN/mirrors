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

#ifndef _RK_AIQ_PARAM_AIRMS_H_
#define _RK_AIQ_PARAM_AIRMS_H_

#define AIRMS_MODEL_FILENAME_LEN    128

typedef struct airms_model_path_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_aiRmsCfg_model_file),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 128),
        M4_DEFAULT("/etc/iqfiles/rkrms_model_info.bin"),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(Model info file.\n
        Freq of use: High))  */
    char sw_aiRmsCfg_model_file[AIRMS_MODEL_FILENAME_LEN];
} airms_model_path_t;

typedef struct airms_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(model_dir),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    airms_model_path_t model_path;
} airms_params_static_t;

typedef struct airms_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The static params of airms module))  */
    airms_params_static_t sta;
} airms_param_t;

#endif
