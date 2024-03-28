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

#ifndef _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_ASHARP_ALGO_PRVT_H_

#include "asharp/rk_aiq_types_asharp_algo_int.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

RKAIQ_BEGIN_DECLARE

//sharp fix
#define reg_sharpenHW_lratio_fix_bits               8
#define reg_sharpenHW_hratio_fix_bits               8
#define reg_sharpenHW_M_ratio_fix_bits              2
#define reg_sharpenHW_H_ratio_fix_bits              2
#define reg_sharpenHW_pbf_ratio_fix_bits            7
#define reg_sharpenHW_lum_min_m_fix_bits            4
#define reg_sharpenHW_hbf_ratio_fix_bits            8
#define reg_sharpenHW_pPBfCoeff_fix_bits            6
#define reg_sharpenHW_pMRfCoeff_fix_bits            7
#define reg_sharpenHW_pMBfCoeff_fix_bits            6
#define reg_sharpenHW_pHRfCoeff_fix_bits            7
#define reg_sharpenHW_pHBfCoeff_fix_bits            6

//edgefilter fix
#define RK_EDGEFILTER_COEF_BIT                  (6)
#define reg_dir_min_fix_bits                    RK_EDGEFILTER_COEF_BIT
#define reg_l_alpha_fix_bits                    8
#define reg_g_alpha_fix_bits                    8
#define reg_detail_alpha_dog_fix_bits           6
#define reg_h0_h_coef_5x5_fix_bits              6
#define reg_h_coef_5x5_fix_bits                 6
#define reg_gf_coef_3x3_fix_bits                4
#define reg_dog_kernel_fix_bits                 6
#define reg_smoth4_fix_bits                     8

typedef struct AsharpContext_s {
    AsharpExpInfo_t stExpInfo;
    float fEnvLight;
    AsharpState_t eState;
    AsharpOPMode_t eMode;

    Asharp_Auto_Attr_t stAuto;
    Asharp_Manual_Attr_t stManual;

    //xml
    CalibDb_Sharp_2_t stSharpCalib;
    CalibDb_EdgeFilter_2_t stEdgeFltCalib;

    float fStrength;

	bool isIQParaUpdate;
	bool isGrayMode;
	AsharpParamMode_t eParamMode;

	int prepare_type;
	int mfnr_mode_3to1;

	//json
	CalibDbV2_Edgefilter_t edgefilter_v1;	
	CalibDbV2_SharpV1_t sharp_v1;
	
} AsharpContext_t;


RKAIQ_END_DECLARE

#endif










