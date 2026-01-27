/*
 *  Copyright (c) 2021 Rockchip Corporation
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
#ifndef _AIQ_RAW_STREAM_CAP_UNIT_H_
#define _AIQ_RAW_STREAM_CAP_UNIT_H_

#if RKAIQ_HAVE_DUMPSYS
#include "dumpcam_server/info/include/st_string.h"
#endif
#include "hwi_c/aiq_stream.h"
#include "hwi_c/aiq_AiRmsStreamProcUnit.h"
#include "xcore_c/aiq_v4l2_device.h"

typedef struct rk_sensor_full_info_s rk_sensor_full_info_t;
typedef struct AiqList_s AiqList_t;
typedef struct AiqRawStreamProcUnit_s AiqRawStreamProcUnit_t;

enum RawCapState {
    RAW_CAP_STATE_INVALID,
    RAW_CAP_STATE_INITED,
    RAW_CAP_STATE_PREPARED,
    RAW_CAP_STATE_STARTED,
    RAW_CAP_STATE_STOPPED,
};

typedef struct AiqRawStreamCapUnit_s {
    int mCamPhyId;
    bool _is_1608_stream;
    AiqV4l2Device_t* _dev[3];
    AiqV4l2Device_t* _dev_bakup[3];
    bool _isExtDev;
    int _dev_index[3];
    AiqRawStream_t* _stream[3];
    AiqMutex_t _buf_mutex;
    int _working_mode;
    int _mipi_dev_max;
    int _skip_num;
    int64_t _skip_to_seq;
    AiqMutex_t _mipi_mutex;
    enum RawCapState _state;
    AiqPollCallback_t* _pcb[3];
    bool _is_split;

    // AiqV4l2Buffer_t*
    AiqList_t* buf_list[3];
    AiqCamHwBase_t* _camHw;
    AiqV4l2SubDevice_t* _isp_core_dev;
    AiqRawStreamProcUnit_t* _proc_stream;
    AiqAiRmsStreamProcUnit_t* _pAirmsStream;
    void* _sw_stream_ctx;
    rawStream_send_sync_buf_func _send_sync_buf_func;
    bool _single_buffer_async_mode;

#if RKAIQ_HAVE_DUMPSYS
    FrameDumpInfo_t fe;
    int data_mode;
#endif
} AiqRawStreamCapUnit_t;

XCamReturn AiqRawStreamCapUnit_init(AiqRawStreamCapUnit_t* pRawStrCapUnitconst,
                                    const rk_sensor_full_info_t* s_info, bool linked_to_isp,
                                    int tx_buf_cnt);
void AiqRawStreamCapUnit_deinit(AiqRawStreamCapUnit_t* pRawStrCapUnit);
XCamReturn AiqRawStreamCapUnit_start(AiqRawStreamCapUnit_t* pRawStrCapUnit, int mode);
XCamReturn AiqRawStreamCapUnit_stop(AiqRawStreamCapUnit_t* pRawStrCapUnit);
void AiqRawStreamCapUnit_set_working_mode(AiqRawStreamCapUnit_t* pRawStrCapUnit, int mode);
void AiqRawStreamCapUnit_set_devices(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                     AiqV4l2SubDevice_t* ispdev, AiqCamHwBase_t* handle,
                                     AiqRawStreamProcUnit_t* proc, AiqAiRmsStreamProcUnit_t* pAirmsStream);
void AiqRawStreamCapUnit_set_tx_devices(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                        AiqV4l2Device_t* mipi_tx_devs[3]);
AiqV4l2Device_t* AiqRawStreamCapUnit_get_tx_device(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                                   int index);
XCamReturn AiqRawStreamCapUnit_set_tx_format(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                             const struct v4l2_subdev_format* sns_sd_fmt,
                                             uint32_t sns_v4l_pix_fmt);
XCamReturn AiqRawStreamCapUnit_set_tx_format2(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                              const struct v4l2_subdev_selection* sns_sd_sel,
                                              uint32_t sns_v4l_pix_fmt);
XCamReturn AiqRawStreamCapUnit_prepare(AiqRawStreamCapUnit_t* pRawStrCapUnit, int idx);
void AiqRawStreamCapUnit_prepare_cif_mipi(AiqRawStreamCapUnit_t* pRawStrCapUnit);
void AiqRawStreamCapUnit_skip_frames(AiqRawStreamCapUnit_t* pRawStrCapUnit, int skip_num,
                                     int32_t skip_seq);
void AiqRawStreamCapUnit_setCamPhyId(AiqRawStreamCapUnit_t* pRawStrCapUnit, int phyId);
void AiqRawStreamCapUnit_setSensorCategory(AiqRawStreamCapUnit_t* pRawStrCapUnit, bool sensorState);
XCamReturn AiqRawStreamCapUnit_reset_hardware(AiqRawStreamCapUnit_t* pRawStrCapUnit);
XCamReturn AiqRawStreamCapUnit_set_csi_mem_word_align_mode(AiqRawStreamCapUnit_t* pRawStrCapUnit, int mode);
XCamReturn AiqRawStreamCapUnit_setVicapStreamMode(AiqRawStreamCapUnit_t* pRawStrCapUnit, int mode,
                                                  uint32_t* frameId, bool isSingleMode);
void AiqRawStreamCapUnit_setPollCallback(AiqRawStreamCapUnit_t* pRawStrCapUnit,
                                         AiqPollCallback_t* cb);

XCamReturn sync_raw_buf(AiqRawStreamCapUnit_t* pRawStrCapUnit, AiqV4l2Buffer_t** buf_s,
                        AiqV4l2Buffer_t** buf_m, AiqV4l2Buffer_t** buf_l);
bool check_skip_frame(AiqRawStreamCapUnit_t* pRawStrCapUnit, int32_t buf_seq);

void AiqRawStreamCapUnit_stop_vicap_stream_only(AiqRawStreamCapUnit_t* pRawStrCapUnit);
void AiqRawStreamCapUnit_skip_frame_and_restart_vicap_stream(AiqRawStreamCapUnit_t* pRawStrCapUnit, int skip_frm_cnt);
void AiqRawStreamCapUnit_setTxBufferCnt(AiqRawStreamCapUnit_t* pRawStrCapUnit, uint16_t buf_num);
XCamReturn AiqRawStreamCapUnit_setSwStreamInfo(AiqRawStreamCapUnit_t* pRawStrCapUnit, void* sw_stream_ctx, rawStream_send_sync_buf_func send_sync_buf_func);
XCamReturn AiqRawStreamCapUnit_setSingleBufAsyncMode(AiqRawStreamCapUnit_t* pRawStrCapUnit, bool async);
bool AiqRawStreamCapUnit_getStreamOnStatus(AiqRawStreamCapUnit_t* pRawStrCapUnit);

#if RKAIQ_HAVE_DUMPSYS
int AiqRawStreamCapUnit_dump(void* dumper, st_string* result, int argc, void* argv[]);
#endif

#endif
