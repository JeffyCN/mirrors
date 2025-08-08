/*
 * rk_aiq_auvnr_algo_v1.h
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

#ifndef __RKAIQ_AUVNR_V1_H__
#define __RKAIQ_AUVNR_V1_H__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "rk_aiq_auvnr_algo_uvnr_v1.h"
#include "rk_aiq_types_auvnr_algo_prvt_v1.h"

RKAIQ_BEGIN_DECLARE

Auvnr_result_t Auvnr_Start_V1(Auvnr_Context_V1_t *pAuvnrCtx);

Auvnr_result_t Auvnr_Stop_V1(Auvnr_Context_V1_t *pAuvnrCtx);

//anr inint
Auvnr_result_t Auvnr_Init_V1(Auvnr_Context_V1_t **ppAuvnrCtx, CamCalibDbContext_t *pCalibDb);

Auvnr_result_t Auvnr_Init_Json_V1(Auvnr_Context_V1_t **ppAuvnrCtx, CamCalibDbV2Context_t *pCalibDbV2);

//anr release
Auvnr_result_t Auvnr_Release_V1(Auvnr_Context_V1_t *pAuvnrCtx);

//anr config
Auvnr_result_t Auvnr_Prepare_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_Config_V1_t* pANRConfig);

//anr reconfig
Auvnr_result_t Auvnr_ReConfig_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_Config_V1_t* pANRConfig);

Auvnr_result_t Auvnr_IQParaUpdate_V1(Auvnr_Context_V1_t *pAuvnrCtx);

//anr preprocess
Auvnr_result_t Auvnr_PreProcess_V1(Auvnr_Context_V1_t *pAuvnrCtx);

Auvnr_result_t Auvnr_GainRatioProcess_V1(Auvnr_GainState_t *pGainState, Auvnr_ExpInfo_t *pExpInfo);

//anr process
Auvnr_result_t Auvnr_Process_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_ExpInfo_t *pExpInfo);

//anr get result
Auvnr_result_t Auvnr_GetProcResult_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_ProcResult_V1_t* pANRResult);

Auvnr_result_t Auvnr_ConfigSettingParam_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_ParamMode_t eParamMode, int snr_mode);

Auvnr_result_t Auvnr_ParamModeProcess_V1(Auvnr_Context_V1_t *pAuvnrCtx, Auvnr_ExpInfo_t *pExpInfo, Auvnr_ParamMode_t *mode);


RKAIQ_END_DECLARE

#endif
