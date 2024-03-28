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
#include "uAPI2/rk_aiq_user_api2_acnr_v30.h"

#include "RkAiqCamGroupHandleInt.h"
#include "algo_handlers/RkAiqAcnrV30Handle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if ((RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)) && (USE_NEWSTRUCT == 0)

XCamReturn
rk_aiq_user_api2_acnrV30_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_cnr_attrib_v30_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ACNR);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP

        RkAiqCamGroupAcnrV30HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);

        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->setAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAcnrV30HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcnrV30HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->setAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        RkAiqAcnrV30HandleInt* algo_handle =
            algoHandle<RkAiqAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->setAttrib(attr);
        }
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_acnrV30_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_cnr_attrib_v30_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAcnrV30HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);

        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->getAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;
                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAcnrV30HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcnrV30HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        RkAiqAcnrV30HandleInt* algo_handle =
            algoHandle<RkAiqAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!!single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }


    return ret;
}

XCamReturn
rk_aiq_user_api2_acnrV30_SetStrength(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_cnr_strength_v30_t *pStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP

        RkAiqCamGroupAcnrV30HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);

        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->setStrength(pStrength);
        } else {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAcnrV30HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcnrV30HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->setStrength(pStrength);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        LOGD_ANR("%s:%d !!!!!!!!!!!!!single!!!!!!!!\n", __FUNCTION__, __LINE__);
        RkAiqAcnrV30HandleInt* algo_handle =
            algoHandle<RkAiqAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);

        if (algo_handle) {
            return algo_handle->setStrength(pStrength);
        }
    }

    return ret;
}


XCamReturn
rk_aiq_user_api2_acnrV30_GetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_cnr_strength_v30_t *pStrength)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);


    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP

        RkAiqCamGroupAcnrV30HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);

        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->getStrength(pStrength);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;
                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAcnrV30HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcnrV30HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getStrength(pStrength);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        RkAiqAcnrV30HandleInt* algo_handle =
            algoHandle<RkAiqAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!!single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->getStrength(pStrength);
        }
    }

    return ret;

}



XCamReturn
rk_aiq_user_api2_acnrV30_GetInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_cnr_info_v30_t *pInfo)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);


    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP

        RkAiqCamGroupAcnrV30HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);

        if (algo_handle) {
            LOGD_ANR("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->getInfo(pInfo);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;
                LOGD_ANR("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAcnrV30HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAcnrV30HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ACNR);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getInfo(pInfo);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        RkAiqAcnrV30HandleInt* algo_handle =
            algoHandle<RkAiqAcnrV30HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ACNR);
        LOGD_ANR("%s:%d !!!!!!!!!!!!!single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->getInfo(pInfo);
        }
    }

    return ret;

}


#else
XCamReturn
rk_aiq_user_api2_acnrV30_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_cnr_attrib_v30_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}


XCamReturn
rk_aiq_user_api2_acnrV30_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_cnr_attrib_v30_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}


XCamReturn
rk_aiq_user_api2_acnrV30_SetStrength(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_cnr_strength_v30_t *pStrength)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}


XCamReturn
rk_aiq_user_api2_acnrV30_GetStrength(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_cnr_strength_v30_t *pStrength)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_acnrV30_GetInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_cnr_info_v30_t *pInfo)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

RKAIQ_END_DECLARE
