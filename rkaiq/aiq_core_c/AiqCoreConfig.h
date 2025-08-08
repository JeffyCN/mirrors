/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef _AIQ_CORE_CONFIG_H_
#define _AIQ_CORE_CONFIG_H_

#define grp_array_size(x)       (sizeof(x) / sizeof(x[0]))
#define grp_conds_array_info(x) x, grp_array_size(x)

enum XCamMessageType {
    XCAM_MESSAGE_SOF_INFO_OK,
    XCAM_MESSAGE_ISP_STATS_OK,
    XCAM_MESSAGE_ISPP_STATS_OK,
    XCAM_MESSAGE_ISP_POLL_SP_OK,
    XCAM_MESSAGE_ISP_POLL_TX_OK,
    XCAM_MESSAGE_ISP_GAIN_OK,
    XCAM_MESSAGE_ISPP_GAIN_KG_OK,
    XCAM_MESSAGE_NR_IMG_OK,
    XCAM_MESSAGE_AWB_PROC_RES_OK,
    XCAM_MESSAGE_AE_PROC_RES_OK,
    XCAM_MESSAGE_AF_PROC_RES_OK,
    XCAM_MESSAGE_AEC_STATS_OK,
    XCAM_MESSAGE_AWB_STATS_OK,
    XCAM_MESSAGE_AF_STATS_OK,
    XCAM_MESSAGE_ATMO_STATS_OK,
    XCAM_MESSAGE_ADEHAZE_STATS_OK,
    XCAM_MESSAGE_AGAIN_STATS_OK,
    XCAM_MESSAGE_ISPP_GAIN_WR_OK,
    XCAM_MESSAGE_AMD_PROC_RES_OK,
    XCAM_MESSAGE_AE_PRE_RES_OK,
    XCAM_MESSAGE_PDAF_STATS_OK,
    XCAM_MESSAGE_ORB_STATS_OK,
    XCAM_MESSAGE_BLC_PROC_RES_OK,
    XCAM_MESSAGE_BLC_V32_PROC_RES_OK,
    XCAM_MESSAGE_YNR_V3_PROC_RES_OK,
    XCAM_MESSAGE_YNR_V22_PROC_RES_OK,
    XCAM_MESSAGE_YNR_V24_PROC_RES_OK,
    XCAM_MESSAGE_YNR_PROC_RES_OK,
    XCAM_MESSAGE_VICAP_POLL_SCL_OK,
    XCAM_MESSAGE_TNR_STATS_OK,
    XCAM_MESSAGE_AIBNR_PROC_RES_OK,
    XCAM_MESSAGE_MAX,
};

static const char* MessageType2Str[XCAM_MESSAGE_MAX] = {
    [XCAM_MESSAGE_SOF_INFO_OK]         = "SOF_INFO",
    [XCAM_MESSAGE_ISP_STATS_OK]        = "ISP_STATS",
    [XCAM_MESSAGE_ISPP_STATS_OK]       = "ISPP_STATS",
    [XCAM_MESSAGE_ISP_POLL_SP_OK]      = "ISP_SP",
    [XCAM_MESSAGE_ISP_POLL_TX_OK]      = "ISP_TX",
    [XCAM_MESSAGE_ISP_GAIN_OK]         = "ISP_GAIN",
    [XCAM_MESSAGE_ISPP_GAIN_KG_OK]     = "ISPP_KG_GAIN",
    [XCAM_MESSAGE_NR_IMG_OK]           = "NR_IMG",
    [XCAM_MESSAGE_AWB_PROC_RES_OK]     = "AWB_PROC_RES",
    [XCAM_MESSAGE_AE_PROC_RES_OK]      = "AE_PROC_RES",
    [XCAM_MESSAGE_AF_PROC_RES_OK]      = "AF_PROC_RES",
    [XCAM_MESSAGE_AEC_STATS_OK]        = "AEC_STATS",
    [XCAM_MESSAGE_AWB_STATS_OK]        = "AWB_STATS",
    [XCAM_MESSAGE_AF_STATS_OK]         = "AF_STATS",
    [XCAM_MESSAGE_ATMO_STATS_OK]       = "ATMO_STATS",
    [XCAM_MESSAGE_ADEHAZE_STATS_OK]    = "ADHAZ_STATS",
    [XCAM_MESSAGE_AGAIN_STATS_OK]      = "AGAIN_STATS",
    [XCAM_MESSAGE_ISPP_GAIN_WR_OK]     = "ISPP_WR_GAIN",
    [XCAM_MESSAGE_AMD_PROC_RES_OK]     = "AMD_PROC_RES",
    [XCAM_MESSAGE_AE_PRE_RES_OK]       = "AE_PRE_RES",
    [XCAM_MESSAGE_PDAF_STATS_OK]       = "PDAF_STATS",
    [XCAM_MESSAGE_ORB_STATS_OK]        = "ORB_STATS",
    [XCAM_MESSAGE_BLC_PROC_RES_OK]     = "BLC_PROC_RES",
    [XCAM_MESSAGE_BLC_V32_PROC_RES_OK] = "BLC_V32_PROC_RES",
    [XCAM_MESSAGE_YNR_V3_PROC_RES_OK]  = "YNR_V3_PROC_RES",
    [XCAM_MESSAGE_YNR_V22_PROC_RES_OK] = "YNR_V22_PROC_RES",
    [XCAM_MESSAGE_YNR_V24_PROC_RES_OK] = "YNR_V24_PROC_RES",
    [XCAM_MESSAGE_YNR_PROC_RES_OK]     = "YNR_PROC_RES",
    [XCAM_MESSAGE_VICAP_POLL_SCL_OK]   = "VICAP_POLL_SCL",
    [XCAM_MESSAGE_TNR_STATS_OK]        = "TNR_STATS",
    [XCAM_MESSAGE_AIBNR_PROC_RES_OK]    = "AIBNR_PROC_RES",
};

typedef struct RkAiqGrpCondition_s {
    enum XCamMessageType cond;
    int8_t delay;
} RkAiqGrpCondition_t;

typedef struct RkAiqGrpConditions_s {
    RkAiqGrpCondition_t* conds;
    u32 size;
} RkAiqGrpConditions_t;

struct RkAiqAlgoDesCommExt {
    RkAiqAlgoDesComm* des;
    enum rk_aiq_core_analyze_type_e group;
    uint8_t algo_ver;
    uint8_t module_hw_ver;
    uint8_t handle_ver;
    RkAiqGrpConditions_t grpConds;
};

static const char* algo_str_map_array[RK_AIQ_ALGO_TYPE_MAX] = {
    // clang-format off
    [RK_AIQ_ALGO_TYPE_AE]       = "Ae",
    [RK_AIQ_ALGO_TYPE_AWB]=         "Awb",
    [RK_AIQ_ALGO_TYPE_AF]  =        "Af"        ,
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_ABLC]     =   "Blc"      ,
#else
    [ RK_AIQ_ALGO_TYPE_ABLC]        "Ablc"     ,
#endif
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_ADPCC]=       "Dpcc"   ,
#else
    [RK_AIQ_ALGO_TYPE_ADPCC]=       "Adpcc"      ,
#endif
    [RK_AIQ_ALGO_TYPE_AMERGE]=      "Merge"     ,
    [RK_AIQ_ALGO_TYPE_ATMO]=        "Atmo"       ,
    [RK_AIQ_ALGO_TYPE_ANR]=         "Anr"        ,
    [RK_AIQ_ALGO_TYPE_ALSC]=        "Alsc"       ,
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_AGIC]=        "Gic"       ,
#else
    [RK_AIQ_ALGO_TYPE_AGIC]=        "Agic"       ,
#endif
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_ADEBAYER]=    "Dm"   ,
#else
    [RK_AIQ_ALGO_TYPE_ADEBAYER]=    "Adebayer"   ,
#endif
    [RK_AIQ_ALGO_TYPE_ACCM]=        "Ccm"       ,
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_AGAMMA]=      "Gamma"      ,
#else
    [RK_AIQ_ALGO_TYPE_AGAMMA]=      "Agamma"     ,
#endif
    [RK_AIQ_ALGO_TYPE_AWDR]=        "Awdr"       ,
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_ADHAZ]=      "Dehaze"      ,
#else
    [RK_AIQ_ALGO_TYPE_ADHAZ]=      "Adehaze"     ,
#endif
    [RK_AIQ_ALGO_TYPE_A3DLUT]=      "3dlut"     ,
    [RK_AIQ_ALGO_TYPE_ALDCH]=       "Ldch"      ,
    [RK_AIQ_ALGO_TYPE_ACSM]=        "Csm"       ,
    [RK_AIQ_ALGO_TYPE_ACP]=         "Cp"        ,
    [RK_AIQ_ALGO_TYPE_AIE]=         "Ie"        ,
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_ASHARP]=      "Sharp"      ,
#else
    [RK_AIQ_ALGO_TYPE_ASHARP]=      "Asharp"     ,
#endif
    [RK_AIQ_ALGO_TYPE_AORB]=        "Aorb"       ,
    [RK_AIQ_ALGO_TYPE_ACGC]=        "Cgc"       ,
    [RK_AIQ_ALGO_TYPE_ASD]=         "Asd"        ,
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_ADRC]=        "Drc"       ,
#else
    [RK_AIQ_ALGO_TYPE_ADRC]=        "Adrc"       ,
#endif
    [RK_AIQ_ALGO_TYPE_ADEGAMMA]=    "Adegamma"   ,
#if USE_NEWSTRUCT
    [RK_AIQ_ALGO_TYPE_ARAWNR]=      "Abayer2dnr" ,
    [RK_AIQ_ALGO_TYPE_AMFNR]=       "Btnr"       ,
    [RK_AIQ_ALGO_TYPE_AYNR]=        "Ynr"        ,
    [RK_AIQ_ALGO_TYPE_ACNR]=        "Cnr"        ,
#else
#if defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE) || defined(ISP_HW_V35)
    [RK_AIQ_ALGO_TYPE_ARAWNR]=      "Abayer2dnr" ,
    [RK_AIQ_ALGO_TYPE_AMFNR]=       "Abayertnr"  ,
#else
    [RK_AIQ_ALGO_TYPE_ARAWNR]=      "Arawnr"     ,
    [RK_AIQ_ALGO_TYPE_AMFNR]=       "Amfnr"      ,
#endif
    [RK_AIQ_ALGO_TYPE_AYNR]=        "Aynr"       ,
    [RK_AIQ_ALGO_TYPE_ACNR]=        "Acnr"       ,
#endif
    [RK_AIQ_ALGO_TYPE_AEIS]=        "Aeis"       ,
    [RK_AIQ_ALGO_TYPE_AFEC]=        "Afec"       ,
#if defined(ISP_HW_V39)
    [RK_AIQ_ALGO_TYPE_AMD]=         "Yme"        ,
#else
    [RK_AIQ_ALGO_TYPE_AMD]=         "Amd"        ,
#endif
    [RK_AIQ_ALGO_TYPE_AGAIN]=       "Gain"      ,
    [RK_AIQ_ALGO_TYPE_ACAC]=        "Cac"       ,
    [RK_AIQ_ALGO_TYPE_AFD]=         "Afd"        ,
    [RK_AIQ_ALGO_TYPE_ARGBIR]=      "Rgbir"     ,
    [RK_AIQ_ALGO_TYPE_ALDC]=        "Aldc"       ,
    [RK_AIQ_ALGO_TYPE_AHISTEQ]=        "Histeq"       ,
    [RK_AIQ_ALGO_TYPE_AENH]=        "Enh"       ,
    [RK_AIQ_ALGO_TYPE_AHSV]=        "Hsv"       ,
    [RK_AIQ_ALGO_TYPE_AIBNR]=        "Aibnr"       ,
    [RK_AIQ_ALGO_TYPE_AIRMS]=        "Airms"       ,
    [RK_AIQ_ALGO_TYPE_AIYNR]=        "Aiynr"       ,
    // clang-format oon
};


static const char* AlgoTypeToString(RkAiqAlgoType_t type) {
    return algo_str_map_array[type];
}

#ifdef ISP_HW_V20
#include "AiqCoreConfigV20.h"
#endif

#ifdef ISP_HW_V21
#include "AiqCoreConfigV21.h"
#endif

#ifdef ISP_HW_V30
#include "AiqCoreConfigV30.h"
#endif

#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
#include "AiqCoreConfigV32.h"
#endif

#if defined(ISP_HW_V39)
#include "RkAiqCoreConfigV39.h"
#endif

#if defined(ISP_HW_V33)
#include "RkAiqCoreConfigV33.h"
#endif

#if defined(ISP_HW_V35)
#include "RkAiqCoreConfigV35.h"
#endif

#endif
