/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqCore.h"

#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

#include "RkAiqHandleInt.h"
#include "v4l2_buffer_proxy.h"
#ifdef RK_SIMULATOR_HW
#include "simulator/isp20_hw_simulator.h"
#else
#include "common/rkisp2-config.h"
#include "common/rkisp21-config.h"
#include "isp20/Isp20Evts.h"
#include "isp20/Isp20StatsBuffer.h"
#include "isp20/rkispp-config.h"
#endif

#include "PdafStreamProcUnit.h"
#include "SPStreamProcUnit.h"
// #include "MessageBus.h"
#include "RkAiqAnalyzeGroupManager.h"
#include "RkAiqResourceTranslatorV21.h"
#include "RkAiqResourceTranslatorV32.h"
#include "RkAiqResourceTranslatorV3x.h"
#ifdef RKAIQ_ENABLE_CAMGROUP
#include "RkAiqCamGroupManager.h"
#endif
#include "RkAiqCamProfiles.h"
#include "RkAiqCoreConfig.h"
#include "RkAiqCustomAeHandle.h"
#include "RkAiqCustomAwbHandle.h"

#include "RkAiqCoreConfig.h"

namespace RkCam {

#define EPSINON 0.0000001

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
uint16_t RkAiqCore::DEFAULT_POOL_SIZE = 6;

bool RkAiqCore::isGroupAlgo(int algoType) {
    static auto policy = mProfiles.getAlgoPolicies();
    return policy[algoType] == CamProfiles::AlgoSchedPolicy::kGroupOnly;
}

RkAiqCore::RkAiqCore(int isp_hw_ver)
    :
#ifndef RKAIQ_DISABLE_CORETHRD
    mRkAiqCoreTh(new RkAiqCoreThread(this)),
#endif
#if defined(ISP_HW_V20)
    mRkAiqCorePpTh(new RkAiqCoreThread(this)),
#endif
    mRkAiqCoreEvtsTh(new RkAiqCoreEvtsThread(this))
    , mState(RK_AIQ_CORE_STATE_INVALID)
    , mCb(NULL)
    , mIsSingleThread(false)
    , mAiqParamsPool(new RkAiqFullParamsPool("RkAiqFullParams", 32))
    , mAiqCpslParamsPool(new RkAiqCpslParamsPool("RkAiqCpslParamsPool", RkAiqCore::DEFAULT_POOL_SIZE))
    , mAiqStatsPool(nullptr)
    , mAiqSofInfoWrapperPool(new RkAiqSofInfoWrapperPool("RkAiqSofPoolWrapper", RkAiqCore::DEFAULT_POOL_SIZE))
    , mAiqIspStatsIntPool(new RkAiqIspStatsIntPool("RkAiqIspStatsIntPool", RkAiqCore::DEFAULT_POOL_SIZE))
    , mAiqAecStatsPool(nullptr)
    , mAiqAwbStatsPool(nullptr)
    , mAiqAtmoStatsPool(nullptr)
    , mAiqAdehazeStatsPool(nullptr)
    , mAiqAfStatsPool(nullptr)
    , mAiqOrbStatsIntPool(nullptr)
#if RKAIQ_HAVE_PDAF
    , mAiqPdafStatsPool(nullptr)
#endif
    , mCustomEnAlgosMask(0xffffffffffffffff)
    , groupUpdateMask(0x00)
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
    mIspOnline = false;
    mIspHwVer  = isp_hw_ver;
    mSafeEnableAlgo = true;
    mLastAnalyzedId = 0;
#ifdef RKAIQ_ENABLE_CAMGROUP
    mCamGroupCoreManager = NULL;
#endif
    mAllReqAlgoResMask = 0;
    mAlgosDesArray = g_default_3a_des;

#ifdef ISP_HW_V20
    mHasPp      = true;
    mTranslator = new RkAiqResourceTranslator();
#endif
#ifdef ISP_HW_V21
    mHasPp      = false;
    mTranslator = new RkAiqResourceTranslatorV21();
#endif
#ifdef ISP_HW_V30
    mHasPp      = false;
    mTranslator = new RkAiqResourceTranslatorV3x();
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    mHasPp          = false;
    mTranslator     = new RkAiqResourceTranslatorV32();
    mIsSingleThread = true;
#endif
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

#if RKAIQ_HAVE_ASD_V10
void RkAiqCore::initCpsl()
{
    queryCpsLtCap(mCpslCap);

    rk_aiq_cpsl_cfg_t* cfg = &mAlogsComSharedParams.cpslCfg;
    const CamCalibDbV2Context_t* aiqCalib = mAlogsComSharedParams.calibv2;
    CalibDbV2_Cpsl_t* calibv2_cpsl_db =
        (CalibDbV2_Cpsl_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, cpsl));
    CalibDbV2_Cpsl_Param_t* calibv2_cpsl_calib = &calibv2_cpsl_db->param;
    // TODO: something from calib
    LOGD_ASD("init cpsl enable %d num %d", calibv2_cpsl_calib->enable, mCpslCap.modes_num);
    if (mCpslCap.modes_num > 0 && calibv2_cpsl_calib->enable) {
        cfg->mode = calibv2_cpsl_calib->mode;

        if (calibv2_cpsl_calib->light_src == 1) {
            cfg->lght_src = RK_AIQ_CPSLS_LED;
        } else if (calibv2_cpsl_calib->light_src == 2) {
            cfg->lght_src = RK_AIQ_CPSLS_IR;
        } else if (calibv2_cpsl_calib->light_src == 3) {
            cfg->lght_src = RK_AIQ_CPSLS_MIX;
        } else {
            cfg->lght_src = RK_AIQ_CPSLS_INVALID;
        }
        cfg->gray_on = calibv2_cpsl_calib->force_gray;
        if (cfg->mode == RK_AIQ_OP_MODE_AUTO) {
            cfg->u.a.sensitivity = calibv2_cpsl_calib->auto_adjust_sens;
            cfg->u.a.sw_interval = calibv2_cpsl_calib->auto_sw_interval;
            LOGD_ASD("mode sensitivity %f, interval time %d s\n",
                     cfg->u.a.sensitivity, cfg->u.a.sw_interval);
        } else {
            cfg->u.m.on = calibv2_cpsl_calib->manual_on;
            cfg->u.m.strength_ir = calibv2_cpsl_calib->manual_strength;
            cfg->u.m.strength_led = calibv2_cpsl_calib->manual_strength;
            LOGD_ASD("on %d, strength_led %f, strength_ir %f \n",
                     cfg->u.m.on, cfg->u.m.strength_led, cfg->u.m.strength_ir);
        }
    } else {
        cfg->mode = RK_AIQ_OP_MODE_INVALID;
        LOGD_ASD("not support light compensation \n");
    }
}
#endif

#ifdef RKAIQ_ENABLE_CAMGROUP
void RkAiqCore::setCamGroupManager(RkAiqCamGroupManager* cam_group_manager) {
    mCamGroupCoreManager = cam_group_manager;
    if (mTranslator.ptr() && cam_group_manager) mTranslator->setGroupMode(true);

    string ini_path{mAlogsComSharedParams.resourcePath};
    if (ini_path.length() > 0) {
        ini_path.append("/profile.ini");
    } else {
        ini_path = "/etc/iqfiles/profile.ini";
    }
    mProfiles.ParseFromIni(ini_path);

    uint64_t removed_mask = 0xffffffffffffffff;
    if (mCamGroupCoreManager != nullptr) {
        if (isGroupAlgo(RK_AIQ_ALGO_TYPE_AE)) {
            removed_mask &=
                ~((1ULL << XCAM_MESSAGE_AE_PRE_RES_OK) | (1ULL << XCAM_MESSAGE_AE_PROC_RES_OK));
        }
        if (isGroupAlgo(RK_AIQ_ALGO_TYPE_AWB)) {
            removed_mask &= ~(1ULL << XCAM_MESSAGE_AWB_PROC_RES_OK);
        }
        if (isGroupAlgo(RK_AIQ_ALGO_TYPE_AF)) {
            removed_mask &= ~(1ULL << XCAM_MESSAGE_AF_PROC_RES_OK);
        }
        if (isGroupAlgo(RK_AIQ_ALGO_TYPE_AYNR)) {
            removed_mask &= ~((1ULL << XCAM_MESSAGE_YNR_V3_PROC_RES_OK) |
                              (1ULL << XCAM_MESSAGE_YNR_V22_PROC_RES_OK));
        }
        if (isGroupAlgo(RK_AIQ_ALGO_TYPE_ABLC)) {
            removed_mask &= ~((1ULL << XCAM_MESSAGE_BLC_PROC_RES_OK) |
                              (1ULL << XCAM_MESSAGE_BLC_V32_PROC_RES_OK));
        }
        auto groups = mRkAiqCoreGroupManager->getGroups();
        for (auto& g : groups) {
            auto f = g.second->getDepsFlag();
            f &= removed_mask;
            g.second->setDepsFlag(f);
        }
    }
}
#endif

XCamReturn
RkAiqCore::init(const char* sns_ent_name, const CamCalibDbContext_t* aiqCalib,
                const CamCalibDbV2Context_t* aiqCalibv2)
{
    ENTER_ANALYZER_FUNCTION();

    if (mState != RK_AIQ_CORE_STATE_INVALID) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }


#ifdef RKAIQ_ENABLE_PARSER_V1
    mAlogsComSharedParams.calib = aiqCalib;
#endif
    mAlogsComSharedParams.calibv2 = aiqCalibv2;

    const CalibDb_AlgoSwitch_t *algoSwitch = &aiqCalibv2->sys_cfg->algoSwitch;
    if (algoSwitch->enable && algoSwitch->enable_algos) {
        mCustomEnAlgosMask = 0x0;
        for (uint16_t i = 0; i < algoSwitch->enable_algos_len; i++)
            mCustomEnAlgosMask |= 1ULL << algoSwitch->enable_algos[i];
    }
    LOGI_ANALYZER("mCustomEnAlgosMask: 0x%" PRIx64 "\n", mCustomEnAlgosMask);
    addDefaultAlgos(mAlgosDesArray);
#if RKAIQ_HAVE_ASD_V10
    initCpsl();
#endif
    newAiqParamsPool();
    newAiqGroupAnayzer();
#if RKAIQ_HAVE_PDAF
    newPdafStatsPool();
#endif

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

    if (mState == RK_AIQ_CORE_STATE_STARTED || mState == RK_AIQ_CORE_STATE_RUNNING) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }

#if RKAIQ_HAVE_PDAF
    delPdafStatsPool();
#endif
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

#ifndef RKAIQ_DISABLE_CORETHRD
    mRkAiqCoreTh->triger_start();
    mRkAiqCoreTh->start();
#endif
#if defined(ISP_HW_V20)
    if (mHasPp) {
        mRkAiqCorePpTh->triger_start();
        mRkAiqCorePpTh->start();
    }
#endif
    mRkAiqCoreEvtsTh->triger_start();
    mRkAiqCoreEvtsTh->start();

#if RKAIQ_HAVE_PDAF
    uint64_t deps = mRkAiqCoreGroupManager->getGrpDeps(RK_AIQ_CORE_ANALYZE_AF);
    if (get_pdaf_support()) {
        deps |= 1LL << XCAM_MESSAGE_PDAF_STATS_OK;
    } else {
        deps &= ~(1LL << XCAM_MESSAGE_PDAF_STATS_OK);
    }
    mRkAiqCoreGroupManager->setGrpDeps(RK_AIQ_CORE_ANALYZE_AF, deps);
#endif
    mRkAiqCoreGroupManager->start();

#if defined(RKAIQ_HAVE_THUMBNAILS)
    if (mThumbnailsService.ptr()) {
        mThumbnailsService->Start();
    }
#endif

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

#ifndef RKAIQ_DISABLE_CORETHRD
    mRkAiqCoreTh->triger_stop();
    mRkAiqCoreTh->stop();
#endif

#if defined(ISP_HW_V20)
    if (mHasPp) {
        mRkAiqCorePpTh->triger_stop();
        mRkAiqCorePpTh->stop();
    }
#endif
    mRkAiqCoreEvtsTh->triger_stop();
    mRkAiqCoreEvtsTh->stop();

    mRkAiqCoreGroupManager->stop();
#if defined(RKAIQ_HAVE_THUMBNAILS)
    if (mThumbnailsService.ptr()) {
        mThumbnailsService->Stop();
    }
#endif

    {
        SmartLock locker (ispStatsListMutex);
        mAiqStatsCachedList.clear();
        mAiqStatsOutMap.clear();
        mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_INIT;
        mState = RK_AIQ_CORE_STATE_STOPED;
        firstStatsReceived = false;
        mLastAnalyzedId = 0;
    }
    mIspStatsCond.broadcast ();
    mSafeEnableAlgo = true;
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
    } else
        mAlogsComSharedParams.conf_type &= ~RK_AIQ_ALGO_CONFTYPE_CHANGERES;

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
#if RKAIQ_HAVE_ASD_V10
    CalibDbV2_Cpsl_t* calibv2_cpsl_db =
        (CalibDbV2_Cpsl_t*)CALIBDBV2_GET_MODULE_PTR((void*)(mAlogsComSharedParams.calibv2), cpsl);
#else
    CalibDbV2_Cpsl_t* calibv2_cpsl_db = NULL;
#endif
#if defined(RKAIQ_HAVE_THUMBNAILS)
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
#endif

    mTranslator->setWorkingMode(mode);

#if defined(RKAIQ_HAVE_MULTIISP)

    if (mHwInfo.is_multi_isp_mode) {
        XCAM_ASSERT((sensor_des->isp_acq_width % 32 == 0));// &&
        //(sensor_des->isp_acq_height % 16 == 0));
        uint32_t extended_pixel = mHwInfo.multi_isp_extended_pixel;
        RkAiqResourceTranslatorV3x* translator = static_cast<RkAiqResourceTranslatorV3x*>(mTranslator.ptr());
        translator->SetMultiIspMode(true)
        .SetPicInfo({0, 0, sensor_des->isp_acq_width, sensor_des->isp_acq_height})
        .SetLeftIspRect(
        {0, 0, sensor_des->isp_acq_width / 2 + extended_pixel, sensor_des->isp_acq_height})
        .SetRightIspRect({sensor_des->isp_acq_width / 2 - extended_pixel, 0,
                          sensor_des->isp_acq_width / 2 + extended_pixel,
                          sensor_des->isp_acq_height});
        RkAiqResourceTranslatorV3x::Rectangle f = translator->GetPicInfo();
        RkAiqResourceTranslatorV3x::Rectangle l = translator->GetLeftIspRect();
        RkAiqResourceTranslatorV3x::Rectangle r = translator->GetRightIspRect();
        LOGD_ANALYZER(
            "Set Multi-ISP mode Translator info :"
            " F: { %u, %u, %u, %u }"
            " L: { %u, %u, %u, %u }"
            " R: { %u, %u, %u, %u }",
            f.x, f.y, f.w, f.h, l.x, l.y, l.w, l.h, r.x, r.y, r.w, r.h);
        mAlogsComSharedParams.is_multi_isp_mode = mHwInfo.is_multi_isp_mode;
        mAlogsComSharedParams.multi_isp_extended_pixels = extended_pixel;
    } else {
        if (mIspHwVer == 3)
            static_cast<RkAiqResourceTranslatorV3x*>(mTranslator.ptr())->SetMultiIspMode(false);
    }
#endif

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
        } else if (calibv2_cpsl_db && calibv2_cpsl_db->param.enable) {
            mGrayMode = RK_AIQ_GRAY_MODE_CPSL;
            mAlogsComSharedParams.gray_mode =
                mAlogsComSharedParams.fill_light_on && calibv2_cpsl_db->param.force_gray;
        } else {
            mGrayMode                       = RK_AIQ_GRAY_MODE_OFF;
            mAlogsComSharedParams.gray_mode = false;
        }
    }

    for (auto& algoHdl : mCurIspAlgoHandleList) {
        RkAiqHandle* curHdl = algoHdl.ptr();
        while (curHdl) {
            if (curHdl->getEnable()) {
                /* update user initial params */
                ret = curHdl->updateConfig(true);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d update initial user params failed", curHdl->getAlgoType());
                algoHdl->setReConfig(mState == RK_AIQ_CORE_STATE_STOPED);
                ret = curHdl->prepare();
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed", curHdl->getAlgoType());
            }
            curHdl = curHdl->getNextHdl();
        }
    }


    mAlogsComSharedParams.init = true;
    // run algos without stats to generate
    // initial params
    CalibDb_Aec_ParaV2_t* calibv2_ae_calib =
        (CalibDb_Aec_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(mAlogsComSharedParams.calibv2), ae_calib));

    auto mapIter = mAlogsGroupSharedParamsMap.begin();
    while (mapIter != mAlogsGroupSharedParamsMap.end()) {
        RkAiqAlgosGroupShared_t* &shared = mapIter->second;
        if (!(mAlogsComSharedParams.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES)) {
            shared->reset();
        } else {
            // do not reset all for case of resolution changed
            shared->frameId = 0;
            shared->sof     = 0;
            shared->ispStats = 0;
        }
        SmartPtr<RkAiqFullParams> curParams = mAiqCurParams->data();
        if (curParams.ptr() && curParams->mExposureParams.ptr()) {
            shared->curExp =
                curParams->mExposureParams->data()->exp_tbl[0];
        }
        mapIter++;
    }

    analyzeInternal(RK_AIQ_CORE_ANALYZE_AE);
    analyzeInternal(RK_AIQ_CORE_ANALYZE_ALL);
    freeSharebuf(RK_AIQ_CORE_ANALYZE_GRP1);

    mAlogsComSharedParams.init = false;

    mState = RK_AIQ_CORE_STATE_PREPARED;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

void RkAiqCore::getDummyAlgoRes(int type, uint32_t frame_id) {
    if (type == RK_AIQ_ALGO_TYPE_AE) {
        SmartPtr<BufferProxy> bp = new BufferProxy(new RkAiqAlgoPreResAeIntShared);
        bp->set_sequence(frame_id);
        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AE_PRE_RES_OK, frame_id, bp);
        post_message(msg);

        bp = new BufferProxy(new RkAiqAlgoProcResAeIntShared);
        bp->set_sequence(frame_id);
        msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AE_PROC_RES_OK, frame_id, bp);
        post_message(msg);
    } else if (type == RK_AIQ_ALGO_TYPE_AWB) {
        SmartPtr<BufferProxy> bp = new BufferProxy(new RkAiqAlgoProcResAwbIntShared);
        bp->set_sequence(frame_id);
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AWB_PROC_RES_OK, frame_id, bp);
        post_message(msg);
    } else if (type == RK_AIQ_ALGO_TYPE_ABLC) {
#if ISP_HW_V32
        SmartPtr<BufferProxy> bp = new BufferProxy(new RkAiqAlgoProcResAblcV32IntShared);
#else
        SmartPtr<BufferProxy> bp = new BufferProxy(new RkAiqAlgoProcResAblcIntShared);
#endif
        bp->set_sequence(frame_id);
#if ISP_HW_V32
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_BLC_V32_PROC_RES_OK, frame_id, bp);
#else
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_BLC_PROC_RES_OK, frame_id, bp);
#endif
        post_message(msg);
    } else if (type == RK_AIQ_ALGO_TYPE_ABLC) {
#if ISP_HW_V32
        SmartPtr<BufferProxy> bp = new BufferProxy(new RkAiqAlgoProcResAynrV22IntShared);
#else
        SmartPtr<BufferProxy> bp = new BufferProxy(new RkAiqAlgoProcResAynrV3IntShared);
#endif
        bp->set_sequence(frame_id);
#if ISP_HW_V32
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_YNR_V22_PROC_RES_OK, frame_id, bp);
#else
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_YNR_V3_PROC_RES_OK, frame_id, bp);
#endif
        post_message(msg);
    }
}

SmartPtr<RkAiqFullParamsProxy>
RkAiqCore::analyzeInternal(enum rk_aiq_core_analyze_type_e type)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    SmartPtr<RkAiqFullParamsProxy> aiqParamProxy = NULL;
    if (mAiqParamsPool->has_free_items())
        aiqParamProxy = mAiqParamsPool->get_item();

    if (!aiqParamProxy.ptr()) {
        LOGE_ANALYZER("no free aiq params buffer!");
        return NULL;
    }

    RkAiqFullParams* aiqParams = aiqParamProxy->data().ptr();

    RKAIQCORE_CHECK_RET_NULL(ret, "get params failed");

    std::vector<SmartPtr<RkAiqHandle>>& algo_list = mRkAiqCoreGroupManager->getGroupAlgoList(type);
    SmartPtr<RkAiqFullParams> curParams           = mAiqCurParams->data();
    for (auto& algoHdl : algo_list) {
        RkAiqHandle* curHdl = algoHdl.ptr();
        bool got_buffer     = false;
        while (curHdl) {
            if (curHdl->getEnable()) {
                int type = curHdl->getAlgoType();
                if (!got_buffer) {
                    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)(curHdl->getGroupShared());
                    uint32_t frame_id = -1;
                    if (shared) frame_id = shared->frameId;
                    ret = getAiqParamsBuffer(aiqParams, type, frame_id);
                    if (ret) break;
                    if (!mAlogsComSharedParams.init && isGroupAlgo(type)) {
                        getDummyAlgoRes(type, frame_id);
                    }
                    got_buffer = true;
                }
                if (mAlogsComSharedParams.init || !isGroupAlgo(type)) {
                    ret = curHdl->updateConfig(true);
                    ret = curHdl->preProcess();
                    ret = curHdl->processing();
                    ret = algoHdl->postProcess();
                    curHdl->genIspResult(aiqParams, curParams.ptr());
                    if (ret) break;
                }
            }
            curHdl = curHdl->getNextHdl();
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return aiqParamProxy;
}

XCamReturn RkAiqCore::freeSharebuf(uint64_t grpId) {
    RkAiqAlgosGroupShared_t* shared = nullptr;
    uint64_t grpMask = grpId2GrpMask(grpId);
    if (!getGroupSharedParams(grpMask, shared)) {
        if (shared) {
            if (shared->aecStatsBuf) {
                shared->aecStatsBuf->unref(shared->aecStatsBuf);
                shared->aecStatsBuf = nullptr;
            }
            if (shared->awbStatsBuf) {
                shared->awbStatsBuf->unref(shared->awbStatsBuf);
                shared->awbStatsBuf = nullptr;
            }
            if (shared->afStatsBuf) {
                shared->afStatsBuf->unref(shared->afStatsBuf);
                shared->afStatsBuf = nullptr;
            }
            if (shared->ispStats) {
                shared->ispStats->unref(shared->ispStats);
                shared->ispStats = nullptr;
            }
            if (shared->tx) {
                shared->tx->unref(shared->tx);
                shared->tx = nullptr;
            }
            if (shared->sp) {
                shared->sp->unref(shared->sp);
                shared->sp = nullptr;
            }
            if (shared->ispGain) {
                shared->ispGain->unref(shared->ispGain);
                shared->ispGain = nullptr;
            }
            if (shared->kgGain) {
                shared->kgGain->unref(shared->kgGain);
                shared->kgGain = nullptr;
            }
            if (shared->wrGain) {
                shared->wrGain->unref(shared->wrGain);
                shared->wrGain = nullptr;
            }
            if (shared->orbStats) {
                shared->orbStats->unref(shared->orbStats);
                shared->orbStats = nullptr;
            }
            if (shared->nrImg) {
                shared->nrImg->unref(shared->nrImg);
                shared->nrImg = nullptr;
            }
            if (shared->pdafStatsBuf) {
                shared->pdafStatsBuf->unref(shared->pdafStatsBuf);
                shared->pdafStatsBuf = nullptr;
            }
            if (shared->res_comb.ae_pre_res) {
                shared->res_comb.ae_pre_res->unref(shared->res_comb.ae_pre_res);
                shared->res_comb.ae_pre_res = nullptr;
            }
            if (shared->res_comb.ae_proc_res) {
                shared->res_comb.ae_proc_res->unref(shared->res_comb.ae_proc_res);
                shared->res_comb.ae_proc_res = nullptr;
            }
            if (shared->res_comb.awb_proc_res) {
                shared->res_comb.awb_proc_res->unref(shared->res_comb.awb_proc_res);
                shared->res_comb.awb_proc_res = nullptr;
            }

            return XCAM_RETURN_NO_ERROR;
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::getAiqParamsBuffer(RkAiqFullParams* aiqParams, int type, uint32_t frame_id)
{
#define NEW_PARAMS_BUFFER(lc, BC)                                                              \
    if (mAiqIsp##lc##ParamsPool->has_free_items()) {                                           \
        aiqParams->m##lc##Params                        = mAiqIsp##lc##ParamsPool->get_item(); \
        aiqParams->m##lc##Params->data()->frame_id      = frame_id;                            \
    } else {                                                                                   \
        LOGE_ANALYZER("no free %s buffer!", #BC);                                              \
        return XCAM_RETURN_ERROR_MEM;                                                          \
    }

#define NEW_PARAMS_BUFFER_WITH_V(lc, BC, v)                                         \
    if (mAiqIsp##lc##V##v##ParamsPool->has_free_items()) {                          \
        aiqParams->m##lc##V##v##Params = mAiqIsp##lc##V##v##ParamsPool->get_item(); \
        aiqParams->m##lc##V##v##Params->data()->frame_id = frame_id;                \
    } else {                                                                        \
        LOGE_ANALYZER("no free %s buffer!", #BC);                                   \
        return XCAM_RETURN_ERROR_MEM;                                               \
    }

    switch (type) {
    case RK_AIQ_ALGO_TYPE_AE:
        if (mAiqExpParamsPool->has_free_items()) {
            aiqParams->mExposureParams = mAiqExpParamsPool->get_item();
            aiqParams->mExposureParams->data()->frame_id = frame_id;
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
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Awb, awb, 32);
#elif defined(ISP_HW_V30)
        NEW_PARAMS_BUFFER_WITH_V(Awb, awb, 3x);
#elif defined(ISP_HW_V21)
        NEW_PARAMS_BUFFER_WITH_V(Awb, awb, 21);
#else
        NEW_PARAMS_BUFFER(Awb, awb);
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(AwbGain, awb_gain, 32);
#else
        NEW_PARAMS_BUFFER(AwbGain, awb_gain);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AF:
        if (mAiqFocusParamsPool->has_free_items()) {
            aiqParams->mFocusParams = mAiqFocusParamsPool->get_item();
        } else {
            LOGE_ANALYZER("no free focus params buffer!");
            return XCAM_RETURN_ERROR_MEM;
        }
#if defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Af, af, 32Lite);
#elif defined(ISP_HW_V32)
        NEW_PARAMS_BUFFER_WITH_V(Af, af, 32);
#elif defined(ISP_HW_V30)
        NEW_PARAMS_BUFFER_WITH_V(Af, af, 3x);
#else
        NEW_PARAMS_BUFFER(Af, af);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ABLC:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Blc, blc, 32);
#elif defined(ISP_HW_V30) || defined(ISP_HW_V21)
        NEW_PARAMS_BUFFER_WITH_V(Blc, blc, 21);
#else
        NEW_PARAMS_BUFFER(Blc, blc);
#endif
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
#if RKAIQ_HAVE_TMO_V1
    case RK_AIQ_ALGO_TYPE_ATMO:
        NEW_PARAMS_BUFFER(Tmo, tmo);
        break;
#endif
#endif
    case RK_AIQ_ALGO_TYPE_ALSC:
        NEW_PARAMS_BUFFER(Lsc, lsc);
        break;
    case RK_AIQ_ALGO_TYPE_AGIC:
        NEW_PARAMS_BUFFER(Gic, gic);
        break;
    case RK_AIQ_ALGO_TYPE_ADEBAYER:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Debayer, debayer, 32);
#else
        NEW_PARAMS_BUFFER(Debayer, debayer);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ACCM:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Ccm, ccm, 32);
#else
        NEW_PARAMS_BUFFER(Ccm, ccm);
#endif
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
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Ldch, ldch, 32);
#else
        NEW_PARAMS_BUFFER(Ldch, ldch);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ACSM:
        NEW_PARAMS_BUFFER(Csm, csm);
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
        NEW_PARAMS_BUFFER(Drc, drc);
        break;
    case RK_AIQ_ALGO_TYPE_ADEGAMMA:
        NEW_PARAMS_BUFFER(Adegamma, adegamma);
        break;
    case RK_AIQ_ALGO_TYPE_ARAWNR:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Baynr, baynr, 32);
#elif defined(ISP_HW_V30)
        NEW_PARAMS_BUFFER_WITH_V(Baynr, baynr, 3x);
#elif defined(ISP_HW_V21)
        NEW_PARAMS_BUFFER_WITH_V(Baynr, baynr, 21);
#else
        NEW_PARAMS_BUFFER(Rawnr, rawnr);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AMFNR:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Tnr, tnr, 32);
#elif defined(ISP_HW_V30)
        NEW_PARAMS_BUFFER_WITH_V(Tnr, tnr, 3x);
#else
        NEW_PARAMS_BUFFER(Tnr, tnr);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_AYNR:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Ynr, ynr, 32);
#elif defined(ISP_HW_V30)
        NEW_PARAMS_BUFFER_WITH_V(Ynr, ynr, 3x);
#elif defined(ISP_HW_V21)
        NEW_PARAMS_BUFFER_WITH_V(Ynr, ynr, 21);
#else
        NEW_PARAMS_BUFFER(Ynr, ynr);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ACNR:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Cnr, cnr, 32);
#elif defined(ISP_HW_V30)
        NEW_PARAMS_BUFFER_WITH_V(Cnr, cnr, 3x);
#elif defined(ISP_HW_V21)
        NEW_PARAMS_BUFFER_WITH_V(Cnr, cnr, 21);
#else
        NEW_PARAMS_BUFFER(Uvnr, uvnr);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ASHARP:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Sharp, sharp, 32);
#elif defined(ISP_HW_V30)
        NEW_PARAMS_BUFFER_WITH_V(Sharpen, sharpen, 3x);
#elif defined(ISP_HW_V21)
        NEW_PARAMS_BUFFER_WITH_V(Sharpen, sharpen, 21);
#else
        NEW_PARAMS_BUFFER(Sharpen, sharpen);
        NEW_PARAMS_BUFFER(Edgeflt, edgeflt);
#endif
        break;
#if RKAIQ_HAVE_ORB_V1
    case RK_AIQ_ALGO_TYPE_AORB:
        NEW_PARAMS_BUFFER(Orb, orb);
        break;
#endif
    case RK_AIQ_ALGO_TYPE_AFEC:
    case RK_AIQ_ALGO_TYPE_AEIS:
        NEW_PARAMS_BUFFER(Fec, fec);
        break;
#if defined(ISP_HW_V20)
    case RK_AIQ_ALGO_TYPE_ANR:
        NEW_PARAMS_BUFFER(Rawnr, rawnr);
        NEW_PARAMS_BUFFER(Tnr, tnr);
        NEW_PARAMS_BUFFER(Ynr, ynr);
        NEW_PARAMS_BUFFER(Uvnr, uvnr);
        NEW_PARAMS_BUFFER(Gain, gain);
        NEW_PARAMS_BUFFER(Motion, motion);
        break;
#endif
#if RKAIQ_HAVE_AMD_V1
    case RK_AIQ_ALGO_TYPE_AMD:
        NEW_PARAMS_BUFFER(Md, md);
        break;
#endif
    case RK_AIQ_ALGO_TYPE_AGAIN:
#if defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        NEW_PARAMS_BUFFER_WITH_V(Gain, Gain, 3x);
#endif
        break;
    case RK_AIQ_ALGO_TYPE_ACAC:
#if RKAIQ_HAVE_CAC_V11
        NEW_PARAMS_BUFFER_WITH_V(Cac, cac, 32);
#elif RKAIQ_HAVE_CAC_V10 || RKAIQ_HAVE_CAC_V03
        NEW_PARAMS_BUFFER_WITH_V(Cac, cac, 3x);
#endif
        break;
    default:
        break;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::pushStats(SmartPtr<VideoBuffer> &buffer)
{
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(buffer.ptr());
#ifndef RKAIQ_DISABLE_CORETHRD
    mRkAiqCoreTh->push_stats(buffer);
#else
    auto ret = analyze (buffer);
#endif
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

RkAiqHandle*
RkAiqCore::getAiqAlgoHandle(const int algo_type)
{
    // get defalut algo handle(id == 0)
    SmartPtr<RkAiqHandle>* handlePtr = getCurAlgoTypeHandle(algo_type);
    if (handlePtr == nullptr) {
        return NULL;
    }

    return (*handlePtr).ptr();
}

SmartPtr<RkAiqHandle>*
RkAiqCore::getCurAlgoTypeHandle(int algo_type)
{
    // get defalut algo handle(id == 0)
    if (mCurAlgoHandleMaps.find(algo_type) != mCurAlgoHandleMaps.end())
        return &mCurAlgoHandleMaps.at(algo_type);

    LOGE("can't find algo handle %d", algo_type);
    return NULL;
}

std::map<int, SmartPtr<RkAiqHandle>>*
                                  RkAiqCore::getAlgoTypeHandleMap(int algo_type)
{
    if (mAlgoHandleMaps.find(algo_type) != mAlgoHandleMaps.end())
        return &mAlgoHandleMaps.at(algo_type);

    LOGE("can't find algo map %d", algo_type);
    return NULL;
}

// add rk default handlers(id == 0), can't be removed
void
RkAiqCore::addDefaultAlgos(const struct RkAiqAlgoDesCommExt* algoDes)
{
    map<int, SmartPtr<RkAiqHandle>> algoMap;
    for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
#ifdef RKAIQ_ENABLE_PARSER_V1
        mAlogsComSharedParams.ctxCfigs[i].calib =
            const_cast<CamCalibDbContext_t*>(mAlogsComSharedParams.calib);
#endif
        mAlogsComSharedParams.ctxCfigs[i].calibv2 =
            const_cast<CamCalibDbV2Context_t*>(mAlogsComSharedParams.calibv2);
        mAlogsComSharedParams.ctxCfigs[i].isp_hw_version = mIspHwVer;
    }

    std::vector<uint32_t> group_list;
    for (size_t i = 0; algoDes[i].des != NULL; i++) {
        int algo_type = algoDes[i].des->type;
        // enable only the specified algorithm modules
        if (!((1ULL << algo_type) & mCustomEnAlgosMask))
            continue;
        int64_t grpMask = 1ULL << algoDes[i].group;
#ifdef RKAIQ_ENABLE_PARSER_V1
        mAlogsComSharedParams.ctxCfigs[algo_type].calib =
            const_cast<CamCalibDbContext_t*>(mAlogsComSharedParams.calib);
#endif
        mAlogsComSharedParams.ctxCfigs[algo_type].calibv2 =
            const_cast<CamCalibDbV2Context_t*>(mAlogsComSharedParams.calibv2);
        mAlogsComSharedParams.ctxCfigs[algo_type].module_hw_version = algoDes[i].module_hw_ver;
        mAlgoTypeToGrpMaskMap[algo_type] = grpMask;
        bool isExist = false;
        for(auto it = group_list.begin(); it != group_list.end(); it++) {
            if (*it == algoDes[i].group)
                isExist = true;
        }
        if (!isExist) {
            group_list.push_back(algoDes[i].group);
            auto groupId = algoDes[i].group;
            mAlogsGroupSharedParamsMap[grpMask] = new RkAiqAlgosGroupShared_t;
            mAlogsGroupSharedParamsMap[grpMask]->reset();
            mAlogsGroupSharedParamsMap[grpMask]->groupId = algoDes[i].group;
            mAlogsGroupSharedParamsMap[grpMask]->frameId = 0;
            mAlogsGroupSharedParamsMap[grpMask]->ispStats = NULL;

        }
        algoMap[0] = newAlgoHandle(algoDes[i].des, algoDes[i].algo_ver, algoDes[i].handle_ver);
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
RkAiqCore::newAlgoHandle(RkAiqAlgoDesComm* algo, int hw_ver, int handle_ver)
{
    std::string className = "RkAiq";
    className.append(AlgoTypeToString(algo->type));
    if (handle_ver)
        className.append("V" + std::to_string(handle_ver));
    className.append("HandleInt");
    LOGD_ANALYZER("Creating algorithm: %s, version: %d", className.c_str(), hw_ver);
    return SmartPtr<RkAiqHandle>(RkAiqHandleFactory::createInstance(className, algo, this));
}

// register custom algos
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

    SmartPtr<RkAiqHandle> new_hdl;
    if (algo.type == RK_AIQ_ALGO_TYPE_AE)
        new_hdl = new RkAiqCustomAeHandle(&algo, this);
#if RKAIQ_HAVE_AWB_V21 ||RKAIQ_HAVE_AWB_V32
    else if (algo.type == RK_AIQ_ALGO_TYPE_AWB)
        new_hdl = new RkAiqCustomAwbHandle(&algo, this);
#endif
    else {
        LOGE_ANALYZER("not supported custom algo type: %d ", algo.type);
        return XCAM_RETURN_ERROR_FAILED;
    }
    new_hdl->setEnable(false);
    new_hdl->setGroupId((*algo_map)[0]->getGroupId());
    new_hdl->setGroupShared((*algo_map)[0]->getGroupShared());
    rit->second->setNextHdl(new_hdl.ptr());
    new_hdl->setParentHdl((*algo_map)[0].ptr());

    (*algo_map)[algo.id] = new_hdl;

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

std::bitset<RK_AIQ_ALGO_TYPE_MAX> RkAiqCore::getReqAlgoResMask(int algoType) {
    std::bitset<RK_AIQ_ALGO_TYPE_MAX> tmp{};
    switch (algoType) {
        case RK_AIQ_ALGO_TYPE_AE:
            tmp[RESULT_TYPE_EXPOSURE_PARAM] = 1;
            tmp[RESULT_TYPE_AEC_PARAM]      = 1;
            tmp[RESULT_TYPE_HIST_PARAM]     = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AWB:
            tmp[RESULT_TYPE_AWB_PARAM]     = 1;
            tmp[RESULT_TYPE_AWBGAIN_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AF:
            tmp[RESULT_TYPE_AF_PARAM]    = 1;
            tmp[RESULT_TYPE_FOCUS_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ADPCC:
            tmp[RESULT_TYPE_DPCC_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AMERGE:
            tmp[RESULT_TYPE_MERGE_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ATMO:
            tmp[RESULT_TYPE_TMO_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ACCM:
            tmp[RESULT_TYPE_CCM_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ALSC:
            tmp[RESULT_TYPE_LSC_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ABLC:
            tmp[RESULT_TYPE_BLC_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ARAWNR:
            tmp[RESULT_TYPE_RAWNR_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AGIC:
            tmp[RESULT_TYPE_GIC_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ADEBAYER:
            tmp[RESULT_TYPE_DEBAYER_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ALDCH:
            tmp[RESULT_TYPE_LDCH_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_A3DLUT:
            tmp[RESULT_TYPE_LUT3D_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ADHAZ:
            tmp[RESULT_TYPE_DEHAZE_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AGAMMA:
            tmp[RESULT_TYPE_AGAMMA_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ADEGAMMA:
            tmp[RESULT_TYPE_ADEGAMMA_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AWDR:
            tmp[RESULT_TYPE_WDR_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AGAIN:
            tmp[RESULT_TYPE_GAIN_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ACP:
            tmp[RESULT_TYPE_CP_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ACSM:
            tmp[RESULT_TYPE_CSM_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AIE:
            tmp[RESULT_TYPE_IE_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AMD:
            tmp[RESULT_TYPE_MOTION_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AMFNR:
            tmp[RESULT_TYPE_TNR_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AYNR:
            tmp[RESULT_TYPE_YNR_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ACNR:
            tmp[RESULT_TYPE_UVNR_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ASHARP:
            tmp[RESULT_TYPE_SHARPEN_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_AFEC:
        case RK_AIQ_ALGO_TYPE_AEIS:
            tmp[RESULT_TYPE_FEC_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ADRC:
            tmp[RESULT_TYPE_DRC_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ACAC:
            tmp[RESULT_TYPE_CAC_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ACGC:
            tmp[RESULT_TYPE_CGC_PARAM] = 1;
            break;
        case RK_AIQ_ALGO_TYPE_ASD:
            tmp[RESULT_TYPE_CPSL_PARAM] = 1;
            break;
        default:
            break;
    }

    return tmp;
}

void RkAiqCore::setReqAlgoResMask(int algoType, bool req) {
    auto tmp = getReqAlgoResMask(algoType);
    if (req) {
        mAllReqAlgoResMask |= tmp;
    } else {
        mAllReqAlgoResMask &= ~tmp;
    }

    LOGI_ANALYZER("mAllReqAlgoResMask : %llx", mAllReqAlgoResMask);
}

XCamReturn
RkAiqCore::enableAlgo(int algoType, int id, bool enable)
{
    ENTER_ANALYZER_FUNCTION();
    // get default algotype handle, id should be 0
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

    SmartLock locker (mApiMutex);
    while (mSafeEnableAlgo != true)
        mApiMutexCond.wait(mApiMutex);

    LOGI_ANALYZER("set algo type_id <%d,%d> to %d", algoType, id, enable);

    it->second->setEnable(enable);
    /* WARNING:
     * Be careful when use SmartPtr<RkAiqxxxHandle> = SmartPtr<RkAiqHandle>
     * if RkAiqxxxHandle is derived from multiple RkAiqHandle,
     * the ptr of RkAiqxxxHandle and RkAiqHandle IS NOT the same
     * (RkAiqHandle ptr = RkAiqxxxHandle ptr + offset), but seams like
     * SmartPtr do not deal with this correctly.
     */

    if (enable) {
        if (mState >= RK_AIQ_CORE_STATE_PREPARED)
            it->second->prepare();
    }

    int enable_cnt = 0;
    RkAiqHandle* curHdl = (*cur_algo_hdl).ptr();

    while (curHdl) {
        if (curHdl->getEnable()) {
            enable_cnt++;
        }
        curHdl = curHdl->getNextHdl();
    }

    setReqAlgoResMask(algoType, enable_cnt > 0 ? true : false);

    curHdl = (*cur_algo_hdl).ptr();

    while (curHdl) {
        curHdl->setMulRun(enable_cnt > 1 ? true : false);
        curHdl = curHdl->getNextHdl();
    }

    LOGI_ANALYZER("algo type %d enabled count :%d", algoType, enable_cnt);

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

    SmartLock locker (mApiMutex);
    while (mState == RK_AIQ_CORE_STATE_RUNNING && mSafeEnableAlgo != true)
        mApiMutexCond.wait(mApiMutex);

    RkAiqHandle* rmHdl = it->second.ptr();
    RkAiqHandle* curHdl = (*cur_algo_hdl).ptr();
    while (curHdl) {
        RkAiqHandle* nextHdl = curHdl->getNextHdl();
        if (nextHdl == rmHdl) {
            curHdl->setNextHdl(nextHdl->getNextHdl());
            break;
        }
        curHdl = nextHdl;
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

// get the last enabled ctx of same type
RkAiqAlgoContext*
RkAiqCore::getEnabledAxlibCtx(const int algo_type)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE ||
            algo_type >= RK_AIQ_ALGO_TYPE_MAX)
        return NULL;

    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algo_type);
    std::map<int, SmartPtr<RkAiqHandle>>::reverse_iterator rit = algo_map->rbegin();
    if (rit !=  algo_map->rend() && rit->second->getEnable())
        return rit->second->getAlgoCtx();
    else
        return NULL;
}


RkAiqAlgoContext*
RkAiqCore::getAxlibCtx(const int algo_type, const int lib_id)
{
    if (algo_type <= RK_AIQ_ALGO_TYPE_NONE ||
            algo_type >= RK_AIQ_ALGO_TYPE_MAX)
        return NULL;

    std::map<int, SmartPtr<RkAiqHandle>>* algo_map = getAlgoTypeHandleMap(algo_type);

    std::map<int, SmartPtr<RkAiqHandle>>::iterator it = algo_map->find(lib_id);

    if (it != algo_map->end()) {
        return it->second->getAlgoCtx();
    }

    EXIT_ANALYZER_FUNCTION();

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
        to->frame_id  = aecStat->data()->frame_id;
    }
    if (mIspHwVer == 4) {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        to->awb_hw_ver = 4;
        if (awbStat.ptr()) {
            to->awb_stats_v32 = awbStat->data()->awb_stats_v32;
        }
#endif
    } else if (mIspHwVer == 3) {
#if ISP_HW_V30
        to->awb_hw_ver = 3;
        if (awbStat.ptr()) {
            memcpy(to->awb_stats_v3x.light, awbStat->data()->awb_stats_v3x.light,
                   sizeof(to->awb_stats_v3x.light));
#ifdef ISP_HW_V30
            memcpy(to->awb_stats_v3x.WpNo2, awbStat->data()->awb_stats_v3x.WpNo2,
                   sizeof(to->awb_stats_v3x.WpNo2));
#endif
            memcpy(to->awb_stats_v3x.blockResult, awbStat->data()->awb_stats_v3x.blockResult,
                   sizeof(to->awb_stats_v3x.blockResult));
#ifdef ISP_HW_V30
            memcpy(to->awb_stats_v3x.multiwindowLightResult,
                   awbStat->data()->awb_stats_v3x.multiwindowLightResult,
                   sizeof(to->awb_stats_v3x.multiwindowLightResult));
            memcpy(to->awb_stats_v3x.excWpRangeResult,
                   awbStat->data()->awb_stats_v3x.excWpRangeResult,
                   sizeof(to->awb_stats_v3x.excWpRangeResult));
#endif
            memcpy(to->awb_stats_v3x.WpNoHist, awbStat->data()->awb_stats_v3x.WpNoHist,
                   sizeof(to->awb_stats_v3x.WpNoHist));
        }
#endif
    } else if (mIspHwVer == 1) {
#if ISP_HW_V21
        to->awb_hw_ver = 1;
        if (awbStat.ptr()) {
            memcpy(to->awb_stats_v21.light, awbStat->data()->awb_stats_v201.light,
                   sizeof(to->awb_stats_v21.light));
            memcpy(to->awb_stats_v21.blockResult, awbStat->data()->awb_stats_v201.blockResult,
                   sizeof(to->awb_stats_v21.blockResult));
            memcpy(to->awb_stats_v21.WpNoHist, awbStat->data()->awb_stats_v201.WpNoHist,
                   sizeof(to->awb_stats_v21.WpNoHist));
        }
#endif
    } else {
#if ISP_HW_V20
        to->awb_hw_ver = 0;
        if (awbStat.ptr()) to->awb_stats_v200 = awbStat->data()->awb_stats;
#endif
    }
    if (mIspHwVer == 4) {
#if RKAIQ_HAVE_AF_V32_LITE || RKAIQ_ONLY_AF_STATS_V32_LITE
        to->af_hw_ver = RKAIQ_AF_HW_V32_LITE;
        if (afStat.ptr()) to->af_stats_v3x = afStat->data()->af_stats_v3x;
#endif
#if RKAIQ_HAVE_AF_V31 || RKAIQ_ONLY_AF_STATS_V31
        to->af_hw_ver = RKAIQ_AF_HW_V31;
        if (afStat.ptr()) to->af_stats_v3x = afStat->data()->af_stats_v3x;
#endif
    } else if (mIspHwVer == 3) {
#if RKAIQ_HAVE_AF_V30 || RKAIQ_ONLY_AF_STATS_V30
        to->af_hw_ver = RKAIQ_AF_HW_V30;
        if (afStat.ptr()) to->af_stats_v3x = afStat->data()->af_stats_v3x;
#endif
    } else {
#if RKAIQ_HAVE_AF_V20 || RKAIQ_HAVE_AF_V21 || RKAIQ_ONLY_AF_STATS_V20
        to->af_hw_ver = RKAIQ_AF_HW_V20;
        if (afStat.ptr()) to->af_stats = afStat->data()->af_stats;
#endif
    }
}

void
RkAiqCore::cacheIspStatsToList(SmartPtr<RkAiqAecStatsProxy>& aecStat,
                               SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                               SmartPtr<RkAiqAfStatsProxy>& afStat)
{
    SmartLock locker (ispStatsListMutex);
    SmartPtr<RkAiqStatsProxy> stats = NULL;
    if (!mAiqStatsPool.ptr()) return;

    if (mAiqStatsPool->has_free_items()) {
        stats = mAiqStatsPool->get_item();
    } else {
        if (mAiqStatsCachedList.empty()) {
            LOGW_ANALYZER("no free or cached stats, user may hold all stats buf !");
            return;
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
    if (!mAiqStatsPool.ptr())
        mAiqStatsPool = new RkAiqStatsPool("RkAiqStatsPool", 2);

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
    if (!mAiqStatsPool.ptr())
        mAiqStatsPool = new RkAiqStatsPool("RkAiqStatsPool", 2);

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

    if (__builtin_expect(buffer->_buf_type == ISP_POLL_3A_STATS, true)) {
        SmartPtr<RkAiqAecStatsProxy> aecStat = NULL;
        SmartPtr<RkAiqAwbStatsProxy> awbStat = NULL;
        SmartPtr<RkAiqAfStatsProxy> afStat = NULL;
        SmartPtr<RkAiqAtmoStatsProxy> tmoStat = NULL;
        SmartPtr<RkAiqAdehazeStatsProxy> dehazeStat = NULL;
        if (mTranslator->getParams(buffer))
            return ret;
        handleAecStats(buffer, aecStat);
        handleAwbStats(buffer, awbStat);
        handleAfStats(buffer, afStat);
#if RKAIQ_HAVE_TMO_V1
        handleAtmoStats(buffer, tmoStat);
#endif
        handleAdehazeStats(buffer, dehazeStat);
#if defined(ISP_HW_V20)
        handleIspStats(buffer, aecStat, awbStat, afStat, tmoStat, dehazeStat);
#endif
        mTranslator->releaseParams();
        cacheIspStatsToList(aecStat, awbStat, afStat);
    } else {
        int type = -1;
        switch (buffer->_buf_type) {
            case ISPP_POLL_NR_STATS: {
                handleOrbStats(buffer);
                break;
            }
            case ISP_POLL_SP:
                type = XCAM_MESSAGE_ISP_POLL_SP_OK;
                break;
            case ISP_GAIN:
                type = XCAM_MESSAGE_ISP_GAIN_OK;
                break;
            case ISPP_GAIN_KG:
                type = XCAM_MESSAGE_ISPP_GAIN_KG_OK;
                break;
            case ISPP_GAIN_WR:
                type = XCAM_MESSAGE_ISPP_GAIN_WR_OK;
                break;
            case ISP_NR_IMG:
                type = XCAM_MESSAGE_NR_IMG_OK;
                break;
            case ISP_POLL_TX: {
                type = XCAM_MESSAGE_ISP_POLL_TX_OK;
                break;
            }
#if RKAIQ_HAVE_PDAF
            case ISP_POLL_PDAF_STATS: {
                handlePdafStats(buffer);
                break;
            }
#endif
            case ISPP_POLL_TNR_STATS:
            case ISPP_POLL_FEC_PARAMS:
            case ISPP_POLL_TNR_PARAMS:
            case ISPP_POLL_NR_PARAMS:
            case ISP_POLL_LUMA:
            case ISP_POLL_PARAMS:
            case ISP_POLL_SOF:
            case ISP_POLL_RX:
            case ISP_POLL_ISPSTREAMSYNC:
            case VICAP_STREAM_ON_EVT:
                LOGW_ANALYZER("buffer type: 0x%x is not used!", buffer->_buf_type);
                assert(false);
                break;
            default:
                LOGW_ANALYZER("don't know buffer type: 0x%x!", buffer->_buf_type);
                assert(false);
                break;
        }
        if (type != -1) {
            SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(static_cast<XCamMessageType>(type),
                                                              buffer->get_sequence(), buffer);
            post_message(msg);
        }
    }

    return ret;
}

XCamReturn
RkAiqCore::events_analyze(const SmartPtr<ispHwEvt_t> &evts)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqExpParamsProxy> preExpParams = nullptr;
    SmartPtr<RkAiqExpParamsProxy> curExpParams = nullptr;
    SmartPtr<RkAiqExpParamsProxy> nxtExpParams = nullptr;

    const SmartPtr<Isp20Evt> isp20Evts =
        evts.dynamic_cast_ptr<Isp20Evt>();
    uint32_t sequence = isp20Evts->sequence;
    if (sequence == (uint32_t)(-1))
        return ret;

    uint32_t id = 0, maxId = 0;
    if (sequence > 0)
        id = mLastAnalyzedId + 1 > sequence ? mLastAnalyzedId + 1 : sequence;
    maxId = sequence == 0 ? 0 : sequence + isp20Evts->expDelay - 1;

    LOGD_ANALYZER("camId:%d, sequence(%d), expDelay(%d), id(%d), maxId(%d)",
                  mAlogsComSharedParams.mCamPhyId,
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
        if (isp20Evts->getExpInfoParams(nxtExpParams, id + 1) < 0) {
            LOGE_ANALYZER("id(%d) get exp failed!", id + 1);
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
        sofInfo->data()->nxtExp = nxtExpParams;
        sofInfo->data()->sof = isp20Evts->getSofTimeStamp();


        sofInfo->setId(id);
        sofInfo->setType(RK_AIQ_SHARED_TYPE_SOF_INFO);

        int64_t sofTime = isp20Evts->getSofTimeStamp() / 1000LL;
        if (mSofTime != 0LL)
            mFrmInterval = sofTime - mSofTime;
        mSofTime = sofTime;

        SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_SOF_INFO_OK, id, sofInfo);
        post_message(msg);

        mLastAnalyzedId = id;
        id++;

        LOGV_ANALYZER(">>> Framenum=%d, id=%d, Cur sgain=%f,stime=%f,mgain=%f,mtime=%f,lgain=%f,ltime=%f",
                      isp20Evts->sequence, id, curExpParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                      curExpParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                      curExpParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                      curExpParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                      curExpParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                      curExpParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time);
        LOGV_ANALYZER(">>> Framenum=%d, id=%d, nxt sgain=%f,stime=%f,mgain=%f,mtime=%f,lgain=%f,ltime=%f",
                      isp20Evts->sequence, id, nxtExpParams->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                      nxtExpParams->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                      nxtExpParams->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                      nxtExpParams->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                      nxtExpParams->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                      nxtExpParams->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time);

        LOGV_ANALYZER("analyze the id(%d), sequence(%d), mLastAnalyzedId(%d)",
                      id, sequence, mLastAnalyzedId);
    }

    return ret;
}

XCamReturn
RkAiqCore::prepare(enum rk_aiq_core_analyze_type_e type)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_BYPASS;
    XCamReturn ret2 = XCAM_RETURN_BYPASS;

    std::vector<SmartPtr<RkAiqHandle>>& algo_list =
        mRkAiqCoreGroupManager->getGroupAlgoList(type);

    for (auto& algoHdl : algo_list) {
        RkAiqHandle* curHdl = algoHdl.ptr();
        while (curHdl) {
            if (curHdl->getEnable()) {
                ret2 = curHdl->updateConfig(true);
                RKAIQCORE_CHECK_BYPASS(ret2, "algoHdl %d updateConfig failed", curHdl->getAlgoType());
                ret2 = curHdl->prepare();
                RKAIQCORE_CHECK_BYPASS(ret2, "algoHdl %d processing failed", curHdl->getAlgoType());
                if (ret2 == XCAM_RETURN_NO_ERROR) {
                    ret = XCAM_RETURN_NO_ERROR;
                }
            }
            curHdl = curHdl->getNextHdl();
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
    if (mTranslator.ptr())
        mTranslator->setModuleRot(mHwInfo.module_rotation);
    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_ASD_V10
XCamReturn
RkAiqCore::setCpsLtCfg(rk_aiq_cpsl_cfg_t &cfg)
{
    ENTER_ANALYZER_FUNCTION();
    if (mState < RK_AIQ_CORE_STATE_INITED) {
        LOGE_ASD("should call afer init");
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
    LOGD_ASD("set cpsl: mode %d", cfg.mode);
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

    LOGD_ASD("cpsl cap: light_src_num %d, led_step %f, ir_step %f",
             cap.lght_src_num, cap.strength_led.step, cap.strength_ir.step);

    EXIT_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::genCpslResult(RkAiqFullParams* params, RkAiqAlgoPreResAsd* asd_pre_rk)
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

    RkAiqAlgosGroupShared_t* shared = nullptr;
    int groupId = getGroupId(RK_AIQ_ALGO_TYPE_ASD);
    if (groupId >= 0) {
        if (getGroupSharedParams(groupId, shared) == XCAM_RETURN_NO_ERROR) {
            if (mAlogsComSharedParams.init)
                cpsl_param->frame_id = 0;
            else
                cpsl_param->frame_id = shared->frameId;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif

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

#ifdef RKAIQ_ENABLE_PARSER_V1
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
#endif

XCamReturn
RkAiqCore::setCalib(const CamCalibDbV2Context_t* aiqCalib)
{
    ENTER_ANALYZER_FUNCTION();

#if 0
    if (mState != RK_AIQ_CORE_STATE_STOPED) {
        LOGE_ANALYZER("wrong state %d\n", mState);
        return XCAM_RETURN_ERROR_ANALYZER;
    }
#endif
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
    mAlogsComSharedParams.calibv2 = aiqCalib;
    mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    std::for_each(std::begin(*change_name_list), std::end(*change_name_list),
    [this](const std::string & name) {
        if (!name.compare(0, 4, "cpsl", 0, 4)) {
#if RKAIQ_HAVE_ASD_V10
            initCpsl();
#endif
        } else if (!name.compare(0, 11, "colorAsGrey", 0, 11)) {
            CalibDbV2_ColorAsGrey_t* colorAsGrey =
                (CalibDbV2_ColorAsGrey_t*)CALIBDBV2_GET_MODULE_PTR(
                    (void*)(mAlogsComSharedParams.calibv2), colorAsGrey);
#if RKAIQ_HAVE_ASD_V10
            CalibDbV2_Cpsl_t* calibv2_cpsl_db = (CalibDbV2_Cpsl_t*)CALIBDBV2_GET_MODULE_PTR(
                (void*)(mAlogsComSharedParams.calibv2), cpsl);
#else
            CalibDbV2_Cpsl_t* calibv2_cpsl_db = NULL;
#endif

            if (colorAsGrey->param.enable) {
                mGrayMode                       = RK_AIQ_GRAY_MODE_ON;
                mAlogsComSharedParams.gray_mode = true;
            } else if (calibv2_cpsl_db && calibv2_cpsl_db->param.enable) {
                mGrayMode = RK_AIQ_GRAY_MODE_CPSL;
                mAlogsComSharedParams.gray_mode =
                    mAlogsComSharedParams.fill_light_on && calibv2_cpsl_db->param.force_gray;
            } else {
                mGrayMode                       = RK_AIQ_GRAY_MODE_OFF;
                mAlogsComSharedParams.gray_mode = false;
            }
        }
    });

    uint64_t grpMask = 0;
    auto algoGroupMap = mRkAiqCoreGroupManager->getGroupAlgoListMap();
    for (const auto& group : algoGroupMap) {
        if (group.first != RK_AIQ_CORE_ANALYZE_ALL) {
            grpMask |= grpId2GrpMask(group.first);
        }
    }

    notifyUpdate(grpMask);
    if (mState != RK_AIQ_CORE_STATE_RUNNING)
        updateCalib(RK_AIQ_CORE_ANALYZE_ALL);
    else {
        waitUpdateDone();
    }
    mAlogsComSharedParams.conf_type &= ~RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

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
    mRkAiqCoreGroupManager->handleMessage(msg);
#ifdef RKAIQ_ENABLE_CAMGROUP
    if (mCamGroupCoreManager)
        mCamGroupCoreManager->processAiqCoreMsgs(this, msg);
#endif
}

XCamReturn
RkAiqCore::handle_message (const SmartPtr<XCamMessage> &msg)
{
    //return mRkAiqCoreGroupManager->handleMessage(msg);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCore::groupAnalyze(uint64_t grpId, const RkAiqAlgosGroupShared_t* shared) {
    ENTER_ANALYZER_FUNCTION();

    SmartPtr<RkAiqFullParamsProxy> fullParam;

    fullParam = analyzeInternal(static_cast<rk_aiq_core_analyze_type_e>(grpId));
    if (fullParam.ptr()) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        if (mCamGroupCoreManager)
            mCamGroupCoreManager->RelayAiqCoreResults(this, fullParam);
        else if (mCb)
#else
        if (mCb)
#endif
            mCb->rkAiqCalcDone(fullParam);
    }

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

#if defined(RKAIQ_HAVE_THUMBNAILS)
XCamReturn
RkAiqCore::thumbnailsGroupAnalyze(rkaiq_image_source_t &thumbnailsSrc)
{
    uint32_t frameId = thumbnailsSrc.frame_id;

    if (mThumbnailsService.ptr())
        mThumbnailsService->OnFrameEvent(thumbnailsSrc);

    thumbnailsSrc.image_source->unref(thumbnailsSrc.image_source);

    return XCAM_RETURN_NO_ERROR;
}
#endif

void RkAiqCore::newAiqParamsPool()
{
    for (auto& algoHdl : mCurIspAlgoHandleList) {
        bool alloc_pool = false;
        RkAiqHandle* curHdl = algoHdl.ptr();
        while (curHdl) {
            if (curHdl->getEnable()) {
                alloc_pool = true;
                break;
            }
            curHdl = curHdl->getNextHdl();
        }
        if (alloc_pool) {
            switch (algoHdl->getAlgoType()) {
            case RK_AIQ_ALGO_TYPE_AE:
                if (!mAiqAecStatsPool.ptr())
                    mAiqAecStatsPool = new RkAiqAecStatsPool("RkAiqAecStatsPool", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqExpParamsPool =
                    new RkAiqExpParamsPool("RkAiqExpParams", MAX_AEC_EFFECT_FNUM * 4);
                mAiqIrisParamsPool = new RkAiqIrisParamsPool("RkAiqIrisParams", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspAecParamsPool =
                    new RkAiqIspAecParamsPool("RkAiqIspAecParams", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspHistParamsPool =
                    new RkAiqIspHistParamsPool("RkAiqIspHistParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AWB:
                if (!mAiqAwbStatsPool.ptr())
                    mAiqAwbStatsPool = new RkAiqAwbStatsPool("RkAiqAwbStatsPool", RkAiqCore::DEFAULT_POOL_SIZE);
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspAwbV32ParamsPool = new RkAiqIspAwbParamsPoolV32(
                    "RkAiqIspAwbV32Params", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V30)
                mAiqIspAwbV3xParamsPool = new RkAiqIspAwbParamsPoolV3x(
                    "RkAiqIspAwbV3xParams", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V21)
                mAiqIspAwbV21ParamsPool = new RkAiqIspAwbParamsPoolV21(
                    "RkAiqIspAwbV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspAwbParamsPool =
                    new RkAiqIspAwbParamsPool("RkAiqIspAwbParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspAwbGainV32ParamsPool = new RkAiqIspAwbGainParamsPoolV32(
                    "RkAiqIspAwbGainParams", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspAwbGainParamsPool = new RkAiqIspAwbGainParamsPool(
                    "RkAiqIspAwbGainParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_AF:
                if (!mAiqAfStatsPool.ptr())
                    mAiqAfStatsPool = new RkAiqAfStatsPool("RkAiqAfStatsPool", RkAiqCore::DEFAULT_POOL_SIZE);
#if RKAIQ_HAVE_PDAF
                if (!mAiqPdafStatsPool.ptr())
                    mAiqPdafStatsPool = new RkAiqPdafStatsPool("RkAiqPdafStatsPool", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                mAiqFocusParamsPool = new RkAiqFocusParamsPool("RkAiqFocusParams", RkAiqCore::DEFAULT_POOL_SIZE);
#if defined(ISP_HW_V32_LITE)
                mAiqIspAfV32LiteParamsPool =
                    new RkAiqIspAfParamsPoolV32Lite("RkAiqIspAfParams", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V32)
                mAiqIspAfV32ParamsPool =
                    new RkAiqIspAfParamsPoolV32("RkAiqIspAfParams", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V30)
                mAiqIspAfV3xParamsPool =
                    new RkAiqIspAfParamsPoolV3x("RkAiqIspAfParams", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspAfParamsPool =
                    new RkAiqIspAfParamsPool("RkAiqIspAfParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_ADPCC:
                mAiqIspDpccParamsPool       = new RkAiqIspDpccParamsPool("RkAiqIspDpccParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AMERGE:
                mAiqIspMergeParamsPool      = new RkAiqIspMergeParamsPool("RkAiqIspMergeParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ATMO:
#if defined(ISP_HW_V20)
                if (!mAiqAtmoStatsPool.ptr())
                    mAiqAtmoStatsPool = new RkAiqAtmoStatsPool("RkAiqAtmoStatsPool", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspTmoParamsPool =
                    new RkAiqIspTmoParamsPool("RkAiqIspTmoParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_ACCM:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspCcmV32ParamsPool     = new RkAiqIspCcmParamsPoolV32("RkAiqIspCcmV32Params", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspCcmParamsPool        = new RkAiqIspCcmParamsPool("RkAiqIspCcmParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_ALSC:
                mAiqIspLscParamsPool        = new RkAiqIspLscParamsPool("RkAiqIspLscParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ABLC:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspBlcV32ParamsPool     = new RkAiqIspBlcParamsPoolV32("RkAiqIspBlcV32Params", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V21) || defined(ISP_HW_V30)
                mAiqIspBlcV21ParamsPool     = new RkAiqIspBlcParamsPoolV21("RkAiqIspBlcParamsV21", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspBlcParamsPool        = new RkAiqIspBlcParamsPool("RkAiqIspBlcParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_ARAWNR:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspBaynrV32ParamsPool      = new RkAiqIspBaynrParamsPoolV32("RkAiqIspRawnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V30)
                mAiqIspBaynrV3xParamsPool      = new RkAiqIspBaynrParamsPoolV3x("RkAiqIspRawnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V21)
                mAiqIspBaynrV21ParamsPool      = new RkAiqIspBaynrParamsPoolV21("RkAiqIspRawnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspRawnrParamsPool      = new RkAiqIspRawnrParamsPool("RkAiqIspRawnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_AGIC:
                mAiqIspGicParamsPool        = new RkAiqIspGicParamsPool("RkAiqIspGicParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADEBAYER:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspDebayerV32ParamsPool = new RkAiqIspDebayerParamsPoolV32("RkAiqIspDebayerParams", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspDebayerParamsPool    = new RkAiqIspDebayerParamsPool("RkAiqIspDebayerParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_ALDCH:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspLdchV32ParamsPool    = new RkAiqIspLdchParamsPoolV32("RkAiqIspLdchParams", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspLdchParamsPool       = new RkAiqIspLdchParamsPool("RkAiqIspLdchParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_A3DLUT:
                mAiqIspLut3dParamsPool      = new RkAiqIspLut3dParamsPool("RkAiqIspLut3dParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ADHAZ:
                if (!mAiqAdehazeStatsPool.ptr())
                    mAiqAdehazeStatsPool = new RkAiqAdehazeStatsPool("RkAiqAdehazeStatsPool", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspDehazeParamsPool = new RkAiqIspDehazeParamsPool(
                    "RkAiqIspDehazeParams", RkAiqCore::DEFAULT_POOL_SIZE);
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
            case RK_AIQ_ALGO_TYPE_ACSM:
                mAiqIspCsmParamsPool        = new RkAiqIspCsmParamsPool("RkAiqIspCsmParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACGC:
                mAiqIspCgcParamsPool        = new RkAiqIspCgcParamsPool("RkAiqIspCgcParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_AGAIN:
#if defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspGainV3xParamsPool     = new RkAiqIspGainParamsPoolV3x("RkAiqIspGainV3xParams", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspGainParamsPool       = new RkAiqIspGainParamsPool("RkAiqIspGainParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
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
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspTnrV32ParamsPool     = new RkAiqIspTnrParamsPoolV32("RkAiqIspTnrV32Params", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V30)
                mAiqIspTnrV3xParamsPool     = new RkAiqIspTnrParamsPoolV3x("RkAiqIspTnrV3xParams", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspTnrParamsPool        = new RkAiqIspTnrParamsPool("RkAiqIspTnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_AYNR:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspYnrV32ParamsPool     = new RkAiqIspYnrParamsPoolV32("RkAiqIspYnrV32Params", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V30)
                mAiqIspYnrV3xParamsPool     = new RkAiqIspYnrParamsPoolV3x("RkAiqIspYnrV3xParams", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V21)
                mAiqIspYnrV21ParamsPool     = new RkAiqIspYnrParamsPoolV21("RkAiqIspYnrV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspYnrParamsPool        = new RkAiqIspYnrParamsPool("RkAiqIspYnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_ACNR:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspCnrV32ParamsPool     = new RkAiqIspCnrParamsPoolV32("RkAiqIspCnrV32Params", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V30)
                mAiqIspCnrV3xParamsPool     = new RkAiqIspCnrParamsPoolV3x("RkAiqIspCnrV3xParams", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V21)
                mAiqIspCnrV21ParamsPool     = new RkAiqIspCnrParamsPoolV21("RkAiqIspCnrV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspUvnrParamsPool       = new RkAiqIspUvnrParamsPool("RkAiqIspUvnrParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_ASHARP:
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                mAiqIspSharpV32ParamsPool   = new RkAiqIspSharpParamsPoolV32("RkAiqIspSharpV32Params", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V30)
                mAiqIspSharpenV3xParamsPool   = new RkAiqIspSharpenParamsPoolV3x("RkAiqIspSharpenV3xParams", RkAiqCore::DEFAULT_POOL_SIZE);
#elif defined(ISP_HW_V21)
                mAiqIspSharpenV21ParamsPool   = new RkAiqIspSharpenParamsPoolV21("RkAiqIspSharpenV21Params", RkAiqCore::DEFAULT_POOL_SIZE);
#else
                mAiqIspSharpenParamsPool    = new RkAiqIspSharpenParamsPool("RkAiqIspSharpenParams", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspEdgefltParamsPool    = new RkAiqIspEdgefltParamsPool("RkAiqIspEdgefltParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
#if RKAIQ_HAVE_ORB_V1
            case RK_AIQ_ALGO_TYPE_AORB:
                if (!mAiqOrbStatsIntPool.ptr())
                    mAiqOrbStatsIntPool =
                        new RkAiqOrbStatsPool("RkAiqOrbStatsPool", RkAiqCore::DEFAULT_POOL_SIZE);
                mAiqIspOrbParamsPool =
                    new RkAiqIspOrbParamsPool("RkAiqIspOrbParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
#endif
            case RK_AIQ_ALGO_TYPE_AFEC:
            case RK_AIQ_ALGO_TYPE_AEIS:
                mAiqIspFecParamsPool        = new RkAiqIspFecParamsPool("RkAiqIspFecParams", RkAiqCore::DEFAULT_POOL_SIZE);
                break;
            case RK_AIQ_ALGO_TYPE_ACAC:
#if RKAIQ_HAVE_CAC_V11
                mAiqIspCacV32ParamsPool     = new RkAiqIspCacParamsPoolV32("RkAiqIspCacV32Params", RkAiqCore::DEFAULT_POOL_SIZE);
#elif RKAIQ_HAVE_CAC_V10 || RKAIQ_HAVE_CAC_V03
                mAiqIspCacV3xParamsPool     = new RkAiqIspCacParamsPoolV3x("RkAiqIspCacV3xParams", RkAiqCore::DEFAULT_POOL_SIZE);
#endif
                break;
            case RK_AIQ_ALGO_TYPE_ADRC:
                mAiqIspDrcParamsPool        = new RkAiqIspDrcParamsPool("RkAiqIspDrcParamsPool", RkAiqCore::DEFAULT_POOL_SIZE);
            default:
                break;
            }
        }
    }
}

#if RKAIQ_HAVE_PDAF
void RkAiqCore::newPdafStatsPool() {
    if (!mAiqPdafStatsPool.ptr())
        return;
    const CamCalibDbContext_t* aiqCalib     = mAlogsComSharedParams.calibv2;
    uint32_t max_cnt                        = mAiqPdafStatsPool->get_free_buffer_size();
    SmartPtr<RkAiqPdafStatsProxy> pdafStats = NULL;
    CalibDbV2_Af_Pdaf_t* pdaf;
    int pd_size;

    if (CHECK_ISP_HW_V32()) {
        CalibDbV2_AFV31_t* af_v31 =
            (CalibDbV2_AFV31_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, af_v31));
        pdaf = &af_v31->TuningPara.pdaf;
    } else if (CHECK_ISP_HW_V30()) {
        CalibDbV2_AFV30_t* af_v30 =
            (CalibDbV2_AFV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, af_v30));
        pdaf = &af_v30->TuningPara.pdaf;
    } else if (CHECK_ISP_HW_V32_LITE()) {
        CalibDbV2_AFV32_t* af_v32 =
            (CalibDbV2_AFV32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, af_v32));
        pdaf = &af_v32->TuningPara.pdaf;
    } else {
        CalibDbV2_AF_t* af = (CalibDbV2_AF_t*)CALIBDBV2_GET_MODULE_PTR((void*)aiqCalib, af);
        pdaf               = &af->TuningPara.pdaf;
    }

    pd_size = pdaf->pdWidth * pdaf->pdHeight * sizeof(short);
    if (pd_size > 0 && pdaf->enable) {
        for (uint32_t i = 0; i < max_cnt; i++) {
            pdafStats = mAiqPdafStatsPool->get_item();

            rk_aiq_isp_pdaf_stats_t* pdaf_stats = &pdafStats->data().ptr()->pdaf_stats;
            pdaf_stats->pdWidth                 = pdaf->pdWidth;
            pdaf_stats->pdHeight                = pdaf->pdHeight;
            pdaf_stats->pdLData                 = (unsigned short*)malloc(pd_size);
            pdaf_stats->pdRData                 = (unsigned short*)malloc(pd_size);
        }
    } else {
        for (uint32_t i = 0; i < max_cnt; i++) {
            pdafStats = mAiqPdafStatsPool->get_item();

            rk_aiq_isp_pdaf_stats_t* pdaf_stats = &pdafStats->data().ptr()->pdaf_stats;
            pdaf_stats->pdLData                 = NULL;
            pdaf_stats->pdRData                 = NULL;
        }
    }
}

void RkAiqCore::delPdafStatsPool() {
    if (!mAiqPdafStatsPool.ptr())
        return;
    SmartPtr<RkAiqPdafStatsProxy> pdafStats = NULL;
    uint32_t max_cnt                        = mAiqPdafStatsPool->get_free_buffer_size();
    rk_aiq_isp_pdaf_stats_t* pdaf_stats;

    for (uint32_t i = 0; i < max_cnt; i++) {
        pdafStats = mAiqPdafStatsPool->get_item();
        pdaf_stats = &pdafStats->data().ptr()->pdaf_stats;

        if (pdaf_stats->pdLData) {
            free(pdaf_stats->pdLData);
            pdaf_stats->pdLData = NULL;
        }
        if (pdaf_stats->pdRData) {
            free(pdaf_stats->pdRData);
            pdaf_stats->pdRData = NULL;
        }
    }

    if (mPdafStats.ptr()) {
        if (mPdafStats->data().ptr()) {
            pdaf_stats = &mPdafStats->data().ptr()->pdaf_stats;
            if (pdaf_stats->pdLData) {
                free(pdaf_stats->pdLData);
                pdaf_stats->pdLData = NULL;
            }
            if (pdaf_stats->pdRData) {
                free(pdaf_stats->pdRData);
                pdaf_stats->pdRData = NULL;
            }
        }
    }
}
#endif

#if defined(RKAIQ_HAVE_THUMBNAILS)
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
#endif

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
RkAiqCore::getGroupSharedParams(uint64_t groupId, RkAiqAlgosGroupShared_t* &shared)
{
    auto mapIter = mAlogsGroupSharedParamsMap.find(groupId);
    if (mapIter != mAlogsGroupSharedParamsMap.end()) {
        shared = mapIter->second;
    } else {
        LOGW_ANALYZER("don't find the group shared params of group(0x%llx)", groupId);
        return XCAM_RETURN_ERROR_FAILED;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCore::handleIspStats(const SmartPtr<VideoBuffer>& buffer,
                                     const SmartPtr<RkAiqAecStatsProxy>& aecStat,
                                     const SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                                     const SmartPtr<RkAiqAfStatsProxy>& afStat,
                                     const SmartPtr<RkAiqAtmoStatsProxy>& tmoStat,
                                     const SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqIspStatsIntProxy> ispStatsInt = NULL;

    if (mAiqIspStatsIntPool->has_free_items()) {
        ispStatsInt = mAiqIspStatsIntPool->get_item();
    } else {
        LOGE_ANALYZER("no free ispStatsInt!");
        return XCAM_RETURN_BYPASS;
    }

    ret = mTranslator->translateIspStats(buffer, ispStatsInt, aecStat, awbStat, afStat, tmoStat,
                                         dehazeStat);
    if (ret) {
        LOGE_ANALYZER("translate isp stats failed!");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ISP_STATS_OK,
            id, ispStatsInt);
    post_message(msg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqCore::handleAecStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAecStatsProxy>& aecStat_ret)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqAecStatsProxy> aecStats = NULL;

    if (!mAiqAecStatsPool.ptr()) {
        return XCAM_RETURN_BYPASS;
    } else if (mAiqAecStatsPool->has_free_items()) {
        aecStats = mAiqAecStatsPool->get_item();
    } else {
        LOGW_AEC("no free aecStats buffer!");
        ret = XCAM_RETURN_BYPASS;
        goto out;
    }
    ret = mTranslator->translateAecStats(buffer, aecStats);
    if (ret < 0) {
        LOGE_ANALYZER("translate aec stats failed!");
        aecStats = NULL;
        ret = XCAM_RETURN_BYPASS;
    }

out:
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

    if (!mAiqAwbStatsPool.ptr()) {
        return XCAM_RETURN_BYPASS;
    } else if (mAiqAwbStatsPool->has_free_items()) {
        awbStats = mAiqAwbStatsPool->get_item();
    } else {
        LOGW_AWB("no free awbStats buffer!");
        ret = XCAM_RETURN_BYPASS;
        goto out;
    }
    ret = mTranslator->translateAwbStats(buffer, awbStats);
    if (ret < 0) {
        LOGE_ANALYZER("translate awb stats failed!");
        awbStats = NULL;
        ret = XCAM_RETURN_BYPASS;
    }

out:
    awbStat_ret = awbStats;

    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AWB_STATS_OK,
            id, awbStats);
    post_message(msg);

    return ret;
}

XCamReturn
RkAiqCore::handleAfStats(const SmartPtr<VideoBuffer> &buffer, SmartPtr<RkAiqAfStatsProxy>& afStat_ret)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqAfStatsProxy> afStats = NULL;

    if (!mAiqAfStatsPool.ptr()) {
        return XCAM_RETURN_BYPASS;
    } else if (mAiqAfStatsPool->has_free_items()) {
        afStats = mAiqAfStatsPool->get_item();
    } else {
        LOGW_AF("no free afStats buffer!");
        ret = XCAM_RETURN_BYPASS;
        goto out;
    }
    ret = mTranslator->translateAfStats(buffer, afStats);
    if (ret < 0) {
        LOGE_ANALYZER("translate af stats failed!");
        afStats = NULL;
        ret = XCAM_RETURN_BYPASS;
    }

out:
    afStat_ret = afStats;
    if (mPdafSupport) {
        mAfStats = afStats;
        mAfStatsFrmId = buffer->get_sequence();
        mAfStatsTime = buffer->get_timestamp();
        if (((ABS(mAfStatsTime - mPdafStatsTime) < mFrmInterval / 2LL) && mIspOnline) ||
            ((mAfStatsTime - mPdafStatsTime < mFrmInterval) && (mAfStatsTime >= mPdafStatsTime) && !mIspOnline)) {
            SmartPtr<XCamMessage> afStatsMsg =
                new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AF_STATS_OK, mAfStatsFrmId, mAfStats);
            SmartPtr<XCamMessage> pdafStatsMsg =
                new RkAiqCoreVdBufMsg(XCAM_MESSAGE_PDAF_STATS_OK, mAfStatsFrmId, mPdafStats);
            post_message(afStatsMsg);
            post_message(pdafStatsMsg);
            mAfStats = NULL;
            mPdafStats = NULL;
        }
    } else {
        uint32_t id = buffer->get_sequence();
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AF_STATS_OK, id, afStats);
        post_message(msg);
    }

    return ret;
}

#if RKAIQ_HAVE_PDAF
XCamReturn RkAiqCore::handlePdafStats(const SmartPtr<VideoBuffer>& buffer) {
    XCamReturn ret                          = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqPdafStatsProxy> pdafStats = NULL;
    bool sns_mirror                         = mAlogsComSharedParams.sns_mirror;

    if (mAiqPdafStatsPool->has_free_items()) {
        pdafStats = mAiqPdafStatsPool->get_item();
    } else {
        LOGE_ANALYZER("no free afStats buffer!");
        return XCAM_RETURN_BYPASS;
    }
    ret = mTranslator->translatePdafStats(buffer, pdafStats, sns_mirror);
    if (ret < 0) {
        LOGE_ANALYZER("translate af stats failed!");
        return XCAM_RETURN_BYPASS;
    }

    mPdafStats = pdafStats;
    mPdafStatsTime = buffer->get_timestamp();
    if (((ABS(mAfStatsTime - mPdafStatsTime) < mFrmInterval / 2LL) && mIspOnline) ||
        ((mAfStatsTime - mPdafStatsTime < mFrmInterval) && (mAfStatsTime >= mPdafStatsTime) && !mIspOnline)) {
        SmartPtr<XCamMessage> afStatsMsg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AF_STATS_OK, mAfStatsFrmId, mAfStats);
        SmartPtr<XCamMessage> pdafStatsMsg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_PDAF_STATS_OK, mAfStatsFrmId, mPdafStats);

        post_message(afStatsMsg);
        post_message(pdafStatsMsg);
        mAfStats = NULL;
        mPdafStats = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn RkAiqCore::handleAtmoStats(const SmartPtr<VideoBuffer>& buffer,
                                      SmartPtr<RkAiqAtmoStatsProxy>& tmoStat) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!mAiqAtmoStatsPool.ptr()) {
        return XCAM_RETURN_BYPASS;
    } else if (mAiqAtmoStatsPool->has_free_items()) {
        tmoStat = mAiqAtmoStatsPool->get_item();
    } else {
        LOGW_ATMO("no free atmoStats buffer!");
        ret = XCAM_RETURN_BYPASS;
        goto out;
    }
    ret = mTranslator->translateAtmoStats(buffer, tmoStat);
    if (ret < 0) {
        LOGE_ANALYZER("translate tmo stats failed!");
        tmoStat = NULL;
        ret = XCAM_RETURN_BYPASS;
    }

out:
    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ATMO_STATS_OK,
            id, tmoStat);
    post_message(msg);

    return ret;
}

XCamReturn RkAiqCore::handleAdehazeStats(const SmartPtr<VideoBuffer>& buffer,
        SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!mAiqAdehazeStatsPool.ptr()) {
        return XCAM_RETURN_BYPASS;
    } else if (mAiqAdehazeStatsPool->has_free_items()) {
        dehazeStat = mAiqAdehazeStatsPool->get_item();
    } else {
        LOGW_ADEHAZE("no free adehazeStats buffer!");
        ret = XCAM_RETURN_BYPASS;
        goto out;
    }
    ret = mTranslator->translateAdehazeStats(buffer, dehazeStat);
    if (ret < 0) {
        LOGE_ANALYZER("translate dehaze stats failed!");
        dehazeStat = NULL;
        ret = XCAM_RETURN_BYPASS;
    }

out:
    uint32_t id = buffer->get_sequence();
    SmartPtr<XCamMessage> msg = new RkAiqCoreVdBufMsg(XCAM_MESSAGE_ADEHAZE_STATS_OK,
            id, dehazeStat);
    post_message(msg);

    return ret;
}

XCamReturn
RkAiqCore::handleOrbStats(const SmartPtr<VideoBuffer> &buffer)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqOrbStatsProxy> orbStats = NULL;
    if (!mAiqOrbStatsIntPool.ptr()) {
        return XCAM_RETURN_BYPASS;
    } else if (mAiqOrbStatsIntPool->has_free_items()) {
        orbStats = mAiqOrbStatsIntPool->get_item();
    } else {
        LOGW_AORB("no free orbStats!");
        ret = XCAM_RETURN_BYPASS;
        goto out;
    }

    ret = mTranslator->translateOrbStats(buffer, orbStats);
    if (ret) {
        orbStats = NULL;
        return XCAM_RETURN_BYPASS;
    }

out:
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

#if RKAIQ_HAVE_PDAF
XCamReturn
RkAiqCore::set_pdaf_support(bool support)
{
    mPdafSupport = support;
    return XCAM_RETURN_NO_ERROR;
}

bool RkAiqCore::get_pdaf_support()
{
    return mPdafSupport;
}
#endif

XCamReturn
RkAiqCore::newAiqGroupAnayzer()
{
    mRkAiqCoreGroupManager = new RkAiqAnalyzeGroupManager(this, mIsSingleThread);
    mRkAiqCoreGroupManager->parseAlgoGroup(mAlgosDesArray);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCore::updateCalibDbBrutal(CamCalibDbV2Context_t* aiqCalib)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // Fill new calib to the AlogsSharedParams
    mAlogsComSharedParams.calibv2 = aiqCalib;
    mAlogsComSharedParams.conf_type = RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

#if RKAIQ_HAVE_ASD_V10
    initCpsl();
#endif
    CalibDbV2_ColorAsGrey_t* colorAsGrey =
        (CalibDbV2_ColorAsGrey_t*)CALIBDBV2_GET_MODULE_PTR(
            (void*)(mAlogsComSharedParams.calibv2), colorAsGrey);
    if (colorAsGrey->param.enable) {
        mGrayMode                       = RK_AIQ_GRAY_MODE_ON;
        mAlogsComSharedParams.gray_mode = true;
    } else {
        mGrayMode                       = RK_AIQ_GRAY_MODE_OFF;
        mAlogsComSharedParams.gray_mode = false;
    }
    setGrayMode(mGrayMode);

    for (auto& algoHdl : mCurIspAlgoHandleList) {
        RkAiqHandle* curHdl = algoHdl.ptr();
        while (curHdl) {
            if (curHdl->getEnable()) {
                /* update user initial params */
                ret = curHdl->updateConfig(true);
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d update initial user params failed", curHdl->getAlgoType());
                algoHdl->setReConfig(true);
                ret = curHdl->prepare();
                RKAIQCORE_CHECK_BYPASS(ret, "algoHdl %d prepare failed", curHdl->getAlgoType());
            }
            curHdl = curHdl->getNextHdl();
        }
    }

    mAlogsComSharedParams.conf_type &= ~RK_AIQ_ALGO_CONFTYPE_UPDATECALIB;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCore::updateCalib(enum rk_aiq_core_analyze_type_e type)
{
    SmartLock lock (_update_mutex);
    // check if group bit still set
    uint64_t need_update = groupUpdateMask & grpId2GrpMask(type);
    if (!need_update) {
        return XCAM_RETURN_NO_ERROR;
    }

    prepare(type);
    // clear group bit after update
    groupUpdateMask &= (~need_update);
    // notify update done
    _update_done_cond.broadcast();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCore::notifyUpdate(uint64_t mask)
{
    SmartLock lock (_update_mutex);

    groupUpdateMask |= mask;

    return XCamReturn();
}

XCamReturn RkAiqCore::waitUpdateDone()
{
    SmartLock lock (_update_mutex);

    while (groupUpdateMask != 0) {
        _update_done_cond.timedwait(_update_mutex, 100000ULL);
    }

    return XCamReturn();
}

void RkAiqCore::setDelayCnts(int8_t delayCnts) {
    mRkAiqCoreGroupManager->setDelayCnts(delayCnts);
}

} //namespace RkCam
