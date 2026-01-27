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

#include "FlashLight.h"
#include "xcam_log.h"
#include "linux/rk-led-flash.h"

namespace RkCam {

FlashLightHw::FlashLightHw(std::string name[], int num)
{
    if (num >= FLASH_MAX_NUM) {
        LOGE_CAMHW_SUBM(FL_SUBM, "not support flash num %d", num);
        return ;
    }

    _dev_num = 0;
    for (int i = 0 ; i < num; i++) {
        _fl_device[i] = new V4l2SubDevice(name[i].c_str());
        _dev_num++;
    }
    _active_fl_num = 0;
    _keep_status = false;
    xcam_mem_clear (_flash_settings);
    memset(_v4l_flash_info, 0, sizeof(struct rk_aiq_v4l_flash_info_s) * FLASH_MAX_NUM);
}

FlashLightHw::~FlashLightHw()
{

}

XCamReturn
FlashLightHw::init(int active_num)
{
    XCAM_ASSERT (active_num <= _dev_num);

    for (int i = 0 ; i < active_num; i++) {
        _fl_device[i]->open();
        _active_fl_num++;
    }

    get_flash_info ();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FlashLightHw::deinit()
{
    if (!_keep_status) {
        v4l_set_params(RK_AIQ_FLASH_MODE_OFF, 0, 0, false);
        xcam_mem_clear (_flash_settings);
    }
    for (int i = 0 ; i < _active_fl_num; i++) {
        _fl_device[i]->close();
    }
    _active_fl_num = 0;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FlashLightHw::start()
{
    for (int i = 0 ; i < _active_fl_num; i++) {
        _fl_device[i]->start();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FlashLightHw::stop()
{
    if (!_keep_status) {
        v4l_set_params(RK_AIQ_FLASH_MODE_OFF, 0, 0, false);
        xcam_mem_clear (_flash_settings);
    }
    for (int i = 0 ; i < _active_fl_num; i++) {
        _fl_device[i]->stop();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FlashLightHw::set_params(rk_aiq_flash_setting_t& flash_settings)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // set flash if needed
    rk_aiq_flash_setting_t* old_flash_settings = &_flash_settings;
    if ((old_flash_settings->flash_mode != flash_settings.flash_mode) ||
            (old_flash_settings->strobe != flash_settings.strobe) ||
            (old_flash_settings->power[0] != flash_settings.power[0]) ||
            (old_flash_settings->power[1] != flash_settings.power[1])
       ) {
       LOGD_CAMHW_SUBM(FL_SUBM, "flash_settings: mode:%d,power:%f,timeout_ms:%d,strobe:%d",
                             flash_settings.flash_mode, flash_settings.power[0],
                             flash_settings.timeout_ms, flash_settings.strobe);
        ret = v4l_set_params(flash_settings.flash_mode, flash_settings.power,
                             flash_settings.timeout_ms, flash_settings.strobe);
        if (!ret)
            _flash_settings = flash_settings;
    }

    return ret;
}

XCamReturn
FlashLightHw::get_status (rk_aiq_flash_setting_t& flash_settings, uint32_t frame_id)
{
    if (!_active_fl_num)
        return XCAM_RETURN_ERROR_FAILED;

    flash_settings = _flash_settings;

    if (_fl_device[0].ptr()) {
        struct timeval flash_time;

        if (_fl_device[0]->io_control (RK_VIDIOC_FLASH_TIMEINFO, &flash_time) < 0) {
            LOGE_CAMHW_SUBM(FL_SUBM, " get RK_VIDIOC_FLASH_TIMEINFO failed. cmd = 0x%x", RK_VIDIOC_FLASH_TIMEINFO);
            /* return XCAM_RETURN_ERROR_IOCTL; */
        }
        flash_settings.effect_ts = (int64_t)flash_time.tv_sec * 1000 * 1000 +
                                   (int64_t)flash_time.tv_usec;
        LOGD_CAMHW_SUBM(FL_SUBM, "frameid %u, get RK_VIDIOC_FLASH_TIMEINFO flash ts %lld",
                        frame_id, flash_settings.effect_ts);
    }

    // for the following case:
    // 1) set to flash mode
    // 2) one flash power set to 0
    // then we can't get the effect ts from the node which power set to 0
    if (_fl_device[1].ptr() && flash_settings.effect_ts == 0 &&
            flash_settings.power[0] != flash_settings.power[1]) {
        struct timeval flash_time;

        if (_fl_device[1]->io_control (RK_VIDIOC_FLASH_TIMEINFO, &flash_time) < 0) {
            LOGE_CAMHW_SUBM(FL_SUBM, " get RK_VIDIOC_FLASH_TIMEINFO failed. cmd = 0x%x", RK_VIDIOC_FLASH_TIMEINFO);
            /* return XCAM_RETURN_ERROR_IOCTL; */
        }
        flash_settings.effect_ts = (int64_t)flash_time.tv_sec * 1000 * 1000 +
                                   (int64_t)flash_time.tv_usec;
        LOGD_CAMHW_SUBM(FL_SUBM, "frameid %u, get RK_VIDIOC_FLASH_TIMEINFO flash ts %lld",
                        frame_id, flash_settings.effect_ts);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
FlashLightHw::v4l_set_params(int fl_mode, float fl_intensity[], int fl_timeout, int fl_on)
{
    struct v4l2_control control;
    int fl_v4l_mode;
    int i = 0;
#define set_fl_contol_to_dev(fl_dev,control_id,val) \
        {\
            xcam_mem_clear (control); \
            control.id = control_id; \
            control.value = val; \
            if (fl_dev->io_control (VIDIOC_S_CTRL, &control) < 0) { \
                LOGE_CAMHW_SUBM(FL_SUBM, " set fl %s to %d failed", #control_id, val); \
                return XCAM_RETURN_ERROR_IOCTL; \
            } \
            LOGD_CAMHW_SUBM(FL_SUBM, "set fl %p, cid %s to %d, success",\
                            fl_dev.ptr(), #control_id, val); \
        }\

    if (fl_mode == RK_AIQ_FLASH_MODE_OFF)
        fl_v4l_mode = V4L2_FLASH_LED_MODE_NONE;
    else if (fl_mode == RK_AIQ_FLASH_MODE_FLASH || fl_mode == RK_AIQ_FLASH_MODE_FLASH_MAIN)
        fl_v4l_mode = V4L2_FLASH_LED_MODE_FLASH;
    else if (fl_mode == RK_AIQ_FLASH_MODE_FLASH_PRE || fl_mode ==  RK_AIQ_FLASH_MODE_TORCH)
        fl_v4l_mode = V4L2_FLASH_LED_MODE_TORCH;
    else {
        LOGE_CAMHW_SUBM(FL_SUBM, " set fl to mode  %d failed", fl_mode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SmartPtr<V4l2SubDevice> fl_device;

    if (fl_v4l_mode == V4L2_FLASH_LED_MODE_NONE) {
        for (i = 0; i < _active_fl_num; i++) {
            fl_device = _fl_device[i];
            set_fl_contol_to_dev(fl_device, V4L2_CID_FLASH_LED_MODE, V4L2_FLASH_LED_MODE_NONE);
        }
    } else if (fl_v4l_mode == V4L2_FLASH_LED_MODE_FLASH) {
        for (i = 0; i < _active_fl_num; i++) {
            fl_device = _fl_device[i];
            set_fl_contol_to_dev(fl_device, V4L2_CID_FLASH_LED_MODE, V4L2_FLASH_LED_MODE_FLASH);
            set_fl_contol_to_dev(fl_device, V4L2_CID_FLASH_TIMEOUT, fl_timeout * 1000);
            if (_v4l_flash_info[i].fl_strth_adj_enable) {
                int flash_power =
                    fl_intensity[i] * (_v4l_flash_info[i].flash_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MAX]);
                set_fl_contol_to_dev(fl_device, V4L2_CID_FLASH_INTENSITY, flash_power);
                LOGD_CAMHW_SUBM(FL_SUBM, "set flash: flash:%f max:%d set:%d\n",
                                fl_intensity[i],
                                _v4l_flash_info[i].flash_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MAX],
                                flash_power);
            }
        }

        // shoude flash on all finally
        for (i = 0; i < _active_fl_num; i++) {
            set_fl_contol_to_dev(fl_device,
                                 fl_on ? V4L2_CID_FLASH_STROBE : V4L2_CID_FLASH_STROBE_STOP, 0);
        }
    } else if (fl_v4l_mode == V4L2_FLASH_LED_MODE_TORCH) {
        for (i = 0; i < _active_fl_num; i++) {
            fl_device = _fl_device[i];
            if (_v4l_flash_info[i].tc_strth_adj_enable) {
                int torch_power =
                    fl_intensity[i] * (_v4l_flash_info[i].torch_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MAX]);
                set_fl_contol_to_dev(fl_device, V4L2_CID_FLASH_TORCH_INTENSITY, torch_power);
                LOGD_CAMHW_SUBM(FL_SUBM, "set flash: torch:%f max:%d set:%d\n",
                                fl_intensity[i],
                                _v4l_flash_info[i].torch_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MAX],
                                torch_power);
            }
            set_fl_contol_to_dev(fl_device, V4L2_CID_FLASH_LED_MODE, V4L2_FLASH_LED_MODE_TORCH);
        }
    } else {
        LOGE_CAMHW_SUBM(FL_SUBM, "|||set_3a_fl error fl mode %d", fl_mode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

int
FlashLightHw::get_flash_info ()
{
    struct v4l2_queryctrl ctrl;
    SmartPtr<V4l2SubDevice> fl_device;

    for (int i = 0; i < _active_fl_num; i++) {
        fl_device = _fl_device[i];

        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_FLASH_INTENSITY;
        if (fl_device->io_control(VIDIOC_QUERYCTRL, &ctrl) < 0) {
            LOGE_CAMHW_SUBM(FL_SUBM, "query V4L2_CID_FLASH_INTENSITY failed. cmd = 0x%x",
                            V4L2_CID_FLASH_INTENSITY);
            return -errno;
        }

        _v4l_flash_info[i].flash_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MIN] =
            ctrl.minimum;
        _v4l_flash_info[i].flash_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MAX] =
            ctrl.maximum;
        _v4l_flash_info[i].flash_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_DEFAULT] =
            ctrl.default_value;
        _v4l_flash_info[i].flash_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_STEP] =
            ctrl.step;
        _v4l_flash_info[i].fl_strth_adj_enable = !(ctrl.flags & V4L2_CTRL_FLAG_READ_ONLY);

        LOGD_CAMHW_SUBM(FL_SUBM, "fl_dev[%d], flash power range:[%d,%d], adjust enable %d",
                        i, ctrl.minimum, ctrl.maximum, _v4l_flash_info[i].fl_strth_adj_enable);

        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id = V4L2_CID_FLASH_TORCH_INTENSITY;
        if (fl_device->io_control(VIDIOC_QUERYCTRL, &ctrl) < 0) {
            LOGE_CAMHW_SUBM(FL_SUBM, "query V4L2_CID_FLASH_TORCH_INTENSITY failed. cmd = 0x%x",
                            V4L2_CID_FLASH_TORCH_INTENSITY);
            return -errno;
        }

        _v4l_flash_info[i].torch_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MIN] =
            ctrl.minimum;
        _v4l_flash_info[i].torch_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MAX] =
            ctrl.maximum;
        _v4l_flash_info[i].torch_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_DEFAULT] =
            ctrl.default_value;
        _v4l_flash_info[i].torch_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_STEP] =
            ctrl.step;

        _v4l_flash_info[i].tc_strth_adj_enable = !(ctrl.flags & V4L2_CTRL_FLAG_READ_ONLY);


        LOGD_CAMHW_SUBM(FL_SUBM, "fl_dev[%d], torch power range:[%d,%d], adjust enable %d",
                        i, ctrl.minimum, ctrl.maximum, _v4l_flash_info[i].tc_strth_adj_enable);
    }

    return XCAM_RETURN_NO_ERROR;
}

}
