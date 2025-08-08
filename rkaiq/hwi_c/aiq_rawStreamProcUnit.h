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
#ifndef _AIQ_RAW_STREAM_PROC_UNIT_H_
#define _AIQ_RAW_STREAM_PROC_UNIT_H_

#include <linux/v4l2-subdev.h>
#include <stdint.h>

#include "c_base/aiq_cond.h"
#include "c_base/aiq_mutex.h"
#include "common/rk_aiq_types_priv_c.h"
#if RKAIQ_HAVE_DUMPSYS
#include "dumpcam_server/info/include/st_string.h"
#endif
#include "hwi_c/aiq_dumpRkRaw.h"
#include "include/common/rk_aiq_types.h"

typedef struct AiqCamHwBase_s AiqCamHwBase_t;
typedef struct AiqMap_s AiqMap_t;
typedef struct AiqList_s AiqList_t;
typedef struct AiqPollCallback_s AiqPollCallback_t;
typedef struct AiqRawStream_s AiqRawStream_t;
typedef struct AiqStream_s AiqStream_t;
typedef struct AiqThread_s AiqThread_t;
typedef struct AiqV4l2Device_s AiqV4l2Device_t;
typedef struct AiqV4l2SubDevice_s AiqV4l2SubDevice_t;
typedef struct AiqV4l2Buffer_s AiqV4l2Buffer_t;
typedef struct AiqV4l2Device_s AiqV4l2Device_t;
typedef struct rk_sensor_full_info_s rk_sensor_full_info_t;

typedef XCamReturn (*rawStream_send_sync_buf_func)(void* ctx, AiqV4l2Buffer_t* buf_s, AiqV4l2Buffer_t* buf_m, AiqV4l2Buffer_t* buf_l);

typedef struct AiqRawStreamProcUnit_s {
    int mCamPhyId;
    AiqV4l2Device_t* _dev[3];
    bool _isExtDev;
    int _dev_index[3];
    AiqRawStream_t* _stream[3];
    int32_t _working_mode;
    int _mipi_dev_max;
    bool _is_multi_cam_conc;
    AiqMutex_t _buf_mutex;
    AiqCond_t _buf_cond;
    // <uint32_t, bool>
    AiqMap_t* _isp_hdr_fid2ready_map;
    // aiq_VideoBuffer_t*
    AiqList_t* buf_list[3];  // after trigger
    // aiq_VideoBuffer_t*
    AiqList_t* cache_list[3];  // before trigger
    AiqPollCallback_t* _pcb[3];

    AiqThread_t* _raw_proc_thread;
    bool _isRawProcThQuit;
    AiqCamHwBase_t* _camHw;
    AiqV4l2SubDevice_t* _isp_core_dev;
    bool _first_trigger;
    AiqMutex_t _mipi_trigger_mutex;
    AiqPollCallback_t* _PollCallback;
    bool _is_1608_sensor;
    int8_t dumpRkRawType;
    aiq_DumpRkRaw_t _rawCap;

#if RKAIQ_HAVE_DUMPSYS
    FrameDumpInfo_t fe;
    FrameDumpInfo_t trig;
    int32_t trig_times;
    int data_mode;
#endif
} AiqRawStreamProcUnit_t;

XCamReturn AiqRawStreamProcUnit_init(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                     const rk_sensor_full_info_t* s_info, bool linked_to_isp,
                                     int tx_buf_cnt);
void AiqRawStreamProcUnit_deinit(AiqRawStreamProcUnit_t* pRawStrProcUnit);
XCamReturn AiqRawStreamProcUnit_start(AiqRawStreamProcUnit_t* pRawStrProcUnit, int mode);
XCamReturn AiqRawStreamProcUnit_stop(AiqRawStreamProcUnit_t* pRawStrProcUnit);
void AiqRawStreamProcUnit_set_working_mode(AiqRawStreamProcUnit_t* pRawStrProcUnit, int mode);
XCamReturn AiqRawStreamProcUnit_prepare(AiqRawStreamProcUnit_t* pRawStrProcUnit, int idx);
void AiqRawStreamProcUnit_set_rx_devices(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                         AiqV4l2Device_t* mipi_rx_devs[3]);
AiqV4l2Device_t* AiqRawStreamProcUnit_get_rx_device(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                                    int index);
XCamReturn AiqRawStreamProcUnit_set_rx_format(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                              const struct v4l2_subdev_format* sns_sd_fmt,
                                              uint32_t sns_v4l_pix_fmt);
XCamReturn AiqRawStreamProcUnit_set_rx_format2(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                               const struct v4l2_subdev_selection* sns_sd_sel,
                                               uint32_t sns_v4l_pix_fmt);
void AiqRawStreamProcUnit_set_devices(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                      AiqV4l2SubDevice_t* ispdev, AiqCamHwBase_t* handle);
void AiqRawStreamProcUnit_send_sync_buf(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                        AiqV4l2Buffer_t* buf_s, AiqV4l2Buffer_t* buf_m,
                                        AiqV4l2Buffer_t* buf_l);
bool AiqRawStreamProcUnit_raw_buffer_proc(AiqRawStreamProcUnit_t* pRawStrProcUnit);
void AiqRawStreamProcUnit_setMulCamConc(AiqRawStreamProcUnit_t* pRawStrProcUnit, bool cc);
// notify CamHwIsp one frame has been processed
void AiqRawStreamProcUnit_setPollCallback(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                          AiqPollCallback_t* cb);
void AiqRawStreamProcUnit_setCamPhyId(AiqRawStreamProcUnit_t* pRawStrProcUnit, int phyId);
void AiqRawStreamProcUnit_setSensorCategory(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                            bool sensorState);
XCamReturn AiqRawStreamProcUnit_capture_raw_ctl(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                                capture_raw_t type, int count,
                                                const char* capture_dir, char* output_dir);
XCamReturn AiqRawStreamProcUnit_notify_capture_raw(AiqRawStreamProcUnit_t* pRawStrProcUnit);
XCamReturn AiqRawStreamProcUnit_set_csi_mem_word_big_align(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                                           uint32_t width, uint32_t height,
                                                           uint32_t sns_v4l_pix_fmt,
                                                           int8_t sns_bpp);
void AiqRawStreamProcUnit_setPollCallback(AiqRawStreamProcUnit_t* pRawStrProcUnit,
                                          AiqPollCallback_t* cb);

XCamReturn setIspInfoToDump(AiqRawStreamProcUnit_t* pRawStrProcUnit);
void AiqRawStreamProcUnit_setRxBufferCnt(AiqRawStreamProcUnit_t* pRawStrProcUnit, uint16_t buf_num);

#if RKAIQ_HAVE_DUMPSYS
int AiqRawStreamProcUnit_dump(void* dumper, st_string* result, int argc, void* argv[]);
#endif
#endif
