/*
 * uvnr_head_v1.h
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

#ifndef __CALIBDBV2_CNRV31_HEADER_H__
#define __CALIBDBV2_CNRV31_HEADER_H__

#include "rk_aiq_comm.h"
// clang-format off

RKAIQ_BEGIN_DECLARE

///////////////////////////uvnr//////////////////////////////////////
typedef struct CalibDbV2_CNRV31_T_ISO_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50, 204800), "50", M4_DIGIT(1), "index2")
    float iso;

    // M4_BOOL_DESC("hw_cnrT_exgain_bypass", "0")
    bool hw_cnrT_exgain_bypass;

    ///////////new/////////////
    // M4_NUMBER_DESC("sw_cnrT_ds_scaleX", "u8", M4_RANGE(4, 8), "4", M4_DIGIT(0))
    uint8_t sw_cnrT_ds_scaleX;
    // M4_NUMBER_DESC("sw_cnrT_ds_scaleY", "u8", M4_RANGE(4, 4), "4", M4_DIGIT(0))
    uint8_t sw_cnrT_ds_scaleY;
    // M4_ARRAY_DESC("sw_cnrT_thumbBf_coeff", "f32", M4_SIZE(1,4), M4_RANGE(0.0,1.0), "1.0", M4_DIGIT(4), M4_DYNAMIC(0))
    float sw_cnrT_thumbBf_coeff[4];


    // M4_NUMBER_DESC("sw_cnrT_loBfFlt_vsigma", "f32", M4_RANGE(0.0, 1.0), "0.0039", M4_DIGIT(4))
    float sw_cnrT_loBfFlt_vsigma;
    // M4_NUMBER_DESC("sw_cnrT_loBfFlt_alpha", "f32", M4_RANGE(0.0, 1.0), "1.0", M4_DIGIT(2))
    float sw_cnrT_loBfFlt_alpha;

    // M4_ARRAY_DESC("hw_cnrT_loFlt_coeff", "u8", M4_SIZE(1,3), M4_RANGE(0,1), "1,1,1", M4_DIGIT(0), M4_DYNAMIC(0))
    uint8_t hw_cnrT_loFlt_coeff[3];
    // M4_NUMBER_DESC("sw_cnrT_loFlt_vsigma", "f32", M4_RANGE(0.0, 1.0), "0.0078", M4_DIGIT(4))
    float sw_cnrT_loFlt_vsigma;
    // M4_NUMBER_DESC("sw_cnrT_loFltWgt_maxLimit", "f32", M4_RANGE(0.0, 16.0), "1.0", M4_DIGIT(2))
    float sw_cnrT_loFltWgt_maxLimit;
    // M4_NUMBER_DESC("sw_cnrT_loFltWgt_minThred", "f32", M4_RANGE(0.0, 1.0), "0.0313", M4_DIGIT(4))
    float sw_cnrT_loFltWgt_minThred;
    // M4_NUMBER_DESC("sw_cnrT_loFltUV_gain", "f32", M4_RANGE(0.0, 1.0), "0.333", M4_DIGIT(3))
    float sw_cnrT_loFltUV_gain;
    // M4_NUMBER_DESC("sw_cnrT_loFltWgt_slope", "f32", M4_RANGE(0.0, 8.0), "1.0", M4_DIGIT(2))
    float sw_cnrT_loFltWgt_slope;

    // M4_NUMBER_DESC("sw_cnrT_gausFlt_alpha", "f32", M4_RANGE(0.0, 1.0), "1.0", M4_DIGIT(2))
    float sw_cnrT_gausFlt_alpha;


    // M4_ARRAY_DESC("sw_cnrT_hiFltVsigma_idx", "u16", M4_SIZE(1,8), M4_RANGE(0,1024), "0,64,128,256,384,640,896,1024", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t sw_cnrT_hiFltVsigma_idx[8];
    // M4_ARRAY_DESC("sw_cnrT_hiFlt_vsigma", "f32", M4_SIZE(1,8), M4_RANGE(0.0,1.0), "0.03", M4_DIGIT(2), M4_DYNAMIC(0))
    float sw_cnrT_hiFlt_vsigma[8];
    // M4_NUMBER_DESC("sw_cnrT_hiFltUV_gain", "f32", M4_RANGE(0.0, 8.0), "3.0", M4_DIGIT(2))
    float sw_cnrT_hiFltUV_gain;
    // M4_NUMBER_DESC("sw_cnrT_hiFltCur_wgt", "f32", M4_RANGE(0.0, 1.0), "0.0625", M4_DIGIT(4))
    float sw_cnrT_hiFltCur_wgt;
    // M4_NUMBER_DESC("sw_cnrT_hiFltWgt_minLimit", "f32", M4_RANGE(0.0, 1.0), "0.0078", M4_DIGIT(4))
    float sw_cnrT_hiFltWgt_minLimit;
    // M4_BOOL_DESC("hw_cnrT_hiFltWgt0_mode", "0")
    bool hw_cnrT_hiFltWgt0_mode;
    // M4_NUMBER_DESC("sw_cnrT_hiFlt_alpha", "f32", M4_RANGE(0.0, 1.0), "1.0", M4_DIGIT(2))
    float sw_cnrT_hiFlt_alpha;

    // M4_NUMBER_DESC("hw_cnrT_satAdj_offset", "f32", M4_RANGE(0.0, 511.0), "0", M4_DIGIT(4))
    float hw_cnrT_satAdj_offset;
    // M4_NUMBER_DESC("sw_cnrT_satAdj_scale", "f32", M4_RANGE(0.0, 32.0), "0.0", M4_DIGIT(2))
    float sw_cnrT_satAdj_scale;

    // M4_NUMBER_DESC("sw_cnrT_global_gain", "f32", M4_RANGE(0.0, 64.0), "1.0", M4_DIGIT(2))
    float sw_cnrT_global_gain;
    // M4_NUMBER_DESC("sw_cnrT_globalGain_alpha", "f32", M4_RANGE(0.0, 1.0), "0.0", M4_DIGIT(2))
    float sw_cnrT_globalGain_alpha;
    // M4_NUMBER_DESC("sw_cnrT_localGain_scale", "f32", M4_RANGE(0.0625, 1.0), "1.0", M4_DIGIT(2))
    float sw_cnrT_localGain_scale;
    // M4_NUMBER_DESC("sw_cnrT_loFltGlobalSgm_ratio", "f32", M4_RANGE(0.0, 4.0), "1.0", M4_DIGIT(2))
    float sw_cnrT_loFltGlobalSgm_ratio;
    // M4_NUMBER_DESC("sw_cnrT_loFltGlobalSgmRto_alpha", "f32", M4_RANGE(8.0, 8.0), "8.0", M4_DIGIT(0))
    float sw_cnrT_loFltGlobalSgmRto_alpha;

    // M4_ARRAY_DESC("sw_cnrT_gainAdjHiFltSgm_ratio", "f32", M4_SIZE(1,13), M4_RANGE(0.0,4.0), "1.0", M4_DIGIT(2), M4_DYNAMIC(0))
    float sw_cnrT_gainAdjHiFltSgm_ratio[13];
    // M4_ARRAY_DESC("sw_cnrT_gainAdjHiFltCur_wgt", "f32", M4_SIZE(1,13), M4_RANGE(0.0,4.0), "1.0", M4_DIGIT(2), M4_DYNAMIC(0))
    float sw_cnrT_gainAdjHiFltCur_wgt[13];


    // M4_BOOL_DESC("hw_cnrT_gausFltSigma_en", "1")
    bool hw_cnrT_gausFltSigma_en;
    // M4_NUMBER_DESC("sw_cnrT_gausFlt_sigma", "f32", M4_RANGE(0.0, 100.0), "1.0", M4_DIGIT(3))
    float sw_cnrT_gausFlt_sigma;
    // M4_ARRAY_DESC("sw_cnrT_gausFlt_coeff", "f32", M4_SIZE(1,6), M4_RANGE(0,127), "0.1758,0.1094,0.0234,0.1094,0.0664,0.0156", M4_DIGIT(4), M4_DYNAMIC(0))
    float sw_cnrT_gausFlt_coeff[6];

} CalibDbV2_CNRV31_T_ISO_t;

typedef struct CalibDbV2_CNRV31_T_Set_s {
    // M4_STRING_MARK_DESC("SNR_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "LSNR",M4_DYNAMIC(0), "index1")
    char *SNR_Mode;
    // M4_STRING_DESC("Sensor_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "lcg", M4_DYNAMIC(0))
    char *Sensor_Mode;
    // M4_STRUCT_LIST_DESC("Tuning_ISO", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_CNRV31_T_ISO_t *Tuning_ISO;
    int Tuning_ISO_len;

} CalibDbV2_CNRV31_T_Set_t;


typedef struct CalibDbV2_CNRV31_Tuning_s {
    // M4_BOOL_DESC("enable", "1")
    bool enable;
    // M4_STRUCT_LIST_DESC("Setting", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_CNRV31_T_Set_t *Setting;
    int Setting_len;
} CalibDbV2_CNRV31_Tuning_t;


typedef struct CalibDbV2_CNRV31_s {
    // M4_STRING_DESC("Version", M4_SIZE(1,1), M4_RANGE(0, 64), "V1", M4_DYNAMIC(0))
    char *Version;
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_CNRV31_Tuning_t TuningPara;
} CalibDbV2_CNRV31_t;

RKAIQ_END_DECLARE
// clang-format on

#endif
