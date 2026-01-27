/*
 * rk_aiq_isp_3dlut20.h
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
#ifndef _RK_AIQ_PARAM_3DLUT20_H_
#define _RK_AIQ_PARAM_3DLUT20_H_

typedef struct {   
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_lut3dC_lutR_val),
       M4_TYPE(u16),
       M4_SIZE_EX(81,9),
       M4_RANGE_EX(0,1023),
       M4_DEFAULT(0),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(0),
       M4_NOTES(TODO))  */
    uint16_t hw_lut3dC_lutR_val[729];
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_lut3dC_lutG_val),
       M4_TYPE(u16),
       M4_SIZE_EX(81,9),
       M4_RANGE_EX(0,4095),
       M4_DEFAULT(0),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(1),
       M4_NOTES(TODO))  */
    uint16_t hw_lut3dC_lutG_val[729];
    /* M4_GENERIC_DESC(
       M4_ALIAS(hw_lut3dC_lutB_val),
       M4_TYPE(u16),
       M4_SIZE_EX(81,9),
       M4_RANGE_EX(0,1023),
       M4_DEFAULT(0),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(2),
       M4_NOTES(TODO))  */
    uint16_t hw_lut3dC_lutB_val[729];
} lut3d_meshGain_t;


typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(meshGain),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO)) */
    lut3d_meshGain_t meshGain;
} lut3d_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    lut3d_params_dyn_t dyn;
} lut3d_param_t;

#define RK_AIQ_3DLUT20_LUT_WSIZE      9*9*9
#define RK_AIQ_3DLU20_LUT_GRID_NUM    9

#endif
