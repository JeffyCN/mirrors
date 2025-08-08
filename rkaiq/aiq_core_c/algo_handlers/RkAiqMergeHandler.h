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
#ifndef _AIQ_ALGO_HANDLE_MERGE_H_
#define _AIQ_ALGO_HANDLE_MERGE_H_

#include "aiq_algo_handler.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqAlgoHandlerMerge_s {
    AiqAlgoHandler_t _base;
    RkAiqAlgoProcResAeShared_t mAeProcRes;
} AiqAlgoHandlerMerge_t;

AiqAlgoHandler_t* AiqAlgoHandlerMerge_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
#if 0
XCamReturn AiqAlgoHandlerMerge_setAttrib(AiqAlgoHandlerMerge_t* pHdlMerge, mge_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerMerge_getAttrib(AiqAlgoHandlerMerge_t* pHdlMerge, mge_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerMerge_queryStatus(AiqAlgoHandlerMerge_t* pHdlMerge, mge_status_t* status);
#endif
void AiqAlgoHandlerMerge_setAeProcRes(AiqAlgoHandlerMerge_t* pHdlMerge, RkAiqAlgoProcResAeShared_t* aeProcRes);

RKAIQ_END_DECLARE

#endif
