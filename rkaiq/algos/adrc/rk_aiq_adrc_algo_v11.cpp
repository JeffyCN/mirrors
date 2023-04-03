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
 * @file rk_aiq_adrc_algo_v11.cpp
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include "math.h"
//#include "rk_aiq_types_adrc_algo_int.h"
#include "rk_aiq_types_adrc_algo_prvt.h"
#include "xcam_log.h"

/******************************************************************************
 * AdrcStart()
 *****************************************************************************/
XCamReturn AdrcStart(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAdrcCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    if ((ADRC_STATE_RUNNING == pAdrcCtx->state) || (ADRC_STATE_LOCKED == pAdrcCtx->state)) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    pAdrcCtx->state = ADRC_STATE_RUNNING;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
    return (XCAM_RETURN_NO_ERROR);
}

/******************************************************************************
 * DrcGetCurrParaV11()
 *****************************************************************************/
float DrcGetCurrParaV11(float inPara, float* inMatrixX, float* inMatrixY, int Max_Knots) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    float x1      = 0.0f;
    float x2      = 0.0f;
    float value1  = 0.0f;
    float value2  = 0.0f;
    float outPara = 0.0f;

    if (inPara < inMatrixX[0])
        outPara = inMatrixY[0];
    else if (inPara >= inMatrixX[Max_Knots - 1])
        outPara = inMatrixY[Max_Knots - 1];
    else
        for (int i = 0; i < Max_Knots - 1; i++) {
            if (inPara >= inMatrixX[i] && inPara < inMatrixX[i + 1]) {
                x1      = inMatrixX[i];
                x2      = inMatrixX[i + 1];
                value1  = inMatrixY[i];
                value2  = inMatrixY[i + 1];
                outPara = value1 + (inPara - x1) * (value1 - value2) / (x1 - x2);
                break;
            } else
                continue;
        }

    return outPara;
    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * DrcGetCurrParaV11()
 *****************************************************************************/
int DrcGetCurrParaV11Int(float inPara, float* inMatrixX, int* inMatrixY, int Max_Knots) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    float x1     = 0.0f;
    float x2     = 0.0f;
    float value1 = 0.0f;
    float value2 = 0.0f;
    int outPara  = 0;

    if (inPara < inMatrixX[0])
        outPara = inMatrixY[0];
    else if (inPara >= inMatrixX[Max_Knots - 1])
        outPara = inMatrixY[Max_Knots - 1];
    else
        for (int i = 0; i < Max_Knots - 1; i++) {
            if (inPara >= inMatrixX[i] && inPara < inMatrixX[i + 1]) {
                x1      = inMatrixX[i];
                x2      = inMatrixX[i + 1];
                value1  = inMatrixY[i];
                value2  = inMatrixY[i + 1];
                outPara = (int)(value1 + (inPara - x1) * (value1 - value2) / (x1 - x2));
                break;
            } else
                continue;
        }

    return outPara;
    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcStop()
 *****************************************************************************/
XCamReturn AdrcStop(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pAdrcCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    // before stopping, unlock the AHDR if locked
    if (ADRC_STATE_LOCKED == pAdrcCtx->state) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    pAdrcCtx->state = ADRC_STATE_STOPPED;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}

/******************************************************************************
 * CalibrateDrcGainYV11()
 *****************************************************************************/
void CalibrateDrcGainYV11(DrcProcRes_t* para, float DraGain, float alpha) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);

    float tmp      = 0;
    float luma[17] = {0,    256,  512,  768,  1024, 1280, 1536, 1792, 2048,
                      2304, 2560, 2816, 3072, 3328, 3584, 3840, 4096};
    float gainTable[17];

    for (int i = 0; i < 17; ++i) {
        tmp                     = luma[i];
        tmp                     = 1 - alpha * pow((1 - tmp / 4096), 2);
        gainTable[i]            = 1024 * pow(DraGain, tmp);
        para->Drc_v11.gain_y[i] = (int)(gainTable[i]);
    }

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * SetDefaultValueV11()
 *set default Config data
 *****************************************************************************/
void SetDefaultValueV11(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pAdrcCtx != NULL);

    // config default Prev Ae data
    pAdrcCtx->CurrData.AEData.LongFrmMode = false;
    pAdrcCtx->CurrData.AEData.L2S_Ratio   = RATIO_DEFAULT;
    pAdrcCtx->CurrData.AEData.L2M_Ratio   = RATIO_DEFAULT;
    pAdrcCtx->CurrData.AEData.M2S_Ratio   = RATIO_DEFAULT;

    // config default CurrData data
    pAdrcCtx->CurrData.FrameID                            = 0;
    pAdrcCtx->CurrData.AEData.EnvLv                       = 0;
    pAdrcCtx->CurrData.MotionCoef                         = 0;
    pAdrcCtx->CurrData.ApiMode                            = DRC_OPMODE_AUTO;
    pAdrcCtx->CurrData.FrameNumber                        = LINEAR_NUM;
    pAdrcCtx->CurrData.HandleData.Drc_v11.DrcGain         = 4.0;
    pAdrcCtx->CurrData.HandleData.Drc_v11.Alpha           = 0.2;
    pAdrcCtx->CurrData.HandleData.Drc_v11.Clip            = 16.0;
    pAdrcCtx->CurrData.HandleData.Drc_v11.Strength        = 0.01;
    pAdrcCtx->CurrData.HandleData.Drc_v11.LocalWeit       = 1.00;
    pAdrcCtx->CurrData.HandleData.Drc_v11.LocalAutoEnable = 1;
    pAdrcCtx->CurrData.HandleData.Drc_v11.LocalAutoWeit   = 0.037477;
    pAdrcCtx->CurrData.HandleData.Drc_v11.GlobalContrast  = 0;
    pAdrcCtx->CurrData.HandleData.Drc_v11.LoLitContrast   = 0;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

void AdrcGetEnvLvV11(AdrcContext_t* pAdrcCtx, AecPreResult_t AecHdrPreResult) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // transfer AeResult data into AhdrHandle
    switch (pAdrcCtx->FrameNumber) {
        case LINEAR_NUM:
            pAdrcCtx->NextData.AEData.EnvLv = AecHdrPreResult.GlobalEnvLv[0];
            break;
        case HDR_2X_NUM:
            pAdrcCtx->NextData.AEData.EnvLv = AecHdrPreResult.GlobalEnvLv[1];
            break;
        case HDR_3X_NUM:
            pAdrcCtx->NextData.AEData.EnvLv = AecHdrPreResult.GlobalEnvLv[1];
            break;
        default:
            LOGE_ATMO("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
            break;
    }

    // Normalize the current envLv for AEC
    pAdrcCtx->NextData.AEData.EnvLv =
        (pAdrcCtx->NextData.AEData.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
    pAdrcCtx->NextData.AEData.EnvLv =
        LIMIT_VALUE(pAdrcCtx->NextData.AEData.EnvLv, ENVLVMAX, ENVLVMIN);

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * DrcEnableSetting()
 *
 *****************************************************************************/
bool DrcEnableSetting(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    if (pAdrcCtx->FrameNumber == HDR_2X_NUM || pAdrcCtx->FrameNumber == HDR_3X_NUM)
        pAdrcCtx->NextData.Enable = true;
    else if (pAdrcCtx->FrameNumber == LINEAR_NUM) {
        if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_AUTO)
            pAdrcCtx->NextData.Enable = pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Enable;
        else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_MANUAL)
            pAdrcCtx->NextData.Enable = pAdrcCtx->drcAttrV11.stManual.Enable;
        else {
            LOGE_ATMO("%s: Drc api in WRONG MODE!!!, drc by pass!!!\n", __FUNCTION__);
            pAdrcCtx->NextData.Enable = false;
        }
    }

    return pAdrcCtx->NextData.Enable;
    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}
/******************************************************************************
 * AdrcDampingV11()
 *****************************************************************************/
void AdrcDampingV11(NextData_t* pNextData, CurrData_t* pCurrData, int FrameID,
                    CtrlDataType_t CtrlDataType) {
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);

    if (FrameID && pNextData->FrameNumber == pCurrData->FrameNumber) {
            pNextData->HandleData.Drc_v11.DrcGain =
                pNextData->Others.damp * pNextData->HandleData.Drc_v11.DrcGain +
                (1 - pNextData->Others.damp) * pCurrData->HandleData.Drc_v11.DrcGain;
            pNextData->HandleData.Drc_v11.Alpha =
                pNextData->Others.damp * pNextData->HandleData.Drc_v11.Alpha +
                (1 - pNextData->Others.damp) * pCurrData->HandleData.Drc_v11.Alpha;
            pNextData->HandleData.Drc_v11.Clip =
                pNextData->Others.damp * pNextData->HandleData.Drc_v11.Clip +
                (1 - pNextData->Others.damp) * pCurrData->HandleData.Drc_v11.Clip;
            pNextData->HandleData.Drc_v11.Strength =
                pNextData->Others.damp * pNextData->HandleData.Drc_v11.Strength +
                (1 - pNextData->Others.damp) * pCurrData->HandleData.Drc_v11.Strength;
            pNextData->HandleData.Drc_v11.LocalWeit =
                pNextData->Others.damp * pNextData->HandleData.Drc_v11.LocalWeit +
                (1 - pNextData->Others.damp) * pCurrData->HandleData.Drc_v11.LocalWeit;
            pNextData->HandleData.Drc_v11.LocalAutoWeit =
                pNextData->Others.damp * pNextData->HandleData.Drc_v11.LocalAutoWeit +
                (1 - pNextData->Others.damp) * pCurrData->HandleData.Drc_v11.LocalAutoWeit;
            pNextData->HandleData.Drc_v11.GlobalContrast =
                pNextData->Others.damp * pNextData->HandleData.Drc_v11.GlobalContrast +
                (1 - pNextData->Others.damp) * pCurrData->HandleData.Drc_v11.GlobalContrast;
            pNextData->HandleData.Drc_v11.LoLitContrast =
                pNextData->Others.damp * pNextData->HandleData.Drc_v11.LoLitContrast +
                (1 - pNextData->Others.damp) * pCurrData->HandleData.Drc_v11.LoLitContrast;
    }

    LOG1_ATMO("%s:Eixt!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcGetTuningProcResV11()
 *****************************************************************************/
void AdrcGetTuningProcResV11(RkAiqAdrcProcResult_t* pAdrcProcRes, NextData_t* pNextData,
                             int FrameNumber, int FrameID) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // DrcProcRes
    pAdrcProcRes->DrcProcRes.Drc_v11.bypass_en   = FUNCTION_DISABLE;
    pAdrcProcRes->DrcProcRes.Drc_v11.offset_pow2 = SW_DRC_OFFSET_POW2_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v11.position =
        (int)(SHIFT8BIT(pNextData->HandleData.Drc_v11.Clip) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.hpdetail_ratio =
        (int)(SHIFT12BIT(pNextData->HandleData.Drc_v11.LoLitContrast) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.lpdetail_ratio =
        (int)(SHIFT12BIT(pNextData->HandleData.Drc_v11.GlobalContrast) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.weicur_pix =
        (int)(SHIFT8BIT(pNextData->Others.curPixWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.weipre_frame =
        (int)(SHIFT8BIT(pNextData->Others.preFrameWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.weipre_frame =
        LIMIT_VALUE(pAdrcProcRes->DrcProcRes.Drc_v11.weipre_frame, INT8BITMAX, 0);
    pAdrcProcRes->DrcProcRes.Drc_v11.bilat_wt_off =
        SW_DRC_BILAT_WT_OFF_FIX;  // LIMIT_VALUE(pNextData->HandleData.Drc_v11.MotionStr *
                                  // INT8BITMAX, INT8BITMAX, 0);
    pAdrcProcRes->DrcProcRes.Drc_v11.force_sgm_inv0 =
        (int)(SHIFT13BIT(pNextData->Others.Range_force_sgm) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.edge_scl = (int)(SHIFT8BIT(pNextData->Others.Edge_Weit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.motion_scl     = SW_DRC_MOTION_SCL_FIX;
    pAdrcProcRes->DrcProcRes.Drc_v11.space_sgm_inv1 = (int)(pNextData->Others.Space_sgm_cur);
    pAdrcProcRes->DrcProcRes.Drc_v11.space_sgm_inv0 = (int)(pNextData->Others.Space_sgm_pre);
    pAdrcProcRes->DrcProcRes.Drc_v11.range_sgm_inv1 =
        (int)(SHIFT13BIT(pNextData->Others.Range_sgm_cur) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.range_sgm_inv0 =
        (int)(SHIFT13BIT(pNextData->Others.Range_sgm_pre) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.weig_maxl =
        (int)(SHIFT4BIT(pNextData->HandleData.Drc_v11.Strength) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.adrc_gain = pNextData->HandleData.Drc_v11.DrcGain;
    pAdrcProcRes->DrcProcRes.Drc_v11.weig_bilat =
        (int)(SHIFT4BIT(pNextData->HandleData.Drc_v11.LocalWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.enable_soft_thd =
        pNextData->HandleData.Drc_v11.LocalAutoEnable;
    pAdrcProcRes->DrcProcRes.Drc_v11.bilat_soft_thd =
        (int)(SHIFT14BIT(pNextData->HandleData.Drc_v11.LocalAutoWeit) + 0.5);
    pAdrcProcRes->DrcProcRes.Drc_v11.bilat_soft_thd =
        LIMIT_VALUE(pAdrcProcRes->DrcProcRes.Drc_v11.bilat_soft_thd, INT14BITMAX, 0);
    for (int i = 0; i < ADRC_Y_NUM; ++i) {
        pAdrcProcRes->DrcProcRes.Drc_v11.scale_y[i] = (int)(pNextData->Others.Scale_y[i]);
    }

    // get sw_drc_gain_y
    CalibrateDrcGainYV11(&pAdrcProcRes->DrcProcRes, pAdrcProcRes->DrcProcRes.Drc_v11.adrc_gain,
                         pNextData->HandleData.Drc_v11.Alpha);

    pAdrcProcRes->DrcProcRes.Drc_v11.wr_cycle = FUNCTION_DISABLE;
    float iir_frame = (float)(MIN(FrameID + 1, pNextData->Others.IIR_frame));
    pAdrcProcRes->DrcProcRes.Drc_v11.iir_weight =
        (int)(SHIFT6BIT((iir_frame - 1) / iir_frame) + 0.5);

    // sw_drc_delta_scalein FIX
    pAdrcProcRes->DrcProcRes.Drc_v11.delta_scalein = DELTA_SCALEIN_FIX;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

void AdrcParams2ApiV11(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // ctrl info
    pAdrcCtx->drcAttrV11.Info.CtrlInfo.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pAdrcCtx->drcAttrV11.Info.CtrlInfo.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;

    // paras
    if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_MANUAL)
        memcpy(&pAdrcCtx->drcAttrV11.Info.ValidParams, &pAdrcCtx->drcAttrV11.stManual,
               sizeof(mdrcAttr_V11_t));
    else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_AUTO) {
        pAdrcCtx->drcAttrV11.Info.ValidParams.Enable = pAdrcCtx->NextData.Enable;
        pAdrcCtx->drcAttrV11.Info.ValidParams.DrcGain.Alpha =
            pAdrcCtx->NextData.HandleData.Drc_v11.Alpha;
        pAdrcCtx->drcAttrV11.Info.ValidParams.DrcGain.DrcGain =
            pAdrcCtx->NextData.HandleData.Drc_v11.DrcGain;
        pAdrcCtx->drcAttrV11.Info.ValidParams.DrcGain.Clip =
            pAdrcCtx->NextData.HandleData.Drc_v11.Clip;
        pAdrcCtx->drcAttrV11.Info.ValidParams.HiLight.Strength =
            pAdrcCtx->NextData.HandleData.Drc_v11.Strength;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.LocalData.LocalWeit =
            pAdrcCtx->NextData.HandleData.Drc_v11.LocalWeit;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.LocalData.LocalAutoEnable =
            pAdrcCtx->NextData.HandleData.Drc_v11.LocalAutoEnable;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.LocalData.LocalAutoWeit =
            pAdrcCtx->NextData.HandleData.Drc_v11.LocalAutoWeit;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.LocalData.GlobalContrast =
            pAdrcCtx->NextData.HandleData.Drc_v11.GlobalContrast;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.LocalData.LoLitContrast =
            pAdrcCtx->NextData.HandleData.Drc_v11.LoLitContrast;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.curPixWeit =
            pAdrcCtx->NextData.Others.curPixWeit;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.preFrameWeit =
            pAdrcCtx->NextData.Others.preFrameWeit;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.Range_force_sgm =
            pAdrcCtx->NextData.Others.Range_force_sgm;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.Range_sgm_cur =
            pAdrcCtx->NextData.Others.Range_sgm_cur;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.Range_sgm_pre =
            pAdrcCtx->NextData.Others.Range_sgm_pre;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.Space_sgm_cur =
            pAdrcCtx->NextData.Others.Space_sgm_cur;
        pAdrcCtx->drcAttrV11.Info.ValidParams.LocalSetting.Space_sgm_pre =
            pAdrcCtx->NextData.Others.Space_sgm_pre;
        pAdrcCtx->drcAttrV11.Info.ValidParams.Edge_Weit = pAdrcCtx->NextData.Others.Edge_Weit;
        pAdrcCtx->drcAttrV11.Info.ValidParams.OutPutLongFrame =
            pAdrcCtx->NextData.Others.OutPutLongFrame;
        pAdrcCtx->drcAttrV11.Info.ValidParams.IIR_frame = pAdrcCtx->NextData.Others.IIR_frame;
        pAdrcCtx->drcAttrV11.Info.ValidParams.CompressSetting.Mode =
            pAdrcCtx->NextData.HandleData.Drc_v11.Mode;
        for (int i = 0; i < ADRC_Y_NUM; i++) {
            pAdrcCtx->drcAttrV11.Info.ValidParams.CompressSetting.Manual_curve[i] =
                pAdrcCtx->NextData.HandleData.Drc_v11.Manual_curve[i];
            pAdrcCtx->drcAttrV11.Info.ValidParams.Scale_y[i] = pAdrcCtx->NextData.Others.Scale_y[i];
        }
    }

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcTuningParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcTuningParaProcessing(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    pAdrcCtx->NextData.FrameID = pAdrcCtx->FrameID;

    // para setting
    if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_AUTO) {
        float CtrlValue = pAdrcCtx->NextData.AEData.EnvLv;
        if (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAdrcCtx->NextData.AEData.ISO;

        // get Drc gain
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.DrcGain[i] =
                LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.DrcGain[i],
                            DRCGAINMAX, DRCGAINMIN);
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Alpha[i] =
                LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Alpha[i],
                            NORMALIZE_MAX, NORMALIZE_MIN);
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Clip[i] = LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Clip[i], CLIPMAX, CLIPMIN);
        }
        pAdrcCtx->NextData.HandleData.Drc_v11.DrcGain = DrcGetCurrParaV11(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.DrcGain, ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.HandleData.Drc_v11.Alpha = DrcGetCurrParaV11(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Alpha, ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.HandleData.Drc_v11.Clip = DrcGetCurrParaV11(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.DrcGain.Clip, ADRC_ENVLV_STEP_MAX);

        // get hi lit
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.Strength[i] =
                LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.Strength[i],
                            NORMALIZE_MAX, NORMALIZE_MIN);
        }
        pAdrcCtx->NextData.HandleData.Drc_v11.Strength = DrcGetCurrParaV11(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.HiLight.Strength, ADRC_ENVLV_STEP_MAX);

        // get local
        for (int i = 0; i < ADRC_ENVLV_STEP_MAX; i++) {
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i] =
                LIMIT_VALUE(
                    pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit[i],
                    NORMALIZE_MAX, NORMALIZE_MIN);
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                .GlobalContrast[i] = LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                .LoLitContrast[i] = LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                .LocalAutoEnable[i] = LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData
                .LocalAutoWeit[i] = LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit[i],
                NORMALIZE_MAX, NORMALIZE_MIN);
        }
        pAdrcCtx->NextData.HandleData.Drc_v11.LocalWeit = DrcGetCurrParaV11(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalWeit,
            ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.HandleData.Drc_v11.GlobalContrast = DrcGetCurrParaV11(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.GlobalContrast,
            ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.HandleData.Drc_v11.LoLitContrast = DrcGetCurrParaV11(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LoLitContrast,
            ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.HandleData.Drc_v11.LocalAutoEnable = DrcGetCurrParaV11Int(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoEnable,
            ADRC_ENVLV_STEP_MAX);
        pAdrcCtx->NextData.HandleData.Drc_v11.LocalAutoWeit = DrcGetCurrParaV11(
            CtrlValue, pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.CtrlData,
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.LocalData.LocalAutoWeit,
            ADRC_ENVLV_STEP_MAX);

        // compress
        pAdrcCtx->NextData.HandleData.Drc_v11.Mode =
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CompressSetting.Mode;
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.HandleData.Drc_v11.Manual_curve[i] = LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CompressSetting.Manual_curve[i],
                MANUALCURVEMAX, MANUALCURVEMIN);

        // update others
        pAdrcCtx->NextData.Others.OutPutLongFrame =
            pAdrcCtx->NextData.AEData.LongFrmMode ||
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.OutPutLongFrame;
        pAdrcCtx->NextData.Others.curPixWeit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.curPixWeit,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.preFrameWeit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.preFrameWeit,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Range_force_sgm =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_force_sgm,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Range_sgm_cur =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_sgm_cur,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Range_sgm_pre =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Range_sgm_pre,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Space_sgm_cur =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Space_sgm_cur,
                        SPACESGMMAX, SPACESGMMIN);
        pAdrcCtx->NextData.Others.Space_sgm_pre =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.LocalSetting.Space_sgm_pre,
                        SPACESGMMAX, SPACESGMMIN);
        // scale y
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.Others.Scale_y[i] = LIMIT_VALUE(
                pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Scale_y[i], SCALEYMAX, SCALEYMIN);
        pAdrcCtx->NextData.Others.ByPassThr = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Edge_Weit = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.Edge_Weit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.IIR_frame = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
        pAdrcCtx->NextData.Others.damp = LIMIT_VALUE(pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.damp,
                                                     NORMALIZE_MAX, NORMALIZE_MIN);

        // damp
        if (0 != memcmp(&pAdrcCtx->CurrData.HandleData, &pAdrcCtx->NextData.HandleData,
                        sizeof(DrcHandleData_t))) {
            AdrcDampingV11(&pAdrcCtx->NextData, &pAdrcCtx->CurrData, pAdrcCtx->FrameID,
                           pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType);
        }
    } else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_MANUAL) {
        // update drc gain
        pAdrcCtx->NextData.HandleData.Drc_v11.DrcGain =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.DrcGain.DrcGain, DRCGAINMAX, DRCGAINMIN);
        pAdrcCtx->NextData.HandleData.Drc_v11.Alpha =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.DrcGain.Alpha, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.HandleData.Drc_v11.Clip =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.DrcGain.Clip, CLIPMAX, CLIPMIN);

        // update hight light
        pAdrcCtx->NextData.HandleData.Drc_v11.Strength = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.HiLight.Strength, NORMALIZE_MAX, NORMALIZE_MIN);

        // update local
        pAdrcCtx->NextData.HandleData.Drc_v11.LocalWeit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.LocalWeit,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.HandleData.Drc_v11.LocalAutoEnable =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.LocalAutoEnable,
                        ADRCNORMALIZEINTMAX, ADRCNORMALIZEINTMIN);
        pAdrcCtx->NextData.HandleData.Drc_v11.LocalAutoWeit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.LocalAutoWeit,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.HandleData.Drc_v11.GlobalContrast =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.GlobalContrast,
                        NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.HandleData.Drc_v11.LoLitContrast =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.LocalData.LoLitContrast,
                        NORMALIZE_MAX, NORMALIZE_MIN);

        // compress
        pAdrcCtx->NextData.HandleData.Drc_v11.Mode =
            pAdrcCtx->drcAttrV11.stManual.CompressSetting.Mode;
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.HandleData.Drc_v11.Manual_curve[i] =
                LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.CompressSetting.Manual_curve[i],
                            MANUALCURVEMAX, MANUALCURVEMIN);

        // others
        pAdrcCtx->NextData.Others.OutPutLongFrame =
            pAdrcCtx->NextData.AEData.LongFrmMode || pAdrcCtx->drcAttrV11.stManual.OutPutLongFrame;
        pAdrcCtx->NextData.Others.curPixWeit      = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.curPixWeit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.preFrameWeit = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.preFrameWeit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Range_force_sgm =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.LocalSetting.Range_force_sgm, NORMALIZE_MAX,
                        NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Range_sgm_cur = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.Range_sgm_cur, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Range_sgm_pre = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.Range_sgm_pre, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.Space_sgm_cur = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.Space_sgm_cur, SPACESGMMAX, SPACESGMMIN);
        pAdrcCtx->NextData.Others.Space_sgm_pre = LIMIT_VALUE(
            pAdrcCtx->drcAttrV11.stManual.LocalSetting.Space_sgm_pre, SPACESGMMAX, SPACESGMMIN);
        for (int i = 0; i < ADRC_Y_NUM; i++)
            pAdrcCtx->NextData.Others.Scale_y[i] =
                LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.Scale_y[i], SCALEYMAX, SCALEYMIN);
        pAdrcCtx->NextData.Others.Edge_Weit =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.Edge_Weit, NORMALIZE_MAX, NORMALIZE_MIN);
        pAdrcCtx->NextData.Others.IIR_frame =
            LIMIT_VALUE(pAdrcCtx->drcAttrV11.stManual.IIR_frame, IIRFRAMEMAX, IIRFRAMEMIN);
    }

    // clip drc gain
    if (pAdrcCtx->NextData.AEData.L2S_Ratio * pAdrcCtx->NextData.HandleData.Drc_v11.DrcGain >
        MAX_AE_DRC_GAIN) {
        LOGE_ATMO("%s:  AERatio*DrcGain > 256!!!\n", __FUNCTION__);
        pAdrcCtx->NextData.HandleData.Drc_v11.DrcGain =
            MAX(MAX_AE_DRC_GAIN / pAdrcCtx->NextData.AEData.L2S_Ratio, GAINMIN);
    }

    LOGD_ATMO(
        "%s: Current Enable:%d DrcGain:%f Alpha:%f Clip:%f Strength:%f CompressMode:%d\n",
        __FUNCTION__, pAdrcCtx->NextData.Enable, pAdrcCtx->NextData.HandleData.Drc_v11.DrcGain,
        pAdrcCtx->NextData.HandleData.Drc_v11.Alpha, pAdrcCtx->NextData.HandleData.Drc_v11.Clip,
        pAdrcCtx->NextData.HandleData.Drc_v11.Strength, pAdrcCtx->NextData.HandleData.Drc_v11.Mode);
    LOGD_ATMO(
        "%s: Current LocalWeit:%f LocalAutoEnable:%d LocalAutoWeit:%f GlobalContrast:%f "
        "LoLitContrast:%f\n",
        __FUNCTION__, pAdrcCtx->NextData.HandleData.Drc_v11.LocalWeit,
        pAdrcCtx->NextData.HandleData.Drc_v11.LocalAutoEnable,
        pAdrcCtx->NextData.HandleData.Drc_v11.LocalAutoWeit,
        pAdrcCtx->NextData.HandleData.Drc_v11.GlobalContrast,
        pAdrcCtx->NextData.HandleData.Drc_v11.LoLitContrast);

    // get io data
    AdrcGetTuningProcResV11(&pAdrcCtx->AdrcProcRes, &pAdrcCtx->NextData, pAdrcCtx->FrameNumber,
                            pAdrcCtx->FrameID);

    // store curr data 2 api
    AdrcParams2ApiV11(pAdrcCtx);

    // store current handle data to pre data for next loop
    pAdrcCtx->CurrData.AEData.EnvLv = pAdrcCtx->NextData.AEData.EnvLv;
    pAdrcCtx->CurrData.AEData.ISO   = pAdrcCtx->NextData.AEData.ISO;
    pAdrcCtx->CurrData.MotionCoef   = pAdrcCtx->NextData.MotionCoef;
    pAdrcCtx->CurrData.ApiMode      = pAdrcCtx->drcAttrV11.opMode;
    pAdrcCtx->CurrData.FrameNumber  = pAdrcCtx->FrameNumber;
    if (0 != memcmp(&pAdrcCtx->CurrData.HandleData, &pAdrcCtx->NextData.HandleData,
                    sizeof(DrcHandleData_t))) {
        memcpy(&pAdrcCtx->CurrData.HandleData, &pAdrcCtx->NextData.HandleData,
               sizeof(DrcHandleData_t));
        pAdrcCtx->isDampStable = false;
    } else
        pAdrcCtx->isDampStable = true;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcExpoParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void AdrcExpoParaProcessing(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    // get sw_drc_compres_scl
    float adrc_gain      = pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.adrc_gain;
    float log_ratio2     = log(pAdrcCtx->NextData.AEData.L2S_Ratio * adrc_gain) / log(2.0f) + 12.0f;
    float offsetbits_int = (float)(pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.offset_pow2);
    float offsetbits     = offsetbits_int * (1 << MFHDR_LOG_Q_BITS);
    float hdrbits        = log_ratio2 * (1 << MFHDR_LOG_Q_BITS);
    float hdrvalidbits   = hdrbits - offsetbits;
    float compres_scl    = (12.0f * (1 << (MFHDR_LOG_Q_BITS * 2))) / hdrvalidbits;
    pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_scl = (int)(compres_scl);

    // get sw_drc_min_ogain
    if (pAdrcCtx->NextData.Others.OutPutLongFrame)
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.min_ogain = 1 << 15;
    else {
        float sw_drc_min_ogain = 1.0f / (pAdrcCtx->NextData.AEData.L2S_Ratio * adrc_gain);
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.min_ogain =
            (int)(sw_drc_min_ogain * (1 << 15) + 0.5f);
    }

    // get sw_drc_compres_y
    if (pAdrcCtx->NextData.HandleData.Drc_v11.Mode == COMPRESS_AUTO) {
        float curveparam, curveparam2, curveparam3, tmp;
        float luma2[17] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                           6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                           16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};
        float curveTable[17];
        float dstbits   = ISP_RAW_BIT * (1 << MFHDR_LOG_Q_BITS);
        float validbits = dstbits - offsetbits;
        for (int i = 0; i < ISP21_DRC_Y_NUM; ++i) {
            curveparam  = (float)(validbits - 0.0f) / (hdrvalidbits - validbits + pow(2.0f, -6.0f));
            curveparam2 = validbits * (1.0f + curveparam);
            curveparam3   = hdrvalidbits * curveparam;
            tmp           = luma2[i] * hdrvalidbits / 24576.0f;
            curveTable[i] = (tmp * curveparam2 / (tmp + curveparam3));
            pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[i] = (int)(curveTable[i]);
        }
    } else if (pAdrcCtx->NextData.HandleData.Drc_v11.Mode == COMPRESS_MANUAL) {
        for (int i = 0; i < ADRC_Y_NUM; ++i) {
            pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[i] =
                pAdrcCtx->NextData.HandleData.Drc_v11.Manual_curve[i];
        }
    }

    // store expo data
    pAdrcCtx->CurrData.AEData.L2M_Ratio = pAdrcCtx->NextData.AEData.L2M_Ratio;
    pAdrcCtx->CurrData.AEData.M2S_Ratio = pAdrcCtx->NextData.AEData.M2S_Ratio;
    pAdrcCtx->CurrData.AEData.L2S_Ratio = pAdrcCtx->NextData.AEData.L2S_Ratio;

    LOGV_ATMO("%s: nextRatioLS:%f sw_drc_position:%d sw_drc_compres_scl:%d sw_drc_offset_pow2:%d\n",
              __FUNCTION__, pAdrcCtx->NextData.AEData.L2S_Ratio,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.position,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_scl,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.offset_pow2);
    LOGV_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n",
              __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.lpdetail_ratio,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.hpdetail_ratio,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.delta_scalein);
    LOGV_ATMO("%s: sw_drc_bilat_wt_off:%d sw_drc_weipre_frame:%d sw_drc_weicur_pix:%d\n",
              __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.bilat_wt_off,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.weipre_frame,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.weicur_pix);
    LOGV_ATMO("%s: sw_drc_edge_scl:%d sw_drc_motion_scl:%d sw_drc_force_sgm_inv0:%d\n",
              __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.edge_scl,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.motion_scl,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.force_sgm_inv0);
    LOGV_ATMO("%s: sw_drc_space_sgm_inv0:%d sw_drc_space_sgm_inv1:%d\n", __FUNCTION__,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.space_sgm_inv0,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.space_sgm_inv1);
    LOGV_ATMO("%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d\n", __FUNCTION__,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.range_sgm_inv0,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.range_sgm_inv1);
    LOGV_ATMO(
        "%s: sw_drc_weig_bilat:%d sw_drc_weig_maxl:%d sw_drc_bilat_soft_thd:%d "
        "sw_drc_enable_soft_thd:%d\n",
        __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.weig_bilat,
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.weig_maxl,
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.bilat_soft_thd,
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.enable_soft_thd);
    LOGV_ATMO("%s: sw_drc_min_ogain:%d sw_drc_iir_weight:%d\n", __FUNCTION__,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.min_ogain,
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.iir_weight);
    LOGV_ATMO("%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[0],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[1],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[2],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[3],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[4],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[5],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[6],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[7],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[8],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[9],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[10],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[11],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[12],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[13],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[14],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[15],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.gain_y[16]);
    LOGV_ATMO("%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[0],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[1],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[2],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[3],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[4],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[5],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[6],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[7],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[8],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[9],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[10],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[11],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[12],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[13],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[14],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[15],
              pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.scale_y[16]);
    LOGV_ATMO(
        "%s: CompressMode:%d sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, pAdrcCtx->NextData.HandleData.Drc_v11.Mode,
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[0],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[1],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[2],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[3],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[4],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[5],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[6],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[7],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[8],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[9],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[10],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[11],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[12],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[13],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[14],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[15],
        pAdrcCtx->AdrcProcRes.DrcProcRes.Drc_v11.compres_y[16]);

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * AdrcByPassTuningProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
bool AdrcByPassTuningProcessing(AdrcContext_t* pAdrcCtx, AecPreResult_t AecHdrPreResult) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);

    bool bypass = false;
    float diff  = 0.0;

    // get current EnvLv from AecPreRes
    AdrcGetEnvLvV11(pAdrcCtx, AecHdrPreResult);

    // motion coef
    pAdrcCtx->NextData.MotionCoef = MOVE_COEF_DEFAULT;

    if (pAdrcCtx->FrameID <= 2)
        bypass = false;
    else if (pAdrcCtx->drcAttrV11.opMode != pAdrcCtx->CurrData.ApiMode)
        bypass = false;
    else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_MANUAL)
        bypass = !pAdrcCtx->ifReCalcStManual;
    else if (pAdrcCtx->drcAttrV11.opMode == DRC_OPMODE_AUTO) {
        if (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ENVLV) {
            diff = pAdrcCtx->CurrData.AEData.EnvLv - pAdrcCtx->NextData.AEData.EnvLv;
            if (pAdrcCtx->CurrData.AEData.EnvLv == 0.0) {
                diff = pAdrcCtx->NextData.AEData.EnvLv;
                if (diff == 0.0)
                    bypass = true;
                else
                    bypass = false;
            } else {
                diff /= pAdrcCtx->CurrData.AEData.EnvLv;
                if (diff >= pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr ||
                    diff <= (0 - pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr))
                    bypass = false;
                else
                    bypass = true;
            }
        } else if (pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType == CTRLDATATYPE_ISO) {
            diff = pAdrcCtx->CurrData.AEData.ISO - pAdrcCtx->NextData.AEData.ISO;
            diff /= pAdrcCtx->CurrData.AEData.ISO;
            if (diff >= pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr ||
                diff <= (0 - pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr))
                bypass = false;
            else
                bypass = true;
        }
        bypass = bypass && !pAdrcCtx->ifReCalcStAuto;
    }

    LOGD_ATMO(
        "%s: FrameID:%d HDRFrameNum:%d LongFrmMode:%d DRCApiMode:%d ifReCalcStAuto:%d "
        "ifReCalcStManual:%d CtrlDataType:%d EnvLv:%f "
        "ISO:%f bypass:%d\n",
        __FUNCTION__, pAdrcCtx->FrameID, pAdrcCtx->FrameNumber,
        pAdrcCtx->NextData.AEData.LongFrmMode, pAdrcCtx->drcAttrV11.opMode,
        pAdrcCtx->ifReCalcStAuto, pAdrcCtx->ifReCalcStManual,
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.CtrlDataType, pAdrcCtx->NextData.AEData.EnvLv,
        pAdrcCtx->NextData.AEData.ISO, bypass);

    LOG1_ATMO(
        "%s: NextEnvLv:%f CurrEnvLv:%f NextISO:%f CurrISO:%f diff:%f ByPassThr:%f opMode:%d "
        "bypass:%d!\n",
        __FUNCTION__, pAdrcCtx->NextData.AEData.EnvLv, pAdrcCtx->CurrData.AEData.EnvLv,
        pAdrcCtx->NextData.AEData.ISO, pAdrcCtx->CurrData.AEData.ISO, diff,
        pAdrcCtx->drcAttrV11.stAuto.DrcTuningPara.ByPassThr, pAdrcCtx->drcAttrV11.opMode, bypass);

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);
    return bypass;
}

/******************************************************************************
 * AdrcInit()
 *****************************************************************************/
XCamReturn AdrcInit(AdrcContext_t** ppAdrcCtx, CamCalibDbV2Context_t* pCalibDb) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx;
    pAdrcCtx = (AdrcContext_t*)malloc(sizeof(AdrcContext_t));
    if (pAdrcCtx == NULL) {
        LOGE_ATMO("%s(%d): invalid inputparams\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_MEM;
    }

    memset(pAdrcCtx, 0x00, sizeof(AdrcContext_t));
    *ppAdrcCtx      = pAdrcCtx;
    pAdrcCtx->state = ADRC_STATE_INITIALIZED;

    CalibDbV2_drc_V11_t* calibv2_adrc_calib =
        (CalibDbV2_drc_V11_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, adrc_calib));

    pAdrcCtx->drcAttrV11.opMode = DRC_OPMODE_AUTO;
    SetDefaultValueV11(pAdrcCtx);  // set default para
    memcpy(&pAdrcCtx->drcAttrV11.stAuto, calibv2_adrc_calib,
           sizeof(CalibDbV2_drc_V11_t));  // set stAuto

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
/******************************************************************************
 * AhdrRelease()
 *****************************************************************************/
XCamReturn AdrcRelease(AdrcContext_t* pAdrcCtx) {
    LOG1_ATMO("%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    // initial checks
    if (NULL == pAdrcCtx) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    result = AdrcStop(pAdrcCtx);
    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_ATMO("%s: AHDRStop() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ADRC_STATE_RUNNING == pAdrcCtx->state) || (ADRC_STATE_LOCKED == pAdrcCtx->state)) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    free(pAdrcCtx);
    pAdrcCtx = NULL;

    LOG1_ATMO("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
