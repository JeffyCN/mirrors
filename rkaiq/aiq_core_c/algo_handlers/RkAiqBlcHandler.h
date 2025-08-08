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
#ifndef _AIQ_ALGO_HANDLE_BLC_H_
#define _AIQ_ALGO_HANDLE_BLC_H_

#include "aiq_algo_handler.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqAlgoHandlerBlc_s {
    AiqAlgoHandler_t _base;
    blc_param_t* mLatestparam;
    bool mLatesten;
    RkAiqAlgoProcResAeShared_t mAeProcRes;
    bool damping;
    bool aeIsConverged;
} AiqAlgoHandlerBlc_t;

AiqAlgoHandler_t* AiqAlgoHandlerBlc_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
#if 0
XCamReturn AiqAlgoHandlerBlc_setAttrib(AiqAlgoHandlerBlc_t* pHdlBlc, blc_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerBlc_getAttrib(AiqAlgoHandlerBlc_t* pHdlBlc, blc_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerBlc_queryStatus(AiqAlgoHandlerBlc_t* pHdlBlc, blc_status_t* status);
#endif

#define AiqBlcHandler_setAeProcRes(pHdBlc, pAeProcRes) \
    (pHdBlc)->mAeProcRes = *pAeProcRes

RKAIQ_END_DECLARE

#endif
