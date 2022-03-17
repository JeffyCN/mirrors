/*
 * csm_head.h
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

#ifndef __CALIBDBV2_CSM_HEAD_H__
#define __CALIBDBV2_CSM_HEAD_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

// #define M4_BOOL_DESC(ALIAS, DEFAULT, HIDE)

// #define M4_ARRAY_DESC(ALIAS, TYPE, SIZE, RANGE, DEFAULT,DIGIT,DYNAMIC, HIDE)

// #define M4_NUMBER_DESC(ALIAS, TYPE, RANGE, DEFAULT,DIGIT, HIDE)

#define RK_AIQ_CSM_COEFF_NUM 9

typedef struct __csm_param {
    // M4_BOOL_DESC("op_mode", "RKAiqOPMode_t", "RK_AIQ_OP_MODE_AUTO")
    RKAiqOPMode_t op_mode;
    // M4_NUMBER_DESC("full_range", "u8", M4_RANGE(0,1), "1", M4_DIGIT(0), 0)
    bool full_range;
    // M4_NUMBER_DESC("y_offset", "u16", M4_RANGE(0,63), "0", M4_DIGIT(0), 0)
    unsigned short y_offset;
    // M4_NUMBER_DESC("y_offset", "u16", M4_RANGE(0,255), "0", M4_DIGIT(0), 0)
    unsigned short c_offset;
    // M4_ARRAY_DESC("coeff", "u32", M4_SIZE(3,3), M4_RANGE(0,511), "0", M4_DIGIT(4), M4_DYNAMIC(0))
    unsigned int coeff[RK_AIQ_CSM_COEFF_NUM];
} Csm_Param_t;

typedef struct __csm {
    // M4_STRUCT_DESC("CsmTuningPara", "normal_ui_style")
    Csm_Param_t param;
} CalibDbV2_csm_t;

RKAIQ_END_DECLARE

#endif
