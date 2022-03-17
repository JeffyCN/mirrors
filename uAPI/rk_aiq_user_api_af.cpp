/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#include "rk_aiq_user_api_af.h"
#include "RkAiqHandleInt.h"
#include "rk_aiq_user_api_sysctl.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn
rk_aiq_user_api_af_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AF);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->setAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->getAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_Lock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->lock();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_Unlock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->unlock();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_Oneshot(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->Oneshot();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_ManualTriger(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->ManualTriger();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_Tracking(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->Tracking();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_SetZoomPos(const rk_aiq_sys_ctx_t* sys_ctx, int zoom_pos)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->setZoomPos(zoom_pos);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_GetZoomPos(const rk_aiq_sys_ctx_t* sys_ctx, int* zoom_pos)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->getZoomPosition(*zoom_pos);

    return ret;
}

XCamReturn
rk_aiq_user_api_af_SetVcmCfg(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lens_vcmcfg* cfg)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->setLensVcmCfg(*cfg);

    return ret;
}

XCamReturn
rk_aiq_user_api_af_GetVcmCfg(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lens_vcmcfg* cfg)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->getLensVcmCfg(*cfg);

    return ret;
}

XCamReturn
rk_aiq_user_api_af_GetSearchPath(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_sec_path_t* path)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->GetSearchPath(path);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_GetSearchResult(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_result_t* result)
{
    RkAiqAfHandleInt* algo_handle =
        algoHandle<RkAiqAfHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AF);

    if (algo_handle) {
        return algo_handle->GetSearchResult(result);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_af_GetZoomRange(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_af_zoomrange* range)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t calibdbv2_ctx =
        RkAiqCalibDbV2::toDefaultCalibDb(sys_ctx->_calibDbProj);
    CalibDbV2_AF_t *af =
        (CalibDbV2_AF_t*)(CALIBDBV2_GET_MODULE_PTR((&calibdbv2_ctx), af));
    CalibDbV2_Af_ZoomFocusTbl_t *zoomfocus_tbl = (CalibDbV2_Af_ZoomFocusTbl_t*)(&af->TuningPara.zoomfocus_tbl);
    int tbl_len = zoomfocus_tbl->tbl_len;

    range->min_fl = zoomfocus_tbl->focal_length[0];
    range->max_fl = zoomfocus_tbl->focal_length[tbl_len - 1];
    range->min_pos = zoomfocus_tbl->zoom_pos[0];
    range->max_pos = zoomfocus_tbl->zoom_pos[tbl_len - 1];

    return ret;
}

XCamReturn
rk_aiq_user_api_af_FocusCorrection(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->FocusCorrection();

    return ret;
}

XCamReturn
rk_aiq_user_api_af_ZoomCorrection(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = sys_ctx->_camHw->ZoomCorrection();

    return ret;
}

RKAIQ_END_DECLARE
