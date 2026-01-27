/*
 * rk_aiq_param_fpnSw.h
 *
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef _RK_AIQ_PARAM_FPN_SW_H_
#define _RK_AIQ_PARAM_FPN_SW_H_

#define RKFPNSW_MAX_PATH_LEN 255

typedef enum fpnSw_swOnAt_isoIdx_e {
    fpnSw_swOnAt_isoIdx0 = 0,
    fpnSw_swOnAt_isoIdx1,
    fpnSw_swOnAt_isoIdx2,
    fpnSw_swOnAt_isoIdx3,
    fpnSw_swOnAt_isoIdx4,
    fpnSw_swOnAt_isoIdx5,
    fpnSw_swOnAt_isoIdx6,
    fpnSw_swOnAt_isoIdx7,
    fpnSw_swOnAt_isoIdx8,
    fpnSw_swOnAt_isoIdx9,
    fpnSw_swOnAt_isoIdx10,
    fpnSw_swOnAt_isoIdx11,
    fpnSw_swOnAt_isoIdx12,
    fpnSw_swOnAt_isoIdxMax
} fpnSw_swOnAt_isoIdx_t;

typedef enum fpnSw_working_mode_e {
    fpnSw_highCpu_mode = 0,
    fpnSw_highBW_mode,
} fpnSw_working_mode_t;

typedef struct fpnSw_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_fpnSw_offset),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(128),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(.))  */
    int sw_fpnSw_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_fpnSw_autoSwOn_thred),
        M4_TYPE(enum),
        M4_ENUM_DEF(fpnSw_swOnAt_isoIdx_t),
        M4_DEFAULT(fpnSw_swOnAt_isoIdx0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(.))  */
    fpnSw_swOnAt_isoIdx_t sw_fpnSw_autoSwOn_thred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_fpnSw_working_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(fpnSw_working_mode_t),
        M4_DEFAULT(fpnSw_highCpu_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(.))  */
    fpnSw_working_mode_t sw_fpnSw_working_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_fpnSw_procInSegmFirst_val),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(.))  */
    float sw_fpnSw_procInSegmFirst_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_fpnSw_raw_path),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT("/etc/iqfiles/fpn.raw"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    char sw_fpnSw_raw_path[RKFPNSW_MAX_PATH_LEN];
} fpnSw_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    fpnSw_params_static_t sta;
} fpnSw_param_t;

#endif
