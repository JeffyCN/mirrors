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
#ifndef _AIQ_ALGO_HANDLE_IE_H_
#define _AIQ_ALGO_HANDLE_IE_H_

#include "aiq_algo_handler.h"

RKAIQ_BEGIN_DECLARE

typedef AiqAlgoHandler_t AiqAlgoHandlerIe_t;

AiqAlgoHandler_t* AiqAlgoHandlerIe_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
#if 0
XCamReturn AiqAlgoHandlerIe_setAttrib(AiqAlgoHandlerIe_t* pHdlIe, ie_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerIe_getAttrib(AiqAlgoHandlerIe_t* pHdlIe, ie_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerIe_queryStatus(AiqAlgoHandlerIe_t* pHdlIe, ie_status_t* status);
#endif
RKAIQ_END_DECLARE

#endif
