/*
 * agamma_head.h
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

#ifndef __CALIBDBV2_AGAMMA_HEAD_H__
#define __CALIBDBV2_AGAMMA_HEAD_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

#define CALIBDB_AGAMMA_KNOTS_NUM               45

typedef enum GammaType_e {
    GAMMATYPE_LOG        = 0,
    GAMMATYPE_EQU         = 1,
} GammaType_t;

typedef struct CalibDbGammaV2_s {
    // M4_BOOL_DESC("Gamma_en", "1")
    bool Gamma_en;
    // M4_ENUM_DESC("Gamma_out_segnum", "GammaType_t", "GAMMATYPE_LOG")
    GammaType_t Gamma_out_segnum;
    // M4_NUMBER_DESC("Gamma_out_offset", "u16", M4_RANGE(0,4095), "0", M4_DIGIT(0))
    uint16_t Gamma_out_offset;
    // M4_ARRAY_MARK_DESC("Gamma_curve", "u16", M4_SIZE(1,45),  M4_RANGE(0, 4095), "[0.0000, 6.0000, 11.0000, 17.0000, 22.0000, 28.0000, 33.0000, 39.0000, 44.0000, 55.0000, 66.0000, 77.0000, 88.0000, 109.0000, 130.0000, 150.0000, 170.0000, 210.0000, 248.0000, 286.0000, 323.0000, 393.0000, 460.0000, 525.0000, 586.0000, 702.0000, 809.0000, 909.0000, 1002.0000, 1172.0000, 1325.0000, 1462.0000, 1588.0000, 1811.0000, 2004.0000, 2174.0000, 2327.0000, 2590.0000, 2813.0000, 3006.0000, 3177.0000, 3467.0000, 3708.0000, 3915.0000, 4095.0000]", M4_DIGIT(4), M4_DYNAMIC(0), "curve_table")
    uint16_t         Gamma_curve[CALIBDB_AGAMMA_KNOTS_NUM];
} CalibDbGammaV2_t;

typedef struct CalibDbV2_gamma_s {
    // M4_STRUCT_DESC("GammaTuningPara", "curve_ui_type_A")
    CalibDbGammaV2_t GammaTuningPara;
} CalibDbV2_gamma_t;

RKAIQ_END_DECLARE

#endif
