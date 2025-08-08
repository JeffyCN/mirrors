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
#ifndef _AIQ_ALGO_HANDLE_AE_C_H_
#define _AIQ_ALGO_HANDLE_AE_C_H_

#include "aiq_pool.h"
#include "aiq_list.h"

#include "aiq_algo_handler.h"
#include "algos/ae/rk_aiq_uapi_ae_int_types_v2.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqAlgoHandlerAe_s {
    AiqAlgoHandler_t _base;
    AiqMutex_t mLockAebyAfMutex;
    bool lockaebyaf;

    AiqMutex_t mGetAfdResMutex;
    AfdPeakRes_t mAfdRes;
#if RKAIQ_HAVE_AF
    AiqAlgoHandler_t* mAf_handle;
#endif
#if RKAIQ_HAVE_AFD_V1 || RKAIQ_HAVE_AFD_V2
    AiqAlgoHandler_t* mAfd_handle;
#endif
    AiqAlgoHandler_t* mAmerge_handle;
    AiqAlgoHandler_t* mAdrc_handle;
    AiqAlgoHandler_t* mAblc_handle;
    uint32_t mMeasSyncFlag;
    uint32_t mHistSyncFlag;
    bool useStatsApiCfg;
    bool updateStatsApiCfg;
    aeStats_cfg_t mStatsApiCfg;
    AlgoRstShared_t* mPreResShared;
} AiqAlgoHandlerAe_t;

AiqAlgoHandler_t* AiqAlgoHandlerAe_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);

#ifndef USE_NEWSTRUCT
XCamReturn AiqAlgoHandlerAe_setExpSwAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpSwAttrV2_t ExpSwAttr);
XCamReturn AiqAlgoHandlerAe_getExpSwAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpSwAttrV2_t* pExpSwAttr);
XCamReturn AiqAlgoHandlerAe_setLinExpAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_LinExpAttrV2_t LinExpAttr);
XCamReturn AiqAlgoHandlerAe_getLinExpAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_LinExpAttrV2_t* pLinExpAttr);
XCamReturn AiqAlgoHandlerAe_setHdrExpAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_HdrExpAttrV2_t HdrExpAttr);
XCamReturn AiqAlgoHandlerAe_getHdrExpAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_HdrExpAttrV2_t* pHdrExpAttr);

XCamReturn AiqAlgoHandlerAe_setLinAeRouteAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_LinAeRouteAttr_t LinAeRouteAttr);
XCamReturn AiqAlgoHandlerAe_getLinAeRouteAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_LinAeRouteAttr_t* pLinAeRouteAttr);
XCamReturn AiqAlgoHandlerAe_setHdrAeRouteAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_HdrAeRouteAttr_t HdrAeRouteAttr);
XCamReturn AiqAlgoHandlerAe_getHdrAeRouteAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr);

XCamReturn AiqAlgoHandlerAe_setIrisAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_IrisAttrV2_t IrisAttr);
XCamReturn AiqAlgoHandlerAe_getIrisAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_IrisAttrV2_t* pIrisAttr);
XCamReturn AiqAlgoHandlerAe_setSyncTestAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_AecSyncTest_t SyncTestAttr);
XCamReturn AiqAlgoHandlerAe_getSyncTestAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_AecSyncTest_t* pSyncTestAttr);
XCamReturn AiqAlgoHandlerAe_queryExpInfo(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpQueryInfo_t* pExpQueryInfo);

#else
XCamReturn AiqAlgoHandlerAe_setExpSwAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_expSwAttr_t ExpSwAttr);
XCamReturn AiqAlgoHandlerAe_getExpSwAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_expSwAttr_t* pExpSwAttr);
XCamReturn AiqAlgoHandlerAe_setLinExpAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_linExpAttr_t LinExpAttr);
XCamReturn AiqAlgoHandlerAe_getLinExpAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_linExpAttr_t* pLinExpAttr);
XCamReturn AiqAlgoHandlerAe_setHdrExpAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_hdrExpAttr_t HdrExpAttr);
XCamReturn AiqAlgoHandlerAe_getHdrExpAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_hdrExpAttr_t* pHdrExpAttr);
XCamReturn AiqAlgoHandlerAe_setIrisAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_irisAttr_t IrisAttr);
XCamReturn AiqAlgoHandlerAe_getIrisAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_irisAttr_t* pIrisAttr);
XCamReturn AiqAlgoHandlerAe_setSyncTestAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_syncTestAttr_t SyncTestAttr);
XCamReturn AiqAlgoHandlerAe_getSyncTestAttr(AiqAlgoHandlerAe_t* pAeHdl, ae_api_syncTestAttr_t* pSyncTestAttr);
XCamReturn AiqAlgoHandlerAe_queryExpInfo(AiqAlgoHandlerAe_t* pAeHdl, ae_api_queryInfo_t* pExpQueryInfo);

XCamReturn AiqAlgoHandlerAe_getRkAeStats(AiqAlgoHandlerAe_t* pAeHdl, Uapi_RkAeStats_t* pRkAeStats);
#endif

XCamReturn AiqAlgoHandlerAe_setLockAeForAf(AiqAlgoHandlerAe_t* pAeHdl, bool lock_ae);
XCamReturn AiqAlgoHandlerAe_getAfdResForAE(AiqAlgoHandlerAe_t* pAeHdl, AfdPeakRes_t AfdRes);
XCamReturn AiqAlgoHandlerAe_setExpWinAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpWin_t ExpWinAttr);
XCamReturn AiqAlgoHandlerAe_getExpWinAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpWin_t* pExpWinAttr);
XCamReturn AiqAlgoHandlerAe_setExpSubWinAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpSubWin_t ExpSubWinAttr);
XCamReturn AiqAlgoHandlerAe_getExpSubWinAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_ExpSubWin_t* pExpSubWinAttr);

XCamReturn AiqAlgoHandlerAe_setFrameHdrAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_FrameHdrAttr_t FrameHdrAttr);
XCamReturn AiqAlgoHandlerAe_getFrameHdrAttr(AiqAlgoHandlerAe_t* pAeHdl, Uapi_FrameHdrAttr_t* pFrameHdrAttr);

XCamReturn AiqAlgoHandlerAe_setAecStatsCfg(AiqAlgoHandlerAe_t* pAeHdl, Uapi_AecStatsCfg_t AecStatsCfg);
XCamReturn AiqAlgoHandlerAe_getAecStatsCfg(AiqAlgoHandlerAe_t* pAeHdl, Uapi_AecStatsCfg_t* pAecStatsCfg);
XCamReturn AiqAlgoHandlerAe_setAOVForAE(AiqAlgoHandlerAe_t* pAeHdl, bool en);
XCamReturn AiqAlgoHandlerAe_setStatsApiCfg(AiqAlgoHandlerAe_t* pAeHdl, rk_aiq_op_mode_t mode, aeStats_cfg_t *cfg);

RKAIQ_END_DECLARE

#endif
