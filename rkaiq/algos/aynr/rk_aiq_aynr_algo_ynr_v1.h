/*
 * rk_aiq_aynr_algo_ynr_v1.h
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

#ifndef _RKAIQ_YNR_V1_H_
#define _RKAIQ_YNR_V1_H_

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "aynr/rk_aiq_types_aynr_algo_prvt_v1.h"
#include "RkAiqCalibDbTypesV2.h"

RKAIQ_BEGIN_DECLARE

Aynr_Result_V1_t ynr_get_mode_cell_idx_by_name_v1(CalibDb_YNR_2_t *pCalibdb, char *name, int *mode_idx);

Aynr_Result_V1_t ynr_get_setting_idx_by_name_v1(CalibDb_YNR_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx);

Aynr_Result_V1_t ynr_config_setting_param_v1(RK_YNR_Params_V1_t *pParams, CalibDb_YNR_2_t *pCalibdb, char* param_mode, char* snr_name);

Aynr_Result_V1_t init_ynr_params_v1(RK_YNR_Params_V1_t *pYnrParams, CalibDb_YNR_2_t* pYnrCalib, int mode_idx, int setting_idx);

Aynr_Result_V1_t select_ynr_params_by_ISO_v1(RK_YNR_Params_V1_t *stYnrParam, RK_YNR_Params_V1_Select_t *stYnrParamSelected, Aynr_ExpInfo_V1_t *pExpInfo, short bitValue);

Aynr_Result_V1_t ynr_fix_transfer_v1(RK_YNR_Params_V1_Select_t* ynr, RK_YNR_Fix_V1_t *pNrCfg, float gain_ratio, float fStrength);

Aynr_Result_V1_t ynr_fix_printf_v1(RK_YNR_Fix_V1_t * pNrCfg);

Aynr_Result_V1_t ynr_get_setting_idx_by_name_json_v1(CalibDbV2_YnrV1_t *pCalibdb, char *name, int *calib_idx, int *tuning_idx);

Aynr_Result_V1_t init_ynr_params_json_v1(RK_YNR_Params_V1_t *pYnrParams, CalibDbV2_YnrV1_t* pYnrCalib,  int calib_idx, int tuning_idx);

Aynr_Result_V1_t ynr_config_setting_param_json_v1(RK_YNR_Params_V1_t *pParams, CalibDbV2_YnrV1_t*pCalibdb, char* param_mode, char* snr_name);

Aynr_Result_V1_t ynr_calibdbV2_assign_v1(CalibDbV2_YnrV1_t *pDst, CalibDbV2_YnrV1_t *pSrc);

void ynr_calibdbV2_free_v1(CalibDbV2_YnrV1_t *pCalibdbV2);

Aynr_Result_V1_t ynr_algo_param_printf_v1(RK_YNR_Params_V1_t *pYnrParams);



RKAIQ_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

