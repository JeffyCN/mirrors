/*
 * ablc_uapi_head.h
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

#ifndef __CALIBDBV2_UAPI_ABLC_HEADER_H__
#define __CALIBDBV2_UAPI_ABLC_HEADER_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE


typedef struct AblcSelect_s {
    // M4_BOOL_DESC("enable", "1")
    bool enable;

    // M4_NUMBER_DESC("blc_r", "s16", M4_RANGE(0, 4095), "0", M4_DIGIT(0))
    int16_t blc_r;
    // M4_NUMBER_DESC("blc_gr", "s16", M4_RANGE(0, 4095), "0", M4_DIGIT(0))
    int16_t blc_gr;
    // M4_NUMBER_DESC("blc_gb", "s16", M4_RANGE(0, 4095), "0", M4_DIGIT(0))
    int16_t blc_gb;
    // M4_NUMBER_DESC("blc_b", "s16", M4_RANGE(0, 4095), "0", M4_DIGIT(0))
    int16_t blc_b;
} AblcSelect_t;

typedef AblcSelect_t AblcManualAttr_t;


RKAIQ_END_DECLARE

#endif
