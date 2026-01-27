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

#include "rk_aiq_user_api2_common.h"
#include "RkAiqGlobalParamsManager_c.h"

RKAIQ_BEGIN_DECLARE

rk_aiq_sys_ctx_array_t
rk_aiq_user_api2_common_getSysCtxArray(const rk_aiq_sys_ctx_t* sys_ctx)
{
    rk_aiq_sys_ctx_array_t ctxs;
    ctxs.num = 0;  
	const rk_aiq_sys_ctx_t* aiq_ctx = NULL;
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (camgroup_ctx->cam_ctxs_array[i]) {
                ctxs.ctx[ctxs.num++] = camgroup_ctx->cam_ctxs_array[i];
            }
        }
#else
        LOGE("not compile with group mode !");
#endif
	} else {
		ctxs.ctx[0] = sys_ctx;
        ctxs.num++;  
	}
	return ctxs;

}

const rk_aiq_sys_ctx_t*
rk_aiq_user_api2_common_getSysCtx(const rk_aiq_sys_ctx_t* sys_ctx)
{
	const rk_aiq_sys_ctx_t* aiq_ctx = NULL;
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (camgroup_ctx->cam_ctxs_array[i]) {
                aiq_ctx = camgroup_ctx->cam_ctxs_array[i];
                break;
            }
        }
#endif
	} else {
		aiq_ctx = sys_ctx;
	}
	return aiq_ctx;
}

XCamReturn
rk_aiq_user_api2_common_processParams(const rk_aiq_sys_ctx_t* sys_ctx, bool set,
		rk_aiq_op_mode_t* opMode, bool* en, bool* bypass, int type,
		int man_param_size, void* man_param_ptr, int aut_param_size, void* aut_param_ptr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

	rk_aiq_global_params_wrap_t params;
	params.type = type;
	params.man_param_size = man_param_size;
	params.man_param_ptr = man_param_ptr;
	params.aut_param_size = aut_param_size;
	params.aut_param_ptr = aut_param_ptr;
	if (set) {
            if (opMode) params.opMode = *opMode;
            if (bypass) params.bypass = *bypass;
            if (en) params.en = *en;
            ret = GlobalParamsManager_set(&sys_ctx->_rkAiqManager->mGlobalParamsManager, &params);
	} else {
		ret = GlobalParamsManager_get(&sys_ctx->_rkAiqManager->mGlobalParamsManager, &params);
		if (ret == XCAM_RETURN_NO_ERROR) {
                    if (en) *en = params.en;
                    if (bypass) *bypass = params.bypass;
                    if (opMode) *opMode = (RKAiqOPMode_t)params.opMode;
                }
	}
	return ret;
}

RKAIQ_END_DECLARE
