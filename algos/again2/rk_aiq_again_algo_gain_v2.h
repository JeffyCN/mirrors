/*
 * rk_aiq_again_algo_gain.h
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


#ifndef __RKAIQ_GAIN_V2_H_
#define __RKAIQ_GAIN_V2_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "again2/rk_aiq_types_again_algo_prvt_v2.h"




Again_result_V2_t gain_fix_transfer_v2(RK_GAIN_Params_V2_Select_t *pSelect, RK_GAIN_Fix_V2_t* pGainFix,  Again_ExpInfo_V2_t *pExpInfo, float gain_ratio);

Again_result_V2_t gain_fix_Printf_v2(RK_GAIN_Fix_V2_t  * pFix);









#endif

