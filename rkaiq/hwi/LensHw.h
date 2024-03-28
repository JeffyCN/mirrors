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
#include <xcam_mutex.h>
#include "xcam_thread.h"
#include "smartptr.h"
#include "safe_list.h"
#include "v4l2_device.h"
#include "rk_aiq_pool.h"
#include "common/rk-camera-module.h"

#define VCMDRV_SETZOOM_MAXCNT        300U
#define LENSHW_RECORD_SOF_NUM        256
#define LENSHW_RECORD_LOWPASSFV_NUM  256

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

#define RK_VIDIOC_FOCUS_SET_BACKLASH \
    _IOR('V', BASE_VIDIOC_PRIVATE + 8, unsigned int)
#define RK_VIDIOC_IRIS_SET_BACKLASH \
    _IOR('V', BASE_VIDIOC_PRIVATE + 9, unsigned int)
#define RK_VIDIOC_ZOOM_SET_BACKLASH \
    _IOR('V', BASE_VIDIOC_PRIVATE + 10, unsigned int)

#define RK_VIDIOC_ZOOM1_TIMEINFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 11, struct rk_cam_vcm_tim)
#define RK_VIDIOC_ZOOM1_CORRECTION \
    _IOR('V', BASE_VIDIOC_PRIVATE + 12, unsigned int)
#define RK_VIDIOC_ZOOM1_SET_BACKLASH \
    _IOR('V', BASE_VIDIOC_PRIVATE + 13, unsigned int)

#define RK_VIDIOC_ZOOM_SET_POSITION \
    _IOW('V', BASE_VIDIOC_PRIVATE + 14, struct rk_cam_set_zoom)
#define RK_VIDIOC_FOCUS_SET_POSITION \
    _IOW('V', BASE_VIDIOC_PRIVATE + 15, struct rk_cam_set_focus)
#define RK_VIDIOC_MODIFY_POSITION \
    _IOW('V', BASE_VIDIOC_PRIVATE + 16, struct rk_cam_modify_pos)

#define RK_VIDIOC_GET_DCIRIS_HALL_ADC \
    _IOR('V', BASE_VIDIOC_PRIVATE + 20, unsigned int)

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

#define RK_VIDIOC_SET_VCM_MAX_LOGICALPOS \
    _IOW('V', BASE_VIDIOC_PRIVATE + 17, unsigned int)

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
    s32 focus_pos;
};

struct rk_cam_set_zoom {
    bool is_need_zoom_reback;
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


#define LENSHW_RECORD_SOF_NUM   256

using namespace XCam;

namespace RkCam {

#define LENS_SUBM (0x10)

class LensHwHelperThd;

class LensHw : public V4l2SubDevice {
public:
    explicit LensHw(const char* name);
    virtual ~LensHw();

    XCamReturn getOTPData();
    XCamReturn start(bool prepared = false) override;
    XCamReturn stop() override;
    XCamReturn start_internal();
    XCamReturn getLensModeData(rk_aiq_lens_descriptor& lens_des);
    XCamReturn getLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg);
    XCamReturn setLensVcmCfg(rk_aiq_lens_vcmcfg& lens_cfg);
    XCamReturn getLensVcmMaxlogpos(int& max_log_pos);
    XCamReturn setLensVcmMaxlogpos(int& max_log_pos);
    XCamReturn setPIrisParams(int step);
    XCamReturn setDCIrisParams(int pwmDuty);
    XCamReturn setHDCIrisParams(int target);
    XCamReturn getHDCIrisParams(int* adc);
    XCamReturn setFocusParams(SmartPtr<RkAiqFocusParamsProxy>& focus_params);
    XCamReturn setFocusParamsSync(int position, bool is_update_time, bool focus_noreback);
    XCamReturn setZoomParams(int position);
    XCamReturn setZoomFocusParams(SmartPtr<RkAiqFocusParamsProxy>& focus_params);
    XCamReturn setZoomFocusParamsSync(SmartPtr<rk_aiq_focus_params_t> attrPtr, bool is_update_time);
    XCamReturn setZoomFocusRebackSync(SmartPtr<rk_aiq_focus_params_t> attrPtr, bool is_update_time);
    XCamReturn endZoomChgSync(SmartPtr<rk_aiq_focus_params_t> attrPtr, bool is_update_time);
    XCamReturn getPIrisParams(int* step);
    XCamReturn getFocusParams(int* position);
    XCamReturn getZoomParams(int* position);
    XCamReturn FocusCorrectionSync();
    XCamReturn ZoomCorrectionSync();
    XCamReturn FocusCorrection();
    XCamReturn ZoomCorrection();
    XCamReturn ZoomFocusModifyPositionSync(SmartPtr<rk_aiq_focus_params_t> attrPtr);
    XCamReturn ZoomFocusModifyPosition(SmartPtr<RkAiqFocusParamsProxy>& focus_params);
    XCamReturn handle_sof(int64_t time, uint32_t frameid);
    XCamReturn setLowPassFv(uint32_t sub_shp4_4[RKAIQ_RAWAF_SUMDATA_NUM], uint32_t sub_shp8_8[RKAIQ_RAWAF_SUMDATA_NUM],
                            uint32_t high_light[RKAIQ_RAWAF_SUMDATA_NUM], uint32_t high_light2[RKAIQ_RAWAF_SUMDATA_NUM], uint32_t frameid);
    XCamReturn getIrisInfoParams(SmartPtr<RkAiqIrisParamsProxy>& irisParams, uint32_t frame_id);
    XCamReturn getAfInfoParams(SmartPtr<RkAiqAfInfoProxy>& afInfo, uint32_t frame_id);
    XCamReturn setAngleZ(float angleZ);

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
    int _last_zoomchg_focus;
    int _last_zoomchg_zoom;
    int64_t _frame_time[LENSHW_RECORD_SOF_NUM];
    uint32_t _frame_sequence[LENSHW_RECORD_SOF_NUM];
    int _rec_sof_idx;
    int32_t _lowfv_fv4_4[LENSHW_RECORD_LOWPASSFV_NUM][RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t _lowfv_fv8_8[LENSHW_RECORD_LOWPASSFV_NUM][RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t _lowfv_highlht[LENSHW_RECORD_LOWPASSFV_NUM][RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t _lowfv_highlht2[LENSHW_RECORD_LOWPASSFV_NUM][RKAIQ_RAWAF_SUMDATA_NUM];
    uint32_t _lowfv_seq[LENSHW_RECORD_LOWPASSFV_NUM];
    int _rec_lowfv_idx;
    SmartPtr<LensHwHelperThd> _lenshw_thd;
    SmartPtr<LensHwHelperThd> _lenshw_thd1;

    bool _otp_valid;
    float _posture;
    float _hysteresis;
    float _startCurrent;
    float _endCurrent;
    float _angleZ;
    int _max_logical_pos;
};

class LensHwHelperThd
    : public Thread {
public:
    LensHwHelperThd(LensHw *lenshw, int id)
        : Thread("LensHwHelperThread")
        , mLensHw(lenshw), mId(id) {
        (void)(mId);
    };
    ~LensHwHelperThd() {
        mAttrQueue.clear ();
    };

    void triger_stop() {
        mAttrQueue.pause_pop ();
    };

    void triger_start() {
        mAttrQueue.resume_pop ();
    };

    bool push_attr (const SmartPtr<rk_aiq_focus_params_t> buffer) {
        mAttrQueue.push (buffer);
        return true;
    };

    bool is_empty () {
        return mAttrQueue.is_empty();
    };

    void clear_attr () {
        mAttrQueue.clear ();
    };

protected:
    //virtual bool started ();
    virtual void stopped () {
        mAttrQueue.clear ();
    };
    virtual bool loop ();
private:
    LensHw *mLensHw;
    int mId;
    SafeList<rk_aiq_focus_params_t> mAttrQueue;
};

} //namespace RkCam

#endif
