/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
#ifndef _AIQ_ALGO_HANDLE_3DLUT_H_
#define _AIQ_ALGO_HANDLE_3DLUT_H_

#include "aiq_algo_handler.h"

RKAIQ_BEGIN_DECLARE
#if RKAIQ_HAVE_3DLUT
typedef AiqAlgoHandler_t AiqAlgoHandler3dlut_t;

AiqAlgoHandler_t* AiqAlgoHandler3dlut_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
XCamReturn AiqAlgoHandler3dlut_queryalut3dStatus(AiqAlgoHandler3dlut_t* pHdl3dlut, alut3d_status_t* status);
XCamReturn AiqAlgoHandler3dlut_setCalib(AiqAlgoHandler3dlut_t* pHdl3dlut, alut3d_lut3dCalib_t* calib);
XCamReturn AiqAlgoHandler3dlut_getCalib(AiqAlgoHandler3dlut_t* pHdl3dlut, alut3d_lut3dCalib_t* calib);
#endif
RKAIQ_END_DECLARE

#endif
