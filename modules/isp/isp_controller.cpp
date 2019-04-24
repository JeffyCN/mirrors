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
    _frame_sequence(-(EXPOSURE_TIME_DELAY - 1)),
    _frame_sof_time(0),
    _isp_acq_out_width(-1),
    _isp_acq_out_height(-1)
{
    xcam_mem_clear(_last_aiq_results);
    xcam_mem_clear(_full_active_isp_params);
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

void IspController::exit(bool pause) {
    SmartLock locker (_mutex);
    XCAM_LOG_DEBUG("ISP controller has exit %d", pause);
    _is_exit = pause;
    _frame_sequence = -(EXPOSURE_TIME_DELAY - 1);
    _effecting_exposure_map.clear();
    _pending_ispparams_queue.clear();
    _effecting_ispparm_map.clear();
    _isp_acq_out_width = -1;
    _isp_acq_out_height = -1;
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

    //exposure.coarse_integration_time = _exposure_queue[EXPOSURE_TIME_DELAY - 1].coarse_integration_time;
    //exposure.analog_gain = _exposure_queue[EXPOSURE_GAIN_DELAY - 1].analog_gain;
    //exposure.digital_gain = _exposure_queue[EXPOSURE_GAIN_DELAY - 1].digital_gain;
    //exposure.frame_line_length = _exposure_queue[EXPOSURE_GAIN_DELAY - 1].frame_line_length;
    exposure = _exposure_queue[EXPOSURE_GAIN_DELAY - 1];
    exposure = _exposure_queue[_cur_apply_index++];
    if (_cur_apply_index == 3) {
        _cur_apply_index = 2;
        LOGD("no new expoure, use the latest !");
    }
    set_3a_exposure(exposure);

    _effecting_ispparm_map[frameid].frame_sof_ts = _frame_sof_time;
    set_3a_config_sync();

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
    if (fmt.format.code == MEDIA_BUS_FMT_Y8_1X8 |
        fmt.format.code == MEDIA_BUS_FMT_Y10_1X10 |
        fmt.format.code == MEDIA_BUS_FMT_Y12_1X12)
         _is_bw_sensor = true;
     else {
        _is_bw_sensor = false;
     }

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

int
IspController::get_sensor_fps(float& fps)
{
    struct v4l2_subdev_frame_interval finterval;

    memset(&finterval, 0, sizeof(finterval));
    finterval.pad = 0;

    if (_sensor_subdev->io_control(VIDIOC_SUBDEV_G_FRAME_INTERVAL, &finterval) < 0)
        return -errno;

    fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;

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
    // pixel rate is not equal to pclk sometimes
    // prefer to use pclk = ppl * lpp * fps
    float fps = 0;
    if (get_sensor_fps(fps) == 0)
        sensor_desc->pixel_clock_freq_mhz =
            (float)(sensor_desc->pixel_periods_per_line) *
            sensor_desc->line_periods_per_field * fps / 1000000.0;
    else if (get_pixel(sensor_desc))
        return XCAM_RETURN_ERROR_IOCTL;

    if (get_exposure_range(sensor_desc))
        return XCAM_RETURN_ERROR_IOCTL;

    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn
IspController::get_sensor_mode_data (struct isp_supplemental_sensor_mode_data &sensor_mode_data, int frame_id)
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

        sensor_mode_data.is_bw_sensor = _is_bw_sensor;
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

        _isp_acq_out_width =
            sensor_mode_data.sensor_output_width;
        _isp_acq_out_height =
            sensor_mode_data.sensor_output_height;

        {
            SmartLock locker (_mutex);
            std::map<int, struct rkisp_exposure>::iterator it;
            int num = _effecting_exposure_map.size();
            int search_id = frame_id < 0 ? 0 : frame_id;

            do {
                it = _effecting_exposure_map.find(search_id);
                if (it != _effecting_exposure_map.end()) {
                    sensor_mode_data.exp_time_seconds =
                      _effecting_exposure_map[search_id].SmoothIntTimes[0];
                    sensor_mode_data.gains =
                      _effecting_exposure_map[search_id].SmoothGains[0];
                    sensor_mode_data.exp_time =
                      _effecting_exposure_map[search_id].RegSmoothTime[0];
                    sensor_mode_data.gain =
                      _effecting_exposure_map[search_id].RegSmoothGains[0];
                    break;
                }
            } while (--num > 0 && --search_id >=0);

            if (it == _effecting_exposure_map.end()) {
                LOGW("can't find %d expoure in effecting map.", frame_id);
                // can't be 0, or will cause awb algo error
                sensor_mode_data.exp_time_seconds = 0.0001f;
                sensor_mode_data.gains = 1.0f;
                sensor_mode_data.exp_time =
                  _exposure_queue[0].coarse_integration_time;
                sensor_mode_data.gain =
                  _exposure_queue[0].analog_gain;
            }
        }
            XCAM_LOG_DEBUG("|||sensormode (%d-%d) expsync, time %f,gains %f, frame_id %d\n",
                sensor_mode_data.exp_time ,
                sensor_mode_data.gain,
                sensor_mode_data.exp_time_seconds,
                sensor_mode_data.gains,
                frame_id);
    }
#endif

	return XCAM_RETURN_NO_ERROR;
}

XCamReturn
IspController::get_isp_parameter (struct rkisp_parameters& parameters, int frame_id)
{
    SmartLock locker (_mutex);
    std::map<int, struct rkisp_effect_params>::iterator it;
    struct rkisp_effect_params* isp_effect_params;
    int num = _effecting_ispparm_map.size();
    int search_id = frame_id;

    if (_effecting_ispparm_map.empty()) {
        XCAM_LOG_WARNING ("no effecting isp params !");
        return XCAM_RETURN_ERROR_PARAM;
    }

    do {
        it = _effecting_ispparm_map.find(search_id);
        if (it != _effecting_ispparm_map.end()) {
            isp_effect_params = &it->second;
            LOGD("use isp param %d for %d",search_id, frame_id);
            break;
        }
    } while (--num > 0 && --search_id >= 0);

    // havn't found
    if (it == _effecting_ispparm_map.end()) {
        /* use the latest */
        std::map<int, struct rkisp_effect_params>::reverse_iterator rit;

        rit = _effecting_ispparm_map.rbegin();
        isp_effect_params = &rit->second;
        LOGW("FIXME! unpossible case. frame_id %d",frame_id);
    }

    parameters.awb_gain_config =
        isp_effect_params->isp_params.others.awb_gain_config;
    parameters.ctk_config =
        isp_effect_params->isp_params.others.ctk_config;
    parameters.awb_algo_results =
        isp_effect_params->awb_algo_results;
    parameters.frame_sof_ts =
        isp_effect_params->frame_sof_ts;

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

        /* isp_stats->params.ae = aiq_stats->params.ae; */
        /* isp_stats->params.hist= aiq_stats->params.hist; */
        /* isp_stats->params.awb = aiq_stats->params.awb; */
        /* isp_stats->params.af = aiq_stats->params.af; */
        /* compatible with no emd info, so we could use new camera engine with
         * old rkisp driver that has no emd field in stats
         */
        if (isp_stats->meas_type & CIFISP_STAT_EMB_DATA)
            *isp_stats = *aiq_stats;
        else
            memcpy(isp_stats, aiq_stats,
                   sizeof(*isp_stats) - sizeof(struct cifisp_embedded_data));
        isp_stats->frame_id = cur_frame_id;
        ret = _isp_stats_device->queue_buffer (v4l2buf);
        if (ret != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("queue stats buffer failed");
            return ret;
        }

        XCAM_LOG_DEBUG("|||get_3a_statistics[%d-%d] MEAS AE: %d MEAS AWB[%d] [%d-%d-%d] expsync, meastype 0x%x",
            cur_frame_id,
            _frame_sequence,
            isp_stats->params.ae.exp_mean[0],
            isp_stats->params.awb.awb_mean[0].cnt,
            isp_stats->params.awb.awb_mean[0].mean_y_or_g,
            isp_stats->params.awb.awb_mean[0].mean_cr_or_r,
            isp_stats->params.awb.awb_mean[0].mean_cb_or_b,
            isp_stats->meas_type);
        //print embeded data stats
        if (0 && isp_stats->meas_type & CIFISP_STAT_EMB_DATA) {
            cifisp_preisp_hdr_ae_embeded_type_t *emb =
                (cifisp_preisp_hdr_ae_embeded_type_t*)isp_stats->params.emd.data;
            XCAM_LOG_DEBUG("%s: emb head: size %u, fid %u, msg count %u",
                           __FUNCTION__, emb->head.mesg_total_size,
                           emb->head.frame_id, emb->head.mesg_count);

            struct cifisp_preisp_hdrae_result *hdr_ae_result = &emb->result;

            XCAM_LOG_DEBUG("%s: emb hdr result: lgmean %u, timereg[%u-%u-%u], gainreg[%u-%u-%u]",
                           __FUNCTION__, hdr_ae_result->lgmean,
                          hdr_ae_result->reg_exp_time[0],
                          hdr_ae_result->reg_exp_time[1],
                          hdr_ae_result->reg_exp_time[2],
                          hdr_ae_result->reg_exp_gain[0],
                          hdr_ae_result->reg_exp_gain[1],
                          hdr_ae_result->reg_exp_gain[2]);
            struct cifisp_preisp_hdrae_OE_meas_res *OEMeasRes =
                &hdr_ae_result->OEMeasRes; //zlj add

            XCAM_LOG_DEBUG("%s: emb hdr oe meas result: oe_pixel %u,sumhistpixel %u, maxluma %u",
                            __FUNCTION__, OEMeasRes->OE_Pixel,
                            OEMeasRes->SumHistPixel, OEMeasRes->SframeMaxLuma);
            struct cifisp_preisp_hdrae_DRIndex_res *DRIndexRes =
                &hdr_ae_result->DRIndexRes; //zlj add

            XCAM_LOG_DEBUG("%s: emb hdr dr result: fNormalIndex %u,fLongIndex %u",
                           __FUNCTION__, DRIndexRes->fNormalIndex, DRIndexRes->fLongIndex);
            struct cifisp_preisp_hdrae_oneframe_result *oneframe =
                hdr_ae_result->oneframe; //zlj add
            struct cifisp_preisp_hdrae_hist_meas_res *hist_meas;
            struct cifisp_preisp_hdrae_mean_meas_res *mean_meas;

            for (int i = 0; i < CIFISP_PREISP_HDRAE_MAXFRAMES; i++) {
                hist_meas = &oneframe[i].hist_meas;
                mean_meas = &oneframe[i].mean_meas;
                uint32_t *hist_bin = (uint32_t*)hist_meas->hist_bin;
                uint16_t *y_meas = (uint16_t*)mean_meas->y_meas;
                XCAM_LOG_DEBUG("\n-------------%s: frame [%d] emb hdr hist bins:"
                               "---------------\n", __FUNCTION__, i);
                for (int j = 0; j < CIFISP_PREISP_HDRAE_HIST_BIN_NUM; j +=16) {
                    XCAM_LOG_DEBUG("%s:%d->%d: %u  %u  %u  %u  %u %u  %u  %u  "
                                   "%u  %u  %u  %u  %u  %u  %u %u",
                                   __FUNCTION__, j, j+15,
                                   hist_bin[j],hist_bin[j+1],hist_bin[j+2],hist_bin[j+3],
                                   hist_bin[j+4],hist_bin[j+5],hist_bin[j+6],hist_bin[j+7],
                                   hist_bin[j+8],hist_bin[j+9],hist_bin[j+10],hist_bin[j+11],
                                   hist_bin[j+12],hist_bin[j+13],hist_bin[j+14],hist_bin[j+15]);
                }

                XCAM_LOG_DEBUG("\n--------------%s: frame [%d] emb hdr max grid items:"
                               "----------------\n", __FUNCTION__, i);
                for (int j = 0; j < CIFISP_PREISP_HDRAE_MAXGRIDITEMS; j +=15) {
                    XCAM_LOG_DEBUG("%s:%d->%d: %u  %u  %u  %u  %u  %u  %u  %u  "
                                   "%u  %u  %u  %u  %u  %u  %u",
                                   __FUNCTION__, j, j+14,
                                   y_meas[j],y_meas[j+1],y_meas[j+2],y_meas[j+3],
                                   y_meas[j+4],y_meas[j+5],y_meas[j+6],y_meas[j+7],
                                   y_meas[j+8],y_meas[j+9],y_meas[j+10],y_meas[j+11],
                                   y_meas[j+12],y_meas[j+13],y_meas[j+14]);
                }
            }

        }
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

void
IspController::gen_full_isp_params(const struct rkisp1_isp_params_cfg *update_params,
                                   struct rkisp1_isp_params_cfg *full_params)
{
    XCAM_ASSERT (update_params);
    XCAM_ASSERT (full_params);
    int i = 0;

	unsigned int module_en_update;
	unsigned int module_ens;
	unsigned int module_cfg_update;

	struct cifisp_isp_meas_cfg meas;
	struct cifisp_isp_other_cfg others;
    for (; i <= CIFISP_RK_IESHARP_ID; i++)
        if (update_params->module_en_update & (1 << i)) {
            full_params->module_en_update |= 1 << i;
            // clear old bit value
            full_params->module_ens &= ~(1 << i);
            // set new bit value
            full_params->module_ens |= update_params->module_ens & (1 << i);
        }

    for (i = 0; i <= CIFISP_RK_IESHARP_ID; i++) {
        if (update_params->module_cfg_update & (1 << i)) {
            full_params->module_cfg_update |= 1 << i;
            switch (i) {
            case CIFISP_DPCC_ID:
                full_params->others.dpcc_config = update_params->others.dpcc_config;
                break;
            case CIFISP_BLS_ID:
                full_params->others.bls_config = update_params->others.bls_config;
                break;
            case CIFISP_SDG_ID:
                full_params->others.sdg_config = update_params->others.sdg_config;
                break;
            case CIFISP_HST_ID:
                full_params->meas.hst_config = update_params->meas.hst_config;
                break;
            case CIFISP_LSC_ID:
                full_params->others.lsc_config = update_params->others.lsc_config;
                break;
            case CIFISP_AWB_GAIN_ID:
                full_params->others.awb_gain_config = update_params->others.awb_gain_config;
                break;
            case CIFISP_FLT_ID:
                full_params->others.flt_config = update_params->others.flt_config;
                break;
            case CIFISP_BDM_ID:
                full_params->others.bdm_config = update_params->others.bdm_config;
                break;
            case CIFISP_CTK_ID:
                full_params->others.ctk_config = update_params->others.ctk_config;
                break;
            case CIFISP_GOC_ID:
                full_params->others.goc_config = update_params->others.goc_config;
                break;
            case CIFISP_CPROC_ID:
                full_params->others.cproc_config = update_params->others.cproc_config;
                break;
            case CIFISP_AFC_ID:
                full_params->meas.afc_config = update_params->meas.afc_config;
                break;
            case CIFISP_AWB_ID:
                full_params->meas.awb_meas_config = update_params->meas.awb_meas_config;
                break;
            case CIFISP_IE_ID:
                full_params->others.ie_config = update_params->others.ie_config;
                break;
            case CIFISP_AEC_ID:
                full_params->meas.aec_config = update_params->meas.aec_config;
                break;
            case CIFISP_WDR_ID:
                full_params->others.wdr_config = update_params->others.wdr_config;
                break;
            case CIFISP_DPF_ID:
                full_params->others.dpf_config = update_params->others.dpf_config;
                break;
            case CIFISP_DPF_STRENGTH_ID:
                full_params->others.dpf_strength_config = update_params->others.dpf_strength_config;
                break;
            case CIFISP_DEMOSAICLP_ID:
                full_params->others.demosaiclp_config = update_params->others.demosaiclp_config;
                break;
            case CIFISP_RK_IESHARP_ID:
                full_params->others.rkiesharp_config = update_params->others.rkiesharp_config;
                break;
            default:
                break;
            }
        }
    }
}

XCamReturn
IspController::set_3a_config_sync ()
{
    struct rkisp_parameters config;
    struct rkisp_parameters* isp_cfg = NULL;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (_effecting_ispparm_map.size() > 10)
        _effecting_ispparm_map.erase(_effecting_ispparm_map.begin());

    if (_pending_ispparams_queue.empty()) {
        LOGD("no new isp params !");
        // reuse last params
        if (_frame_sequence >= 0)
            _effecting_ispparm_map[_frame_sequence + 1] =
                _effecting_ispparm_map[_frame_sequence];
        else
            LOGE("FIXME! no initial isp params !");

        return ret;
    }

    // merge all pending params
    while (!_pending_ispparams_queue.empty()) {
        struct rkisp1_isp_params_cfg update_params;

        xcam_mem_clear (update_params);
        LOGD("merge isp params num %d\n", _pending_ispparams_queue.size());
        config = *_pending_ispparams_queue.begin();
        _pending_ispparams_queue.erase(_pending_ispparams_queue.begin());
        isp_cfg = &config;

        ret = rkisp1_convert_results(&update_params,isp_cfg, _last_aiq_results);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE("rkisp1_convert_results error\n");
        }
        if (_is_bw_sensor) {
             /* bypass BDM forcely */
             /* note that BDM en bit means bypass enable actually */
             update_params.module_ens |= CIFISP_MODULE_BDM;
             update_params.module_en_update |= CIFISP_MODULE_BDM;
             update_params.module_cfg_update &= ~CIFISP_MODULE_BDM;
        }

        // if _isp_ver < 2, not support rkiesharp and demosaic_lp
        if (_isp_ver < 2) {
             update_params.module_ens &= ~(CIFISP_MODULE_RK_IESHARP | CIFISP_MODULE_DEMOSAICLP);
             update_params.module_en_update &= ~(CIFISP_MODULE_RK_IESHARP | CIFISP_MODULE_DEMOSAICLP);
             update_params.module_cfg_update &= ~(CIFISP_MODULE_RK_IESHARP | CIFISP_MODULE_DEMOSAICLP);
        }

        gen_full_isp_params(&update_params, &_full_active_isp_params);
        // ie and rkiesharp can't be active concurrently, and ie has the high
        // privilege.
        if ((_full_active_isp_params.module_ens & CIFISP_MODULE_RK_IESHARP) &&
            (_full_active_isp_params.module_ens & CIFISP_MODULE_IE)) {
            _full_active_isp_params.module_ens &= ~CIFISP_MODULE_RK_IESHARP;
            _full_active_isp_params.module_en_update |= CIFISP_MODULE_RK_IESHARP;
            _full_active_isp_params.module_cfg_update &= ~CIFISP_MODULE_RK_IESHARP;
        }

        XCAM_ASSERT (_isp_acq_out_width != -1);
        XCAM_ASSERT (_isp_acq_out_height != -1);
        ret = rkisp1_check_params(&_full_active_isp_params, _isp_acq_out_width,
                                  _isp_acq_out_height, _isp_ver);
        if (ret != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_ERROR ("rkisp1_check_params error\n");
            return XCAM_RETURN_ERROR_PARAM;
        }
        dump_isp_config(&_full_active_isp_params, isp_cfg);
    }

    if (_frame_sequence < 0) {
        _effecting_ispparm_map[0].isp_params = _full_active_isp_params;
        _effecting_ispparm_map[0].awb_algo_results = isp_cfg->awb_algo_results;
    } else {
        _effecting_ispparm_map[_frame_sequence + 1].isp_params = _full_active_isp_params;
        _effecting_ispparm_map[_frame_sequence + 1].awb_algo_results = isp_cfg->awb_algo_results;
    }

#if RKISP
    if (_isp_params_device.ptr()) {
        struct rkisp1_isp_params_cfg* isp_params;
        SmartPtr<V4l2Buffer> v4l2buf;
        int buf_cnt = _isp_params_device->get_buffer_count();
        int buf_queued_cnt = _isp_params_device->get_queued_bufcnt();
        int buf_index = 0;

        // use ping-pong buffer
        //XCAM_ASSERT (buf_cnt == 2);
        if (buf_queued_cnt == buf_cnt) {
            /* wait params done, this may block the thread */
            if (_is_exit)
                return XCAM_RETURN_BYPASS;
            if (_isp_params_device->dequeue_buffer(v4l2buf) != 0) {
                XCAM_LOG_ERROR ("RKISP1: failed to ioctl VIDIOC_DQBUF for %d %s.\n",
                       errno, strerror(errno));
                return ret;
            }
            buf_index = v4l2buf->get_buf().index;
        } else {
            buf_index = buf_queued_cnt ;
            v4l2buf = _isp_params_device->get_buffer_by_index(buf_index);
        }

        isp_params = (struct rkisp1_isp_params_cfg*)v4l2buf->get_buf().m.userptr;
        *isp_params = _full_active_isp_params;

        if (_isp_params_device->queue_buffer (v4l2buf) != 0) {
            XCAM_LOG_ERROR ("RKISP1: failed to ioctl VIDIOC_QBUF for index %d, %d %s.\n",
                   buf_index, errno, strerror(errno));
            return ret;
        }
        XCAM_LOG_DEBUG ("device(%s) queue buffer index %d, queue cnt %d, check exit status again[exit: %d]",
            XCAM_STR (_isp_params_device->get_device_name()), buf_index, _isp_params_device->get_queued_bufcnt(), _is_exit);
        if (_is_exit)
            return XCAM_RETURN_BYPASS;
    }
#endif

    XCAM_LOG_DEBUG ("   set_3a_config done\n");

    return XCAM_RETURN_NO_ERROR;

}

XCamReturn
IspController::apply_otp_config (struct rkisp_parameters *isp_cfg) {
    if (isp_cfg->otp_info_avl) {
        if (isp_cfg->awb_otp_info.enable &&
            _sensor_subdev->io_control(RKMODULE_AWB_CFG, &isp_cfg->awb_otp_info) < 0) {
            XCAM_LOG_ERROR ("failed to apply camera module awb otp");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        if (isp_cfg->af_otp_info.enable &&
            _sensor_subdev->io_control(RKMODULE_AF_CFG, &isp_cfg->af_otp_info) < 0) {
            XCAM_LOG_ERROR ("failed to apply camera module af otp");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        if (isp_cfg->lsc_otp_info.enable &&
            _sensor_subdev->io_control(RKMODULE_LSC_CFG, &isp_cfg->lsc_otp_info) < 0) {
            XCAM_LOG_ERROR ("failed to apply camera module af otp");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

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

    SmartLock locker (_mutex);

    if (_pending_ispparams_queue.size() > 3) {
        XCAM_LOG_DEBUG ("too many pending isp params:%d !", _pending_ispparams_queue.size());
        _pending_ispparams_queue.erase(_pending_ispparams_queue.begin());
    }
    _pending_ispparams_queue.push_back(*isp_cfg);

    // set initial isp params
    if (_frame_sequence < 0) {
        set_3a_config_sync();
        apply_otp_config (isp_cfg);
    }

    return ret;
}

void
IspController::exposure_delay(struct rkisp_exposure isp_exposure)
{
    int i = 0;

    SmartLock locker (_mutex);
    if (isp_exposure.RegSmoothTime[2] != 0 &&
        _exposure_queue[0].RegSmoothGains[2] ==
        isp_exposure.RegSmoothGains[2] &&
        _exposure_queue[0].RegSmoothTime[2] ==
        isp_exposure.RegSmoothTime[2]) {
        LOGD("exposure reg(%d,%d) haven't changed , drop it !",
             isp_exposure.RegSmoothGains[2],
             isp_exposure.RegSmoothTime[2]);

        return ;
    }
    _exposure_queue[0] = isp_exposure;
    _cur_apply_index = 0;

    isp_exposure.RegSmoothGains[0] =
        isp_exposure.RegSmoothGains[1];
    isp_exposure.RegSmoothTime[0] =
        isp_exposure.RegSmoothTime[1];
    isp_exposure.SmoothGains[0] =
        isp_exposure.SmoothGains[1];
    isp_exposure.SmoothIntTimes[0] =
        isp_exposure.SmoothIntTimes[1];
    isp_exposure.RegSmoothFll[0] =
        isp_exposure.RegSmoothFll[1];
    _exposure_queue[1] = isp_exposure;

    if (memcmp(&_exposure_queue[0], &_exposure_queue[1], sizeof(_exposure_queue[0])) == 0)
        _cur_apply_index++;

    isp_exposure.RegSmoothGains[0] =
        isp_exposure.RegSmoothGains[2];
    isp_exposure.RegSmoothTime[0] =
        isp_exposure.RegSmoothTime[2];
    isp_exposure.SmoothGains[0] =
        isp_exposure.SmoothGains[2];
    isp_exposure.SmoothIntTimes[0] =
        isp_exposure.SmoothIntTimes[2];
    isp_exposure.RegSmoothFll[0] =
        isp_exposure.RegSmoothFll[2];

    _exposure_queue[2] = isp_exposure;

    if (memcmp(&_exposure_queue[1], &_exposure_queue[2], sizeof(_exposure_queue[0])) == 0)
        _cur_apply_index++;

    // set the initial exposure before streaming
    if (_frame_sequence < 0) {
        set_3a_exposure(_exposure_queue[_cur_apply_index]);
        _frame_sequence++;
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

    if (_effecting_exposure_map.size() > 10)
        _effecting_exposure_map.erase(_effecting_exposure_map.begin());
    // map the exposure to corresponded effect frame id
    int effecting_frame_id = _frame_sequence + EXPOSURE_TIME_DELAY - 1;
    if (effecting_frame_id < 0)
        effecting_frame_id = 0;
    _effecting_exposure_map[effecting_frame_id] = isp_exposure;

    LOGD("----------------------------------------------");
    if (!isp_exposure.IsHdrExp)
        LOGD("|||set_3a_exposure (%d-%d) fll 0x%x expsync in sof %d\n",
            isp_exposure.RegSmoothTime[0],
            isp_exposure.RegSmoothGains[0],
            isp_exposure.RegSmoothFll[0],
            _frame_sequence);
    else
        LOGD("|||set_3a_exposure timereg (%d-%d-%d), gainreg (%d-%d-%d)"
             "fll 0x%x expsync in sof %d\n",
            isp_exposure.RegHdrTime[0], isp_exposure.RegHdrTime[1],
            isp_exposure.RegHdrTime[2], isp_exposure.RegHdrGains[0],
            isp_exposure.RegHdrGains[1],isp_exposure.RegHdrGains[2],
            isp_exposure.frame_line_length, _frame_sequence);
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
        if (!isp_exposure.IsHdrExp) {
            struct v4l2_control ctrl;

            // set vts before exposure time firstly
            rk_aiq_exposure_sensor_descriptor sensor_desc;
            get_sensor_descriptor (&sensor_desc);

            isp_exposure.frame_line_length =
                (sensor_desc.line_periods_per_field < isp_exposure.RegSmoothFll[0]) ?
                isp_exposure.RegSmoothFll[0] : sensor_desc.line_periods_per_field;
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id = V4L2_CID_VBLANK;
            ctrl.value = isp_exposure.frame_line_length - sensor_desc.sensor_output_height;
            if (_sensor_subdev->io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                XCAM_LOG_ERROR ("failed to set vblank result(val: %d)", ctrl.value);
                return XCAM_RETURN_ERROR_IOCTL;
            }

            if (isp_exposure.analog_gain!= 0) {
                memset(&ctrl, 0, sizeof(ctrl));
                ctrl.id = V4L2_CID_ANALOGUE_GAIN;
                /* ctrl.value = isp_exposure.analog_gain; */
                ctrl.value = isp_exposure.RegSmoothGains[0];
                if (_sensor_subdev->io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                    XCAM_LOG_ERROR ("failed to  set again result(val: %d)", isp_exposure.RegSmoothGains[0]);
                    return XCAM_RETURN_ERROR_IOCTL;
                }
            }
            if (isp_exposure.digital_gain!= 0) {
                memset(&ctrl, 0, sizeof(ctrl));
                ctrl.id = V4L2_CID_GAIN;
                /* ctrl.value = isp_exposure.digital_gain; */
                ctrl.value = isp_exposure.RegSmoothGains[0];
                if (_sensor_subdev->io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                    XCAM_LOG_ERROR ("failed to set dgain result(val: %d)", isp_exposure.digital_gain);
                    return XCAM_RETURN_ERROR_IOCTL;
                }
            }

            if (isp_exposure.coarse_integration_time!= 0) {
                memset(&ctrl, 0, sizeof(ctrl));
                ctrl.id = V4L2_CID_EXPOSURE;
                /* ctrl.value = isp_exposure.coarse_integration_time; */
                ctrl.value = isp_exposure.RegSmoothTime[0];
                if (_sensor_subdev->io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                    XCAM_LOG_ERROR ("failed to set integration time result(val: %d)", isp_exposure.RegSmoothTime[0]);
                    return XCAM_RETURN_ERROR_IOCTL;
                }
            }
        } else {
            struct preisp_hdrae_exp_s hdrae;

            memset(&hdrae, 0, sizeof(hdrae));
            hdrae.long_exp_reg = isp_exposure.RegHdrTime[0];
            hdrae.long_gain_reg = isp_exposure.RegHdrGains[0];
            hdrae.middle_exp_reg = isp_exposure.RegHdrTime[1];
            hdrae.middle_gain_reg = isp_exposure.RegHdrGains[1];
            hdrae.short_exp_reg = isp_exposure.RegHdrTime[2];
            hdrae.short_gain_reg = isp_exposure.RegHdrGains[2];
            memcpy(&hdrae.long_exp_val,
                   &isp_exposure.HdrIntTimes[0],
                   sizeof(hdrae.long_exp_val));
            memcpy(&hdrae.long_gain_val,
                   &isp_exposure.HdrGains[0],
                   sizeof(hdrae.long_gain_val));
            memcpy(&hdrae.middle_exp_val,
                   &isp_exposure.HdrIntTimes[1],
                   sizeof(hdrae.middle_exp_val));
            memcpy(&hdrae.middle_gain_val,
                   &isp_exposure.HdrGains[1],
                   sizeof(hdrae.middle_gain_val));
            memcpy(&hdrae.short_exp_val,
                   &isp_exposure.HdrIntTimes[2],
                   sizeof(hdrae.short_exp_val));
            memcpy(&hdrae.short_gain_val,
                   &isp_exposure.HdrGains[2],
                   sizeof(hdrae.short_gain_val));

            if (_sensor_subdev->io_control(CIFISP_CMD_SET_HDRAE_EXP, &hdrae) < 0) {
                XCAM_LOG_ERROR ("failed to set hdrae exp");
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
               afc window0[%d-%d-%d-%d]\n \
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

        isp_params->meas.afc_config.afm_win[0].h_offs,
        isp_params->meas.afc_config.afm_win[0].v_offs,
        isp_params->meas.afc_config.afm_win[0].h_size,
        isp_params->meas.afc_config.afm_win[0].v_size,
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
