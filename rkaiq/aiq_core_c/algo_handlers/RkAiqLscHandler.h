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
#ifndef _AIQ_ALGO_HANDLE_LSC_H_
#define _AIQ_ALGO_HANDLE_LSC_H_

#include "aiq_algo_handler.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqAlgoHandlerLsc_s {
    AiqAlgoHandler_t _base;
    bool colorConstFlag;
    rk_aiq_color_info_t colorSwInfo;
} AiqAlgoHandlerLsc_t;

AiqAlgoHandler_t* AiqAlgoHandlerLsc_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
XCamReturn AiqAlgoHandlerLsc_queryalscStatus(AiqAlgoHandlerLsc_t* pHdlLsc, alsc_status_t* status);
XCamReturn AiqAlgoHandlerLsc_setCalib(AiqAlgoHandlerLsc_t* pHdlLsc, alsc_lscCalib_t* attr);
XCamReturn AiqAlgoHandlerLsc_getCalib(AiqAlgoHandlerLsc_t* pHdlLsc, alsc_lscCalib_t* attr);
XCamReturn AiqAlgoHandlerLsc_setAcolorSwInfo(AiqAlgoHandlerLsc_t* pHdlLsc, rk_aiq_color_info_t* aColor_sw_info);

RKAIQ_END_DECLARE

#endif
