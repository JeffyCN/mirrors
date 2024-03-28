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

#include "SensorHw.h"

#include <linux/v4l2-subdev.h>

#include <algorithm>

#include "code_to_pixel_format.h"

namespace RkCam {

uint16_t SensorHw::DEFAULT_POOL_SIZE = MAX_AEC_EFFECT_FNUM * 4;

SensorHw::SensorHw(const char* name)
    : BaseSensorHw (name)
    , _working_mode(RK_AIQ_WORKING_MODE_NORMAL)
    , _first(true)
{
    ENTER_CAMHW_FUNCTION();
    _last_exp_time = nullptr;
    _last_exp_gain = nullptr;
    _gain_delay = 0;
    _time_delay = 0;
    _gain_delayed = false;
    _frame_sequence = -1;
    _dcg_gain_mode_delay = 0;
    _dcg_gain_mode_delayed = false;
    _expParamsPool = new RkAiqSensorExpParamsPool("SensorLocalExpParams", SensorHw::DEFAULT_POOL_SIZE);
    _flip = false;
    _mirror = false;
    _update_mirror_flip = false;
    _is_i2c_exp = false;
    _dcg_gain_mode_with_time = false;

    EXIT_CAMHW_FUNCTION();
}

SensorHw::~SensorHw()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
SensorHw::setHdrSensorExposure(RKAiqAecExpInfo_t* expPar)
{
    ENTER_CAMHW_FUNCTION();
    struct hdrae_exp_s hdrExp;
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "camId: %d, frameId: %d: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x, "
                    "l-dcg %d, m-dcg %d, s-dcg %d\n",
                    mCamPhyId, _frame_sequence,
                    expPar->HdrExp[2].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[2].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[1].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[1].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[0].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[0].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[2].exp_real_params.dcg_mode,
                    expPar->HdrExp[1].exp_real_params.dcg_mode,
                    expPar->HdrExp[0].exp_real_params.dcg_mode);

    get_sensor_descriptor (&sensor_desc);

    frame_line_length = expPar->frame_length_lines > sensor_desc.line_periods_per_field ?
                        expPar->frame_length_lines : sensor_desc.line_periods_per_field;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set vblank result(val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_ANALOGUE_GAIN;
    ctrl.value = expPar->LinearExp.exp_sensor_params.analog_gain_code_global;
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to  set again result(val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&hdrExp, 0, sizeof(hdrExp));
    hdrExp.long_exp_reg =
        expPar->HdrExp[2].exp_sensor_params.coarse_integration_time;
    hdrExp.long_gain_reg =
        expPar->HdrExp[2].exp_sensor_params.analog_gain_code_global;
    hdrExp.middle_exp_reg =
        expPar->HdrExp[1].exp_sensor_params.coarse_integration_time;
    hdrExp.middle_gain_reg =
        expPar->HdrExp[1].exp_sensor_params.analog_gain_code_global;
    hdrExp.short_exp_reg =
        expPar->HdrExp[0].exp_sensor_params.coarse_integration_time;
    hdrExp.short_gain_reg =
        expPar->HdrExp[0].exp_sensor_params.analog_gain_code_global;

    int dcg_mode = expPar->HdrExp[2].exp_real_params.dcg_mode;

    if (dcg_mode == 1/*AEC_DCG_MODE_HCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0/*AEC_DCG_MODE_LCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_LCG;
    else //default
        hdrExp.long_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->HdrExp[1].exp_real_params.dcg_mode;

    if (dcg_mode == 1/*AEC_DCG_MODE_HCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0/*AEC_DCG_MODE_LCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;
    else //default
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->HdrExp[0].exp_real_params.dcg_mode;

    if (dcg_mode == 1/*AEC_DCG_MODE_HCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0/*AEC_DCG_MODE_LCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_LCG;
    else //default
        hdrExp.short_cg_mode = GAIN_MODE_LCG;

    if (io_control(SENSOR_CMD_SET_HDRAE_EXP, &hdrExp) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdrExp exp");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::setSensorDpcc(Sensor_dpcc_res_t* SensorDpccInfo)
{
    struct rkmodule_dpcc_cfg dpcc_cfg;

    dpcc_cfg.enable = SensorDpccInfo->enable;
    dpcc_cfg.cur_single_dpcc = SensorDpccInfo->cur_single_dpcc;
    dpcc_cfg.cur_multiple_dpcc = SensorDpccInfo->cur_multiple_dpcc;
    dpcc_cfg.total_dpcc = SensorDpccInfo->total_dpcc;
    LOG1_CAMHW_SUBM(SENSOR_SUBM, "camId: %d, frameId: %d: enable:%d,single:%d,multi:%d,total:%d",
                    mCamPhyId, _frame_sequence,
                    dpcc_cfg.enable, dpcc_cfg.cur_single_dpcc,
                    dpcc_cfg.cur_multiple_dpcc, dpcc_cfg.total_dpcc);
    if (io_control(RKMODULE_SET_DPCC_CFG, &dpcc_cfg) < 0) {
        //LOGE_CAMHW_SUBM(SENSOR_SUBM,"failed to set sensor dpcc");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::setLinearSensorExposure(RKAiqAecExpInfo_t* expPar)
{
    ENTER_CAMHW_FUNCTION();
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "camId: %d, frameId: %d: a-gain: %d, time: %d, dcg: %d, snr: %d\n",
                    mCamPhyId, _frame_sequence,
                    expPar->LinearExp.exp_sensor_params.analog_gain_code_global,
                    expPar->LinearExp.exp_sensor_params.coarse_integration_time,
                    expPar->LinearExp.exp_real_params.dcg_mode,
                    expPar->CISFeature.SNR);

    // set vts before exposure time firstly
    get_sensor_descriptor (&sensor_desc);

    frame_line_length = expPar->frame_length_lines > sensor_desc.line_periods_per_field ?
                        expPar->frame_length_lines : sensor_desc.line_periods_per_field;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set vblank result(val: %d)", mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    int dcg_mode = expPar->LinearExp.exp_real_params.dcg_mode;
    int dcg_mode_drv;

    if (dcg_mode == 1/*AEC_DCG_MODE_HCG*/)
        dcg_mode_drv = GAIN_MODE_HCG;
    else if (dcg_mode == 0/*AEC_DCG_MODE_LCG*/)
        dcg_mode_drv = GAIN_MODE_LCG;
    else //default
        dcg_mode_drv = -1;

    if (dcg_mode_drv != -1 ) {
        if (io_control(RKMODULE_SET_CONVERSION_GAIN, &dcg_mode_drv) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set conversion gain !", mCamPhyId);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_ANALOGUE_GAIN;
    ctrl.value = expPar->LinearExp.exp_sensor_params.analog_gain_code_global;
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to  set again result(val: %d)", mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (expPar->LinearExp.exp_sensor_params.digital_gain_global != 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_GAIN;
        ctrl.value = expPar->LinearExp.exp_sensor_params.digital_gain_global;
        if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set dgain result(val: %d)", mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    if (expPar->LinearExp.exp_sensor_params.coarse_integration_time != 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_EXPOSURE;
        ctrl.value = expPar->LinearExp.exp_sensor_params.coarse_integration_time;
        if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set dgain result(val: %d)", mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::setLinearSensorExposure(pending_split_exps_t* expPar)
{
    ENTER_CAMHW_FUNCTION();
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: cam%d frameid: %u, a-gain: %d, time: %d, dcg: %d\n", __FUNCTION__,
                    mCamPhyId,
                    _frame_sequence, expPar->rk_exp_res.sensor_params[0].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[0].coarse_integration_time,
                    expPar->rk_exp_res.dcg_mode[0]);

    // set vts before exposure time firstly
    get_sensor_descriptor (&sensor_desc);

    frame_line_length = expPar->rk_exp_res.frame_length_lines > sensor_desc.line_periods_per_field ?
                        expPar->rk_exp_res.frame_length_lines : sensor_desc.line_periods_per_field;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set vblank result(val: %d)", mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_DCG)) {
        int dcg_mode = expPar->rk_exp_res.dcg_mode[0];
        int dcg_mode_drv;

        if (dcg_mode == 1/*AEC_DCG_MODE_HCG*/)
            dcg_mode_drv = GAIN_MODE_HCG;
        else if (dcg_mode == 0/*AEC_DCG_MODE_LCG*/)
            dcg_mode_drv = GAIN_MODE_LCG;
        else //default
            dcg_mode_drv = -1;

        if (dcg_mode_drv != -1 ) {
            if (io_control(RKMODULE_SET_CONVERSION_GAIN, &dcg_mode_drv) < 0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to set conversion gain !");
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_GAIN)) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_ANALOGUE_GAIN;
        ctrl.value = expPar->rk_exp_res.sensor_params[0].analog_gain_code_global;
        if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to  set again result(val: %d)", mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }

        if (expPar->rk_exp_res.sensor_params[0].digital_gain_global != 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id = V4L2_CID_GAIN;
            ctrl.value = expPar->rk_exp_res.sensor_params[0].digital_gain_global;
            if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set dgain result(val: %d)", mCamPhyId, ctrl.value);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_TIME)) {
        if (expPar->rk_exp_res.sensor_params[0].coarse_integration_time != 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id = V4L2_CID_EXPOSURE;
            ctrl.value = expPar->rk_exp_res.sensor_params[0].coarse_integration_time;
            if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set dgain result(val: %d)", mCamPhyId, ctrl.value);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::setHdrSensorExposure(pending_split_exps_t* expPar)
{
    ENTER_CAMHW_FUNCTION();
    struct hdrae_exp_s hdrExp;
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "camId: %d, frameId: %d: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x, "
                    "l-dcg %d, m-dcg %d, s-dcg %d\n",
                    mCamPhyId, _frame_sequence,
                    expPar->rk_exp_res.sensor_params[2].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[2].coarse_integration_time,
                    expPar->rk_exp_res.sensor_params[1].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[1].coarse_integration_time,
                    expPar->rk_exp_res.sensor_params[0].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[0].coarse_integration_time,
                    expPar->rk_exp_res.dcg_mode[2],
                    expPar->rk_exp_res.dcg_mode[1],
                    expPar->rk_exp_res.dcg_mode[0]);

    get_sensor_descriptor (&sensor_desc);

    frame_line_length = expPar->rk_exp_res.frame_length_lines > sensor_desc.line_periods_per_field ?
                        expPar->rk_exp_res.frame_length_lines : sensor_desc.line_periods_per_field;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set vblank result(val: %d)", mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&hdrExp, 0, sizeof(hdrExp));
    hdrExp.long_exp_reg =
        expPar->rk_exp_res.sensor_params[2].coarse_integration_time;
    hdrExp.long_gain_reg =
        expPar->rk_exp_res.sensor_params[2].analog_gain_code_global;
    hdrExp.middle_exp_reg =
        expPar->rk_exp_res.sensor_params[1].coarse_integration_time;
    hdrExp.middle_gain_reg =
        expPar->rk_exp_res.sensor_params[1].analog_gain_code_global;
    hdrExp.short_exp_reg =
        expPar->rk_exp_res.sensor_params[0].coarse_integration_time;
    hdrExp.short_gain_reg =
        expPar->rk_exp_res.sensor_params[0].analog_gain_code_global;

    int dcg_mode = expPar->rk_exp_res.dcg_mode[2];

    if (dcg_mode == 1/*AEC_DCG_MODE_HCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0/*AEC_DCG_MODE_LCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_LCG;
    else //default
        hdrExp.long_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->rk_exp_res.dcg_mode[1];

    if (dcg_mode == 1/*AEC_DCG_MODE_HCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0/*AEC_DCG_MODE_LCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;
    else //default
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->rk_exp_res.dcg_mode[0];

    if (dcg_mode == 1/*AEC_DCG_MODE_HCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0/*AEC_DCG_MODE_LCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_LCG;
    else //default
        hdrExp.short_cg_mode = GAIN_MODE_LCG;

    if (io_control(SENSOR_CMD_SET_HDRAE_EXP, &hdrExp) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set hdrExp exp", mCamPhyId);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

int
SensorHw::get_blank(rk_aiq_exposure_sensor_descriptor* sns_des)
{
    struct v4l2_queryctrl ctrl;
    int horzBlank, vertBlank;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HBLANK;
    if (io_control(VIDIOC_QUERYCTRL, &ctrl) < 0) {
        return -errno;
    }
    horzBlank = ctrl.minimum;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    if (io_control(VIDIOC_QUERYCTRL, &ctrl) < 0) {
        return -errno;
    }
    vertBlank = ctrl.minimum;

    sns_des->pixel_periods_per_line = horzBlank + sns_des->sensor_output_width;
    sns_des->line_periods_per_field = vertBlank + sns_des->sensor_output_height;

    return 0;
}

int
SensorHw::get_pixel(rk_aiq_exposure_sensor_descriptor* sns_des)
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

    if (io_control(VIDIOC_G_EXT_CTRLS, &controls) < 0)
        return -errno;

    pixel = ext_control.value64;

    sns_des->pixel_clock_freq_mhz = (float)pixel / 1000000;

    return 0;
}

int
SensorHw::get_sensor_fps(float& fps)
{
    struct v4l2_subdev_frame_interval finterval;

    memset(&finterval, 0, sizeof(finterval));
    finterval.pad = 0;

    if (io_control(VIDIOC_SUBDEV_G_FRAME_INTERVAL, &finterval) < 0)
        return -errno;

    fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;

    return 0;
}

int
SensorHw::get_sensor_desc(rk_aiq_exposure_sensor_descriptor* sns_des)
{
    struct v4l2_subdev_format fmt;

    memset(&fmt, 0, sizeof(fmt));
    fmt.pad = 0;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;

    if (io_control(VIDIOC_SUBDEV_G_FMT, &fmt) < 0)
        return -errno;

    sns_des->sensor_output_width = fmt.format.width;
    sns_des->sensor_output_height = fmt.format.height;
    sns_des->sensor_pixelformat = get_v4l2_pixelformat(fmt.format.code);
    return 0;
}

int
SensorHw::get_exposure_range(rk_aiq_exposure_sensor_descriptor* sns_des)
{
    struct v4l2_queryctrl ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE;

    if (io_control(VIDIOC_QUERYCTRL, &ctrl) < 0)
        return -errno;

    sns_des->coarse_integration_time_min = ctrl.minimum;
    sns_des->coarse_integration_time_max_margin = 10;

    return 0;
}

int
SensorHw::get_nr_switch(rk_aiq_sensor_nr_switch_t* nr_switch)
{
    struct rkmodule_nr_switch_threshold nr_switch_drv;

    if (io_control(RKMODULE_GET_NR_SWITCH_THRESHOLD, &nr_switch_drv) < 0) {
        //LOGE_CAMHW_SUBM(SENSOR_SUBM,"failed to get sensor nr switch");
        nr_switch->valid = false;
        return XCAM_RETURN_ERROR_IOCTL;
    }

    nr_switch->valid = true;
    nr_switch->direct = nr_switch_drv.direct;
    nr_switch->up_thres = nr_switch_drv.up_thres;
    nr_switch->down_thres = nr_switch_drv.down_thres;
    nr_switch->div_coeff = nr_switch_drv.div_coeff;

    return 0;
}

int
SensorHw::get_dcg_ratio(rk_aiq_sensor_dcg_ratio_t* dcg_ratio)
{
    struct rkmodule_dcg_ratio dcg_ratio_drv;

    if (io_control(RKMODULE_GET_DCG_RATIO, &dcg_ratio_drv) < 0) {
        //LOGD_CAMHW_SUBM(SENSOR_SUBM,"failed to get sensor dcg_ratio");
        dcg_ratio->valid = false;
        return XCAM_RETURN_ERROR_IOCTL;
    }

    dcg_ratio->valid = true;
    dcg_ratio->integer = dcg_ratio_drv.integer;
    dcg_ratio->decimal = dcg_ratio_drv.decimal;
    dcg_ratio->div_coeff = dcg_ratio_drv.div_coeff;

    return 0;
}

XCamReturn
SensorHw::get_sensor_descriptor(rk_aiq_exposure_sensor_descriptor *sns_des)
{
    memset(sns_des, 0, sizeof(rk_aiq_exposure_sensor_descriptor));

    if (get_sensor_desc(sns_des))
        return XCAM_RETURN_ERROR_IOCTL;

    if (get_blank(sns_des))
        return XCAM_RETURN_ERROR_IOCTL;

    /*
     * pixel rate is not equal to pclk sometimes
     * prefer to use pclk = ppl * lpp * fps
     */
    float fps = 0;
    if (get_sensor_fps(fps) == 0)
        sns_des->pixel_clock_freq_mhz =
            (float)(sns_des->pixel_periods_per_line) *
            sns_des->line_periods_per_field * fps / 1000000.0;
    else if (get_pixel(sns_des))
        return XCAM_RETURN_ERROR_IOCTL;

    if (get_exposure_range(sns_des))
        return XCAM_RETURN_ERROR_IOCTL;

    if (get_nr_switch(&sns_des->nr_switch)) {
        // do nothing;
    }
    if (get_dcg_ratio(&sns_des->dcg_ratio)) {
        // do nothing;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::setI2cDAta(pending_split_exps_t* exps) {
    struct rkmodule_reg regs;

    regs.num_regs = (__u64)(exps->i2c_exp_res.nNumRegs);
    regs.preg_addr = (__u64)(exps->i2c_exp_res.RegAddr);
    regs.preg_value = (__u64)(exps->i2c_exp_res.RegValue);
    regs.preg_addr_bytes = (__u64)(exps->i2c_exp_res.AddrByteNum);
    regs.preg_value_bytes = (__u64)(exps->i2c_exp_res.ValueByteNum);

    LOG1_CAMHW_SUBM(SENSOR_SUBM, "set sensor reg array num %d ------", exps->i2c_exp_res.nNumRegs);
    if (exps->i2c_exp_res.nNumRegs <= 0)
        return XCAM_RETURN_NO_ERROR;

    for (uint32_t i = 0; i < regs.num_regs; i++) {
        LOG1_CAMHW_SUBM(SENSOR_SUBM, "reg:(0x%04x,%d,0x%04x,%d)",
                        exps->i2c_exp_res.RegAddr[i], exps->i2c_exp_res.AddrByteNum[i],
                        exps->i2c_exp_res.RegValue[i], exps->i2c_exp_res.ValueByteNum[i]);
    }

    if (io_control(RKMODULE_SET_REGISTER, &regs) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set i2c regs !");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    RKAiqAecExpInfoWrapper_t* exp = &expPar->data()->result;

    if (_first) {
        if (exp->algo_id == 0) {
            if (exp->ae_proc_res_rk.exp_set_cnt > 0) {
                int lastIdx = exp->ae_proc_res_rk.exp_set_cnt - 1;
                exp->new_ae_exp = exp->ae_proc_res_rk.exp_set_tbl[lastIdx];
            }
        }
        if (!exp->exp_i2c_params.bValid) {
            _is_i2c_exp = false;
            if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
                setLinearSensorExposure(&exp->new_ae_exp);
            else
                setHdrSensorExposure(&exp->new_ae_exp);
            setSensorDpcc(&exp->SensorDpccInfo);
        } else {
            _is_i2c_exp = true;
            pending_split_exps_t new_exps;
            memset(&new_exps, 0, sizeof(pending_split_exps_t));
            new_exps.i2c_exp_res.nNumRegs = exp->exp_i2c_params.nNumRegs;
            for (uint32_t i = 0; i < exp->exp_i2c_params.nNumRegs; i++) {
                new_exps.i2c_exp_res.RegAddr[i] = exp->exp_i2c_params.RegAddr[i];
                new_exps.i2c_exp_res.RegValue[i] = exp->exp_i2c_params.RegValue[i];
                new_exps.i2c_exp_res.AddrByteNum[i] = exp->exp_i2c_params.AddrByteNum[i];
                new_exps.i2c_exp_res.ValueByteNum[i] = exp->exp_i2c_params.ValueByteNum[i];
            }
            setI2cDAta(&new_exps);
        }

        SmartPtr<RkAiqSensorExpParamsProxy> expParamsProxy = NULL;
        if (_expParamsPool->has_free_items()) {
            expParamsProxy = _expParamsPool->get_item();
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
            return XCAM_RETURN_ERROR_MEM;
        }
        expParamsProxy->data()->aecExpInfo = exp->new_ae_exp;
        expParamsProxy->data()->SensorDpccInfo = exp->SensorDpccInfo;
        expParamsProxy->data()->exp_i2c_params = &exp->exp_i2c_params ;
        _effecting_exp_map[0] = expParamsProxy;
        _first = false;
        _last_exp_time = expParamsProxy;
        _last_exp_gain = expParamsProxy;
        _last_dcg_gain_mode = expParamsProxy;
        exp->exp_i2c_params.bValid = false;
        exp->ae_proc_res_rk.exp_set_cnt = 0;
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "exp-sync: first set exp, add id[0] to the effected exp map\n");
    } else {
        if (exp->algo_id == 0) {
            if (exp->ae_proc_res_rk.exp_set_cnt > 0) {
                SmartPtr<RkAiqSensorExpParamsProxy> expParamsProxy = NULL;

                LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s: exp_tbl_size:%d, exp_list remain:%d\n", __FUNCTION__,
                                exp->ae_proc_res_rk.exp_set_cnt, _exp_list.size());
                /* when new exp-table comes, remove elem until meet the first one of last exp-table */
                if (!_exp_list.empty()) {
                    _exp_list.erase(std::remove_if(
                                        _exp_list.begin(), _exp_list.end(),
                    [](const std::pair<SmartPtr<RkAiqSensorExpParamsProxy>, bool>& p) {
                        return !p.second;
                    }), _exp_list.end());
                }

                SmartPtr<rk_aiq_sensor_exp_info_t> tmp;
                for(int i = 0; i < exp->ae_proc_res_rk.exp_set_cnt; i++) {
                    if (_expParamsPool->has_free_items()) {
                        expParamsProxy = _expParamsPool->get_item();
                        tmp = expParamsProxy->data();
                    } else {
                        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
                        return XCAM_RETURN_ERROR_MEM;
                    }

                    tmp->aecExpInfo = exp->new_ae_exp;
                    tmp->aecExpInfo.LinearExp = exp->ae_proc_res_rk.exp_set_tbl[i].LinearExp;
                    tmp->aecExpInfo.HdrExp[0] = exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[0];
                    tmp->aecExpInfo.HdrExp[1] = exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[1];
                    tmp->aecExpInfo.HdrExp[2] = exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[2];
                    tmp->aecExpInfo.frame_length_lines = exp->ae_proc_res_rk.exp_set_tbl[i].frame_length_lines;
                    tmp->aecExpInfo.CISFeature.SNR = exp->ae_proc_res_rk.exp_set_tbl[i].CISFeature.SNR;
                    tmp->SensorDpccInfo = exp->SensorDpccInfo;
                    tmp->exp_i2c_params = &exp->exp_i2c_params;

                    /* set a flag when it's fisrt elem of exp-table*/
                    _exp_list.push_back(std::make_pair(expParamsProxy, (i == 0 ? true : false)));

                    if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                        LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d add tbl[%d] to list: a-gain: %d, time: %d, snr: %d\n",
                                        __FUNCTION__, mCamPhyId, i,
                                        tmp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                                        tmp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                                        tmp->aecExpInfo.CISFeature.SNR);
                    } else {
                        LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d add tbl[%d] to list: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                                        __FUNCTION__, mCamPhyId, i,
                                        tmp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                                        tmp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                                        tmp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                                        tmp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                                        tmp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                                        tmp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
                    }
                }
                exp->exp_i2c_params.bValid = false;
                exp->ae_proc_res_rk.exp_set_cnt = 0;
            }
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "unsurpported now !");
        }
    }
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::getEffectiveExpParams(SmartPtr<RkAiqSensorExpParamsProxy>& expParams, uint32_t frame_id)
{
    ENTER_CAMHW_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    std::map<uint32_t, SmartPtr<RkAiqSensorExpParamsProxy>>::iterator it;
    uint32_t search_id = frame_id == (uint32_t)(-1) ? 0 : frame_id;
//#ifdef ADD_LOCK
    SmartLock locker (_mutex);
//#endif

    it = _effecting_exp_map.find(search_id);
    // havn't found
    if (it == _effecting_exp_map.end()) {
        /* use the latest */
        std::map<uint32_t, SmartPtr<RkAiqSensorExpParamsProxy>>::reverse_iterator rit;

        for (rit = _effecting_exp_map.rbegin(); rit != _effecting_exp_map.rend(); rit++) {
            if (rit->first <= search_id)
                break;
        }

        if (rit == _effecting_exp_map.rend()) {
            if (_effecting_exp_map.size() > 0) {
                rit = _effecting_exp_map.rbegin();
                LOGW_CAMHW_SUBM(SENSOR_SUBM, "use effecting exposure of %d for %d, may be something wrong !",
                                rit->first, search_id);
            } else {
                LOGE_CAMHW_SUBM(SENSOR_SUBM, "can't find the latest effecting exposure for id %d, impossible case !", search_id);
                return  XCAM_RETURN_ERROR_PARAM;
            }
        }

        expParams = rit->second;
        if (expParams.ptr()) {
            if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d search_id: %d, get-last %d, a-gain: %d, time: %d\n",
                                __FUNCTION__, mCamPhyId, search_id, rit->first,
                                expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);
            } else {
                LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d search_id: %d, get-last %d, lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                                __FUNCTION__, mCamPhyId, search_id, rit->first,
                                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
            }
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: expParams is invalid!", __FUNCTION__);
        }
        ret = XCAM_RETURN_BYPASS;
    } else {
        expParams = it->second;
        if (expParams.ptr()) {
            if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d search_id: %d, get-find %d, a-gain: %d, time: %d\n",
                                __FUNCTION__, mCamPhyId, search_id, it->first,
                                expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);
            } else {
                LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d search_id: %d, get-find %d, lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                                __FUNCTION__, mCamPhyId, search_id, it->first,
                                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
            }
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: expParams is invalid!", __FUNCTION__);
        }
        ret = XCAM_RETURN_NO_ERROR;
    }

    EXIT_CAMHW_FUNCTION();

    return ret;
}

XCamReturn
SensorHw::getSensorModeData(const char* sns_ent_name,
                            rk_aiq_exposure_sensor_descriptor& sns_des)
{
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    get_sensor_descriptor (&sensor_desc);

    _sns_entity_name = sns_ent_name;
    sns_des.coarse_integration_time_min =
        sensor_desc.coarse_integration_time_min;
    sns_des.coarse_integration_time_max_margin =
        sensor_desc.coarse_integration_time_max_margin;
    sns_des.fine_integration_time_min =
        sensor_desc.fine_integration_time_min;
    sns_des.fine_integration_time_max_margin =
        sensor_desc.fine_integration_time_max_margin;

    sns_des.frame_length_lines = sensor_desc.line_periods_per_field;
    sns_des.line_length_pck = sensor_desc.pixel_periods_per_line;
    sns_des.vt_pix_clk_freq_hz = sensor_desc.pixel_clock_freq_mhz * 1000000;
    sns_des.pixel_clock_freq_mhz = sensor_desc.pixel_clock_freq_mhz/* * 1000000 */;

    //add nr_switch
    sns_des.nr_switch = sensor_desc.nr_switch;
    sns_des.dcg_ratio = sensor_desc.dcg_ratio;

    sns_des.sensor_output_width = sensor_desc.sensor_output_width;
    sns_des.sensor_output_height = sensor_desc.sensor_output_height;
    sns_des.sensor_pixelformat = sensor_desc.sensor_pixelformat;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "vts-hts-pclk: %d-%d-%d-%f, rect: [%dx%d]\n",
                    sns_des.frame_length_lines,
                    sns_des.line_length_pck,
                    sns_des.vt_pix_clk_freq_hz,
                    sns_des.pixel_clock_freq_mhz,
                    sns_des.sensor_output_width,
                    sns_des.sensor_output_height);

    return XCAM_RETURN_NO_ERROR;
}

// sof_id: the sof_id which new exp is set
XCamReturn
SensorHw::split_locked(SmartPtr<RkAiqSensorExpParamsProxy>& exp_param, uint32_t sof_id) {
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    uint32_t dst_id = 0, max_dst_id = 0;
    // custom mode
    RKAiqExpI2cParam_t* i2c_param = exp_param->data()->exp_i2c_params;
    if (i2c_param->bValid) {
        unsigned int num_regs = i2c_param->nNumRegs;
        LOG1_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res num_regs %d!", num_regs);
        for (uint32_t i = 0; i < num_regs; i++) {
            dst_id = sof_id + i2c_param->DelayFrames[i];
            LOG1_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res delay: %d, dst_id %d",
                            i2c_param->DelayFrames[i], dst_id);
            if (max_dst_id < dst_id)
                max_dst_id = dst_id;
            if (_pending_spilt_map.count(dst_id) == 0) {
                pending_split_exps_t new_exps;
                memset(&new_exps, 0, sizeof(pending_split_exps_t));
                new_exps.is_rk_exp_res = false;
                new_exps.i2c_exp_res.RegAddr[0] = i2c_param->RegAddr[i];
                new_exps.i2c_exp_res.RegValue[0] = i2c_param->RegValue[i];
                new_exps.i2c_exp_res.AddrByteNum[0] = i2c_param->AddrByteNum[i];
                new_exps.i2c_exp_res.ValueByteNum[0] = i2c_param->ValueByteNum[i];
                new_exps.i2c_exp_res.nNumRegs = 1;
                _pending_spilt_map[dst_id] = new_exps;
            } else {
                pending_split_exps_t* tmp = &_pending_spilt_map[dst_id];
                unsigned int num_regs = tmp->i2c_exp_res.nNumRegs;
                if (num_regs >= MAX_I2CDATA_LEN) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res array overflow for frame %d!", dst_id);
                    return XCAM_RETURN_ERROR_FAILED;
                }
                tmp->i2c_exp_res.RegAddr[num_regs] = i2c_param->RegAddr[i];
                tmp->i2c_exp_res.RegValue[num_regs] = i2c_param->RegValue[i];
                tmp->i2c_exp_res.AddrByteNum[num_regs] = i2c_param->AddrByteNum[i];
                tmp->i2c_exp_res.ValueByteNum[num_regs] = i2c_param->ValueByteNum[i];
                tmp->i2c_exp_res.nNumRegs++;
            }
        }

        if (max_dst_id < sof_id)
            max_dst_id = sof_id + 1;

        _effecting_exp_map[max_dst_id + 1] = exp_param;

        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid: %d, num_reg:%d, efid:%d, isp_dgain:%0.3f \n",
                        num_regs, mCamPhyId, max_dst_id + 1,
                        exp_param->data()->aecExpInfo.LinearExp.exp_real_params.isp_dgain);
    } else {
        RKAiqAecExpInfo_t* exp_info = &exp_param->data()->aecExpInfo;

        uint32_t dst_time_id = sof_id;
        uint32_t dst_gain_id = sof_id + _time_delay - _gain_delay;
        uint32_t dst_dcg_id = sof_id + _time_delay - _dcg_gain_mode_delay;

        pending_split_exps_t new_exps;
        pending_split_exps_t* p_new_exps = NULL;
        bool is_id_exist = true;
        max_dst_id = sof_id + _time_delay;

        struct {
            uint32_t dst_id;
            uint32_t type;
        } update_exps[3] = {
            {dst_time_id, RK_EXP_UPDATE_TIME},
            {dst_gain_id, RK_EXP_UPDATE_GAIN},
            {dst_dcg_id, RK_EXP_UPDATE_DCG}
        };

        for (auto& update_exp : update_exps) {
            dst_id = update_exp.dst_id;
            pending_split_exps_t* p_new_exps = &new_exps;

            if (_pending_spilt_map.count(dst_id) == 0) {
                memset(p_new_exps, 0, sizeof(pending_split_exps_t));
                is_id_exist = false;
            } else {
                p_new_exps = &_pending_spilt_map[dst_id];
                is_id_exist = true;
            }

            p_new_exps->is_rk_exp_res = true;
            p_new_exps->rk_exp_res.update_bits |= 1 << update_exp.type;
            p_new_exps->rk_exp_res.line_length_pixels =
                exp_info->line_length_pixels;
            p_new_exps->rk_exp_res.frame_length_lines =
                exp_info->frame_length_lines;
            p_new_exps->rk_exp_res.pixel_clock_freq_mhz =
                exp_info->pixel_clock_freq_mhz;

            if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                if (update_exp.type == RK_EXP_UPDATE_TIME) {
                    p_new_exps->rk_exp_res.sensor_params[0].coarse_integration_time =
                        exp_info->LinearExp.exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[0].fine_integration_time =
                        exp_info->LinearExp.exp_sensor_params.fine_integration_time;
                } else if (update_exp.type == RK_EXP_UPDATE_GAIN) {
                    p_new_exps->rk_exp_res.sensor_params[0].analog_gain_code_global =
                        exp_info->LinearExp.exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[0].digital_gain_global =
                        exp_info->LinearExp.exp_sensor_params.digital_gain_global;
                } else if (update_exp.type == RK_EXP_UPDATE_DCG) {
                    p_new_exps->rk_exp_res.dcg_mode[0] =
                        exp_info->LinearExp.exp_real_params.dcg_mode;
                } else {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "wrong exposure params type %d!", update_exp.type);
                    return XCAM_RETURN_ERROR_FAILED;
                }
            } else {
                if (update_exp.type == RK_EXP_UPDATE_TIME) {
                    p_new_exps->rk_exp_res.sensor_params[0].coarse_integration_time =
                        exp_info->HdrExp[0].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[0].fine_integration_time =
                        exp_info->HdrExp[0].exp_sensor_params.fine_integration_time;

                    p_new_exps->rk_exp_res.sensor_params[1].coarse_integration_time =
                        exp_info->HdrExp[1].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[1].fine_integration_time =
                        exp_info->HdrExp[1].exp_sensor_params.fine_integration_time;

                    p_new_exps->rk_exp_res.sensor_params[2].coarse_integration_time =
                        exp_info->HdrExp[2].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[2].fine_integration_time =
                        exp_info->HdrExp[2].exp_sensor_params.fine_integration_time;
                } else if (update_exp.type == RK_EXP_UPDATE_GAIN) {
                    p_new_exps->rk_exp_res.sensor_params[0].analog_gain_code_global =
                        exp_info->HdrExp[0].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[0].digital_gain_global =
                        exp_info->HdrExp[0].exp_sensor_params.digital_gain_global;

                    p_new_exps->rk_exp_res.sensor_params[1].analog_gain_code_global =
                        exp_info->HdrExp[1].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[1].digital_gain_global =
                        exp_info->HdrExp[1].exp_sensor_params.digital_gain_global;

                    p_new_exps->rk_exp_res.sensor_params[2].analog_gain_code_global =
                        exp_info->HdrExp[2].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[2].digital_gain_global =
                        exp_info->HdrExp[2].exp_sensor_params.digital_gain_global;
                } else if (update_exp.type == RK_EXP_UPDATE_DCG) {
                    p_new_exps->rk_exp_res.dcg_mode[0] =
                        exp_info->HdrExp[0].exp_real_params.dcg_mode;
                    p_new_exps->rk_exp_res.dcg_mode[1] =
                        exp_info->HdrExp[1].exp_real_params.dcg_mode;
                    p_new_exps->rk_exp_res.dcg_mode[2] =
                        exp_info->HdrExp[2].exp_real_params.dcg_mode;
                } else {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "wrong exposure params type %d!", update_exp.type);
                    return XCAM_RETURN_ERROR_FAILED;
                }
            }

            if (!is_id_exist)
                _pending_spilt_map[dst_id] = *p_new_exps;

        }
        _effecting_exp_map[max_dst_id] = exp_param;
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn
SensorHw::handle_sof(int64_t time, uint32_t frameid)
{
    ENTER_CAMHW_FUNCTION();
    int effecting_frame_id = 0;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // TODO: only i2c exps using new handler now
    if (_is_i2c_exp) {
        return handle_sof_internal(time, frameid);
    }

    _mutex.lock();
    if (_frame_sequence != (uint32_t)(-1) && frameid - _frame_sequence > 1)
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "!!!!frame losed,last frameid:%u,current farmeid:%u!!!!\n", _frame_sequence, frameid);

    _frame_sequence = frameid;
    LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d frameid=%u, exp_list size=%d, gain_list size=%d",
                    __FUNCTION__, mCamPhyId, frameid, _exp_list.size(), _delayed_gain_list.size());

    SmartPtr<RkAiqSensorExpParamsProxy> exp_time = nullptr;
    SmartPtr<RkAiqSensorExpParamsProxy> exp_gain = nullptr;
    SmartPtr<RkAiqSensorExpParamsProxy> dcg_gain_mode = nullptr;
    bool set_time = false, set_gain = false, set_dcg_gain_mode = false;

    while (_effecting_exp_map.size() > 10)
        _effecting_exp_map.erase(_effecting_exp_map.begin());

    if(!_exp_list.empty()) {
        exp_time = _last_exp_time = _exp_list.front().first;
        set_time = true;
        _exp_list.pop_front();
    } else {
        exp_time = _last_exp_time;
    }

    if(!_delayed_gain_list.empty()) {
        exp_gain = _last_exp_gain = _delayed_gain_list.front();
        set_gain = true;
        _delayed_gain_list.pop_front();
    } else {
        exp_gain = _last_exp_gain;
    }

    if(!_delayed_dcg_gain_mode_list.empty()) {
        dcg_gain_mode = _last_dcg_gain_mode = _delayed_dcg_gain_mode_list.front();
        set_dcg_gain_mode = true;
        _delayed_dcg_gain_mode_list.pop_front();
    } else {
        dcg_gain_mode = _last_dcg_gain_mode;
    }
    _mutex.unlock();
    // update flip, skip _frame_sequence
    if (_update_mirror_flip) {
        _set_mirror_flip();
        _update_mirror_flip = false;
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: cam%d working_mode=%d,frameid=%u, status: set_time=%d,set_gain=%d\n",
                    __FUNCTION__, mCamPhyId, _working_mode, frameid, set_time, set_gain);

    if (set_time || set_gain || set_dcg_gain_mode) {
        RKAiqAecExpInfo_t *ptr_new_exp = NULL, new_exp;
        if (_dcg_gain_mode_delayed) {
            // _gain_delayed should be false
            composeExpParam(&exp_time->data()->aecExpInfo,
                            &exp_time->data()->aecExpInfo,
                            &dcg_gain_mode->data()->aecExpInfo,
                            &new_exp);
            ptr_new_exp = &new_exp;
        } else {
            if (_gain_delayed) {
                if (_dcg_gain_mode_with_time)
                    dcg_gain_mode = exp_time;
                else
                    dcg_gain_mode = exp_gain;
                composeExpParam(&exp_time->data()->aecExpInfo,
                                &exp_gain->data()->aecExpInfo,
                                &dcg_gain_mode->data()->aecExpInfo,
                                &new_exp);
                ptr_new_exp = &new_exp;
            } else {
                ptr_new_exp = &exp_time->data()->aecExpInfo;
            }
        }

        if(_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            ret = setLinearSensorExposure(ptr_new_exp);
        } else {
            ret = setHdrSensorExposure(ptr_new_exp);
        }

        setSensorDpcc(&exp_time->data()->SensorDpccInfo);
    }
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: sof_id[%u]: set exposure failed!!!\n",
                        __FUNCTION__,
                        frameid);
    }

    if(set_time) {
        _mutex.lock();

        if(_gain_delayed) {
            _delayed_gain_list.push_back(exp_time);
        }

        if(_dcg_gain_mode_delayed) {
            _delayed_dcg_gain_mode_list.push_back(exp_time);
        }
        effecting_frame_id = frameid + _time_delay;
        if (mPauseFlag && (mIsSingleMode || (frameid == mPauseId)) &&
            _time_delay > 1) {
            effecting_frame_id = frameid + 1;
        }
        _effecting_exp_map[effecting_frame_id] = exp_time;

        if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d sof_id[%d], _effecting_exp_map: add %d, a-gain: %d, time: %d, snr: %d\n",
                            __FUNCTION__, mCamPhyId, frameid, effecting_frame_id,
                            exp_time->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                            exp_time->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                            exp_time->data()->aecExpInfo.CISFeature.SNR);
        } else {
            LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d sof_id[%d], _effecting_exp_map: add %d, lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                            __FUNCTION__,  mCamPhyId, frameid, effecting_frame_id,
                            exp_time->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                            exp_time->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                            exp_time->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                            exp_time->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                            exp_time->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                            exp_time->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
        }

        _mutex.unlock();
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn
SensorHw::handle_sof_internal(int64_t time, uint32_t frameid)
{
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    _mutex.lock();
    if (_frame_sequence != (uint32_t)(-1) && (frameid - _frame_sequence > 1))
        LOGE_CAMHW_SUBM(SENSOR_SUBM,
                        "cam%d !!!!frame losed,last frameid:%u,current farmeid:%u!!!!\n", mCamPhyId,
                        _frame_sequence, frameid);

    _frame_sequence = frameid;
    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: cam%d frameid=%u, exp_list size=%d, gain_list size=%d",
                    __FUNCTION__, mCamPhyId, frameid, _exp_list.size(), _delayed_gain_list.size());

    SmartPtr<RkAiqSensorExpParamsProxy> new_exp = nullptr;

    while (_effecting_exp_map.size() > 10)
        _effecting_exp_map.erase(_effecting_exp_map.begin());

    if(!_exp_list.empty()) {
        new_exp = _exp_list.front().first;
        _exp_list.pop_front();
        split_locked(new_exp, frameid);
    }

    // update flip, skip _frame_sequence
    if (_update_mirror_flip && !_is_i2c_exp) {
        _set_mirror_flip();
        _update_mirror_flip = false;
    }

    bool set_new_exp = false;
    pending_split_exps_t pending_exp;

    std::map<uint32_t, pending_split_exps_t>::iterator it, it_end;
    it = it_end = _pending_spilt_map.begin();
    for (; it != _pending_spilt_map.end(); it++) {
        if (it->first <= (uint32_t)frameid) {
            pending_exp = _pending_spilt_map[frameid];
            _mutex.unlock();

            if (pending_exp.is_rk_exp_res) {
                if(_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                    ret = setLinearSensorExposure(&pending_exp);
                } else {
                    ret = setHdrSensorExposure(&pending_exp);
                }
            } else {
                setI2cDAta(&pending_exp);
            }
            it_end = it;
            it_end++;
            _mutex.lock();
        } else {
            break;
        }
    }

    _pending_spilt_map.erase(_pending_spilt_map.begin(), it_end);
    if (_pending_spilt_map.size() > 100) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "cam%d _pending_spilt_map size %d > 100, may be error",
                        mCamPhyId, _pending_spilt_map.size());
    }
    _mutex.unlock();

    if (!_is_i2c_exp && new_exp.ptr())
        setSensorDpcc(&new_exp->data()->SensorDpccInfo);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

uint32_t
BaseSensorHw::get_v4l2_pixelformat(uint32_t pixelcode)
{
    uint32_t pixelformat = -1;

    switch (pixelcode) {
    case MEDIA_BUS_FMT_SRGGB8_1X8:
        pixelformat = V4L2_PIX_FMT_SRGGB8;
        break;
    case MEDIA_BUS_FMT_SBGGR8_1X8:
        pixelformat = V4L2_PIX_FMT_SBGGR8;
        break;
    case MEDIA_BUS_FMT_SGBRG8_1X8:
        pixelformat = V4L2_PIX_FMT_SGBRG8;
        break;
    case MEDIA_BUS_FMT_SGRBG8_1X8:
        pixelformat = V4L2_PIX_FMT_SGRBG8;
        break;
    case MEDIA_BUS_FMT_SBGGR10_1X10:
        pixelformat = V4L2_PIX_FMT_SBGGR10;
        break;
    case MEDIA_BUS_FMT_SRGGB10_1X10:
        pixelformat = V4L2_PIX_FMT_SRGGB10;
        break;
    case MEDIA_BUS_FMT_SGBRG10_1X10:
        pixelformat = V4L2_PIX_FMT_SGBRG10;
        break;
    case MEDIA_BUS_FMT_SGRBG10_1X10:
        pixelformat = V4L2_PIX_FMT_SGRBG10;
        break;
    case MEDIA_BUS_FMT_SRGGB12_1X12:
        pixelformat = V4L2_PIX_FMT_SRGGB12;
        break;
    case MEDIA_BUS_FMT_SBGGR12_1X12:
        pixelformat = V4L2_PIX_FMT_SBGGR12;
        break;
    case MEDIA_BUS_FMT_SGBRG12_1X12:
        pixelformat = V4L2_PIX_FMT_SGBRG12;
        break;
    case MEDIA_BUS_FMT_SGRBG12_1X12:
        pixelformat = V4L2_PIX_FMT_SGRBG12;
        break;
    case MEDIA_BUS_FMT_Y8_1X8:
        pixelformat = V4L2_PIX_FMT_GREY;
        break;
    case MEDIA_BUS_FMT_Y10_1X10:
        pixelformat = V4L2_PIX_FMT_Y10;
        break;
    case MEDIA_BUS_FMT_Y12_1X12:
        pixelformat = V4L2_PIX_FMT_Y12;
        break;
    case MEDIA_BUS_FMT_SBGGR16_1X16:
        pixelformat = V4L2_PIX_FMT_SBGGR16;
        break;
    case MEDIA_BUS_FMT_SGBRG16_1X16:
        pixelformat = V4L2_PIX_FMT_SGBRG16;
        break;
    case MEDIA_BUS_FMT_SGRBG16_1X16:
        pixelformat = V4L2_PIX_FMT_SGRBG16;
        break;
    case MEDIA_BUS_FMT_SRGGB16_1X16:
        pixelformat = V4L2_PIX_FMT_SRGGB16;
        break;
    default:
        //TODO add other
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s no support pixelcode:0x%x\n",
                        __func__, pixelcode);
    }
    return pixelformat;
}

XCamReturn
SensorHw::set_sync_mode(uint32_t mode)
{
    if (io_control(RKMODULE_SET_SYNC_MODE, &mode) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set sync mode %d", mode);
        //return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGI_CAMHW_SUBM(SENSOR_SUBM, "set sync mode %d", mode);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::set_working_mode(int mode)
{
    rkmodule_hdr_cfg hdr_cfg;
    __u32 hdr_mode = NO_HDR;

    xcam_mem_clear(hdr_cfg);
    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        hdr_mode = NO_HDR;
    } else if (mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
               mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        hdr_mode = HDR_X2;
    } else if (mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
               mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        hdr_mode = HDR_X3;
    } else {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdr mode to %d", mode);
        return XCAM_RETURN_ERROR_FAILED;
    }
    hdr_cfg.hdr_mode = hdr_mode;
    if (io_control(RKMODULE_SET_HDR_CFG, &hdr_cfg) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdr mode %d", hdr_mode);
        //return XCAM_RETURN_ERROR_IOCTL;
    }

    _working_mode = mode;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s _working_mode: %d\n",
                    __func__, _working_mode);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::_set_mirror_flip() {
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HFLIP;
    ctrl.value = _mirror ? 1 : 0;
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    ctrl.id = V4L2_CID_VFLIP;
    ctrl.value = _flip ? 1 : 0;
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set vflip (val: %d)", ctrl.value);
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "set mirror %d, flip %d", _mirror, _flip);

    return XCAM_RETURN_NO_ERROR;

}

XCamReturn
SensorHw::set_mirror_flip(bool mirror, bool flip, int32_t& skip_frame_sequence)
{
    _mutex.lock();

    if (!is_activated()) {
        _flip = flip;
        _mirror = mirror;
        _set_mirror_flip();
        goto end;
    }

    if (_mirror != mirror || _flip != flip) {
        _flip = flip;
        _mirror = mirror;
        // will be set at _frame_sequence + 1
        _update_mirror_flip = true;
        // skip pre and current frame
        skip_frame_sequence = _frame_sequence;
        if (skip_frame_sequence < 0)
            skip_frame_sequence = 0;
    } else
        skip_frame_sequence = -1;

end:
    _mutex.unlock();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::get_mirror_flip(bool& mirror, bool& flip)
{
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HFLIP;
    if (io_control(VIDIOC_G_CTRL, &ctrl) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set hflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    mirror = ctrl.value ? true : false;

    ctrl.id = V4L2_CID_VFLIP;
    if (io_control(VIDIOC_G_CTRL, &ctrl) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set vflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    flip = ctrl.value ? true : false;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::set_exp_delay_info(int time_delay, int gain_delay, int hcg_lcg_mode_delay)
{
    _time_delay = time_delay;
    _gain_delay = gain_delay;
    _dcg_gain_mode_delay = hcg_lcg_mode_delay;

    LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s _time_delay: %d, _gain_delay:%d, _dcg_delay:%d\n",
                    __func__, _time_delay, _gain_delay, _dcg_gain_mode_delay);
    if (_time_delay > _gain_delay) {
        _gain_delayed = true;
    } else if (_time_delay == _gain_delay) {
        _gain_delayed = false;
    } else {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Not support gain's delay greater than time's delay!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (_dcg_gain_mode_delay > time_delay) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Not support dcg gain's delay %d, greater than time_delay %d!",
                        _dcg_gain_mode_delay, time_delay);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (_dcg_gain_mode_delay > 0 &&
            _dcg_gain_mode_delay != time_delay &&
            _dcg_gain_mode_delay != _gain_delay) {
        _dcg_gain_mode_delayed = true;
    }  else {
        if (_dcg_gain_mode_delay == time_delay)
            _dcg_gain_mode_with_time = true;
        else
            _dcg_gain_mode_with_time = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::start(bool prepared)
{
    ENTER_CAMHW_FUNCTION();
    V4l2SubDevice::start(prepared);
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::stop()
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    _exp_list.clear();
    _last_exp_time = nullptr;
    _last_exp_gain = nullptr;
    _last_dcg_gain_mode = nullptr;
    _effecting_exp_map.clear();
    _delayed_gain_list.clear();
    _delayed_dcg_gain_mode_list.clear();
    _pending_spilt_map.clear();
    _frame_sequence = -1;
    _first = true;
    set_sync_mode(NO_SYNC_MODE);
    V4l2SubDevice::stop();
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::composeExpParam
(
    RKAiqAecExpInfo_t* timeValid,
    RKAiqAecExpInfo_t* gainValid,
    RKAiqAecExpInfo_t* dcgGainModeValid,
    RKAiqAecExpInfo_t* newExp
)
{
    *newExp = *timeValid;
    if(_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        newExp->LinearExp.exp_sensor_params.analog_gain_code_global =
            gainValid->LinearExp.exp_sensor_params.analog_gain_code_global;
        newExp->LinearExp.exp_sensor_params.coarse_integration_time =
            timeValid->LinearExp.exp_sensor_params.coarse_integration_time;
        newExp->LinearExp.exp_real_params.dcg_mode =
            dcgGainModeValid->LinearExp.exp_real_params.dcg_mode;
    } else {
        newExp->HdrExp[2].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[2].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[2].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[2].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[2].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[2].exp_real_params.dcg_mode;
        newExp->HdrExp[1].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[1].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[1].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[1].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[1].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[1].exp_real_params.dcg_mode;
        newExp->HdrExp[0].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[0].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[0].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[0].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[0].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[0].exp_real_params.dcg_mode;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::set_effecting_exp_map(uint32_t sequence, void *exp_ptr, int mode)
{
    _mutex.lock();
    if (mode) {
        while (_effecting_exp_map.size() > 4)
            _effecting_exp_map.erase(_effecting_exp_map.begin());

        SmartPtr<RkAiqSensorExpParamsProxy> exp_param_prx = _expParamsPool->get_item();
        rk_aiq_frame_info_t off_finfo;
        memcpy(&off_finfo, exp_ptr, sizeof(off_finfo));
        exp_param_prx->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global = (uint32_t)off_finfo.normal_gain_reg;
        exp_param_prx->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time = (uint32_t)off_finfo.normal_exp_reg;
        exp_param_prx->data()->aecExpInfo.LinearExp.exp_real_params.analog_gain               = (float)off_finfo.normal_gain;
        exp_param_prx->data()->aecExpInfo.LinearExp.exp_real_params.integration_time          = (float)off_finfo.normal_exp;
        exp_param_prx->data()->aecExpInfo.LinearExp.exp_sensor_params.digital_gain_global = 1;
        exp_param_prx->data()->aecExpInfo.LinearExp.exp_sensor_params.isp_digital_gain = 1;
        exp_param_prx->data()->aecExpInfo.LinearExp.exp_real_params.digital_gain = 1.0f;
        exp_param_prx->data()->aecExpInfo.LinearExp.exp_real_params.isp_dgain = 1.0f;

        exp_param_prx->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global = (uint32_t)off_finfo.hdr_gain_l;
        exp_param_prx->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time = (uint32_t)off_finfo.hdr_exp_l;
        exp_param_prx->data()->aecExpInfo.HdrExp[2].exp_real_params.analog_gain               = (float)off_finfo.hdr_gain_l_reg;
        exp_param_prx->data()->aecExpInfo.HdrExp[2].exp_real_params.integration_time          = (float)off_finfo.hdr_exp_l_reg;
        exp_param_prx->data()->aecExpInfo.HdrExp[2].exp_sensor_params.digital_gain_global = 1;
        exp_param_prx->data()->aecExpInfo.HdrExp[2].exp_sensor_params.isp_digital_gain = 1;
        exp_param_prx->data()->aecExpInfo.HdrExp[2].exp_real_params.digital_gain = 1.0f;
        exp_param_prx->data()->aecExpInfo.HdrExp[2].exp_real_params.isp_dgain = 1.0f;

        exp_param_prx->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global = (uint32_t)off_finfo.hdr_gain_m;
        exp_param_prx->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time = (uint32_t)off_finfo.hdr_exp_m;
        exp_param_prx->data()->aecExpInfo.HdrExp[1].exp_real_params.analog_gain               = (float)off_finfo.hdr_gain_m_reg;
        exp_param_prx->data()->aecExpInfo.HdrExp[1].exp_real_params.integration_time          = (float)off_finfo.hdr_exp_m_reg;
        exp_param_prx->data()->aecExpInfo.HdrExp[1].exp_sensor_params.digital_gain_global = 1;
        exp_param_prx->data()->aecExpInfo.HdrExp[1].exp_sensor_params.isp_digital_gain = 1;
        exp_param_prx->data()->aecExpInfo.HdrExp[1].exp_real_params.digital_gain = 1.0f;
        exp_param_prx->data()->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain = 1.0f;


        exp_param_prx->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global = (uint32_t)off_finfo.hdr_gain_s;
        exp_param_prx->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time = (uint32_t)off_finfo.hdr_exp_s;
        exp_param_prx->data()->aecExpInfo.HdrExp[0].exp_real_params.analog_gain               = (float)off_finfo.hdr_gain_s_reg;
        exp_param_prx->data()->aecExpInfo.HdrExp[0].exp_real_params.integration_time          = (float)off_finfo.hdr_exp_s_reg;
        exp_param_prx->data()->aecExpInfo.HdrExp[0].exp_sensor_params.digital_gain_global = 1;
        exp_param_prx->data()->aecExpInfo.HdrExp[0].exp_sensor_params.isp_digital_gain = 1;
        exp_param_prx->data()->aecExpInfo.HdrExp[0].exp_real_params.digital_gain = 1.0f;
        exp_param_prx->data()->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain = 1.0f;

        _effecting_exp_map[sequence] = exp_param_prx;
    } else {
        if (!_effecting_exp_map.empty())
            _effecting_exp_map.clear();
        if (!_exp_list.empty())
            _exp_list.clear();
        if (!_pending_spilt_map.empty())
            _pending_spilt_map.clear();
        rk_aiq_exposure_params_t *senosrExp = (rk_aiq_exposure_params_t *)exp_ptr;
        SmartPtr<RkAiqSensorExpParamsProxy> exp_param_prx = _expParamsPool->get_item();
        memcpy(&exp_param_prx->data()->aecExpInfo, senosrExp, sizeof(rk_aiq_exposure_params_t));
        _effecting_exp_map[sequence] = exp_param_prx;

    }

    _mutex.unlock();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::set_pause_flag(bool isPause, uint32_t frameId, bool isSingleMode)
{
    _mutex.lock();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t new_exp_id, last_exp_id;
    RKAiqAecExpInfo_t *ptr_new_exp = NULL;
    mPauseFlag = isPause;
    if (mPauseFlag && _time_delay > 1) {
        mPauseId = frameId;
        mIsSingleMode = isSingleMode;
        new_exp_id = frameId + _time_delay;
        if (_effecting_exp_map.find(new_exp_id) != _effecting_exp_map.end()) {
            _effecting_exp_map.erase(new_exp_id);
            auto it = _effecting_exp_map.rbegin();
            ptr_new_exp = &it->second->data()->aecExpInfo;
            if(_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                ret = setLinearSensorExposure(ptr_new_exp);
            } else {
                ret = setHdrSensorExposure(ptr_new_exp);
            }
            if (ret != XCAM_RETURN_NO_ERROR)
                LOGE_CAMHW("set frame id %u exp again faile", it->first);

            LOGD_CAMHW("erase effect exp id %u, set new exp id is %u", new_exp_id, it->first);
        }
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "switch to %s mode, pauseId %u, handle sof id %u, _time_delay %d",
                                        mIsSingleMode ? "single" : "multi",
                                        mPauseId, _frame_sequence, _time_delay);
    }
    _mutex.unlock();
    return XCAM_RETURN_NO_ERROR;
}

} //namespace RkCam
