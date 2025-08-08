/*
 * rk_aiq_again_algo.h
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

#ifndef __RKAIQ_AGAIN_V1_H__
#define __RKAIQ_AGAIN_V1_H__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "rk_aiq_again_algo_gain.h"
#include "rk_aiq_types_again_algo_prvt.h"

RKAIQ_BEGIN_DECLARE

Again_result_t Again_Start_V1(Again_Context_V1_t *pAgainCtx);

Again_result_t Again_Stop_V1(Again_Context_V1_t *pAgainCtx);

//anr inint
Again_result_t Again_Init_V1(Again_Context_V1_t **ppAgainCtx, CamCalibDbContext_t *pCalibDb);

Again_result_t Again_Init_Json_V1(Again_Context_V1_t **ppAgainCtx, CamCalibDbV2Context_t *pCalibDbV2);

//anr release
Again_result_t Again_Release_V1(Again_Context_V1_t *pAgainCtx);

//anr config
Again_result_t Again_Prepare_V1(Again_Context_V1_t *pAgainCtx, Again_Config_V1_t* pANRConfig);

//anr reconfig
Again_result_t Again_ReConfig_V1(Again_Context_V1_t *pAgainCtx, Again_Config_V1_t* pANRConfig);

//anr preprocess
Again_result_t Again_PreProcess_V1(Again_Context_V1_t *pAgainCtx);

Again_result_t Again_GainRatioProcess_V1(Again_GainState_t *pGainState, Again_ExpInfo_t *pExpInfo);

//anr process
Again_result_t Again_Process_V1(Again_Context_V1_t *pAgainCtx, Again_ExpInfo_t *pExpInfo);

//anr get result
Again_result_t Again_GetProcResult_V1(Again_Context_V1_t *pAgainCtx, Again_ProcResult_V1_t* pANRResult);



RKAIQ_END_DECLARE

#endif
