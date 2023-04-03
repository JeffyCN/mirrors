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
#include "algos/agic/rk_aiq_uapi_agic_int.h"

#include "algos/agic/rk_aiq_types_algo_agic_prvt.h"

XCamReturn rk_aiq_uapi_agic_v1_SetAttrib(RkAiqAlgoContext* ctx, const rkaiq_gic_v1_api_attr_t* attr,
                                         bool need_sync) {
    (void)(need_sync);

    if (ctx == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AgicContext_t* pAgicCtx   = (AgicContext_t*)&ctx->agicCtx;
    pAgicCtx->attr.v1.gic_en = attr->gic_en;
    pAgicCtx->attr.v1.op_mode = attr->op_mode;
    pAgicCtx->attr.v1.edge_open = attr->edge_open;
    pAgicCtx->attr.v1.noise_cut_en = attr->noise_cut_en;
    pAgicCtx->attr.v1.iso_cnt = attr->iso_cnt;
    memcpy(pAgicCtx->attr.v1.auto_params, attr->auto_params, sizeof(attr->auto_params));
    memcpy(&pAgicCtx->attr.v1.manual_param, &attr->manual_param, sizeof(attr->manual_param));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_agic_v1_GetAttrib(RkAiqAlgoContext* ctx, rkaiq_gic_v1_api_attr_t* attr) {
    if (ctx == NULL || attr == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AgicContext_t* pAgicCtx = (AgicContext_t*)&ctx->agicCtx;

    attr->gic_en = pAgicCtx->attr.v1.gic_en;
    attr->op_mode = pAgicCtx->attr.v1.op_mode;
    attr->edge_open = pAgicCtx->attr.v1.edge_open;
    attr->noise_cut_en = pAgicCtx->attr.v1.noise_cut_en;
    attr->iso_cnt = pAgicCtx->attr.v1.iso_cnt;
    memcpy(attr->auto_params, pAgicCtx->attr.v1.auto_params, sizeof(attr->auto_params));
    memcpy(&attr->manual_param, &pAgicCtx->attr.v1.manual_param, sizeof(attr->manual_param));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_agic_v2_SetAttrib(RkAiqAlgoContext* ctx, const rkaiq_gic_v2_api_attr_t* attr,
                                         bool need_sync) {
    (void)(need_sync);

    if (ctx == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AgicContext_t* pAgicCtx   = (AgicContext_t*)&ctx->agicCtx;
    pAgicCtx->attr.v2.op_mode = attr->op_mode;
    pAgicCtx->attr.v2.gic_en = attr->gic_en;
    pAgicCtx->attr.v2.iso_cnt = attr->iso_cnt;
    memcpy(pAgicCtx->attr.v2.auto_params, attr->auto_params, sizeof(attr->auto_params));
    memcpy(&pAgicCtx->attr.v2.manual_param, &attr->manual_param, sizeof(attr->manual_param));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_agic_v2_GetAttrib(RkAiqAlgoContext* ctx, rkaiq_gic_v2_api_attr_t* attr) {
    if (ctx == NULL || attr == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AgicContext_t* pAgicCtx = (AgicContext_t*)&ctx->agicCtx;

    attr->gic_en = pAgicCtx->attr.v2.gic_en;
    attr->op_mode = pAgicCtx->attr.v2.op_mode;
    attr->iso_cnt = pAgicCtx->attr.v2.iso_cnt;
    memcpy(attr->auto_params, pAgicCtx->attr.v2.auto_params, sizeof(attr->auto_params));
    memcpy(&attr->manual_param, &pAgicCtx->attr.v2.manual_param, sizeof(attr->manual_param));

    return XCAM_RETURN_NO_ERROR;
}

