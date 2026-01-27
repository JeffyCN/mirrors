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
#ifndef _AIQ_DRC_HANDLE_H_
#define _AIQ_DRC_HANDLE_H_

#include "aiq_algo_handler.h"

typedef struct AiqDrcHandler_s {
    AiqAlgoHandler_t _base;
    RkAiqAlgoProcResAeShared_t mAeProcRes;
    bool damping;
} AiqDrcHandler_t;

AiqAlgoHandler_t* AiqAlgoHandlerDrc_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
#if 0
XCamReturn AiqDrcHandler_setAttrib(AiqDrcHandler_t* pHdlDrc, drc_api_attrib_t* attr);
XCamReturn AiqDrcHandler_getAttrib(AiqDrcHandler_t* pHdlDrc, drc_api_attrib_t* attr);
#endif
XCamReturn AiqDrcHandler_queryStatus(AiqDrcHandler_t* pHdlDrc, drc_status_t* status);
XCamReturn AiqDrcHandler_setStrength(AiqDrcHandler_t* pHdlDrc, adrc_strength_t* ctrl);
XCamReturn AiqDrcHandler_getStrength(AiqDrcHandler_t* pHdlDrc, adrc_strength_t* ctrl);
XCamReturn AiqDrcHandler_queryTransStatus(AiqDrcHandler_t* pHdlDrc, trans_status_t* status);

#define AiqDrcHandler_setAeProcRes(pHdlDrc, pAeProcRes) \
    (pHdlDrc)->mAeProcRes = *pAeProcRes

#endif
