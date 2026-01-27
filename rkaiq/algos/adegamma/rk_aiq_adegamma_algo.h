/*
 * rk_aiq_adegamma_algo.h
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

#ifndef __RK_AIQ_ADEGAMMA_ALGO_H__
#define __RK_AIQ_ADEGAMMA_ALGO_H__

#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "adegamma/rk_aiq_types_adegamma_algo_prvt.h"


RKAIQ_BEGIN_DECLARE

XCamReturn AdegammaInit(AdegammaHandle_t** para, CamCalibDbV2Context_t* calib);
XCamReturn AdegammaRelease(AdegammaHandle_t* para);
XCamReturn AdegammaPreProc(AdegammaHandle_t* para);
void AdegammaProcessing(AdegammaHandle_t* para, AdegammaProcRes_t* AdegammaProcRes);


RKAIQ_END_DECLARE

#endif //_RK_AIQ_ALGO_ACPRC_ITF_H_
