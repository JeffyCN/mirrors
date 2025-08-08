
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

#include "RkAiqCamGroupAeHandle.h"

#ifndef USE_NEWSTRUCT
XCamReturn AiqAlgoCamGroupAeHandler_setExpSwAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_ExpSwAttrV2_t ExpSwAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_setExpSwAttr(pHdl->mAlgoCtx, &ExpSwAttr, true, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getExpSwAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_ExpSwAttrV2_t* pExpSwAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_getExpSwAttr(pHdl->mAlgoCtx, pExpSwAttr, true);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setLinExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_LinExpAttrV2_t LinExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_setLinExpAttr(pHdl->mAlgoCtx, &LinExpAttr, true, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getLinExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_LinExpAttrV2_t* pLinExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_getLinExpAttr(pHdl->mAlgoCtx, pLinExpAttr, true);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setHdrExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_HdrExpAttrV2_t HdrExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_setHdrExpAttr(pHdl->mAlgoCtx, &HdrExpAttr, true, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getHdrExpAttr (AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_HdrExpAttrV2_t* pHdrExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_getHdrExpAttr(pHdl->mAlgoCtx, pHdrExpAttr, true);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setLinAeRouteAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_LinAeRouteAttr_t LinAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_setLinAeRouteAttr(pHdl->mAlgoCtx, &LinAeRouteAttr, true, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getLinAeRouteAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_LinAeRouteAttr_t* pLinAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_getLinAeRouteAttr(pHdl->mAlgoCtx, pLinAeRouteAttr, true);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setHdrAeRouteAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_HdrAeRouteAttr_t HdrAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_setHdrAeRouteAttr(pHdl->mAlgoCtx, &HdrAeRouteAttr, true, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getHdrAeRouteAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_getHdrAeRouteAttr(pHdl->mAlgoCtx, pHdrAeRouteAttr, true);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setSyncTestAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_AecSyncTest_t SyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_setSyncTest(pHdl->mAlgoCtx, &SyncTestAttr, true, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getSyncTestAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_AecSyncTest_t* pSyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_getSyncTest(pHdl->mAlgoCtx, pSyncTestAttr, true);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_queryExpInfo(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_ExpQueryInfo_t* pExpQueryInfo)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_queryExpInfo(pHdl->mAlgoCtx, pExpQueryInfo, true);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#else
XCamReturn AiqAlgoCamGroupAeHandler_setExpSwAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_expSwAttr_t ExpSwAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
        ret = rk_aiq_uapi_ae_setExpSwAttr(pHdl->mAlgoCtx, &ExpSwAttr, true);
        aiqMutex_unlock(&pHdl->mCfgMutex);
        EXIT_ANALYZER_FUNCTION();
        return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getExpSwAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_expSwAttr_t* pExpSwAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
        ret = rk_aiq_uapi_ae_getExpSwAttr(pHdl->mAlgoCtx, pExpSwAttr, true);
        aiqMutex_unlock(&pHdl->mCfgMutex);
        EXIT_ANALYZER_FUNCTION();
        return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setLinExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_linExpAttr_t LinExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_setLinExpAttr(pHdl->mAlgoCtx, &LinExpAttr, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getLinExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_linExpAttr_t* pLinExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_getLinExpAttr(pHdl->mAlgoCtx, pLinExpAttr, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setHdrExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_hdrExpAttr_t HdrExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_setHdrExpAttr(pHdl->mAlgoCtx, &HdrExpAttr, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getHdrExpAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_hdrExpAttr_t* pHdrExpAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_getHdrExpAttr(pHdl->mAlgoCtx, pHdrExpAttr, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setSyncTestAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_syncTestAttr_t SyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_setSyncTest(pHdl->mAlgoCtx, &SyncTestAttr, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getSyncTestAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_syncTestAttr_t* pSyncTestAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_getSyncTest(pHdl->mAlgoCtx, pSyncTestAttr, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_queryExpInfo(AiqAlgoCamGroupAeHandler_t* pAeHdl, ae_api_queryInfo_t* pExpQueryInfo)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_queryExpInfo(pHdl->mAlgoCtx, pExpQueryInfo, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getRkAeStats(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_RkAeStats_t* pRkAeStats)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapi_ae_getRKAeStats(pHdl->mAlgoCtx, pRkAeStats, true);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn AiqAlgoCamGroupAeHandler_setAecStatsCfg(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_AecStatsCfg_t AecStatsCfg)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_setAecStatsCfg(pHdl->mAlgoCtx, &AecStatsCfg, true, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getAecStatsCfg(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_AecStatsCfg_t* pAecStatsCfg)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_getAecStatsCfg(pHdl->mAlgoCtx, pAecStatsCfg, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_setFrameHdrAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_FrameHdrAttr_t FrameHdrAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_setFrameHdrAttr(pHdl->mAlgoCtx, &FrameHdrAttr, true, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAeHandler_getFrameHdrAttr(AiqAlgoCamGroupAeHandler_t* pAeHdl, Uapi_FrameHdrAttr_t* pFrameHdrAttr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAeHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapi_ae_getFrameHdrAttr(pHdl->mAlgoCtx, pFrameHdrAttr, true);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

