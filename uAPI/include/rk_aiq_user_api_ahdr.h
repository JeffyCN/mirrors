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

#ifndef _RK_AIQ_USER_API_AHDR_H_
#define _RK_AIQ_USER_API_AHDR_H_

#include "RkAiqCalibDbTypes.h"
#include "uAPI2/rk_aiq_user_api2_atmo.h"
#include "uAPI2/rk_aiq_user_api2_amerge.h"


/*
typedef enum hdr_OpMode_s {
    HDR_OpMode_Api_OFF = 0, // run IQ ahdr
    HDR_OpMode_Auto = 1, //run api auto ahdr
    HDR_OpMode_MANU = 2, //run api manual ahdr
    HDR_OpMode_SET_LEVEL = 3, // its prevously fast mode, run api set level
    HDR_OpMode_DarkArea = 4, // for dark area luma inprove, no matter the scene is night, normal, or hdr
    HDR_OpMode_Tool = 5, // for dark area luma inprove, no matter the scene is night, normal, or hdr
} hdr_OpMode_t;

typedef struct ahdrAttr_s
{
    bool      bUpdateTmo;
    bool      bUpdateMge;
    amgeAttr_t stMgeAuto;
    atmoAttr_t stTmoAuto;
} ahdrAttr_t;

typedef struct mhdrAttr_s
{
    bool      bUpdateTmo;
    bool      bUpdateMge;
    mmgeAttr_t stMgeManual;
    mtmoAttr_t stTmoManual;
} mhdrAttr_t;

typedef struct CurrCtlData_s
{
    int SceneMode;
    float GlobalLumaMode;
    float DetailsHighLightMode;
    float DetailsLowLightMode;
    float GlobalTmoMode;
    float LocalTMOMode;
    float Envlv;
    float MoveCoef;
    float ISO;
    float OEPdf;
    float FocusLuma;
    float DarkPdf;
    float DynamicRange;
} CurrCtlData_t;

typedef struct CurrRegData_s
{
    float OECurve_smooth;
    float OECurve_offset;
    float MDCurveLM_smooth;
    float MDCurveLM_offset;
    float MDCurveMS_smooth;
    float MDCurveMS_offset;

    float GlobalLuma;
    float DetailsLowlight;
    float DetailsHighlight;
    float LocalTmoStrength;
    float GlobaltmoStrength;
} CurrRegData_t;

typedef struct CalibDb_HdrMerge_s
{
    float envLevel[13];
    float oeCurve_smooth[13];
    float oeCurve_offset[13];
    float moveCoef[13];
    float mdCurveLm_smooth[13];
    float mdCurveLm_offset[13];
    float mdCurveMs_smooth[13];
    float mdCurveMs_offset[13];
    float oeCurve_damp;
    float mdCurveLm_damp;
    float mdCurveMs_damp;
} CalibDb_HdrMerge_t;

typedef struct CalibDb_HdrTmo_s
{
    TMO_en_t en[CALIBDB_MAX_MODE_NUM];
    GlobalLuma_t luma[CALIBDB_MAX_MODE_NUM];
    DetailsHighLight_t HighLight[CALIBDB_MAX_MODE_NUM];
    DetailsLowLight_t LowLight[CALIBDB_MAX_MODE_NUM];
    LocalTMO_t LocalTMO[CALIBDB_MAX_MODE_NUM];
    GlobaTMO_t GlobaTMO[CALIBDB_MAX_MODE_NUM];
    float damp;
} CalibDb_HdrTmo_t;

typedef struct CalibDb_Ahdr_Para_s {
    CalibDb_HdrMerge_t merge;
    CalibDb_HdrTmo_t tmo;
} CalibDb_Ahdr_Para_t;

typedef struct ahdr_attrib_s
{
    hdr_OpMode_t    opMode;
    ahdrAttr_t    stAuto;
    mhdrAttr_t stManual;
    FastMode_t stSetLevel;
    DarkArea_t stDarkArea;
    CurrCtlData_t CtlInfo;
    CurrRegData_t RegInfo;
    CalibDb_Ahdr_Para_t stTool;
} ahdr_attrib_t;

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
*/

RKAIQ_BEGIN_DECLARE

/*
XCamReturn
rk_aiq_user_api_ahdr_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ahdr_attrib_t attr);
XCamReturn
rk_aiq_user_api_ahdr_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ahdr_attrib_t* attr);
*/

RKAIQ_END_DECLARE

#endif
