/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _AIQ_FAKE_SENSOR_HW_BASE_C_H_
#define _AIQ_FAKE_SENSOR_HW_BASE_C_H_

#include "aiq_sensorHw.h"
#include "c_base/aiq_cond.h"
#include "xcore_c/aiq_fake_v4l2_device.h"

RKAIQ_BEGIN_DECLARE

typedef struct CTimer_s CTimer_t;
typedef struct AiqFakeSensorHw_s AiqFakeSensorHw_t;
struct AiqFakeSensorHw_s {
    AiqSensorHw_t _base;
    int _width;
    int _height;
    uint32_t _fmt_code;
    rk_aiq_rawbuf_type_t _rawbuf_type;
    AiqList_t* _vbuf_list;
    AiqV4l2Device_t* _mipi_tx_dev[3];
    CTimer_t* _timer;
    void (*pFunc)(void* addr);
    AiqMutex_t _sync_mutex;
    AiqCond_t _sync_cond;
    bool _need_sync;
    bool use_rkrawstream;
    XCamReturn (*set_mipi_tx_devs)(AiqFakeSensorHw_t* pFakeSns, AiqV4l2Device_t* mipi_tx_devs[3]);
    XCamReturn (*enqueue_rawbuffer)(AiqFakeSensorHw_t* pFakeSns, struct rk_aiq_vbuf* vbuf,
                                    bool sync);
    XCamReturn (*on_dqueue)(AiqFakeSensorHw_t* pFakeSns, int dev_idx, AiqV4l2Buffer_t* v4l2_buf);
    XCamReturn (*prepare)(AiqSensorHw_t* pBaseSns, rk_aiq_raw_prop_t* prop);
    XCamReturn (*register_rawdata_callback)(AiqFakeSensorHw_t* pFakeSns, void (*callback)(void*));
    XCamReturn (*set_fake_sensor_format)(AiqFakeSensorHw_t* pFakeSns, int width, int height, uint32_t fmt_core);
};

void AiqFakeSensorHw_init(AiqFakeSensorHw_t* pFakeSnsHw, const char* name, int cid);
void AiqFakeSensorHw_deinit(AiqFakeSensorHw_t* pFakeSnsHw);
void AiqFakeSensorHw_deinit(AiqFakeSensorHw_t* pFakeSnsHw);

RKAIQ_END_DECLARE

#endif
