/*
 *rk_aiq_types_alsc_algo_prvt.h
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

#ifndef _RK_AIQ_TYPES_AYNR_ALGO_PRVT_V1_H_
#define _RK_AIQ_TYPES_AYNR_ALGO_PRVT_V1_H_

#include "aynr/rk_aiq_types_aynr_algo_int_v1.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"


RKAIQ_BEGIN_DECLARE

/************ynr fix***************/
#define FIX_BIT_CI                     5//7
#define FIX_BIT_NOISE_SIGMA            5//7
#define FIX_BIT_DENOISE_WEIGHT         7
#define FIX_BIT_BF_SCALE               4//7
#define FIX_BIT_LUMA_CURVE             4//7
#define FIX_BIT_EDGE_SOFTNESS          7
#define FIX_BIT_GRAD_ADJUST_CURVE      4//7
#define FIX_BIT_LSC_ADJUST_RATIO       4
#define FIX_BIT_RADIAL_ADJUST_CURVE    4
#define FIX_BIT_VAR_TEXTURE            4
#define FIX_BIT_BF_W                   7
#define FIX_BIT_DENOISE_STRENGTH       4//7
#define FIX_BIT_SOFT_THRESHOLD_SCALE   8
#define FIX_BIT_SOFT_THRESHOLD_SCALE_V2   4
#define FIX_BIT_DIRECTION_STRENGTH     FIX_BIT_BF_SCALE
#define FIX_BIT_4                      4
#define FIX_BIT_6                      6
#define FIX_BIT_7                      7
#define FIX_COEF_BIT                   2
#define YNR_FILT_MODE0                 0
#define YNR_FILT_MODE1                 1
#define YNR_DMA_NUM                    4

#define YNR_exp_lut_num 16
#define YNR_exp_lut_x 7
#define YNR_exp_lut_y 7
#define CLIPVALUE

#define WAVELET_LEVEL_1 0
#define WAVELET_LEVEL_2 1
#define WAVELET_LEVEL_3 2
#define WAVELET_LEVEL_4 3
#define YNR_SIGMA_BITS  10

typedef struct Aynr_GainState_s {
    int gain_stat_full_last;
    int gainState;
    int gainState_last;
    float gain_th0[2];
    float gain_th1[2];
    float gain_cur;
    float ratio;
} Aynr_GainState_t;


//anr context
typedef struct Aynr_Context_V1_s {
    Aynr_ExpInfo_V1_t stExpInfo;
    Aynr_State_V1_t eState;
    Aynr_OPMode_V1_t eMode;

    Aynr_Auto_Attr_V1_t stAuto;
    Aynr_Manual_Attr_V1_t stManual;
	
	float fLuma_SF_Strength;

	int refYuvBit;
	bool isIQParaUpdate;
	bool isGrayMode;
	Aynr_ParamMode_V1_t eParamMode;
	int prepare_type;

	CalibDb_YNR_2_t stYnrCalib;
	CalibDbV2_YnrV1_t ynr_v1;

	int mfnr_mode_3to1;
	Aynr_GainState_t stGainState;
	
} Aynr_Context_V1_t;






RKAIQ_END_DECLARE

#endif


