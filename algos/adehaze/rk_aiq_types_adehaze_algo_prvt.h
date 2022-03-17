/*
 *rk_aiq_types_adehaze_algo_prvt.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_TYPES_ADEHAZE_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_ADEHAZE_ALGO_PRVT_H_

#include "adehaze/rk_aiq_types_adehaze_algo_int.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"

#include "rk_aiq_types_adehaze_stat.h"

#include "rk_aiq_algo_types.h"
#include "xcam_log.h"




#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)
#define DEHAZEBIGMODE     (2560)
#define RK_DEHAZE_ISO_NUM 9
#define FUNCTION_ENABLE 1
#define FUNCTION_DISABLE 0

#define ENVLVMAX     (1.0)
#define ENVLVMIN     (0.0)


typedef struct AdehazeAePreResV20_s {
    float ISO;
    dehaze_api_mode_t ApiMode;
} AdehazeAePreResV20_t;

typedef struct AdehazeAePreResV21_s {
    float EnvLv;
    dehaze_api_mode_t ApiMode;
} AdehazeAePreResV21_t;

typedef struct AdehazeAePreRes_s {
    union {
        AdehazeAePreResV20_t V20;
        AdehazeAePreResV21_t V21;
    };
} AdehazeAePreRes_t;

typedef struct AdehazeHandle_s {
    adehaze_sw_V2_t AdehazeAtrr;
    CalibDbV2_dehaze_V21_t calib_dehazV21;
    CalibDbV2_dehaze_V20_t calib_dehazV20;
    RkAiqAdehazeProcResult_t ProcRes;
    rkisp_adehaze_stats_t stats;
    AdehazeVersion_t HWversion;
    AdehazeAePreRes_t CurrData;
    AdehazeAePreRes_t PreData;
    bool byPassProc;
    int width;
    int height;
    int strength;
    int working_mode;
    int FrameNumber;
    int FrameID;
} AdehazeHandle_t;


#endif

