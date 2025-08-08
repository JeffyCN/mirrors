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

#ifndef _AIQ_LENS_HW_BASE_H_
#define _AIQ_LENS_HW_BASE_H_

#include "c_base/aiq_pool.h"
#include "c_base/aiq_list.h"
#include "c_base/aiq_thread.h"
#include "common/rk-camera-module.h"
#include "common/rk_aiq_types_priv_c.h"
#include "include/algos/af/rk_aiq_types_af_algo.h"
#include "include/common/rk_aiq_types.h"
#include "xcore_c/aiq_v4l2_device.h"

#define VCMDRV_SETZOOM_MAXCNT       300U
#define LENSHW_RECORD_SOF_NUM       256
#define LENSHW_RECORD_LOWPASSFV_NUM 256

#define RK_VIDIOC_VCM_TIMEINFO  _IOR('V', BASE_VIDIOC_PRIVATE + 0, struct rk_cam_vcm_tim)
#define RK_VIDIOC_IRIS_TIMEINFO _IOR('V', BASE_VIDIOC_PRIVATE + 1, struct rk_cam_vcm_tim)
#define RK_VIDIOC_ZOOM_TIMEINFO _IOR('V', BASE_VIDIOC_PRIVATE + 2, struct rk_cam_vcm_tim)

#define RK_VIDIOC_GET_VCM_CFG _IOR('V', BASE_VIDIOC_PRIVATE + 3, struct rk_cam_vcm_cfg)
#define RK_VIDIOC_SET_VCM_CFG _IOW('V', BASE_VIDIOC_PRIVATE + 4, struct rk_cam_vcm_cfg)

#define RK_VIDIOC_FOCUS_CORRECTION _IOR('V', BASE_VIDIOC_PRIVATE + 5, unsigned int)
#define RK_VIDIOC_IRIS_CORRECTION  _IOR('V', BASE_VIDIOC_PRIVATE + 6, unsigned int)
#define RK_VIDIOC_ZOOM_CORRECTION  _IOR('V', BASE_VIDIOC_PRIVATE + 7, unsigned int)

#define RK_VIDIOC_FOCUS_SET_BACKLASH _IOR('V', BASE_VIDIOC_PRIVATE + 8, unsigned int)
#define RK_VIDIOC_IRIS_SET_BACKLASH  _IOR('V', BASE_VIDIOC_PRIVATE + 9, unsigned int)
#define RK_VIDIOC_ZOOM_SET_BACKLASH  _IOR('V', BASE_VIDIOC_PRIVATE + 10, unsigned int)

#define RK_VIDIOC_ZOOM1_TIMEINFO     _IOR('V', BASE_VIDIOC_PRIVATE + 11, struct rk_cam_vcm_tim)
#define RK_VIDIOC_ZOOM1_CORRECTION   _IOR('V', BASE_VIDIOC_PRIVATE + 12, unsigned int)
#define RK_VIDIOC_ZOOM1_SET_BACKLASH _IOR('V', BASE_VIDIOC_PRIVATE + 13, unsigned int)

#define RK_VIDIOC_ZOOM_SET_POSITION  _IOW('V', BASE_VIDIOC_PRIVATE + 14, struct rk_cam_set_zoom)
#define RK_VIDIOC_FOCUS_SET_POSITION _IOW('V', BASE_VIDIOC_PRIVATE + 15, struct rk_cam_set_focus)
#define RK_VIDIOC_MODIFY_POSITION    _IOW('V', BASE_VIDIOC_PRIVATE + 16, struct rk_cam_modify_pos)

#define RK_VIDIOC_GET_DCIRIS_HALL_ADC _IOR('V', BASE_VIDIOC_PRIVATE + 20, unsigned int)

#ifdef CONFIG_COMPAT
#define RK_VIDIOC_COMPAT_VCM_TIMEINFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 0, struct rk_cam_compat_vcm_tim)
#define RK_VIDIOC_COMPAT_IRIS_TIMEINFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 1, struct rk_cam_compat_vcm_tim)
#define RK_VIDIOC_COMPAT_ZOOM_TIMEINFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 2, struct rk_cam_compat_vcm_tim)
#define RK_VIDIOC_COMPAT_ZOOM1_TIMEINFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 11, struct rk_cam_compat_vcm_tim)
#endif

#define RK_VIDIOC_SET_VCM_MAX_LOGICALPOS _IOW('V', BASE_VIDIOC_PRIVATE + 17, unsigned int)

typedef int s32;
typedef unsigned int u32;

struct rk_cam_modify_pos {
    s32 focus_pos;
    s32 zoom_pos;
    s32 zoom1_pos;
};

struct rk_cam_set_focus {
    bool is_need_reback;
    s32 focus_pos;
};

struct rk_cam_zoom_pos {
    s32 zoom_pos;
    s32 zoom1_pos;
    s32 focus_pos;
};

struct rk_cam_set_zoom {
    bool is_need_zoom_reback;
    bool is_need_zoom1_reback;
    bool is_need_focus_reback;
    u32 setzoom_cnt;
    struct rk_cam_zoom_pos zoom_pos[VCMDRV_SETZOOM_MAXCNT];
};

struct rk_cam_vcm_tim {
    struct timeval vcm_start_t;
    struct timeval vcm_end_t;
};

#ifdef CONFIG_COMPAT
struct rk_cam_compat_vcm_tim {
    struct compat_timeval vcm_start_t;
    struct compat_timeval vcm_end_t;
};
#endif

struct rk_cam_motor_tim {
    struct timeval motor_start_t;
    struct timeval motor_end_t;
};

struct rk_cam_vcm_cfg {
    int start_ma;
    int rated_ma;
    int step_mode;
};

#define LENSHW_RECORD_SOF_NUM 256

#define LENS_SUBM (0x10)

typedef struct AiqLensHw_s AiqLensHw_t;

typedef struct LensHwHelperThd_s {
    AiqThread_t* _base;
    AiqLensHw_t* mLensHw;
    int mId;
    /* rk_aiq_focus_params_t */
    AiqList_t* mMsgsQueue;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;
} LensHwHelperThd_t;

struct AiqLensHw_s {
    AiqV4l2SubDevice_t mSd;
    AiqMutex_t _mutex;
    struct v4l2_queryctrl _iris_query;
    struct v4l2_queryctrl _focus_query;
    struct v4l2_queryctrl _zoom_query;
    struct v4l2_queryctrl _zoom1_query;
    struct rk_cam_motor_tim _dciris_tim;
    struct rk_cam_motor_tim _piris_tim;
    struct rk_cam_vcm_tim _focus_tim;
    struct rk_cam_vcm_tim _zoom_tim;
    struct rk_cam_vcm_tim _zoom1_tim;
    bool _iris_enable;
    bool _focus_enable;
    bool _zoom_enable;
    bool _zoom1_enable;
    bool _zoom_correction;
    bool _focus_correction;
    int _piris_step;
    int _last_piris_step;
    int _dciris_pwmduty;
    int _last_dciris_pwmduty;
    int _hdciris_target;
    int _last_hdciris_target;
    int _focus_pos;
    int _zoom_pos;
    int _zoom1_pos;
    int _last_zoomchg_focus;
    int _last_zoomchg_zoom;
    int _last_zoomchg_zoom1;
    int64_t _frame_time[LENSHW_RECORD_SOF_NUM];
    uint32_t _frame_sequence[LENSHW_RECORD_SOF_NUM];
    int _rec_sof_idx;
    int32_t _lowfv_fv4_4[LENSHW_RECORD_LOWPASSFV_NUM][RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t _lowfv_fv8_8[LENSHW_RECORD_LOWPASSFV_NUM][RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t _lowfv_highlht[LENSHW_RECORD_LOWPASSFV_NUM][RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t _lowfv_highlht2[LENSHW_RECORD_LOWPASSFV_NUM][RKAIQ_RAWAF_SUMDATA_NUM];
    uint32_t _lowfv_seq[LENSHW_RECORD_LOWPASSFV_NUM];
    int _rec_lowfv_idx;
    LensHwHelperThd_t _lenshw_thd;
    LensHwHelperThd_t _lenshw_thd1;

    bool _otp_valid;
    float _posture;
    float _hysteresis;
    float _startCurrent;
    float _endCurrent;
    float _angleZ;
    int _max_logical_pos;
    // override v4l2
    XCamReturn (*open)(AiqLensHw_t* pLensHw);
    XCamReturn (*start)(AiqLensHw_t* pLensHw, bool prepared);
    XCamReturn (*stop)(AiqLensHw_t* pLensHw);
    XCamReturn (*close)(AiqLensHw_t* pLensHw);
};

XCamReturn AiqLensHw_queryLensSupport(AiqLensHw_t* pLensHw);
XCamReturn AiqLensHw_getOTPData(AiqLensHw_t* pLensHw);
XCamReturn AiqLensHw_getLensModeData(AiqLensHw_t* pLensHw, rk_aiq_lens_descriptor* lens_des);
XCamReturn AiqLensHw_getLensVcmCfg(AiqLensHw_t* pLensHw, rk_aiq_lens_vcmcfg* lens_cfg);
XCamReturn AiqLensHw_setLensVcmCfg(AiqLensHw_t* pLensHw, rk_aiq_lens_vcmcfg* lens_cfg);
XCamReturn AiqLensHw_getLensVcmMaxlogpos(AiqLensHw_t* pLensHw, int* max_log_pos);
XCamReturn AiqLensHw_setLensVcmMaxlogpos(AiqLensHw_t* pLensHw, int* max_log_pos);
XCamReturn AiqLensHw_setPIrisParams(AiqLensHw_t* pLensHw, int step);
XCamReturn AiqLensHw_setDCIrisParams(AiqLensHw_t* pLensHw, int pwmDuty);
XCamReturn AiqLensHw_setHDCIrisParams(AiqLensHw_t* pLensHw, int target);
XCamReturn AiqLensHw_getHDCIrisParams(AiqLensHw_t* pLensHw, int* adc);
XCamReturn AiqLensHw_setFocusParams(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* focus_params);
XCamReturn AiqLensHw_setFocusParamsSync(AiqLensHw_t* pLensHw, int position, bool is_update_time,
                                        bool focus_noreback);
XCamReturn AiqLensHw_setZoomParams(AiqLensHw_t* pLensHw, int position);
XCamReturn AiqLensHw_setZoomFocusParams(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* focus_params);
XCamReturn AiqLensHw_setZoomFocusParamsSync(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* attrPtr,
                                            bool is_update_time);
XCamReturn AiqLensHw_setZoomFocusRebackSync(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* attrPtr,
                                            bool is_update_time);
XCamReturn AiqLensHw_endZoomChgSync(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* attrPtr,
                                    bool is_update_time);
XCamReturn AiqLensHw_startZoomChgSync(AiqLensHw_t* pLensHw, rk_aiq_focus_params_t* attrPtr,
                                    bool is_update_time);
XCamReturn AiqLensHw_getPIrisParams(AiqLensHw_t* pLensHw, int* step);
XCamReturn AiqLensHw_getFocusParams(AiqLensHw_t* pLensHw, int* position);
XCamReturn AiqLensHw_getZoomParams(AiqLensHw_t* pLensHw, int* position);
bool AiqLensHw_IsFocusInCorrectionSync(AiqLensHw_t* pLensHw);
bool AiqLensHw_IsZoomInCorrectionSync(AiqLensHw_t* pLensHw);
XCamReturn AiqLensHw_FocusCorrectionSync(AiqLensHw_t* pLensHw);
XCamReturn AiqLensHw_ZoomCorrectionSync(AiqLensHw_t* pLensHw);
XCamReturn AiqLensHw_FocusCorrection(AiqLensHw_t* pLensHw);
XCamReturn AiqLensHw_ZoomCorrection(AiqLensHw_t* pLensHw);
XCamReturn AiqLensHw_ZoomFocusModifyPositionSync(AiqLensHw_t* pLensHw,
                                                 rk_aiq_focus_params_t* attrPtr);
XCamReturn AiqLensHw_ZoomFocusModifyPosition(AiqLensHw_t* pLensHw,
                                             rk_aiq_focus_params_t* focus_params);
XCamReturn AiqLensHw_handle_sof(AiqLensHw_t* pLensHw, int64_t time, uint32_t frameid);
XCamReturn AiqLensHw_setLowPassFv(AiqLensHw_t* pLensHw,
                                  uint32_t sub_shp4_4[RKAIQ_RAWAF_SUMDATA_NUM],
                                  uint32_t sub_shp8_8[RKAIQ_RAWAF_SUMDATA_NUM],
                                  uint32_t high_light[RKAIQ_RAWAF_SUMDATA_NUM],
                                  uint32_t high_light2[RKAIQ_RAWAF_SUMDATA_NUM], uint32_t frameid);
XCamReturn AiqLensHw_setAngleZ(AiqLensHw_t* pLensHw, float angleZ);
XCamReturn AiqLensHw_getIrisInfoParams(AiqLensHw_t* pLensHw, uint32_t frame_id, AiqIrisInfoWrapper_t* pIrisInfo);
XCamReturn AiqLensHw_getAfInfoParams(AiqLensHw_t* pLensHw, uint32_t frame_id, AiqAfInfoWrapper_t* pAfInfo);
XCamReturn AiqLensHw_queryLensSupport(AiqLensHw_t* pLensHw);

XCamReturn AiqLensHw_init(AiqLensHw_t* pLensHw, const char* name);
XCamReturn AiqLensHw_deinit(AiqLensHw_t* pLensHw);

#endif
