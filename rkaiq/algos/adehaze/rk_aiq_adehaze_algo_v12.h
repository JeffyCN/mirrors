/*
 * rk_aiq_adehaze_algo.h
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

#ifndef __RK_AIQ_ADEHAZE_ALGO_V12_H__
#define __RK_AIQ_ADEHAZE_ALGO_V12_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbV2Helper.h"
#include "adehaze/rk_aiq_types_adehaze_algo_prvt.h"
#ifdef RKAIQ_ENABLE_CAMGROUP
#include "rk_aiq_algo_camgroup_types.h"
#endif
#include "rk_aiq_comm.h"
#include "rk_aiq_types_adehaze_stat.h"

RKAIQ_BEGIN_DECLARE

XCamReturn AdehazeGetCurrData(AdehazeHandle_t* pAdehazeCtx, RkAiqAlgoProcAdhaz* procPara);
#ifdef RKAIQ_ENABLE_CAMGROUP
XCamReturn AdehazeGetCurrDataGroup(AdehazeHandle_t* pAdehazeCtx,
                                   rk_aiq_singlecam_3a_result_t* pCamgrpParams);
#endif
XCamReturn AdehazeInit(AdehazeHandle_t** para, CamCalibDbV2Context_t* calib);
XCamReturn AdehazeRelease(AdehazeHandle_t* para);
XCamReturn AdehazeProcess(AdehazeHandle_t* pAdehazeCtx, dehaze_stats_v12_t* pStats, bool stats_true,
                          RkAiqAdehazeProcResult_t* pAdehzeProcRes);
bool AdehazeByPassProcessing(AdehazeHandle_t* pAdehazeCtx);
bool DehazeEnableSetting(AdehazeHandle_t* pAdehazeCtx, RkAiqAdehazeProcResult_t* pAdehzeProcRes);

RKAIQ_END_DECLARE

#endif  //__RK_AIQ_ADEHAZE_ALGO_V12_H__
