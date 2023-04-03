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
#ifndef RK_AIQ_CORE_CONFIG_V20_H
#define RK_AIQ_CORE_CONFIG_V20_H

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
#include "aeis/rk_aiq_algo_aeis_itf.h"
#include "af/rk_aiq_algo_af_itf.h"
#include "afec/rk_aiq_algo_afec_itf.h"
#include "again/rk_aiq_again_algo_itf.h"
#include "agamma/rk_aiq_algo_agamma_itf.h"
#include "agic/rk_aiq_algo_agic_itf.h"
#include "aie/rk_aiq_algo_aie_itf.h"
#include "aldch/rk_aiq_algo_aldch_itf.h"
#include "alsc/rk_aiq_algo_alsc_itf.h"
#include "amd/rk_aiq_algo_amd_itf.h"
#include "amerge/rk_aiq_algo_amerge_itf.h"
#include "amfnr/rk_aiq_amfnr_algo_itf_v1.h"
#include "anr/rk_aiq_algo_anr_itf.h"
#include "aorb/rk_aiq_algo_aorb_itf.h"
#include "arawnr/rk_aiq_abayernr_algo_itf_v1.h"
#include "asd/rk_aiq_algo_asd_itf.h"
#include "asharp/rk_aiq_algo_asharp_itf.h"
#include "atmo/rk_aiq_algo_atmo_itf.h"
#include "auvnr/rk_aiq_auvnr_algo_itf_v1.h"
#include "awb/rk_aiq_algo_awb_itf.h"
#include "awdr/rk_aiq_algo_awdr_itf.h"
#include "aynr/rk_aiq_aynr_algo_itf_v1.h"

namespace RkCam {

/*
 * isp gets the stats from frame n-1,
 * and the new parameters take effect on frame n+1
 */
#define ISP_PARAMS_EFFECT_DELAY_CNT 2

static RkAiqGrpCondition_t aeGrpCond[] = {
    [0] = {XCAM_MESSAGE_AEC_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
};
static RkAiqGrpConditions_t aeGrpConds = {grp_conds_array_info(aeGrpCond)};

static RkAiqGrpCondition_t awbGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_AE_PRE_RES_OK, 0},
    [2] = {XCAM_MESSAGE_AWB_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
};
static RkAiqGrpConditions_t awbGrpConds = {grp_conds_array_info(awbGrpCond)};

static RkAiqGrpCondition_t measGrpCond[] = {
    [0] = {XCAM_MESSAGE_ISP_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
};
static RkAiqGrpConditions_t measGrpConds = {grp_conds_array_info(measGrpCond)};

static RkAiqGrpCondition_t otherGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
};
static RkAiqGrpConditions_t otherGrpConds = {grp_conds_array_info(otherGrpCond)};

static RkAiqGrpCondition_t amdGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_ISP_POLL_SP_OK, 0},
    [2] = {XCAM_MESSAGE_ISP_GAIN_OK, 0},
};
static RkAiqGrpConditions_t amdGrpConds = {grp_conds_array_info(amdGrpCond)};

static RkAiqGrpCondition_t amfnrGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_ISP_GAIN_OK, 0},
    [2] = {XCAM_MESSAGE_ISPP_GAIN_KG_OK, 0},
    [3] = {XCAM_MESSAGE_AMD_PROC_RES_OK, 0},
};
static RkAiqGrpConditions_t amfnrGrpConds = {grp_conds_array_info(amfnrGrpCond)};

static RkAiqGrpCondition_t aynrGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_ISP_GAIN_OK, 0},
    [2] = {XCAM_MESSAGE_ISPP_GAIN_WR_OK, 0},
    [3] = {XCAM_MESSAGE_AMD_PROC_RES_OK, 0},
};
static RkAiqGrpConditions_t aynrGrpConds = {grp_conds_array_info(aynrGrpCond)};

static RkAiqGrpCondition_t lscGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_ISP_POLL_TX_OK, 0},
    [2] = {XCAM_MESSAGE_AWB_STATS_OK, ISP_PARAMS_EFFECT_DELAY_CNT},
    [3] = {XCAM_MESSAGE_AE_PROC_RES_OK, 0},
};
static RkAiqGrpConditions_t lscGrpConds = {grp_conds_array_info(lscGrpCond)};

static RkAiqGrpCondition_t eisGrpCond[] = {
    [0] = {XCAM_MESSAGE_SOF_INFO_OK, 0},
    [1] = {XCAM_MESSAGE_ORB_STATS_OK, 0},
    [2] = {XCAM_MESSAGE_NR_IMG_OK, 0},
};
static RkAiqGrpConditions_t eisGrpConds = {grp_conds_array_info(eisGrpCond)};

static RkAiqGrpCondition_t orbGrpCond[] = {
    [0] = {XCAM_MESSAGE_ORB_STATS_OK, 0},
};
static RkAiqGrpConditions_t orbGrpConds = {grp_conds_array_info(orbGrpCond)};

const static struct RkAiqAlgoDesCommExt g_default_3a_des[] = {
// clang-format off
#if RKAIQ_HAVE_AE_V1
    { &g_RkIspAlgoDescAe.common,        RK_AIQ_CORE_ANALYZE_AE,    0, 0, 0, aeGrpConds    },
#endif
#if RKAIQ_HAVE_AWB_V20
    { &g_RkIspAlgoDescAwb.common,       RK_AIQ_CORE_ANALYZE_AWB,   0, 0, 0, awbGrpConds   },
#endif
#if RKAIQ_HAVE_AF_V20 || RKAIQ_ONLY_AF_STATS_V20
    { &g_RkIspAlgoDescAf.common,        RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
#endif
#if RKAIQ_HAVE_BLC_V1
    { &g_RkIspAlgoDescAblc.common,      RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_DEGAMMA_V1
    { &g_RkIspAlgoDescAdegamma.common,  RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
    { &g_RkIspAlgoDescAdpcc.common,     RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
#if RKAIQ_HAVE_MERGE_V10
    { &g_RkIspAlgoDescAmerge.common,    RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
#endif
#if RKAIQ_HAVE_TMO_V1
    { &g_RkIspAlgoDescAtmo.common,      RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
#endif
#if ANR_NO_SEPERATE_MARCO
#if RKAIQ_HAVE_ANR_V1
    { &g_RkIspAlgoDescAnr.common,       RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
#endif
#else
#if 0
    { &g_RkIspAlgoDescArawnr.common,    RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
    { &g_RkIspAlgoDescAynr.common,      RK_AIQ_CORE_ANALYZE_AYNR,  0, 0, 0, aynrGrpConds  },
    { &g_RkIspAlgoDescAcnr.common,      RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
    { &g_RkIspAlgoDescAmfnr.common,     RK_AIQ_CORE_ANALYZE_AMFNR, 0, 0, 0, amfnrGrpConds },
    { &g_RkIspAlgoDescAgain.common,     RK_AIQ_CORE_ANALYZE_AMFNR, 0, 0, 0, amfnrGrpConds },
#else
#if RKAIQ_HAVE_BAYERNR_V1
    { &g_RkIspAlgoDescArawnr.common,    RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
#endif
#if RKAIQ_HAVE_YNR_V1
    { &g_RkIspAlgoDescAynr.common,      RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_UVNR_V1
    { &g_RkIspAlgoDescAcnr.common,      RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_MFNR_V1
    { &g_RkIspAlgoDescAmfnr.common,     RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_GAIN_V1
    { &g_RkIspAlgoDescAgain.common,     RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#endif
#endif
#if RKAIQ_HAVE_LSC_V1
    { &g_RkIspAlgoDescAlsc.common,      RK_AIQ_CORE_ANALYZE_LSC,   0, 0, 0, lscGrpConds   },
#endif
#if RKAIQ_HAVE_GIC_V1
    { &g_RkIspAlgoDescAgic.common,      RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_DEBAYER_V1
    { &g_RkIspAlgoDescAdebayer.common,  RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_CCM_V1
    { &g_RkIspAlgoDescAccm.common,      RK_AIQ_CORE_ANALYZE_MEAS,  0, 0, 0, measGrpConds  },
#endif
#if RKAIQ_HAVE_GAMMA_V10
    { &g_RkIspAlgoDescAgamma.common,    RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_WDR_V1
    { &g_RkIspAlgoDescAwdr.common,      RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_DEHAZE_V10
    { &g_RkIspAlgoDescAdhaz.common,     RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_3DLUT_V1
    { &g_RkIspAlgoDescA3dlut.common,    RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_LDCH_V10
    { &g_RkIspAlgoDescAldch.common,     RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_CSM_V1
    { &g_RkIspAlgoDescAcsm.common,      RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_ACP_V10
    { &g_RkIspAlgoDescAcp.common,       RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_AIE_V10
    { &g_RkIspAlgoDescAie.common,       RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_SHARP_V1
    { &g_RkIspAlgoDescAsharp.common,    RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_ORB_V1
    { &g_RkIspAlgoDescAorb.common,      RK_AIQ_CORE_ANALYZE_ORB,   0, 0, 0, orbGrpConds   },
#endif
#if RKAIQ_HAVE_CGC_V1
    { &g_RkIspAlgoDescAcgc.common,      RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_ASD_V10
    { &g_RkIspAlgoDescAsd.common,       RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
#if RKAIQ_HAVE_FEC_V10
    { &g_RkIspAlgoDescAfec.common,    RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, 0, otherGrpConds },
#endif
    { &g_RkIspAlgoDescAeis.common,      RK_AIQ_CORE_ANALYZE_EIS,   0, 0, 0, eisGrpConds   },
#if 0
    { &g_RkIspAlgoDescAmd.common,       RK_AIQ_CORE_ANALYZE_AMD,   0, 0, 0, amdGrpConds   },
#endif
    { NULL,                             RK_AIQ_CORE_ANALYZE_ALL,   0, 0, 0, {0, 0}        },
    // clang-format on
};

}  // namespace RkCam

#endif  // RK_AIQ_CORE_CONFIG_V20_H
