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
#include "uAPI2/rk_aiq_user_api2_awb.h"

#include "RkAiqCamGroupHandleInt.h"
#include "algo_handlers/RkAiqAwbV21Handle.h"
//#include "rk_aiq_user_api2_sysctl.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif


XCamReturn
rk_aiq_user_api2_awbV21_SetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV21_attrib_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setWbV21Attrib(attr);
    } else {
        XCamReturn ret                            = XCAM_RETURN_NO_ERROR;
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle) {
                ret = singleCam_algo_handle->setWbV21Attrib(attr);
                if (ret != XCAM_RETURN_NO_ERROR) LOGE("%s returned: %d", __FUNCTION__, ret);
            }
        }
        return ret;
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbV21Attrib(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV21_GetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV21_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getWbV21Attrib(attr);
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->getWbV21Attrib(attr);
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbV21Attrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV30_SetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV30_attrib_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setWbV21Attrib((rk_aiq_uapiV2_wbV21_attrib_t&)attr);
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->setWbV21Attrib((rk_aiq_uapiV2_wbV21_attrib_t&)attr);
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbV21Attrib((rk_aiq_uapiV2_wbV21_attrib_t&)attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV30_GetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV30_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getWbV21Attrib((rk_aiq_uapiV2_wbV21_attrib_t*)attr);
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->getWbV21Attrib((rk_aiq_uapiV2_wbV21_attrib_t*)attr);
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbV21Attrib((rk_aiq_uapiV2_wbV21_attrib_t*)attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetCCT(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_cct_t *cct)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getCct(cct);
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->getCct(cct);
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getCct(cct);
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
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->queryWBInfo(wb_querry_info);
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->queryWBInfo(wb_querry_info);
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->queryWBInfo(wb_querry_info);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_Lock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->lock();
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->lock();
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->lock();
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_Unlock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->unlock();
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->unlock();
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->unlock();
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetWpModeAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_opMode_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbOpModeAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->setWbOpModeAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbOpModeAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetWpModeAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_opMode_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbOpModeAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->getWbOpModeAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbOpModeAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetMwbAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_mwb_attrib_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setMwbAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->setMwbAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setMwbAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetMwbAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_mwb_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getMwbAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->getMwbAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getMwbAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_user_api2_awb_SetWbGainAdjustAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbAwbWbGainAdjustAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->setWbAwbWbGainAdjustAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbAwbWbGainAdjustAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetWbGainAdjustAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbAwbWbGainAdjustAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->getWbAwbWbGainAdjustAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbAwbWbGainAdjustAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetWbGainOffsetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainOffset_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbAwbWbGainOffsetAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->setWbAwbWbGainOffsetAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbAwbWbGainOffsetAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetWbGainOffsetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainOffset_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbAwbWbGainOffsetAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->getWbAwbWbGainOffsetAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbAwbWbGainOffsetAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_mulWindow_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbAwbMultiWindowAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->setWbAwbMultiWindowAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->setWbAwbMultiWindowAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_mulWindow_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
    #ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAwbHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbAwbMultiWindowAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAwbV21HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
                if (singleCam_algo_handle)
                    return singleCam_algo_handle->getWbAwbMultiWindowAttrib(attr);
            }
        }
    #else
        return XCAM_RETURN_ERROR_FAILED;
    #endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getWbAwbMultiWindowAttrib(attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_user_api2_awbV30_getAlgoStat(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_stat_res_full_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getAlgoStat((rk_tool_awb_stat_res_full_t*)attr);
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->getAlgoStat((rk_tool_awb_stat_res_full_t*)attr);
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getAlgoStat((rk_tool_awb_stat_res_full_t*)attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV30_getStrategyResult(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_strategy_result_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getStrategyResult((rk_tool_awb_strategy_result_t*)attr);
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->getStrategyResult((rk_tool_awb_strategy_result_t*)attr);
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getStrategyResult((rk_tool_awb_strategy_result_t*)attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV30_getAlgoSta(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_stat_res_full_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    RkAiqCamGroupAwbHandleInt* algo_handle =
        camgroupAlgoHandle<RkAiqCamGroupAwbHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getAlgoStat((rk_tool_awb_stat_res_full_t*)attr);
    } else {
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
            if (!camCtx)
                continue;

            RkAiqAwbV21HandleInt* singleCam_algo_handle =
                algoHandle<RkAiqAwbV21HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AWB);
            if (singleCam_algo_handle)
                return singleCam_algo_handle->getAlgoStat((rk_tool_awb_stat_res_full_t*)attr);
        }
    }
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAwbV21HandleInt* algo_handle =
            algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

        if (algo_handle) {
            return algo_handle->getAlgoStat((rk_tool_awb_stat_res_full_t*)attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

RKAIQ_END_DECLARE
