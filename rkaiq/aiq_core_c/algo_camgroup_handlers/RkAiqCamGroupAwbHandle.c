
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

#include "RkAiqCamGroupAwbHandle.h"
#include "awb/rk_aiq_uapiv3_awb_int.h"


XCamReturn AiqAlgoCamGroupAwbHandler_setAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_api_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_api_attrib_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setWbGainCtrlAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCtrl_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetWbGainCtrlAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbGainCtrlAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCtrl_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetWbGainCtrlAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setAwbStatsAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_Stats_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetAwbStatsAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getAwbStatsAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_Stats_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAwbStatsAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setAwbGnCalcStepAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCalcStep_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetAwbGnCalcStepAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getAwbGnCalcStepAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCalcStep_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAwbGnCalcStepAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setAwbGnCalcOthAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCalcOth_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetAwbGnCalcOthAttrib(pHdl->mAlgoCtx, att, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getAwbGnCalcOthAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCalcOth_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAwbGnCalcOthAttrib(pHdl->mAlgoCtx, att);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn AiqAlgoCamGroupAwbHandler_writeAwbIn(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_awb_wrtIn_attr_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapiV3_awb_WriteInput(pHdl->mAlgoCtx, att, false);

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn AiqAlgoCamGroupAwbHandler_awbIqMap2Main(AiqAlgoCamGroupAwbHandler_t* pAwbHdl,  rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapiV3_awb_IqMap2Main(pHdl->mAlgoCtx, att, false);

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setAwbPreWbgain(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, const float att[4])
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = rk_aiq_uapiV3_awb_SetPreWbgain(pHdl->mAlgoCtx, att, false);

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn AiqAlgoCamGroupAwbHandler_queryWBInfo(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_querry_info_t* wb_querry_info)
{
    ENTER_ANALYZER_FUNCTION();

	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV3_awb_QueryWBInfo(pHdl->mAlgoCtx, wb_querry_info);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}

XCamReturn AiqAlgoCamGroupAwbHandler_lock(AiqAlgoCamGroupAwbHandler_t* pAwbHdl)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;

    rk_aiq_uapiV3_awb_Lock(pHdl->mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_unlock(AiqAlgoCamGroupAwbHandler_t* pAwbHdl)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;

    rk_aiq_uapiV3_awb_Unlock(pHdl->mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}




XCamReturn AiqAlgoCamGroupAwbHandler_setFFWbgainAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_awb_ffwbgain_attr_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV3_awb_SetFstFrWbgain(pHdl->mAlgoCtx, att.wggain, false);
    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getAlgoStat(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_tool_awb_stat_res_full_t *awb_stat_algo)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetAlgoStat(pHdl->mAlgoCtx, awb_stat_algo);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;

}

XCamReturn AiqAlgoCamGroupAwbHandler_getStrategyResult(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_tool_awb_strategy_result_t *awb_strategy_result)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
    aiqMutex_lock(&pHdl->mCfgMutex);

    rk_aiq_uapiV3_awb_GetStrategyResult(pHdl->mAlgoCtx, awb_strategy_result);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

