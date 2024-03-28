/*
 * rk_aiq_param_ynr34.h
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

#ifndef _RK_AIQ_PARAM_TRANS10_H_
#define _RK_AIQ_PARAM_TRANS10_H_

typedef enum trans_mode_e {
    //hw_adrc_comps_mode == 0
    trans_lgi4f8_mode = 0,
    //hw_adrc_comps_mode == 1
    trans_lgi3f9_mode = 1,
} trans_mode_t;

typedef struct trans_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_comps_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(drc_dataInDecmps_mode_t),
        M4_DEFAULT(trans_lgi4f8_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n
        Reference enum types.\n
        Freq of use: low))  */
    // @reg: hw_adrc_comps_mode
    trans_mode_t hw_transCfg_trans_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_comps_offsetbits),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Compression offset for lg and dlg.\n
        Freq of use: low))  */
    // @reg: hw_adrc_comps_offsetbits
    uint16_t hw_transCfg_lscOutTrans_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_adrc_logTransform_offsetbits),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,15),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Offset used for de. It must be  be a power of 2.\n
        Freq of use: low))  */
    // reg: hw_adrc_logTransform_offsetbits
    uint16_t hw_transCfg_transOfDrc_offset;
} trans_params_static_t;

typedef struct trans_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    trans_params_static_t sta;
} trans_param_t;

#endif
