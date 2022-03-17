/*
 * amerge_head.h
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

#ifndef __CALIBDBV2_AMERGE_HEADER_H__
#define __CALIBDBV2_AMERGE_HEADER_H__

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef struct MergeOECurveV20_s {
    // M4_ARRAY_DESC("EnvLv", "f32", M4_SIZE(1,100), M4_RANGE(0,1), "0",M4_DIGIT(4), M4_DYNAMIC(1))
    float* EnvLv;
    int EnvLv_len;
    // M4_ARRAY_DESC("Smooth", "f32", M4_SIZE(1,100), M4_RANGE(0,1), "0.4",M4_DIGIT(4), M4_DYNAMIC(1))
    float* Smooth;
    int Smooth_len;
    // M4_ARRAY_DESC("Offset", "f32", M4_SIZE(1,100), M4_RANGE(108,300), "210",M4_DIGIT(4), M4_DYNAMIC(1))
    float* Offset;
    int Offset_len;
} MergeOECurveV20_t;

typedef struct MergeMDCurveV20_s {
    // M4_ARRAY_DESC("MoveCoef", "f32", M4_SIZE(1,100), M4_RANGE(0,1), "0",M4_DIGIT(4), M4_DYNAMIC(1))
    float* MoveCoef;
    int MoveCoef_len;
    // M4_ARRAY_DESC("LM_smooth", "f32", M4_SIZE(1,100), M4_RANGE(0,1), "0.4",M4_DIGIT(4), M4_DYNAMIC(1))
    float* LM_smooth;
    int LM_smooth_len;
    // M4_ARRAY_DESC("LM_offset", "f32", M4_SIZE(1,100), M4_RANGE(0,1), "0.38",M4_DIGIT(4), M4_DYNAMIC(1))
    float* LM_offset;
    int LM_offset_len;
    // M4_ARRAY_DESC("MS_smooth", "f32", M4_SIZE(1,100), M4_RANGE(0,1), "0.4",M4_DIGIT(4), M4_DYNAMIC(1))
    float* MS_smooth;
    int MS_smooth_len;
    // M4_ARRAY_DESC("MS_offset", "f32", M4_SIZE(1,100), M4_RANGE(0,1), "0.38",M4_DIGIT(4), M4_DYNAMIC(1))
    float* MS_offset;
    int MS_offset_len;
} MergeMDCurveV20_t;

typedef struct MergeV20_s {
    // M4_ARRAY_TABLE_DESC("OECurve", "array_table_ui", "none")
    MergeOECurveV20_t OECurve;
    // M4_ARRAY_TABLE_DESC("MDCurve", "array_table_ui", "none")
    MergeMDCurveV20_t MDCurve;
    // M4_NUMBER_DESC("ByPassThr", "f32", M4_RANGE(0,1), "0", M4_DIGIT(4))
    float ByPassThr;
    // M4_NUMBER_DESC("OECurve_damp", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(4))
    float OECurve_damp;
    // M4_NUMBER_DESC("MDCurveLM_damp", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(4))
    float MDCurveLM_damp;
    // M4_NUMBER_DESC("MDCurveMS_damp", "f32", M4_RANGE(0,1), "0.9", M4_DIGIT(4))
    float MDCurveMS_damp;
} MergeV20_t;

typedef struct CalibDbV2_merge_s {
    // M4_STRUCT_DESC("MergeTuningPara", "normal_ui_style")
    MergeV20_t MergeTuningPara;
} CalibDbV2_merge_t;


RKAIQ_END_DECLARE

#endif
