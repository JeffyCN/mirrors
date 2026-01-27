/*
 *rk_aiq_types_asharp_algo_prvt.h
 *
 *  Copyright (c) 2024 Rockchip Corporation
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
#ifndef _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_V34_H_
#define _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_V34_H_

#include "RkAiqCalibDbTypes.h"
#include "asharpV34/rk_aiq_types_asharp_algo_int_v34.h"
#include "xcam_common.h"
#include "xcam_log.h"

// RKAIQ_BEGIN_DECLARE

// anr context
typedef struct Asharp_Context_V34_s {
    Asharp_ExpInfo_V34_t stExpInfo;
    Asharp_State_V34_t eState;
    Asharp_OPMode_V34_t eMode;

    Asharp_Auto_Attr_V34_t stAuto;
    Asharp_Manual_Attr_V34_t stManual;

    struct list_head* list_sharp_v34;

    rk_aiq_sharp_strength_v34_t stStrength;

    bool isIQParaUpdate;
    bool isGrayMode;
    Asharp_ParamMode_V34_t eParamMode;

    int rawWidth;
    int rawHeight;

    int prepare_type;
    CalibDbV2_SharpV34_t sharp_v34;

    int isReCalculate;
} Asharp_Context_V34_t;

// RKAIQ_END_DECLARE
#endif