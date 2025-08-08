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

#include "CamHwSimulator.h"
#include "xcam_thread.h"
#include "isp20_hw_simulator.h"
// globals for communicating with simulator

static XCam::Mutex     g_sim_luma_mutex{false};
static XCam::Cond      g_sim_luma_cond{false};
static volatile bool   g_wait_luma = true;
static volatile bool   g_luma_terminal = false;
static volatile bool   g_luma_wait_result = true;
static XCam::Mutex     g_luma_mutex{false};
static XCam::Cond      g_luma_cond{false};

static XCam::Mutex     g_sim_mutex{false};
static XCam::Cond      g_sim_cond{false};
static volatile bool   g_wait_stats = true;
static volatile bool   g_terminal = false;
static volatile bool   g_wait_result = true;
static XCam::Mutex     g_3a_mutex{false};
static XCam::Cond      g_3a_cond{false};

// extern shared var from demo.cpp
extern rk_sim_isp_v200_luma_t g_simisp_luma;
extern rk_sim_isp_v200_stats_t g_simisp_stats;
extern rk_sim_isp_v200_params_t g_simisp_params;

int analyze_sim_luma()
{
// wake up FakePollThread
    //RkCam::CamHwSimulator* simctx = (RkCam::CamHwSimulator*)ctx;
    {
        SmartLock locker(g_sim_luma_mutex);
        g_wait_luma = false;
        SmartLock resultlocker(g_luma_mutex);
        g_luma_wait_result = true;
        LOGD("gsim broadcast...");
        g_sim_luma_cond.broadcast ();
    }
// wait result
    {
        LOGD("wait luma result ...");
        SmartLock locker(g_luma_mutex);
        if (g_luma_wait_result)
            g_luma_cond.wait(g_luma_mutex);
        LOGD("wait luma result over...");
    }
    return 0;
}

int analyze_sim_stats()
{
// wake up FakePollThread
    //RkCam::CamHwSimulator* simctx = (RkCam::CamHwSimulator*)ctx;
    {
        SmartLock locker(g_sim_mutex);
        g_wait_stats = false;
        SmartLock resultlocker(g_3a_mutex);
        g_wait_result = true;
        LOGD("gsim broadcast...");
        g_sim_cond.broadcast ();
    }
// wait result
    {
        LOGD("wait 3a result ...");
        SmartLock locker(g_3a_mutex);
        if (g_wait_result)
            g_3a_cond.wait(g_3a_mutex);
        LOGD("wait 3a result over...");
    }
    return 0;
}

namespace RkCam {
class FakePollLumaThread : public PollThread
{
public:
    FakePollLumaThread(): PollThread() {};
    virtual ~FakePollLumaThread() {};
protected:
    SmartPtr<VideoBuffer> new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                           SmartPtr<V4l2Device> dev,
                                           int type);
};

SmartPtr<VideoBuffer>
FakePollLumaThread::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                     SmartPtr<V4l2Device> dev,
                                     int type)
{
    {
        LOGD("wait sim luma ...");
        SmartLock locker(g_sim_luma_mutex);
        if (g_wait_luma && !g_luma_terminal) {
            g_sim_luma_cond.wait(g_sim_luma_mutex);
        }
        g_wait_luma = true;
        LOGD("wait sim luma over ...");
    }
    // TODO: convert simulator stats to video buffer
    SmartPtr<VideoBuffer> video_buf = new V4l2BufferProxy (buf, dev);

    return video_buf;
}

class FakePollThread : public PollThread
{
public:
    FakePollThread(): PollThread() {};
    virtual ~FakePollThread() {};
protected:
    SmartPtr<VideoBuffer> new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                           SmartPtr<V4l2Device> dev,
                                           int type);
};

SmartPtr<VideoBuffer>
FakePollThread::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                 SmartPtr<V4l2Device> dev,
                                 int type)
{
    {
        LOGD("wait sim stats ...");
        SmartLock locker(g_sim_mutex);
        if (g_wait_stats && !g_terminal) {
            g_sim_cond.wait(g_sim_mutex);
        }
        g_wait_stats = true;
        LOGD("wait sim stats over ...");
    }
    // TODO: convert simulator stats to video buffer
    SmartPtr<VideoBuffer> video_buf = new V4l2BufferProxy (buf, dev);

    return video_buf;
}

class FakeLumaV4l2Device : public V4l2Device
{
public:
    FakeLumaV4l2Device () : V4l2Device("/dev/null") {};
    virtual ~FakeLumaV4l2Device () {};
    virtual XCamReturn start () {
        _active = true;
        return XCAM_RETURN_NO_ERROR;
    };
    virtual XCamReturn stop () {
        _active = false;
        return XCAM_RETURN_NO_ERROR;
    };
    virtual XCamReturn dequeue_buffer (SmartPtr<V4l2Buffer> &buf) {
        ENTER_CAMHW_FUNCTION();
        g_simisp_luma.valid_luma = true;
        v4l2_buf.m.userptr = (unsigned long)&g_simisp_luma;
        buf = new V4l2Buffer(v4l2_buf, format);
        EXIT_CAMHW_FUNCTION();
        return XCAM_RETURN_NO_ERROR;
    };
    virtual XCamReturn queue_buffer (SmartPtr<V4l2Buffer> &buf) {
        ENTER_CAMHW_FUNCTION();
        EXIT_CAMHW_FUNCTION();
        return XCAM_RETURN_NO_ERROR;
    };
private:
    struct v4l2_buffer v4l2_buf;
    struct v4l2_format format;
};

class FakeV4l2Device : public V4l2Device
{
public:
    FakeV4l2Device () : V4l2Device("/dev/null") {};
    virtual ~FakeV4l2Device () {};
    virtual XCamReturn start () {
        _active = true;
        return XCAM_RETURN_NO_ERROR;
    };
    virtual XCamReturn stop () {
        _active = false;
        return XCAM_RETURN_NO_ERROR;
    };
    virtual XCamReturn dequeue_buffer (SmartPtr<V4l2Buffer> &buf) {
        ENTER_CAMHW_FUNCTION();
        g_simisp_stats.valid_awb = true;
        g_simisp_stats.valid_ae = true;
        g_simisp_stats.valid_af = true;
        v4l2_buf.m.userptr = (unsigned long)&g_simisp_stats;
        buf = new V4l2Buffer(v4l2_buf, format);
        EXIT_CAMHW_FUNCTION();
        return XCAM_RETURN_NO_ERROR;
    };
    virtual XCamReturn queue_buffer (SmartPtr<V4l2Buffer> &buf) {
        ENTER_CAMHW_FUNCTION();
        EXIT_CAMHW_FUNCTION();
        return XCAM_RETURN_NO_ERROR;
    };
private:
    struct v4l2_buffer v4l2_buf;
    struct v4l2_format format;
};

CamHwSimulator::CamHwSimulator()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

CamHwSimulator::~CamHwSimulator()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
CamHwSimulator::init(const char* sns_ent_name)
{
    ENTER_CAMHW_FUNCTION();
    mIspLumaDev = new FakeLumaV4l2Device ();
    mIspLumaDev->open();
    mPollLumathread = new FakePollLumaThread();
    mPollLumathread->set_isp_luma_device(mIspLumaDev);
    mPollLumathread->set_poll_callback (this);

    mIspStatsDev = new FakeV4l2Device ();
    mIspStatsDev->open();
    mPollthread = new FakePollThread();
    mPollthread->set_isp_stats_device(mIspStatsDev);
    mPollthread->set_poll_callback (this);
    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwSimulator::setIspParams(SmartPtr<RkAiqIspParamsProxy>& ispParams)
{
    ENTER_CAMHW_FUNCTION();
    CamHwBase::setIspParams(ispParams);
    // TODO: set new params to variable shared with simulator
    //awb
    g_simisp_params.awb_gain_algo = ispParams->data()->awb_gain;
    g_simisp_params.awb_hw0_para = ispParams->data()->awb_cfg_v200;
    g_simisp_params.awb_hw1_para = ispParams->data()->awb_cfg_v201;
    //g_simisp_params.awb_smooth_factor = ispParams->data()->awb_smooth_factor;
    LOGD("new awb threeDyuvIllu[2] %d ...", g_simisp_params.awb_hw0_para.threeDyuvIllu[2]);
    LOGD("new awb stat3aAwbGainOut.rgain %f ...", g_simisp_params.awb_gain_algo.rgain);
    //ae
    g_simisp_params.ae_hw_config.ae_meas = ispParams->data()->aec_meas;
    g_simisp_params.ae_hw_config.hist_meas = ispParams->data()->hist_meas;
    LOGE("return new AeHwConfig,rawae1.wnd_num=%d,rawae2.win.h_size=%d,rawhist2.weight[25]=%d",
         g_simisp_params.ae_hw_config.ae_meas.rawae1.wnd_num,
         g_simisp_params.ae_hw_config.ae_meas.rawae2.win.h_size,
         g_simisp_params.ae_hw_config.hist_meas.rawhist1.weight[25]);

#if 1
    g_simisp_params.rkaiq_anr_proc_res = ispParams->data()->rkaiq_anr_proc_res;
#endif

#if 1
    g_simisp_params.rkaiq_asharp_proc_res = ispParams->data()->rkaiq_asharp_proc_res;
#endif
    // adhaz
    g_simisp_params.adhaz_config = ispParams->data()->adhaz_config;

    //agamma
    g_simisp_params.agamma_config = ispParams->data()->agamma;

    g_simisp_params.blc = ispParams->data()->blc;

    //ahdr
    g_simisp_params.ahdr_proc_res = ispParams->data()->ahdr_proc_res;

    g_simisp_params.lscHwConf = ispParams->data()->lsc;
    g_simisp_params.ccmHwConf = ispParams->data()->ccm;

    g_simisp_params.dpcc = ispParams->data()->dpcc;

    g_simisp_params.lut3d_hw_conf = ispParams->data()->lut3d;

    //debayer
    g_simisp_params.adebayer_config = ispParams->data()->demosaic;
    //agic
    g_simisp_params.agic_config = ispParams->data()->gic;

    {
        SmartLock locker(g_3a_mutex);
        g_wait_result = false;
        g_3a_cond.broadcast ();
    }
    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwSimulator::setHdrProcessCount(uint32_t frame_id, rk_aiq_luma_params_t luma_params)
{
    ENTER_CAMHW_FUNCTION();

    //TODO::set HDR process count to register
    printf("lumatest set process count: %d\n", count);
    {
        SmartLock locker(g_luma_mutex);
        g_luma_wait_result = false;
        g_luma_cond.broadcast();
    }

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwSimulator::deInit()
{
    ENTER_CAMHW_FUNCTION();
    mIspStatsDev->close();
    mIspLumaDev->close();
    EXIT_CAMHW_FUNCTION();

    return CamHwBase::deInit();
}

XCamReturn
CamHwSimulator::prepare(uint32_t width, uint32_t height, rk_aiq_working_mode_t mode)
{
    return CamHwBase::prepare(width, height, mode, 0, 0);
}

XCamReturn
CamHwSimulator::start()
{
    ENTER_CAMHW_FUNCTION();
    mIspStatsDev->start();
    mIspLumaDev->start();
    EXIT_CAMHW_FUNCTION();

    return CamHwBase::start();
}

XCamReturn
CamHwSimulator::stop()
{
    ENTER_CAMHW_FUNCTION();
    {
        SmartLock locker(g_sim_mutex);
        g_wait_stats = false;
        g_terminal = true;
        g_sim_cond.broadcast ();

        g_wait_luma = false;
        g_luma_terminal = true;
        g_sim_luma_cond.broadcast ();
    }
    mIspStatsDev->stop();
    mIspLumaDev->stop();

    EXIT_CAMHW_FUNCTION();

    return CamHwBase::stop();
}

XCamReturn
CamHwSimulator::getSensorModeData(const char* sns_ent_name,
                                  rk_aiq_exposure_sensor_descriptor& sns_des)
{
    sns_des.sensor_output_width = g_simisp_params.rawWidth;
    sns_des.sensor_output_height = g_simisp_params.rawHeight;
    sns_des.isp_acq_width = g_simisp_params.rawWidth;
    sns_des.isp_acq_height = g_simisp_params.rawHeight;

    return XCAM_RETURN_NO_ERROR;
}
XCamReturn
CamHwSimulator::setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar)
{
    return CamHwBase::setExposureParams(expPar);
}

XCamReturn
CamHwSimulator::setFocusParams(SmartPtr<RkAiqFocusParamsProxy>& focus_params)
{
    return CamHwBase::setFocusParams(focus_params);
}

XCamReturn
CamHwSimulator::setIspLumaListener(IspLumaListener* lumaListener)
{
    return CamHwBase::setIspLumaListener(lumaListener);
}

XCamReturn
CamHwSimulator::setIspStatsListener(IspStatsListener* statsListener)
{
    return CamHwBase::setIspStatsListener(statsListener);
}

XCamReturn
CamHwSimulator::setEvtsListener(IspEvtsListener* evtListener)
{
    return CamHwBase::setEvtsListener(evtListener);
}

};
