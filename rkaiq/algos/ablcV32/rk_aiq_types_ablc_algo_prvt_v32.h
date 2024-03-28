/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef _RK_AIQ_TYPES_ABLC_ALGO_PRVT_V32_H_
#define _RK_AIQ_TYPES_ABLC_ALGO_PRVT_V32_H_

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "ablcV32/rk_aiq_types_ablc_algo_int_v32.h"
#include "abayer2dnrV23/rk_aiq_types_abayer2dnr_algo_int_v23.h"
#include "xcam_common.h"
#include "xcam_log.h"

typedef struct AblcContext_V32_s {
    AblcOPMode_V32_t eMode;
    AblcParams_V32_t stBlc0Params;
    AblcParams_V32_t stBlc1Params;
    AblcOBParams_V32_t stBlcOBParams;
    AblcSelect_V32_t stBlc0Select;
    AblcSelect_V32_t stBlc1Select;
    AblcOBSelect_V32_t stBlcOBSelect;
    AblcManualAttr_V32_t stBlc0Manual;
    AblcManualAttr_V32_t stBlc1Manual;
    AblcManualOBAttr_V32_t stBlcOBManual;
    AblcRefParams_V32_t stBlcRefParams;
    // AblcProc_V32_t ProcRes;

    AblcExpInfo_V32_t stExpInfo;
    AblcState_V32_t eState;

    CalibDbV2_Blc_V32_t stBlcCalib;
#if (RKAIQ_HAVE_BAYER2DNR_V23)
    CalibDbV2_Bayer2dnrV23_Calib_t stBayer2dnrCalib;
#endif
    int prepare_type;

    int isReCalculate;
    bool isUpdateParam;

} AblcContext_V32_t;

#endif  // _RK_AIQ_TYPES_ABLC_ALGO_PRVT_V32_H_