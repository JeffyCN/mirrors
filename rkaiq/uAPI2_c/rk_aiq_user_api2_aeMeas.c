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

#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "aiq_core_c/algo_handlers/RkAiqAeHandler.h"
#include "base/xcam_common.h"

RKAIQ_BEGIN_DECLARE

XCamReturn
rk_aiq_user_api2_aeMeas_setStatsCfg(const rk_aiq_sys_ctx_t* ctx, rk_aiq_op_mode_t opMode, aeStats_cfg_t *cfg)
{
    AiqAlgoHandlerAe_t* algo_handle =
        (AiqAlgoHandlerAe_t*)ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];
    if (algo_handle) {
        return AiqAlgoHandlerAe_setStatsApiCfg(algo_handle, opMode, cfg);
    }

    return XCAM_RETURN_NO_ERROR;
}

RKAIQ_END_DECLARE
