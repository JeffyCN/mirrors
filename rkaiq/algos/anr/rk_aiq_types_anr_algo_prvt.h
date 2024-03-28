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

#ifndef _RK_AIQ_TYPES_ANR_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_ANR_ALGO_PRVT_H_

#include "anr/rk_aiq_types_anr_algo_int.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"


RKAIQ_BEGIN_DECLARE

#define NR_ISO_REG_DIV 1


/************bayernr fix***************/
#define FIXNLMCALC      10
#define FIXDIFMAX       ((long long)1<<(14))
#define LUTMAXM1_FIX    300.0 //29.0
#define LUTPRECISION_FIX (1<<FIXNLMCALC)

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



typedef struct ANRGainState_s {
    int gain_stat_full_last;
    int gainState;
    int gainState_last;
    float gain_th0[2];
    float gain_th1[2];
    float gain_cur;
    float ratio;
} ANRGainState_t;

//anr context
typedef struct ANRContext_s {
    ANRExpInfo_t stExpInfo;

    float fEnvLight;
    ANRState_t eState;
    ANROPMode_t eMode;

    ANR_Auto_Attr_t stAuto;
    ANR_Manual_Attr_t stManual;

    int refYuvBit;

    CalibDb_BayerNr_2_t stBayernrCalib;
    CalibDb_MFNR_2_t stMfnrCalib;
    CalibDb_UVNR_2_t stUvnrCalib;
    CalibDb_YNR_2_t stYnrCalib;

    ANRGainState_t stGainState;
	
	float fLuma_TF_Strength;
	float fLuma_SF_Strength;
	float fChroma_TF_Strength;
	float fChroma_SF_Strength;
	float fRawnr_SF_Strength;

	bool isIQParaUpdate;
	bool isGrayMode;
	ANRParamMode_t eParamMode;

	int prepare_type;

	//json
	CalibDbV2_BayerNrV1_t bayernr_v1;
  	CalibDbV2_MFNR_t mfnr_v1;
  	CalibDbV2_UVNR_t uvnr_v1;
  	CalibDbV2_YnrV1_t ynr_v1;

	CalibDb_MFNR_Motion_t  stMotion;
} ANRContext_t;






RKAIQ_END_DECLARE

#endif


