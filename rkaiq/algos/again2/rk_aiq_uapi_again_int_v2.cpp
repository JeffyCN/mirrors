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

#include "again2/rk_aiq_uapi_again_int_v2.h"
#include "again2/rk_aiq_types_again_algo_prvt_v2.h"
#include "again2/rk_aiq_again_algo_gain_v2.h"


XCamReturn
rk_aiq_uapi_againV2_SetAttrib(RkAiqAlgoContext *ctx,
                              const rk_aiq_gain_attrib_v2_t *attr,
                              bool need_sync)
{

    Again_Context_V2_t* pCtx = (Again_Context_V2_t*)ctx;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == AGAINV2_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == AGAINV2_OP_MODE_MANUAL) {
        pCtx->stManual.stSelect = attr->stManual.stSelect;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_againV2_GetAttrib(const RkAiqAlgoContext *ctx,
                              rk_aiq_gain_attrib_v2_t *attr)
{

    Again_Context_V2_t* pCtx = (Again_Context_V2_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_againV2_GetInfo(const RkAiqAlgoContext *ctx,
                            rk_aiq_gain_info_v2_t *pInfo)
{

    Again_Context_V2_t* pCtx = (Again_Context_V2_t*)ctx;

    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pCtx->stExpInfo;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapiV2_againV2_WriteInput(RkAiqAlgoContext *ctx,
                          const rk_aiq_uapiV2_again_wrtIn_attr_t &attr,
                          bool need_sync)
{

    Again_Context_V2_t* pCtx = (Again_Context_V2_t*)ctx;

    pCtx->wrt2ddr.need2wrt = attr.enable;
    strcpy(pCtx->wrt2ddr.path_name, attr.path);
    pCtx->wrt2ddr.again2ddr_mode = attr.mode;
    pCtx->wrt2ddr.buf_cnt = attr.call_cnt;

    return XCAM_RETURN_NO_ERROR;
}
