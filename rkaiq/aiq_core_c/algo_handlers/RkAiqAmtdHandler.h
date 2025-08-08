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
#ifndef _AIQ_AMTD_HANDLE_H_
#define _AIQ_AMTD_HANDLE_H_

#include "aiq_pool.h"
#include "aiq_list.h"

#include "aiq_algo_handler.h"
#include "algos/amtd/rk_aiq_uapi_amtd_int.h"

typedef struct AiqAmtdHandler_s {
    AiqAlgoHandler_t _base;
    bool mAmtdSyncAly;
    AiqPool_t* mImuDataPool;
    AiqList_t* mImuDataList;
    AiqList_t* mUsedImuList;
} AiqAmtdHandler_t;

AiqAlgoHandler_t* AiqAlgoHandlerAmtd_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);

XCamReturn
AiqAlgoHandlerAmtd_setImuData(AiqAmtdHandler_t* pAmtdHdl, AiqImuData_t* data);


#endif
