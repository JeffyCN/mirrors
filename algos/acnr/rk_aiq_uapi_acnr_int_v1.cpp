#include "acnr/rk_aiq_uapi_acnr_int_v1.h"
#include "acnr/rk_aiq_types_acnr_algo_prvt_v1.h"

#if 1
#define ACNRV1_CHROMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_acnrV1_SetAttrib(RkAiqAlgoContext *ctx,
                             rk_aiq_cnr_attrib_v1_t *attr,
                             bool need_sync)
{

    Acnr_Context_V1_t* pCtx = (Acnr_Context_V1_t*)ctx;

    pCtx->eMode = attr->eMode;
    pCtx->stAuto = attr->stAuto;
    pCtx->stManual = attr->stManual;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_acnrV1_GetAttrib(const RkAiqAlgoContext *ctx,
                             rk_aiq_cnr_attrib_v1_t *attr)
{

    Acnr_Context_V1_t* pCtx = (Acnr_Context_V1_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
rk_aiq_uapi_acnrV1_SetIQPara(RkAiqAlgoContext *ctx,
                             rk_aiq_nr_IQPara_t *pPara,
                             bool need_sync)
{

    Acnr_Context_V1_t* pCtx = (Acnr_Context_V1_t*)ctx;

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
rk_aiq_uapi_acnrV1_GetIQPara(RkAiqAlgoContext *ctx,
                             rk_aiq_nr_IQPara_t *pPara)
{

    Acnr_Context_V1_t* pCtx = (Acnr_Context_V1_t*)ctx;

    pPara->stBayernrPara = pCtx->stBayernrCalib;
    pPara->stMfnrPara = pCtx->stMfnrCalib;
    pPara->stUvnrPara = pCtx->stUvnrCalib;
    pPara->stYnrPara = pCtx->stYnrCalib;

    return XCAM_RETURN_NO_ERROR;
}
#endif


XCamReturn
rk_aiq_uapi_acnrV1_SetChromaSFStrength(const RkAiqAlgoContext *ctx,
                                       float fPercent)
{
    Acnr_Context_V1_t* pCtx = (Acnr_Context_V1_t*)ctx;

    float fStrength = 1.0f;
    float fMax = ACNRV1_CHROMA_SF_STRENGTH_MAX_PERCENT;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pCtx->fCnr_SF_Strength = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_acnrV1_GetChromaSFStrength(const RkAiqAlgoContext *ctx,
                                       float *pPercent)
{
    Acnr_Context_V1_t* pCtx = (Acnr_Context_V1_t*)ctx;

    float fStrength = 1.0f;
    float fMax = ACNRV1_CHROMA_SF_STRENGTH_MAX_PERCENT;

    fStrength = pCtx->fCnr_SF_Strength;


    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }


    return XCAM_RETURN_NO_ERROR;
}


#endif

