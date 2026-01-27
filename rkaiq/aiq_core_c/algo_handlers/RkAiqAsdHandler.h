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
#ifndef _AIQ_ALGO_HANDLE_ASD_H_
#define _AIQ_ALGO_HANDLE_ASD_H_

#include "aiq_algo_handler.h"

#define ASD_FACE_MIN_X        0
#define ASD_FACE_MIN_Y        0
#define ASD_FACE_MAX_X        9999
#define ASD_FACE_MAX_Y        9999
#define ASD_FACE_MIN_WIDTH    10
#define ASD_FACE_MIN_HEIGHT   10
#define ASD_FACE_MAX_WIDTH    10000
#define ASD_FACE_MAX_HEIGHT   10000

RKAIQ_BEGIN_DECLARE

typedef struct AiqAlgoHandlerAsd_s {
    AiqAlgoHandler_t _base;
    uint32_t isp_acq_width;
    uint32_t isp_acq_height;
} AiqAlgoHandlerAsd_t;

AiqAlgoHandler_t* AiqAlgoHandlerAsd_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);
XCamReturn AiqAlgoHandlerAsd_setFaceInfo(AiqAlgoHandlerAsd_t* pHdlAsd, rk_aiq_face_info_t *face);
XCamReturn AiqAlgoHandlerAsd_getSelFaceInfo(AiqAlgoHandlerAsd_t* pHdlAsd, rk_aiq_face_roi_t* selface);

#if 0
XCamReturn AiqAlgoHandlerAsd_setAttrib(AiqAlgoHandlerAsd_t* pHdlAsd, asd_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerAsd_getAttrib(AiqAlgoHandlerAsd_t* pHdlAsd, asd_api_attrib_t* attr);
XCamReturn AiqAlgoHandlerAsd_queryStatus(AiqAlgoHandlerAsd_t* pHdlAsd, asd_status_t* status);
#endif


RKAIQ_END_DECLARE

#endif
