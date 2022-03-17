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

#ifndef _LENS_HW_BASE_H_
#define _LENS_HW_BASE_H_

#include <map>
#include <list>
#include "v4l2_device.h"
#include "rk_aiq_pool.h"
#include "rk-camera-module.h"

struct rk_cam_vcm_tim {
    struct timeval vcm_start_t;
    struct timeval vcm_end_t;
};

struct rk_cam_vcm_cfg {
    int start_ma;
    int rated_ma;
    int step_mode;
};

struct rk_cam_motor_tim {
    struct timeval motor_start_t;
    struct timeval motor_end_t;
};

#define RK_VIDIOC_VCM_TIMEINFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 0, struct rk_cam_vcm_tim)
#define RK_VIDIOC_IRIS_TIMEINFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 1, struct rk_cam_vcm_tim)
#define RK_VIDIOC_ZOOM_TIMEINFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 2, struct rk_cam_vcm_tim)

#define RK_VIDIOC_GET_VCM_CFG \
    _IOR('V', BASE_VIDIOC_PRIVATE + 3, struct rk_cam_vcm_cfg)
#define RK_VIDIOC_SET_VCM_CFG \
    _IOW('V', BASE_VIDIOC_PRIVATE + 4, struct rk_cam_vcm_cfg)

#define RK_VIDIOC_FOCUS_CORRECTION \
    _IOR('V', BASE_VIDIOC_PRIVATE + 5, unsigned int)
#define RK_VIDIOC_IRIS_CORRECTION \
    _IOR('V', BASE_VIDIOC_PRIVATE + 6, unsigned int)
#define RK_VIDIOC_ZOOM_CORRECTION \
    _IOR('V', BASE_VIDIOC_PRIVATE + 7, unsigned int)

#define LENSHW_RECORD_SOF_NUM   256

using namespace XCam;

namespace RkCam {

#define LENS_SUBM (0x10)

class LensHw : public V4l2SubDevice {
public:
    explicit LensHw(const char* name);
    virtual ~LensHw();

    XCamReturn start();
    XCamReturn stop();
    XCamReturn start_internal();
    XCamReturn getLensModeData(rk_aiq_lens_descriptor& lens_des);
    XCamReturn getLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg);
    XCamReturn setLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg);
    XCamReturn setPIrisParams(int step);
    XCamReturn setDCIrisParams(int pwmDuty);
    XCamReturn setFocusParams(int position);
    XCamReturn setZoomParams(int position);
    XCamReturn getPIrisParams(int* step);
    XCamReturn getFocusParams(int* position);
    XCamReturn getZoomParams(int* position);
    XCamReturn FocusCorrection();
    XCamReturn ZoomCorrection();
    XCamReturn handle_sof(int64_t time, int frameid);
    XCamReturn getIrisInfoParams(SmartPtr<RkAiqIrisParamsProxy>& irisParams, int frame_id);
    XCamReturn getAfInfoParams(SmartPtr<RkAiqAfInfoProxy>& afInfo, int frame_id);

private:
    XCamReturn queryLensSupport();

    XCAM_DEAD_COPY (LensHw);
    Mutex _mutex;
    SmartPtr<RkAiqAfInfoPool> _afInfoPool;
    SmartPtr<RkAiqIrisParamsPool> _irisInfoPool;
    static uint16_t DEFAULT_POOL_SIZE;
    struct v4l2_queryctrl _iris_query;
    struct v4l2_queryctrl _focus_query;
    struct v4l2_queryctrl _zoom_query;
    struct rk_cam_motor_tim _dciris_tim;
    struct rk_cam_motor_tim _piris_tim;
    struct rk_cam_vcm_tim _focus_tim;
    struct rk_cam_vcm_tim _zoom_tim;
    bool _iris_enable;
    bool _focus_enable;
    bool _zoom_enable;
    int _piris_step;
    int _last_piris_step;
    int _dciris_pwmduty;
    int _last_dciris_pwmduty;
    int _focus_pos;
    int _zoom_pos;
    int64_t _frame_time[LENSHW_RECORD_SOF_NUM];
    int _frame_sequence[LENSHW_RECORD_SOF_NUM];
    int _rec_sof_idx;
};

}; //namespace RkCam

#endif
