/*
 * moduleinfo_head.h
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

#ifndef __CALIBDBV2_MODULE_INFO_HEADER_H__
#define __CALIBDBV2_MODULE_INFO_HEADER_H__
// #define M4_STRUCT_DESC(ALIAS, SIZE, UI_MODULE)
// #define M4_NUMBER_DESC(ALIAS, TYPE, SIZE, RANGE, DEFAULT)
// #define M4_STRING_DESC(ALIAS, SIZE, RANGE, DEFAULT)
// #define M4_ENUM_DESC(ALIAS, ENUM, DEFAULT)

#include "common/rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE
#define CIS_TNR_SIGMACURVE_SEGMENT_MAX 20
#define CIS_TNR_SIGMACURVE_FRMNUM_MAX 4
#define CIS_TNR_SIGMACURVE_ISO_MAX 13

typedef struct CalibDb_Sensor_Module_s {
    // M4_NUMBER_DESC("FNumber", "f32", M4_RANGE(1,100), "1.6", M4_DIGIT(2),M4_HIDE(0))
    float FNumber;
    // M4_NUMBER_DESC("EFL", "f32", M4_RANGE(0,200), "3.5", M4_DIGIT(2),M4_HIDE(0))
    float EFL;
    // M4_NUMBER_DESC("LensT", "f32", M4_RANGE(0,100), "90", M4_DIGIT(2),M4_HIDE(0))
    float LensT;
    // M4_NUMBER_DESC("IRCutT", "f32", M4_RANGE(0,100), "90", M4_DIGIT(2),M4_HIDE(0))
    float IRCutT;
    // M4_NUMBER_DESC("PixelSize", "f32", M4_RANGE(0,20), "2", M4_DIGIT(3),M4_HIDE(0))
    float PixelSize;
} CalibDb_Sensor_Module_t;

typedef struct CalibDb_Cis_PreLsc_s {
    // add param for preLsc

} CalibDb_Cis_PreLsc_t;

typedef struct CalibDb_Cis_Lsc_s {
    CalibDb_Cis_PreLsc_t cisPreLsc;
} CalibDb_Cis_Lsc_t;

typedef struct CalibDb_Cis_TnrSigmaCurve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,20),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    uint16_t idx[CIS_TNR_SIGMACURVE_SEGMENT_MAX];
    /* M4_GENERIC_DESC(
       M4_ALIAS(val),
       M4_TYPE(u16),
       M4_UI_PARAM(data_y),
       M4_SIZE_EX(1,20),
       M4_RANGE_EX(0,4095),
       M4_DEFAULT(256),
       M4_HIDE_EX(0),
       M4_RO(0),
       M4_ORDER(1),
       M4_NOTES(TODO))  */
    uint16_t val[CIS_TNR_SIGMACURVE_SEGMENT_MAX];
} CalibDb_Cis_TnrSigmaCurve_t;

typedef struct CalibDb_Cis_TnrSigmaCurveDyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(cisSigmaCurve_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: low))  */
    int sigmaCurve_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sigmaCurve),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,4),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The sigma calibration curve of the MD module. \n
        The MD only use the sigma data of the curve when sw_btnrCfg_sigma_mode == btnr_manualSigma_mode.\n
        The MD will use the sigma data of the curve and sigma statistics when sw_btnrCfg_sigma_mode == btnr_autoSigma_mode.\n
        Freq of use: low))  */
    CalibDb_Cis_TnrSigmaCurve_t sigmaCurve[CIS_TNR_SIGMACURVE_FRMNUM_MAX];
} CalibDb_Cis_TnrSigmaCurveDyn_t;


typedef struct CalibDb_Cis_Tnr_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(cisTnrSigmaCurve),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,13),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    CalibDb_Cis_TnrSigmaCurveDyn_t sigmaCurve_dyn[CIS_TNR_SIGMACURVE_ISO_MAX];
} CalibDb_Cis_Tnr_t;


typedef struct CalibDb_Module_Cis_s {
    // M4_STRUCT_DESC("cisLsc", "normal_ui_style")
    CalibDb_Cis_Lsc_t cisLsc;
    // M4_STRUCT_DESC("cisTnr", "normal_ui_style")
    CalibDb_Cis_Tnr_t cisTnr;
} CalibDb_Module_Cis_t;

typedef struct CalibDb_Module_ParaV2_s {
    // M4_STRUCT_DESC("SensorModule", "normal_ui_style")
    CalibDb_Sensor_Module_t sensor_module;
    // M4_STRUCT_DESC("CisModule", "normal_ui_style")
    CalibDb_Module_Cis_t module_cis;
} CalibDb_Module_ParaV2_t;

RKAIQ_END_DECLARE

#endif
