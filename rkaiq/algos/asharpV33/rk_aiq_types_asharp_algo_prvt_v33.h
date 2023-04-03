/*
 *rk_aiq_types_asharp_algo_prvt.h
 *
 *  Copyright (c) 2022 Rockchip Corporation
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
#ifndef _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_V33_H_
#define _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_V33_H_

#include "RkAiqCalibDbTypes.h"
#include "asharpV33/rk_aiq_types_asharp_algo_int_v33.h"
#include "xcam_common.h"
#include "xcam_log.h"

// RKAIQ_BEGIN_DECLARE

// anr context
typedef struct Asharp_Context_V33_s {
    Asharp_ExpInfo_V33_t stExpInfo;
    Asharp_State_V33_t eState;
    Asharp_OPMode_V33_t eMode;

#if RKAIQ_HAVE_SHARP_V33
    Asharp_Auto_Attr_V33_t stAuto;
    Asharp_Manual_Attr_V33_t stManual;
#else
    Asharp_Auto_Attr_V33LT_t stAuto;
    Asharp_Manual_Attr_V33LT_t stManual;
#endif

    struct list_head* list_sharp_v33;

    rk_aiq_sharp_strength_v33_t stStrength;

    bool isIQParaUpdate;
    bool isGrayMode;
    Asharp_ParamMode_V33_t eParamMode;

    int rawWidth;
    int rawHeight;

    int prepare_type;
#if RKAIQ_HAVE_SHARP_V33
    CalibDbV2_SharpV33_t sharp_v33;
#else
    CalibDbV2_SharpV33Lite_t sharp_v33;
#endif

    int isReCalculate;
} Asharp_Context_V33_t;

// RKAIQ_END_DECLARE
#endif