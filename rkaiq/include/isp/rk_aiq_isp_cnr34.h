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

#ifndef _RK_AIQ_PARAM_CNR34_H_
#define _RK_AIQ_PARAM_CNR34_H_

#include "rk_aiq_isp_common_cnr.h"

typedef struct cnr_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(locSgmStrg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_locSgmStrg_dyn_t locSgmStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNrGuide_preProc),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_loNr_preProc_t loNrGuide_preProc;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNrGuide_bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_loNr_bifilt_t loNrGuide_bifilt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(loNrGuide_iirFilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_loNr_iirFilt_t loNrGuide_iirFilt;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiNr_preLpf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_hiNr_preLpf_t hiNr_preLpf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrC_luma2HiNrSgm_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(...\n
        ))  */
    // reg: hw_cnrT_hiFltVsigma_idx0 ~ 7, hw_cnrT_hiFltVsigma_val0 ~ 7
    cnr_sigmaCurve_t hw_cnrC_luma2HiNrSgm_curve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiNr_bifilt),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_hiNr_bifilt_t hiNr_bifilt;
} cnr_params_dyn_t;

typedef struct cnr_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_params_dyn_t dyn;
} cnr_param_t;

#endif
