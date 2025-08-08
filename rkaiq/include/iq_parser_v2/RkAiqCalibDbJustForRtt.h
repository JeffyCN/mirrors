/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef ___RK_AIQ_CALIB_DB_JUST_FOR_RTT_H__
#define ___RK_AIQ_CALIB_DB_JUST_FOR_RTT_H__

#include "iq_parser_v2/sensorinfo_head.h"

RKAIQ_BEGIN_DECLARE

#if USE_NEWSTRUCT
typedef struct simplified_subscene_s
{
    char name[64];
    ae_param_t ae;
    float awb_gain[4];
} simplified_subscene_t;

typedef struct simplified_calibproj_s {
    uint32_t bin_type;
    CalibDb_Sensor_ParaV2_t sensor_calib;
    simplified_subscene_t sub_scene[3];
} simplified_calibproj_t;
#endif

RKAIQ_END_DECLARE

#endif