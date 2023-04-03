/*
 *rk_aiq_accm_algo_com.h
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

#ifndef _RK_AIQ_ACCM_ALGO_COM_H_
#define _RK_AIQ_ACCM_ALGO_COM_H_
#include "accm/rk_aiq_types_accm_algo_prvt.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"


RKAIQ_BEGIN_DECLARE

XCamReturn illuminant_index_estimation_ccm(int light_num, const CalibDbV2_Ccm_Accm_Cof_Para_t illAll[], float awbGain[2], int* illuminant_index);
XCamReturn Damping(const float damp, float *pMatrixUndamped, float *pMatrixDamped, float *pOffsetUndamped, float *pOffsetDamped);
void Saturationadjust(float fScale, float flevel1, float *pccMatrixA);
XCamReturn CamCalibDbGetCcmProfileByName(const CalibDbV2_Ccm_Tuning_Para_t *calibCcm, char* name, const CalibDbV2_Ccm_Matrix_Para_t **pCcmMatrixProfile);
XCamReturn interpCCMbywbgain(const CalibDbV2_Ccm_Tuning_Para_t* pCcm, accm_handle_t hAccm,
                             float fSaturation);
XCamReturn selectCCM(const CalibDbV2_Ccm_Tuning_Para_t* pCcm, accm_handle_t hAccm,
                     float fSaturation);
bool JudgeCcmRes3aConverge(ccm_3ares_info_t *res3a_info, accm_sw_info_t *accmSwInfo, float gain_th, float wbgain_th);
XCamReturn Swinfo_wbgain_init(float awbGain[2], const CalibDbV2_Ccm_Tuning_Para_t *pCalib, const char* illuName);
XCamReturn pCcmMatrixAll_init(accm_context_t* accm_context, const CalibDbV2_Ccm_Tuning_Para_t *pCalib );
#if RKAIQ_ACCM_ILLU_VOTE
XCamReturn ReloadCCMCalibV2(accm_handle_t hAccm, const CalibDbV2_Ccm_Tuning_Para_t* TuningPara);
#endif



RKAIQ_END_DECLARE

#endif

