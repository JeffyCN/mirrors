/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef __AEC_UAPI_HEAD_H__
#define __AEC_UAPI_HEAD_H__

#include "aec_head.h"
#include "ae/rk_aiq_types_ae_hw.h"
#include "rk_aiq_comm.h"

#define MAX_HDR_FRAMENUM (3)
#define RAWAEBIG_SUBWIN_NUM 4
#define RAWAEBIG_WIN_NUM 225

#define ISP2_RAWAE_WINNUM_MAX  ((RAWAELITE_WIN_NUM > RAWAEBIG_WIN_NUM) ? RAWAELITE_WIN_NUM : RAWAEBIG_WIN_NUM)
#define ISP2_HIST_BINNUM_MAX  ((RAWHIST_BIN_N_MAX > SIHIST_BIN_N_MAX) ? RAWHIST_BIN_N_MAX : SIHIST_BIN_N_MAX)

typedef struct Aec_AeRange_uapi_s {
    float Min;
    float Max;
} Aec_AeRange_uapi_t;

typedef struct Aec_LinAeRange_uapi_s {
    Aec_AeRange_uapi_t stExpTimeRange;
    Aec_AeRange_uapi_t stGainRange;
    Aec_AeRange_uapi_t stIspDGainRange;
    Aec_AeRange_uapi_t stPIrisRange;
} Aec_LinAeRange_uapi_t;

typedef struct Aec_HdrAeRange_uapi_s {
    Aec_AeRange_uapi_t stExpTimeRange[3];
    Aec_AeRange_uapi_t stGainRange[3];
    Aec_AeRange_uapi_t stIspDGainRange[3];
    Aec_AeRange_uapi_t stPIrisRange;
} Aec_HdrAeRange_uapi_t;

typedef struct Uapi_AeAttrV2_uapi_s {
    CalibDb_AeSpeedV2_t stAeSpeed;
    // DelayFrmNum
    uint8_t BlackDelayFrame;
    uint8_t WhiteDelayFrame;
    // Auto/Fixed fps
    CalibDb_AeFrmRateAttrV2_t stFrmRate;
    CalibDb_AntiFlickerAttrV2_t stAntiFlicker;
    // auto range
    Aec_LinAeRange_uapi_t LinAeRange; // result LinAerange
    Aec_HdrAeRange_uapi_t HdrAeRange; // result HdrAerange
} Uapi_AeAttrV2_uapi_t;

// manual exposure
typedef struct Uapi_LinMeAttrV2_uapi_s {
    // M4_BOOL_DESC("ManualTimeEn", "1")
    bool ManualTimeEn;
    // M4_BOOL_DESC("ManualGainEn", "1")
    bool ManualGainEn;
    // M4_BOOL_DESC("ManualIspDgainEn", "1")
    bool ManualIspDgainEn;
    // M4_NUMBER_DESC("TimeValue", "f32", M4_RANGE(0,1), "0.003", M4_DIGIT(6),M4_HIDE(0))
    float TimeValue; // unit: s = 10^6 us
    // M4_NUMBER_DESC("GainValue", "f32", M4_RANGE(1,4096), "1", M4_DIGIT(3),M4_HIDE(0))
    float GainValue;
    // M4_NUMBER_DESC("IspDGainValue", "f32", M4_RANGE(1,4096), "1", M4_DIGIT(3),M4_HIDE(0))
    float IspDGainValue;
} Uapi_LinMeAttrV2_uapi_t;

typedef struct Uapi_HdrMeAttrV2_uapi_s {
    // M4_BOOL_DESC("ManualTimeEn", "1")
    bool ManualTimeEn;
    // M4_BOOL_DESC("ManualGainEn", "1")
    bool ManualGainEn;
    // M4_BOOL_DESC("ManualIspDgainEn", "1")
    bool ManualIspDgainEn;
    // M4_ARRAY_DESC("TimeValue", "f32", M4_SIZE(1,3), M4_RANGE(0,1), "0.003", M4_DIGIT(6), M4_DYNAMIC(0))
    float TimeValue[MAX_HDR_FRAMENUM];
    // M4_ARRAY_DESC("GainValue", "f32", M4_SIZE(1,3), M4_RANGE(1,4096), "1.0", M4_DIGIT(3), M4_DYNAMIC(0))
    float GainValue[MAX_HDR_FRAMENUM];
    // M4_ARRAY_DESC("IspDGainValue", "f32", M4_SIZE(1,3), M4_RANGE(1,4096), "1.0", M4_DIGIT(3), M4_DYNAMIC(0))
    float IspDGainValue[MAX_HDR_FRAMENUM];
} Uapi_HdrMeAttrV2_uapi_t;

typedef struct Uapi_MeAttrV2_uapi_s {
    // M4_STRUCT_DESC("LinearAE", "normal_ui_style")
    Uapi_LinMeAttrV2_uapi_t LinearAE;

    // M4_STRUCT_DESC("HdrAE", "normal_ui_style")
    Uapi_HdrMeAttrV2_uapi_t HdrAE;
} Uapi_MeAttrV2_uapi_t;

typedef struct Aec_uapi_advanced_attr_uapi_s {
    bool enable;
    uint8_t GridWeights[15 * 15];
    uint8_t DayGridWeights[RAWAEBIG_WIN_NUM];
    uint8_t NightGridWeights[RAWAEBIG_WIN_NUM];
    bool SetAeRangeEn;
    Aec_LinAeRange_uapi_t SetLinAeRange;
    Aec_HdrAeRange_uapi_t SetHdrAeRange;
} Aec_uapi_advanced_attr_uapi_t;

typedef struct __uapi_expsw_attr_uapi_s {
    uint8_t Enable;
    CalibDb_CamRawStatsModeV2_t RawStatsMode;
    CalibDb_CamHistStatsModeV2_t HistStatsMode;
    CalibDb_CamYRangeModeV2_t YRangeMode;
    uint8_t AecRunInterval;

    // M4_ENUM_DESC("OPMode", "RKAiqOPMode_uapi_t","RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t AecOpType;
    Cam15x15UCharMatrix_t GridWeights;
    Uapi_AeAttrV2_uapi_t stAuto;

    // M4_STRUCT_DESC("ManualCtrl", "normal_ui_style")
    Uapi_MeAttrV2_uapi_t stManual;

    Aec_uapi_advanced_attr_uapi_t stAdvanced;
} uapi_expsw_attr_t;

typedef struct uapi_expinfo {
    // M4_BOOL_DESC("IsConverged", "0",M4_HIDE(1))
    bool              IsConverged;
    // M4_BOOL_DESC("IsExpMax", "0",M4_HIDE(1))
    bool              IsExpMax;
    // M4_NUMBER_DESC("LumaDeviation", "f32", M4_RANGE(-256,256), "0.0", M4_DIGIT(3),M4_HIDE(1))
    float             LumaDeviation;
    // M4_ARRAY_DESC("HdrLumaDeviation", "f32", M4_SIZE(1,3), M4_RANGE(-256,256), "0.0", M4_DIGIT(3), M4_DYNAMIC(0), M4_HIDE(1))
    float             HdrLumaDeviation[3];
    // M4_NUMBER_DESC("MeanLuma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float             MeanLuma;
    // M4_ARRAY_DESC("HdrMeanLuma", "f32", M4_SIZE(1,3), M4_RANGE(0,256), "0.0", M4_DIGIT(2), M4_DYNAMIC(0))
    float             HdrMeanLuma[3];
    // M4_NUMBER_DESC("GlobalEnvLux", "f32", M4_RANGE(0,65535), "0.0", M4_DIGIT(2),M4_HIDE(1))
    float             GlobalEnvLux;
    // M4_ARRAY_DESC("BlockEnvLux", "f32", M4_SIZE(15,15), M4_RANGE(0,65535), "0.0", M4_DIGIT(2), M4_DYNAMIC(0), M4_HIDE(1))
    float             BlockEnvLux[ISP2_RAWAE_WINNUM_MAX];
    // M4_STRUCT_DESC("CurExpInfo", "normal_ui_style");
    RKAiqAecExpInfo_t CurExpInfo;

    // M4_NUMBER_DESC("Piris", "u16", M4_RANGE(0,1024), "0", M4_DIGIT(0),M4_HIDE(1))
    unsigned short    Piris;

    // M4_NUMBER_DESC("LinePeriodsPerField", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(2),M4_HIDE(1))
    float             LinePeriodsPerField;

    // M4_NUMBER_DESC("PixelPeriodsPerLine", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(2),M4_HIDE(1))
    float             PixelPeriodsPerLine;

    // M4_NUMBER_DESC("PixelClockFreqMHZ", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(2),M4_HIDE(1))
    float             PixelClockFreqMHZ;
} uapi_expinfo_t;

#endif /*__AEC_UAPI_HEAD_H__*/
