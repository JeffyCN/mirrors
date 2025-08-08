/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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
 */
#ifndef _HWI_API_LSC21_H_
#define _HWI_API_LSC21_H_

#include "rk_aiq_comm.h"
#include "isp/rk_aiq_isp_lsc21.h"
#include "common/rkisp32-config.h"

RKAIQ_BEGIN_DECLARE

int hwi_api_lsc21_params_check(lsc_param_t* attr);
void hwi_api_lsc21_params_dump(lsc_param_t* attr, struct isp3x_lsc_cfg *phwcfg);
void hwi_api_lsc21_params_cvt(lsc_param_t* attr, struct isp32_isp_params_cfg* isp_cfg);

RKAIQ_END_DECLARE

#endif