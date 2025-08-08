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
#include "uAPI2/rk_aiq_user_api2_awb_v3.h"

//#include "RkAiqCamGroupHandleInt.h"
#include "aiq_core_c/algo_handlers/RkAiqAwbHandler.h"
#include "aiq_core_c/algo_camgroup_handlers/RkAiqCamGroupAwbHandle.h"
#include "awb/rk_aiq_uapiv3_awb_int.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn
rk_aiq_user_api2_awb_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_api_attrib_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if ISP_HW_V35
    GlobalParamsManager_checkStatsrc(&sys_ctx->_rkAiqManager->mGlobalParamsManager, attr, RESULT_TYPE_AWB_PARAM);
#endif

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_api_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCtrl_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setWbGainCtrlAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setWbGainCtrlAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setWbGainCtrlAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCtrl_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getWbGainCtrlAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getWbGainCtrlAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getWbGainCtrlAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetAwbStatsAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_Stats_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if ISP_HW_V35
    awb_api_attrib_t awb_api_attr;
    awb_api_attr.awbStats = *attr;
    GlobalParamsManager_checkStatsrc(&sys_ctx->_rkAiqManager->mGlobalParamsManager, &awb_api_attr, RESULT_TYPE_AWB_PARAM);
#endif

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setAwbStatsAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setAwbStatsAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setAwbStatsAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetAwbStatsAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_Stats_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getAwbStatsAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getAwbStatsAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getAwbStatsAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetAwbGnCalcStepAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCalcStep_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setAwbGnCalcStepAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setAwbGnCalcStepAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setAwbGnCalcStepAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetAwbGnCalcStepAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCalcStep_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getAwbGnCalcStepAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getAwbGnCalcStepAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getAwbGnCalcStepAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetAwbGnCalcOthAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCalcOth_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setAwbGnCalcOthAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setAwbGnCalcOthAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setAwbGnCalcOthAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetAwbGnCalcOthAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCalcOth_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getAwbGnCalcOthAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getAwbGnCalcOthAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getAwbGnCalcOthAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_user_api2_awb_QueryWBInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_querry_info_t *wb_querry_info)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];


        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_queryWBInfo(algo_handle, wb_querry_info);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_queryWBInfo(algo_handle, wb_querry_info);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_queryWBInfo(algo_handle, wb_querry_info);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_Lock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];


        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_lock(algo_handle);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_lock(algo_handle);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_lock(algo_handle);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_Unlock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];


        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_unlock(algo_handle);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_unlock(algo_handle);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_unlock(algo_handle);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_getStrategyResult(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_strategy_result_t *attr)
{

    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getStrategyResult(algo_handle, (rk_tool_awb_strategy_result_t*)attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getStrategyResult(algo_handle, (rk_tool_awb_strategy_result_t*)attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getStrategyResult(algo_handle, (rk_tool_awb_strategy_result_t*)attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_getAlgoSta(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_stat_res_full_t *attr)
{

    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getAlgoStat(algo_handle, (rk_tool_awb_stat_res_full_t*)attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getAlgoStat(algo_handle, (rk_tool_awb_stat_res_full_t*)attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getAlgoStat(algo_handle, (rk_tool_awb_stat_res_full_t*)attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_user_api2_awb_WriteAwbIn(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_awb_wrtIn_attr_t attr)
{
#if RKAIQ_HAVE_AWB_V39

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGW_AWB("%s:bypass",__FUNCTION__);
        return XCAM_RETURN_NO_ERROR;//TODO
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_writeAwbIn(algo_handle, attr);
        }
        return XCAM_RETURN_NO_ERROR;
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awb_SetFFWbgainAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_awb_ffwbgain_attr_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setFFWbgainAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_user_api2_awb_IqMap2Main(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att)
{
#if RKAIQ_HAVE_AWB_V39
    //rk_aiq_uapiV2_awb_freeConvertLut(cct_lut_cfg);

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        //TODO
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_awbIqMap2Main(algo_handle, att);
        }
    }
    return ret;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awb_setAwbPreWbgain(const rk_aiq_sys_ctx_t* sys_ctx,  const float att[4])
{
#if RKAIQ_HAVE_AWB_V39

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        //TODO
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setAwbPreWbgain(algo_handle, att);
        }
    }
    return ret;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}


XCamReturn
rk_aiq_user_api2_awb_SetNNres(const rk_aiq_sys_ctx_t* sys_ctx, awb_ai_res_t *attr)
{
#if RKAIQ_HAVE_AWB_V39

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGW_AWB("%s:bypass",__FUNCTION__);
        return XCAM_RETURN_NO_ERROR;//TODO
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_SetNNres(algo_handle, attr);
        }
        return XCAM_RETURN_NO_ERROR;
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}