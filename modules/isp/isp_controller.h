/*
 * isp_controller.h - isp controller
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
#ifndef XCAM_ISP_CONTROLLER_H
#define XCAM_ISP_CONTROLLER_H

#include <xcam_std.h>
#include <map>
#include "x3a_isp_config.h"
#include <v4l2_buffer_proxy.h>
#include <rk_aiq.h>
#include <v4l2-subdev.h>

namespace XCam {

class V4l2Device;
class V4l2SubDevice;
class X3aIspStatistics;
class X3aIspConfig;

class IspController {
public:
    explicit IspController ();
    ~IspController ();

    void exit(bool pause);
    void set_isp_device(SmartPtr<V4l2Device> &dev);
    void set_video_device(SmartPtr<V4l2Device> &dev);

    void set_sensor_subdev (SmartPtr<V4l2SubDevice> &subdev);
    void set_vcm_subdev (SmartPtr<V4l2SubDevice> &subdev);
    void set_isp_stats_device(SmartPtr<V4l2Device> &dev);
    void set_isp_params_device(SmartPtr<V4l2Device> &dev);
    void set_isp_ver(int isp_ver) { _isp_ver = isp_ver; }
    int  get_isp_ver() { return _isp_ver; }

    XCamReturn handle_sof(int64_t time, int frameid);

    int get_pixel(rk_aiq_exposure_sensor_descriptor* sensor_desc);
    int get_blank(rk_aiq_exposure_sensor_descriptor* sensor_desc);
    int get_exposure_range(rk_aiq_exposure_sensor_descriptor* sensor_desc);
    int get_format(rk_aiq_exposure_sensor_descriptor* sensor_desc);
    XCamReturn get_sensor_descriptor (rk_aiq_exposure_sensor_descriptor *sensor_desc);
    XCamReturn get_sensor_mode_data (struct isp_supplemental_sensor_mode_data &sensor_mode_data,
                                     int frame_id = -1);
    XCamReturn get_isp_parameter (struct rkisp_parameters& parameters, int frame_id = -1);
    XCamReturn get_frame_softime (int64_t &sof_tim);
    XCamReturn get_vcm_time (struct rk_cam_vcm_tim *vcm_tim);

    XCamReturn get_3a_statistics (SmartPtr<X3aIspStatistics> &stats);
    XCamReturn set_3a_config (X3aIspConfig *config);

    void push_3a_exposure (X3aIspExposureResult *res);
    void push_3a_exposure (struct rkisp_exposure isp_exposure);

    XCamReturn set_3a_exposure (X3aIspExposureResult *res);
    XCamReturn set_3a_exposure (struct rkisp_exposure isp_exposure);
    XCamReturn set_3a_focus (X3aIspFocusResult *res);

    void exposure_delay(struct rkisp_exposure isp_exposure);
#if RKISP
    void dump_isp_config(struct rkisp1_isp_params_cfg* isp_params,
                                struct rkisp_parameters *isp_cfg);
#endif

private:

    XCAM_DEAD_COPY (IspController);
    int get_sensor_fps(float& fps);
    void gen_full_isp_params(const struct rkisp1_isp_params_cfg *update_params,
                             struct rkisp1_isp_params_cfg *full_params);
    XCamReturn set_3a_config_sync ();
    XCamReturn apply_otp_config (struct rkisp_parameters *isp_cfg);

private:
    volatile bool            _is_exit;
    /* rkisp1x */
    SmartPtr<V4l2Device>     _device;
    SmartPtr<V4l2Device>     _isp_device;
    SmartPtr<V4l2Isp10Ioctl> _isp_ioctl;

    struct rkisp_parameters  _last_aiq_results;

    /* rkisp1 */
    SmartPtr<V4l2SubDevice>  _sensor_subdev;
    SmartPtr<V4l2Device>     _isp_stats_device;
    SmartPtr<V4l2Device>     _isp_params_device;

    SmartPtr<V4l2SubDevice>  _vcm_device;
    bool                     _is_bw_sensor;
    /* frame sync */
#define EXPOSURE_GAIN_DELAY 3
#define EXPOSURE_TIME_DELAY 3

    int64_t                  _frame_sof_time;
    int                      _frame_sequence;
    int                      _max_delay;
    /* exposure syncronization */
    struct rkisp_exposure *_exposure_queue;
    int                   _cur_apply_index;

    Mutex             _mutex;
    XCam::Cond        _frame_sequence_cond;

    struct rkisp1_isp_params_cfg _full_active_isp_params;
    int               _isp_ver;
    std::map<int, struct rkisp_exposure> _effecting_exposure_map;
    struct rkisp_effect_params {
        struct rkisp1_isp_params_cfg isp_params;
        struct rkisp_awb_algo awb_algo_results;
        int64_t frame_sof_ts;
    };
    std::map<int, struct rkisp_effect_params> _effecting_ispparm_map;
    std::vector<struct rkisp_parameters> _pending_ispparams_queue;
    int _isp_acq_out_width;
    int _isp_acq_out_height;
};

};

#endif //XCAM_ISP_CONTROLLER_H
