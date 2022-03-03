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

#ifndef _RK_AIQ_TYPES_AUVNR_ALGO_PRVT_V1_H_
#define _RK_AIQ_TYPES_AUVNR_ALGO_PRVT_V1_H_

#include "auvnr/rk_aiq_types_auvnr_algo_int_v1.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

//RKAIQ_BEGIN_DECLARE

/************uvnr fix***************/
#define RKUVNR_ratio 0
#define RKUVNR_offset 4
#define RKUVNR_kernels 7
#define RKUVNR_medRatio 4
#define RKUVNR_sigmaR 0
#define RKUVNR_uvgain 4
#define RKUVNR_exp2_lut_y 7
#define RKUVNR_bfRatio RKUVNR_exp2_lut_y
#define RKUVNR_gainRatio 7
#define RKUVNR_imgBit_set 8
#define RKUVNR_log2e 6

typedef struct Auvnr_GainState_s {
    int gain_stat_full_last;
    int gainState;
    int gainState_last;
    float gain_th0[2];
    float gain_th1[2];
    float gain_cur;
    float ratio;
} Auvnr_GainState_t;


//anr context
typedef struct Auvnr_Context_V1_s {
    Auvnr_ExpInfo_t stExpInfo;
    Auvnr_State_t eState;
    Auvnr_OPMode_t eMode;

    Auvnr_Auto_Attr_V1_t stAuto;
    Auvnr_Manual_Attr_V1_t stManual;

    CalibDb_UVNR_2_t stUvnrCalib;
	CalibDbV2_UVNR_t uvnr_v1;
	
	float fChrom_SF_Strength;

	bool isIQParaUpdate;
	bool isGrayMode;
	Auvnr_ParamMode_t eParamMode;

	int rawWidth;
	int rawHeight;

	Auvnr_GainState_t stGainState;
	int mfnr_mode_3to1;
	int mfnr_local_gain_en;
	int prepare_type;
} Auvnr_Context_V1_t;






//RKAIQ_END_DECLARE

#endif


