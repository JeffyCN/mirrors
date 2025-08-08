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
#ifndef _AIQ_ALGO_HANDLE_AF_C_H_
#define _AIQ_ALGO_HANDLE_AF_C_H_

#include "aiq_algo_handler.h"
#include "algos/af/rk_aiq_types_af_algo_int.h"

RKAIQ_BEGIN_DECLARE
#if RKAIQ_HAVE_AF
typedef struct AiqAlgoHandlerAf_s {
    AiqAlgoHandler_t _base;
    bool isUpdateZoomPosDone;
    int mLastZoomIndex;
    RkAiqAlgoProcResAf mLastAfResult;
    AiqMutex_t mAeStableMutex;
    bool mAeStable;
    uint32_t mAfMeasResSyncFalg;
    uint32_t mAfFocusResSyncFalg;
} AiqAlgoHandlerAf_t;

AiqAlgoHandler_t* AiqAlgoHandlerAf_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);

XCamReturn AiqAlgoHandlerAf_setCalib(AiqAlgoHandlerAf_t* pAfHdl, void* calib);
XCamReturn AiqAlgoHandlerAf_getCalib(AiqAlgoHandlerAf_t* pAfHdl, void* calib);
XCamReturn AiqAlgoHandlerAf_setAttrib(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_attrib_t* att);
XCamReturn AiqAlgoHandlerAf_getAttrib(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_attrib_t* att);
XCamReturn AiqAlgoHandlerAf_lock(AiqAlgoHandlerAf_t* pAfHdl);
XCamReturn AiqAlgoHandlerAf_unlock(AiqAlgoHandlerAf_t* pAfHdl);
XCamReturn AiqAlgoHandlerAf_Oneshot(AiqAlgoHandlerAf_t* pAfHdl);
XCamReturn AiqAlgoHandlerAf_ManualTriger(AiqAlgoHandlerAf_t* pAfHdl);
XCamReturn AiqAlgoHandlerAf_Tracking(AiqAlgoHandlerAf_t* pAfHdl);
XCamReturn AiqAlgoHandlerAf_setZoomIndex(AiqAlgoHandlerAf_t* pAfHdl, int index);
XCamReturn AiqAlgoHandlerAf_getZoomIndex(AiqAlgoHandlerAf_t* pAfHdl, int* index);
XCamReturn AiqAlgoHandlerAf_endZoomChg(AiqAlgoHandlerAf_t* pAfHdl);
XCamReturn AiqAlgoHandlerAf_startZoomCalib(AiqAlgoHandlerAf_t* pAfHdl);
XCamReturn AiqAlgoHandlerAf_resetZoom(AiqAlgoHandlerAf_t* pAfHdl);
XCamReturn AiqAlgoHandlerAf_GetSearchPath(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_sec_path_t* path);
XCamReturn AiqAlgoHandlerAf_GetSearchResult(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_result_t* result);
XCamReturn AiqAlgoHandlerAf_GetFocusRange(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_focusrange* range);
XCamReturn AiqAlgoHandlerAf_GetZoomRange(AiqAlgoHandlerAf_t* pAfHdl, rk_aiq_af_zoomrange* range);
XCamReturn AiqAlgoHandlerAf_setAeStable(AiqAlgoHandlerAf_t* pAfHdl, bool ae_stable);
#endif
RKAIQ_END_DECLARE

#endif
