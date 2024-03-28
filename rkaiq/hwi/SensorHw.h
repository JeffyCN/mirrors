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

#ifndef _SENSOR_HW_BASE_H_
#define _SENSOR_HW_BASE_H_

#include <map>
#include <list>
#include <istream>
#include "v4l2_device.h"
#include "rk_aiq_pool.h"
#include "common/rk-camera-module.h"
#include "v4l2_buffer_proxy.h"
#include "rk_aiq_offline_raw.h"
#include "rk_aiq.h"

/************ BELOW FROM kernel/include/uapi/linux/rk-preisp.h ************/

/* test hdr function */
/*
* struct hdrae_exp_s - hdrae exposure
*
*/

enum cg_mode_e {
    GAIN_MODE_LCG,
    GAIN_MODE_HCG,
};

struct hdrae_exp_s {
    unsigned int long_exp_reg;
    unsigned int long_gain_reg;
    unsigned int middle_exp_reg;
    unsigned int middle_gain_reg;
    unsigned int short_exp_reg;
    unsigned int short_gain_reg;
    unsigned int long_exp_val;
    unsigned int long_gain_val;
    unsigned int middle_exp_val;
    unsigned int middle_gain_val;
    unsigned int short_exp_val;
    unsigned int short_gain_val;
    unsigned char long_cg_mode;
    unsigned char middle_cg_mode;
    unsigned char short_cg_mode;
};

#define SENSOR_CMD_SET_HDRAE_EXP        \
    _IOW('V', BASE_VIDIOC_PRIVATE + 0, struct hdrae_exp_s)

/************ UPPER FROM kernel/include/uapi/linux/rk-preisp.h ************/

using namespace XCam;

namespace RkCam {

#define SENSOR_SUBM (0x4)

class BaseSensorHw : public V4l2SubDevice {
public:
    explicit BaseSensorHw(const char* name): V4l2SubDevice (name) {}
    virtual ~BaseSensorHw() {}
    virtual void setCamPhyId(int phyId) {
        mCamPhyId = phyId;
    }
     virtual XCamReturn setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar) { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn getSensorModeData(const char* sns_ent_name,
                                 rk_aiq_exposure_sensor_descriptor& sns_des) { return XCAM_RETURN_NO_ERROR;}

    virtual XCamReturn handle_sof(int64_t time, uint32_t frameid) { return XCAM_RETURN_NO_ERROR;}
    virtual int get_pixel(rk_aiq_exposure_sensor_descriptor* sns_des)   { return 0;}
    virtual int get_blank(rk_aiq_exposure_sensor_descriptor* sns_des) { return 0;}
    virtual int get_exposure_range(rk_aiq_exposure_sensor_descriptor* sns_des) { return 0;}
    virtual int get_sensor_desc(rk_aiq_exposure_sensor_descriptor* sns_des) { return 0;}

    virtual XCamReturn get_sensor_descriptor (rk_aiq_exposure_sensor_descriptor* sns_des) { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn getEffectiveExpParams(SmartPtr<RkAiqSensorExpParamsProxy>& ExpParams, uint32_t frame_id) { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn set_working_mode(int mode) { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn set_exp_delay_info(int time_delay, int gain_delay, int hcg_lcg_mode_delay) { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn set_mirror_flip(bool mirror, bool flip, int32_t& skip_frame_sequence) { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn get_mirror_flip(bool& mirror, bool& flip) { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn start(bool prepared = false) { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn stop() { return XCAM_RETURN_NO_ERROR;}
    virtual XCamReturn on_dqueue(int dev_idx, SmartPtr<V4l2BufferProxy> buf_proxy) { return XCAM_RETURN_NO_ERROR; }
    virtual bool is_virtual_sensor() { return false; }
    virtual XCamReturn set_sync_mode(uint32_t mode) {return XCAM_RETURN_NO_ERROR;}
protected:
    XCAM_DEAD_COPY (BaseSensorHw);
    uint32_t get_v4l2_pixelformat(uint32_t pixelcode);
    int mCamPhyId{-1};
};

class SensorHw : public BaseSensorHw {
public:
    explicit SensorHw(const char* name);
    virtual ~SensorHw();

    virtual XCamReturn setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar);
    virtual XCamReturn getSensorModeData(const char* sns_ent_name,
                                 rk_aiq_exposure_sensor_descriptor& sns_des);

    virtual XCamReturn handle_sof(int64_t time, uint32_t frameid);
    virtual int get_pixel(rk_aiq_exposure_sensor_descriptor* sns_des);
    virtual int get_blank(rk_aiq_exposure_sensor_descriptor* sns_des);
    virtual int get_exposure_range(rk_aiq_exposure_sensor_descriptor* sns_des);
    virtual int get_sensor_desc(rk_aiq_exposure_sensor_descriptor* sns_des);

    virtual XCamReturn get_sensor_descriptor (rk_aiq_exposure_sensor_descriptor* sns_des);
    virtual XCamReturn getEffectiveExpParams(SmartPtr<RkAiqSensorExpParamsProxy>& ExpParams, uint32_t frame_id);
    virtual XCamReturn set_working_mode(int mode);
    virtual XCamReturn set_exp_delay_info(int time_delay, int gain_delay, int hcg_lcg_mode_delay);
    virtual XCamReturn set_mirror_flip(bool mirror, bool flip, int32_t& skip_frame_sequence);
    virtual XCamReturn get_mirror_flip(bool& mirror, bool& flip);
    virtual XCamReturn start(bool prepared = false);
    virtual XCamReturn stop();
    virtual XCamReturn set_sync_mode(uint32_t mode);

    virtual XCamReturn set_effecting_exp_map(uint32_t sequence, void *exp_ptr, int mode);
    virtual XCamReturn set_pause_flag(bool isPause, uint32_t frameId, bool isSingleMode);
    bool get_is_single_mode() {
        return mIsSingleMode;
    }
    XCAM_DEAD_COPY (SensorHw);
protected:
    Mutex _mutex;
    int _working_mode;
    std::list<std::pair<SmartPtr<RkAiqSensorExpParamsProxy>, bool>> _exp_list;
    std::map<uint32_t, SmartPtr<RkAiqSensorExpParamsProxy>> _effecting_exp_map;
    bool _first;
    uint32_t _frame_sequence;
    rk_aiq_exposure_sensor_descriptor _sensor_desc;
    std::list<SmartPtr<RkAiqSensorExpParamsProxy>> _delayed_gain_list;
    std::list<SmartPtr<RkAiqSensorExpParamsProxy>> _delayed_dcg_gain_mode_list;
    SmartPtr<RkAiqSensorExpParamsProxy> _last_exp_time;
    SmartPtr<RkAiqSensorExpParamsProxy> _last_exp_gain;
    SmartPtr<RkAiqSensorExpParamsProxy> _last_dcg_gain_mode;
    int _gain_delay;
    int _time_delay;
    bool _gain_delayed;
    int _dcg_gain_mode_delay;
    bool _dcg_gain_mode_delayed;
    bool _dcg_gain_mode_with_time;
    bool _is_i2c_exp;
    SmartPtr<RkAiqSensorExpParamsPool> _expParamsPool;

    enum {
        RK_EXP_UPDATE_TIME,
        RK_EXP_UPDATE_GAIN,
        RK_EXP_UPDATE_DCG,
    };
    typedef struct pending_split_exps_s {
        bool is_rk_exp_res;
        union {
            struct {
                uint16_t line_length_pixels;
                uint32_t frame_length_lines;
                float pixel_clock_freq_mhz;
                int   dcg_mode[3];
                RkAiqExpSensorParam_t sensor_params[3];
                uint32_t update_bits;
            } rk_exp_res;

            struct {
                unsigned int   nNumRegs;
                unsigned int   RegAddr[MAX_I2CDATA_LEN];
                unsigned int   RegValue[MAX_I2CDATA_LEN];
                unsigned int   AddrByteNum[MAX_I2CDATA_LEN];
                unsigned int   ValueByteNum[MAX_I2CDATA_LEN];
            } i2c_exp_res;
        };
    } pending_split_exps_t;

    std::map<uint32_t, pending_split_exps_t> _pending_spilt_map;

    static uint16_t DEFAULT_POOL_SIZE;
    std::string _sns_entity_name;
    bool _mirror;
    bool _flip;
    bool _update_mirror_flip;
    int get_sensor_fps(float& fps);
    XCamReturn setLinearSensorExposure(RKAiqAecExpInfo_t* expPar);
    XCamReturn setHdrSensorExposure(RKAiqAecExpInfo_t* expPar);
    XCamReturn setLinearSensorExposure(pending_split_exps_t* expPar);
    XCamReturn setHdrSensorExposure(pending_split_exps_t* expPar);
    XCamReturn setExposure(uint32_t frameid);
    XCamReturn setSensorDpcc(Sensor_dpcc_res_t* SensorDpccInfo);
    XCamReturn composeExpParam( RKAiqAecExpInfo_t* timeValid, RKAiqAecExpInfo_t* gainValid, RKAiqAecExpInfo_t* dcgGainModeValid, RKAiqAecExpInfo_t* newExp);
    XCamReturn split_locked(SmartPtr<RkAiqSensorExpParamsProxy>& exp_param, uint32_t sof_id);
    XCamReturn handle_sof_internal(int64_t time, uint32_t frameid);
    XCamReturn setI2cDAta(pending_split_exps_t* exps);
    int get_nr_switch(rk_aiq_sensor_nr_switch_t* nr_switch);
    int get_dcg_ratio(rk_aiq_sensor_dcg_ratio_t* dcg_ratio);
    XCamReturn _set_mirror_flip();

    bool mPauseFlag{false};
    uint32_t mPauseId{uint32_t(-1)};
    bool mIsSingleMode{false};
};

} //namespace RkCam

#endif
