/*
 * rk_aiq_algo_argbir_hw.h
 *
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

#include "argbir_head.h"

#ifndef _RK_AIQ_ALGO_ARGBIR_HW_H_
#define _RK_AIQ_ALGO_ARGBIR_HW_H_

#define ARGBIR_SCALE_NUM 4

typedef struct RgbirProcResV10_s {
    /* THETA */
    u16 coe_theta;
    /* DELTA */
    u16 coe_delta;
    /* SCALE */
    u16 scale[ARGBIR_SCALE_NUM];
    /* LUMA_POINT */
    u16 luma_point[ARGBIR_LUM2SCALE_TABLE_LEN];
    /* SCALE_MAP */
    u16 scale_map[ARGBIR_LUM2SCALE_TABLE_LEN];
} RgbirProcResV10_t;

typedef struct RgbirProcRes_s {
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
    RgbirProcResV10_t rgbir_v10;
#endif
} RgbirProcRes_t;

typedef struct RkAiqArgbirProcResult_s {
    bool bRgbirEn;
    RgbirProcRes_t RgbirProcRes;
} RkAiqArgbirProcResult_t;

#endif  //_RK_AIQ_ALGO_ARGBIR_HW_H_
