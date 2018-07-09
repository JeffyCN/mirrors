/*
 * isp_controller.cpp - isp controller
 *
 *  Copyright (c) 2014-2015 Intel Corporation
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#include "isp_controller.h"
#include "v4l2_device.h"
#include "x3a_statistics_queue.h"
#include "x3a_isp_config.h"

#include <linux/rkisp.h>
#include <rkiq_params.h>

namespace XCam {

IspController::IspController ():
    _is_exit(false),
    _device(NULL),
    _isp_device(NULL),
    _sensor_subdev(NULL),
    _isp_stats_device(NULL),
    _isp_params_device(NULL),
    _isp_ioctl(NULL),
    _frame_sequence(0),
    _frame_sof_time(0),
    _ae_stats_delay(-1)
{
    xcam_mem_clear(_last_aiq_results);

    _max_delay = EXPOSURE_GAIN_DELAY > EXPOSURE_TIME_DELAY ?
                    EXPOSURE_GAIN_DELAY : EXPOSURE_TIME_DELAY;

    _exposure_queue = 
        (struct rkisp_exposure *)xcam_malloc0(sizeof(struct rkisp_exposure) * _max_delay);

    XCAM_LOG_DEBUG ("IspController construction");
}

IspController::~IspController ()
{
    XCAM_LOG_DEBUG ("~IspController destruction");
    free(_exposure_queue);
}

void IspController::exit() {
    XCAM_LOG_DEBUG("ISP controller has exit");
    _is_exit = true;
}

void
IspController::set_isp_device(SmartPtr<V4l2Device> &dev) {
    _isp_device = dev;
    _isp_ioctl = new V4l2Isp10Ioctl(_isp_device->get_fd());
}

void
IspController::set_video_device(SmartPtr<V4l2Device> &dev) {
    _device = dev;
};

void
IspController::set_sensor_subdev (SmartPtr<V4l2SubDevice> &subdev) {
    _sensor_subdev = subdev;
};

void
IspController::set_vcm_subdev(SmartPtr<V4l2SubDevice> &subdev) {
    _vcm_device = subdev;
};

void
IspController::set_isp_stats_device(SmartPtr<V4l2Device> &dev) {
    _isp_stats_device = dev;
};

void
IspController::set_isp_params_device(SmartPtr<V4l2Device> &dev) {
    _isp_params_device = dev;
};

#if RKISP
XCamReturn
IspController::handle_sof(int64_t time, int frameid)
{
    SmartLock locker (_mutex);
    _frame_sequence_cond.signal();

    _frame_sof_time = time;
    _frame_sequence = frameid;

    char log_str[1024];
    int num = 0;
    for(int i=0; i<EXPOSURE_TIME_DELAY; i++) {
        num += sprintf(log_str + num, "        |||queue(%d) (%d-%d) expsync\n",
                    i,
                    _exposure_queue[i].coarse_integration_time,
                    _exposure_queue[i].analog_gain);
    }

    XCAM_LOG_DEBUG("--SOF[%d]------------------expsync-statsync\n%s", frameid, log_str);

    struct rkisp_exposure exposure;
    exposure.coarse_integration_time = _exposure_queue[EXPOSURE_TIME_DELAY - 1].coarse_integration_time;
    exposure.analog_gain = _exposure_queue[EXPOSURE_GAIN_DELAY - 1].analog_gain;
    exposure.digital_gain = _exposure_queue[EXPOSURE_GAIN_DELAY - 1].digital_gain;

    set_3a_exposure(exposure);

    return XCAM_RETURN_NO_ERROR;
}

int
IspController::get_format(rk_aiq_exposure_sensor_descriptor* sensor_desc)
{
    struct v4l2_subdev_format fmt;

    memset(&fmt, 0, sizeof(fmt));
    fmt.pad = 0;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;

    if (_sensor_subdev->io_control(VIDIOC_SUBDEV_G_FMT, &fmt) < 0)
        return -errno;

    sensor_desc->sensor_output_width = fmt.format.width;
    sensor_desc->sensor_output_height = fmt.format.height;

    sensor_desc->isp_input_width= fmt.format.width;
    sensor_desc->isp_input_height= fmt.format.height;

    return 0;
}

int
IspController::get_exposure_range(rk_aiq_exposure_sensor_descriptor* sensor_desc)
{
    struct v4l2_queryctrl ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE;

    if (_sensor_subdev->io_control(VIDIOC_QUERYCTRL, &ctrl) < 0)
        return -errno;

    sensor_desc->coarse_integration_time_min = ctrl.minimum;
    sensor_desc->coarse_integration_time_max_margin = 10;

    return 0;
}

int
IspController::get_blank(rk_aiq_exposure_sensor_descriptor* sensor_desc)
{
    struct v4l2_queryctrl ctrl;
    int horzBlank, vertBlank;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HBLANK;
    if (_sensor_subdev->io_control(VIDIOC_QUERYCTRL, &ctrl) < 0) {
        return -errno;
    }
    horzBlank = ctrl.minimum;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    if (_sensor_subdev->io_control(VIDIOC_QUERYCTRL, &ctrl) < 0) {
        return -errno;
    }
    vertBlank = ctrl.minimum;

    sensor_desc->pixel_periods_per_line = horzBlank + sensor_desc->sensor_output_width;
    sensor_desc->line_periods_per_field = vertBlank + sensor_desc->sensor_output_height;
    sensor_desc->line_periods_vertical_blanking = vertBlank;

    //INFO: fine integration is not supported by v4l2
    sensor_desc->fine_integration_time_min = 0;
    sensor_desc->fine_integration_time_max_margin = sensor_desc->pixel_periods_per_line;

    return 0;
}

int
IspController::get_pixel(rk_aiq_exposure_sensor_descriptor* sensor_desc)
{
    struct v4l2_ext_controls controls;
    struct v4l2_ext_control ext_control;
    signed long pixel;

    memset(&controls, 0, sizeof(controls));
    memset(&ext_control, 0, sizeof(ext_control));

    ext_control.id = V4L2_CID_PIXEL_RATE;
    controls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_control.id);
    controls.count = 1;
    controls.controls = &ext_control;

    if (_sensor_subdev->io_control(VIDIOC_G_EXT_CTRLS, &controls) < 0)
        return -errno;

    pixel = ext_control.value64;

    sensor_desc->pixel_clock_freq_mhz = (float)pixel / 1000000;

    return 0;
}

XCamReturn
IspController::get_sensor_descriptor (rk_aiq_exposure_sensor_descriptor *sensor_desc)
{
    memset(sensor_desc, 0, sizeof(rk_aiq_exposure_sensor_descriptor));

    if (get_format(sensor_desc))
        return XCAM_RETURN_ERROR_IOCTL;

    if (get_blank(sensor_desc))
        return XCAM_RETURN_ERROR_IOCTL;

    if (get_pixel(sensor_desc))
        return XCAM_RETURN_ERROR_IOCTL;

    if (get_exposure_range(sensor_desc))
        return XCAM_RETURN_ERROR_IOCTL;

    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn
IspController::get_sensor_mode_data (struct isp_supplemental_sensor_mode_data &sensor_mode_data)
{
    if (_is_exit)
        return XCAM_RETURN_BYPASS;

    if (_device.ptr()) {
        if(_device->io_control(RK_VIDIOC_SENSOR_MODE_DATA, &sensor_mode_data) < 0) {
            XCAM_LOG_WARNING ("get ISP sensor mode data failed (err[%s]), use initialized sensor mode data", strerror(errno));
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

#if RKISP
    if (_sensor_subdev.ptr()){
        rk_aiq_exposure_sensor_descriptor sensor_desc;
        get_sensor_descriptor (&sensor_desc);

        sensor_mode_data.coarse_integration_time_min        =
            sensor_desc.coarse_integration_time_min;
        sensor_mode_data.coarse_integration_time_max_margin =
            sensor_desc.coarse_integration_time_max_margin;
        sensor_mode_data.fine_integration_time_min          =
            sensor_desc.fine_integration_time_min;
        sensor_mode_data.fine_integration_time_max_margin   =
            sensor_desc.fine_integration_time_max_margin;

        sensor_mode_data.frame_length_lines                 =
            sensor_desc.line_periods_per_field;
        sensor_mode_data.line_length_pck                    =
            sensor_desc.pixel_periods_per_line;
        sensor_mode_data.vt_pix_clk_freq_hz                 =
            sensor_desc.pixel_clock_freq_mhz * 1000000;

        sensor_mode_data.sensor_output_width                =
            sensor_desc.sensor_output_width;
        sensor_mode_data.sensor_output_height               =
            sensor_desc.sensor_output_height;
        sensor_mode_data.isp_input_width                    =
            sensor_desc.isp_input_width;
        sensor_mode_data.isp_input_height                   =
            sensor_desc.isp_input_height;
        sensor_mode_data.isp_output_width                   =
            sensor_desc.isp_output_width;
        sensor_mode_data.isp_output_height                  =
            sensor_desc.isp_output_height;

        sensor_mode_data.exposure_valid_frame[0]            =
            EXPOSURE_TIME_DELAY;
        {
            SmartLock locker (_mutex);
            sensor_mode_data.exp_time                           =
                _exposure_queue[0].coarse_integration_time;
            sensor_mode_data.gain                               =
                _exposure_queue[0].analog_gain;
        }
        XCAM_LOG_DEBUG("|||sensormode (%d-%d) expsync\n",
            sensor_mode_data.exp_time,
            sensor_mode_data.gain);

    }
#endif

	return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspController::get_isp_parameter (struct rkisp_parm &parameters)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspController::get_3a_statistics (SmartPtr<X3aIspStatistics> &stats)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (_is_exit)
        return XCAM_RETURN_BYPASS;

    if (_isp_device.ptr()) {
        SmartPtr<V4l2Buffer> v4l2buf;
        struct cifisp_stat_buffer *isp_stats = NULL;

        XCAM_ASSERT (stats.ptr());
        XCAM_FAIL_RETURN (WARNING, stats.ptr(),
                          XCAM_RETURN_ERROR_PARAM, "stats empty");

        isp_stats =  (struct cifisp_stat_buffer*)stats->get_isp_stats ();

        ret = _isp_device->dequeue_buffer (v4l2buf);
        if (ret != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("capture buffer failed");
            return ret;
        }
        XCAM_ASSERT (v4l2buf.ptr());

        memcpy(isp_stats, v4l2buf->map(), sizeof(struct cifisp_stat_buffer));

        _isp_device->queue_buffer (v4l2buf);
    }

#if RKISP
    if (_isp_stats_device.ptr()) {
        SmartPtr<V4l2Buffer> v4l2buf;
        struct cifisp_stat_buffer* isp_stats = NULL;

        isp_stats =  (struct cifisp_stat_buffer*)stats->get_isp_stats ();
        ret = _isp_stats_device->dequeue_buffer (v4l2buf);
        if (ret != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("dequeue stats buffer failed");
            return ret;
        }
        XCAM_ASSERT (v4l2buf.ptr());

        int cur_frame_id = v4l2buf->get_buf().sequence;
        int64_t cur_time = v4l2buf->get_buf().timestamp.tv_sec * 1000 * 1000 * 1000 +
                            v4l2buf->get_buf().timestamp.tv_usec * 1000;

        //translate stats to struct cifisp_stat_buffer
        struct cifisp_stat_buffer *aiq_stats = (struct cifisp_stat_buffer*)v4l2buf->map();
        isp_stats->params.ae = aiq_stats->params.ae;
        isp_stats->params.hist= aiq_stats->params.hist;
        isp_stats->params.awb = aiq_stats->params.awb;
        isp_stats->params.af = aiq_stats->params.af;

        ret = _isp_stats_device->queue_buffer (v4l2buf);
        if (ret != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("queue stats buffer failed");
            return ret;
        }

        XCAM_LOG_DEBUG("|||get_3a_statistics[%d-%d] MEAS AE: %d MEAS AWB[%d] [%d-%d-%d] expsync",
            cur_frame_id,
            _frame_sequence,
            isp_stats->params.ae.exp_mean[0],
            isp_stats->params.awb.awb_mean[0].cnt,
            isp_stats->params.awb.awb_mean[0].mean_y_or_g,
            isp_stats->params.awb.awb_mean[0].mean_cr_or_r,
            isp_stats->params.awb.awb_mean[0].mean_cb_or_b);

        XCAM_LOG_DEBUG("stats event sequence: [%d-%d], time: [%ld-%ld], during: %ld - statsync",
            _frame_sequence, cur_frame_id,
            _frame_sof_time, cur_time,
            cur_time - _frame_sof_time);

resync:
        if (_frame_sequence < cur_frame_id) {
            // impossible case
            XCAM_LOG_DEBUG("[%d-%d] wait for next SOF signal[exit: %d] - statsync",
                _frame_sequence, cur_frame_id, _is_exit);
            if (!_is_exit) {
                SmartLock locker (_mutex);
                _frame_sequence_cond.timedwait(_mutex, 3000); //3ms
                goto resync;
            } else {
                return XCAM_RETURN_BYPASS;
            }
        } else if (_frame_sequence > cur_frame_id) {
            if ( cur_time - _frame_sof_time < 10 * 1000 * 1000) {
                XCAM_LOG_DEBUG("measurement late %lld for frame %d - statsync",
                    _frame_sof_time - cur_time, cur_frame_id);
                // only one delayed stats is allowed
                if (_ae_stats_delay != -1)
                    XCAM_LOG_ERROR("busy ! one stats has been already dealyed !");
                _ae_stats_delay = _frame_sequence;
            } else {
                XCAM_LOG_ERROR(" stats comes late over 10ms than sof !");
            }
        }
    }
#endif

    return ret;
}

XCamReturn
IspController::get_frame_softime (int64_t &sof_tim)
{
    sof_tim = _frame_sof_time;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspController::get_vcm_time (struct rk_cam_vcm_tim *vcm_tim)
{
#if RKISP
    if (_vcm_device.ptr()) {
        if (_vcm_device->io_control (RK_VIDIOC_VCM_TIMEINFO, vcm_tim) < 0) {
            XCAM_LOG_ERROR (" get RK_VIDIOC_VCM_TIMEINFO failed. cmd = 0x%x", RK_VIDIOC_VCM_TIMEINFO);
            return XCAM_RETURN_ERROR_IOCTL;
        }
        XCAM_LOG_DEBUG("get RK_VIDIOC_VCM_TIMEINFO vcm_tim 0x%lx, 0x%lx, 0x%lx, 0x%lx",
            vcm_tim->vcm_start_t.tv_sec, vcm_tim->vcm_start_t.tv_usec,
            vcm_tim->vcm_end_t.tv_sec, vcm_tim->vcm_end_t.tv_usec);
    }
#endif
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
IspController::set_3a_config (X3aIspConfig *config)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (_is_exit) {
        XCAM_LOG_DEBUG ("set 3a config bypass since ia engine has stop");
        return XCAM_RETURN_BYPASS;
    }
    struct rkisp_parameters &isp_config = config->get_isp_configs ();
    struct rkisp_parameters *isp_cfg = &isp_config;

    XCAM_ASSERT (isp_cfg);

#if RKISP
    if (_isp_params_device.ptr()) {
        struct rkisp1_isp_params_cfg* isp_params;
        struct v4l2_buffer v4l2buf = _isp_params_device->get_buffer_by_index(0);
        int sequence = v4l2buf.sequence;
        if (0)
            printf("params buf.sequence: %d\n", sequence);

        isp_params = (struct rkisp1_isp_params_cfg*)v4l2buf.m.userptr;
        memset(isp_params, 0, sizeof(struct rkisp1_isp_params_cfg));
        //rkisp1_convert_params(isp_params, &rkisp1_core->aiq_results);
        ret = rkisp1_convert_results(isp_params,isp_cfg, _last_aiq_results);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE("rkisp1_convert_results error\n");
        }

        dump_isp_config(isp_params, isp_cfg);

        ret = rkisp1_check_params(isp_params);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE("rkisp1_check_params error\n");
            return XCAM_RETURN_ERROR_PARAM;
        }

        /* apply isp_params */
        xcam_mem_clear(v4l2buf);
        v4l2buf.type = V4L2_BUF_TYPE_META_OUTPUT;
        v4l2buf.memory = V4L2_MEMORY_MMAP;

        /* params should use one buffers */
        v4l2buf.index = 0;
        if (_isp_params_device->io_control(VIDIOC_QBUF, &v4l2buf) != 0) {
            printf("RKISP1: failed to ioctl VIDIOC_QBUF for %d %s.\n",
                   errno, strerror(errno));
            return ret;
        }

        XCAM_LOG_DEBUG ("device(%s) dequeue buffer, check exit status again[exit: %d]",
            XCAM_STR (_isp_params_device->get_device_name()), _is_exit);
        if (_is_exit)
            return XCAM_RETURN_BYPASS;

        /* wait params done, this may block the thread */
        if (_isp_params_device->io_control(VIDIOC_DQBUF, &v4l2buf) != 0) {
            printf("RKISP1: failed to ioctl VIDIOC_DQBUF for %d %s.\n",
                   errno, strerror(errno));
            return ret;
        }

    }
#endif

#if RK_ISP10
    if (_isp_ioctl.ptr()) {

        if (isp_cfg->active_configs & ISP_BPC_MASK) {
            if (_isp_ioctl->setDpccCfg(
                        isp_cfg->dpcc_config,
                        isp_cfg->enabled[HAL_ISP_BPC_ID]) < 0) {
                LOGE("%s: setDpccCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_BLS_MASK) {
            if (_isp_ioctl->setBlsCfg(
                        isp_cfg->bls_config,
                        isp_cfg->enabled[HAL_ISP_BLS_ID]) < 0) {
                LOGE("%s: setBlsCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_SDG_MASK) {
            if (_isp_ioctl->setSdgCfg(
                        isp_cfg->sdg_config,
                        isp_cfg->enabled[HAL_ISP_SDG_ID]) < 0) {
                LOGE("%s: setSdgCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_HST_MASK) {
            if (_isp_ioctl->setHstCfg(
                        isp_cfg->hst_config,
                        isp_cfg->enabled[HAL_ISP_HST_ID]) < 0) {
                LOGE("%s: setHstCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_LSC_MASK) {
            if (_isp_ioctl->setLscCfg(
                        isp_cfg->lsc_config,
                        isp_cfg->enabled[HAL_ISP_LSC_ID]) < 0) {
                LOGE("%s: setLscCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_AWB_MEAS_MASK) {
            if (_isp_ioctl->setAwbMeasCfg(
                        isp_cfg->awb_meas_config,
                        isp_cfg->enabled[HAL_ISP_AWB_MEAS_ID]) < 0) {
                LOGE("%s: setAwbMeasCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_AWB_GAIN_MASK) {
            if (_isp_ioctl->setAwbGainCfg(
                        isp_cfg->awb_gain_config,
                        isp_cfg->enabled[HAL_ISP_AWB_GAIN_ID]) < 0) {
                LOGE("%s: setAwbGainCfg failed", __func__);
            }
        }
        if (isp_cfg->active_configs & ISP_FLT_MASK) {
            if (_isp_ioctl->setFltCfg(
                        isp_cfg->flt_config,
                        isp_cfg->enabled[HAL_ISP_FLT_ID]) < 0) {
                LOGE("%s: setFltCfg failed", __func__);
            }
        }
        if (isp_cfg->active_configs & ISP_BDM_MASK) {
            if (_isp_ioctl->setBdmCfg(
                        isp_cfg->bdm_config,
                        isp_cfg->enabled[HAL_ISP_BDM_ID]) < 0) {
                LOGE("%s: setBdmCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_CTK_MASK) {
            if (_isp_ioctl->setCtkCfg(
                        isp_cfg->ctk_config,
                        isp_cfg->enabled[HAL_ISP_CTK_ID]) < 0) {
                LOGE("%s: setCtkCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_GOC_MASK) {
            if (_isp_ioctl->setGocCfg(
                        isp_cfg->goc_config,
                        isp_cfg->enabled[HAL_ISP_GOC_ID]) < 0) {
                LOGE("%s: setGocCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_CPROC_MASK) {
            if (_isp_ioctl->setCprocCfg(
                        isp_cfg->cproc_config,
                        isp_cfg->enabled[HAL_ISP_CPROC_ID]) < 0) {
                LOGE("%s: setCprocCfg failed", __func__);
            }
            LOGV("%s:apply cproc config!enabled %d",
                  __func__, isp_cfg->enabled[HAL_ISP_CPROC_ID]
                 );
        }

        if (isp_cfg->active_configs & ISP_AEC_MASK) {
            if (_isp_ioctl->setAecCfg(
                        isp_cfg->aec_config,
                        isp_cfg->enabled[HAL_ISP_AEC_ID]) < 0) {
                LOGE("%s: setAecCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_AFC_MASK) {
            if (_isp_ioctl->setAfcCfg(
                        isp_cfg->afc_config,
                        isp_cfg->enabled[HAL_ISP_AFC_ID]) < 0) {
                LOGE("%s: setAfcCfg failed", __func__);
            }
        }

        if (isp_cfg->active_configs & ISP_IE_MASK) {
            if (_isp_ioctl->setIeCfg(
                        isp_cfg->ie_config,
                        isp_cfg->enabled[HAL_ISP_IE_ID]) < 0) {
                LOGE("%s: setIeCfg failed", __func__);
            }
            LOGV("%s:apply ie config,enabled %d!", __func__,
                  isp_cfg->enabled[HAL_ISP_IE_ID]);
        }

        if (isp_cfg->active_configs & ISP_DPF_MASK) {
            if (!_isp_ioctl->setDpfCfg(
                        isp_cfg->dpf_config,
                        isp_cfg->enabled[HAL_ISP_DPF_ID])) {
                LOGE("%s: setDpfCfg failed, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
                      __func__,
                      isp_cfg->dpf_config.gain.mode,
                      isp_cfg->dpf_config.gain.nf_b_gain,
                      isp_cfg->dpf_config.gain.nf_gb_gain,
                      isp_cfg->dpf_config.gain.nf_gr_gain,
                      isp_cfg->dpf_config.gain.nf_r_gain);
            }
        }

        if (isp_cfg->active_configs & ISP_DPF_STRENGTH_MASK) {
            if (_isp_ioctl->setDpfStrengthCfg(
                        isp_cfg->dpf_strength_config,
                        isp_cfg->enabled[HAL_ISP_DPF_STRENGTH_ID]) < 0) {
                LOGE("%s: setDpfStrengthCfg failed", __func__);
            }
        }
    }
#endif

    XCAM_LOG_DEBUG ("   set_3a_config done\n");

    return XCAM_RETURN_NO_ERROR;
}

void
IspController::exposure_delay(struct rkisp_exposure isp_exposure)
{
    int i = 0;

    SmartLock locker (_mutex);
    for (i = 0; i < _max_delay - 1; ++i) {
        _exposure_queue[i] = _exposure_queue[i + 1];
    }
    _exposure_queue[_max_delay - 1] = isp_exposure;
    for(int i=0; i<3; i++) {
        XCAM_LOG_DEBUG("|||exp queue(%d) (%d-%d)\n",
            i,
            _exposure_queue[i].coarse_integration_time,
            _exposure_queue[i].analog_gain);
    }
    /* if missing the sof, update immediately */
    if (_ae_stats_delay != -1) {
        XCAM_LOG_DEBUG ("set exposure for delay stats %u immediately !", _ae_stats_delay);
        set_3a_exposure(isp_exposure);
        _ae_stats_delay = -1;
    }
}

void
IspController::push_3a_exposure (X3aIspExposureResult *res)
{
    const struct rkisp_exposure &exposure = res->get_isp_config ();
    push_3a_exposure(exposure);
}

void
IspController::push_3a_exposure (struct rkisp_exposure isp_exposure)
{
    exposure_delay(isp_exposure);
}

XCamReturn
IspController::set_3a_exposure (X3aIspExposureResult *res)
{
    const struct rkisp_exposure &exposure = res->get_isp_config ();
    return set_3a_exposure (exposure);
}

XCamReturn
IspController::set_3a_exposure (struct rkisp_exposure isp_exposure)
{
    if (_is_exit)
        return XCAM_RETURN_BYPASS;

    LOGD("----------------------------------------------");
    LOGD("|||set_3a_exposure (%d-%d) expsync in sof %d\n",
        isp_exposure.coarse_integration_time,
        isp_exposure.analog_gain,
        _frame_sequence);

    if (_device.ptr()) {
        struct v4l2_ext_control exp_gain[3];
        struct v4l2_ext_controls ctrls;

        exp_gain[0].id = V4L2_CID_EXPOSURE;

        exp_gain[0].value = isp_exposure.coarse_integration_time;
        exp_gain[1].id = V4L2_CID_GAIN;
        exp_gain[1].value = isp_exposure.analog_gain;
        exp_gain[2].id = RK_V4L2_CID_GAIN_PERCENT;
        exp_gain[2].value = 100;

        ctrls.count = 3;
        ctrls.ctrl_class = V4L2_CTRL_CLASS_USER;
        ctrls.controls = exp_gain;
        ctrls.reserved[0] = 0;
        ctrls.reserved[1] = 0;

        if ( _device->io_control (VIDIOC_S_EXT_CTRLS, &ctrls) < 0) {
            XCAM_LOG_WARNING (" set exposure result failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    } else {
        struct v4l2_control ctrl;

        if (isp_exposure.analog_gain!= 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id = V4L2_CID_ANALOGUE_GAIN;
            ctrl.value = isp_exposure.analog_gain;
            if (_sensor_subdev->io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                XCAM_LOG_ERROR ("failed to  set again result(val: %d)", isp_exposure.analog_gain);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
        if (isp_exposure.digital_gain!= 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id = V4L2_CID_GAIN;
            ctrl.value = isp_exposure.digital_gain;
            if (_sensor_subdev->io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                XCAM_LOG_ERROR ("failed to set dgain result(val: %d)", isp_exposure.digital_gain);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }

        // set vts before exposure time firstly
        rk_aiq_exposure_sensor_descriptor sensor_desc;
        get_sensor_descriptor (&sensor_desc);
        if (sensor_desc.line_periods_per_field < isp_exposure.coarse_integration_time) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id = V4L2_CID_VBLANK;
            /* TODO: havn't consider the margin */
            ctrl.value = isp_exposure.coarse_integration_time - sensor_desc.sensor_output_width;
            if (_sensor_subdev->io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                XCAM_LOG_ERROR ("failed to set vblank result(val: %d)", ctrl.value);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }

        if (isp_exposure.coarse_integration_time!= 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id = V4L2_CID_EXPOSURE;
            ctrl.value = isp_exposure.coarse_integration_time;
            if (_sensor_subdev->io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                XCAM_LOG_ERROR ("failed to set integration time result(val: %d)", isp_exposure.coarse_integration_time);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    LOGD("set_3a_exposure done");

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspController::set_3a_focus (X3aIspFocusResult *res)
{
    const struct rkisp_focus &focus = res->get_isp_config ();

#if RKISP
    int position = focus.next_lens_position;
    struct v4l2_control control;

    xcam_mem_clear (control);
    control.id = V4L2_CID_FOCUS_ABSOLUTE;
    control.value = position;

    if (_vcm_device.ptr()) {
        XCAM_LOG_DEBUG ("|||set focus result: %d", focus.next_lens_position);
        if (_vcm_device->io_control (VIDIOC_S_CTRL, &control) < 0) {
            XCAM_LOG_ERROR (" set focus result failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }
#endif
    return XCAM_RETURN_NO_ERROR;
}

#if RKISP
void
IspController::dump_isp_config(struct rkisp1_isp_params_cfg* isp_params,
                            struct rkisp_parameters *isp_cfg) {
    
    XCAM_LOG_DEBUG("-------------------------------------------------------\n \
            |||set_3a_config rkisp1_isp_params_cfg size: %d, meas: %d, others: %d\n  \
               module enable mask - update: %x - %x, cfg update: %x\n \
               awb config - rbgain: %d - %d, y: %d - %d\n \
               coeff config: [%d-%d-%d-%d-%d-%d-%d-%d-%d], offset: [%d-%d-%d]\n \
               aec window[%d-%d-%d-%d]\n \
               hst mode: %d, predivider: %d, window[%d-%d-%d-%d]\n \
               bdm 10bit bypass: %d, demosaic_th: %d",
        sizeof (struct rkisp1_isp_params_cfg),
        sizeof (struct cifisp_isp_meas_cfg),
        sizeof (struct cifisp_isp_other_cfg),
        
        isp_params->module_ens,
        isp_params->module_en_update,
        isp_params->module_cfg_update,
        
        isp_cfg->awb_gain_config.gain_red,
        isp_cfg->awb_gain_config.gain_blue,
        isp_cfg->awb_gain_config.gain_green_b,
        isp_cfg->awb_gain_config.gain_green_r,
        
        isp_params->others.ctk_config.coeff0,
        isp_params->others.ctk_config.coeff1,
        isp_params->others.ctk_config.coeff2,
        isp_params->others.ctk_config.coeff3,
        isp_params->others.ctk_config.coeff4,
        isp_params->others.ctk_config.coeff5,
        isp_params->others.ctk_config.coeff6,
        isp_params->others.ctk_config.coeff7,
        isp_params->others.ctk_config.coeff8,
        isp_params->others.ctk_config.ct_offset_r,
        isp_params->others.ctk_config.ct_offset_g,
        isp_params->others.ctk_config.ct_offset_b,

        isp_params->meas.aec_config.meas_window.h_offs,
        isp_params->meas.aec_config.meas_window.v_offs,
        isp_params->meas.aec_config.meas_window.h_size,
        isp_params->meas.aec_config.meas_window.v_size,

        isp_params->meas.hst_config.mode,
        isp_params->meas.hst_config.histogram_predivider,
        isp_params->meas.hst_config.meas_window.h_offs,
        isp_params->meas.hst_config.meas_window.v_offs,
        isp_params->meas.hst_config.meas_window.h_size,
        isp_params->meas.hst_config.meas_window.v_size,

        isp_params->module_ens & HAL_ISP_BDM_MASK,
        isp_params->others.bdm_config.demosaic_th);

}
#endif
};
