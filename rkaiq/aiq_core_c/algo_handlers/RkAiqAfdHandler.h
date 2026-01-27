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
#ifndef _AIQ_AFD_HANDLE_H_
#define _AIQ_AFD_HANDLE_H_

#include "aiq_algo_handler.h"

typedef struct AiqAfdHandler_s {
    AiqAlgoHandler_t _base;
    RkAiqAlgoProcResAeShared_t mAeProcRes;
    bool mAfdSyncAly;
} AiqAfdHandler_t;

AiqAlgoHandler_t* AiqAlgoHandlerAfd_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);

XCamReturn AiqAfdHandler_getScaleRatio(AiqAfdHandler_t* pHdlAfd, int* scale_ratio);
XCamReturn AiqAfdHandler_getAfdEn(AiqAfdHandler_t* pHdlAfd, bool* en);

#define AiqAfdHandler_setAeProcRes(pHdlAfd, pAeProcRes) \
    (pHdlAfd)->mAeProcRes = *pAeProcRes

#endif
