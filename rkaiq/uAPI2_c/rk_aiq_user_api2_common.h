
/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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

#ifndef _RK_AIQ_USER_API2_COMMON_C_H_
#define _RK_AIQ_USER_API2_COMMON_C_H_

#include "uAPI2_c/rk_aiq_api_private_c.h"

RKAIQ_BEGIN_DECLARE

typedef struct rk_aiq_sys_ctx_array_s {
    const rk_aiq_sys_ctx_t* ctx[RK_AIQ_CAM_GROUP_MAX_CAMS];
    int num;
} rk_aiq_sys_ctx_array_t; 

rk_aiq_sys_ctx_array_t
rk_aiq_user_api2_common_getSysCtxArray(const rk_aiq_sys_ctx_t* sys_ctx);

const rk_aiq_sys_ctx_t*
rk_aiq_user_api2_common_getSysCtx(const rk_aiq_sys_ctx_t* sys_ctx);

XCamReturn
rk_aiq_user_api2_common_processParams(const rk_aiq_sys_ctx_t* sys_ctx, bool set,
		rk_aiq_op_mode_t* opMode, bool* en, bool* bypass, int type,
		int man_param_size, void* man_param_ptr, int aut_param_size, void* aut_param_ptr);

RKAIQ_END_DECLARE
#endif
