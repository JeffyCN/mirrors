/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
/**
 * @file ahdr.cpp
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include "math.h"
#include "rk_aiq_types_atmo_algo_int.h"
#include "rk_aiq_types_atmo_algo_prvt.h"
#include "xcam_log.h"

/******************************************************************************
 * AtmoStart()
 *****************************************************************************/
RESULT AtmoStart
(
    AtmoHandle_t pAtmoCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAtmoCtx == NULL) {
        return (ATMO_RET_WRONG_HANDLE);
    }

    if ((ATMO_STATE_RUNNING == pAtmoCtx->state)
            || (ATMO_STATE_LOCKED == pAtmoCtx->state)) {
        return (ATMO_RET_WRONG_STATE);
    }

    pAtmoCtx->state = ATMO_STATE_RUNNING;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
    return (ATMO_RET_SUCCESS);
}
/******************************************************************************
 * AtmoStop()
 *****************************************************************************/
RESULT AtmoStop
(
    AtmoHandle_t pAtmoCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAtmoCtx == NULL) {
        return (ATMO_RET_WRONG_HANDLE);
    }

    // before stopping, unlock the Atmo if locked
    if (ATMO_STATE_LOCKED == pAtmoCtx->state) {
        return (ATMO_RET_WRONG_STATE);
    }

    pAtmoCtx->state = ATMO_STATE_STOPPED;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);

    return (ATMO_RET_SUCCESS);
}
/******************************************************************************
 * TmoGetCurrPara()
 *****************************************************************************/
float TmoGetCurrPara
(
    float           inPara,
    float*         inMatrixX,
    float*         inMatrixY,
    int Max_Knots
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);
    float x1 = 0.0f;
    float x2 = 0.0f;
    float value1 = 0.0f;
    float value2 = 0.0f;
    float outPara = 0.0f;

    if(inPara < inMatrixX[0])
        outPara = inMatrixY[0];
    else if (inPara >= inMatrixX[Max_Knots - 1])
        outPara = inMatrixY[Max_Knots - 1];
    else
        for(int i = 0; i < Max_Knots - 1; i++)
        {
            if(inPara >= inMatrixX[i] && inPara < inMatrixX[i + 1])
            {
                x1 = inMatrixX[i];
                x2 = inMatrixX[i + 1];
                value1 = inMatrixY[i];
                value2 = inMatrixY[i + 1];
                outPara = value1 + (inPara - x1) * (value1 - value2) / (x1 - x2);
                break;
            }
            else
                continue;
        }

    return outPara;
    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}
/******************************************************************************
 * AtmoConfig()
 *set default AtmoConfig data
 *****************************************************************************/
void AtmoConfig
(
    AtmoHandle_t           pAtmoCtx
) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAtmoCtx != NULL);

    //config default PrevData data
    pAtmoCtx->PrevData.CtrlData.FrameCnt = 0;
    pAtmoCtx->PrevData.ro_hdrtmo_lgmean = 20000;
    pAtmoCtx->PrevData.HandleData.GlobeLuma = 0.18;
    pAtmoCtx->PrevData.HandleData.GlobeMaxLuma = 0.3;
    pAtmoCtx->PrevData.HandleData.DetailsHighLight = 0.5;
    pAtmoCtx->PrevData.HandleData.DetailsLowLight = 1;
    pAtmoCtx->PrevData.HandleData.LocalTmoStrength = 0.3;
    pAtmoCtx->PrevData.HandleData.GlobalTmoStrength = 0.5;

    //set default ctrl info
    pAtmoCtx->tmoAttr.CtlInfo.GlobalLumaMode = GLOBALLUMAMODE_ENVLV;
    pAtmoCtx->tmoAttr.CtlInfo.DetailsHighLightMode = DETAILSHIGHLIGHTMODE_ENVLV;
    pAtmoCtx->tmoAttr.CtlInfo.DetailsLowLightMode = DETAILSLOWLIGHTMODE_ISO;
    pAtmoCtx->tmoAttr.CtlInfo.GlobalTmoMode = TMOTYPEMODE_DYNAMICRANGE;
    pAtmoCtx->tmoAttr.CtlInfo.LocalTMOMode = TMOTYPEMODE_DYNAMICRANGE;

    pAtmoCtx->tmoAttr.CtlInfo.Envlv = 1.0;
    pAtmoCtx->tmoAttr.CtlInfo.ISO = 1.0;
    pAtmoCtx->tmoAttr.CtlInfo.OEPdf = 1.0;
    pAtmoCtx->tmoAttr.CtlInfo.FocusLuma = 1.0;
    pAtmoCtx->tmoAttr.CtlInfo.DarkPdf = 1.0;
    pAtmoCtx->tmoAttr.CtlInfo.DynamicRange = 1.0;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

void AtmoGetStats
(
    AtmoHandle_t           pAtmoCtx,
    rkisp_atmo_stats_t*         ROData
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lglow = ROData->tmo_stats.ro_hdrtmo_lglow;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgmin = ROData->tmo_stats.ro_hdrtmo_lgmin;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgmax = ROData->tmo_stats.ro_hdrtmo_lgmax;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lghigh = ROData->tmo_stats.ro_hdrtmo_lghigh;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgmean = ROData->tmo_stats.ro_hdrtmo_lgmean;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_weightkey = ROData->tmo_stats.ro_hdrtmo_weightkey;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgrange0 = ROData->tmo_stats.ro_hdrtmo_lgrange0;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgrange1 = ROData->tmo_stats.ro_hdrtmo_lgrange1;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgavgmax = ROData->tmo_stats.ro_hdrtmo_lgavgmax;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_palpha = ROData->tmo_stats.ro_hdrtmo_palpha;
    pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_linecnt = ROData->tmo_stats.ro_hdrtmo_linecnt;
    for(int i = 0; i < 32; i++)
        pAtmoCtx->CurrStatsData.tmo_stats.ro_array_min_max[i] = ROData->tmo_stats.ro_array_min_max[i];

    //get other stats from stats
    for(int i = 0; i < 225; i++)
    {
        pAtmoCtx->CurrStatsData.other_stats.short_luma[i] = ROData->other_stats.short_luma[i];
        pAtmoCtx->CurrStatsData.other_stats.long_luma[i] = ROData->other_stats.long_luma[i];
        pAtmoCtx->CurrStatsData.other_stats.tmo_luma[i] = ROData->other_stats.tmo_luma[i];
    }

    if(pAtmoCtx->FrameNumber == HDR_3X_NUM)
    {
        for(int i = 0; i < 25; i++)
            pAtmoCtx->CurrStatsData.other_stats.middle_luma[i] = ROData->other_stats.middle_luma[i];
    }

    LOGV_ATMO("%s:  Ahdr RO data from register:\n", __FUNCTION__);
    LOGV_ATMO("%s:  ro_hdrtmo_lglow:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lglow);
    LOGV_ATMO("%s:  ro_hdrtmo_lgmin:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgmin);
    LOGV_ATMO("%s:  ro_hdrtmo_lgmax:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgmax);
    LOGV_ATMO("%s:  ro_hdrtmo_lghigh:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lghigh);
    LOGV_ATMO("%s:  ro_hdrtmo_weightkey:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_weightkey);
    LOGV_ATMO("%s:  ro_hdrtmo_lgmean:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgmean);
    LOGV_ATMO("%s:  ro_hdrtmo_lgrange0:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgrange0);
    LOGV_ATMO("%s:  ro_hdrtmo_lgrange1:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgrange1);
    LOGV_ATMO("%s:  ro_hdrtmo_lgavgmax:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgavgmax);
    LOGV_ATMO("%s:  ro_hdrtmo_palpha:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_palpha);
    LOGV_ATMO("%s:  ro_hdrtmo_linecnt:%d:\n", __FUNCTION__, pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_linecnt);
    for(int i = 0; i < 32; i++)
        LOGV_ATMO("%s:  ro_array_min_max[%d]:%d:\n", __FUNCTION__, i, pAtmoCtx->CurrStatsData.tmo_stats.ro_array_min_max[i]);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

void AtmoGetAeResult
(
    AtmoHandle_t           pAtmoCtx,
    AecPreResult_t  AecHdrPreResult
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    //get Ae Pre Result
    pAtmoCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[AecHdrPreResult.NormalIndex];
    //pAtmoCtx->CurrAeResult.M2S_Ratio = AecHdrPreResult.M2S_ExpRatio;
    //pAtmoCtx->CurrAeResult.M2S_Ratio = pAtmoCtx->CurrAeResult.M2S_Ratio < 1 ? 1 : pAtmoCtx->CurrAeResult.M2S_Ratio;
    //pAtmoCtx->CurrAeResult.L2M_Ratio = AecHdrPreResult.L2M_ExpRatio;
    //pAtmoCtx->CurrAeResult.L2M_Ratio = pAtmoCtx->CurrAeResult.L2M_Ratio < 1 ? 1 : pAtmoCtx->CurrAeResult.L2M_Ratio;
    pAtmoCtx->CurrAeResult.DynamicRange = AecHdrPreResult.DynamicRange;
    pAtmoCtx->CurrAeResult.OEPdf = AecHdrPreResult.OverExpROIPdf[1];
    pAtmoCtx->CurrAeResult.DarkPdf = AecHdrPreResult.LowLightROIPdf[1];
    for(int i = 0; i < 225; i++)
    {
        pAtmoCtx->CurrAeResult.BlockLumaS[i] = pAtmoCtx->CurrStatsData.other_stats.short_luma[i];
        pAtmoCtx->CurrAeResult.BlockLumaL[i] = pAtmoCtx->CurrStatsData.other_stats.long_luma[i];
    }
    if(pAtmoCtx->FrameNumber == HDR_3X_NUM)
        for(int i = 0; i < 25; i++)
            pAtmoCtx->CurrAeResult.BlockLumaM[i] = pAtmoCtx->CurrStatsData.other_stats.middle_luma[i];
    else
        for(int i = 0; i < 25; i++)
            pAtmoCtx->CurrAeResult.BlockLumaM[i] = 0;

    //transfer CurrAeResult data into AhdrHandle
    switch (pAtmoCtx->FrameNumber)
    {
    case LINEAR_NUM:
        pAtmoCtx->CurrData.CtrlData.LExpo = AecHdrPreResult.LinearExp.exp_real_params.analog_gain * AecHdrPreResult.LinearExp.exp_real_params.integration_time;
        pAtmoCtx->CurrData.CtrlData.L2S_Ratio = 1;
        pAtmoCtx->CurrData.CtrlData.L2M_Ratio = 1;
        pAtmoCtx->CurrData.CtrlData.L2L_Ratio = 1;
        pAtmoCtx->CurrAeResult.ISO = AecHdrPreResult.LinearExp.exp_real_params.analog_gain * 50.0;
        pAtmoCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[0];
        pAtmoCtx->CurrAeResult.OEPdf = AecHdrPreResult.OverExpROIPdf[0];
        pAtmoCtx->CurrAeResult.DarkPdf = AecHdrPreResult.LowLightROIPdf[0];
        break;
    case HDR_2X_NUM:
        pAtmoCtx->CurrData.CtrlData.L2S_Ratio = pAtmoCtx->CurrAeResult.M2S_Ratio;
        pAtmoCtx->CurrData.CtrlData.L2M_Ratio = 1;
        pAtmoCtx->CurrData.CtrlData.L2L_Ratio = 1;
        pAtmoCtx->CurrData.CtrlData.LExpo = AecHdrPreResult.HdrExp[1].exp_real_params.analog_gain * AecHdrPreResult.HdrExp[1].exp_real_params.integration_time;
        pAtmoCtx->CurrAeResult.ISO = AecHdrPreResult.HdrExp[1].exp_real_params.analog_gain * 50.0;
        pAtmoCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[1];
        pAtmoCtx->CurrAeResult.OEPdf = AecHdrPreResult.OverExpROIPdf[1];
        pAtmoCtx->CurrAeResult.DarkPdf = AecHdrPreResult.LowLightROIPdf[1];
        break;
    case HDR_3X_NUM:
        pAtmoCtx->CurrData.CtrlData.L2S_Ratio = pAtmoCtx->CurrAeResult.L2M_Ratio * pAtmoCtx->CurrAeResult.M2S_Ratio;
        pAtmoCtx->CurrData.CtrlData.L2M_Ratio = pAtmoCtx->CurrAeResult.L2M_Ratio;
        pAtmoCtx->CurrData.CtrlData.L2L_Ratio = 1;
        pAtmoCtx->CurrData.CtrlData.LExpo = AecHdrPreResult.HdrExp[2].exp_real_params.analog_gain * AecHdrPreResult.HdrExp[2].exp_real_params.integration_time;
        pAtmoCtx->CurrAeResult.ISO = AecHdrPreResult.HdrExp[2].exp_real_params.analog_gain * 50.0;
        pAtmoCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[2];
        pAtmoCtx->CurrAeResult.OEPdf = AecHdrPreResult.OverExpROIPdf[1];
        pAtmoCtx->CurrAeResult.DarkPdf = AecHdrPreResult.LowLightROIPdf[1];
        break;
    default:
        LOGE_ATMO("%s:  Wrong frame number in TMO mode!!!\n", __FUNCTION__);
        break;
    }

    //Normalize the current envLv for AEC
    float maxEnvLuma = 65 / 10;
    float minEnvLuma = 0;
    pAtmoCtx->CurrData.CtrlData.EnvLv = (pAtmoCtx->CurrAeResult.GlobalEnvLv  - minEnvLuma) / (maxEnvLuma - minEnvLuma);
    pAtmoCtx->CurrData.CtrlData.EnvLv = LIMIT_VALUE(pAtmoCtx->CurrData.CtrlData.EnvLv, ENVLVMAX, ENVLVMIN);

    LOGD_ATMO("%s:  Current L2S_Ratio:%f L2M_Ratio:%f L2L_Ratio:%f\n", __FUNCTION__, pAtmoCtx->CurrData.CtrlData.L2S_Ratio,
              pAtmoCtx->CurrData.CtrlData.L2M_Ratio, pAtmoCtx->CurrData.CtrlData.L2L_Ratio);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

void AtmoGetSensorInfo
(
    AtmoHandle_t     pAtmoCtx,
    AecProcResult_t  AecHdrProcResult
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    pAtmoCtx->SensorInfo.LongFrmMode = AecHdrProcResult.LongFrmMode && (pAtmoCtx->FrameNumber != LINEAR_NUM);

    for(int i = 0; i < 3; i++)
    {
        pAtmoCtx->SensorInfo.HdrMinGain[i] = AecHdrProcResult.HdrMinGain[i];
        pAtmoCtx->SensorInfo.HdrMaxGain[i] = AecHdrProcResult.HdrMaxGain[i];
        pAtmoCtx->SensorInfo.HdrMinIntegrationTime[i] = AecHdrProcResult.HdrMinIntegrationTime[i];
        pAtmoCtx->SensorInfo.HdrMaxIntegrationTime[i] = AecHdrProcResult.HdrMaxIntegrationTime[i];
    }

    if(pAtmoCtx->FrameNumber == LINEAR_NUM)
    {
        //pAtmoCtx->SensorInfo.MaxExpoL = pAtmoCtx->SensorInfo.HdrMaxGain[1] * pAtmoCtx->SensorInfo.HdrMaxIntegrationTime[1];
        //pAtmoCtx->SensorInfo.MinExpoL = pAtmoCtx->SensorInfo.HdrMinGain[1] * pAtmoCtx->SensorInfo.HdrMinIntegrationTime[1];
        //pAtmoCtx->SensorInfo.MaxExpoM = 0;
        //pAtmoCtx->SensorInfo.MinExpoM = 0;

        pAtmoCtx->CurrAeResult.LumaDeviationLinear = AecHdrProcResult.LumaDeviation;
        pAtmoCtx->CurrAeResult.LumaDeviationLinear = abs(pAtmoCtx->CurrAeResult.LumaDeviationLinear);
    }
    else if(pAtmoCtx->FrameNumber == HDR_2X_NUM)
    {
        pAtmoCtx->SensorInfo.MaxExpoL = pAtmoCtx->SensorInfo.HdrMaxGain[1] * pAtmoCtx->SensorInfo.HdrMaxIntegrationTime[1];
        pAtmoCtx->SensorInfo.MinExpoL = pAtmoCtx->SensorInfo.HdrMinGain[1] * pAtmoCtx->SensorInfo.HdrMinIntegrationTime[1];
        pAtmoCtx->SensorInfo.MaxExpoM = 0;
        pAtmoCtx->SensorInfo.MinExpoM = 0;

        pAtmoCtx->CurrAeResult.LumaDeviationL = AecHdrProcResult.HdrLumaDeviation[1];
        pAtmoCtx->CurrAeResult.LumaDeviationL = abs(pAtmoCtx->CurrAeResult.LumaDeviationL);
        pAtmoCtx->CurrAeResult.LumaDeviationS = AecHdrProcResult.HdrLumaDeviation[0];
        pAtmoCtx->CurrAeResult.LumaDeviationS = abs(pAtmoCtx->CurrAeResult.LumaDeviationS);
    }
    else if(pAtmoCtx->FrameNumber == HDR_3X_NUM)
    {
        pAtmoCtx->SensorInfo.MaxExpoL = pAtmoCtx->SensorInfo.HdrMaxGain[2] * pAtmoCtx->SensorInfo.HdrMaxIntegrationTime[2];
        pAtmoCtx->SensorInfo.MinExpoL = pAtmoCtx->SensorInfo.HdrMinGain[2] * pAtmoCtx->SensorInfo.HdrMinIntegrationTime[2];
        pAtmoCtx->SensorInfo.MaxExpoM = pAtmoCtx->SensorInfo.HdrMaxGain[1] * pAtmoCtx->SensorInfo.HdrMaxIntegrationTime[1];
        pAtmoCtx->SensorInfo.MinExpoM = pAtmoCtx->SensorInfo.HdrMinGain[1] * pAtmoCtx->SensorInfo.HdrMinIntegrationTime[1];

        pAtmoCtx->CurrAeResult.LumaDeviationL = AecHdrProcResult.HdrLumaDeviation[2];
        pAtmoCtx->CurrAeResult.LumaDeviationL = abs(pAtmoCtx->CurrAeResult.LumaDeviationL);
        pAtmoCtx->CurrAeResult.LumaDeviationM = AecHdrProcResult.HdrLumaDeviation[1];
        pAtmoCtx->CurrAeResult.LumaDeviationM = abs(pAtmoCtx->CurrAeResult.LumaDeviationM);
        pAtmoCtx->CurrAeResult.LumaDeviationS = AecHdrProcResult.HdrLumaDeviation[0];
        pAtmoCtx->CurrAeResult.LumaDeviationS = abs(pAtmoCtx->CurrAeResult.LumaDeviationS);
    }

    pAtmoCtx->SensorInfo.MaxExpoS = pAtmoCtx->SensorInfo.HdrMaxGain[0] * pAtmoCtx->SensorInfo.HdrMaxIntegrationTime[0];
    pAtmoCtx->SensorInfo.MinExpoS = pAtmoCtx->SensorInfo.HdrMinGain[0] * pAtmoCtx->SensorInfo.HdrMinIntegrationTime[0];


    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AtmoApiSetLevel()
 *
 *****************************************************************************/
void AtmoApiSetLevel
(
    AtmoHandle_t     pAtmoCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //tmo data
    pAtmoCtx->tmoAttr.stSetLevel.level = LIMIT_VALUE(pAtmoCtx->tmoAttr.stSetLevel.level, FASTMODELEVELMAX, FASTMODELEVELMIN);

    float level = ((float)(pAtmoCtx->tmoAttr.stSetLevel.level)) / FASTMODELEVELMAX;
    float level_default = 0.5;
    float level_diff = level - level_default;

    pAtmoCtx->CurrData.HandleData.GlobeLuma *= 1 + level_diff;
    float GlobeLuma = pAtmoCtx->CurrData.HandleData.GlobeLuma;
    pAtmoCtx->CurrData.HandleData.GlobeMaxLuma = MAXLUMAK * GlobeLuma + MAXLUMAB;
    pAtmoCtx->CurrData.HandleData.GlobeMaxLuma =
        LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.GlobeMaxLuma, GLOBEMAXLUMAMAX, GLOBEMAXLUMAMIN);

    pAtmoCtx->CurrData.HandleData.DetailsHighLight *= 1 + level_diff;
    pAtmoCtx->CurrData.HandleData.DetailsHighLight =
        LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.DetailsHighLight, DETAILSHIGHLIGHTMAX, DETAILSHIGHLIGHTMIN);

    pAtmoCtx->CurrData.HandleData.DetailsLowLight *= 1 + level_diff;
    pAtmoCtx->CurrData.HandleData.DetailsLowLight =
        LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.DetailsLowLight, DETAILSLOWLIGHTMAX, DETAILSLOWLIGHTMIN);

    pAtmoCtx->AtmoConfig.bTmoEn = true;
    pAtmoCtx->ProcRes.isLinearTmo = pAtmoCtx->FrameNumber == LINEAR_NUM ;

    /*
        pAtmoCtx->CurrData.HandleData.TmoContrast *= 1 + level_diff;
        pAtmoCtx->CurrData.HandleData.TmoContrast =
            LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.LocalTmoStrength, TMOCONTRASTMAX, TMOCONTRASTMIN);
    */
    //paras after updating
    LOGD_ATMO("%s:  AHDR_OpMode_Fast set level:%d\n", __FUNCTION__, pAtmoCtx->tmoAttr.stSetLevel.level);
    LOGD_ATMO("%s:  After fast mode GlobeLuma:%f GlobeMaxLuma:%f DetailsHighLight:%f DetailsLowLight:%f LocalTmoStrength:%f \n", __FUNCTION__,
              pAtmoCtx->CurrData.HandleData.GlobeLuma, pAtmoCtx->CurrData.HandleData.GlobeMaxLuma,
              pAtmoCtx->CurrData.HandleData.DetailsHighLight, pAtmoCtx->CurrData.HandleData.DetailsLowLight,
              pAtmoCtx->CurrData.HandleData.LocalTmoStrength);


    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * ApiOffProcess()
 *
 *****************************************************************************/
void ApiOffProcess(AtmoHandle_t     pAtmoCtx)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //get Current GlobeLuma GlobeMaxLuma
    if(pAtmoCtx->AtmoConfig.Luma.globalLumaMode == GLOBALLUMAMODE_ENVLV)
        pAtmoCtx->CurrData.HandleData.GlobeLuma = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.EnvLv,
                pAtmoCtx->AtmoConfig.Luma.EnvLv, pAtmoCtx->AtmoConfig.Luma.GlobeLuma, pAtmoCtx->AtmoConfig.Luma.len);
    else if(pAtmoCtx->AtmoConfig.Luma.globalLumaMode == GLOBALLUMAMODE_ISO)
        pAtmoCtx->CurrData.HandleData.GlobeLuma = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.ISO,
                pAtmoCtx->AtmoConfig.Luma.ISO, pAtmoCtx->AtmoConfig.Luma.GlobeLuma, pAtmoCtx->AtmoConfig.Luma.len);

    float GlobeLuma = pAtmoCtx->CurrData.HandleData.GlobeLuma;
    pAtmoCtx->CurrData.HandleData.GlobeMaxLuma = MAXLUMAK * GlobeLuma + MAXLUMAB;
    pAtmoCtx->CurrData.HandleData.GlobeMaxLuma = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.GlobeMaxLuma, GLOBEMAXLUMAMAX, GLOBEMAXLUMAMIN);

    //get Current local tmo
    if(pAtmoCtx->AtmoConfig.local.localtmoMode == TMOTYPEMODE_DYNAMICRANGE)
        pAtmoCtx->CurrData.HandleData.LocalTmoStrength = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.DynamicRange,
                pAtmoCtx->AtmoConfig.local.DynamicRange, pAtmoCtx->AtmoConfig.local.LocalTmoStrength, pAtmoCtx->AtmoConfig.local.len);
    else if(pAtmoCtx->AtmoConfig.local.localtmoMode == TMOTYPEMODE_ENVLV)
        pAtmoCtx->CurrData.HandleData.LocalTmoStrength = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.EnvLv,
                pAtmoCtx->AtmoConfig.local.EnvLv, pAtmoCtx->AtmoConfig.local.LocalTmoStrength, pAtmoCtx->AtmoConfig.local.len);

    if(pAtmoCtx->AtmoConfig.global.isHdrGlobalTmo)
        pAtmoCtx->CurrData.HandleData.LocalTmoStrength = 0;

    //get Current global tmo
    if(pAtmoCtx->AtmoConfig.global.mode == TMOTYPEMODE_DYNAMICRANGE)
        pAtmoCtx->CurrData.HandleData.GlobalTmoStrength = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.DynamicRange,
                pAtmoCtx->AtmoConfig.global.DynamicRange, pAtmoCtx->AtmoConfig.global.GlobalTmoStrength, pAtmoCtx->AtmoConfig.global.len);
    else if(pAtmoCtx->AtmoConfig.global.mode  == TMOTYPEMODE_ENVLV)
        pAtmoCtx->CurrData.HandleData.GlobalTmoStrength = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.EnvLv,
                pAtmoCtx->AtmoConfig.global.EnvLv, pAtmoCtx->AtmoConfig.global.GlobalTmoStrength, pAtmoCtx->AtmoConfig.global.len);

    //get Current DetailsHighLight
    if(pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode == DETAILSHIGHLIGHTMODE_OEPDF)
        pAtmoCtx->CurrData.HandleData.DetailsHighLight = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.OEPdf,
                pAtmoCtx->AtmoConfig.DtsHiLit.OEPdf, pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight, pAtmoCtx->AtmoConfig.DtsHiLit.len);
    else if(pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode == DETAILSHIGHLIGHTMODE_ENVLV)
        pAtmoCtx->CurrData.HandleData.DetailsHighLight = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.EnvLv,
                pAtmoCtx->AtmoConfig.DtsHiLit.EnvLv, pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight, pAtmoCtx->AtmoConfig.DtsHiLit.len);

    //get Current DetailsLowLight
    if (pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode == DETAILSLOWLIGHTMODE_FOCUSLUMA)
    {
#if 0
        int focs = pAtmoCtx->CurrAfResult.CurrAfTargetPos;
        int focs_width = pAtmoCtx->CurrAfResult.CurrAfTargetWidth / (pAtmoCtx->width / 15);
        int focs_height = pAtmoCtx->CurrAfResult.CurrAfTargetHeight / (pAtmoCtx->height / 15);
        float focs_luma = 0;
        for(int i = 0; i < focs_height; i++)
            for(int j = 0; j < focs_width; j++)
                focs_luma += pAtmoCtx->CurrAeResult.BlockLumaL[focs + i + 15 * j];
        focs_luma = focs_luma / (focs_width * focs_height);
        pAtmoCtx->CurrData.CtrlData.FocusLuma = focs_luma / 15;
        pAtmoCtx->CurrData.CtrlData.FocusLuma = LIMIT_VALUE(pAtmoCtx->CurrData.CtrlData.FocusLuma, FOCUSLUMAMAX, FOCUSLUMAMIN);
        pAtmoCtx->CurrData.HandleData.DetailsLowLight = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.FocusLuma,
                pAtmoCtx->AtmoConfig.DtsLoLit.FocusLuma, pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight, pAtmoCtx->AtmoConfig.DtsLoLit.len);

#endif
        pAtmoCtx->CurrData.HandleData.DetailsLowLight = pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight[0];

    }
    else if (pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode == DETAILSLOWLIGHTMODE_DARKPDF)
        pAtmoCtx->CurrData.HandleData.DetailsLowLight = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.DarkPdf,
                pAtmoCtx->AtmoConfig.DtsLoLit.DarkPdf, pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight, pAtmoCtx->AtmoConfig.DtsLoLit.len);
    else if (pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode == DETAILSLOWLIGHTMODE_ISO)
        pAtmoCtx->CurrData.HandleData.DetailsLowLight = TmoGetCurrPara(pAtmoCtx->CurrData.CtrlData.ISO,
                pAtmoCtx->AtmoConfig.DtsLoLit.ISO, pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight, pAtmoCtx->AtmoConfig.DtsLoLit.len);

    //get Current tmo TmoDamp
    pAtmoCtx->CurrData.CtrlData.TmoDamp = pAtmoCtx->AtmoConfig.damp;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AtmoTranferData2Api()
 *
 *****************************************************************************/
void AtmoTranferData2Api
(
    AtmoHandle_t     pAtmoCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //transfer control data to api
    pAtmoCtx->tmoAttr.CtlInfo.GlobalLumaMode = pAtmoCtx->AtmoConfig.Luma.globalLumaMode;
    pAtmoCtx->tmoAttr.CtlInfo.DetailsHighLightMode = pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode;
    pAtmoCtx->tmoAttr.CtlInfo.DetailsLowLightMode = pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode;
    pAtmoCtx->tmoAttr.CtlInfo.GlobalTmoMode = pAtmoCtx->AtmoConfig.global.mode;
    pAtmoCtx->tmoAttr.CtlInfo.LocalTMOMode = pAtmoCtx->AtmoConfig.local.localtmoMode;
    pAtmoCtx->tmoAttr.CtlInfo.Envlv = pAtmoCtx->CurrData.CtrlData.EnvLv;
    pAtmoCtx->tmoAttr.CtlInfo.ISO = pAtmoCtx->CurrData.CtrlData.ISO;
    pAtmoCtx->tmoAttr.CtlInfo.OEPdf = pAtmoCtx->CurrData.CtrlData.OEPdf;
    pAtmoCtx->tmoAttr.CtlInfo.DarkPdf = pAtmoCtx->CurrData.CtrlData.DarkPdf;
    pAtmoCtx->tmoAttr.CtlInfo.FocusLuma = pAtmoCtx->CurrData.CtrlData.FocusLuma;
    pAtmoCtx->tmoAttr.CtlInfo.DynamicRange = pAtmoCtx->CurrData.CtrlData.DynamicRange;

    //transfer register data to api
    pAtmoCtx->tmoAttr.RegInfo.GlobalLuma = pAtmoCtx->CurrData.HandleData.GlobeLuma / GLOBELUMAMAX;
    pAtmoCtx->tmoAttr.RegInfo.DetailsLowlight = pAtmoCtx->CurrData.HandleData.DetailsLowLight / DETAILSLOWLIGHTMIN;
    pAtmoCtx->tmoAttr.RegInfo.DetailsHighlight = pAtmoCtx->CurrData.HandleData.DetailsHighLight / DETAILSHIGHLIGHTMAX;
    pAtmoCtx->tmoAttr.RegInfo.LocalTmoStrength = pAtmoCtx->CurrData.HandleData.LocalTmoStrength / TMOCONTRASTMAX;
    pAtmoCtx->tmoAttr.RegInfo.GlobaltmoStrength = pAtmoCtx->CurrData.HandleData.GlobalTmoStrength;


    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AtmoApiAutoUpdate()
 *
 *****************************************************************************/
void AtmoApiAutoUpdate
(
    AtmoHandle_t     pAtmoCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //update tmo data in Auto mode
    if (pAtmoCtx->tmoAttr.stAuto.bUpdateTmo == true)
    {
        pAtmoCtx->CurrData.HandleData.DetailsLowLight =
            LIMIT_PARA(pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsLL.stCoef, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsLL.stMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsLL.stMin,
                       pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsLL.stCoefMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsLL.stMin);
        pAtmoCtx->CurrData.HandleData.DetailsLowLight *= IQDETAILSLOWLIGHTMIN;
        pAtmoCtx->CurrData.HandleData.DetailsLowLight = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.DetailsLowLight
                , IQDETAILSLOWLIGHTMAX, IQDETAILSLOWLIGHTMIN);

        pAtmoCtx->CurrData.HandleData.DetailsHighLight =
            LIMIT_PARA(pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsHL.stCoef, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsHL.stMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsHL.stMin,
                       pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsHL.stCoefMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stDtlsHL.stMin);
        pAtmoCtx->CurrData.HandleData.DetailsHighLight *= DETAILSHIGHLIGHTMAX;
        pAtmoCtx->CurrData.HandleData.DetailsHighLight = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.DetailsHighLight
                , DETAILSHIGHLIGHTMAX, DETAILSHIGHLIGHTMIN);

        pAtmoCtx->CurrData.HandleData.LocalTmoStrength =
            LIMIT_PARA(pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stLocalTMO.stCoef, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stLocalTMO.stMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stLocalTMO.stMin,
                       pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stLocalTMO.stCoefMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stLocalTMO.stMin);
        pAtmoCtx->CurrData.HandleData.LocalTmoStrength *= TMOCONTRASTMAX;
        pAtmoCtx->CurrData.HandleData.LocalTmoStrength = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.LocalTmoStrength
                , TMOCONTRASTMAX, TMOCONTRASTMIN);

        pAtmoCtx->CurrData.HandleData.GlobeLuma =
            LIMIT_PARA(pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobeLuma.stCoef, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobeLuma.stMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobeLuma.stMin,
                       pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobeLuma.stCoefMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobeLuma.stMin);
        pAtmoCtx->CurrData.HandleData.GlobeLuma *= GLOBELUMAMAX;
        pAtmoCtx->CurrData.HandleData.GlobeLuma = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.GlobeLuma
                , GLOBELUMAMAX, GLOBELUMAMIN);

        float GlobeLuma = pAtmoCtx->CurrData.HandleData.GlobeLuma;
        pAtmoCtx->CurrData.HandleData.GlobeMaxLuma = MAXLUMAK * GlobeLuma + MAXLUMAB;
        pAtmoCtx->CurrData.HandleData.GlobeMaxLuma = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.GlobeMaxLuma,
                GLOBEMAXLUMAMAX, GLOBEMAXLUMAMIN);

        if(pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobalTMO.en)
        {
            float strength = LIMIT_PARA(pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobalTMO.stCoef, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobalTMO.stMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobalTMO.stMin,
                                        pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobalTMO.stCoefMax, pAtmoCtx->tmoAttr.stAuto.stTmoAuto.stGlobalTMO.stMin);
            pAtmoCtx->CurrData.HandleData.GlobalTmoStrength = strength;
        }
        else
            pAtmoCtx->CurrData.HandleData.GlobalTmoStrength = 0.5;

    }
    else
        ApiOffProcess(pAtmoCtx);

    //paras after updating
    LOGD_ATMO("%s:	Current GlobeLuma:%f GlobeMaxLuma:%f DetailsHighLight:%f DetailsLowLight:%f GlobalTmoStrength:%f LocalTmoStrength:%f\n", __FUNCTION__,
              pAtmoCtx->CurrData.HandleData.GlobeLuma, pAtmoCtx->CurrData.HandleData.GlobeMaxLuma,
              pAtmoCtx->CurrData.HandleData.DetailsHighLight, pAtmoCtx->CurrData.HandleData.DetailsLowLight,
              pAtmoCtx->CurrData.HandleData.GlobalTmoStrength, pAtmoCtx->CurrData.HandleData.LocalTmoStrength);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AtmoApiManualUpdate()
 *
 *****************************************************************************/
void AtmoApiManualUpdate
(
    AtmoHandle_t     pAtmoCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //update tmo data in manual mode
    if (pAtmoCtx->tmoAttr.stManual.bUpdateTmo == true)
    {
        pAtmoCtx->AtmoConfig.bTmoEn = pAtmoCtx->tmoAttr.stManual.stTmoManual.Enable;
        pAtmoCtx->AtmoConfig.isLinearTmo = pAtmoCtx->AtmoConfig.bTmoEn && pAtmoCtx->FrameNumber == LINEAR_NUM;
        pAtmoCtx->CurrData.HandleData.DetailsLowLight = pAtmoCtx->tmoAttr.stManual.stTmoManual.stDtlsLL * DETAILSLOWLIGHTMIN ;
        pAtmoCtx->CurrData.HandleData.DetailsLowLight = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.DetailsLowLight
                , DETAILSLOWLIGHTMAX, DETAILSLOWLIGHTMIN);

        pAtmoCtx->CurrData.HandleData.DetailsHighLight = pAtmoCtx->tmoAttr.stManual.stTmoManual.stDtlsHL * DETAILSHIGHLIGHTMAX ;
        pAtmoCtx->CurrData.HandleData.DetailsHighLight = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.DetailsHighLight
                , DETAILSHIGHLIGHTMAX, DETAILSHIGHLIGHTMIN);

        pAtmoCtx->CurrData.HandleData.LocalTmoStrength = pAtmoCtx->tmoAttr.stManual.stTmoManual.stLocalTMOStrength * TMOCONTRASTMAX;
        pAtmoCtx->CurrData.HandleData.LocalTmoStrength = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.LocalTmoStrength
                , TMOCONTRASTMAX, TMOCONTRASTMIN);

        pAtmoCtx->CurrData.HandleData.GlobeLuma = pAtmoCtx->tmoAttr.stManual.stTmoManual.stGlobeLuma * GLOBELUMAMAX;
        pAtmoCtx->CurrData.HandleData.GlobeLuma = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.GlobeLuma
                , GLOBELUMAMAX, GLOBELUMAMIN);

        float GlobeLuma = pAtmoCtx->CurrData.HandleData.GlobeLuma;
        pAtmoCtx->CurrData.HandleData.GlobeMaxLuma = MAXLUMAK * GlobeLuma + MAXLUMAB;
        pAtmoCtx->CurrData.HandleData.GlobeMaxLuma = LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.GlobeMaxLuma,
                GLOBEMAXLUMAMAX, GLOBEMAXLUMAMIN);

        pAtmoCtx->CurrData.HandleData.GlobalTmoStrength = LIMIT_VALUE(pAtmoCtx->tmoAttr.stManual.stTmoManual.stGlobalTMOStrength,
                1, 0);

        pAtmoCtx->CurrData.CtrlData.TmoDamp = pAtmoCtx->tmoAttr.stManual.stTmoManual.damp;

    }
    else
        ApiOffProcess(pAtmoCtx);

    //paras after updating
    LOGD_ATMO("%s:  Current GlobeLuma:%f GlobeMaxLuma:%f DetailsHighLight:%f DetailsLowLight:%f GlobalTmoStrength:%f LocalTmoStrength:%f\n", __FUNCTION__,
              pAtmoCtx->CurrData.HandleData.GlobeLuma, pAtmoCtx->CurrData.HandleData.GlobeMaxLuma,
              pAtmoCtx->CurrData.HandleData.DetailsHighLight, pAtmoCtx->CurrData.HandleData.DetailsLowLight,
              pAtmoCtx->CurrData.HandleData.GlobalTmoStrength, pAtmoCtx->CurrData.HandleData.LocalTmoStrength);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * newMalloc()
 ***************************************************************************/
void newMalloc
(
    AtmoConfig_t*           pAtmoConfig,
    CalibDbV2_tmo_t*         pCalibDb
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAtmoConfig != NULL);
    DCT_ASSERT(pCalibDb != NULL);

    if(pAtmoConfig->Luma.len != pCalibDb->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len) {
        free(pAtmoConfig->Luma.EnvLv);
        free(pAtmoConfig->Luma.ISO);
        free(pAtmoConfig->Luma.GlobeLuma);
        pAtmoConfig->Luma.len = pCalibDb->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
        pAtmoConfig->Luma.EnvLv = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len));
        pAtmoConfig->Luma.ISO = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO_len));
        pAtmoConfig->Luma.GlobeLuma = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength_len));
    }
    if(pAtmoConfig->DtsHiLit.len != pCalibDb->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len) {
        free(pAtmoConfig->DtsHiLit.OEPdf);
        free(pAtmoConfig->DtsHiLit.EnvLv);
        free(pAtmoConfig->DtsHiLit.DetailsHighLight);
        pAtmoConfig->DtsHiLit.len = pCalibDb->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len;
        pAtmoConfig->DtsHiLit.OEPdf = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len));
        pAtmoConfig->DtsHiLit.EnvLv = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv_len));
        pAtmoConfig->DtsHiLit.DetailsHighLight = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.DetailsHighLight.HighLightData.Strength_len));
    }
    if(pAtmoConfig->DtsLoLit.len != pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len) {
        free(pAtmoConfig->DtsLoLit.FocusLuma);
        free(pAtmoConfig->DtsLoLit.DarkPdf);
        free(pAtmoConfig->DtsLoLit.ISO);
        free(pAtmoConfig->DtsLoLit.DetailsLowLight);
        pAtmoConfig->DtsLoLit.len = pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len;
        pAtmoConfig->DtsLoLit.FocusLuma = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len));
        pAtmoConfig->DtsLoLit.DarkPdf = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf_len));
        pAtmoConfig->DtsLoLit.ISO = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.ISO_len));
        pAtmoConfig->DtsLoLit.DetailsLowLight = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.Strength_len));
    }
    if(pAtmoConfig->local.len != pCalibDb->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len) {
        free(pAtmoConfig->local.DynamicRange);
        free(pAtmoConfig->local.EnvLv);
        free(pAtmoConfig->local.LocalTmoStrength);
        pAtmoConfig->local.len = pCalibDb->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len;
        pAtmoConfig->local.DynamicRange = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len));
        pAtmoConfig->local.EnvLv = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv_len));
        pAtmoConfig->local.LocalTmoStrength = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.LocalTMO.LocalTmoData.Strength_len));

    }
    if(pAtmoConfig->global.len != pCalibDb->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len) {
        free(pAtmoConfig->global.DynamicRange);
        free(pAtmoConfig->global.EnvLv);
        free(pAtmoConfig->global.GlobalTmoStrength);
        pAtmoConfig->global.len = pCalibDb->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len;
        pAtmoConfig->global.DynamicRange = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len));
        pAtmoConfig->global.EnvLv = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv_len));
        pAtmoConfig->global.GlobalTmoStrength = (float*)malloc(sizeof(float) * (pCalibDb->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength_len));
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AtmoUpdateConfig()
 *transfer html parameter into handle
 ***************************************************************************/
void AtmoUpdateConfig
(
    AtmoHandle_t           pAtmoCtx,
    CalibDbV2_tmo_t*         pCalibDb
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAtmoCtx != NULL);
    DCT_ASSERT(pCalibDb != NULL);

    pAtmoCtx->AtmoConfig.Luma.globalLumaMode = pCalibDb->TmoTuningPara.GlobalLuma.Mode;
    pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode = pCalibDb->TmoTuningPara.DetailsLowLight.Mode;
    pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode = pCalibDb->TmoTuningPara.DetailsHighLight.Mode;
    pAtmoCtx->AtmoConfig.local.localtmoMode = pCalibDb->TmoTuningPara.LocalTMO.Mode;
    pAtmoCtx->AtmoConfig.damp = LIMIT_VALUE(pCalibDb->TmoTuningPara.damp, DAMPMAX, DAMPMIN);
    pAtmoCtx->AtmoConfig.Luma.Tolerance = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobalLuma.Tolerance, TOLERANCEMAX, TOLERANCEMIN);
    pAtmoCtx->AtmoConfig.DtsHiLit.Tolerance = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsHighLight.Tolerance, TOLERANCEMAX, TOLERANCEMIN);
    pAtmoCtx->AtmoConfig.DtsLoLit.Tolerance = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsLowLight.Tolerance, TOLERANCEMAX, TOLERANCEMIN);
    pAtmoCtx->AtmoConfig.local.Tolerance = LIMIT_VALUE(pCalibDb->TmoTuningPara.LocalTMO.Tolerance, TOLERANCEMAX, TOLERANCEMIN);
    for(int i = 0; i < pAtmoCtx->AtmoConfig.Luma.len; i++) {
        pAtmoCtx->AtmoConfig.Luma.EnvLv[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv[i], ENVLVMAX, ENVLVMIN);
        pAtmoCtx->AtmoConfig.Luma.ISO[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO[i], ISOMAX, ISOMIN);
        pAtmoCtx->AtmoConfig.Luma.GlobeLuma[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength[i], IQPARAMAX, IQPARAMIN) ;
    }
    for(int i = 0; i < pAtmoCtx->AtmoConfig.DtsHiLit.len; i++) {
        pAtmoCtx->AtmoConfig.DtsHiLit.OEPdf[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf[i], OEPDFMAX, OEPDFMIN) ;
        pAtmoCtx->AtmoConfig.DtsHiLit.EnvLv[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv[i], ENVLVMAX, ENVLVMIN);
        pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsHighLight.HighLightData.Strength[i], IQPARAMAX, IQPARAMIN) ;
    }
    for(int i = 0; i < pAtmoCtx->AtmoConfig.DtsLoLit.len; i++) {
        pAtmoCtx->AtmoConfig.DtsLoLit.FocusLuma[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma[i], FOCUSLUMAMAX, FOCUSLUMAMIN) ;
        pAtmoCtx->AtmoConfig.DtsLoLit.DarkPdf[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf[i], DARKPDFMAX, DARKPDFMIN) ;
        pAtmoCtx->AtmoConfig.DtsLoLit.ISO[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.ISO[i], ISOMAX, ISOMIN) ;
        pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.DetailsLowLight.LowLightData.Strength[i], IQDETAILSLOWLIGHTMAX, IQDETAILSLOWLIGHTMIN) ;
    }
    for(int i = 0; i < pAtmoCtx->AtmoConfig.local.len; i++) {
        pAtmoCtx->AtmoConfig.local.DynamicRange[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange[i], DYNAMICRANGEMAX, DYNAMICRANGEMIN) ;
        pAtmoCtx->AtmoConfig.local.EnvLv[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv[i], ENVLVMAX, ENVLVMIN) ;
        pAtmoCtx->AtmoConfig.local.LocalTmoStrength[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.LocalTMO.LocalTmoData.Strength[i], IQPARAMAX, IQPARAMIN) ;
    }

    //Global Tmo
    pAtmoCtx->AtmoConfig.global.isHdrGlobalTmo = pCalibDb->TmoTuningPara.GlobaTMO.Enable;
    pAtmoCtx->AtmoConfig.global.mode = pCalibDb->TmoTuningPara.GlobaTMO.Mode;
    pAtmoCtx->AtmoConfig.global.Tolerance = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobaTMO.Tolerance, TOLERANCEMAX, TOLERANCEMIN);
    for(int i = 0; i < pAtmoCtx->AtmoConfig.global.len; i++) {
        pAtmoCtx->AtmoConfig.global.DynamicRange[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange[i], DYNAMICRANGEMAX, DYNAMICRANGEMIN) ;
        pAtmoCtx->AtmoConfig.global.EnvLv[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv[i], ENVLVMAX, ENVLVMIN) ;
        pAtmoCtx->AtmoConfig.global.GlobalTmoStrength[i] = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength[i], IQPARAMAX, IQPARAMIN) ;
    }
    pAtmoCtx->AtmoConfig.global.iir = LIMIT_VALUE(pCalibDb->TmoTuningPara.GlobaTMO.IIR, IIRMAX, IIRMIN);

    //tmo En
    if(pAtmoCtx->FrameNumber == HDR_2X_NUM || pAtmoCtx->FrameNumber == HDR_3X_NUM) {
        pAtmoCtx->AtmoConfig.bTmoEn = true;
        pAtmoCtx->AtmoConfig.isLinearTmo = false;
    }
    else if(pAtmoCtx->FrameNumber == LINEAR_NUM)
    {
        pAtmoCtx->AtmoConfig.bTmoEn = pCalibDb->TmoTuningPara.Enable;
        pAtmoCtx->AtmoConfig.isLinearTmo = pAtmoCtx->AtmoConfig.bTmoEn;
    }

    for(int i = 0; i < pAtmoCtx->AtmoConfig.Luma.len; i++)
        LOG1_ATMO("%s: mode:%d Globalluma[%d]:%f EnvLv[%d]:%f ISO[%d]:%f Tolerance:%f\n", __FUNCTION__,
                  pAtmoCtx->AtmoConfig.Luma.globalLumaMode, i, pAtmoCtx->AtmoConfig.Luma.GlobeLuma[i],
                  i, pAtmoCtx->AtmoConfig.Luma.EnvLv[i], i, pAtmoCtx->AtmoConfig.Luma.ISO[i], pAtmoCtx->AtmoConfig.Luma.Tolerance);

    for(int i = 0; i < pAtmoCtx->AtmoConfig.DtsHiLit.len; i++)
        LOG1_ATMO("%s: mode:%d DetailsHighLight[%d]:%f OEPdf[%d]:%f EnvLv[%d]:%f Tolerance:%f\n", __FUNCTION__,
                  pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode, i, pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight[i],
                  i, pAtmoCtx->AtmoConfig.DtsHiLit.OEPdf[i], i, pAtmoCtx->AtmoConfig.DtsHiLit.EnvLv[i], pAtmoCtx->AtmoConfig.DtsHiLit.Tolerance);

    for(int i = 0; i < pAtmoCtx->AtmoConfig.DtsLoLit.len; i++)
        LOG1_ATMO("%s: mode:%d DetailsLowLight[%d]:%f FocusLuma[%d]:%f DarkPdf[%d]:%f ISO[%d]:%f Tolerance:%f\n", __FUNCTION__,
                  pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode, i, pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight[i],
                  i, pAtmoCtx->AtmoConfig.DtsLoLit.FocusLuma[i], i, pAtmoCtx->AtmoConfig.DtsLoLit.DarkPdf[i],
                  i, pAtmoCtx->AtmoConfig.DtsLoLit.ISO[i], pAtmoCtx->AtmoConfig.DtsLoLit.Tolerance);

    for(int i = 0; i < pAtmoCtx->AtmoConfig.local.len; i++)
        LOG1_ATMO("%s: mode:%d LocalTmoStrength[%d]:%f DynamicRange[%d]:%f EnvLv[%d]:%f Tolerance:%f\n", __FUNCTION__,
                  pAtmoCtx->AtmoConfig.local.localtmoMode, i, pAtmoCtx->AtmoConfig.local.LocalTmoStrength[i],
                  i, pAtmoCtx->AtmoConfig.local.DynamicRange[i], i, pAtmoCtx->AtmoConfig.local.EnvLv[i], pAtmoCtx->AtmoConfig.local.Tolerance);

    LOGD_ATMO("%s:  Tmo En:%d linear Tmo en:%d\n", __FUNCTION__, pAtmoCtx->AtmoConfig.bTmoEn, pAtmoCtx->AtmoConfig.isLinearTmo);
    LOG1_ATMO("%s:  GlobalTmo En:%d IIR:%f\n", __FUNCTION__, pCalibDb->TmoTuningPara.GlobaTMO.Enable, pAtmoCtx->AtmoConfig.global.iir);
    for(int i = 0; i < pAtmoCtx->AtmoConfig.global.len; i++)
        LOG1_ATMO("%s: mode:%d GlobalTmoStrength[%d]:%f DynamicRange[%d]:%f EnvLv[%d]:%f Tolerance:%f\n", __FUNCTION__,
                  pAtmoCtx->AtmoConfig.global.mode, i, pAtmoCtx->AtmoConfig.global.GlobalTmoStrength[i],
                  i, pAtmoCtx->AtmoConfig.global.DynamicRange[i], i, pAtmoCtx->AtmoConfig.global.EnvLv[i], pAtmoCtx->AtmoConfig.global.Tolerance);

    //turn the IQ paras into algo paras
    for(int i = 0; i < pAtmoCtx->AtmoConfig.Luma.len; i++) {
        pAtmoCtx->AtmoConfig.Luma.GlobeLuma[i] = pAtmoCtx->AtmoConfig.Luma.GlobeLuma[i] * GLOBELUMAMAX;
        pAtmoCtx->AtmoConfig.Luma.GlobeLuma[i] = LIMIT_VALUE(pAtmoCtx->AtmoConfig.Luma.GlobeLuma[i], GLOBELUMAMAX, GLOBELUMAMIN) ;
    }
    for(int i = 0; i < pAtmoCtx->AtmoConfig.DtsHiLit.len; i++) {
        pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight[i] = pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight[i] * DETAILSHIGHLIGHTMAX;
        pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight[i] = LIMIT_VALUE(pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight[i], DETAILSHIGHLIGHTMAX, DETAILSHIGHLIGHTMIN) ;
    }
    for(int i = 0; i < pAtmoCtx->AtmoConfig.DtsLoLit.len; i++) {
        pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight[i] = pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight[i] * DETAILSLOWLIGHTMIN;
        pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight[i] = LIMIT_VALUE(pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight[i], DETAILSLOWLIGHTMAX, DETAILSLOWLIGHTMIN) ;
    }
    for(int i = 0; i < pAtmoCtx->AtmoConfig.local.len; i++) {
        pAtmoCtx->AtmoConfig.local.LocalTmoStrength[i] = pAtmoCtx->AtmoConfig.local.LocalTmoStrength[i] * TMOCONTRASTMAX;
        pAtmoCtx->AtmoConfig.local.LocalTmoStrength[i] = LIMIT_VALUE(pAtmoCtx->AtmoConfig.local.LocalTmoStrength[i], TMOCONTRASTMAX, TMOCONTRASTMIN) ;
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * GetSetLgmean()
 *****************************************************************************/
unsigned short GetSetLgmean(AtmoHandle_t pAtmoCtx)
{
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    float value = 0;
    float value_default = 20000;
    unsigned short returnValue;
    int iir_frame = 0;

    if(pAtmoCtx->AtmoConfig.global.iir < IIRMAX) {
        iir_frame = (int)(pAtmoCtx->AtmoConfig.global.iir);
        int iir_frame_real = MIN(pAtmoCtx->frameCnt + 1, iir_frame);

        float PrevLgMean = pAtmoCtx->PrevData.ro_hdrtmo_lgmean / 2048.0;
        float CurrLgMean = pAtmoCtx->CurrData.CtrlData.LgMean;

        CurrLgMean *= 1 + 0.5 - pAtmoCtx->CurrData.HandleData.GlobalTmoStrength;
        value_default *= 1 + 0.5 - pAtmoCtx->CurrData.HandleData.GlobalTmoStrength;

        value = pAtmoCtx->frameCnt == 0 ? value_default :
                (iir_frame_real - 1) * PrevLgMean / iir_frame_real + CurrLgMean / iir_frame_real;
        returnValue = (int)SHIFT11BIT(value) ;
    }
    else
        returnValue = value_default;

    LOG1_ATMO( "%s: frameCnt:%d iir_frame:%d set_lgmean_float:%f set_lgmean_return:%d\n", __FUNCTION__,
               pAtmoCtx->frameCnt, iir_frame, value, returnValue);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);

    return returnValue;
}
/******************************************************************************
 * GetSetLgAvgMax()
 *****************************************************************************/
unsigned short GetSetLgAvgMax(AtmoHandle_t pAtmoCtx, float set_lgmin, float set_lgmax)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    float WeightKey = pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_weightkey / 256.0;
    float value = 0.0;
    float set_lgmean = pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmean / 2048.0;
    float lgrange1 = pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgrange1 / 2048.0;
    unsigned short returnValue;

    value = WeightKey * set_lgmax + (1 - WeightKey) * set_lgmean;
    value = MIN(value, lgrange1);
    returnValue = (int)SHIFT11BIT(value);

    LOG1_ATMO( "%s: set_lgmin:%f set_lgmax:%f set_lgmean:%f lgrange1:%f value:%f returnValue:%d\n", __FUNCTION__, set_lgmin, set_lgmax, set_lgmean, lgrange1, value, returnValue);

    return returnValue;
    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);

}
/******************************************************************************
 * GetSetLgRange0()
 *****************************************************************************/
unsigned short GetSetLgRange0(AtmoHandle_t pAtmoCtx, float set_lgmin, float set_lgmax)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    float value = 0.0;
    float clipratio0 = (float)(pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipratio0) / 256.0;
    float clipgap0 = pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipgap0 / 4.0;
    unsigned short returnValue;


    value = set_lgmin * (1 - clipratio0) + set_lgmax * clipratio0;
    value = MIN(value, (set_lgmin + clipgap0));
    returnValue = (int)SHIFT11BIT(value);

    LOG1_ATMO( "%s: set_lgmin:%f set_lgmax:%f clipratio0:%f clipgap0:%f value:%f returnValue:%d\n", __FUNCTION__, set_lgmin, set_lgmax, clipratio0, clipgap0, value, returnValue);

    return returnValue;
    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);

}
/******************************************************************************
 * GetSetLgRange1()
 *****************************************************************************/
unsigned short GetSetLgRange1(AtmoHandle_t pAtmoCtx, float set_lgmin, float set_lgmax)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    float value = 0.0;
    float clipratio1 = pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipratio1 / 256.0;
    float clipgap1 = pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipgap1 / 4.0;
    unsigned short returnValue;

    value = set_lgmin * (1 - clipratio1) + set_lgmax * clipratio1;
    value = MAX(value, (set_lgmax - clipgap1));
    returnValue = (int)SHIFT11BIT(value);

    LOG1_ATMO( "%s: set_lgmin:%f set_lgmax:%f clipratio1:%f clipgap1:%f value:%f returnValue:%d\n", __FUNCTION__, set_lgmin, set_lgmax, clipratio1, clipgap1, value, returnValue);

    return returnValue;
    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);

}
/******************************************************************************
 * GetSetPalhpa()
 *****************************************************************************/
unsigned short GetSetPalhpa(AtmoHandle_t pAtmoCtx, float set_lgmin, float set_lgmax)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);

    float index = 0.0;
    float value = 0.0;
    float set_lgmean = pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmean / 2048.0;
    float palpha_0p18 = pAtmoCtx->ProcRes.Res.sw_hdrtmo_palpha_0p18 / 1024.0;
    int value_int = 0;
    unsigned short returnValue;

    index = 2 * set_lgmean - set_lgmin - set_lgmax;
    index = index / (set_lgmax - set_lgmin);
    value = palpha_0p18 * pow(4, index);
    value_int = (int)SHIFT10BIT(value);
    value_int = MIN(value_int, pAtmoCtx->ProcRes.Res.sw_hdrtmo_maxpalpha);
    returnValue = value_int;

    LOG1_ATMO( "%s: set_lgmin:%f set_lgmax:%f set_lgmean:%f palpha_0p18:%f value:%f returnValue:%d\n", __FUNCTION__, set_lgmin, set_lgmax, set_lgmean, palpha_0p18, value, returnValue);

    return returnValue;
    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);

}

/******************************************************************************
 * GetCurrIOData()
 *****************************************************************************/
void TmoGetCurrIOData
(
    AtmoHandle_t pAtmoCtx
)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);

    //default IO data
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipratio0 = 64;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipratio1 = 166;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipgap0 = 12;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipgap1 = 12;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_ratiol = 32;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_high = (int)(pAtmoCtx->height * pAtmoCtx->width * 0.01 / 16);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_min = 0;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_low = 0;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_0p3 = 0;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_shift = 3;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_gain_ld_off1 = 10;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_gain_ld_off2 = 5;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_newhist_en = 1;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_cnt_mode = 1;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_cnt_vsize = (int)(pAtmoCtx->height - 256);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_cfg_alpha = 255;

    //IO data from IQ
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_maxpalpha = (int)(pAtmoCtx->CurrData.HandleData.GlobeMaxLuma + 0.5);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_palpha_0p18 =  (int)(pAtmoCtx->CurrData.HandleData.GlobeLuma + 0.5);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_palpha_lw0p5 = (int)(pAtmoCtx->CurrData.HandleData.DetailsHighLight + 0.5);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_weightkey = (int)(pAtmoCtx->CurrData.HandleData.LocalTmoStrength + 0.5);

    float lwscl = pAtmoCtx->CurrData.HandleData.DetailsLowLight;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_palpha_lwscl = (int)(lwscl + 0.5);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_maxgain = (int)(SHIFT12BIT(lwscl / 16) + 0.5);


    //calc other IO data
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_big_en = pAtmoCtx->width > BIGMODE ? 1 : 0;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_nobig_en = (int)(1 - pAtmoCtx->ProcRes.Res.sw_hdrtmo_big_en);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_expl_lgratio = (int)SHIFT11BIT(log(pAtmoCtx->PrevData.CtrlData.LExpo / pAtmoCtx->CurrData.CtrlData.LExpo) / log(2));
    if(pAtmoCtx->AtmoConfig.isLinearTmo)
        pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgscl_ratio = 128;
    else
        pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgscl_ratio = (int)SHIFT7BIT(log(pAtmoCtx->CurrData.CtrlData.L2S_Ratio) / log(pAtmoCtx->PrevData.CtrlData.L2S_ratio));
    float lgmax = 12 + log(pAtmoCtx->CurrData.CtrlData.L2S_Ratio) / log(2);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgmax = (int)SHIFT11BIT(lgmax);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgscl = (int)SHIFT12BIT(16 / lgmax);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgscl_inv = (int)SHIFT12BIT(lgmax / 16);
    float set_lgmin = 0;
    float set_lgmax = lgmax;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmin = (int)SHIFT11BIT(set_lgmin) ;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmax = pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgmax;
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_gainoff = pow(2, set_lgmin);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmean = GetSetLgmean(pAtmoCtx);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgrange0 = GetSetLgRange0(pAtmoCtx, set_lgmin, set_lgmax);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgrange1 = GetSetLgRange1(pAtmoCtx, set_lgmin, set_lgmax);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgavgmax = GetSetLgAvgMax(pAtmoCtx, set_lgmin, set_lgmax);
    pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_palpha = GetSetPalhpa(pAtmoCtx, set_lgmin, set_lgmax);


    //for avoid tmo flicker
    pAtmoCtx->ProcRes.TmoFlicker.cnt_mode = pAtmoCtx->ProcRes.Res.sw_hdrtmo_cnt_mode;
    pAtmoCtx->ProcRes.TmoFlicker.cnt_vsize = pAtmoCtx->ProcRes.Res.sw_hdrtmo_cnt_vsize;
    if(pAtmoCtx->CurrData.HandleData.GlobalTmoStrength > 0.5)
        pAtmoCtx->ProcRes.TmoFlicker.GlobalTmoStrengthDown = false;
    else
        pAtmoCtx->ProcRes.TmoFlicker.GlobalTmoStrengthDown = true;
    pAtmoCtx->ProcRes.TmoFlicker.GlobalTmoStrength = pAtmoCtx->CurrData.HandleData.GlobalTmoStrength - 0.5;
    pAtmoCtx->ProcRes.TmoFlicker.GlobalTmoStrength = pAtmoCtx->ProcRes.TmoFlicker.GlobalTmoStrength < 0 ? (1 - pAtmoCtx->ProcRes.TmoFlicker.GlobalTmoStrength)
            : (1 + pAtmoCtx->ProcRes.TmoFlicker.GlobalTmoStrength);
    pAtmoCtx->ProcRes.TmoFlicker.iir = (int)(pAtmoCtx->AtmoConfig.global.iir) +
                                       3 * pAtmoCtx->CurrAeResult.AecDelayframe;
    pAtmoCtx->ProcRes.TmoFlicker.iirmax = IIRMAX;
    pAtmoCtx->ProcRes.TmoFlicker.height = pAtmoCtx->height;
    pAtmoCtx->ProcRes.TmoFlicker.width = pAtmoCtx->width;
    pAtmoCtx->ProcRes.TmoFlicker.PredictK.correction_factor = 1.05;
    pAtmoCtx->ProcRes.TmoFlicker.PredictK.correction_offset = 0;
    pAtmoCtx->ProcRes.TmoFlicker.PredictK.Hdr3xLongPercent = 0.5;
    pAtmoCtx->ProcRes.TmoFlicker.PredictK.UseLongLowTh = 1.02;
    pAtmoCtx->ProcRes.TmoFlicker.PredictK.UseLongUpTh = 0.98;
    if(pAtmoCtx->FrameNumber == LINEAR_NUM)
        pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[0] = pAtmoCtx->CurrAeResult.LumaDeviationLinear;
    else if(pAtmoCtx->FrameNumber == HDR_2X_NUM) {
        pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[0] = pAtmoCtx->CurrAeResult.LumaDeviationS;
        pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[1] = pAtmoCtx->CurrAeResult.LumaDeviationL;
    }
    else if(pAtmoCtx->FrameNumber == HDR_3X_NUM) {
        pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[0] = pAtmoCtx->CurrAeResult.LumaDeviationS;
        pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[1] = pAtmoCtx->CurrAeResult.LumaDeviationM;
        pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[2] = pAtmoCtx->CurrAeResult.LumaDeviationL;
    }
    pAtmoCtx->ProcRes.TmoFlicker.StableThr = 0.1;

    LOGV_ATMO("%s:  Tmo set IOdata to register:\n", __FUNCTION__);
    LOGV_ATMO("%s:  float lgmax:%f\n", __FUNCTION__, lgmax);
    LOGV_ATMO("%s:  sw_hdrtmo_lgmax:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgmax);
    LOGV_ATMO("%s:  sw_hdrtmo_lgscl:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgscl);
    LOGV_ATMO("%s:  sw_hdrtmo_lgscl_inv:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgscl_inv);
    LOGV_ATMO("%s:  sw_hdrtmo_clipratio0:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipratio0);
    LOGV_ATMO("%s:  sw_hdrtmo_clipratio1:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipratio1);
    LOGV_ATMO("%s:  sw_hdrtmo_clipgap0:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipgap0);
    LOGV_ATMO("%s:  sw_hdrtmo_clipgap:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_clipgap1);
    LOGV_ATMO("%s:  sw_hdrtmo_ratiol:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_ratiol);
    LOGV_ATMO("%s:  sw_hdrtmo_hist_min:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_min);
    LOGV_ATMO("%s:  sw_hdrtmo_hist_low:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_low);
    LOGV_ATMO("%s:  sw_hdrtmo_hist_high:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_high);
    LOGV_ATMO("%s:  sw_hdrtmo_hist_0p3:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_0p3);
    LOGV_ATMO("%s:  sw_hdrtmo_hist_shift:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_hist_shift);
    LOGV_ATMO("%s:  sw_hdrtmo_palpha_0p18:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_palpha_0p18);
    LOGV_ATMO("%s:  sw_hdrtmo_palpha_lw0p5:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_palpha_lw0p5);
    LOGV_ATMO("%s:  sw_hdrtmo_palpha_lwscl:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_palpha_lwscl);
    LOGV_ATMO("%s:  sw_hdrtmo_maxpalpha:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_maxpalpha);
    LOGV_ATMO("%s:  sw_hdrtmo_maxgain:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_maxgain);
    LOGV_ATMO("%s:  sw_hdrtmo_cfg_alpha:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_cfg_alpha);
    LOGV_ATMO("%s:  sw_hdrtmo_set_gainoff:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_gainoff);
    LOGV_ATMO("%s:  sw_hdrtmo_set_lgmin:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmin);
    LOGV_ATMO("%s:  sw_hdrtmo_set_lgmax:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmax);
    LOGV_ATMO("%s:  sw_hdrtmo_set_lgmean:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmean);
    LOGV_ATMO("%s:  sw_hdrtmo_set_weightkey:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_weightkey);
    LOGV_ATMO("%s:  sw_hdrtmo_set_lgrange0:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgrange0);
    LOGV_ATMO("%s:  sw_hdrtmo_set_lgrange1:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgrange1);
    LOGV_ATMO("%s:  sw_hdrtmo_set_lgavgmax:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgavgmax);
    LOGV_ATMO("%s:  sw_hdrtmo_set_palpha:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_palpha);
    LOGV_ATMO("%s:  sw_hdrtmo_big_en:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_big_en);
    LOGV_ATMO("%s:  sw_hdrtmo_nobig_en:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_nobig_en);
    LOGV_ATMO("%s:  sw_hdrtmo_newhist_en:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_newhist_en);
    LOGV_ATMO("%s:  sw_hdrtmo_cnt_mode:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_cnt_mode);
    LOGV_ATMO("%s:  sw_hdrtmo_cnt_vsize:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_cnt_vsize);
    LOGV_ATMO("%s:  sw_hdrtmo_expl_lgratio:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_expl_lgratio);
    LOGV_ATMO("%s:  sw_hdrtmo_lgscl_ratio:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_lgscl_ratio);
    LOGV_ATMO("%s:  sw_hdrtmo_gain_ld_off1:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_gain_ld_off1);
    LOGV_ATMO("%s: sw_hdrtmo_gain_ld_off2:%d\n", __FUNCTION__, pAtmoCtx->ProcRes.Res.sw_hdrtmo_gain_ld_off2);
    LOGV_ATMO("%s: LumaDeviation:%f %f %f\n", __FUNCTION__, pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[0],
              pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[1], pAtmoCtx->ProcRes.TmoFlicker.LumaDeviation[2]);

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * TmoDamp()
 *****************************************************************************/
void TmoDamp
(
    AtmoHandle_t pAtmoCtx
)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    float tmo_damp = pAtmoCtx->CurrData.CtrlData.TmoDamp;

    bool enDampLuma;
    bool enDampDtlsHighLgt;
    bool enDampDtlslowLgt;
    bool enDampLocal;
    bool enDampGlobal;
    bool ifHDRModeChange = pAtmoCtx->CurrData.CtrlData.FrameCnt == pAtmoCtx->PrevData.CtrlData.FrameCnt ? false : true;

    if(pAtmoCtx->tmoAttr.opMode == TMO_OPMODE_API_OFF && pAtmoCtx->frameCnt != 0 && !ifHDRModeChange)
    {
        float diff = 0;

        if(pAtmoCtx->AtmoConfig.Luma.globalLumaMode == GLOBALLUMAMODE_ENVLV)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.EnvLv - pAtmoCtx->PrevData.CtrlData.EnvLv);
            diff = diff / pAtmoCtx->PrevData.CtrlData.EnvLv;
        }
        else if(pAtmoCtx->AtmoConfig.Luma.globalLumaMode == GLOBALLUMAMODE_ISO)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.ISO - pAtmoCtx->PrevData.CtrlData.ISO);
            diff = diff / pAtmoCtx->PrevData.CtrlData.ISO;
        }
        if (diff < pAtmoCtx->AtmoConfig.Luma.Tolerance)
            enDampLuma = false;
        else
            enDampLuma = true;

        if(pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode == DETAILSHIGHLIGHTMODE_OEPDF)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.OEPdf - pAtmoCtx->PrevData.CtrlData.OEPdf);
            diff = diff / pAtmoCtx->PrevData.CtrlData.OEPdf;
        }
        else if(pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode == DETAILSHIGHLIGHTMODE_ENVLV)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.EnvLv - pAtmoCtx->PrevData.CtrlData.EnvLv);
            diff = diff / pAtmoCtx->PrevData.CtrlData.EnvLv;
        }
        if (diff < pAtmoCtx->AtmoConfig.DtsHiLit.Tolerance)
            enDampDtlsHighLgt = false;
        else
            enDampDtlsHighLgt = true;

        if(pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode == DETAILSLOWLIGHTMODE_FOCUSLUMA)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.FocusLuma - pAtmoCtx->PrevData.CtrlData.FocusLuma);
            diff = diff / pAtmoCtx->PrevData.CtrlData.FocusLuma;
        }
        else if(pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode == DETAILSLOWLIGHTMODE_DARKPDF)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.DarkPdf - pAtmoCtx->PrevData.CtrlData.DarkPdf);
            diff = diff / pAtmoCtx->PrevData.CtrlData.DarkPdf;
        }
        else if(pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode == DETAILSLOWLIGHTMODE_ISO)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.ISO - pAtmoCtx->PrevData.CtrlData.ISO);
            diff = diff / pAtmoCtx->PrevData.CtrlData.ISO;
        }
        if (diff < pAtmoCtx->AtmoConfig.DtsLoLit.Tolerance)
            enDampDtlslowLgt = false;
        else
            enDampDtlslowLgt = true;

        if(pAtmoCtx->AtmoConfig.local.localtmoMode == TMOTYPEMODE_DYNAMICRANGE) {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.DynamicRange - pAtmoCtx->PrevData.CtrlData.DynamicRange);
            diff = diff / pAtmoCtx->PrevData.CtrlData.DynamicRange;
        }
        else if(pAtmoCtx->AtmoConfig.local.localtmoMode == TMOTYPEMODE_ENVLV)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.EnvLv - pAtmoCtx->PrevData.CtrlData.EnvLv);
            diff = diff / pAtmoCtx->PrevData.CtrlData.EnvLv;
        }
        if (diff < pAtmoCtx->AtmoConfig.local.Tolerance)
            enDampLocal = false;
        else
            enDampLocal = true;

        if(pAtmoCtx->AtmoConfig.global.mode == TMOTYPEMODE_DYNAMICRANGE) {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.DynamicRange - pAtmoCtx->PrevData.CtrlData.DynamicRange);
            diff = diff / pAtmoCtx->PrevData.CtrlData.DynamicRange;
        }
        else if(pAtmoCtx->AtmoConfig.global.mode == TMOTYPEMODE_ENVLV)
        {
            diff = ABS(pAtmoCtx->CurrData.CtrlData.EnvLv - pAtmoCtx->PrevData.CtrlData.EnvLv);
            diff = diff / pAtmoCtx->PrevData.CtrlData.EnvLv;
        }
        if (diff < pAtmoCtx->AtmoConfig.global.Tolerance)
            enDampGlobal = false;
        else
            enDampGlobal = true;


        //get finnal cfg data by damp
        if (enDampLuma == true)
        {
            pAtmoCtx->CurrData.HandleData.GlobeMaxLuma = tmo_damp * pAtmoCtx->CurrData.HandleData.GlobeMaxLuma
                    + (1 - tmo_damp) * pAtmoCtx->PrevData.HandleData.GlobeMaxLuma;
            pAtmoCtx->CurrData.HandleData.GlobeLuma = tmo_damp * pAtmoCtx->CurrData.HandleData.GlobeLuma
                    + (1 - tmo_damp) * pAtmoCtx->PrevData.HandleData.GlobeLuma;
        }

        if (enDampDtlsHighLgt == true)
            pAtmoCtx->CurrData.HandleData.DetailsHighLight = tmo_damp * pAtmoCtx->CurrData.HandleData.DetailsHighLight
                    + (1 - tmo_damp) * pAtmoCtx->PrevData.HandleData.DetailsHighLight;

        if (enDampDtlslowLgt == true)
            pAtmoCtx->CurrData.HandleData.DetailsLowLight = tmo_damp * pAtmoCtx->CurrData.HandleData.DetailsLowLight
                    + (1 - tmo_damp) * pAtmoCtx->PrevData.HandleData.DetailsLowLight;

        if (enDampLocal == true)
            pAtmoCtx->CurrData.HandleData.LocalTmoStrength = tmo_damp * pAtmoCtx->CurrData.HandleData.LocalTmoStrength
                    + (1 - tmo_damp) * pAtmoCtx->PrevData.HandleData.LocalTmoStrength;

        if (enDampGlobal == true)
            pAtmoCtx->CurrData.HandleData.GlobalTmoStrength = tmo_damp * pAtmoCtx->CurrData.HandleData.GlobalTmoStrength
                    + (1 - tmo_damp) * pAtmoCtx->PrevData.HandleData.GlobalTmoStrength;

    }

    LOGD_ATMO("%s:Current damp GlobeLuma:%f GlobeMaxLuma:%f DetailsHighLight:%f DetailsLowLight:%f LocalTmoStrength:%f GlobalTmoStrength:%f\n", __FUNCTION__, pAtmoCtx->CurrData.HandleData.GlobeLuma
              , pAtmoCtx->CurrData.HandleData.GlobeMaxLuma, pAtmoCtx->CurrData.HandleData.DetailsHighLight, pAtmoCtx->CurrData.HandleData.DetailsLowLight
              , pAtmoCtx->CurrData.HandleData.LocalTmoStrength, pAtmoCtx->CurrData.HandleData.GlobalTmoStrength);

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}
/******************************************************************************
 * SetGlobalTMO()
 *****************************************************************************/
bool SetGlobalTMO
(
    AtmoHandle_t pAtmoCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    bool returnValue = false;

    if(pAtmoCtx->AtmoConfig.global.isHdrGlobalTmo == true) {
        returnValue = true;
        pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_weightkey = 0;
    }

    else
        returnValue = false;

    LOGD_ATMO("%s: set GlobalTMO:%d\n", __FUNCTION__, returnValue);

    return returnValue;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AhdrGetProcRes()
 *****************************************************************************/
void AtmoGetProcRes
(
    AtmoHandle_t pAtmoCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    //tmo para damp
    TmoDamp(pAtmoCtx);

    //tmo enable
    pAtmoCtx->ProcRes.bTmoEn = pAtmoCtx->AtmoConfig.bTmoEn;
    pAtmoCtx->ProcRes.isLinearTmo = pAtmoCtx->AtmoConfig.isLinearTmo;

    //get tmo proc res
    TmoGetCurrIOData(pAtmoCtx);

    // Set Global TMO
    pAtmoCtx->ProcRes.isHdrGlobalTmo = SetGlobalTMO(pAtmoCtx);

    // store current handle data to pre data for next loop
    pAtmoCtx->PrevData.HandleData.MergeMode = pAtmoCtx->CurrData.HandleData.MergeMode;
    pAtmoCtx->PrevData.ro_hdrtmo_lgmean = pAtmoCtx->ProcRes.Res.sw_hdrtmo_set_lgmean;
    pAtmoCtx->PrevData.CtrlData.L2S_ratio = pAtmoCtx->CurrData.CtrlData.L2S_Ratio;
    pAtmoCtx->PrevData.CtrlData.LExpo = pAtmoCtx->CurrData.CtrlData.LExpo;
    pAtmoCtx->PrevData.CtrlData.EnvLv = pAtmoCtx->CurrData.CtrlData.EnvLv;
    pAtmoCtx->PrevData.CtrlData.OEPdf = pAtmoCtx->CurrData.CtrlData.OEPdf;
    pAtmoCtx->PrevData.CtrlData.FocusLuma = pAtmoCtx->CurrData.CtrlData.FocusLuma;
    pAtmoCtx->PrevData.CtrlData.DarkPdf = pAtmoCtx->CurrData.CtrlData.DarkPdf;
    pAtmoCtx->PrevData.CtrlData.ISO = pAtmoCtx->CurrData.CtrlData.ISO;
    pAtmoCtx->PrevData.CtrlData.DynamicRange = pAtmoCtx->CurrData.CtrlData.DynamicRange;
    memcpy(&pAtmoCtx->PrevData.HandleData, &pAtmoCtx->CurrData.HandleData, sizeof(TmoHandleData_s));
    ++pAtmoCtx->frameCnt;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AtmoProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AtmoProcessing
(
    AtmoHandle_t     pAtmoCtx,
    AecPreResult_t  AecHdrPreResult,
    af_preprocess_result_t AfPreResult
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    LOGD_ATMO("%s:  Atmo Current frame cnt:%d:\n",  __FUNCTION__, pAtmoCtx->frameCnt);
    pAtmoCtx->CurrData.CtrlData.FrameCnt = pAtmoCtx->frameCnt;

    //get current ae data from AecPreRes
    AtmoGetAeResult(pAtmoCtx, AecHdrPreResult);

    //transfer ae data to CurrHandle
    pAtmoCtx->CurrData.CtrlData.EnvLv = LIMIT_VALUE(pAtmoCtx->CurrData.CtrlData.EnvLv, ENVLVMAX, ENVLVMIN);

    pAtmoCtx->CurrData.CtrlData.ISO = pAtmoCtx->CurrAeResult.ISO;
    pAtmoCtx->CurrData.CtrlData.ISO = LIMIT_VALUE(pAtmoCtx->CurrData.CtrlData.ISO, ISOMAX, ISOMIN);

    pAtmoCtx->CurrData.CtrlData.OEPdf = pAtmoCtx->CurrAeResult.OEPdf;
    pAtmoCtx->CurrData.CtrlData.OEPdf = LIMIT_VALUE(pAtmoCtx->CurrData.CtrlData.OEPdf, OEPDFMAX, OEPDFMIN);

    pAtmoCtx->CurrData.CtrlData.FocusLuma = 1;
    pAtmoCtx->CurrData.CtrlData.FocusLuma = LIMIT_VALUE(pAtmoCtx->CurrData.CtrlData.FocusLuma, FOCUSLUMAMAX, FOCUSLUMAMIN);

    pAtmoCtx->CurrData.CtrlData.DarkPdf = pAtmoCtx->CurrAeResult.DarkPdf;
    pAtmoCtx->CurrData.CtrlData.DarkPdf = LIMIT_VALUE(pAtmoCtx->CurrData.CtrlData.DarkPdf, DARKPDFTHMAX, DARKPDFTHMIN);

    pAtmoCtx->CurrData.CtrlData.DynamicRange = pAtmoCtx->CurrAeResult.DynamicRange;
    pAtmoCtx->CurrData.CtrlData.DynamicRange = LIMIT_VALUE(pAtmoCtx->CurrData.CtrlData.DynamicRange, DYNAMICRANGEMAX, DYNAMICRANGEMIN);

    if(pAtmoCtx->tmoAttr.opMode == TMO_OPMODE_API_OFF) {
        LOGD_ATMO("%s:  Atmo api OFF!! Current Handle data:\n", __FUNCTION__);

        ApiOffProcess(pAtmoCtx);

        //log after updating
        LOGD_ATMO("%s:  GlobalLumaMode:%f CtrlData.EnvLv:%f CtrlData.ISO:%f GlobeLuma:%f GlobeMaxLuma:%f \n", __FUNCTION__,  pAtmoCtx->AtmoConfig.Luma.globalLumaMode,
                  pAtmoCtx->CurrData.CtrlData.EnvLv, pAtmoCtx->CurrData.CtrlData.ISO, pAtmoCtx->CurrData.HandleData.GlobeLuma, pAtmoCtx->CurrData.HandleData.GlobeMaxLuma);
        LOGD_ATMO("%s:  DetailsHighLightMode:%f CtrlData.OEPdf:%f CtrlData.EnvLv:%f DetailsHighLight:%f\n", __FUNCTION__, pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode, pAtmoCtx->CurrData.CtrlData.OEPdf
                  , pAtmoCtx->CurrData.CtrlData.EnvLv, pAtmoCtx->CurrData.HandleData.DetailsHighLight);
        LOGD_ATMO("%s:  DetailsLowLightMode:%f CtrlData.FocusLuma:%f CtrlData.DarkPdf:%f CtrlData.ISO:%f DetailsLowLight:%f\n", __FUNCTION__, pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode,
                  pAtmoCtx->CurrData.CtrlData.FocusLuma, pAtmoCtx->CurrData.CtrlData.DarkPdf, pAtmoCtx->CurrData.CtrlData.ISO, pAtmoCtx->CurrData.HandleData.DetailsLowLight);
        LOGD_ATMO("%s:  localtmoMode:%f CtrlData.DynamicRange:%f CtrlData.EnvLv:%f LocalTmoStrength:%f\n", __FUNCTION__,  pAtmoCtx->AtmoConfig.local.localtmoMode, pAtmoCtx->CurrData.CtrlData.DynamicRange,
                  pAtmoCtx->CurrData.CtrlData.EnvLv, pAtmoCtx->CurrData.HandleData.LocalTmoStrength);
        LOGD_ATMO("%s:  GlobalTMO en:%d mode:%f CtrlData.DynamicRange:%f CtrlData.EnvLv:%f Strength:%f\n", __FUNCTION__,  pAtmoCtx->AtmoConfig.global.isHdrGlobalTmo, pAtmoCtx->AtmoConfig.global.mode, pAtmoCtx->CurrData.CtrlData.DynamicRange,
                  pAtmoCtx->CurrData.CtrlData.EnvLv, pAtmoCtx->CurrData.HandleData.GlobalTmoStrength);

    }
    else if(pAtmoCtx->tmoAttr.opMode == TMO_OPMODE_AUTO) {
        LOGD_ATMO("%s:  Atmo api Auto!! Current Handle data:\n", __FUNCTION__);
        AtmoApiAutoUpdate(pAtmoCtx);
    }
    else if(pAtmoCtx->tmoAttr.opMode == TMO_OPMODE_MANU) {
        LOGD_ATMO("%s:  Atmo api Manual!! Current Handle data:\n", __FUNCTION__);
        AtmoApiManualUpdate(pAtmoCtx);
    }
    else if(pAtmoCtx->tmoAttr.opMode == TMO_OPMODE_SET_LEVEL) {
        LOGD_ATMO("%s:  Atmo api set level!! Current Handle data:\n", __FUNCTION__);

        ApiOffProcess(pAtmoCtx);
        AtmoApiSetLevel(pAtmoCtx);
    }
    else if(pAtmoCtx->tmoAttr.opMode == TMO_OPMODE_DARKAREA) {
        LOGD_ATMO("%s:  Atmo api DarkArea!! Current Handle data:\n", __FUNCTION__);
        ApiOffProcess(pAtmoCtx);

        pAtmoCtx->AtmoConfig.bTmoEn = true;
        pAtmoCtx->AtmoConfig.isLinearTmo = pAtmoCtx->FrameNumber == LINEAR_NUM;
        pAtmoCtx->CurrData.HandleData.DetailsLowLight *= 1 + (float)(pAtmoCtx->tmoAttr.stDarkArea.level) * 0.4;
        pAtmoCtx->CurrData.HandleData.DetailsLowLight =
            LIMIT_VALUE(pAtmoCtx->CurrData.HandleData.DetailsLowLight, DETAILSLOWLIGHTMAX, DETAILSLOWLIGHTMIN);
        LOGD_ATMO("%s: Linear TMO en:%d DetailsLowLightMode:%f CtrlData.FocusLuma:%f CtrlData.DarkPdf:%f CtrlData.ISO:%f DetailsLowLight:%f\n", __FUNCTION__, pAtmoCtx->ProcRes.isLinearTmo,
                  pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode, pAtmoCtx->CurrData.CtrlData.FocusLuma, pAtmoCtx->CurrData.CtrlData.DarkPdf,
                  pAtmoCtx->CurrData.CtrlData.ISO, pAtmoCtx->CurrData.HandleData.DetailsLowLight);
    }
    else if(pAtmoCtx->tmoAttr.opMode == TMO_OPMODE_TOOL) {
        LOGD_ATMO("%s:  Atmo api Tool!! Current Handle data:\n", __FUNCTION__);
        ApiOffProcess(pAtmoCtx);

        //tmo en
        pAtmoCtx->AtmoConfig.bTmoEn = pAtmoCtx->AtmoConfig.bTmoEn;
        pAtmoCtx->AtmoConfig.isLinearTmo = pAtmoCtx->AtmoConfig.bTmoEn && pAtmoCtx->FrameNumber == LINEAR_NUM;

        //log after updating
        LOGD_ATMO("%s:  GlobalLumaMode:%f CtrlData.EnvLv:%f CtrlData.ISO:%f GlobeLuma:%f GlobeMaxLuma:%f \n", __FUNCTION__,  pAtmoCtx->AtmoConfig.Luma.globalLumaMode,
                  pAtmoCtx->CurrData.CtrlData.EnvLv, pAtmoCtx->CurrData.CtrlData.ISO, pAtmoCtx->CurrData.HandleData.GlobeLuma, pAtmoCtx->CurrData.HandleData.GlobeMaxLuma);
        LOGD_ATMO("%s:  DetailsHighLightMode:%f CtrlData.OEPdf:%f CtrlData.EnvLv:%f DetailsHighLight:%f\n", __FUNCTION__, pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLightMode, pAtmoCtx->CurrData.CtrlData.OEPdf
                  , pAtmoCtx->CurrData.CtrlData.EnvLv, pAtmoCtx->CurrData.HandleData.DetailsHighLight);
        LOGD_ATMO("%s:  DetailsLowLightMode:%f CtrlData.FocusLuma:%f CtrlData.DarkPdf:%f CtrlData.ISO:%f DetailsLowLight:%f\n", __FUNCTION__, pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLightMode,
                  pAtmoCtx->CurrData.CtrlData.FocusLuma, pAtmoCtx->CurrData.CtrlData.DarkPdf, pAtmoCtx->CurrData.CtrlData.ISO, pAtmoCtx->CurrData.HandleData.DetailsLowLight);
        LOGD_ATMO("%s:  localtmoMode:%f CtrlData.DynamicRange:%f CtrlData.EnvLv:%f LocalTmoStrength:%f\n", __FUNCTION__,  pAtmoCtx->AtmoConfig.local.localtmoMode, pAtmoCtx->CurrData.CtrlData.DynamicRange,
                  pAtmoCtx->CurrData.CtrlData.EnvLv, pAtmoCtx->CurrData.HandleData.LocalTmoStrength);
        LOGD_ATMO("%s:  GlobalTMO en:%d mode:%f CtrlData.DynamicRange:%f CtrlData.EnvLv:%f Strength:%f\n", __FUNCTION__,  pAtmoCtx->AtmoConfig.global.isHdrGlobalTmo, pAtmoCtx->AtmoConfig.global.mode, pAtmoCtx->CurrData.CtrlData.DynamicRange,
                  pAtmoCtx->CurrData.CtrlData.EnvLv, pAtmoCtx->CurrData.HandleData.GlobalTmoStrength);
    }
    else
        LOGE_ATMO("%s:  Atmo wrong mode!!!\n", __FUNCTION__);

    //transfer data to api
    AtmoTranferData2Api(pAtmoCtx);

    //read current rodata
    pAtmoCtx->CurrData.CtrlData.LgMean = pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgmean / 2048.0;

    //calc the current merge luma
    float MergeLuma = (float)pAtmoCtx->CurrStatsData.tmo_stats.ro_hdrtmo_lgmean;
    MergeLuma = MergeLuma / 2048.0;
    float lgmean = MergeLuma;
    MergeLuma = pow(2, MergeLuma);
    MergeLuma =  MergeLuma / 16;

    //get pre frame tmo mean luma
    unsigned long tmo_mean = 0;
    for(int i = 0; i < 225; i++) {
        tmo_mean += pAtmoCtx->CurrStatsData.other_stats.tmo_luma[i];
    }
    tmo_mean = tmo_mean / 225;
    tmo_mean = tmo_mean / 16;

    //calc short middle long frame mean luma
    unsigned long short_mean = 0, middle_mean = 0, long_mean = 0;
    for (int i = 0; i < 225; i++) {
        short_mean += pAtmoCtx->CurrAeResult.BlockLumaS[i];
        long_mean += pAtmoCtx->CurrAeResult.BlockLumaL[i];
    }
    short_mean = short_mean / 225;
    long_mean = long_mean / 225;
    short_mean = short_mean / 16;
    long_mean = long_mean / 16;

    for(int i = 0; i < 25; i++)
        middle_mean += pAtmoCtx->CurrAeResult.BlockLumaM[i];
    middle_mean = middle_mean / 25;
    middle_mean = middle_mean / 16;

    LOGD_ATMO("%s:  preFrame lgMergeLuma:%f MergeLuma(8bit):%f TmoLuma(8bit):%d\n", __FUNCTION__, lgmean, MergeLuma, tmo_mean);
    LOGD_ATMO("%s:  preFrame SLuma(8bit):%d MLuma(8bit):%d LLuma(8bit):%d\n", __FUNCTION__, short_mean, middle_mean, long_mean);

    //get tmo proc res
    AtmoGetProcRes(pAtmoCtx);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AtmoInit()
 *****************************************************************************/
RESULT AtmoInit
(
    AtmoInstanceConfig_t* pInstConfig,
    CamCalibDbV2Context_t* pCalibV2
) {

    AtmoContext_s *pAtmoCtx;

    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    RESULT result = ATMO_RET_SUCCESS;

    // initial checks
    if (pInstConfig == NULL)
        return (ATMO_RET_INVALID_PARM);

    // allocate ATMO control context
    pAtmoCtx = (AtmoContext_s*)malloc(sizeof(AtmoContext_s));
    if (NULL == pAtmoCtx) {
        LOGE_ATMO( "%s: Can't allocate ATMO context\n",  __FUNCTION__);
        return (ATMO_RET_OUTOFMEM);
    }

    CalibDbV2_tmo_t* calibv2_atmo_calib =
        (CalibDbV2_tmo_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibV2, atmo_calib));

    // pre-initialize context
    memset(pAtmoCtx, 0x00, sizeof(*pAtmoCtx));
    pAtmoCtx->AtmoConfig.Luma.len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    pAtmoCtx->AtmoConfig.Luma.EnvLv = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len));
    pAtmoCtx->AtmoConfig.Luma.ISO = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO_len));
    pAtmoCtx->AtmoConfig.Luma.GlobeLuma = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength_len));
    pAtmoCtx->AtmoConfig.DtsHiLit.len = calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len;
    pAtmoCtx->AtmoConfig.DtsHiLit.OEPdf = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len));
    pAtmoCtx->AtmoConfig.DtsHiLit.EnvLv = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv_len));
    pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.Strength_len));
    pAtmoCtx->AtmoConfig.DtsLoLit.len = calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len;
    pAtmoCtx->AtmoConfig.DtsLoLit.FocusLuma = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len));
    pAtmoCtx->AtmoConfig.DtsLoLit.DarkPdf = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf_len));
    pAtmoCtx->AtmoConfig.DtsLoLit.ISO = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.ISO_len));
    pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.Strength_len));
    pAtmoCtx->AtmoConfig.local.len = calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len;
    pAtmoCtx->AtmoConfig.local.DynamicRange = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len));
    pAtmoCtx->AtmoConfig.local.EnvLv = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv_len));
    pAtmoCtx->AtmoConfig.local.LocalTmoStrength = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.Strength_len));
    pAtmoCtx->AtmoConfig.global.len = calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len;
    pAtmoCtx->AtmoConfig.global.DynamicRange = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len));
    pAtmoCtx->AtmoConfig.global.EnvLv = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv_len));
    pAtmoCtx->AtmoConfig.global.GlobalTmoStrength = (float*)malloc(sizeof(float) * (calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength_len));
    pAtmoCtx->state = ATMO_STATE_INITIALIZED;
    pAtmoCtx->tmoAttr.opMode = TMO_OPMODE_API_OFF;
    AtmoConfig(pAtmoCtx); //set default para
    memcpy(&pAtmoCtx->pCalibDB, calibv2_atmo_calib, sizeof(CalibDbV2_tmo_t));//load iq paras
    memcpy(&pAtmoCtx->tmoAttr.stTool, calibv2_atmo_calib, sizeof(CalibDbV2_tmo_t));//load iq paras to stTool
    pInstConfig->hAtmo = (AtmoHandle_t)pAtmoCtx;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
    return (ATMO_RET_SUCCESS);
}
/******************************************************************************
 * AtmoRelease()
 *****************************************************************************/
RESULT AtmoRelease
(
    AtmoHandle_t pAtmoCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);
    RESULT result = ATMO_RET_SUCCESS;

    // initial checks
    if (NULL == pAtmoCtx) {
        return (ATMO_RET_WRONG_HANDLE);
    }

    result = AtmoStop(pAtmoCtx);
    if (result != ATMO_RET_SUCCESS) {
        LOGE_ATMO( "%s: AHDRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ATMO_STATE_RUNNING == pAtmoCtx->state)
            || (ATMO_STATE_LOCKED == pAtmoCtx->state)) {
        return (ATMO_RET_BUSY);
    }

    free(pAtmoCtx->AtmoConfig.Luma.EnvLv);
    free(pAtmoCtx->AtmoConfig.Luma.ISO);
    free(pAtmoCtx->AtmoConfig.Luma.GlobeLuma);
    free(pAtmoCtx->AtmoConfig.DtsHiLit.OEPdf);
    free(pAtmoCtx->AtmoConfig.DtsHiLit.EnvLv);
    free(pAtmoCtx->AtmoConfig.DtsHiLit.DetailsHighLight);
    free(pAtmoCtx->AtmoConfig.DtsLoLit.FocusLuma);
    free(pAtmoCtx->AtmoConfig.DtsLoLit.DarkPdf);
    free(pAtmoCtx->AtmoConfig.DtsLoLit.ISO);
    free(pAtmoCtx->AtmoConfig.DtsLoLit.DetailsLowLight);
    free(pAtmoCtx->AtmoConfig.local.DynamicRange);
    free(pAtmoCtx->AtmoConfig.local.EnvLv);
    free(pAtmoCtx->AtmoConfig.local.LocalTmoStrength);
    free(pAtmoCtx->AtmoConfig.global.DynamicRange);
    free(pAtmoCtx->AtmoConfig.global.EnvLv);
    free(pAtmoCtx->AtmoConfig.global.GlobalTmoStrength);
    memset(pAtmoCtx, 0, sizeof(AtmoContext_s));
    free(pAtmoCtx);
    pAtmoCtx = NULL;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
    return (ATMO_RET_SUCCESS);
}

