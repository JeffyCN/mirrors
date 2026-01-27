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

#include "newStruct/enh/include/enh_algo_api.h"
#include "aiq_core_c/algo_handlers/RkAiqEnhHandler.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "uAPI2_c/rk_aiq_user_api2_common.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

/*
*****************************
*
* Desc: set/get manual dehaze strength
*     this function is active for dehaze is manual mode
* Argument:
*   level: [0, 100]
*
*****************************
*/
static XCamReturn
_enhance_SetMDehazeStrth(const rk_aiq_sys_ctx_t* sys_ctx, aenh_strength_t *strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_ENHANCE
    AiqEnhHandler_t* algo_handle =
        (AiqEnhHandler_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AENH];
    if (algo_handle) {
        return AiqEnhHandler_setStrength(algo_handle, strg);
    }
#else
	ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return ret;
}

static XCamReturn
_enhance_GetMDehazeStrth(const rk_aiq_sys_ctx_t* sys_ctx, aenh_strength_t *strg)
{
	XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_ENHANCE
    AiqEnhHandler_t* algo_handle =
        (AiqEnhHandler_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AENH];
    if (algo_handle) {
        return AiqEnhHandler_getStrength(algo_handle, strg);
    }
#else
	ret = XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return ret;
}

XCamReturn rk_aiq_user_api2_enh_SetEnhanceStrth(const rk_aiq_sys_ctx_t* sys_ctx, aenh_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AENH);
    RKAIQ_API_SMART_LOCK(sys_ctx);
	rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);
    for (int i = 0; i < ctx_array.num; i++) {
        ret = _enhance_SetMDehazeStrth(ctx_array.ctx[i], strg);
    }
    return ret;
}

XCamReturn rk_aiq_user_api2_enh_GetEnhanceStrth(const rk_aiq_sys_ctx_t* sys_ctx, aenh_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AENH);
    RKAIQ_API_SMART_LOCK(sys_ctx);
	const rk_aiq_sys_ctx_t* _ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);
    return _enhance_GetMDehazeStrth(_ctx, strg);
}

XCamReturn
rk_aiq_user_api2_enh_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, enh_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AENH);
    RKAIQ_API_SMART_LOCK(sys_ctx);

	rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);

	int type = RESULT_TYPE_ENH_PARAM;
	int man_param_size = sizeof(enh_param_t);
	int aut_param_size = sizeof(enh_param_auto_t);

	for (int i = 0; i < ctx_array.num; i++) {
		if (attr->opMode == RK_AIQ_OP_MODE_MANUAL || attr->opMode == RK_AIQ_OP_MODE_AUTO) {
			ret = rk_aiq_user_api2_common_processParams(ctx_array.ctx[i], true,
				&attr->opMode, &attr->en, &attr->bypass,
				type, man_param_size, &attr->stMan, aut_param_size, &attr->stAuto);
			if (ret != XCAM_RETURN_NO_ERROR) {
                LOGE("ynr, cnr, sharp and enh en should be on or off in the same time, "
                     "please use rk_aiq_uapi2_sysctl_setModuleEn to set them");
            }
		}
		else {
			ret = XCAM_RETURN_ERROR_FAILED;
			LOGE_ADEHAZE("wrong mode %d !", attr->opMode);
		}
	}

    return ret;
}

XCamReturn
rk_aiq_user_api2_enh_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, enh_api_attrib_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AENH);
    RKAIQ_API_SMART_LOCK(sys_ctx);

	const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);

	int type = RESULT_TYPE_ENH_PARAM;
	int man_param_size = sizeof(enh_param_t);
	int aut_param_size = sizeof(enh_param_auto_t);

    return rk_aiq_user_api2_common_processParams(ctx, false,
				&attr->opMode, &attr->en, &attr->bypass,
				type, man_param_size, &attr->stMan, aut_param_size,  &attr->stAuto);
}

XCamReturn
rk_aiq_user_api2_enh_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, enh_status_t* status)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AENH);
    RKAIQ_API_SMART_LOCK(sys_ctx);

	const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);

	AiqAlgoHandler_t* algo_handle =
		ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AENH];

	if (algo_handle) {
		ret = AiqAlgoHandler_queryStatus_common(algo_handle,
				&status->opMode, &status->en, &status->bypass, &status->stMan);
	}

	return ret;
}

RKAIQ_END_DECLARE
