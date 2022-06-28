/*
 * bayertnr_uapi_head_v2.h
 *
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

#ifndef __CALIBDBV2_UAPI_BAYER_TNR_HEADER_V2_H__
#define __CALIBDBV2_UAPI_BAYER_TNR_HEADER_V2_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE


typedef struct RK_Bayertnr_Params_V2_Select_s
{
    // M4_BOOL_DESC("enable", "1")
    int enable;
    // M4_BOOL_DESC("lo_enable", "1")
    int lo_enable;
    // M4_BOOL_DESC("hi_enable", "1")
    int hi_enable;
    // M4_BOOL_DESC("lo_med_en", "1")
    int lo_med_en;
    // M4_BOOL_DESC("lo_gsbay_en", "1")
    int lo_gsbay_en;
    // M4_BOOL_DESC("lo_gslum_en", "1")
    int lo_gslum_en;
    // M4_BOOL_DESC("hi_med_en", "1")
    int hi_med_en;
    // M4_BOOL_DESC("hi_gslum_en", "1")
    int hi_gslum_en;


    // M4_ARRAY_DESC("lumapoint", "s32", M4_SIZE(1,16), M4_RANGE(0,65535), "0.0", M4_DIGIT(0), M4_DYNAMIC(0))
    int lumapoint[16];
    // M4_ARRAY_DESC("sigma", "s32", M4_SIZE(1,16), M4_RANGE(0,65535), "0.0", M4_DIGIT(0), M4_DYNAMIC(0))
    int sigma[16];

    // M4_ARRAY_DESC("lumapoint2", "s32", M4_SIZE(1,16), M4_RANGE(0,65535), "0.0", M4_DIGIT(0), M4_DYNAMIC(0))
    int lumapoint2[16];
    // M4_ARRAY_DESC("lo_sigma", "s32", M4_SIZE(1,16), M4_RANGE(0,65535), "0.0", M4_DIGIT(0), M4_DYNAMIC(0))
    int lo_sigma[16];
    // M4_ARRAY_DESC("hi_sigma", "s32", M4_SIZE(1,16), M4_RANGE(0,65535), "0.0", M4_DIGIT(0), M4_DYNAMIC(0))
    int hi_sigma[16];

    // M4_NUMBER_DESC("thumbds", "f32", M4_RANGE(0, 8), "8", M4_DIGIT(2))
    int thumbds;

    // M4_BOOL_DESC("global_pk_en", "1")
    int global_pk_en;
    // M4_NUMBER_DESC("global_pksq", "s32", M4_RANGE(0, 268435455), "1024", M4_DIGIT(0))
    int global_pksq;

    // M4_NUMBER_DESC("lo_filter_strength", "f32", M4_RANGE(0, 16), "1.0", M4_DIGIT(2))
    float lo_filter_strength;
    // M4_NUMBER_DESC("hi_filter_strength", "f32", M4_RANGE(0, 16), "1.0", M4_DIGIT(2))
    float hi_filter_strength;
    // M4_NUMBER_DESC("soft_threshold_ratio", "f32", M4_RANGE(0, 1.0), "0.0", M4_DIGIT(2))
    float soft_threshold_ratio;

    // M4_NUMBER_DESC("clipwgt", "f32", M4_RANGE(0, 1), "0.03215", M4_DIGIT(5))
    float clipwgt;
    // M4_NUMBER_DESC("hi_wgt_comp", "f32", M4_RANGE(0, 1), "0.16", M4_DIGIT(2))
    float hi_wgt_comp;
    // M4_NUMBER_DESC("hidif_th", "f32", M4_RANGE(0, 65535), "32767", M4_DIGIT(0))
    float hidif_th;
} RK_Bayertnr_Params_V2_Select_t;

RKAIQ_END_DECLARE

#endif
