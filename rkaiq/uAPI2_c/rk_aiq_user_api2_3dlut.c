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

#if RKAIQ_HAVE_3DLUT_V1
#include "isp/rk_aiq_isp_3dlut20.h"
#else
#error "wrong lut3d hw version !"
#endif
#include "uAPI2/rk_aiq_user_api2_3dlut.h"
#include "aiq_core_c/algo_handlers/RkAiq3dlutHandler.h"
#include "uAPI2_c/rk_aiq_user_api2_common.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn
rk_aiq_user_api2_3dlut_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);
	rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);

	int type = RESULT_TYPE_LUT3D_PARAM;
	int man_param_size = sizeof(lut3d_param_t);
	int aut_param_size = sizeof(lut3d_param_auto_t);

    for (int i = 0; i < ctx_array.num; i++) {
        if (attr->opMode == RK_AIQ_OP_MODE_MANUAL || attr->opMode == RK_AIQ_OP_MODE_AUTO) {
            ret = rk_aiq_user_api2_common_processParams(ctx_array.ctx[i], true,
                    &attr->opMode, &attr->en, &attr->bypass,
                    type, man_param_size, &attr->stMan, aut_param_size,  &attr->stAuto);
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_A3DLUT("wrong mode %d !", attr->opMode);
        }
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_3dlut_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_api_attrib_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);

	const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);
	int type = RESULT_TYPE_LUT3D_PARAM;
	int man_param_size = sizeof(lut3d_param_t);
	int aut_param_size = sizeof(lut3d_param_auto_t);

    return rk_aiq_user_api2_common_processParams(ctx, false,
				&attr->opMode, &attr->en, &attr->bypass,
				type, man_param_size, &attr->stMan, aut_param_size,  &attr->stAuto);
}

XCamReturn
rk_aiq_user_api2_3dlut_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, lut3d_status_t* status)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);

	const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);

	AiqAlgoHandler_t* algo_handle =
		ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_A3DLUT];

	if (algo_handle) {
		ret = AiqAlgoHandler_queryStatus_common(algo_handle,
				&status->opMode, &status->en, &status->bypass, &status->stMan);
        if (status->opMode == RK_AIQ_OP_MODE_AUTO)
            AiqAlgoHandler3dlut_queryalut3dStatus((AiqAlgoHandler3dlut_t*)algo_handle, &status->alut3dStatus);
	}

	return ret;
}

XCamReturn
rk_aiq_user_api2_3dlut_SetCalib(const rk_aiq_sys_ctx_t* sys_ctx, alut3d_lut3dCalib_t* calib)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);

	rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);

    for (int i = 0; i < ctx_array.num; i++) {
        AiqAlgoHandler_t* algo_handle =
		    ctx_array.ctx[i]->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_A3DLUT];
        if (algo_handle) {
            ret = AiqAlgoHandler3dlut_setCalib((AiqAlgoHandler3dlut_t*)algo_handle, calib);
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_A3DLUT("ctx[%d]: SetCalib failed!", i);
        }
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_3dlut_GetCalib(const rk_aiq_sys_ctx_t* sys_ctx, alut3d_lut3dCalib_t* calib)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_A3DLUT);
    RKAIQ_API_SMART_LOCK(sys_ctx);

	const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);
    AiqAlgoHandler_t* algo_handle =
		ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_A3DLUT];

	if (algo_handle) 
        return AiqAlgoHandler3dlut_getCalib((AiqAlgoHandler3dlut_t*)algo_handle, calib);
    else {
        LOGE_A3DLUT("GetCalib failed!");
        return XCAM_RETURN_ERROR_FAILED;
    }
        
}

RKAIQ_END_DECLARE
