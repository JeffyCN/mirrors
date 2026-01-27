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

#include "rk_aiq_user_api2_debug.h"
#include "RkAiqManager.h"
#ifdef RK_SIMULATOR_HW
#include "simulator/CamHwSimulator.h"
#else
#include "isp20/CamHwIsp20.h"
#endif

using namespace RkCam;
using namespace XCam;

typedef struct rk_aiq_sys_ctx_s {
    const char* _sensor_entity_name;
    SmartPtr<RkAiqManager> _rkAiqManager;
    SmartPtr<ICamHw> _camHw;
    SmartPtr<RkAiqCore> _analyzer;
#ifdef ISP_HW_V20
    SmartPtr<RkLumaCore> _lumaAnalyzer;
#endif
#ifdef RKAIQ_ENABLE_PARSER_V1
    CamCalibDbContext_t *_calibDb;
#endif
} rk_aiq_sys_ctx_t;

RKAIQ_BEGIN_DECLARE

XCamReturn
rk_aiq_uapi2_debug_captureRawYuvSync(const rk_aiq_sys_ctx_t* ctx, capture_raw_t type)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = ctx->_camHw->capture_raw_ctl(type);
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_debug_captureRawSync(const rk_aiq_sys_ctx_t* ctx, capture_raw_t type,
                                 int count, const char* capture_dir,
                                 char* output_dir)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = ctx->_camHw->capture_raw_ctl(type, count, capture_dir, output_dir);
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn
rk_aiq_uapi2_debug_captureRawNotify(const rk_aiq_sys_ctx_t* ctx)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = ctx->_camHw->notify_capture_raw();
    EXIT_XCORE_FUNCTION();

    return ret;
}

RKAIQ_END_DECLARE
