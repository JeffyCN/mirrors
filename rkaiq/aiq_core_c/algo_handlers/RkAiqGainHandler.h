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
#ifndef _AIQ_ALGO_HANDLE_GAIN_H_
#define _AIQ_ALGO_HANDLE_GAIN_H_

#include "aiq_algo_handler.h"

RKAIQ_BEGIN_DECLARE

typedef AiqAlgoHandler_t AiqAlgoHandlerGain_t;

AiqAlgoHandler_t* AiqAlgoHandlerGain_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
#if 0
XCamReturn AiqAlgoHandlerGain_setAttrib(AiqAlgoHandlerGain_t* pHdlGain, gain_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerGain_getAttrib(AiqAlgoHandlerGain_t* pHdlGain, gain_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerGain_queryStatus(AiqAlgoHandlerGain_t* pHdlGain, gain_status_t* status);
#endif
RKAIQ_END_DECLARE

#endif
