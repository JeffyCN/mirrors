/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _AIQ_TYPES_PRIV_C_H_
#define _AIQ_TYPES_PRIV_C_H_

#include "algo_types_priv.h"
#include "algos/ae/rk_aiq_types_ae_algo_int.h"
#include "algos/af/rk_aiq_types_af_algo.h"
#include "c_base/aiq_base.h"
#include "common/rk-isp35-config.h"
#include "common/rk-isp33-config.h"
#include "common/rk-isp39-config.h"
#include "common/rk_aiq_types.h"
#include "common/rkisp2-config.h"
#include "common/rkisp21-config.h"
#include "common/rkisp3-config.h"
#include "common/rkisp32-config.h"
#include "common/rk-aiisp-config.h"
#include "xcore_c/aiq_video_buffer.h"
#if defined(ISP_HW_V39)
typedef awbStats_cfg_priv_t rk_aiq_isp_awb_meas_cfg_v39_t;
typedef awbStats_stats_priv_t rk_aiq_isp_awb_stats_v39_t;
#elif defined(ISP_HW_V33)
typedef awbStats_cfg_priv_t rk_aiq_isp_awb_meas_cfg_v33_t;
typedef awbStats_stats_priv_t rk_aiq_isp_awb_stats_v39_t;
#elif defined(ISP_HW_V35)
typedef awbStats_cfg_priv_t rk_aiq_isp_awb_meas_cfg_v33_t;
typedef awbStats_stats_priv_t rk_aiq_isp_awb_stats_v39_t;
#endif

typedef rk_aiq_isp_aec_meas_t rk_aiq_isp_aec_params_t;
typedef rk_aiq_isp_hist_meas_t rk_aiq_isp_hist_params_t;

#if USE_NEWSTRUCT
typedef dm_param_t                      rk_aiq_isp_dm_params_t;
typedef btnr_param_t                    rk_aiq_isp_btnr_params_t;
typedef gamma_param_t                   rk_aiq_isp_gamma_params_t;
typedef ynr_param_t                     rk_aiq_isp_ynr_params_t;
typedef sharp_param_t                   rk_aiq_isp_sharp_params_t;
#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
typedef texEst_param_t                   rk_aiq_isp_texEst_params_t;
#endif
typedef cnr_param_t                     rk_aiq_isp_cnr_params_t;
typedef rk_aiq_isp_drc_v39_t            rk_aiq_isp_drc_params_t;
typedef dpc_param_t                     rk_aiq_isp_dpcc_params_t;
typedef rk_aiq_isp_blc_v33_t            rk_aiq_isp_blc_params_t;
#if RKAIQ_HAVE_3DLUT
typedef lut3d_param_t                   rk_aiq_isp_lut3d_params_t;
#endif
typedef csm_param_t                     rk_aiq_isp_csm_params_t;
typedef afStats_cfg_t                   rk_aiq_isp_af_params_t;
typedef ccm_param_t                     rk_aiq_isp_ccm_params_t;
typedef aeStats_cfg_t                   rk_aiq_isp_ae_stats_cfg_t;
typedef gic_param_t                     rk_aiq_isp_gic_params_t;
#if RKAIQ_HAVE_YUVME
typedef yme_param_t                     rk_aiq_isp_yme_params_t;
#endif
typedef cac_param_t                     rk_aiq_isp_cac_params_t;
typedef ldc_param_t                     rk_aiq_isp_ldc_params_t;
typedef mge_param_t                     rk_aiq_isp_merge_params_t;
typedef lsc_param_t                     rk_aiq_isp_lsc_params_t;
#if RKAIQ_HAVE_RGBIR_REMOSAIC
typedef rgbir_param_t                   rk_aiq_isp_rgbir_params_t;
#endif
typedef cgc_param_t                     rk_aiq_isp_cgc_params_t;
typedef cp_param_t                      rk_aiq_isp_cp_params_t;
typedef ie_param_t                      rk_aiq_isp_ie_params_t;
typedef gain_param_t                    rk_aiq_isp_gain_params_t;
#if RKAIQ_HAVE_DEHAZE_V14
typedef dehaze_param_t                  rk_aiq_isp_dehaze_params_t;
#endif
typedef histeq_param_t                  rk_aiq_isp_histeq_params_t;
#if RKAIQ_HAVE_ENHANCE_V10
typedef enh_param_t                  rk_aiq_isp_enh_params_t;
#endif
#if RKAIQ_HAVE_HSV
typedef hsv_param_t                  rk_aiq_isp_hsv_params_t;
#endif
typedef rk_aiq_isp_degamma_t            rk_aiq_isp_adegamma_params_t;
typedef rk_aiq_isp_afd_t                rk_aiq_isp_afd_params_t;
typedef rk_aiq_isp_fec_t                rk_aiq_isp_fec_params_t;
typedef rk_aiq_isp_wdr_t                rk_aiq_isp_wdr_params_t;

typedef awbStats_cfg_priv_t             rk_aiq_isp_awb_params_t;
typedef rk_aiq_isp_wb_gain_v32_t        rk_aiq_isp_awb_gain_params_t;
typedef rk_aiq_isp_baynr_v32_t rk_aiq_isp_baynr_params_t;
#endif

typedef struct aiq_params_base_s aiq_params_base_t;

struct aiq_params_base_s {
    /* should be the first member */
    union {
        struct {
            bool is_update;
            bool en;
            bool bypass;
        } __attribute__ ((packed));
        char aligned[4]; // for aligned to 4
    };
    uint32_t frame_id;
    uint32_t sync_flag;
    int type;
    aiq_ref_base_t _ref_base;
    char _data[]; /* Flexible Array, C99 feature */
};

#define MALLOC_ISP_PARAMS_WITH_HEAD(type)                                                   \
    ({                                                                                      \
        void* ret = aiq_mallocz(XCAM_ALIGN_UP(sizeof(aiq_params_base_t), 4) + sizeof(type)); \
        ret;                                                                                \
    })

#define CALC_SIZE_WITH_HEAD(type_h, type_v) (sizeof(type_h) + sizeof(type_v))

#define SET_HEAD_DATA_PTR(head)

#define AIQ_PARAMS_BASE_RESET(param) \
    (param)->is_update = false;      \
    (param)->sync_flag = (uint32_t)(-2)

#define AIQ_PARAMS_BASE_INIT(param)                \
    memset((param), 0, sizeof(aiq_params_base_t)); \
    AIQ_PARAMS_BASE_RESET(param);

typedef struct AiqAecExpInfoWrapper_s {
    rk_aiq_exposure_params_t new_ae_exp;
    RKAiqExpI2cParam_t exp_i2c_params;
    AecProcResult_t      ae_proc_res_rk;
    Sensor_dpcc_res_t SensorDpccInfo;
    int algo_id;
} AiqAecExpInfoWrapper_t;

#define AIQ_AEC_EXPINFO_WRAPPER_INIT(param) memset((param), 0, sizeof(AiqAecExpInfoWrapper_t));

typedef struct AiqSensorExpInfo_s {
    aiq_params_base_t _base;
    rk_aiq_exposure_params_t aecExpInfo;
    Sensor_dpcc_res_t SensorDpccInfo;
    RKAiqExpI2cParam_t* exp_i2c_params;
    uint64_t sofTime;
} AiqSensorExpInfo_t;

#define AIQ_SENSOR_EXPINFO_INIT(param) \
        memset((param), 0, sizeof(AiqSensorExpInfo_t));

#define AIQ_EXP_PARAMS_WRAPPER_INIT(param) memset((param), 0, sizeof(AiqAecExpInfoWrapper_t));

typedef AiqSensorExpInfo_t aiq_sensor_exp_info_t;

#ifdef ISP_HW_V20
typedef struct aiq_isp_effect_params_s {
    aiq_ref_base_t _ref_base;
    struct isp2x_isp_params_cfg isp_params;
    rk_aiq_awb_stat_cfg_v200_t awb_cfg;
} aiq_isp_effect_params_t;
#elif ISP_HW_V21
typedef struct aiq_isp_effect_params_s {
    aiq_ref_base_t _ref_base;
    struct isp21_isp_params_cfg isp_params_v21;
    rk_aiq_awb_stat_cfg_v201_t awb_cfg_v201;
    rk_aiq_isp_blc_v21_t blc_cfg;
} aiq_isp_effect_params_t;
#elif ISP_HW_V30
typedef struct aiq_isp_effect_params_s {
    aiq_ref_base_t _ref_base;
    struct isp3x_isp_params_cfg isp_params_v3x[3];
    rk_aiq_isp_awb_meas_cfg_v3x_t awb_cfg_v3x;
    rk_aiq_isp_blc_v21_t blc_cfg;
} aiq_isp_effect_params_t;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
typedef struct aiq_isp_effect_params_s {
    aiq_ref_base_t _ref_base;
#if defined(RKAIQ_HAVE_MULTIISP)
    struct isp32_isp_params_cfg isp_params_v32[4];
#endif
    struct isp32_isp_meas_cfg meas;
    struct isp32_bls_cfg bls_cfg;
    struct isp32_awb_gain_cfg awb_gain_cfg;
    rk_aiq_awb_stat_cfg_v32_t awb_cfg_v32;
} aiq_isp_effect_params_t;
#elif defined(ISP_HW_V39)
typedef struct aiq_isp_effect_params_s {
    aiq_ref_base_t _ref_base;
#if defined(RKAIQ_HAVE_MULTIISP)
    struct isp39_isp_params_cfg isp_params_v39[2];
#endif
    struct isp39_isp_meas_cfg meas;
    struct isp32_bls_cfg bls_cfg;
    struct isp32_awb_gain_cfg awb_gain_cfg;
    awbStats_cfg_priv_t awb_cfg_v39;
#if defined(USE_NEWSTRUCT)
    aeStats_cfg_t ae_cfg_v39;
#endif
} aiq_isp_effect_params_t;
#elif defined(ISP_HW_V33)
typedef struct aiq_isp_effect_params_s {
    aiq_ref_base_t _ref_base;
#if defined(RKAIQ_HAVE_MULTIISP)
    struct isp33_isp_params_cfg isp_params_v33[2];
#endif
    struct isp33_isp_meas_cfg meas;
    struct isp32_bls_cfg bls_cfg;
    int blc0_diff;
    float offset2Blc1;
    struct isp32_awb_gain_cfg awb_gain_cfg;
    awbStats_cfg_priv_t awb_cfg_v33;
#if defined(USE_NEWSTRUCT)
    aeStats_cfg_t ae_cfg_v39;
#endif
} aiq_isp_effect_params_t;
#elif defined(ISP_HW_V35)
typedef struct aiq_isp_effect_params_s {
    aiq_ref_base_t _ref_base;
#if defined(RKAIQ_HAVE_MULTIISP)
    struct isp35_isp_params_cfg isp_params_v35[2];
#endif
    struct isp35_isp_meas_cfg meas;
    struct isp35_bls_cfg bls_cfg;
    struct isp32_awb_gain_cfg awb_gain_cfg;
    awbStats_cfg_priv_t awb_cfg_v35;
#if defined(USE_NEWSTRUCT)
    aeStats_cfg_t ae_cfg_v39;
#endif
} aiq_isp_effect_params_t;
#else
typedef struct aiq_isp_effect_params_s {
    aiq_ref_base_t _ref_base;
    union {
        struct isp2x_isp_params_cfg isp_params;
        struct isp21_isp_params_cfg isp_params_v21;
        struct isp3x_isp_params_cfg isp_params_v3x[3];
        struct isp32_isp_params_cfg isp_params_v32;
    };
    union {
        rk_aiq_awb_stat_cfg_v200_t awb_cfg;
        rk_aiq_awb_stat_cfg_v201_t awb_cfg_v201;
        rk_aiq_isp_awb_meas_cfg_v3x_t awb_cfg_v3x;
        rk_aiq_awb_stat_cfg_v32_t awb_cfg_v32;
    };
    rk_aiq_isp_blc_v21_t blc_cfg;
} aiq_isp_effect_params_t;
#endif

typedef struct AiqAfInfoWrapper_s {
    aiq_ref_base_t _ref_base;
    struct timeval focusStartTim;
    struct timeval focusEndTim;
    struct timeval zoomStartTim;
    struct timeval zoomEndTim;
    int64_t sofTime;
    int32_t focusCode;
    int32_t zoomCode;
    float angleZ;
    uint32_t lowPassId;
    int32_t lowPassFv4_4[RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t lowPassFv8_8[RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t lowPassHighLht[RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t lowPassHighLht2[RKAIQ_RAWAF_SUMDATA_NUM];
    bool zoomCorrection;
    bool focusCorrection;
} AiqAfInfoWrapper_t;

typedef struct RkAiqPirisInfoWrapper_s {
    int step;
    int laststep;
    bool update;
    struct timeval StartTim;
    struct timeval EndTim;
} RkAiqPirisInfoWrapper_t;

typedef struct AiqIrisInfoWrapper_s {
    // TODO Merge
    // RkAiqIrisType_t           IrisType;
    RkAiqPirisInfoWrapper_t PIris;
    RkAiqDCIrisParam_t DCIris;
    RkAiqHDCIrisParam_t HDCIris;
    uint64_t sofTime;
} AiqIrisInfoWrapper_t;

typedef struct AiqSofInfoWrapper_s {
    uint32_t sequence;
    aiq_sensor_exp_info_t* preExp;
    aiq_sensor_exp_info_t* curExp;
    aiq_sensor_exp_info_t* nxtExp;
    int64_t sof;
    int iso;
} AiqSofInfoWrapper_t;

struct aiq_shared_base_s {
    aiq_ref_base_t _ref_base;
    int type;
    bool bValid;
    uint32_t frame_id;
    char _data[];
};

typedef struct aiq_shared_base_s aiq_stats_base_t;

typedef struct aiq_awb_stats_wrapper_s {
    union {
        rk_aiq_awb_stat_res_v200_t awb_stats;
        rk_aiq_awb_stat_res_v201_t awb_stats_v201;
        rk_aiq_isp_awb_stats_v3x_t awb_stats_v3x;
        rk_aiq_isp_awb_stats_v32_t awb_stats_v32;
        awbStats_stats_priv_t awb_stats_v39;
    };
    bool awb_cfg_effect_valid;
    rk_aiq_isp_blc_t blc_cfg_effect;
} aiq_awb_stats_wrapper_t;

typedef struct aiq_ae_stats_wrapper_s {
    union {
        rk_aiq_isp_aec_stats_t aec_stats;
        RKAiqAecStatsV25_t     aec_stats_v25;
    };
    bool af_prior;
} aiq_ae_stats_wrapper_t;

typedef struct aiq_af_stats_wrapper_s {
    union {
        rk_aiq_isp_af_stats_t af_stats;
        rk_aiq_isp_af_stats_v3x_t af_stats_v3x;
#ifdef USE_NEWSTRUCT
        afStats_stats_t afStats_stats;
#endif
    };
    rk_aiq_af_algo_motor_stat_t stat_motor;
    RKAiqAecExpInfo_t aecExpInfo;
} aiq_af_stats_wrapper_t;

// rk_aiq_isp_pdaf_stats_t pdaf_stats;
// rk_aiq_again_stat_t again_stats;
// rkisp_adehaze_stats_t adehaze_stats;
typedef struct isp39_bay3d_stat rk_aiq_bay3d_stat_t;

#define MALLOC_ISP_STATS_WITH_HEAD(type)                                                            \
    ({                                                                                              \
        void* ret = aiq_mallocz(XCAM_ALIGN_UP(sizeof(aiq_stats_base_t), 4) + sizeof(type));          \
        ((aiq_stats_base_t*)ret)->bValid = false;                                                   \
        ret;                                                                                        \
    })

typedef struct rk_aiq_scale_raw_info_s {
    uint8_t             bpp;
    uint8_t             flags;
    aiq_VideoBuffer_t* raw_s;
    aiq_VideoBuffer_t* raw_m;
    aiq_VideoBuffer_t* raw_l;
} rk_aiq_scale_raw_info_t;

typedef enum RkAiqVicapRawBufFlag_e {
    RK_AIQ_VICAP_SCALE_HDR_MODE_NORMAL = 0x1,
    RK_AIQ_VICAP_SCALE_HDR_MODE_2_HDR  = 0x3,
    RK_AIQ_VICAP_SCALE_HDR_MODE_3_HDR  = 0x7,
} RkAiqVicapRawBufFlag_t;

#if 0
typedef enum _cam3aResultType {
    RESULT_TYPE_INVALID = -1,
    RESULT_TYPE_EXPOSURE_PARAM,
    RESULT_TYPE_AEC_PARAM,
    RESULT_TYPE_HIST_PARAM,
    RESULT_TYPE_AWB_PARAM,
    RESULT_TYPE_AWBGAIN_PARAM,
    RESULT_TYPE_AF_PARAM,
    RESULT_TYPE_DPCC_PARAM,
    RESULT_TYPE_MERGE_PARAM,
    RESULT_TYPE_TMO_PARAM,
    RESULT_TYPE_CCM_PARAM,
    RESULT_TYPE_LSC_PARAM,
    RESULT_TYPE_BLC_PARAM,
    RESULT_TYPE_RAWNR_PARAM,
    RESULT_TYPE_GIC_PARAM,
    RESULT_TYPE_DEBAYER_PARAM,
    RESULT_TYPE_LDCH_PARAM,
    RESULT_TYPE_LUT3D_PARAM = 0x10,
    RESULT_TYPE_DEHAZE_PARAM,
    RESULT_TYPE_AGAMMA_PARAM,
    RESULT_TYPE_ADEGAMMA_PARAM,
    RESULT_TYPE_WDR_PARAM,
    RESULT_TYPE_CSM_PARAM,
    RESULT_TYPE_CGC_PARAM,
    RESULT_TYPE_CONV422_PARAM,
    RESULT_TYPE_YUVCONV_PARAM,
    RESULT_TYPE_GAIN_PARAM,
    RESULT_TYPE_CP_PARAM,
    RESULT_TYPE_IE_PARAM,
    RESULT_TYPE_MOTION_PARAM,
    RESULT_TYPE_IRIS_PARAM,
    RESULT_TYPE_CPSL_PARAM,
    RESULT_TYPE_FLASH_PARAM,

    RESULT_TYPE_TNR_PARAM = 0x20,
    RESULT_TYPE_YNR_PARAM,
    RESULT_TYPE_UVNR_PARAM,
    RESULT_TYPE_SHARPEN_PARAM,
    RESULT_TYPE_EDGEFLT_PARAM,
    RESULT_TYPE_FEC_PARAM,
    RESULT_TYPE_ORB_PARAM,

    RESULT_TYPE_FOCUS_PARAM,

    // isp21 result
    RESULT_TYPE_DRC_PARAM = 0x28,
    // isp3x result
    RESULT_TYPE_CAC_PARAM = 0x29,
    RESULT_TYPE_AFD_PARAM = 0x2a,
    // isp39 result
    RESULT_TYPE_RGBIR_PARAM = 0x2b,
    RESULT_TYPE_TRANS_PARAM = 0x2c,
    RESULT_TYPE_LDC_PARAM,
    RESULT_TYPE_MAX_PARAM,
} cam3aResultType;
#endif

enum rk_aiq_core_analyze_type_e {
    RK_AIQ_CORE_ANALYZE_MEAS,
    RK_AIQ_CORE_ANALYZE_OTHER,
    RK_AIQ_CORE_ANALYZE_AMD,
    RK_AIQ_CORE_ANALYZE_THUMBNAILS,
    RK_AIQ_CORE_ANALYZE_LSC,
    RK_AIQ_CORE_ANALYZE_AE,
    RK_AIQ_CORE_ANALYZE_AMFNR,
    RK_AIQ_CORE_ANALYZE_AYNR,
    RK_AIQ_CORE_ANALYZE_AWB,
    RK_AIQ_CORE_ANALYZE_DHAZ,
    RK_AIQ_CORE_ANALYZE_GRP0,
    RK_AIQ_CORE_ANALYZE_GRP1,
    RK_AIQ_CORE_ANALYZE_AF,
    RK_AIQ_CORE_ANALYZE_EIS,
    RK_AIQ_CORE_ANALYZE_ORB,
    RK_AIQ_CORE_ANALYZE_AFD,
    RK_AIQ_CORE_ANALYZE_MAX,
    RK_AIQ_CORE_ANALYZE_ALL = 0xffffffff,
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
static const char* Cam3aResultType2Str[RESULT_TYPE_MAX_PARAM] = {
    [RESULT_TYPE_EXPOSURE_PARAM] = "EXPOSURE",
    [RESULT_TYPE_AEC_PARAM]      = "AEC",
    [RESULT_TYPE_HIST_PARAM]     = "HIST",
    [RESULT_TYPE_AWB_PARAM]      = "AWB",
    [RESULT_TYPE_AWBGAIN_PARAM]  = "AWB_GAIN",
    [RESULT_TYPE_AF_PARAM]       = "AF",
#ifdef USE_NEWSTRUCT
    [RESULT_TYPE_DPCC_PARAM]     = "DPC",
#else
    [RESULT_TYPE_DPCC_PARAM]     = "DPCC",
#endif
    [RESULT_TYPE_MERGE_PARAM]    = "MERGE",
    [RESULT_TYPE_TMO_PARAM]      = "TMO",
    [RESULT_TYPE_CCM_PARAM]      = "CCM",
    [RESULT_TYPE_LSC_PARAM]      = "LSC",
    [RESULT_TYPE_BLC_PARAM]      = "BLC",
    [RESULT_TYPE_RAWNR_PARAM]    = "RAWNR",
    [RESULT_TYPE_GIC_PARAM]      = "GIC",
    [RESULT_TYPE_DEBAYER_PARAM]  = "DEBAYER",
    [RESULT_TYPE_LDCH_PARAM]     = "LDCH",
    [RESULT_TYPE_LUT3D_PARAM]    = "LUT3D",
    [RESULT_TYPE_DEHAZE_PARAM]   = "DEHAZE",
    [RESULT_TYPE_HISTEQ_PARAM]   = "HISTEQ",
    [RESULT_TYPE_ENH_PARAM]   = "ENH",
    [RESULT_TYPE_HSV_PARAM]   = "HSV",
    [RESULT_TYPE_AGAMMA_PARAM]   = "AGAMMA",
    [RESULT_TYPE_ADEGAMMA_PARAM] = "ADEGAMMA",
    [RESULT_TYPE_WDR_PARAM]      = "WDR",
    [RESULT_TYPE_CSM_PARAM]      = "CSM",
    [RESULT_TYPE_CGC_PARAM]      = "CGC",
    [RESULT_TYPE_CONV422_PARAM]  = "CONV22",
    [RESULT_TYPE_YUVCONV_PARAM]  = "YUVCONV",
    [RESULT_TYPE_GAIN_PARAM]     = "GAIN",
    [RESULT_TYPE_CP_PARAM]       = "CP",
    [RESULT_TYPE_IE_PARAM]       = "IE",
    [RESULT_TYPE_MOTION_PARAM]   = "MOTION",
    [RESULT_TYPE_IRIS_PARAM]     = "IRIS",
    [RESULT_TYPE_CPSL_PARAM]     = "CPSL",
    [RESULT_TYPE_FLASH_PARAM]    = "FLASH",
    [RESULT_TYPE_TNR_PARAM]      = "TNR",
    [RESULT_TYPE_YNR_PARAM]      = "YNR",
    [RESULT_TYPE_UVNR_PARAM]     = "UVNR",
    [RESULT_TYPE_SHARPEN_PARAM]  = "SHARPEN",
    [RESULT_TYPE_EDGEFLT_PARAM]  = "EDGEFLT",
    [RESULT_TYPE_FEC_PARAM]      = "FEC",
    [RESULT_TYPE_ORB_PARAM]      = "ORB",
    [RESULT_TYPE_FOCUS_PARAM]    = "FOCUS",
    [RESULT_TYPE_DRC_PARAM]      = "DRC",
    [RESULT_TYPE_CAC_PARAM]      = "CAC",
    [RESULT_TYPE_AFD_PARAM]      = "AFD",
    [RESULT_TYPE_RGBIR_PARAM]    = "RGBIR",
    [RESULT_TYPE_TRANS_PARAM]    = "TRANS",
    [RESULT_TYPE_LDC_PARAM]      = "LDC",
    [RESULT_TYPE_AESTATS_PARAM]  = "AEC",
    [RESULT_TYPE_TEXEST_PARAM]   = "TEXEST",
    [RESULT_TYPE_POSTISP_PARAM]  = "POSTISP",
    [RESULT_TYPE_AIBNR_PARAM]     = "AIBNR",
    [RESULT_TYPE_AIRMS_PARAM]     = "AIRMS",
    [RESULT_TYPE_AIYNR_PARAM]     = "AIYNR",
};

static const char* AnalyzerGroupType2Str[RK_AIQ_CORE_ANALYZE_MAX] = {
    [RK_AIQ_CORE_ANALYZE_MEAS] = "GRP_MEAS",   [RK_AIQ_CORE_ANALYZE_OTHER] = "GRP_OTHER",
    [RK_AIQ_CORE_ANALYZE_AMD] = "GRP_AMD",     [RK_AIQ_CORE_ANALYZE_THUMBNAILS] = "GRP_THUMBNAILS",
    [RK_AIQ_CORE_ANALYZE_LSC] = "GRP_LSC",     [RK_AIQ_CORE_ANALYZE_AE] = "GRP_AE",
    [RK_AIQ_CORE_ANALYZE_AMFNR] = "GRP_AMFNR", [RK_AIQ_CORE_ANALYZE_AYNR] = "GRP_AYNR",
    [RK_AIQ_CORE_ANALYZE_AWB] = "GRP_AWB",     [RK_AIQ_CORE_ANALYZE_DHAZ] = "DHAZ",
    [RK_AIQ_CORE_ANALYZE_GRP0] = "GRP0",       [RK_AIQ_CORE_ANALYZE_GRP1] = "GRP1",
    [RK_AIQ_CORE_ANALYZE_AF] = "AF",           [RK_AIQ_CORE_ANALYZE_EIS] = "EIS",
    [RK_AIQ_CORE_ANALYZE_ORB] = "ORB",         [RK_AIQ_CORE_ANALYZE_AFD] = "AFD",
};
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

typedef struct AiqFullParams_s {
    aiq_params_base_t _base;
    aiq_params_base_t* pParamsArray[RESULT_TYPE_MAX_PARAM];
} AiqFullParams_t;

typedef enum _RkAiqIspUniteMode {
    RK_AIQ_ISP_UNITE_MODE_NORMAL    = 0,
    RK_AIQ_ISP_UNITE_MODE_TWO_GRID  = 1,
    RK_AIQ_ISP_UNITE_MODE_FOUR_GRID = 2,
} RkAiqIspUniteMode;

#if defined(ISP_HW_V32_LITE)
#define RK_AIQ_ISP_CIF_INPUT_MAX_SIZE 4224 * 3136
#elif defined(ISP_HW_V32)
#define RK_AIQ_ISP_CIF_INPUT_MAX_SIZE 3072 * 1728
#elif defined(ISP_HW_V30)
#define RK_AIQ_ISP_CIF_INPUT_MAX_SIZE 3840 * 2160
#elif defined(ISP_HW_V33)
#define RK_AIQ_ISP_CIF_INPUT_MAX_SIZE 2880 * 1620
#elif defined(ISP_HW_V35)
#define RK_AIQ_ISP_CIF_INPUT_MAX_SIZE 2880 * 1620
#else
#define RK_AIQ_ISP_CIF_INPUT_MAX_SIZE 3840 * 2160
#endif

typedef struct rk_aiq_tb_info_s {
    bool is_fastboot;
    bool is_start_again;
    float pixel_clock_freq_mhz;
} rk_aiq_tb_info_t;

typedef struct aiq_shared_base_s AlgoRstShared_t;

typedef enum rk_aiq_drv_share_mem_type_e {
    MEM_TYPE_LDCH,
    MEM_TYPE_FEC,
    MEM_TYPE_CAC,
    MEM_TYPE_DBG_INFO,
    MEM_TYPE_LDCV,
    MEM_TYPE_BTNR,
} rk_aiq_drv_share_mem_type_t;

typedef void (*alloc_mem_t)(uint8_t id, void* ops_ctx, void* cfg, void** mem_ctx);
typedef void (*release_mem_t)(uint8_t id, void* mem_ctx);
typedef void* (*get_free_item_t)(uint8_t id, void* mem_ctx);
typedef struct isp_drv_share_mem_ops_s {
    alloc_mem_t alloc_mem;
    release_mem_t release_mem;
    get_free_item_t get_free_item;
} isp_drv_share_mem_ops_t;

typedef struct rk_aiq_lut_share_mem_info_s {
    int32_t size;
    void* map_addr;
    void* addr;
    void* addr1;
    int32_t fd;
    char* state;
} rk_aiq_lut_share_mem_info_t;

typedef rk_aiq_lut_share_mem_info_t rk_aiq_ldch_share_mem_info_t;
typedef rk_aiq_lut_share_mem_info_t rk_aiq_ldcv_share_mem_info_t;
typedef rk_aiq_lut_share_mem_info_t rk_aiq_cac_share_mem_info_t;
typedef rk_aiq_lut_share_mem_info_t rk_aiq_dbg_share_mem_info_t;
typedef rk_aiq_lut_share_mem_info_t rk_aiq_cac_share_mem_info_t;
typedef rk_aiq_lut_share_mem_info_t rk_aiq_btnr_share_mem_info_t;

typedef struct rk_aiq_fec_share_mem_info_s {
    int size;
    int fd;
    void* map_addr;
    unsigned char* meshxf;
    unsigned char* meshyf;
    unsigned short* meshxi;
    unsigned short* meshyi;
    char* state;
} rk_aiq_fec_share_mem_info_t;

typedef struct rk_aiq_share_mem_alloc_param_s {
    int width;
    int height;
    char reserved[8];
} rk_aiq_share_mem_alloc_param_t;

typedef struct rk_aiq_share_mem_config_s {
    rk_aiq_drv_share_mem_type_t mem_type;
    rk_aiq_share_mem_alloc_param_t alloc_param;
} rk_aiq_share_mem_config_t;

typedef enum CamThreadType_e {
    ISP_POLL_LUMA,
    ISP_POLL_3A_STATS,
    ISP_POLL_PARAMS,
    ISPP_POLL_FEC_PARAMS,
    ISPP_POLL_TNR_PARAMS,
    ISPP_POLL_NR_PARAMS,
    ISPP_POLL_TNR_STATS,
    ISPP_POLL_NR_STATS,
    ISP_POLL_SOF,
    ISP_POLL_TX,
    ISP_POLL_RX,
    ISP_POLL_SP,
    ISP_POLL_PDAF_STATS,
    ISP_GAIN,
    ISP_NR_IMG,
    ISPP_GAIN_KG,
    ISPP_GAIN_WR,
    ISP_POLL_ISPSTREAMSYNC,
    VICAP_STREAM_ON_EVT,
    VICAP_RESET_EVT,
    VICAP_WITH_RK1608_RESET_EVT,
    VICAP_POLL_SCL,
    ISP_POLL_AIISP,
    ISP_POLL_AIBNR_DONE,
    ISP_POLL_RKNN_DONE,
    ISP_POLL_AIRMS_DONE,
    ISP_POLL_AIYNR_DONE,
    ISP_POLL_MEMC,
    ISP_POLL_POST_MAX,
} CamThreadType_t;

typedef struct FrameDumpInfo_s {
    uint64_t timestamp;
    uint32_t interval;
    uint32_t delay;
    uint32_t id;
    uint32_t frameloss;
} FrameDumpInfo_t;

#endif
