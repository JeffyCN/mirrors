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

#ifndef _AIQ_ALGO_CAMGROUP_AE_HANDLE_H_
#define _AIQ_ALGO_CAMGROUP_AE_HANDLE_H_

#include "aiq_algo_camgroup_handler.h"
#include "ae/rk_aiq_uapi_ae_int.h"

RKAIQ_BEGIN_DECLARE

typedef AiqAlgoCamGroupHandler_t AiqAlgoCamGroupAeHandler_t;

#ifndef USE_NEWSTRUCT
XCamReturn AiqAlgoCamGroupAeHandler_setExpSwAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_ExpSwAttrV2_t ExpSwAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getExpSwAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_ExpSwAttrV2_t* pExpSwAttr);
XCamReturn AiqAlgoCamGroupAeHandler_setLinExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_LinExpAttrV2_t LinExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getLinExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_LinExpAttrV2_t* pLinExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_setHdrExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_HdrExpAttrV2_t HdrExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getHdrExpAttr (AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_HdrExpAttrV2_t* pHdrExpAttr);

XCamReturn AiqAlgoCamGroupAeHandler_setLinAeRouteAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_LinAeRouteAttr_t LinAeRouteAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getLinAeRouteAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_LinAeRouteAttr_t* pLinAeRouteAttr);
XCamReturn AiqAlgoCamGroupAeHandler_setHdrAeRouteAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_HdrAeRouteAttr_t HdrAeRouteAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getHdrAeRouteAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr);

XCamReturn AiqAlgoCamGroupAeHandler_setSyncTestAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_AecSyncTest_t SyncTestAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getSyncTestAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_AecSyncTest_t* pSyncTestAttr);
XCamReturn AiqAlgoCamGroupAeHandler_queryExpInfo(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_ExpQueryInfo_t* pExpQueryInfo);

#else
XCamReturn AiqAlgoCamGroupAeHandler_setExpSwAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_expSwAttr_t ExpSwAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getExpSwAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_expSwAttr_t* pExpSwAttr);
XCamReturn AiqAlgoCamGroupAeHandler_setFaceExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_faceExpAttr_t FaceExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getFaceExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_faceExpAttr_t* pFaceExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_setLinExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_linExpAttr_t LinExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getLinExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_linExpAttr_t* pLinExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_setHdrExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_hdrExpAttr_t HdrExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getHdrExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_hdrExpAttr_t* pHdrExpAttr);
XCamReturn AiqAlgoCamGroupAeHandler_setSyncTestAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_syncTestAttr_t SyncTestAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getSyncTestAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_syncTestAttr_t* pSyncTestAttr);
XCamReturn AiqAlgoCamGroupAeHandler_queryExpInfo(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_queryInfo_t* pExpQueryInfo);

XCamReturn AiqAlgoCamGroupAeHandler_getRkAeStats(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_RkAeStats_t* pRkAeStats);
#endif

XCamReturn AiqAlgoCamGroupAeHandler_setAecStatsCfg(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_AecStatsCfg_t AecStatsCfg);
XCamReturn AiqAlgoCamGroupAeHandler_getAecStatsCfg(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_AecStatsCfg_t* pAecStatsCfg);
XCamReturn AiqAlgoCamGroupAeHandler_setFrameHdrAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_FrameHdrAttr_t FrameHdrAttr);
XCamReturn AiqAlgoCamGroupAeHandler_getFrameHdrAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_FrameHdrAttr_t* pFrameHdrAttr);

RKAIQ_END_DECLARE

#endif
