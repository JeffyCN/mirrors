/*
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

#include "RkAiqManager.h"
#include "isp20/Isp20_module_dbg.h"
#include "isp20/CamHwIsp20.h"
#include "isp21/CamHwIsp21.h"
#include <fcntl.h>
#include <unistd.h>

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
    , mRkLumaAnalyzer(NULL)
    , mAiqRstAppTh(new RkAiqRstApplyThread(this))
    , mAiqMngCmdTh(new RkAiqMngCmdThread(this))
    , mErrCb(err_cb)
    , mMetasCb(metas_cb)
    , mSnsEntName(sns_ent_name)
    , mWorkingMode(RK_AIQ_WORKING_MODE_NORMAL)
    , mOldWkModeForGray(RK_AIQ_WORKING_MODE_NORMAL)
    , mWkSwitching(false)
    , mCalibDb(NULL)
    , mCalibDbV2(NULL)
    , _state(AIQ_STATE_INVALID)
    , mCurMirror(false)
    , mCurFlip(false)
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

void
RkAiqManager::setLumaAnalyzer(SmartPtr<RkLumaCore> analyzer)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!mRkLumaAnalyzer.ptr());
    mRkLumaAnalyzer = analyzer;
    EXIT_XCORE_FUNCTION();
}

void
RkAiqManager::setAiqCalibDb(const CamCalibDbContext_t* calibDb)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!mCalibDb);
    mCalibDb = calibDb;
    EXIT_XCORE_FUNCTION();
}

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
    // set hw infos
    struct RkAiqHwInfo hw_info;
    xcam_mem_clear(hw_info);

#ifndef RK_SIMULATOR_HW
    rk_aiq_static_info_t* s_info = CamHwIsp20::getStaticCamHwInfo(mSnsEntName);
    hw_info.fl_supported = s_info->has_fl;
    hw_info.irc_supported = s_info->has_irc;
    hw_info.lens_supported = s_info->has_lens_vcm;
    hw_info.fl_strth_adj = s_info->fl_strth_adj_sup;
    hw_info.fl_ir_strth_adj = s_info->fl_ir_strth_adj_sup;
    mIspHwVer = s_info->isp_hw_ver;
#endif
    mRkAiqAnalyzer->setHwInfos(hw_info);
    ret = mRkAiqAnalyzer->init(mSnsEntName, mCalibDb, mCalibDbV2);
    RKAIQMNG_CHECK_RET(ret, "analyzer init error %d !", ret);

    mRkLumaAnalyzer->setAnalyzeResultCb(this);
    CalibDbV2_LUMA_DETECT_t *lumaDetect =
        (CalibDbV2_LUMA_DETECT_t*)(CALIBDBV2_GET_MODULE_PTR((void*)mCalibDbV2, lumaDetect));
    ret = mRkLumaAnalyzer->init(lumaDetect);
    RKAIQMNG_CHECK_RET(ret, "luma analyzer init error %d !", ret);

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
        // depreate mCalibDb->sysContrl.hdr_mode
        /* if (mode != RK_AIQ_HDR_GET_WORKING_MODE(mCalibDb->sysContrl.hdr_mode)) { */
        /*     ret = XCAM_RETURN_ERROR_PARAM; */
        /*     RKAIQMNG_CHECK_RET(ret, "Not supported HDR mode!"); */
        /* } else { */
        /*     working_mode_hw = mCalibDb->sysContrl.hdr_mode; */
        /* } */
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
    int w,h,aligned_w,aligned_h;
    ret = mCamHw->get_sp_resolution(w, h, aligned_w, aligned_h);
    ret = mRkAiqAnalyzer->set_sp_resolution(w, h, aligned_w, aligned_h);
    ret = mRkLumaAnalyzer->prepare(working_mode_hw);

    RKAIQMNG_CHECK_RET(ret, "getSensorModeData error %d", ret);
    mRkAiqAnalyzer->notifyIspStreamMode(mCamHw->getIspStreamMode());
    ret = mRkAiqAnalyzer->prepare(&sensor_des, working_mode_hw);
    RKAIQMNG_CHECK_RET(ret, "analyzer prepare error %d", ret);

    SmartPtr<RkAiqFullParamsProxy> initParams = mRkAiqAnalyzer->getAiqFullParams();

    ret = applyAnalyzerResult(initParams);
    RKAIQMNG_CHECK_RET(ret, "set initial params error %d", ret);

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
    }

    mAiqRstAppTh->triger_start();

    bool bret = mAiqRstAppTh->start();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "apply result thread start error");

    ret = mRkAiqAnalyzer->start();
    RKAIQMNG_CHECK_RET(ret, "analyzer start error %d", ret);

    ret = mRkLumaAnalyzer->start();
    RKAIQMNG_CHECK_RET(ret, "luma analyzer start error %d", ret);

    ret = mCamHw->start();
    RKAIQMNG_CHECK_RET(ret, "camhw start error %d", ret);

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

    ret = mRkLumaAnalyzer->stop();
    RKAIQMNG_CHECK_RET(ret, "luma analyzer stop error %d", ret);

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

    mAiqMngCmdTh->triger_stop();

    bool bret = mAiqMngCmdTh->stop();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "cmd thread stop error");

    ret = mRkAiqAnalyzer->deInit();
    RKAIQMNG_CHECK_RET(ret, "analyzer deinit error %d", ret);

    ret = mRkLumaAnalyzer->deInit();
    RKAIQMNG_CHECK_RET(ret, "luma analyzer deinit error %d", ret);

    ret = mCamHw->deInit();
    RKAIQMNG_CHECK_RET(ret, "camhw deinit error %d", ret);
    if (mCalibDbV2) {
        delete mCalibDbV2;
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
    SmartPtr<RkAiqFullParamsProxy> initParams;

    if (_state != AIQ_STATE_STARTED) {
        LOGW_ANALYZER("should be called at STARTED state");
        return ret;
    }

    mAiqRstAppTh->triger_stop();
    bool bret = mAiqRstAppTh->stop();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "apply result thread stop error");

    ret = mRkAiqAnalyzer->stop();
    RKAIQMNG_CHECK_RET(ret, "analyzer stop error %d", ret);

    //ret = mRkLumaAnalyzer->stop();
    //RKAIQMNG_CHECK_RET(ret, "luma analyzer stop error %d", ret);

    // ret = mRkAiqAnalyzer->deInit();
    //ret = mRkLumaAnalyzer->deInit();

    *mCalibDbV2 = *newCalibDb;

    CalibDbV2_LUMA_DETECT_t *lumaDetect =
        (CalibDbV2_LUMA_DETECT_t*)(CALIBDBV2_GET_MODULE_PTR((void*)mCalibDbV2, lumaDetect));

    ret = mRkAiqAnalyzer->setCalib(mCalibDbV2);
    //ret = mRkLumaAnalyzer->init(lumaDetect);

    // 3. re-prepare analyzer
    LOGI_ANALYZER("reprepare analyzer ...");
    rk_aiq_exposure_sensor_descriptor sensor_des;
    ret = mCamHw->getSensorModeData(mSnsEntName, sensor_des);

    int working_mode_hw = RK_AIQ_WORKING_MODE_NORMAL;
    if (mWorkingMode == RK_AIQ_WORKING_MODE_ISP_HDR2)
        working_mode_hw = RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR;
    else if (mWorkingMode == RK_AIQ_WORKING_MODE_ISP_HDR3)
        working_mode_hw = RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR;

    mRkAiqAnalyzer->notifyIspStreamMode(mCamHw->getIspStreamMode());
    ret = mRkAiqAnalyzer->prepare(&sensor_des, working_mode_hw);
    RKAIQMNG_CHECK_RET(ret, "analyzer prepare error %d", ret);

    // update calib to hw
    mCamHw->setCalib(mCalibDbV2);

    initParams = mRkAiqAnalyzer->getAiqFullParams();

    ret = applyAnalyzerResult(initParams);
    RKAIQMNG_CHECK_RET(ret, "set initial params error %d", ret);

    // 4. restart analyzer
    LOGI_ANALYZER("restart analyzer");
    mAiqRstAppTh->triger_start();
    bret = mAiqRstAppTh->start();
    ret = bret ? XCAM_RETURN_NO_ERROR : XCAM_RETURN_ERROR_FAILED;
    RKAIQMNG_CHECK_RET(ret, "apply result thread start error");

    ret = mRkAiqAnalyzer->start();
    RKAIQMNG_CHECK_RET(ret, "analyzer start error %d", ret);

    //ret = mRkLumaAnalyzer->start();
    //RKAIQMNG_CHECK_RET(ret, "luma analyzer start error %d", ret);

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RkAiqManager::hwResCb(SmartPtr<VideoBuffer>& hwres)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (hwres->_buf_type == ISP_POLL_3A_STATS) {
        ret = mRkAiqAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISP_POLL_LUMA) {
        ret = mRkLumaAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISP_POLL_PARAMS) {
    } else if (hwres->_buf_type == ISPP_POLL_NR_STATS) {
        ret = mRkAiqAnalyzer->pushStats(hwres);
    } else if (hwres->_buf_type == ISP_POLL_SOF){
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
            (*mMetasCb)(&metas);
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
                "size: %d\n components: %d\n strides[0]: %d\n strides[1]: %d\n offset[0]: %d\n offset[1]: %d\n",hwres->get_sequence(),
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
    }

    EXIT_XCORE_FUNCTION();

    return ret;
}

#if 0
XCamReturn
RkAiqManager::applyAnalyzerResult(SmartPtr<RkAiqFullParamsProxy>& results)
{
    ENTER_XCORE_FUNCTION();
    xcam_get_runtime_log_level();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqFullParams* aiqParams = NULL;

    if (!results.ptr()) {
        LOGW_ANALYZER("empty aiq params results!");
        return ret;
    }
    // TODO: couldn't get dynamic debug env now
#if 0//def RUNTIME_MODULE_DEBUG
    get_dbg_force_disable_mods_env();
#endif

    aiqParams = results->data().ptr();

#ifdef RUNTIME_MODULE_DEBUG
#ifndef RK_SIMULATOR_HW
    if (g_bypass_exp_params)
        goto set_exp_end;
#endif
#endif

    /* #define FLASH_CTL_DEBUG */
#ifdef FLASH_CTL_DEBUG
    {
        // for test
        int fd = open("/tmp/flash_ctl", O_RDWR);
        if (fd != -1) {
            char c;
            read(fd, &c, 1);
            int enable = atoi(&c);
            SmartPtr<rk_aiq_flash_setting_t> fl = new rk_aiq_flash_setting_t();
            fl->flash_mode = enable ? RK_AIQ_FLASH_MODE_TORCH : RK_AIQ_FLASH_MODE_OFF;
            fl->power[0] = 10000;
            fl->strobe = enable ? true : false;
            aiqParams->mFlParams = new SharedItemProxy<rk_aiq_flash_setting_t>(fl);
            aiqParams->mFlParams->setType(RESULT_TYPE_CPSL_PARAM);
            mCamHw->applyAnalyzerResult(aiqParams->mFlParams, false);
            close(fd);
        }
    }
#else
#ifndef RK_SIMULATOR_HW
    if (aiqParams->mCpslParams.ptr() && aiqParams->mIspOtherParams.ptr()) {
        SmartPtr<CamHwIsp20> mCamHwIsp20 = mCamHw.dynamic_cast_ptr<CamHwIsp20>();
        // TODO Merge : consider isp21 ?
        rk_aiq_isp_other_params_v20_t* ispParams =
            static_cast<rk_aiq_isp_other_params_v20_t*>(aiqParams->mIspOtherParams->data().ptr());
        int gray_mode = ispParams->ie.base.mode;

        bool cpsl_ir_en = aiqParams->mCpslParams->data()->update_ir &&
                          aiqParams->mCpslParams->data()->ir.irc_on;
        bool cpsl_update = aiqParams->mCpslParams->data()->update_ir ||
                           aiqParams->mCpslParams->data()->update_fl;

        if (cpsl_ir_en) {
            mDelayCpslApplyFrmNum = 2;
            mDleayCpslParams = aiqParams->mCpslParams;
            LOGD_ANALYZER("gray mode on, cpsl ir on delay 2 frames");
        } else if (cpsl_update) {
            mDleayCpslParams.release();
            mDelayCpslApplyFrmNum = 0;
            aiqParams->mCpslParams->setType(RESULT_TYPE_CPSL_PARAM);
            mCamHw->applyAnalyzerResult(aiqParams->mCpslParams, false);
        }
    }

    if (mDleayCpslParams.ptr() && --mDelayCpslApplyFrmNum == 0) {
        LOGD_ANALYZER("set delyay cpsl ir on");
        aiqParams->mCpslParams->setType(RESULT_TYPE_CPSL_PARAM);
        mCamHw->applyAnalyzerResult(mDleayCpslParams, false);
    }
#endif
#endif

    if (aiqParams->mExposureParams.ptr()) {
//#define DEBUG_FIXED_EXPOSURE
#ifdef DEBUG_FIXED_EXPOSURE
        /* test aec with fixed sensor exposure */
        int cnt = aiqParams->mIspMeasParams->data()->frame_id ;
        if (aiqParams->mExposureParams->data()->algo_id == 0) {
            aiqParams->mExposureParams->data()->exp_tbl_size = 1;
            RKAiqAecExpInfo_t* exp_tbl = &aiqParams->mExposureParams->data()->exp_tbl[0];
            if(cnt % 40 <= 19) {
                exp_tbl->HdrExp[2].exp_sensor_params.coarse_integration_time = 984;
                exp_tbl->HdrExp[2].exp_sensor_params.analog_gain_code_global = 48;
                exp_tbl->HdrExp[1].exp_sensor_params.coarse_integration_time = 984;
                exp_tbl->HdrExp[1].exp_sensor_params.analog_gain_code_global = 48;
                exp_tbl->HdrExp[0].exp_sensor_params.coarse_integration_time = 246;
                exp_tbl->HdrExp[0].exp_sensor_params.analog_gain_code_global = 16;

                exp_tbl->HdrExp[2].exp_real_params.integration_time = 0.02;
                exp_tbl->HdrExp[2].exp_real_params.analog_gain = 3;
                exp_tbl->HdrExp[1].exp_real_params.integration_time = 0.02;
                exp_tbl->HdrExp[1].exp_real_params.analog_gain = 3;
                exp_tbl->HdrExp[0].exp_real_params.integration_time = 0.005;
                exp_tbl->HdrExp[0].exp_real_params.analog_gain = 1;
            } else {
                exp_tbl->HdrExp[2].exp_sensor_params.coarse_integration_time = 1475;
                exp_tbl->HdrExp[2].exp_sensor_params.analog_gain_code_global = 144;
                exp_tbl->HdrExp[1].exp_sensor_params.coarse_integration_time = 1475;
                exp_tbl->HdrExp[1].exp_sensor_params.analog_gain_code_global = 144;
                exp_tbl->HdrExp[0].exp_sensor_params.coarse_integration_time = 492;
                exp_tbl->HdrExp[0].exp_sensor_params.analog_gain_code_global = 48;

                exp_tbl->HdrExp[2].exp_real_params.integration_time = 0.03;
                exp_tbl->HdrExp[2].exp_real_params.analog_gain = 9;
                exp_tbl->HdrExp[1].exp_real_params.integration_time = 0.03;
                exp_tbl->HdrExp[1].exp_real_params.analog_gain = 9;
                exp_tbl->HdrExp[0].exp_real_params.integration_time = 0.01;
                exp_tbl->HdrExp[0].exp_real_params.analog_gain = 3;
            }
        }
        aiqParams->mExposureParams->setType(RESULT_TYPE_EXPOSURE);
        mCamHw->applyAnalyzerResult(aiqParams->mExposureParams, false);
#else
        aiqParams->mExposureParams->setType(RESULT_TYPE_EXPOSURE);
        if (_state == AIQ_STATE_STARTED)
            mCamHw->applyAnalyzerResult(aiqParams->mExposureParams, false);
        else
            mCamHw->applyAnalyzerResult(aiqParams->mExposureParams, true);
#endif
    }
set_exp_end:

    CalibDb_Aec_ParaV2_t *aec =
        (CalibDb_Aec_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)mCalibDbV2, ae_calib));
    if (aiqParams->mIrisParams.ptr()) {
        aiqParams->mIrisParams->setType(RESULT_TYPE_IRIS_PARAM);
        mCamHw->applyAnalyzerResult(aiqParams->mIrisParams, false);
    }


#ifdef RUNTIME_MODULE_DEBUG
#ifndef RK_SIMULATOR_HW
    if (g_bypass_isp_params)
        goto set_isp_end;
#endif
#endif

    if (aiqParams->mIspOtherParams.ptr()) {
        aiqParams->mIspOtherParams->setType(RESULT_TYPE_ISP_OTHER);
        if (_state == AIQ_STATE_STARTED)
            mCamHw->applyAnalyzerResult(aiqParams->mIspOtherParams, false);
        else
            mCamHw->applyAnalyzerResult(aiqParams->mIspOtherParams, true);
    }

    if (aiqParams->mIspMeasParams.ptr()) {
#ifndef RK_SIMULATOR_HW
        if (mWorkingMode != RK_AIQ_WORKING_MODE_NORMAL) {
            SmartPtr<CamHwIsp20> mCamHwIsp20 = mCamHw.dynamic_cast_ptr<CamHwIsp20>();
            // TODO Merge : consider isp21 ?
            rk_aiq_isp_meas_params_v20_t* ispParams =
                static_cast<rk_aiq_isp_meas_params_v20_t*>(aiqParams->mIspMeasParams->data().ptr());
            bool isHdrGlobalTmo = ispParams->atmo_proc_res.isHdrGlobalTmo;
            mCamHwIsp20->setHdrGlobalTmoMode(aiqParams->mIspMeasParams->data()->frame_id, isHdrGlobalTmo);
        }
#endif
        aiqParams->mIspMeasParams->setType(RESULT_TYPE_ISP_MEAS);
        if (_state == AIQ_STATE_STARTED)
            mCamHw->applyAnalyzerResult(aiqParams->mIspMeasParams, false);
        else
            mCamHw->applyAnalyzerResult(aiqParams->mIspMeasParams, true);
    }

set_isp_end:

#ifdef RUNTIME_MODULE_DEBUG
#ifndef RK_SIMULATOR_HW
    if (g_bypass_ispp_params)
        goto set_ispp_end;
#endif
#endif

#ifndef DISABLE_PP
    if (aiqParams->mIsppOtherParams.ptr()) {
        aiqParams->mIsppOtherParams->setType(RESULT_TYPE_ISPP_OTHER);
        if (_state == AIQ_STATE_STARTED)
            mCamHw->applyAnalyzerResult(aiqParams->mIsppOtherParams, false);
        else
            mCamHw->applyAnalyzerResult(aiqParams->mIsppOtherParams, true);
    }

    if (aiqParams->mIsppMeasParams.ptr()) {
        aiqParams->mIsppMeasParams->setType(RESULT_TYPE_ISPP_MEAS);
        if (_state == AIQ_STATE_STARTED)
            mCamHw->applyAnalyzerResult(aiqParams->mIsppMeasParams, false);
        else
            mCamHw->applyAnalyzerResult(aiqParams->mIsppMeasParams, true);
    }
#endif
set_ispp_end:

    if (aiqParams->mFocusParams.ptr()) {
        aiqParams->mFocusParams->setType(RESULT_TYPE_AF_PARAM);
        mCamHw->applyAnalyzerResult(aiqParams->mFocusParams, false);
    }

// disable this feature now, this require the hdr mode set to auto
#if 0
    // switch working mode by gray_mode ?
    if (aiqParams->mIspMeasParams.ptr()) {
        SmartPtr<rk_aiq_isp_params_t> isp_params = aiqParams->mIspMeasParams->data();
        LOGD_ANALYZER("ie mode %d, mWkSwitching %d, mWorkingMode %d, mOldWkModeForGray %d",
                      isp_params->ie.base.mode, mWkSwitching, mWorkingMode, mOldWkModeForGray);
        if (isp_params->ie.base.mode == RK_AIQ_IE_EFFECT_BW &&
                mWorkingMode != RK_AIQ_WORKING_MODE_NORMAL && !mWkSwitching) {
            mOldWkModeForGray = mWorkingMode;
            mWkSwitching = true;
            LOGD_ANALYZER("switch to BW, old mode %d", mOldWkModeForGray);
            SmartPtr<RkAiqMngCmdThread::msg_t> msg = new RkAiqMngCmdThread::msg_t();
            msg->cmd = RkAiqMngCmdThread::MSG_CMD_SW_WORKING_MODE;
            msg->sync = false;
            msg->data.sw_wk_mode.mode = RK_AIQ_WORKING_MODE_NORMAL;
            mAiqMngCmdTh->send_cmd(msg);
        } else if (isp_params->ie.base.mode != RK_AIQ_IE_EFFECT_BW &&
                   mOldWkModeForGray != RK_AIQ_WORKING_MODE_NORMAL && !mWkSwitching) {
            LOGD_ANALYZER("switch to color, old mode %d", mOldWkModeForGray);
            mWkSwitching = true;
            SmartPtr<RkAiqMngCmdThread::msg_t> msg = new RkAiqMngCmdThread::msg_t();
            msg->cmd = RkAiqMngCmdThread::MSG_CMD_SW_WORKING_MODE;
            msg->sync = false;
            msg->data.sw_wk_mode.mode = mOldWkModeForGray;
            mAiqMngCmdTh->send_cmd(msg);
            mOldWkModeForGray = RK_AIQ_WORKING_MODE_NORMAL;
            LOGD_ANALYZER("done switch to color, old mode %d", mOldWkModeForGray);
        }
    }
#endif
    EXIT_XCORE_FUNCTION();
out:
    return ret;
}
#else
/* TODO: add other features */
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
        aiqParams->mExposureParams->setType(RESULT_TYPE_EXPOSURE);
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

    APPLY_ANALYZER_RESULT(Aec, AEC);
    APPLY_ANALYZER_RESULT(Hist, HIST);
    APPLY_ANALYZER_RESULT(Awb, AWB);
    APPLY_ANALYZER_RESULT(AwbGain, AWBGAIN);
    APPLY_ANALYZER_RESULT(Af, AF);
    APPLY_ANALYZER_RESULT(Dpcc, DPCC);
    APPLY_ANALYZER_RESULT(Merge, MERGE);
    APPLY_ANALYZER_RESULT(Tmo, TMO);
    APPLY_ANALYZER_RESULT(Ccm, CCM);
    APPLY_ANALYZER_RESULT(Lsc, LSC);
    APPLY_ANALYZER_RESULT(Blc, BLC);
    APPLY_ANALYZER_RESULT(Rawnr, RAWNR);
    APPLY_ANALYZER_RESULT(Gic, GIC);
    APPLY_ANALYZER_RESULT(Debayer, DEBAYER);
    APPLY_ANALYZER_RESULT(Ldch, LDCH);
    APPLY_ANALYZER_RESULT(Lut3d, LUT3D);
    APPLY_ANALYZER_RESULT(Dehaze, DEHAZE);
    APPLY_ANALYZER_RESULT(Agamma, AGAMMA);
    APPLY_ANALYZER_RESULT(Adegamma, ADEGAMMA);
    APPLY_ANALYZER_RESULT(Wdr, WDR);
    APPLY_ANALYZER_RESULT(Csm, CSM);
    APPLY_ANALYZER_RESULT(Cgc, CGC);
    APPLY_ANALYZER_RESULT(Conv422, CONV422);
    APPLY_ANALYZER_RESULT(Yuvconv, YUVCONV);
    APPLY_ANALYZER_RESULT(Gain, GAIN);
    APPLY_ANALYZER_RESULT(Cp, CP);
    APPLY_ANALYZER_RESULT(Ie, IE);
    APPLY_ANALYZER_RESULT(Motion, MOTION);

    APPLY_ANALYZER_RESULT(Tnr, TNR);
    APPLY_ANALYZER_RESULT(Ynr, YNR);
    APPLY_ANALYZER_RESULT(Uvnr, UVNR);
    APPLY_ANALYZER_RESULT(Sharpen, SHARPEN);
    APPLY_ANALYZER_RESULT(Edgeflt, EDGEFLT);
    APPLY_ANALYZER_RESULT(Fec, FEC);
    APPLY_ANALYZER_RESULT(Orb, ORB);
    // ispv21
    APPLY_ANALYZER_RESULT(DrcV21, DRC);
    APPLY_ANALYZER_RESULT(AwbV21, AWB);
    APPLY_ANALYZER_RESULT(BlcV21, BLC);
    APPLY_ANALYZER_RESULT(GicV21, GIC);
    APPLY_ANALYZER_RESULT(DehazeV21, DEHAZE);
    APPLY_ANALYZER_RESULT(YnrV21, YNR);
    APPLY_ANALYZER_RESULT(CnrV21, UVNR);
    APPLY_ANALYZER_RESULT(SharpenV21, SHARPEN);
    APPLY_ANALYZER_RESULT(BaynrV21, RAWNR);

    mCamHw->applyAnalyzerResult(results_list);

    EXIT_XCORE_FUNCTION();

    return ret;
}

#endif

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

    ret = mRkLumaAnalyzer->stop();
    RKAIQMNG_CHECK_RET(ret, "luma analyzer stop error %d", ret);

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

    ret = mRkLumaAnalyzer->start();
    RKAIQMNG_CHECK_RET(ret, "luma analyzer start error %d", ret);

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
#endif
}

CamCalibDbV2Context_t* RkAiqManager::getCurrentCalibDBV2()
{
  return mCalibDbV2;
}

XCamReturn RkAiqManager::calibTuning(const CamCalibDbV2Context_t* aiqCalib,
                                     ModuleNameList& change_list)
{
  if (!aiqCalib) {
    return XCAM_RETURN_ERROR_PARAM;
  }
  *mCalibDbV2 = *aiqCalib;

  mCamHw->setCalib(mCalibDbV2);

  return mRkAiqAnalyzer->calibTuning(aiqCalib, change_list);
}

}; //namespace RkCam
