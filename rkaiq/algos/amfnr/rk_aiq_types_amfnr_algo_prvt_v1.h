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

#ifndef _RK_AIQ_TYPES_AMFNR_ALGO_PRVT_V1_H_
#define _RK_AIQ_TYPES_AMFNR_ALGO_PRVT_V1_H_

#include "amfnr/rk_aiq_types_amfnr_algo_int_v1.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"


RKAIQ_BEGIN_DECLARE

#define MFNR_MAX_WEIGHT_LIMIT_UV (255)
/************mfnr fix***************/
#define MFNR_MATALAB_FLG                    0
#define FIX_VAL                             (1 && !MFNR_MATALAB_FLG)//  1 for rtl_3dnr, 0 for matlab_3dnr
#define ACCURATE_LOW                        0
#define F_ACCURATE_FLG_SCALE_L              ACCURATE_LOW
#if FIX_VAL
#define FIX_ENABLE_GAIN_IN                  FIX_ENABLE
#define FIX_ENABLE_DELTA_SCALE              FIX_ENABLE
#define FIX_ENABLE_DELTA_CALC               FIX_ENABLE
#define FIX_ENABLE_DELTA_TO_DELTA_SQR       FIX_ENABLE
#define FIX_ENABLE_DELTA_CONV               FIX_ENABLE
#define FIX_ENABLE_PK_CALC                  FIX_ENABLE
#define F_ACCURATE_FLG_SCALE_L              ACCURATE_LOW
#define F_ACCURATE_FLG_DELTA_SCALE_L        ACCURATE_LOW
#define F_ACCURATE_FLG_LUMA_W_IN_CHROMA     ACCURATE_LOW
#define F_ACCURATE_FLG_CONV_WEIGHT          ACCURATE_LOW
#define F_ACCURATE_FLG_CONV_OUT             ACCURATE_LOW
#define F_ACCURATE_FLG_CONV1_OUT            ACCURATE_LOW
#define F_ACCURATE_FLG_SIGMA                ACCURATE_LOW
#define F_ACCURATE_FLG_FRQ_DELTA            ACCURATE_LOW
#define F_ACCURATE_FLG_TXT_THRD_RATIO       ACCURATE_LOW
#define F_ACCURATE_FLG_TXT_THRD             ACCURATE_LOW
#define F_ACCURATE_FLG_TXT_RATIO            ACCURATE_LOW
#define F_ACCURATE_FLG_DELTA                ACCURATE_LOW
#define F_ACCURATE_FLG_EXP_VAL              ACCURATE_LOW
#define F_ACCURATE_FLG_PK_MID               ACCURATE_LOW
#define F_ACCURATE_FLG_EXP_SIGMA            ACCURATE_LOW
#define F_ACCURATE_FLG_EXP_SIGMA_RATIO      ACCURATE_LOW
#define F_ACCURATE_FLG_GAIN                 ACCURATE_LOW
#define F_ACCURATE_FLG_PIXEL_RECON          ACCURATE_LOW
#define F_ACCURATE_FLG_PIXEL_ORI            ACCURATE_LOW
#else
#define FIX_ENABLE_GAIN_IN                  FIX_DISABLE
#define FIX_ENABLE_DELTA_SCALE              FIX_DISABLE
#define FIX_ENABLE_DELTA_CALC               FIX_DISABLE
#define FIX_ENABLE_DELTA_TO_DELTA_SQR       FIX_DISABLE
#define FIX_ENABLE_DELTA_CONV               FIX_DISABLE
#define FIX_ENABLE_PK_CALC                  FIX_DISABLE
#define F_ACCURATE_FLG_SCALE_L              ACCURATE_HIGH
#define F_ACCURATE_FLG_DELTA_SCALE_L        ACCURATE_HIGH
#define F_ACCURATE_FLG_LUMA_W_IN_CHROMA     ACCURATE_HIGH
#define F_ACCURATE_FLG_CONV_WEIGHT          ACCURATE_HIGH
#define F_ACCURATE_FLG_CONV_OUT             ACCURATE_HIGH
#define F_ACCURATE_FLG_CONV1_OUT            ACCURATE_HIGH
#define F_ACCURATE_FLG_SIGMA                ACCURATE_HIGH
#define F_ACCURATE_FLG_FRQ_DELTA            ACCURATE_HIGH
#define F_ACCURATE_FLG_TXT_THRD_RATIO       ACCURATE_HIGH
#define F_ACCURATE_FLG_TXT_THRD             ACCURATE_HIGH
#define F_ACCURATE_FLG_TXT_RATIO            ACCURATE_HIGH
#define F_ACCURATE_FLG_DELTA                ACCURATE_HIGH
#define F_ACCURATE_FLG_EXP_VAL              ACCURATE_HIGH
#define F_ACCURATE_FLG_PK_MID               ACCURATE_HIGH
#define F_ACCURATE_FLG_EXP_SIGMA            ACCURATE_HIGH
#define F_ACCURATE_FLG_EXP_SIGMA_RATIO      ACCURATE_HIGH
#define F_ACCURATE_FLG_GAIN                 ACCURATE_HIGH
#define F_ACCURATE_FLG_PIXEL_RECON          ACCURATE_HIGH
#define F_ACCURATE_FLG_PIXEL_ORI            ACCURATE_HIGH
#endif


#define F_DECI_CONV_WEIGHT_ACCURATE                 13
#define F_DECI_CONV_WEIGHT_REAL                     8
#define F_DECI_CONV_WEIGHT                          (F_ACCURATE_FLG_CONV_WEIGHT ? F_DECI_CONV_WEIGHT_ACCURATE : F_DECI_CONV_WEIGHT_REAL)
#define F_DECI_PIXEL_SIGMA_CONV_WEIGHT              F_DECI_CONV_WEIGHT
#define F_DECI_GAIN_ACCURATE                        (16)
#define F_DECI_GAIN_REAL                            8 //(MAX(F_DECI_GAIN_IN*2, 8))
#define F_DECI_GAIN                                 (F_ACCURATE_FLG_GAIN ? (F_DECI_GAIN_ACCURATE) : (F_DECI_GAIN_REAL))
#define F_DECI_GAIN_SQRT                            4// 8 for rtl_sqrt(F_DECI_GAIN / 2)
#define F_DECI_GAIN_GLB_SQRT                        F_DECI_GAIN_SQRT
#define F_DECI_GAIN_GLB_SQRT_INV                    13 // (F_INTE_GAIN_SQRT + F_DECI_GAIN_SQRT)     // 13   // (F_INTE_GAIN_SQRT + F_DECI_GAIN_SQRT + 4) is better jmj_3dnr
#define F_DECI_LUMASCALE                            6       //8 for rtl_3dnr
#define F_DECI_SCALE_L_ACCURATE                     18
#define F_DECI_SCALE_L_REAL                         8
#define F_DECI_SCALE_L                              (F_ACCURATE_FLG_SCALE_L ? F_DECI_SCALE_L_ACCURATE : F_DECI_SCALE_L_REAL)
#define F_DECI_SCALE_L_UV_ACCURATE                  18
#define F_DECI_SCALE_L_UV_REAL                      6
#define F_DECI_SCALE_L_UV                           (F_ACCURATE_FLG_SCALE_L ? F_DECI_SCALE_L_UV_ACCURATE : F_DECI_SCALE_L_UV_REAL)
#define F_DECI_LUMA_W_IN_CHROMA_ACCURATE            16
#define F_DECI_LUMA_W_IN_CHROMA_REAL                5
#define F_DECI_LUMA_W_IN_CHROMA                     (F_ACCURATE_FLG_LUMA_W_IN_CHROMA ? F_DECI_LUMA_W_IN_CHROMA_ACCURATE : F_DECI_LUMA_W_IN_CHROMA_REAL)
#define F_DECI_SIGMA_ACCURATE                       (24)
#define F_DECI_SIGMA_REAL                           (6 )
#define F_DECI_SIGMA                                (F_ACCURATE_FLG_SIGMA ? F_DECI_SIGMA_ACCURATE : F_DECI_SIGMA_REAL)
#define F_DECI_TXT_THRD_RATIO_ACCURATE              (16)
#define F_DECI_TXT_THRD_RATIO_REAL                  (6)     //(8)   
#define F_DECI_TXT_THRD_RATIO                       (F_ACCURATE_FLG_TXT_THRD_RATIO ? F_DECI_TXT_THRD_RATIO_ACCURATE : F_DECI_TXT_THRD_RATIO_REAL)
#define F_INTE_GAIN_GLB_SQRT_INV                    0
#define F_DECI_GAIN_GLB_SQRT_INV                    13
#define GAIN_SIGMA_BITS_ACT     10
#define MAX_INTEPORATATION_LUMAPOINT    17
#define GAIN_HDR_MERGE_IN_FIX_BITS_DECI             6
#define GAIN_HDR_MERGE_IN2_FIX_BITS_INTE            12
#define GAIN_HDR_MERGE_IN0_FIX_BITS_INTE            8



typedef struct Amfnr_GainState_s {
    int gain_stat_full_last;
    int gainState;
    int gainState_last;
    float gain_th0[2];
    float gain_th1[2];
    float gain_cur;
    float ratio;
} Amfnr_GainState_t;


//anr context
typedef struct Amfnr_Context_V1_s {
    Amfnr_ExpInfo_V1_t stExpInfo;
    Amfnr_State_V1_t eState;
    Amfnr_OPMode_V1_t eMode;

    Amfnr_Auto_Attr_V1_t stAuto;
    Amfnr_Manual_Attr_V1_t stManual;
	
	float fLuma_TF_Strength;
	float fChroma_TF_Strength;

	int refYuvBit;
	bool isIQParaUpdate;
	bool isGrayMode;
	Amfnr_ParamMode_V1_t eParamMode;
	int prepare_type;

	CalibDb_MFNR_2_t stMfnrCalib;
	CalibDbV2_MFNR_t mfnr_v1;

	int mfnr_mode_3to1;
	Amfnr_GainState_t stGainState;

	CalibDb_MFNR_Motion_t  stMotion;
} Amfnr_Context_V1_t;






RKAIQ_END_DECLARE

#endif


