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

#ifndef _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_V4_H_
#define _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_V4_H_

#include "asharp4/rk_aiq_types_asharp_algo_int_v4.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"

//RKAIQ_BEGIN_DECLARE



//anr context
typedef struct Asharp_Context_V4_s {
    Asharp4_ExpInfo_t stExpInfo;
    Asharp4_State_t eState;
    Asharp4_OPMode_t eMode;

    Asharp_Auto_Attr_V4_t stAuto;
    Asharp_Manual_Attr_V4_t stManual;

    struct list_head* list_sharp_v4;

    rk_aiq_sharp_strength_v4_t stStrength;

    bool isIQParaUpdate;
    bool isGrayMode;
    Asharp4_ParamMode_t eParamMode;

    int rawWidth;
    int rawHeight;

    int prepare_type;
    CalibDbV2_SharpV4_t sharp_v4;

    int isReCalculate;
} Asharp_Context_V4_t;






//RKAIQ_END_DECLARE

#endif


