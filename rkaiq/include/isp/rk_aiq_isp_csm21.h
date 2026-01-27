/*
 * rk_aiq_param_csm21.h
 *
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
#ifndef _RK_AIQ_PARAM_CSM21_H_
#define _RK_AIQ_PARAM_CSM21_H_

#define RK_AIQ_CSM_COEFF_NUM 9

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_csmT_full_range),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable full rang))  */
    bool hw_csmT_full_range;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_csmT_y_offset),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,63),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The y offset.))  */
    uint8_t hw_csmT_y_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_csmT_c_offset),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The c offset.))  */
    uint8_t hw_csmT_c_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_csmT_coeff),
        M4_TYPE(f32),
        M4_SIZE_EX(3,3),
        M4_RANGE_EX(-2, 1.992),
        M4_DEFAULT([0.299, 0.587, 0.114, -0.169, -0.331, 0.500, 0.500 -0.419 -0.081]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(The color space conversion matrix coeff.))  */
    float sw_csmT_coeff[RK_AIQ_CSM_COEFF_NUM];
} csm_params_static_t;


typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    csm_params_static_t sta;
} csm_param_t;

#endif
