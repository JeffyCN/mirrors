/*
 * rk_aiq_amfnr_algo_mfnr_v1.h
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

#ifndef _RKAIQ_MFNR_V1_H_
#define _RKAIQ_MFNR_V1_H_

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "amfnr/rk_aiq_types_amfnr_algo_prvt_v1.h"
#include "RkAiqCalibDbTypesV2.h"

//RKAIQ_BEGIN_DECLARE

Amfnr_Result_V1_t mfnr_get_mode_cell_idx_by_name_v1(CalibDb_MFNR_2_t *pCalibdb, const char *name, int *mode_idx);

Amfnr_Result_V1_t mfnr_get_setting_idx_by_name_v1(CalibDb_MFNR_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx);

Amfnr_Result_V1_t init_mfnr_dynamic_params_v1(RK_MFNR_Dynamic_V1_t *pDynamic, CalibDb_MFNR_2_t *pCalibdb, int mode_idx);

Amfnr_Result_V1_t mfnr_config_dynamic_param_v1(RK_MFNR_Dynamic_V1_t *pDynamic,  CalibDb_MFNR_2_t *pCalibdb, char* param_mode);

Amfnr_Result_V1_t mfnr_config_setting_param_v1(RK_MFNR_Params_V1_t *pParams, CalibDb_MFNR_2_t *pCalibdb, char* param_mode, char* snr_name);

Amfnr_Result_V1_t init_mfnr_params_v1(RK_MFNR_Params_V1_t *pParams, CalibDb_MFNR_2_t *pCalibdb, int mode_idx, int setting_idx);

Amfnr_Result_V1_t select_mfnr_params_by_ISO_v1(RK_MFNR_Params_V1_t *stmfnrParams, RK_MFNR_Params_V1_Select_t *stmfnrParamsSelected, Amfnr_ExpInfo_V1_t *pExpInfo, int bits_proc);

Amfnr_Result_V1_t mfnr_fix_transfer_v1(RK_MFNR_Params_V1_Select_t* tnr, RK_MFNR_Fix_V1_t *pMfnrCfg, Amfnr_ExpInfo_V1_t *pExpInfo, float gain_ratio, float fLumaStrength, float fChromaStrength);

Amfnr_Result_V1_t mfnr_fix_Printf_v1(RK_MFNR_Fix_V1_t  * pMfnrCfg);

Amfnr_Result_V1_t mfnr_dynamic_calc_v1(RK_MFNR_Dynamic_V1_t  * pDynamic, Amfnr_ExpInfo_V1_t *pExpInfo);

Amfnr_Result_V1_t mfnr_get_setting_idx_by_name_json_v1(CalibDbV2_MFNR_t *pCalibdb, char *name, int* calib_idx, int *tuning_idx);

Amfnr_Result_V1_t init_mfnr_params_json_v1(RK_MFNR_Params_V1_t *pParams, CalibDbV2_MFNR_t *pCalibdb, int calib_idx, int tuning_idx);

Amfnr_Result_V1_t mfnr_config_setting_param_json_v1(RK_MFNR_Params_V1_t *pParams, CalibDbV2_MFNR_t *pCalibdb, char* param_mode, char* snr_name);

Amfnr_Result_V1_t init_mfnr_dynamic_params_json_v1(RK_MFNR_Dynamic_V1_t *pDynamic, CalibDbV2_MFNR_t *pCalibdb);

Amfnr_Result_V1_t mfnr_config_dynamic_param_json_v1(RK_MFNR_Dynamic_V1_t *pDynamic, CalibDbV2_MFNR_t *pCalibdb, char* param_mode);

Amfnr_Result_V1_t mfnr_config_motion_param_json_v1(CalibDb_MFNR_Motion_t *pMotion,  CalibDbV2_MFNR_t *pCalibdb, char* param_mode);

Amfnr_Result_V1_t mfnr_calibdbV2_assign_v1(CalibDbV2_MFNR_t *pDst, CalibDbV2_MFNR_t *pSrc);

void mfnr_calibdbV2_free_v1(CalibDbV2_MFNR_t *pCalibdbV2);

Amfnr_Result_V1_t mfnr_algo_param_printf_v1(RK_MFNR_Params_V1_t *pParams);




//RKAIQ_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

