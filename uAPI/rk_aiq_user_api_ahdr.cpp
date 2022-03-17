/*
 *  Copyright (c) 2019 Rockchip Corporation
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
 *
 */

#include "rk_aiq_user_api_ahdr.h"
#include "RkAiqHandleInt.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

void
MergeSetData(CalibDbV2_merge_t* CalibV2, CalibDb_HdrMerge_t*CalibV1)
{
    //len
    CalibV2->MergeTuningPara.OECurve.EnvLv_len = 13;
    CalibV2->MergeTuningPara.OECurve.Smooth_len = CalibV2->MergeTuningPara.OECurve.EnvLv_len;
    CalibV2->MergeTuningPara.OECurve.Offset_len = CalibV2->MergeTuningPara.OECurve.EnvLv_len;

    CalibV2->MergeTuningPara.MDCurve.MoveCoef_len = 13;
    CalibV2->MergeTuningPara.MDCurve.LM_smooth_len = CalibV2->MergeTuningPara.MDCurve.MoveCoef_len;
    CalibV2->MergeTuningPara.MDCurve.LM_offset_len = CalibV2->MergeTuningPara.MDCurve.MoveCoef_len;
    CalibV2->MergeTuningPara.MDCurve.MS_smooth_len = CalibV2->MergeTuningPara.MDCurve.MoveCoef_len;
    CalibV2->MergeTuningPara.MDCurve.MS_offset_len = CalibV2->MergeTuningPara.MDCurve.MoveCoef_len;

    //malloc
    CalibV2->MergeTuningPara.OECurve.EnvLv = (float *) malloc(sizeof(float) * 13);
    CalibV2->MergeTuningPara.OECurve.Smooth = (float *) malloc(sizeof(float) * 13);
    CalibV2->MergeTuningPara.OECurve.Offset = (float *) malloc(sizeof(float) * 13);

    CalibV2->MergeTuningPara.MDCurve.MoveCoef = (float *) malloc(sizeof(float) * 13);
    CalibV2->MergeTuningPara.MDCurve.LM_smooth = (float *) malloc(sizeof(float) * 13);
    CalibV2->MergeTuningPara.MDCurve.LM_offset = (float *) malloc(sizeof(float) * 13);
    CalibV2->MergeTuningPara.MDCurve.MS_smooth = (float *) malloc(sizeof(float) * 13);
    CalibV2->MergeTuningPara.MDCurve.MS_offset = (float *) malloc(sizeof(float) * 13);


    for(int i = 0; i < 13; i++) {
        CalibV2->MergeTuningPara.OECurve.EnvLv[i] = CalibV1->envLevel[i];
        CalibV2->MergeTuningPara.OECurve.Smooth[i] = CalibV1->oeCurve_smooth[i];
        CalibV2->MergeTuningPara.OECurve.Offset[i] = CalibV1->oeCurve_offset[i];
        CalibV2->MergeTuningPara.MDCurve.MoveCoef[i] = CalibV1->moveCoef[i];
        CalibV2->MergeTuningPara.MDCurve.LM_smooth[i] = CalibV1->mdCurveLm_smooth[i];
        CalibV2->MergeTuningPara.MDCurve.LM_offset[i] = CalibV1->mdCurveLm_offset[i];
        CalibV2->MergeTuningPara.MDCurve.MS_smooth[i] = CalibV1->mdCurveMs_smooth[i];
        CalibV2->MergeTuningPara.MDCurve.MS_offset[i] = CalibV1->mdCurveMs_offset[i];
    }
    CalibV2->MergeTuningPara.OECurve_damp = CalibV1->oeCurve_damp;
    CalibV2->MergeTuningPara.MDCurveLM_damp = CalibV1->mdCurveLm_damp;
    CalibV2->MergeTuningPara.MDCurveMS_damp = CalibV1->mdCurveMs_damp;
}

void
TmoSetData(CalibDbV2_tmo_t* CalibV2, CalibDb_HdrTmo_t*CalibV1)
{
    //len
    CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len = 13;
    CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.Strength_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.ISO_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.Strength_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.Strength_len = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;

    //malloc
    CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.Strength = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.ISO = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.Strength = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv = (float *) malloc(sizeof(float) * 13);
    CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.Strength = (float *) malloc(sizeof(float) * 13);


    CalibV2->TmoTuningPara.Enable = CalibV1->en[0].en;
    //global luma
    CalibV2->TmoTuningPara.GlobalLuma.Mode = CalibV1->luma[0].GlobalLumaMode ? GLOBALLUMAMODE_ISO : GLOBALLUMAMODE_ENVLV;
    for(int i = 0; i < 13; i++) {
        CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv[i] = CalibV1->luma[0].envLevel[i];
        CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO[i] = CalibV1->luma[0].ISO[i];
        CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength[i] = CalibV1->luma[0].globalLuma[i];
    }
    CalibV2->TmoTuningPara.GlobalLuma.Tolerance = CalibV1->luma[0].Tolerance;
    //details high light
    CalibV2->TmoTuningPara.DetailsHighLight.Mode = CalibV1->HighLight[0].DetailsHighLightMode ? DETAILSHIGHLIGHTMODE_ENVLV : DETAILSHIGHLIGHTMODE_OEPDF;
    for(int i = 0; i < 13; i++) {

        CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf[i] = CalibV1->HighLight[0].OEPdf[i];
        CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv[i] = CalibV1->HighLight[0].EnvLv[i];
        CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.Strength[i] = CalibV1->HighLight[0].detailsHighLight[i];
    }
    CalibV2->TmoTuningPara.DetailsHighLight.Tolerance = CalibV1->HighLight[0].Tolerance;
    //details low light
    if(CalibV1->LowLight[0].DetailsLowLightMode == 0)
        CalibV2->TmoTuningPara.DetailsLowLight.Mode = DETAILSLOWLIGHTMODE_FOCUSLUMA;
    else if(CalibV1->LowLight[0].DetailsLowLightMode > 0 && CalibV1->LowLight[0].DetailsLowLightMode < 2)
        CalibV2->TmoTuningPara.DetailsLowLight.Mode = DETAILSLOWLIGHTMODE_DARKPDF;
    else if(CalibV1->LowLight[0].DetailsLowLightMode >= 2)
        CalibV2->TmoTuningPara.DetailsLowLight.Mode = DETAILSLOWLIGHTMODE_ISO;
    for(int i = 0; i < 13; i++) {

        CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma[i] = CalibV1->LowLight[0].FocusLuma[i];
        CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf[i] = CalibV1->LowLight[0].DarkPdf[i];
        CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.ISO[i] = CalibV1->LowLight[0].ISO[i];
        CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.Strength[i] = CalibV1->LowLight[0].detailsLowLight[i];
    }
    CalibV2->TmoTuningPara.DetailsLowLight.Tolerance = CalibV1->LowLight[0].Tolerance;
    //global tmo
    CalibV2->TmoTuningPara.GlobaTMO.IIR = CalibV1->GlobaTMO[0].iir;
    CalibV2->TmoTuningPara.GlobaTMO.Mode = CalibV1->GlobaTMO[0].mode ? TMOTYPEMODE_ENVLV : TMOTYPEMODE_DYNAMICRANGE;
    for(int i = 0; i < 13; i++) {

        CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange[i] = CalibV1->GlobaTMO[0].DynamicRange[i];
        CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv[i] = CalibV1->GlobaTMO[0].EnvLv[i];
        CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength[i] = CalibV1->GlobaTMO[0].Strength[i];
    }
    CalibV2->TmoTuningPara.GlobaTMO.Tolerance = CalibV1->GlobaTMO[0].Tolerance;
    //local tmo
    CalibV2->TmoTuningPara.LocalTMO.Mode = CalibV1->LocalTMO[0].LocalTMOMode ? TMOTYPEMODE_ENVLV : TMOTYPEMODE_DYNAMICRANGE;
    for(int i = 0; i < 13; i++) {

        CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange[i] = CalibV1->LocalTMO[0].DynamicRange[i];
        CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv[i] = CalibV1->LocalTMO[0].EnvLv[i];
        CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.Strength[i] = CalibV1->LocalTMO[0].Strength[i];
    }
    CalibV2->TmoTuningPara.LocalTMO.Tolerance = CalibV1->LocalTMO[0].Tolerance;
    CalibV2->TmoTuningPara.damp = CalibV1->damp;
}

void
TransferSetData(amerge_attrib_t* merge, atmo_attrib_t*tmo, ahdr_attrib_t *attr)
{
    switch (attr->opMode)
    {
    case HDR_OpMode_Api_OFF:
        merge->opMode = MERGE_OPMODE_API_OFF;
        tmo->opMode = TMO_OPMODE_API_OFF;
        break;
    case HDR_OpMode_Auto:
        merge->opMode = MERGE_OPMODE_AUTO;
        tmo->opMode = TMO_OPMODE_AUTO;
        break;
    case HDR_OpMode_MANU:
        merge->opMode = MERGE_OPMODE_MANU;
        tmo->opMode = TMO_OPMODE_MANU;
        break;
    case HDR_OpMode_SET_LEVEL:
        merge->opMode = MERGE_OPMODE_API_OFF;
        tmo->opMode = TMO_OPMODE_SET_LEVEL;
        break;
    case HDR_OpMode_DarkArea:
        merge->opMode = MERGE_OPMODE_API_OFF;
        tmo->opMode = TMO_OPMODE_DARKAREA;
        break;
    case HDR_OpMode_Tool:
        merge->opMode = MERGE_OPMODE_TOOL;
        tmo->opMode = TMO_OPMODE_TOOL;
        break;
    default:
        LOGE_ATMO("%s: Wrong HDR api opmode!!!\n", __FUNCTION__);
        break;
    }

    //st auto
    merge->stAuto.bUpdateMge = attr->stAuto.bUpdateMge;
    memcpy(&merge->stAuto.stMgeAuto, &attr->stAuto.stMgeAuto, sizeof(amgeAttr_t));
    tmo->stAuto.bUpdateTmo = attr->stAuto.bUpdateTmo;
    memcpy(&tmo->stAuto.stTmoAuto, &attr->stAuto.stTmoAuto, sizeof(atmoAttr_t));

    //st manu
    merge->stManual.bUpdateMge = attr->stManual.bUpdateMge;
    memcpy(&merge->stManual.stMgeManual, &attr->stManual.stMgeManual, sizeof(mmgeAttr_t));
    tmo->stManual.bUpdateTmo = attr->stManual.bUpdateTmo;
    memcpy(&tmo->stManual.stTmoManual, &attr->stManual.stTmoManual, sizeof(mtmoAttr_t));

    //st level
    tmo->stSetLevel.level = attr->stSetLevel.level;

    //st darkare
    tmo->stDarkArea.level = attr->stDarkArea.level;

    //st tool
    MergeSetData(&merge->stTool, &attr->stTool.merge);
    TmoSetData(&tmo->stTool, &attr->stTool.tmo);

}

void
MergeGetData(CalibDbV2_merge_t* CalibV2, CalibDb_HdrMerge_t*CalibV1)
{
    //oe curve
    if(CalibV2->MergeTuningPara.OECurve.EnvLv_len >= 13 ) {
        for(int i = 0 ; i < 13; i++) {
            CalibV1->envLevel[i] = CalibV2->MergeTuningPara.OECurve.EnvLv[i];
            CalibV1->oeCurve_smooth[i] = CalibV2->MergeTuningPara.OECurve.Smooth[i];
            CalibV1->oeCurve_offset[i] = CalibV2->MergeTuningPara.OECurve.Offset[i];
        }
    }
    else if (CalibV2->MergeTuningPara.OECurve.EnvLv_len >= 1 && CalibV2->MergeTuningPara.OECurve.EnvLv_len < 13 ) {
        for(int i = 0 ; i < CalibV2->MergeTuningPara.OECurve.EnvLv_len; i++) {
            CalibV1->envLevel[i] = CalibV2->MergeTuningPara.OECurve.EnvLv[i];
            CalibV1->oeCurve_smooth[i] = CalibV2->MergeTuningPara.OECurve.Smooth[i];
            CalibV1->oeCurve_offset[i] = CalibV2->MergeTuningPara.OECurve.Offset[i];
        }
        int j = CalibV2->MergeTuningPara.OECurve.EnvLv_len - 1;
        for(int i = CalibV2->MergeTuningPara.OECurve.EnvLv_len ; i < 13; i++) {
            CalibV1->envLevel[i] = CalibV2->MergeTuningPara.OECurve.EnvLv[j];
            CalibV1->oeCurve_smooth[i] = CalibV2->MergeTuningPara.OECurve.Smooth[j];
            CalibV1->oeCurve_offset[i] = CalibV2->MergeTuningPara.OECurve.Offset[j];
        }
    }
    //md curve
    if(CalibV2->MergeTuningPara.MDCurve.MoveCoef_len >= 13 ) {
        for(int i = 0 ; i < 13; i++) {
            CalibV1->moveCoef[i] = CalibV2->MergeTuningPara.MDCurve.MoveCoef[i];
            CalibV1->mdCurveLm_smooth[i] = CalibV2->MergeTuningPara.MDCurve.LM_smooth[i];
            CalibV1->mdCurveLm_offset[i] = CalibV2->MergeTuningPara.MDCurve.LM_offset[i];
            CalibV1->mdCurveMs_smooth[i] = CalibV2->MergeTuningPara.MDCurve.MS_smooth[i];
            CalibV1->mdCurveMs_offset[i] = CalibV2->MergeTuningPara.MDCurve.MS_offset[i];
        }
    }
    else if (CalibV2->MergeTuningPara.MDCurve.MoveCoef_len >= 1 && CalibV2->MergeTuningPara.MDCurve.MoveCoef_len < 13 ) {
        for(int i = 0 ; i < CalibV2->MergeTuningPara.MDCurve.MoveCoef_len; i++) {
            CalibV1->moveCoef[i] = CalibV2->MergeTuningPara.MDCurve.MoveCoef[i];
            CalibV1->mdCurveLm_smooth[i] = CalibV2->MergeTuningPara.MDCurve.LM_smooth[i];
            CalibV1->mdCurveLm_offset[i] = CalibV2->MergeTuningPara.MDCurve.LM_offset[i];
            CalibV1->mdCurveMs_smooth[i] = CalibV2->MergeTuningPara.MDCurve.MS_smooth[i];
            CalibV1->mdCurveMs_offset[i] = CalibV2->MergeTuningPara.MDCurve.MS_offset[i];
        }
        int j = CalibV2->MergeTuningPara.MDCurve.MoveCoef_len - 1;
        for(int i = CalibV2->MergeTuningPara.MDCurve.MoveCoef_len; i < 13; i++) {
            CalibV1->moveCoef[i] = CalibV2->MergeTuningPara.MDCurve.MoveCoef[j];
            CalibV1->mdCurveLm_smooth[i] = CalibV2->MergeTuningPara.MDCurve.LM_smooth[j];
            CalibV1->mdCurveLm_offset[i] = CalibV2->MergeTuningPara.MDCurve.LM_offset[j];
            CalibV1->mdCurveMs_smooth[i] = CalibV2->MergeTuningPara.MDCurve.MS_smooth[j];
            CalibV1->mdCurveMs_offset[i] = CalibV2->MergeTuningPara.MDCurve.MS_offset[j];
        }
    }
    //other
    CalibV1->oeCurve_damp = CalibV2->MergeTuningPara.OECurve_damp;
    CalibV1->mdCurveLm_damp = CalibV2->MergeTuningPara.MDCurveLM_damp;
    CalibV1->mdCurveMs_damp = CalibV2->MergeTuningPara.MDCurveMS_damp;

}

void
TmoGetData(CalibDbV2_tmo_t* CalibV2, CalibDb_HdrTmo_t*CalibV1)
{

    for(int i = 0; i < 3; i++) {
        //global luma
        if(CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len >= 13 ) {
            for(int j = 0 ; j < 13; j++) {
                CalibV1->luma[i].envLevel[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv[j];
                CalibV1->luma[i].ISO[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO[j];
                CalibV1->luma[i].globalLuma[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength[j];
            }
        } else if(CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len >= 1 && CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len < 13 ) {
            for(int j = 0 ; j < CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len; j++) {
                CalibV1->luma[i].envLevel[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv[j];
                CalibV1->luma[i].ISO[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO[j];
                CalibV1->luma[i].globalLuma[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength[j];
            }
            int k = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len - 1;
            for(int j = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len; j < 13; j++) {
                CalibV1->luma[i].envLevel[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv[k];
                CalibV1->luma[i].ISO[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO[k];
                CalibV1->luma[i].globalLuma[j] = CalibV2->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength[k];
            }
        }

        //high light
        if(CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len >= 13 ) {
            for(int j = 0 ; j < 13; j++) {
                CalibV1->HighLight[i].OEPdf[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf[j];
                CalibV1->HighLight[i].EnvLv[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv[j];
                CalibV1->HighLight[i].detailsHighLight[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.Strength[j];
            }
        } else if(CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len >= 1 && CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len < 13 ) {
            for(int j = 0 ; j < CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len; j++) {
                CalibV1->HighLight[i].OEPdf[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf[j];
                CalibV1->HighLight[i].EnvLv[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv[j];
                CalibV1->HighLight[i].detailsHighLight[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.Strength[j];
            }
            int k = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len - 1;
            for(int j = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len; j < 13; j++) {
                CalibV1->HighLight[i].OEPdf[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf[k];
                CalibV1->HighLight[i].EnvLv[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv[k];
                CalibV1->HighLight[i].detailsHighLight[j] = CalibV2->TmoTuningPara.DetailsHighLight.HighLightData.Strength[k];
            }
        }
        //low light
        if(CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len >= 13 ) {
            for(int j = 0 ; j < 13; j++) {
                CalibV1->LowLight[i].FocusLuma[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma[j];
                CalibV1->LowLight[i].DarkPdf[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf[j];
                CalibV1->LowLight[i].ISO[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.ISO[j];
                CalibV1->LowLight[i].detailsLowLight[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.Strength[j];
            }
        } else if(CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len >= 1 && CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len < 13 ) {
            for(int j = 0 ; j < CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len; j++) {
                CalibV1->LowLight[i].FocusLuma[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma[j];
                CalibV1->LowLight[i].DarkPdf[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf[j];
                CalibV1->LowLight[i].ISO[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.ISO[j];
                CalibV1->LowLight[i].detailsLowLight[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.Strength[j];
            }
            int k = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len - 1;
            for(int j = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len; j < 13; j++) {
                CalibV1->LowLight[i].FocusLuma[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma[k];
                CalibV1->LowLight[i].DarkPdf[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf[k];
                CalibV1->LowLight[i].ISO[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.ISO[k];
                CalibV1->LowLight[i].detailsLowLight[j] = CalibV2->TmoTuningPara.DetailsLowLight.LowLightData.Strength[k];
            }
        }
        //global tmo
        if(CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len >= 13 ) {
            for(int j = 0 ; j < 13; j++) {
                CalibV1->GlobaTMO[i].DynamicRange[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange[j];
                CalibV1->GlobaTMO[i].EnvLv[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv[j];
                CalibV1->GlobaTMO[i].Strength[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength[j];
            }
        } else if(CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len >= 1 && CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len < 13 ) {
            for(int j = 0 ; j < CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len; j++) {
                CalibV1->GlobaTMO[i].DynamicRange[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange[j];
                CalibV1->GlobaTMO[i].EnvLv[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv[j];
                CalibV1->GlobaTMO[i].Strength[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength[j];
            }
            int k = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len - 1;
            for(int j = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len; j < 13; j++) {
                CalibV1->GlobaTMO[i].DynamicRange[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange[k];
                CalibV1->GlobaTMO[i].EnvLv[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv[k];
                CalibV1->GlobaTMO[i].Strength[j] = CalibV2->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength[k];
            }
        }
        //local tmo
        if(CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len >= 13 ) {
            for(int j = 0 ; j < 13; j++) {
                CalibV1->LocalTMO[i].DynamicRange[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange[j];
                CalibV1->LocalTMO[i].EnvLv[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv[j];
                CalibV1->LocalTMO[i].Strength[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.Strength[j];
            }
        } else if(CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len >= 1 && CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len < 13 ) {
            for(int j = 0 ; j < CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len; j++) {
                CalibV1->LocalTMO[i].DynamicRange[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange[j];
                CalibV1->LocalTMO[i].EnvLv[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv[j];
                CalibV1->LocalTMO[i].Strength[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.Strength[j];
            }
            int k = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len - 1;
            for(int j = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len; j < 13; j++) {
                CalibV1->LocalTMO[i].DynamicRange[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange[k];
                CalibV1->LocalTMO[i].EnvLv[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv[k];
                CalibV1->LocalTMO[i].Strength[j] = CalibV2->TmoTuningPara.LocalTMO.LocalTmoData.Strength[k];
            }
        }
    }

    //other
    for(int i = 0; i < 3; i++) {
        CalibV1->en[i].en = CalibV2->TmoTuningPara.Enable ? 1 : 0;
        CalibV1->luma[i].GlobalLumaMode = CalibV2->TmoTuningPara.GlobalLuma.Mode;
        CalibV1->luma[i].Tolerance = CalibV2->TmoTuningPara.GlobalLuma.Tolerance;
        CalibV1->HighLight[i].DetailsHighLightMode = CalibV2->TmoTuningPara.DetailsHighLight.Mode;
        CalibV1->HighLight[i].Tolerance = CalibV2->TmoTuningPara.DetailsHighLight.Tolerance;
        CalibV1->LowLight[i].DetailsLowLightMode = CalibV2->TmoTuningPara.DetailsLowLight.Mode;
        CalibV1->LowLight[i].Tolerance = CalibV2->TmoTuningPara.DetailsLowLight.Tolerance;
        CalibV1->LocalTMO[i].LocalTMOMode = CalibV2->TmoTuningPara.LocalTMO.Mode;
        CalibV1->LocalTMO[i].Tolerance = CalibV2->TmoTuningPara.LocalTMO.Tolerance;
        CalibV1->GlobaTMO[i].en = CalibV2->TmoTuningPara.GlobaTMO.Enable ? 1 : 0;
        CalibV1->GlobaTMO[i].iir = CalibV2->TmoTuningPara.GlobaTMO.IIR;
        CalibV1->GlobaTMO[i].mode = CalibV2->TmoTuningPara.GlobaTMO.Mode;
        CalibV1->GlobaTMO[i].Tolerance = CalibV2->TmoTuningPara.GlobaTMO.Tolerance;
    }
    CalibV1->damp = CalibV2->TmoTuningPara.damp;


}

void
TransferGetData(amerge_attrib_t* merge, atmo_attrib_t*tmo, ahdr_attrib_t *attr)
{
    switch (tmo->opMode)
    {
    case TMO_OPMODE_API_OFF:
        attr->opMode = HDR_OpMode_Api_OFF;
        break;
    case TMO_OPMODE_AUTO:
        attr->opMode = HDR_OpMode_Auto;
        break;
    case TMO_OPMODE_MANU:
        attr->opMode = HDR_OpMode_MANU;
        break;
    case TMO_OPMODE_SET_LEVEL:
        attr->opMode = HDR_OpMode_SET_LEVEL;
        break;
    case TMO_OPMODE_DARKAREA:
        attr->opMode = HDR_OpMode_DarkArea;
        break;
    case TMO_OPMODE_TOOL:
        attr->opMode = HDR_OpMode_Tool;
        break;
    default:
        LOGE_ATMO("%s: Wrong HDR api opmode!!!\n", __FUNCTION__);
        break;
    }

    //st auto
    attr->stAuto.bUpdateMge = merge->stAuto.bUpdateMge;
    memcpy(&attr->stAuto.stMgeAuto, &merge->stAuto.stMgeAuto, sizeof(amgeAttr_t));
    attr->stAuto.bUpdateTmo = tmo->stAuto.bUpdateTmo;
    memcpy(&attr->stAuto.stTmoAuto, &tmo->stAuto.stTmoAuto, sizeof(atmoAttr_t));

    //st manu
    attr->stManual.bUpdateMge = merge->stManual.bUpdateMge;
    memcpy(&attr->stManual.stMgeManual, &merge->stManual.stMgeManual, sizeof(mmgeAttr_t));
    attr->stManual.bUpdateTmo = tmo->stManual.bUpdateTmo;
    memcpy(&attr->stManual.stTmoManual, &tmo->stManual.stTmoManual, sizeof(mtmoAttr_t));

    //st level
    attr->stSetLevel.level = tmo->stSetLevel.level;

    //st darkare
    attr->stDarkArea.level = tmo->stDarkArea.level;

    //st tool
    MergeGetData(&merge->stTool, &attr->stTool.merge);
    TmoGetData(&tmo->stTool, &attr->stTool.tmo);

    //get ctrl data
    attr->CtlInfo.MoveCoef = merge->CtlInfo.MoveCoef;
    attr->CtlInfo.SceneMode = 0;
    attr->CtlInfo.GlobalLumaMode = tmo->CtlInfo.GlobalLumaMode;
    attr->CtlInfo.DetailsHighLightMode = tmo->CtlInfo.DetailsHighLightMode;
    attr->CtlInfo.DetailsLowLightMode = tmo->CtlInfo.DetailsLowLightMode;
    attr->CtlInfo.GlobalTmoMode = tmo->CtlInfo.GlobalTmoMode;
    attr->CtlInfo.Envlv = tmo->CtlInfo.Envlv;
    attr->CtlInfo.ISO = tmo->CtlInfo.ISO;
    attr->CtlInfo.OEPdf = tmo->CtlInfo.OEPdf;
    attr->CtlInfo.FocusLuma = tmo->CtlInfo.FocusLuma;
    attr->CtlInfo.DarkPdf = tmo->CtlInfo.DarkPdf;
    attr->CtlInfo.DynamicRange = tmo->CtlInfo.DynamicRange;

    //get reg data
    attr->RegInfo.OECurve_smooth = merge->RegInfo.OECurve_smooth;
    attr->RegInfo.OECurve_offset = merge->RegInfo.OECurve_offset;
    attr->RegInfo.MDCurveLM_smooth = merge->RegInfo.MDCurveLM_smooth;
    attr->RegInfo.MDCurveLM_offset = merge->RegInfo.MDCurveLM_offset;
    attr->RegInfo.MDCurveMS_smooth = merge->RegInfo.MDCurveMS_smooth;
    attr->RegInfo.MDCurveMS_offset = merge->RegInfo.MDCurveMS_offset;

    attr->RegInfo.GlobalLuma = tmo->RegInfo.GlobalLuma;
    attr->RegInfo.DetailsHighlight = tmo->RegInfo.DetailsHighlight;
    attr->RegInfo.DetailsLowlight = tmo->RegInfo.DetailsLowlight;
    attr->RegInfo.GlobaltmoStrength = tmo->RegInfo.GlobaltmoStrength;
    attr->RegInfo.LocalTmoStrength = tmo->RegInfo.LocalTmoStrength;

}

XCamReturn
rk_aiq_user_api_ahdr_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ahdr_attrib_t attr)
{

    amerge_attrib_t mergeAttr;
    memset(&mergeAttr, 0, sizeof(amerge_attrib_t));
    atmo_attrib_t tmoAttr;
    memset(&tmoAttr, 0, sizeof(atmo_attrib_t));

    TransferSetData(&mergeAttr, &tmoAttr, &attr);

    XCamReturn ret_amerge = rk_aiq_user_api2_amerge_SetAttrib(sys_ctx, mergeAttr);
    XCamReturn ret_atmo = rk_aiq_user_api2_atmo_SetAttrib(sys_ctx, tmoAttr);


    if (ret_amerge != XCAM_RETURN_NO_ERROR)
        return ret_amerge;

    if (ret_atmo != XCAM_RETURN_NO_ERROR)
        return ret_atmo;

    return  XCAM_RETURN_NO_ERROR;

}

XCamReturn
rk_aiq_user_api_ahdr_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ahdr_attrib_t* attr)
{
    amerge_attrib_t mergeAttr;
    memset(&mergeAttr, 0, sizeof(amerge_attrib_t));
    atmo_attrib_t tmoAttr;
    memset(&tmoAttr, 0, sizeof(atmo_attrib_t));

    TransferGetData(&mergeAttr, &tmoAttr, attr);

    XCamReturn ret_amerge = rk_aiq_user_api2_amerge_GetAttrib(sys_ctx, &mergeAttr);
    XCamReturn ret_atmo = rk_aiq_user_api2_atmo_GetAttrib(sys_ctx, &tmoAttr);

    if (ret_amerge != XCAM_RETURN_NO_ERROR)
        return ret_amerge;

    if (ret_atmo != XCAM_RETURN_NO_ERROR)
        return ret_atmo;

    return XCAM_RETURN_NO_ERROR;
}

RKAIQ_END_DECLARE
