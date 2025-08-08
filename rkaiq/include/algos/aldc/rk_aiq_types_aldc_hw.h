/*
 * rk_aiq_types_aldc_hw.h
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd
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

#ifndef __RK_AIQ_TYPES_ALDC_HW_H__
#define __RK_AIQ_TYPES_ALDC_HW_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LDC_MESH_XY_NUM_ALGO 524288  // ((4096/16) * (4096/8))*4

#ifndef ISP39_LDC_BIC_NUM
#define ISP39_LDC_BIC_NUM 36
#endif

typedef struct rkaiq_ldch_v22_hw_param_s {
    uint8_t frm_end_dis;
    uint8_t sample_avr_en;
    uint8_t bic_mode_en;
    uint8_t force_map_en;
    uint8_t map13p3_en;

    uint8_t bicubic[ISP39_LDC_BIC_NUM];

    uint16_t out_hsize;

    uint32_t hsize;
    uint32_t vsize;
    int32_t buf_fd;
} __attribute__((packed)) rkaiq_ldch_v22_hw_param_t;

typedef struct rkaiq_ldcv_v22_hw_param_s {
    /* CTRL */
    uint8_t thumb_mode;
    uint8_t dth_bypass;
    uint8_t force_map_en;
    uint8_t map13p3_en;

    /* BIC_TABLE */
    uint8_t bicubic[ISP39_LDC_BIC_NUM];

    /* LAST_OFFSET */
    uint16_t last_offset;

    uint32_t out_vsize;

    uint32_t hsize;
    uint32_t vsize;
    int32_t buf_fd;
} __attribute__((packed)) rkaiq_ldcv_v22_hw_param_t;

typedef struct rkaiq_ldc_v22_hw_param_s {
    rkaiq_ldch_v22_hw_param_t ldch;
    rkaiq_ldcv_v22_hw_param_t ldcv;
} __attribute__((packed)) rkaiq_ldc_hw_param_t;

#ifdef __cplusplus
}
#endif

#endif
