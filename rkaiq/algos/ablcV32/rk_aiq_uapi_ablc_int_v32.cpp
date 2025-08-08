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
#include "rk_aiq_uapi_ablc_int_v32.h"
#include "rk_aiq_ablc_algo_v32.h"

#include "ablcV32/rk_aiq_types_ablc_algo_prvt_v32.h"

XCamReturn rk_aiq_uapi_ablc_V32_SetAttrib(RkAiqAlgoContext* ctx, rk_aiq_blc_attrib_V32_t* attr,
    bool need_sync) {
    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    pAblcCtx->eMode = attr->eMode;
    if (attr->eMode == ABLC_V32_OP_MODE_MANUAL) {
        pAblcCtx->stBlc0Manual = attr->stBlc0Manual;
        pAblcCtx->stBlc1Manual = attr->stBlc1Manual;
        pAblcCtx->stBlcOBManual = attr->stBlcOBManual;
    }
    else if (attr->eMode == ABLC_V32_OP_MODE_AUTO) {
        memcpy(&pAblcCtx->stBlc0Params, &attr->stBlc0Auto, sizeof(pAblcCtx->stBlc0Params));
        memcpy(&pAblcCtx->stBlc1Params, &attr->stBlc1Auto, sizeof(pAblcCtx->stBlc1Params));
        memcpy(&pAblcCtx->stBlcOBParams, &attr->stBlcOBAuto, sizeof(pAblcCtx->stBlcOBParams));
    }

    pAblcCtx->isReCalculate |= 1;
    return ret;
}

XCamReturn rk_aiq_uapi_ablc_V32_GetAttrib(const RkAiqAlgoContext* ctx, rk_aiq_blc_attrib_V32_t* attr) {
    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    attr->eMode = pAblcCtx->eMode;
    memcpy(&attr->stBlc0Manual, &pAblcCtx->stBlc0Manual, sizeof(attr->stBlc0Manual));
    memcpy(&attr->stBlc1Manual, &pAblcCtx->stBlc1Manual, sizeof(attr->stBlc1Manual));
    memcpy(&attr->stBlcOBManual, &pAblcCtx->stBlcOBManual, sizeof(attr->stBlcOBManual));

    memcpy(&attr->stBlc0Auto, &pAblcCtx->stBlc0Params, sizeof(attr->stBlc0Auto));
    memcpy(&attr->stBlc1Auto, &pAblcCtx->stBlc1Params, sizeof(attr->stBlc1Auto));
    memcpy(&attr->stBlcOBAuto, &pAblcCtx->stBlcOBParams, sizeof(attr->stBlcOBAuto));
    return ret;
}

XCamReturn rk_aiq_uapi_ablc_V32_GetProc(const RkAiqAlgoContext* ctx, AblcProc_V32_t* ProcRes) {
    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    Ablc_GetProcResult_V32(pAblcCtx, ProcRes);
    return ret;
}

XCamReturn rk_aiq_uapi_ablc_V32_GetInfo(const RkAiqAlgoContext* ctx, rk_aiq_blc_info_v32_t* pInfo) {

    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)ctx;

    pInfo->iso = pAblcCtx->stExpInfo.arIso[pAblcCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pAblcCtx->stExpInfo;
    return XCAM_RETURN_NO_ERROR;
}