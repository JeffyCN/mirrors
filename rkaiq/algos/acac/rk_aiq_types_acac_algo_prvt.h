/*
 * rk_aiq_types_acac_algo_prvt.h
 *
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
#ifndef _RK_AIQ_TYPES_ACAC_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_ACAC_ALGO_PRVT_H_

#include "base/xcam_common.h"
#include "algos/acac/cac_adaptor.h"
#include "common/rk_aiq_comm.h"
#include "common/rk_aiq_types.h"

RKAIQ_BEGIN_DECLARE

typedef struct _RkAiqAlgoContext {
    void* handle;
} RkAiqAlgoContext;

RKAIQ_END_DECLARE

#endif  // _RK_AIQ_TYPES_ACAC_ALGO_PRVT_H_
