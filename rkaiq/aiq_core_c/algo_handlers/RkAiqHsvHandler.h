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
#ifndef _AIQ_HSV_HANDLE_H_
#define _AIQ_HSV_HANDLE_H_

#include "aiq_algo_handler.h"
#if RKAIQ_HAVE_HSV
typedef AiqAlgoHandler_t AiqHsvHandler_t;

AiqAlgoHandler_t* AiqAlgoHandlerHsv_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
XCamReturn AiqHsvHandler_queryahsvStatus(AiqHsvHandler_t* pHdlHsv, ahsv_status_t* status);
XCamReturn AiqHsvHandler_setCalib(AiqHsvHandler_t* pHdlHsv, ahsv_hsvCalib_t* calib);
XCamReturn AiqHsvHandler_getCalib(AiqHsvHandler_t* pHdlHsv, ahsv_hsvCalib_t* calib);
XCamReturn AiqHsvHandler_setSatStrth(AiqHsvHandler_t* pHdlHsv, ahsv_satStrg_t* strg);
XCamReturn AiqHsvHandler_getSatStrth(AiqHsvHandler_t* pHdlHsv, ahsv_satStrg_t* strg);
XCamReturn AiqHsvHandler_setHueOffset(AiqHsvHandler_t* pHdlHsv, ahsv_hueOffset_t* offset);
XCamReturn AiqHsvHandler_getHueOffset(AiqHsvHandler_t* pHdlHsv, ahsv_hueOffset_t* offset);
XCamReturn AiqHsvHandler_setValOffset(AiqHsvHandler_t* pHdlHsv, ahsv_valOffset_t* offset);
XCamReturn AiqHsvHandler_getValOffset(AiqHsvHandler_t* pHdlHsv, ahsv_valOffset_t* offset);

#endif
#endif