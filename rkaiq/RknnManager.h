/*
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

#ifndef _RKNN_MANAGER_H_
#define _RKNN_MANAGER_H_

#include "hwi_c/aiq_CamHwBase.h"

RKAIQ_BEGIN_DECLARE

typedef struct RknnManager_s RknnManager_t;
typedef struct RknnManager_s {
    AiqCamHwBase_t* pCamHw;
    CamCalibDbV2Context_t* pCalibDbV2;

} RknnManager_t;

XCamReturn RknnManager_init(RknnManager_t* pRknnManager);
XCamReturn RknnManager_deinit(RknnManager_t* pRknnManagerr);
XCamReturn RknnManager_prepare(RknnManager_t* pRknnManager, AiqCamHwBase_t* pCamHw, CamCalibDbV2Context_t* pCalibDbV2);
XCamReturn RknnManager_start(RknnManager_t* pRknnManager);
XCamReturn RknnManager_stop(RknnManager_t* pRknnManager);
XCamReturn RknnManager_hdlEvent(RknnManager_t* pRknnManager, AiqHwAinnEvt_t *event);

RKAIQ_END_DECLARE

#endif
