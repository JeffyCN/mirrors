/*
 * rk_aiq_abayernr_algo_bayernr_v1.h
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

#ifndef _RKAIQ_BAYERNR_H_
#define _RKAIQ_BAYERNR_H_

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "arawnr/rk_aiq_types_abayernr_algo_prvt_v1.h"
#include "RkAiqCalibDbTypesV2.h"

RKAIQ_BEGIN_DECLARE



#if ABAYERNR_USE_JSON_FILE_V1

Abayernr_result_v1_t bayernrV1_get_setting_idx_by_name_json(CalibDbV2_BayerNrV1_t *pCalibdb, char *name,  int *calib_idx,  int * tuning_idx);

Abayernr_result_v1_t init_bayernrV1_params_json(RK_Bayernr_Params_V1_t *pParams, CalibDbV2_BayerNrV1_t *pCalibdb, int calib_idx, int tuning_idx);

Abayernr_result_v1_t bayernrV1_config_setting_param_json(RK_Bayernr_Params_V1_t *pParams, CalibDbV2_BayerNrV1_t *pCalibdb, char* param_mode, char * snr_name);

Abayernr_result_v1_t bayernrV1_calibdbV2_assign(CalibDbV2_BayerNrV1_t *pDst, CalibDbV2_BayerNrV1_t *pSrc);

void bayernrV1_calibdbV2_release(CalibDbV2_BayerNrV1_t *pCalibdbV2);

#else

Abayernr_result_v1_t bayernrV1_get_mode_cell_idx_by_name(CalibDb_BayerNr_2_t *pCalibdb, char *name, int *mode_idx);

Abayernr_result_v1_t bayernrV1_get_setting_idx_by_name(CalibDb_BayerNr_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx);

Abayernr_result_v1_t bayernrV1_config_setting_param(RK_Bayernr_Params_V1_t *pParams, CalibDb_BayerNr_2_t *pCalibdb, char* param_mode, char * snr_name);

#endif

Abayernr_result_v1_t init_bayernrV1_params(RK_Bayernr_Params_V1_t *pParams, CalibDb_BayerNr_2_t *pCalibdb, int mode_idx, int setting_idx);

Abayernr_result_v1_t select_bayernrV1_params_by_ISO(RK_Bayernr_Params_V1_t *stBayerNrParams, RK_Bayernr_Params_Select_V1_t *stBayerNrParamsSelected, Abayernr_ExpInfo_V1_t *pExpInfo);

unsigned short bayernrV1_get_trans(int tmpfix);

Abayernr_result_v1_t bayernrV1_fix_tranfer(RK_Bayernr_Params_Select_V1_t* rawnr, RK_Bayernr_Fix_V1_t *pRawnrCfg, float fStrength);

Abayernr_result_v1_t bayernrV1_fix_printf(RK_Bayernr_Fix_V1_t * pRawnrCfg);


RKAIQ_END_DECLARE



#endif  // BAYERNR_READ_PARM_H_

