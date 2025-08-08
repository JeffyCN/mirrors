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

typedef struct fpnSw_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_fpnSw_off),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(128),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(.))  */
    int sw_fpnSw_off;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_fpnSw_fps_val),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,30),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(.))  */
    int sw_fpnSw_fps_val;
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
