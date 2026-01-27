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

#include "iq_parser_v2/aec_head.h"
#include "algos/ae/rk_aiq_types_ae_hw.h"
#include "common/rk_aiq_comm.h"

#define MAX_HDR_FRAMENUM (3)
#define RAWAEBIG_SUBWIN_NUM 4
#define RAWAEBIG_WIN_NUM 225

#define ISP2_RAWAE_WINNUM_MAX  ((RAWAELITE_WIN_NUM > RAWAEBIG_WIN_NUM) ? RAWAELITE_WIN_NUM : RAWAEBIG_WIN_NUM)
#define ISP2_HIST_BINNUM_MAX  ((RAWHIST_BIN_N_MAX > SIHIST_BIN_N_MAX) ? RAWHIST_BIN_N_MAX : SIHIST_BIN_N_MAX)

typedef struct Aec_AeRange_uapi_s {
    // M4_NUMBER_DESC("Min", "f32", M4_RANGE(0,65535), "0.003", M4_DIGIT(6),M4_HIDE(0))
    float Min;
    // M4_NUMBER_DESC("Max", "f32", M4_RANGE(0,65535), "0.003", M4_DIGIT(6),M4_HIDE(0))
    float Max;
} Aec_AeRange_uapi_t;

typedef struct Aec_LinAeRange_uapi_s {
    // M4_STRUCT_DESC("Time", "normal_ui_style")
    Aec_AeRange_uapi_t stExpTimeRange;

    // M4_STRUCT_DESC("Gain", "normal_ui_style")
    Aec_AeRange_uapi_t stGainRange;

    // M4_STRUCT_DESC("IspDGain", "normal_ui_style")
    Aec_AeRange_uapi_t stIspDGainRange;

    // M4_STRUCT_DESC("PIris", "normal_ui_style",M4_HIDE(1))
    Aec_AeRange_uapi_t stPIrisRange;
} Aec_LinAeRange_uapi_t;

typedef struct Aec_HdrAeRange_uapi_s {
    // M4_STRUCT_LIST_DESC("Time",  M4_SIZE(1,3), "normal_ui_style")
    Aec_AeRange_uapi_t stExpTimeRange[3];

    // M4_STRUCT_LIST_DESC("Gain", M4_SIZE(1,3), "normal_ui_style")
    Aec_AeRange_uapi_t stGainRange[3];

    // M4_STRUCT_LIST_DESC("IspDGain",  M4_SIZE(1,3), "normal_ui_style")
    Aec_AeRange_uapi_t stIspDGainRange[3];

    // M4_STRUCT_DESC("PIris", "normal_ui_style",M4_HIDE(1))
    Aec_AeRange_uapi_t stPIrisRange;
} Aec_HdrAeRange_uapi_t;

typedef struct Uapi_AeAttrV2_uapi_s {
    CalibDb_AeSpeedV2_t stAeSpeed;
    // DelayFrmNum
    CalibDb_DelayTypeV2_t      DelayType;
    uint8_t                    BlackDelay;
    uint8_t                    WhiteDelay;
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
    rk_aiq_uapi_sync_t                 sync;
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

typedef struct uapi_linaeinfo_s {
    // M4_NUMBER_DESC("LumaDeviation", "f32", M4_RANGE(-256,256), "0.0", M4_DIGIT(3),M4_HIDE(1))
    float             LumaDeviation;

    // M4_NUMBER_DESC("MeanLuma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float             MeanLuma;

    // M4_STRUCT_DESC("LinAeRange", "normal_ui_style")
    Aec_LinAeRange_uapi_t LinAeRange; // result LinAerange

    // M4_STRUCT_DESC("LinearExp", "normal_ui_style")
    RkAiqExpRealParam_t LinearExp;
} uapi_linaeinfo_t;

typedef struct uapi_hdraeinfo_s {
    // M4_ARRAY_DESC("HdrLumaDeviation", "f32", M4_SIZE(1,3), M4_RANGE(-256,256), "0.0", M4_DIGIT(3), M4_DYNAMIC(0), M4_HIDE(1))
    float             HdrLumaDeviation[3];

    // M4_NUMBER_DESC("Frm0Luma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float             Frm0Luma;

    // M4_NUMBER_DESC("Frm1Luma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float             Frm1Luma;

    // M4_NUMBER_DESC("Frm2Luma", "f32", M4_RANGE(0,256), "0.0", M4_DIGIT(2))
    float             Frm2Luma;

    // M4_STRUCT_DESC("HdrAeRange", "normal_ui_style")
    Aec_HdrAeRange_uapi_t HdrAeRange; // result HdrAerange

    // M4_STRUCT_LIST_DESC("HdrExp",  M4_SIZE(1,3), "normal_ui_style")
    RkAiqExpRealParam_t HdrExp[3];
} uapi_hdraeinfo_t;


typedef struct uapi_expinfo_s {
    // M4_BOOL_DESC("IsConverged", "0")
    bool              IsConverged;

    // M4_BOOL_DESC("IsExpMax", "0")
    bool              IsExpMax;

    // M4_BOOL_DESC("EnvChange", "0",M4_HIDE(1))
    bool              EnvChange;

    // M4_STRUCT_DESC("LinAeInfo", "normal_ui_style")
    uapi_linaeinfo_t  LinAeInfo;

    // M4_STRUCT_DESC("HdrAeInfo", "normal_ui_style")
    uapi_hdraeinfo_t  HdrAeInfo;

    // M4_NUMBER_DESC("LinePeriodsPerField", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(2),M4_HIDE(1))
    float             LinePeriodsPerField;

    // M4_NUMBER_DESC("PixelPeriodsPerLine", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(2),M4_HIDE(1))
    float             PixelPeriodsPerLine;

    // M4_NUMBER_DESC("PixelClockFreqMHZ", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(2),M4_HIDE(1))
    float             PixelClockFreqMHZ;

    // M4_NUMBER_DESC("GlobalEnvLv", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(2),M4_HIDE(1))
    float             GlobalEnvLv;

    // M4_NUMBER_DESC("OverExpROIPdf", "f32", M4_RANGE(0,1), "0", M4_DIGIT(2),M4_HIDE(1))
    float             OverExpROIPdf;

    // M4_NUMBER_DESC("HighLightROIPdf", "f32", M4_RANGE(0,1), "0", M4_DIGIT(2),M4_HIDE(1))
    float             HighLightROIPdf;

    // M4_NUMBER_DESC("LowLightROIPdf", "f32", M4_RANGE(0,1), "0", M4_DIGIT(2),M4_HIDE(1))
    float             LowLightROIPdf;

    // M4_NUMBER_DESC("Fps", "f32", M4_RANGE(0,4096), "0", M4_DIGIT(2))
    float             Fps;
} uapi_expinfo_t;

/*****************************************************************************/
/**
 * @brief   ISP AEC HW-Meas Stats Params
 */
/*****************************************************************************/

typedef struct uapi_rawhist_stats_s {
    // M4_ARRAY_DESC("bins", "u32", M4_SIZE(1,256), M4_RANGE(0,268435456), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned int bins[RAWHIST_BIN_N_MAX];
} uapi_rawhist_stats_t;

typedef struct uapi_sihist_stats_s {
    // M4_ARRAY_DESC("bins", "u32", M4_SIZE(1,32), M4_RANGE(0,268435456), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    unsigned int bins[SIHIST_BIN_N_MAX];
} uapi_sihist_stats_t;

typedef struct uapi_rawae_big_stats_s {
    // M4_ARRAY_DESC("channelr_xy", "u16", M4_SIZE(15,15), M4_RANGE(0,1023), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t channelr_xy[RAWAEBIG_WIN_NUM];

    // M4_ARRAY_DESC("channelg_xy", "u16", M4_SIZE(15,15), M4_RANGE(0,4095), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t channelg_xy[RAWAEBIG_WIN_NUM];

    // M4_ARRAY_DESC("channelb_xy", "u16", M4_SIZE(15,15), M4_RANGE(0,1023), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t channelb_xy[RAWAEBIG_WIN_NUM];

    // M4_ARRAY_DESC("channely_xy", "u16", M4_SIZE(15,15), M4_RANGE(0,4095), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t channely_xy[RAWAEBIG_WIN_NUM]; //not HW!

    // M4_ARRAY_DESC("wndx_channelr", "u16", M4_SIZE(1,4), M4_RANGE(0,1023), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint16_t wndx_channelr[RAWAEBIG_SUBWIN_NUM]; //not HW!

    // M4_ARRAY_DESC("wndx_channelg", "u16", M4_SIZE(1,4), M4_RANGE(0,4095), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint16_t wndx_channelg[RAWAEBIG_SUBWIN_NUM]; //not HW!

    // M4_ARRAY_DESC("wndx_channelb", "u16", M4_SIZE(1,4), M4_RANGE(0,1023), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint16_t wndx_channelb[RAWAEBIG_SUBWIN_NUM]; //not HW!

    // M4_ARRAY_DESC("wndx_channely", "u16", M4_SIZE(1,4), M4_RANGE(0,4095), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint16_t wndx_channely[RAWAEBIG_SUBWIN_NUM]; //not HW!

    // M4_ARRAY_DESC("wndx_sumr", "u32", M4_SIZE(1,4), M4_RANGE(0,536870912), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint64_t wndx_sumr[RAWAEBIG_SUBWIN_NUM];

    // M4_ARRAY_DESC("wndx_sumg", "u32", M4_SIZE(1,4), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint64_t wndx_sumg[RAWAEBIG_SUBWIN_NUM];

    // M4_ARRAY_DESC("wndx_sumb", "u32", M4_SIZE(1,4), M4_RANGE(0,536870912), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint64_t wndx_sumb[RAWAEBIG_SUBWIN_NUM];
} uapi_rawae_big_stats_t;

typedef struct uapi_rawae_lite_stat_s {
    // M4_ARRAY_DESC("channelr_xy", "u16", M4_SIZE(5,5), M4_RANGE(0,1023), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t channelr_xy[RAWAELITE_WIN_NUM];

    // M4_ARRAY_DESC("channelg_xy", "u16", M4_SIZE(5,5), M4_RANGE(0,4095), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t channelg_xy[RAWAELITE_WIN_NUM];

    // M4_ARRAY_DESC("channelb_xy", "u16", M4_SIZE(5,5), M4_RANGE(0,1023), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t channelb_xy[RAWAELITE_WIN_NUM];

    // M4_ARRAY_DESC("channely_xy", "u16", M4_SIZE(5,5), M4_RANGE(0,4095), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t channely_xy[RAWAELITE_WIN_NUM]; //not HW!
} uapi_rawae_lite_stat_t;

typedef struct uapi_yuvae_stats_s {
    // M4_ARRAY_DESC("channely_xy", "u8", M4_SIZE(15,15), M4_RANGE(0,255), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    unsigned char mean[YUVAE_WIN_NUM];

    // M4_ARRAY_DESC("channely_xy", "u64", M4_SIZE(1,4), M4_RANGE(0,4294967296), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint64_t ro_yuvae_sumy[YUVAE_SUBWIN_NUM];
} uapi_yuvae_stats_t;

typedef struct uapi_raw_stats_s {
    //rawae
    // M4_STRUCT_DESC("rawae_big", "normal_ui_style")
    uapi_rawae_big_stats_t rawae_big;
    // M4_STRUCT_DESC("rawae_lite", "normal_ui_style")
    uapi_rawae_lite_stat_t rawae_lite;
    //rawhist
    // M4_STRUCT_DESC("rawhist_big", "normal_ui_style")
    uapi_rawhist_stats_t rawhist_big;
    // M4_STRUCT_DESC("rawhist_lite", "normal_ui_style")
    uapi_rawhist_stats_t rawhist_lite;
} uapi_raw_stats_t;

typedef struct uapi_ae_hwstats_s {
    // M4_STRUCT_LIST_DESC("chn", M4_SIZE(1,3), "normal_ui_style")
    uapi_raw_stats_t chn[3];
    // M4_STRUCT_DESC("extra", "normal_ui_style")
    uapi_raw_stats_t extra;
#if ISP_HW_V20 || ISP_HW_V21
    // M4_STRUCT_DESC("yuvae", "normal_ui_style",M4_HIDE(1))
    uapi_yuvae_stats_t yuvae;
    // M4_STRUCT_DESC("sihist", "normal_ui_style",M4_HIDE(1))
    uapi_sihist_stats_t sihist;
#endif
    // M4_ARRAY_DESC("raw_mean", "u16", M4_SIZE(1,4), M4_RANGE(0,256), "0", M4_DIGIT(0), M4_DYNAMIC(0),M4_HIDE(1))
    uint16_t raw_mean[4];  //not HW! The last 8bits are decimal places, raw_mean[3] = extra-chn-mean
} uapi_ae_hwstats_t;

typedef struct uapi_aeStats_mainWinStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_meanBayerR_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,225),
        M4_RANGE_EX(0,0xfff),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of R channel in ae main roi))  */
    uint16_t hw_ae_meanBayerR_val[RAWAEBIG_WIN_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_meanBayerGrGb_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,225),
        M4_RANGE_EX(0,0xfff),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of G channel in ae main roi))  */
    uint16_t hw_ae_meanBayerGrGb_val[RAWAEBIG_WIN_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_meanBayerB_val),
        M4_TYPE(u16),
        M4_SIZE_EX(1,225),
        M4_RANGE_EX(0,0xfff),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of G channel in ae main roi))  */
    uint16_t hw_ae_meanBayerB_val[RAWAEBIG_WIN_NUM];
} uapi_aeStats_mainWinStats_t;

typedef struct uapi_aeStats_subWinStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_meanBayerR_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0xfff),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of R channel in ae main roi))  */
    uint32_t hw_ae_sumBayerR_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_sumBayerGrGb_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0xfff),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of G channel in ae main roi))  */
    uint32_t hw_ae_sumBayerGrGb_val;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_sumBayerB_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,0xfff),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(meanluma of B channel in ae main roi))  */
    uint32_t hw_ae_sumBayerB_val;
} uapi_aeStats_subWinStats_t;

typedef struct uapi_aeStats_histStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ae_histBin_val),
        M4_TYPE(u32),
        M4_SIZE_EX(1,256),
        M4_RANGE_EX(0,0xffffffff),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(1),
        M4_ORDER(0),
        M4_NOTES(histogram stats))  */
    unsigned int hw_ae_histBin_val[RAWHIST_BIN_N_MAX];
} uapi_aeStats_histStats_t;

typedef struct uapi_aeStats_entityStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(mainWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(.....))  */
    uapi_aeStats_mainWinStats_t mainWin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(subWin),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(.....))  */
    uapi_aeStats_subWinStats_t subWin[RAWAEBIG_SUBWIN_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hist),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(.....))  */
    uapi_aeStats_histStats_t hist;
} uapi_aeStats_entityStats_t;

typedef struct uapi_aeStats_entitiesStats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(entity0),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(It is only valid in aeStats_entity03_indWk_mode | aeStats_entity0_wkOnly_mode))  */
    uapi_aeStats_entityStats_t entity0;
    /* M4_GENERIC_DESC(
         M4_ALIAS(entity3),
         M4_TYPE(struct),
         M4_UI_MODULE(normal_ui_style),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(22),
         M4_NOTES(It is only valid in aeStats_entity03_indWk_mode mode | aeStats_entity3_wkOnly_mode))  */
    uapi_aeStats_entityStats_t entity3;
} uapi_aeStats_entitiesStats_t;

typedef struct uapi_aeStats_entityGrpStats_s {
    /* M4_GENERIC_DESC(
         M4_ALIAS(coWkEnt03),
         M4_TYPE(struct),
         M4_UI_MODULE(normal_ui_style),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(22),
         M4_NOTES(It is only valid in aeStats_entity03_coWk_mode mode))  */
    uapi_aeStats_entitiesStats_t coWkEnt03;
    /* M4_GENERIC_DESC(
         M4_ALIAS(entities),
         M4_TYPE(struct),
         M4_UI_MODULE(normal_ui_style),
         M4_HIDE_EX(0),
         M4_RO(0),
         M4_ORDER(22),
         M4_NOTES(It is only valid in aeStats_entity0_wkOnly_mode | aeStats_entity3_wkOnly_mode | aeStats_entity03_indWk_mode mode))  */
    uapi_aeStats_entitiesStats_t entities;
} uapi_aeStats_entityGrpStats_t;

typedef enum uapi_entityGroupWk_mode_e {
    entity03_coWk_mode = 0,
    entity03_indWk_mode = 1,
    entity0_wkOnly_mode = 2,
    entity3_wkOnly_mode = 3,
} uapi_entityGroupWk_mode_t;

typedef struct uapi_ae_v39_hwstats_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_aeCfg_entityGroup_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(uapi_entityGroupWk_mode_t),
        M4_DEFAULT(entity03_indWk_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(..... Reference enum types.\n
        Freq of use: high))  */
    uapi_entityGroupWk_mode_t hw_ae_entityGroup_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(entityGroup),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(...))  */
    uapi_aeStats_entityGrpStats_t entityGroup;
} uapi_ae_v39_hwstats_t;

#endif /*__AEC_UAPI_HEAD_H__*/
