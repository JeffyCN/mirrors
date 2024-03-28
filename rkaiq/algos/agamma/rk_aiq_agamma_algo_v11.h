/*
 * rk_aiq_agamma_algo_v11.h
 *
 *  Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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

#ifndef __RK_AIQ_AGAMMA_ALGO_V11_H__
#define __RK_AIQ_AGAMMA_ALGO_V11_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbV2Helper.h"
#include "agamma/rk_aiq_types_agamma_algo_prvt.h"
#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

XCamReturn AgammaInit(AgammaHandle_t** pGammaCtx, CamCalibDbV2Context_t* pCalib);
XCamReturn AgammaRelease(AgammaHandle_t* pGammaCtx);
void AgammaProcessing(AgammaHandle_t* pGammaCtx, AgammaProcRes_t* pProcRes);

RKAIQ_END_DECLARE

#endif  //__RK_AIQ_AGAMMA_ALGO_V11_H__
