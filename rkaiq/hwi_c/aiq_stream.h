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
#ifndef _AIQ_STREAM_H_
#define _AIQ_STREAM_H_

#include <sys/mman.h>

#include "common/rk_aiq_types_priv_c.h"

#include "hwi_c/aiq_camHw.h"
#include "xcore_c/aiq_v4l2_device.h"

XCAM_BEGIN_DECLARE

enum {
    ISP_MIPI_HDR_S = 0,
    ISP_MIPI_HDR_M,
    ISP_MIPI_HDR_L,
    ISP_MIPI_HDR_MAX,
};

typedef struct AiqLensHw_s AiqLensHw_t;
typedef struct AiqThread_s AiqThread_t;
typedef struct AiqSensorHw_s AiqSensorHw_t;
typedef struct AiqStatsStream_s AiqStatsStream_t;
typedef struct AiqAiIspStream_s AiqAiIspStream_t;
typedef struct AiqAibnrIspStream_s AiqAibnrIspStream_t;
typedef struct AiqAibnrAiispStream_s AiqAibnrAiispStream_t;
typedef struct AiqAiynrIspStream_s AiqAiynrIspStream_t;
typedef struct AiqAiynrAiispStream_s AiqAiynrAiispStream_t;
typedef struct AiqMemcIirStream_s AiqMemcIirStream_t;

typedef struct AiqStream_s AiqStream_t;
typedef struct AiqPollThread_s AiqPollThread_t;

typedef struct AiqPollCallback_s {
    void* _pCtx;
    XCamReturn (*poll_buffer_ready)(void* ctx, AiqHwEvt_t* evt, int dev_index);
    XCamReturn (*poll_event_ready)(void* ctx, uint32_t sequence, int type);
} AiqPollCallback_t;

struct AiqPollThread_s {
    AiqThread_t* _thread;
    int mCamPhyId;
    AiqV4l2Device_t* _dev;
    AiqPollCallback_t* _poll_callback;
    AiqStream_t* _stream;
    // frame syncronization
    uint32_t frameid;
    int _dev_type;
    int _poll_stop_fd[2];
    XCamReturn (*start)(AiqPollThread_t* pTh);
    XCamReturn (*stop)(AiqPollThread_t* pTh);
    XCamReturn (*poll_loop)(AiqPollThread_t* pTh);
};

XCamReturn AiqPollThread_init(AiqPollThread_t* pTh, const char* thName, int type,
                              AiqV4l2Device_t* pDev, AiqStream_t* pStream);
void AiqPollThread_deinit(AiqPollThread_t* pTh);
void AiqPollThread_setPollCallback(AiqPollThread_t* pTh, AiqPollCallback_t* callback);
bool AiqPollThread_setCamPhyId(AiqPollThread_t* pTh, int phyId);

typedef struct AiqEventPollThread_s {
    AiqPollThread_t _base;
    struct v4l2_event _event;
} AiqEventPollThread_t;

XCamReturn AiqEventPollThread_init(AiqEventPollThread_t* pTh, const char* thName, int type,
                                   AiqV4l2Device_t* pDev, AiqStream_t* pStream);
void AiqEventPollThread_deinit(AiqEventPollThread_t* pTh);

typedef struct AiqStreamEventPollThread_s {
    AiqEventPollThread_t _base;
    AiqCamHwBase_t* _pIsp;
    AiqV4l2SubDevice_t* _subdev;
} AiqStreamEventPollThread_t;

XCamReturn AiqStreamEventPollThread_init(AiqStreamEventPollThread_t* pTh, char* thName,
                                         const char* pDev, AiqCamHwBase_t* isp);
void AiqStreamEventPollThread_deinit(AiqStreamEventPollThread_t* pTh);

static const char* poll_type_to_str[ISP_POLL_POST_MAX];
struct AiqStream_s {
    AiqV4l2Device_t* _dev;
    int _dev_type;
    AiqPollThread_t* _poll_thread;
    bool _dev_prepared;
    int mCamPhyId;
    void (*start)(AiqStream_t* pStream);
    void (*startThreadOnly)(AiqStream_t* pStream);
    void (*startDeviceOnly)(AiqStream_t* pStream);
    void (*stop)(AiqStream_t* pStream);
    void (*stopThreadOnly)(AiqStream_t* pStream);
    void (*stopDeviceOnly)(AiqStream_t* pStream);
    void (*pause)(AiqStream_t* pStream);
    void (*resume)(AiqStream_t* pStream);
    bool (*setPollCallback)(AiqStream_t* pStream, AiqPollCallback_t* callback);
    aiq_VideoBuffer_t* (*new_video_buffer_v4l2Evt)(AiqStream_t* pStream, struct v4l2_event* event,
                                                   AiqV4l2Device_t* dev);
    void (*set_device_prepared)(AiqStream_t* pStream, bool prepare);
    XCamReturn (*getFormat)(AiqStream_t* pStream, struct v4l2_format* format);
    XCamReturn (*getSubDevFormat)(AiqStream_t* pStream, struct v4l2_subdev_format* format);
    void (*setCamPhyId)(AiqStream_t* pStream, int phyId);
};

XCamReturn AiqStream_init(AiqStream_t* pStream, AiqV4l2Device_t* pDev, int type);
void AiqStream_deinit(AiqStream_t* pStream);

typedef struct AiqStatsStream_s {
    AiqStream_t _base;
    AiqSensorHw_t* _event_handle_dev;
    AiqLensHw_t* _iris_handle_dev;
    AiqLensHw_t* _focus_handle_dev;
    AiqCamHwBase_t* _rx_handle_dev;
    bool (*set_event_handle_dev)(AiqStatsStream_t* pStream, AiqSensorHw_t* dev);
    bool (*set_iris_handle_dev)(AiqStatsStream_t* pStream, AiqLensHw_t* dev);
    bool (*set_focus_handle_dev)(AiqStatsStream_t* pStream, AiqLensHw_t* dev);
    bool (*set_rx_handle_dev)(AiqStatsStream_t* pStream, AiqCamHwBase_t* dev);
} AiqStatsStream_t;

XCamReturn AiqStatsStream_init(AiqStatsStream_t* pStream, AiqV4l2Device_t* pDev, int type);
void AiqStatsStream_deinit(AiqStatsStream_t* pStream);

#if RKAIQ_HAVE_AIBNR
typedef struct AiqAibnrIspStream_s {
    AiqStream_t _base;

    XCamReturn (*set_aiisp_linecnt)(AiqAibnrIspStream_t* pIspPartStrm, struct rkisp_aiisp_cfg aiisp_cfg);
    XCamReturn (*start_ispbe_hdl)(AiqAibnrIspStream_t* pIspPartStrm, struct rkisp_aiisp_st *aiisp_st);
    XCamReturn (*close_aiisp)(AiqAibnrIspStream_t* pIspPartStrm);
    void (*start)(AiqAibnrIspStream_t* pIspPartStrm);
    void (*stop)(AiqAibnrIspStream_t* pIspPartStrm);
} AiqAibnrIspStream_t;

typedef struct AiqAibnrAiispStream_s {
    AiqStream_t _base;

    void (*start)(AiqAibnrAiispStream_t* pStream);
    void (*stop)(AiqAibnrAiispStream_t* pStream);
} AiqAibnrAiispStream_t;

XCamReturn AiqAibnr_IspStream_init(AiqAibnrIspStream_t* pStream, AiqV4l2Device_t* pDev, int32_t type);
XCamReturn AiqAibnr_AiispStream_init(AiqAibnrAiispStream_t* pStream, AiqV4l2Device_t* pDev, int32_t type);
void AiqAibnr_IspStream_deinit(AiqAibnrIspStream_t* pStream);
void AiqAibnr_AiispStream_deinit(AiqAibnrAiispStream_t* pStream);
#endif

#if RKAIQ_HAVE_AIYNR
typedef struct AiqAiynrIspStream_s {
    AiqStream_t _base;

    XCamReturn (*set_aiisp_linecnt)(AiqAiynrIspStream_t* pIspPartStrm, struct rkisp_aiisp_cfg aiisp_cfg);
    XCamReturn (*start_ispbe_hdl)(AiqAiynrIspStream_t* pIspPartStrm, struct rkisp_aiisp_st *aiisp_st);
    XCamReturn (*close_aiisp)(AiqAiynrIspStream_t* pIspPartStrm);
    void (*start)(AiqAiynrIspStream_t* pIspPartStrm);
    void (*stop)(AiqAiynrIspStream_t* pIspPartStrm);
} AiqAiynrIspStream_t;

typedef struct AiqAiynrAiispStream_s {
    AiqStream_t _base;

    void (*start)(AiqAiynrAiispStream_t* pStream);
    void (*stop)(AiqAiynrAiispStream_t* pStream);
} AiqAiynrAiispStream_t;

XCamReturn AiqAiynr_IspStream_init(AiqAiynrIspStream_t* pStream, AiqV4l2Device_t* pDev, int32_t type);
XCamReturn AiqAiynr_AiispStream_init(AiqAiynrAiispStream_t* pStream, AiqV4l2Device_t* pDev, int32_t type);
void AiqAiynr_IspStream_deinit(AiqAiynrIspStream_t* pStream);
void AiqAiynr_AiispStream_deinit(AiqAiynrAiispStream_t* pStream);
#endif

#if RKAIQ_HAVE_MEMC
typedef struct AiqMemcIirStream_s {
    AiqStream_t _base;
    //rkisp_bay3dbuf_info_t bay3dbuf;
    // struct rkisp_bnr_buf_info bay3dbuf;
    void* iir_address[RKISP_BUFFER_MAX];

    XCamReturn (*set_Memc_linecnt)(AiqMemcIirStream_t* pStream, struct rkisp_aiisp_cfg aiisp_cfg);
    XCamReturn(*close_Memc)(AiqMemcIirStream_t* pStream);
    XCamReturn(*start_Memc_hdl)(AiqMemcIirStream_t* pStream, struct rkisp_aiisp_st* aiisp_st);
    void (*start)(AiqMemcIirStream_t* pStream);
    void (*stop)(AiqMemcIirStream_t* pStream);
} AiqMemcIirStream_t;

XCamReturn AiqMemcIirStream_init(AiqMemcIirStream_t* pStream, AiqV4l2Device_t* pDev, int32_t type);
void AiqMemcIirStream_deinit(AiqMemcIirStream_t* pStream);
#endif

typedef struct AiqAiIspStream_s {
    AiqStream_t _base;
    //rkisp_bay3dbuf_info_t bay3dbuf;
    struct rkisp_bnr_buf_info bay3dbuf;
    void* iir_address[RKISP_BUFFER_MAX];
    void* gain_address[RKISP_BUFFER_MAX];
    void* aiisp_address[RKISP_BUFFER_MAX];

    XCamReturn (*set_aiisp_linecnt)(AiqAiIspStream_t* pStream, struct rkisp_aiisp_cfg* aiisp_cfg);
    XCamReturn (*call_aiisp_rd_start)(AiqAiIspStream_t* pStream, struct rkisp_aiisp_st* pAiispDrvParams);
    XCamReturn (*close_aiisp)(AiqAiIspStream_t* pStream);
    void (*start)(AiqAiIspStream_t* pStream);
    void (*stop)(AiqAiIspStream_t* pStream);
} AiqAiIspStream_t;

XCamReturn AiqAiIspStream_init(AiqAiIspStream_t* pStream, AiqV4l2Device_t* pDev, int32_t type);
void AiqAiIspStream_deinit(AiqAiIspStream_t* pStream);

typedef struct AiqSofEvtStream_s {
    AiqStream_t _base;
    bool _linked_to_1608;
    /* the sensors which are connected to Rk1608 only subscribe a reset event */
    bool _is_subscribed;
} AiqSofEvtStream_t;

XCamReturn AiqSofEvtStream_init(AiqSofEvtStream_t* pStream, AiqV4l2Device_t* pDev, int type,
                                bool linkedToRk1608);
void AiqSofEvtStream_deinit(AiqSofEvtStream_t* pStream);

typedef struct AiqRawStream_s AiqRawStream_t;
struct AiqRawStream_s {
    AiqStream_t _base;
    int _dev_index;
    int _bpp;
    int _reserved[2];
    void (*set_reserved_data)(AiqRawStream_t* pStream, int32_t bpp);
};

XCamReturn AiqRawStream_init(AiqRawStream_t* pStream, AiqV4l2Device_t* pDev, int type, int index);
void AiqRawStream_deinit(AiqRawStream_t* pStream);

XCamReturn AiqPdafStream_init(AiqStream_t* pStream, AiqV4l2Device_t* pDev, int type);
void AiqPdafStream_deinit(AiqStream_t* pStream);

XCAM_END_DECLARE

#endif
