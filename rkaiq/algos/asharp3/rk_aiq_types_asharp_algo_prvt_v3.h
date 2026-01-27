/*
 *rk_aiq_types_alsc_algo_prvt.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_V3_H_
#define _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_V3_H_

#include "asharp3/rk_aiq_types_asharp_algo_int_v3.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"

//RKAIQ_BEGIN_DECLARE



//anr context
typedef struct Asharp_Context_V3_s {
    Asharp3_ExpInfo_t stExpInfo;
    Asharp3_State_t eState;
    Asharp3_OPMode_t eMode;

    Asharp_Auto_Attr_V3_t stAuto;
    Asharp_Manual_Attr_V3_t stManual;

    struct list_head* list_sharp_v3;

    float fSharp_Strength;

    bool isIQParaUpdate;
    bool isGrayMode;
    Asharp3_ParamMode_t eParamMode;

    int rawWidth;
    int rawHeight;

    int prepare_type;
    CalibDbV2_SharpV3_t sharp_v3;

    int isReCalculate;
} Asharp_Context_V3_t;






//RKAIQ_END_DECLARE

#endif


