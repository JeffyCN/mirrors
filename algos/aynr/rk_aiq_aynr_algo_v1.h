/*
 * rk_aiq_aynr_algo_v1.h
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

#ifndef __RKAIQ_AYNR_V1_H__
#define __RKAIQ_AYNR_V1_H__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "rk_aiq_aynr_algo_ynr_v1.h"
#include "rk_aiq_types_aynr_algo_prvt_v1.h"

RKAIQ_BEGIN_DECLARE

Aynr_Result_V1_t Aynr_Start_V1(Aynr_Context_V1_t *pAynrCtx);

Aynr_Result_V1_t Aynr_Stop_V1(Aynr_Context_V1_t *pAynrCtx);

//anr inint
Aynr_Result_V1_t Aynr_Init_V1(Aynr_Context_V1_t **ppAynrCtx, CamCalibDbContext_t *pCalibDb);

//anr release
Aynr_Result_V1_t Aynr_Release_V1(Aynr_Context_V1_t *pAynrCtx);

//anr config
Aynr_Result_V1_t Aynr_Prepare_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_Config_V1_t* pANRConfig);

//anr reconfig
Aynr_Result_V1_t Aynr_ReConfig_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_Config_V1_t* pANRConfig);

Aynr_Result_V1_t Aynr_IQParaUpdate_V1(Aynr_Context_V1_t *pAynrCtx);

//anr preprocess
Aynr_Result_V1_t Aynr_PreProcess_V1(Aynr_Context_V1_t *pAynrCtx);

//anr process
Aynr_Result_V1_t Aynr_Process_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_ExpInfo_V1_t *pExpInfo);

//anr get result
Aynr_Result_V1_t Aynr_GetProcResult_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_ProcResult_V1_t* pANRResult);

Aynr_Result_V1_t Aynr_ConfigSettingParam_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_ParamMode_V1_t eParamMode, int snr_mode);

Aynr_Result_V1_t Aynr_ParamModeProcess_V1(Aynr_Context_V1_t *pAynrCtx, Aynr_ExpInfo_V1_t *pExpInfo, Aynr_ParamMode_V1_t *mode);

Aynr_Result_V1_t Aynr_Init_Json_V1(Aynr_Context_V1_t **ppAynrCtx, CamCalibDbV2Context_t *pCalibDbV2);


RKAIQ_END_DECLARE

#endif
