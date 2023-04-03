/*
 * rk_aiq_a3dlut_algo.h
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

#ifndef _RK_AIQ_A3DLUT_ALGO_H_
#define _RK_AIQ_A3DLUT_ALGO_H_

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "a3dlut/rk_aiq_types_a3dlut_algo_prvt.h"

RKAIQ_BEGIN_DECLARE

//XCamReturn Alut3dInit(alut3d_handle_t *hAlut3d, const CamCalibDbContext_t* calib);
XCamReturn Alut3dInit(alut3d_handle_t *hAlut3d, const CamCalibDbV2Context_t* calibv2);
XCamReturn Alut3dRelease(alut3d_handle_t hAlut3d);
XCamReturn Alut3dPrepare(alut3d_handle_t hAlut3d);
XCamReturn Alut3dConfig(alut3d_handle_t hAlut3d);
XCamReturn Alut3dPreProc(alut3d_handle_t hAlut3d);
XCamReturn Alut3dProcessing(alut3d_handle_t hAlut3d);




RKAIQ_END_DECLARE

#endif

