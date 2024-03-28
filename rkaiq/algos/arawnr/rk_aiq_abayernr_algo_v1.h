/*
 * rk_aiq_abayernr_algo_v1.h
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

#ifndef __RKAIQ_ABAYERNR_V1_H__
#define __RKAIQ_ABAYERNR_V1_H__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "rk_aiq_abayernr_algo_bayernr_v1.h"
#include "rk_aiq_types_abayernr_algo_prvt_v1.h"

RKAIQ_BEGIN_DECLARE

Abayernr_result_v1_t Abayernr_Start_V1(Abayernr_Context_V1_t *pAbayernrCtx);

Abayernr_result_v1_t Abayernr_Stop_V1(Abayernr_Context_V1_t *pAbayernrCtx);

#if ABAYERNR_USE_JSON_FILE_V1
Abayernr_result_v1_t Abayernr_Init_Json_V1(Abayernr_Context_V1_t **ppAbayernrCtx, CamCalibDbV2Context_t *pCalibDbV2);
#else
//anr inint
Abayernr_result_v1_t Abayernr_Init_V1(Abayernr_Context_V1_t **ppAbayernrCtx, CamCalibDbContext_t *pCalibDb);
#endif

//anr release
Abayernr_result_v1_t Abayernr_Release_V1(Abayernr_Context_V1_t *pAbayernrCtx);

//anr config
Abayernr_result_v1_t Abayernr_Prepare_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_Config_V1_t* pANRConfig);

//anr reconfig
Abayernr_result_v1_t Abayernr_ReConfig_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_Config_V1_t* pANRConfig);

Abayernr_result_v1_t Abayernr_IQParaUpdate_V1(Abayernr_Context_V1_t *pAbayernrCtx);

//anr preprocess
Abayernr_result_v1_t Abayernr_PreProcess_V1(Abayernr_Context_V1_t *pAbayernrCtx);

//anr process
Abayernr_result_v1_t Abayernr_Process_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_ExpInfo_V1_t *pExpInfo);

//anr get result
Abayernr_result_v1_t Abayernr_GetProcResult_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_ProcResult_V1_t* pANRResult);

Abayernr_result_v1_t Abayernr_ConfigSettingParam_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_ParamMode_V1_t eParamMode, int snr_mode);

Abayernr_result_v1_t Abayernr_ParamModeProcess_V1(Abayernr_Context_V1_t *pAbayernrCtx, Abayernr_ExpInfo_V1_t *pExpInfo, Abayernr_ParamMode_V1_t *mode);




RKAIQ_END_DECLARE

#endif
