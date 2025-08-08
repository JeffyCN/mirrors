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

#ifndef _RK_AIQ_PARAM_CP10_H_
#define _RK_AIQ_PARAM_CP10_H_


typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cpT_bright_adjVal),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(128),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The brightness adjustment value.\n The actual change is hw_cpT_bright_adjVal-128.))  */
    unsigned char brightness;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cpT_contrast_adjStrg),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(128),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The contrast adjustment value.\n The actual strength is hw_cpT_contrast_adjStrg/128.))  */
    unsigned char contrast;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cpT_sat_adjStrg),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(128),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The saturation adjustment value.\n The actual strength is hw_cpT_sat_adjStrg/128.))  */
    unsigned char saturation;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cpT_hue_adjVal),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(128),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The hue adjustment value.\n The changing radian value is arctan(hw_cpT_hue_adjVal-128).))  */
    unsigned char hue;
} cp_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    cp_params_static_t sta;
} cp_param_t;

#endif
