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

#ifndef _FAKE_SENSOR_HW_BASE_H_
#define _FAKE_SENSOR_HW_BASE_H_

#include <pthread.h>
#include <sys/time.h>
#include "SensorHw.h"

using namespace XCam;

namespace RkCam {
#define FAKECAM_SUBM (0x40)

class CTimer;
class FakeSensorHw : public SensorHw {
    friend class CTimer;
public:
    explicit FakeSensorHw();
    virtual ~FakeSensorHw();

    virtual XCamReturn setExposureParams(SmartPtr<RkAiqExpParamsProxy>& expPar);
    virtual XCamReturn getSensorModeData(const char* sns_ent_name,
                                         rk_aiq_exposure_sensor_descriptor& sns_des);

    virtual XCamReturn handle_sof(int64_t time, uint32_t frameid);
    virtual int get_pixel(rk_aiq_exposure_sensor_descriptor* sns_des);
    virtual int get_blank(rk_aiq_exposure_sensor_descriptor* sns_des);
    virtual int get_exposure_range(rk_aiq_exposure_sensor_descriptor* sns_des);
    virtual int get_sensor_desc(rk_aiq_exposure_sensor_descriptor* sns_des);

    virtual XCamReturn get_sensor_descriptor (rk_aiq_exposure_sensor_descriptor* sns_des);
    virtual XCamReturn set_working_mode(int mode);
    virtual XCamReturn set_exp_delay_info(int time_delay, int gain_delay, int hcg_lcg_mode_delay);
    virtual XCamReturn set_mirror_flip(bool mirror, bool flip, int32_t& skip_frame_sequence);
    virtual XCamReturn get_mirror_flip(bool& mirror, bool& flip);
    virtual XCamReturn start(bool prepared);
    virtual XCamReturn stop();
    virtual XCamReturn get_selection (int pad, uint32_t target, struct v4l2_subdev_selection &select);
    virtual XCamReturn getFormat(struct v4l2_subdev_format &aFormat);
    XCamReturn prepare(rk_aiq_raw_prop_t prop);
    XCamReturn set_mipi_tx_devs(SmartPtr<V4l2Device> mipi_tx_devs[3]);
    XCamReturn enqueue_rawbuffer(struct rk_aiq_vbuf *vbuf, bool sync);
    virtual XCamReturn on_dqueue(int dev_idx, SmartPtr<V4l2BufferProxy> buf_proxy);
    XCamReturn register_rawdata_callback(void (*callback)(void *));
    virtual bool is_virtual_sensor() {
        return true;
    }

    bool use_rkrawstream;
private:
    XCAM_DEAD_COPY (FakeSensorHw);
    int get_sensor_fps(float& fps);
    int get_nr_switch(rk_aiq_sensor_nr_switch_t* nr_switch);
    int get_dcg_ratio(rk_aiq_sensor_dcg_ratio_t* dcg_ratio);
    int _width;
    int _height;
    uint32_t _fmt_code;
    rk_aiq_rawbuf_type_t _rawbuf_type;
    std::list<struct rk_aiq_vbuf> _vbuf_list;
    SmartPtr<V4l2Device>  _mipi_tx_dev[3];
    CTimer *_timer;
    void (*pFunc)(void *addr);
    Mutex _sync_mutex;
    Cond _sync_cond;
    bool _need_sync;
};

class CTimer
{
public:
    CTimer(FakeSensorHw *dev);
    virtual ~CTimer();
    void SetTimer(long second, long microsecond);
    void StartTimer();
    void StopTimer();
private:
    pthread_t thread_timer;
    long m_second, m_microsecond;
    static void *OnTimer_stub(void *p)
    {
        (static_cast<CTimer*>(p))->thread_proc();
        return NULL;
    }
    void thread_proc();
    void OnTimer();
    FakeSensorHw *_dev;
};
} //namespace RkCam

#endif
