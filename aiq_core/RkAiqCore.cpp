/*
 * rkisp_aiq_core.h
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
#include "RkAiqCore.h"

#include <iostream>
#include <fstream>

#include "RkAiqHandleInt.h"
#include "v4l2_buffer_proxy.h"
#include "acp/rk_aiq_algo_acp_itf.h"
#include "ae/rk_aiq_algo_ae_itf.h"
#include "awb/rk_aiq_algo_awb_itf.h"
#include "af/rk_aiq_algo_af_itf.h"
#include "anr/rk_aiq_algo_anr_itf.h"
#include "asd/rk_aiq_algo_asd_itf.h"
#include "amerge/rk_aiq_algo_amerge_itf.h"
#include "atmo/rk_aiq_algo_atmo_itf.h"
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
#include "aeis/rk_aiq_algo_aeis_itf.h"
#include "amd/rk_aiq_algo_amd_itf.h"
#include "arawnr/rk_aiq_abayernr_algo_itf_v1.h"
#include "aynr/rk_aiq_aynr_algo_itf_v1.h"
#include "auvnr/rk_aiq_auvnr_algo_itf_v1.h"
#include "amfnr/rk_aiq_amfnr_algo_itf_v1.h"
#include "again/rk_aiq_again_algo_itf.h"


#ifdef RK_SIMULATOR_HW
#include "simulator/isp20_hw_simulator.h"
#else
#include "isp20/Isp20Evts.h"
#include "isp20/Isp20StatsBuffer.h"
#include "isp20/rkisp2-config.h"
#include "isp20/rkispp-config.h"
#endif
#include <fcntl.h>
#include <unistd.h>
#include "SPStreamProcUnit.h"
// #include "MessageBus.h"
#include "shared_data_api_wrapper.h"
#include "RkAiqAnalyzeGroupManager.h"

namespace RkCam {
#define EPSINON 0.0000001

/*
 * isp/ispp pipeline algos ordered array, and the analyzer
 * will run these algos one by one.
 */
/*
 * isp gets the stats from frame n-1,
 * and the new parameters take effect on frame n+1
 */
#define ISP_PARAMS_EFFECT_DELAY_CNT 2

static RkAiqGrpCondition_t aeGrpCond[]     = {
    [0] = { XCAM_MESSAGE_AEC_STATS_OK,      ISP_PARAMS_EFFECT_DELAY_CNT },
};
static RkAiqGrpConditions_t aeGrpConds     = { grp_conds_array_info(aeGrpCond) };

static RkAiqGrpCondition_t awbGrpCond[]    = {
    [0] = {XCAM_MESSAGE_AE_PRE_RES_OK,      0},
    [1] = {XCAM_MESSAGE_AWB_STATS_OK,       ISP_PARAMS_EFFECT_DELAY_CNT },
};
static RkAiqGrpConditions_t awbGrpConds    = {grp_conds_array_info(awbGrpCond) };

static RkAiqGrpCondition_t measGrpCond[]   = {
    [0] = { XCAM_MESSAGE_ISP_STATS_OK,      ISP_PARAMS_EFFECT_DELAY_CNT },
};
static RkAiqGrpConditions_t measGrpConds   = { grp_conds_array_info(measGrpCond) };

static RkAiqGrpCondition_t otherGrpCond[]  = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
};
static RkAiqGrpConditions_t otherGrpConds  = { grp_conds_array_info(otherGrpCond) };

static RkAiqGrpCondition_t amdGrpCond[]    = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
    [1] = { XCAM_MESSAGE_ISP_POLL_SP_OK,    0 },
    [2] = { XCAM_MESSAGE_ISP_GAIN_OK,       0 },
};
static RkAiqGrpConditions_t amdGrpConds    = { grp_conds_array_info(amdGrpCond) };

static RkAiqGrpCondition_t amfnrGrpCond[]  = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
    [1] = { XCAM_MESSAGE_ISP_GAIN_OK,       0 },
    [2] = { XCAM_MESSAGE_ISPP_GAIN_KG_OK,   0 },
    [3] = { XCAM_MESSAGE_AMD_PROC_RES_OK,   0 },
};
static RkAiqGrpConditions_t amfnrGrpConds  = { grp_conds_array_info(amfnrGrpCond) };

static RkAiqGrpCondition_t aynrGrpCond[]   = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
    [1] = { XCAM_MESSAGE_ISP_GAIN_OK,       0 },
    [2] = { XCAM_MESSAGE_ISPP_GAIN_WR_OK,   0 },
    [3] = { XCAM_MESSAGE_AMD_PROC_RES_OK,   0 },
};
static RkAiqGrpConditions_t aynrGrpConds   = { grp_conds_array_info(aynrGrpCond) };

static RkAiqGrpCondition_t lscGrpCond[]    = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
    [1] = { XCAM_MESSAGE_ISP_POLL_TX_OK,    0 },
    [2] = { XCAM_MESSAGE_AWB_STATS_OK,      ISP_PARAMS_EFFECT_DELAY_CNT },
    [3] = { XCAM_MESSAGE_AE_PROC_RES_OK,    0 },
};
static RkAiqGrpConditions_t lscGrpConds    = { grp_conds_array_info(lscGrpCond) };

static RkAiqGrpCondition_t eisGrpCond[]    = {
    [0] = { XCAM_MESSAGE_SOF_INFO_OK,       0 },
    [1] = { XCAM_MESSAGE_ORB_STATS_OK,      0 },
    [2] = { XCAM_MESSAGE_NR_IMG_OK,         0 },
};
static RkAiqGrpConditions_t eisGrpConds    = { grp_conds_array_info(eisGrpCond) };

static RkAiqGrpCondition_t orbGrpCond[]    = {
    [0] = { XCAM_MESSAGE_ORB_STATS_OK,      0 },
};
static RkAiqGrpConditions_t orbGrpConds    = { grp_conds_array_info(orbGrpCond) };


const static struct RkAiqAlgoDesCommExt g_default_3a_des[] = {
    { &g_RkIspAlgoDescAe.common, RK_AIQ_CORE_ANALYZE_AE, 0, 0, aeGrpConds },
    { &g_RkIspAlgoDescAwb.common, RK_AIQ_CORE_ANALYZE_AWB, 0, 0, awbGrpConds },
    { &g_RkIspAlgoDescAf.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds },
    { &g_RkIspAlgoDescAblc.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAdegamma.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAdpcc.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds },
    { &g_RkIspAlgoDescAmerge.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds },
    { &g_RkIspAlgoDescAtmo.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds },
#if ANR_NO_SEPERATE_MARCO
    { &g_RkIspAlgoDescAnr.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds },
#else
#if 0
    { &g_RkIspAlgoDescArawnr.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds },
    { &g_RkIspAlgoDescAynr.common, RK_AIQ_CORE_ANALYZE_AYNR, 0, 0, aynrGrpConds },
    { &g_RkIspAlgoDescAcnr.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds },
    { &g_RkIspAlgoDescAmfnr.common, RK_AIQ_CORE_ANALYZE_AMFNR, 0, 0, amfnrGrpConds },
    { &g_RkIspAlgoDescAgain.common, RK_AIQ_CORE_ANALYZE_AMFNR, 0, 0, amfnrGrpConds },
#else
    { &g_RkIspAlgoDescArawnr.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds},
    { &g_RkIspAlgoDescAynr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds},
    { &g_RkIspAlgoDescAcnr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds},
    { &g_RkIspAlgoDescAmfnr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds},
    { &g_RkIspAlgoDescAgain.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds},
#endif
#endif
    { &g_RkIspAlgoDescAlsc.common, RK_AIQ_CORE_ANALYZE_LSC, 0, 0, lscGrpConds },
    { &g_RkIspAlgoDescAgic.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAdebayer.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAccm.common, RK_AIQ_CORE_ANALYZE_MEAS, 0, 0, measGrpConds },
    { &g_RkIspAlgoDescAgamma.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAwdr.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAdhaz.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescA3dlut.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAldch.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAr2y.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAcp.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAie.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAsharp.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAorb.common, RK_AIQ_CORE_ANALYZE_ORB, 0, 0, orbGrpConds },
    { &g_RkIspAlgoDescAcgc.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAsd.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    //{ &g_RkIspAlgoDescAfec.common, RK_AIQ_CORE_ANALYZE_OTHER, 0, 0, otherGrpConds },
    { &g_RkIspAlgoDescAeis.common, RK_AIQ_CORE_ANALYZE_EIS, 0, 0, eisGrpConds },
#if 0
    { &g_RkIspAlgoDescAmd.common, RK_AIQ_CORE_ANALYZE_AMD, 0, 0, amdGrpConds },
#endif
    { NULL, RK_AIQ_CORE_ANALYZE_ALL, 0, 0 },
};

bool
RkAiqCoreThread::loop()
{
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;
    SmartPtr<VideoBuffer> stats = mStatsQueue.pop (timeout);

    if (!stats.ptr()) {
        LOGW_ANALYZER("RkAiqCoreThread got empty stats, stop thread");
        return false;
    }

    XCamReturn ret = mRkAiqCore->analyze (stats);
    if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_BYPASS)
        return true;

    LOGE_ANALYZER("RkAiqCoreThread failed to analyze 3a stats");

    EXIT_ANALYZER_FUNCTION();

    return false;
}

bool
RkAiqCoreEvtsThread::loop()
{
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;

    SmartPtr<ispHwEvt_t> evts = mEvtsQueue.pop (timeout);
    if (!evts.ptr()) {
        LOGW_ANALYZER("RkAiqCoreEvtsThread got empty stats, stop thread");
        return false;
    }

    XCamReturn ret = mRkAiqCore->events_analyze (evts);
    if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_BYPASS)
        return true;

    LOGE_ANALYZER("RkAiqCoreEvtsThread failed to analyze events");

    EXIT_ANALYZER_FUNCTION();

    return false;
}

// notice that some pool shared items may be cached by other
// modules(e.g. CamHwIsp20), so here should consider the cached number
uint16_t RkAiqCore::DEFAULT_POOL_SIZE = 15;

RkAiqCore::RkAiqCore()
    : mRkAiqCoreTh(new RkAiqCoreThread(this))
    , mRkAiqCorePpTh(new RkAiqCoreThread(this))
    , mRkAiqCoreEvtsTh(new RkAiqCoreEvtsThread(this))
    , mState(RK_AIQ_CORE_STATE_INVALID)
    , mRkAiqCoreMsgTh(new MessageThread(this))
    , mCb(NULL)
    , mAiqParamsPool(new RkAiqFullParamsPool("RkAiqFullParams", 16))
    , mAiqCpslParamsPool(new RkAiqCpslParamsPool("RkAiqCpslParamsPool", 4))
    , mAiqStatsPool(new RkAiqStatsPool("RkAiqStatsPool", 4))
    , mAiqIspStatsIntPool(new RkAiqIspStatsIntPool("RkAiqIspStatsIntPool", 10))
    , mAiqSofInfoWrapperPool(new RkAiqSofInfoWrapperPool("RkAiqSofPoolWrapper", 16))
    , mAiqAecStatsPool(new RkAiqAecStatsPool("RkAiqAecStatsPool", 10))
    , mAiqAwbStatsPool(new RkAiqAwbStatsPool("RkAiqAwbStatsPool", 10))
    , mAiqAtmoStatsPool(new RkAiqAtmoStatsPool("RkAiqAtmoStatsPool", 10))
    , mAiqAdehazeStatsPool(new RkAiqAdehazeStatsPool("RkAiqAdehazeStatsPool", 10))
    , mAiqAfStatsPool(new RkAiqAfStatsPool("RkAiqAfStatsPool", 10))
    , mAiqOrbStatsIntPool(new RkAiqOrbStatsPool("RkAiqOrbStatsPool", 10))
    , mCustomEnAlgosMask(0xffffffff)
{
    ENTER_ANALYZER_FUNCTION();
    // mAlogsSharedParams.reset();
    mAlogsComSharedParams.reset();
    xcam_mem_clear(mHwInfo);
    mCurCpslOn = false;
    mStrthLed = 0.0f;
    mStrthIr = 0.0f;
    mGrayMode = RK_AIQ_GRAY_MODE_CPSL;
    firstStatsReceived = false;

    SmartPtr<RkAiqFullParams> fullParam = new RkAiqFullParams();
    mAiqCurParams = new RkAiqFullParamsProxy(fullParam );
    mHasPp = true;
    mIspOnline = false;
    mAlgosDesArray = g_default_3a_des;
    mIspHwVer  = 0;
    mSafeEnableAlgo = true;
    mLastAnalyzedId = 0;

    mTranslator = new RkAiqResourceTranslator();
    EXIT_ANALYZER_FUNCTION();
}

RkAiqCore::~RkAiqCore()
{
    ENTER_ANALYZER_FUNCTION();
    if (mAlogsComSharedParams.resourcePath) {
        xcam_free((void*)(mAlogsComSharedParams.resourcePath));
        mAlogsComSharedParams.resourcePath = NULL;
    }
    EXIT_ANALYZER_FUNCTION();
}

void RkAiqCore::initCpsl()
{
    queryCpsLtCap(mCpslCap);

    rk_aiq_cpsl_cfg_t* cfg = &mAlogsComSharedParams.cpslCfg;
    const CamCalibDbContext_t* aiqCalib = mAlogsComSharedParams.calibv2;
    CalibDbV2_Cpsl_t* calibv2_cpsl_db =
        (CalibDbV2_Cpsl_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, cpsl));
    CalibDbV2_Cpsl_Param_t* calibv2_cpsl_calib = &calibv2_cpsl_db->param;
    // TODO: something from calib
    if (mCpslCap.modes_num > 0 && calibv2_cpsl_calib->enable) {
        if (calibv2_cpsl_calib->mode == 0) {
            cfg->mode = RK_AIQ_OP_MODE_AUTO;
        } else if (calibv2_cpsl_calib->mode == 1) {
            cfg->mode = RK_AIQ_OP_MODE_MANUAL;
        } else {
            cfg->mode = RK_AIQ_OP_MODE_INVALID;
        }

        if (calibv2_cpsl_calib->light_src == 0) {
            cfg->lght_src = RK_AIQ_CPSLS_LED;
        } else if (calibv2_cpsl_calib->light_src == 1) {
            cfg->lght_src = RK_AIQ_CPSLS_IR;
        } else if (calibv2_cpsl_calib->light_src == 2) {
            cfg->lght_src = RK_AIQ_CPSLS_MIX;
        } else {
            cfg->lght_src = RK_AIQ_CPSLS_INVALID;
        }
        cfg->gray_on = calibv2_cpsl_calib->force_gray;
        if (cfg->mode == RK_AIQ_OP_MODE_AUTO) {
            cfg->u.a.sensitivity = calibv2_cpsl_calib->auto_adjust_sens;
            cfg->u.a.sw_interval = calibv2_cpsl_calib->auto_sw_interval;
            LOGI_ANALYZER("mode sensitivity %f, interval time %d s\n",
                          cfg->u.a.sensitivity, cfg->u.a.sw_interval);
        } else {
            cfg->u.m.on = calibv2_cpsl_calib->manual_on;
            cfg->u.m.strength_ir = calibv2_cpsl_calib->manual_strength;
            cfg->u.m.strength_led = calibv2_cpsl_calib->manual_strength;
            LOGI_ANALYZER("on %d, strength_led %f, strength_ir %f \n",
                          cfg->u.m.on, cfg->u.m.strength_led, cfg->u.m.strength_ir);
        }
    } else {
        cfg->mode = RK_AIQ_OP_MODE_INVALID;
        LOGI_ANALYZER("not support light compensation \n");
    }
}

XCamReturn
RkAiqCore::init(const char* sns_ent_name, const CamCalibDbContext_t* aiqCalib,
                const CamCalibDbV2Context_t* aiqCalibv2)
{
    ENTER_ANALYZER_FUNCTION();

    if (mState != RK_AIQ_CORE_STATE_INVALID) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    mAlogsComSharedParams.calib = aiqCalib;
    mAlogsComSharedParams.calibv2 = aiqCalibv2;

    const CalibDb_AlgoSwitch_t *algoSwitch = &aiqCalibv2->sys_cfg->algoSwitch;
    if (algoSwitch->enable && algoSwitch->enable_algos) {
        mCustomEnAlgosMask = 0x0;
        for (uint16_t i = 0; i < algoSwitch->enable_algos_len; i++)
            mCustomEnAlgosMask |= 1 << algoSwitch->enable_algos[i];
    }
    LOGI_ANALYZER("mCustomEnAlgosMask: 0x%x\n", mCustomEnAlgosMask);
    addDefaultAlgos(mAlgosDesArray);
    initCpsl();
    newAiqParamsPool();
    newAiqGroupAnayzer();

    mState = RK_AIQ_CORE_STATE_INITED;
    return XCAM_RETURN_NO_ERROR;

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn
RkAiqCore::deInit()
{
    ENTER_ANALYZER_FUNCTION();

    for (auto mapIt = mAlogsGroupSharedParamsMap.begin(); \
         mapIt != mAlogsGroupSharedParamsMap.end();) {
        delete mapIt->second;
        mAlogsGroupSharedParamsMap.erase(mapIt++);
    }
    mAlogsGroupList.clear();

    if (mState == RK_AIQ_CORE_STATE_STARTED || mState == RK_AIQ_CORE_STATE_RUNNING) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    mState = RK_AIQ_CORE_STATE_INVALID;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::start()
{
    ENTER_ANALYZER_FUNCTION();

    if ((mState != RK_AIQ_CORE_STATE_PREPARED) &&
            (mState != RK_AIQ_CORE_STATE_STOPED)) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    mRkAiqCoreTh->triger_start();
    mRkAiqCoreTh->start();
    if (mHasPp) {
        mRkAiqCorePpTh->triger_start();
        mRkAiqCorePpTh->start();
    }
    mRkAiqCoreEvtsTh->triger_start();
    mRkAiqCoreEvtsTh->start();
    mRkAiqCoreMsgTh->triger_start();
    mRkAiqCoreMsgTh->start();
    mRkAiqCoreGroupManager->start();
    if (mThumbnailsService.ptr()) {
        mThumbnailsService->Start();
    }

    mState = RK_AIQ_CORE_STATE_STARTED;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::stop()
{
    ENTER_ANALYZER_FUNCTION();

    if (mState != RK_AIQ_CORE_STATE_STARTED && mState != RK_AIQ_CORE_STATE_RUNNING) {
        LOGW_ANALYZER("in state %d\n", mState);
        return XCAM_RETURN_NO_ERROR;
    }

    mRkAiqCoreTh->triger_stop();
    mRkAiqCoreTh->stop();

    if (mHasPp) {
        mRkAiqCorePpTh->triger_stop();
        mRkAiqCorePpTh->stop();
    }
    mRkAiqCoreEvtsTh->triger_stop();
    mRkAiqCoreEvtsTh->stop();


    mRkAiqCoreMsgTh->triger_stop();
    mRkAiqCoreMsgTh->stop();
    mRkAiqCoreGroupManager->stop();
    if (mThumbnailsService.ptr()) {
        mThumbnailsService->Stop();
    }

    mAiqStatsCachedList.clear();
    mAiqStatsOutMap.clear();
    mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_INIT;
    mState = RK_AIQ_CORE_STATE_STOPED;
    firstStatsReceived = false;
    mLastAnalyzedId = 0;
    mIspStatsCond.broadcast ();
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::prepare(const rk_aiq_exposure_sensor_descriptor* sensor_des,
                   int mode)
{
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // check state
    if ((mState == RK_AIQ_CORE_STATE_STARTED) ||
            (mState == RK_AIQ_CORE_STATE_INVALID) ||
            (mState == RK_AIQ_CORE_STATE_RUNNING)) {
        LOGW_ANALYZER("in state %d\n", mState);
        return XCAM_RETURN_NO_ERROR;
    }

    bool res_changed = (mAlogsComSharedParams.snsDes.isp_acq_width != 0) &&
        (sensor_des->isp_acq_width != mAlogsComSharedParams.snsDes.isp_acq_width ||
        sensor_des->isp_acq_height != mAlogsComSharedParams.snsDes.isp_acq_height);
    if (res_changed) {
        mAlogsComSharedParams.conf_type |= RK_AIQ_ALGO_CONFTYPE_CHANGERES;
        LOGD_ANALYZER("resolution changed !");
    }

    if ((mState == RK_AIQ_CORE_STATE_STOPED) ||
        (mState == RK_AIQ_CORE_STATE_PREPARED)) {
        mAlogsComSharedParams.conf_type |= RK_AIQ_ALGO_CONFTYPE_KEEPSTATUS;
        LOGD_ANALYZER("prepare from stopped, should keep algo status !");
    }

    mAlogsComSharedParams.snsDes = *sensor_des;
    mAlogsComSharedParams.working_mode = mode;
    mAlogsComSharedParams.spWidth = mSpWidth;
    mAlogsComSharedParams.spHeight = mSpHeight;
    mAlogsComSharedParams.spAlignedWidth = mSpAlignedWidth;
    mAlogsComSharedParams.spAlignedHeight = mSpAlignedHeight;
    CalibDbV2_ColorAsGrey_t *colorAsGrey =
        (CalibDbV2_ColorAsGrey_t*)CALIBDBV2_GET_MODULE_PTR((void*)(mAlogsComSharedParams.calibv2), colorAsGrey);

    CalibDbV2_Thumbnails_t* thumbnails_config_db =
        (CalibDbV2_Thumbnails_t*)CALIBDBV2_GET_MODULE_PTR((void*)(mAlogsComSharedParams.calibv2), thumbnails);
    if (thumbnails_config_db) {
        CalibDbV2_Thumbnails_Param_t* thumbnails_config = &thumbnails_config_db->param;
        if (thumbnails_config->thumbnail_configs_len > 0) {
            mThumbnailsService = new ThumbnailsService();
            if (mThumbnailsService.ptr()) {
                auto ret = mThumbnailsService->Prepare(thumbnails_config);
                if (ret == XCAM_RETURN_NO_ERROR) {
                    auto cb = std::bind(&RkAiqCore::onThumbnailsResult, this, std::placeholders::_1);
                    mThumbnailsService->SetResultCallback(cb);
                } else {
                    mThumbnailsService.release();
                }
            }
        }
    }

    if ((mAlogsComSharedParams.snsDes.sensor_pixelformat == V4L2_PIX_FMT_GREY) ||
            (mAlogsComSharedParams.snsDes.sensor_pixelformat == V4L2_PIX_FMT_Y10) ||
            (mAlogsComSharedParams.snsDes.sensor_pixelformat == V4L2_PIX_FMT_Y12)) {
        mAlogsComSharedParams.is_bw_sensor = true;
        mGrayMode = RK_AIQ_GRAY_MODE_ON;
        mAlogsComSharedParams.gray_mode = true;
    } else {
        mAlogsComSharedParams.is_bw_sensor = false;
        if (colorAsGrey->param.enable) {
            mAlogsComSharedParams.gray_mode = true;
            mGrayMode = RK_AIQ_GRAY_MODE_ON;
        }
    }

    for (auto algoHdl : mCurIspAlgoHandleList) {
        if (algoHdl.ptr() && algoHdl->getEnable()) {
            /* update user initial params */ \
            ret = algoHdl->updateConfig(true);
            RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d update initial user params failed", algoHdl->getAlgoType());
            algoHdl->setReConfig(mState == RK_AIQ_CORE_STATE_STOPED);
            ret = algoHdl->prepare();
            RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed", algoHdl->getAlgoType());
        }
    }

    for (auto algoHdl : mCurIsppAlgoHandleList) {
        if (algoHdl.ptr() && algoHdl->getEnable()) {
            /* update user initial params */ \
            ret = algoHdl->updateConfig(true);
            RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d update initial user params failed", algoHdl->getAlgoType());
            algoHdl->setReConfig(mState == RK_AIQ_CORE_STATE_STOPED);
            ret = algoHdl->prepare();
            RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed", algoHdl->getAlgoType());
        }
    }

    mAlogsComSharedParams.init = true;
    analyzeInternal(RK_AIQ_CORE_ANALYZE_ALL);
    mAlogsComSharedParams.init = false;

#if 0
    if (mAiqCurParams->data()->mIspMeasParams.ptr()) {
        mAiqCurParams->data()->mIspMeasParams->data()->frame_id = 0;
    }

    if (mAiqCurParams->data()->mIspOtherParams.ptr()) {
        mAiqCurParams->data()->mIspOtherParams->data()->frame_id = 0;
    }

    if (mAiqCurParams->data()->mIsppMeasParams.ptr()) {
        mAiqCurParams->data()->mIsppMeasParams->data()->frame_id = 0;
    }

    if (mAiqCurParams->data()->mIsppOtherParams.ptr()) {
        mAiqCurParams->data()->mIsppOtherParams->data()->frame_id = 0;
    }
#endif

    mState = RK_AIQ_CORE_STATE_PREPARED;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

SmartPtr<RkAiqFullParamsProxy>
RkAiqCore::analyzeInternal(enum rk_aiq_core_analyze_type_e type)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (mAlogsComSharedParams.init) {
         // run algos without stats to generate
         // initial params
        CalibDb_Aec_ParaV2_t* calibv2_ae_calib =
            (CalibDb_Aec_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(mAlogsComSharedParams.calibv2), ae_calib));

        auto mapIter = mAlogsGroupSharedParamsMap.begin();
        while (mapIter != mAlogsGroupSharedParamsMap.end()) {
            RkAiqAlgosGroupShared_t* &shared = mapIter->second;
            shared->reset();
            if (!shared->ispStats) {
                SmartPtr<RkAiqIspStatsIntProxy> ispStats = NULL;
                if (mAiqIspStatsIntPool->has_free_items()) {
                    ispStats = mAiqIspStatsIntPool->get_item();
                }
                shared->ispStats = ispStats->data().ptr();
            }
            shared->frameId = 0;
            shared->ispStats->aec_stats_valid = false;
            shared->ispStats->awb_stats_valid = false;
            shared->ispStats->awb_cfg_effect_valid = false;
            shared->ispStats->af_stats_valid = false;
            shared->ispStats->atmo_stats_valid = false;
            shared->aecStatsBuf = nullptr;

            shared->curExp.LinearExp.exp_real_params.analog_gain = \
                calibv2_ae_calib->LinearAeCtrl.InitExp.InitGainValue;
            shared->curExp.LinearExp.exp_real_params.integration_time = \
                calibv2_ae_calib->LinearAeCtrl.InitExp.InitTimeValue;
            for (int32_t i = 0; i < 3; i++) {
                shared->curExp.HdrExp[i].exp_real_params.analog_gain = \
                    calibv2_ae_calib->HdrAeCtrl.InitExp.InitGainValue[i];
                shared->curExp.HdrExp[i].exp_real_params.integration_time = \
                    calibv2_ae_calib->HdrAeCtrl.InitExp.InitTimeValue[i];
            }

            mapIter++;
        }
    }

    SmartPtr<RkAiqFullParamsProxy> aiqParamProxy = NULL;
    if (mAiqParamsPool->has_free_items())
        aiqParamProxy = mAiqParamsPool->get_item();

    if (!aiqParamProxy.ptr()) {
        LOGE_ANALYZER("no free aiq params buffer!");
        return NULL;
    }

    RkAiqFullParams* aiqParams = aiqParamProxy->data().ptr();
    aiqParams->reset();

    ret = getAiqParamsBuffer(aiqParams, type);
    if (ret != XCAM_RETURN_NO_ERROR)
        return NULL;

#if 0
    // for test
    int fd = open("/tmp/cpsl", O_RDWR);
    if (fd != -1) {
        char c;
        read(fd, &c, 1);
        int enable = atoi(&c);

        rk_aiq_cpsl_cfg_t cfg;

        cfg.mode = (RKAiqOPMode_t)enable;
        cfg.lght_src = RK_AIQ_CPSLS_LED;
        if (cfg.mode == RK_AIQ_OP_MODE_AUTO) {
            cfg.u.a.sensitivity = 100;
            cfg.u.a.sw_interval = 60;
            cfg.gray_on = false;
            LOGI_ANALYZER("mode sensitivity %f, interval time %d s\n",
                          cfg.u.a.sensitivity, cfg.u.a.sw_interval);
        } else {
            cfg.gray_on = true;
            cfg.u.m.on = true;
            cfg.u.m.strength_ir = 100;
            cfg.u.m.strength_led = 100;
            LOGI_ANALYZER("on %d, strength_led %f, strength_ir %f\n",
                          cfg.u.m.on, cfg.u.m.strength_led, cfg.u.m.strength_ir);
        }
        close(fd);
        setCpsLtCfg(cfg);
    }
#endif
    ret = preProcess(type);
    RKAIQCORE_CHECK_RET_NULL(ret, "preprocess failed");
    if (type == RK_AIQ_CORE_ANALYZE_OTHER) {
        genCpslResult(aiqParams);
    }

    ret = processing(type);
    RKAIQCORE_CHECK_RET_NULL(ret, "processing failed");

    ret = postProcess(type);
    RKAIQCORE_CHECK_RET_NULL(ret, "post process failed");

    ret = genIspParamsResult(aiqParams, type);

    EXIT_ANALYZER_FUNCTION();

    return aiqParamProxy;
}

XCamReturn
RkAiqCore::getAiqParamsBuffer(RkAiqFullParams* aiqParams, enum rk_aiq_core_analyze_type_e type)
{
#define NEW_PARAMS_BUFFER(lc, BC) \
    if (mAiqIsp##lc##ParamsPool->has_free_items()) { \
        aiqParams->m##lc##Params = mAiqIsp##lc##ParamsPool->get_item(); \
        aiqParams->m##lc##Params->data()->frame_id = -1; \
    } else { \
        LOGE_ANALYZER("no free %s buffer!", #BC); \
        return XCAM_RETURN_ERROR_MEM; \
    } \

    std::list<int>& algo_list =
        mRkAiqCoreGroupManager->getGroupAlgoList(type);

    for (auto type : algo_list) {
            switch (type) {
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
                NEW_PARAMS_BUFFER(Awb, awb);
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
                NEW_PARAMS_BUFFER(Blc, blc);
                break;
            case RK_AIQ_ALGO_TYPE_ADPCC:
                NEW_PARAMS_BUFFER(Dpcc, dpcc);
                break;
#if 0
            case RK_AIQ_ALGO_TYPE_AHDR:
                NEW_PARAMS_BUFFER(Hdr, hdr);
                break;
#else
            case RK_AIQ_ALGO_TYPE_AMERGE:
                NEW_PARAMS_BUFFER(Merge, merge);
                break;
            case RK_AIQ_ALGO_TYPE_ATMO:
                NEW_PARAMS_BUFFER(Tmo, tmo);
                break;
#endif
            case RK_AIQ_ALGO_TYPE_ALSC:
                NEW_PARAMS_BUFFER(Lsc, lsc);
                break;
            case RK_AIQ_ALGO_TYPE_AGIC:
                NEW_PARAMS_BUFFER(Gic, gic);
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
            case RK_AIQ_ALGO_TYPE_AWDR:
                NEW_PARAMS_BUFFER(Wdr, wdr);
                break;
            case RK_AIQ_ALGO_TYPE_ADHAZ:
                NEW_PARAMS_BUFFER(Dehaze, dehaze);
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
                break;
            case RK_AIQ_ALGO_TYPE_ADEGAMMA:
                NEW_PARAMS_BUFFER(Adegamma, adegamma);
                break;
            case RK_AIQ_ALGO_TYPE_ARAWNR:
                NEW_PARAMS_BUFFER(Rawnr, rawnr);
                break;
            case RK_AIQ_ALGO_TYPE_AMFNR:
                NEW_PARAMS_BUFFER(Tnr, tnr);
                break;
            case RK_AIQ_ALGO_TYPE_AYNR:
                NEW_PARAMS_BUFFER(Ynr, ynr);
                break;
            case RK_AIQ_ALGO_TYPE_ACNR:
                NEW_PARAMS_BUFFER(Uvnr, uvnr);
                break;
            case RK_AIQ_ALGO_TYPE_ASHARP:
                NEW_PARAMS_BUFFER(Sharpen, sharpen);
                NEW_PARAMS_BUFFER(Edgeflt, edgeflt);
                break;
            case RK_AIQ_ALGO_TYPE_AORB:
                NEW_PARAMS_BUFFER(Orb, orb);
                break;
            case RK_AIQ_ALGO_TYPE_AFEC:
            case RK_AIQ_ALGO_TYPE_AEIS:
                NEW_PARAMS_BUFFER(Fec, fec);
                break;
            case RK_AIQ_ALGO_TYPE_ANR:
                NEW_PARAMS_BUFFER(Rawnr, rawnr);
                NEW_PARAMS_BUFFER(Tnr, tnr);
                NEW_PARAMS_BUFFER(Ynr, ynr);
                NEW_PARAMS_BUFFER(Uvnr, uvnr);
                NEW_PARAMS_BUFFER(Gain, gain);
                NEW_PARAMS_BUFFER(Motion, motion);
                break;
            case RK_AIQ_ALGO_TYPE_AMD:
                NEW_PARAMS_BUFFER(Md, md);
                break;
            default:
                break;
            }
        }

    return XCAM_RETURN_NO_ERROR;
}

void
RkAiqCore::setResultExpectedEffId(uint32_t& eff_id, enum RkAiqAlgoType_e type)
{
    int groupId = getGroupId(type);
    RkAiqAlgosGroupShared_t* shared = nullptr;

    if (groupId < 0) {
        LOGE_ANALYZER("get group of type %d failed !", type);
        return;
    }

    if (getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR) {
        LOGE_ANALYZER("no shared params for type %d !", type);
        return;
    }

    if (mAlogsComSharedParams.init) {
        // init params, set before streaming
        eff_id = 0;
    } else
        eff_id = shared->frameId;
}

XCamReturn
RkAiqCore::genIspParamsResult(RkAiqFullParams *aiqParams, enum rk_aiq_core_analyze_type_e type)
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
                    curParams->mAwbParams     = aiqParams->mAwbParams;
                    curParams->mAwbGainParams = aiqParams->mAwbGainParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AF:
                    genIspAfResult(aiqParams);
                    curParams->mAfParams = aiqParams->mAfParams;
                    curParams->mFocusParams = aiqParams->mFocusParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ABLC:
                    genIspAblcResult(aiqParams);
                    curParams->mBlcParams = aiqParams->mBlcParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ADPCC:
                    genIspAdpccResult(aiqParams);
                    curParams->mDpccParams = aiqParams->mDpccParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AMERGE:
                    genIspAmergeResult(aiqParams);
                    curParams->mMergeParams = aiqParams->mMergeParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ATMO:
                    genIspAtmoResult(aiqParams);
                    curParams->mTmoParams = aiqParams->mTmoParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ALSC:
                    genIspAlscResult(aiqParams);
                    curParams->mLscParams = aiqParams->mLscParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AGIC:
                    genIspAgicResult(aiqParams);
                    curParams->mGicParams = aiqParams->mGicParams;
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
                case RK_AIQ_ALGO_TYPE_AWDR:
                    genIspAwdrResult(aiqParams);
                    curParams->mWdrParams = aiqParams->mWdrParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ADHAZ:
                    genIspAdhazResult(aiqParams);
                    curParams->mDehazeParams = aiqParams->mDehazeParams;
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
                    break;
                case RK_AIQ_ALGO_TYPE_ADEGAMMA:
                    genIspAdegammaResult(aiqParams);
                    curParams->mAdegammaParams = aiqParams->mAdegammaParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ARAWNR:
                    genIspArawnrResult(aiqParams);
                    curParams->mRawnrParams = aiqParams->mRawnrParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AMFNR:
                    genIspAmfnrResult(aiqParams);
                    curParams->mTnrParams = aiqParams->mTnrParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ACNR:
                    genIspAcnrResult(aiqParams);
                    curParams->mUvnrParams = aiqParams->mUvnrParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AYNR:
                    genIspAynrResult(aiqParams);
                    curParams->mYnrParams = aiqParams->mYnrParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AGAIN:
                    genIspAgainResult(aiqParams);
                    curParams->mGainParams = aiqParams->mGainParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ANR:
                    genIspAnrResult(aiqParams);
                    curParams->mRawnrParams  = aiqParams->mRawnrParams;
                    curParams->mUvnrParams   = aiqParams->mUvnrParams;
                    curParams->mYnrParams    = aiqParams->mYnrParams;
                    curParams->mTnrParams    = aiqParams->mTnrParams;
                    curParams->mYnrParams    = aiqParams->mYnrParams;
                    curParams->mGainParams   = aiqParams->mGainParams;
                    curParams->mMotionParams = aiqParams->mMotionParams;
                    break;
                case RK_AIQ_ALGO_TYPE_ASHARP:
                    genIspAsharpResult(aiqParams);
                    curParams->mSharpenParams = aiqParams->mSharpenParams;
                    curParams->mEdgefltParams = aiqParams->mEdgefltParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AMD:
                    genIspAmdResult(aiqParams);
                    curParams->mMdParams = aiqParams->mMdParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AORB:
                    genIspAorbResult(aiqParams);
                    curParams->mOrbParams = aiqParams->mOrbParams;
                    break;
                case RK_AIQ_ALGO_TYPE_AFEC:
                case RK_AIQ_ALGO_TYPE_AEIS:
                    genIspAfecResult(aiqParams);
                    genIspAeisResult(aiqParams);
                    curParams->mFecParams = aiqParams->mFecParams;
                    break;
                default:
                    break;
            }
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::genIspAeResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AE);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAe* ae_proc =
        shared->procResComb.ae_proc_res;
    RkAiqAlgoPostResAe* ae_post =
        shared->postResComb.ae_post_res;

    rk_aiq_isp_aec_params_v20_t *aec_param = params->mAecParams->data().ptr();
    rk_aiq_isp_hist_params_v20_t *hist_param = params->mHistParams->data().ptr();

    SmartPtr<rk_aiq_exposure_params_wrapper_t> exp_param =
        params->mExposureParams->data();
    SmartPtr<rk_aiq_iris_params_wrapper_t> iris_param =
        params->mIrisParams->data();

    if (!ae_proc) {
        LOGD_ANALYZER("no ae_proc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!ae_post) {
        LOGD_ANALYZER("no ae_post result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ae common result
    int algo_id = (*handle)->getAlgoId();
    // gen common result

    exp_param->aecExpInfo.LinearExp = ae_proc->new_ae_exp.LinearExp;
    memcpy(exp_param->aecExpInfo.HdrExp, ae_proc->new_ae_exp.HdrExp, sizeof(ae_proc->new_ae_exp.HdrExp));
    exp_param->aecExpInfo.frame_length_lines = ae_proc->new_ae_exp.frame_length_lines;
    exp_param->aecExpInfo.line_length_pixels = ae_proc->new_ae_exp.line_length_pixels;
    exp_param->aecExpInfo.pixel_clock_freq_mhz = ae_proc->new_ae_exp.pixel_clock_freq_mhz;
    exp_param->aecExpInfo.Iris.PIris = ae_proc->new_ae_exp.Iris.PIris;

    // TODO Merge
    //iris_param->IrisType = ae_proc->new_ae_exp.Iris.IrisType;
    iris_param->PIris.step = ae_proc->new_ae_exp.Iris.PIris.step;
    iris_param->PIris.update = ae_proc->new_ae_exp.Iris.PIris.update;

#if 0
    isp_param->aec_meas = ae_proc->ae_meas;
    isp_param->hist_meas = ae_proc->hist_meas;
#else
    setResultExpectedEffId(aec_param->frame_id, RK_AIQ_ALGO_TYPE_AE);
    setResultExpectedEffId(hist_param->frame_id, RK_AIQ_ALGO_TYPE_AE);

    aec_param->result = ae_proc->ae_meas;
    hist_param->result = ae_proc->hist_meas;
#endif

    // gen rk ae result
    if (algo_id == 0) {
        RkAiqAlgoProcResAeInt* ae_rk = (RkAiqAlgoProcResAeInt*)ae_proc;
        memcpy(exp_param->exp_tbl, ae_rk->ae_proc_res_rk.exp_set_tbl, sizeof(exp_param->exp_tbl));
        exp_param->exp_tbl_size = ae_rk->ae_proc_res_rk.exp_set_cnt;
        exp_param->algo_id = algo_id;

        RkAiqAlgoPostResAeInt* ae_post_rk = (RkAiqAlgoPostResAeInt*)ae_post;
        iris_param->DCIris.update = ae_post_rk->ae_post_res_rk.DCIris.update;
        iris_param->DCIris.pwmDuty = ae_post_rk->ae_post_res_rk.DCIris.pwmDuty;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAwbResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AWB);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAwb* awb_com =
        shared->procResComb.awb_proc_res;

    rk_aiq_isp_awb_params_v20_t *awb_param = params->mAwbParams->data().ptr();
    rk_aiq_isp_awb_gain_params_v20_t *awb_gain_param = params->mAwbGainParams->data().ptr();

    if (!awb_com) {
        LOGD_ANALYZER("no awb result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen awb common result
    RkAiqAlgoProcResAwb* awb_rk = (RkAiqAlgoProcResAwb*)awb_com;
#if 0
    isp_param->awb_gain_update = awb_rk->awb_gain_update;
    isp_param->awb_cfg_update = awb_rk->awb_cfg_update;
    isp_param->awb_gain = awb_rk->awb_gain_algo;
    isp_param->awb_cfg = awb_rk->awb_hw0_para;
    //isp_param->awb_cfg_v201 = awb_rk->awb_hw1_para;
#else
    // TODO: update states
    // awb_gain_param->result.awb_gain_update = awb_rk->awb_gain_update;
    // isp_param->awb_cfg_update = awb_rk->awb_cfg_update;
    setResultExpectedEffId(awb_gain_param->frame_id, RK_AIQ_ALGO_TYPE_AWB);
    setResultExpectedEffId(awb_param->frame_id, RK_AIQ_ALGO_TYPE_AWB);

    awb_gain_param->result = awb_rk->awb_gain_algo;
    awb_param->result = awb_rk->awb_hw0_para;
#endif

    int algo_id = (*handle)->getAlgoId();

    // gen rk awb result
    if (algo_id == 0) {
        RkAiqAlgoProcResAwbInt* awb_rk_int = (RkAiqAlgoProcResAwbInt*)awb_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAfResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AF);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAf* af_com =
        shared->procResComb.af_proc_res;

    rk_aiq_isp_af_params_v20_t* af_param = params->mAfParams->data().ptr();

    SmartPtr<rk_aiq_focus_params_wrapper_t> focus_param =
        params->mFocusParams->data();
    rk_aiq_focus_params_t* p_focus_param = &focus_param->result;

#if 0
    isp_param->af_cfg_update = false;
#else
    // af_param->af_cfg_update = false;
#endif
    p_focus_param->lens_pos_valid = false;
    p_focus_param->zoom_pos_valid = false;
    if (!af_com) {
        LOGD_ANALYZER("no af result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen af common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk af result
    if (algo_id == 0) {
        RkAiqAlgoProcResAfInt* af_rk = (RkAiqAlgoProcResAfInt*)af_com;

#if 0
        isp_param->af_meas = af_rk->af_proc_res_com.af_isp_param;
        isp_param->af_cfg_update = af_rk->af_proc_res_com.af_cfg_update;
#else
        setResultExpectedEffId(af_param->frame_id, RK_AIQ_ALGO_TYPE_AF);
        af_param->result = af_rk->af_proc_res_com.af_isp_param;
        // isp_param->af_cfg_update = af_rk->af_proc_res_com.af_cfg_update;
#endif

        p_focus_param->next_lens_pos = af_rk->af_proc_res_com.af_focus_param.next_lens_pos;
        p_focus_param->next_zoom_pos = af_rk->af_proc_res_com.af_focus_param.next_zoom_pos;
        p_focus_param->lens_pos_valid = af_rk->af_proc_res_com.af_focus_param.lens_pos_valid;
        p_focus_param->zoom_pos_valid = af_rk->af_proc_res_com.af_focus_param.zoom_pos_valid;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAmergeResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AMERGE);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAmerge* amerge_com =
        shared->procResComb.amerge_proc_res;

    rk_aiq_isp_merge_params_v20_t* merge_param = params->mMergeParams->data().ptr();

    if (!amerge_com) {
        LOGD_ANALYZER("no amerge result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ahdr common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk ahdr result
    if (algo_id == 0) {
        RkAiqAlgoProcResAmergeInt* amerge_rk = (RkAiqAlgoProcResAmergeInt*)amerge_com;

        setResultExpectedEffId(merge_param->frame_id, RK_AIQ_ALGO_TYPE_AMERGE);

        merge_param->result.Res.sw_hdrmge_mode =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_mode;
        merge_param->result.Res.sw_hdrmge_lm_dif_0p9 =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_lm_dif_0p9;
        merge_param->result.Res.sw_hdrmge_ms_dif_0p8 =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_ms_dif_0p8;
        merge_param->result.Res.sw_hdrmge_lm_dif_0p15 =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_lm_dif_0p15;
        merge_param->result.Res.sw_hdrmge_ms_dif_0p15 =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_ms_dif_0p15;
        merge_param->result.Res.sw_hdrmge_gain0 =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_gain0;
        merge_param->result.Res.sw_hdrmge_gain0_inv =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_gain0_inv;
        merge_param->result.Res.sw_hdrmge_gain1 =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_gain1;
        merge_param->result.Res.sw_hdrmge_gain1_inv =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_gain1_inv;
        merge_param->result.Res.sw_hdrmge_gain2 =
            amerge_rk->AmergeProcRes.Res.sw_hdrmge_gain2;
        for(int i = 0; i < 17; i++)
        {
            merge_param->result.Res.sw_hdrmge_e_y[i] =
                amerge_rk->AmergeProcRes.Res.sw_hdrmge_e_y[i];
            merge_param->result.Res.sw_hdrmge_l1_y[i] =
                amerge_rk->AmergeProcRes.Res.sw_hdrmge_l1_y[i];
            merge_param->result.Res.sw_hdrmge_l0_y[i] =
                amerge_rk->AmergeProcRes.Res.sw_hdrmge_l0_y[i];
        }

        merge_param->result.LongFrameMode =
            amerge_rk->AmergeProcRes.LongFrameMode;

    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAtmoResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ATMO);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAtmo* atmo_com =
        shared->procResComb.atmo_proc_res;

    rk_aiq_isp_tmo_params_v20_t* tmo_param = params->mTmoParams->data().ptr();

    if (!atmo_com) {
        LOGD_ANALYZER("no atmo result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ahdr common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk ahdr result
    if (algo_id == 0) {
        RkAiqAlgoProcResAtmoInt* atmo_rk = (RkAiqAlgoProcResAtmoInt*)atmo_com;

        setResultExpectedEffId(tmo_param->frame_id, RK_AIQ_ALGO_TYPE_ATMO);

        tmo_param->result.Res.sw_hdrtmo_lgmax =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgmax;
        tmo_param->result.Res.sw_hdrtmo_lgscl =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgscl;
        tmo_param->result.Res.sw_hdrtmo_lgscl_inv =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgscl_inv;
        tmo_param->result.Res.sw_hdrtmo_clipratio0 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_clipratio0;
        tmo_param->result.Res.sw_hdrtmo_clipratio1 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_clipratio1;
        tmo_param->result.Res.sw_hdrtmo_clipgap0 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_clipgap0;
        tmo_param->result.Res.sw_hdrtmo_clipgap1 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_clipgap1;
        tmo_param->result.Res.sw_hdrtmo_ratiol =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_ratiol;
        tmo_param->result.Res.sw_hdrtmo_hist_min =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_min;
        tmo_param->result.Res.sw_hdrtmo_hist_low =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_low;
        tmo_param->result.Res.sw_hdrtmo_hist_high =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_high;
        tmo_param->result.Res.sw_hdrtmo_hist_0p3 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_0p3;
        tmo_param->result.Res.sw_hdrtmo_hist_shift =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_shift;
        tmo_param->result.Res.sw_hdrtmo_palpha_0p18 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_palpha_0p18;
        tmo_param->result.Res.sw_hdrtmo_palpha_lw0p5 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_palpha_lw0p5;
        tmo_param->result.Res.sw_hdrtmo_palpha_lwscl =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_palpha_lwscl;
        tmo_param->result.Res.sw_hdrtmo_maxpalpha =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_maxpalpha;
        tmo_param->result.Res.sw_hdrtmo_maxgain =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_maxgain;
        tmo_param->result.Res.sw_hdrtmo_cfg_alpha =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_cfg_alpha;
        tmo_param->result.Res.sw_hdrtmo_set_gainoff =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_gainoff;
        tmo_param->result.Res.sw_hdrtmo_set_lgmin =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgmin;
        tmo_param->result.Res.sw_hdrtmo_set_lgmax =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgmax;
        tmo_param->result.Res.sw_hdrtmo_set_lgmean =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgmean;
        tmo_param->result.Res.sw_hdrtmo_set_weightkey =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_weightkey;
        tmo_param->result.Res.sw_hdrtmo_set_lgrange0 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgrange0;
        tmo_param->result.Res.sw_hdrtmo_set_lgrange1 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgrange1;
        tmo_param->result.Res.sw_hdrtmo_set_lgavgmax =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgavgmax;
        tmo_param->result.Res.sw_hdrtmo_set_palpha =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_palpha;
        tmo_param->result.Res.sw_hdrtmo_gain_ld_off1 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_gain_ld_off1;
        tmo_param->result.Res.sw_hdrtmo_gain_ld_off2 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_gain_ld_off2;
        tmo_param->result.Res.sw_hdrtmo_cnt_vsize =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_cnt_vsize;
        tmo_param->result.Res.sw_hdrtmo_big_en =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_big_en;
        tmo_param->result.Res.sw_hdrtmo_nobig_en =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_nobig_en;
        tmo_param->result.Res.sw_hdrtmo_newhist_en =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_newhist_en;
        tmo_param->result.Res.sw_hdrtmo_cnt_mode =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_cnt_mode;
        tmo_param->result.Res.sw_hdrtmo_expl_lgratio =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_expl_lgratio;
        tmo_param->result.Res.sw_hdrtmo_lgscl_ratio =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgscl_ratio;

        tmo_param->result.LongFrameMode =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgscl_ratio;
        tmo_param->result.isHdrGlobalTmo =
            atmo_rk->AtmoProcRes.isHdrGlobalTmo;

        tmo_param->result.bTmoEn =
            atmo_rk->AtmoProcRes.bTmoEn;

        tmo_param->result.isLinearTmo =
            atmo_rk->AtmoProcRes.isLinearTmo;

        tmo_param->result.TmoFlicker.GlobalTmoStrengthDown =
            atmo_rk->AtmoProcRes.TmoFlicker.GlobalTmoStrengthDown;
        tmo_param->result.TmoFlicker.GlobalTmoStrength =
            atmo_rk->AtmoProcRes.TmoFlicker.GlobalTmoStrength;
        tmo_param->result.TmoFlicker.iir =
            atmo_rk->AtmoProcRes.TmoFlicker.iir;
        tmo_param->result.TmoFlicker.iirmax =
            atmo_rk->AtmoProcRes.TmoFlicker.iirmax;
        tmo_param->result.TmoFlicker.height =
            atmo_rk->AtmoProcRes.TmoFlicker.height;
        tmo_param->result.TmoFlicker.width =
            atmo_rk->AtmoProcRes.TmoFlicker.width;

        tmo_param->result.TmoFlicker.PredictK.correction_factor =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.correction_factor;
        tmo_param->result.TmoFlicker.PredictK.correction_offset =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.correction_offset;
        tmo_param->result.TmoFlicker.PredictK.Hdr3xLongPercent =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.Hdr3xLongPercent;
        tmo_param->result.TmoFlicker.PredictK.UseLongUpTh =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.UseLongUpTh;
        tmo_param->result.TmoFlicker.PredictK.UseLongLowTh =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.UseLongLowTh;
        for(int i = 0; i < 3; i++)
            tmo_param->result.TmoFlicker.LumaDeviation[i] =
                atmo_rk->AtmoProcRes.TmoFlicker.LumaDeviation[i];
        tmo_param->result.TmoFlicker.StableThr =
            atmo_rk->AtmoProcRes.TmoFlicker.StableThr;

    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAnrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ANR);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAnr* anr_com =
        shared->procResComb.anr_proc_res;

    if (!anr_com /*|| !params->mIsppOtherParams.ptr()*/) {
        LOGD_ANALYZER("no anr result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen anr common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk anr result
    if (algo_id == 0) {
        RkAiqAlgoProcResAnrInt* anr_rk = (RkAiqAlgoProcResAnrInt*)anr_com;

#ifdef RK_SIMULATOR_HW
        rk_aiq_isp_meas_params_v20_t* isp_param =
            static_cast<rk_aiq_isp_meas_params_v20_t*>(params->mIspMeasParams->data().ptr());

        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->rkaiq_anr_proc_res.stBayernrParamSelect,
               &anr_rk->stAnrProcResult.stBayernrParamSelect,
               sizeof(RKAnr_Bayernr_Params_Select_t));
        memcpy(&isp_param->rkaiq_anr_proc_res.stUvnrParamSelect,
               &anr_rk->stAnrProcResult.stUvnrParamSelect,
               sizeof(RKAnr_Uvnr_Params_Select_t));

        memcpy(&isp_param->rkaiq_anr_proc_res.stMfnrParamSelect,
               &anr_rk->stAnrProcResult.stMfnrParamSelect,
               sizeof(RKAnr_Mfnr_Params_Select_t));

        memcpy(&isp_param->rkaiq_anr_proc_res.stYnrParamSelect,
               &anr_rk->stAnrProcResult.stYnrParamSelect,
               sizeof(RKAnr_Ynr_Params_Select_t));

        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#else
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);

        rk_aiq_isp_rawnr_params_v20_t *rawnr_params = params->mRawnrParams->data().ptr();
        rawnr_params ->update_mask |= RKAIQ_ISPP_NR_ID;
        memcpy(&rawnr_params->result,
                &anr_rk->stAnrProcResult.stBayernrFix,
                sizeof(rk_aiq_isp_rawnr_t));

        setResultExpectedEffId(rawnr_params->frame_id, RK_AIQ_ALGO_TYPE_ANR);

        rk_aiq_isp_gain_params_v20_t *gain_params = params->mGainParams->data().ptr();
        memcpy(&gain_params->result,
                &anr_rk->stAnrProcResult.stGainFix,
                sizeof(rk_aiq_isp_gain_t));

        setResultExpectedEffId(gain_params->frame_id, RK_AIQ_ALGO_TYPE_ANR);

        rk_aiq_isp_motion_params_v20_t *motion_params = params->mMotionParams->data().ptr();
        memcpy(&motion_params->result,
               &anr_rk->stAnrProcResult.stMotion,
               sizeof(anr_rk->stAnrProcResult.stMotion));

        setResultExpectedEffId(motion_params->frame_id, RK_AIQ_ALGO_TYPE_ANR);
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);


        LOGD_ANR("oyyf: %s:%d output ispp param start\n", __FUNCTION__, __LINE__);

        rk_aiq_isp_ynr_params_v20_t *ynr_params = params->mYnrParams->data().ptr();
        memcpy(&ynr_params->result,
                &anr_rk->stAnrProcResult.stYnrFix,
                sizeof(RKAnr_Ynr_Fix_t));

        setResultExpectedEffId(ynr_params->frame_id, RK_AIQ_ALGO_TYPE_ANR);

        rk_aiq_isp_uvnr_params_v20_t *uvnr_params = params->mUvnrParams->data().ptr();
        uvnr_params ->update_mask |= RKAIQ_ISPP_NR_ID;
        memcpy(&uvnr_params->result,
                &anr_rk->stAnrProcResult.stUvnrFix,
                sizeof(RKAnr_Uvnr_Fix_t));

        setResultExpectedEffId(uvnr_params->frame_id, RK_AIQ_ALGO_TYPE_ANR);

        rk_aiq_isp_tnr_params_v20_t *tnr_params = params->mTnrParams->data().ptr();
        memcpy(&tnr_params->result,
                &anr_rk->stAnrProcResult.stMfnrFix,
                sizeof(RKAnr_Mfnr_Fix_t));
        setResultExpectedEffId(tnr_params->frame_id, RK_AIQ_ALGO_TYPE_ANR);
        LOGD_ANR("oyyf: %s:%d output ispp param end \n", __FUNCTION__, __LINE__);
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAdhazResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADHAZ);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAdhaz* adhaz_com =
        shared->procResComb.adhaz_proc_res;

#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_dehaze_params_v20_t* dehaze_param = params->mDehazeParams->data().ptr();

#endif

    if (!adhaz_com) {
        LOGD_ANALYZER("no adhaz result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen adhaz common result
    RkAiqAlgoProcResAdhaz* adhaz_rk = (RkAiqAlgoProcResAdhaz*)adhaz_com;

    setResultExpectedEffId(dehaze_param->frame_id, RK_AIQ_ALGO_TYPE_ADHAZ);
    dehaze_param->result = adhaz_rk->AdehzeProcRes.ProcResV20;

    int algo_id = (*handle)->getAlgoId();

    // gen rk adhaz result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdhazInt* adhaz_rk = (RkAiqAlgoProcResAdhazInt*)adhaz_com;

        dehaze_param->result.enhance_en     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.enhance_en;
        dehaze_param->result.hist_chn   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.hist_chn;
        dehaze_param->result.hpara_en   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.hpara_en;
        dehaze_param->result.hist_en    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.hist_en;
        dehaze_param->result.dc_en  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.dc_en;
        dehaze_param->result.big_en     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.big_en;
        dehaze_param->result.nobig_en   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.nobig_en;
        dehaze_param->result.yblk_th    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.yblk_th;
        dehaze_param->result.yhist_th   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.yhist_th;
        dehaze_param->result.dc_max_th  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.dc_max_th;
        dehaze_param->result.dc_min_th  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.dc_min_th;
        dehaze_param->result.wt_max     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.wt_max;
        dehaze_param->result.bright_max     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.bright_max;
        dehaze_param->result.bright_min     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.bright_min;
        dehaze_param->result.tmax_base  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.tmax_base;
        dehaze_param->result.dark_th    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.dark_th;
        dehaze_param->result.air_max    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.air_max;
        dehaze_param->result.air_min    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.air_min;
        dehaze_param->result.tmax_max   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.tmax_max;
        dehaze_param->result.tmax_off   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.tmax_off;
        dehaze_param->result.hist_k     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.hist_k;
        dehaze_param->result.hist_th_off    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.hist_th_off;
        dehaze_param->result.hist_min   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.hist_min;
        dehaze_param->result.hist_gratio    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.hist_gratio;
        dehaze_param->result.hist_scale     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.hist_scale;
        dehaze_param->result.enhance_value  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.enhance_value;
        dehaze_param->result.iir_wt_sigma   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.iir_wt_sigma;
        dehaze_param->result.iir_sigma  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.iir_sigma;
        dehaze_param->result.stab_fnum  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.stab_fnum;
        dehaze_param->result.iir_tmax_sigma     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.iir_tmax_sigma;
        dehaze_param->result.iir_air_sigma  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.iir_air_sigma;
        dehaze_param->result.cfg_wt     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.cfg_wt;
        dehaze_param->result.cfg_air    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.cfg_air;
        dehaze_param->result.cfg_alpha  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.cfg_alpha;
        dehaze_param->result.cfg_gratio     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.cfg_gratio;
        dehaze_param->result.cfg_tmax   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.cfg_tmax;
        dehaze_param->result.dc_weitcur     = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.dc_weitcur;
        dehaze_param->result.dc_thed    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.dc_thed;
        dehaze_param->result.sw_dhaz_dc_bf_h0   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.sw_dhaz_dc_bf_h0;
        dehaze_param->result.sw_dhaz_dc_bf_h1   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.sw_dhaz_dc_bf_h1;
        dehaze_param->result.sw_dhaz_dc_bf_h2   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.sw_dhaz_dc_bf_h2;
        dehaze_param->result.sw_dhaz_dc_bf_h3   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.sw_dhaz_dc_bf_h3;
        dehaze_param->result.sw_dhaz_dc_bf_h4   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.sw_dhaz_dc_bf_h4;
        dehaze_param->result.sw_dhaz_dc_bf_h5   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.sw_dhaz_dc_bf_h5;
        dehaze_param->result.air_weitcur    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.air_weitcur;
        dehaze_param->result.air_thed   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.air_thed;
        dehaze_param->result.air_bf_h0  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.air_bf_h0;
        dehaze_param->result.air_bf_h1  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.air_bf_h1;
        dehaze_param->result.air_bf_h2  = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.air_bf_h2;
        dehaze_param->result.gaus_h0    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.gaus_h0;
        dehaze_param->result.gaus_h1    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.gaus_h1;
        dehaze_param->result.gaus_h2    = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.gaus_h2;

        for(int i = 0; i < 6; i++) {
            dehaze_param->result.conv_t0[i]   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.conv_t0[i];
            dehaze_param->result.conv_t1[i]   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.conv_t1[i];
            dehaze_param->result.conv_t2[i]   = adhaz_rk->adhaz_proc_res_com.AdehzeProcRes.ProcResV20.conv_t2[i];
        }

    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAsdResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ASD);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAsd* asd_com =
        shared->procResComb.asd_proc_res;

#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    /*
     * rk_aiq_isp_other_params_v20_t* isp_param =
     *     arams->mIspOtherParams->data().ptr();
     */

#endif

    if (!asd_com) {
        LOGD_ANALYZER("no asd result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asd common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asd result
    if (algo_id == 0) {
        RkAiqAlgoProcResAsdInt* asd_rk = (RkAiqAlgoProcResAsdInt*)asd_com;

#if 0 // flash test
        RkAiqAlgoPreResAsdInt* asd_pre_rk = (RkAiqAlgoPreResAsdInt*)shared->preResComb.asd_pre_res;
        if (asd_pre_rk->asd_result.fl_on) {
            fl_param->flash_mode = RK_AIQ_FLASH_MODE_TORCH;
            fl_param->power[0] = 1000;
            fl_param->strobe = true;
        } else {
            fl_param->flash_mode = RK_AIQ_FLASH_MODE_OFF;
            fl_param->power[0] = 0;
            fl_param->strobe = false;
        }
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAcpResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACP);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAcp* acp_com =
        shared->procResComb.acp_proc_res;

// #ifdef RK_SIMULATOR_HW
#if 0
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_cp_params_v20_t* cp_param = params->mCpParams->data().ptr();
    setResultExpectedEffId(cp_param->frame_id, RK_AIQ_ALGO_TYPE_ACP);
#endif


    if (!acp_com) {
        LOGD_ANALYZER("no acp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen acp common result
    rk_aiq_acp_params_t* isp_cp = &cp_param->result;

    *isp_cp = acp_com->acp_res;

    int algo_id = (*handle)->getAlgoId();

    // gen rk acp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAcpInt* acp_rk = (RkAiqAlgoProcResAcpInt*)acp_com;
#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAieResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AIE);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAie* aie_com =
        shared->procResComb.aie_proc_res;

#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_ie_params_v20_t* ie_param = params->mIeParams->data().ptr();
    setResultExpectedEffId(ie_param->frame_id, RK_AIQ_ALGO_TYPE_AIE);
#endif


    if (!aie_com) {
        LOGD_ANALYZER("no aie result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen aie common result
    rk_aiq_isp_ie_t* isp_ie = &ie_param->result;
    isp_ie->base = aie_com->params;
    int algo_id = (*handle)->getAlgoId();

    // gen rk aie result
    if (algo_id == 0) {
        RkAiqAlgoProcResAieInt* aie_rk = (RkAiqAlgoProcResAieInt*)aie_com;

        isp_ie->extra = aie_rk->params;
#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAsharpResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ASHARP);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
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
        RkAiqAlgoProcResAsharpInt* asharp_rk = (RkAiqAlgoProcResAsharpInt*)asharp_com;

#ifdef RK_SIMULATOR_HW
#if 0
        rk_aiq_isp_meas_params_v20_t* isp_meas_param =
            params->mIspMeasParams->data().ptr();


        LOGD_ASHARP("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_meas_param->rkaiq_asharp_proc_res.stSharpParamSelect.rk_sharpen_params_selected_V1,
               &asharp_rk->stAsharpProcResult.stSharpParamSelect.rk_sharpen_params_selected_V1,
               sizeof(RKAsharp_Sharp_HW_Params_Select_t));

        memcpy(&isp_meas_param->rkaiq_asharp_proc_res.stSharpParamSelect.rk_sharpen_params_selected_V2,
               &asharp_rk->stAsharpProcResult.stSharpParamSelect.rk_sharpen_params_selected_V2,
               sizeof(RKAsharp_Sharp_HW_V2_Params_Select_t));

        memcpy(&isp_meas_param->rkaiq_asharp_proc_res.stSharpParamSelect.rk_sharpen_params_selected_V3,
               &asharp_rk->stAsharpProcResult.stSharpParamSelect.rk_sharpen_params_selected_V3,
               sizeof(RKAsharp_Sharp_HW_V3_Params_Select_t));

        memcpy(&isp_meas_param->rkaiq_asharp_proc_res.stEdgefilterParamSelect,
               &asharp_rk->stAsharpProcResult.stEdgefilterParamSelect,
               sizeof(RKAsharp_EdgeFilter_Params_Select_t));

        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#endif
#else

        // rk_aiq_ispp_other_params_t* ispp_other_param =
            // static_cast<rk_aiq_ispp_other_params_t*>(params->mIsppOtherParams->data().ptr());

        LOGD_ASHARP("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_sharpen_params_v20_t *sharpen_params = params->mSharpenParams->data().ptr();
        sharpen_params->update_mask |= RKAIQ_ISPP_SHARP_ID;
        setResultExpectedEffId(sharpen_params->frame_id, RK_AIQ_ALGO_TYPE_ASHARP);
        memcpy(&sharpen_params->result,
               &asharp_rk->stAsharpProcResult.stSharpFix,
               sizeof(rk_aiq_isp_sharpen_t));

        rk_aiq_isp_edgeflt_params_v20_t *edgeflt_params = params->mEdgefltParams->data().ptr();
        memcpy(&edgeflt_params->result,
               &asharp_rk->stAsharpProcResult.stEdgefltFix,
               sizeof(rk_aiq_isp_edgeflt_t));
        setResultExpectedEffId(edgeflt_params->frame_id, RK_AIQ_ALGO_TYPE_ASHARP);

        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspA3dlutResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_A3DLUT);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResA3dlut* a3dlut_com =
        shared->procResComb.a3dlut_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_lut3d_params_v20_t* lut3d_param = params->mLut3dParams->data().ptr();
    setResultExpectedEffId(lut3d_param->frame_id, RK_AIQ_ALGO_TYPE_A3DLUT);

#endif


    if (!a3dlut_com) {
        LOGD_ANALYZER("no a3dlut result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen a3dlut common result
    RkAiqAlgoProcResA3dlut* a3dlut_rk = (RkAiqAlgoProcResA3dlut*)a3dlut_com;
    lut3d_param->result = a3dlut_rk->lut3d_hw_conf;

    int algo_id = (*handle)->getAlgoId();

    // gen rk a3dlut result
    if (algo_id == 0) {
        RkAiqAlgoProcResA3dlutInt* a3dlut_rk_int = (RkAiqAlgoProcResA3dlutInt*)a3dlut_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAblcResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ABLC);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAblc* ablc_com =
        shared->procResComb.ablc_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();
#else
#if 0
    rk_aiq_isp_other_params_v20_t* isp_param = params->mIspOtherParams->data().ptr();
#else
    rk_aiq_isp_blc_params_v20_t *blc_param = params->mBlcParams->data().ptr();
#endif
#endif


    if (!ablc_com) {
        LOGD_ANALYZER("no ablc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ablc common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk ablc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAblcInt* ablc_rk = (RkAiqAlgoProcResAblcInt*)ablc_com;

#if 0
        memcpy(&isp_param->blc, &ablc_rk->ablc_proc_res,
               sizeof(rk_aiq_isp_blc_t));
#else
        setResultExpectedEffId(blc_param->frame_id, RK_AIQ_ALGO_TYPE_ABLC);
        memcpy(&blc_param->result, &ablc_rk->ablc_proc_res,
               sizeof(rk_aiq_isp_blc_t));
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAccmResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACCM);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAccm* accm_com =
        shared->procResComb.accm_proc_res;
#if 0
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();
#else
    rk_aiq_isp_ccm_params_v20_t* ccm_param = params->mCcmParams->data().ptr();
#endif

    if (!accm_com) {
        LOGD_ANALYZER("no accm result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen accm common result
    RkAiqAlgoProcResAccm* accm_rk = (RkAiqAlgoProcResAccm*)accm_com;

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
RkAiqCore::genIspAcgcResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACGC);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAcgc* acgc_com =
        shared->procResComb.acgc_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_cgc_params_v20_t* cgc_param = params->mCgcParams->data().ptr();

    setResultExpectedEffId(cgc_param->frame_id, RK_AIQ_ALGO_TYPE_ACGC);
#endif

    if (!acgc_com) {
        LOGD_ANALYZER("no acgc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen acgc common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk acgc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAcgcInt* acgc_rk = (RkAiqAlgoProcResAcgcInt*)acgc_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAdebayerResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADEBAYER);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAdebayer* adebayer_com =
        shared->procResComb.adebayer_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
#if 0
    rk_aiq_isp_other_params_v20_t* isp_param = params->mIspOtherParams->data().ptr();
#else
    rk_aiq_isp_debayer_params_v20_t* debayer_param = params->mDebayerParams->data().ptr();
#endif
#endif

    if (!adebayer_com) {
        LOGD_ANALYZER("no adebayer result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen adebayer common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk adebayer result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdebayerInt* adebayer_rk = (RkAiqAlgoProcResAdebayerInt*)adebayer_com;
        setResultExpectedEffId(debayer_param->frame_id, RK_AIQ_ALGO_TYPE_ADEBAYER);
        memcpy(&debayer_param->result, &adebayer_rk->debayerRes.config, sizeof(AdebayerConfig_t));
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAdpccResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADPCC);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAdpcc* adpcc_com =
        shared->procResComb.adpcc_proc_res;
    rk_aiq_isp_dpcc_params_v20_t *dpcc_param = params->mDpccParams->data().ptr();

#if 0
    /* TODO: xuhf */
    SmartPtr<rk_aiq_exposure_params_wrapper_t> exp_param =
        params->mExposureParams->data();
#endif

    if (!adpcc_com) {
        LOGD_ANALYZER("no adpcc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen adpcc common result

    int algo_id = (*handle)->getAlgoId();

#if 0
    // gen rk adpcc result
    exp_param->SensorDpccInfo.enable = adpcc_com->SenDpccRes.enable;
    exp_param->SensorDpccInfo.cur_single_dpcc = adpcc_com->SenDpccRes.cur_single_dpcc;
    exp_param->SensorDpccInfo.cur_multiple_dpcc = adpcc_com->SenDpccRes.cur_multiple_dpcc;
    exp_param->SensorDpccInfo.total_dpcc = adpcc_com->SenDpccRes.total_dpcc;
#endif

    if (algo_id == 0) {
        RkAiqAlgoProcResAdpccInt* adpcc_rk = (RkAiqAlgoProcResAdpccInt*)adpcc_com;

        LOGD_ADPCC("oyyf: %s:%d output dpcc param start\n", __FUNCTION__, __LINE__);
#if 0
        memcpy(&isp_param->dpcc,
               &adpcc_rk->stAdpccProcResult,
               sizeof(rk_aiq_isp_dpcc_t));
#else
        setResultExpectedEffId(dpcc_param->frame_id, RK_AIQ_ALGO_TYPE_ADPCC);
        memcpy(&dpcc_param->result,
               &adpcc_rk->stAdpccProcResult,
               sizeof(rk_aiq_isp_dpcc_t));
#endif
        LOGD_ADPCC("oyyf: %s:%d output dpcc param end\n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAfecResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AFEC);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAfec* afec_com =
        shared->procResComb.afec_proc_res;


// #ifdef RK_SIMULATOR_HW
#if 0
    if (!afec_com || !params->mIsppMeasParams.ptr()) {
        LOGD_ANALYZER("no afec result");
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_ispp_meas_params_t* ispp_param = params->mIsppMeasParams->data().ptr();
#else
    if (!afec_com) {
        LOGD_ANALYZER("no afec result");
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_isp_fec_params_v20_t *fec_params = params->mFecParams->data().ptr();
#endif

    if (fec_params->result.usage == ISPP_MODULE_FEC_ST) {
        LOGD_ANALYZER("afec not update because EIS enabled");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen afec common result
    int algo_id = (*handle)->getAlgoId();
    // gen rk afec result
    if (algo_id == 0) {
        RkAiqAlgoProcResAfecInt* afec_rk = (RkAiqAlgoProcResAfecInt*)afec_com;

        setResultExpectedEffId(fec_params->frame_id, RK_AIQ_ALGO_TYPE_AFEC);
        if (afec_rk->afec_result.update) {
            fec_params->update_mask |= RKAIQ_ISPP_FEC_ID;
            fec_params->result.fec_en = afec_rk->afec_result.sw_fec_en;
            if (fec_params->result.fec_en) {
                fec_params->result.crop_en = afec_rk->afec_result.crop_en;
                fec_params->result.crop_width = afec_rk->afec_result.crop_width;
                fec_params->result.crop_height = afec_rk->afec_result.crop_height;
                fec_params->result.mesh_density = afec_rk->afec_result.mesh_density;
                fec_params->result.mesh_size = afec_rk->afec_result.mesh_size;
                fec_params->result.mesh_buf_fd = afec_rk->afec_result.mesh_buf_fd;
                //memcpy(fec_params->result.sw_mesh_xi, afec_rk->afec_result.meshxi, sizeof(fec_params->result.sw_mesh_xi));
                //memcpy(fec_params->result.sw_mesh_xf, afec_rk->afec_result.meshxf, sizeof(fec_params->result.sw_mesh_xf));
                //memcpy(fec_params->result.sw_mesh_yi, afec_rk->afec_result.meshyi, sizeof(fec_params->result.sw_mesh_yi));
                //memcpy(fec_params->result.sw_mesh_yf, afec_rk->afec_result.meshyf, sizeof(fec_params->result.sw_mesh_yf));
            }
        } else {
            fec_params->update_mask &= ~RKAIQ_ISPP_FEC_ID;
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAgammaResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AGAMMA);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAgamma* agamma_com =
        shared->procResComb.agamma_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_agamma_params_v20_t* agamma_param = params->mAgammaParams->data().ptr();
#endif


    if (!agamma_com) {
        LOGD_ANALYZER("no agamma result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen agamma common result
    RkAiqAlgoProcResAgamma* agamma_rk = (RkAiqAlgoProcResAgamma*)agamma_com;

    setResultExpectedEffId(agamma_param->frame_id, RK_AIQ_ALGO_TYPE_AGAMMA);
    agamma_param->result = agamma_rk->agamma_proc_res;

    int algo_id = (*handle)->getAlgoId();

    // gen rk agamma result
    if (algo_id == 0) {
        RkAiqAlgoProcResAgammaInt* agamma_rk = (RkAiqAlgoProcResAgammaInt*)agamma_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAdegammaResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADEGAMMA);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAdegamma* adegamma_com =
        shared->procResComb.adegamma_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_adegamma_params_v20_t* degamma_param = params->mAdegammaParams->data().ptr();

#endif


    if (!adegamma_com) {
        LOGD_ANALYZER("no adegamma result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen agamma common result
    RkAiqAlgoProcResAdegamma* adegamma_rk = (RkAiqAlgoProcResAdegamma*)adegamma_com;

    setResultExpectedEffId(degamma_param->frame_id, RK_AIQ_ALGO_TYPE_ADEGAMMA);
    degamma_param->result = adegamma_rk->adegamma_proc_res;

    int algo_id = (*handle)->getAlgoId();

    // gen rk adegamma result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdegammaInt* adegamma_rk = (RkAiqAlgoProcResAdegammaInt*)adegamma_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}


XCamReturn
RkAiqCore::genIspAgicResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AGIC);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAgic* agic_com =
        shared->procResComb.agic_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
#if 0
    rk_aiq_isp_other_params_v20_t* isp_param = params->mIspOtherParams->data().ptr();
#else
    rk_aiq_isp_gic_params_v20_t* gic_param = params->mGicParams->data().ptr();
#endif
#endif

    if (!agic_com) {
        LOGD_ANALYZER("no agic result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen agic common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk agic result
    if (algo_id == 0) {
        RkAiqAlgoProcResAgicInt* agic_rk = (RkAiqAlgoProcResAgicInt*)agic_com;
        setResultExpectedEffId(gic_param->frame_id, RK_AIQ_ALGO_TYPE_AGIC);
        memcpy(&gic_param->result, &agic_rk->gicRes, sizeof(AgicProcResult_t));
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAldchResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ALDCH);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAldch* aldch_com =
        shared->procResComb.aldch_proc_res;
#if 0
// #ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_ldch_params_v20_t* ldch_param = params->mLdchParams->data().ptr();
#endif


    if (!aldch_com) {
        LOGD_ANALYZER("no aldch result");
        return XCAM_RETURN_NO_ERROR;
    }

    RkAiqAlgoProcResAldchInt* aldch_rk = (RkAiqAlgoProcResAldchInt*)aldch_com;
    setResultExpectedEffId(ldch_param->frame_id, RK_AIQ_ALGO_TYPE_ALDCH);
    if (aldch_rk->ldch_result.update) {
        ldch_param->update_mask |= RKAIQ_ISP_LDCH_ID;
        ldch_param->result.ldch_en = aldch_rk->ldch_result.sw_ldch_en;
        if (ldch_param->result.ldch_en) {
            ldch_param->result.lut_h_size = aldch_rk->ldch_result.lut_h_size;
            ldch_param->result.lut_v_size = aldch_rk->ldch_result.lut_v_size;
            ldch_param->result.lut_size = aldch_rk->ldch_result.lut_map_size;
            ldch_param->result.lut_mem_fd = aldch_rk->ldch_result.lut_mapxy_buf_fd;
        }
    } else {
        ldch_param->update_mask &= ~RKAIQ_ISP_LDCH_ID;
    }

    int algo_id = (*handle)->getAlgoId();

    // gen rk aldch result
    if (algo_id == 0) {
        RkAiqAlgoProcResAldchInt* aldch_rk = (RkAiqAlgoProcResAldchInt*)aldch_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAlscResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ALSC);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAlsc* alsc_com =
        shared->procResComb.alsc_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
#if 0
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();
#else
    rk_aiq_isp_lsc_params_v20_t* lsc_param = params->mLscParams->data().ptr();
#endif
#endif

    if (!alsc_com) {
        LOGD_ANALYZER("no alsc result");
        return XCAM_RETURN_NO_ERROR;
    }
    // TODO: gen alsc common result
    RkAiqAlgoProcResAlsc* alsc_rk = (RkAiqAlgoProcResAlsc*)alsc_com;
    setResultExpectedEffId(lsc_param->frame_id, RK_AIQ_ALGO_TYPE_ALSC);
    lsc_param->result = alsc_rk->alsc_hw_conf;
    if(mAlogsComSharedParams.sns_mirror) {
        for(int i = 0; i < LSC_DATA_TBL_V_SIZE; i++) {
            for(int j = 0; j < LSC_DATA_TBL_H_SIZE; j++) {
                SWAP(unsigned short, lsc_param->result.r_data_tbl[i * LSC_DATA_TBL_H_SIZE + j], lsc_param->result.r_data_tbl[i * LSC_DATA_TBL_H_SIZE + (LSC_DATA_TBL_H_SIZE - 1 - j)]);
                SWAP(unsigned short, lsc_param->result.gr_data_tbl[i * LSC_DATA_TBL_H_SIZE + j], lsc_param->result.gr_data_tbl[i * LSC_DATA_TBL_H_SIZE + (LSC_DATA_TBL_H_SIZE - 1 - j)]);
                SWAP(unsigned short, lsc_param->result.gb_data_tbl[i * LSC_DATA_TBL_H_SIZE + j], lsc_param->result.gb_data_tbl[i * LSC_DATA_TBL_H_SIZE + (LSC_DATA_TBL_H_SIZE - 1 - j)]);
                SWAP(unsigned short, lsc_param->result.b_data_tbl[i * LSC_DATA_TBL_H_SIZE + j], lsc_param->result.b_data_tbl[i * LSC_DATA_TBL_H_SIZE + (LSC_DATA_TBL_H_SIZE - 1 - j)]);
            }
        }
    }
    if(mAlogsComSharedParams.sns_flip) {
        for(int i = 0; i < LSC_DATA_TBL_V_SIZE; i++) {
            for(int j = 0; j < LSC_DATA_TBL_H_SIZE; j++) {
                SWAP(unsigned short, lsc_param->result.r_data_tbl[i * LSC_DATA_TBL_H_SIZE + j], lsc_param->result.r_data_tbl[(LSC_DATA_TBL_V_SIZE - 1 - i)*LSC_DATA_TBL_H_SIZE + j]);
                SWAP(unsigned short, lsc_param->result.gr_data_tbl[i * LSC_DATA_TBL_H_SIZE + j], lsc_param->result.gr_data_tbl[(LSC_DATA_TBL_V_SIZE - 1 - i)*LSC_DATA_TBL_H_SIZE + j]);
                SWAP(unsigned short, lsc_param->result.gb_data_tbl[i * LSC_DATA_TBL_H_SIZE + j], lsc_param->result.gb_data_tbl[(LSC_DATA_TBL_V_SIZE - 1 - i)*LSC_DATA_TBL_H_SIZE + j]);
                SWAP(unsigned short, lsc_param->result.b_data_tbl[i * LSC_DATA_TBL_H_SIZE + j], lsc_param->result.b_data_tbl[(LSC_DATA_TBL_V_SIZE - 1 - i)*LSC_DATA_TBL_H_SIZE + j]);
            }
        }
    }
    int algo_id = (*handle)->getAlgoId();

    // gen rk alsc result
    if (algo_id == 0) {
        RkAiqAlgoProcResAlscInt* alsc_rk_int = (RkAiqAlgoProcResAlscInt*)alsc_com;
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAorbResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AORB);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAorb* aorb_com =
        shared->procResComb.aorb_proc_res;

    if (!aorb_com) {
        LOGE_ANALYZER("no aorb result");
        return XCAM_RETURN_NO_ERROR;
    }

    int algo_id = (*handle)->getAlgoId();

    // gen rk aorb result
    if (algo_id == 0) {
        RkAiqAlgoProcResAorbInt* aorb_rk = (RkAiqAlgoProcResAorbInt*)aorb_com;
        rk_aiq_isp_orb_params_v20_t *orb_params = params->mOrbParams->data().ptr();
        if (orb_params != nullptr) {
            setResultExpectedEffId(orb_params->frame_id, RK_AIQ_ALGO_TYPE_AORB);
            if (aorb_rk->aorb_meas.update) {
                orb_params->update_mask |= RKAIQ_ISPP_ORB_ID;
                orb_params->result.orb_en = aorb_rk->aorb_meas.orb_en;
                if (orb_params->result.orb_en) {
                    orb_params->result.limit_value = aorb_rk->aorb_meas.limit_value;
                    orb_params->result.max_feature = aorb_rk->aorb_meas.max_feature;
                }
            } else {
                orb_params->update_mask &= ~RKAIQ_ISPP_ORB_ID;
            }
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAeisResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AEIS);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAeis* aeis_com =
        shared->procResComb.aeis_proc_res;

#ifdef RK_SIMULATOR_HW
    if (!aeis_com) {
        LOGD_ANALYZER("no aeis result");
        return XCAM_RETURN_NO_ERROR;
    }

#else
    if (!aeis_com) {
        LOGD_ANALYZER("no aeis result");
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_isp_fec_params_v20_t *fec_params = params->mFecParams->data().ptr();
#endif
    int algo_id = (*handle)->getAlgoId();
    if (algo_id == 0) {
        RkAiqAlgoProcResAeisInt* aeis_rk = (RkAiqAlgoProcResAeisInt*)aeis_com;
        if (aeis_rk->update) {
            fec_params->update_mask |= ISPP_MODULE_FEC_ST;
            fec_params->result.fec_en = aeis_rk->fec_en;
            fec_params->result.usage = ISPP_MODULE_FEC_ST;
            if (fec_params->result.fec_en) {
#if 1
                if (1) {
#else
                if (aeis_rk->fd >= 0) {
#endif
                    fec_params->result.config = true;
                    fec_params->result.frame_id = aeis_rk->frame_id;
                    fec_params->frame_id = aeis_rk->frame_id;
                    fec_params->result.mesh_density = aeis_rk->mesh_density;
                    fec_params->result.mesh_size = aeis_rk->mesh_size;
                    fec_params->result.mesh_buf_fd = aeis_rk->fd;
                    fec_params->result.img_buf_index = aeis_rk->img_buf_index;
                    fec_params->result.img_buf_size = aeis_rk->img_size;
                } else {
                    fec_params->result.config = false;
                }
            } else {
                fec_params->update_mask &= ~ISPP_MODULE_FEC_ST;
            }
        }
        LOGD_AEIS("eis update %d, id %d, fec: en %d, config %d, fd %d, idx %d", aeis_rk->update,
                  fec_params->result.frame_id, fec_params->result.fec_en, fec_params->result.config,
                  fec_params->result.mesh_buf_fd, fec_params->result.img_buf_index);
    }


    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAr2yResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AR2Y);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAr2y* ar2y_com =
        shared->procResComb.ar2y_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    /*
     * rk_aiq_isp_other_params_v20_t* isp_param =
     *     params->mIspOtherParams->data().ptr();
     */

#endif


    if (!ar2y_com) {
        LOGD_ANALYZER("no ar2y result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ar2y common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk ar2y result
    if (algo_id == 0) {
        RkAiqAlgoProcResAr2yInt* ar2y_rk = (RkAiqAlgoProcResAr2yInt*)ar2y_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

XCamReturn
RkAiqCore::genIspAwdrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AWDR);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAwdr* awdr_com =
        shared->procResComb.awdr_proc_res;
#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_wdr_params_v20_t* wdr_param = params->mWdrParams->data().ptr();

    setResultExpectedEffId(wdr_param->frame_id, RK_AIQ_ALGO_TYPE_AWDR);
#endif


    if (!awdr_com) {
        LOGD_ANALYZER("no awdr result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen awdr common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk awdr result
    if (algo_id == 0) {
        RkAiqAlgoProcResAwdrInt* awdr_rk = (RkAiqAlgoProcResAwdrInt*)awdr_com;

#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}


#if 0
XCamReturn
RkAiqCore::genIspAdrcResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosGroupShared_t* shared = nullptr;
    int groupId = getGroupId(RK_AIQ_ALGO_TYPE_ADRC);
    if (groupId >= 0) {
        if (getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
            return XCAM_RETURN_BYPASS;
    RkAiqAlgoProcResAdrc* adrc_com =
        shared->procResComb.adrc_proc_res;
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

    if (!adrc_com) {
        LOGD_ANALYZER("no adrc result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen ahdr common result

    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ADRC);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    int algo_id = (*handle)->getAlgoId();

    // gen rk ahdr result
    if (algo_id == 0) {
        RkAiqAlgoProcResAdrcInt* ahdr_rk = (RkAiqAlgoProcResAdrcInt*)adrc_com;


        isp_param->drc.DrcProcRes.sw_drc_offset_pow2     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_offset_pow2;
        isp_param->drc.DrcProcRes.sw_drc_compres_scl  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_compres_scl;
        isp_param->drc.DrcProcRes.sw_drc_position  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_position;
        isp_param->drc.DrcProcRes.sw_drc_delta_scalein        = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_delta_scalein;
        isp_param->drc.DrcProcRes.sw_drc_hpdetail_ratio      = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_hpdetail_ratio;
        isp_param->drc.DrcProcRes.sw_drc_lpdetail_ratio     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_lpdetail_ratio;
        isp_param->drc.DrcProcRes.sw_drc_weicur_pix      = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_weicur_pix;
        isp_param->drc.DrcProcRes.sw_drc_weipre_frame  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_weipre_frame;
        isp_param->drc.DrcProcRes.sw_drc_force_sgm_inv0   = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_force_sgm_inv0;
        isp_param->drc.DrcProcRes.sw_drc_motion_scl     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_motion_scl;
        isp_param->drc.DrcProcRes.sw_drc_edge_scl   = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_edge_scl;
        isp_param->drc.DrcProcRes.sw_drc_space_sgm_inv1    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_space_sgm_inv1;
        isp_param->drc.DrcProcRes.sw_drc_space_sgm_inv0     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_range_sgm_inv1;
        isp_param->drc.DrcProcRes.sw_drc_range_sgm_inv1     = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_range_sgm_inv0;
        isp_param->drc.DrcProcRes.sw_drc_range_sgm_inv0 = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_space_sgm_inv0;
        isp_param->drc.DrcProcRes.sw_drc_weig_maxl    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_weig_maxl;
        isp_param->drc.DrcProcRes.sw_drc_weig_bilat  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_weig_bilat;
        isp_param->drc.DrcProcRes.sw_drc_iir_weight  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_iir_weight;
        isp_param->drc.DrcProcRes.sw_drc_min_ogain  = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_min_ogain;
        for(int i = 0; i < 17; i++) {
            isp_param->drc.DrcProcRes.sw_drc_gain_y[i]    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_gain_y[i];
            isp_param->drc.DrcProcRes.sw_drc_compres_y[i]    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_compres_y[i];
            isp_param->drc.DrcProcRes.sw_drc_scale_y[i]    = ahdr_rk->AdrcProcRes.DrcProcRes.sw_drc_scale_y[i];
        }


        isp_param->drc.isHdrGlobalTmo =
            ahdr_rk->AdrcProcRes.isHdrGlobalTmo;

        isp_param->drc.bTmoEn =
            ahdr_rk->AdrcProcRes.bTmoEn;

        isp_param->drc.isLinearTmo =
            ahdr_rk->AdrcProcRes.isLinearTmo;

        LOGE("sw_drc_offset_pow2 %d", isp_param->drc.DrcProcRes.sw_drc_offset_pow2);
        LOGE("sw_drc_compres_scl %d", isp_param->drc.DrcProcRes.sw_drc_compres_scl);
        LOGE("sw_drc_position %d", isp_param->drc.DrcProcRes.sw_drc_position);
        LOGE("sw_drc_delta_scalein %d", isp_param->drc.DrcProcRes.sw_drc_delta_scalein);
        LOGE("sw_drc_hpdetail_ratio %d", isp_param->drc.DrcProcRes.sw_drc_hpdetail_ratio);
        LOGE("sw_drc_lpdetail_ratio %d", isp_param->drc.DrcProcRes.sw_drc_lpdetail_ratio);
        LOGE("sw_drc_weicur_pix %d", isp_param->drc.DrcProcRes.sw_drc_weicur_pix);


    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}
#endif

XCamReturn
RkAiqCore::genIspAmdResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AMD);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAmd* amd_com =
        shared->procResComb.amd_proc_res;

#ifdef RK_SIMULATOR_HW
    rk_aiq_isp_meas_params_v20_t* isp_param = params->mIspMeasParams->data().ptr();

#else
    rk_aiq_isp_md_params_v20_t* md_param = params->mMdParams->data().ptr();
    setResultExpectedEffId(md_param->frame_id, RK_AIQ_ALGO_TYPE_AMD);
#endif


    if (!amd_com) {
        LOGD_ANALYZER("no amd result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen amd common result
    md_param->result = amd_com->amd_proc_res;

    int algo_id = (*handle)->getAlgoId();

    // gen rk amd result
    if (algo_id == 0) {
        RkAiqAlgoProcResAmdInt* amd_rk = (RkAiqAlgoProcResAmdInt*)amd_com;
#ifdef RK_SIMULATOR_HW
#else
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}


XCamReturn
RkAiqCore::genIspArawnrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ARAWNR);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResArawnr* arawnr_com =
        shared->procResComb.arawnr_proc_res;

    if (!arawnr_com) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResArawnrInt* arawnr_rk = (RkAiqAlgoProcResArawnrInt*)arawnr_com;

#ifdef RK_SIMULATOR_HW

        rk_aiq_isp_meas_params_v20_t* isp_meas_param =
            static_cast<rk_aiq_isp_meas_params_v20_t*>(params->mIspMeasParams->data().ptr());


        LOGD_ASHARP("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->rkaiq_anr_proc_res.stBayernrParamSelect,
               &arawnr_rk->stArawnrProcResult.stSelect,
               sizeof(RK_Bayernr_Params_Select_V1_t));

        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#else

        if (params->mRawnrParams.ptr()) {
            rk_aiq_isp_rawnr_params_v20_t* rawnr_param = params->mRawnrParams->data().ptr();
            LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
            setResultExpectedEffId(rawnr_param->frame_id, RK_AIQ_ALGO_TYPE_ARAWNR);
            memcpy(&rawnr_param->result,
                   &arawnr_rk->stArawnrProcResult.stFix,
                   sizeof(RK_Bayernr_Fix_V1_t));
        }
        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}




XCamReturn
RkAiqCore::genIspAynrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AYNR);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAynr* aynr_com =
        shared->procResComb.aynr_proc_res;

    if (!aynr_com) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAynrInt* aynr_rk = (RkAiqAlgoProcResAynrInt*)aynr_com;

#ifdef RK_SIMULATOR_HW

        rk_aiq_isp_meas_params_v20_t* isp_meas_param =
            static_cast<rk_aiq_isp_meas_params_v20_t*>(params->mIspMeasParams->data().ptr());

        LOGD_ASHARP("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->rkaiq_anr_proc_res.stYnrParamSelect,
               &aynr_rk->stAynrProcResult.stSelect,
               sizeof(RK_YNR_Params_V1_Select_t));
        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#else

        if (params->mYnrParams.ptr()) {
            rk_aiq_isp_ynr_params_v20_t* ynr_param = params->mYnrParams->data().ptr();
            LOGD_ANR("oyyf: %s:%d output ispp param start\n", __FUNCTION__, __LINE__);

            setResultExpectedEffId(ynr_param->frame_id, RK_AIQ_ALGO_TYPE_AYNR);
            memcpy(&ynr_param->result,
                   &aynr_rk->stAynrProcResult.stFix,
                   sizeof(RK_YNR_Fix_V1_t));
        }
        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}



XCamReturn
RkAiqCore::genIspAcnrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ACNR);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAcnr* acnr_com =
        shared->procResComb.acnr_proc_res;

    if (!acnr_com) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAcnrInt* acnr_rk = (RkAiqAlgoProcResAcnrInt*)acnr_com;

#ifdef RK_SIMULATOR_HW

        rk_aiq_isp_meas_params_v20_t* isp_meas_param =
            static_cast<rk_aiq_isp_meas_params_v20_t*>(params->mIspMeasParams->data().ptr());

        LOGD_ASHARP("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->rkaiq_anr_proc_res.stUvnrParamSelect,
               &acnr_rk->stAuvnrProcResult.stSelect,
               sizeof(RK_UVNR_Params_V1_Select_t));
        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#else

        if (params->mUvnrParams.ptr()) {
            rk_aiq_isp_uvnr_params_v20_t* cnr_param = params->mUvnrParams->data().ptr();
            LOGD_ANR("oyyf: %s:%d output ispp param start\n", __FUNCTION__, __LINE__);

            setResultExpectedEffId(cnr_param->frame_id, RK_AIQ_ALGO_TYPE_ACNR);
            cnr_param->update_mask |= RKAIQ_ISPP_NR_ID;
            memcpy(&cnr_param->result,
                   &acnr_rk->stAuvnrProcResult.stFix,
                   sizeof(RK_UVNR_Fix_V1_t));
        }
        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}




XCamReturn
RkAiqCore::genIspAmfnrResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AMFNR);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAmfnr* amfnr_com =
        shared->procResComb.amfnr_proc_res;

    if (!amfnr_com) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAmfnrInt* amfnr_rk = (RkAiqAlgoProcResAmfnrInt*)amfnr_com;

#ifdef RK_SIMULATOR_HW

        rk_aiq_isp_meas_params_v20_t* isp_meas_param =
            static_cast<rk_aiq_isp_meas_params_v20_t*>(params->mIspMeasParams->data().ptr());

        LOGD_ASHARP("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        memcpy(&isp_param->rkaiq_anr_proc_res.stMfnrParamSelect,
               &amfnr_rk->stAmfnrProcResult.stSelect,
               sizeof(RK_MFNR_Params_V1_Select_t));
        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#else

        if (params->mTnrParams.ptr()) {
            rk_aiq_isp_tnr_params_v20_t* tnr_param = params->mTnrParams->data().ptr();
            LOGD_ANR("oyyf: %s:%d output ispp param start\n", __FUNCTION__, __LINE__);

            setResultExpectedEffId(tnr_param->frame_id, RK_AIQ_ALGO_TYPE_AMFNR);
            tnr_param->update_mask |= RKAIQ_ISPP_TNR_ID;
            memcpy(&tnr_param->result,
                   &amfnr_rk->stAmfnrProcResult.stFix,
                   sizeof(RK_MFNR_Fix_V1_t));
        }
        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}



XCamReturn
RkAiqCore::genIspAgainResult(RkAiqFullParams* params)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqHandle>* handle = getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AGAIN);
    if (handle == nullptr) {
        return XCAM_RETURN_BYPASS;
    }
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)((*handle)->getGroupShared());
    RkAiqAlgoProcResAgain* again_com =
        shared->procResComb.again_proc_res;

    if (!again_com) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    // TODO: gen asharp common result

    int algo_id = (*handle)->getAlgoId();

    // gen rk asharp result
    if (algo_id == 0) {
        RkAiqAlgoProcResAgainInt* again_rk = (RkAiqAlgoProcResAgainInt*)again_com;

#ifdef RK_SIMULATOR_HW

#else

        if (params->mGainParams.ptr()) {
            rk_aiq_isp_gain_params_v20_t* gain_param = params->mGainParams->data().ptr();
            setResultExpectedEffId(gain_param ->frame_id, RK_AIQ_ALGO_TYPE_AGAIN);
            LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
            memcpy(&gain_param->result,
                   &again_rk->stAgainProcResult.stFix,
                   sizeof(RK_GAIN_Fix_V1_t));
        }

        LOGD_ASHARP("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
#endif
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}


XCamReturn
RkAiqCore::pushStats(SmartPtr<VideoBuffer> &buffer)
{
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(buffer.ptr());
    mRkAiqCoreTh->push_stats(buffer);
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::pushEvts(SmartPtr<ispHwEvt_t> &evts)
{
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(evts.ptr());

    if (evts->evt_code == V4L2_EVENT_FRAME_SYNC)
        mRkAiqCoreEvtsTh->push_evts(evts);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

const RkAiqHandle*
RkAiqCore::getAiqAlgoHandle(const int algo_type)
{
    SmartPtr<RkAiqHandle>* handlePtr = getCurAlgoTypeHandle(algo_type);
    if (handlePtr == nullptr) {
        return NULL;
    }

    return (*handlePtr).ptr();
}

SmartPtr<RkAiqHandle>*
RkAiqCore::getCurAlgoTypeHandle(int algo_type)
{
    if (mCurAlgoHandleMaps.count(algo_type) > 0)
        return &mCurAlgoHandleMaps.at(algo_type);

    LOGE("can't find algo handle %d", algo_type);
    return NULL;
}

std::map<int, SmartPtr<RkAiqHandle>>*
RkAiqCore::getAlgoTypeHandleMap(int algo_type)
{
    if (mAlgoHandleMaps.count(algo_type) > 0)
        return &mAlgoHandleMaps.at(algo_type);

    LOGE("can't find algo map %d", algo_type);
    return NULL;
}

void
RkAiqCore::addDefaultAlgos(const struct RkAiqAlgoDesCommExt* algoDes)
{
    map<int, SmartPtr<RkAiqHandle>> algoMap;
    for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        mAlogsComSharedParams.ctxCfigs[i].calib =
            const_cast<CamCalibDbContext_t*>(mAlogsComSharedParams.calib);
        mAlogsComSharedParams.ctxCfigs[i].calibv2 =
            const_cast<CamCalibDbV2Context_t*>(mAlogsComSharedParams.calibv2);
        mAlogsComSharedParams.ctxCfigs[i].cfg_com.isp_hw_version = mIspHwVer;
    }

    for (size_t i = 0; algoDes[i].des != NULL; i++) {
        int algo_type = algoDes[i].des->type;
        // enable only the specified algorithm modules
        if (!(1 << algo_type & mCustomEnAlgosMask))
            continue;
        int32_t grpMask = 1ULL << algoDes[i].group;
        mAlogsComSharedParams.ctxCfigs[algo_type].calib =
            const_cast<CamCalibDbContext_t*>(mAlogsComSharedParams.calib);
        mAlogsComSharedParams.ctxCfigs[algo_type].calibv2 =
            const_cast<CamCalibDbV2Context_t*>(mAlogsComSharedParams.calibv2);
        mAlogsComSharedParams.ctxCfigs[algo_type].cfg_com.module_hw_version = algoDes[i].module_hw_ver;
		mAlgoTypeToGrpMaskMap[algo_type] = grpMask;
		bool isExist = false;
		for(auto it = mAlogsGroupList.begin();it != mAlogsGroupList.end();it++){
			if (*it == algoDes[i].group)
				isExist = true;
		}
        if (!isExist) {
			mAlogsGroupList.push_back(algoDes[i].group);
            auto groupId = algoDes[i].group;
            mAlogsGroupSharedParamsMap[grpMask] = new RkAiqAlgosGroupShared_t;
            mAlogsGroupSharedParamsMap[grpMask]->reset();
            mAlogsGroupSharedParamsMap[grpMask]->groupId = algoDes[i].group;
            mAlogsGroupSharedParamsMap[grpMask]->frameId = 0;
            mAlogsGroupSharedParamsMap[grpMask]->ispStats = NULL;

        }
		algoMap[0] = newAlgoHandle(algoDes[i].des, false, algoDes[i].algo_ver);
        if (!algoMap[0].ptr()) {
            LOGE_ANALYZER("new algo_type %d handle failed", algo_type);
            continue;
        }
        algoMap[0]->setGroupId(grpMask);
        algoMap[0]->setGroupShared(mAlogsGroupSharedParamsMap[grpMask]);
        mAlgoHandleMaps[algo_type] = algoMap;
        mCurIspAlgoHandleList.push_back(algoMap[0]);
        mCurAlgoHandleMaps[algo_type] = algoMap[0];
        enableAlgo(algo_type, 0, true);
        algoMap.clear();
    }
}

SmartPtr<RkAiqHandle>
RkAiqCore::newAlgoHandle(RkAiqAlgoDesComm* algo, bool generic, int version)
{
#define NEW_ALGO_HANDLE(lc, BC) \
    if (algo->type == RK_AIQ_ALGO_TYPE_##BC) { \
        if (generic) { \
            if (version == 0) \
                return new RkAiq##lc##Handle(algo, this); \
        } else { \
            if (version == 0) \
                return new RkAiq##lc##HandleInt(algo, this); \
        }\
    }\

    NEW_ALGO_HANDLE(Ae, AE);
    NEW_ALGO_HANDLE(Awb, AWB);
    NEW_ALGO_HANDLE(Af, AF);
    NEW_ALGO_HANDLE(Amerge, AMERGE);
    NEW_ALGO_HANDLE(Atmo, ATMO);
    NEW_ALGO_HANDLE(Anr, ANR);
    NEW_ALGO_HANDLE(Adhaz, ADHAZ);
    NEW_ALGO_HANDLE(Asd, ASD);
    NEW_ALGO_HANDLE(Acp, ACP);
    NEW_ALGO_HANDLE(Asharp, ASHARP);
    NEW_ALGO_HANDLE(A3dlut, A3DLUT);
    NEW_ALGO_HANDLE(Ablc, ABLC);
    NEW_ALGO_HANDLE(Accm, ACCM);
    NEW_ALGO_HANDLE(Acgc, ACGC);
    NEW_ALGO_HANDLE(Adebayer, ADEBAYER);
    NEW_ALGO_HANDLE(Adpcc, ADPCC);
    NEW_ALGO_HANDLE(Afec, AFEC);
    NEW_ALGO_HANDLE(Agamma, AGAMMA);
    NEW_ALGO_HANDLE(Agic, AGIC);
    NEW_ALGO_HANDLE(Aie, AIE);
    NEW_ALGO_HANDLE(Aldch, ALDCH);
    NEW_ALGO_HANDLE(Alsc, ALSC);
    NEW_ALGO_HANDLE(Aorb, AORB);
    NEW_ALGO_HANDLE(Ar2y, AR2Y);
    NEW_ALGO_HANDLE(Awdr, AWDR);
    NEW_ALGO_HANDLE(Arawnr, ARAWNR);
    NEW_ALGO_HANDLE(Amfnr, AMFNR);
    NEW_ALGO_HANDLE(Aynr, AYNR);
    NEW_ALGO_HANDLE(Acnr, ACNR);
    NEW_ALGO_HANDLE(Adrc, ADRC);
    NEW_ALGO_HANDLE(Adegamma, ADEGAMMA);
    NEW_ALGO_HANDLE(Aeis, AEIS);
    NEW_ALGO_HANDLE(Amd, AMD);
    NEW_ALGO_HANDLE(Again, AGAIN);

    return NULL;
}

XCamReturn
RkAiqCore::addAlgo(RkAiqAlgoDesComm& algo)
{
    ENTER_ANALYZER_FUNCTION();

    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algo.type);

    if (!algo_map) {
        LOGE_ANALYZER("do not support this algo type %d !", algo.type);
        return XCAM_RETURN_ERROR_FAILED;
    }
    // TODO, check if exist befor insert ?
    std::map<int, SmartPtr<RkAiqHandle>>::reverse_iterator rit = algo_map->rbegin();

    algo.id = rit->first + 1;

    int i = 0;
    const struct RkAiqAlgoDesCommExt* def_des = NULL;
    while (mAlgosDesArray[i].des != NULL) {
        if (mAlgosDesArray[i].des->type == algo.type) {
            def_des = &mAlgosDesArray[i];
            break;
        }
        i++;
    }

    // add to map
    SmartPtr<RkAiqHandle> new_hdl = newAlgoHandle(&algo, true, def_des->algo_ver);
    new_hdl->setEnable(false);
    (*algo_map)[algo.id] = new_hdl;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::enableAlgo(int algoType, int id, bool enable)
{
    ENTER_ANALYZER_FUNCTION();
    // get current algotype handle, get id
    SmartPtr<RkAiqHandle>* cur_algo_hdl = getCurAlgoTypeHandle(algoType);
    if (!cur_algo_hdl) {
        LOGE_ANALYZER("can't find current type %d algo", algoType);
        return XCAM_RETURN_ERROR_FAILED;
    }
    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    NULL_RETURN_RET(algo_map, XCAM_RETURN_ERROR_FAILED);
    std::map<int, SmartPtr<RkAiqHandle>>::iterator it = algo_map->find(id);
    bool switch_algo = false;

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    it->second->setEnable(enable);
    /* WARNING:
     * Be careful when use SmartPtr<RkAiqxxxHandle> = SmartPtr<RkAiqHandle>
     * if RkAiqxxxHandle is derived from multiple RkAiqHandle,
     * the ptr of RkAiqxxxHandle and RkAiqHandle IS NOT the same
     * (RkAiqHandle ptr = RkAiqxxxHandle ptr + offset), but seams like
     * SmartPtr do not deal with this correctly.
     */
    if (enable) {
        SmartLock locker (mApiMutex);
        while (mSafeEnableAlgo != true)
            mApiMutexCond.wait(mApiMutex);
        if ((*cur_algo_hdl).ptr() &&
                (*cur_algo_hdl).ptr() != it->second.ptr()) {
            (*cur_algo_hdl)->setEnable(false);
            switch_algo = true;
        }
        *cur_algo_hdl = it->second;
        for (auto& algo : mCurIspAlgoHandleList) {
            if (algo->getAlgoType() == algoType)
                algo = it->second;
        }
        for (auto& algo : mCurIsppAlgoHandleList) {
            if (algo->getAlgoType() == algoType)
                algo = it->second;
        }
        if (switch_algo && (mState >= RK_AIQ_CORE_STATE_PREPARED))
            (*cur_algo_hdl)->prepare();
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::rmAlgo(int algoType, int id)
{
    ENTER_ANALYZER_FUNCTION();

    // can't remove default algos
    if (id == 0)
        return XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqHandle>* cur_algo_hdl = getCurAlgoTypeHandle(algoType);
    if (!cur_algo_hdl) {
        LOGE_ANALYZER("can't find current type %d algo", algoType);
        return XCAM_RETURN_ERROR_FAILED;
    }
    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    NULL_RETURN_RET(algo_map, XCAM_RETURN_ERROR_FAILED);
    std::map<int, SmartPtr<RkAiqHandle>>::iterator it = algo_map->find(id);

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return XCAM_RETURN_ERROR_FAILED;
    }

    // if it's the current algo handle, clear it
    if ((*cur_algo_hdl).ptr() == it->second.ptr()) {
        (*cur_algo_hdl).release();
        for (auto& algo : mCurIspAlgoHandleList) {
            if (algo->getAlgoType() == algoType)
                algo.release();
        }
        for (auto& algo : mCurIsppAlgoHandleList) {
            if (algo->getAlgoType() == algoType)
                algo.release();
        }
    }

    algo_map->erase(it);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

bool
RkAiqCore::getAxlibStatus(int algoType, int id)
{
    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algoType);
    NULL_RETURN_RET(algo_map, false);
    std::map<int, SmartPtr<RkAiqHandle>>::iterator it = algo_map->find(id);

    if (it == algo_map->end()) {
        LOGE_ANALYZER("can't find type id <%d, %d> algo", algoType, id);
        return false;
    }

    LOGD_ANALYZER("algo type id <%d,%d> status %s", algoType, id,
                  it->second->getEnable() ? "enable" : "disable");

    return it->second->getEnable();
}

const RkAiqAlgoContext*
RkAiqCore::getEnabledAxlibCtx(const int algo_type)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE ||
            algo_type >= RK_AIQ_ALGO_TYPE_MAX)
        return NULL;

    SmartPtr<RkAiqHandle>* algo_handle = getCurAlgoTypeHandle(algo_type);

    if (algo_handle == nullptr) {
        return NULL;
    }

    if ((*algo_handle).ptr())
        return (*algo_handle)->getAlgoCtx();
    else
        return NULL;
}

void
RkAiqCore::copyIspStats(SmartPtr<RkAiqAecStatsProxy>& aecStat,
                        SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                        SmartPtr<RkAiqAfStatsProxy>& afStat,
                        rk_aiq_isp_stats_t* to)
{
    if (aecStat.ptr()) {
        to->aec_stats = aecStat->data()->aec_stats;
        to->frame_id = aecStat->data()->frame_id;
    }
    to->awb_hw_ver = 0;
    if (awbStat.ptr())
        to->awb_stats_v200 = awbStat->data()->awb_stats;
    if (afStat.ptr())
        to->af_stats = afStat->data()->af_stats;
}

void
RkAiqCore::cacheIspStatsToList(SmartPtr<RkAiqAecStatsProxy>& aecStat,
                               SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                               SmartPtr<RkAiqAfStatsProxy>& afStat)
{
    SmartLock locker (ispStatsListMutex);
    SmartPtr<RkAiqStatsProxy> stats = NULL;
    if (mAiqStatsPool->has_free_items()) {
        stats = mAiqStatsPool->get_item();
    } else {
        if(mAiqStatsCachedList.empty()) {
            LOGW_ANALYZER("no free or cached stats, user may hold all stats buf !");
            return ;
        }
        stats = mAiqStatsCachedList.front();
        mAiqStatsCachedList.pop_front();
    }

    copyIspStats(aecStat, awbStat, afStat, &stats->data()->result);

    mAiqStatsCachedList.push_back(stats);
    mIspStatsCond.broadcast ();
}

XCamReturn RkAiqCore::get3AStatsFromCachedList(rk_aiq_isp_stats_t **stats, int timeout_ms)
{
    SmartLock locker (ispStatsListMutex);
    int code = 0;
    while (mState != RK_AIQ_CORE_STATE_STOPED &&
            mAiqStatsCachedList.empty() &&
            code == 0) {
        if (timeout_ms < 0)
            code = mIspStatsCond.wait(ispStatsListMutex);
        else
            code = mIspStatsCond.timedwait(ispStatsListMutex, timeout_ms * 1000);
    }

    if (mState == RK_AIQ_CORE_STATE_STOPED) {
        *stats = NULL;
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqStatsCachedList.empty()) {
        if (code == ETIMEDOUT) {
            *stats = NULL;
            return XCAM_RETURN_ERROR_TIMEOUT;
        } else {
            *stats = NULL;
            return XCAM_RETURN_ERROR_FAILED;
        }
    }
    SmartPtr<RkAiqStatsProxy> stats_proxy = mAiqStatsCachedList.front();
    mAiqStatsCachedList.pop_front();
    *stats = &stats_proxy->data()->result;
    mAiqStatsOutMap[*stats] = stats_proxy;
    stats_proxy.release();

    return XCAM_RETURN_NO_ERROR;
}

void RkAiqCore::release3AStatsRef(rk_aiq_isp_stats_t *stats)
{
    SmartLock locker (ispStatsListMutex);

    std::map<rk_aiq_isp_stats_t*, SmartPtr<RkAiqStatsProxy>>::iterator it;
    it = mAiqStatsOutMap.find(stats);
    if (it != mAiqStatsOutMap.end()) {
        mAiqStatsOutMap.erase(it);
    }
}

XCamReturn RkAiqCore::get3AStatsFromCachedList(rk_aiq_isp_stats_t &stats)
{
    SmartLock locker (ispStatsListMutex);
    if(!mAiqStatsCachedList.empty()) {
        SmartPtr<RkAiqStatsProxy> stats_proxy = mAiqStatsCachedList.front();
        mAiqStatsCachedList.pop_front();
        stats = stats_proxy->data()->result;
        stats_proxy.release();
        return XCAM_RETURN_NO_ERROR;
    } else {
        return XCAM_RETURN_ERROR_FAILED;
    }
}

XCamReturn
RkAiqCore::analyze(const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    {
        SmartLock locker (mApiMutex);
        mSafeEnableAlgo = false;
    }

    if (!firstStatsReceived) {
        firstStatsReceived = true;
        mState = RK_AIQ_CORE_STATE_RUNNING;
    }

    switch (buffer->_buf_type) {
    case ISP_POLL_3A_STATS:
        {
            SmartPtr<RkAiqAecStatsProxy> aecStat = NULL;
            SmartPtr<RkAiqAwbStatsProxy> awbStat = NULL;
            SmartPtr<RkAiqAfStatsProxy> afStat = NULL;
            handleIspStats(buffer);
            handleAecStats(buffer, aecStat);
            handleAwbStats(buffer, awbStat);
            handleAfStats(buffer, afStat);
            cacheIspStatsToList(aecStat, awbStat, afStat);
            handleAtmoStats(buffer);
            handleAdehazeStats(buffer);
        }
        break;
    case ISPP_POLL_TNR_STATS:
        break;
    case ISPP_POLL_NR_STATS:
        handleOrbStats(buffer);
        break;
    case ISP_POLL_SP:
    {
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ISP_POLL_SP_OK,
                buffer->get_sequence(), buffer);
        post_message(msg);
        break;
    }
    case ISP_GAIN:
    {
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ISP_GAIN_OK,
                buffer->get_sequence(), buffer);
        post_message(msg);
        break;
    }
    case ISPP_GAIN_KG:
    {
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ISPP_GAIN_KG_OK,
                buffer->get_sequence(), buffer);
        post_message(msg);
        break;
    }
    case ISPP_GAIN_WR:
    {
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ISPP_GAIN_WR_OK,
                buffer->get_sequence(), buffer);
        post_message(msg);
        break;
    }
    case ISP_NR_IMG:
    {
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_NR_IMG_OK,
                buffer->get_sequence(), buffer);
        post_message(msg);
        break;
    }
    case ISP_POLL_TX:
    {
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ISP_POLL_TX_OK,
                buffer->get_sequence(), buffer);
        post_message(msg);
        break;
    }
    defalut:
        LOGW_ANALYZER("don't know buffer type: 0x%x!", buffer->_buf_type);
        break;
    }

    return ret;
}

XCamReturn
RkAiqCore::events_analyze(const SmartPtr<ispHwEvt_t> &evts)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqExpParamsProxy> preExpParams = nullptr;
    SmartPtr<RkAiqExpParamsProxy> curExpParams = nullptr;

    const SmartPtr<Isp20Evt> isp20Evts =
        evts.dynamic_cast_ptr<Isp20Evt>();
    uint32_t sequence = isp20Evts->sequence;
    if (sequence == 0)
        return ret;

    uint32_t id = 0, maxId = 0;
    if (sequence > 0)
        id = mLastAnalyzedId + 1 > sequence ? mLastAnalyzedId + 1 : sequence;
    maxId = sequence + isp20Evts->expDelay;

    LOGD_ANALYZER("sequence(%d), expDelay(%d), id(%d), maxId(%d)",
                  isp20Evts->sequence, isp20Evts->expDelay,
                  id, maxId);

    while (id <= maxId) {
        if (isp20Evts->getExpInfoParams(preExpParams, id > 0 ? id - 1 : 0 ) < 0) {
            LOGE_ANALYZER("id(%d) get pre exp failed!", id);
            break;
        }
        if (isp20Evts->getExpInfoParams(curExpParams, id) < 0) {
            LOGE_ANALYZER("id(%d) get exp failed!", id);
            break;
        }

        SmartPtr<RkAiqSofInfoWrapperProxy> sofInfo = NULL;
        if (mAiqSofInfoWrapperPool->has_free_items()) {
            sofInfo = mAiqSofInfoWrapperPool->get_item();
        } else {
            LOGE_ANALYZER("no free item for sofInfo!");
            return XCAM_RETURN_BYPASS;
        }

        sofInfo->data()->sequence = id;
        sofInfo->data()->preExp = preExpParams;
        sofInfo->data()->curExp = curExpParams;
        sofInfo->data()->sof = isp20Evts->getSofTimeStamp();


        sofInfo->setId(id);
        sofInfo->setType(RK_AIQ_SHARED_TYPE_SOF_INFO);

        SmartPtr<XCamMessage> msg = new RkAiqCoreExpMsg(XCAM_MESSAGE_SOF_INFO_OK,
                id, sofInfo);
        post_message(msg);

        mLastAnalyzedId = id;
        id++;

        LOGD_ANALYZER("analyze the id(%d), sequence(%d), mLastAnalyzedId(%d)",
                id, sequence, mLastAnalyzedId);
    }

    return ret;
}

XCamReturn
RkAiqCore::preProcess(enum rk_aiq_core_analyze_type_e type)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    std::list<int>& algo_list =
        mRkAiqCoreGroupManager->getGroupAlgoList(type);

    for (auto algo : algo_list) {
        SmartPtr<RkAiqHandle>* algoHdl = getCurAlgoTypeHandle(algo);
        if (algoHdl && (*algoHdl)->getEnable()) {
            ret = (*algoHdl)->updateConfig(true);
            RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d updateConfig failed", algo);
            ret = (*algoHdl)->preProcess();
            RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d processing failed", algo);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::processing(enum rk_aiq_core_analyze_type_e type)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    std::list<int>& algo_list =
        mRkAiqCoreGroupManager->getGroupAlgoList(type);

    for (auto algo : algo_list) {
        SmartPtr<RkAiqHandle>* algoHdl = getCurAlgoTypeHandle(algo);
        if (algoHdl && (*algoHdl)->getEnable()) {
            ret = (*algoHdl)->processing();
            RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d processing failed", algo);
        }
    }


    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::postProcess(enum rk_aiq_core_analyze_type_e type)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    std::list<int>& algo_list =
        mRkAiqCoreGroupManager->getGroupAlgoList(type);

    for (auto algo : algo_list) {
        SmartPtr<RkAiqHandle>* algoHdl = getCurAlgoTypeHandle(algo);
        if (algoHdl && (*algoHdl)->getEnable()) {
            ret = (*algoHdl)->postProcess();
            RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d postProcess failed", algo);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::setHwInfos(struct RkAiqHwInfo &hw_info)
{
    ENTER_ANALYZER_FUNCTION();
    mHwInfo = hw_info;
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::setCpsLtCfg(rk_aiq_cpsl_cfg_t &cfg)
{
    ENTER_ANALYZER_FUNCTION();
    if (mState < RK_AIQ_CORE_STATE_INITED) {
        LOGE_ANALYZER("should call afer init");
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (mCpslCap.modes_num == 0)
        return XCAM_RETURN_ERROR_PARAM;

    int i = 0;
    for (; i < mCpslCap.modes_num; i++) {
        if (mCpslCap.supported_modes[i] == cfg.mode)
            break;
    }

    if (i == mCpslCap.modes_num) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (cfg.mode == RK_AIQ_OP_MODE_AUTO) {
        mAlogsComSharedParams.cpslCfg.u.a = cfg.u.a;
    } else if (cfg.mode == RK_AIQ_OP_MODE_MANUAL) {
        mAlogsComSharedParams.cpslCfg.u.m = cfg.u.m;
    } else {
        return XCAM_RETURN_ERROR_PARAM;
    }

    mAlogsComSharedParams.cpslCfg.mode = cfg.mode;

    for (i = 0; i < mCpslCap.lght_src_num; i++) {
        if (mCpslCap.supported_lght_src[i] == cfg.lght_src)
            break;
    }

    if (i == mCpslCap.lght_src_num) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    mAlogsComSharedParams.cpslCfg = cfg;
    LOGD("set cpsl: mode %d", cfg.mode);
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::getCpsLtInfo(rk_aiq_cpsl_info_t &info)
{
    ENTER_ANALYZER_FUNCTION();
    if (mState < RK_AIQ_CORE_STATE_INITED) {
        LOGE_ANALYZER("should call afer init");
        return XCAM_RETURN_ERROR_FAILED;
    }

    info.mode = mAlogsComSharedParams.cpslCfg.mode;
    if (info.mode == RK_AIQ_OP_MODE_MANUAL) {
        info.on = mAlogsComSharedParams.cpslCfg.u.m.on;
        info.strength_led = mAlogsComSharedParams.cpslCfg.u.m.strength_led;
        info.strength_ir = mAlogsComSharedParams.cpslCfg.u.m.strength_ir;
    } else {
        info.on = mCurCpslOn;
        info.gray = mAlogsComSharedParams.gray_mode;
    }

    info.lght_src = mAlogsComSharedParams.cpslCfg.lght_src;
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::queryCpsLtCap(rk_aiq_cpsl_cap_t &cap)
{
    ENTER_ANALYZER_FUNCTION();
    if (mHwInfo.fl_supported || mHwInfo.irc_supported) {
        cap.supported_modes[0] = RK_AIQ_OP_MODE_AUTO;
        cap.supported_modes[1] = RK_AIQ_OP_MODE_MANUAL;
        cap.modes_num = 2;
    } else {
        cap.modes_num = 0;
    }

    cap.lght_src_num = 0;
    if (mHwInfo.fl_supported) {
        cap.supported_lght_src[0] = RK_AIQ_CPSLS_LED;
        cap.lght_src_num++;
    }

    if (mHwInfo.irc_supported) {
        cap.supported_lght_src[cap.lght_src_num] = RK_AIQ_CPSLS_IR;
        cap.lght_src_num++;
    }

    if (cap.lght_src_num > 1) {
        cap.supported_lght_src[cap.lght_src_num] = RK_AIQ_CPSLS_MIX;
        cap.lght_src_num++;
    }

    cap.strength_led.min = 0;
    cap.strength_led.max = 100;
    if (!mHwInfo.fl_strth_adj)
        cap.strength_led.step = 100;
    else
        cap.strength_led.step = 1;

    cap.strength_ir.min = 0;
    cap.strength_ir.max = 100;
    if (!mHwInfo.fl_ir_strth_adj)
        cap.strength_ir.step = 100;
    else
        cap.strength_ir.step = 1;

    cap.sensitivity.min = 0;
    cap.sensitivity.max = 100;
    cap.sensitivity.step = 1;

    LOGI("cpsl cap: light_src_num %d, led_step %f, ir_step %f",
         cap.lght_src_num, cap.strength_led.step, cap.strength_ir.step);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::genCpslResult(RkAiqFullParams* params)
{
    rk_aiq_cpsl_cfg_t* cpsl_cfg = &mAlogsComSharedParams.cpslCfg;

    if (cpsl_cfg->mode == RK_AIQ_OP_MODE_INVALID)
        return XCAM_RETURN_NO_ERROR;

    if (mAiqCpslParamsPool->has_free_items()) {
        params->mCpslParams = mAiqCpslParamsPool->get_item();
    } else {
        LOGW_ANALYZER("no free cpsl params buffer!");
        return XCAM_RETURN_NO_ERROR;
    }

    RKAiqCpslInfoWrapper_t* cpsl_param = params->mCpslParams->data().ptr();
    //xcam_mem_clear(*cpsl_param);

    LOGD_ANALYZER("cpsl mode %d, light src %d", cpsl_cfg->mode, cpsl_cfg->lght_src);
    bool cpsl_on = false;
    bool need_update = false;

    if (cpsl_cfg->mode == RK_AIQ_OP_MODE_MANUAL) {
        if ((mCurCpslOn != cpsl_cfg->u.m.on) ||
                (fabs(mStrthLed - cpsl_cfg->u.m.strength_led) > EPSINON) ||
                (fabs(mStrthIr - cpsl_cfg->u.m.strength_ir) > EPSINON)) {
            need_update = true;
            cpsl_on = cpsl_cfg->u.m.on;
            cpsl_param->fl.power[0] = cpsl_cfg->u.m.strength_led / 100.0f;
            cpsl_param->fl_ir.power[0] = cpsl_cfg->u.m.strength_ir / 100.0f;
        }
    } else {
        RkAiqAlgosGroupShared_t* shared = nullptr;
        int groupId = getGroupId(RK_AIQ_ALGO_TYPE_ASD);
        if (groupId >= 0) {
            if (getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
                return XCAM_RETURN_BYPASS;
        } else
            return XCAM_RETURN_BYPASS;
        RkAiqAlgoPreResAsdInt* asd_pre_rk = (RkAiqAlgoPreResAsdInt*)shared->preResComb.asd_pre_res;
        if (asd_pre_rk) {
            asd_preprocess_result_t* asd_result = &asd_pre_rk->asd_result;
            if (mCurCpslOn != asd_result->cpsl_on) {
                need_update = true;
                cpsl_on = asd_result->cpsl_on;
            }
        }
        cpsl_param->fl.power[0] = 1.0f;
        cpsl_param->fl_ir.power[0] = 1.0f;
    }

    // need to init cpsl status, cause the cpsl driver state
    // may be not correct
    if (mState == RK_AIQ_CORE_STATE_INITED)
        need_update = true;

    if (need_update) {
        if (cpsl_cfg->lght_src & RK_AIQ_CPSLS_LED) {
            cpsl_param->update_fl = true;
            if (cpsl_on)
                cpsl_param->fl.flash_mode = RK_AIQ_FLASH_MODE_TORCH;
            else
                cpsl_param->fl.flash_mode = RK_AIQ_FLASH_MODE_OFF;
            if (cpsl_on ) {
                cpsl_param->fl.strobe = true;
                mAlogsComSharedParams.fill_light_on = true;
            } else {
                cpsl_param->fl.strobe = false;
                mAlogsComSharedParams.fill_light_on = false;
            }
            LOGD_ANALYZER("cpsl fl mode %d, strength %f, strobe %d",
                          cpsl_param->fl.flash_mode, cpsl_param->fl.power[0],
                          cpsl_param->fl.strobe);
        }

        if (cpsl_cfg->lght_src & RK_AIQ_CPSLS_IR) {
            cpsl_param->update_ir = true;
            if (cpsl_on) {
                cpsl_param->ir.irc_on = true;
                cpsl_param->fl_ir.flash_mode = RK_AIQ_FLASH_MODE_TORCH;
                cpsl_param->fl_ir.strobe = true;
                mAlogsComSharedParams.fill_light_on = true;
            } else {
                cpsl_param->ir.irc_on = false;
                cpsl_param->fl_ir.flash_mode = RK_AIQ_FLASH_MODE_OFF;
                cpsl_param->fl_ir.strobe = false;
                mAlogsComSharedParams.fill_light_on = false;
            }
            LOGD_ANALYZER("cpsl irc on %d, fl_ir: mode %d, strength %f, strobe %d",
                          cpsl_param->ir.irc_on, cpsl_param->fl_ir.flash_mode, cpsl_param->fl_ir.power[0],
                          cpsl_param->fl_ir.strobe);
        }

        if (mGrayMode == RK_AIQ_GRAY_MODE_CPSL) {
            if (mAlogsComSharedParams.fill_light_on && cpsl_cfg->gray_on) {
                mAlogsComSharedParams.gray_mode = true;
            } else
                mAlogsComSharedParams.gray_mode = false;

        } else {
            /* no mutex lock protection for gray_mode with setGrayMode,
             * so need set again here
             */
            if (mGrayMode == RK_AIQ_GRAY_MODE_OFF)
                mAlogsComSharedParams.gray_mode = false;
            else if (mGrayMode == RK_AIQ_GRAY_MODE_ON)
                mAlogsComSharedParams.gray_mode = true;
        }
        mCurCpslOn = cpsl_on;
        mStrthLed = cpsl_cfg->u.m.strength_led;
        mStrthIr = cpsl_cfg->u.m.strength_ir;
    } else {
        cpsl_param->update_ir = false;
        cpsl_param->update_fl = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::setGrayMode(rk_aiq_gray_mode_t mode)
{
    LOGD_ANALYZER("%s: gray mode %d", __FUNCTION__, mode);

    if (mAlogsComSharedParams.is_bw_sensor) {
        LOGE_ANALYZER("%s: not support for black&white sensor", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CalibDbV2_ColorAsGrey_t *colorAsGrey =
        (CalibDbV2_ColorAsGrey_t*)CALIBDBV2_GET_MODULE_PTR((void*)(mAlogsComSharedParams.calibv2), colorAsGrey);

    if (colorAsGrey->param.enable) {
        LOGE_ANALYZER("%s: not support,since color_as_grey is enabled in xml", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    mGrayMode = mode;
    if (mode == RK_AIQ_GRAY_MODE_OFF)
        mAlogsComSharedParams.gray_mode = false;
    else if (mode == RK_AIQ_GRAY_MODE_ON)
        mAlogsComSharedParams.gray_mode = true;
    else if (mode == RK_AIQ_GRAY_MODE_CPSL)
        ; // do nothing
    else
        LOGE_ANALYZER("%s: gray mode %d error", __FUNCTION__, mode);

    return XCAM_RETURN_NO_ERROR;
}

rk_aiq_gray_mode_t
RkAiqCore::getGrayMode()
{
    LOGD_ANALYZER("%s: gray mode %d", __FUNCTION__, mGrayMode);
    return mGrayMode;
}

void
RkAiqCore::setSensorFlip(bool mirror, bool flip)
{
    mAlogsComSharedParams.sns_mirror = mirror;
    mAlogsComSharedParams.sns_flip = flip;
}

XCamReturn
RkAiqCore::setCalib(const CamCalibDbContext_t* aiqCalib)
{
    ENTER_ANALYZER_FUNCTION();

    if (mState != RK_AIQ_CORE_STATE_STOPED) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    /* TODO: xuhf WARNING */
    mAlogsComSharedParams.calib = aiqCalib;
    mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::setCalib(const CamCalibDbV2Context_t* aiqCalib)
{
    ENTER_ANALYZER_FUNCTION();

    if (mState != RK_AIQ_CORE_STATE_STOPED) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

    /* TODO: xuhf WARNING */
    mAlogsComSharedParams.calibv2 = aiqCalib;
    mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCore::calibTuning(const CamCalibDbV2Context_t* aiqCalib,
                                  ModuleNameList& change_name_list)
{
    ENTER_ANALYZER_FUNCTION();

    if (!aiqCalib || !change_name_list) {
        LOGE_ANALYZER("invalied tuning param\n");
        return XCAM_RETURN_ERROR_PARAM;
    }

    // Fill new calib to the AlogsSharedParams
    /* TODO: xuhf WARNING */
    mAlogsComSharedParams.calibv2 = aiqCalib;
    mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    std::for_each(std::begin(*change_name_list), std::end(*change_name_list),
                  [this](const std::string& name) {
                      if (!name.compare(0, 4, "cpsl", 0, 4)) {
                          initCpsl();
                      } else if (!name.compare(0, 11, "colorAsGrey", 0, 11)) {
                          setGrayMode(mGrayMode);
                      }
                  });

    AlgoList change_list = std::make_shared<std::list<RkAiqAlgoType_t>>();
    std::transform(
        change_name_list->begin(), change_name_list->end(), std::back_inserter(*change_list),
        [](const std::string name) { return RkAiqCalibDbV2::string2algostype(name.c_str()); });

    change_list->sort();
    change_list->unique();

    // Call prepare of the Alog handle annd notify update param
    list<RkAiqAlgoType_t>::iterator it;
    for(it = change_list->begin(); it != change_list->end(); it++) {
        auto algo_handle = getCurAlgoTypeHandle(*it);
        if (algo_handle) {
            (*algo_handle)->updateConfig(true);
            (*algo_handle)->prepare();
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCore::setMemsSensorIntf(const rk_aiq_mems_sensor_intf_t* intf) {
    if (mState != RK_AIQ_CORE_STATE_INITED) {
        LOGE_ANALYZER("set MEMS sensor intf in wrong aiq state %d !", mState);
        return XCAM_RETURN_ERROR_FAILED;
    }

    mMemsSensorIntf = intf;

    return XCAM_RETURN_NO_ERROR;
}

const rk_aiq_mems_sensor_intf_t* RkAiqCore::getMemsSensorIntf() {
    return mMemsSensorIntf;
}

void RkAiqCore::post_message (SmartPtr<XCamMessage> &msg)
{
    //mRkAiqCoreMsgTh->push_msg(msg);
    mRkAiqCoreGroupManager->handleMessage(msg);
}

XCamReturn
RkAiqCore::handle_message (const SmartPtr<XCamMessage> &msg)
{
    //return mRkAiqCoreGroupManager->handleMessage(msg);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::otherGroupAnalye(uint32_t id, const SmartPtr<RkAiqCoreExpMsg> &msg)
{
    ENTER_ANALYZER_FUNCTION();

    SmartPtr<RkAiqFullParamsProxy> fullParam;
    uint32_t sequence = msg->msg->data()->sequence;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_OTHER);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId = msg->msg->data()->sequence;
    shared->preExp = msg->msg->data()->preExp->data()->aecExpInfo;
    shared->curExp = msg->msg->data()->curExp->data()->aecExpInfo;

    LOGI_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_OTHER);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::aeGroupAnalyze(uint32_t id, XCamVideoBuffer* aecStatsBuf)
{
    uint32_t frameId = id;

    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_AE);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId = frameId;
    shared->aecStatsBuf = aecStatsBuf;

    LOGI_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_AE);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::awbGroupAnalyze(int32_t id,
                           XCamVideoBuffer* aePreRes,
                           XCamVideoBuffer* awbStatsBuf)
{
    uint32_t frameId = id;

    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_AWB);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId                 = frameId;
    shared->awbStatsBuf             = awbStatsBuf;
    shared->res_comb.ae_pre_res     = aePreRes;

    LOGI_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_AWB);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::measGroupAnalyze(uint32_t id, const SmartPtr<RkAiqCoreIspStatsMsg> &msg, XCamVideoBuffer* aePreRes)
{
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_MEAS);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId                 = id;
    shared->ispStats                = msg->msg->data().ptr();
    shared->res_comb.ae_pre_res     = aePreRes;

    LOGI_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_MEAS);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::amdGroupAnalyze(uint32_t id, const SmartPtr<RkAiqCoreExpMsg> &expMsg,
                           XCamVideoBuffer* sp, XCamVideoBuffer* ispGain)
{
    uint32_t frameId = sp->frame_id;
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_AMD);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId = frameId;
    shared->sp = sp;
    shared->ispGain = ispGain;
    shared->curExp = expMsg->msg->data()->curExp->data()->aecExpInfo;

    LOGD_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_AMD);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::lscGroupAnalyze(uint32_t id, const SmartPtr<RkAiqCoreExpMsg> &expMsg,
                           XCamVideoBuffer* awbProcRes, XCamVideoBuffer* tx)
{
    uint32_t frameId = expMsg->frame_id;
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_LSC);
    RkAiqAlgosGroupShared_t *shared     = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId                     = frameId;
    shared->tx                          = tx;
    shared->res_comb.awb_proc_res       = awbProcRes;
    shared->curExp                      = expMsg->msg->data()->curExp->data()->aecExpInfo;

    LOGD_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_LSC);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    tx->unref(tx);
    awbProcRes->unref(awbProcRes);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::thumbnailsGroupAnalyze(rkaiq_image_source_t &thumbnailsSrc)
{
    uint32_t frameId = thumbnailsSrc.frame_id;

    if (mThumbnailsService.ptr())
        mThumbnailsService->OnFrameEvent(thumbnailsSrc);

    thumbnailsSrc.image_source->unref(thumbnailsSrc.image_source);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::mfnrGroupAnalyze(uint32_t id, const SmartPtr<RkAiqCoreExpMsg> &expMsg,
                            XCamVideoBuffer* ispGain, XCamVideoBuffer* kgGain,
                            rk_aiq_amd_params_t &amdParams)
{
    uint32_t frameId = expMsg->frame_id;
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_AMFNR);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId = frameId;
    shared->ispGain = ispGain;
    shared->kgGain = kgGain;
    // shared->amdResParams = amdParams;
    shared->curExp = expMsg->msg->data()->curExp->data()->aecExpInfo;

    LOGD_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_AMFNR);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::ynrGroupAnalyze(uint32_t id, const SmartPtr<RkAiqCoreExpMsg> &expMsg,
                           XCamVideoBuffer* ispGain, XCamVideoBuffer* wrGain,
                           rk_aiq_amd_params_t &amdParams)
{
    uint32_t frameId = expMsg->frame_id;
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_AYNR);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId = frameId;
    shared->ispGain = ispGain;
    shared->wrGain = wrGain;
    // shared->amdResParams = amdParams;
    shared->curExp = expMsg->msg->data()->curExp->data()->aecExpInfo;

    LOGD_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_AYNR);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::grp0Analyze(grp0AnalyzerInParams_t &inParams)
{
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_GRP0);
    RkAiqAlgosGroupShared_t *shared     = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId                     = inParams.id;
    shared->aecStatsBuf                 = inParams.aecStatsBuf;
    shared->awbStatsBuf                 = inParams.awbStatsBuf;
    shared->afStatsBuf                  = inParams.afStatsBuf;
    shared->res_comb.ae_pre_res         = inParams.aePreRes;
    shared->res_comb.ae_proc_res        = inParams.aeProcRes;
    shared->curExp                      = inParams.sofInfoMsg->msg->data()->curExp->data()->aecExpInfo;

    LOGI_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_GRP0);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::grp1Analyze(grp1AnalyzerInParams_t &inParams)
{
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_GRP1);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId                 = inParams.id;
    shared->aecStatsBuf             = inParams.aecStatsBuf;
    shared->awbStatsBuf             = inParams.awbStatsBuf;
    shared->afStatsBuf              = inParams.afStatsBuf;
    shared->res_comb.ae_pre_res     = inParams.aePreRes;
    shared->res_comb.awb_proc_res   = inParams.awbProcRes;
    shared->curExp                  = inParams.sofInfoMsg->msg->data()->curExp->data()->aecExpInfo;

    LOGI_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_GRP1);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::afAnalyze(afAnalyzerInParams_t &inParams)
{
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_AF);
    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[grpMask];
    shared->frameId                 = inParams.id;
    shared->aecStatsBuf             = inParams.aecStatsBuf;
    shared->afStatsBuf              = inParams.afStatsBuf;
    shared->res_comb.ae_pre_res     = inParams.aePreRes;
    shared->res_comb.ae_proc_res    = inParams.aeProcRes;
    shared->curExp                  = inParams.expInfo->msg->data()->curExp->data()->aecExpInfo;

    LOGI_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_AF);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn RkAiqCore::eisGroupAnalyze(uint32_t id, const SmartPtr<RkAiqCoreExpMsg>& expMsg,
                                      XCamVideoBuffer* orbStats, XCamVideoBuffer* nrImg) {
    uint32_t frameId = orbStats->frame_id;
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    RkAiqAlgosGroupShared_t* shared = mAlogsGroupSharedParamsMap[RK_AIQ_CORE_ANALYZE_EIS];
    shared->frameId                 = frameId;
    shared->orbStats                = orbStats;
    shared->nrImg                   = nrImg;
    shared->sof                     = expMsg->msg->data()->sof;
    shared->curExp                  = expMsg->msg->data()->curExp->data()->aecExpInfo;

    LOGD_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_EIS);
    if (fullParam.ptr() && mCb) {
        if (fullParam->data()->mFecParams.ptr()) {
            rk_aiq_isp_fec_params_v20_t* fec_params = fullParam->data()->mFecParams->data().ptr();
            if (fec_params->update_mask != 0) {
                mCb->rkAiqCalcDone(fullParam);
            }
        }
    }

    if (orbStats != nullptr)
        orbStats->unref(orbStats);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::orbGroupAnalyze(uint32_t id, XCamVideoBuffer* orbStats)
{
    uint32_t frameId = orbStats->frame_id;
    SmartPtr<RkAiqFullParamsProxy> fullParam;

    RkAiqAlgosGroupShared_t *shared = mAlogsGroupSharedParamsMap[RK_AIQ_CORE_ANALYZE_ORB];
    shared->frameId = frameId;
    shared->orbStats = orbStats;

    LOGD_ANALYZER("%s, start analyzing id(%d)", __FUNCTION__, shared->frameId);

    fullParam = analyzeInternal(RK_AIQ_CORE_ANALYZE_ORB);
    if (fullParam.ptr() && mCb) {
        mCb->rkAiqCalcDone(fullParam);
    }

    if (orbStats != nullptr)
        orbStats->unref(orbStats);

    return XCAM_RETURN_NO_ERROR;
}

void RkAiqCore::newAiqParamsPool()
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
                mAiqIspAwbParamsPool        = new RkAiqIspAwbParamsPool("RkAiqIspAwbParams", RkAiqCore::DEFAULT_POOL_SIZE);
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
            case RK_AIQ_ALGO_TYPE_ATMO:
                mAiqIspTmoParamsPool        = new RkAiqIspTmoParamsPool("RkAiqIspTmoParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACCM:
                mAiqIspCcmParamsPool        = new RkAiqIspCcmParamsPool("RkAiqIspCcmParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ALSC:
                mAiqIspLscParamsPool        = new RkAiqIspLscParamsPool("RkAiqIspLscParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ABLC:
                mAiqIspBlcParamsPool        = new RkAiqIspBlcParamsPool("RkAiqIspBlcParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ARAWNR:
                mAiqIspRawnrParamsPool      = new RkAiqIspRawnrParamsPool("RkAiqIspRawnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AGIC:
                mAiqIspGicParamsPool        = new RkAiqIspGicParamsPool("RkAiqIspGicParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADEBAYER:
                mAiqIspDebayerParamsPool    = new RkAiqIspDebayerParamsPool("RkAiqIspDebayerParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ALDCH:
                mAiqIspLdchParamsPool       = new RkAiqIspLdchParamsPool("RkAiqIspLdchParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_A3DLUT:
                mAiqIspLut3dParamsPool      = new RkAiqIspLut3dParamsPool("RkAiqIspLut3dParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADHAZ:
                mAiqIspDehazeParamsPool     = new RkAiqIspDehazeParamsPool("RkAiqIspDehazeParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AGAMMA:
                mAiqIspAgammaParamsPool     = new RkAiqIspAgammaParamsPool("RkAiqIspAgammaParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADEGAMMA:
                mAiqIspAdegammaParamsPool   = new RkAiqIspAdegammaParamsPool("RkAiqIspAdegammaParams", RkAiqCore::DEFAULT_POOL_SIZE);
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
            case RK_AIQ_ALGO_TYPE_AGAIN:
                mAiqIspGainParamsPool       = new RkAiqIspGainParamsPool("RkAiqIspGainParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACP:
                mAiqIspCpParamsPool         = new RkAiqIspCpParamsPool("RkAiqIspCpParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AIE:
                mAiqIspIeParamsPool         = new RkAiqIspIeParamsPool("RkAiqIspIeParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AMD:
                mAiqIspMdParamsPool         = new RkAiqIspMdParamsPool("RkAiqIspMdParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AMFNR:
                mAiqIspTnrParamsPool        = new RkAiqIspTnrParamsPool("RkAiqIspTnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AYNR:
                mAiqIspYnrParamsPool        = new RkAiqIspYnrParamsPool("RkAiqIspYnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACNR:
                mAiqIspUvnrParamsPool       = new RkAiqIspUvnrParamsPool("RkAiqIspUvnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ASHARP:
                mAiqIspSharpenParamsPool    = new RkAiqIspSharpenParamsPool("RkAiqIspSharpenParams", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspEdgefltParamsPool    = new RkAiqIspEdgefltParamsPool("RkAiqIspEdgefltParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AORB:
                mAiqIspOrbParamsPool        = new RkAiqIspOrbParamsPool("RkAiqIspOrbParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AFEC:
            case RK_AIQ_ALGO_TYPE_AEIS:
                mAiqIspFecParamsPool        = new RkAiqIspFecParamsPool("RkAiqIspFecParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            default:
                break;
            }
        }
    }
}

void RkAiqCore::onThumbnailsResult(const rkaiq_thumbnails_t& thumbnail) {
    LOGV_ANALYZER("Callback thumbnail : id:%d, type:%d, 1/%dx1/%d, %dx%d", thumbnail.frame_id,
                  thumbnail.config.stream_type, thumbnail.config.width_intfactor,
                  thumbnail.config.height_intfactor, thumbnail.buffer->info.width,
                  thumbnail.buffer->info.height);
#if 0
    thumbnail.buffer->ref(thumbnail.buffer);
    if (thumbnail.frame_id == 1) {
        char* ptr = reinterpret_cast<char*>(thumbnail.buffer->map(thumbnail.buffer));
        size_t size = thumbnail.buffer->info.size;
        std::string path = "/data/thumbnails_";
        path.append(std::to_string(thumbnail.frame_id));
        path.append("_");
        path.append(std::to_string(thumbnail.buffer->info.width));
        path.append("x");
        path.append(std::to_string(thumbnail.buffer->info.height));
        path.append(".yuv");
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(ptr, size);
        thumbnail.buffer->unmap(thumbnail.buffer);
    }
    thumbnail.buffer->unref(thumbnail.buffer);
#endif
}

int32_t
RkAiqCore::getGroupId(RkAiqAlgoType_t type)
{
    auto mapIter = mAlgoTypeToGrpMaskMap.find(type);
    if (mapIter != mAlgoTypeToGrpMaskMap.end()) {
        return mapIter->second;
    } else {
        LOGW_ANALYZER("don't find the group id of module(0x%x)", type);
        return XCAM_RETURN_ERROR_FAILED;
    }
}

XCamReturn
RkAiqCore::getGroupSharedParams(int32_t groupId, RkAiqAlgosGroupShared_t* &shared)
{
    auto mapIter = mAlogsGroupSharedParamsMap.find(groupId);
    if (mapIter != mAlogsGroupSharedParamsMap.end()) {
        shared = mapIter->second;
    } else {
        LOGE_ANALYZER("don't find the group shared params of group(0x%x)", groupId);
        return XCAM_RETURN_ERROR_FAILED;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::handleIspStats(const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqIspStatsIntProxy> ispStatsInt = NULL;

    if (mAiqIspStatsIntPool->has_free_items()) {
        ispStatsInt = mAiqIspStatsIntPool->get_item();
    } else {
        LOGE_ANALYZER("no free ispStatsInt!");
        return XCAM_RETURN_BYPASS;
    }

    ret = mTranslator->translateIspStats(buffer, ispStatsInt);
    if (ret) {
        LOGE_ANALYZER("translate isp stats failed!");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreIspStatsMsg(XCAM_MESSAGE_ISP_STATS_OK,
            id, ispStatsInt);
    post_message(msg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::handleAecStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAecStatsProxy>& aecStat_ret)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqAecStatsProxy> aecStats = NULL;

    if (mAiqAecStatsPool->has_free_items()) {
        aecStats = mAiqAecStatsPool->get_item();
    } else {
        LOGE_ANALYZER("no free aecStats buffer!");
        return XCAM_RETURN_BYPASS;
    }
    ret = mTranslator->translateAecStats(buffer, aecStats);
    if (ret < 0) {
        LOGE_ANALYZER("translate aec stats failed!");
        return XCAM_RETURN_BYPASS;
    }

    aecStat_ret = aecStats;

    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AEC_STATS_OK,
            id, aecStats);
    post_message(msg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::handleAwbStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAwbStatsProxy>& awbStat_ret)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqAwbStatsProxy> awbStats = NULL;

    if (mAiqAwbStatsPool->has_free_items()) {
        awbStats = mAiqAwbStatsPool->get_item();
    } else {
        LOGE_ANALYZER("no free awbStats buffer!");
        return XCAM_RETURN_BYPASS;
    }
    ret = mTranslator->translateAwbStats(buffer, awbStats);
    if (ret < 0) {
        LOGE_ANALYZER("translate awb stats failed!");
        return XCAM_RETURN_BYPASS;
    }

    awbStat_ret = awbStats;

    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AWB_STATS_OK,
            id, awbStats);
    post_message(msg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::handleAfStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAfStatsProxy>& afStat_ret)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqAfStatsProxy> afStats = NULL;

    if (mAiqAfStatsPool->has_free_items()) {
        afStats = mAiqAfStatsPool->get_item();
    } else {
        LOGE_ANALYZER("no free afStats buffer!");
        return XCAM_RETURN_BYPASS;
    }
    ret = mTranslator->translateAfStats(buffer, afStats);
    if (ret < 0) {
        LOGE_ANALYZER("translate af stats failed!");
        return XCAM_RETURN_BYPASS;
    }

    afStat_ret = afStats;

    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AF_STATS_OK,
            id, afStats);
    post_message(msg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::handleAtmoStats(const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqAtmoStatsProxy> atmoStats = NULL;

    if (mAiqAtmoStatsPool->has_free_items()) {
        atmoStats = mAiqAtmoStatsPool->get_item();
    } else {
        LOGE_ANALYZER("no free atmoStats buffer!");
        return XCAM_RETURN_BYPASS;
    }
    ret = mTranslator->translateAtmoStats(buffer, atmoStats);
    if (ret < 0) {
        LOGE_ANALYZER("translate tmo stats failed!");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ATMO_STATS_OK,
            id, atmoStats);
    post_message(msg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::handleAdehazeStats(const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqAdehazeStatsProxy> adehazeStats = NULL;

    if (mAiqAdehazeStatsPool->has_free_items()) {
        adehazeStats = mAiqAdehazeStatsPool->get_item();
    } else {
        LOGE_ANALYZER("no free adehazeStats buffer!");
        return XCAM_RETURN_BYPASS;
    }
    ret = mTranslator->translateAdehazeStats(buffer, adehazeStats);
    if (ret < 0) {
        LOGE_ANALYZER("translate dehaze stats failed!");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ADEHAZE_STATS_OK,
            id, adehazeStats);
    post_message(msg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::handleOrbStats(const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqOrbStatsProxy> orbStats = NULL;
    if (mAiqOrbStatsIntPool->has_free_items()) {
        orbStats = mAiqOrbStatsIntPool->get_item();
    } else {
        LOGE_ANALYZER("no free orbStats!");
        return XCAM_RETURN_BYPASS;
    }

    ret = mTranslator->translateOrbStats(buffer, orbStats);
    if (ret)
        return XCAM_RETURN_BYPASS;

    uint32_t id = buffer->get_sequence();
    orbStats->setId(id);
    orbStats->setType(RK_AIQ_SHARED_TYPE_ORB_STATS);
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ORB_STATS_OK,
                                                      id, orbStats);
    post_message(msg);

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn RkAiqCore::set_sp_resolution(int &width, int &height, int &aligned_w, int &aligned_h)
{
    mSpWidth = width;
    mSpHeight = height;
    mSpAlignedWidth = aligned_w;
    mSpAlignedHeight = aligned_h;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::newAiqGroupAnayzer()
{
    mRkAiqCoreGroupManager = new RkAiqAnalyzeGroupManager(this);
    mRkAiqCoreGroupManager->parseAlgoGroup(mAlgosDesArray);
    return XCAM_RETURN_NO_ERROR;
}

} //namespace RkCam
