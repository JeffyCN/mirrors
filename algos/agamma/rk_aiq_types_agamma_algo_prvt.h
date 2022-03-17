/*
 *rk_aiq_types_agamma_algo_prvt.h
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

#ifndef _RK_AIQ_TYPES_AGAMMA_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_AGAMMA_ALGO_PRVT_H_

#include "agamma/rk_aiq_types_agamma_algo_int.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"

#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)

typedef struct AgammaHandle_s {
    rk_aiq_gamma_cfg_t  agamma_config;
    CalibDbV2_gamma_t *pCalibDb;
    rk_aiq_gamma_attr_t agammaAttr;
    AgammaProcRes_t ProcRes;
    int working_mode;
    int prepare_type;
    int HWversion;
} AgammaHandle_t;

#endif

