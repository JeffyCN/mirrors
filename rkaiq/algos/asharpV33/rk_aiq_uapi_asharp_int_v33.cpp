/*
 * rk_aiq_uapi_asharp_int_v33.cpp
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
#include "asharpV33/rk_aiq_uapi_asharp_int_v33.h"

#include "asharpV33/rk_aiq_types_asharp_algo_prvt_v33.h"

#define ASHSRPV33_STRENGTH_SLOPE_FACTOR (4.0)

XCamReturn rk_aiq_uapi_asharpV33_SetAttrib(RkAiqAlgoContext* ctx,
                                           const rk_aiq_sharp_attrib_v33_t* attr, bool need_sync) {
#if RKAIQ_HAVE_SHARP_V33
    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)ctx;

    pAsharpCtx->eMode = attr->eMode;

    LOGE_ASHARP("mode:%d \n", attr->eMode);
    if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_AUTO) {
        pAsharpCtx->stAuto = attr->stAuto;
    } else if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_MANUAL) {
        pAsharpCtx->stManual.stSelect = attr->stManual.stSelect;
    } else if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_REG_MANUAL) {
        pAsharpCtx->stManual.stFix = attr->stManual.stFix;
    }

    pAsharpCtx->isReCalculate |= 1;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_asharpV33_GetAttrib(const RkAiqAlgoContext* ctx,
                                           rk_aiq_sharp_attrib_v33_t* attr) {
#if RKAIQ_HAVE_SHARP_V33
    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)ctx;

    attr->eMode = pAsharpCtx->eMode;
    memcpy(&attr->stAuto, &pAsharpCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pAsharpCtx->stManual, sizeof(attr->stManual));
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_asharpV33Lite_SetAttrib(RkAiqAlgoContext* ctx,
                                               const rk_aiq_sharp_attrib_v33LT_t* attr,
                                               bool need_sync) {
#if RKAIQ_HAVE_SHARP_V33_LITE
    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)ctx;

    pAsharpCtx->eMode = attr->eMode;

    LOGE_ASHARP("mode:%d \n", attr->eMode);
    if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_AUTO) {
        pAsharpCtx->stAuto = attr->stAuto;
    } else if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_MANUAL) {
        pAsharpCtx->stManual.stSelect = attr->stManual.stSelect;
    } else if (pAsharpCtx->eMode == ASHARP_V33_OP_MODE_REG_MANUAL) {
        pAsharpCtx->stManual.stFix = attr->stManual.stFix;
    }

    pAsharpCtx->isReCalculate |= 1;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_asharpV33Lite_GetAttrib(const RkAiqAlgoContext* ctx,
                                               rk_aiq_sharp_attrib_v33LT_t* attr) {
#if RKAIQ_HAVE_SHARP_V33_LITE
    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)ctx;

    attr->eMode = pAsharpCtx->eMode;
    memcpy(&attr->stAuto, &pAsharpCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pAsharpCtx->stManual, sizeof(attr->stManual));
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_asharpV33_SetStrength(RkAiqAlgoContext* ctx,
        const rk_aiq_sharp_strength_v33_t* pStrength) {

    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)ctx;
    float fslope = ASHSRPV33_STRENGTH_SLOPE_FACTOR;
    float fStrength = 1.0f;
    float fPercent = 0.5f;

    fPercent = pStrength->percent;

    if (fPercent <= 0.5) {
        fStrength = fPercent / 0.5;
    } else {
        if (fPercent >= 0.999999) fPercent = 0.999999;
        fStrength = 0.5 * fslope / (1.0 - fPercent) - fslope + 1;
    }

    pAsharpCtx->stStrength = *pStrength;
    pAsharpCtx->stStrength.percent = fStrength;
    pAsharpCtx->isReCalculate |= 1;

    LOGD_ASHARP("%s:%d percent:%f fStrength:%f \n", __FUNCTION__, __LINE__, fStrength, fPercent);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_asharpV33_GetStrength(const RkAiqAlgoContext* ctx,
        rk_aiq_sharp_strength_v33_t* pStrength) {

    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)ctx;
    float fslope = ASHSRPV33_STRENGTH_SLOPE_FACTOR;
    float fStrength = 1.0f;
    float fPercent = 0.5f;

    fStrength = pAsharpCtx->stStrength.percent;

    if (fStrength <= 1) {
        fPercent = fStrength * 0.5;
    } else {
        float tmp = 1.0;
        tmp       = 1 - 0.5 * fslope / (fStrength + fslope - 1);
        if (abs(tmp - 0.999999) < 0.000001) {
            tmp = 1.0;
        }
        fPercent = tmp;
    }

    *pStrength = pAsharpCtx->stStrength;
    pStrength->percent = fPercent;

    LOGD_ASHARP("%s:%d fStrength:%f percent:%f\n", __FUNCTION__, __LINE__, fStrength, fPercent);

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn rk_aiq_uapi_asharpV33_GetInfo(const RkAiqAlgoContext* ctx,
        rk_aiq_sharp_info_v33_t* pInfo) {

    Asharp_Context_V33_t* pAsharpCtx = (Asharp_Context_V33_t*)ctx;

    pInfo->iso = pAsharpCtx->stExpInfo.arIso[pAsharpCtx->stExpInfo.hdr_mode];


    pInfo->expo_info = pAsharpCtx->stExpInfo;
    return XCAM_RETURN_NO_ERROR;
}


