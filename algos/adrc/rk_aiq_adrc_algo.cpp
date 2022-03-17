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
 * @file adrc.cpp
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include "math.h"
#include "rk_aiq_types_adrc_algo_int.h"
#include "xcam_log.h"
#include "rk_aiq_types_adrc_algo_prvt.h"


/******************************************************************************
 * AdrcStart()
 *****************************************************************************/
XCamReturn AdrcStart
(
    AdrcHandle_t pAdrcCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAdrcCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    if ((ADRC_STATE_RUNNING == pAdrcCtx->state)
            || (ADRC_STATE_LOCKED == pAdrcCtx->state)) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    pAdrcCtx->state = ADRC_STATE_RUNNING;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
    return (XCAM_RETURN_NO_ERROR);
}

/******************************************************************************
 * DrcGetCurrPara()
 *****************************************************************************/
float DrcGetCurrPara
(
    float           inPara,
    float*         inMatrixX,
    float*         inMatrixY,
    int Max_Knots
) {
    LOG1_AMERGE( "%s:enter!\n", __FUNCTION__);
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
    LOG1_AMERGE( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcStop()
 *****************************************************************************/
XCamReturn AdrcStop
(
    AdrcHandle_t pAdrcCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAdrcCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    // before stopping, unlock the AHDR if locked
    if (ADRC_STATE_LOCKED == pAdrcCtx->state) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    pAdrcCtx->state = ADRC_STATE_STOPPED;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
/******************************************************************************
 * DrcNewMalloc()
 ***************************************************************************/
void DrcNewMalloc
(
    AdrcConfig_t*           pConfig,
    CalibDbV2_drc_t*         pCalibDb
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pConfig != NULL);
    DCT_ASSERT(pCalibDb != NULL);
    LOG1_ATMO( "%s: Pre DrcGain len:%d, Cur DrcGain len:%d\n", __FUNCTION__, pConfig->DrcGain.len, pCalibDb->DrcTuningPara.DrcGain.EnvLv_len);
    LOG1_ATMO( "%s: Pre HiLit len:%d, Cur HiLit len:%d\n", __FUNCTION__, pConfig->HiLit.len, pCalibDb->DrcTuningPara.HiLight.EnvLv_len);
    LOG1_ATMO( "%s: Pre Local len:%d, Cur Local len:%d\n", __FUNCTION__, pConfig->Local.len, pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);

    //drc gain
    if(pConfig->DrcGain.len != pCalibDb->DrcTuningPara.DrcGain.EnvLv_len) {
        free(pConfig->DrcGain.EnvLv);
        free(pConfig->DrcGain.DrcGain);
        free(pConfig->DrcGain.Alpha);
        free(pConfig->DrcGain.Clip);
        pConfig->DrcGain.len = pCalibDb->DrcTuningPara.DrcGain.EnvLv_len;
        pConfig->DrcGain.EnvLv = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.DrcGain.EnvLv_len));
        pConfig->DrcGain.DrcGain = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.DrcGain.EnvLv_len));
        pConfig->DrcGain.Alpha = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.DrcGain.EnvLv_len));
        pConfig->DrcGain.Clip = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.DrcGain.EnvLv_len));
    }

    //hilit
    if(pConfig->HiLit.len != pCalibDb->DrcTuningPara.HiLight.EnvLv_len) {
        free(pConfig->HiLit.EnvLv);
        free(pConfig->HiLit.Strength);
        pConfig->HiLit.len = pCalibDb->DrcTuningPara.HiLight.EnvLv_len;
        pConfig->HiLit.EnvLv = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.HiLight.EnvLv_len));
        pConfig->HiLit.Strength = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.HiLight.EnvLv_len));
    }

    //local
    if(pConfig->Local.len != pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len) {
        free(pConfig->Local.EnvLv);
        free(pConfig->Local.LocalWeit);
        free(pConfig->Local.GlobalContrast);
        free(pConfig->Local.LoLitContrast);
        pConfig->Local.len = pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
        pConfig->Local.EnvLv = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
        pConfig->Local.LocalWeit = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
        pConfig->Local.GlobalContrast = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
        pConfig->Local.LoLitContrast = (float*)malloc(sizeof(float) * (pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
* CalibrateDrcGainY()
*****************************************************************************/
void CalibrateDrcGainY( DrcProcRes_t *para, float DraGain, float alpha)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);

    float tmp = 0;
    float luma[17] = { 0, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4096 };
    float gainTable[17];

    for(int i = 0; i < 17; ++i) {
        tmp = luma[i];
        tmp = 1 - alpha * pow((1 - tmp / 4096), 2);
        gainTable[i] = 1024 * pow(DraGain, tmp);
        para->sw_drc_gain_y[i] = (int)(gainTable[i]) ;
    }

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);

}

/******************************************************************************
 * AdrcApiOffConfig()
 *set default Config data
 *****************************************************************************/
void AdrcApiOffConfig
(
    AdrcHandle_t           pAdrcCtx
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);




    LOGI_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * Config()
 *set default Config data
 *****************************************************************************/
void Config
(
    AdrcHandle_t           pAdrcCtx
) {
    LOGI_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAdrcCtx != NULL);

    if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_API_OFF)
    {
        AdrcApiOffConfig(pAdrcCtx);
        LOGD_ATMO("%s: Ahdr Api is OFF!!!:\n", __FUNCTION__);
    }
    else
        LOGD_ATMO("%s: Ahdr Api is ON!!!:\n", __FUNCTION__);

    //config default PrevData data
    pAdrcCtx->PrevData.frameCnt = 0;
    pAdrcCtx->PrevData.EnvLv = 0;
    pAdrcCtx->PrevData.ApiMode = DRC_OPMODE_API_OFF;
    pAdrcCtx->PrevData.HandleData.DrcGain = 4.0;
    pAdrcCtx->PrevData.HandleData.Alpha = 0.2;
    pAdrcCtx->PrevData.HandleData.Clip = 16.0 ;
    pAdrcCtx->PrevData.HandleData.Strength = 0.01 ;
    pAdrcCtx->PrevData.HandleData.LocalWeit = 1.00 ;
    pAdrcCtx->PrevData.HandleData.GlobalContrast = 0 ;
    pAdrcCtx->PrevData.HandleData.LoLitContrast = 0;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}
void AdrcGetAeResult
(
    AdrcHandle_t           pAdrcCtx,
    AecPreResult_t  AecHdrPreResult
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    //get Ae Pre Result
    pAdrcCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[AecHdrPreResult.NormalIndex];
    pAdrcCtx->CurrAeResult.M2S_Ratio = AecHdrPreResult.M2S_ExpRatio;
    pAdrcCtx->CurrAeResult.M2S_Ratio = pAdrcCtx->CurrAeResult.M2S_Ratio < 1 ? 1 : pAdrcCtx->CurrAeResult.M2S_Ratio;
    pAdrcCtx->CurrAeResult.L2M_Ratio = AecHdrPreResult.L2M_ExpRatio;
    pAdrcCtx->CurrAeResult.L2M_Ratio = pAdrcCtx->CurrAeResult.L2M_Ratio < 1 ? 1 : pAdrcCtx->CurrAeResult.L2M_Ratio;

    //transfer CurrAeResult data into AhdrHandle
    switch (pAdrcCtx->FrameNumber)
    {
    case 1:
        pAdrcCtx->CurrData.Ratio = 1;
        pAdrcCtx->CurrAeResult.ISO = AecHdrPreResult.LinearExp.exp_real_params.analog_gain * 50.0;
        pAdrcCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[0];
        pAdrcCtx->CurrAeResult.AEMaxRatio = 1.0;
        break;
    case 2:
        pAdrcCtx->CurrData.Ratio = pAdrcCtx->CurrAeResult.M2S_Ratio;
        pAdrcCtx->CurrAeResult.ISO = AecHdrPreResult.HdrExp[1].exp_real_params.analog_gain * 50.0;
        pAdrcCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[1];
        //todo
        pAdrcCtx->CurrAeResult.AEMaxRatio = 64.0;
        break;
    default:
        LOGE_ATMO("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
        break;
    }

    //Normalize the current envLv for AEC
    float maxEnvLuma = 65 / 10;
    float minEnvLuma = 0;
    pAdrcCtx->CurrData.EnvLv = (pAdrcCtx->CurrAeResult.GlobalEnvLv  - minEnvLuma) / (maxEnvLuma - minEnvLuma);
    pAdrcCtx->CurrData.EnvLv = LIMIT_VALUE(pAdrcCtx->CurrData.EnvLv, 1, 0);

    LOGD_ATMO("%s:  Current Ratio:%f\n", __FUNCTION__, pAdrcCtx->CurrData.Ratio);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

void AdrcGetSensorInfo
(
    AdrcHandle_t     pAdrcCtx,
    AecProcResult_t  AecHdrProcResult
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    pAdrcCtx->SensorInfo.LongFrmMode = AecHdrProcResult.LongFrmMode;

    for(int i = 0; i < 3; i++)
    {
#if 0 // TODO Merge
        pAdrcCtx->SensorInfo.HdrMinGain[i] = AecHdrProcResult.HdrExpRange.stGainRange[i].Min;
        pAdrcCtx->SensorInfo.HdrMaxGain[i] = AecHdrProcResult.HdrExpRange.stGainRange[i].Max;
        pAdrcCtx->SensorInfo.HdrMinIntegrationTime[i] = AecHdrProcResult.HdrExpRange.stExpTimeRange[i].Min;
        pAdrcCtx->SensorInfo.HdrMaxIntegrationTime[i] = AecHdrProcResult.HdrExpRange.stExpTimeRange[i].Max;
#endif
    }

    if(pAdrcCtx->FrameNumber == 1) {
        pAdrcCtx->SensorInfo.MaxExpoL = 0;
        pAdrcCtx->SensorInfo.MinExpoL = 0;
    }
    else if(pAdrcCtx->FrameNumber == 2) {
        pAdrcCtx->SensorInfo.MaxExpoL = pAdrcCtx->SensorInfo.HdrMaxGain[1] * pAdrcCtx->SensorInfo.HdrMaxIntegrationTime[2];
        pAdrcCtx->SensorInfo.MinExpoL = pAdrcCtx->SensorInfo.HdrMinGain[1] * pAdrcCtx->SensorInfo.HdrMinIntegrationTime[2];
    }

    pAdrcCtx->SensorInfo.MaxExpoS = pAdrcCtx->SensorInfo.HdrMaxGain[0] * pAdrcCtx->SensorInfo.HdrMaxIntegrationTime[0];
    pAdrcCtx->SensorInfo.MinExpoS = pAdrcCtx->SensorInfo.HdrMinGain[0] * pAdrcCtx->SensorInfo.HdrMinIntegrationTime[0];


    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcIQUpdate()
 *
 *****************************************************************************/
void AdrcIQUpdate
(
    AdrcHandle_t     pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //get Drc gain
    pAdrcCtx->CurrData.HandleData.DrcGain = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
                                            pAdrcCtx->Config.DrcGain.EnvLv, pAdrcCtx->Config.DrcGain.DrcGain, pAdrcCtx->Config.DrcGain.len);
    pAdrcCtx->CurrData.HandleData.Alpha = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
                                          pAdrcCtx->Config.DrcGain.EnvLv, pAdrcCtx->Config.DrcGain.Alpha, pAdrcCtx->Config.DrcGain.len);
    pAdrcCtx->CurrData.HandleData.Clip = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
                                         pAdrcCtx->Config.DrcGain.EnvLv, pAdrcCtx->Config.DrcGain.Clip, pAdrcCtx->Config.DrcGain.len);

    //get hi lit
    pAdrcCtx->CurrData.HandleData.Strength = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.HiLit.EnvLv, pAdrcCtx->Config.HiLit.Strength, pAdrcCtx->Config.HiLit.len);

    //get local
    pAdrcCtx->CurrData.HandleData.LocalWeit = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Local.EnvLv, pAdrcCtx->Config.Local.LocalWeit, pAdrcCtx->Config.Local.len);
    pAdrcCtx->CurrData.HandleData.GlobalContrast = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Local.EnvLv, pAdrcCtx->Config.Local.GlobalContrast, pAdrcCtx->Config.Local.len);
    pAdrcCtx->CurrData.HandleData.LoLitContrast = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Local.EnvLv, pAdrcCtx->Config.Local.LoLitContrast, pAdrcCtx->Config.Local.len);

    //compress
    pAdrcCtx->CurrData.HandleData.Mode = pAdrcCtx->Config.Compress.Mode;
    for(int i = 0; i < ADRC_Y_NUM; i++)
        pAdrcCtx->CurrData.HandleData.Manual_curve[i] = pAdrcCtx->Config.Compress.Manual_curve[i];

    //others
    if(0 != memcmp(&pAdrcCtx->CurrData.Others, &pAdrcCtx->Config.Others, sizeof(DrcOhters_t)))
        memcpy(&pAdrcCtx->CurrData.Others, &pAdrcCtx->Config.Others, sizeof(DrcOhters_t));//load iq paras

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcTranferData2Api()
 *
 *****************************************************************************/
void AdrcTranferData2Api
(
    AdrcHandle_t     pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //info
    pAdrcCtx->drcAttr.Info.EnvLv = pAdrcCtx->CurrData.EnvLv;

    //reg
    pAdrcCtx->drcAttr.stManual.DrcGain.DrcGain = pAdrcCtx->CurrData.HandleData.DrcGain;
    pAdrcCtx->drcAttr.stManual.DrcGain.Alpha = pAdrcCtx->CurrData.HandleData.Alpha;
    pAdrcCtx->drcAttr.stManual.DrcGain.Clip = pAdrcCtx->CurrData.HandleData.Clip;

    pAdrcCtx->drcAttr.stManual.HiLit.Strength = pAdrcCtx->CurrData.HandleData.Strength;

    pAdrcCtx->drcAttr.stManual.Local.LocalWeit = pAdrcCtx->CurrData.HandleData.LocalWeit;
    pAdrcCtx->drcAttr.stManual.Local.GlobalContrast = pAdrcCtx->CurrData.HandleData.GlobalContrast;
    pAdrcCtx->drcAttr.stManual.Local.LoLitContrast = pAdrcCtx->CurrData.HandleData.LoLitContrast;


    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcApiManualUpdate()
 *
 *****************************************************************************/
void AdrcApiManualUpdate
(
    CurrData_t*     pAdrcCurr,
    mdrcAttr_t*  pStManual

)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //get Drc gain
    pAdrcCurr->HandleData.DrcGain = pStManual->DrcGain.DrcGain;
    pAdrcCurr->HandleData.Alpha = pStManual->DrcGain.Alpha;
    pAdrcCurr->HandleData.Clip = pStManual->DrcGain.Clip;

    //get hi lit
    pAdrcCurr->HandleData.Strength = pStManual->HiLit.Strength;

    //get local
    pAdrcCurr->HandleData.LocalWeit = pStManual->Local.LocalWeit;
    pAdrcCurr->HandleData.GlobalContrast = pStManual->Local.GlobalContrast;
    pAdrcCurr->HandleData.LoLitContrast = pStManual->Local.LoLitContrast;

    //compress
    pAdrcCurr->HandleData.Mode = pStManual->Compress.Mode;
    for(int i = 0; i < ADRC_Y_NUM; i++)
        pAdrcCurr->HandleData.Manual_curve[i] = pStManual->Compress.Manual_curve[i];

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcUpdateConfig()
 *transfer html parameter into handle
 ***************************************************************************/
void AdrcUpdateConfig
(
    AdrcHandle_t           pAdrcCtx,
    CalibDbV2_drc_t*         pCalibDb
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAdrcCtx != NULL);
    DCT_ASSERT(pCalibDb != NULL);

    //enbale
    pAdrcCtx->Config.Enable = pCalibDb->DrcTuningPara.Enable;
    pAdrcCtx->Config.OutPutLongFrame = pCalibDb->DrcTuningPara.OutPutLongFrame;

    //drc gain
    for (int i = 0; i < pAdrcCtx->Config.DrcGain.len; i++ ) {
        pAdrcCtx->Config.DrcGain.EnvLv[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.DrcGain.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.DrcGain.DrcGain[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.DrcGain.DrcGain[i], DRCGAINMAX, DRCGAINMIN);
        pAdrcCtx->Config.DrcGain.Alpha[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.DrcGain.Alpha[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.DrcGain.Clip[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);
    }
    //hight ligjt
    for (int i = 0; i < pAdrcCtx->Config.HiLit.len; i++ ) {
        pAdrcCtx->Config.HiLit.EnvLv[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.HiLight.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.HiLit.Strength[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.HiLight.Strength[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    }
    // local
    for (int i = 0; i < pAdrcCtx->Config.Local.len; i++ ) {
        pAdrcCtx->Config.Local.EnvLv[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Local.LocalWeit[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Local.GlobalContrast[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Local.LoLitContrast[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    }
    //others
    pAdrcCtx->Config.Others.curPixWeit = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.curPixWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    pAdrcCtx->Config.Others.preFrameWeit = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.preFrameWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    pAdrcCtx->Config.Others.Range_force_sgm = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.Range_force_sgm, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    pAdrcCtx->Config.Others.Range_sgm_cur = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.Range_sgm_cur, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    pAdrcCtx->Config.Others.Range_sgm_pre = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.Range_sgm_pre, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    pAdrcCtx->Config.Others.Space_sgm_cur = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
    pAdrcCtx->Config.Others.Space_sgm_pre = LIMIT_VALUE(pCalibDb->DrcTuningPara.LocalTMOSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
    //compress and scale y
    pAdrcCtx->Config.Compress.Mode = pCalibDb->DrcTuningPara.CompressSetting.Mode;
    for(int i = 0; i < ADRC_Y_NUM; i++ ) {
        pAdrcCtx->Config.Others.Scale_y[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
        pAdrcCtx->Config.Compress.Manual_curve[i] = LIMIT_VALUE(pCalibDb->DrcTuningPara.CompressSetting.Manual_curve[i], MANUALCURVEMAX, MANUALCURVEMIN);
    }
    pAdrcCtx->Config.Others.ByPassThr = LIMIT_VALUE(pCalibDb->DrcTuningPara.ByPassThr, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    pAdrcCtx->Config.Others.Edge_Weit = LIMIT_VALUE(pCalibDb->DrcTuningPara.Edge_Weit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    pAdrcCtx->Config.Others.IIR_frame = LIMIT_VALUE(pCalibDb->DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
    pAdrcCtx->Config.Others.Tolerance = LIMIT_VALUE(pCalibDb->DrcTuningPara.Tolerance, IIRFRAMEMAX, IIRFRAMEMIN);
    pAdrcCtx->Config.Others.damp = LIMIT_VALUE(pCalibDb->DrcTuningPara.damp, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);


    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}
/******************************************************************************
 * AdrcDamping()
 *****************************************************************************/
void AdrcDamping
(
    CurrData_t* pCurrData,
    AdrcPrevData_t* pPreData,
    drc_OpMode_t opMode,
    int FrameCnt
)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    float Drc_damp = pCurrData->Others.damp;
    bool enDamp;
    bool ifHDRModeChange = pCurrData->frameCnt == pPreData->frameCnt ? false : true;

    if((opMode == DRC_OPMODE_API_OFF || opMode == DRC_OPMODE_AUTO) && FrameCnt != 0 && !ifHDRModeChange)
    {
        float diff = 0.0;

        diff = ABS(pCurrData->EnvLv - pPreData->EnvLv);
        diff = diff / pPreData->EnvLv;
        if (diff < pCurrData->Others.Tolerance)
            enDamp = false;
        else
            enDamp = true;

        //get finnal cfg data by damp
        if (enDamp) {
            pCurrData->HandleData.DrcGain = Drc_damp * pCurrData->HandleData.DrcGain
                                            + (1 - Drc_damp) * pPreData->HandleData.DrcGain;
            pCurrData->HandleData.Alpha = Drc_damp * pCurrData->HandleData.Alpha
                                          + (1 - Drc_damp) * pPreData->HandleData.Alpha;
            pCurrData->HandleData.Clip = Drc_damp * pCurrData->HandleData.Clip
                                         + (1 - Drc_damp) * pPreData->HandleData.Clip;
            pCurrData->HandleData.Strength = Drc_damp * pCurrData->HandleData.Strength
                                             + (1 - Drc_damp) * pPreData->HandleData.Strength;
            pCurrData->HandleData.LocalWeit = Drc_damp * pCurrData->HandleData.LocalWeit
                                              + (1 - Drc_damp) * pPreData->HandleData.LocalWeit;
            pCurrData->HandleData.GlobalContrast = Drc_damp * pCurrData->HandleData.GlobalContrast
                                                   + (1 - Drc_damp) * pPreData->HandleData.GlobalContrast;
            pCurrData->HandleData.LoLitContrast = Drc_damp * pCurrData->HandleData.LoLitContrast
                                                  + (1 - Drc_damp) * pPreData->HandleData.LoLitContrast;
        }


    }

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcGetCurrProcRes()
 *****************************************************************************/
void AdrcGetCurrProcRes
(
    AdrcProcResData_t*           pAdrcProcRes,
    CurrData_t*          pCurrData,
    bool LongFrmMode,
    int FrameNumber,
    int FrameCnt,
    bool OutPutLongFrame
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    //enable
    if(FrameNumber == 2 || FrameNumber == 3) {
        pAdrcProcRes->bTmoEn = true;
        pAdrcProcRes->isLinearTmo = false;
    }
    else if(FrameNumber == 1) {
        pAdrcProcRes->bTmoEn = pCurrData->Enable;
        pAdrcProcRes->isLinearTmo = pAdrcProcRes->bTmoEn;

    }

    //Long Frame mode
    pAdrcProcRes->LongFrameMode = LongFrmMode;

    //Global tmo
    pAdrcProcRes->isHdrGlobalTmo = pCurrData->HandleData.LocalWeit == 0 ? true : false;

    //compress mode
    pAdrcProcRes->CompressMode = pCurrData->HandleData.Mode;

    //DrcProcRes
    pAdrcProcRes->DrcProcRes.sw_drc_offset_pow2 = SW_DRC_OFFSET_POW2_FIX;
    pAdrcProcRes->DrcProcRes.sw_drc_position = (int)(SHIFT8BIT(pCurrData->HandleData.Clip) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_hpdetail_ratio = (int)(SHIFT12BIT(pCurrData->HandleData.LoLitContrast) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_lpdetail_ratio = (int)(SHIFT12BIT(pCurrData->HandleData.GlobalContrast) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_weicur_pix = (int)(SHIFT8BIT(pCurrData->Others.curPixWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_weipre_frame = (int)(SHIFT8BIT(pCurrData->Others.preFrameWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_weipre_frame = LIMIT_VALUE(pAdrcProcRes->DrcProcRes.sw_drc_weipre_frame, 255, 0);
    pAdrcProcRes->DrcProcRes.sw_drc_force_sgm_inv0 = (int)(SHIFT13BIT(pCurrData->Others.Range_force_sgm) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_edge_scl = (int)(SHIFT8BIT(pCurrData->Others.Edge_Weit) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_motion_scl = SW_DRC_MOTION_SCL_FIX;
    pAdrcProcRes->DrcProcRes.sw_drc_space_sgm_inv1 = (int)(pCurrData->Others.Space_sgm_cur);
    pAdrcProcRes->DrcProcRes.sw_drc_space_sgm_inv0 = (int)(pCurrData->Others.Space_sgm_pre);
    pAdrcProcRes->DrcProcRes.sw_drc_range_sgm_inv1 = (int)(SHIFT13BIT(pCurrData->Others.Range_sgm_cur) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_range_sgm_inv0 = (int)(SHIFT13BIT(pCurrData->Others.Range_sgm_pre) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_weig_maxl = (int)(SHIFT4BIT(pCurrData->HandleData.Strength) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_adrc_gain = pCurrData->HandleData.DrcGain;
    pAdrcProcRes->DrcProcRes.sw_drc_weig_bilat = (int)(SHIFT4BIT(pCurrData->HandleData.LocalWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.sw_drc_min_ogain = OutPutLongFrame ? 1 : 0;
    for(int i = 0; i < ADRC_Y_NUM; ++i) {
        pAdrcProcRes->DrcProcRes.sw_drc_scale_y[i] = (int)(pCurrData->Others.Scale_y[i]) ;
        pAdrcProcRes->DrcProcRes.sw_drc_compres_y[i] = pCurrData->HandleData.Manual_curve[i] ;
    }

    //get sw_drc_gain_y
    CalibrateDrcGainY(&pAdrcProcRes->DrcProcRes, pAdrcProcRes->DrcProcRes.sw_drc_adrc_gain, pCurrData->HandleData.Alpha) ;

    float iir_frame = (float)(MIN(FrameCnt + 1, pCurrData->Others.IIR_frame));
    pAdrcProcRes->DrcProcRes.sw_drc_iir_weight = (int)(SHIFT6BIT((iir_frame - 1) / iir_frame) + 0.5);

    pAdrcProcRes->DrcProcRes.sw_drc_compres_scl =  20000.0;

    //sw_drc_delta_scalein FIX
    int ISP_RAW_BIT = 12;
    int MFHDR_LOG_Q_BITS = 11;
    int dstbits = ISP_RAW_BIT << MFHDR_LOG_Q_BITS;
    int offsetbits_int = SW_DRC_OFFSET_POW2_FIX;
    int offsetbits = offsetbits_int << MFHDR_LOG_Q_BITS;
    int validbits = dstbits - offsetbits;
    int delta_scalein = (256 << MFHDR_LOG_Q_BITS) / validbits;
    pAdrcProcRes->DrcProcRes.sw_drc_delta_scalein = delta_scalein;


    LOGD_ATMO("%s:Current damp Enable:%d DrcGain:%f Alpha:%f Clip:%f Strength:%f LocalWeit:%f GlobalContrast:%f LoLitContrast:%f CompressMode:%d\n", __FUNCTION__, pAdrcProcRes->bTmoEn,
              pCurrData->HandleData.DrcGain, pCurrData->HandleData.Alpha, pCurrData->HandleData.Clip, pCurrData->HandleData.Strength,
              pCurrData->HandleData.LocalWeit, pCurrData->HandleData.GlobalContrast, pCurrData->HandleData.LoLitContrast, pCurrData->HandleData.Mode);

    LOGV_ATMO("%s:  sw_drc_adrc_gain:%f Alpha:%f\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_adrc_gain, pCurrData->HandleData.Alpha);
    LOGV_ATMO("%s:  sw_drc_offset_pow2:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_offset_pow2);
    LOGV_ATMO("%s:  sw_drc_compres_scl:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_compres_scl);
    LOGV_ATMO("%s:  sw_drc_position:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_position);
    LOGV_ATMO("%s:  sw_drc_delta_scalein:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_delta_scalein);
    LOGV_ATMO("%s:  sw_drc_hpdetail_ratio:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_hpdetail_ratio);
    LOGV_ATMO("%s:  sw_drc_lpdetail_ratio:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_lpdetail_ratio);
    LOGV_ATMO("%s:  sw_drc_weicur_pix:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_weicur_pix);
    LOGV_ATMO("%s:  sw_drc_weipre_frame:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_weipre_frame);
    LOGV_ATMO("%s:  sw_drc_force_sgm_inv0:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_force_sgm_inv0);
    LOGV_ATMO("%s:  sw_drc_edge_scl:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_edge_scl);
    LOGV_ATMO("%s:  sw_drc_space_sgm_inv1:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_space_sgm_inv1);
    LOGV_ATMO("%s:  sw_drc_space_sgm_inv0:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_space_sgm_inv0);
    LOGV_ATMO("%s:  sw_drc_range_sgm_inv1:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_range_sgm_inv1);
    LOGV_ATMO("%s:  sw_drc_range_sgm_inv0:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_range_sgm_inv0);
    LOGV_ATMO("%s:  sw_drc_weig_maxl:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_weig_maxl);
    LOGV_ATMO("%s:  sw_drc_weig_bilat:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_weig_bilat);
    LOGV_ATMO("%s:  sw_drc_adrc_gain:%f\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_adrc_gain);
    LOGV_ATMO("%s:  sw_drc_iir_weight:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_iir_weight);
    LOGV_ATMO("%s:  sw_drc_min_ogain:%d\n", __FUNCTION__, pAdrcProcRes->DrcProcRes.sw_drc_min_ogain);
    for(int i = 0; i < 17; i++)
        LOGV_ATMO("%s:  sw_drc_gain_y[%d]:%d\n", __FUNCTION__, i, pAdrcProcRes->DrcProcRes.sw_drc_gain_y[i]);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcGetCurrData()
 *****************************************************************************/
void AdrcGetCurrData
(
    AdrcHandle_t pAdrcCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    //damp
    AdrcDamping(&pAdrcCtx->CurrData, &pAdrcCtx->PrevData, pAdrcCtx->drcAttr.opMode, pAdrcCtx->frameCnt);

    //get io data
    AdrcGetCurrProcRes(&pAdrcCtx->AdrcProcRes, &pAdrcCtx->CurrData, pAdrcCtx->SensorInfo.LongFrmMode,
                       pAdrcCtx->FrameNumber, pAdrcCtx->frameCnt, pAdrcCtx->Config.OutPutLongFrame);

    // store current handle data to pre data for next loop
    pAdrcCtx->PrevData.EnvLv = pAdrcCtx->CurrData.EnvLv;
    pAdrcCtx->PrevData.ISO = pAdrcCtx->CurrData.ISO;
    if(0 != memcmp(&pAdrcCtx->PrevData.HandleData, &pAdrcCtx->CurrData.HandleData, sizeof(DrcHandleData_t)))
        memcpy(&pAdrcCtx->PrevData.HandleData, &pAdrcCtx->CurrData.HandleData, sizeof(DrcHandleData_t));
    ++pAdrcCtx->frameCnt;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcProcessing
(
    AdrcHandle_t     pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    pAdrcCtx->CurrData.frameCnt = pAdrcCtx->frameCnt;

    //enable setting
    if(pAdrcCtx->drcAttr.Enable)
        pAdrcCtx->CurrData.Enable = true;
    else
        pAdrcCtx->CurrData.Enable = pAdrcCtx->Config.Enable;

    //para setting
    AdrcIQUpdate(pAdrcCtx);

    //api
    if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_API_OFF)
        LOGD_ATMO("%s:  Adrc api OFF!! Current Handle data:\n", __FUNCTION__);
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_MANU) {
        LOGD_ATMO("%s:  Adrc api Manual!! Current Handle data:\n", __FUNCTION__);
        AdrcApiManualUpdate(&pAdrcCtx->CurrData, &pAdrcCtx->drcAttr.stManual);
    } else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_AUTO)
        LOGD_ATMO("%s:  Adrc api ATUO!! Current Handle data:\n", __FUNCTION__);
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_DRC_GAIN) {
        LOGD_ATMO("%s:  Adrc api GDRC Gain!! Current Handle data:\n", __FUNCTION__);
        pAdrcCtx->CurrData.HandleData.DrcGain = pAdrcCtx->drcAttr.stManual.DrcGain.DrcGain;
        pAdrcCtx->CurrData.HandleData.Alpha = pAdrcCtx->drcAttr.stManual.DrcGain.Alpha;
        pAdrcCtx->CurrData.HandleData.Clip = pAdrcCtx->drcAttr.stManual.DrcGain.Clip;
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_HILIT) {
        LOGD_ATMO("%s:  Adrc api HiLit!! Current Handle data:\n", __FUNCTION__);
        pAdrcCtx->CurrData.HandleData.Strength = pAdrcCtx->drcAttr.stManual.HiLit.Strength;
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_LOCAL_TMO) {
        LOGD_ATMO("%s:  Adrc api Local TMO!! Current Handle data:\n", __FUNCTION__);
        pAdrcCtx->CurrData.HandleData.LocalWeit = pAdrcCtx->drcAttr.stManual.Local.LocalWeit;
        pAdrcCtx->CurrData.HandleData.GlobalContrast = pAdrcCtx->drcAttr.stManual.Local.GlobalContrast;
        pAdrcCtx->CurrData.HandleData.LoLitContrast = pAdrcCtx->drcAttr.stManual.Local.LoLitContrast;
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_COMPRESS) {
        LOGD_ATMO("%s:  Adrc api Compress!! Current Handle data:\n", __FUNCTION__);
        pAdrcCtx->CurrData.HandleData.Mode = pAdrcCtx->drcAttr.stManual.Compress.Mode;
        for(int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->CurrData.HandleData.Manual_curve[i] = pAdrcCtx->drcAttr.stManual.Compress.Manual_curve[i];
    }
    else
        LOGE_ATMO("%s:  Adrc wrong mode!!!\n", __FUNCTION__);

    //clip drc gain
    if(pAdrcCtx->CurrAeResult.AEMaxRatio * pAdrcCtx->CurrData.HandleData.DrcGain > MAX_AE_DRC_GAIN) {
        LOGE_ATMO("%s:  AERatio*DrcGain > 256!!!\n", __FUNCTION__);
        pAdrcCtx->CurrData.HandleData.DrcGain = MAX_AE_DRC_GAIN / pAdrcCtx->CurrAeResult.AEMaxRatio;
    }

    LOGD_ATMO("%s:Current Enable:%d DrcGain:%f Alpha:%f Clip:%f Strength:%f LocalWeit:%f GlobalContrast:%f LoLitContrast:%f CompressMode:%d\n", __FUNCTION__, pAdrcCtx->CurrData.Enable,
              pAdrcCtx->CurrData.HandleData.DrcGain, pAdrcCtx->CurrData.HandleData.Alpha, pAdrcCtx->CurrData.HandleData.Clip, pAdrcCtx->CurrData.HandleData.Strength,
              pAdrcCtx->CurrData.HandleData.LocalWeit, pAdrcCtx->CurrData.HandleData.GlobalContrast, pAdrcCtx->CurrData.HandleData.LoLitContrast
              , pAdrcCtx->CurrData.HandleData.Mode);

    //transfer data to api
    AdrcTranferData2Api(pAdrcCtx);

    //get io and store data in proc res
    AdrcGetCurrData(pAdrcCtx);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcByPassProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
bool AdrcByPassProcessing
(
    AdrcHandle_t     pAdrcCtx,
    AecPreResult_t  AecHdrPreResult
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    bool bypass = false;
    float diff = 0.0;

    if(pAdrcCtx->frameCnt <= 2)//start frame
        bypass = false;
    else if(pAdrcCtx->drcAttr.opMode > DRC_OPMODE_API_OFF)//api
        bypass = false;
    else if(pAdrcCtx->drcAttr.opMode !=  pAdrcCtx->PrevData.ApiMode)//api change
        bypass = false;
    else { //EnvLv change
        //get current ae data from AecPreRes
        AdrcGetAeResult(pAdrcCtx, AecHdrPreResult);

        //transfer ae data to CurrHandle
        pAdrcCtx->CurrData.EnvLv = LIMIT_VALUE(pAdrcCtx->CurrData.EnvLv, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);

        pAdrcCtx->CurrData.ISO = pAdrcCtx->CurrAeResult.ISO;
        pAdrcCtx->CurrData.ISO = LIMIT_VALUE(pAdrcCtx->CurrData.ISO, ISOMAX, ISOMIN);

        //use Envlv for now
        diff = pAdrcCtx->PrevData.EnvLv - pAdrcCtx->CurrData.EnvLv;
        if(pAdrcCtx->PrevData.EnvLv == 0.0) {
            diff = pAdrcCtx->CurrData.EnvLv;
            if(diff == 0.0)
                bypass = true;
            else
                bypass = false;
        }
        else {
            diff /= pAdrcCtx->PrevData.EnvLv;
            if(diff >= pAdrcCtx->Config.Others.ByPassThr || diff <= (0 - pAdrcCtx->Config.Others.ByPassThr))
                bypass = false;
            else
                bypass = true;
        }
    }

    LOG1_ATMO( "%s:CtrlEnvLv:%f PrevEnvLv:%f diff:%f ByPassThr:%f opMode:%d bypass:%d!\n", __FUNCTION__, pAdrcCtx->CurrData.EnvLv,
               pAdrcCtx->PrevData.EnvLv, diff, pAdrcCtx->Config.Others.ByPassThr, pAdrcCtx->drcAttr.opMode, bypass);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
    return bypass;
}

/******************************************************************************
 * AdrcInit()
 *****************************************************************************/
XCamReturn AdrcInit
(
    AdrcInstanceConfig_t* pInstConfig,
    CamCalibDbV2Context_t* calibv2
) {

    AdrcContext_s *pAdrcCtx;

    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    XCamReturn result = XCAM_RETURN_NO_ERROR;

    // initial checks
    if (pInstConfig == NULL)
        return (XCAM_RETURN_ERROR_PARAM);

    // allocate Adrc control context
    pAdrcCtx = (AdrcContext_s*)calloc(sizeof(AdrcContext_t), 1);
    if (NULL == pAdrcCtx) {
        LOGE_ATMO( "%s: Can't allocate Adrc context\n",  __FUNCTION__);
        return (XCAM_RETURN_ERROR_OUTOFRANGE);
    }

    CalibDbV2_drc_t* calibv2_adrc_calib =
        (CalibDbV2_drc_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, adrc_calib));

    // pre-initialize context
    memset(pAdrcCtx, 0x00, sizeof(*pAdrcCtx));

    //malloc DrcGain
    pAdrcCtx->Config.DrcGain.len = calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len;
    pAdrcCtx->Config.DrcGain.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
    pAdrcCtx->Config.DrcGain.DrcGain = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
    pAdrcCtx->Config.DrcGain.Alpha = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
    pAdrcCtx->Config.DrcGain.Clip = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));

    //malloc HiLit
    pAdrcCtx->Config.HiLit.len = calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len;
    pAdrcCtx->Config.HiLit.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len));
    pAdrcCtx->Config.HiLit.Strength = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len));

    //malloc local
    pAdrcCtx->Config.Local.len = calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
    pAdrcCtx->Config.Local.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
    pAdrcCtx->Config.Local.LocalWeit = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
    pAdrcCtx->Config.Local.GlobalContrast = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
    pAdrcCtx->Config.Local.LoLitContrast = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
    pAdrcCtx->drcAttr.Enable = false;
    pAdrcCtx->drcAttr.opMode = DRC_OPMODE_API_OFF;
    pAdrcCtx->frameCnt = 0;
    Config(pAdrcCtx); //set default para
    memcpy(&pAdrcCtx->pCalibDB, calibv2_adrc_calib, sizeof(CalibDbV2_drc_t));//load iq paras
    //memcpy(&pAdrcCtx->drcAttr.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_t));//load iq paras to stTool

    pAdrcCtx->state = ADRC_STATE_INITIALIZED;

    pInstConfig->hAdrc = (AdrcHandle_t)pAdrcCtx;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
/******************************************************************************
 * AhdrRelease()
 *****************************************************************************/
XCamReturn AdrcRelease
(
    AdrcHandle_t pAdrcCtx
) {

    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    // initial checks
    if (NULL == pAdrcCtx) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    result = AdrcStop(pAdrcCtx);
    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_ATMO( "%s: AHDRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ADRC_STATE_RUNNING == pAdrcCtx->state)
            || (ADRC_STATE_LOCKED == pAdrcCtx->state)) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    free(pAdrcCtx->Config.DrcGain.EnvLv);
    free(pAdrcCtx->Config.DrcGain.DrcGain);
    free(pAdrcCtx->Config.DrcGain.Alpha);
    free(pAdrcCtx->Config.DrcGain.Clip);
    free(pAdrcCtx->Config.HiLit.EnvLv);
    free(pAdrcCtx->Config.HiLit.Strength);
    free(pAdrcCtx->Config.Local.EnvLv);
    free(pAdrcCtx->Config.Local.LocalWeit);
    free(pAdrcCtx->Config.Local.GlobalContrast);
    free(pAdrcCtx->Config.Local.LoLitContrast);
    memset(pAdrcCtx, 0, sizeof(AdrcContext_t));
    free(pAdrcCtx);
    pAdrcCtx = NULL;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
