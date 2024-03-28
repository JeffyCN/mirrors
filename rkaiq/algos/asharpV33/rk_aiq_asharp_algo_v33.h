/*
 * rk_aiq_asharp_algo_v33.h
 *
 *  Copyright (c) 2022 Rockchip Corporation
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

#ifndef __RKAIQ_ASHARP_SHARP_V33_H__
#define __RKAIQ_ASHARP_SHARP_V33_H__

#include "RkAiqCalibDbV2Helper.h"
#include "math.h"
#include "rk_aiq_asharp_algo_sharp_v33.h"
#include "rk_aiq_types_asharp_algo_prvt_v33.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

RKAIQ_BEGIN_DECLARE

Asharp_result_V33_t Asharp_Start_V33(Asharp_Context_V33_t* pAsharpCtx);

Asharp_result_V33_t Asharp_Stop_V33(Asharp_Context_V33_t* pAsharpCtx);

// anr inint
Asharp_result_V33_t Asharp_Init_V33(Asharp_Context_V33_t** ppAsharpCtx, void* pCalibDb);

// anr release
Asharp_result_V33_t Asharp_Release_V33(Asharp_Context_V33_t* pAsharpCtx);

// anr config
Asharp_result_V33_t Asharp_Prepare_V33(Asharp_Context_V33_t* pAsharpCtx,
                                       Asharp_Config_V33_t* pANRConfig);

// anr reconfig
Asharp_result_V33_t Asharp_ReConfig_V33(Asharp_Context_V33_t* pAsharpCtx,
                                        Asharp_Config_V33_t* pANRConfig);

Asharp_result_V33_t Asharp_IQParaUpdate_V33(Asharp_Context_V33_t* pAsharpCtx);

// anr preprocess
Asharp_result_V33_t Asharp_PreProcess_V33(Asharp_Context_V33_t* pAsharpCtx);

// anr process
Asharp_result_V33_t Asharp_Process_V33(Asharp_Context_V33_t* pAsharpCtx,
                                       Asharp_ExpInfo_V33_t* pExpInfo);

// anr get result
Asharp_result_V33_t Asharp_GetProcResult_V33(Asharp_Context_V33_t* pAsharpCtx,
                                             Asharp_ProcResult_V33_t* pANRResult);

Asharp_result_V33_t Asharp_ConfigSettingParam_V33(Asharp_Context_V33_t* pAsharpCtx,
                                                  Asharp_ParamMode_V33_t eParamMode, int snr_mode);

Asharp_result_V33_t Asharp_ParamModeProcess_V33(Asharp_Context_V33_t* pAsharpCtx,
                                                Asharp_ExpInfo_V33_t* pExpInfo,
                                                Asharp_ParamMode_V33_t* mode);

RKAIQ_END_DECLARE

#endif