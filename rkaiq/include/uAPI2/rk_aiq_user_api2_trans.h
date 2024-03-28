/*
 *  Copyright (c) 2023 Rockchip Corporation
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

#ifndef _RK_AIQ_USER_API2_TRANS_H_
#define _RK_AIQ_USER_API2_TRANS_H_

#include "rk_aiq_api_types_trans.h"

RKAIQ_BEGIN_DECLARE

#ifndef RK_AIQ_SYS_CTX_T
#define RK_AIQ_SYS_CTX_T
typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
#endif

XCamReturn
rk_aiq_user_api2_trans_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, trans_api_attrib_t* attr);
XCamReturn
rk_aiq_user_api2_trans_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, trans_api_attrib_t* attr);
XCamReturn
rk_aiq_user_api2_trans_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, trans_status_t* status);

RKAIQ_END_DECLARE

#endif
