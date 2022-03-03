#include "aynr3/rk_aiq_uapi_aynr_int_v3.h"
#include "aynr3/rk_aiq_types_aynr_algo_prvt_v3.h"

#if 1
#define AYNRV3_CHROMA_SF_STRENGTH_SLOPE_FACTOR (5.0)


XCamReturn
rk_aiq_uapi_aynrV3_SetAttrib(RkAiqAlgoContext *ctx,
                             rk_aiq_ynr_attrib_v3_t *attr,
                             bool need_sync)
{

    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == AYNRV3_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == AYNRV3_OP_MODE_MANUAL) {
        pCtx->stManual.stSelect = attr->stManual.stSelect;
    } else if(pCtx->eMode == AYNRV3_OP_MODE_REG_MANUAL) {
        pCtx->stManual.stFix = attr->stManual.stFix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_aynrV3_GetAttrib(const RkAiqAlgoContext *ctx,
                             rk_aiq_ynr_attrib_v3_t *attr)
{

    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_aynrV3_SetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                     rk_aiq_ynr_strength_v3_t *pStrength)
{
    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;

    float fStrength = 1.0f;
    float fPercent = 0.5;
    float fslope = AYNRV3_CHROMA_SF_STRENGTH_SLOPE_FACTOR;

    fPercent = pStrength->percent;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        if(fPercent >= 0.999999)
            fPercent = 0.999999;
        fStrength = 0.5 * fslope / (1.0 - fPercent) - fslope + 1;
    }

    pCtx->stStrength = *pStrength;
    pCtx->stStrength.percent = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_aynrV3_GetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                     rk_aiq_ynr_strength_v3_t *pStrength)
{
    Aynr_Context_V3_t* pCtx = (Aynr_Context_V3_t*)ctx;

    float fStrength = 1.0f;
    float fPercent = 0.5;
    float fslope = AYNRV3_CHROMA_SF_STRENGTH_SLOPE_FACTOR;

    fStrength = pCtx->stStrength.percent;

    if(fStrength <= 1) {
        fPercent = fStrength * 0.5;
    } else {
        float tmp = 1.0;
        tmp = 1 - 0.5 * fslope / (fStrength + fslope - 1);
        if(abs(tmp - 0.999999) < 0.000001) {
            tmp = 1.0;
        }
        fPercent = tmp;
    }

    *pStrength = pCtx->stStrength;
    pStrength->percent = fPercent;

    return XCAM_RETURN_NO_ERROR;
}


#endif

