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
    , _first(true)
    , _working_mode(RK_AIQ_WORKING_MODE_NORMAL)
{
    ENTER_CAMHW_FUNCTION();
    _last_exp_time = nullptr;
    _last_exp_gain = nullptr;
    _gain_delay = 0;
    _time_delay = 0;
    _gain_delayed = false;
    _frame_sequence = 0;
    _dcg_gain_mode_delay = 0;
    _dcg_gain_mode_delayed = false;
    _expParamsPool = new RkAiqExpParamsPool("SensorLocalExpParams", SensorHw::DEFAULT_POOL_SIZE);
    _flip = false;
    _mirror = false;
    _update_mirror_flip = false;
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

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x,\n"
                    "l-dcg %d, m-dcg %d, s-dcg %d\n",
                    __FUNCTION__,
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

    if (expPar->LinearExp.exp_sensor_params.analog_gain_code_global >= 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_ANALOGUE_GAIN;
        ctrl.value = expPar->LinearExp.exp_sensor_params.analog_gain_code_global;
        if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to  set again result(val: %d)", ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
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
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdrExp exp");
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
    LOGD_CAMHW_SUBM(SENSOR_SUBM, "enable:%d,single:%d,multi:%d,total:%d", dpcc_cfg.enable,
                    dpcc_cfg.cur_single_dpcc, dpcc_cfg.cur_multiple_dpcc, dpcc_cfg.total_dpcc);
    if (io_control(RKMODULE_SET_DPCC_CFG, &dpcc_cfg) < 0) {
        //LOGD_CAMHW_SUBM(SENSOR_SUBM,"failed to set sensor dpcc");
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

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: frameid: %d, a-gain: %d, time: %d, dcg: %d, snr: %d\n",
                    __FUNCTION__,
                    _frame_sequence,
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
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set vblank result(val: %d)", ctrl.value);
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
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to set conversion gain !");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    if (expPar->LinearExp.exp_sensor_params.analog_gain_code_global >= 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_ANALOGUE_GAIN;
        ctrl.value = expPar->LinearExp.exp_sensor_params.analog_gain_code_global;
        if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to  set again result(val: %d)", ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    if (expPar->LinearExp.exp_sensor_params.digital_gain_global != 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_GAIN;
        ctrl.value = expPar->LinearExp.exp_sensor_params.digital_gain_global;
        if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to set dgain result(val: %d)", ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    if (expPar->LinearExp.exp_sensor_params.coarse_integration_time != 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_EXPOSURE;
        ctrl.value = expPar->LinearExp.exp_sensor_params.coarse_integration_time;
        if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to set dgain result(val: %d)", ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
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
SensorHw::get_format(rk_aiq_exposure_sensor_descriptor* sns_des)
{
    struct v4l2_subdev_format fmt;
    uint32_t format_code;

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
        //LOGD_CAMHW_SUBM(SENSOR_SUBM,"failed to get sensor nr switch");
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

XCamReturn
SensorHw::get_sensor_descriptor(rk_aiq_exposure_sensor_descriptor *sns_des)
{
    memset(sns_des, 0, sizeof(rk_aiq_exposure_sensor_descriptor));

    if (get_format(sns_des))
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

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    SmartPtr<RKAiqAecExpInfoWrapper_t> exp = expPar->data();

    if (_first) {
        if (exp->algo_id == 0) {
            if (exp->exp_tbl_size > 0) {
                int lastIdx = exp->exp_tbl_size - 1;
                exp->aecExpInfo.LinearExp = exp->exp_tbl[lastIdx].LinearExp;
                exp->aecExpInfo.HdrExp[0] = exp->exp_tbl[lastIdx].HdrExp[0];
                exp->aecExpInfo.HdrExp[1] = exp->exp_tbl[lastIdx].HdrExp[1];
                exp->aecExpInfo.HdrExp[2] = exp->exp_tbl[lastIdx].HdrExp[2];
                exp->aecExpInfo.frame_length_lines = exp->exp_tbl[lastIdx].frame_length_lines;
                exp->aecExpInfo.CISFeature.SNR = exp->exp_tbl[lastIdx].CISFeature.SNR;
            }
        }
        if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
            setLinearSensorExposure(&exp->aecExpInfo);
        else
            setHdrSensorExposure(&exp->aecExpInfo);

        setSensorDpcc(&exp->SensorDpccInfo);

        _effecting_exp_map[0] = expPar;
        _first = false;
        _last_exp_time = expPar;
        _last_exp_gain = expPar;
        _last_dcg_gain_mode = expPar;
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "exp-sync: first set exp, add id[0] to the effected exp map\n");
    } else {
        if (exp->algo_id == 0) {
            if (exp->exp_tbl_size > 0) {
                SmartPtr<RkAiqExpParamsProxy> expParamsProxy = NULL;

                LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: exp_tbl_size:%d, exp_list remain:%d\n", __FUNCTION__, exp->exp_tbl_size, _exp_list.size());
                /* when new exp-table comes, remove elem until meet the first one of last exp-table */
                if (!_exp_list.empty()) {
                    _exp_list.erase(std::remove_if(
                        _exp_list.begin(), _exp_list.end(),
                        [](const std::pair<SmartPtr<RkAiqExpParamsProxy>, bool>& p) {
                            return !p.second;
                        }), _exp_list.end());
                }

                SmartPtr<RKAiqAecExpInfoWrapper_t> tmp;
                for(int i = 0; i < exp->exp_tbl_size; i++) {
                    if (_expParamsPool->has_free_items()) {
                        expParamsProxy = (SmartPtr<RkAiqExpParamsProxy>)_expParamsPool->get_item();
                        tmp = expParamsProxy->data();
                    } else {
                        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
                        return XCAM_RETURN_ERROR_MEM;
                    }
                    //memcpy(exp.ptr(), expPar.ptr(), sizeof(*(exp.ptr())));
                    *(tmp.ptr()) = *(exp.ptr());
                    tmp->aecExpInfo.LinearExp = tmp->exp_tbl[i].LinearExp;
                    tmp->aecExpInfo.HdrExp[0] = tmp->exp_tbl[i].HdrExp[0];
                    tmp->aecExpInfo.HdrExp[1] = tmp->exp_tbl[i].HdrExp[1];
                    tmp->aecExpInfo.HdrExp[2] = tmp->exp_tbl[i].HdrExp[2];
                    tmp->aecExpInfo.frame_length_lines = tmp->exp_tbl[i].frame_length_lines;
                    tmp->aecExpInfo.CISFeature.SNR = tmp->exp_tbl[i].CISFeature.SNR;


                    /* set a flag when it's fisrt elem of exp-table*/
                    _exp_list.push_back(std::make_pair(expParamsProxy, (i == 0 ? true : false)));

                    if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                        LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s:add tbl[%d] to list: a-gain: %d, time: %d, snr: %d\n",
                                        __FUNCTION__, i,
                                        tmp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                                        tmp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                                        tmp->aecExpInfo.CISFeature.SNR);
                    } else {
                        LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s:add tbl[%d] to list: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                                        __FUNCTION__, i,
                                        tmp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                                        tmp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                                        tmp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                                        tmp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                                        tmp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                                        tmp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
                    }
                }
            }
        }
        else {
            _exp_list.push_back(std::make_pair(expPar, true));

            if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s:add to list: a-gain: %d, time: %d\n",
                                __FUNCTION__,
                                exp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                                exp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);
            } else {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s:add to list: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                                __FUNCTION__,
                                exp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                                exp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                                exp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                                exp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                                exp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                                exp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
            }
        }
    }
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SensorHw::getEffectiveExpParams(SmartPtr<RkAiqExpParamsProxy>& expParams, int frame_id)
{
    ENTER_CAMHW_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    std::map<int, SmartPtr<RkAiqExpParamsProxy>>::iterator it;
    int search_id = frame_id < 0 ? 0 : frame_id;
//#ifdef ADD_LOCK
    SmartLock locker (_mutex);
//#endif

    it = _effecting_exp_map.find(search_id);
    // havn't found
    if (it == _effecting_exp_map.end()) {
        /* use the latest */
        std::map<int, SmartPtr<RkAiqExpParamsProxy>>::reverse_iterator rit;

        for (rit = _effecting_exp_map.rbegin(); rit != _effecting_exp_map.rend(); rit++) {
            if (rit->first <= search_id)
                break;
        }

        if (rit == _effecting_exp_map.rend()) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "can't find the latest effecting exposure for id %d, impossible case !", search_id);
            return  XCAM_RETURN_ERROR_PARAM;
        }

        expParams = rit->second;
        if (expParams.ptr()) {
            if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: search_id: %d, get-last %d, a-gain: %d, time: %d\n",
                                __FUNCTION__, search_id, rit->first,
                                expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);
            } else {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: search_id: %d, get-last %d, lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                                __FUNCTION__, search_id, rit->first,
                                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
            }
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: expParams is invalid!",__FUNCTION__);
        }
        ret = XCAM_RETURN_BYPASS;
    } else {
        expParams = it->second;
        if (expParams.ptr()) {
            if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: search_id: %d, get-find %d, a-gain: %d, time: %d\n",
                                __FUNCTION__, search_id, it->first,
                                expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);
            } else {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: search_id: %d, get-find %d, lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                                __FUNCTION__, search_id, it->first,
                                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                                expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
            }
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: expParams is invalid!",__FUNCTION__);
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

XCamReturn
SensorHw::handle_sof(int64_t time, int frameid)
{
    ENTER_CAMHW_FUNCTION();
    int effecting_frame_id = 0;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    _mutex.lock();
    if (frameid - _frame_sequence > 1)
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "!!!!frame losed,last frameid:%d,current farmeid:%d!!!!\n", _frame_sequence, frameid);

    _frame_sequence = frameid;
    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: frameid=%d, exp_list size=%d, gain_list size=%d",
                    __FUNCTION__, frameid, _exp_list.size(), _delayed_gain_list.size());

    SmartPtr<RkAiqExpParamsProxy> exp_time = nullptr;
    SmartPtr<RkAiqExpParamsProxy> exp_gain = nullptr;
    SmartPtr<RkAiqExpParamsProxy> dcg_gain_mode = nullptr;
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
    // update flip, skip _frame_sequence
    if (_update_mirror_flip) {
        _set_mirror_flip();
        _update_mirror_flip = false;
    }

    _mutex.unlock();
    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: working_mode=%d,frameid=%d, status: set_time=%d,set_gain=%d\n",
                    __FUNCTION__, _working_mode, frameid, set_time, set_gain);

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
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: sof_id[%d]: set exposure failed!!!\n",
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
        _effecting_exp_map[effecting_frame_id] = exp_time;

        if (_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: sof_id[%d], _effecting_exp_map: add %d, a-gain: %d, time: %d, snr: %d\n",
                            __FUNCTION__, frameid, effecting_frame_id,
                            exp_time->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                            exp_time->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                            exp_time->data()->aecExpInfo.CISFeature.SNR);
        } else {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: sof_id[%d], _effecting_exp_map: add %d, lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x\n",
                            __FUNCTION__,  frameid, effecting_frame_id,
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
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdr mode to %d", mode);
        return XCAM_RETURN_ERROR_FAILED;
    }
    hdr_cfg.hdr_mode = hdr_mode;
    if (io_control(RKMODULE_SET_HDR_CFG, &hdr_cfg) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM,"failed to set hdr mode %d", hdr_mode);
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
    if (io_control(VIDIOC_S_CTRL, &ctrl) < 0) {
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

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s _time_delay: %d, _gain_delay:%d, _dcg_delay:%d\n",
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
    _frame_sequence = 0;
    _first = true;
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

}; //namespace RkCam
