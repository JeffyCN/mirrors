/*
 *  Copyright (c) 2023 Rockchip Corporation
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

#ifndef _RK_AIQ_PARAM_CGC10_H_
#define _RK_AIQ_PARAM_CGC10_H_


typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(yuv_limit_group),
        M4_NOTES(Yuv Color Gamut Compression (cgc) module enable. \n
        true: Limit range, false: Full range(bypass)))  */
    bool cgc_yuv_limit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cgcT_ratio_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(yuv_limit_group),
        M4_NOTES(Cgc ratio, true: use y ratio = 219/256, c ratio = 224/256.))  */
    bool cgc_ratio_en;
} cgc_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    cgc_params_static_t sta;
} cgc_param_t;

#endif
