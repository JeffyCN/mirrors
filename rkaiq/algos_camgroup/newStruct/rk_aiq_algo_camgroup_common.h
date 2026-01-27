/*
 * rk_aiq_algo_camgroup_common.h
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

#include "rk_aiq_algo_camgroup_types.h"

#ifndef __RK_AIQ_ALGO_CAMGROUP_COMMON__ 
#define __RK_AIQ_ALGO_CAMGROUP_COMMON__ 

RKAIQ_BEGIN_DECLARE

void algo_camgroup_update_results(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, void** pGpRstArray,int itemSize);

XCamReturn algo_camgroup_CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg);
XCamReturn algo_camgroup_DestroyCtx(RkAiqAlgoContext *context);
XCamReturn algo_camgroup_Prepare(RkAiqAlgoCom* params);

RKAIQ_END_DECLARE

#endif
