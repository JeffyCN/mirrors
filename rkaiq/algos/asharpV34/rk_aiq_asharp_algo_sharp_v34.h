/*
 * rk_aiq_asharp_algo_v34.h
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

#ifndef __RK_AIQ_ASHRP_ALGO_SHARP_V34_H__
#define __RK_AIQ_ASHRP_ALGO_SHARP_V34_H__

#define LOG2(x) (log((double)x) / log((double)2))

#include "RkAiqCalibDbTypes.h"
#include "base/xcam_log.h"
#include "math.h"
#include "rk_aiq_comm.h"
#include "rk_aiq_types_asharp_algo_prvt_v34.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

RKAIQ_BEGIN_DECLARE

Asharp_result_V34_t sharp_select_params_by_ISO_V34(void* pParams_v, void* pSelect_v,
                                                   Asharp_ExpInfo_V34_t* pExpInfo);

Asharp_result_V34_t sharp_fix_transfer_V34(void* pSelect_v, RK_SHARP_Fix_V34_t* pFix,
                                           rk_aiq_sharp_strength_v34_t* pStrength,
                                           Asharp_ExpInfo_V34_t* pExpInfo);

Asharp_result_V34_t sharp_fix_printf_V34(RK_SHARP_Fix_V34_t* pFix);

Asharp_result_V34_t sharp_get_setting_by_name_json_V34(void* pCalibdbV2_v, char* name,
                                                       int* tuning_idx);

Asharp_result_V34_t sharp_init_params_json_V34(void* pSharpParams_v, void* pCalibdbV2_v,
                                               int tuning_idx);

Asharp_result_V34_t sharp_config_setting_param_json_V34(void* pParams_v, void* pCalibdbV2_v,
                                                        char* param_mode, char* snr_name);

RKAIQ_END_DECLARE
#endif