#include "acnrV31/rk_aiq_uapi_acnr_int_v31.h"
#include "acnrV31/rk_aiq_types_acnr_algo_prvt_v31.h"

#if 1
#define ACNRV31_CHROMA_SF_STRENGTH_SLOPE_FACTOR (5.0)


XCamReturn
rk_aiq_uapi_acnrV31_SetAttrib(RkAiqAlgoContext *ctx,
                              const rk_aiq_cnr_attrib_v31_t *attr,
                              bool need_sync)
{

    Acnr_Context_V31_t* pCtx = (Acnr_Context_V31_t*)ctx;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == ACNRV31_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == ACNRV31_OP_MODE_MANUAL) {
        pCtx->stManual.stSelect = attr->stManual.stSelect;
    } else if(pCtx->eMode == ACNRV31_OP_MODE_REG_MANUAL) {
        pCtx->stManual.stFix = attr->stManual.stFix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_acnrV31_GetAttrib(const RkAiqAlgoContext *ctx,
                              rk_aiq_cnr_attrib_v31_t *attr)
{

    Acnr_Context_V31_t* pCtx = (Acnr_Context_V31_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_acnrV31_SetChromaSFStrength(const RkAiqAlgoContext *ctx,
                                        const rk_aiq_cnr_strength_v31_t* pStrength)
{
    Acnr_Context_V31_t* pCtx = (Acnr_Context_V31_t*)ctx;

    float fStrength = 1.0f;
    float fslope = ACNRV31_CHROMA_SF_STRENGTH_SLOPE_FACTOR;
    float fPercent = 0.5f;

    fPercent = pStrength->percent;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        if(fPercent >= 0.999999)
            fPercent = 0.999999;
        fStrength = 0.5 * fslope / (1.0 - fPercent) - fslope  + 1;
    }

    pCtx->stStrength = *pStrength;
    pCtx->stStrength.percent = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_acnrV31_GetChromaSFStrength(const RkAiqAlgoContext *ctx,
                                        rk_aiq_cnr_strength_v31_t* pStrength)
{
    Acnr_Context_V31_t* pCtx = (Acnr_Context_V31_t*)ctx;

    float fStrength = 1.0f;
    float fslope = ACNRV31_CHROMA_SF_STRENGTH_SLOPE_FACTOR;
    float fPercent = 0.5;

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


XCamReturn
rk_aiq_uapi_acnrV31_GetInfo(const RkAiqAlgoContext* ctx,
                            rk_aiq_cnr_info_v31_t* pInfo) {

    Acnr_Context_V31_t* pCtx = (Acnr_Context_V31_t*)ctx;

    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];


    pInfo->expo_info = pCtx->stExpInfo;
    return XCAM_RETURN_NO_ERROR;
}


#endif

