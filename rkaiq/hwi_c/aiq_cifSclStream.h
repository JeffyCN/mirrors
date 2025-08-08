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
#ifndef _AIQ_CIF_SCALE_STREAM_H_
#define _AIQ_CIF_SCALE_STREAM_H_

#include <linux/v4l2-subdev.h>
#include <stdint.h>

#include "include/xcore/base/xcam_common.h"

typedef struct AiqRawStream_s AiqRawStream_t;
typedef struct AiqStream_s AiqStream_t;
typedef struct AiqV4l2Device_s AiqV4l2Device_t;
typedef struct rk_sensor_full_info_s rk_sensor_full_info_t;
typedef struct AiqPollCallback_s AiqPollCallback_t;

typedef struct AiqCifSclStream_s {
    AiqV4l2Device_t* _dev[3];
    AiqRawStream_t* _stream[3];
    int _working_mode;
    int _bpp;
    int _index;
    uint32_t _width;
    uint32_t _height;
    uint32_t _sns_v4l_pix_fmt;
    int _ratio;
    bool _first_start;
    bool _active;
    bool _init;
} AiqCifSclStream_t;

XCamReturn AiqCifSclStream_init(AiqCifSclStream_t* pCifSclStrm, const rk_sensor_full_info_t* s_info,
                                AiqPollCallback_t* cb);
void AiqCifSclStream_deinit(AiqCifSclStream_t* pCifSclStrm);

XCamReturn AiqCifSclStream_start(AiqCifSclStream_t* pCifSclStrm);
XCamReturn AiqCifSclStream_stop(AiqCifSclStream_t* pCifSclStrm);
void AiqCifSclStream_set_working_mode(AiqCifSclStream_t* pCifSclStrm, int mode);
XCamReturn AiqCifSclStream_set_format(AiqCifSclStream_t* pCifSclStrm,
                                      const struct v4l2_subdev_format* sns_sd_fmt,
                                      uint32_t sns_v4l_pix_fmt, int bpp);
XCamReturn AiqCifSclStream_set_format2(AiqCifSclStream_t* pCifSclStrm,
                                       const struct v4l2_subdev_selection* sns_sd_sel,
                                       uint32_t sns_v4l_pix_fmt, int bpp);
XCamReturn AiqCifSclStream_prepare(AiqCifSclStream_t* pCifSclStrm);
XCamReturn AiqCifSclStream_restart(AiqCifSclStream_t* pCifSclStrm,
                                   const rk_sensor_full_info_t* s_info, int ratio,
                                   AiqPollCallback_t* callback, int mode);
bool AiqCifSclStream_getIsActive(AiqCifSclStream_t* pCifSclStrm);
bool AiqCifSclStream_setPollCallback(AiqCifSclStream_t* pCifSclStrm, AiqPollCallback_t* callback);

#endif
