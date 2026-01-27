/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _AIQ_ISP39_PARAMS_CVT_H_
#define _AIQ_ISP39_PARAMS_CVT_H_

#include <stdbool.h>
#include "modules/rk_aiq_isp39_modules.h"

typedef struct AiqIspParamsCvt_s AiqIspParamsCvt_t;
typedef struct aiq_params_base_s aiq_params_base_t;

bool Convert3aResultsToIsp39Cfg(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase,
                                void* isp_cfg_p, bool is_multi_isp);
void convertAiqAeToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqExpIspDgainToIsp39Params(AiqIspParamsCvt_t* pCvt, RKAiqAecExpInfo_t* ae_exp);
void convertAiqCcmToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqMergeToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqDrcToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqBlcToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqCsmToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqDpccToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqGammaToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqLscToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqCgcToIsp21Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqGainToIsp3xParams(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqCpToIsp20Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqIeToIsp20Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
void convertAiqAldcToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase,
                                 bool is_multi_isp);
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
void convertAiqRgbirToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
#endif
#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
void convertAiqAfToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
#endif
#if RKAIQ_HAVE_DUMPSYS
void AiqIspParamsCvt_updIsp39Params(void* src, void* dst);
#endif

#endif
