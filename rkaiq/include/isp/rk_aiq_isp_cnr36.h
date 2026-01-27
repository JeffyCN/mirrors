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

#ifndef _RK_AIQ_PARAM_CNR36_H_
#define _RK_AIQ_PARAM_CNR36_H_

#include "rk_aiq_isp_common_cnr.h"

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiBfFlt_localFltAlpha_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(cnrLocFiltAlpha_en_group),
        M4_NOTES(CNR bf3x3 out local alpha disable signal.\n
        Freq of use: low))  */
    // reg: !en = sw_cnr_local_alpha_dis
    bool hw_cnrT_locFiltAlpha_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiBfFlt_mergeFrames_maxLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1.0,1024.0),
        M4_DEFAULT(256.0),
        M4_DIGIT_EX(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(cnrLocFiltAlpha_en_group),
        M4_NOTES(CNR bf3x3 merge maxlimit..\n
        Higher the value, the higher the max merge frames strength for static region.\n
        Freq of use: low))  */
    // reg: hw_cnrT_bfMerge_maxLimit
    float hw_cnrT_locFiltAlpha_maxLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiBfFlt_localFltAlpha_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0.0,1.0),
        M4_DEFAULT(0.0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(cnrLocFiltAlpha_en_group),
        M4_NOTES(CNR bf3x3 alpha maxlimit..\n
        Higher the value, the higher the max merge frames strength for motion region.\n
        Freq of use: low))  */
    // reg: hiBfFlt_locAlpha_maxLimit
    float hw_cnrT_locFiltAlpha_minLimit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_hueSatAdj_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP_CTRL(cnr_hueSatAdj_en_group),
        M4_NOTES(Enable the hue & sat adjust control.Turn on by setting this bit to 1.
        Freq of use: high))  */
    // @reg: sw_cnr_hsv_alpha_en
    bool hw_cnrT_hueSatAdj_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_locSgmStrg2NrOutAlpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0.0, 16.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f1b),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 2, 4, 8, 12, 16, 80, 208, 464, 976]),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(cnr_hueSatAdj_en_group),
        M4_NOTES(TODO.\n
        Freq of use: high))  */
    // @reg: hw_cnrT_gainAdjAlpha_table0~10
    float hw_cnrT_locSgmStrg2NrOutAlpha[10];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_hue2NrOutAlpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f1b),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 128, 256, 320, 354, 384, 512, 640, 896, 1024]),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(cnr_hueSatAdj_en_group),
        M4_NOTES(TODO.\n
        Freq of use: high))  */
    // @reg: hw_cnrT_hsvAdjAlpha_table0~10
    float hw_cnrT_hue2NrOutAlpha[10];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_cnrT_sat2NrOutAlpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0.0, 1.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(3f1b),
        M4_HIDE_EX(0),
        M4_UI_MODULE(curve),
        M4_DATAX([0, 16, 32, 64, 96, 128, 192, 256, 384, 512]),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP(cnr_hueSatAdj_en_group),
        M4_NOTES(TODO.\n
        Freq of use: high))  */
    // @reg: hw_cnrT_satAdjAlpha_table0~10
    float hw_cnrT_sat2NrOutAlpha[10];
} cnr_hiNr_locFiltAlpha_t;


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
    /* M4_GENERIC_DESC(
        M4_ALIAS(hiNr_locFiltAlpha),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    cnr_hiNr_locFiltAlpha_t hiNr_locFiltAlpha;
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

