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
#ifndef _AIQ_BTNR_HANDLE_H_
#define _AIQ_BTNR_HANDLE_H_

#include "aiq_algo_handler.h"

typedef AiqAlgoHandler_t AiqBtnrHandler_t;

AiqAlgoHandler_t* AiqAlgoHandlerBtnr_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
#if 0
XCamReturn AiqBtnrHandler_setAttrib(AiqBtnrHandler_t* pHdlBtnr, btnr_api_attrib_t* attr);
XCamReturn AiqBtnrHandler_getAttrib(AiqBtnrHandler_t* pHdlBtnr, btnr_api_attrib_t* attr);
XCamReturn AiqBtnrHandler_queryStatus(AiqBtnrHandler_t* pHdlBtnr, btnr_status_t* status);
#endif
XCamReturn AiqBtnrHandler_setStrength(AiqBtnrHandler_t* pHdlBtnr, abtnr_strength_t* strg);
XCamReturn AiqBtnrHandler_getStrength(AiqBtnrHandler_t* pHdlBtnr, abtnr_strength_t* strg);

#endif
