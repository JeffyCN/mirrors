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
#include "uAPI2/rk_aiq_user_api2_abayertnr_v2.h"

#include "RkAiqCamGroupHandleInt.h"
#include "algo_handlers/RkAiqAbayertnrV2Handle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_BAYERTNR_V2

XCamReturn
rk_aiq_user_api2_abayertnrV2_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_attrib_v2_t* attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AMFNR);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAbayertnrV2HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->setAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAbayertnrV2HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAbayertnrV2HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMFNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->setAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAbayertnrV2HandleInt* algo_handle =
            algoHandle<RkAiqAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_abayertnrV2_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_attrib_v2_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAbayertnrV2HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->getAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAbayertnrV2HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAbayertnrV2HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMFNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAbayertnrV2HandleInt* algo_handle =
            algoHandle<RkAiqAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_user_api2_abayertnrV2_SetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_strength_v2_t *pStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAbayertnrV2HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->setStrength(pStrength);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAbayertnrV2HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAbayertnrV2HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMFNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->setStrength(pStrength);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAbayertnrV2HandleInt* algo_handle =
            algoHandle<RkAiqAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->setStrength(pStrength);
        }
    }

    return ret;
}




XCamReturn
rk_aiq_user_api2_abayertnrV2_GetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_strength_v2_t *pStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAbayertnrV2HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->getStrength(pStrength);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAbayertnrV2HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAbayertnrV2HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMFNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getStrength(pStrength);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAbayertnrV2HandleInt* algo_handle =
            algoHandle<RkAiqAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->getStrength(pStrength);
        }
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_abayertnrV2_GetInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_info_v2_t *pInfo)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAbayertnrV2HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->getInfo(pInfo);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAbayertnrV2HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAbayertnrV2HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AMFNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getInfo(pInfo);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAbayertnrV2HandleInt* algo_handle =
            algoHandle<RkAiqAbayertnrV2HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AMFNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->getInfo(pInfo);
        }
    }

    return ret;
}

#else

XCamReturn
rk_aiq_user_api2_abayertnrV2_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_attrib_v2_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_abayertnrV2_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_attrib_v2_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_abayertnrV2_SetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_strength_v2_t *pStrength)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_abayertnrV2_GetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_strength_v2_t *pStrength)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_abayertnrV2_GetInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_bayertnr_info_v2_t *pInfo)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif
RKAIQ_END_DECLARE
