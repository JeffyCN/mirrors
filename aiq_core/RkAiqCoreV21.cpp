/*
 * RkAiqCoreV21.cpp
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
#include "RkAiqHandleIntV21.h"
#include "RkAiqCoreV21.h"
#include "v4l2_buffer_proxy.h"
#include "acp/rk_aiq_algo_acp_itf.h"
#include "ae/rk_aiq_algo_ae_itf.h"
#include "awb/rk_aiq_algo_awb_itf.h"
#include "af/rk_aiq_algo_af_itf.h"
#include "anr/rk_aiq_algo_anr_itf.h"
#include "asd/rk_aiq_algo_asd_itf.h"
#include "amerge/rk_aiq_algo_amerge_itf.h"
#include "adrc/rk_aiq_algo_adrc_itf.h"

#include "asharp/rk_aiq_algo_asharp_itf.h"
#include "adehaze/rk_aiq_algo_adhaz_itf.h"
#include "ablc/rk_aiq_algo_ablc_itf.h"
#include "adpcc/rk_aiq_algo_adpcc_itf.h"
#include "alsc/rk_aiq_algo_alsc_itf.h"
#include "agic/rk_aiq_algo_agic_itf.h"
#include "adebayer/rk_aiq_algo_adebayer_itf.h"
#include "accm/rk_aiq_algo_accm_itf.h"
#include "agamma/rk_aiq_algo_agamma_itf.h"
#include "adegamma/rk_aiq_algo_adegamma_itf.h"
#include "awdr/rk_aiq_algo_awdr_itf.h"
#include "a3dlut/rk_aiq_algo_a3dlut_itf.h"
#include "aldch/rk_aiq_algo_aldch_itf.h"
#include "ar2y/rk_aiq_algo_ar2y_itf.h"
#include "aie/rk_aiq_algo_aie_itf.h"
#include "aorb/rk_aiq_algo_aorb_itf.h"
#include "afec/rk_aiq_algo_afec_itf.h"
#include "acgc/rk_aiq_algo_acgc_itf.h"
#include "asharp3/rk_aiq_asharp_algo_itf_v3.h"
#include "aynr2/rk_aiq_aynr_algo_itf_v2.h"
#include "acnr/rk_aiq_acnr_algo_itf_v1.h"
#include "arawnr2/rk_aiq_abayernr_algo_itf_v2.h"



#ifdef RK_SIMULATOR_HW
#include "simulator/isp20_hw_simulator.h"
#else
#include "isp20/Isp20StatsBuffer.h"
#include "isp20/rkisp2-config.h"
#include "isp21/rkisp21-config.h"
#endif
#include <fcntl.h>
#include <unistd.h>
#include "RkAiqResourceTranslatorV21.h"
#include "RkAiqAnalyzeGroupManager.h"

namespace RkCam {
/*
 * isp/ispp pipeline algos ordered array, and the analyzer
 * will run these algos one by one.
 */

/*
 * isp gets the stats from frame n-1,
 * and the new parameters take effect on frame n+1
 */
#define ISP_PARAMS_EFFECT_DELAY_CNT 2

static RkAiqGrpCondition_t aeGrpCondV21[]       = {
    [0] = { XCAM_MESSAGE_AEC_STATS_OK,      ISP_PARAMS_EFFECT_DELAY_CNT },
};
static RkAiqGrpConditions_t aeGrpCondsV21       = { grp_conds_array_info(aeGrpCondV21) };

static RkAiqGrpCondition_t awbGrpCond[]         = {
    [0] = { XCAM_MESSAGE_AE_PRE_RES_OK,     0 },
    [1] = { XCAM_MESSAGE_AWB_STATS_OK,      ISP_PARAMS_EFFECT_DELAY_CNT },
};
static RkAiqGrpConditions_t  awbGrpConds        = { grp_conds_array_info(awbGrpCond) };

static RkAiqGrpCondition_t afGrpCondV21[]     = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
    [1] = { XCAM_MESSAGE_AE_PRE_RES_OK,     0 },
    [2] = { XCAM_MESSAGE_AE_PROC_RES_OK,    0 },
    [3] = { XCAM_MESSAGE_AF_STATS_OK,       ISP_PARAMS_EFFECT_DELAY_CNT },
    [4] = { XCAM_MESSAGE_AEC_STATS_OK,      ISP_PARAMS_EFFECT_DELAY_CNT },
};
static RkAiqGrpConditions_t  afGrpCondsV21    = { grp_conds_array_info(afGrpCondV21) };

static RkAiqGrpCondition_t otherGrpCondV21[]    = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
};
static RkAiqGrpConditions_t  otherGrpCondsV21   = { grp_conds_array_info(otherGrpCondV21) };

static RkAiqGrpCondition_t grp0Cond[]           = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
    [1] = { XCAM_MESSAGE_AE_PRE_RES_OK,     0 },
    [2] = { XCAM_MESSAGE_AE_PROC_RES_OK,    0 },
    [3] = { XCAM_MESSAGE_AEC_STATS_OK,      ISP_PARAMS_EFFECT_DELAY_CNT },
    [4] = { XCAM_MESSAGE_AWB_STATS_OK,      ISP_PARAMS_EFFECT_DELAY_CNT },
};
static RkAiqGrpConditions_t  grp0Conds          = { grp_conds_array_info(grp0Cond) };

static RkAiqGrpCondition_t grp1Cond[]           = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
    [1] = { XCAM_MESSAGE_AWB_PROC_RES_OK,   0 },
};
static RkAiqGrpConditions_t  grp1Conds          = { grp_conds_array_info(grp1Cond) };

static struct RkAiqAlgoDesCommExt g_default_3a_des_v21[] = {
    { &g_RkIspAlgoDescAe.common, RK_AIQ_CORE_ANALYZE_AE, 0, 1, aeGrpCondsV21 },
    { &g_RkIspAlgoDescAwb.common, RK_AIQ_CORE_ANALYZE_AWB, 1, 1, awbGrpConds },
    { &g_RkIspAlgoDescAdebayer.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 0, grp0Conds },
    { &g_RkIspAlgoDescAgamma.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 0, grp0Conds },
    { &g_RkIspAlgoDescAdegamma.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 0, grp0Conds },
    { &g_RkIspAlgoDescAmerge.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 0, grp0Conds },
    { &g_RkIspAlgoDescAdhaz.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 1, grp0Conds },
    { &g_RkIspAlgoDescArawnrV2.common, RK_AIQ_CORE_ANALYZE_GRP0, 2, 2, grp0Conds },
    { &g_RkIspAlgoDescAynrV2.common, RK_AIQ_CORE_ANALYZE_GRP0, 2, 2, grp0Conds },
    { &g_RkIspAlgoDescAcnrV1.common, RK_AIQ_CORE_ANALYZE_GRP0, 1, 1, grp0Conds },
    { &g_RkIspAlgoDescAsharpV3.common, RK_AIQ_CORE_ANALYZE_GRP0, 3, 3, grp0Conds },
    { &g_RkIspAlgoDescAdrc.common, RK_AIQ_CORE_ANALYZE_GRP0, 0, 0, grp0Conds },
    { &g_RkIspAlgoDescA3dlut.common, RK_AIQ_CORE_ANALYZE_GRP1, 0, 0, grp1Conds },
    { &g_RkIspAlgoDescAlsc.common, RK_AIQ_CORE_ANALYZE_GRP1, 0, 0, grp1Conds },
    { &g_RkIspAlgoDescAccm.common, RK_AIQ_CORE_ANALYZE_GRP1, 0, 0, grp1Conds },
    { &g_RkIspAlgoDescAcp.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21 },
    { &g_RkIspAlgoDescAie.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21 },
    { &g_RkIspAlgoDescAdpcc.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21},
    { &g_RkIspAlgoDescAldch.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21 },
    { &g_RkIspAlgoDescAcgc.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21 },
    { &g_RkIspAlgoDescAr2y.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21 },
    { &g_RkIspAlgoDescAf.common, RK_AIQ_CORE_ANALYZE_AF, 0, 0, afGrpCondsV21 },
    { &g_RkIspAlgoDescAblc.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21 },
    { &g_RkIspAlgoDescAgic.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 1, otherGrpCondsV21 },
    { &g_RkIspAlgoDescAwdr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21 },
    { &g_RkIspAlgoDescAsd.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpCondsV21 },

    { NULL, RK_AIQ_CORE_ANALYZE_ALL, 0, 0 },
};

RkAiqCoreV21::RkAiqCoreV21()
    : RkAiqCore()
{
    ENTER_ANALYZER_FUNCTION();

    mHasPp = false;
    mIspHwVer = 1;
    mAlgosDesArray = g_default_3a_des_v21;

    mTranslator = new  RkAiqResourceTranslatorV21();

    EXIT_ANALYZER_FUNCTION();
}

RkAiqCoreV21::~RkAiqCoreV21()
{
    ENTER_ANALYZER_FUNCTION();

    EXIT_ANALYZER_FUNCTION();
}

void
RkAiqCoreV21::newAiqParamsPool()
{
    for (auto algoHdl : mCurIspAlgoHandleList) {
        if (algoHdl.ptr() && algoHdl->getEnable()) {
            switch (algoHdl->getAlgoType()) {
            case RK_AIQ_ALGO_TYPE_AE:
                mAiqExpParamsPool           = new RkAiqExpParamsPool("RkAiqExpParams", MAX_AEC_EFFECT_FNUM * 4);
                mAiqIrisParamsPool          = new RkAiqIrisParamsPool("RkAiqIrisParams", 4);
                mAiqIspAecParamsPool        = new RkAiqIspAecParamsPool("RkAiqIspAecParams", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspHistParamsPool       = new RkAiqIspHistParamsPool("RkAiqIspHistParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AWB:
                mAiqIspAwbV21ParamsPool     = new RkAiqIspAwbParamsPoolV21("RkAiqIspAwbV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspAwbGainParamsPool    = new RkAiqIspAwbGainParamsPool("RkAiqIspAwbGainParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AF:
                mAiqFocusParamsPool         = new RkAiqFocusParamsPool("RkAiqFocusParams", 4);
                mAiqIspAfParamsPool         = new RkAiqIspAfParamsPool("RkAiqIspAfParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADPCC:
                mAiqIspDpccParamsPool       = new RkAiqIspDpccParamsPool("RkAiqIspDpccParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AMERGE:
                mAiqIspMergeParamsPool      = new RkAiqIspMergeParamsPool("RkAiqIspMergeParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADHAZ:
                mAiqIspDehazeV21ParamsPool  = new RkAiqIspDehazeParamsPoolV21("RkAiqIspDehazeV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_A3DLUT:
                mAiqIspLut3dParamsPool      = new RkAiqIspLut3dParamsPool("RkAiqIspLut3dParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACCM:
                mAiqIspCcmParamsPool        = new RkAiqIspCcmParamsPool("RkAiqIspCcmParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ALSC:
                mAiqIspLscParamsPool        = new RkAiqIspLscParamsPool("RkAiqIspLscParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ABLC:
                mAiqIspBlcV21ParamsPool     = new RkAiqIspBlcParamsPoolV21("RkAiqIspBlcParamsV21", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ARAWNR:
                mAiqIspBaynrV21ParamsPool      = new RkAiqIspBaynrParamsPoolV21("RkAiqIspRawnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AGIC:
                mAiqIspGicV21ParamsPool     = new RkAiqIspGicParamsPoolV21("RkAiqIspGicV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADEBAYER:
                mAiqIspDebayerParamsPool    = new RkAiqIspDebayerParamsPool("RkAiqIspDebayerParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ALDCH:
                mAiqIspLdchParamsPool       = new RkAiqIspLdchParamsPool("RkAiqIspLdchParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AGAMMA:
                mAiqIspAgammaParamsPool     = new RkAiqIspAgammaParamsPool("RkAiqIspAgammaParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADEGAMMA:
                mAiqIspAdegammaParamsPool     = new RkAiqIspAdegammaParamsPool("RkAiqIspAdegammaParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AWDR:
                mAiqIspWdrParamsPool        = new RkAiqIspWdrParamsPool("RkAiqIspWdrParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AR2Y:
                mAiqIspCsmParamsPool        = new RkAiqIspCsmParamsPool("RkAiqIspCsmParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACGC:
                mAiqIspCgcParamsPool        = new RkAiqIspCgcParamsPool("RkAiqIspCgcParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACP:
                mAiqIspCpParamsPool         = new RkAiqIspCpParamsPool("RkAiqIspCpParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AIE:
                mAiqIspIeParamsPool         = new RkAiqIspIeParamsPool("RkAiqIspIeParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AYNR:
                mAiqIspYnrV21ParamsPool     = new RkAiqIspYnrParamsPoolV21("RkAiqIspYnrV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACNR:
                mAiqIspCnrV21ParamsPool     = new RkAiqIspCnrParamsPoolV21("RkAiqIspCnrV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ASHARP:
                mAiqIspSharpenV21ParamsPool   = new RkAiqIspSharpenParamsPoolV21("RkAiqIspSharpenV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADRC:
                mAiqIspDrcV21ParamsPool     = new RkAiqIspDrcParamsPoolV21("RkAiqIspDrcV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            default:
                break;
            }
        }
    }
}

XCamReturn
RkAiqCoreV21::getAiqParamsBuffer(RkAiqFullParams* aiqParams, enum rk_aiq_core_analyze_type_e type)
{
#define NEW_PARAMS_BUFFER(lc, BC) \
    if (mAiqIsp##lc##ParamsPool->has_free_items()) { \
        aiqParams->m##lc##Params = mAiqIsp##lc##ParamsPool->get_item(); \
    } else { \
        LOGE_ANALYZER("no free %s buffer!", #BC); \
        return XCAM_RETURN_ERROR_MEM; \
    } \

#define NEW_PARAMS_BUFFER_V21(lc, BC) \
    if (mAiqIsp##lc##V21ParamsPool->has_free_items()) { \
        aiqParams->m##lc##V21Params = mAiqIsp##lc##V21ParamsPool->get_item(); \
    } else { \
        LOGE_ANALYZER("no free %s buffer!", #BC); \
        return XCAM_RETURN_ERROR_MEM; \
    } \

    std::list<int>& algo_list =
        mRkAiqCoreGroupManager->getGroupAlgoList(type);

    for (auto algo_type : algo_list) {
        SmartPtr<RkAiqHandle>* algoHdl = getCurAlgoTypeHandle(algo_type);
        if (!(algoHdl && (*algoHdl)->getEnable()))
                continue;

        switch (algo_type) {
        case RK_AIQ_ALGO_TYPE_AE:
            if (mAiqExpParamsPool->has_free_items()) {
                aiqParams->mExposureParams = mAiqExpParamsPool->get_item();
            } else {
                LOGE_ANALYZER("no free exposure params buffer!");
                return XCAM_RETURN_ERROR_MEM;
            }

            if (mAiqIrisParamsPool->has_free_items()) {
                aiqParams->mIrisParams = mAiqIrisParamsPool->get_item();
            } else {
                LOGE_ANALYZER("no free iris params buffer!");
                return XCAM_RETURN_ERROR_MEM;
            }

            NEW_PARAMS_BUFFER(Aec, aec);
            NEW_PARAMS_BUFFER(Hist, hist);
            break;
        case RK_AIQ_ALGO_TYPE_AWB:
            NEW_PARAMS_BUFFER_V21(Awb, awb);
            NEW_PARAMS_BUFFER(AwbGain, awb_gain);
            break;
        case RK_AIQ_ALGO_TYPE_AF:
            if (mAiqFocusParamsPool->has_free_items()) {
                aiqParams->mFocusParams = mAiqFocusParamsPool->get_item();
            } else {
                LOGE_ANALYZER("no free focus params buffer!");
                return XCAM_RETURN_ERROR_MEM;
            }
            NEW_PARAMS_BUFFER(Af, af);
            break;
        case RK_AIQ_ALGO_TYPE_ABLC:
            NEW_PARAMS_BUFFER_V21(Blc, blc);
            break;
        case RK_AIQ_ALGO_TYPE_ADPCC:
            NEW_PARAMS_BUFFER(Dpcc, dpcc);
            break;
        case RK_AIQ_ALGO_TYPE_AMERGE:
            NEW_PARAMS_BUFFER(Merge, merge);
            break;
        case RK_AIQ_ALGO_TYPE_ALSC:
            NEW_PARAMS_BUFFER(Lsc, lsc);
            break;
        case RK_AIQ_ALGO_TYPE_AGIC:
            NEW_PARAMS_BUFFER_V21(Gic, gic);
            break;
        case RK_AIQ_ALGO_TYPE_ADEBAYER:
            NEW_PARAMS_BUFFER(Debayer, debayer);
            break;
        case RK_AIQ_ALGO_TYPE_ACCM:
            NEW_PARAMS_BUFFER(Ccm, ccm);
            break;
        case RK_AIQ_ALGO_TYPE_AGAMMA:
            NEW_PARAMS_BUFFER(Agamma, agamma);
            break;
        case RK_AIQ_ALGO_TYPE_ADEGAMMA:
            NEW_PARAMS_BUFFER(Adegamma, adegamma);
            break;
        case RK_AIQ_ALGO_TYPE_AWDR:
            NEW_PARAMS_BUFFER(Wdr, wdr);
            break;
        case RK_AIQ_ALGO_TYPE_ADHAZ:
            NEW_PARAMS_BUFFER_V21(Dehaze, dehaze);
            break;
        case RK_AIQ_ALGO_TYPE_A3DLUT:
            NEW_PARAMS_BUFFER(Lut3d, lut3d);
            break;
        case RK_AIQ_ALGO_TYPE_ALDCH:
            NEW_PARAMS_BUFFER(Ldch, ldch);
            break;
        case RK_AIQ_ALGO_TYPE_AR2Y:
            break;
        case RK_AIQ_ALGO_TYPE_ACP:
            NEW_PARAMS_BUFFER(Cp, cp);
            break;
        case RK_AIQ_ALGO_TYPE_AIE:
            NEW_PARAMS_BUFFER(Ie, ie);
            break;
        case RK_AIQ_ALGO_TYPE_ACGC:
            NEW_PARAMS_BUFFER(Cgc, cgc);
            break;
        case RK_AIQ_ALGO_TYPE_ASD:
            break;
        case RK_AIQ_ALGO_TYPE_ADRC:
            NEW_PARAMS_BUFFER_V21(Drc, drc);
            break;
        case RK_AIQ_ALGO_TYPE_ARAWNR:
            NEW_PARAMS_BUFFER_V21(Baynr, baynr);
            break;
        case RK_AIQ_ALGO_TYPE_AMFNR:
            NEW_PARAMS_BUFFER(Tnr, tnr);
            break;
        case RK_AIQ_ALGO_TYPE_AYNR:
            NEW_PARAMS_BUFFER_V21(Ynr, ynr);
            break;
        case RK_AIQ_ALGO_TYPE_ACNR:
            NEW_PARAMS_BUFFER_V21(Cnr, cnr);
            break;
        case RK_AIQ_ALGO_TYPE_ASHARP:
            NEW_PARAMS_BUFFER_V21(Sharpen, sharpen);
            break;
        default:
            break;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCoreV21::genIspParamsResult(RkAiqFullParams *aiqParams, enum rk_aiq_core_analyze_type_e type)
{
    SmartPtr<RkAiqFullParams> curParams = mAiqCurParams->data();
    for (auto algoHdl : mCurIspAlgoHandleList) {
        if (algoHdl.ptr() && algoHdl->getEnable() &&
                (mAlgoTypeToGrpMaskMap[algoHdl->getAlgoType()] & grpId2GrpMask(type))) {
            switch (algoHdl->getAlgoType()) {
                case RK_AIQ_ALGO_TYPE_AE:
                    genIspAeResult(aiqParams);
                    curParams->mExposureParams = aiqParams->mExposureParams;
                    curParams->mAecParams      = aiqParams->mAecParams;
                    curParams->mHistParams     = aiqParams->mHistParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AWB:
                    genIspAwbResult(aiqParams);
                    curParams->mAwbV21Params  = aiqParams->mAwbV21Params;
                    curParams->mAwbGainParams = aiqParams->mAwbGainParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AF:
                    genIspAfResult(aiqParams);
                    curParams->mAfParams    = aiqParams->mAfParams;
                    curParams->mFocusParams = aiqParams->mFocusParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ABLC:
                    genIspAblcResult(aiqParams);
                    curParams->mBlcV21Params = aiqParams->mBlcV21Params;
                    break;
                case RK_AIQ_ALGO_TYPE_ADPCC:
                    genIspAdpccResult(aiqParams);
                    curParams->mDpccParams = aiqParams->mDpccParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AMERGE:
                    genIspAmergeResult(aiqParams);
                    curParams->mMergeParams = aiqParams->mMergeParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ALSC:
                    genIspAlscResult(aiqParams);
                    curParams->mLscParams = aiqParams->mLscParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AGIC:
                    genIspAgicResult(aiqParams);
                    curParams->mGicV21Params = aiqParams->mGicV21Params;
                    break;
                case RK_AIQ_ALGO_TYPE_ADEBAYER:
                    genIspAdebayerResult(aiqParams);
                    curParams->mDebayerParams = aiqParams->mDebayerParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ACCM:
                    genIspAccmResult(aiqParams);
                    curParams->mCcmParams = aiqParams->mCcmParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AGAMMA:
                    genIspAgammaResult(aiqParams);
                    curParams->mAgammaParams = aiqParams->mAgammaParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ADEGAMMA:
                    genIspAdegammaResult(aiqParams);
                    curParams->mAdegammaParams = aiqParams->mAdegammaParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AWDR:
                    genIspAwdrResult(aiqParams);
                    curParams->mWdrParams = aiqParams->mWdrParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ADHAZ:
                    genIspAdhazResult(aiqParams);
                    curParams->mDehazeV21Params = aiqParams->mDehazeV21Params;
                    break;
                case RK_AIQ_ALGO_TYPE_A3DLUT:
                    genIspA3dlutResult(aiqParams);
                    curParams->mLut3dParams = aiqParams->mLut3dParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ALDCH:
                    genIspAldchResult(aiqParams);
                    curParams->mLdchParams = aiqParams->mLdchParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AR2Y:
                    genIspAr2yResult(aiqParams);
                    break;
                case RK_AIQ_ALGO_TYPE_ACP:
                    genIspAcpResult(aiqParams);
                    curParams->mCpParams = aiqParams->mCpParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AIE:
                    genIspAieResult(aiqParams);
                    curParams->mIeParams = aiqParams->mIeParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ACGC:
                    genIspAcgcResult(aiqParams);
                    curParams->mCgcParams = aiqParams->mCgcParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ASD:
                    break;
                case RK_AIQ_ALGO_TYPE_ADRC:
                    genIspAdrcResult(aiqParams);
                    curParams->mDrcV21Params = aiqParams->mDrcV21Params;
                    break;
                case RK_AIQ_ALGO_TYPE_ARAWNR:
                    genIspArawnrResult(aiqParams);
                    curParams->mBaynrV21Params = aiqParams->mBaynrV21Params;
                    break;
                case RK_AIQ_ALGO_TYPE_ACNR:
                    genIspAcnrResult(aiqParams);
                    curParams->mCnrV21Params = aiqParams->mCnrV21Params;
                    break;
                case RK_AIQ_ALGO_TYPE_AYNR:
                    genIspAynrResult(aiqParams);
                    curParams->mYnrV21Params = aiqParams->mYnrV21Params;
                    break;
                case RK_AIQ_ALGO_TYPE_ASHARP:
                    genIspAsharpResult(aiqParams);
                    curParams->mSharpenV21Params = aiqParams->mSharpenV21Params;
                    break;
                default:
                    break;
            }
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
SmartPtr<RkAiqHandle>
RkAiqCoreV21::newAlgoHandle(RkAiqAlgoDesComm* algo, bool generic, int version)
{
    if (version == 0)
        return RkAiqCore::newAlgoHandle(algo, generic, 0);

#define NEW_ALGO_HANDLE_WITH_V(lc, BC, v) \
    do {\
        if (algo->type == RK_AIQ_ALGO_TYPE_##BC) { \
            if (generic) { \
                return new RkAiq##lc##V##v##Handle(algo, this); \
            } else { \
                return new RkAiq##lc##V##v##HandleInt(algo, this); \
            }\
        }\
    } while(0)\

    /* NEW_ALGO_HANDLE_V2(Arawnr, ARAWNR); */
    /* NEW_ALGO_HANDLE_V2(Amfnr, AMFNR); */
    /* NEW_ALGO_HANDLE_V2(Aynr, AYNR); */
    /* NEW_ALGO_HANDLE_V2(Acnr, ACNR); */

    //TODO: need to define RkAiqAdrcV1Handle ?
    typedef RkAiqAdrcHandle RkAiqAdrcV1Handle;
    typedef RkAiqAsharpHandle RkAiqAsharpV3Handle;
    typedef RkAiqAynrHandle RkAiqAynrV2Handle;
    typedef RkAiqAcnrHandle RkAiqAcnrV1Handle;
    typedef RkAiqArawnrHandle RkAiqArawnrV2Handle;
    typedef RkAiqAwbHandle RkAiqAwbV21Handle;

    NEW_ALGO_HANDLE_WITH_V(Adrc, ADRC, 1);
    NEW_ALGO_HANDLE_WITH_V(Asharp, ASHARP, 3);
    NEW_ALGO_HANDLE_WITH_V(Aynr, AYNR, 2);
    NEW_ALGO_HANDLE_WITH_V(Acnr, ACNR, 1);
    NEW_ALGO_HANDLE_WITH_V(Arawnr, ARAWNR, 2);
    NEW_ALGO_HANDLE_WITH_V(Awb, AWB, 21);
    return NULL;
}

void
RkAiqCoreV21::copyIspStats(SmartPtr<RkAiqAecStatsProxy>& aecStat,
                        SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                        SmartPtr<RkAiqAfStatsProxy>& afStat,
                        rk_aiq_isp_stats_t* to)
{
    if (aecStat.ptr()) {
        to->aec_stats = aecStat->data()->aec_stats;
        to->frame_id = aecStat->data()->frame_id;
    }
    to->awb_hw_ver = 1;
    if (awbStat.ptr())
        to->awb_stats_v21 = awbStat->data()->awb_stats_v201;
    if (afStat.ptr())
        to->af_stats = afStat->data()->af_stats;
}

XCamReturn
RkAiqCoreV21::genIspArawnrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ARAWNR);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResArawnr* arawnr_com =
        shared->procResComb.arawnr_proc_res;

    if (!arawnr_com) {
        LOGD_ANALYZER("no arawnr result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResArawnrV2Int* arawnr_rk = (RkAiqAlgoProcResArawnrV2Int*)arawnr_com;

        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);

        rk_aiq_isp_baynr_params_v21_t* rawnr_param = params->mBaynrV21Params->data().ptr();
        setResultExpectedEffId(rawnr_param->frame_id, RK_AIQ_ALGO_TYPE_ARAWNR);
        memcpy(&rawnr_param->result.st2DParam,
               &arawnr_rk->stArawnrProcResult.st2DFix,
               sizeof(RK_Bayernr_2D_Fix_V2_t));

        memcpy(&rawnr_param->result.st3DParam,
               &arawnr_rk->stArawnrProcResult.st3DFix,
               sizeof(RK_Bayernr_3D_Fix_V2_t));
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAmfnrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAynrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AYNR);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAynr* aynr_com =
        shared->procResComb.aynr_proc_res;

    if (!aynr_com) {
        LOGD_ANALYZER("no aynr result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAynrV2Int* aynr_rk = (RkAiqAlgoProcResAynrV2Int*)aynr_com;

        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);

        rk_aiq_isp_ynr_params_v21_t* ynr_param = params->mYnrV21Params->data().ptr();
        setResultExpectedEffId(ynr_param->frame_id, RK_AIQ_ALGO_TYPE_AYNR);
        memcpy(&ynr_param->result,
               &aynr_rk->stAynrProcResult.stFix,
               sizeof(RK_YNR_Fix_V2_t));
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAcnrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACNR);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAcnr* acnr_com =
        shared->procResComb.acnr_proc_res;

    if (!acnr_com) {
        LOGD_ANALYZER("no aynr result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAcnrV1Int* acnr_rk = (RkAiqAlgoProcResAcnrV1Int*)acnr_com;

        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_cnr_params_v21_t* cnr_param = params->mCnrV21Params->data().ptr();
        setResultExpectedEffId(cnr_param->frame_id, RK_AIQ_ALGO_TYPE_ACNR);
        memcpy(&cnr_param->result,
               &acnr_rk->stAcnrProcResult.stFix,
               sizeof(RK_CNR_Fix_V1_t));
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAdrcResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADRC);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAdrc* adrc_com =
        shared->procResComb.adrc_proc_res;

    if (!adrc_com) {
        LOGD_ANALYZER("no adrc result");
        return XCAM_RETURN_NO_ERROR;
    }
    // TODO: gen adrc common result
    //RkAiqCore::genIspAdrcResult(params);
    RkAiqAlgoProcResAdrc* adrc_rk = (RkAiqAlgoProcResAdrc*)adrc_com;
    int algo_id = (*handle)->getAlgoId();


    // gen rk adrc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdrcInt* ahdr_rk = (RkAiqAlgoProcResAdrcInt*)adrc_com;

        rk_aiq_isp_drc_params_v21_t* drc_param = params->mDrcV21Params->data().ptr();
        setResultExpectedEffId(drc_param->frame_id, RK_AIQ_ALGO_TYPE_ADRC);

        drc_param->result.DrcProcRes.sw_drc_offset_pow2     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_offset_pow2;
        drc_param->result.DrcProcRes.sw_drc_compres_scl  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_compres_scl;
        drc_param->result.DrcProcRes.sw_drc_position  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_position;
        drc_param->result.DrcProcRes.sw_drc_delta_scalein        = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_delta_scalein;
        drc_param->result.DrcProcRes.sw_drc_hpdetail_ratio      = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_hpdetail_ratio;
        drc_param->result.DrcProcRes.sw_drc_lpdetail_ratio     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_lpdetail_ratio;
        drc_param->result.DrcProcRes.sw_drc_weicur_pix      = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_weicur_pix;
        drc_param->result.DrcProcRes.sw_drc_weipre_frame  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_weipre_frame;
        drc_param->result.DrcProcRes.sw_drc_force_sgm_inv0   = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_force_sgm_inv0;
        drc_param->result.DrcProcRes.sw_drc_motion_scl     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_motion_scl;
        drc_param->result.DrcProcRes.sw_drc_edge_scl   = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_edge_scl;
        drc_param->result.DrcProcRes.sw_drc_space_sgm_inv1    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_space_sgm_inv1;
        drc_param->result.DrcProcRes.sw_drc_space_sgm_inv0     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_space_sgm_inv0;
        drc_param->result.DrcProcRes.sw_drc_range_sgm_inv1     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_range_sgm_inv1;
        drc_param->result.DrcProcRes.sw_drc_range_sgm_inv0 = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_range_sgm_inv0;
        drc_param->result.DrcProcRes.sw_drc_weig_maxl    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_weig_maxl;
        drc_param->result.DrcProcRes.sw_drc_weig_bilat  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_weig_bilat;
        drc_param->result.DrcProcRes.sw_drc_iir_weight  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_iir_weight;
        drc_param->result.DrcProcRes.sw_drc_min_ogain  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_min_ogain;
        drc_param->result.DrcProcRes.sw_drc_adrc_gain  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_adrc_gain;
        for(int i = 0; i < 17; i++) {
            drc_param->result.DrcProcRes.sw_drc_gain_y[i]    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_gain_y[i];
            drc_param->result.DrcProcRes.sw_drc_compres_y[i]    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_compres_y[i];
            drc_param->result.DrcProcRes.sw_drc_scale_y[i]    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_scale_y[i];
        }

        drc_param->result.CompressMode =
            ahdr_rk->AdrcProcRes.CompressMode;

        drc_param->result.isHdrGlobalTmo =
            ahdr_rk->AdrcProcRes.isHdrGlobalTmo;

        drc_param->result.bTmoEn =
            ahdr_rk->AdrcProcRes.bTmoEn;

        drc_param->result.isLinearTmo =
            ahdr_rk->AdrcProcRes.isLinearTmo;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAwbResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AWB);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAwb* awb_com =
        shared->procResComb.awb_proc_res;

    if (!awb_com) {
        LOGD_ANALYZER("no awb result");
        return XCAM_RETURN_NO_ERROR;
    }
    // TODO: gen awb common result
    //RkAiqCore::genIspAwbResult(params);
    RkAiqAlgoProcResAwb* awb_rk = (RkAiqAlgoProcResAwb*)awb_com;

    rk_aiq_isp_awb_params_v21_t *awb_param = params->mAwbV21Params->data().ptr();
    rk_aiq_isp_awb_gain_params_v20_t *awb_gain_param = params->mAwbGainParams->data().ptr();
    setResultExpectedEffId(awb_param->frame_id, RK_AIQ_ALGO_TYPE_AWB);
    setResultExpectedEffId(awb_gain_param->frame_id, RK_AIQ_ALGO_TYPE_AWB);

    awb_gain_param->update_mask = awb_rk->awb_gain_update;
    awb_gain_param->result = awb_rk->awb_gain_algo;

    awb_param->update_mask = awb_rk->awb_cfg_update;
    awb_param->result = awb_rk->awb_hw1_para;

    int algo_id = (*handle)->getAlgoId();

    // gen rk awb result
    if (algo_id == 0) {
        RkAiqAlgoProcResAwbInt* awb_rk_int = (RkAiqAlgoProcResAwbInt*)awb_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAblcResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ABLC);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAblc* ablc_com =
        shared->procResComb.ablc_proc_res;

    if (!ablc_com) {
        LOGD_ANALYZER("no ablc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ablc common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk ablc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAblcInt* ablc_rk = (RkAiqAlgoProcResAblcInt*)ablc_com;
        rk_aiq_isp_blc_params_v21_t *blc_param = params->mBlcV21Params->data().ptr();
        setResultExpectedEffId(blc_param->frame_id, RK_AIQ_ALGO_TYPE_ABLC);

        memcpy(&blc_param->result.v0, &ablc_rk->ablc_proc_res,
               sizeof(rk_aiq_isp_blc_t));
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAgicResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AGIC);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAgic* agic_com =
        shared->procResComb.agic_proc_res;

    if (!agic_com) {
        LOGD_ANALYZER("no gic result");
        return XCAM_RETURN_NO_ERROR;
    }
    // TODO: gen adrc common result
    //RkAiqCore::genIspAdrcResult(params);
    RkAiqAlgoProcResAgic* agic_rk = (RkAiqAlgoProcResAgic*)agic_com;
    int algo_id = (*handle)->getAlgoId();

    // gen rk adrc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAgicInt* agic_rk = (RkAiqAlgoProcResAgicInt*)agic_com;
        rk_aiq_isp_gic_params_v21_t *gic_param = params->mGicV21Params->data().ptr();
        setResultExpectedEffId(gic_param->frame_id, RK_AIQ_ALGO_TYPE_AGIC);

        gic_param->result.gic_en  = agic_rk->gicRes.gic_en;
        gic_param->result.ProcResV21.regmingradthrdark2  = agic_rk->gicRes.ProcResV21.regmingradthrdark2;
        gic_param->result.ProcResV21.regmingradthrdark1  = agic_rk->gicRes.ProcResV21.regmingradthrdark1;
        gic_param->result.ProcResV21.regminbusythre  = agic_rk->gicRes.ProcResV21.regminbusythre;
        gic_param->result.ProcResV21.regdarkthre  = agic_rk->gicRes.ProcResV21.regdarkthre;
        gic_param->result.ProcResV21.regmaxcorvboth  = agic_rk->gicRes.ProcResV21.regmaxcorvboth;
        gic_param->result.ProcResV21.regdarktthrehi  = agic_rk->gicRes.ProcResV21.regdarktthrehi;
        gic_param->result.ProcResV21.regkgrad2dark  = agic_rk->gicRes.ProcResV21.regkgrad2dark;
        gic_param->result.ProcResV21.regkgrad1dark  = agic_rk->gicRes.ProcResV21.regkgrad1dark;
        gic_param->result.ProcResV21.regstrengthglobal_fix  = agic_rk->gicRes.ProcResV21.regstrengthglobal_fix;
        gic_param->result.ProcResV21.regdarkthrestep  = agic_rk->gicRes.ProcResV21.regdarkthrestep;
        gic_param->result.ProcResV21.regkgrad2  = agic_rk->gicRes.ProcResV21.regkgrad2;
        gic_param->result.ProcResV21.regkgrad1  = agic_rk->gicRes.ProcResV21.regkgrad1;
        gic_param->result.ProcResV21.reggbthre  = agic_rk->gicRes.ProcResV21.reggbthre;
        gic_param->result.ProcResV21.regmaxcorv  = agic_rk->gicRes.ProcResV21.regmaxcorv;
        gic_param->result.ProcResV21.regmingradthr2  = agic_rk->gicRes.ProcResV21.regmingradthr2;
        gic_param->result.ProcResV21.regmingradthr1  = agic_rk->gicRes.ProcResV21.regmingradthr1;
        gic_param->result.ProcResV21.gr_ratio  = agic_rk->gicRes.ProcResV21.gr_ratio;
        gic_param->result.ProcResV21.noise_scale  = agic_rk->gicRes.ProcResV21.noise_scale;
        gic_param->result.ProcResV21.noise_base  = agic_rk->gicRes.ProcResV21.noise_base;
        gic_param->result.ProcResV21.diff_clip  = agic_rk->gicRes.ProcResV21.diff_clip;
        for(int i = 0; i < 15; i++)
            gic_param->result.ProcResV21.sigma_y[i]  = agic_rk->gicRes.ProcResV21.sigma_y[i];

    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAccmResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACCM);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAccm* accm_com =
        shared->procResComb.accm_proc_res;

    if (!accm_com) {
        LOGD_ANALYZER("no accm result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen accm common result
    RkAiqAlgoProcResAccm* accm_rk = (RkAiqAlgoProcResAccm*)accm_com;
    rk_aiq_isp_ccm_params_v20_t *ccm_param = params->mCcmParams->data().ptr();
    setResultExpectedEffId(ccm_param->frame_id, RK_AIQ_ALGO_TYPE_ACCM);
    ccm_param->result = accm_rk->accm_hw_conf;

    int algo_id = (*handle)->getAlgoId();

    // gen rk accm result
    if (algo_id == 0) {
        RkAiqAlgoProcResAccmInt* accm_rk_int = (RkAiqAlgoProcResAccmInt*)accm_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAdhazResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADHAZ);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAdhaz* adhaz_com =
        shared->procResComb.adhaz_proc_res;

    if (!adhaz_com) {
        LOGD_ANALYZER("no adehaze result");
        return XCAM_RETURN_NO_ERROR;
    }
    // TODO: gen adrc common result
    //RkAiqCore::genIspAdrcResult(params);
    RkAiqAlgoProcResAdhaz* adhaz_rk = (RkAiqAlgoProcResAdhaz*)adhaz_com;
    int algo_id = (*handle)->getAlgoId();

    // gen rk adrc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdhazInt* adhaz_rk = (RkAiqAlgoProcResAdhazInt*)adhaz_com;
        rk_aiq_isp_dehaze_params_v21_t* dehaze_param = params->mDehazeV21Params->data().ptr();
        setResultExpectedEffId(dehaze_param->frame_id, RK_AIQ_ALGO_TYPE_ADHAZ);

        dehaze_param->result.enhance_en     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.enhance_en;
        dehaze_param->result.air_lc_en  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.air_lc_en;
        dehaze_param->result.hpara_en   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.hpara_en;
        dehaze_param->result.hist_en    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.hist_en;
        dehaze_param->result.dc_en  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.dc_en;
        dehaze_param->result.yblk_th    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.yblk_th;
        dehaze_param->result.yhist_th   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.yhist_th;
        dehaze_param->result.dc_max_th  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.dc_max_th;
        dehaze_param->result.dc_min_th  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.dc_min_th;
        dehaze_param->result.wt_max     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.wt_max;
        dehaze_param->result.bright_max     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.bright_max;
        dehaze_param->result.bright_min     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.bright_min;
        dehaze_param->result.tmax_base  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.tmax_base;
        dehaze_param->result.dark_th    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.dark_th;
        dehaze_param->result.air_max    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.air_max;
        dehaze_param->result.air_min    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.air_min;
        dehaze_param->result.tmax_max   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.tmax_max;
        dehaze_param->result.tmax_off   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.tmax_off;
        dehaze_param->result.hist_k     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.hist_k;
        dehaze_param->result.hist_th_off    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.hist_th_off;
        dehaze_param->result.hist_min   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.hist_min;
        dehaze_param->result.hist_gratio    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.hist_gratio;
        dehaze_param->result.hist_scale     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.hist_scale;
        dehaze_param->result.enhance_value  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.enhance_value;
        dehaze_param->result.enhance_chroma     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.enhance_chroma;
        dehaze_param->result.iir_wt_sigma   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.iir_wt_sigma;
        dehaze_param->result.iir_sigma  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.iir_sigma;
        dehaze_param->result.stab_fnum  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.stab_fnum;
        dehaze_param->result.iir_tmax_sigma     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.iir_tmax_sigma;
        dehaze_param->result.iir_air_sigma  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.iir_air_sigma;
        dehaze_param->result.iir_pre_wet    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.iir_pre_wet;
        dehaze_param->result.cfg_wt     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.cfg_wt;
        dehaze_param->result.cfg_air    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.cfg_air;
        dehaze_param->result.cfg_alpha  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.cfg_alpha;
        dehaze_param->result.cfg_gratio     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.cfg_gratio;
        dehaze_param->result.cfg_tmax   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.cfg_tmax;
        dehaze_param->result.range_sima     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.range_sima;
        dehaze_param->result.space_sigma_cur    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.space_sigma_cur;
        dehaze_param->result.space_sigma_pre    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.space_sigma_pre;
        dehaze_param->result.dc_weitcur     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.dc_weitcur;
        dehaze_param->result.bf_weight  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.bf_weight;
        dehaze_param->result.gaus_h0    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.gaus_h0;
        dehaze_param->result.gaus_h1    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.gaus_h1;
        dehaze_param->result.gaus_h2    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.gaus_h2;

        //LOGE_ANALYZER("dc_en%d enhance_en:%d",dehaze_param->result.dc_en,dehaze_param->result.enhance_en);

        for(int i = 0; i < 17; i++)
            dehaze_param->result.enh_curve[i]     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV21.enh_curve[i];

    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCoreV21::genIspAsharpResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ASHARP);
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAsharp* asharp_com =
        shared->procResComb.asharp_proc_res;

    if (!asharp_com) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAsharpV3Int* asharp_rk = (RkAiqAlgoProcResAsharpV3Int*)asharp_com;

        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);

        rk_aiq_isp_sharpen_params_v21_t* sharp_param = params->mSharpenV21Params->data().ptr();
        setResultExpectedEffId(sharp_param->frame_id, RK_AIQ_ALGO_TYPE_ASHARP);
        memcpy(&sharp_param->result,
               &asharp_rk->stAsharpProcResult.stFix,
               sizeof(RK_SHARP_Fix_V3_t));
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

} //namespace RkCam
