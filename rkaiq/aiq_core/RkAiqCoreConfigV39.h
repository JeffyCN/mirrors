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
#ifndef RK_AIQ_CORE_CONFIG_V39_H
#define RK_AIQ_CORE_CONFIG_V39_H

#include "a3dlut/rk_aiq_algo_a3dlut_itf.h"
#include "abayer2dnr2/rk_aiq_abayer2dnr_algo_itf_v2.h"
#include "abayer2dnrV23/rk_aiq_abayer2dnr_algo_itf_v23.h"
#include "abayertnr2/rk_aiq_abayertnr_algo_itf_v2.h"
#include "abayertnrV23/rk_aiq_abayertnr_algo_itf_v23.h"
#include "abayertnrV30/rk_aiq_abayertnr_algo_itf_v30.h"
#include "ablc/rk_aiq_algo_ablc_itf.h"
#include "ablcV32/rk_aiq_ablc_algo_itf_v32.h"
#include "acac/rk_aiq_algo_acac_itf.h"
#include "accm/rk_aiq_algo_accm_itf.h"
#include "acgc/rk_aiq_algo_acgc_itf.h"
#include "acnr2/rk_aiq_acnr_algo_itf_v2.h"
#include "acnrV30/rk_aiq_acnr_algo_itf_v30.h"
#include "acnrV31/rk_aiq_acnr_algo_itf_v31.h"
#include "acp/rk_aiq_algo_acp_itf.h"
#include "acsm/rk_aiq_algo_acsm_itf.h"
#include "adebayer/rk_aiq_algo_adebayer_itf.h"
#include "adegamma/rk_aiq_algo_adegamma_itf.h"
#include "adehaze/rk_aiq_algo_adehaze_itf.h"
#include "adpcc/rk_aiq_algo_adpcc_itf.h"
#include "adrc/rk_aiq_algo_adrc_itf.h"
#include "ae/rk_aiq_algo_ae_itf.h"
#include "af/rk_aiq_algo_af_itf.h"
#include "afd/rk_aiq_algo_afd_itf.h"
#include "afec/rk_aiq_algo_afec_itf.h"
#include "again2/rk_aiq_again_algo_itf_v2.h"
#include "agamma/rk_aiq_algo_agamma_itf.h"
#include "agic/rk_aiq_algo_agic_itf.h"
#include "aie/rk_aiq_algo_aie_itf.h"
#include "aldc/rk_aiq_algo_aldc_itf.h"
#include "aldch/rk_aiq_algo_aldch_itf.h"
#include "alsc/rk_aiq_algo_alsc_itf.h"
#include "amerge/rk_aiq_algo_amerge_itf.h"
#include "argbir/rk_aiq_algo_argbir_itf.h"
#include "asd/rk_aiq_algo_asd_itf.h"
#include "asharp4/rk_aiq_asharp_algo_itf_v4.h"
#include "asharpV34/rk_aiq_asharp_algo_itf_v34.h"
#include "awb/rk_aiq_algo_awb_itf.h"
#include "awdr/rk_aiq_algo_awdr_itf.h"
#include "aynr3/rk_aiq_aynr_algo_itf_v3.h"
#include "aynrV24/rk_aiq_aynr_algo_itf_v24.h"
#include "ayuvmeV1/rk_aiq_ayuvme_algo_itf_v1.h"
#include "newStruct/demosaic/include/demosaic_algo_api.h"
#include "newStruct/bayertnr/include/bayertnr_algo_api.h"
#include "newStruct/gamma/include/gamma_algo_api.h"
#include "newStruct/ynr/include/ynr_algo_api.h"
#include "newStruct/drc/include/drc_algo_api.h"
#include "newStruct/dehaze/include/dehaze_algo_api.h"
#include "newStruct/histeq/include/histeq_algo_api.h"
#include "newStruct/blc/include/blc_algo_api.h"
#include "newStruct/dpc/include/dpc_algo_api.h"
#include "newStruct/cac/include/cac_algo_api.h"
#include "newStruct/csm/include/csm_algo_api.h"
#include "newStruct/merge/include/merge_algo_api.h"
#include "newStruct/lsc/include/lsc_algo_api.h"
#include "newStruct/rgbir/include/rgbir_algo_api.h"
#include "newStruct/cgc/include/cgc_algo_api.h"
#include "newStruct/cp/include/cp_algo_api.h"
#include "newStruct/ie/include/ie_algo_api.h"
#include "newStruct/gain/include/gain_algo_api.h"
#include "newStruct/3dlut/include/3dlut_algo_api.h"
#include "newStruct/ccm/include/ccm_algo_api.h"
#include "newStruct/ldc/include/ldc_algo_api.h"
#include "amtd/rk_aiq_algo_amtd_itf.h"

#if RKAIQ_ENABLE_CAMGROUP
#include "algos_camgroup/abayer2dnrV23/rk_aiq_algo_camgroup_abayer2dnr_itf_v23.h"
#include "algos_camgroup/abayertnrV23/rk_aiq_algo_camgroup_abayertnr_itf_v23.h"
#include "algos_camgroup/acnrV30/rk_aiq_algo_camgroup_acnr_itf_v30.h"
#include "algos_camgroup/ae/rk_aiq_algo_camgroup_ae_itf.h"
#include "algos_camgroup/again2/rk_aiq_algo_camgroup_again_itf_v2.h"
#include "algos_camgroup/asharpV34/rk_aiq_algo_camgroup_asharp_itf_v34.h"
#include "algos_camgroup/awb/rk_aiq_algo_camgroup_awb_itf.h"
#include "algos_camgroup/aynrV24/rk_aiq_algo_camgroup_aynr_itf_v24.h"
#include "algos_camgroup/misc/rk_aiq_algo_camgroup_misc_itf.h"
#include "algos_camgroup/abayertnrV30/rk_aiq_algo_camgroup_abayertnr_itf_v30.h"
#include "algos_camgroup/ayuvmeV1/rk_aiq_algo_camgroup_ayuvme_itf_v1.h"
#include "algos_camgroup/acnrV31/rk_aiq_algo_camgroup_acnr_itf_v31.h"
#include "algos_camgroup/newStruct/bayertnr/rk_aiq_algo_camgroup_btnr.h"
#include "algos_camgroup/newStruct/drc/rk_aiq_algo_camgroup_drc.h"
#include "algos_camgroup/newStruct/dehaze/rk_aiq_algo_camgroup_dehaze.h"
#include "algos_camgroup/newStruct/histeq/rk_aiq_algo_camgroup_histeq.h"
#include "algos_camgroup/newStruct/sharp/rk_aiq_algo_camgroup_sharp.h"
#include "algos_camgroup/newStruct/ynr/rk_aiq_algo_camgroup_ynr.h"
#include "algos_camgroup/newStruct/blc/rk_aiq_algo_camgroup_blc.h"
#include "algos_camgroup/newStruct/lsc/rk_aiq_algo_camgroup_lsc.h"
#include "algos_camgroup/newStruct/rgbir/rk_aiq_algo_camgroup_rgbir.h"
#include "algos_camgroup/newStruct/gain/rk_aiq_algo_camgroup_gain.h"
#include "algos_camgroup/newStruct/3dlut/rk_aiq_algo_camgroup_3dlut.h"
#include "algos_camgroup/newStruct/ccm/rk_aiq_algo_camgroup_ccm.h"
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

static const RkAiqGrpConditions_t aeGrpCondsV3xs = {grp_conds_array_info(aeGrpCondV3x)};

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
#if defined(ISP_HW_V39)
    // { &g_RkIspAlgoDescAe.common,            RK_AIQ_CORE_ANALYZE_AE,     0, 5, 0,    {grp_conds_array_info(aeGrpCondV3x)}      },
#else
    { &g_RkIspAlgoDescAe.common,            RK_AIQ_CORE_ANALYZE_AE,     0, 3, 0,    {grp_conds_array_info(aeGrpCondV3x)}      },
#endif
#endif
#if RKAIQ_HAVE_BLC_V32
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescBlc.common,           RK_AIQ_CORE_ANALYZE_AWB,    0, 0, 0,       {grp_conds_array_info(awbGrpCond)}        },
#else
    { &g_RkIspAlgoDescAblcV32.common,          RK_AIQ_CORE_ANALYZE_OTHER,    32, 32, 32,    {grp_conds_array_info(otherGrpCondV3x)}        },
#endif
#endif
#if RKAIQ_HAVE_AWB_V39
    // { &g_RkIspAlgoDescAwb.common,           RK_AIQ_CORE_ANALYZE_AWB,    1, 2, 39,   {grp_conds_array_info(awbGrpCond)}        },
#endif
#if RKAIQ_HAVE_CAC_V11
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAcac.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 11,   {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescCac.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,   {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if RKAIQ_HAVE_GAMMA_V11
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAgamma.common,        RK_AIQ_CORE_ANALYZE_OTHER,   0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}          },
#else
    { &g_RkIspAlgoDescGamma.common,         RK_AIQ_CORE_ANALYZE_OTHER,   0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}          },
#endif
#endif
#if RKAIQ_HAVE_CGC_V1
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAcgc.common,          RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#else
    { &g_RkIspAlgoDescCgc.common,          RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#endif
#endif
#if RKAIQ_HAVE_CSM_V1
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCsm.common,           RK_AIQ_CORE_ANALYZE_OTHER,   0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}        },
#else
    { &g_RkIspAlgoDescAcsm.common,          RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#endif
#endif
#if RKAIQ_HAVE_ACP_V10
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAcp.common,           RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#else
    { &g_RkIspAlgoDescCp.common,           RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#endif
#endif
#if RKAIQ_HAVE_AIE_V10
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAie.common,           RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#else
    { &g_RkIspAlgoDescIe.common,           RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#endif
#endif
#if RKAIQ_HAVE_GIC_V2
#ifdef USE_NEWSTRUCT
    { &g_RkIspAlgoDescGic.common,           RK_AIQ_CORE_ANALYZE_OTHER,   0, 0, 0,   {grp_conds_array_info(otherGrpCondV3x)}   },
#else
    { &g_RkIspAlgoDescAgic.common,          RK_AIQ_CORE_ANALYZE_OTHER,  0, 1, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#endif
#endif
#if RKAIQ_HAVE_CCM_V3
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCcm.common,           RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}        },
#else
    { &g_RkIspAlgoDescAccm.common,          RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if RKAIQ_HAVE_DEBAYER_V3
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAdebayer.common,      RK_AIQ_CORE_ANALYZE_OTHER,   0, 3, 0,    {grp_conds_array_info(otherGrpCondV3x)}          },
#else
    { &g_RkIspAlgoDescDemosaic.common,      RK_AIQ_CORE_ANALYZE_OTHER,   0, 3, 0,    {grp_conds_array_info(otherGrpCondV3x)}          },
#endif
#endif
#if RKAIQ_HAVE_DRC_V20
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAdrc.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescDrc.common,           RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescARGBIR.common,            RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescRgbir.common,            RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if RKAIQ_HAVE_YNR_V24
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescYnr.common,           RK_AIQ_CORE_ANALYZE_GRP0,    0,  1,  0, {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescAynrV24.common,       RK_AIQ_CORE_ANALYZE_GRP0,   24, 24, 24, {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if RKAIQ_HAVE_DEHAZE_V14
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAdhaz.common,         RK_AIQ_CORE_ANALYZE_DHAZ,   0, 1, 0,    {grp_conds_array_info(grpDhazCond)}         },
#else
    { &g_RkIspAlgoDescDehaze.common,          RK_AIQ_CORE_ANALYZE_DHAZ,   0, 1, 0,    {grp_conds_array_info(grpDhazCond)}         },
    { &g_RkIspAlgoDescHisteq.common,          RK_AIQ_CORE_ANALYZE_DHAZ,   0, 1, 0,    {grp_conds_array_info(grpDhazCond)}         },
#endif
#endif
#if (RKAIQ_HAVE_SHARP_V34)
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescSharp.common,           RK_AIQ_CORE_ANALYZE_GRP0,   0, 1, 0,       {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescAsharpV34.common,       RK_AIQ_CORE_ANALYZE_GRP0,   34, 34, 34,    {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if (RKAIQ_HAVE_BAYERTNR_V30)
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescBayertnr.common,     RK_AIQ_CORE_ANALYZE_GRP0,    0, 0, 0, {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescAbayertnrV30.common, RK_AIQ_CORE_ANALYZE_GRP0,    30, 30, 30, {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if RKAIQ_HAVE_YUVME_V1
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescYme.common,      RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0, {grp_conds_array_info(grp0Cond)}         },
#else
    { &g_RkIspAlgoDescAyuvmeV1.common,      RK_AIQ_CORE_ANALYZE_GRP0,   1, 1, 1, {grp_conds_array_info(grp0Cond)}         },
#endif
#endif
#if (RKAIQ_HAVE_CNR_V31)
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCnr.common,           RK_AIQ_CORE_ANALYZE_GRP0,    0,  1,  0, {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescAcnrV31.common, RK_AIQ_CORE_ANALYZE_GRP0,    31, 31, 31, {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if RKAIQ_HAVE_GAIN_V2
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescGain.common,       RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    {grp_conds_array_info(grp0Cond)} },
#else
    { &g_RkIspAlgoDescAgainV2.common,       RK_AIQ_CORE_ANALYZE_GRP0,   2, 2, 2,    {grp_conds_array_info(grp0Cond)} },
#endif

#endif

#if RKAIQ_HAVE_DPCC_V2
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAdpcc.common,         RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#else
    { &g_RkIspAlgoDescDpc.common,         RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#endif

#endif
#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
    { &g_RkIspAlgoDescAf.common,            RK_AIQ_CORE_ANALYZE_AF,     0, 4, 0,    {grp_conds_array_info(afGrpCondV3x)}   },
#endif

#if RKAIQ_HAVE_MERGE_V12
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAmerge.common,        RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    {grp_conds_array_info(grp0Cond)} },
#else
    { &g_RkIspAlgoDescMerge.common,     RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    {grp_conds_array_info(grp0Cond)} },
#endif
#endif
#if RKAIQ_HAVE_LSC_V3
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAlsc.common,          RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescLsc.common,           RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if RKAIQ_HAVE_3DLUT_V1
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescLut3d.common,         RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#else
    { &g_RkIspAlgoDescA3dlut.common,        RK_AIQ_CORE_ANALYZE_GRP1,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if defined(ISP_HW_V39)
#if RKAIQ_HAVE_ASD_V10
    { &g_RkIspAlgoDescAsd.common,           RK_AIQ_CORE_ANALYZE_GRP0,   0, 0, 0,    {grp_conds_array_info(grp0Cond)}          },
#endif
#endif
#if (RKAIQ_HAVE_AFD_V2)
    { &g_RkIspAlgoDescAfd.common,            RK_AIQ_CORE_ANALYZE_AFD,     0, 1, 0,    {grp_conds_array_info(grpAfdCond)}      },
#endif
#if RKAIQ_HAVE_LDC
    { &g_RkIspAlgoDescLdc.common,           RK_AIQ_CORE_ANALYZE_OTHER,   1, 1, 0,    {grp_conds_array_info(otherGrpCondV3x)}      },
#elif RKAIQ_HAVE_LDCH_V21
#ifndef USE_NEWSTRUCT
    { &g_RkIspAlgoDescAldch.common,         RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)}   },
#else
    { &g_RkIspAlgoDescLdch.common,         RK_AIQ_CORE_ANALYZE_OTHER,  0, 0, 0,    {grp_conds_array_info(otherGrpCondV3x)} },
#endif
#endif
#if (RKAIQ_HAVE_AMTD_V1)
    {   &g_RkIspAlgoDescAmtd.common,        RK_AIQ_CORE_ANALYZE_AE,     0, 0, 0,    {grp_conds_array_info(aeGrpCondV3x)}},
#endif
    { NULL,                                 RK_AIQ_CORE_ANALYZE_ALL,    0, 0, 0,    {0, 0}             },
    // clang-format on
};

#if RKAIQ_ENABLE_CAMGROUP
static struct RkAiqAlgoDesCommExt g_camgroup_algos[] = {
    // clang-format off
    //{ &g_RkIspAlgoDescCamgroupAe.common,            RK_AIQ_CORE_ANALYZE_AE,     0,  5,  0, {0, 0} },
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupBlc.common,           RK_AIQ_CORE_ANALYZE_AWB,   0, 0, 0,    {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupAblcV32.common,       RK_AIQ_CORE_ANALYZE_GRP0,  32, 32, 32, {0, 0} },
#endif
    // { &g_RkIspAlgoDescCamgroupAwb.common,           RK_AIQ_CORE_ANALYZE_AWB,    1,  2, 32, {0, 0} },
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupLsc.common,           RK_AIQ_CORE_ANALYZE_AWB,   0,  0,  0, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupAlsc.common,          RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
#endif
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupCcm.common,          RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupAccm.common,          RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
#endif
#ifndef ENABLE_PARTIAL_ALOGS
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupLut3d.common,          RK_AIQ_CORE_ANALYZE_GRP0,  0,  0,  0, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupA3dlut.common,        RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
#endif
    // { &g_RkIspAlgoDescCamgroupAdpcc.common,      RK_AIQ_CORE_ANALYZE_AWB,    0,  0,  0, {0, 0} },
    // { &g_RkIspAlgoDescamgroupAgamma.common,      RK_AIQ_CORE_ANALYZE_GRP0,   0,  0,  0, {0, 0} },
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupDrc.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0,  1,  0, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupAdrc.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0,  1,  0, {0, 0} },
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupRgbir.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0,  1,  0, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupArgbir.common,          RK_AIQ_CORE_ANALYZE_GRP0,   0,  1,  0, {0, 0} },
#endif
#endif
    // { &g_RkIspAlgoDescCamgroupAmerge.common,     RK_AIQ_CORE_ANALYZE_GRP0,   0,  0,  0, {0, 0} },
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupYnr.common,       RK_AIQ_CORE_ANALYZE_OTHER, 24, 24, 24, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupAynrV24.common,       RK_AIQ_CORE_ANALYZE_OTHER, 24, 24, 24, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAcnrV31.common,       RK_AIQ_CORE_ANALYZE_OTHER, 31, 31, 31, {0, 0} },
#endif

#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupBtnr.common,          RK_AIQ_CORE_ANALYZE_OTHER, 40, 40, 40, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupAbayertnrV30.common,  RK_AIQ_CORE_ANALYZE_OTHER, 30, 30, 30, {0, 0} },
#endif
#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupSharp.common,         RK_AIQ_CORE_ANALYZE_OTHER, 34, 34, 34, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupAsharpV34.common,     RK_AIQ_CORE_ANALYZE_OTHER, 34, 34, 34, {0, 0} },
#endif

#if USE_NEWSTRUCT
    { &g_RkIspAlgoDescCamgroupGain.common,       RK_AIQ_CORE_ANALYZE_OTHER,  0,  0,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupDehaze.common,         RK_AIQ_CORE_ANALYZE_DHAZ,   0,  1,  0, {0, 0} },
    { &g_RkIspAlgoDescCamgroupHisteq.common,         RK_AIQ_CORE_ANALYZE_DHAZ,   0,  1,  0, {0, 0} },
#else
    { &g_RkIspAlgoDescCamgroupAgainV2.common,       RK_AIQ_CORE_ANALYZE_OTHER,  2,  2,  2, {0, 0} },
    { &g_RkIspAlgoDescCamgroupAdhaz.common,         RK_AIQ_CORE_ANALYZE_DHAZ,   0,  1,  0, {0, 0} },
#endif
    //{ &g_RkIspAlgoDescCamgroupAyuvmeV1.common,       RK_AIQ_CORE_ANALYZE_OTHER, 1, 1, 1, {0, 0} },
#endif
    { NULL,                                         RK_AIQ_CORE_ANALYZE_ALL,    0,  0,  0, {0, 0} },
    // clang-format on
};
#endif

#ifdef  __cplusplus
}  // namespace RkCam
#endif

#endif  // RK_AIQ_CORE_CONFIG_V32_H
