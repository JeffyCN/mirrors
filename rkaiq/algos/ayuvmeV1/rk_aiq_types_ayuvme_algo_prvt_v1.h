/*
 *rk_aiq_types_alsc_algo_prvt.h
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

#ifndef _RK_AIQ_TYPES_AYUVME_ALGO_PRVT_V1_H_
#define _RK_AIQ_TYPES_AYUVME_ALGO_PRVT_V1_H_

#include "ayuvmeV1/rk_aiq_types_ayuvme_algo_int_v1.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"

//RKAIQ_BEGIN_DECLARE



//anr context
typedef struct Ayuvme_Context_V1_s {
    Ayuvme_ExpInfo_V1_t stExpInfo;
    Ayuvme_State_V1_t eState;
    Ayuvme_OPMode_V1_t eMode;

    Ayuvme_Auto_Attr_V1_t stAuto;
    Ayuvme_Manual_Attr_V1_t stManual;

    //struct list_head* list_yuvme_v1;

    rk_aiq_yuvme_strength_v1_t stStrength;

    bool isIQParaUpdate;
    bool isGrayMode;
    Ayuvme_ParamMode_V1_t eParamMode;

    int rawWidth;
    int rawHeight;
    int prepare_type;

    CalibDbV2_YuvmeV1_t yuvme_v1;
    int isReCalculate;
} Ayuvme_Context_V1_t;






//RKAIQ_END_DECLARE

#endif


