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

#include <linux/v4l2-subdev.h>
#include "LensHw.h"

//#define DISABLE_ZOOM_FOCUS

namespace RkCam {

uint16_t LensHw::DEFAULT_POOL_SIZE = 20;

LensHw::LensHw(const char* name)
    : V4l2SubDevice (name)
{
    ENTER_CAMHW_FUNCTION();
    _rec_sof_idx = 0;
    memset(_frame_time, 0, sizeof(_frame_time));
    memset(_frame_sequence, 0, sizeof(_frame_sequence));
    _afInfoPool = new RkAiqAfInfoPool("LensLocalAfInfoParams", LensHw::DEFAULT_POOL_SIZE);
    _irisInfoPool = new RkAiqIrisParamsPool("LensLocalIrisInfoParams", LensHw::DEFAULT_POOL_SIZE);
    _lenshw_thd = new LensHwHelperThd(this, 0);
    _lenshw_thd1 = new LensHwHelperThd(this, 1);
    _piris_step = -1;
    _max_logical_pos = 64;
    _iris_enable = false;
    _focus_enable = false;
    _zoom_enable = false;
    _zoom_correction = false;
    _focus_correction = false;
    _last_piris_step = -1;
    _dciris_pwmduty = 0;
    _last_dciris_pwmduty = 0;
    _hdciris_target = 0;
    _last_hdciris_target = 0;
    _focus_pos = 0;
    _zoom_pos = 0;
    _last_zoomchg_focus = 0;
    _last_zoomchg_zoom = 0;
    memset(_lowfv_fv4_4, 0, sizeof(_lowfv_fv4_4));
    memset(_lowfv_fv8_8, 0, sizeof(_lowfv_fv8_8));
    memset(_lowfv_highlht, 0, sizeof(_lowfv_highlht));
    memset(_lowfv_highlht2, 0, sizeof(_lowfv_highlht2));
    memset(_lowfv_seq, 0, sizeof(_lowfv_seq));
    _rec_lowfv_idx = 0;
    _otp_valid = false;
    _posture = 0.0;
    _hysteresis = 0.0;
    _startCurrent = 0.0;
    _endCurrent = 0.0;
    _angleZ = 0.0;

    EXIT_CAMHW_FUNCTION();
}

LensHw::~LensHw()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
LensHw::getOTPData()
{
    ENTER_CAMHW_FUNCTION();

    _otp_valid = false;
    _startCurrent = -1;
    _endCurrent = -1;
    _posture = -1;
    _hysteresis = -1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::queryLensSupport()
{
    ENTER_CAMHW_FUNCTION();

    _iris_enable = false;
    _focus_enable = false;
    _zoom_enable = false;

    if (!_name)
        return XCAM_RETURN_NO_ERROR;

    memset(&_iris_query, 0, sizeof(_iris_query));
    _iris_query.id = V4L2_CID_IRIS_ABSOLUTE;
    if (io_control(VIDIOC_QUERYCTRL, &_iris_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query iris ctrl failed");
        _iris_enable = false;
    } else {
        _iris_enable = true;
    }

    memset(&_focus_query, 0, sizeof(_focus_query));
    _focus_query.id = V4L2_CID_FOCUS_ABSOLUTE;
    if (io_control(VIDIOC_QUERYCTRL, &_focus_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query focus ctrl failed");
        _focus_enable = false;
    } else {
        _focus_enable = true;
    }

    memset(&_zoom_query, 0, sizeof(_zoom_query));
    _zoom_query.id = V4L2_CID_ZOOM_ABSOLUTE;
    if (io_control(VIDIOC_QUERYCTRL, &_zoom_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query zoom ctrl failed");
        _zoom_enable = false;
    } else {
        _zoom_enable = true;
    }

    getOTPData();

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::start_internal()
{
    ENTER_CAMHW_FUNCTION();

    if (_active)
        return XCAM_RETURN_NO_ERROR;

    _rec_sof_idx = 0;
    _rec_lowfv_idx = 0;
    _piris_step = 0;
    _last_piris_step = 0;
    _dciris_pwmduty = 0;
    _last_dciris_pwmduty = 0;
    _focus_pos = -1;
    _zoom_pos = -1;
    _angleZ = 0;
    _zoom_correction = false;
    _focus_correction = false;
    memset(&_focus_tim, 0, sizeof(_focus_tim));
    memset(&_zoom_tim, 0, sizeof(_zoom_tim));
    memset(_frame_time, 0, sizeof(_frame_time));
    memset(_frame_sequence, 0, sizeof(_frame_sequence));
    memset(_lowfv_fv4_4, 0, sizeof(_lowfv_fv4_4));
    memset(_lowfv_fv8_8, 0, sizeof(_lowfv_fv8_8));
    memset(_lowfv_highlht, 0, sizeof(_lowfv_highlht));
    memset(_lowfv_seq, 0, sizeof(_lowfv_seq));
    queryLensSupport();
    if (_zoom_enable) {
        _lenshw_thd->triger_start();
        _lenshw_thd->start();
        _lenshw_thd1->triger_start();
        _lenshw_thd1->start();
    }

    _active = true;
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::start(bool prepared)
{
    (void)prepared;
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    if (!_name)
        return XCAM_RETURN_NO_ERROR;

    start_internal();

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::stop()
{
    ENTER_CAMHW_FUNCTION();

    if (!_name)
        return XCAM_RETURN_NO_ERROR;

    if (_zoom_enable) {
        _lenshw_thd->triger_stop();
        _lenshw_thd->stop();
        _lenshw_thd1->triger_stop();
        _lenshw_thd1->stop();
    }

    _mutex.lock();
    _active = false;
    _mutex.unlock();
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getLensModeData(rk_aiq_lens_descriptor& lens_des)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    if (!_name)
        return XCAM_RETURN_NO_ERROR;

    if (!_active)
        queryLensSupport();
    lens_des.focus_support = _focus_enable;
    lens_des.iris_support = _iris_enable;
    lens_des.zoom_support = _zoom_enable;
    lens_des.focus_minimum = _focus_query.minimum;
    lens_des.focus_maximum = _focus_query.maximum;
    lens_des.zoom_minimum = _zoom_query.minimum;
    lens_des.zoom_maximum = _zoom_query.maximum;
    lens_des.otp_valid = _otp_valid;
    lens_des.posture = _posture;
    lens_des.hysteresis = _hysteresis;
    lens_des.startCurrent = _startCurrent;
    lens_des.endCurrent = _endCurrent;
    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_cfg cfg;

    if (!_name)
        return XCAM_RETURN_NO_ERROR;

    if (io_control (RK_VIDIOC_GET_VCM_CFG, &cfg) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get vcm cfg failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    lens_cfg.start_ma = cfg.start_ma;
    lens_cfg.rated_ma = cfg.rated_ma;
    lens_cfg.step_mode = cfg.step_mode;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_cfg cfg;

    if (!_name)
        return XCAM_RETURN_NO_ERROR;

    cfg.start_ma = lens_cfg.start_ma;
    cfg.rated_ma = lens_cfg.rated_ma;
    cfg.step_mode = lens_cfg.step_mode;
    if (io_control (RK_VIDIOC_SET_VCM_CFG, &cfg) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set vcm cfg failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGD_AF("%s: start_ma %d, rated_ma %d, step_mode %d",
            __func__, cfg.start_ma, cfg.rated_ma, cfg.step_mode);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LensHw::getLensVcmMaxlogpos(int& max_log_pos)
{
    ENTER_CAMHW_FUNCTION();

    if (!_name)
        return XCAM_RETURN_NO_ERROR;

    max_log_pos = _max_logical_pos;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LensHw::setLensVcmMaxlogpos(int& max_log_pos)
{
    ENTER_CAMHW_FUNCTION();

    struct v4l2_queryctrl focus_query;

    _max_logical_pos = max_log_pos;
    if (!_name)
        return XCAM_RETURN_NO_ERROR;

    if (io_control (RK_VIDIOC_SET_VCM_MAX_LOGICALPOS, &max_log_pos) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set vcm cfg failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&focus_query, 0, sizeof(focus_query));
    focus_query.id = V4L2_CID_FOCUS_ABSOLUTE;
    if (io_control(VIDIOC_QUERYCTRL, &focus_query) < 0) {
        LOGI_CAMHW_SUBM(LENS_SUBM, "query focus ctrl failed");
        return XCAM_RETURN_ERROR_IOCTL;
    } else {
        _focus_query = focus_query;
    }

    LOGD_AF("%s: max_log_pos %d", __func__, max_log_pos);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setFocusParamsSync(int position, bool is_update_time, bool focus_noreback)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_set_focus set_focus;
    unsigned long end_time;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (position < _focus_query.minimum)
        position = _focus_query.minimum;
    if (position > _focus_query.maximum)
        position = _focus_query.maximum;

    xcam_mem_clear (set_focus);
    set_focus.focus_pos = position;
    if ((position < _focus_pos) && !focus_noreback)
        set_focus.is_need_reback = true;

    if (io_control (RK_VIDIOC_FOCUS_SET_POSITION, &set_focus) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set focus result failed to device");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _focus_pos = position;

    if (!focus_noreback)
        _last_zoomchg_focus = position;

    struct rk_cam_vcm_tim tim;
    if (io_control (RK_VIDIOC_VCM_TIMEINFO, &tim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
        _mutex.lock();
        if (is_update_time)
            _focus_tim.vcm_end_t.tv_sec += 2;
        _mutex.unlock();
    } else {
        _mutex.lock();
        if (is_update_time)
            _focus_tim = tim;
        _mutex.unlock();

        end_time = _focus_tim.vcm_end_t.tv_sec * 1000 + _focus_tim.vcm_end_t.tv_usec / 1000;
        LOGD_CAMHW_SUBM(LENS_SUBM, "|||set focus result: %d, focus_pos %d, _last_zoomchg_focus %d, end time %ld, is_update_time %d, is_need_reback %d",
                        position, set_focus.focus_pos, _last_zoomchg_focus, end_time, is_update_time, set_focus.is_need_reback);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setFocusParams(SmartPtr<RkAiqFocusParamsProxy>& focus_params)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    rk_aiq_focus_params_t* p_focus = &focus_params->data()->result;

    if (!_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (_zoom_enable) {
        SmartPtr<rk_aiq_focus_params_t> attrPtr = new rk_aiq_focus_params_t;

        attrPtr->zoomfocus_modifypos = false;
        attrPtr->focus_correction = false;
        attrPtr->zoom_correction = false;
        attrPtr->lens_pos_valid = true;
        attrPtr->zoom_pos_valid = false;
        attrPtr->send_zoom_reback = p_focus->send_zoom_reback;
        attrPtr->send_focus_reback = p_focus->send_focus_reback;
        attrPtr->end_zoom_chg = p_focus->end_zoom_chg;
        attrPtr->focus_noreback = p_focus->focus_noreback;
        attrPtr->next_pos_num = 1;
        attrPtr->next_lens_pos[0] = p_focus->next_lens_pos[0];

        LOGI_AF("set focus position: %d", attrPtr->next_lens_pos[0]);
        _lenshw_thd->push_attr(attrPtr);
    } else {
        struct v4l2_control control;
        unsigned long start_time, end_time;
        int algo_pos, driver_pos;

        algo_pos = p_focus->next_lens_pos[0];
        driver_pos = algo_pos * (_focus_query.maximum - _focus_query.minimum) / _max_logical_pos + _focus_query.minimum;
        if (driver_pos < _focus_query.minimum)
            driver_pos = _focus_query.minimum;
        if (driver_pos > _focus_query.maximum)
            driver_pos = _focus_query.maximum;

        xcam_mem_clear (control);
        control.id = V4L2_CID_FOCUS_ABSOLUTE;
        control.value = driver_pos;

        if (io_control (VIDIOC_S_CTRL, &control) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "set focus result failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        _focus_pos = algo_pos;

        struct rk_cam_vcm_tim tim;
        if (io_control (RK_VIDIOC_VCM_TIMEINFO, &tim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        _focus_tim = tim;

        start_time = _focus_tim.vcm_start_t.tv_sec * 1000 + _focus_tim.vcm_start_t.tv_usec / 1000;
        end_time = _focus_tim.vcm_end_t.tv_sec * 1000 + _focus_tim.vcm_end_t.tv_usec / 1000;
        LOGI_AF("|||set focus: algo_pos %d, driver_pos %d, end time %ld, need time %d",
                algo_pos, driver_pos, end_time, end_time - start_time);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setPIrisParams(int step)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;

    if (!_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

    if (_piris_step == step)
        return XCAM_RETURN_NO_ERROR;

    //get old Piris-step
    _last_piris_step = _piris_step;

    xcam_mem_clear (control);
    control.id = V4L2_CID_IRIS_ABSOLUTE;
    control.value = step;

    LOGD_CAMHW_SUBM(LENS_SUBM, "|||set iris result: %d, control.value %d", step, control.value);
    if (io_control (VIDIOC_S_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set iris result failed to device");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _piris_step = step;

    struct rk_cam_motor_tim tim;
    if (io_control (RK_VIDIOC_IRIS_TIMEINFO, &tim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get iris timeinfo failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _piris_tim = tim;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setDCIrisParams(int pwmDuty)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;

    if (!_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

    if (_dciris_pwmduty == pwmDuty)
        return XCAM_RETURN_NO_ERROR;

    //get old DCiris-step
    _last_dciris_pwmduty = pwmDuty;

    xcam_mem_clear (control);
    control.id = V4L2_CID_IRIS_ABSOLUTE;
    control.value = pwmDuty;

    LOGD_CAMHW_SUBM(LENS_SUBM, "|||set dc-iris result: %d, control.value %d", pwmDuty, control.value);
    if (io_control (VIDIOC_S_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set dc-iris result failed to device");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _dciris_pwmduty = pwmDuty;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setHDCIrisParams(int target)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;

    if (!_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

    if (_hdciris_target == target)
        return XCAM_RETURN_NO_ERROR;

    //get old HDCiris-step
    _last_hdciris_target = _hdciris_target;

    xcam_mem_clear (control);
    control.id = V4L2_CID_IRIS_ABSOLUTE;
    control.value = target;

    LOGD_CAMHW_SUBM(LENS_SUBM, "|||set hdc-iris result: %d, control.value %d", target, control.value);
    if (io_control (VIDIOC_S_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set hdc-iris result failed to device");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _hdciris_target = target;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getHDCIrisParams(int* adc)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    if (!_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    int iris_adc = 0;
    // No work for now
    /*
    if (io_control(RK_VIDIOC_GET_DCIRIS_HALL_ADC, &iris_adc) < 0) {
        LOGD_CAMHW_SUBM(LENS_SUBM, "failed to get iris adc");
        *adc = 0;
        return XCAM_RETURN_ERROR_IOCTL;
    }
    */
    *adc = iris_adc;
    LOGD("|||get dciris adc value=%d\n", iris_adc);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getPIrisParams(int* step)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;

    if (!_iris_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "iris is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    xcam_mem_clear (control);
    control.id = V4L2_CID_IRIS_ABSOLUTE;

    if (io_control (VIDIOC_G_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get iris result failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    *step = control.value;
    LOGD_CAMHW_SUBM(LENS_SUBM, "|||get iris result: %d, control.value %d", *step, control.value);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setZoomParams(int position)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setZoomFocusRebackSync(SmartPtr<rk_aiq_focus_params_t> attrPtr, bool is_update_time)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_tim zoomtim, focustim;
    struct rk_cam_set_zoom set_zoom;
    struct v4l2_control control;
    unsigned long time0;
    int zoom_pos = 0, focus_pos = 0;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!_zoom_enable || !_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom or focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    xcam_mem_clear (set_zoom);
    set_zoom.setzoom_cnt = 1;
    if (attrPtr->send_zoom_reback)
        set_zoom.is_need_zoom_reback = true;
    if (attrPtr->send_focus_reback)
        set_zoom.is_need_focus_reback = true;
    _mutex.lock();
    zoom_pos = _zoom_pos;
    focus_pos = _focus_pos;
    _mutex.unlock();
    set_zoom.zoom_pos[0].zoom_pos = zoom_pos;
    set_zoom.zoom_pos[0].focus_pos = focus_pos;

    if (io_control (RK_VIDIOC_ZOOM_SET_POSITION, &set_zoom) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set zoom position failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (io_control (RK_VIDIOC_ZOOM_TIMEINFO, &zoomtim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom timeinfo failed");
        _mutex.lock();
        zoomtim = _zoom_tim;
        zoomtim.vcm_end_t.tv_sec += 1;
        _mutex.unlock();
    }

    if (io_control (RK_VIDIOC_VCM_TIMEINFO, &focustim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
        _mutex.lock();
        focustim = _focus_tim;
        focustim.vcm_end_t.tv_sec += 1;
        _mutex.unlock();
    }

    _mutex.lock();
    if (is_update_time) {
        _zoom_tim = zoomtim;
    }
    _zoom_pos = zoom_pos;
    _focus_pos = focus_pos;
    _mutex.unlock();

    time0 = _zoom_tim.vcm_end_t.tv_sec * 1000 + _zoom_tim.vcm_end_t.tv_usec / 1000;
    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, focus_pos %d, is_need_zoom_reback %d, is_need_focus_reback %d, end time %ld, is_update_time %d",
                    zoom_pos, focus_pos, set_zoom.is_need_zoom_reback, set_zoom.is_need_focus_reback, time0, is_update_time);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::endZoomChgSync(SmartPtr<rk_aiq_focus_params_t> attrPtr, bool is_update_time)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_tim zoomtim, focustim;
    struct rk_cam_set_zoom set_zoom;
    struct v4l2_control control;
    unsigned long time0, time1;
    int zoom_pos = 0, focus_pos = 0;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!_zoom_enable || !_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom or focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    xcam_mem_clear (set_zoom);
    if (attrPtr->end_zoom_chg) {
        set_zoom.setzoom_cnt = 1;
        _mutex.lock();
        zoom_pos = _zoom_pos;
        focus_pos = _focus_pos;
        _mutex.unlock();
        set_zoom.zoom_pos[0].zoom_pos = zoom_pos;
        set_zoom.zoom_pos[0].focus_pos = focus_pos;

        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, focus_pos %d, _last_zoomchg_zoom %d, _last_zoomchg_focus %d\n",
                        zoom_pos, focus_pos, _last_zoomchg_zoom, _last_zoomchg_focus);
        if (zoom_pos < _last_zoomchg_zoom)
            set_zoom.is_need_zoom_reback = true;
        else
            set_zoom.is_need_zoom_reback = false;

        if (focus_pos < _last_zoomchg_focus)
            set_zoom.is_need_focus_reback = true;
        else
            set_zoom.is_need_focus_reback = false;

        _last_zoomchg_zoom = zoom_pos;
        _last_zoomchg_focus = focus_pos;

        if (io_control (RK_VIDIOC_ZOOM_SET_POSITION, &set_zoom) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "set zoom position failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }

        if (io_control (RK_VIDIOC_ZOOM_TIMEINFO, &zoomtim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom timeinfo failed");
            _mutex.lock();
            zoomtim = _zoom_tim;
            zoomtim.vcm_end_t.tv_sec += 1;
            _mutex.unlock();
        }

        if (io_control (RK_VIDIOC_VCM_TIMEINFO, &focustim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
            _mutex.lock();
            focustim = _focus_tim;
            focustim.vcm_end_t.tv_sec += 1;
            _mutex.unlock();
        }

        time0 = zoomtim.vcm_end_t.tv_sec * 1000 + zoomtim.vcm_end_t.tv_usec / 1000;
        time1 = focustim.vcm_end_t.tv_sec * 1000 + focustim.vcm_end_t.tv_usec / 1000;
        if (time1 > time0)
            zoomtim = focustim;

        _mutex.lock();
        if (is_update_time) {
            _zoom_tim = zoomtim;
        }
        _zoom_pos = zoom_pos;
        _focus_pos = focus_pos;
        _mutex.unlock();

        time0 = _zoom_tim.vcm_end_t.tv_sec * 1000 + _zoom_tim.vcm_end_t.tv_usec / 1000;
        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, focus_pos %d, zoom focus move end time %ld, is_need_zoom_reback %d, is_need_focus_reback %d, is_update_time %d",
                        zoom_pos, focus_pos, time0, set_zoom.is_need_zoom_reback, set_zoom.is_need_focus_reback, is_update_time);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
LensHw::setZoomFocusParamsSync(SmartPtr<rk_aiq_focus_params_t> attrPtr, bool is_update_time)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_tim zoomtim, focustim;
    struct rk_cam_set_zoom set_zoom;
    struct v4l2_control control;
    unsigned long time0, time1;
    int zoom_pos = 0, focus_pos = 0;

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    if (!_zoom_enable || !_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom or focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!attrPtr->lens_pos_valid && !attrPtr->zoom_pos_valid) {
        return XCAM_RETURN_NO_ERROR;
    }

    xcam_mem_clear (set_zoom);
    if (attrPtr->lens_pos_valid || attrPtr->zoom_pos_valid) {
        set_zoom.setzoom_cnt = attrPtr->next_pos_num;
        set_zoom.is_need_zoom_reback = false;
        set_zoom.is_need_focus_reback = false;
        for (unsigned int i = 0; i < set_zoom.setzoom_cnt; i++) {
            zoom_pos = attrPtr->next_zoom_pos[i];
            focus_pos = attrPtr->next_lens_pos[i];

            if (zoom_pos < _zoom_query.minimum)
                zoom_pos = _zoom_query.minimum;
            if (zoom_pos > _zoom_query.maximum)
                zoom_pos = _zoom_query.maximum;

            if (focus_pos < _focus_query.minimum)
                focus_pos = _focus_query.minimum;
            if (focus_pos > _focus_query.maximum)
                focus_pos = _focus_query.maximum;

            set_zoom.zoom_pos[i].zoom_pos = zoom_pos;
            set_zoom.zoom_pos[i].focus_pos = focus_pos;
            LOGD_CAMHW_SUBM(LENS_SUBM, "i %d, zoom_pos %d, focus_pos %d\n", i, zoom_pos, focus_pos);
        }

        if (io_control (RK_VIDIOC_ZOOM_SET_POSITION, &set_zoom) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "set zoom position failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }

        if (io_control (RK_VIDIOC_ZOOM_TIMEINFO, &zoomtim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom timeinfo failed");
            _mutex.lock();
            zoomtim = _zoom_tim;
            zoomtim.vcm_end_t.tv_sec += 1;
            _mutex.unlock();
        }

        if (io_control (RK_VIDIOC_VCM_TIMEINFO, &focustim) < 0) {
            LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
            _mutex.lock();
            focustim = _focus_tim;
            focustim.vcm_end_t.tv_sec += 1;
            _mutex.unlock();
        }

        time0 = zoomtim.vcm_end_t.tv_sec * 1000 + zoomtim.vcm_end_t.tv_usec / 1000;
        time1 = focustim.vcm_end_t.tv_sec * 1000 + focustim.vcm_end_t.tv_usec / 1000;
        if (time1 > time0)
            zoomtim = focustim;

        _mutex.lock();
        if (is_update_time) {
            _zoom_tim = zoomtim;
        }
        _zoom_pos = zoom_pos;
        _focus_pos = focus_pos;
        _mutex.unlock();

        time0 = _zoom_tim.vcm_end_t.tv_sec * 1000 + _zoom_tim.vcm_end_t.tv_usec / 1000;
        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, focus_pos %d, zoom focus move end time %ld, is_need_zoom_reback %d, is_need_focus_reback %d, is_update_time %d",
                        zoom_pos, focus_pos, time0, set_zoom.is_need_zoom_reback, set_zoom.is_need_focus_reback, is_update_time);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setZoomFocusParams(SmartPtr<RkAiqFocusParamsProxy>& focus_params)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    rk_aiq_focus_params_t* p_focus = &focus_params->data()->result;
    int zoom_pos = 0, focus_pos = 0;

    if (!_zoom_enable || !_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom or focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    SmartPtr<rk_aiq_focus_params_t> attrPtr = new rk_aiq_focus_params_t;

    memset(attrPtr.ptr(), 0, sizeof(rk_aiq_focus_params_t));
    attrPtr->zoomfocus_modifypos = false;
    attrPtr->focus_correction = false;
    attrPtr->zoom_correction = false;
    attrPtr->lens_pos_valid = p_focus->lens_pos_valid;
    attrPtr->zoom_pos_valid = p_focus->zoom_pos_valid;
    attrPtr->send_zoom_reback = p_focus->send_zoom_reback;
    attrPtr->send_focus_reback = p_focus->send_focus_reback;
    attrPtr->end_zoom_chg = p_focus->end_zoom_chg;
    attrPtr->focus_noreback = false;
    if (attrPtr->lens_pos_valid || attrPtr->zoom_pos_valid) {
        attrPtr->next_pos_num = p_focus->next_pos_num;
        for (int i = 0; i < attrPtr->next_pos_num; i++) {
            zoom_pos = p_focus->next_zoom_pos[i];
            focus_pos = p_focus->next_lens_pos[i];

            if (zoom_pos < _zoom_query.minimum)
                zoom_pos = _zoom_query.minimum;
            if (zoom_pos > _zoom_query.maximum)
                zoom_pos = _zoom_query.maximum;

            if (focus_pos < _focus_query.minimum)
                focus_pos = _focus_query.minimum;
            if (focus_pos > _focus_query.maximum)
                focus_pos = _focus_query.maximum;

            attrPtr->next_zoom_pos[i] = zoom_pos;
            attrPtr->next_lens_pos[i] = focus_pos;
        }

        LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_pos %d, focus_pos %d", zoom_pos, focus_pos);
        _lenshw_thd->push_attr(attrPtr);
    } else if (attrPtr->send_zoom_reback || attrPtr->send_focus_reback) {
        LOGD_CAMHW_SUBM(LENS_SUBM, "send reback zoom_pos %d, focus_pos %d", _zoom_pos, _focus_pos);
        _lenshw_thd->push_attr(attrPtr);
    } else if (attrPtr->end_zoom_chg) {
        LOGD_CAMHW_SUBM(LENS_SUBM, "end_zoom_chg zoom_pos %d, focus_pos %d, next_pos_num %d",
                        _zoom_pos, _focus_pos, attrPtr->next_pos_num);
        _lenshw_thd->push_attr(attrPtr);
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getFocusParams(int* position)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;

    if (!_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    xcam_mem_clear (control);
    control.id = V4L2_CID_FOCUS_ABSOLUTE;

    if (io_control (VIDIOC_G_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get focus result failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    *position = control.value;
    LOGD_CAMHW_SUBM(LENS_SUBM, "|||get focus result: %d, control.value %d", *position, control.value);

    _focus_pos = *position;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getZoomParams(int* position)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    //struct v4l2_control control;

    if (!_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }
#if 0
    xcam_mem_clear (control);
    control.id = V4L2_CID_ZOOM_ABSOLUTE;

    if (io_control (VIDIOC_G_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom result failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    *position = control.value;
    LOGD_CAMHW_SUBM(LENS_SUBM, "|||get zoom result: %d, control.value %d", *position, control.value);

    _zoom_pos = *position;
#else
    *position = _zoom_pos;
#endif
    LOGD_CAMHW_SUBM(LENS_SUBM, "*position %d", *position);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::FocusCorrectionSync()
{
    ENTER_CAMHW_FUNCTION();
    int correction = 0;

    if (!_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "focus_correction start");
    if (io_control (RK_VIDIOC_FOCUS_CORRECTION, &correction) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus correction failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    _mutex.lock();
    _focus_pos = 0;
    _focus_correction = false;
    _mutex.unlock();
    LOGD_CAMHW_SUBM(LENS_SUBM, "focus_correction end");

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::FocusCorrection()
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    if (!_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    SmartPtr<rk_aiq_focus_params_t> attrPtr = new rk_aiq_focus_params_t;

    attrPtr->zoomfocus_modifypos = false;
    attrPtr->focus_correction = true;
    attrPtr->zoom_correction = false;

    LOGD_CAMHW_SUBM(LENS_SUBM, "focus_correction");
    _focus_correction = true;
    _lenshw_thd->push_attr(attrPtr);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::ZoomCorrectionSync()
{
    ENTER_CAMHW_FUNCTION();
    int correction = 0;

    if (!_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_correction start");
    if (io_control (RK_VIDIOC_ZOOM_CORRECTION, &correction) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom correction failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    _mutex.lock();
    _zoom_pos = 0;
    _zoom_correction = false;
    _mutex.unlock();
    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_correction end");

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::ZoomCorrection()
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    if (!_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    SmartPtr<rk_aiq_focus_params_t> attrPtr = new rk_aiq_focus_params_t;

    attrPtr->zoomfocus_modifypos = false;
    attrPtr->zoom_correction = true;
    attrPtr->focus_correction = false;

    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom_correction");
    _zoom_correction = true;
    _lenshw_thd1->push_attr(attrPtr);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::ZoomFocusModifyPositionSync(SmartPtr<rk_aiq_focus_params_t> attrPtr)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_modify_pos modify_pos;

    if (!_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (!attrPtr->use_manual) {
        modify_pos.zoom_pos = attrPtr->auto_zoompos;
        modify_pos.zoom1_pos = attrPtr->auto_zoompos;
        modify_pos.focus_pos = attrPtr->auto_focpos;
    } else {
        modify_pos.zoom_pos = attrPtr->manual_zoompos;
        modify_pos.zoom1_pos = attrPtr->manual_zoompos;
        modify_pos.focus_pos = attrPtr->manual_focpos;
    }
    if (io_control (RK_VIDIOC_MODIFY_POSITION, &modify_pos) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom focus modify position failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    _zoom_pos = modify_pos.zoom_pos;
    _focus_pos = modify_pos.focus_pos;
    _last_zoomchg_zoom = attrPtr->auto_zoompos;
    _last_zoomchg_focus = attrPtr->auto_focpos;

    LOGD_CAMHW_SUBM(LENS_SUBM, "zoom focus modify position, use_manual %d, zoom_pos %d, focus_pos %d",
                    attrPtr->use_manual, modify_pos.zoom_pos, modify_pos.focus_pos);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::ZoomFocusModifyPosition(SmartPtr<RkAiqFocusParamsProxy>& focus_params)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    rk_aiq_focus_params_t* p_focus = &focus_params->data()->result;

    if (!_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

#ifdef DISABLE_ZOOM_FOCUS
    return XCAM_RETURN_NO_ERROR;
#endif

    SmartPtr<rk_aiq_focus_params_t> attrPtr = new rk_aiq_focus_params_t;

    attrPtr->zoomfocus_modifypos = true;
    attrPtr->zoom_correction = false;
    attrPtr->focus_correction = false;
    attrPtr->use_manual = p_focus->use_manual;
    attrPtr->auto_focpos = p_focus->auto_focpos;
    attrPtr->auto_zoompos = p_focus->auto_zoompos;
    attrPtr->manual_focpos = p_focus->manual_focpos;
    attrPtr->manual_zoompos = p_focus->manual_zoompos;

    _lenshw_thd->push_attr(attrPtr);

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::handle_sof(int64_t time, uint32_t frameid)
{
    ENTER_CAMHW_FUNCTION();
    //SmartLock locker (_mutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int idx;

    idx = (_rec_sof_idx + 1) % LENSHW_RECORD_SOF_NUM;
    _frame_sequence[idx] = frameid;
    _frame_time[idx] = time;
    _rec_sof_idx = idx;

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %u, time %lld\n", __func__, frameid, time);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn
LensHw::setLowPassFv(uint32_t sub_shp4_4[RKAIQ_RAWAF_SUMDATA_NUM], uint32_t sub_shp8_8[RKAIQ_RAWAF_SUMDATA_NUM],
                     uint32_t high_light[RKAIQ_RAWAF_SUMDATA_NUM], uint32_t high_light2[RKAIQ_RAWAF_SUMDATA_NUM], uint32_t frameid)
{
    ENTER_CAMHW_FUNCTION();
    //SmartLock locker (_mutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int idx;
    int64_t lowPassFv4_4, lowPassFv8_8, lowPassLight;

    if (!_active)
        start_internal();

    idx = (_rec_lowfv_idx + 1) % LENSHW_RECORD_LOWPASSFV_NUM;
    _lowfv_seq[idx] = frameid;
    memcpy(&_lowfv_fv4_4[idx], sub_shp4_4, sizeof(_lowfv_fv4_4[idx]));
    memcpy(&_lowfv_fv8_8[idx], sub_shp8_8, sizeof(_lowfv_fv8_8[idx]));
    memcpy(&_lowfv_highlht[idx], high_light, sizeof(_lowfv_highlht[idx]));
    memcpy(&_lowfv_highlht2[idx], high_light2, sizeof(_lowfv_highlht2[idx]));
    _rec_lowfv_idx = idx;

    lowPassFv4_4 = 0;
    lowPassFv8_8 = 0;
    lowPassLight = 0;
    for (int i = 0; i < RKAIQ_RAWAF_SUMDATA_NUM; i++) {
        lowPassFv4_4 += sub_shp4_4[i];
        lowPassFv8_8 += sub_shp8_8[i];
        lowPassLight += high_light[i];
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %d, lowPassFv4_4 %lld, lowPassFv8_8 %lld, lowPassLight %lld\n",
                    __func__, frameid, lowPassFv4_4, lowPassFv8_8, lowPassLight);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn
LensHw::getIrisInfoParams(SmartPtr<RkAiqIrisParamsProxy>& irisParams, uint32_t frame_id)
{
    ENTER_CAMHW_FUNCTION();
    //SmartLock locker (_mutex);

    int i;

    irisParams = NULL;
    if (_irisInfoPool->has_free_items()) {
        irisParams = (SmartPtr<RkAiqIrisParamsProxy>)_irisInfoPool->get_item();
    } else {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    for (i = 0; i < LENSHW_RECORD_SOF_NUM; i++) {
        if (frame_id == _frame_sequence[i])
            break;
    }

    //Piris
    irisParams->data()->PIris.StartTim = _piris_tim.motor_start_t;
    irisParams->data()->PIris.EndTim = _piris_tim.motor_end_t;
    irisParams->data()->PIris.laststep = _last_piris_step;
    irisParams->data()->PIris.step = _piris_step;

    //DCiris

    if (i < LENSHW_RECORD_SOF_NUM) {
        irisParams->data()->sofTime = _frame_time[i];
    } else {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s: frame_id %u, can not find sof time!\n", __FUNCTION__, frame_id);
        return  XCAM_RETURN_ERROR_PARAM;
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %u, time %lld\n", __func__, frame_id, irisParams->data()->sofTime);

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getAfInfoParams(SmartPtr<RkAiqAfInfoProxy>& afInfo, uint32_t frame_id)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    int i;

    afInfo = NULL;
    if (_afInfoPool->has_free_items()) {
        afInfo = (SmartPtr<RkAiqAfInfoProxy>)_afInfoPool->get_item();
    } else {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    for (i = 0; i < LENSHW_RECORD_SOF_NUM; i++) {
        if (frame_id == _frame_sequence[i])
            break;
    }

    afInfo->data()->focusStartTim = _focus_tim.vcm_start_t;
    afInfo->data()->focusEndTim = _focus_tim.vcm_end_t;
    afInfo->data()->zoomStartTim = _zoom_tim.vcm_start_t;
    afInfo->data()->zoomEndTim = _zoom_tim.vcm_end_t;
    afInfo->data()->focusCode = _focus_pos;
    afInfo->data()->zoomCode = _zoom_pos;
    afInfo->data()->angleZ = _angleZ;
    afInfo->data()->zoomCorrection = _zoom_correction;
    afInfo->data()->focusCorrection = _focus_correction;
    if (i < LENSHW_RECORD_SOF_NUM) {
        afInfo->data()->sofTime = _frame_time[i];
    } else {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s: frame_id %d, can not find sof time!\n", __FUNCTION__, frame_id);
        return  XCAM_RETURN_ERROR_PARAM;
    }

    for (i = 0; i < LENSHW_RECORD_LOWPASSFV_NUM; i++) {
        if (frame_id == _lowfv_seq[i] + 1)
            break;
    }

    if (i < LENSHW_RECORD_SOF_NUM) {
        afInfo->data()->lowPassId = _lowfv_seq[i];
        memcpy(afInfo->data()->lowPassFv4_4,
               _lowfv_fv4_4[i], RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memcpy(afInfo->data()->lowPassFv8_8,
               _lowfv_fv8_8[i], RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memcpy(afInfo->data()->lowPassHighLht,
               _lowfv_highlht[i], RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memcpy(afInfo->data()->lowPassHighLht2,
               _lowfv_highlht2[i], RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
    } else {
        afInfo->data()->lowPassId = 0;
        memset(afInfo->data()->lowPassFv4_4, 0, RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memset(afInfo->data()->lowPassFv8_8, 0, RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memset(afInfo->data()->lowPassHighLht, 0, RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
        memset(afInfo->data()->lowPassHighLht2, 0, RKAIQ_RAWAF_SUMDATA_NUM * sizeof(int32_t));
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %u, time %lld, lowPassFv4_4[0] %d, lowPassId %d\n",
                    __func__, frame_id, afInfo->data()->sofTime, afInfo->data()->lowPassFv4_4[0], afInfo->data()->lowPassId);


    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %u, time %lld\n", __func__, frame_id, afInfo->data()->sofTime);

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setAngleZ(float angleZ)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    _angleZ = angleZ;
    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

bool LensHwHelperThd::loop()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    const static int32_t timeout = -1;
    SmartPtr<rk_aiq_focus_params_t> attrib = mAttrQueue.pop (timeout);

    if (!attrib.ptr()) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "LensHwHelperThd got empty attrib, stop thread");
        return false;
    }

    if (attrib->zoomfocus_modifypos) {
        mLensHw->ZoomFocusModifyPositionSync(attrib);
    } else if (attrib->focus_correction) {
        mLensHw->FocusCorrectionSync();
    } else if (attrib->zoom_correction) {
        mLensHw->ZoomCorrectionSync();
    } else if (attrib->lens_pos_valid == 1 && attrib->zoom_pos_valid == 0) {
        if (attrib->end_zoom_chg) {
            ret = mLensHw->endZoomChgSync(attrib, true);
        }
        ret = mLensHw->setFocusParamsSync(attrib->next_lens_pos[0], true, attrib->focus_noreback);
    } else {
        if (attrib->send_zoom_reback == 1 || attrib->send_focus_reback == 1) {
            mLensHw->setZoomFocusRebackSync(attrib, false);
        }
        if (attrib->end_zoom_chg) {
            ret = mLensHw->setZoomFocusParamsSync(attrib, false);
            ret = mLensHw->endZoomChgSync(attrib, true);
        } else {
            ret = mLensHw->setZoomFocusParamsSync(attrib, true);
        }
    }
    if (ret == XCAM_RETURN_NO_ERROR)
        return true;

    LOGE_CAMHW_SUBM(LENS_SUBM, "LensHwHelperThd failed to run command!");

    EXIT_CAMHW_FUNCTION();

    return false;
}

} //namespace RkCam
