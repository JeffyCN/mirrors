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

#ifndef _RK_AIQ_TYPES_ABAYERNR_ALGO_PRVT_V1_H_
#define _RK_AIQ_TYPES_ABAYERNR_ALGO_PRVT_V1_H_

#include "arawnr/rk_aiq_types_abayernr_algo_int_v1.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "RkAiqCalibDbV2Helper.h"




//RKAIQ_BEGIN_DECLARE

/************bayernr fix***************/
#define FIXNLMCALC      10
#define FIXDIFMAX       ((long long)1<<(14))
#define LUTMAXM1_FIX    300.0 //29.0
#define LUTPRECISION_FIX (1<<FIXNLMCALC)



//anr context
typedef struct Abayernr_Context_V1_s {
    Abayernr_ExpInfo_V1_t stExpInfo;
    Abayernr_State_V1_t eState;
    Abayernr_OPMode_V1_t eMode;

    Abayernr_Auto_Attr_V1_t stAuto;
    Abayernr_Manual_Attr_V1_t stManual;

	#if ABAYERNR_USE_JSON_FILE_V1
	CalibDbV2_BayerNrV1_t bayernr_v1;  
	#else
	CalibDb_BayerNr_2_t stBayernrCalib;
	#endif
	
	float fRawnr_SF_Strength;

	bool isIQParaUpdate;
	bool isGrayMode;
	int prepare_type;
	Abayernr_ParamMode_V1_t eParamMode;
} Abayernr_Context_V1_t;






//RKAIQ_END_DECLARE

#endif


