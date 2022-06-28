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
//#include "rk_aiq_types_adrc_algo_int.h"
#include "xcam_log.h"
#include "rk_aiq_types_adrc_algo_prvt.h"

/******************************************************************************
 * AdrcStart()
 *****************************************************************************/
XCamReturn AdrcStart
(
    AdrcContext_t* pAdrcCtx
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
 * DrcGetCurrPara()
 *****************************************************************************/
int DrcGetCurrParaInt
(
    float           inPara,
    float*         inMatrixX,
    int*         inMatrixY,
    int Max_Knots
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);
    float x1 = 0.0f;
    float x2 = 0.0f;
    float value1 = 0.0f;
    float value2 = 0.0f;
    int outPara = 0;

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
                outPara = (int)(value1 + (inPara - x1) * (value1 - value2) / (x1 - x2));
                break;
            }
            else
                continue;
        }

    return outPara;
    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcStop()
 *****************************************************************************/
XCamReturn AdrcStop
(
    AdrcContext_t* pAdrcCtx
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
 * DrcProcApiMalloc()
 ***************************************************************************/
void DrcProcApiMalloc
(
    AdrcConfig_t*           pConfig,
    drcAttr_t*               pDrcAttr,
    DrcCalibDB_t*         pCalibDb
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pConfig != NULL);
    DCT_ASSERT(pCalibDb != NULL);
    DCT_ASSERT(pDrcAttr != NULL);

    int New_DrcGain_EnvLv_len = 1;
    int New_HiLit_EnvLv_len = 1;
    int New_Local_EnvLv_len = 1;

    if(CHECK_ISP_HW_V21()) {
        if(pDrcAttr->opMode == DRC_OPMODE_MANU) {
            New_DrcGain_EnvLv_len = 1;
            New_HiLit_EnvLv_len = 1;
            New_Local_EnvLv_len = 1;
        }
        else if(pDrcAttr->opMode == DRC_OPMODE_DRC_GAIN) {
            New_DrcGain_EnvLv_len = 1;
            New_HiLit_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.HiLight.EnvLv_len;
            New_Local_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
        }
        else if(pDrcAttr->opMode == DRC_OPMODE_HILIT) {
            New_DrcGain_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.DrcGain.EnvLv_len;
            New_HiLit_EnvLv_len = 1;
            New_Local_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
        }
        else if(pDrcAttr->opMode == DRC_OPMODE_LOCAL_TMO) {
            New_DrcGain_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.DrcGain.EnvLv_len;
            New_HiLit_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.HiLight.EnvLv_len;
            New_Local_EnvLv_len = 1;
        }

        LOG1_ATMO( "%s: Pre DrcGain len:%d, Cur DrcGain len:%d\n", __FUNCTION__, pConfig->Drc_v21.DrcGain.len, New_DrcGain_EnvLv_len);
        LOG1_ATMO( "%s: Pre HiLit len:%d, Cur HiLit len:%d\n", __FUNCTION__, pConfig->Drc_v21.HiLit.len, New_HiLit_EnvLv_len);
        LOG1_ATMO( "%s: Pre Local len:%d, Cur Local len:%d\n", __FUNCTION__, pConfig->Drc_v21.Local.len, New_Local_EnvLv_len);

        //drc gain
        if(pConfig->Drc_v21.DrcGain.len != New_DrcGain_EnvLv_len) {
            free(pConfig->Drc_v21.DrcGain.EnvLv);
            free(pConfig->Drc_v21.DrcGain.DrcGain);
            free(pConfig->Drc_v21.DrcGain.Alpha);
            free(pConfig->Drc_v21.DrcGain.Clip);
            pConfig->Drc_v21.DrcGain.len = New_DrcGain_EnvLv_len;
            pConfig->Drc_v21.DrcGain.EnvLv = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v21.DrcGain.DrcGain = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v21.DrcGain.Alpha = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v21.DrcGain.Clip = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
        }

        //hilit
        if(pConfig->Drc_v21.HiLit.len != New_HiLit_EnvLv_len) {
            free(pConfig->Drc_v21.HiLit.EnvLv);
            free(pConfig->Drc_v21.HiLit.Strength);
            pConfig->Drc_v21.HiLit.len = New_HiLit_EnvLv_len;
            pConfig->Drc_v21.HiLit.EnvLv = (float*)malloc(sizeof(float) * (New_HiLit_EnvLv_len));
            pConfig->Drc_v21.HiLit.Strength = (float*)malloc(sizeof(float) * (New_HiLit_EnvLv_len));
        }

        //local
        if(pConfig->Drc_v21.Local.len != New_Local_EnvLv_len) {
            free(pConfig->Drc_v21.Local.EnvLv);
            free(pConfig->Drc_v21.Local.LocalWeit);
            free(pConfig->Drc_v21.Local.GlobalContrast);
            free(pConfig->Drc_v21.Local.LoLitContrast);
            pConfig->Drc_v21.Local.len = New_Local_EnvLv_len;
            pConfig->Drc_v21.Local.EnvLv = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v21.Local.LocalWeit = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v21.Local.GlobalContrast = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v21.Local.LoLitContrast = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        if(pDrcAttr->opMode == DRC_OPMODE_MANU) {
            New_DrcGain_EnvLv_len = 1;
            New_HiLit_EnvLv_len = 1;
            New_Local_EnvLv_len = 1;
        }
        else if(pDrcAttr->opMode == DRC_OPMODE_DRC_GAIN) {
            New_DrcGain_EnvLv_len = 1;
            New_HiLit_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.HiLight.EnvLv_len;
            New_Local_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.EnvLv_len;
        }
        else if(pDrcAttr->opMode == DRC_OPMODE_HILIT) {
            New_DrcGain_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.DrcGain.EnvLv_len;
            New_HiLit_EnvLv_len = 1;
            New_Local_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.EnvLv_len;
        }
        else if(pDrcAttr->opMode == DRC_OPMODE_LOCAL_TMO) {
            New_DrcGain_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.DrcGain.EnvLv_len;
            New_HiLit_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.HiLight.EnvLv_len;
            New_Local_EnvLv_len = 1;
        }

        LOG1_ATMO( "%s: Pre DrcGain len:%d, Cur DrcGain len:%d\n", __FUNCTION__, pConfig->Drc_v30.DrcGain.len, New_DrcGain_EnvLv_len);
        LOG1_ATMO( "%s: Pre HiLit len:%d, Cur HiLit len:%d\n", __FUNCTION__, pConfig->Drc_v30.HiLit.len, New_HiLit_EnvLv_len);
        LOG1_ATMO( "%s: Pre Local len:%d, Cur Local len:%d\n", __FUNCTION__, pConfig->Drc_v30.Local.len, New_Local_EnvLv_len);

        //drc gain
        if(pConfig->Drc_v30.DrcGain.len != New_DrcGain_EnvLv_len) {
            free(pConfig->Drc_v30.DrcGain.EnvLv);
            free(pConfig->Drc_v30.DrcGain.DrcGain);
            free(pConfig->Drc_v30.DrcGain.Alpha);
            free(pConfig->Drc_v30.DrcGain.Clip);
            pConfig->Drc_v30.DrcGain.len = New_DrcGain_EnvLv_len;
            pConfig->Drc_v30.DrcGain.EnvLv = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v30.DrcGain.DrcGain = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v30.DrcGain.Alpha = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v30.DrcGain.Clip = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
        }

        //hilit
        if(pConfig->Drc_v30.HiLit.len != New_HiLit_EnvLv_len) {
            free(pConfig->Drc_v30.HiLit.EnvLv);
            free(pConfig->Drc_v30.HiLit.Strength);
            pConfig->Drc_v30.HiLit.len = New_HiLit_EnvLv_len;
            pConfig->Drc_v30.HiLit.EnvLv = (float*)malloc(sizeof(float) * (New_HiLit_EnvLv_len));
            pConfig->Drc_v30.HiLit.Strength = (float*)malloc(sizeof(float) * (New_HiLit_EnvLv_len));
        }

        //local
        if(pConfig->Drc_v30.Local.len != New_Local_EnvLv_len) {
            free(pConfig->Drc_v30.Local.EnvLv);
            free(pConfig->Drc_v30.Local.LocalWeit);
            free(pConfig->Drc_v30.Local.LocalAutoEnable);
            free(pConfig->Drc_v30.Local.LocalAutoWeit);
            free(pConfig->Drc_v30.Local.GlobalContrast);
            free(pConfig->Drc_v30.Local.LoLitContrast);
            pConfig->Drc_v30.Local.len = New_Local_EnvLv_len;
            pConfig->Drc_v30.Local.EnvLv = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.LocalWeit = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.LocalAutoEnable = (int*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.LocalAutoWeit = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.GlobalContrast = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.LoLitContrast = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
        }
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}
/******************************************************************************
 * DrcPrepareJsonMalloc()
 ***************************************************************************/
void DrcPrepareJsonMalloc
(
    AdrcConfig_t*           pConfig,
    DrcCalibDB_t*         pCalibDb
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pConfig != NULL);
    DCT_ASSERT(pCalibDb != NULL);

    int New_DrcGain_EnvLv_len = 1;
    int New_HiLit_EnvLv_len = 1;
    int New_Local_EnvLv_len = 1;

    if(CHECK_ISP_HW_V21()) {
        New_DrcGain_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.DrcGain.EnvLv_len;
        New_HiLit_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.HiLight.EnvLv_len;
        New_Local_EnvLv_len = pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;

        LOG1_ATMO( "%s: Pre DrcGain len:%d, Cur DrcGain len:%d\n", __FUNCTION__, pConfig->Drc_v21.DrcGain.len, New_DrcGain_EnvLv_len);
        LOG1_ATMO( "%s: Pre HiLit len:%d, Cur HiLit len:%d\n", __FUNCTION__, pConfig->Drc_v21.HiLit.len, New_HiLit_EnvLv_len);
        LOG1_ATMO( "%s: Pre Local len:%d, Cur Local len:%d\n", __FUNCTION__, pConfig->Drc_v21.Local.len, New_Local_EnvLv_len);

        //drc gain
        if(pConfig->Drc_v21.DrcGain.len != New_DrcGain_EnvLv_len) {
            free(pConfig->Drc_v21.DrcGain.EnvLv);
            free(pConfig->Drc_v21.DrcGain.DrcGain);
            free(pConfig->Drc_v21.DrcGain.Alpha);
            free(pConfig->Drc_v21.DrcGain.Clip);
            pConfig->Drc_v21.DrcGain.len = New_DrcGain_EnvLv_len;
            pConfig->Drc_v21.DrcGain.EnvLv = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v21.DrcGain.DrcGain = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v21.DrcGain.Alpha = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v21.DrcGain.Clip = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
        }

        //hilit
        if(pConfig->Drc_v21.HiLit.len != New_HiLit_EnvLv_len) {
            free(pConfig->Drc_v21.HiLit.EnvLv);
            free(pConfig->Drc_v21.HiLit.Strength);
            pConfig->Drc_v21.HiLit.len = New_HiLit_EnvLv_len;
            pConfig->Drc_v21.HiLit.EnvLv = (float*)malloc(sizeof(float) * (New_HiLit_EnvLv_len));
            pConfig->Drc_v21.HiLit.Strength = (float*)malloc(sizeof(float) * (New_HiLit_EnvLv_len));
        }

        //local
        if(pConfig->Drc_v21.Local.len != New_Local_EnvLv_len) {
            free(pConfig->Drc_v21.Local.EnvLv);
            free(pConfig->Drc_v21.Local.LocalWeit);
            free(pConfig->Drc_v21.Local.GlobalContrast);
            free(pConfig->Drc_v21.Local.LoLitContrast);
            pConfig->Drc_v21.Local.len = New_Local_EnvLv_len;
            pConfig->Drc_v21.Local.EnvLv = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v21.Local.LocalWeit = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v21.Local.GlobalContrast = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v21.Local.LoLitContrast = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        New_DrcGain_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.DrcGain.EnvLv_len;
        New_HiLit_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.HiLight.EnvLv_len;
        New_Local_EnvLv_len = pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.EnvLv_len;

        LOG1_ATMO( "%s: Pre DrcGain len:%d, Cur DrcGain len:%d\n", __FUNCTION__, pConfig->Drc_v30.DrcGain.len, New_DrcGain_EnvLv_len);
        LOG1_ATMO( "%s: Pre HiLit len:%d, Cur HiLit len:%d\n", __FUNCTION__, pConfig->Drc_v30.HiLit.len, New_HiLit_EnvLv_len);
        LOG1_ATMO( "%s: Pre Local len:%d, Cur Local len:%d\n", __FUNCTION__, pConfig->Drc_v30.Local.len, New_Local_EnvLv_len);

        //drc gain
        if(pConfig->Drc_v30.DrcGain.len != New_DrcGain_EnvLv_len) {
            free(pConfig->Drc_v30.DrcGain.EnvLv);
            free(pConfig->Drc_v30.DrcGain.DrcGain);
            free(pConfig->Drc_v30.DrcGain.Alpha);
            free(pConfig->Drc_v30.DrcGain.Clip);
            pConfig->Drc_v30.DrcGain.len = New_DrcGain_EnvLv_len;
            pConfig->Drc_v30.DrcGain.EnvLv = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v30.DrcGain.DrcGain = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v30.DrcGain.Alpha = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
            pConfig->Drc_v30.DrcGain.Clip = (float*)malloc(sizeof(float) * (New_DrcGain_EnvLv_len));
        }

        //hilit
        if(pConfig->Drc_v30.HiLit.len != New_HiLit_EnvLv_len) {
            free(pConfig->Drc_v30.HiLit.EnvLv);
            free(pConfig->Drc_v30.HiLit.Strength);
            pConfig->Drc_v30.HiLit.len = New_HiLit_EnvLv_len;
            pConfig->Drc_v30.HiLit.EnvLv = (float*)malloc(sizeof(float) * (New_HiLit_EnvLv_len));
            pConfig->Drc_v30.HiLit.Strength = (float*)malloc(sizeof(float) * (New_HiLit_EnvLv_len));
        }

        //local
        if(pConfig->Drc_v30.Local.len != New_Local_EnvLv_len) {
            free(pConfig->Drc_v30.Local.EnvLv);
            free(pConfig->Drc_v30.Local.LocalWeit);
            free(pConfig->Drc_v30.Local.LocalAutoEnable);
            free(pConfig->Drc_v30.Local.LocalAutoWeit);
            free(pConfig->Drc_v30.Local.GlobalContrast);
            free(pConfig->Drc_v30.Local.LoLitContrast);
            pConfig->Drc_v30.Local.len = New_Local_EnvLv_len;
            pConfig->Drc_v30.Local.EnvLv = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.LocalWeit = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.LocalAutoEnable = (int*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.LocalAutoWeit = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.GlobalContrast = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
            pConfig->Drc_v30.Local.LoLitContrast = (float*)malloc(sizeof(float) * (New_Local_EnvLv_len));
        }
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
        if(CHECK_ISP_HW_V21())
            para->Drc_v21.sw_drc_gain_y[i] = (int)(gainTable[i]) ;
        else if(CHECK_ISP_HW_V30())
            para->Drc_v30.gain_y[i] = (int)(gainTable[i]) ;
    }

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * ConfigV21()
 *set default Config data
 *****************************************************************************/
void ConfigV21
(
    AdrcContext_t*           pAdrcCtx
) {
    LOGI_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAdrcCtx != NULL);

    //config default PrevData data
    pAdrcCtx->PrevData.frameCnt = 0;
    pAdrcCtx->PrevData.EnvLv = 0;
    pAdrcCtx->PrevData.ApiMode = DRC_OPMODE_API_OFF;
    pAdrcCtx->PrevData.HandleData.Drc_v21.DrcGain = 4.0;
    pAdrcCtx->PrevData.HandleData.Drc_v21.Alpha = 0.2;
    pAdrcCtx->PrevData.HandleData.Drc_v21.Clip = 16.0 ;
    pAdrcCtx->PrevData.HandleData.Drc_v21.Strength = 0.01 ;
    pAdrcCtx->PrevData.HandleData.Drc_v21.LocalWeit = 1.00 ;
    pAdrcCtx->PrevData.HandleData.Drc_v21.GlobalContrast = 0 ;
    pAdrcCtx->PrevData.HandleData.Drc_v21.LoLitContrast = 0;

    //set default value for api
    pAdrcCtx->drcAttr.stManualV21.Enable = true;
    pAdrcCtx->drcAttr.stManualV21.DrcGain.Alpha = 0.1;
    pAdrcCtx->drcAttr.stManualV21.DrcGain.DrcGain = 1;
    pAdrcCtx->drcAttr.stManualV21.DrcGain.Clip = 1;
    pAdrcCtx->drcAttr.stManualV21.HiLit.Strength = 0.0;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.LocalData.LocalWeit = 1;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.LocalData.GlobalContrast = 0.0;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.LocalData.LoLitContrast = 0.0;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.curPixWeit = 0.376471;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.preFrameWeit = 0.8;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.Range_force_sgm = 0.0;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.Range_sgm_cur = 0.2;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.Range_sgm_pre = 0.2;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.Space_sgm_cur = 4068;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.Space_sgm_pre = 3068;
    pAdrcCtx->drcAttr.stManualV21.LocalSetting.Space_sgm_pre = 3068;
    pAdrcCtx->drcAttr.stManualV21.Compress.Mode = COMPRESS_AUTO;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[0] = 0;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[1] = 558;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[2] = 1087;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[3] = 1588;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[4] = 2063;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[5] = 2515;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[6] = 2944;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[7] = 3353;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[8] = 3744;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[9] = 4473;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[10] = 5139;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[11] = 5751;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[12] = 6316;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[13] = 6838;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[14] = 7322;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[15] = 7772;
    pAdrcCtx->drcAttr.stManualV21.Compress.Manual_curve[16] = 8192;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[0] = 0;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[1] = 2;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[2] = 20;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[3] = 76;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[4] = 193;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[5] = 381;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[6] = 631;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[7] = 772;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[8] = 919;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[9] = 1066;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[10] = 1211;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[11] = 1479;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[12] = 1700;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[13] = 1863;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[14] = 1968;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[15] = 2024;
    pAdrcCtx->drcAttr.stManualV21.Scale_y[16] = 2048;
    pAdrcCtx->drcAttr.stManualV21.Edge_Weit = 0.02;
    pAdrcCtx->drcAttr.stManualV21.OutPutLongFrame = false;
    pAdrcCtx->drcAttr.stManualV21.IIR_frame = 2;

    pAdrcCtx->drcAttr.stDrcGain.DrcGain = 1;
    pAdrcCtx->drcAttr.stDrcGain.Clip = 1;
    pAdrcCtx->drcAttr.stDrcGain.Alpha = 0.2;

    pAdrcCtx->drcAttr.stHiLit.Strength = 0.0;

    pAdrcCtx->drcAttr.stLocalDataV21.LocalWeit = 1;
    pAdrcCtx->drcAttr.stLocalDataV21.GlobalContrast = 0.0;
    pAdrcCtx->drcAttr.stLocalDataV21.LoLitContrast = 0.0;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * Config()
 *set default Config data
 *****************************************************************************/
void ConfigV30
(
    AdrcContext_t*           pAdrcCtx
) {
    LOGI_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAdrcCtx != NULL);

    //config default PrevData data
    pAdrcCtx->PrevData.frameCnt = 0;
    pAdrcCtx->PrevData.EnvLv = 0;
    pAdrcCtx->PrevData.MotionCoef = 0;
    pAdrcCtx->PrevData.ApiMode = DRC_OPMODE_API_OFF;
    pAdrcCtx->PrevData.HandleData.Drc_v30.DrcGain = 4.0;
    pAdrcCtx->PrevData.HandleData.Drc_v30.Alpha = 0.2;
    pAdrcCtx->PrevData.HandleData.Drc_v30.Clip = 16.0 ;
    pAdrcCtx->PrevData.HandleData.Drc_v30.Strength = 0.01 ;
    pAdrcCtx->PrevData.HandleData.Drc_v30.LocalWeit = 1.00 ;
    pAdrcCtx->PrevData.HandleData.Drc_v30.LocalAutoEnable = 1 ;
    pAdrcCtx->PrevData.HandleData.Drc_v30.LocalAutoWeit = 0.037477 ;
    pAdrcCtx->PrevData.HandleData.Drc_v30.GlobalContrast = 0 ;
    pAdrcCtx->PrevData.HandleData.Drc_v30.LoLitContrast = 0;

    //set default value for api
    pAdrcCtx->drcAttr.stManualV30.Enable = true;
    pAdrcCtx->drcAttr.stManualV30.DrcGain.Alpha = 0.1;
    pAdrcCtx->drcAttr.stManualV30.DrcGain.DrcGain = 1;
    pAdrcCtx->drcAttr.stManualV30.DrcGain.Clip = 1;
    pAdrcCtx->drcAttr.stManualV30.HiLight.Strength = 0.0;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.LocalData.LocalWeit = 1;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.LocalData.LocalAutoEnable = 1;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.LocalData.LocalAutoWeit = 0.5;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.LocalData.GlobalContrast = 0.0;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.LocalData.LoLitContrast = 0.0;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.curPixWeit = 0.376471;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.preFrameWeit = 0.8;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.Range_force_sgm = 0.0;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.Range_sgm_cur = 0.2;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.Range_sgm_pre = 0.2;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.Space_sgm_cur = 4068;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.Space_sgm_pre = 3068;
    pAdrcCtx->drcAttr.stManualV30.LocalSetting.Space_sgm_pre = 3068;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Mode = COMPRESS_AUTO;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[0] = 0;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[1] = 558;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[2] = 1087;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[3] = 1588;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[4] = 2063;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[5] = 2515;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[6] = 2944;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[7] = 3353;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[8] = 3744;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[9] = 4473;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[10] = 5139;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[11] = 5751;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[12] = 6316;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[13] = 6838;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[14] = 7322;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[15] = 7772;
    pAdrcCtx->drcAttr.stManualV30.CompressSetting.Manual_curve[16] = 8192;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[0] = 0;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[1] = 2;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[2] = 20;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[3] = 76;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[4] = 193;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[5] = 381;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[6] = 631;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[7] = 772;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[8] = 919;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[9] = 1066;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[10] = 1211;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[11] = 1479;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[12] = 1700;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[13] = 1863;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[14] = 1968;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[15] = 2024;
    pAdrcCtx->drcAttr.stManualV30.Scale_y[16] = 2048;
    pAdrcCtx->drcAttr.stManualV30.Edge_Weit = 0.02;
    pAdrcCtx->drcAttr.stManualV30.OutPutLongFrame = false;
    pAdrcCtx->drcAttr.stManualV30.IIR_frame = 2;

    pAdrcCtx->drcAttr.stDrcGain.DrcGain = 1;
    pAdrcCtx->drcAttr.stDrcGain.Clip = 1;
    pAdrcCtx->drcAttr.stDrcGain.Alpha = 0.2;

    pAdrcCtx->drcAttr.stHiLit.Strength = 0.0;

    pAdrcCtx->drcAttr.stLocalDataV30.LocalWeit = 1;
    pAdrcCtx->drcAttr.stLocalDataV30.LocalAutoEnable = 1;
    pAdrcCtx->drcAttr.stLocalDataV30.LocalAutoWeit = 0.5;
    pAdrcCtx->drcAttr.stLocalDataV30.GlobalContrast = 0.0;
    pAdrcCtx->drcAttr.stLocalDataV30.LoLitContrast = 0.0;


    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

void AdrcGetEnvLv
(
    AdrcContext_t*           pAdrcCtx,
    AecPreResult_t  AecHdrPreResult
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    //transfer CurrAeResult data into AhdrHandle
    switch (pAdrcCtx->FrameNumber)
    {
    case LINEAR_NUM:
        pAdrcCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[0];
        pAdrcCtx->CurrAeResult.AEMaxRatio = 1.0;
        break;
    case HDR_2X_NUM:
        pAdrcCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[1];
        //todo
        pAdrcCtx->CurrAeResult.AEMaxRatio = 64.0;
        break;
    case HDR_3X_NUM:
        if(CHECK_ISP_HW_V30()) {
            pAdrcCtx->CurrAeResult.GlobalEnvLv = AecHdrPreResult.GlobalEnvLv[1];
            //todo
            pAdrcCtx->CurrAeResult.AEMaxRatio = 64.0;
        }
        break;
    default:
        LOGE_ATMO("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
        break;
    }

    //Normalize the current envLv for AEC
    pAdrcCtx->CurrData.EnvLv = (pAdrcCtx->CurrAeResult.GlobalEnvLv  - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
    pAdrcCtx->CurrData.EnvLv = LIMIT_VALUE(pAdrcCtx->CurrData.EnvLv, ENVLVMAX, ENVLVMIN);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

void AdrcGetSensorInfo
(
    AdrcContext_t*     pAdrcCtx,
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

    if(pAdrcCtx->FrameNumber == LINEAR_NUM) {
        pAdrcCtx->SensorInfo.MaxExpoL = 0;
        pAdrcCtx->SensorInfo.MinExpoL = 0;
    }
    else if(pAdrcCtx->FrameNumber == HDR_2X_NUM) {
        pAdrcCtx->SensorInfo.MaxExpoL = pAdrcCtx->SensorInfo.HdrMaxGain[1] * pAdrcCtx->SensorInfo.HdrMaxIntegrationTime[1];
        pAdrcCtx->SensorInfo.MinExpoL = pAdrcCtx->SensorInfo.HdrMinGain[1] * pAdrcCtx->SensorInfo.HdrMinIntegrationTime[1];
    }
    else if(pAdrcCtx->FrameNumber == HDR_3X_NUM) {
        pAdrcCtx->SensorInfo.MaxExpoL = pAdrcCtx->SensorInfo.HdrMaxGain[2] * pAdrcCtx->SensorInfo.HdrMaxIntegrationTime[2];
        pAdrcCtx->SensorInfo.MinExpoL = pAdrcCtx->SensorInfo.HdrMinGain[2] * pAdrcCtx->SensorInfo.HdrMinIntegrationTime[2];
    }

    pAdrcCtx->SensorInfo.MaxExpoS = pAdrcCtx->SensorInfo.HdrMaxGain[0] * pAdrcCtx->SensorInfo.HdrMaxIntegrationTime[0];
    pAdrcCtx->SensorInfo.MinExpoS = pAdrcCtx->SensorInfo.HdrMinGain[0] * pAdrcCtx->SensorInfo.HdrMinIntegrationTime[0];

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcIQUpdateV21()
 *
 *****************************************************************************/
void AdrcIQUpdateV21
(
    AdrcContext_t*     pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //store enable
    pAdrcCtx->CurrData.Enable = pAdrcCtx->Config.Drc_v21.Enable;

    //get Drc gain
    pAdrcCtx->CurrData.HandleData.Drc_v21.DrcGain = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv, pAdrcCtx->Config.Drc_v21.DrcGain.DrcGain, pAdrcCtx->Config.Drc_v21.DrcGain.len);
    pAdrcCtx->CurrData.HandleData.Drc_v21.Alpha = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv, pAdrcCtx->Config.Drc_v21.DrcGain.Alpha, pAdrcCtx->Config.Drc_v21.DrcGain.len);
    pAdrcCtx->CurrData.HandleData.Drc_v21.Clip = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv, pAdrcCtx->Config.Drc_v21.DrcGain.Clip, pAdrcCtx->Config.Drc_v21.DrcGain.len);

    //get hi lit
    pAdrcCtx->CurrData.HandleData.Drc_v21.Strength = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v21.HiLit.EnvLv, pAdrcCtx->Config.Drc_v21.HiLit.Strength, pAdrcCtx->Config.Drc_v21.HiLit.len);

    //get local
    pAdrcCtx->CurrData.HandleData.Drc_v21.LocalWeit = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v21.Local.EnvLv, pAdrcCtx->Config.Drc_v21.Local.LocalWeit, pAdrcCtx->Config.Drc_v21.Local.len);
    pAdrcCtx->CurrData.HandleData.Drc_v21.GlobalContrast = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v21.Local.EnvLv, pAdrcCtx->Config.Drc_v21.Local.GlobalContrast, pAdrcCtx->Config.Drc_v21.Local.len);
    pAdrcCtx->CurrData.HandleData.Drc_v21.LoLitContrast = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v21.Local.EnvLv, pAdrcCtx->Config.Drc_v21.Local.LoLitContrast, pAdrcCtx->Config.Drc_v21.Local.len);

    //compress
    pAdrcCtx->CurrData.HandleData.Drc_v21.Mode = pAdrcCtx->Config.Drc_v21.Compress.Mode;
    for(int i = 0; i < ADRC_Y_NUM; i++)
        pAdrcCtx->CurrData.HandleData.Drc_v21.Manual_curve[i] = pAdrcCtx->Config.Drc_v21.Compress.Manual_curve[i];

    //others
    if(0 != memcmp(&pAdrcCtx->CurrData.Others, &pAdrcCtx->Config.Drc_v21.Others, sizeof(DrcOhters_t)))
        memcpy(&pAdrcCtx->CurrData.Others, &pAdrcCtx->Config.Drc_v21.Others, sizeof(DrcOhters_t));//load iq paras

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcIQUpdateV30()
 *
 *****************************************************************************/
void AdrcIQUpdateV30
(
    AdrcContext_t*     pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    //store enable
    pAdrcCtx->CurrData.Enable = pAdrcCtx->Config.Drc_v30.Enable;

    //get Drc gain
    pAdrcCtx->CurrData.HandleData.Drc_v30.DrcGain = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv, pAdrcCtx->Config.Drc_v30.DrcGain.DrcGain, pAdrcCtx->Config.Drc_v30.DrcGain.len);
    pAdrcCtx->CurrData.HandleData.Drc_v30.Alpha = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv, pAdrcCtx->Config.Drc_v30.DrcGain.Alpha, pAdrcCtx->Config.Drc_v30.DrcGain.len);
    pAdrcCtx->CurrData.HandleData.Drc_v30.Clip = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv, pAdrcCtx->Config.Drc_v30.DrcGain.Clip, pAdrcCtx->Config.Drc_v30.DrcGain.len);

    //get hi lit
    pAdrcCtx->CurrData.HandleData.Drc_v30.Strength = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.HiLit.EnvLv, pAdrcCtx->Config.Drc_v30.HiLit.Strength, pAdrcCtx->Config.Drc_v30.HiLit.len);

    //get local
    pAdrcCtx->CurrData.HandleData.Drc_v30.LocalWeit = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.Local.EnvLv, pAdrcCtx->Config.Drc_v30.Local.LocalWeit, pAdrcCtx->Config.Drc_v30.Local.len);
    pAdrcCtx->CurrData.HandleData.Drc_v30.GlobalContrast = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.Local.EnvLv, pAdrcCtx->Config.Drc_v30.Local.GlobalContrast, pAdrcCtx->Config.Drc_v30.Local.len);
    pAdrcCtx->CurrData.HandleData.Drc_v30.LoLitContrast = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.Local.EnvLv, pAdrcCtx->Config.Drc_v30.Local.LoLitContrast, pAdrcCtx->Config.Drc_v30.Local.len);
    pAdrcCtx->CurrData.HandleData.Drc_v30.LocalAutoEnable = DrcGetCurrParaInt(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.Local.EnvLv, pAdrcCtx->Config.Drc_v30.Local.LocalAutoEnable, pAdrcCtx->Config.Drc_v30.Local.len);
    pAdrcCtx->CurrData.HandleData.Drc_v30.LocalAutoWeit = DrcGetCurrPara(pAdrcCtx->CurrData.EnvLv,
            pAdrcCtx->Config.Drc_v30.Local.EnvLv, pAdrcCtx->Config.Drc_v30.Local.LocalAutoWeit, pAdrcCtx->Config.Drc_v30.Local.len);

    //compress
    pAdrcCtx->CurrData.HandleData.Drc_v30.Mode = pAdrcCtx->Config.Drc_v30.Compress.Mode;
    for(int i = 0; i < ADRC_Y_NUM; i++)
        pAdrcCtx->CurrData.HandleData.Drc_v30.Manual_curve[i] = pAdrcCtx->Config.Drc_v30.Compress.Manual_curve[i];

    //others
    if(0 != memcmp(&pAdrcCtx->CurrData.Others, &pAdrcCtx->Config.Drc_v30.Others, sizeof(DrcOhters_t)))
        memcpy(&pAdrcCtx->CurrData.Others, &pAdrcCtx->Config.Drc_v30.Others, sizeof(DrcOhters_t));//load iq paras

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}
/******************************************************************************
 * AdrcProcUpdateConfig()
 *transfer html parameter into handle
 ***************************************************************************/
void AdrcProcUpdateConfig
(
    AdrcContext_t*           pAdrcCtx,
    DrcCalibDB_t*         pCalibDb,
    drcAttr_t*               pDrcAttr
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAdrcCtx != NULL);
    DCT_ASSERT(pDrcAttr != NULL);
    DCT_ASSERT(pCalibDb != NULL);

    if(CHECK_ISP_HW_V21()) {
        if (pAdrcCtx->drcAttr.opMode > DRC_OPMODE_MANU) {
            //drc gain
            if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_DRC_GAIN) {
                pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv[0] = ADRCNORMALIZEMIN;
                pAdrcCtx->Config.Drc_v21.DrcGain.DrcGain[0] = LIMIT_VALUE(pDrcAttr->stDrcGain.DrcGain, DRCGAINMAX, DRCGAINMIN);
                pAdrcCtx->Config.Drc_v21.DrcGain.Alpha[0] = LIMIT_VALUE(pDrcAttr->stDrcGain.Alpha, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                pAdrcCtx->Config.Drc_v21.DrcGain.Clip[0] = LIMIT_VALUE(pDrcAttr->stDrcGain.Clip, CLIPMAX, CLIPMIN);
            }
            else
                for (int i = 0; i < pAdrcCtx->Config.Drc_v21.DrcGain.len; i++ ) {
                    pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.DrcGain.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v21.DrcGain.DrcGain[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.DrcGain.DrcGain[i], DRCGAINMAX, DRCGAINMIN);
                    pAdrcCtx->Config.Drc_v21.DrcGain.Alpha[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.DrcGain.Alpha[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v21.DrcGain.Clip[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);
                }
            //hight ligjt
            if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_HILIT) {
                pAdrcCtx->Config.Drc_v21.HiLit.EnvLv[0] = ADRCNORMALIZEMIN;
                pAdrcCtx->Config.Drc_v21.HiLit.Strength[0] = LIMIT_VALUE(pDrcAttr->stHiLit.Strength, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            }
            else
                for (int i = 0; i < pAdrcCtx->Config.Drc_v21.HiLit.len; i++ ) {
                    pAdrcCtx->Config.Drc_v21.HiLit.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.HiLight.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v21.HiLit.Strength[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.HiLight.Strength[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                }
            // local
            if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_LOCAL_TMO) {
                pAdrcCtx->Config.Drc_v21.Local.EnvLv[0] = ADRCNORMALIZEMIN;
                pAdrcCtx->Config.Drc_v21.Local.LocalWeit[0] = LIMIT_VALUE(pDrcAttr->stLocalDataV21.LocalWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                pAdrcCtx->Config.Drc_v21.Local.GlobalContrast[0] = LIMIT_VALUE(pDrcAttr->stLocalDataV21.GlobalContrast, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                pAdrcCtx->Config.Drc_v21.Local.LoLitContrast[0] = LIMIT_VALUE(pDrcAttr->stLocalDataV21.LoLitContrast, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            }
            else
                for (int i = 0; i < pAdrcCtx->Config.Drc_v21.Local.len; i++ ) {
                    pAdrcCtx->Config.Drc_v21.Local.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v21.Local.LocalWeit[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v21.Local.GlobalContrast[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v21.Local.LoLitContrast[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                }
            //others
            pAdrcCtx->Config.Drc_v21.OutPutLongFrame = pCalibDb->Drc_v21.DrcTuningPara.OutPutLongFrame;
            pAdrcCtx->Config.Drc_v21.Others.curPixWeit = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.curPixWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.preFrameWeit = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.preFrameWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Range_force_sgm = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Range_force_sgm, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Range_sgm_cur = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Range_sgm_cur, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Range_sgm_pre = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Range_sgm_pre, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Space_sgm_cur = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
            pAdrcCtx->Config.Drc_v21.Others.Space_sgm_pre = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
            //compress and scale y
            pAdrcCtx->Config.Drc_v21.Compress.Mode = pCalibDb->Drc_v21.DrcTuningPara.CompressSetting.Mode;
            for(int i = 0; i < ADRC_Y_NUM; i++ ) {
                pAdrcCtx->Config.Drc_v21.Others.Scale_y[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
                pAdrcCtx->Config.Drc_v21.Compress.Manual_curve[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.CompressSetting.Manual_curve[i], MANUALCURVEMAX, MANUALCURVEMIN);
            }
            pAdrcCtx->Config.Drc_v21.Others.ByPassThr = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.ByPassThr, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Edge_Weit = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.Edge_Weit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.IIR_frame = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Tolerance = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.Tolerance, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.damp = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.damp, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        }
        else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_MANU) {
            //drc gain
            pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv[0] = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v21.DrcGain.DrcGain[0] = LIMIT_VALUE(pDrcAttr->stManualV21.DrcGain.DrcGain, DRCGAINMAX, DRCGAINMIN);
            pAdrcCtx->Config.Drc_v21.DrcGain.Alpha[0] = LIMIT_VALUE(pDrcAttr->stManualV21.DrcGain.Alpha, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.DrcGain.Clip[0] = LIMIT_VALUE(pDrcAttr->stManualV21.DrcGain.Clip, CLIPMAX, CLIPMIN);
            //hight ligjt
            pAdrcCtx->Config.Drc_v21.HiLit.EnvLv[0] = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v21.HiLit.Strength[0] = LIMIT_VALUE(pDrcAttr->stManualV21.HiLit.Strength, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            // local
            pAdrcCtx->Config.Drc_v21.Local.EnvLv[0] = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v21.Local.LocalWeit[0] = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.LocalData.LocalWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Local.GlobalContrast[0] = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.LocalData.GlobalContrast, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Local.LoLitContrast[0] = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.LocalData.LoLitContrast, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            //others
            pAdrcCtx->Config.Drc_v21.OutPutLongFrame = pDrcAttr->stManualV21.OutPutLongFrame;
            pAdrcCtx->Config.Drc_v21.Others.curPixWeit = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.curPixWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.preFrameWeit = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.preFrameWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Range_force_sgm = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.Range_force_sgm, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Range_sgm_cur = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.Range_sgm_cur, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Range_sgm_pre = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.Range_sgm_pre, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Space_sgm_cur = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
            pAdrcCtx->Config.Drc_v21.Others.Space_sgm_pre = LIMIT_VALUE(pDrcAttr->stManualV21.LocalSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
            for(int i = 0; i < ADRC_Y_NUM; i++ )
                pAdrcCtx->Config.Drc_v21.Others.Scale_y[i] = LIMIT_VALUE(pDrcAttr->stManualV21.Scale_y[i], SCALEYMAX, SCALEYMIN);
            pAdrcCtx->Config.Drc_v21.Others.ByPassThr = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v21.Others.Edge_Weit = LIMIT_VALUE(pDrcAttr->stManualV21.Edge_Weit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Others.IIR_frame = LIMIT_VALUE(pDrcAttr->stManualV21.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
            pAdrcCtx->Config.Drc_v21.Others.Tolerance = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v21.Others.damp = ADRCNORMALIZEMIN;
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        if(pAdrcCtx->drcAttr.opMode > DRC_OPMODE_MANU)  {
            //drc gain
            if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_DRC_GAIN) {
                pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv[0] = ADRCNORMALIZEMIN;
                pAdrcCtx->Config.Drc_v30.DrcGain.DrcGain[0] = LIMIT_VALUE(pDrcAttr->stDrcGain.DrcGain, DRCGAINMAX, DRCGAINMIN);
                pAdrcCtx->Config.Drc_v30.DrcGain.Alpha[0] = LIMIT_VALUE(pDrcAttr->stDrcGain.Alpha, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                pAdrcCtx->Config.Drc_v30.DrcGain.Clip[0] = LIMIT_VALUE(pDrcAttr->stDrcGain.Clip, CLIPMAX, CLIPMIN);
            }
            else
                for (int i = 0; i < pAdrcCtx->Config.Drc_v30.DrcGain.len; i++ ) {
                    pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.DrcGain.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v30.DrcGain.DrcGain[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.DrcGain.DrcGain[i], DRCGAINMAX, DRCGAINMIN);
                    pAdrcCtx->Config.Drc_v30.DrcGain.Alpha[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.DrcGain.Alpha[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v30.DrcGain.Clip[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);
                }
            //hight light
            if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_DRC_GAIN) {
                pAdrcCtx->Config.Drc_v30.HiLit.EnvLv[0] = ADRCNORMALIZEMIN;
                pAdrcCtx->Config.Drc_v30.HiLit.Strength[0] = LIMIT_VALUE(pDrcAttr->stHiLit.Strength, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            }
            else
                for (int i = 0; i < pAdrcCtx->Config.Drc_v30.HiLit.len; i++ ) {
                    pAdrcCtx->Config.Drc_v30.HiLit.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.HiLight.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v30.HiLit.Strength[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.HiLight.Strength[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                }
            // local
            if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_DRC_GAIN) {
                pAdrcCtx->Config.Drc_v30.Local.EnvLv[0] = ADRCNORMALIZEMIN;
                pAdrcCtx->Config.Drc_v30.Local.LocalWeit[0] = LIMIT_VALUE(pDrcAttr->stLocalDataV30.LocalWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                pAdrcCtx->Config.Drc_v30.Local.LocalAutoEnable[0] = LIMIT_VALUE(pDrcAttr->stLocalDataV30.LocalAutoEnable, ADRCNORMALIZEINTMAX, ADRCNORMALIZEINTMIN);
                pAdrcCtx->Config.Drc_v30.Local.LocalAutoWeit[0] = LIMIT_VALUE(pDrcAttr->stLocalDataV30.LocalAutoWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                pAdrcCtx->Config.Drc_v30.Local.GlobalContrast[0] = LIMIT_VALUE(pDrcAttr->stLocalDataV30.GlobalContrast, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                pAdrcCtx->Config.Drc_v30.Local.LoLitContrast[0] = LIMIT_VALUE(pDrcAttr->stLocalDataV30.LoLitContrast, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            }
            else
                for (int i = 0; i < pAdrcCtx->Config.Drc_v30.Local.len; i++ ) {
                    pAdrcCtx->Config.Drc_v30.Local.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v30.Local.LocalWeit[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v30.Local.LocalAutoEnable[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i], ADRCNORMALIZEINTMAX, ADRCNORMALIZEINTMIN);
                    pAdrcCtx->Config.Drc_v30.Local.LocalAutoWeit[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v30.Local.GlobalContrast[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                    pAdrcCtx->Config.Drc_v30.Local.LoLitContrast[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
                }
            //others
            pAdrcCtx->Config.Drc_v30.OutPutLongFrame = pCalibDb->Drc_v30.DrcTuningPara.OutPutLongFrame;
            pAdrcCtx->Config.Drc_v30.Others.curPixWeit = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.curPixWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.preFrameWeit = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.preFrameWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Range_force_sgm = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Range_force_sgm, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Range_sgm_cur = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Range_sgm_cur, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Range_sgm_pre = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Range_sgm_pre, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Space_sgm_cur = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
            pAdrcCtx->Config.Drc_v30.Others.Space_sgm_pre = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
            //compress and scale y
            pAdrcCtx->Config.Drc_v30.Compress.Mode = pCalibDb->Drc_v30.DrcTuningPara.CompressSetting.Mode;
            for(int i = 0; i < ADRC_Y_NUM; i++ ) {
                pAdrcCtx->Config.Drc_v30.Others.Scale_y[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
                pAdrcCtx->Config.Drc_v30.Compress.Manual_curve[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.CompressSetting.Manual_curve[i], MANUALCURVEMAX, MANUALCURVEMIN);
            }
            pAdrcCtx->Config.Drc_v30.Others.ByPassThr = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.ByPassThr, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Edge_Weit = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.Edge_Weit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.IIR_frame = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Tolerance = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.Tolerance, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.damp = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.damp, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        }
        else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_MANU) {
            //drc gain
            pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv[0] = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v30.DrcGain.DrcGain[0] = LIMIT_VALUE(pDrcAttr->stManualV30.DrcGain.DrcGain, DRCGAINMAX, DRCGAINMIN);
            pAdrcCtx->Config.Drc_v30.DrcGain.Alpha[0] = LIMIT_VALUE(pDrcAttr->stManualV30.DrcGain.Alpha, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.DrcGain.Clip[0] = LIMIT_VALUE(pDrcAttr->stManualV30.DrcGain.Clip, CLIPMAX, CLIPMIN);
            //hight light
            pAdrcCtx->Config.Drc_v30.HiLit.EnvLv[0] = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v30.HiLit.Strength[0] = LIMIT_VALUE(pDrcAttr->stManualV30.HiLight.Strength, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            // local
            pAdrcCtx->Config.Drc_v30.Local.EnvLv[0] = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v30.Local.LocalWeit[0] = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.LocalData.LocalWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Local.LocalAutoEnable[0] = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.LocalData.LocalAutoEnable, ADRCNORMALIZEINTMAX, ADRCNORMALIZEINTMIN);
            pAdrcCtx->Config.Drc_v30.Local.LocalAutoWeit[0] = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.LocalData.LocalAutoWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Local.GlobalContrast[0] = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.LocalData.GlobalContrast, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Local.LoLitContrast[0] = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.LocalData.LoLitContrast, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            //others
            pAdrcCtx->Config.Drc_v30.OutPutLongFrame = pDrcAttr->stManualV30.OutPutLongFrame;
            pAdrcCtx->Config.Drc_v30.Others.curPixWeit = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.curPixWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.preFrameWeit = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.preFrameWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Range_force_sgm = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.Range_force_sgm, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Range_sgm_cur = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.Range_sgm_cur, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Range_sgm_pre = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.Range_sgm_pre, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Space_sgm_cur = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
            pAdrcCtx->Config.Drc_v30.Others.Space_sgm_pre = LIMIT_VALUE(pDrcAttr->stManualV30.LocalSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
            for(int i = 0; i < ADRC_Y_NUM; i++ )
                pAdrcCtx->Config.Drc_v30.Others.Scale_y[i] = LIMIT_VALUE(pDrcAttr->stManualV30.Scale_y[i], SCALEYMAX, SCALEYMIN);
            pAdrcCtx->Config.Drc_v30.Others.ByPassThr = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v30.Others.Edge_Weit = LIMIT_VALUE(pDrcAttr->stManualV30.Edge_Weit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Others.IIR_frame = LIMIT_VALUE(pDrcAttr->stManualV30.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
            pAdrcCtx->Config.Drc_v30.Others.Tolerance = ADRCNORMALIZEMIN;
            pAdrcCtx->Config.Drc_v30.Others.damp = ADRCNORMALIZEMIN;
        }
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}
/******************************************************************************
 * AdrcPrePareJsonUpdateConfig()
 *transfer html parameter into handle
 ***************************************************************************/
void AdrcPrePareJsonUpdateConfig
(
    AdrcContext_t*           pAdrcCtx,
    DrcCalibDB_t*         pCalibDb
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAdrcCtx != NULL);
    DCT_ASSERT(pCalibDb != NULL);

    if(CHECK_ISP_HW_V21()) {
        //drc gain
        for (int i = 0; i < pAdrcCtx->Config.Drc_v21.DrcGain.len; i++ ) {
            pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.DrcGain.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.DrcGain.DrcGain[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.DrcGain.DrcGain[i], DRCGAINMAX, DRCGAINMIN);
            pAdrcCtx->Config.Drc_v21.DrcGain.Alpha[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.DrcGain.Alpha[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.DrcGain.Clip[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);
        }
        //hight light
        for (int i = 0; i < pAdrcCtx->Config.Drc_v21.HiLit.len; i++ ) {
            pAdrcCtx->Config.Drc_v21.HiLit.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.HiLight.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.HiLit.Strength[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.HiLight.Strength[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        }
        // local
        for (int i = 0; i < pAdrcCtx->Config.Drc_v21.Local.len; i++ ) {
            pAdrcCtx->Config.Drc_v21.Local.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Local.LocalWeit[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Local.GlobalContrast[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v21.Local.LoLitContrast[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        }
        //others
        pAdrcCtx->Config.Drc_v21.OutPutLongFrame = pCalibDb->Drc_v21.DrcTuningPara.OutPutLongFrame;
        pAdrcCtx->Config.Drc_v21.Others.curPixWeit = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.curPixWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v21.Others.preFrameWeit = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.preFrameWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v21.Others.Range_force_sgm = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Range_force_sgm, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v21.Others.Range_sgm_cur = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Range_sgm_cur, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v21.Others.Range_sgm_pre = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Range_sgm_pre, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v21.Others.Space_sgm_cur = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
        pAdrcCtx->Config.Drc_v21.Others.Space_sgm_pre = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.LocalTMOSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
        //compress and scale y
        pAdrcCtx->Config.Drc_v21.Compress.Mode = pCalibDb->Drc_v21.DrcTuningPara.CompressSetting.Mode;
        for(int i = 0; i < ADRC_Y_NUM; i++ ) {
            pAdrcCtx->Config.Drc_v21.Others.Scale_y[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
            pAdrcCtx->Config.Drc_v21.Compress.Manual_curve[i] = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.CompressSetting.Manual_curve[i], MANUALCURVEMAX, MANUALCURVEMIN);
        }
        pAdrcCtx->Config.Drc_v21.Others.ByPassThr = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.ByPassThr, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v21.Others.Edge_Weit = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.Edge_Weit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v21.Others.IIR_frame = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
        pAdrcCtx->Config.Drc_v21.Others.Tolerance = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.Tolerance, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v21.Others.damp = LIMIT_VALUE(pCalibDb->Drc_v21.DrcTuningPara.damp, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    }
    else if(CHECK_ISP_HW_V30()) {
        //drc gain
        for (int i = 0; i < pAdrcCtx->Config.Drc_v30.DrcGain.len; i++ ) {
            pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.DrcGain.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.DrcGain.DrcGain[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.DrcGain.DrcGain[i], DRCGAINMAX, DRCGAINMIN);
            pAdrcCtx->Config.Drc_v30.DrcGain.Alpha[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.DrcGain.Alpha[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.DrcGain.Clip[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);
        }
        //hight light
        for (int i = 0; i < pAdrcCtx->Config.Drc_v30.HiLit.len; i++ ) {
            pAdrcCtx->Config.Drc_v30.HiLit.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.HiLight.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.HiLit.Strength[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.HiLight.Strength[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        }
        // local
        for (int i = 0; i < pAdrcCtx->Config.Drc_v30.Local.len; i++ ) {
            pAdrcCtx->Config.Drc_v30.Local.EnvLv[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.EnvLv[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Local.LocalWeit[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Local.LocalAutoEnable[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i], ADRCNORMALIZEINTMAX, ADRCNORMALIZEINTMIN);
            pAdrcCtx->Config.Drc_v30.Local.LocalAutoWeit[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Local.GlobalContrast[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
            pAdrcCtx->Config.Drc_v30.Local.LoLitContrast[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i], ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        }
        //others
        pAdrcCtx->Config.Drc_v30.OutPutLongFrame = pCalibDb->Drc_v30.DrcTuningPara.OutPutLongFrame;
        pAdrcCtx->Config.Drc_v30.Others.curPixWeit = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.curPixWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v30.Others.preFrameWeit = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.preFrameWeit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v30.Others.Range_force_sgm = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Range_force_sgm, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v30.Others.Range_sgm_cur = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Range_sgm_cur, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v30.Others.Range_sgm_pre = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Range_sgm_pre, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v30.Others.Space_sgm_cur = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
        pAdrcCtx->Config.Drc_v30.Others.Space_sgm_pre = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.LocalSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
        //compress and scale y
        pAdrcCtx->Config.Drc_v30.Compress.Mode = pCalibDb->Drc_v30.DrcTuningPara.CompressSetting.Mode;
        for(int i = 0; i < ADRC_Y_NUM; i++ ) {
            pAdrcCtx->Config.Drc_v30.Others.Scale_y[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
            pAdrcCtx->Config.Drc_v30.Compress.Manual_curve[i] = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.CompressSetting.Manual_curve[i], MANUALCURVEMAX, MANUALCURVEMIN);
        }
        pAdrcCtx->Config.Drc_v30.Others.ByPassThr = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.ByPassThr, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v30.Others.Edge_Weit = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.Edge_Weit, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v30.Others.IIR_frame = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
        pAdrcCtx->Config.Drc_v30.Others.Tolerance = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.Tolerance, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
        pAdrcCtx->Config.Drc_v30.Others.damp = LIMIT_VALUE(pCalibDb->Drc_v30.DrcTuningPara.damp, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * DrcEnableSetting()
 *
 *****************************************************************************/
void DrcEnableSetting
(
    AdrcContext_t* pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    if(CHECK_ISP_HW_V21()) {
        if(pAdrcCtx->FrameNumber == HDR_2X_NUM)
            pAdrcCtx->Config.Drc_v21.Enable = true;
        else if(pAdrcCtx->FrameNumber == LINEAR_NUM) {
            if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_API_OFF)
                pAdrcCtx->Config.Drc_v21.Enable = pAdrcCtx->pCalibDB.Drc_v21.DrcTuningPara.Enable;
            else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_MANU)
                pAdrcCtx->Config.Drc_v21.Enable = pAdrcCtx->drcAttr.stManualV21.Enable;
            else if(pAdrcCtx->drcAttr.opMode >= DRC_OPMODE_DRC_GAIN && pAdrcCtx->drcAttr.opMode <= DRC_OPMODE_LOCAL_TMO)
                pAdrcCtx->Config.Drc_v21.Enable = true;
            else {
                LOGE_ATMO( "%s: Drc api in WRONG MODE!!!, drc by pass!!!\n", __FUNCTION__);
                pAdrcCtx->Config.Drc_v21.Enable = false;
            }
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        if(pAdrcCtx->FrameNumber == HDR_2X_NUM || pAdrcCtx->FrameNumber == HDR_3X_NUM)
            pAdrcCtx->Config.Drc_v30.Enable = true;
        else if(pAdrcCtx->FrameNumber == LINEAR_NUM) {
            if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_API_OFF)
                pAdrcCtx->Config.Drc_v30.Enable = pAdrcCtx->pCalibDB.Drc_v30.DrcTuningPara.Enable;
            else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_MANU)
                pAdrcCtx->Config.Drc_v30.Enable = pAdrcCtx->drcAttr.stManualV30.Enable;
            else if(pAdrcCtx->drcAttr.opMode >= DRC_OPMODE_DRC_GAIN && pAdrcCtx->drcAttr.opMode <= DRC_OPMODE_LOCAL_TMO)
                pAdrcCtx->Config.Drc_v30.Enable = true;
            else {
                LOGE_ATMO( "%s: Drc api in WRONG MODE!!!, drc by pass!!!\n", __FUNCTION__);
                pAdrcCtx->Config.Drc_v30.Enable = false;
            }
        }
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcDampingV21()
 *****************************************************************************/
void AdrcDampingV21
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

    if(opMode != DRC_OPMODE_MANU && FrameCnt != 0 && !ifHDRModeChange)
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
            if(opMode != DRC_OPMODE_DRC_GAIN) {
                pCurrData->HandleData.Drc_v21.DrcGain = Drc_damp * pCurrData->HandleData.Drc_v21.DrcGain
                                                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v21.DrcGain;
                pCurrData->HandleData.Drc_v21.Alpha = Drc_damp * pCurrData->HandleData.Drc_v21.Alpha
                                                      + (1 - Drc_damp) * pPreData->HandleData.Drc_v21.Alpha;
                pCurrData->HandleData.Drc_v21.Clip = Drc_damp * pCurrData->HandleData.Drc_v21.Clip
                                                     + (1 - Drc_damp) * pPreData->HandleData.Drc_v21.Clip;
            }
            if(opMode != DRC_OPMODE_HILIT)
                pCurrData->HandleData.Drc_v21.Strength = Drc_damp * pCurrData->HandleData.Drc_v21.Strength
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v21.Strength;
            if(opMode != DRC_OPMODE_LOCAL_TMO) {
                pCurrData->HandleData.Drc_v21.LocalWeit = Drc_damp * pCurrData->HandleData.Drc_v21.LocalWeit
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v21.LocalWeit;
                pCurrData->HandleData.Drc_v21.GlobalContrast = Drc_damp * pCurrData->HandleData.Drc_v21.GlobalContrast
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v21.GlobalContrast;
                pCurrData->HandleData.Drc_v21.LoLitContrast = Drc_damp * pCurrData->HandleData.Drc_v21.LoLitContrast
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v21.LoLitContrast;
            }
        }
    }

    LOGD_ATMO("%s:%d: Current damp DrcGain:%f Alpha:%f Clip:%f Strength:%f LocalWeit:%f GlobalContrast:%f LoLitContrast:%f CompressMode:%d\n", __FUNCTION__, __LINE__,
              pCurrData->HandleData.Drc_v21.DrcGain, pCurrData->HandleData.Drc_v21.Alpha, pCurrData->HandleData.Drc_v21.Clip, pCurrData->HandleData.Drc_v21.Strength,
              pCurrData->HandleData.Drc_v21.LocalWeit, pCurrData->HandleData.Drc_v21.GlobalContrast, pCurrData->HandleData.Drc_v21.LoLitContrast, pCurrData->HandleData.Drc_v21.Mode);

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}
/******************************************************************************
 * AdrcDamping()
 *****************************************************************************/
void AdrcDampingV30
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

    if(opMode != DRC_OPMODE_MANU && FrameCnt != 0 && !ifHDRModeChange)
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
            if(opMode != DRC_OPMODE_DRC_GAIN) {
                pCurrData->HandleData.Drc_v30.DrcGain = Drc_damp * pCurrData->HandleData.Drc_v30.DrcGain
                                                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v30.DrcGain;
                pCurrData->HandleData.Drc_v30.Alpha = Drc_damp * pCurrData->HandleData.Drc_v30.Alpha
                                                      + (1 - Drc_damp) * pPreData->HandleData.Drc_v30.Alpha;
                pCurrData->HandleData.Drc_v30.Clip = Drc_damp * pCurrData->HandleData.Drc_v30.Clip
                                                     + (1 - Drc_damp) * pPreData->HandleData.Drc_v30.Clip;
            }
            if(opMode != DRC_OPMODE_HILIT)
                pCurrData->HandleData.Drc_v30.Strength = Drc_damp * pCurrData->HandleData.Drc_v30.Strength
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v30.Strength;
            if(opMode != DRC_OPMODE_LOCAL_TMO) {
                pCurrData->HandleData.Drc_v30.LocalWeit = Drc_damp * pCurrData->HandleData.Drc_v30.LocalWeit
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v30.LocalWeit;
                pCurrData->HandleData.Drc_v30.LocalAutoWeit = Drc_damp * pCurrData->HandleData.Drc_v30.LocalAutoWeit
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v30.LocalAutoWeit;
                pCurrData->HandleData.Drc_v30.GlobalContrast = Drc_damp * pCurrData->HandleData.Drc_v30.GlobalContrast
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v30.GlobalContrast;
                pCurrData->HandleData.Drc_v30.LoLitContrast = Drc_damp * pCurrData->HandleData.Drc_v30.LoLitContrast
                        + (1 - Drc_damp) * pPreData->HandleData.Drc_v30.LoLitContrast;
            }
        }
    }

    LOGD_ATMO("%s: Current damp DrcGain:%f Alpha:%f Clip:%f Strength:%f CompressMode:%d\n", __FUNCTION__,
              pCurrData->HandleData.Drc_v30.DrcGain, pCurrData->HandleData.Drc_v30.Alpha, pCurrData->HandleData.Drc_v30.Clip, pCurrData->HandleData.Drc_v30.Strength,
              pCurrData->HandleData.Drc_v30.Mode);
    LOGD_ATMO("%s: Current damp LocalWeit:%f LocalAutoEnable:%d LocalAutoWeit:%f GlobalContrast:%f LoLitContrast:%f\n", __FUNCTION__,
              pCurrData->HandleData.Drc_v30.LocalWeit, pCurrData->HandleData.Drc_v30.LocalAutoEnable, pCurrData->HandleData.Drc_v30.LocalAutoWeit,
              pCurrData->HandleData.Drc_v30.GlobalContrast, pCurrData->HandleData.Drc_v30.LoLitContrast);

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcGetTuningProcResV21()
 *****************************************************************************/
void AdrcGetTuningProcResV21
(
    AdrcProcResData_t*           pAdrcProcRes,
    CurrData_t*          pCurrData,
    bool LongFrmMode,
    int FrameNumber,
    int FrameCnt
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    //enable
    if(FrameNumber == HDR_2X_NUM || FrameNumber == HDR_3X_NUM) {
        pAdrcProcRes->bTmoEn = true;
        pAdrcProcRes->isLinearTmo = false;
    }
    else if(FrameNumber == LINEAR_NUM) {
        pAdrcProcRes->bTmoEn = pCurrData->Enable;
        pAdrcProcRes->isLinearTmo = pAdrcProcRes->bTmoEn;
    }

    //Long Frame mode
    pAdrcProcRes->LongFrameMode = LongFrmMode;

    //Global tmo
    pAdrcProcRes->isHdrGlobalTmo = pCurrData->HandleData.Drc_v21.LocalWeit == 0 ? true : false;

    //compress mode
    pAdrcProcRes->CompressMode = pCurrData->HandleData.Drc_v21.Mode;

    //DrcProcRes
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_offset_pow2 = SW_DRC_OFFSET_POW2_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_position = (int)(SHIFT8BIT(pCurrData->HandleData.Drc_v21.Clip) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_hpdetail_ratio = (int)(SHIFT12BIT(pCurrData->HandleData.Drc_v21.LoLitContrast) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_lpdetail_ratio = (int)(SHIFT12BIT(pCurrData->HandleData.Drc_v21.GlobalContrast) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_weicur_pix = (int)(SHIFT8BIT(pCurrData->Others.curPixWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_weipre_frame = (int)(SHIFT8BIT(pCurrData->Others.preFrameWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_weipre_frame = LIMIT_VALUE(pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_weipre_frame, 255, 0);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_force_sgm_inv0 = (int)(SHIFT13BIT(pCurrData->Others.Range_force_sgm) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_edge_scl = (int)(SHIFT8BIT(pCurrData->Others.Edge_Weit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_motion_scl = SW_DRC_MOTION_SCL_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_space_sgm_inv1 = (int)(pCurrData->Others.Space_sgm_cur);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_space_sgm_inv0 = (int)(pCurrData->Others.Space_sgm_pre);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_range_sgm_inv1 = (int)(SHIFT13BIT(pCurrData->Others.Range_sgm_cur) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_range_sgm_inv0 = (int)(SHIFT13BIT(pCurrData->Others.Range_sgm_pre) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_weig_maxl = (int)(SHIFT4BIT(pCurrData->HandleData.Drc_v21.Strength) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_adrc_gain = pCurrData->HandleData.Drc_v21.DrcGain;
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_weig_bilat = (int)(SHIFT4BIT(pCurrData->HandleData.Drc_v21.LocalWeit) + 0.5);
    for(int i = 0; i < ADRC_Y_NUM; ++i) {
        pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_scale_y[i] = (int)(pCurrData->Others.Scale_y[i]) ;
        pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_compres_y[i] = pCurrData->HandleData.Drc_v21.Manual_curve[i] ;
    }

    //get sw_drc_gain_y
    CalibrateDrcGainY(&pAdrcProcRes->DrcProcRes, pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_adrc_gain, pCurrData->HandleData.Drc_v21.Alpha) ;

    float iir_frame = (float)(MIN(FrameCnt + 1, pCurrData->Others.IIR_frame));
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_iir_weight = (int)(SHIFT6BIT((iir_frame - 1) / iir_frame) + 0.5);

    //sw_drc_delta_scalein FIX
    pAdrcProcRes->DrcProcRes.Drc_v21.sw_drc_delta_scalein = DELTA_SCALEIN_FIX;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcGetTuningProcResV30()
 *****************************************************************************/
void AdrcGetTuningProcResV30
(
    AdrcProcResData_t*           pAdrcProcRes,
    CurrData_t*          pCurrData,
    bool LongFrmMode,
    int FrameNumber,
    int FrameCnt
) {
    LOG1_ATMO( "%s:enter!\n", __FUNCTION__);

    //enable
    if(FrameNumber == HDR_2X_NUM || FrameNumber == HDR_3X_NUM) {
        pAdrcProcRes->bTmoEn = true;
        pAdrcProcRes->isLinearTmo = false;
    }
    else if(FrameNumber == LINEAR_NUM) {
        pAdrcProcRes->bTmoEn = pCurrData->Enable;
        pAdrcProcRes->isLinearTmo = pAdrcProcRes->bTmoEn;
    }

    //Long Frame mode
    pAdrcProcRes->LongFrameMode = LongFrmMode;

    //Global tmo
    pAdrcProcRes->isHdrGlobalTmo = pCurrData->HandleData.Drc_v30.LocalWeit == 0 ? true : false;

    //compress mode
    pAdrcProcRes->CompressMode = pCurrData->HandleData.Drc_v30.Mode;

    //DrcProcRes
    pAdrcProcRes->DrcProcRes.Drc_v30.bypass_en = FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v30.offset_pow2 = SW_DRC_OFFSET_POW2_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v30.position = (int)(SHIFT8BIT(pCurrData->HandleData.Drc_v30.Clip) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.hpdetail_ratio = (int)(SHIFT12BIT(pCurrData->HandleData.Drc_v30.LoLitContrast) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.lpdetail_ratio = (int)(SHIFT12BIT(pCurrData->HandleData.Drc_v30.GlobalContrast) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.weicur_pix = (int)(SHIFT8BIT(pCurrData->Others.curPixWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.weipre_frame = (int)(SHIFT8BIT(pCurrData->Others.preFrameWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.weipre_frame = LIMIT_VALUE(pAdrcProcRes->DrcProcRes.Drc_v30.weipre_frame, INT8BITMAX, 0);
    pAdrcProcRes->DrcProcRes.Drc_v30.bilat_wt_off = SW_DRC_BILAT_WT_OFF_FIX;//LIMIT_VALUE(pCurrData->HandleData.Drc_v30.MotionStr * INT8BITMAX, INT8BITMAX, 0);
    pAdrcProcRes->DrcProcRes.Drc_v30.force_sgm_inv0 = (int)(SHIFT13BIT(pCurrData->Others.Range_force_sgm) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.edge_scl = (int)(SHIFT8BIT(pCurrData->Others.Edge_Weit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.motion_scl = SW_DRC_MOTION_SCL_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v30.space_sgm_inv1 = (int)(pCurrData->Others.Space_sgm_cur);
    pAdrcProcRes->DrcProcRes.Drc_v30.space_sgm_inv0 = (int)(pCurrData->Others.Space_sgm_pre);
    pAdrcProcRes->DrcProcRes.Drc_v30.range_sgm_inv1 = (int)(SHIFT13BIT(pCurrData->Others.Range_sgm_cur) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.range_sgm_inv0 = (int)(SHIFT13BIT(pCurrData->Others.Range_sgm_pre) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.weig_maxl = (int)(SHIFT4BIT(pCurrData->HandleData.Drc_v30.Strength) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.adrc_gain = pCurrData->HandleData.Drc_v30.DrcGain;
    pAdrcProcRes->DrcProcRes.Drc_v30.weig_bilat = (int)(SHIFT4BIT(pCurrData->HandleData.Drc_v30.LocalWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.enable_soft_thd = pCurrData->HandleData.Drc_v30.LocalAutoEnable;
    pAdrcProcRes->DrcProcRes.Drc_v30.bilat_soft_thd = (int)(SHIFT14BIT(pCurrData->HandleData.Drc_v30.LocalAutoWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v30.bilat_soft_thd = LIMIT_VALUE(pAdrcProcRes->DrcProcRes.Drc_v30.bilat_soft_thd, INT14BITMAX, 0);
    for(int i = 0; i < ADRC_Y_NUM; ++i) {
        pAdrcProcRes->DrcProcRes.Drc_v30.scale_y[i] = (int)(pCurrData->Others.Scale_y[i]) ;
        pAdrcProcRes->DrcProcRes.Drc_v30.compres_y[i] = pCurrData->HandleData.Drc_v30.Manual_curve[i] ;
    }

    //get sw_drc_gain_y
    CalibrateDrcGainY(&pAdrcProcRes->DrcProcRes, pAdrcProcRes->DrcProcRes.Drc_v30.adrc_gain, pCurrData->HandleData.Drc_v30.Alpha) ;

    pAdrcProcRes->DrcProcRes.Drc_v30.wr_cycle = FUNCTION_DISABLE;
    float iir_frame = (float)(MIN(FrameCnt + 1, pCurrData->Others.IIR_frame));
    pAdrcProcRes->DrcProcRes.Drc_v30.iir_weight = (int)(SHIFT6BIT((iir_frame - 1) / iir_frame) + 0.5);

    //sw_drc_delta_scalein FIX
    pAdrcProcRes->DrcProcRes.Drc_v30.delta_scalein = DELTA_SCALEIN_FIX;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcTuningParaProcessingV21()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcTuningParaProcessingV21
(
    AdrcContext_t*     pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    pAdrcCtx->CurrData.frameCnt = pAdrcCtx->frameCnt;

    //para setting
    AdrcIQUpdateV21(pAdrcCtx);

    //api
    if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_API_OFF)
        LOGD_ATMO("%s:  Adrc api OFF!! Current Handle data:\n", __FUNCTION__);
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_MANU) {
        LOGD_ATMO("%s:  Adrc api Manual!! Current Handle data:\n", __FUNCTION__);
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_DRC_GAIN) {
        LOGD_ATMO("%s: Adrc api DRC Gain!! Data from api: DrcGain:%f Alpha:%f Clip:%f\n", __FUNCTION__, pAdrcCtx->drcAttr.stDrcGain.DrcGain,
                  pAdrcCtx->drcAttr.stDrcGain.Alpha, pAdrcCtx->drcAttr.stDrcGain.Clip);
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_HILIT) {
        LOGD_ATMO("%s: Adrc api HiLit!! Data from api: HiLitStrength:%f\n", __FUNCTION__, pAdrcCtx->drcAttr.stHiLit.Strength);
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_LOCAL_TMO) {
        LOGD_ATMO("%s: Adrc api Local!! Data from api: LocalWeit:%f GlobalContrast:%f LoLitContrast:%f\n", __FUNCTION__,
                  pAdrcCtx->drcAttr.stLocalDataV21.LocalWeit, pAdrcCtx->drcAttr.stLocalDataV21.GlobalContrast,
                  pAdrcCtx->drcAttr.stLocalDataV21.LoLitContrast);
    }
    else
        LOGE_ATMO("%s:  Adrc wrong mode!!!\n", __FUNCTION__);

    //clip drc gain
    if(pAdrcCtx->CurrAeResult.AEMaxRatio * pAdrcCtx->CurrData.HandleData.Drc_v21.DrcGain > MAX_AE_DRC_GAIN) {
        LOGE_ATMO("%s:  AERatio*DrcGain > 256!!!\n", __FUNCTION__);
        pAdrcCtx->CurrData.HandleData.Drc_v21.DrcGain = MAX_AE_DRC_GAIN / pAdrcCtx->CurrAeResult.AEMaxRatio;
    }

    LOGD_ATMO("%s:Current Enable:%d DrcGain:%f Alpha:%f Clip:%f Strength:%f LocalWeit:%f GlobalContrast:%f LoLitContrast:%f CompressMode:%d\n", __FUNCTION__, pAdrcCtx->CurrData.Enable,
              pAdrcCtx->CurrData.HandleData.Drc_v21.DrcGain, pAdrcCtx->CurrData.HandleData.Drc_v21.Alpha, pAdrcCtx->CurrData.HandleData.Drc_v21.Clip, pAdrcCtx->CurrData.HandleData.Drc_v21.Strength,
              pAdrcCtx->CurrData.HandleData.Drc_v21.LocalWeit, pAdrcCtx->CurrData.HandleData.Drc_v21.GlobalContrast, pAdrcCtx->CurrData.HandleData.Drc_v21.LoLitContrast
              , pAdrcCtx->CurrData.HandleData.Drc_v21.Mode);

    //transfer data to api
    //info
    pAdrcCtx->drcAttr.Info.EnvLv = pAdrcCtx->CurrData.EnvLv;

    //damp
    AdrcDampingV21(&pAdrcCtx->CurrData, &pAdrcCtx->PrevData, pAdrcCtx->drcAttr.opMode, pAdrcCtx->frameCnt);

    //get tuning proc res
    AdrcGetTuningProcResV21(&pAdrcCtx->AdrcProcRes, &pAdrcCtx->CurrData, pAdrcCtx->SensorInfo.LongFrmMode,
                            pAdrcCtx->FrameNumber, pAdrcCtx->frameCnt);

    // store current handle data to pre data for next loop
    pAdrcCtx->PrevData.EnvLv = pAdrcCtx->CurrData.EnvLv;
    pAdrcCtx->PrevData.ISO = pAdrcCtx->CurrData.ISO;
    if(0 != memcmp(&pAdrcCtx->PrevData.HandleData, &pAdrcCtx->CurrData.HandleData, sizeof(DrcHandleData_t)))
        memcpy(&pAdrcCtx->PrevData.HandleData, &pAdrcCtx->CurrData.HandleData, sizeof(DrcHandleData_t));

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcTuningParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcTuningParaProcessingV30
(
    AdrcContext_t*     pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    pAdrcCtx->CurrData.frameCnt = pAdrcCtx->frameCnt;

    //para setting
    AdrcIQUpdateV30(pAdrcCtx);

    //api
    if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_API_OFF)
        LOG1_ATMO("%s: Adrc api OFF!! Current Handle data:\n", __FUNCTION__);
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_MANU) {
        LOG1_ATMO("%s: Adrc api Manual!! Current Handle data:\n", __FUNCTION__);
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_DRC_GAIN) {
        LOGD_ATMO("%s: Adrc api DRC Gain!! Data from api: DrcGain:%f Alpha:%f Clip:%f\n", __FUNCTION__, pAdrcCtx->drcAttr.stDrcGain.DrcGain,
                  pAdrcCtx->drcAttr.stDrcGain.Alpha, pAdrcCtx->drcAttr.stDrcGain.Clip);
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_HILIT) {
        LOGD_ATMO("%s: Adrc api HiLit!! Data from api: HiLitStrength:%f\n", __FUNCTION__, pAdrcCtx->drcAttr.stHiLit.Strength);
    }
    else if(pAdrcCtx->drcAttr.opMode == DRC_OPMODE_LOCAL_TMO) {
        LOGD_ATMO("%s: Adrc api Local!! Data from api: LocalWeit:%f LocalAutoEnable:%f LocalAutoWeit:%f GlobalContrast:%f LoLitContrast:%f\n", __FUNCTION__,
                  pAdrcCtx->drcAttr.stLocalDataV30.LocalWeit, pAdrcCtx->drcAttr.stLocalDataV30.LocalAutoEnable,
                  pAdrcCtx->drcAttr.stLocalDataV30.LocalAutoWeit, pAdrcCtx->drcAttr.stLocalDataV30.GlobalContrast,
                  pAdrcCtx->drcAttr.stLocalDataV30.LoLitContrast);
    }
    else
        LOGE_ATMO("%s:  Adrc wrong mode!!!\n", __FUNCTION__);

    //clip drc gain
    if(pAdrcCtx->CurrAeResult.AEMaxRatio * pAdrcCtx->CurrData.HandleData.Drc_v30.DrcGain > MAX_AE_DRC_GAIN) {
        LOGE_ATMO("%s:  AERatio*DrcGain > 256!!!\n", __FUNCTION__);
        pAdrcCtx->CurrData.HandleData.Drc_v30.DrcGain = MAX_AE_DRC_GAIN / pAdrcCtx->CurrAeResult.AEMaxRatio;
    }

    LOGD_ATMO("%s: Current Enable:%d DrcGain:%f Alpha:%f Clip:%f Strength:%f CompressMode:%d\n", __FUNCTION__, pAdrcCtx->CurrData.Enable,
              pAdrcCtx->CurrData.HandleData.Drc_v30.DrcGain, pAdrcCtx->CurrData.HandleData.Drc_v30.Alpha, pAdrcCtx->CurrData.HandleData.Drc_v30.Clip,
              pAdrcCtx->CurrData.HandleData.Drc_v30.Strength, pAdrcCtx->CurrData.HandleData.Drc_v30.Mode);
    LOGD_ATMO("%s: Current LocalWeit:%f LocalAutoEnable:%d LocalAutoWeit:%f GlobalContrast:%f LoLitContrast:%f\n", __FUNCTION__,
              pAdrcCtx->CurrData.HandleData.Drc_v30.LocalWeit, pAdrcCtx->CurrData.HandleData.Drc_v30.LocalAutoEnable, pAdrcCtx->CurrData.HandleData.Drc_v30.LocalAutoWeit,
              pAdrcCtx->CurrData.HandleData.Drc_v30.GlobalContrast, pAdrcCtx->CurrData.HandleData.Drc_v30.LoLitContrast);

    //transfer data to api
    //info
    pAdrcCtx->drcAttr.Info.EnvLv = pAdrcCtx->CurrData.EnvLv;

    //damp
    AdrcDampingV30(&pAdrcCtx->CurrData, &pAdrcCtx->PrevData, pAdrcCtx->drcAttr.opMode, pAdrcCtx->frameCnt);

    //get io data
    AdrcGetTuningProcResV30(&pAdrcCtx->AdrcProcRes, &pAdrcCtx->CurrData, pAdrcCtx->SensorInfo.LongFrmMode,
                            pAdrcCtx->FrameNumber, pAdrcCtx->frameCnt);

    // store current handle data to pre data for next loop
    pAdrcCtx->PrevData.EnvLv = pAdrcCtx->CurrData.EnvLv;
    pAdrcCtx->PrevData.ISO = pAdrcCtx->CurrData.ISO;
    pAdrcCtx->PrevData.MotionCoef = pAdrcCtx->CurrData.MotionCoef;
    if(0 != memcmp(&pAdrcCtx->PrevData.HandleData, &pAdrcCtx->CurrData.HandleData, sizeof(DrcHandleData_t)))
        memcpy(&pAdrcCtx->PrevData.HandleData, &pAdrcCtx->CurrData.HandleData, sizeof(DrcHandleData_t));

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcTuningParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcTuningParaProcessing
(
    AdrcContext_t*     pAdrcCtx
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    pAdrcCtx->CurrData.frameCnt = pAdrcCtx->frameCnt;

    if(CHECK_ISP_HW_V21())
        AdrcTuningParaProcessingV21(pAdrcCtx);
    else if(CHECK_ISP_HW_V30())
        AdrcTuningParaProcessingV30(pAdrcCtx);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcExpoParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcExpoParaProcessing
(
    AdrcContext_t*     pAdrcCtx,
    DrcExpoData_t*     pExpoData
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    if(CHECK_ISP_HW_V21()) {
        //get sw_drc_compres_scl
        float adrc_gain = pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_adrc_gain;
        float log_ratio2 = log(pExpoData->nextRatioLS * adrc_gain) / log(2.0f) + 12;
        float offsetbits_int = (float)(pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_offset_pow2);
        float offsetbits = offsetbits_int * pow(2, MFHDR_LOG_Q_BITS);
        float hdrbits = log_ratio2 * pow(2, MFHDR_LOG_Q_BITS);
        float hdrvalidbits = hdrbits - offsetbits;
        float compres_scl = (12 * pow(2, MFHDR_LOG_Q_BITS * 2)) / hdrvalidbits;
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_scl = (int)(compres_scl);

        //get sw_drc_min_ogain
        if(pAdrcCtx->Config.Drc_v21.OutPutLongFrame)
            pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_min_ogain = 1 << 15;
        else {
            float sw_drc_min_ogain = 1 / (pExpoData->nextRatioLS * adrc_gain);
            pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_min_ogain = (int)(sw_drc_min_ogain * pow(2, 15) + 0.5);
        }

        //get sw_drc_compres_y
        if(pAdrcCtx->AdrcProcRes.CompressMode == COMPRESS_AUTO) {
            float curveparam, curveparam2, curveparam3, tmp;
            float luma2[17] = { 0, 1024, 2048, 3072, 4096, 5120, 6144, 7168, 8192, 10240, 12288, 14336, 16384, 18432, 20480, 22528, 24576 };
            float curveTable[17];
            float dstbits = ISP_RAW_BIT * pow(2, MFHDR_LOG_Q_BITS);
            float validbits = dstbits - offsetbits;
            for(int i = 0; i < ISP21_DRC_Y_NUM; ++i)
            {
                curveparam = (float)(validbits - 0) / (hdrvalidbits - validbits + pow(2, -6));
                curveparam2 = validbits * (1 + curveparam);
                curveparam3 = hdrvalidbits * curveparam;
                tmp = luma2[i] * hdrvalidbits / 24576;
                curveTable[i] = (tmp * curveparam2 / (tmp + curveparam3));
                pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[i] = (int)(curveTable[i]) ;
            }
        }

        LOGV_ATMO("%s: nextRatioLS:%f sw_drc_position:%d sw_drc_compres_scl:%d sw_drc_offset_pow2:%d\n", __FUNCTION__,
                  pExpoData->nextRatioLS, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_position, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_scl,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_offset_pow2);
        LOGV_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_lpdetail_ratio,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_hpdetail_ratio, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_delta_scalein);
        LOGV_ATMO("%s: sw_drc_weipre_frame:%d sw_drc_weicur_pix:%d\n", __FUNCTION__,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_weipre_frame, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_weicur_pix);
        LOGV_ATMO("%s: sw_drc_edge_scl:%d sw_drc_motion_scl:%d sw_drc_force_sgm_inv0:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_edge_scl,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_motion_scl, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_force_sgm_inv0);
        LOGV_ATMO("%s: sw_drc_space_sgm_inv0:%d sw_drc_space_sgm_inv1:%d\n", __FUNCTION__,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_space_sgm_inv0, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_space_sgm_inv1);
        LOGV_ATMO("%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_range_sgm_inv0,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_range_sgm_inv1);
        LOGV_ATMO("%s: sw_drc_weig_bilat:%d sw_drc_weig_maxl:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_weig_bilat,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_weig_maxl);
        LOGV_ATMO("%s: sw_drc_min_ogain:%d sw_drc_iir_weight:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_min_ogain,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_iir_weight);
        LOGV_ATMO("%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[0], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[1],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[2], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[3],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[4], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[5],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[6], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[7],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[8], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[9],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[10], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[11],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[12], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[13],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[14], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[15],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_gain_y[16]);
        LOGV_ATMO("%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[0], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[1],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[2], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[3],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[4], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[5],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[6], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[7],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[8], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[9],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[10], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[11],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[12], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[13],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[14], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[15],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_scale_y[16]);
        LOGV_ATMO( "%s: CompressMode:%d sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.CompressMode,
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[0], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[1],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[2], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[3],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[4], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[5],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[6], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[7],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[8], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[9],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[10], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[11],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[12], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[13],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[14], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[15],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v21.sw_drc_compres_y[16]);
    }
    else if(CHECK_ISP_HW_V30()) {
        //get sw_drc_compres_scl
        float adrc_gain = pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.adrc_gain;
        float log_ratio2 = log(pExpoData->nextRatioLS * adrc_gain) / log(2.0f) + 12;
        float offsetbits_int = (float)(pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.offset_pow2);
        float offsetbits = offsetbits_int * pow(2, MFHDR_LOG_Q_BITS);
        float hdrbits = log_ratio2 * pow(2, MFHDR_LOG_Q_BITS);
        float hdrvalidbits = hdrbits - offsetbits;
        float compres_scl = (12 * pow(2, MFHDR_LOG_Q_BITS * 2)) / hdrvalidbits;
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_scl = (int)(compres_scl);

        //get sw_drc_min_ogain
        if(pAdrcCtx->Config.Drc_v30.OutPutLongFrame)
            pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.min_ogain = 1 << 15;
        else {
            float sw_drc_min_ogain = 1 / (pExpoData->nextRatioLS * adrc_gain);
            pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.min_ogain = (int)(sw_drc_min_ogain * pow(2, 15) + 0.5);
        }

        //get sw_drc_compres_y
        if(pAdrcCtx->AdrcProcRes.CompressMode == COMPRESS_AUTO) {
            float curveparam, curveparam2, curveparam3, tmp;
            float luma2[17] = { 0, 1024, 2048, 3072, 4096, 5120, 6144, 7168, 8192, 10240, 12288, 14336, 16384, 18432, 20480, 22528, 24576 };
            float curveTable[17];
            float dstbits = ISP_RAW_BIT * pow(2, MFHDR_LOG_Q_BITS);
            float validbits = dstbits - offsetbits;
            for(int i = 0; i < ISP21_DRC_Y_NUM; ++i)
            {
                curveparam = (float)(validbits - 0) / (hdrvalidbits - validbits + pow(2, -6));
                curveparam2 = validbits * (1 + curveparam);
                curveparam3 = hdrvalidbits * curveparam;
                tmp = luma2[i] * hdrvalidbits / 24576;
                curveTable[i] = (tmp * curveparam2 / (tmp + curveparam3));
                pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[i] = (int)(curveTable[i]) ;
            }
        }

        LOGV_ATMO("%s: nextRatioLS:%f sw_drc_position:%d sw_drc_compres_scl:%d sw_drc_offset_pow2:%d\n", __FUNCTION__, pExpoData->nextRatioLS,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.position, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_scl, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.offset_pow2);
        LOGV_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.lpdetail_ratio,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.hpdetail_ratio, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.delta_scalein);
        LOGV_ATMO("%s: sw_drc_bilat_wt_off:%d sw_drc_weipre_frame:%d sw_drc_weicur_pix:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.bilat_wt_off,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.weipre_frame, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.weicur_pix);
        LOGV_ATMO("%s: sw_drc_edge_scl:%d sw_drc_motion_scl:%d sw_drc_force_sgm_inv0:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.edge_scl,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.motion_scl, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.force_sgm_inv0);
        LOGV_ATMO("%s: sw_drc_space_sgm_inv0:%d sw_drc_space_sgm_inv1:%d\n", __FUNCTION__,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.space_sgm_inv0, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.space_sgm_inv1);
        LOGV_ATMO("%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.range_sgm_inv0,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.range_sgm_inv1);
        LOGV_ATMO("%s: sw_drc_weig_bilat:%d sw_drc_weig_maxl:%d sw_drc_bilat_soft_thd:%d sw_drc_enable_soft_thd:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.weig_bilat,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.weig_maxl, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.bilat_soft_thd, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.enable_soft_thd);
        LOGV_ATMO("%s: sw_drc_min_ogain:%d sw_drc_iir_weight:%d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.min_ogain,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.iir_weight);
        LOGV_ATMO("%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[0], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[1],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[2], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[3],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[4], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[5],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[6], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[7],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[8], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[9],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[10], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[11],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[12], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[13],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[14], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[15],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.gain_y[16]);
        LOGV_ATMO("%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[0], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[1],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[2], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[3],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[4], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[5],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[6], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[7],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[8], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[9],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[10], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[11],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[12], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[13],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[14], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[15],
                  pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.scale_y[16]);
        LOGV_ATMO( "%s: CompressMode:%d sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__, pAdrcCtx->AdrcProcRes.CompressMode,
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[0], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[1],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[2], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[3],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[4], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[5],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[6], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[7],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[8], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[9],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[10], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[11],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[12], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[13],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[14], pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[15],
                   pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v30.compres_y[16]);
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcByPassProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
bool AdrcByPassProcessing
(
    AdrcContext_t*     pAdrcCtx,
    AecPreResult_t  AecHdrPreResult
)
{
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    bool bypass = false;
    float diff = 0.0;
    float ByPassThr = 0.0f;

    // get current EnvLv from AecPreRes
    AdrcGetEnvLv(pAdrcCtx, AecHdrPreResult);

    // motion coef
    pAdrcCtx->CurrData.MotionCoef = MOVE_COEF_DEFAULT;

    // transfer ae data to CurrHandle
    pAdrcCtx->CurrData.EnvLv =
        LIMIT_VALUE(pAdrcCtx->CurrData.EnvLv, ADRCNORMALIZEMAX, ADRCNORMALIZEMIN);

    pAdrcCtx->CurrData.ISO = pAdrcCtx->CurrAeResult.ISO;
    pAdrcCtx->CurrData.ISO = LIMIT_VALUE(pAdrcCtx->CurrData.ISO, ISOMAX, ISOMIN);

    if (pAdrcCtx->frameCnt <= 2)  // start frame
        bypass = false;
    else if (pAdrcCtx->drcAttr.opMode > DRC_OPMODE_API_OFF)  // api
        bypass = false;
    else if (pAdrcCtx->drcAttr.opMode != pAdrcCtx->PrevData.ApiMode)  // api change
        bypass = false;
    else {  // EnvLv change
        if (CHECK_ISP_HW_V21())
            ByPassThr = pAdrcCtx->Config.Drc_v21.Others.ByPassThr;
        else if (CHECK_ISP_HW_V30())
            ByPassThr = pAdrcCtx->Config.Drc_v30.Others.ByPassThr;

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
            if(diff >= ByPassThr || diff <= (0 - ByPassThr))
                bypass = false;
            else
                bypass = true;
        }
    }

    LOGD_ATMO( "%s: FrameID:%d HDRFrameNum:%d LongFrmMode:%d DRCApiMode:%d EnvLv:%f bypass:%d\n", __FUNCTION__, pAdrcCtx->frameCnt, pAdrcCtx->FrameNumber,
               pAdrcCtx->SensorInfo.LongFrmMode, pAdrcCtx->drcAttr.opMode, pAdrcCtx->CurrData.EnvLv, bypass);

    LOG1_ATMO( "%s: CtrlEnvLv:%f PrevEnvLv:%f diff:%f ByPassThr:%f opMode:%d bypass:%d!\n", __FUNCTION__, pAdrcCtx->CurrData.EnvLv,
               pAdrcCtx->PrevData.EnvLv, diff, ByPassThr, pAdrcCtx->drcAttr.opMode, bypass);

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);
    return bypass;
}

/******************************************************************************
 * AdrcInit()
 *****************************************************************************/
XCamReturn AdrcInit
(
    AdrcContext_t **ppAdrcCtx,
    CamCalibDbV2Context_t *pCalibDb
) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    AdrcContext_t * pAdrcCtx;
    pAdrcCtx = (AdrcContext_t *)malloc(sizeof(AdrcContext_t));
    if(pAdrcCtx == NULL) {
        LOGE_ATMO("%s(%d): invalid inputparams\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_MEM;
    }

    memset(pAdrcCtx, 0x00, sizeof(AdrcContext_t));
    *ppAdrcCtx = pAdrcCtx;
    pAdrcCtx->state = ADRC_STATE_INITIALIZED;

    if(CHECK_ISP_HW_V21()) {
        CalibDbV2_drc_t* calibv2_adrc_calib =
            (CalibDbV2_drc_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adrc_calib));

        // pre-initialize context
        memset(pAdrcCtx, 0x00, sizeof(*pAdrcCtx));

        //malloc DrcGain
        pAdrcCtx->Config.Drc_v21.DrcGain.len = calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len;
        pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
        pAdrcCtx->Config.Drc_v21.DrcGain.DrcGain = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
        pAdrcCtx->Config.Drc_v21.DrcGain.Alpha = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
        pAdrcCtx->Config.Drc_v21.DrcGain.Clip = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));

        //malloc HiLit
        pAdrcCtx->Config.Drc_v21.HiLit.len = calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len;
        pAdrcCtx->Config.Drc_v21.HiLit.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len));
        pAdrcCtx->Config.Drc_v21.HiLit.Strength = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len));

        //malloc local
        pAdrcCtx->Config.Drc_v21.Local.len = calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
        pAdrcCtx->Config.Drc_v21.Local.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
        pAdrcCtx->Config.Drc_v21.Local.LocalWeit = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
        pAdrcCtx->Config.Drc_v21.Local.GlobalContrast = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));
        pAdrcCtx->Config.Drc_v21.Local.LoLitContrast = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len));

        pAdrcCtx->drcAttr.Version = ADRC_VERSION_356X;
        pAdrcCtx->drcAttr.opMode = DRC_OPMODE_API_OFF;
        ConfigV21(pAdrcCtx); //set default para
        memcpy(&pAdrcCtx->pCalibDB.Drc_v21, calibv2_adrc_calib, sizeof(CalibDbV2_drc_t));//load iq paras

    }
    else if(CHECK_ISP_HW_V30()) {
        CalibDbV2_drc_V2_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V2_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adrc_calib));

        // pre-initialize context
        memset(pAdrcCtx, 0x00, sizeof(*pAdrcCtx));

        //malloc DrcGain
        pAdrcCtx->Config.Drc_v30.DrcGain.len = calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len;
        pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.DrcGain.DrcGain = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.DrcGain.Alpha = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.DrcGain.Clip = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.DrcGain.EnvLv_len));

        //malloc HiLit
        pAdrcCtx->Config.Drc_v30.HiLit.len = calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len;
        pAdrcCtx->Config.Drc_v30.HiLit.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.HiLit.Strength = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.HiLight.EnvLv_len));

        //malloc local
        pAdrcCtx->Config.Drc_v30.Local.len = calibv2_adrc_calib->DrcTuningPara.LocalSetting.LocalData.EnvLv_len;
        pAdrcCtx->Config.Drc_v30.Local.EnvLv = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalSetting.LocalData.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.Local.LocalAutoEnable = (int*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalSetting.LocalData.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.Local.LocalAutoWeit = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalSetting.LocalData.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.Local.LocalWeit = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalSetting.LocalData.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.Local.GlobalContrast = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalSetting.LocalData.EnvLv_len));
        pAdrcCtx->Config.Drc_v30.Local.LoLitContrast = (float*)malloc(sizeof(float) * (calibv2_adrc_calib->DrcTuningPara.LocalSetting.LocalData.EnvLv_len));

        pAdrcCtx->drcAttr.Version = ADRC_VERSION_3588;
        pAdrcCtx->drcAttr.opMode = DRC_OPMODE_API_OFF;
        ConfigV30(pAdrcCtx); //set default para
        memcpy(&pAdrcCtx->pCalibDB.Drc_v30, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V2_t));//load iq paras
    }

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
/******************************************************************************
 * AhdrRelease()
 *****************************************************************************/
XCamReturn AdrcRelease
(
    AdrcContext_t* pAdrcCtx
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

    if(CHECK_ISP_HW_V21()) {
        free(pAdrcCtx->Config.Drc_v21.DrcGain.EnvLv);
        free(pAdrcCtx->Config.Drc_v21.DrcGain.DrcGain);
        free(pAdrcCtx->Config.Drc_v21.DrcGain.Alpha);
        free(pAdrcCtx->Config.Drc_v21.DrcGain.Clip);
        free(pAdrcCtx->Config.Drc_v21.HiLit.EnvLv);
        free(pAdrcCtx->Config.Drc_v21.HiLit.Strength);
        free(pAdrcCtx->Config.Drc_v21.Local.EnvLv);
        free(pAdrcCtx->Config.Drc_v21.Local.LocalWeit);
        free(pAdrcCtx->Config.Drc_v21.Local.GlobalContrast);
        free(pAdrcCtx->Config.Drc_v21.Local.LoLitContrast);
    }
    else if(CHECK_ISP_HW_V30()) {
        free(pAdrcCtx->Config.Drc_v30.DrcGain.EnvLv);
        free(pAdrcCtx->Config.Drc_v30.DrcGain.DrcGain);
        free(pAdrcCtx->Config.Drc_v30.DrcGain.Alpha);
        free(pAdrcCtx->Config.Drc_v30.DrcGain.Clip);
        free(pAdrcCtx->Config.Drc_v30.HiLit.EnvLv);
        free(pAdrcCtx->Config.Drc_v30.HiLit.Strength);
        free(pAdrcCtx->Config.Drc_v30.Local.EnvLv);
        free(pAdrcCtx->Config.Drc_v30.Local.LocalAutoEnable);
        free(pAdrcCtx->Config.Drc_v30.Local.LocalAutoWeit);
        free(pAdrcCtx->Config.Drc_v30.Local.LocalWeit);
        free(pAdrcCtx->Config.Drc_v30.Local.GlobalContrast);
        free(pAdrcCtx->Config.Drc_v30.Local.LoLitContrast);
    }
    memset(pAdrcCtx, 0, sizeof(AdrcContext_t));
    free(pAdrcCtx);
    pAdrcCtx = NULL;

    LOG1_ATMO( "%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
