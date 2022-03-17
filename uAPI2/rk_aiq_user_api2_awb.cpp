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

#include "include/uAPI2/rk_aiq_user_api2_awb.h"
#include "RkAiqHandleIntV21.h"
//#include "rk_aiq_user_api2_sysctl.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn
rk_aiq_user_api2_awbV20_SetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV20_attrib_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setWbV20Attrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV20_GetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV20_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getWbV20Attrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV21_SetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV21_attrib_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setWbV21Attrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV21_GetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV21_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getWbV21Attrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_user_api2_awb_GetCCT(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_cct_t *cct)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getCct(cct);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_QueryWBInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_querry_info_t *wb_querry_info)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->queryWBInfo(wb_querry_info);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_Lock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->lock();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_Unlock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->unlock();
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetWpModeAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_op_mode_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setWbOpModeAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetWpModeAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_op_mode_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getWbOpModeAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetMwbAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_mwb_attrib_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setMwbAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetMwbAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_mwb_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getMwbAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_user_api2_awbV20_SetAwbAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV20_awb_attrib_t attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setAwbV20Attrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_user_api2_awbV20_GetAwbAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV20_awb_attrib_t *attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getAwbV20Attrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetWbGainAdjustAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setWbAwbWbGainAdjustAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetWbGainAdjustAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getWbAwbWbGainAdjustAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetWbGainOffsetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, CalibDbV2_Awb_gain_offset_cfg_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setWbAwbWbGainOffsetAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetWbGainOffsetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, CalibDbV2_Awb_gain_offset_cfg_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getWbAwbWbGainOffsetAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, CalibDbV2_Awb_Mul_Win_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->setWbAwbMultiWindowAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_GetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, CalibDbV2_Awb_Mul_Win_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAwbV21HandleInt* algo_handle =
        algoHandle<RkAiqAwbV21HandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AWB);

    if (algo_handle) {
        return algo_handle->getWbAwbMultiWindowAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

RKAIQ_END_DECLARE
