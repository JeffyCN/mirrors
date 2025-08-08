/*
 * af_head.h
 *
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

#ifndef __CALIBDBV2_AF_HEADER_H__
#define __CALIBDBV2_AF_HEADER_H__

#include "common/rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

#define CALIBDBV2_MAX_ISO_LEVEL                    13
#define CALIBDBV2_ZOOM_FOCUS_POSITION_NUM          32
#define CALIBDBV2_ZOOM_FOCUS_ZOOM_MOVE_TBL_SIZE    32
#define CALIBDBV2_AF_CORRECT_SEARCHTBL_MAX         100
#define CALIBDBV2_MAX_MEAS_CONFIG_NUM              32

typedef enum CalibDbV2_Af_SearchDir_s {
    CalibDbV2_AF_POSITIVE_SEARCH     = 0,
    CalibDbV2_AF_NEGATIVE_SEARCH     = 1,
    CalibDbV2_AF_ADAPTIVE_SEARCH     = 2
} CalibDbV2_Af_SearchDir_t;

typedef enum CalibDbV2_Af_SS_s {
    CalibDbV2_AFSS_FULLRANGE       = 0,    /**< scan the full focus range to find the point of best focus */
    CalibDbV2_AFSS_ADAPTIVE_RANGE  = 1,    /**< similar to full range search, but with multiple subsequent scans
                                                with decreasing range and step size will be performed. */
    CalibDbV2_AFSS_ZOOM_FIXSTEP    = 2,
    CalibDbV2_AFSS_ZOOM_VARSTEP    = 3,
} CalibDbV2_Af_SS_t;

typedef enum CalibDbV2_AF_MODE_s
{
    CalibDbV2_AFMODE_NOT_SET = -1,
    CalibDbV2_AFMODE_AUTO,
    CalibDbV2_AFMODE_MACRO,
    CalibDbV2_AFMODE_INFINITY,
    CalibDbV2_AFMODE_FIXED,
    CalibDbV2_AFMODE_EDOF,
    CalibDbV2_AFMODE_CONT_VIDEO,
    CalibDbV2_AFMODE_CONT_PICTURE,
    CalibDbV2_AFMODE_ZOOM_ONESHOT,
} CalibDbV2_AF_MODE_t;

typedef enum CalibDbV2_PDAF_DATA_DIR_s {
    CalibDbV2_PDAF_DIR_INVAL = 0,
    CalibDbV2_PDAF_DIR_LEFT  = 1,
    CalibDbV2_PDAF_DIR_RIGHT = 2,
    CalibDbV2_PDAF_DIR_TOP   = 3,
    CalibDbV2_PDAF_DIR_DOWN  = 4
} CalibDbV2_PDAF_DATA_DIR_t;

typedef enum {
    CalibDbV2_PDAF_SENSOR_TYPE1,
    CalibDbV2_PDAF_SENSOR_TYPE2,
    CalibDbV2_PDAF_SENSOR_TYPE3
} CalibDbV2_PDAF_SENSOR_TYPE_t;

typedef struct CalibDbV2_Af_VarStepCfg_s {
    // M4_NUMBER_DESC("ZoomIdx", "u32", M4_RANGE(0,100000), "0", M4_DIGIT(0), M4_HIDE(0))
    unsigned short          ZoomIdx;
    // M4_ARRAY_DESC("Iso", "s32", M4_SIZE(1,13), M4_RANGE(0,1000000), "[50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]", M4_DIGIT(0), M4_DYNAMIC(0))
    int                     Iso[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_ARRAY_DESC("SearchStep", "u16", M4_SIZE(1,13), M4_RANGE(0,255), "1", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          SearchStep[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_ARRAY_DESC("Stage1SkipThers", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "0.02", M4_DIGIT(3), M4_DYNAMIC(0))
    float                   Stage1SkipThers[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_ARRAY_DESC("Stage2QuickEndThers", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "0.02", M4_DIGIT(3), M4_DYNAMIC(0))
    float                   Stage2QuickEndThers[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_ARRAY_DESC("SkipStepCnt", "u16", M4_SIZE(1,13), M4_RANGE(0,255), "4", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          SkipStepCnt[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_ARRAY_DESC("SkipStepThers", "f32", M4_SIZE(1,13), M4_RANGE(0,1), "0.02", M4_DIGIT(3), M4_DYNAMIC(0))
    float                   SkipStepThers[CALIBDBV2_MAX_ISO_LEVEL];
} CalibDbV2_Af_VarStepCfg_t;

typedef struct CalibDbV2_Af_ContrastZoom_s {
    // M4_ARRAY_DESC("QuickFoundThersZoomIdx", "u16", M4_SIZE(1,32), M4_RANGE(0,65535), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          QuickFoundThersZoomIdx[32];
    // M4_NUMBER_DESC("QuickFoundThersZoomIdx_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                     QuickFoundThersZoomIdx_len;
    // M4_ARRAY_DESC("QuickFoundThers", "f32", M4_SIZE(1,32), M4_RANGE(0,1), "0.2", M4_DIGIT(3), M4_DYNAMIC(0))
    float                   QuickFoundThers[32];
    // M4_NUMBER_DESC("QuickFoundThers_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                     QuickFoundThers_len;
    // M4_ARRAY_DESC("SearchStepZoomIdx", "u16", M4_SIZE(1,32), M4_RANGE(0,65535), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          SearchStepZoomIdx[32];
    // M4_NUMBER_DESC("SearchStepZoomIdx_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                     SearchStepZoomIdx_len;
    // M4_ARRAY_DESC("SearchStep", "u16", M4_SIZE(1,32), M4_RANGE(0,65535), "16", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          SearchStep[32];
    // M4_NUMBER_DESC("SearchStep_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                     SearchStep_len;
    // M4_ARRAY_DESC("StopStepZoomIdx", "u16", M4_SIZE(1,32), M4_RANGE(0,65535), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          StopStepZoomIdx[32];
    // M4_NUMBER_DESC("StopStepZoomIdx_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                     StopStepZoomIdx_len;
    // M4_ARRAY_DESC("StopStep", "u16", M4_SIZE(1,32), M4_RANGE(0,65535), "4", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          StopStep[32];
    // M4_NUMBER_DESC("StopStep_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                     StopStep_len;
    // M4_NUMBER_DESC("SkipHighPassZoomIdx", "u16", M4_RANGE(0, 65535), "10000", M4_DIGIT(0))
    unsigned short          SkipHighPassZoomIdx;
    // M4_NUMBER_DESC("SkipHighPassGain", "f32", M4_RANGE(0, 1000), "1000", M4_DIGIT(3))
    float                   SkipHighPassGain;
    // M4_NUMBER_DESC("SwitchDirZoomIdx", "u16", M4_RANGE(0, 65535), "0", M4_DIGIT(0))
    unsigned short          SwitchDirZoomIdx;

    // M4_NUMBER_DESC("Spotlight HighlightRatio", "f32", M4_RANGE(0, 1), "0.014", M4_DIGIT(3))
    float                   SpotlightHighlightRatio;
    // M4_ARRAY_DESC("Spotlight LumaRatio", "f32", M4_SIZE(1,3), M4_RANGE(0,1), "[0.3, 0.5, 0.8]", M4_DIGIT(3), M4_DYNAMIC(0))
    float                   SpotlightLumaRatio[3];
    // M4_ARRAY_DESC("Spotlight BlkCnt", "f32", M4_SIZE(1,3), M4_RANGE(0,1), "[0.2, 0.5, 0.25]", M4_DIGIT(3), M4_DYNAMIC(0))
    float                   SpotlightBlkCnt[3];

    // M4_STRUCT_LIST_DESC("VarStepCfg", M4_SIZE(1,10), "normal_ui_style")
    CalibDbV2_Af_VarStepCfg_t VarStepCfg[10];
    // M4_NUMBER_DESC("VarStepCfg_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                       VarStepCfg_len;
} CalibDbV2_Af_ContrastZoom_t;

typedef struct CalibDbV2_Af_Contrast_s {
    // M4_BOOL_DESC("enable", "1")
    bool enable;
    // M4_ENUM_DESC("SearchStrategy", "CalibDbV2_Af_SS_t", "CalibDbV2_AFSS_ADAPTIVE_RANGE")
    CalibDbV2_Af_SS_t         Afss;                       /**< enumeration type for search strategy */
    // M4_ENUM_DESC("FullDir", "CalibDbV2_Af_SearchDir_t", "CalibDbV2_AF_ADAPTIVE_SEARCH")
    CalibDbV2_Af_SearchDir_t  FullDir;
    // M4_ARRAY_DESC("FullRangeTbl", "s16", M4_SIZE(1,3), M4_RANGE(-1023,1023), "[0,1,64]", M4_DIGIT(0), M4_DYNAMIC(0))
    short                   FullRangeTbl[3];
    // M4_ENUM_DESC("AdaptiveDir", "CalibDbV2_Af_SearchDir_t", "CalibDbV2_AF_ADAPTIVE_SEARCH")
    CalibDbV2_Af_SearchDir_t  AdaptiveDir;
    // M4_ARRAY_DESC("AdaptRangeTbl", "u16", M4_SIZE(1,65), M4_RANGE(0,1023), "[0,8,16,24,32,40,48,56,64]", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          AdaptRangeTbl[65];            /**< adaptive range search table*/
    // M4_NUMBER_DESC("AdaptRangeTbl_len", "s32", M4_RANGE(1,65), "9", M4_DIGIT(0), M4_HIDE(0))
    int                     AdaptRangeTbl_len;
    // M4_ARRAY_DESC("FineSearchStep", "u16", M4_SIZE(1,65), M4_RANGE(0,1023), "[4,4,4,4,4,4,4,4,4]", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          FineSearchStep[65];
    // M4_NUMBER_DESC("FineSearchStep_len", "s32", M4_RANGE(1,65), "9", M4_DIGIT(0), M4_HIDE(0))
    int                     FineSearchStep_len;
    // M4_NUMBER_DESC("SkipCurveFitGain", "f32", M4_RANGE(0, 1000), "0", M4_DIGIT(3))
    float                   SkipCurveFitGain;
    // M4_ARRAY_DESC("TrigThers", "f32", M4_SIZE(1,32), M4_RANGE(0,1), "0.075", M4_DIGIT(3), M4_DYNAMIC(0))
    float                   TrigThers[32];                    /**< AF trigger threshold */
    // M4_NUMBER_DESC("TrigThers_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                     TrigThers_len;
    // M4_ARRAY_DESC("TrigThersFv", "f32", M4_SIZE(1,32), M4_RANGE(0,100000000000), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    float                   TrigThersFv[32];                    /**< AF trigger threshold */
    // M4_NUMBER_DESC("TrigThersFv_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int                     TrigThersFv_len;
    // M4_NUMBER_DESC("LumaTrigThers", "f32", M4_RANGE(0,1), "1", M4_DIGIT(3),M4_HIDE(0))
    float                   LumaTrigThers;
    // M4_NUMBER_DESC("ExpTrigThers", "f32", M4_RANGE(0,1), "1", M4_DIGIT(3),M4_HIDE(0))
    float                   ExpTrigThers;
    // M4_NUMBER_DESC("ChangedFrames", "u16", M4_RANGE(0,1000), "10", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          ChangedFrames;

    // M4_NUMBER_DESC("StableThers", "f32", M4_RANGE(0,1), "0.02", M4_DIGIT(3),M4_HIDE(0))
    float                   StableThers;                  /**< AF stable threshold */
    // M4_NUMBER_DESC("StableFrames", "u16", M4_RANGE(0,1023), "3", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          StableFrames;                 /**< AF stable  status must hold frames */
    // M4_NUMBER_DESC("StableTime", "u16", M4_RANGE(0,65535), "200", M4_DIGIT(3),M4_HIDE(0))
    unsigned short          StableTime;                   /**< AF stable status must hold time */

    // M4_NUMBER_DESC("SceneDiffEnable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned char           SceneDiffEnable;
    // M4_NUMBER_DESC("SceneDiffThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   SceneDiffThers;
    // M4_NUMBER_DESC("SceneDiffBlkThers", "u16", M4_RANGE(0,225), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          SceneDiffBlkThers;
    // M4_NUMBER_DESC("CenterSceneDiffThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   CenterSceneDiffThers;

    // M4_NUMBER_DESC("ValidMaxMinRatio", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   ValidMaxMinRatio;
    // M4_NUMBER_DESC("ValidValueThers", "f32", M4_RANGE(0,1000000), "0", M4_DIGIT(0),M4_HIDE(0))
    float                   ValidValueThers;

    // M4_NUMBER_DESC("OutFocusValue", "f32", M4_RANGE(0,1000000), "50", M4_DIGIT(3),M4_HIDE(0))
    float                   OutFocusValue;                /**< out of focus vlaue*/
    // M4_NUMBER_DESC("OutFocusPos", "u16", M4_RANGE(0,1023), "64", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          OutFocusPos;                  /**< out of focus position*/

    // M4_NUMBER_DESC("LowLumaValue", "f32", M4_RANGE(0,4095), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   LowLumaValue;                /**< low luma thr vlaue*/
    // M4_NUMBER_DESC("LowLumaPos", "u16", M4_RANGE(0,1023), "64", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          LowLumaPos;                  /**< low luma focus position*/

    // M4_NUMBER_DESC("WeightEnable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned char           WeightEnable;
    // M4_ARRAY_DESC("Weight", "u16", M4_SIZE(15,15), M4_RANGE(0,65535), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short          Weight[225];                  /**< weight */

    // M4_NUMBER_DESC("SearchPauseLumaEnable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned char           SearchPauseLumaEnable;
    // M4_NUMBER_DESC("SearchPauseLumaThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   SearchPauseLumaThers;
    // M4_NUMBER_DESC("SearchLumaStableFrames", "u16", M4_RANGE(0,1023), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          SearchLumaStableFrames;
    // M4_NUMBER_DESC("SearchLumaStableThers", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   SearchLumaStableThers;

    // M4_NUMBER_DESC("Stage1QuickFoundThers", "f32", M4_RANGE(0,1), "0.01", M4_DIGIT(3),M4_HIDE(0))
    float                   Stage1QuickFoundThers;
    // M4_NUMBER_DESC("Stage2QuickFoundThers", "f32", M4_RANGE(0,1), "0.2", M4_DIGIT(3),M4_HIDE(0))
    float                   Stage2QuickFoundThers;

    // M4_NUMBER_DESC("FlatValue", "f32", M4_RANGE(0,65535), "0", M4_DIGIT(3),M4_HIDE(0))
    float                   FlatValue;

    // M4_NUMBER_DESC("PointLightLumaTh", "u16", M4_RANGE(0,65535), "4095", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          PointLightLumaTh;                 /**< point light luma threshold */
    // M4_NUMBER_DESC("PointLightCntTh", "u16", M4_RANGE(0,65535), "225", M4_DIGIT(0),M4_HIDE(0))
    unsigned short          PointLightCntTh;                  /**< point light count threshold */

    // M4_STRUCT_DESC("ZoomConfig", "normal_ui_style")
    CalibDbV2_Af_ContrastZoom_t ZoomCfg;
} CalibDbV2_Af_Contrast_t;

typedef struct CalibDbV2_Af_Laser_s {
    // M4_BOOL_DESC("enable", "0")
    bool enable;
    // M4_ARRAY_DESC("vcmDot", "f32", M4_SIZE(1,7), M4_RANGE(0,64), "[0, 16, 32, 40, 48, 56, 64]", M4_DIGIT(0), M4_DYNAMIC(0))
    float vcmDot[7];
    // M4_ARRAY_DESC("distanceDot", "f32", M4_SIZE(1,7), M4_RANGE(0,65535), "[0.2, 0.24, 0.34, 0.4, 0.66, 1, 3]", M4_DIGIT(4), M4_DYNAMIC(0))
    float distanceDot[7];
} CalibDbV2_Af_Laser_t;

typedef struct CalibDbV2_Af_Pdaf_fineSearch_s {
    // M4_NUMBER_DESC("confidence", "f32", M4_RANGE(0,1), "0.5", M4_DIGIT(3),M4_HIDE(0))
    float confidence;
    // M4_NUMBER_DESC("range", "s32", M4_RANGE(0,1023), "0", M4_DIGIT(0))
    int range;
    // M4_NUMBER_DESC("stepPos", "s32", M4_RANGE(1,1023), "1", M4_DIGIT(0))
    int stepPos;
} CalibDbV2_Af_Pdaf_fineSearch_t;

typedef struct CalibDbV2_Af_PdafIsoPara_s {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 1000000), "50", M4_DIGIT(0))
    int iso;
    // M4_NUMBER_DESC("pdConfdRatio0", "f32", M4_RANGE(0,256), "1", M4_DIGIT(3),M4_HIDE(0))
    float pdConfdRatio0;
    // M4_NUMBER_DESC("pdConfdRatio1", "f32", M4_RANGE(0,256), "1", M4_DIGIT(3),M4_HIDE(0))
    float pdConfdRatio1;
    // M4_NUMBER_DESC("pdConfdRatio2", "f32", M4_RANGE(0,256), "1", M4_DIGIT(3),M4_HIDE(0))
    float pdConfdRatio2;
    // M4_NUMBER_DESC("pdConfdRatio3", "f32", M4_RANGE(0,256), "1", M4_DIGIT(3),M4_HIDE(0))
    float pdConfdRatio3;
    // M4_NUMBER_DESC("pdConfdRhresh", "f32", M4_RANGE(0,1), "0.4", M4_DIGIT(3),M4_HIDE(0))
    float pdConfdThresh;
    // M4_NUMBER_DESC("convergedInfPdThresh", "u16", M4_RANGE(0,1023), "12", M4_DIGIT(0),M4_HIDE(0))
    unsigned short convergedInfPdThresh;
    // M4_NUMBER_DESC("convergedMacPdThresh", "u16", M4_RANGE(0,1023), "12", M4_DIGIT(0),M4_HIDE(0))
    unsigned short convergedMacPdThresh;
    // M4_NUMBER_DESC("defocusInfPdThresh", "u16", M4_RANGE(0,1023), "12", M4_DIGIT(0),M4_HIDE(0))
    unsigned short defocusInfPdThresh;
    // M4_NUMBER_DESC("defocusMacPdThresh", "u16", M4_RANGE(0,1023), "12", M4_DIGIT(0),M4_HIDE(0))
    unsigned short defocusMacPdThresh;
    // M4_NUMBER_DESC("stablePdRatio", "f32", M4_RANGE(0,255), "0.125", M4_DIGIT(3),M4_HIDE(0))
    float stablePdRatio;
    // M4_NUMBER_DESC("stablePdOffset", "u16", M4_RANGE(0,1023), "8", M4_DIGIT(3),M4_HIDE(0))
    unsigned short stablePdOffset;
    // M4_NUMBER_DESC("stableCntRatio", "f32", M4_RANGE(0,1023), "1.5", M4_DIGIT(3),M4_HIDE(0))
    float stableCntRatio;
    // M4_NUMBER_DESC("noconfCntThresh", "u16", M4_RANGE(0,255), "4", M4_DIGIT(0),M4_HIDE(0))
    unsigned short noconfCntThresh;
    // M4_NUMBER_DESC("roiBlkCntW", "u8", M4_RANGE(1, 10), "3", M4_DIGIT(0))
    unsigned char roiBlkCntW;
    // M4_NUMBER_DESC("roiBlkCntH", "u8", M4_RANGE(1, 10), "3", M4_DIGIT(0))
    unsigned char roiBlkCntH;
    // M4_STRUCT_LIST_DESC("fineSearchTbl", M4_SIZE(1,10), "normal_ui_style")
    CalibDbV2_Af_Pdaf_fineSearch_t fineSearchTbl[10];
    // M4_NUMBER_DESC("fineSearchTbl_len", "s32", M4_RANGE(1,10), "1", M4_DIGIT(0), M4_HIDE(0))
    int fineSearchTbl_len;
} CalibDbV2_Af_PdafIsoPara_t;

typedef struct CalibDbV2_Af_PdafCalibInf_s {
    // M4_NUMBER_DESC("pdGainMapW", "u16", M4_RANGE(1,1000), "32", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdGainMapW;
    // M4_NUMBER_DESC("pdGainMapH", "u16", M4_RANGE(1,1000), "32", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdGainMapH;
    // M4_NUMBER_DESC("pdDccMapW", "u16", M4_RANGE(1,1000), "32", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdDccMapW;
    // M4_NUMBER_DESC("pdDccMapH", "u16", M4_RANGE(1,1000), "32", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdDccMapH;
} CalibDbV2_Af_PdafCalibInf_t;

typedef struct CalibDbV2_Af_PdafResolution_s {
    // M4_ENUM_DESC("pdType", "CalibDbV2_PDAF_SENSOR_TYPE_t", "CalibDbV2_PDAF_SENSOR_TYPE2")
    CalibDbV2_PDAF_SENSOR_TYPE_t pdType;
    // M4_NUMBER_DESC("imageWidth", "u16", M4_RANGE(0, 65535), "508", M4_DIGIT(0))
    unsigned short imageWidth;
    // M4_NUMBER_DESC("imageHeight", "u16", M4_RANGE(0, 65535), "760", M4_DIGIT(0))
    unsigned short imageHeight;
    // M4_NUMBER_DESC("pdOutWidth", "u16", M4_RANGE(0, 65535), "508", M4_DIGIT(0))
    unsigned short pdOutWidth;
    // M4_NUMBER_DESC("pdOutHeight", "u16", M4_RANGE(0, 65535), "760", M4_DIGIT(0))
    unsigned short pdOutHeight;
    // M4_NUMBER_DESC("pdCropX", "u16", M4_RANGE(0, 65535), "508", M4_DIGIT(0))
    unsigned short pdCropX;
    // M4_NUMBER_DESC("pdCropY", "u16", M4_RANGE(0, 65535), "760", M4_DIGIT(0))
    unsigned short pdCropY;
    // M4_NUMBER_DESC("pdBaseWidth", "u16", M4_RANGE(0, 65535), "508", M4_DIGIT(0))
    unsigned short pdBaseWidth;
    // M4_NUMBER_DESC("pdBaseHeight", "u16", M4_RANGE(0, 65535), "760", M4_DIGIT(0))
    unsigned short pdBaseHeight;
    // M4_NUMBER_DESC("pdHBinMode", "u16", M4_RANGE(0, 4), "0", M4_DIGIT(0))
    unsigned short pdHBinMode;
    // M4_NUMBER_DESC("pdVBinMode", "u16", M4_RANGE(0, 4), "0", M4_DIGIT(0))
    unsigned short pdVBinMode;
} CalibDbV2_Af_PdafResolution_t;

typedef struct CalibDbV2_Af_Pdaf_s {
    // M4_BOOL_DESC("enable", "0")
    bool enable;
    // M4_NUMBER_DESC("pdVsCdDebug", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char pdVsCdDebug;
    // M4_NUMBER_DESC("pdDumpDebug", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char pdDumpDebug;
    // M4_NUMBER_DESC("pdDumpMaxFrm", "u16", M4_RANGE(1,10000), "300", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdDumpMaxFrm;
    // M4_NUMBER_DESC("pdDataBit", "u16", M4_RANGE(1,16), "10", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdDataBit;
    // M4_NUMBER_DESC("pdBlkLevel", "u16", M4_RANGE(1,1023), "64", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdBlkLevel;
    // M4_NUMBER_DESC("pdSearchRadius", "u16", M4_RANGE(0,32), "3", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdSearchRadius;
    // M4_NUMBER_DESC("pdMirrorInCalib", "u8", M4_RANGE(0, 3), "1", M4_DIGIT(0))
    unsigned char pdMirrorInCalib;
    // M4_NUMBER_DESC("pdChangeLeftRight", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
    unsigned char pdChangeLeftRight;
    // M4_NUMBER_DESC("pdVsImgoutMirror", "u8", M4_RANGE(0, 3), "0", M4_DIGIT(0))
    unsigned char pdVsImgoutMirror;
    // M4_NUMBER_DESC("pdLRInDiffLine", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char pdLRInDiffLine;
    // M4_NUMBER_DESC("pdWidth", "u16", M4_RANGE(0, 65535), "508", M4_DIGIT(0))
    unsigned short pdWidth;
    // M4_NUMBER_DESC("pdHeight", "u16", M4_RANGE(0, 65535), "760", M4_DIGIT(0))
    unsigned short pdHeight;
    // M4_NUMBER_DESC("pdMaxWidth", "u16", M4_RANGE(0, 65535), "508", M4_DIGIT(0))
    unsigned short pdMaxWidth;
    // M4_NUMBER_DESC("pdMaxHeight", "u16", M4_RANGE(0, 65535), "760", M4_DIGIT(0))
    unsigned short pdMaxHeight;
    // M4_NUMBER_DESC("pdVerBinning", "u8", M4_RANGE(0, 255), "0", M4_DIGIT(0))
    unsigned char pdVerBinning;
    // M4_NUMBER_DESC("pdFrmInValid", "u8", M4_RANGE(0, 4), "1", M4_DIGIT(0))
    unsigned char pdFrmInValid;
    // M4_NUMBER_DESC("pdDgainValid", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char pdDgainValid;
    // M4_NUMBER_DESC("pdGainMapNormEn", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char pdGainMapNormEn;
    // M4_NUMBER_DESC("pdConfdMode", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char pdConfdMode;
    // M4_NUMBER_DESC("pdNoiseSn", "f32", M4_RANGE(0,100), "0.207", M4_DIGIT(3),M4_HIDE(0))
    float pdNoiseSn;
    // M4_ARRAY_DESC("pdNoiseRn", "f32", M4_SIZE(1,2), M4_RANGE(0,100), "[0.14, 2.8]", M4_DIGIT(3), M4_DYNAMIC(0))
    float pdNoiseRn[2];
    // M4_NUMBER_DESC("pdNoisePn", "f32", M4_RANGE(0,100), "0.25", M4_DIGIT(3),M4_HIDE(0))
    float pdNoisePn;
    // M4_NUMBER_DESC("pdSatValRatio", "f32", M4_RANGE(0,1), "0.92", M4_DIGIT(3),M4_HIDE(0))
    float pdSatValRatio;
    // M4_NUMBER_DESC("pdSatCntRatio", "f32", M4_RANGE(0,1), "0.06", M4_DIGIT(3),M4_HIDE(0))
    float pdSatCntRatio;
    // M4_NUMBER_DESC("pdDiscardRegionEn", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char pdDiscardRegionEn;
    // M4_NUMBER_DESC("pdLessTextureRatio", "f32", M4_RANGE(0,5), "0.25", M4_DIGIT(3),M4_HIDE(0))
    float pdLessTextureRatio;
    // M4_NUMBER_DESC("pdTargetOffset", "s16", M4_RANGE(-4096,4096), "0", M4_DIGIT(0),M4_HIDE(0))
    short pdTargetOffset;
    // M4_NUMBER_DESC("pdNumJobs", "u16", M4_RANGE(0,64), "1", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdNumJobs;
    // M4_STRUCT_DESC("pdCalibInf", "normal_ui_style")
    CalibDbV2_Af_PdafCalibInf_t pdCalibInf;
    // M4_NUMBER_DESC("pdConfdMwinFactor", "u16", M4_RANGE(0,225), "3", M4_DIGIT(0),M4_HIDE(0))
    unsigned short pdConfdMwinFactor;
    // M4_NUMBER_DESC("pdCenterMinFv", "u32", M4_RANGE(0,100000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned int pdCenterMinFv;
    // M4_NUMBER_DESC("pdCenterMinRatio", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3),M4_HIDE(0))
    float pdCenterMinRatio;
    // M4_NUMBER_DESC("pdHighlightRatio", "f32", M4_RANGE(0,1), "0.1", M4_DIGIT(3),M4_HIDE(0))
    float pdHighlightRatio;
    // M4_ARRAY_DESC("pdStepRatio", "f32", M4_SIZE(1,7), M4_RANGE(0,1), "[1.0, 1.0, 1.0, 0.9, 0.8, 0.7, 0.7]", M4_DIGIT(3), M4_DYNAMIC(0))
    float pdStepRatio[7];
    // M4_ARRAY_DESC("pdStepDefocus", "u16", M4_SIZE(1,7), M4_RANGE(0,1023), "[32, 64, 96, 128, 160, 192, 224]", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned short pdStepDefocus[7];
    // M4_STRUCT_LIST_DESC("pdIsoPara", M4_SIZE(1,13), "normal_ui_style")
    CalibDbV2_Af_PdafIsoPara_t pdIsoPara[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_NUMBER_DESC("pdIsoPara_len", "s32", M4_RANGE(1,16), "1", M4_DIGIT(0), M4_HIDE(0))
    int pdIsoPara_len;
    // M4_STRUCT_LIST_DESC("pdResoInf", M4_SIZE(1,16), "normal_ui_style")
    CalibDbV2_Af_PdafResolution_t pdResoInf[16];
    // M4_NUMBER_DESC("pdResoInf_len", "s32", M4_RANGE(1,16), "1", M4_DIGIT(0), M4_HIDE(0))
    int pdResoInf_len;
} CalibDbV2_Af_Pdaf_t;

typedef struct CalibDbV2_Af_VcmCfg_s {
    // M4_NUMBER_DESC("max logical pos", "u32", M4_RANGE(0,1023), "64", M4_DIGIT(0))
    unsigned int max_logical_pos;
    // M4_NUMBER_DESC("start current", "s32", M4_RANGE(-1, 1023), "-1", M4_DIGIT(0))
    int start_current;
    // M4_NUMBER_DESC("rated current", "s32", M4_RANGE(-1, 1023), "-1", M4_DIGIT(0))
    int rated_current;
    // M4_NUMBER_DESC("step mode", "s32", M4_RANGE(-1, 1000), "-1", M4_DIGIT(0))
    int step_mode;
    // M4_NUMBER_DESC("extra delay", "s32", M4_RANGE(-10000,10000), "0", M4_DIGIT(0))
    int extra_delay;
    // M4_NUMBER_DESC("posture diff", "f32", M4_RANGE(0,1), "0", M4_DIGIT(3))
    float posture_diff;
} CalibDbV2_Af_VcmCfg_t;

typedef struct CalibDbV2_Af_MeasIsoCfg_s {
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 1000000), "0", M4_DIGIT(0))
    int iso;
    // M4_NUMBER_DESC("afmThres", "u16", M4_RANGE(0, 255), "4", M4_DIGIT(0))
    unsigned short afmThres;
    // M4_ARRAY_MARK_DESC("Gamma_curve", "u16", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0,45,108,179,245,344,409,459,500,567,622,676,759,833,896,962,1023]", M4_DIGIT(0), M4_DYNAMIC(0), "curve_table")
    unsigned short gammaY[17];
    // M4_ARRAY_DESC("gaussWeight", "u8", M4_SIZE(1,3), M4_RANGE(0,128), "[32,16,8]", M4_DIGIT(0), M4_DYNAMIC(0))
    unsigned char gaussWeight[3];
} CalibDbV2_Af_MeasIsoCfg_t;

typedef struct CalibDbV2_Af_DefCode_s {
    // M4_NUMBER_DESC("code", "u8", M4_RANGE(0, 64), "32", M4_DIGIT(0))
    unsigned char code;
} CalibDbV2_Af_DefCode_t;

typedef struct CalibDbV2_Af_ZoomFocusTbl_s {
    // M4_NUMBER_DESC("widemod deviate", "u32", M4_RANGE(0, 1000), "0", M4_DIGIT(0))
    int widemod_deviate;
    // M4_NUMBER_DESC("telemod deviate", "u32", M4_RANGE(0, 1000), "0", M4_DIGIT(0))
    int telemod_deviate;
    // M4_NUMBER_DESC("focus backward value", "u32", M4_RANGE(0, 1000), "0", M4_DIGIT(0))
    int focus_backval;
    // M4_ARRAY_DESC("zoom move dot", "u32", M4_SIZE(1,32), M4_RANGE(0,1000000), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    int zoom_move_dot[32];
    // M4_NUMBER_DESC("zoom move step len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int zoom_move_dot_len;
    // M4_ARRAY_DESC("zoom move step", "u32", M4_SIZE(1,32), M4_RANGE(0,1000000), "16", M4_DIGIT(0), M4_DYNAMIC(0))
    int zoom_move_step[32];
    // M4_NUMBER_DESC("zoom move step len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int zoom_move_step_len;
    // M4_ARRAY_DESC("zoom search table", "s32", M4_SIZE(1,32), M4_RANGE(-32768,32767), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    int ZoomSearchTbl[32];
    // M4_NUMBER_DESC("zoom search table number", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int ZoomSearchTbl_len;
    // M4_NUMBER_DESC("zoom search reference curve", "u32", M4_RANGE(0, 32), "0", M4_DIGIT(0))
    int ZoomSearchRefCurveIdx;
    // M4_NUMBER_DESC("zoom search margin", "u32", M4_RANGE(0, 100000), "50", M4_DIGIT(0))
    int FocusSearchMargin;
    // M4_ARRAY_DESC("zoom search plus range", "u32", M4_SIZE(1,32), M4_RANGE(0,32767), "0", M4_DIGIT(0), M4_DYNAMIC(0))
    int FocusSearchPlusRange[32];
    // M4_NUMBER_DESC("FocusSearchPlusRange_len", "s32", M4_RANGE(1,32), "1", M4_DIGIT(0), M4_HIDE(0))
    int FocusSearchPlusRange_len;
    // M4_NUMBER_DESC("focus stage1 step", "u32", M4_RANGE(0, 100), "0", M4_DIGIT(0))
    int FocusStage1Step;
    // M4_NUMBER_DESC("quickFndRate", "f32", M4_RANGE(0,1), "0.2", M4_DIGIT(3),M4_HIDE(0))
    float QuickFndRate;
    // M4_NUMBER_DESC("quickFndMinFv", "f32", M4_RANGE(0,2147483647), "1000", M4_DIGIT(0),M4_HIDE(0))
    float QuickFndMinFv;
    // M4_NUMBER_DESC("search zoom range", "u32", M4_RANGE(0, 100000), "100", M4_DIGIT(0))
    int searchZoomRange;
    // M4_NUMBER_DESC("search focus range", "u32", M4_RANGE(0, 100000), "100", M4_DIGIT(0))
    int searchFocusRange;
    // M4_NUMBER_DESC("search emax", "f32", M4_RANGE(0,100000), "100", M4_DIGIT(3),M4_HIDE(0))
    float searchEmax;
    // M4_NUMBER_DESC("search eavg", "f32", M4_RANGE(0,100000), "100", M4_DIGIT(3),M4_HIDE(0))
    float searchEavg;

    // M4_NUMBER_DESC("IsZoomFocusRec", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char IsZoomFocusRec;
    // M4_STRING_DESC("ZoomInfoDir", M4_SIZE(1,1), M4_RANGE(0, 64), "/data/", M4_DYNAMIC(0))
    char ZoomInfoDir[64];
    // M4_NUMBER_DESC("zoom init Index", "u32", M4_RANGE(0, 100000), "0", M4_DIGIT(0))
    int ZoomInitIndex;
    // M4_STRING_DESC("ZoomCurveFile", M4_SIZE(1,1), M4_RANGE(0, 64), "/data/zoomcurve.bin", M4_DYNAMIC(0))
    char ZoomCurveFile[64];
} CalibDbV2_Af_ZoomFocusTbl_t;

typedef struct CalibDb_Af_LdgParam_s {
    // M4_NUMBER_DESC("enable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char enable;
    // M4_NUMBER_DESC("ldg xl", "u32", M4_RANGE(0, 255), "10", M4_DIGIT(0))
    int ldg_xl;
    // M4_NUMBER_DESC("ldg yl", "u32", M4_RANGE(0, 255), "28", M4_DIGIT(0))
    int ldg_yl;
    // M4_NUMBER_DESC("ldg kl", "u32", M4_RANGE(0, 2147483647), "1286", M4_DIGIT(0))
    int ldg_kl;
    // M4_NUMBER_DESC("ldg xh", "u32", M4_RANGE(0, 255), "118", M4_DIGIT(0))
    int ldg_xh;
    // M4_NUMBER_DESC("ldg yh", "u32", M4_RANGE(0, 255), "8", M4_DIGIT(0))
    int ldg_yh;
    // M4_NUMBER_DESC("ldg kh", "u32", M4_RANGE(0, 2147483647), "1340", M4_DIGIT(0))
    int ldg_kh;
} CalibDbV2_Af_LdgParam_t;

typedef struct CalibDb_Af_HighLightParam_s {
    // M4_NUMBER_DESC("ther0", "u32", M4_RANGE(0, 255), "245", M4_DIGIT(0))
    int ther0;
    // M4_NUMBER_DESC("ther1", "u32", M4_RANGE(0, 255), "200", M4_DIGIT(0))
    int ther1;
} CalibDbV2_Af_HighLightParam_t;

typedef struct CalibDbV2_AF_Tuning_Para_s {
    // M4_ENUM_DESC("mode", "CalibDbV2_AF_MODE_t", "CalibDbV2_AFMODE_CONT_PICTURE")
    CalibDbV2_AF_MODE_t af_mode;
    // M4_NUMBER_DESC("win_h_offs", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_offs;
    // M4_NUMBER_DESC("win_v_offs", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_offs;
    // M4_NUMBER_DESC("win_h_size", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_size;
    // M4_NUMBER_DESC("win_v_size", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_size;
    // M4_NUMBER_DESC("win_h_offs in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_h_offs;
    // M4_NUMBER_DESC("win_v_offs in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_v_offs;
    // M4_NUMBER_DESC("win_h_size in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_h_size;
    // M4_NUMBER_DESC("win_v_size in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_v_size;
    // M4_STRUCT_DESC("fixed mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t fixed_mode;
    // M4_STRUCT_DESC("macro mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t macro_mode;
    // M4_STRUCT_DESC("infinity mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t infinity_mode;
    // M4_STRUCT_DESC("ldg param", "normal_ui_style")
    CalibDbV2_Af_LdgParam_t ldg_param;
    // M4_STRUCT_DESC("high light", "normal_ui_style")
    CalibDbV2_Af_HighLightParam_t highlight;
    // M4_STRUCT_DESC("contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t contrast_af;
    // M4_STRUCT_DESC("video contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t video_contrast_af;
    // M4_STRUCT_DESC("laser af", "normal_ui_style")
    CalibDbV2_Af_Laser_t laser_af;
    // M4_STRUCT_DESC("vcmcfg", "normal_ui_style")
    CalibDbV2_Af_VcmCfg_t vcmcfg;
    // M4_STRUCT_DESC("zoomfocus_tbl", "normal_ui_style")
    CalibDbV2_Af_ZoomFocusTbl_t zoomfocus_tbl;
    // M4_STRUCT_LIST_DESC("meas iso config", M4_SIZE(1,13), "normal_ui_style")
    CalibDbV2_Af_MeasIsoCfg_t measiso_cfg[CALIBDBV2_MAX_ISO_LEVEL];
} CalibDbV2_AF_Tuning_Para_t;

typedef struct {
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_AF_Tuning_Para_t TuningPara;
} CalibDbV2_AF_t;

typedef struct CalibDbV2_AfV30_MeasCfg_s {
    // M4_NUMBER_DESC("table index", "u32", M4_RANGE(0, 1000000), "0", M4_DIGIT(0))
    unsigned int tbl_idx;
    // M4_NUMBER_DESC("afmThres", "u16", M4_RANGE(0, 255), "4", M4_DIGIT(0))
    unsigned short afmThres;
    // M4_ARRAY_MARK_DESC("Gamma Curve", "u16", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0,45,108,179,245,344,409,459,500,567,622,676,759,833,896,962,1023]", M4_DIGIT(0), M4_DYNAMIC(0), "curve_table")
    unsigned short gammaY[17];
    // M4_NUMBER_DESC("v1fv reliable", "f32", M4_RANGE(0, 1), "0.2", M4_DIGIT(3))
    float v1fv_reliable;
    // M4_NUMBER_DESC("v2fv reliable", "f32", M4_RANGE(0, 1), "0.2", M4_DIGIT(3))
    float v2fv_reliable;
    // M4_NUMBER_DESC("v1 fir sel", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
    unsigned char v1_fir_sel;
    // M4_ARRAY_DESC("v1 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.04, 0.14]", M4_DIGIT(3), M4_DYNAMIC(0))
    float v1_band[2];
    // M4_ARRAY_DESC("vertical first iir filter", "s16", M4_SIZE(1,9), M4_RANGE(-2047,2047), "[0, 483, 0, 0, 27, 0, 0, 28, 0]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v1_iir_coe[9];
    // M4_ARRAY_DESC("vertical first fir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[-1212, 0, 1212]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v1_fir_coe[3];
    // M4_ARRAY_DESC("v2 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.04, 0.14]", M4_DIGIT(3), M4_DYNAMIC(0))
    float v2_band[2];
    // M4_ARRAY_DESC("vertical second iir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[483, 27, 28]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v2_iir_coe[3];
    // M4_ARRAY_DESC("vertical second fir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[-1212, 0, 1212]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v2_fir_coe[3];
    // M4_ARRAY_DESC("h1 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.04, 0.14]", M4_DIGIT(3), M4_DYNAMIC(0))
    float h1_band[2];
    // M4_ARRAY_DESC("horizontal first iir1 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[512, 553, -271, 471, 0, -471]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h1_iir1_coe[6];
    // M4_ARRAY_DESC("horizontal first iir2 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[97, 875, -400, 199, 0, -199]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h1_iir2_coe[6];
    // M4_ARRAY_DESC("h2 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.04, 0.14]", M4_DIGIT(3), M4_DYNAMIC(0))
    float h2_band[2];
    // M4_ARRAY_DESC("horizontal second iir1 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[512, 553, -271, 471, 0, -471]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h2_iir1_coe[6];
    // M4_ARRAY_DESC("horizontal second iir2 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[97, 875, -400, 199, 0, -199]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h2_iir2_coe[6];
    // M4_NUMBER_DESC("ldg enable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char ldg_en;
    // M4_NUMBER_DESC("vertical minluma thresh", "u8", M4_RANGE(0, 255), "64", M4_DIGIT(0))
    unsigned char ve_ldg_lumth_l;
    // M4_NUMBER_DESC("vertical gain for minluma", "u8", M4_RANGE(0, 255), "28", M4_DIGIT(0))
    unsigned char ve_ldg_gain_l;
    // M4_NUMBER_DESC("vertical slope low", "u16", M4_RANGE(0, 8191), "1286", M4_DIGIT(0))
    unsigned short ve_ldg_gslp_l;
    // M4_NUMBER_DESC("vertical maxluma thresh", "u8", M4_RANGE(0, 255), "185", M4_DIGIT(0))
    unsigned char ve_ldg_lumth_h;
    // M4_NUMBER_DESC("vertical gain for maxluma", "u8", M4_RANGE(0, 255), "8", M4_DIGIT(0))
    unsigned char ve_ldg_gain_h;
    // M4_NUMBER_DESC("vertical slope high", "u16", M4_RANGE(0, 8191), "1400", M4_DIGIT(0))
    unsigned short ve_ldg_gslp_h;
    // M4_NUMBER_DESC("horizontal minluma thresh", "u8", M4_RANGE(0, 255), "64", M4_DIGIT(0))
    unsigned char ho_ldg_lumth_l;
    // M4_NUMBER_DESC("horizontal gain for minluma", "u8", M4_RANGE(0, 255), "28", M4_DIGIT(0))
    unsigned char ho_ldg_gain_l;
    // M4_NUMBER_DESC("horizontal slope low", "u16", M4_RANGE(0, 8191), "1286", M4_DIGIT(0))
    unsigned short ho_ldg_gslp_l;
    // M4_NUMBER_DESC("horizontal maxluma thresh", "u8", M4_RANGE(0, 255), "185", M4_DIGIT(0))
    unsigned char ho_ldg_lumth_h;
    // M4_NUMBER_DESC("horizontal gain for maxluma", "u8", M4_RANGE(0, 255), "8", M4_DIGIT(0))
    unsigned char ho_ldg_gain_h;
    // M4_NUMBER_DESC("horizontal slope high", "u16", M4_RANGE(0, 8191), "1400", M4_DIGIT(0))
    unsigned short ho_ldg_gslp_h;
    // M4_NUMBER_DESC("vertical fv thresh", "u16", M4_RANGE(0, 4095), "4", M4_DIGIT(0))
    unsigned short v_fv_thresh;
    // M4_NUMBER_DESC("horizontal fv thresh", "u16", M4_RANGE(0, 4095), "4", M4_DIGIT(0))
    unsigned short h_fv_thresh;
    // M4_NUMBER_DESC("highlight thresh", "u16", M4_RANGE(0, 4095), "912", M4_DIGIT(0))
    unsigned short highlit_thresh;
    // M4_NUMBER_DESC("vertical fv ratio", "f32", M4_RANGE(0, 1), "0.5", M4_DIGIT(3))
    float v_fv_ratio;
} CalibDbV2_AfV30_MeasCfg_t;

typedef struct CalibDbV2_AfV30_ZoomMeas_s {
    // M4_NUMBER_DESC("zoom index", "u32", M4_RANGE(0,100000), "0", M4_DIGIT(0), M4_HIDE(0))
    int zoom_idx;
    // M4_ARRAY_DESC("iso", "f32", M4_SIZE(1,13), M4_RANGE(50, 10000000), "[50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]", M4_DIGIT(0), M4_HIDE(0))
    float iso[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_ARRAY_DESC("meas table index", "u32", M4_SIZE(1,13), M4_RANGE(0,100), "0", M4_DIGIT(0))
    int idx[CALIBDBV2_MAX_ISO_LEVEL];
    // M4_ARRAY_DESC("spotlight scene meas table index", "u32", M4_SIZE(1,13), M4_RANGE(0,100), "0", M4_DIGIT(0))
    int spotlt_scene_idx[CALIBDBV2_MAX_ISO_LEVEL];
} CalibDbV2_AfV30_ZoomMeas_t;

typedef struct CalibDbV2_AFV30_Tuning_Para_s {
    // M4_ENUM_DESC("mode", "CalibDbV2_AF_MODE_t", "CalibDbV2_AFMODE_CONT_PICTURE")
    CalibDbV2_AF_MODE_t af_mode;
    // M4_NUMBER_DESC("win_h_offs", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_offs;
    // M4_NUMBER_DESC("win_v_offs", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_offs;
    // M4_NUMBER_DESC("win_h_size", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_size;
    // M4_NUMBER_DESC("win_v_size", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_size;
    // M4_NUMBER_DESC("win_h_offs in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_h_offs;
    // M4_NUMBER_DESC("win_v_offs in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_v_offs;
    // M4_NUMBER_DESC("win_h_size in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_h_size;
    // M4_NUMBER_DESC("win_v_size in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_v_size;
    // M4_STRUCT_DESC("fixed mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t fixed_mode;
    // M4_STRUCT_DESC("macro mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t macro_mode;
    // M4_STRUCT_DESC("infinity mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t infinity_mode;
    // M4_STRUCT_DESC("contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t contrast_af;
    // M4_STRUCT_DESC("video contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t video_contrast_af;
    // M4_STRUCT_DESC("laser af", "normal_ui_style")
    CalibDbV2_Af_Laser_t laser_af;
    // M4_STRUCT_DESC("pdaf", "normal_ui_style")
    CalibDbV2_Af_Pdaf_t pdaf;
    // M4_STRUCT_DESC("vcmcfg", "normal_ui_style")
    CalibDbV2_Af_VcmCfg_t vcmcfg;
    // M4_STRUCT_DESC("zoomfocus_tbl", "normal_ui_style")
    CalibDbV2_Af_ZoomFocusTbl_t zoomfocus_tbl;
    // M4_STRUCT_LIST_DESC("zoom_meas", M4_SIZE(1,8), "normal_ui_style")
    CalibDbV2_AfV30_ZoomMeas_t zoom_meas[8];
    // M4_NUMBER_DESC("zoom_meas_len", "s32", M4_RANGE(1,8), "1", M4_DIGIT(0), M4_HIDE(0))
    int zoom_meas_len;
    // M4_STRUCT_LIST_DESC("meas config table", M4_SIZE(1,32), "normal_ui_style")
    CalibDbV2_AfV30_MeasCfg_t meascfg_tbl[CALIBDBV2_MAX_MEAS_CONFIG_NUM];
    // M4_NUMBER_DESC("meascfg_tbl_len", "s32", M4_RANGE(1,13), "1", M4_DIGIT(0), M4_HIDE(0))
    int meascfg_tbl_len;
} CalibDbV2_AFV30_Tuning_Para_t;

typedef struct {
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_AFV30_Tuning_Para_t TuningPara;
} CalibDbV2_AFV30_t;

typedef enum CalibDbV2_AF_DNSCL_MODE_s {
    CalibDbV2_AF_DNSCL_0 = 0,
    CalibDbV2_AF_DNSCL_1 = 1,
    CalibDbV2_AF_DNSCL_2 = 2,
    CalibDbV2_AF_DNSCL_4 = 3,
    CalibDbV2_AF_DNSCL_8 = 4
} CalibDbV2_AF_DNSCL_MODE_t;

typedef struct CalibDbV2_AfV31_MeasCfg_s {
    // M4_NUMBER_DESC("table index", "u32", M4_RANGE(0, 1000000), "0", M4_DIGIT(0))
    unsigned int tbl_idx;
    // M4_BOOL_DESC("from awb", "0")
    bool from_awb;
    // M4_BOOL_DESC("from ynr", "0")
    bool from_ynr;
    // M4_NUMBER_DESC("afmThres", "u16", M4_RANGE(0, 255), "4", M4_DIGIT(0))
    unsigned short afmThres;
    // M4_ARRAY_MARK_DESC("Gamma Curve", "u16", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0,45,108,179,245,344,409,459,500,567,622,676,759,833,896,962,1023]", M4_DIGIT(0), M4_DYNAMIC(0), "curve_table")
    unsigned short gammaY[17];
    // M4_ARRAY_DESC("gaus coe", "s16", M4_SIZE(3,3), M4_RANGE(-128, 127), "[0,64,0,0,64,0,0,0,0]", M4_DIGIT(0), M4_DYNAMIC(0))
    short gaus_coe[9];
    // M4_ENUM_DESC("downscale mode", "CalibDbV2_AF_DNSCL_MODE_t", "CalibDbV2_AF_DNSCL_1")
    CalibDbV2_AF_DNSCL_MODE_t dnscl_mode;
    // M4_NUMBER_DESC("v1fv reliable", "f32", M4_RANGE(0, 1), "0.5", M4_DIGIT(3))
    float v1fv_reliable;
    // M4_NUMBER_DESC("v2fv reliable", "f32", M4_RANGE(0, 1), "0.5", M4_DIGIT(3))
    float v2fv_reliable;
    // M4_NUMBER_DESC("v1 fir sel", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
    unsigned char v1_fir_sel;
    // M4_ARRAY_DESC("v1 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.042,0.140]", M4_DIGIT(3), M4_DYNAMIC(0))
    float v1_band[2];
    // M4_ARRAY_DESC("vertical first iir filter", "s16", M4_SIZE(1,3), M4_RANGE(-255,255), "[-265, 686, 512]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v1_iir_coe[3];
    // M4_ARRAY_DESC("vertical first fir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[-124, 0, 124]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v1_fir_coe[3];
    // M4_ARRAY_DESC("v2 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.055, 0.125]", M4_DIGIT(3), M4_DYNAMIC(0))
    float v2_band[2];
    // M4_ARRAY_DESC("vertical second iir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[-325, 724, 512]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v2_iir_coe[3];
    // M4_ARRAY_DESC("vertical second fir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[-94, 0, 94]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v2_fir_coe[3];
    // M4_ARRAY_DESC("h1 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.042, 0.14]", M4_DIGIT(3), M4_DYNAMIC(0))
    float h1_band[2];
    // M4_ARRAY_DESC("horizontal first iir1 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[512, 557, -276, 460, 0, -460]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h1_iir1_coe[6];
    // M4_ARRAY_DESC("horizontal first iir2 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[100, 870, -399, 191, 0, -191]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h1_iir2_coe[6];
    // M4_ARRAY_DESC("h2 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.055, 0.125]", M4_DIGIT(3), M4_DYNAMIC(0))
    float h2_band[2];
    // M4_ARRAY_DESC("horizontal second iir1 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[415, 648, -344, 403, 0, -403]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h2_iir1_coe[6];
    // M4_ARRAY_DESC("horizontal second iir2 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[96, 854, -409, 156, 0, -156]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h2_iir2_coe[6];
    // M4_NUMBER_DESC("ldg enable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char ldg_en;
    // M4_NUMBER_DESC("vertical minluma thresh", "u8", M4_RANGE(0, 255), "64", M4_DIGIT(0))
    unsigned char ve_ldg_lumth_l;
    // M4_NUMBER_DESC("vertical gain for minluma", "u8", M4_RANGE(0, 255), "28", M4_DIGIT(0))
    unsigned char ve_ldg_gain_l;
    // M4_NUMBER_DESC("vertical slope low", "u16", M4_RANGE(0, 8191), "1286", M4_DIGIT(0))
    unsigned short ve_ldg_gslp_l;
    // M4_NUMBER_DESC("vertical maxluma thresh", "u8", M4_RANGE(0, 255), "185", M4_DIGIT(0))
    unsigned char ve_ldg_lumth_h;
    // M4_NUMBER_DESC("vertical gain for maxluma", "u8", M4_RANGE(0, 255), "8", M4_DIGIT(0))
    unsigned char ve_ldg_gain_h;
    // M4_NUMBER_DESC("vertical slope high", "u16", M4_RANGE(0, 8191), "1400", M4_DIGIT(0))
    unsigned short ve_ldg_gslp_h;
    // M4_NUMBER_DESC("horizontal minluma thresh", "u8", M4_RANGE(0, 255), "64", M4_DIGIT(0))
    unsigned char ho_ldg_lumth_l;
    // M4_NUMBER_DESC("horizontal gain for minluma", "u8", M4_RANGE(0, 255), "28", M4_DIGIT(0))
    unsigned char ho_ldg_gain_l;
    // M4_NUMBER_DESC("horizontal slope low", "u16", M4_RANGE(0, 8191), "1286", M4_DIGIT(0))
    unsigned short ho_ldg_gslp_l;
    // M4_NUMBER_DESC("horizontal maxluma thresh", "u8", M4_RANGE(0, 255), "185", M4_DIGIT(0))
    unsigned char ho_ldg_lumth_h;
    // M4_NUMBER_DESC("horizontal gain for maxluma", "u8", M4_RANGE(0, 255), "8", M4_DIGIT(0))
    unsigned char ho_ldg_gain_h;
    // M4_NUMBER_DESC("horizontal slope high", "u16", M4_RANGE(0, 8191), "1400", M4_DIGIT(0))
    unsigned short ho_ldg_gslp_h;
    // M4_NUMBER_DESC("vertical fv thresh", "u16", M4_RANGE(0, 4095), "4", M4_DIGIT(0))
    unsigned short v_fv_thresh;
    // M4_NUMBER_DESC("horizontal fv thresh", "u16", M4_RANGE(0, 4095), "4", M4_DIGIT(0))
    unsigned short h_fv_thresh;
    // M4_NUMBER_DESC("highlight thresh", "u16", M4_RANGE(0, 1023), "912", M4_DIGIT(0))
    unsigned short highlit_thresh;
    // M4_NUMBER_DESC("vertical fv ratio", "f32", M4_RANGE(0, 1), "0.5", M4_DIGIT(3))
    float v_fv_ratio;
} CalibDbV2_AfV31_MeasCfg_t;

typedef struct CalibDbV2_AFV31_Tuning_Para_s {
    // M4_ENUM_DESC("mode", "CalibDbV2_AF_MODE_t", "CalibDbV2_AFMODE_CONT_PICTURE")
    CalibDbV2_AF_MODE_t af_mode;
    // M4_NUMBER_DESC("win_h_offs", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_offs;
    // M4_NUMBER_DESC("win_v_offs", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_offs;
    // M4_NUMBER_DESC("win_h_size", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_size;
    // M4_NUMBER_DESC("win_v_size", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_size;
    // M4_NUMBER_DESC("win_h_offs in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_h_offs;
    // M4_NUMBER_DESC("win_v_offs in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_v_offs;
    // M4_NUMBER_DESC("win_h_size in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_h_size;
    // M4_NUMBER_DESC("win_v_size in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_v_size;
    // M4_STRUCT_DESC("fixed mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t fixed_mode;
    // M4_STRUCT_DESC("macro mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t macro_mode;
    // M4_STRUCT_DESC("infinity mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t infinity_mode;
    // M4_STRUCT_DESC("contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t contrast_af;
    // M4_STRUCT_DESC("video contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t video_contrast_af;
    // M4_STRUCT_DESC("laser af", "normal_ui_style")
    CalibDbV2_Af_Laser_t laser_af;
    // M4_STRUCT_DESC("vcmcfg", "normal_ui_style")
    CalibDbV2_Af_VcmCfg_t vcmcfg;
    // M4_STRUCT_DESC("zoomfocus_tbl", "normal_ui_style")
    CalibDbV2_Af_ZoomFocusTbl_t zoomfocus_tbl;
    // M4_STRUCT_LIST_DESC("zoom_meas", M4_SIZE(1,8), "normal_ui_style")
    CalibDbV2_AfV30_ZoomMeas_t zoom_meas[8];
    // M4_NUMBER_DESC("zoom_meas_len", "s32", M4_RANGE(1,8), "1", M4_DIGIT(0), M4_HIDE(0))
    int zoom_meas_len;
    // M4_STRUCT_LIST_DESC("meas config table", M4_SIZE(1,32), "normal_ui_style")
    CalibDbV2_AfV31_MeasCfg_t meascfg_tbl[CALIBDBV2_MAX_MEAS_CONFIG_NUM];
    // M4_NUMBER_DESC("meascfg_tbl_len", "s32", M4_RANGE(1,13), "1", M4_DIGIT(0), M4_HIDE(0))
    int meascfg_tbl_len;
} CalibDbV2_AFV31_Tuning_Para_t;

typedef struct {
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_AFV31_Tuning_Para_t TuningPara;
} CalibDbV2_AFV31_t;

typedef struct CalibDbV2_AfV32_MeasCfg_s {
    // M4_NUMBER_DESC("table index", "u32", M4_RANGE(0, 1000000), "0", M4_DIGIT(0))
    unsigned int tbl_idx;
    // M4_BOOL_DESC("from awb", "0")
    bool from_awb;
    // M4_BOOL_DESC("from ynr", "0")
    bool from_ynr;
    // M4_BOOL_DESC("from bnr", "0")
    bool from_bnr;
    // M4_BOOL_DESC("bls en", "0")
    bool bls_en;
    // M4_BOOL_DESC("avg ds en", "0")
    bool avg_ds_en;
    // M4_BOOL_DESC("v1 max mode", "0")
    bool v1_maxmode;
    // M4_BOOL_DESC("v2 max mode", "0")
    bool v2_maxmode;
    // M4_BOOL_DESC("h1 max mode", "0")
    bool h1_maxmode;
    // M4_BOOL_DESC("h2 max mode", "0")
    bool h2_maxmode;
    // M4_BOOL_DESC("hiir left border mode", "0")
    bool hiir_left_border_mode;
    // M4_NUMBER_DESC("bnr in shift", "u8", M4_RANGE(0, 15), "0", M4_DIGIT(0))
    unsigned char bnr_in_shift;
    // M4_NUMBER_DESC("bls offset", "s16", M4_RANGE(-256, 255), "0", M4_DIGIT(0))
    short bls_offset;
    // M4_NUMBER_DESC("avg ds mode", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char avg_ds_mode;
    // M4_NUMBER_DESC("afmThres", "u16", M4_RANGE(0, 255), "4", M4_DIGIT(0))
    unsigned short afmThres;
    // M4_ARRAY_MARK_DESC("Gamma Curve", "u16", M4_SIZE(1,17),  M4_RANGE(0, 1023), "[0,45,108,179,245,344,409,459,500,567,622,676,759,833,896,962,1023]", M4_DIGIT(0), M4_DYNAMIC(0), "curve_table")
    unsigned short gammaY[17];
    // M4_ARRAY_DESC("gaus coe", "s16", M4_SIZE(3,3), M4_RANGE(-128, 127), "[0,64,0,0,64,0,0,0,0]", M4_DIGIT(0), M4_DYNAMIC(0))
    short gaus_coe[9];
    // M4_ENUM_DESC("downscale mode", "CalibDbV2_AF_DNSCL_MODE_t", "CalibDbV2_AF_DNSCL_1")
    CalibDbV2_AF_DNSCL_MODE_t dnscl_mode;
    // M4_NUMBER_DESC("v1fv reliable", "f32", M4_RANGE(0, 1), "0.5", M4_DIGIT(3))
    float v1fv_reliable;
    // M4_NUMBER_DESC("v2fv reliable", "f32", M4_RANGE(0, 1), "0.5", M4_DIGIT(3))
    float v2fv_reliable;
    // M4_NUMBER_DESC("v1 fir sel", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
    unsigned char v1_fir_sel;
    // M4_ARRAY_DESC("v1 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.042,0.140]", M4_DIGIT(3), M4_DYNAMIC(0))
    float v1_band[2];
    // M4_ARRAY_DESC("vertical first iir filter", "s16", M4_SIZE(1,3), M4_RANGE(-255,255), "[-265, 686, 512]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v1_iir_coe[3];
    // M4_ARRAY_DESC("vertical first fir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[-124, 0, 124]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v1_fir_coe[3];
    // M4_ARRAY_DESC("v2 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.055, 0.125]", M4_DIGIT(3), M4_DYNAMIC(0))
    float v2_band[2];
    // M4_ARRAY_DESC("vertical second iir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[-325, 724, 512]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v2_iir_coe[3];
    // M4_ARRAY_DESC("vertical second fir filter", "s16", M4_SIZE(1,3), M4_RANGE(-2047,2047), "[-94, 0, 94]", M4_DIGIT(0), M4_DYNAMIC(0))
    short v2_fir_coe[3];
    // M4_ARRAY_DESC("h1 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.042, 0.14]", M4_DIGIT(3), M4_DYNAMIC(0))
    float h1_band[2];
    // M4_ARRAY_DESC("horizontal first iir1 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[512, 557, -276, 460, 0, -460]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h1_iir1_coe[6];
    // M4_ARRAY_DESC("horizontal first iir2 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[512, 870, -399, 37, 0, -37]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h1_iir2_coe[6];
    // M4_ARRAY_DESC("h2 band", "f32", M4_SIZE(1,2), M4_RANGE(0, 1), "[0.055, 0.125]", M4_DIGIT(3), M4_DYNAMIC(0))
    float h2_band[2];
    // M4_ARRAY_DESC("horizontal second iir1 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[512, 648, -344, 327, 0, -327]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h2_iir1_coe[6];
    // M4_ARRAY_DESC("horizontal second iir2 filter", "s16", M4_SIZE(1,6), M4_RANGE(-2047,2047), "[512, 854, -409, 29, 0, -29]", M4_DIGIT(0), M4_DYNAMIC(0))
    short h2_iir2_coe[6];
    // M4_NUMBER_DESC("ldg enable", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    unsigned char ldg_en;
    // M4_NUMBER_DESC("vertical minluma thresh", "u8", M4_RANGE(0, 255), "64", M4_DIGIT(0))
    unsigned char ve_ldg_lumth_l;
    // M4_NUMBER_DESC("vertical gain for minluma", "u8", M4_RANGE(0, 255), "28", M4_DIGIT(0))
    unsigned char ve_ldg_gain_l;
    // M4_NUMBER_DESC("vertical slope low", "u16", M4_RANGE(0, 8191), "1286", M4_DIGIT(0))
    unsigned short ve_ldg_gslp_l;
    // M4_NUMBER_DESC("vertical maxluma thresh", "u8", M4_RANGE(0, 255), "185", M4_DIGIT(0))
    unsigned char ve_ldg_lumth_h;
    // M4_NUMBER_DESC("vertical gain for maxluma", "u8", M4_RANGE(0, 255), "8", M4_DIGIT(0))
    unsigned char ve_ldg_gain_h;
    // M4_NUMBER_DESC("vertical slope high", "u16", M4_RANGE(0, 8191), "1400", M4_DIGIT(0))
    unsigned short ve_ldg_gslp_h;
    // M4_NUMBER_DESC("horizontal minluma thresh", "u8", M4_RANGE(0, 255), "64", M4_DIGIT(0))
    unsigned char ho_ldg_lumth_l;
    // M4_NUMBER_DESC("horizontal gain for minluma", "u8", M4_RANGE(0, 255), "28", M4_DIGIT(0))
    unsigned char ho_ldg_gain_l;
    // M4_NUMBER_DESC("horizontal slope low", "u16", M4_RANGE(0, 8191), "1286", M4_DIGIT(0))
    unsigned short ho_ldg_gslp_l;
    // M4_NUMBER_DESC("horizontal maxluma thresh", "u8", M4_RANGE(0, 255), "185", M4_DIGIT(0))
    unsigned char ho_ldg_lumth_h;
    // M4_NUMBER_DESC("horizontal gain for maxluma", "u8", M4_RANGE(0, 255), "8", M4_DIGIT(0))
    unsigned char ho_ldg_gain_h;
    // M4_NUMBER_DESC("horizontal slope high", "u16", M4_RANGE(0, 8191), "1400", M4_DIGIT(0))
    unsigned short ho_ldg_gslp_h;
    // M4_NUMBER_DESC("horizontal dilate num", "u8", M4_RANGE(0, 7), "0", M4_DIGIT(0))
    unsigned char hldg_dilate_num;
    // M4_NUMBER_DESC("vertical fv thresh", "u16", M4_RANGE(0, 4095), "4", M4_DIGIT(0))
    unsigned short v_fv_thresh;
    // M4_NUMBER_DESC("vertical fv limit", "u16", M4_RANGE(0, 1023), "1023", M4_DIGIT(0))
    unsigned short v_fv_limit;
    // M4_NUMBER_DESC("vertical fv slope", "u16", M4_RANGE(0, 511), "256", M4_DIGIT(0))
    unsigned short v_fv_slope;
    // M4_NUMBER_DESC("horizontal fv thresh", "u16", M4_RANGE(0, 4095), "4", M4_DIGIT(0))
    unsigned short h_fv_thresh;
    // M4_NUMBER_DESC("horizontal fv limit", "u16", M4_RANGE(0, 1023), "1023", M4_DIGIT(0))
    unsigned short h_fv_limit;
    // M4_NUMBER_DESC("horizontal fv slope", "u16", M4_RANGE(0, 511), "256", M4_DIGIT(0))
    unsigned short h_fv_slope;
    // M4_NUMBER_DESC("highlight thresh", "u16", M4_RANGE(0, 1023), "912", M4_DIGIT(0))
    unsigned short highlit_thresh;
    // M4_NUMBER_DESC("vertical fv ratio", "f32", M4_RANGE(0, 1), "0.5", M4_DIGIT(3))
    float v_fv_ratio;
} CalibDbV2_AfV32_MeasCfg_t;

typedef struct CalibDbV2_AFV32_Tuning_Para_s {
    // M4_ENUM_DESC("mode", "CalibDbV2_AF_MODE_t", "CalibDbV2_AFMODE_CONT_PICTURE")
    CalibDbV2_AF_MODE_t af_mode;
    // M4_NUMBER_DESC("win_h_offs", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_offs;
    // M4_NUMBER_DESC("win_v_offs", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_offs;
    // M4_NUMBER_DESC("win_h_size", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_h_size;
    // M4_NUMBER_DESC("win_v_size", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short win_v_size;
    // M4_NUMBER_DESC("win_h_offs in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_h_offs;
    // M4_NUMBER_DESC("win_v_offs in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_v_offs;
    // M4_NUMBER_DESC("win_h_size in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_h_size;
    // M4_NUMBER_DESC("win_v_size in video", "u16", M4_RANGE(0,2000), "0", M4_DIGIT(0),M4_HIDE(0))
    unsigned short video_win_v_size;
    // M4_STRUCT_DESC("fixed mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t fixed_mode;
    // M4_STRUCT_DESC("macro mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t macro_mode;
    // M4_STRUCT_DESC("infinity mode", "normal_ui_style")
    CalibDbV2_Af_DefCode_t infinity_mode;
    // M4_STRUCT_DESC("contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t contrast_af;
    // M4_STRUCT_DESC("video contrast af", "normal_ui_style")
    CalibDbV2_Af_Contrast_t video_contrast_af;
    // M4_STRUCT_DESC("laser af", "normal_ui_style")
    CalibDbV2_Af_Laser_t laser_af;
    // M4_STRUCT_DESC("vcmcfg", "normal_ui_style")
    CalibDbV2_Af_VcmCfg_t vcmcfg;
    // M4_STRUCT_DESC("zoomfocus_tbl", "normal_ui_style")
    CalibDbV2_Af_ZoomFocusTbl_t zoomfocus_tbl;
    // M4_STRUCT_LIST_DESC("zoom_meas", M4_SIZE(1,8), "normal_ui_style")
    CalibDbV2_AfV30_ZoomMeas_t zoom_meas[8];
    // M4_NUMBER_DESC("zoom_meas_len", "s32", M4_RANGE(1,8), "1", M4_DIGIT(0), M4_HIDE(0))
    int zoom_meas_len;
    // M4_STRUCT_LIST_DESC("meas config table", M4_SIZE(1,32), "normal_ui_style")
    CalibDbV2_AfV32_MeasCfg_t meascfg_tbl[CALIBDBV2_MAX_MEAS_CONFIG_NUM];
    // M4_NUMBER_DESC("meascfg_tbl_len", "s32", M4_RANGE(1,13), "1", M4_DIGIT(0), M4_HIDE(0))
    int meascfg_tbl_len;
} CalibDbV2_AFV32_Tuning_Para_t;

typedef struct {
    // M4_STRUCT_DESC("TuningPara", "normal_ui_style")
    CalibDbV2_AFV32_Tuning_Para_t TuningPara;
} CalibDbV2_AFV32_t;

typedef struct Af_ZoomVarStepCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomIdx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Zoom index to get search config.\n
        Freq of use: low))  */
    unsigned short          ZoomIdx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(Iso),
        M4_TYPE(s32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT([50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(1),
        M4_NOTES(Iso value to get search config.\n
        Freq of use: low))  */
    int                     Iso[CALIBDBV2_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchStep),
        M4_TYPE(u16),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(2),
        M4_NOTES(Search step value.\n
        Freq of use: low))  */
    unsigned short          SearchStep[CALIBDBV2_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(Stage1SkipThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(3),
        M4_NOTES(Stage 1 skip threshold value.\n
        Freq of use: low))  */
    float                   Stage1SkipThers[CALIBDBV2_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(Stage2QuickEndThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(4),
        M4_NOTES(Stage 2 quick found threshold value.\n
        Freq of use: low))  */
    float                   Stage2QuickEndThers[CALIBDBV2_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(SkipStepCnt),
        M4_TYPE(u16),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(4),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(5),
        M4_NOTES(Skip next step count value.\n
        Freq of use: low))  */
    unsigned short          SkipStepCnt[CALIBDBV2_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(Stage2QuickEndThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(6),
        M4_NOTES(Skip next step threshold value.\n
        Freq of use: low))  */
    float                   SkipStepThers[CALIBDBV2_MAX_ISO_LEVEL];
} Af_ZoomVarStepCfg_t;

typedef struct Af_ContrastZoom_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(QuickFoundThersZoomIdx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(0),
        M4_NOTES(Zoom index to search quick found threshold value.\n
        Freq of use: low))  */
    unsigned short          QuickFoundThersZoomIdx[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(QuickFoundThersZoomIdx_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Length of zoom index table to search quick found threshold value.\n
        Freq of use: low))  */
    int                     QuickFoundThersZoomIdx_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(QuickFoundThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(2),
        M4_NOTES(Quick found threshold value table.\n
        Freq of use: low))  */
    float                   QuickFoundThers[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(QuickFoundThers_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Length of quick found threshold value table.\n
        Freq of use: low))  */
    int                     QuickFoundThers_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchStepZoomIdx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(4),
        M4_NOTES(Zoom index to find search step.\n
        Freq of use: low))  */
    unsigned short          SearchStepZoomIdx[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchStepZoomIdx_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Length of zoom index table to find search step.\n
        Freq of use: low))  */
    int                     SearchStepZoomIdx_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchStep),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(6),
        M4_NOTES(Search step value table.\n
        Freq of use: low))  */
    unsigned short          SearchStep[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchStep_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Length of search step value table.\n
        Freq of use: low))  */
    int                     SearchStep_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StopStepZoomIdx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(8),
        M4_NOTES(Zoom index to find stop step.\n
        Freq of use: low))  */
    unsigned short          StopStepZoomIdx[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(StopStepZoomIdx_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Length of zoom index table to find stop step.\n
        Freq of use: low))  */
    int                     StopStepZoomIdx_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StopStep),
        M4_TYPE(u16),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(4),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(10),
        M4_NOTES(Stop step value table.\n
        Freq of use: low))  */
    unsigned short          StopStep[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(StopStep_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Length of stop step value table.\n
        Freq of use: low))  */
    int                     StopStep_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SwitchDirZoomIdx),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Zoom index to switch search direction(positive and adaptive).\n
        Freq of use: low))  */
    unsigned short          SwitchDirZoomIdx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SpotlightHighlightRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.014),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(Threshold for mean high light value.\n
        Freq of use: low))  */
    float                   SpotlightHighlightRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SpotlightLumaRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([0.3, 0.5, 0.8]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Threshold for luma ratio(=cur luma blk/max luma blk), (low/mid/high).\n
        Freq of use: low))  */
    float                   SpotlightLumaRatio[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(SpotlightBlkCnt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([0.2, 0.5, 0.25]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Threshold for block count ratio(low/mid/high).\n
        Freq of use: low))  */
    float                   SpotlightBlkCnt[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(VarStepCfg),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,10),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(16),
        M4_NOTES(Parameter for variable step search.\n
        Freq of use: low))  */
    Af_ZoomVarStepCfg_t     VarStepCfg[10];
    /* M4_GENERIC_DESC(
        M4_ALIAS(VarStepCfg_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(17),
        M4_NOTES(Length of  variable step search table.\n
        Freq of use: low))  */
    int                     VarStepCfg_len;
} Af_ContrastZoom_t;

typedef struct Af_Contrast_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(Enable),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(contrast_en_group),
        M4_NOTES(Enable contrast af algo.\n
        Freq of use: high))  */
    bool Enable;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchStrategy),
        M4_TYPE(enum),
        M4_ENUM_DEF(CalibDbV2_Af_SS_t),
        M4_DEFAULT(CalibDbV2_AFSS_ADAPTIVE_RANGE),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Search strategy of contrast af.\n
        Freq of use: low))  */
    CalibDbV2_Af_SS_t         Afss;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FullDir),
        M4_TYPE(enum),
        M4_ENUM_DEF(CalibDbV2_Af_SearchDir_t),
        M4_DEFAULT(CalibDbV2_AF_ADAPTIVE_SEARCH),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Search direction for full range search.\n
        Freq of use: low))  */
    CalibDbV2_Af_SearchDir_t  FullDir;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FullRangeTbl),
        M4_TYPE(s16),
        M4_SIZE_EX(1,3),
        M4_RANGE_EX(-1023,1023),
        M4_DEFAULT([0,1,64]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Full range search table(start/step/end).\n
        Freq of use: low))  */
    short                   FullRangeTbl[3];
    /* M4_GENERIC_DESC(
        M4_ALIAS(AdaptiveDir),
        M4_TYPE(enum),
        M4_ENUM_DEF(CalibDbV2_Af_SearchDir_t),
        M4_DEFAULT(CalibDbV2_AF_ADAPTIVE_SEARCH),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Search direction for adaptive search.\n
        Freq of use: low))  */
    CalibDbV2_Af_SearchDir_t  AdaptiveDir;
    /* M4_GENERIC_DESC(
        M4_ALIAS(AdaptRangeTbl),
        M4_TYPE(u16),
        M4_SIZE_EX(1,65),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([0,8,16,24,32,40,48,56,64]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(5),
        M4_NOTES(Rough search table for adaptive search.\n
        Freq of use: high))  */
    unsigned short          AdaptRangeTbl[65];
    /* M4_GENERIC_DESC(
        M4_ALIAS(AdaptRangeTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,65),
        M4_DEFAULT(9),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Length of rough search table for adaptive search.\n
        Freq of use: high))  */
    int                     AdaptRangeTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FineSearchStep),
        M4_TYPE(u16),
        M4_SIZE_EX(1,65),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([4,4,4,4,4,4,4,4,4]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(7),
        M4_NOTES(Fine search table for adaptive search.\n
        Freq of use: high))  */
    unsigned short          FineSearchStep[65];
    /* M4_GENERIC_DESC(
        M4_ALIAS(FineSearchStep_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,65),
        M4_DEFAULT(9),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Length of fine search table for adaptive search.\n
        Freq of use: high))  */
    int                     FineSearchStep_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SkipCurveFitGain),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Sensor gain value to skip curve fit.\n
        Freq of use: low))  */
    float                   SkipCurveFitGain;
    /* M4_GENERIC_DESC(
        M4_ALIAS(TrigThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.075),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(10),
        M4_NOTES(Focus value trigger threshold table.\n
        Freq of use: high))  */
    float                   TrigThers[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(TrigThers_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Length of focus value trigger threshold table.\n
        Freq of use: high))  */
    int                     TrigThers_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(TrigThersFv),
        M4_TYPE(f32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(12),
        M4_NOTES(Focus value table to find focus value trigger threshold.\n
        Freq of use: high))  */
    float                   TrigThersFv[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(TrigThersFv_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(Length of focus value table to find focus value trigger threshold.\n
        Freq of use: high))  */
    int                     TrigThersFv_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(LumaTrigThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Luma trigger threshold.\n
        Freq of use: high))  */
    float                   LumaTrigThers;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ExpTrigThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Exposure trigger threshold.\n
        Freq of use: high))  */
    float                   ExpTrigThers;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ChangedFrames),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000),
        M4_DEFAULT(10),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Changed frames to trigger af search.\n
        Freq of use: high))  */
    unsigned short          ChangedFrames;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StableThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(17),
        M4_NOTES(Af stable threshold.\n
        Freq of use: high))  */
    float                   StableThers;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StableFrames),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(18),
        M4_NOTES(Af stable status must hold frame number.\n
        Freq of use: high))  */
    unsigned short          StableFrames;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ValidMaxMinRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(19),
        M4_NOTES(Valid block ratio in flat scene.(max-min)/(max+min)\n
        Freq of use: low))  */
    float                   ValidMaxMinRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ValidValueThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(20),
        M4_NOTES(Valid focus value in flat scene.\n
        Freq of use: low))  */
    float                   ValidValueThers;
    /* M4_GENERIC_DESC(
        M4_ALIAS(OutFocusValue),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(21),
        M4_NOTES(Focus value when out of focus.\n
        Freq of use: low))  */
    float                   OutFocusValue;
    /* M4_GENERIC_DESC(
        M4_ALIAS(OutFocusPos),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(Focus position when out of focus.\n
        Freq of use: low))  */
    unsigned short          OutFocusPos;
    /* M4_GENERIC_DESC(
        M4_ALIAS(LowLumaValue),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(23),
        M4_NOTES(Low luma scene vlaue.\n
        Freq of use: low))  */
    float                   LowLumaValue;
    /* M4_GENERIC_DESC(
        M4_ALIAS(LowLumaPos),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(24),
        M4_NOTES(Focus position when in low luma scene.\n
        Freq of use: low))  */
    unsigned short          LowLumaPos;
    /* M4_GENERIC_DESC(
        M4_ALIAS(WeightEnable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(25),
        M4_NOTES(Enable weight matrix.\n
        Freq of use: low))  */
    bool                    WeightEnable;
    /* M4_GENERIC_DESC(
        M4_ALIAS(Weight),
        M4_TYPE(u16),
        M4_SIZE_EX(15,15),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(0),
        M4_ORDER(26),
        M4_UI_MODULE(grid_weight_table),
        M4_NOTES(Weight matrix to get focus value.\n
        Freq of use: low))  */
    unsigned char           Weight[225];
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchPauseLumaEnable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(27),
        M4_NOTES(Enable to check luma change in search.\n
        Freq of use: low))  */
    bool                    SearchPauseLumaEnable;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchPauseLumaThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.05),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(28),
        M4_NOTES(Search pause threshold when luma change much in search.\n
        Freq of use: low))  */
    float                   SearchPauseLumaThers;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchLumaStableFrames),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(29),
        M4_NOTES(Frame number to judge luma is stable.\n
        Freq of use: low))  */
    unsigned short          SearchLumaStableFrames;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchLumaStableThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(30),
        M4_NOTES(Luma change threshold to judge luma is stable.\n
        Freq of use: low))  */
    float                   SearchLumaStableThers;
    /* M4_GENERIC_DESC(
        M4_ALIAS(Stage1QuickFoundThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.02),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(31),
        M4_NOTES(Quick found threshold value in search stage1.\n
        Freq of use: high))  */
    float                   Stage1QuickFoundThers;
    /* M4_GENERIC_DESC(
        M4_ALIAS(Stage2QuickFoundThers),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.04),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(32),
        M4_NOTES(Quick found threshold value in search stage2.\n
        Freq of use: high))  */
    float                   Stage2QuickFoundThers;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FlatValue),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(33),
        M4_NOTES(Focus value in centor threshold to judge flat scene.\n
        Freq of use: low))  */
    float                   FlatValue;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PointLightLumaTh),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(4095),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(34),
        M4_NOTES(Luma threshold in point light scene.\n
        Freq of use: low))  */
    unsigned short          PointLightLumaTh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PointLightCntTh),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,225),
        M4_DEFAULT(225),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(35),
        M4_NOTES(Block count threshold in point light scene.\n
        Freq of use: low))  */
    unsigned short          PointLightCntTh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomConfig),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(36),
        M4_NOTES(Optical zoom config.\n
        Freq of use: low))  */
    Af_ContrastZoom_t ZoomCfg;
} Af_Contrast_t;

typedef struct Af_Laser_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(Enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(laser_en_group),
        M4_NOTES(Enable laser af algo.\n
        Freq of use: low))  */
    bool Enable;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VcmDot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([0, 16, 32, 40, 48, 56, 64]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Motor position table.\n
        Freq of use: low))  */
    float VcmDot[7];
    /* M4_GENERIC_DESC(
        M4_ALIAS(DistanceDot),
        M4_TYPE(f32),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT([0.2, 0.24, 0.34, 0.4, 0.66, 1, 3]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Distance table.\n
        Freq of use: low))  */
    float DistanceDot[7];
} Af_Laser_t;

typedef struct Af_PdafFineSearch_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(FineSearchTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Length of fine search table.\n
        Freq of use: high))  */
    int FineSearchTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(Confidence),
        M4_TYPE(f32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Confidence of pdaf algo.\n
        Freq of use: high))  */
    float Confidence[10];
    /* M4_GENERIC_DESC(
        M4_ALIAS(Range),
        M4_TYPE(s32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Search range in fine search.\n
        Freq of use: high))  */
    int Range[10];
    /* M4_GENERIC_DESC(
        M4_ALIAS(StepPos),
        M4_TYPE(s32),
        M4_SIZE_EX(1,10),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Search step in fine search.\n
        Freq of use: high))  */
    int StepPos[10];
} Af_PdafFineSearch_t;

typedef struct Af_PdafIsoPara_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(Iso),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Iso value to query.\n
        Freq of use: high))  */
    int Iso;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdRatio0),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Confidence adjust ratio 0.\n
        Freq of use: high))  */
    float PdConfdRatio0;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdRatio1),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Confidence adjust ratio 1.\n
        Freq of use: low))  */
    float PdConfdRatio1;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdRatio2),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Confidence adjust ratio 2.\n
        Freq of use: low))  */
    float PdConfdRatio2;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdRatio3),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,256),
        M4_DEFAULT(1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Confidence adjust ratio 3.\n
        Freq of use: high))  */
    float PdConfdRatio3;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdThresh),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Confidence threshold to trust pdaf algo.\n
        Freq of use: high))  */
    float PdConfdThresh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ConvergedInfPdThresh),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(24),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Converged motor position threshold to stop search(infinity side).\n
        Freq of use: high))  */
    unsigned short ConvergedInfPdThresh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ConvergedMacPdThresh),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(24),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Converged motor position threshold to stop search(macro side).\n
        Freq of use: high))  */
    unsigned short ConvergedMacPdThresh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(DefocusInfPdThresh),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(12),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Defocus motor position threshold to stop search(infinity side).\n
        Freq of use: high))  */
    unsigned short DefocusInfPdThresh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(DefocusMacPdThresh),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(12),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Defocus motor position threshold to stop search(macro side).\n
        Freq of use: high))  */
    unsigned short DefocusMacPdThresh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StablePdRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0.125),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Ratio value to judge pd stable.\n
        Freq of use: low))  */
    float StablePdRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StablePdOffset),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Offset value to judge pd stable.\n
        Freq of use: low))  */
    unsigned short StablePdOffset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StableCntRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(1.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Ratio to decide stable frame count threshold.\n
        Freq of use: low))  */
    float StableCntRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(NoConfCntThresh),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(4),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(No confidence frame count to turn to contrast af.\n
        Freq of use: low))  */
    unsigned short NoConfCntThresh;
    /* M4_GENERIC_DESC(
        M4_ALIAS(RoiBlkCntW),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Block count in roi at width direction.\n
        Freq of use: low))  */
    unsigned char RoiBlkCntW;
    /* M4_GENERIC_DESC(
        M4_ALIAS(RoiBlkCntH),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Block count in roi at height direction.\n
        Freq of use: low))  */
    unsigned char RoiBlkCntH;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FineSearchTbl),
        M4_TYPE(struct),
        M4_UI_MODULE(array_table_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Fine search table.\n
        Freq of use: low))  */
    Af_PdafFineSearch_t FineSearchTbl;
} Af_PdafIsoPara_t;

typedef struct Af_PdafCalibInf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdGainMapW),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1000),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Gainmap width in calibration.\n
        Freq of use: high))  */
    unsigned short PdGainMapW;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdGainMapH),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1000),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Gainmap height in calibration.\n
        Freq of use: high))  */
    unsigned short PdGainMapH;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdDccMapW),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1000),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Dccmap width in calibration.\n
        Freq of use: high))  */
    unsigned short PdDccMapW;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdDccMapH),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,1000),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Dccmap height in calibration.\n
        Freq of use: high))  */
    unsigned short PdDccMapH;
} Af_PdafCalibInf_t;

typedef struct Af_PdafResolution_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdType),
        M4_TYPE(enum),
        M4_ENUM_DEF(CalibDbV2_PDAF_SENSOR_TYPE_t),
        M4_DEFAULT(CalibDbV2_PDAF_SENSOR_TYPE2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Pdaf sensor type.\n
        Freq of use: high))  */
    CalibDbV2_PDAF_SENSOR_TYPE_t PdType;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ImageWidth),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(4096),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Image width of sensor input.\n
        Freq of use: high))  */
    unsigned short ImageWidth;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ImageHeight),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(3072),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Image height of sensor input.\n
        Freq of use: high))  */
    unsigned short ImageHeight;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdOutWidth),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(2048),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Pd data width of current sensor setting.\n
        Freq of use: high))  */
    unsigned short PdOutWidth;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdOutHeight),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(768),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Pd data height of current sensor setting.\n
        Freq of use: high))  */
    unsigned short PdOutHeight;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdCropX),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Pd data start x position of current setting vs calibration setting.\n
        Freq of use: high))  */
    unsigned short PdCropX;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdCropY),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Pd data start y position of current setting vs calibration setting.\n
        Freq of use: high))  */
    unsigned short PdCropY;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdBaseWidth),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(2048),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Pd data width in calibration.\n
        Freq of use: high))  */
    unsigned short PdBaseWidth;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdBaseHeight),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(768),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Pd data height in calibration.\n
        Freq of use: high))  */
    unsigned short PdBaseHeight;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdHBinMode),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Pd bin mode.\n
        Freq of use: high))  */
    unsigned short PdHBinMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdVBinMode),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Pd bin mode.\n
        Freq of use: high))  */
    unsigned short PdVBinMode;
} Af_PdafResolution_t;

typedef struct Af_Pdaf_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(Enable),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_GROUP_CTRL(pdaf_en_group),
        M4_NOTES(Enable pdaf algo.\n
        Freq of use: high))  */
    bool Enable;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdVsCdDebug),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Debug flag to compare pdaf and cdaf.\n
        Freq of use: low))  */
    bool PdVsCdDebug;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdDumpDebug),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Debug flag to dump some data.\n
        Freq of use: low))  */
    bool PdDumpDebug;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdDumpMaxFrm),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,10000),
        M4_DEFAULT(300),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Max dump frames when debug flag is on.\n
        Freq of use: low))  */
    unsigned short PdDumpMaxFrm;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdDataBit),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,16),
        M4_DEFAULT(10),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Bit number of one pd data.\n
        Freq of use: low))  */
    unsigned short PdDataBit;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdBlkLevel),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Black level of pd data.\n
        Freq of use: low))  */
    unsigned short PdBlkLevel;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdSearchRadius),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Search radius.\n
        Freq of use: low))  */
    unsigned short PdSearchRadius;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdMirrorInCalib),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Mirror/flip difference with used setting in calibration.\n
        Freq of use: high))  */
    unsigned char PdMirrorInCalib;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdChangeLeftRight),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Change left/right pd data when input pd data to pdaf algo.\n
        Freq of use: high))  */
    bool PdChangeLeftRight;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdVsImgoutMirror),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Mirror/flip difference with normal picture, used in get search roi.\n
        Freq of use: low))  */
    unsigned char PdVsImgoutMirror;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdLRInDiffLine),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Weather left/right pd data in one line, used to separate left/right pd data.\n
        Freq of use: high))  */
    bool PdLRInDiffLine;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdMaxWidth),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(2048),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Max pd data width, used to alloc buffer for pd data.\n
        Freq of use: high))  */
    unsigned short PdMaxWidth;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdMaxHeight),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,65535),
        M4_DEFAULT(768),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Max pd data height, used to alloc buffer for pd data.\n
        Freq of use: high))  */
    unsigned short PdMaxHeight;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdVerBinning),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(Vertical binning setting.\n
        Freq of use: low))  */
    unsigned char PdVerBinning;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdFrmInValid),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Invalid frame number after judge motor is stable.\n
        Freq of use: low))  */
    unsigned char PdFrmInValid;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdDgainValid),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Weather sensor dgain is valid for pd data.\n
        Freq of use: low))  */
    bool PdDgainValid;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdGainMapNormEn),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Enable normalize sensor gainmap.\n
        Freq of use: low))  */
    bool PdGainMapNormEn;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdMode),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(17),
        M4_NOTES(Mode to caculate confidence.\n
        Freq of use: low))  */
    unsigned char PdConfdMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdNoiseSn),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(0.207),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(18),
        M4_NOTES(Pd data noise factor0.\n
        Freq of use: low))  */
    float PdNoiseSn;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdNoiseRn),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0,100),
        M4_DEFAULT([0.14, 2.8]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(19),
        M4_NOTES(Pd data noise factor1.\n
        Freq of use: low))  */
    float PdNoiseRn[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdNoisePn),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(20),
        M4_NOTES(Pd data noise factor2.\n
        Freq of use: low))  */
    float PdNoisePn;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdSatValRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.92),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(21),
        M4_NOTES(Saturation value ratio for pd data.\n
        Freq of use: low))  */
    float PdSatValRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdSatCntRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.06),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(Saturation count ratio for pd data.\n
        Freq of use: low))  */
    float PdSatCntRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdDiscardRegionEn),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(23),
        M4_NOTES(Weather to discard less texture block.\n
        Freq of use: low))  */
    bool PdDiscardRegionEn;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdLessTextureRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,5),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(24),
        M4_NOTES(Ratio to judge less texture block.\n
        Freq of use: low))  */
    float PdLessTextureRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdTargetOffset),
        M4_TYPE(s16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-4096,4096),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(25),
        M4_NOTES(Offet to adjust infocus position, we should use value in otp.\n
        Freq of use: low))  */
    short PdTargetOffset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdNumJobs),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(26),
        M4_NOTES(Thread number to do calculate.\n
        Freq of use: low))  */
    unsigned short PdNumJobs;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdCalibInf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(27),
        M4_NOTES(Calibration information.\n
        Freq of use: high))  */
    Af_PdafCalibInf_t PdCalibInf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdConfdMwinFactor),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,225),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(28),
        M4_NOTES(Confidence adjust factor in muti-window in roi case.\n
        Freq of use: low))  */
    unsigned short PdConfdMwinFactor;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdCenterMinFv),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(29),
        M4_NOTES(Min fv to switch roi, keep 0 generally.\n
        Freq of use: low))  */
    unsigned int PdCenterMinFv;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdCenterMinRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(30),
        M4_NOTES(Ratio to switch roi, keep 0 generally.\n
        Freq of use: low))  */
    float PdCenterMinRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdHighlightRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(31),
        M4_NOTES(Ratio to judge high light scene.\n
        Freq of use: low))  */
    float PdHighlightRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdStepRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([1.0, 1.0, 1.0, 0.9, 0.8, 0.7, 0.7]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(32),
        M4_NOTES(Discount ratio table for motor move.\n
        Freq of use: high))  */
    float PdStepRatio[7];
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdStepDefocus),
        M4_TYPE(u16),
        M4_SIZE_EX(1,7),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT([32, 64, 96, 128, 160, 192, 224]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(33),
        M4_NOTES(Defous step table to search PdStepRatio.\n
        Freq of use: high))  */
    unsigned short PdStepDefocus[7];
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdIsoPara),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,13),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(34),
        M4_NOTES(Parameter table changed with iso.\n
        Freq of use: high))  */
    Af_PdafIsoPara_t PdIsoPara[CALIBDBV2_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdIsoPara_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,13),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(35),
        M4_NOTES(Length of parameter table changed with iso.\n
        Freq of use: high))  */
    int PdIsoPara_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdResoInf),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,16),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(36),
        M4_NOTES(Resolution information table.\n
        Freq of use: high))  */
    Af_PdafResolution_t PdResoInf[16];
    /* M4_GENERIC_DESC(
        M4_ALIAS(PdResoInf_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,16),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(37),
        M4_NOTES(Length of resolution information table.\n
        Freq of use: high))  */
    int PdResoInf_len;
} Af_Pdaf_t;

typedef struct Af_VcmCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(MaxLogicalPos),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1023),
        M4_DEFAULT(64),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Max logical position.\n
        Freq of use: low))  */
    unsigned int MaxLogicalPos;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StartCurrent),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1, 1023),
        M4_DEFAULT(-1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Start current.\n
        Freq of use: low))  */
    int StartCurrent;
    /* M4_GENERIC_DESC(
        M4_ALIAS(RatedCurrent),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1, 1023),
        M4_DEFAULT(-1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Rated current.\n
        Freq of use: low))  */
    int RatedCurrent;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StepMode),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-1, 1000),
        M4_DEFAULT(-1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Step mode.\n
        Freq of use: low))  */
    int StepMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ExtraDelay),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(-10000,10000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Extra delay to judge motor stable.\n
        Freq of use: low))  */
    int ExtraDelay;
    /* M4_GENERIC_DESC(
        M4_ALIAS(PostureDiff),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Posture difference ratio.\n
        Freq of use: low))  */
    float PostureDiff;
} Af_VcmCfg_t;

typedef struct Af_ZoomFocusTbl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(WidemodDeviate),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Module deviated length at wide side.\n
        Freq of use: low))  */
    int WidemodDeviate;
    /* M4_GENERIC_DESC(
        M4_ALIAS(TelemodDeviate),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Module deviated length at tele side.\n
        Freq of use: low))  */
    int TelemodDeviate;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FocusBackVal),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Focue back value when motor change direction.\n
        Freq of use: low))  */
    int FocusBackVal;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomMoveDot),
        M4_TYPE(s32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0, 1000000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(3),
        M4_NOTES(Zoom index table to search zoom move step.\n
        Freq of use: low))  */
    int ZoomMoveDot[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomMoveDot_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Length of zoom index table to search zoom move step.\n
        Freq of use: low))  */
    int ZoomMoveDot_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomMoveStep),
        M4_TYPE(s32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0, 1000000),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(5),
        M4_NOTES(Zoom move step table when change focal length.\n
        Freq of use: low))  */
    int ZoomMoveStep[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomMoveStep_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Length of zoom move step table when change focal length.\n
        Freq of use: low))  */
    int ZoomMoveStep_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomSearchTbl),
        M4_TYPE(s32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(-32768,32767),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(7),
        M4_NOTES(Zoom point table in Calibration.\n
        Freq of use: low))  */
    int ZoomSearchTbl[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomSearchTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Length of zoom point table in Calibration.\n
        Freq of use: low))  */
    int ZoomSearchTbl_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomSearchRefCurveIdx),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Reference curve index in calibration.\n
        Freq of use: low))  */
    int ZoomSearchRefCurveIdx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FocusSearchMargin),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Focus search margin to avoid motor conflict with module in calibration.\n
        Freq of use: low))  */
    int FocusSearchMargin;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FocusSearchPlusRange),
        M4_TYPE(s32),
        M4_SIZE_EX(1,32),
        M4_RANGE_EX(0,32767),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(11),
        M4_NOTES(Plus range table to search clear point in calibration.\n
        Freq of use: low))  */
    int FocusSearchPlusRange[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(FocusSearchPlusRange_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(12),
        M4_NOTES(Length of plus range table to search clear point in calibration.\n
        Freq of use: low))  */
    int FocusSearchPlusRange_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FocusStage1Step),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 100),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(13),
        M4_NOTES(Focus step in stage1.\n
        Freq of use: low))  */
    int FocusStage1Step;
    /* M4_GENERIC_DESC(
        M4_ALIAS(QuickFndRate),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(14),
        M4_NOTES(Focus value drop rate for quick found in calibration.\n
        Freq of use: low))  */
    float QuickFndRate;
    /* M4_GENERIC_DESC(
        M4_ALIAS(QuickFndMinFv),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1000000),
        M4_DEFAULT(1000),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(15),
        M4_NOTES(Min focus value for quick found in calibration.\n
        Freq of use: low))  */
    float QuickFndMinFv;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchZoomRange),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 100000),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(16),
        M4_NOTES(Search zoom range in calibration.\n
        Freq of use: low))  */
    int SearchZoomRange;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchFocusRange),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 100000),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(17),
        M4_NOTES(Search focus range in calibration.\n
        Freq of use: low))  */
    int SearchFocusRange;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchEmax),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(18),
        M4_NOTES(Max error threshold in calibration.\n
        Freq of use: low))  */
    float SearchEmax;
    /* M4_GENERIC_DESC(
        M4_ALIAS(SearchEavg),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(100),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(19),
        M4_NOTES(Average error threshold in calibration.\n
        Freq of use: low))  */
    float SearchEavg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(IsZoomFocusRec),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(20),
        M4_GROUP_CTRL(pdaf_en_group),
        M4_NOTES(Weather to reaord zoom focus position.\n
        Freq of use: low))  */
    bool IsZoomFocusRec;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomInfoDir),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 64),
        M4_DEFAULT("/data/"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(21),
        M4_NOTES(Zoom information directory.\n
        Freq of use: low))  */
    char ZoomInfoDir[64];
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomInitIndex),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 100000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(22),
        M4_NOTES(Zoom index when initialization.\n
        Freq of use: low))  */
    int ZoomInitIndex;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomCurveFile),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 64),
        M4_DEFAULT("/data/zoomcurve.bin"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(23),
        M4_NOTES(Zoom curve file.\n
        Freq of use: low))  */
    char ZoomCurveFile[64];
} Af_ZoomFocusTbl_t;

typedef struct Af_Common_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(AfMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(CalibDbV2_AF_MODE_t),
        M4_DEFAULT(CalibDbV2_AFMODE_CONT_PICTURE),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(AF mode.\n
        Freq of use: low))  */
    CalibDbV2_AF_MODE_t AfMode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(WinHOffs),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Horizontal offset of roi for af algo in picture mode.\n
        Freq of use: high))  */
    unsigned short WinHOffs;
    /* M4_GENERIC_DESC(
        M4_ALIAS(WinVOffs),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Vertical offset of roi for af algo in picture mode.\n
        Freq of use: high))  */
    unsigned short WinVOffs;
    /* M4_GENERIC_DESC(
        M4_ALIAS(WinHSize),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Horizontal size of roi for af algo in picture mode.\n
        Freq of use: high))  */
    unsigned short WinHSize;
    /* M4_GENERIC_DESC(
        M4_ALIAS(WinVSize),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Vertical size of roi for af algo in picture mode.\n
        Freq of use: high))  */
    unsigned short WinVSize;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VideoWinHOffs),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Horizontal offset of roi for af algo in video mode.\n
        Freq of use: high))  */
    unsigned short VideoWinHOffs;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VideoWinVOffs),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Vertical offset of roi for af algo in video mode.\n
        Freq of use: high))  */
    unsigned short VideoWinVOffs;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VideoWinHSize),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Horizontal size of roi for af algo in video mode.\n
        Freq of use: high))  */
    unsigned short VideoWinHSize;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VideoWinVSize),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 2000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Vertical size of roi for af algo in video mode.\n
        Freq of use: high))  */
    unsigned short VideoWinVSize;
    /* M4_GENERIC_DESC(
        M4_ALIAS(FixedModeCode),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1023),
        M4_DEFAULT(8),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Motor position in fixed mode.\n
        Freq of use: low))  */
    unsigned short FixedModeCode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(MacroModeCode),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1023),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Max motor position in macro mode.\n
        Freq of use: low))  */
    unsigned short MacroModeCode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(InfinityModeCode),
        M4_TYPE(u16),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1023),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Min motor position in infinity mode.\n
        Freq of use: low))  */
    unsigned short InfinityModeCode;
} Af_Common_t;

typedef struct AfV33_ZoomMeas_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomIdx),
        M4_TYPE(s32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100000),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Zoom index to get measurement index.\n
        Freq of use: low))  */
    int ZoomIdx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(Iso),
        M4_TYPE(f32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(50, 10000000),
        M4_DEFAULT([50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400, 204800]),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Iso value to get measurement index.\n
        Freq of use: low))  */
    float Iso[CALIBDBV2_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(MeasIndex),
        M4_TYPE(s32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Index of af measurement table(normal scene).\n
        Freq of use: low))  */
    int Idx[CALIBDBV2_MAX_ISO_LEVEL];
    /* M4_GENERIC_DESC(
        M4_ALIAS(SpotlightMeasIndex),
        M4_TYPE(s32),
        M4_SIZE_EX(1,13),
        M4_RANGE_EX(0,32),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Index of af measurement table(spot light scene).\n
        Freq of use: low))  */
    int SpotltIdx[CALIBDBV2_MAX_ISO_LEVEL];
} AfV33_ZoomMeas_t;

#include "isp/rk_aiq_stats_af33.h"
typedef struct AfV33_MeasCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(TableIndex),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,31),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Table index.\n
        Freq of use: low))  */
    unsigned int tblIdx;
    /* M4_GENERIC_DESC(
        M4_ALIAS(V1Band),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0.042,0.140]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Band range for v1 filter(just for understand easily).\n
        Freq of use: low))  */
    float V1Band[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(V2Band),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0.042,0.140]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Band range for v2 filter(just for understand easily).\n
        Freq of use: low))  */
    float V2Band[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(H1Band),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0.042,0.140]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Band range for h1 filter(just for understand easily).\n
        Freq of use: low))  */
    float H1Band[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(H2Band),
        M4_TYPE(f32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT([0.042,0.140]),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Band range for h2 filter(just for understand easily).\n
        Freq of use: low))  */
    float H2Band[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(V1FvReliable),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Discount ratio for v1 fv, used in optical zoom.\n
        Freq of use: low))  */
    float V1FvReliable;
    /* M4_GENERIC_DESC(
        M4_ALIAS(V2FvReliable),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Discount ratio for v2 fv, used in optical zoom.\n
        Freq of use: low))  */
    float V2FvReliable;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VerfvRatio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Discount ratio for v1/v2 fv, used in non-optical zoom.\n
        Freq of use: low))  */
    float VerfvRatio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(StatsCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Hardware measurement config.\n
        Freq of use: low))  */
    afStats_cfg_t StatsCfg;
} AfV33_MeasCfg_t;

typedef struct AfV33_Meas_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomMeas),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,8),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(0),
        M4_NOTES(Index table for search measurement config table.\n
        Freq of use: low))  */
    AfV33_ZoomMeas_t ZoomMeas[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomMeas_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,8),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Length of index table for search measurement config table.\n
        Freq of use: low))  */
    int ZoomMeas_len;
    /* M4_GENERIC_DESC(
        M4_ALIAS(MeasCfgTbl),
        M4_TYPE(struct_list),
        M4_SIZE_EX(1,32),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_DYNAMIC_EX(1),
        M4_ORDER(2),
        M4_NOTES(Measurement config table.\n
        Freq of use: low))  */
    AfV33_MeasCfg_t MeasCfgTbl[CALIBDBV2_MAX_MEAS_CONFIG_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(MeasCfgTbl_len),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,32),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Length of measurement config table.\n
        Freq of use: low))  */
    int MeasCfgTbl_len;
} AfV33_Meas_t;

typedef struct CalibDbV2_AFV33_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(Common),
        M4_TYPE(struct),
        M4_SIZE_EX(1,1),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Common config.\n
        Freq of use: high))  */
    Af_Common_t Common;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ContrastAf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_GROUP(contrast_en_group),
        M4_NOTES(Contrast af config in preview mode.\n
        Freq of use: high))  */
    Af_Contrast_t ContrastAf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VideoContrastAf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_GROUP(contrast_en_group),
        M4_NOTES(Contrast af config in video mode.\n
        Freq of use: high))  */
    Af_Contrast_t VideoContrastAf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(LaserAf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(3),
        M4_GROUP(laser_en_group),
        M4_NOTES(Laser af config.\n
        Freq of use: high))  */
    Af_Laser_t LaserAf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(Pdaf),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_GROUP(pdaf_en_group),
        M4_NOTES(Pdaf config.\n
        Freq of use: high))  */
    Af_Pdaf_t Pdaf;
    /* M4_GENERIC_DESC(
        M4_ALIAS(VcmCfg),
        M4_TYPE(struct),
        M4_SIZE_EX(1,1),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Vcm config.\n
        Freq of use: low))  */
    Af_VcmCfg_t VcmCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(ZoomFocusTbl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_GROUP(contrast_en_group),
        M4_NOTES(Optical zoom config.\n
        Freq of use: low))  */
    Af_ZoomFocusTbl_t ZoomFocusTbl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(MeasCfg),
        M4_TYPE(struct),
        M4_SIZE_EX(1,1),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Af measurement config.\n
        Freq of use: low))  */
    AfV33_Meas_t MeasCfg;
} CalibDbV2_AFV33_t;

RKAIQ_END_DECLARE

#endif
