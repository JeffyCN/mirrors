/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef _RKAIQ_ABLC_V32_H_
#define _RKAIQ_ABLC_V32_H_

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "ablcV32/rk_aiq_types_ablc_algo_prvt_v32.h"
#include "base/xcam_log.h"
#include "math.h"
#include "rk_aiq_comm.h"
#include "stdio.h"
#include "string.h"

AblcResult_V32_t AblcV32Init(AblcContext_V32_t** ppAblcCtx, CamCalibDbV2Context_t* pCalibDb);
AblcResult_V32_t AblcV32Release(AblcContext_V32_t* pAblcCtx);
AblcResult_V32_t AblcV32Process(AblcContext_V32_t* pAblcCtx, AblcExpInfo_V32_t* pExpInfo);
AblcResult_V32_t AblcV32ParamsUpdate(AblcContext_V32_t* pAblcCtx, CalibDbV2_Blc_V32_t* pCalibDb);
AblcResult_V32_t AblcV32_IQParams_Check(AblcParams_V32_t* pBLC0Params, AblcOBParams_V32_t* pBLCOBParams,AblcRefParams_V32_t* pBlcRef);
AblcResult_V32_t Ablc_GetProcResult_V32(AblcContext_V32_t* pAblcCtx, AblcProc_V32_t* pAblcResult);
#endif
