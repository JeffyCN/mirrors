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

#ifndef __RKAIQ_AGAIN_V2_H__
#define __RKAIQ_AGAIN_V2_H__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "rk_aiq_again_algo_gain_v2.h"
#include "rk_aiq_types_again_algo_prvt_v2.h"

RKAIQ_BEGIN_DECLARE

Again_result_V2_t Again_Start_V2(Again_Context_V2_t *pAgainCtx);

Again_result_V2_t Again_Stop_V2(Again_Context_V2_t *pAgainCtx);

Again_result_V2_t Again_Init_V2(Again_Context_V2_t **ppAgainCtx, CamCalibDbV2Context_t *pCalibDbV2);

//anr release
Again_result_V2_t Again_Release_V2(Again_Context_V2_t *pAgainCtx);

//anr config
Again_result_V2_t Again_Prepare_V2(Again_Context_V2_t *pAgainCtx, Again_Config_V2_t* pANRConfig);

//anr reconfig
Again_result_V2_t Again_ReConfig_V2(Again_Context_V2_t *pAgainCtx, Again_Config_V2_t* pANRConfig);

//anr preprocess
Again_result_V2_t Again_PreProcess_V2(Again_Context_V2_t *pAgainCtx);

Again_result_V2_t Again_GainRatioProcess_V2(Again_GainState_V2_t *pGainState, Again_ExpInfo_V2_t *pExpInfo);

//anr process
Again_result_V2_t Again_Process_V2(Again_Context_V2_t *pAgainCtx, Again_ExpInfo_V2_t *pExpInfo);

//anr get result
Again_result_V2_t Again_GetProcResult_V2(Again_Context_V2_t *pAgainCtx, Again_ProcResult_V2_t* pANRResult);

Again_result_V2_t Again_ConfigSettingParam_V2(Again_Context_V2_t *pAgainCtx, Again_ParamMode_V2_t eParamMode, int snr_mode);

Again_result_V2_t Again_ParamModeProcess_V2(Again_Context_V2_t *pAgainCtx, Again_ExpInfo_V2_t *pExpInfo, Again_ParamMode_V2_t *mode);


RKAIQ_END_DECLARE

#endif
