/*
 *rk_aiq_uapi_acgc_int.cpp
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
#include "rk_aiq_uapi_acgc_int.h"

#include "rk_aiq_types_algo_acgc_prvt.h"

XCamReturn
rk_aiq_uapi_acgc_SetAttrib(RkAiqAlgoContext* ctx,
                           const rk_aiq_uapi_acgc_attrib_t* attr,
                           bool need_sync)
{
    if (ctx == NULL) {
        LOGE_ACGC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AcgcContext_t* pAcgcCtx = &ctx->acgcCtx;
    pAcgcCtx->params        = attr->param;
    pAcgcCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_acgc_GetAttrib(const RkAiqAlgoContext* ctx,
                           rk_aiq_uapi_acgc_attrib_t* attr)
{
    if (ctx == NULL || attr == NULL) {
        LOGE_ACGC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AcgcContext_t* pAcgcCtx = (AcgcContext_t*)&ctx->acgcCtx;
    attr->param             = pAcgcCtx->params;
    return XCAM_RETURN_NO_ERROR;
}
