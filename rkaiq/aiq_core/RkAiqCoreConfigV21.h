/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#ifndef RK_AIQ_CORE_CONFIG_V21_H
#define RK_AIQ_CORE_CONFIG_V21_H

#include "a3dlut/rk_aiq_algo_a3dlut_itf.h"
#include "ablc/rk_aiq_algo_ablc_itf.h"
#include "accm/rk_aiq_algo_accm_itf.h"
#include "acgc/rk_aiq_algo_acgc_itf.h"
#include "acnr/rk_aiq_acnr_algo_itf_v1.h"
#include "acp/rk_aiq_algo_acp_itf.h"
#include "acsm/rk_aiq_algo_acsm_itf.h"
#include "adebayer/rk_aiq_algo_adebayer_itf.h"
#include "adegamma/rk_aiq_algo_adegamma_itf.h"
#include "adehaze/rk_aiq_algo_adehaze_itf.h"
#include "adpcc/rk_aiq_algo_adpcc_itf.h"
#include "adrc/rk_aiq_algo_adrc_itf.h"
#include "ae/rk_aiq_algo_ae_itf.h"
#include "af/rk_aiq_algo_af_itf.h"
#include "agamma/rk_aiq_algo_agamma_itf.h"
#include "agic/rk_aiq_algo_agic_itf.h"
#include "aie/rk_aiq_algo_aie_itf.h"
#include "aldch/rk_aiq_algo_aldch_itf.h"
#include "alsc/rk_aiq_algo_alsc_itf.h"
#include "amerge/rk_aiq_algo_amerge_itf.h"
#include "arawnr2/rk_aiq_abayernr_algo_itf_v2.h"
#include "asd/rk_aiq_algo_asd_itf.h"
#include "asharp3/rk_aiq_asharp_algo_itf_v3.h"
#include "awb/rk_aiq_algo_awb_itf.h"
#include "awdr/rk_aiq_algo_awdr_itf.h"
#include "aynr2/rk_aiq_aynr_algo_itf_v2.h"

#if RKAIQ_ENABLE_CAMGROUP
#include "algos_camgroup/abayernr2/rk_aiq_algo_camgroup_abayernr_itf_v2.h"
#include "algos_camgroup/acnr/rk_aiq_algo_camgroup_acnr_itf.h"
#include "algos_camgroup/ae/rk_aiq_algo_camgroup_ae_itf.h"
#include "algos_camgroup/asharp3/rk_aiq_algo_camgroup_asharp_itf_v3.h"
#include "algos_camgroup/awb/rk_aiq_algo_camgroup_awb_itf.h"
#include "algos_camgroup/aynr2/rk_aiq_algo_camgroup_aynr_itf_v2.h"
#include "algos_camgroup/misc/rk_aiq_algo_camgroup_misc_itf.h"
#endif

namespace RkCam {

/*
 * isp gets the stats from frame n-1,
 * and the new parameters take effect on frame n+1
 */
#define ISP_PARAMS_EFFECT_DELAY_CNT 2

static RkAiqGrpCondition_t aeGrpCondV21[] = {
    [0] = {XCAM_MESSAGE_AEC_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
};
static RkAiqGrpConditions_t aeGrpCondsV21 = {grp_conds_array_info(aeGrpCondV21)};

static RkAiqGrpCondition_t awbGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
    [2] = {XCAM_MESSAGE_AWB_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
    [3] = {XCAM_MESSAGE_AEC_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
};
static RkAiqGrpConditions_t awbGrpConds = {grp_conds_array_info(awbGrpCond)};

static RkAiqGrpCondition_t afGrpCondV21[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
    [2] = {XCAM_MESSAGE_AF_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
    [3] = {XCAM_MESSAGE_AEC_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
#if RKAIQ_HAVE_PDAF
    [4] = {XCAM_MESSAGE_PDAF_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
#endif
};
static RkAiqGrpConditions_t afGrpCondsV21 = {grp_conds_array_info(afGrpCondV21)};

static RkAiqGrpCondition_t otherGrpCondV21[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
};
static RkAiqGrpConditions_t otherGrpCondsV21 = {grp_conds_array_info(otherGrpCondV21)};

static RkAiqGrpCondition_t grp0Cond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
    [2] = {XCAM_MESSAGE_AEC_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
    [3] = {XCAM_MESSAGE_AWB_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
};
static RkAiqGrpConditions_t grp0Conds = {grp_conds_array_info(grp0Cond)};

static RkAiqGrpCondition_t grpDhazCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
    [2] = {XCAM_MESSAGE_ADEHAZE_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
};
static RkAiqGrpConditions_t grpDhazConds = {grp_conds_array_info(grpDhazCond)};

static RkAiqGrpCondition_t grp1Cond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AWB_PROC_RES_OK, 0},
};
static RkAiqGrpConditions_t grp1Conds = {grp_conds_array_info(grp1Cond)};

static struct RkAiqAlgoDesCommExt g_default_3a_des[] = {
// clang-format off
#if RKAIQ_HAVE_AE_V1
    { &g_RkIspAlgoDescAe.common,        RK_AIQ_CORE_ANALYZE_AE,     0, 1, 0,  aeGrpCondsV21    },
#endif
#if RKAIQ_HAVE_AWB_V21
    { &g_RkIspAlgoDescAwb.common,       RK_AIQ_CORE_ANALYZE_AWB,    1, 1, 21, awbGrpConds      },
#endif
#if RKAIQ_HAVE_DEBAYER_V1
    { &g_RkIspAlgoDescAdebayer.common,  RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,  grp0Conds        },
#endif
#if RKAIQ_HAVE_GAMMA_V10
    { &g_RkIspAlgoDescAgamma.common,    RK_AIQ_CORE_ANALYZE_OTHER,   0, 0, 0,  otherGrpCondsV21        },
#endif
#if RKAIQ_HAVE_DEGAMMA_V1
    { &g_RkIspAlgoDescAdegamma.common,  RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,  grp0Conds        },
#endif
#if RKAIQ_HAVE_MERGE_V10
    { &g_RkIspAlgoDescAmerge.common,    RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,  grp0Conds        },
#endif
#if RKAIQ_HAVE_DEHAZE_V11
    { &g_RkIspAlgoDescAdhaz.common,     RK_AIQ_CORE_ANALYZE_DHAZ,   0, 1, 0,  grpDhazConds        },
#endif
#if RKAIQ_HAVE_BAYERNR_V2
    { &g_RkIspAlgoDescArawnrV2.common,  RK_AIQ_CORE_ANALYZE_GRP0,   2, 2, 2,  grp0Conds        },
#endif
#if RKAIQ_HAVE_YNR_V2
    { &g_RkIspAlgoDescAynrV2.common,    RK_AIQ_CORE_ANALYZE_GRP0,   2, 2, 2,  grp0Conds        },
#endif
#if RKAIQ_HAVE_CNR_V1
    { &g_RkIspAlgoDescAcnrV1.common,    RK_AIQ_CORE_ANALYZE_GRP0,   1, 1, 1,  grp0Conds        },
#endif
#if RKAIQ_HAVE_SHARP_V3
    { &g_RkIspAlgoDescAsharpV3.common,  RK_AIQ_CORE_ANALYZE_GRP0,   3, 3, 3,  grp0Conds        },
#endif
#if RKAIQ_HAVE_DRC_V10
    { &g_RkIspAlgoDescAdrc.common,      RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,  grp0Conds        },
#endif
#if RKAIQ_HAVE_3DLUT_V1
    { &g_RkIspAlgoDescA3dlut.common,    RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,  grp1Conds        },
#endif
#if RKAIQ_HAVE_LSC_V1
    { &g_RkIspAlgoDescAlsc.common,      RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,  grp1Conds        },
#endif
#if RKAIQ_HAVE_CCM_V1
    { &g_RkIspAlgoDescAccm.common,      RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,  grp1Conds        },
#endif
#if RKAIQ_HAVE_ACP_V10
    { &g_RkIspAlgoDescAcp.common,       RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,  otherGrpCondsV21 },
#endif
#if RKAIQ_HAVE_AIE_V10
    { &g_RkIspAlgoDescAie.common,       RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,  otherGrpCondsV21 },
#endif
    { &g_RkIspAlgoDescAdpcc.common,     RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,  otherGrpCondsV21 },
#if RKAIQ_HAVE_LDCH_V10
    { &g_RkIspAlgoDescAldch.common,     RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,  otherGrpCondsV21 },
#endif
#if RKAIQ_HAVE_CGC_V1
    { &g_RkIspAlgoDescAcgc.common,      RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,  otherGrpCondsV21 },
#endif
#if RKAIQ_HAVE_CSM_V1
    { &g_RkIspAlgoDescAcsm.common,      RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,  otherGrpCondsV21 },
#endif
#if RKAIQ_HAVE_AF_V20 || RKAIQ_ONLY_AF_STATS_V20
    { &g_RkIspAlgoDescAf.common,        RK_AIQ_CORE_ANALYZE_AF,     0, 0, 0,  afGrpCondsV21    },
#endif
#if RKAIQ_HAVE_BLC_V1
    { &g_RkIspAlgoDescAblc.common,      RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,  otherGrpCondsV21 },
#endif
#if RKAIQ_HAVE_GIC_V2
    { &g_RkIspAlgoDescAgic.common,      RK_AIQ_CORE_ANALYZE_OTHER,  0, 1, 0,  otherGrpCondsV21 },
#endif
#if RKAIQ_HAVE_WDR_V1
    { &g_RkIspAlgoDescAwdr.common,      RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,  otherGrpCondsV21 },
#endif
#if RKAIQ_HAVE_ASD_V10
    { &g_RkIspAlgoDescAsd.common,       RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,  grp0Conds        },
#endif
    { NULL,                             RK_AIQ_CORE_ANALYZE_ALL,    0, 0, 0,  {0, 0}           },
    // clang-format on
};

#if RKAIQ_ENABLE_CAMGROUP
const static struct RkAiqAlgoDesCommExt g_camgroup_algos[] = {
    // clang-format off
    { &g_RkIspAlgoDescCamgroupAe.common,            RK_AIQ_CORE_ANALYZE_AE,     0,  1,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAblc.common,          RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAwb.common,           RK_AIQ_CORE_ANALYZE_AWB,    1,  2, 32, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAlsc.common,          RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAccm.common,          RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupA3dlut.common,        RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
    // { &g_RkIspAlgoDescCamgroupAdpcc.common,      RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAdhaz.common,         RK_AIQ_CORE_ANALYZE_DHAZ,   0,  1,  0, {0, 0} },
    // { &g_RkIspAlgoDescamgroupAgamma.common,      RK_AIQ_CORE_ANALYZE_GRP0,   0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAdrc.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0,  1,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAmerge.common,        RK_AIQ_CORE_ANALYZE_GRP0,   0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAynrV2.common,        RK_AIQ_CORE_ANALYZE_OTHER,  2,  2,  2, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAcnr.common,          RK_AIQ_CORE_ANALYZE_OTHER,  1,  1,  1, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAbayernrV2.common,    RK_AIQ_CORE_ANALYZE_OTHER,  2,  2,  2, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAsharpV3.common,      RK_AIQ_CORE_ANALYZE_OTHER,  3,  3,  3, {0, 0} },
    { NULL,                                         RK_AIQ_CORE_ANALYZE_ALL,    0,  0,  0, {0, 0} },
    // clang-format on
};
#endif

}  // namespace RkCam

#endif  // RK_AIQ_CORE_CONFIG_V21_H
