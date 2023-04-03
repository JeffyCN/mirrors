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

#ifndef _RK_AIQ_TYPES_ABLC_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_ABLC_ALGO_PRVT_H_

#include "ablc/rk_aiq_types_ablc_algo_int.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "xcam_log.h"
#include "xcam_common.h"


typedef struct AblcContext_s {
    AblcOPMode_t eMode;
    AblcParams_t stBlc0Params;
    AblcParams_t stBlc1Params;
    AblcSelect_t stBlc0Select;
    AblcSelect_t stBlc1Select;
    AblcManualAttr_t stBlc0Manual;
    AblcManualAttr_t stBlc1Manual;
    AblcProc_t ProcRes;

    AblcExpInfo_t stExpInfo;
    AblcState_t eState;

    CalibDbV2_Ablc_t stBlcCalib;
    int prepare_type;

    int isReCalculate;
    bool isUpdateParam;

} AblcContext_t;



#endif


