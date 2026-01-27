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

#ifndef _AIQ_FAKE_CAM_HW_BASE_C_H_
#define _AIQ_FAKE_CAM_HW_BASE_C_H_

#include "aiq_sensorHw.h"
#include "hwi_c/aiq_CamHwBase.h"
#include "xcore_c/aiq_fake_v4l2_device.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqCamHwFake_s {
    AiqCamHwBase_t _base;
    struct _st_addrinfo _st_addr[3];
    struct _raw_format _rawfmt;
    rk_aiq_frame_info_t _finfo;
    enum v4l2_memory _rx_memory_type;
    enum v4l2_memory _tx_memory_type;
    rk_aiq_rawbuf_type_t _rawbuf_type;
    AiqV4l2Device_t* _mipi_tx_devs[3];
    AiqV4l2Device_t* _mipi_rx_devs[3];
    int isp_index;
    AiqPollCallback_t mPollCb;
} AiqCamHwFake_t;

XCamReturn AiqCamHwFake_init(AiqCamHwFake_t* pFakeCamHw, const char* sns_ent_name);
void AiqCamHwFake_deinit(AiqCamHwFake_t* pFakeCamHw);

XCamReturn AiqCamHwFake_enqueueRawBuffer(AiqCamHwFake_t* pFakeCamHw, void* rawdata, bool sync);
XCamReturn AiqCamHwFake_enqueueRawFile(AiqCamHwFake_t* pFakeCamHw, const char* path);
XCamReturn AiqCamHwFake_registRawdataCb(AiqCamHwFake_t* pFakeCamHw, void (*callback)(void*));
XCamReturn AiqCamHwFake_rawdataPrepare(AiqCamHwFake_t* pFakeCamHw, rk_aiq_raw_prop_t prop);

RKAIQ_END_DECLARE

#endif
