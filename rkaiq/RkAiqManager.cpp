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
#include "RkAiqManager.h"

#include <fcntl.h>
#include <unistd.h>

#ifdef RKAIQ_ENABLE_CAMGROUP
#include "RkAiqCamGroupManager.h"
#endif
#include "isp20/Isp20_module_dbg.h"
#include "isp20/CamHwIsp20.h"
#include "isp21/CamHwIsp21.h"
#include "isp3x/CamHwIsp3x.h"

using namespace XCam;
namespace RkCam {

#define RKAIQMNG_CHECK_RET(ret, format, ...) \
    if (ret) { \
        LOGE(format, ##__VA_ARGS__); \
        return ret; \
    }

bool
RkAiqMngCmdThread::loop ()
{
    ENTER_XCORE_FUNCTION();

    const static int32_t timeout = -1;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<msg_t> msg = mAiqCmdQueue.pop (timeout);

    if (!msg.ptr()) {
        XCAM_LOG_WARNING("RkAiqMngCmdThread got empty result, stop thread");
        return false;
    }

    XCAM_ASSERT (mAiqMng);

    switch (msg->cmd) {
    case MSG_CMD_SW_WORKING_MODE:
        if (msg->sync)
            msg->mutex->lock();
        mAiqMng->swWorkingModeDyn(msg->data.sw_wk_mode.mode);
        mAiqMng->mWkSwitching = false;
        if (msg->sync) {
            msg->cond->broadcast ();
            msg->mutex->unlock();
        }
        break;
    default:
        break;
    }
    // always true
    return true;
}

bool
RkAiqRstApplyThread::loop ()
{
    ENTER_XCORE_FUNCTION();

    const static int32_t timeout = -1;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<RkAiqFullParamsProxy> results = mAiqRstQueue.pop (timeout);

    XCAM_ASSERT (mAiqMng);

    if (!results.ptr()) {
        XCAM_LOG_WARNING("RkAiqRstApplyThread got empty result, stop thread");
        return false;
    }

#ifdef RUNTIME_MODULE_DEBUG
#ifndef RK_SIMULATOR_HW
    if (g_apply_init_params_only)
        goto out;
#endif
#endif
    ret = mAiqMng->applyAnalyzerResult(results);
    if (ret == XCAM_RETURN_NO_ERROR)
        return true;

    EXIT_XCORE_FUNCTION();
out:
    // always true
    return true;
}

RkAiqManager::RkAiqManager(const char* sns_ent_name,
                           rk_aiq_error_cb err_cb,
                           rk_aiq_metas_cb metas_cb)
    : mCamHw(NULL)
    , mRkAiqAnalyzer(NULL)
    , mAiqRstAppTh(new RkAiqRstApplyThread(this))
    , mAiqMngCmdTh(new RkAiqMngCmdThread(this))
#ifdef ISP_HW_V20
    , mRkLumaAnalyzer(NULL)
#endif
    , mErrCb(err_cb)
    , mMetasCb(metas_cb)
    , mHwEvtCb(NULL)
    , mHwEvtCbCtx(NULL)
    , mSnsEntName(sns_ent_name)
#ifdef RKAIQ_ENABLE_PARSER_V1
    , mCalibDb(NULL)
#endif
    , mCalibDbV2(NULL)
    , tuningCalib(NULL)
    , mWorkingMode(RK_AIQ_WORKING_MODE_NORMAL)
    , mOldWkModeForGray(RK_AIQ_WORKING_MODE_NORMAL)
    , mWkSwitching(false)
    , _state(AIQ_STATE_INVALID)
    , mCurMirror(false)
    , mCurFlip(false)
#ifdef RKAIQ_ENABLE_CAMGROUP
    , mCamGroupCoreManager(NULL)
#endif
    , mIsMain(false)
{
    ENTER_XCORE_FUNCTION();
    EXIT_XCORE_FUNCTION();
}

RkAiqManager::~RkAiqManager()
{
    ENTER_XCORE_FUNCTION();
    EXIT_XCORE_FUNCTION();
}

void
RkAiqManager::setCamHw(SmartPtr<ICamHw>& camhw)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!mCamHw.ptr());
    mCamHw = camhw;
    EXIT_XCORE_FUNCTION();
}

void
RkAiqManager::setAnalyzer(SmartPtr<RkAiqCore> analyzer)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!mRkAiqAnalyzer.ptr());
    mRkAiqAnalyzer = analyzer;
    EXIT_XCORE_FUNCTION();
}

#ifdef ISP_HW_V20
void
RkAiqManager::setLumaAnalyzer(SmartPtr<RkLumaCore> analyzer)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!mRkLumaAnalyzer.ptr());
    mRkLumaAnalyzer = analyzer;
    EXIT_XCORE_FUNCTION();
}
#endif

#ifdef RKAIQ_ENABLE_PARSER_V1
void
RkAiqManager::setAiqCalibDb(const CamCalibDbContext_t* calibDb)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!mCalibDb);
    mCalibDb = calibDb;
    EXIT_XCORE_FUNCTION();
}
#endif

void
RkAiqManager::setAiqCalibDb(const CamCalibDbV2Context_t* calibDb)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!mCalibDbV2);
    mCalibDbV2 = new CamCalibDbV2Context_t();
    *mCalibDbV2 = *calibDb;
    EXIT_XCORE_FUNCTION();
}

XCamReturn
RkAiqManager::init()
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    XCAM_ASSERT (mSnsEntName);
    XCAM_ASSERT (mRkAiqAnalyzer.ptr());
    XCAM_ASSERT (mCamHw.ptr());
    XCAM_ASSERT (mCalibDbV2);

    mRkAiqAnalyzer->setAnalyzeResultCb(this);
#ifdef RKAIQ_ENABLE_PARSER_V1
    ret = mRkAiqAnalyzer->init(mSnsEntName, mCalibDb, mCalibDbV2);
#else
    ret = mRkAiqAnalyzer->init(mSnsEntName, nullptr, mCalibDbV2);
#endif
    RKAIQMNG_CHECK_RET(ret, "analyzer init error %d !", ret);

#ifdef ISP_HW_V20
    mRkLumaAnalyzer->setAnalyzeResultCb(this);
    CalibDbV2_LUMA_DETECT_t *lumaDetect =
        (CalibDbV2_LUMA_DETECT_t*)(CALIBDBV2_GET_MODULE_PTR((void*)mCalibDbV2, lumaDetect));
    if (lumaDetect) {
        ret = mRkLumaAnalyzer->init(lumaDetect);
        RKAIQMNG_CHECK_RET(ret, "luma analyzer init error %d !", ret);
    } else {
        mRkLumaAnalyzer.release();
    }
#endif
    mCamHw->setHwResListener(this);
    ret = mCamHw->init(mSnsEntName);
    RKAIQMNG_CHECK_RET(ret, "camHw init error %d !", ret);
    _state = AIQ_STATE_INITED;
    isp_drv_share_mem_ops_t *mem_ops = NULL;
    mCamHw->getShareMemOps(&mem_ops);
    mRkAiqAnalyzer->setShareMemOps(mem_ops);
    // set default mirror & flip
    setDefMirrorFlip();
    mAiqMngCmdTh->triger_start();
    bool bret = mAiqMngCmdTh->start();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "cmd thread start error");

    mDleayCpslParams = NULL;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
RkAiqManager::prepare(uint32_t width, uint32_t height, rk_aiq_working_mode_t mode)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_exposure_sensor_descriptor sensor_des;

    XCAM_ASSERT (mCalibDbV2);
#ifdef RUNTIME_MODULE_DEBUG
#ifndef RK_SIMULATOR_HW
    get_dbg_force_disable_mods_env();
#endif
#endif
    int working_mode_hw = RK_AIQ_WORKING_MODE_NORMAL;
    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        working_mode_hw = mode;
    } else {
        if (mode == RK_AIQ_WORKING_MODE_ISP_HDR2)
            working_mode_hw = RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR;
        else if (mode == RK_AIQ_WORKING_MODE_ISP_HDR3)
            working_mode_hw = RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR;
        else
            LOGE_ANALYZER("Not supported HDR mode !");
    }
    mCamHw->setCalib(mCalibDbV2);
    CalibDb_Sensor_ParaV2_t* sensor_calib =
        (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(mCalibDbV2, sensor_calib));

#ifdef RKAIQ_ENABLE_CAMGROUP
    mCamHw->setGroupMode(mCamGroupCoreManager ? true : false, mIsMain);
#endif

    if(mode != RK_AIQ_WORKING_MODE_NORMAL)
        ret = mCamHw->prepare(width, height, working_mode_hw,
                              sensor_calib->CISExpUpdate.Hdr.time_update,
                              sensor_calib->CISExpUpdate.Hdr.gain_update);
    else
        ret = mCamHw->prepare(width, height, working_mode_hw,
                              sensor_calib->CISExpUpdate.Linear.time_update,
                              sensor_calib->CISExpUpdate.Linear.gain_update);

    RKAIQMNG_CHECK_RET(ret, "camhw prepare error %d", ret);

    xcam_mem_clear(sensor_des);
    ret = mCamHw->getSensorModeData(mSnsEntName, sensor_des);

    sensor_output_width = sensor_des.sensor_output_width;
    sensor_output_height = sensor_des.sensor_output_height;
    int w, h, aligned_w, aligned_h;
    ret = mCamHw->get_sp_resolution(w, h, aligned_w, aligned_h);
    ret = mRkAiqAnalyzer->set_sp_resolution(w, h, aligned_w, aligned_h);
#ifdef ISP_HW_V20
    if (mRkLumaAnalyzer.ptr())
        ret = mRkLumaAnalyzer->prepare(working_mode_hw);
#endif
#if RKAIQ_HAVE_PDAF
    ret = mRkAiqAnalyzer->set_pdaf_support(mCamHw->get_pdaf_support());
#endif

    RKAIQMNG_CHECK_RET(ret, "getSensorModeData error %d", ret);
    mRkAiqAnalyzer->notifyIspStreamMode(mCamHw->getIspStreamMode());
    ret = mRkAiqAnalyzer->prepare(&sensor_des, working_mode_hw);
    RKAIQMNG_CHECK_RET(ret, "analyzer prepare error %d", ret);

    SmartPtr<RkAiqFullParamsProxy> initParams = mRkAiqAnalyzer->getAiqFullParams();

    if (mTbInfo.is_pre_aiq) {
        mAiqRstAppTh->triger_start();
    }
#ifdef RKAIQ_ENABLE_CAMGROUP
    if (!mCamGroupCoreManager) {
#endif
        ret = applyAnalyzerResult(initParams);
        RKAIQMNG_CHECK_RET(ret, "set initial params error %d", ret);
#ifdef RKAIQ_ENABLE_CAMGROUP
    }
#endif

    mWorkingMode = mode;
    mOldWkModeForGray = RK_AIQ_WORKING_MODE_NORMAL;
    mWidth = width;
    mHeight = height;
    _state = AIQ_STATE_PREPARED;
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
RkAiqManager::start()
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // restart
    if (_state == AIQ_STATE_STOPED) {
        SmartPtr<RkAiqFullParamsProxy> initParams = mRkAiqAnalyzer->getAiqFullParams();

#if 0
        if (initParams->data()->mIspMeasParams.ptr()) {
            initParams->data()->mIspMeasParams->data()->frame_id = 0;
        }

        if (initParams->data()->mIsppMeasParams.ptr()) {
            initParams->data()->mIsppMeasParams->data()->frame_id = 0;
        }
#endif
        applyAnalyzerResult(initParams);
    } else if (_state == AIQ_STATE_STARTED) {
        return ret;
    }

    if (mTbInfo.is_pre_aiq) {
        bool bret = mAiqRstAppTh->start();
        ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
        RKAIQMNG_CHECK_RET(ret, "apply result thread start error");
    }

    ret = mRkAiqAnalyzer->start();
    RKAIQMNG_CHECK_RET(ret, "analyzer start error %d", ret);

#ifdef ISP_HW_V20
    if (mRkLumaAnalyzer.ptr()) {
        ret = mRkLumaAnalyzer->start();
        RKAIQMNG_CHECK_RET(ret, "luma analyzer start error %d", ret);
    }
#endif
    ret = mCamHw->start();
    RKAIQMNG_CHECK_RET(ret, "camhw start error %d", ret);

    if (!mTbInfo.is_pre_aiq) {
        mAiqRstAppTh->triger_start();

        bool bret = mAiqRstAppTh->start();
        ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
        RKAIQMNG_CHECK_RET(ret, "apply result thread start error");
    }
    _state = AIQ_STATE_STARTED;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
RkAiqManager::stop(bool keep_ext_hw_st)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (_state  == AIQ_STATE_STOPED) {
        return ret;
    }

    mAiqRstAppTh->triger_stop();
    bool bret = mAiqRstAppTh->stop();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "apply result thread stop error");

    ret = mRkAiqAnalyzer->stop();
    RKAIQMNG_CHECK_RET(ret, "analyzer stop error %d", ret);

#ifdef ISP_HW_V20
    if (mRkLumaAnalyzer.ptr()) {
        ret = mRkLumaAnalyzer->stop();
        RKAIQMNG_CHECK_RET(ret, "luma analyzer stop error %d", ret);
    }
#endif
    mCamHw->keepHwStAtStop(keep_ext_hw_st);
    ret = mCamHw->stop();
    RKAIQMNG_CHECK_RET(ret, "camhw stop error %d", ret);

    mDleayCpslParams = NULL;

    _state = AIQ_STATE_STOPED;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
RkAiqManager::deInit()
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // stop first in prepared status, some resources and process were
    // done at prepare stage
    if (_state == AIQ_STATE_PREPARED)
        stop(false);

    mAiqMngCmdTh->triger_stop();

    bool bret = mAiqMngCmdTh->stop();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "cmd thread stop error");

    ret = mRkAiqAnalyzer->deInit();
    RKAIQMNG_CHECK_RET(ret, "analyzer deinit error %d", ret);

#ifdef ISP_HW_V20
    if (mRkLumaAnalyzer.ptr()) {
        ret = mRkLumaAnalyzer->deInit();
        RKAIQMNG_CHECK_RET(ret, "luma analyzer deinit error %d", ret);
    }
#endif
    ret = mCamHw->deInit();
    RKAIQMNG_CHECK_RET(ret, "camhw deinit error %d", ret);
    if (mCalibDbV2) {
        delete mCalibDbV2;
        mCalibDbV2 = NULL;
    }
    if (tuningCalib) {
        RkAiqCalibDbV2::FreeCalibByJ2S(tuningCalib);
        mCalibDbV2 = NULL;
    }

    _state = AIQ_STATE_INVALID;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
RkAiqManager::updateCalibDb(const CamCalibDbV2Context_t* newCalibDb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    auto update_list = std::make_shared<std::list<std::string>>();
    update_list->push_back("colorAsGrey");

    *mCalibDbV2 = *(CamCalibDbV2Context_t*)newCalibDb;
    mCamHw->setCalib(newCalibDb);

    ret = mRkAiqAnalyzer->setCalib(mCalibDbV2);

    if (!mRkAiqAnalyzer->isRunningState()) {
      mRkAiqAnalyzer->updateCalibDbBrutal(mCalibDbV2);
    } else {
      mRkAiqAnalyzer->calibTuning(mCalibDbV2, update_list);
    }

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqManager::syncSofEvt(SmartPtr<VideoBuffer>& hwres)
{
    ENTER_XCORE_FUNCTION();

    if (hwres->_buf_type == ISP_POLL_SOF) {
        xcam_get_runtime_log_level();
        SmartPtr<CamHwIsp20> mCamHwIsp20 = mCamHw.dynamic_cast_ptr<CamHwIsp20>();
        mCamHwIsp20->notify_sof(hwres);

        SmartPtr<SofEventBuffer> evtbuf = hwres.dynamic_cast_ptr<SofEventBuffer>();
        SmartPtr<SofEventData> evtdata = evtbuf->get_data();
        SmartPtr<ispHwEvt_t> hw_evt = mCamHwIsp20->make_ispHwEvt(evtdata->_frameid, V4L2_EVENT_FRAME_SYNC, evtdata->_timestamp);
        mRkAiqAnalyzer->pushEvts(hw_evt);

        // TODO: moved to aiq core ?
        if (mMetasCb) {
            rk_aiq_metas_t metas;
            metas.frame_id = hwres->get_sequence();
#ifdef ANDROID_OS
            mMetasCb(&metas);
#else
            (*mMetasCb)(&metas);
#endif
        }
    }

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqManager::hwResCb(SmartPtr<VideoBuffer>& hwres)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (hwres->_buf_type == ISP_POLL_3A_STATS) {
        if (mTbInfo.is_pre_aiq) {
            uint32_t seq = -1;
            seq = hwres.dynamic_cast_ptr<VideoBuffer>()->get_sequence();
            if (seq == 0 && mTBStatsCnt == 0) {
                LOGE("<TB> tb hwResCb stats %d\n", seq);
                struct timespec tp;
                clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

                SmartPtr<CamHwIsp20> mCamHwIsp20 =
                    mCamHw.dynamic_cast_ptr<CamHwIsp20>();
                SmartPtr<ispHwEvt_t> hw_evt = mCamHwIsp20->make_ispHwEvt(
                                                  0, V4L2_EVENT_FRAME_SYNC,
                                                  tp.tv_sec * 1000 * 1000 * 1000 + tp.tv_nsec);
                LOGE("<TB> push sof %d\n", seq);
                mRkAiqAnalyzer->pushEvts(hw_evt);
            }

            if (mTBStatsCnt == 0) ret = mRkAiqAnalyzer->pushStats(hwres);

            mTBStatsCnt++;
        } else {
            ret = mRkAiqAnalyzer->pushStats(hwres);
        }
#ifdef ISP_HW_V20
    } else if (hwres->_buf_type == ISP_POLL_LUMA) {
        if (mRkLumaAnalyzer.ptr())
            ret = mRkLumaAnalyzer->pushStats(hwres);
#endif
    } else if (hwres->_buf_type == ISP_POLL_PARAMS) {
        rk_aiq_err_msg_t msg;
        msg.err_code = XCAM_RETURN_BYPASS;
        if (mTbInfo.is_pre_aiq && mErrCb)
            (*mErrCb)(&msg);
    } else if (hwres->_buf_type == ISPP_POLL_NR_STATS) {
        ret = mRkAiqAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISP_POLL_SOF) {
        xcam_get_runtime_log_level();

        SmartPtr<CamHwIsp20> mCamHwIsp20 = mCamHw.dynamic_cast_ptr<CamHwIsp20>();
        mCamHwIsp20->notify_sof(hwres);

        SmartPtr<SofEventBuffer> evtbuf = hwres.dynamic_cast_ptr<SofEventBuffer>();
        SmartPtr<SofEventData> evtdata = evtbuf->get_data();
        SmartPtr<ispHwEvt_t> hw_evt = mCamHwIsp20->make_ispHwEvt(evtdata->_frameid, V4L2_EVENT_FRAME_SYNC, evtdata->_timestamp);
        mRkAiqAnalyzer->pushEvts(hw_evt);
        // TODO: moved to aiq core ?
        if (mMetasCb) {
            rk_aiq_metas_t metas;
            metas.frame_id = evtdata->_frameid;
#ifdef ANDROID_OS
            mMetasCb(&metas);
#else
            (*mMetasCb)(&metas);
#endif
        }
    } else if (hwres->_buf_type == ISP_POLL_TX) {
#if 0
        XCamVideoBuffer* camVBuf = convert_to_XCamVideoBuffer(hwres);
        LOGD_ANALYZER("raw: \n format: 0x%x\n color_bits: %d\n width: %d\n height: %d\n aligned_width: %d\naligned_height: %d\n"
                      "size: %d\n components: %d\n strides[0]: %d\n strides[1]: %d\n offset[0]: %d\n offset[1]: %d\n",
                      camVBuf->info.format, camVBuf->info.color_bits, camVBuf->info.width, camVBuf->info.height,
                      camVBuf->info.aligned_width, camVBuf->info.aligned_height, camVBuf->info.size, camVBuf->info.components,
                      camVBuf->info.strides[0], camVBuf->info.strides[1], camVBuf->info.offsets[0], camVBuf->info.offsets[1]);

        camVBuf->unref(camVBuf);
#endif
        ret = mRkAiqAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISP_POLL_SP) {
#if 0
        XCamVideoBuffer* camVBuf = convert_to_XCamVideoBuffer(hwres);
        LOGD_ANALYZER("spimg: frameid:%d \n format: 0x%x\n color_bits: %d\n width: %d\n height: %d\n aligned_width: %d\naligned_height: %d\n"
                      "size: %d\n components: %d\n strides[0]: %d\n strides[1]: %d\n offset[0]: %d\n offset[1]: %d\n", hwres->get_sequence(),
                      camVBuf->info.format, camVBuf->info.color_bits, camVBuf->info.width, camVBuf->info.height,
                      camVBuf->info.aligned_width, camVBuf->info.aligned_height, camVBuf->info.size, camVBuf->info.components,
                      camVBuf->info.strides[0], camVBuf->info.strides[1], camVBuf->info.offsets[0], camVBuf->info.offsets[1]);
        camVBuf->unref(camVBuf);
#endif
        LOGD_ANALYZER("ISP_IMG");
        ret = mRkAiqAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISP_NR_IMG) {
#if 0
        XCamVideoBuffer* camVBuf = convert_to_XCamVideoBuffer(hwres);
        LOGD_ANALYZER("nrimg: \n format: 0x%x\n color_bits: %d\n width: %d\n height: %d\n aligned_width: %d\naligned_height: %d\n"
                      "size: %d\n components: %d\n strides[0]: %d\n strides[1]: %d\n offset[0]: %d\n offset[1]: %d\n",
                      camVBuf->info.format, camVBuf->info.color_bits, camVBuf->info.width, camVBuf->info.height,
                      camVBuf->info.aligned_width, camVBuf->info.aligned_height, camVBuf->info.size, camVBuf->info.components,
                      camVBuf->info.strides[0], camVBuf->info.strides[1], camVBuf->info.offsets[0], camVBuf->info.offsets[1]);
        camVBuf->unref(camVBuf);
#endif
        ret = mRkAiqAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISP_GAIN) {
        LOGD_ANALYZER("ISP_GAIN");
        ret = mRkAiqAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISPP_GAIN_WR) {
        LOGD_ANALYZER("ISPP_GAIN_WR");
        ret = mRkAiqAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISPP_GAIN_KG) {
        LOGD_ANALYZER("ISPP_GAIN_KG");
        ret = mRkAiqAnalyzer->pushStats(hwres);
#if RKAIQ_HAVE_PDAF
    } else if (hwres->_buf_type == ISP_POLL_PDAF_STATS) {
        LOGD_ANALYZER("ISP_POLL_PDAF_STATS");
        ret = mRkAiqAnalyzer->pushStats(hwres);
#endif
    } else if (hwres->_buf_type == VICAP_STREAM_ON_EVT) {
        LOGD_ANALYZER("VICAP_STREAM_ON_EVT ... ");
        if (mHwEvtCb) {
            rk_aiq_hwevt_t hwevt;
            memset(&hwevt, 0, sizeof(hwevt));
            hwevt.cam_id = mCamHw->getCamPhyId();
#ifdef RKAIQ_ENABLE_CAMGROUP
            if (mCamGroupCoreManager) {
                mCamGroupCoreManager->setVicapReady(&hwevt);
                if (mCamGroupCoreManager->isAllVicapReady())
                    hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
                else
                    hwevt.aiq_status = 0;
            } else
                hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
#else
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
#endif
            hwevt.ctx = mHwEvtCbCtx;
            (*mHwEvtCb)(&hwevt);
        }
    } else if (hwres->_buf_type == VICAP_RESET_EVT) {
        LOGD_ANALYZER(" VICAP_RESET_EVT... ");
        if (mHwEvtCb) {
            rk_aiq_hwevt_t hwevt;

            memset(&hwevt, 0, sizeof(hwevt));
            hwevt.cam_id = mCamHw->getCamPhyId();
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_RESET;
            hwevt.ctx = mHwEvtCbCtx;

            LOGE_ANALYZER("cam: %d, VICAP_RESET_EVT...", hwevt.cam_id);
            (*mHwEvtCb)(&hwevt);
        }
    } else if (hwres->_buf_type == VICAP_WITH_RK1608_RESET_EVT) {
        LOGD_ANALYZER(" VICAP_WITH_RK1608_RESET_EVT... ");
        SmartPtr<CamHwIsp20> mCamHwIsp20 = mCamHw.dynamic_cast_ptr<CamHwIsp20>();
        if (mHwEvtCb && mCamHwIsp20.ptr()) {
            rk_aiq_hwevt_t hwevt;

            memset(&hwevt, 0, sizeof(hwevt));
            for(int id = 0; id < 8; id++)
                hwevt.multi_cam.multi_cam_id[id] = -1;

            int i = 0;
            for(int camPhyId = 0; camPhyId < CAM_INDEX_FOR_1608; camPhyId++) {
                if (CamHwIsp20::rk1608_share_inf.raw_proc_unit[camPhyId]) {
                    hwevt.multi_cam.multi_cam_id[i++] = camPhyId;
                }
            }
            hwevt.multi_cam.cam_count = i;
            hwevt.cam_id = -1;
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_WITH_MULTI_CAM_RESET;
            hwevt.ctx = mHwEvtCbCtx;

            for (i = 0; i < 8; i++) {
                LOGV_ANALYZER("multi_cam_id[%d]: %d \n", i, hwevt.multi_cam.multi_cam_id[i]);
            }

            (*mHwEvtCb)(&hwevt);
        }
    }

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
RkAiqManager::applyAnalyzerResult(SmartPtr<RkAiqFullParamsProxy>& results)
{
    ENTER_XCORE_FUNCTION();
    //xcam_get_runtime_log_level();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqFullParams* aiqParams = NULL;

    if (!results.ptr()) {
        LOGW_ANALYZER("empty aiq params results!");
        return ret;
    }

    aiqParams = results->data().ptr();
    cam3aResultList results_list;

    if (aiqParams->mExposureParams.ptr()) {
        aiqParams->mExposureParams->setType(RESULT_TYPE_EXPOSURE_PARAM);
        results_list.push_back(aiqParams->mExposureParams);
    }

    if (aiqParams->mFocusParams.ptr()) {
        aiqParams->mFocusParams->setType(RESULT_TYPE_FOCUS_PARAM);
        results_list.push_back(aiqParams->mFocusParams);
    }

#define APPLY_ANALYZER_RESULT(lc, BC) \
    if (aiqParams->m##lc##Params.ptr()) { \
        aiqParams->m##lc##Params->setType(RESULT_TYPE_##BC##_PARAM); \
        aiqParams->m##lc##Params->setId(aiqParams->m##lc##Params->data()->frame_id); \
        results_list.push_back(aiqParams->m##lc##Params); \
    } \

#if RKAIQ_HAVE_ASD_V10
    APPLY_ANALYZER_RESULT(Cpsl, CPSL);
#endif

#if RKAIQ_HAVE_AE_V1
    APPLY_ANALYZER_RESULT(Aec, AEC);
    APPLY_ANALYZER_RESULT(Hist, HIST);
#endif
#if RKAIQ_HAVE_AWB_V20
    APPLY_ANALYZER_RESULT(Awb, AWB);
#endif
    APPLY_ANALYZER_RESULT(AwbGain, AWBGAIN);
#if RKAIQ_HAVE_AF_V20 || RKAIQ_ONLY_AF_STATS_V20
    APPLY_ANALYZER_RESULT(Af, AF);
#endif
#if RKAIQ_HAVE_DPCC_V1
    APPLY_ANALYZER_RESULT(Dpcc, DPCC);
#endif
#if (RKAIQ_HAVE_MERGE_V10 | RKAIQ_HAVE_MERGE_V11 | RKAIQ_HAVE_MERGE_V12)
    APPLY_ANALYZER_RESULT(Merge, MERGE);
#endif
#if RKAIQ_HAVE_TMO_V1
    APPLY_ANALYZER_RESULT(Tmo, TMO);
#endif
#if RKAIQ_HAVE_CCM_V1
    APPLY_ANALYZER_RESULT(Ccm, CCM);
#endif
#if RKAIQ_HAVE_BLC_V1
    APPLY_ANALYZER_RESULT(Blc, BLC);
#endif
#if RKAIQ_HAVE_ANR_V1
    APPLY_ANALYZER_RESULT(Rawnr, RAWNR);
#endif
#if (RKAIQ_HAVE_GIC_V1  || RKAIQ_HAVE_GIC_V2)
    APPLY_ANALYZER_RESULT(Gic, GIC);
#endif
#if RKAIQ_HAVE_DEBAYER_V1
    APPLY_ANALYZER_RESULT(Debayer, DEBAYER);
#endif
#if RKAIQ_HAVE_LDCH_V10
    APPLY_ANALYZER_RESULT(Ldch, LDCH);
#endif
#if RKAIQ_HAVE_3DLUT_V1
    APPLY_ANALYZER_RESULT(Lut3d, LUT3D);
#endif
#if (RKAIQ_HAVE_DEHAZE_V10 | RKAIQ_HAVE_DEHAZE_V11 | RKAIQ_HAVE_DEHAZE_V11_DUO | RKAIQ_HAVE_DEHAZE_V12)
    APPLY_ANALYZER_RESULT(Dehaze, DEHAZE);
#endif
#if (RKAIQ_HAVE_GAMMA_V10 | RKAIQ_HAVE_GAMMA_V11)
    APPLY_ANALYZER_RESULT(Agamma, AGAMMA);
#endif
#if RKAIQ_HAVE_DEGAMMA_V1
    APPLY_ANALYZER_RESULT(Adegamma, ADEGAMMA);
#endif
#if RKAIQ_HAVE_WDR_V1
    APPLY_ANALYZER_RESULT(Wdr, WDR);
#endif
#if RKAIQ_HAVE_CSM_V1
    APPLY_ANALYZER_RESULT(Csm, CSM);
#endif
#if RKAIQ_HAVE_CGC_V1
    APPLY_ANALYZER_RESULT(Cgc, CGC);
#endif
    APPLY_ANALYZER_RESULT(Conv422, CONV422);
    APPLY_ANALYZER_RESULT(Yuvconv, YUVCONV);
#if RKAIQ_HAVE_GAIN_V1
    APPLY_ANALYZER_RESULT(Gain, GAIN);
#endif
#if RKAIQ_HAVE_ACP_V10
    APPLY_ANALYZER_RESULT(Cp, CP);
#endif
#if RKAIQ_HAVE_AIE_V10
    APPLY_ANALYZER_RESULT(Ie, IE);
#endif
#if RKAIQ_HAVE_AMD_V1
    APPLY_ANALYZER_RESULT(Motion, MOTION);
#endif
#if RKAIQ_HAVE_ANR_V1
    APPLY_ANALYZER_RESULT(Tnr, TNR);
    APPLY_ANALYZER_RESULT(Ynr, YNR);
    APPLY_ANALYZER_RESULT(Uvnr, UVNR);
    APPLY_ANALYZER_RESULT(Sharpen, SHARPEN);
    APPLY_ANALYZER_RESULT(Edgeflt, EDGEFLT);
#endif
#if RKAIQ_HAVE_FEC_V10
    APPLY_ANALYZER_RESULT(Fec, FEC);
#endif
#if RKAIQ_HAVE_ORB_V1
    APPLY_ANALYZER_RESULT(Orb, ORB);
#endif
#if RKAIQ_HAVE_TMO_V1
    APPLY_ANALYZER_RESULT(Tmo, TMO);
#endif
    // ispv21
#if (RKAIQ_HAVE_DRC_V10 || RKAIQ_HAVE_DRC_V11 || RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE)
    APPLY_ANALYZER_RESULT(Drc, DRC);
#endif
#if RKAIQ_HAVE_AWB_V21
    APPLY_ANALYZER_RESULT(AwbV21, AWB);
#endif
#if RKAIQ_HAVE_YNR_V2
    APPLY_ANALYZER_RESULT(YnrV21, YNR);
#endif
#if RKAIQ_HAVE_CNR_V1
    APPLY_ANALYZER_RESULT(CnrV21, UVNR);
#endif
#if RKAIQ_HAVE_SHARP_V3
    APPLY_ANALYZER_RESULT(SharpenV21, SHARPEN);
#endif
#if RKAIQ_HAVE_BAYERNR_V2
    APPLY_ANALYZER_RESULT(BaynrV21, RAWNR);
#endif
    // ispv3x
#if RKAIQ_HAVE_AWB_V21
    APPLY_ANALYZER_RESULT(AwbV3x, AWB);
#endif
#if RKAIQ_HAVE_BLC_V1
    APPLY_ANALYZER_RESULT(BlcV21, BLC);
#endif
#if (RKAIQ_HAVE_LSC_V1 | RKAIQ_HAVE_LSC_V2 | RKAIQ_HAVE_LSC_V3)
    APPLY_ANALYZER_RESULT(Lsc, LSC);
#endif
#if RKAIQ_HAVE_AF_V30 || RKAIQ_ONLY_AF_STATS_V30
    APPLY_ANALYZER_RESULT(AfV3x, AF);
#endif
#if RKAIQ_HAVE_BAYER2DNR_V2
    APPLY_ANALYZER_RESULT(BaynrV3x, RAWNR);
#endif
#if RKAIQ_HAVE_YNR_V3
    APPLY_ANALYZER_RESULT(YnrV3x, YNR);
#endif
#if RKAIQ_HAVE_CNR_V2
    APPLY_ANALYZER_RESULT(CnrV3x, UVNR);
#endif
#if RKAIQ_HAVE_SHARP_V4
    APPLY_ANALYZER_RESULT(SharpenV3x, SHARPEN);
#endif
#if RKAIQ_HAVE_CAC_V03
    APPLY_ANALYZER_RESULT(CacV3x, CAC);
#endif
#if RKAIQ_HAVE_GAIN_V2
    APPLY_ANALYZER_RESULT(GainV3x, GAIN);
#endif
#if RKAIQ_HAVE_BAYERTNR_V2
    APPLY_ANALYZER_RESULT(TnrV3x, TNR);
#endif
    // ispv32
#if RKAIQ_HAVE_BLC_V32
    APPLY_ANALYZER_RESULT(BlcV32, BLC);
#endif
#if (RKAIQ_HAVE_BAYERTNR_V23 || RKAIQ_HAVE_BAYERTNR_V23_LITE)
    APPLY_ANALYZER_RESULT(TnrV32, TNR);
#endif
#if RKAIQ_HAVE_BAYER2DNR_V23
    APPLY_ANALYZER_RESULT(BaynrV32, RAWNR);
#endif
#if RKAIQ_HAVE_CAC_V11
    APPLY_ANALYZER_RESULT(CacV32, CAC);
#endif
#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE
    APPLY_ANALYZER_RESULT(DebayerV32, DEBAYER);
#endif
#if RKAIQ_HAVE_CCM_V2
    APPLY_ANALYZER_RESULT(CcmV32, CCM);
#endif
    // APPLY_ANALYZER_RESULT(DehazeV32, DEHAZE);
#if RKAIQ_HAVE_LDCH_V21
    APPLY_ANALYZER_RESULT(LdchV32, LDCH);
#endif
#if RKAIQ_HAVE_YNR_V22
    APPLY_ANALYZER_RESULT(YnrV32, YNR);
#endif
#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)
    APPLY_ANALYZER_RESULT(CnrV32, UVNR);
#endif
#if (RKAIQ_HAVE_SHARP_V33 || RKAIQ_HAVE_SHARP_V33_LITE)
    APPLY_ANALYZER_RESULT(SharpV32, SHARPEN);
#endif
#if RKAIQ_HAVE_AWB_V32
    APPLY_ANALYZER_RESULT(AwbV32, AWB);
#endif
#if RKAIQ_HAVE_AF_V31 || RKAIQ_ONLY_AF_STATS_V31
    APPLY_ANALYZER_RESULT(AfV32, AF);
#endif
#if RKAIQ_HAVE_AWB_V32
    APPLY_ANALYZER_RESULT(AwbGainV32, AWBGAIN);
#endif
#if RKAIQ_HAVE_AF_V32_LITE || RKAIQ_ONLY_AF_STATS_V32_LITE
    APPLY_ANALYZER_RESULT(AfV32Lite, AF);
#endif
    mCamHw->applyAnalyzerResult(results_list);

    EXIT_XCORE_FUNCTION();

    return ret;
}

void
RkAiqManager::rkAiqCalcDone(SmartPtr<RkAiqFullParamsProxy> &results)
{
    ENTER_XCORE_FUNCTION();

    XCAM_ASSERT (mAiqRstAppTh.ptr());
    mAiqRstAppTh->push_results(results);

    EXIT_XCORE_FUNCTION();
}

void
RkAiqManager::rkAiqCalcFailed(const char* msg)
{
    ENTER_XCORE_FUNCTION();
    // TODO
    EXIT_XCORE_FUNCTION();
    return ;
}

void
RkAiqManager::rkLumaCalcDone(rk_aiq_luma_params_t luma_params)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = mCamHw->setHdrProcessCount(luma_params);
    EXIT_XCORE_FUNCTION();
}

void
RkAiqManager::rkLumaCalcFailed(const char* msg)
{
    ENTER_XCORE_FUNCTION();
    // TODO
    EXIT_XCORE_FUNCTION();
    return ;
}

XCamReturn
RkAiqManager::setModuleCtl(rk_aiq_module_id_t mId, bool mod_en)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = mCamHw->setModuleCtl(mId, mod_en);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
RkAiqManager::getModuleCtl(rk_aiq_module_id_t mId, bool& mod_en)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = mCamHw->getModuleCtl(mId, mod_en);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
RkAiqManager::rawdataPrepare(rk_aiq_raw_prop_t prop)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = mCamHw->rawdataPrepare(prop);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
RkAiqManager::enqueueRawBuffer(void *rawdata, bool sync)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = mCamHw->enqueueRawBuffer(rawdata, sync);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
RkAiqManager::enqueueRawFile(const char *path)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = mCamHw->enqueueRawFile(path);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn
RkAiqManager::registRawdataCb(void (*callback)(void *))
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = mCamHw->registRawdataCb(callback);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn RkAiqManager::setSharpFbcRotation(rk_aiq_rotation_t rot)
{
#ifndef RK_SIMULATOR_HW
    SmartPtr<CamHwIsp20> camHwIsp20 = mCamHw.dynamic_cast_ptr<CamHwIsp20>();

    if (camHwIsp20.ptr())
        return camHwIsp20->setSharpFbcRotation(rot);
    else
        return XCAM_RETURN_ERROR_FAILED;
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
}

XCamReturn RkAiqManager::setMirrorFlip(bool mirror, bool flip, int skip_frm_cnt)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_XCORE_FUNCTION();
    if (_state == AIQ_STATE_INVALID) {
        LOGE_ANALYZER("wrong aiq state !");
        return XCAM_RETURN_ERROR_FAILED;
    }
    ret = mCamHw->setSensorFlip(mirror, flip, skip_frm_cnt);
    if (ret == XCAM_RETURN_NO_ERROR) {
        // notify aiq sensor flip is changed
        mRkAiqAnalyzer->setSensorFlip(mirror, flip);
        mCurMirror = mirror;
        mCurFlip = flip;
    } else {
        LOGW_ANALYZER("set mirror %d, flip %d error", mirror, flip);
    }
    return ret;
    EXIT_XCORE_FUNCTION();
}

XCamReturn RkAiqManager::getMirrorFlip(bool& mirror, bool& flip)
{
    ENTER_XCORE_FUNCTION();
    if (_state == AIQ_STATE_INVALID) {
        LOGE_ANALYZER("wrong aiq state !");
        return XCAM_RETURN_ERROR_FAILED;
    }

    mirror = mCurMirror;
    flip = mCurFlip;

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqManager::setDefMirrorFlip()
{
    /* set defalut mirror & flip from iq*/
    CalibDb_Sensor_ParaV2_t* sensor =
        (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(mCalibDbV2, sensor_calib));
    bool def_mirr = sensor->CISFlip & 0x1 ? true : false;
    bool def_flip = sensor->CISFlip & 0x2 ? true : false;
    setMirrorFlip(def_mirr, def_flip, 0);
}

XCamReturn RkAiqManager::swWorkingModeDyn_msg(rk_aiq_working_mode_t mode) {
    SmartPtr<RkAiqMngCmdThread::msg_t> msg = new RkAiqMngCmdThread::msg_t();
    msg->cmd = RkAiqMngCmdThread::MSG_CMD_SW_WORKING_MODE;
    msg->sync = true;
    msg->data.sw_wk_mode.mode = mode;
    mAiqMngCmdTh->send_cmd(msg);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqManager::swWorkingModeDyn(rk_aiq_working_mode_t mode)
{
    ENTER_XCORE_FUNCTION();

    SmartPtr<RkAiqFullParamsProxy> initParams;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (mode == mWorkingMode)
        return ret;

    if (_state != AIQ_STATE_STARTED) {
        LOGW_ANALYZER("should be called at STARTED state");
        return ret;
    }
    // 1. stop analyzer, re-preapre with the new mode
    // 2. stop luma analyzer, re-preapre with the new mode
    LOGI_ANALYZER("stop analyzer ...");
    mAiqRstAppTh->triger_stop();
    bool bret = mAiqRstAppTh->stop();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "apply result thread stop error");

    ret = mRkAiqAnalyzer->stop();
    RKAIQMNG_CHECK_RET(ret, "analyzer stop error %d", ret);

#ifdef ISP_HW_V20
    if (mRkLumaAnalyzer.ptr()) {
        ret = mRkLumaAnalyzer->stop();
        RKAIQMNG_CHECK_RET(ret, "luma analyzer stop error %d", ret);
    }
#endif
    // 3. pause hwi
    LOGI_ANALYZER("pause hwi ...");
    ret = mCamHw->pause();
    RKAIQMNG_CHECK_RET(ret, "pause hwi error %d", ret);

    int working_mode_hw = RK_AIQ_WORKING_MODE_NORMAL;
    if (mode == RK_AIQ_WORKING_MODE_ISP_HDR2)
        working_mode_hw = RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR;
    else if (mode == RK_AIQ_WORKING_MODE_ISP_HDR3)
        working_mode_hw = RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR;

    // 4. set new mode to hwi
    ret = mCamHw->swWorkingModeDyn(working_mode_hw);
    if (ret) {
        LOGE_ANALYZER("hwi swWorkingModeDyn error ...");
        goto restart;
    }

    // 5. re-prepare analyzer
    LOGI_ANALYZER("reprepare analyzer ...");
    rk_aiq_exposure_sensor_descriptor sensor_des;
    ret = mCamHw->getSensorModeData(mSnsEntName, sensor_des);
    mRkAiqAnalyzer->notifyIspStreamMode(mCamHw->getIspStreamMode());
    ret = mRkAiqAnalyzer->prepare(&sensor_des, working_mode_hw);
    RKAIQMNG_CHECK_RET(ret, "analyzer prepare error %d", ret);

    initParams = mRkAiqAnalyzer->getAiqFullParams();

    ret = applyAnalyzerResult(initParams);
    RKAIQMNG_CHECK_RET(ret, "set initial params error %d", ret);

restart:
    // 6. resume hwi
    LOGI_ANALYZER("resume hwi");
    ret = mCamHw->resume();
    RKAIQMNG_CHECK_RET(ret, "pause hwi error %d", ret);

    // 7. restart analyzer
    LOGI_ANALYZER("restart analyzer");
    mAiqRstAppTh->triger_start();
    bret = mAiqRstAppTh->start();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "apply result thread start error");

    ret = mRkAiqAnalyzer->start();
    RKAIQMNG_CHECK_RET(ret, "analyzer start error %d", ret);

#ifdef ISP_HW_V20
    if (mRkLumaAnalyzer.ptr()) {
        ret = mRkLumaAnalyzer->start();
        RKAIQMNG_CHECK_RET(ret, "luma analyzer start error %d", ret);
    }
#endif
    /* // 7. resume hwi */
    /* LOGI_ANALYZER("resume hwi"); */
    /* ret = mCamHw->resume(); */
    /* RKAIQMNG_CHECK_RET(ret, "pause hwi error %d", ret); */

    mWorkingMode = mode;
    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqManager::setMulCamConc(bool cc)
{
#ifndef RK_SIMULATOR_HW
    SmartPtr<CamHwIsp20> camHwIsp20 = mCamHw.dynamic_cast_ptr<CamHwIsp20>();

    if (camHwIsp20.ptr())
        camHwIsp20->setMulCamConc(cc);

    mRkAiqAnalyzer->setMulCamConc(cc);
#endif
}

CamCalibDbV2Context_t* RkAiqManager::getCurrentCalibDBV2()
{
    return mCalibDbV2;
}

XCamReturn RkAiqManager::calibTuning(CamCalibDbV2Context_t* aiqCalib,
                                     ModuleNameList& change_list)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCamHw->setCalib(aiqCalib);
    ret = mRkAiqAnalyzer->setCalib(aiqCalib);

    if (change_list != nullptr)
        std::for_each(
            std::begin(*change_list), std::end(*change_list),
        [](const std::string & name) {
        std::cout << "tuning : " << name << std::endl;
    });
    mRkAiqAnalyzer->calibTuning(aiqCalib, change_list);

    // Won't free calib witch from iqfiles
    RkAiqCalibDbV2::FreeCalibByJ2S(tuningCalib);
    *mCalibDbV2 = *aiqCalib;
    tuningCalib = const_cast<CamCalibDbV2Context_t*>(aiqCalib);

    change_list.reset();

    EXIT_XCORE_FUNCTION();

    return ret;
}

void RkAiqManager::unsetTuningCalibDb()
{
    tuningCalib = NULL;
}

} //namespace RkCam

