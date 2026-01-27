#include "asharp3/rk_aiq_uapi_asharp_int_v3.h"
#include "asharp3/rk_aiq_types_asharp_algo_prvt_v3.h"

#define ASHSRPV3_STRENGTH_MAX_PERCENT (100.0)

XCamReturn
rk_aiq_uapi_asharpV3_SetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_sharp_attrib_v3_t *attr,
                               bool need_sync)
{

    Asharp_Context_V3_t* pAsharpCtx = (Asharp_Context_V3_t*)ctx;

    pAsharpCtx->eMode = attr->eMode;
    pAsharpCtx->stAuto = attr->stAuto;
    pAsharpCtx->stManual = attr->stManual;
    pAsharpCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharpV3_GetAttrib(const RkAiqAlgoContext *ctx,
                               rk_aiq_sharp_attrib_v3_t *attr)
{

    Asharp_Context_V3_t* pAsharpCtx = (Asharp_Context_V3_t*)ctx;

    attr->eMode = pAsharpCtx->eMode;
    memcpy(&attr->stAuto, &pAsharpCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pAsharpCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharpV3_SetStrength(const RkAiqAlgoContext *ctx,
                                 float fPercent)
{

    Asharp_Context_V3_t* pAsharpCtx = (Asharp_Context_V3_t*)ctx;
    float fMax = ASHSRPV3_STRENGTH_MAX_PERCENT;
    float fStrength = 1.0;


    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        fStrength = (fPercent - 0.5) * (fMax - 1) * 2 + 1;
    }

    pAsharpCtx->fSharp_Strength = fStrength;
    pAsharpCtx->isReCalculate |= 1;

	printf("percent:%f fStrength:%f \n", fStrength, fPercent);
	
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_asharpV3_GetStrength(const RkAiqAlgoContext *ctx,
                                 float *pPercent)
{

    Asharp_Context_V3_t* pAsharpCtx = (Asharp_Context_V3_t*)ctx;
    float fMax = ASHSRPV3_STRENGTH_MAX_PERCENT;
    float fStrength = 1.0;

    fStrength = pAsharpCtx->fSharp_Strength;

    if(fStrength <= 1) {
        *pPercent = fStrength * 0.5;
    } else {
        *pPercent = (fStrength - 1) / ((fMax - 1) * 2) + 0.5;
    }

	printf("fStrength:%f percent:%f\n", fStrength, *pPercent);

    return XCAM_RETURN_NO_ERROR;
}


