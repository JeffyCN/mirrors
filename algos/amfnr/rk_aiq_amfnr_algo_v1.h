/*
 * rk_aiq_amfnr_algo_v1.h
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

#ifndef __RKAIQ_AMFNR_V1_H__
#define __RKAIQ_AMFNR_V1_H__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "rk_aiq_amfnr_algo_mfnr_v1.h"
#include "rk_aiq_types_amfnr_algo_prvt_v1.h"

RKAIQ_BEGIN_DECLARE

Amfnr_Result_V1_t Amfnr_Start_V1(Amfnr_Context_V1_t *pAmfnrCtx);

Amfnr_Result_V1_t Amfnr_Stop_V1(Amfnr_Context_V1_t *pAmfnrCtx);

//anr inint
Amfnr_Result_V1_t Amfnr_Init_V1(Amfnr_Context_V1_t **ppAmfnrCtx, CamCalibDbContext_t *pCalibDb);

//anr release
Amfnr_Result_V1_t Amfnr_Release_V1(Amfnr_Context_V1_t *pAmfnrCtx);

//anr config
Amfnr_Result_V1_t Amfnr_Prepare_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_Config_V1_t* pANRConfig);

//anr reconfig
Amfnr_Result_V1_t Amfnr_ReConfig_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_Config_V1_t* pANRConfig);

Amfnr_Result_V1_t Amfnr_IQParaUpdate_V1(Amfnr_Context_V1_t *pAmfnrCtx);

//anr preprocess
Amfnr_Result_V1_t Amfnr_PreProcess_V1(Amfnr_Context_V1_t *pAmfnrCtx);

//anr process
Amfnr_Result_V1_t Amfnr_Process_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_ExpInfo_V1_t *pExpInfo);

//anr get result
Amfnr_Result_V1_t Amfnr_GetProcResult_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_ProcResult_V1_t* pANRResult);

Amfnr_Result_V1_t Amfnr_ConfigSettingParam_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_ParamMode_V1_t eParamMode, int snr_mode);

Amfnr_Result_V1_t Amfnr_ParamModeProcess_V1(Amfnr_Context_V1_t *pAmfnrCtx, Amfnr_ExpInfo_V1_t *pExpInfo, Amfnr_ParamMode_V1_t *mode);

Amfnr_Result_V1_t Amfnr_Init_Json_V1(Amfnr_Context_V1_t **ppAmfnrCtx, CamCalibDbV2Context_t *pCalibDbV2);


RKAIQ_END_DECLARE

#endif
