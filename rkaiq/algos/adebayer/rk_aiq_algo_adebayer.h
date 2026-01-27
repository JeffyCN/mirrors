/*
* rk_aiq_algo_adebayer.h

* for rockchip v2.0.0
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
/* for rockchip v2.0.0*/

#ifndef __RK_AIQ_ALGO_ADEBAYER_H__
#define __RK_AIQ_ALGO_ADEBAYER_H__

/**
 * @file rk_aiq_algo_adebayer.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup ADEBAYER Auto debayer Module
 * @{
 *
 */

#include "rk_aiq_types_algo_adebayer_prvt.h"

XCamReturn AdebayerInit(AdebayerContext_t *ppAdebayerCtx, CamCalibDbContext_t *pCalibDb, CamCalibDbV2Context_t *pCalibDbV2);

XCamReturn AdebayerRelease(AdebayerContext_t *pAdebayerCtx);

XCamReturn AdebayerStart(AdebayerContext_t *pAdebayerCtx);

XCamReturn AdebayerStop(AdebayerContext_t *pAdebayerCtx);

XCamReturn AdebayerConfig(AdebayerContext_t *pAdebayerCtx, AdebayerConfig_t* pAdebayerConfig);

XCamReturn AdebayerReConfig(AdebayerContext_t *pAdebayerCtx, AdebayerConfig_t* pAdebayerConfig);

XCamReturn AdebayerPreProcess(AdebayerContext_t *pAdebayerCtx);

XCamReturn AdebayerProcess(AdebayerContext_t *pAdebayerCtx, int ISO);

XCamReturn AdebayerGetProcResult(AdebayerContext_t *pAdebayerCtx, AdebayerProcResult_t* pAdebayerResult);

#endif//__RK_AIQ_ALGO_ADEBAYER_H__