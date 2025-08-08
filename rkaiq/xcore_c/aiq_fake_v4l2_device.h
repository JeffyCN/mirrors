/*
 * aiq_fake_v4l2_device.h
 *
 *  Copyright (c) 2023 Rockchip Corporation
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

#ifndef C_AIQ_FAKE_V4L2_DEVICE_H
#define C_AIQ_FAKE_V4L2_DEVICE_H

#include "aiq_v4l2_device.h"
#include "c_base/aiq_list.h"
#include "common/rk_aiq_types_priv_c.h"
#include "hwi_c/aiq_camHw.h"

XCAM_BEGIN_DECLARE

typedef struct AiqFakeV4l2Device_s AiqFakeV4l2Device_t;
struct AiqFakeV4l2Device_s {
    AiqV4l2SubDevice_t _base;
    AiqMutex_t _mutex;
    int _pipe_fd[2];
    AiqList_t* _buf_list;
    void (*on_timer_proc)(AiqFakeV4l2Device_t* v4l2_dev);
    void (*enqueue_rawbuffer)(AiqFakeV4l2Device_t* v4l2_dev, struct rk_aiq_vbuf_info* vbinfo);
    uint32_t (*get_available_buffer_index)(AiqFakeV4l2Device_t* v4l2_dev);
};

int AiqFakeV4l2Device_init(AiqFakeV4l2Device_t* v4l2_dev, const char* name);
void AiqFakeV4l2Device_deinit(AiqFakeV4l2Device_t* v4l2_dev);

XCAM_END_DECLARE

#endif
