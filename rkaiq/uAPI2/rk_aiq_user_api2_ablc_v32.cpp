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
#include "uAPI2/rk_aiq_user_api2_ablc_v32.h"

#include "RkAiqCamGroupHandleInt.h"
#include "algo_handlers/RkAiqAblcV32Handle.h"
#include "hwi/isp20/Isp20_module_dbg.h"
#include "rk_aiq_user_api2_sysctl.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_BLC_V32 && !USE_NEWSTRUCT

XCamReturn
rk_aiq_user_api2_ablcV32_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_blc_attrib_V32_t *attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ABLC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->_use_aiisp && (attr->stBlcOBAuto.enable || attr->stBlcOBManual.enable)) {
        LOGE_ABLC("Aiisp is on, BlcOB should be off\n");
        return ret;
    }

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP

        RkAiqCamGroupAblcV32HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAblcV32HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ABLC);

        if (algo_handle) {
            LOGD_ABLC("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->setAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                LOGD_ABLC("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAblcV32HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAblcV32HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ABLC);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->setAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAblcV32HandleInt* algo_handle =
            algoHandle<RkAiqAblcV32HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ABLC);
        LOGD_ABLC("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle)
            ret = algo_handle->setAttrib(attr);
    }

    LOGI_ABLC("attr=%d\n", attr->eMode);

    return ret;
}

XCamReturn
rk_aiq_user_api2_ablcV32_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_blc_attrib_V32_t *attr)
{

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP

        RkAiqCamGroupAblcV32HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAblcV32HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ABLC);
        if (algo_handle) {
            LOGD_ABLC("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->getAttrib(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;
                LOGD_ABLC("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAblcV32HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAblcV32HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ABLC);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getAttrib(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAblcV32HandleInt* algo_handle =
            algoHandle<RkAiqAblcV32HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ABLC);
        LOGD_ABLC("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->getAttrib(attr);
        }
    }


    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_user_api2_ablcV32_GetInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_blc_info_v32_t* pInfo)
{

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP

        RkAiqCamGroupAblcV32HandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAblcV32HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ABLC);
        if (algo_handle) {
            LOGD_ABLC("%s:%d !!!!!!!!!!!!!group!!!!!!!!\n", __FUNCTION__, __LINE__);
            return algo_handle->getInfo(pInfo);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;
                LOGD_ABLC("%s:%d !!!!!!!!!!!!!multi single!!!!!!!!\n", __FUNCTION__, __LINE__);
                RkAiqAblcV32HandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAblcV32HandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ABLC);
                if (singleCam_algo_handle)
                    ret = singleCam_algo_handle->getInfo(pInfo);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAblcV32HandleInt* algo_handle =
            algoHandle<RkAiqAblcV32HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ABLC);
        LOGD_ABLC("%s:%d !!!!!!!!!!!!! single!!!!!!!!\n", __FUNCTION__, __LINE__);
        if (algo_handle) {
            return algo_handle->getInfo(pInfo);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else

XCamReturn
rk_aiq_user_api2_ablcV32_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_blc_attrib_V32_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_ablcV32_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_blc_attrib_V32_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_ablcV32_GetInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_blc_info_v32_t* pInfo)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif

RKAIQ_END_DECLARE
