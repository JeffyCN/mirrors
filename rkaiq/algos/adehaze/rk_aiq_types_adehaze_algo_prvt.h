/*
 *rk_aiq_types_adehaze_algo_prvt.h
 *
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

#ifndef _RK_AIQ_TYPES_ADEHAZE_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_ADEHAZE_ALGO_PRVT_H_

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "adehaze/rk_aiq_types_adehaze_algo_int.h"
#include "amerge/rk_aiq_types_amerge_algo_prvt.h"
#include "rk_aiq_algo_types.h"
#include "rk_aiq_types_adehaze_hw.h"
#include "rk_aiq_types_adehaze_stat.h"
#include "xcam_log.h"

#define DEHAZE_GAUS_H0                              (2)
#define DEHAZE_GAUS_H1                              (4)
#define DEHAZE_GAUS_H2                              (2)
#define DEHAZE_GAUS_H3                              (4)
#define DEHAZE_GAUS_H4                              (8)
#define DEHAZE_GAUS_H5                              (4)
#define DEHAZE_GAUS_H6                              (2)
#define DEHAZE_GAUS_H7                              (4)
#define DEHAZE_GAUS_H8                              (2)
#define CFG_ALPHA_MAX                               (255)
#define CFG_ALPHA_MIN                               (NORMALIZE_MIN)
#define CFG_AIR_MAX                                 (255)
#define CFG_AIR_MIN                                 (NORMALIZE_MIN)
#define ENHANCE_VALUE_MAX                           (16.0)
#define ENHANCE_VALUE_MIN                           (1.0)
#define ENHANCE_CHROME_MAX                          (16.0)
#define ENHANCE_CHROME_MIN                          (1.0)
#define HSIT_WR_MIN_STEP                            (16)
#define HSIT_WR_X_MAX                               (1023)
#define HSIT_BLOCK_HEIGHT_MIN                       (64)
#define HIST_BLOCK_HEIGHT_MAX                       (876)
#define HSIT_BLOCK_WIDTH_MIN                        (66)
#define HIST_BLOCK_WIDTH_MAX                        (1168)

//define for dehaze local gain
#define YNR_BIT_CALIB (12)
#define YNR_ISO_CURVE_POINT_BIT             (4)
#define YNR_ISO_CURVE_SECT_VALUE   (1 << (YNR_BIT_CALIB - YNR_ISO_CURVE_POINT_BIT))
#define YNR_ISO_CURVE_SECT_VALUE1   (1 << YNR_BIT_CALIB)
#define YNR_CURVE_STEP             (16)

typedef struct AdehazeExpInfo_s {
    int hdr_mode;
    float arTime[3];
    float arAGain[3];
    float arDGain[3];
    int arIso[3];
} AdehazeExpInfo_t;

typedef struct AdehazeAePreResV10_s {
    float ISO;
    CtrlDataType_t CtrlDataType;
    dehaze_api_mode_t ApiMode;
} AdehazeAePreResV10_t;

typedef struct AdehazeAePreResV11_s {
    float EnvLv;
    float ISO;
    CtrlDataType_t CtrlDataType;
    dehaze_api_mode_t ApiMode;
} AdehazeAePreResV11_t;

typedef struct AdehazeHandle_s {
    bool isCapture;
    bool ifReCalcStAuto;
    bool ifReCalcStManual;
    bool is_multi_isp_mode;
#if RKAIQ_HAVE_DEHAZE_V10
    adehaze_sw_v10_t AdehazeAtrrV10;
    AdehazeAePreResV10_t CurrDataV10;
    AdehazeAePreResV10_t PreDataV10;
#endif
#if RKAIQ_HAVE_DEHAZE_V11
    adehaze_sw_v11_t AdehazeAtrrV11;
    AdehazeAePreResV11_t CurrDataV11;
    AdehazeAePreResV11_t PreDataV11;
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
    adehaze_sw_v11_t AdehazeAtrrV11duo;
    float YnrProcResV3_sigma[YNR_V3_ISO_CURVE_POINT_NUM];
    AdehazeAePreResV11_t CurrDataV11duo;
    AdehazeAePreResV11_t PreDataV11duo;
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    adehaze_sw_v12_t AdehazeAtrrV12;
    float YnrProcResV22_sigma[YNR_V22_ISO_CURVE_POINT_NUM];
    AdehazeAePreResV11_t CurrDataV12;
    AdehazeAePreResV11_t PreDataV12;
#endif
#if RKAIQ_HAVE_DEHAZE_V14
    adehaze_sw_v14_t AdehazeAtrrV14;
    float YnrProcResV24_sigma[YNR_V24_ISO_CURVE_POINT_NUM];
    AdehazeAePreResV11_t CurrDataV14;
    AdehazeAePreResV11_t PreDataV14;
#endif
    int width;
    int height;
    int strength;
    int working_mode;
    FrameNumber_t FrameNumber;
    uint32_t FrameID;
} AdehazeHandle_t;

#endif
