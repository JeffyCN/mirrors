#include "asharp4/rk_aiq_uapi_asharp_int_v4.h"
#include "asharp4/rk_aiq_types_asharp_algo_prvt_v4.h"

#define ASHSRPV4_STRENGTH_MAX_PERCENT (100.0)

XCamReturn
rk_aiq_uapi_asharpV4_SetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_sharp_attrib_v4_t *attr,
                               bool need_sync)
{

    Asharp_Context_V4_t* pAsharpCtx = (Asharp_Context_V4_t*)ctx;

    pAsharpCtx->eMode = attr->eMode;

    if(pAsharpCtx->eMode == ASHARP4_OP_MODE_AUTO) {
        pAsharpCtx->stAuto = attr->stAuto;
    } else if(pAsharpCtx->eMode == ASHARP4_OP_MODE_MANUAL) {
        pAsharpCtx->stManual.stSelect = attr->stManual.stSelect;
    } else if(pAsharpCtx->eMode == ASHARP4_OP_MODE_REG_MANUAL) {
        pAsharpCtx->stManual.stFix = attr->stManual.stFix;
    }

    pAsharpCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharpV4_GetAttrib(const RkAiqAlgoContext *ctx,
                               rk_aiq_sharp_attrib_v4_t *attr)
{

    Asharp_Context_V4_t* pAsharpCtx = (Asharp_Context_V4_t*)ctx;

    attr->eMode = pAsharpCtx->eMode;
    memcpy(&attr->stAuto, &pAsharpCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pAsharpCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharpV4_SetStrength(const RkAiqAlgoContext *ctx,
                                 rk_aiq_sharp_strength_v4_t *pStrength)
{

    Asharp_Context_V4_t* pAsharpCtx = (Asharp_Context_V4_t*)ctx;
    float fMax = ASHSRPV4_STRENGTH_MAX_PERCENT;
    float fStrength = 1.0;
    float fPercent = 0.5;

    fPercent = pStrength->percent;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        if(fPercent >= 0.999999)
            fPercent = 0.999999;
        fStrength = 0.5 / (1.0 - fPercent);
    }

    pAsharpCtx->stStrength = *pStrength;
    pAsharpCtx->stStrength.percent = fStrength;
    pAsharpCtx->isReCalculate |= 1;

    LOGD_ASHARP("%s:%d percent:%f fStrength:%f \n",
                __FUNCTION__, __LINE__,
                fStrength, fPercent);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharpV4_GetStrength(const RkAiqAlgoContext *ctx,
                                 rk_aiq_sharp_strength_v4_t *pStrength)
{

    Asharp_Context_V4_t* pAsharpCtx = (Asharp_Context_V4_t*)ctx;
    float fMax = ASHSRPV4_STRENGTH_MAX_PERCENT;
    float fStrength = 1.0;
    float fPercent = 0.5;

    fStrength = pAsharpCtx->stStrength.percent;

    if(fStrength <= 1) {
        fPercent = fStrength * 0.5;
    } else {
        float tmp = 1.0;
        tmp = 1 - 0.5 / fStrength;
        if(abs(tmp - 0.999999) < 0.000001) {
            tmp = 1.0;
        }
        fPercent = tmp;
    }

    *pStrength = pAsharpCtx->stStrength;
    pStrength->percent = fPercent;

    LOGD_ASHARP("%s:%d fStrength:%f percent:%f\n",
                __FUNCTION__, __LINE__,
                fStrength, fPercent);

    return XCAM_RETURN_NO_ERROR;
}


