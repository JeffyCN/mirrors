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
#ifndef _AIQ_GIC_HANDLE_H_
#define _AIQ_GIC_HANDLE_H_

#include "aiq_algo_handler.h"

typedef AiqAlgoHandler_t AiqGicHandler_t;

AiqAlgoHandler_t* AiqAlgoHandlerGic_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
#if 0
XCamReturn AiqGicHandler_setAttrib(AiqGicHandler_t* pHdlGic, gic_api_attrib_t* attr);
XCamReturn AiqGicHandler_getAttrib(AiqGicHandler_t* pHdlGic, gic_api_attrib_t* attr);
XCamReturn AiqGicHandler_queryStatus(AiqGicHandler_t* pHdlGic, gic_status_t* status);
#endif
#endif
