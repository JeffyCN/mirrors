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

#ifndef _AIQ_PDAF_STREAM_PROC_UNIT_H_
#define _AIQ_PDAF_STREAM_PROC_UNIT_H_

#include "c_base/aiq_list.h"
#include "c_base/aiq_thread.h"
#include "hwi_c/aiq_pdafStreamProcUnit.h"
#include "hwi_c/aiq_stream.h"
#include "include/common/rk_aiq_types.h"
#include "xcore_c/aiq_v4l2_buffer.h"

typedef struct AiqQueue_s AiqQueue_t;
typedef struct AiqPdafStreamProcUnit_s AiqPdafStreamProcUnit_t;

typedef struct AiqPdafBuffer_s {
    AiqV4l2Buffer_t _v4l_buf_base;
    rk_aiq_isp_pdaf_meas_t pdaf_meas;
} AiqPdafBuffer_t;

typedef struct _PdafStreamParam {
    bool valid;
    bool stream_flag;
} AiqPdafStreamParam;

typedef struct PdafStreamHelperThd_s {
    AiqThread_t* _base;
    AiqPdafStreamProcUnit_t* mPdafStreamProc;
    AiqList_t* mMsgsQueue;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;
} PdafStreamHelperThd_t;

typedef struct AiqPdafStreamProcUnit_s {
    AiqStream_t* _pdafStream;
    AiqCamHwBase_t* _camHw;
    AiqV4l2Device_t* mPdafDev;
    bool mStartFlag;
    bool mStartStreamFlag;
    rk_aiq_isp_pdaf_meas_t mPdafMeas;
    int mBufType;
    PdafStreamHelperThd_t mHelperThd;
    AiqMutex_t mStreamMutex;
    rk_sensor_pdaf_info_t mPdafInf;
    AiqPollCallback_t* _pcb;
} AiqPdafStreamProcUnit_t;

XCamReturn AiqPdafStreamProcUnit_init(AiqPdafStreamProcUnit_t* pProcUnit, int type);
XCamReturn AiqPdafStreamProcUnit_deinit(AiqPdafStreamProcUnit_t* pProcUnit);
void AiqPdafStreamProcUnit_start(AiqPdafStreamProcUnit_t* pProcUnit);
void AiqPdafStreamProcUnit_stop(AiqPdafStreamProcUnit_t* pProcUnit);
XCamReturn AiqPdafStreamProcUnit_preapre(AiqPdafStreamProcUnit_t* pProcUnit,
                                         rk_sensor_pdaf_info_t* pdaf_inf);
void AiqPdafStreamProcUnit_set_devices(AiqPdafStreamProcUnit_t* pProcUnit, AiqCamHwBase_t* camHw);

#endif  // _AIQ_PDAF_STREAM_PROC_UNIT_H_
