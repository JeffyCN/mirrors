/*
 * gain_uapi_head_v2.h
 *
 *  Copyright (c) 2022 Rockchip Corporation
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

#ifndef __CALIBDBV2_UAPI_GAIN_HEADER_V2_H__
#define __CALIBDBV2_UAPI_GAIN_HEADER_V2_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef struct RK_GAIN_Select_V2_s {
    // M4_BOOL_DESC("hdrgain_ctrl_enable", "0")
    bool hdrgain_ctrl_enable;

    // M4_NUMBER_DESC("hdr_gain_scale_s", "f32", M4_RANGE(0, 128.0), "1.0", M4_DIGIT(2))
    float hdr_gain_scale_s;

    // M4_NUMBER_DESC("hdr_gain_scale_m", "f32", M4_RANGE(0, 128.0), "1.0", M4_DIGIT(2))
    float hdr_gain_scale_m;

} RK_GAIN_Select_V2_t;

RKAIQ_END_DECLARE

#endif
