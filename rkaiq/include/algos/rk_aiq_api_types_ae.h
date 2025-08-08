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

#ifndef _RK_AIQ_API_TYPES_AE_H_
#define _RK_AIQ_API_TYPES_AE_H_

#include "isp/rk_aiq_isp_ae25.h"
#include "common/rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE
/*****************************************************************************/
/**
 * @brief   AEC Params: AEC Advanced Params
 */
/*****************************************************************************/
typedef struct ae_expRange_s {
    // M4_NUMBER_DESC("sw_aeT_range_min", "f32", M4_RANGE(0,1), "0", M4_DIGIT(2), M4_HIDE(0))
    float sw_aeT_time_min;

    // M4_NUMBER_DESC("sw_aeT_time_max", "f32", M4_RANGE(0,1), "0", M4_DIGIT(2), M4_HIDE(0))
    float sw_aeT_time_max;

    // M4_NUMBER_DESC("sw_aeT_gain_min", "f32", M4_RANGE(1,1024), "0", M4_DIGIT(2), M4_HIDE(0))
    float sw_aeT_gain_min;

    // M4_NUMBER_DESC("sw_aeT_gain_max", "f32", M4_RANGE(1,1024), "0", M4_DIGIT(2), M4_HIDE(0))
    float sw_aeT_gain_max;

    // M4_NUMBER_DESC("sw_aeT_ispDGain_min", "f32", M4_RANGE(1,1024), "0", M4_DIGIT(2), M4_HIDE(0))
    float sw_aeT_ispDGain_min;

    // M4_NUMBER_DESC("sw_aeT_ispDGain_max", "f32", M4_RANGE(1,1024), "0", M4_DIGIT(2), M4_HIDE(0))
    float sw_aeT_ispDGain_max;

    // M4_NUMBER_DESC("sw_aeT_pIrisGain_min", "s32", M4_RANGE(1,1024), "0", M4_DIGIT(2), M4_HIDE(0))
    int sw_aeT_pIrisGain_min;

    // M4_NUMBER_DESC("sw_aeT_pIrisGain_max", "s32", M4_RANGE(1,1024), "0", M4_DIGIT(2), M4_HIDE(0))
    int sw_aeT_pIrisGain_max;
} ae_expRange_t;

typedef struct ae_advanced_s {
    // M4_BOOL_DESC("sw_aeT_advGridWgt_en", "0")
    bool sw_aeT_advGridWgt_en;

    // M4_ARRAY_DESC("sw_aeT_advGrid_wgt", "u8", M4_SIZE(1,255), M4_RANGE(0,32), "1", M4_DIGIT(0), M4_DYNAMIC(0))
    uint8_t sw_aeT_advGrid_wgt[AE_GRIDWEIGHT_MAX_NUM];

    // M4_BOOL_DESC("sw_aeT_advAeRange_en", "0")
    bool sw_aeT_advAeRange_en;

    // M4_STRUCT_DESC("linExpRange", "normal_ui_style")
    ae_expRange_t linExpRange;

    // M4_STRUCT_DESC("hdrExpRange", "normal_ui_style")
    ae_expRange_t hdrExpRange[AE_HDRFRAME_MAX_NUM];
} ae_advanced_t;

/*****************************************************************************/
/**
 * @brief   AEC Params: AEC Info Params
 */
/*****************************************************************************/
typedef struct ae_expParam_s {
    // M4_NUMBER_DESC("CISTime", "f32", M4_RANGE(0,1), "0", M4_DIGIT(6))
    float integration_time;

    // M4_NUMBER_DESC("CISGain", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(3))
    float analog_gain;

    // M4_NUMBER_DESC("digital_gain", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(3),M4_HIDE(1))
    float digital_gain;

    // M4_NUMBER_DESC("isp_dgain", "f32", M4_RANGE(0,256), "0", M4_DIGIT(3),M4_HIDE(1))
    float isp_dgain;

    // M4_NUMBER_DESC("iso", "s32", M4_RANGE(0,524288), "0", M4_DIGIT(0),M4_HIDE(1))
    int iso;

    // M4_NUMBER_DESC("dcg_mode", "s32", M4_RANGE(-1,1), "0", M4_DIGIT(0))
    int dcg_mode;

    // M4_NUMBER_DESC("longfrm_mode", "s32", M4_RANGE(0,1), "0", M4_DIGIT(0), M4_HIDE(1))
    int longfrm_mode;
} ae_expParam_t;

typedef struct ae_linExpInfo_s {
    // M4_NUMBER_DESC("devLuma", "f32", M4_RANGE(-256,256), "0.0", M4_DIGIT(3), M4_HIDE(1))
    float devLuma;

    // M4_NUMBER_DESC("meanLuma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float meanLuma;

    // M4_STRUCT_DESC("expRange", "normal_ui_style")
    ae_expRange_t expRange;

    // M4_STRUCT_DESC("expParam", "normal_ui_style")
    ae_expParam_t expParam;
} ae_linExpInfo_t;

typedef struct ae_hdrExpInfo_s {
    // M4_ARRAY_DESC("devLuma", "f32", M4_SIZE(1,3), M4_RANGE(-256,256), "0.0", M4_DIGIT(3), M4_HIDE(1))
    float devLuma[AE_HDRFRAME_MAX_NUM];

    // M4_NUMBER_DESC("frm0Luma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float frm0Luma;

    // M4_NUMBER_DESC("frm1Luma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float frm1Luma;

    // M4_NUMBER_DESC("frm2Luma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float frm2Luma;

    // M4_STRUCT_LIST_DESC("expRange", M4_SIZE(1,3), "normal_ui_style")
    ae_expRange_t expRange[AE_HDRFRAME_MAX_NUM];

    // M4_STRUCT_LIST_DESC("expParam", M4_SIZE(1,3), "normal_ui_style")
    ae_expParam_t expParam[AE_HDRFRAME_MAX_NUM];
} ae_hdrExpInfo_t;

typedef struct ae_queryInfo_s {
    // M4_BOOL_DESC("isConverged", "0", M4_HIDE(1))
    bool isConverged;

    // M4_BOOL_DESC("isExpMax", "0", M4_HIDE(1))
    bool isExpMax;

    // M4_BOOL_DESC("isExpMin", "0", M4_HIDE(1))
    bool isExpMin;

    // M4_BOOL_DESC("envChange", "0", M4_HIDE(1))
    bool envChange;

    // M4_STRUCT_DESC("linExpInfo", "normal_ui_style")
    ae_linExpInfo_t linExpInfo;

    // M4_STRUCT_DESC("hdrExpInfo", "normal_ui_style")
    ae_hdrExpInfo_t hdrExpInfo;

    // M4_NUMBER_DESC("gblEnvLv", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(2), M4_HIDE(1))
    float gblEnvLv;

    // M4_NUMBER_DESC("oeROIPdf", "f32", M4_RANGE(0,1), "0", M4_DIGIT(2), M4_HIDE(1))
    float oeROIPdf;

    // M4_NUMBER_DESC("hiLitROIPdf", "f32", M4_RANGE(0,1), "0", M4_DIGIT(2), M4_HIDE(1))
    float hiLitROIPdf;

    // M4_NUMBER_DESC("loLitROIPdf", "f32", M4_RANGE(0,1), "0", M4_DIGIT(2), M4_HIDE(1))
    float loLitROIPdf;

    // M4_NUMBER_DESC("vts", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(2), M4_HIDE(1))
    float vts;

    // M4_NUMBER_DESC("hts", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(2), M4_HIDE(1))
    float hts;

    // M4_NUMBER_DESC("pclk", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(2), M4_HIDE(1))
    float pclk;

    // M4_NUMBER_DESC("fps", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(2))
    float fps;
} ae_queryInfo_t;

/*****************************************************************************/
/**
 * @brief   AEC Params: AEC API Params
 */
/*****************************************************************************/
typedef struct ae_api_expSwAttr_s {
    // M4_STRUCT_DESC("commCtrl", "normal_ui_style")
    ae_commCtrl_t commCtrl;
    // M4_STRUCT_DESC("advanced", "normal_ui_style")
    ae_advanced_t advanced;
} ae_api_expSwAttr_t;

typedef ae_linAeCtrl_t ae_api_linExpAttr_t;
typedef ae_hdrAeCtrl_t ae_api_hdrExpAttr_t;
typedef ae_irisCtrl_t ae_api_irisAttr_t;
typedef ae_syncTest_t ae_api_syncTestAttr_t;
typedef ae_queryInfo_t ae_api_queryInfo_t;

RKAIQ_END_DECLARE

#endif
