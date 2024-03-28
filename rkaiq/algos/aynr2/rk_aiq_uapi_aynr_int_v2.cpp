#include "aynr2/rk_aiq_uapi_aynr_int_v2.h"
#include "aynr2/rk_aiq_types_aynr_algo_prvt_v2.h"

#if 1
#define YNRV2_LUMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_aynrV2_SetAttrib(RkAiqAlgoContext *ctx,
                             rk_aiq_ynr_attrib_v2_t *attr,
                             bool need_sync)
{

    Aynr_Context_V2_t* pCtx = (Aynr_Context_V2_t*)ctx;

    pCtx->eMode = attr->eMode;
    pCtx->stAuto = attr->stAuto;
    pCtx->stManual = attr->stManual;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_aynrV2_GetAttrib(const RkAiqAlgoContext *ctx,
                             rk_aiq_ynr_attrib_v2_t *attr)
{

    Aynr_Context_V2_t* pCtx = (Aynr_Context_V2_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
rk_aiq_uapi_ynr_SetIQPara(RkAiqAlgoContext *ctx,
                          rk_aiq_nr_IQPara_t *pPara,
                          bool need_sync)
{

    Aynr_Context_V2_t* pCtx = (Aynr_Context_V2_t*)ctx;

    if(pPara->module_bits & (1 << ANR_MODULE_BAYERNR)) {
        pCtx->stBayernrCalib = pPara->stBayernrPara;
        pCtx->isIQParaUpdate = true;
    }

    if(pPara->module_bits & (1 << ANR_MODULE_MFNR)) {
        pCtx->stMfnrCalib = pPara->stMfnrPara;
        pCtx->isIQParaUpdate = true;
    }

    if(pPara->module_bits & (1 << ANR_MODULE_UVNR)) {
        pCtx->stUvnrCalib = pPara->stUvnrPara;
        pCtx->isIQParaUpdate = true;
    }

    if(pPara->module_bits & (1 << ANR_MODULE_YNR)) {
        pCtx->stYnrCalib = pPara->stYnrPara;
        pCtx->isIQParaUpdate = true;
    }

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_ynr_GetIQPara(RkAiqAlgoContext *ctx,
                          rk_aiq_nr_IQPara_t *pPara)
{

    Aynr_Context_V2_t* pCtx = (Aynr_Context_V2_t*)ctx;

    pPara->stBayernrPara = pCtx->stBayernrCalib;
    pPara->stMfnrPara = pCtx->stMfnrCalib;
    pPara->stUvnrPara = pCtx->stUvnrCalib;
    pPara->stYnrPara = pCtx->stYnrCalib;

    return XCAM_RETURN_NO_ERROR;
}

#endif

XCamReturn
rk_aiq_uapi_aynrV2_SetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                     float fPercent)
{
    Aynr_Context_V2_t* pCtx = (Aynr_Context_V2_t*)ctx;

    float fStrength = 1.0f;
    float fMax = YNRV2_LUMA_SF_STRENGTH_MAX_PERCENT;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pCtx->fYnr_SF_Strength = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_aynrV2_GetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                     float *pPercent)
{
    Aynr_Context_V2_t* pCtx = (Aynr_Context_V2_t*)ctx;

    float fStrength = 1.0f;
    float fMax = YNRV2_LUMA_SF_STRENGTH_MAX_PERCENT;


    fStrength = pCtx->fYnr_SF_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

    return XCAM_RETURN_NO_ERROR;
}


#endif

