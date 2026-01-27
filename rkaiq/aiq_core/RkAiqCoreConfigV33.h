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
#ifndef RK_AIQ_CORE_CONFIG_V33_H
#define RK_AIQ_CORE_CONFIG_V33_H

#include "awb/rk_aiq_algo_awb_itf.h"
#include "awdr/rk_aiq_algo_awdr_itf.h"
#include "ae/rk_aiq_algo_ae_itf.h"
#include "afd/rk_aiq_algo_afd_itf.h"
#include "newStruct/demosaic/include/demosaic_algo_api.h"
#include "newStruct/bayertnr/include/bayertnr_algo_api.h"
#include "newStruct/gamma/include/gamma_algo_api.h"
#include "newStruct/ynr/include/ynr_algo_api.h"
#include "newStruct/drc/include/drc_algo_api.h"
#include "newStruct/blc/include/blc_algo_api.h"
#include "newStruct/dpc/include/dpc_algo_api.h"
#include "newStruct/cac/include/cac_algo_api.h"
#include "newStruct/csm/include/csm_algo_api.h"
#include "newStruct/merge/include/merge_algo_api.h"
#include "newStruct/lsc/include/lsc_algo_api.h"
#include "newStruct/cgc/include/cgc_algo_api.h"
#include "newStruct/cp/include/cp_algo_api.h"
#include "newStruct/ie/include/ie_algo_api.h"
#include "newStruct/gain/include/gain_algo_api.h"
#include "newStruct/ccm/include/ccm_algo_api.h"
#include "newStruct/histeq/include/histeq_algo_api.h"
#include "newStruct/enh/include/enh_algo_api.h"
#include "newStruct/gic/include/gic_algo_api.h"
#include "newStruct/hsv/include/hsv_algo_api.h"
#include "newStruct/ldc/include/ldc_algo_api.h"

#if RKAIQ_ENABLE_CAMGROUP
#include "algos_camgroup/ae/rk_aiq_algo_camgroup_ae_itf.h"
#include "algos_camgroup/awb/rk_aiq_algo_camgroup_awb_itf.h"
#include "algos_camgroup/newStruct/bayertnr/rk_aiq_algo_camgroup_btnr.h"
#include "algos_camgroup/newStruct/drc/rk_aiq_algo_camgroup_drc.h"
#include "algos_camgroup/newStruct/sharp/rk_aiq_algo_camgroup_sharp.h"
#include "algos_camgroup/newStruct/ynr/rk_aiq_algo_camgroup_ynr.h"
#include "algos_camgroup/newStruct/blc/rk_aiq_algo_camgroup_blc.h"
#include "algos_camgroup/newStruct/lsc/rk_aiq_algo_camgroup_lsc.h"
#include "algos_camgroup/newStruct/gain/rk_aiq_algo_camgroup_gain.h"
#include "algos_camgroup/newStruct/ccm/rk_aiq_algo_camgroup_ccm.h"
#include "algos_camgroup/newStruct/histeq/rk_aiq_algo_camgroup_histeq.h"
#include "algos_camgroup/newStruct/enh/rk_aiq_algo_camgroup_enh.h"
#endif

#ifdef  __cplusplus
namespace RkCam {
#endif
/*
 * isp gets the stats from frame n-1,
 * and the new parameters take effect on frame n+1
 */
#define ISP_PARAMS_EFFECT_DELAY_CNT 2

static RkAiqGrpCondition_t aeGrpCondV3x[] = {
    [0] = {XCAM_MESSAGE_AEC_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
};
/* add 'const' to fix C compile error :
 * initializer element is not constant used in g_default_3a_des
 */

static const RkAiqGrpConditions_t aeGrpCondsV3x = {grp_conds_array_info(aeGrpCondV3x)};

static RkAiqGrpCondition_t awbGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
    [2] = {XCAM_MESSAGE_AWB_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
#ifndef USE_NEWSTRUCT
    [3] = {XCAM_MESSAGE_BLC_V32_PROC_RES_OK, 0},
#endif
};
static const RkAiqGrpConditions_t awbGrpConds = {grp_conds_array_info(awbGrpCond)};

static RkAiqGrpCondition_t afGrpCondV3x[] = {
    [0] = {XCAM_MESSAGE_AF_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
#if RKAIQ_HAVE_PDAF
    [1] = {XCAM_MESSAGE_PDAF_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
#endif
};
static const RkAiqGrpConditions_t afGrpCondsV3x = {grp_conds_array_info(afGrpCondV3x)};

static RkAiqGrpCondition_t otherGrpCondV21[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
};
static const RkAiqGrpConditions_t otherGrpCondsV21 = {grp_conds_array_info(otherGrpCondV21)};

static RkAiqGrpCondition_t grp0Cond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
    [2] = {XCAM_MESSAGE_AWB_PROC_RES_OK, 0},
#if USE_NEWSTRUCT
    [3] = {XCAM_MESSAGE_AEC_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
#if RK_GAIN_V2_ENABLE_GAIN2DDR
    [4] = {XCAM_MESSAGE_AGAIN_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT3},
#endif
#else
    [3] = {XCAM_MESSAGE_BLC_V32_PROC_RES_OK, 0},
#if RK_GAIN_V2_ENABLE_GAIN2DDR
    [4] = {XCAM_MESSAGE_AGAIN_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT3},
#endif
#endif
};
static const RkAiqGrpConditions_t grp0Conds = {grp_conds_array_info(grp0Cond)};

static RkAiqGrpCondition_t grpDhazCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
    [2] = {XCAM_MESSAGE_ADEHAZE_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
#if USE_NEWSTRUCT
    // [3] = {XCAM_MESSAGE_YNR_PROC_RES_OK, 0},
#else
    [3] = {XCAM_MESSAGE_YNR_V24_PROC_RES_OK, 0},
    [4] = {XCAM_MESSAGE_BLC_V32_PROC_RES_OK, 0},
#endif
};
static const RkAiqGrpConditions_t grpDhazConds = {grp_conds_array_info(grpDhazCond)};

static RkAiqGrpCondition_t grp1Cond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AWB_PROC_RES_OK, 0},
    [2] = {XCAM_MESSAGE_BLC_V32_PROC_RES_OK, 0},
};
static const RkAiqGrpConditions_t grp1Conds = {grp_conds_array_info(grp1Cond)};

static RkAiqGrpCondition_t otherGrpCondV3x[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
};
static const RkAiqGrpConditions_t otherGrpCondsV3x = {grp_conds_array_info(otherGrpCondV3x)};

static RkAiqGrpCondition_t grpAfdCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_VICAP_POLL_SCL_OK, 0},
};

static const RkAiqGrpConditions_t grpAfdConds = {grp_conds_array_info(grpAfdCond)};

static struct RkAiqAlgoDesCommExt g_default_3a_des[] = {
// clang-format off
#if RKAIQ_HAVE_AE_V1
#if defined(ISP_HW_V33)
    // { &g_RkIspAlgoDescAe.common,            RK_AIQ_CORE_ANALYZE_AE,     0, 6, 0,    aeGrpCondsV3x      },
#else
    { &g_RkIspAlgoDescAe.common,            RK_AIQ_CORE_ANALYZE_AE,     0, 3, 0,    aeGrpCondsV3x      },
#endif
#endif
#if RKAIQ_HAVE_BLC_V32
    { &g_RkIspAlgoDescBlc.common,           RK_AIQ_CORE_ANALYZE_AWB,    0, 0, 0,       awbGrpConds        },
#endif
#if RKAIQ_HAVE_AWB_V39
    // { &g_RkIspAlgoDescAwb.common,           RK_AIQ_CORE_ANALYZE_AWB,    1, 2, 39,   awbGrpConds        },
#endif
#if RKAIQ_HAVE_CAC_V30
    { &g_RkIspAlgoDescCac.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,   grp0Conds          },
#endif
#if RKAIQ_HAVE_GAMMA_V11
    { &g_RkIspAlgoDescGamma.common,         RK_AIQ_CORE_ANALYZE_OTHER,   0, 0, 0,    otherGrpCondsV3x          },
#endif
#if RKAIQ_HAVE_CGC_V1
    { &g_RkIspAlgoDescCgc.common,          RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    otherGrpCondsV3x   },
#endif
#if RKAIQ_HAVE_CSM_V1
    { &g_RkIspAlgoDescCsm.common,           RK_AIQ_CORE_ANALYZE_OTHER,   0, 0, 0,    otherGrpCondsV3x        },
#endif
#if RKAIQ_HAVE_ACP_V10
    { &g_RkIspAlgoDescCp.common,           RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    otherGrpCondsV3x   },
#endif
#if RKAIQ_HAVE_AIE_V10
    { &g_RkIspAlgoDescIe.common,           RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    otherGrpCondsV3x   },
#endif
#if RKAIQ_HAVE_GIC_V3
    { &g_RkIspAlgoDescGic.common,           RK_AIQ_CORE_ANALYZE_OTHER,   0, 0, 0,   otherGrpCondsV3x   },
#endif
#if RKAIQ_HAVE_CCM_V3
    { &g_RkIspAlgoDescCcm.common,           RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    grp0Conds        },
#endif
#if RKAIQ_HAVE_DEBAYER_V4
    { &g_RkIspAlgoDescDemosaic.common,      RK_AIQ_CORE_ANALYZE_OTHER,   0, 3, 0,    otherGrpCondsV3x          },
#endif
#if RKAIQ_HAVE_DRC_V20
    { &g_RkIspAlgoDescDrc.common,           RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    grp0Conds          },
#endif
#if RKAIQ_HAVE_YNR_V40
    { &g_RkIspAlgoDescYnr.common,           RK_AIQ_CORE_ANALYZE_GRP0,    0,  1,  0, grp0Conds          },
#endif
#if RKAIQ_HAVE_HISTEQ_V10
    { &g_RkIspAlgoDescHisteq.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0, 1, 0,    grp0Conds         },
#endif
#if RKAIQ_HAVE_ENHANCE_V10
    { &g_RkIspAlgoDescEnh.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0, 1, 0,    grp0Conds         },
#endif
#if RKAIQ_HAVE_HSV_V10
    { &g_RkIspAlgoDescHsv.common,          RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    grp0Conds         },
#endif
#if (RKAIQ_HAVE_SHARP_V40)
    { &g_RkIspAlgoDescSharp.common,           RK_AIQ_CORE_ANALYZE_GRP0,   0, 1, 0,       grp0Conds          },
#endif
#if (RKAIQ_HAVE_BAYERTNR_V41)
    { &g_RkIspAlgoDescBayertnr.common,     RK_AIQ_CORE_ANALYZE_GRP0,    0, 0, 0, grp0Conds          },
#endif
#if (RKAIQ_HAVE_CNR_V35)
    { &g_RkIspAlgoDescCnr.common,           RK_AIQ_CORE_ANALYZE_GRP0,    0,  1,  0, grp0Conds          },
#endif
#if RKAIQ_HAVE_GAIN_V2
    { &g_RkIspAlgoDescGain.common,       RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    grp0Conds },
#endif

#if RKAIQ_HAVE_DPCC_V2
    { &g_RkIspAlgoDescDpc.common,         RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    otherGrpCondsV3x   },
#endif

#if RKAIQ_HAVE_MERGE_V12
    { &g_RkIspAlgoDescMerge.common,     RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    grp0Conds },
#endif
#if RKAIQ_HAVE_LSC_V3
    { &g_RkIspAlgoDescLsc.common,           RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    grp0Conds          },
#endif
#if defined(ISP_HW_V33)
#if RKAIQ_HAVE_ASD_V10
    { &g_RkIspAlgoDescAsd.common,           RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    grp0Conds          },
#endif
#endif
#if (RKAIQ_HAVE_AFD_V2)
    { &g_RkIspAlgoDescAfd.common,            RK_AIQ_CORE_ANALYZE_AFD,     0, 1, 0,    grpAfdConds      },
#endif
#if RKAIQ_HAVE_LDC
    { &g_RkIspAlgoDescLdc.common,          RK_AIQ_CORE_ANALYZE_OTHER,   1, 1, 0, otherGrpCondsV3x      },
#elif RKAIQ_HAVE_LDCH_V21
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAldch.common,         RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    otherGrpCondsV3x   },
#else
    { &g_RkIspAlgoDescLdch.common,         RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    otherGrpCondsV3x },
#endif
#endif
#if (RKAIQ_HAVE_AMTD_V1)
    { &g_RkIspAlgoDescAmtd.common,      RK_AIQ_CORE_ANALYZE_AE,     0, 0, 0,    {grp_conds_array_info(aeGrpCondV3x)}},
#endif
    { NULL,                                 RK_AIQ_CORE_ANALYZE_ALL,    0, 0, 0,    {0, 0}             },
    // clang-format on
};

#if RKAIQ_ENABLE_CAMGROUP
static struct RkAiqAlgoDesCommExt g_camgroup_algos[] = {
    // clang-format off
    //{ &g_RkIspAlgoDescCamgroupAe.common,            RK_AIQ_CORE_ANALYZE_AE,     0,  5,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupBlc.common,           RK_AIQ_CORE_ANALYZE_AWB,   0, 0, 0,    {0, 0} },
    // { &g_RkIspAlgoDescCamgroupAwb.common,           RK_AIQ_CORE_ANALYZE_AWB,    1,  2, 32, {0, 0} },
    { &g_RkIspAlgoDescCamgroupLsc.common,           RK_AIQ_CORE_ANALYZE_AWB,   0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupCcm.common,          RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
#ifndef ENABLE_PARTIAL_ALOGS
    // { &g_RkIspAlgoDescCamgroupAdpcc.common,      RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
    // { &g_RkIspAlgoDescamgroupAgamma.common,      RK_AIQ_CORE_ANALYZE_GRP0,   0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupDrc.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0,  0,  0, {0, 0} },
    // { &g_RkIspAlgoDescCamgroupAmerge.common,     RK_AIQ_CORE_ANALYZE_GRP0,   0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupYnr.common,       RK_AIQ_CORE_ANALYZE_OTHER, 24, 24, 24, {0, 0} },

    { &g_RkIspAlgoDescCamgroupBtnr.common,          RK_AIQ_CORE_ANALYZE_OTHER, 40, 40, 40, {0, 0} },
    { &g_RkIspAlgoDescCamgroupSharp.common,         RK_AIQ_CORE_ANALYZE_OTHER, 34, 34, 34, {0, 0} },

    { &g_RkIspAlgoDescCamgroupGain.common,       RK_AIQ_CORE_ANALYZE_OTHER,  0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupHisteq.common,         RK_AIQ_CORE_ANALYZE_GRP0,   0,  1,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupEnh.common,         RK_AIQ_CORE_ANALYZE_GRP0,   0,  1,  0, {0, 0} },
#endif
    { NULL,                                         RK_AIQ_CORE_ANALYZE_ALL,    0,  0,  0, {0, 0} },
    // clang-format on
};
#endif

#ifdef  __cplusplus
}  // namespace RkCam
#endif

#endif  // RK_AIQ_CORE_CONFIG_V32_H
