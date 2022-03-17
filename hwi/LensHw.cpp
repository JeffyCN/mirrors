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
    _piris_step = -1;
    EXIT_CAMHW_FUNCTION();
}

LensHw::~LensHw()
{
    ENTER_CAMHW_FUNCTION();
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
LensHw::queryLensSupport()
{
    ENTER_CAMHW_FUNCTION();

    _iris_enable = true;
    _focus_enable = true;
    _zoom_enable = true;

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
    _piris_step = 0;
    _last_piris_step = 0;
    _dciris_pwmduty = 0;
    _last_dciris_pwmduty = 0;
    _focus_pos = -1;
    _zoom_pos = -1;
    memset(&_focus_tim, 0, sizeof(_focus_tim));
    memset(&_zoom_tim, 0, sizeof(_zoom_tim));
    memset(_frame_time, 0, sizeof(_frame_time));
    memset(_frame_sequence, 0, sizeof(_frame_sequence));
    queryLensSupport();

    _active = true;
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::start()
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    start_internal();

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::stop()
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    _active = false;
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getLensModeData(rk_aiq_lens_descriptor& lens_des)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);

    queryLensSupport();
    lens_des.focus_support = _focus_enable;
    lens_des.iris_support = _iris_enable;
    lens_des.zoom_support = _zoom_enable;
    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg)
{
    ENTER_CAMHW_FUNCTION();
    struct rk_cam_vcm_cfg cfg;

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

    cfg.start_ma = lens_cfg.start_ma;
    cfg.rated_ma = lens_cfg.rated_ma;
    cfg.step_mode = lens_cfg.step_mode;
    if (io_control (RK_VIDIOC_SET_VCM_CFG, &cfg) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set vcm cfg failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::setFocusParams(int position)
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;

    if (!_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

    if (position < _focus_query.minimum)
        position = _focus_query.minimum;
    if (position > _focus_query.maximum)
        position = _focus_query.maximum;

    xcam_mem_clear (control);
    control.id = V4L2_CID_FOCUS_ABSOLUTE;
    control.value = position;

    LOGD_CAMHW_SUBM(LENS_SUBM, "|||set focus result: %d, control.value %d", position, control.value);
    if (io_control (VIDIOC_S_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set focus result failed to device");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _focus_pos = position;

    struct rk_cam_vcm_tim tim;
    if (io_control (RK_VIDIOC_VCM_TIMEINFO, &tim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get focus timeinfo failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _focus_tim = tim;

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

    //get old Piris-step
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
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;

    if (!_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!_active)
        start_internal();

    if (position < _zoom_query.minimum)
        position = _zoom_query.minimum;
    if (position > _zoom_query.maximum)
        position = _zoom_query.maximum;
    xcam_mem_clear (control);
    control.id = V4L2_CID_ZOOM_ABSOLUTE;
    control.value = position;

    LOGD_CAMHW_SUBM(LENS_SUBM, "||| set zoom result: %d, control.value %d", position, control.value);
    if (io_control (VIDIOC_S_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "set zoom result failed to device");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _zoom_pos = position;

    struct rk_cam_vcm_tim tim;
    if (io_control (RK_VIDIOC_ZOOM_TIMEINFO, &tim) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom timeinfo failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    _zoom_tim = tim;

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
    struct v4l2_control control;

    if (!_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    xcam_mem_clear (control);
    control.id = V4L2_CID_ZOOM_ABSOLUTE;

    if (io_control (VIDIOC_G_CTRL, &control) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "get zoom result failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }
    *position = control.value;
    LOGD_CAMHW_SUBM(LENS_SUBM, "|||get zoom result: %d, control.value %d", *position, control.value);

    _zoom_pos = *position;

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::FocusCorrection()
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;
    int correction = 0;

    if (!_focus_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (io_control (RK_VIDIOC_FOCUS_CORRECTION, &correction) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "focus correction failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::ZoomCorrection()
{
    ENTER_CAMHW_FUNCTION();
    SmartLock locker (_mutex);
    struct v4l2_control control;
    int correction = 0;

    if (!_zoom_enable) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom is not supported");
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (io_control (RK_VIDIOC_ZOOM_CORRECTION, &correction) < 0) {
        LOGE_CAMHW_SUBM(LENS_SUBM, "zoom correction failed");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::handle_sof(int64_t time, int frameid)
{
    ENTER_CAMHW_FUNCTION();
    //SmartLock locker (_mutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int idx;

    idx = (_rec_sof_idx + 1) % LENSHW_RECORD_SOF_NUM;
    _frame_sequence[idx] = frameid;
    _frame_time[idx] = time;
    _rec_sof_idx = idx;

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %d, time %lld\n", __func__, frameid, time);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn
LensHw::getIrisInfoParams(SmartPtr<RkAiqIrisParamsProxy>& irisParams, int frame_id)
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
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s: frame_id %d, can not find sof time!\n", __FUNCTION__, frame_id);
        return  XCAM_RETURN_ERROR_PARAM;
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %d, time %lld\n", __func__, frame_id, irisParams->data()->sofTime);

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
LensHw::getAfInfoParams(SmartPtr<RkAiqAfInfoProxy>& afInfo, int frame_id)
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
    if (i < LENSHW_RECORD_SOF_NUM) {
        afInfo->data()->sofTime = _frame_time[i];
    } else {
        LOGE_CAMHW_SUBM(LENS_SUBM, "%s: frame_id %d, can not find sof time!\n", __FUNCTION__, frame_id);
        return  XCAM_RETURN_ERROR_PARAM;
    }

    LOGD_CAMHW_SUBM(LENS_SUBM, "%s: frm_id %d, time %lld\n", __func__, frame_id, afInfo->data()->sofTime);

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

}; //namespace RkCam
