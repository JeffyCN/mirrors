/*
 * rk_aiq_auvnr_algo_uvnr_v1.h
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

#ifndef __RKAIQ_UVNR_V1_H_
#define __RKAIQ_UVNR_V1_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "auvnr/rk_aiq_types_auvnr_algo_prvt_v1.h"

RKAIQ_BEGIN_DECLARE


Auvnr_result_t uvnr_get_mode_cell_idx_by_name_v1(CalibDb_UVNR_2_t *pCalibdb, char *name, int *mode_idx);

Auvnr_result_t uvnr_get_setting_idx_by_name_v1(CalibDb_UVNR_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx);

Auvnr_result_t uvnr_config_setting_param_v1(RK_UVNR_Params_V1_t *pParams, CalibDb_UVNR_2_t *pCalibdb, char* param_mode, char * snr_name);

Auvnr_result_t init_uvnr_params_v1(RK_UVNR_Params_V1_t *pParams, CalibDb_UVNR_2_t *pCalibdb, int mode_idx, int setting_idx);

Auvnr_result_t select_uvnr_params_by_ISO_v1(RK_UVNR_Params_V1_t *stRKUVNrParams, RK_UVNR_Params_V1_Select_t *stRKUVNrParamsSelected, Auvnr_ExpInfo_t *pExpInfo);

Auvnr_result_t uvnr_fix_transfer_v1(RK_UVNR_Params_V1_Select_t *uvnr, RK_UVNR_Fix_V1_t *pNrCfg, Auvnr_ExpInfo_t *pExpInfo, float gain_ratio, float fStrength);

Auvnr_result_t uvnr_fix_Printf_v1(RK_UVNR_Fix_V1_t  * pNrCfg);

Auvnr_result_t uvnr_config_setting_param_json_v1(RK_UVNR_Params_V1_t *pParams, CalibDbV2_UVNR_t *pCalibdb, char* param_mode, char * snr_name);

Auvnr_result_t init_uvnr_params_json_v1(RK_UVNR_Params_V1_t *pParams, CalibDbV2_UVNR_t *pCalibdb, int setting_idx);

Auvnr_result_t uvnr_get_setting_idx_by_name_json_v1(CalibDbV2_UVNR_t *pCalibdb, char *name, int *tuning_idx);

Auvnr_result_t uvnr_calibdbV2_assign_v1(CalibDbV2_UVNR_t *pDst, CalibDbV2_UVNR_t *pSrc);

void uvnr_calibdbV2_free_v1(CalibDbV2_UVNR_t *pCalibdbV2);

Auvnr_result_t uvnr_algo_param_printf_v1(RK_UVNR_Params_V1_t *pParams);



RKAIQ_END_DECLARE





#endif

