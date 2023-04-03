/*
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */
#ifndef _RK_AIQ_ALGO_ACAC_ITF_H_
#define _RK_AIQ_ALGO_ACAC_ITF_H_

#include "algos/rk_aiq_algo_des.h"
#include "common/rk_aiq_comm.h"

#define RKISP_ALGO_ACAC_VERSION     "v0.0.2"
#define RKISP_ALGO_ACAC_VENDOR      "Rockchip"
#define RKISP_ALGO_ACAC_DESCRIPTION "Rockchip CAC algo for ISP3.0 and ISP3.2"

RKAIQ_BEGIN_DECLARE

extern RkAiqAlgoDescription g_RkIspAlgoDescAcac;

RKAIQ_END_DECLARE

#endif  //_RK_AIQ_ALGO_ACAC_ITF_H_
