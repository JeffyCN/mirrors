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
#include "rk_aiq_user_api2_acp.h"
#include "algo_handlers/RkAiqAcpHandle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn  rk_aiq_user_api2_acp_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, acp_attrib_t attr)
{
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            rk_aiq_user_api_acp_SetAttrib(camCtx, attr);
        }
        return XCAM_RETURN_NO_ERROR;
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return rk_aiq_user_api_acp_SetAttrib(sys_ctx, attr);
    }
}

XCamReturn  rk_aiq_user_api2_acp_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, acp_attrib_t *attr)
{
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;
            rk_aiq_user_api_acp_GetAttrib(camCtx, attr);
        }
        return XCAM_RETURN_NO_ERROR;
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return rk_aiq_user_api_acp_GetAttrib(sys_ctx, attr);
    }
}

RKAIQ_END_DECLARE



