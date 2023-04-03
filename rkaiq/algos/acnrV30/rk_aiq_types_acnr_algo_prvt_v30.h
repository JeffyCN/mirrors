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

#ifndef _RK_AIQ_TYPES_ACNR_ALGO_PRVT_V30_H_
#define _RK_AIQ_TYPES_ACNR_ALGO_PRVT_V30_H_

#include "acnrV30/rk_aiq_types_acnr_algo_int_v30.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"

//RKAIQ_BEGIN_DECLARE



//anr context
typedef struct Acnr_Context_V30_s {
    AcnrV30_ExpInfo_t stExpInfo;
    AcnrV30_State_t eState;
    AcnrV30_OPMode_t eMode;

    Acnr_Auto_Attr_V30_t stAuto;
    Acnr_Manual_Attr_V30_t stManual;

    // struct list_head* list_cnr_v30;

    rk_aiq_cnr_strength_v30_t stStrength;

    bool isIQParaUpdate;
    bool isGrayMode;
    AcnrV30_ParamMode_t eParamMode;

    int rawWidth;
    int rawHeight;

    int prepare_type;
    CalibDbV2_CNRV30_t cnr_v30;

    int isReCalculate;
} Acnr_Context_V30_t;






//RKAIQ_END_DECLARE

#endif


