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

#ifndef _RK_AIQ_TYPES_ADPCC_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_ADPCC_ALGO_PRVT_H_

#include "adpcc/rk_aiq_types_adpcc_algo_int.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "base/xcam_log.h"
#include "base/xcam_common.h"


#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)
#define FASTMODELEVELMAX     (10)
#define FASTMODELEVELMIN     (1)


typedef struct AdpccContext_s {
    AdpccOPMode_t eMode;
    AdpccState_t eState;

    AdpccExpInfo_t stExpInfo;

    Adpcc_Auto_Attr_t stAuto;
    Adpcc_Manual_Attr_t stManual;
    CalibDbV2_Dpcc_t stTool;
    //xml param
    CalibDbV2_Dpcc_t stDpccCalib;
    //html param
    Adpcc_html_param_t stParams;

    //sensor dpcc proc result
    Sensor_dpcc_res_t SenDpccRes;

    //pre ae result
    Adpcc_pre_ae_res_t PreAe;

    bool isBlackSensor;

    int prepare_type;
    bool isReCal_;
} AdpccContext_t;

typedef struct _RkAiqAlgoContext {
    AdpccContext_t pAdpccCtx;
} RkAiqAlgoContext;



#endif


