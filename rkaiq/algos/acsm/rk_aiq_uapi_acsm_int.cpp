/*
 *rk_aiq_uapi_acsm_int.cpp
 *
 *  Copyright (c) 2022 Rockchip Corporation
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
#include "rk_aiq_uapi_acsm_int.h"
#include "rk_aiq_types_algo_acsm_prvt.h"

XCamReturn
rk_aiq_uapi_acsm_SetAttrib(RkAiqAlgoContext* ctx,
                           const rk_aiq_uapi_acsm_attrib_t* attr,
                           bool need_sync)
{
    if(ctx == NULL) {
        LOGE_ACSM("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AcsmContext_t* pAcsmCtx = &ctx->acsmCtx;
    pAcsmCtx->params = attr->param;
    pAcsmCtx->isReCal_ = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_acsm_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    rk_aiq_uapi_acsm_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ACSM("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AcsmContext_t* pAcsmCtx = &ctx->acsmCtx;
    attr->param = pAcsmCtx->params;
    return XCAM_RETURN_NO_ERROR;
}

