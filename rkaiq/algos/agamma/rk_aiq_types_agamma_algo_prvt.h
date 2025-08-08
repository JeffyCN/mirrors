/*
 *rk_aiq_types_agamma_algo_prvt.h
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

#ifndef _RK_AIQ_TYPES_AGAMMA_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_AGAMMA_ALGO_PRVT_H_

#include "RkAiqCalibDbTypes.h"
#include "agamma/rk_aiq_types_agamma_algo_int.h"
#include "amerge/rk_aiq_types_amerge_algo_prvt.h"
#include "rk_aiq_types_agamma_hw.h"
#include "xcam_log.h"

#define ISP3X_SEGNUM_LOG_49     (2)
#define ISP3X_SEGNUM_LOG_45     (0)
#define ISP3X_SEGNUM_EQU_45     (1)
#define GAMMA_CRUVE_MAX (4095)
#define GAMMA_CRUVE_MIN (0)

typedef struct rk_aiq_gamma_cfg_s {
    bool gamma_en;
    int gamma_out_segnum;//0:log 45 segment ; 1:equal segment ;2:log 49 segment ;
    int gamma_out_offset;
    int gamma_table[CALIBDB_AGAMMA_KNOTS_NUM_V11];
}  rk_aiq_gamma_cfg_t;

typedef struct AgammaHandle_s {
    bool ifReCalcStAuto;
    bool ifReCalcStManual;
    uint32_t FrameID;
    rk_aiq_gamma_cfg_t  agamma_config;
#if RKAIQ_HAVE_GAMMA_V10
    rk_aiq_gamma_v10_attr_t agammaAttrV10;
#endif
#if RKAIQ_HAVE_GAMMA_V11
    rk_aiq_gamma_v11_attr_t agammaAttrV11;
#endif
    rk_aiq_gamma_op_mode_t CurrApiMode;
    int working_mode;
} AgammaHandle_t;

#endif

