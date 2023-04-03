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

#include "ablcV32/rk_aiq_types_ablc_algo_prvt_v32.h"

void BlcParamsNewMalloc(AblcParams_V32_t* pStoreBlcPara, AblcParams_V32_t* pInputBlcPara) {
    LOGD_ABLC("%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pStoreBlcPara != NULL);
    DCT_ASSERT(pInputBlcPara != NULL);

    if (pStoreBlcPara->len != pInputBlcPara->len) {
        LOGD_ABLC("%s:enter store_Len:%d! inputLen:%d\n", __FUNCTION__, pStoreBlcPara->len,
                  pInputBlcPara->len);
        if (pStoreBlcPara->iso) free(pStoreBlcPara->iso);

        if (pStoreBlcPara->blc_b) free(pStoreBlcPara->blc_b);

        if (pStoreBlcPara->blc_gb) free(pStoreBlcPara->blc_gb);

        if (pStoreBlcPara->blc_gr) free(pStoreBlcPara->blc_gr);

        if (pStoreBlcPara->blc_r) free(pStoreBlcPara->blc_r);

        pStoreBlcPara->len    = pInputBlcPara->len;
        pStoreBlcPara->iso    = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
        pStoreBlcPara->blc_r  = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
        pStoreBlcPara->blc_gr = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
        pStoreBlcPara->blc_gb = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
        pStoreBlcPara->blc_b  = (float*)malloc(sizeof(float) * (pInputBlcPara->len));
    }

    pStoreBlcPara->enable = pInputBlcPara->enable;
    for (int i = 0; i < pInputBlcPara->len; i++) {
        pStoreBlcPara->iso[i]    = pInputBlcPara->iso[i];
        pStoreBlcPara->blc_r[i]  = pInputBlcPara->blc_r[i];
        pStoreBlcPara->blc_gr[i] = pInputBlcPara->blc_gr[i];
        pStoreBlcPara->blc_gb[i] = pInputBlcPara->blc_gb[i];
        pStoreBlcPara->blc_b[i]  = pInputBlcPara->blc_b[i];

        LOGD_ABLC("ablc iso:%f blc:%f %f %f %f\n", pStoreBlcPara->iso[i], pStoreBlcPara->blc_r[i],
                  pStoreBlcPara->blc_gr[i], pStoreBlcPara->blc_gb[i], pStoreBlcPara->blc_b[i]);
    }

    LOGD_ABLC("%s:exit!\n", __FUNCTION__);
}

void BlcOBParamsNewMalloc(AblcOBParams_V32_t* pStoreBlcObPara,
                          AblcOBParams_V32_t* pInputBlcObPara) {
    LOGD_ABLC("%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pStoreBlcObPara != NULL);
    DCT_ASSERT(pInputBlcObPara != NULL);

    if (pStoreBlcObPara->len != pInputBlcObPara->len) {
        LOGD_ABLC("%s:enter store_Len:%d! inputLen:%d\n", __FUNCTION__, pStoreBlcObPara->len,
                  pInputBlcObPara->len);
        if (pStoreBlcObPara->iso) free(pStoreBlcObPara->iso);

        if (pStoreBlcObPara->ob_offset) free(pStoreBlcObPara->ob_offset);

        if (pStoreBlcObPara->ob_predgain) free(pStoreBlcObPara->ob_predgain);

        pStoreBlcObPara->len         = pInputBlcObPara->len;
        pStoreBlcObPara->iso         = (float*)malloc(sizeof(float) * (pInputBlcObPara->len));
        pStoreBlcObPara->ob_offset   = (float*)malloc(sizeof(float) * (pInputBlcObPara->len));
        pStoreBlcObPara->ob_predgain = (float*)malloc(sizeof(float) * (pInputBlcObPara->len));
    }

    pStoreBlcObPara->enable = pInputBlcObPara->enable;
    for (int i = 0; i < pInputBlcObPara->len; i++) {
        pStoreBlcObPara->iso[i]         = pInputBlcObPara->iso[i];
        pStoreBlcObPara->ob_offset[i]   = pInputBlcObPara->ob_offset[i];
        pStoreBlcObPara->ob_predgain[i] = pInputBlcObPara->ob_predgain[i];

        LOGD_ABLC("ablc iso:%f isp_ob_offset:%f isp_ob_max%f \n", pStoreBlcObPara->iso[i],
                  pStoreBlcObPara->ob_offset[i], pStoreBlcObPara->ob_predgain[i]);
    }

    LOGD_ABLC("%s:exit!\n", __FUNCTION__);
}

XCamReturn rk_aiq_uapi_ablc_V32_SetAttrib(RkAiqAlgoContext* ctx, rk_aiq_blc_attrib_V32_t* attr,
                                         bool need_sync) {
    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)ctx;
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    pAblcCtx->eMode             = attr->eMode;
    if (attr->eMode == ABLC_V32_OP_MODE_MANUAL) {
        pAblcCtx->stBlc0Manual  = attr->stBlc0Manual;
        pAblcCtx->stBlc1Manual  = attr->stBlc1Manual;
        pAblcCtx->stBlcOBManual = attr->stBlcOBManual;
    } else if (attr->eMode == ABLC_V32_OP_MODE_AUTO) {
        BlcParamsNewMalloc(&pAblcCtx->stBlc0Params, &attr->stBlc0Auto);
        BlcParamsNewMalloc(&pAblcCtx->stBlc1Params, &attr->stBlc1Auto);
        BlcOBParamsNewMalloc(&pAblcCtx->stBlcOBParams, &attr->stBlcOBAuto);
    }

    pAblcCtx->isReCalculate |= 1;
    return ret;
}

XCamReturn rk_aiq_uapi_ablc_V32_GetAttrib(const RkAiqAlgoContext* ctx, rk_aiq_blc_attrib_V32_t* attr) {
    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)ctx;
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;

    attr->eMode = pAblcCtx->eMode;
    memcpy(&attr->stBlc0Manual, &pAblcCtx->stBlc0Manual, sizeof(attr->stBlc0Manual));
    memcpy(&attr->stBlc1Manual, &pAblcCtx->stBlc1Manual, sizeof(attr->stBlc1Manual));
    memcpy(&attr->stBlcOBManual, &pAblcCtx->stBlcOBManual, sizeof(attr->stBlcOBManual));

    BlcParamsNewMalloc(&attr->stBlc0Auto, &pAblcCtx->stBlc0Params);
    BlcParamsNewMalloc(&attr->stBlc1Auto, &pAblcCtx->stBlc1Params);
    BlcOBParamsNewMalloc(&attr->stBlcOBAuto, &pAblcCtx->stBlcOBParams);
    return ret;
}

XCamReturn rk_aiq_uapi_ablc_V32_GetProc(const RkAiqAlgoContext* ctx, AblcProc_V32_t* ProcRes) {
    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)ctx;
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;

    *ProcRes = pAblcCtx->ProcRes;
    return ret;
}

XCamReturn rk_aiq_uapi_ablc_V32_GetInfo(const RkAiqAlgoContext* ctx, rk_aiq_blc_info_v32_t* pInfo) {

    AblcContext_V32_t* pAblcCtx = (AblcContext_V32_t*)ctx;

    pInfo->iso = pAblcCtx->stExpInfo.arIso[pAblcCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pAblcCtx->stExpInfo;
    return XCAM_RETURN_NO_ERROR;
}