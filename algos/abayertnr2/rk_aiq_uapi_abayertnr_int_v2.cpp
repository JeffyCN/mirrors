#include "abayertnr2/rk_aiq_uapi_abayertnr_int_v2.h"
#include "abayertnr2/rk_aiq_types_abayertnr_algo_prvt_v2.h"
#include "RkAiqCalibApi.h"
//#include "bayertnr_xml2json_v2.h"



#if 1

#define ABAYERTNR_LUMA_TF_STRENGTH_MAX_PERCENT (100.0)
#define ABAYERTNR_LUMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_abayertnrV2_SetAttrib(RkAiqAlgoContext *ctx,
                                  rk_aiq_bayertnr_attrib_v2_t *attr,
                                  bool need_sync)
{

    Abayertnr_Context_V2_t* pCtx = (Abayertnr_Context_V2_t*)ctx;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == ABAYERTNRV2_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == ABAYERTNRV2_OP_MODE_MANUAL) {
        pCtx->stManual.st3DSelect = attr->stManual.st3DSelect;
    } else if(pCtx->eMode == ABAYERTNRV2_OP_MODE_REG_MANUAL) {
        pCtx->stManual.st3DFix = attr->stManual.st3DFix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_abayertnrV2_GetAttrib(const RkAiqAlgoContext *ctx,
                                  rk_aiq_bayertnr_attrib_v2_t *attr)
{

    Abayertnr_Context_V2_t* pCtx = (Abayertnr_Context_V2_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_abayertnrV2_SetStrength(const RkAiqAlgoContext *ctx,
                                    rk_aiq_bayertnr_strength_v2_t *pStrength)
{
    Abayertnr_Context_V2_t* pCtx = (Abayertnr_Context_V2_t*)ctx;

    float fStrength = 1.0;
    float fMax = ABAYERTNR_LUMA_TF_STRENGTH_MAX_PERCENT;
    float fPercent = 0.5;

    fPercent = pStrength->percent;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        if(fPercent >= 0.999999)
            fPercent = 0.999999;
        fStrength = 0.5 / (1.0 - fPercent);
    }

    pCtx->stStrength = *pStrength;
    pCtx->stStrength.percent = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}




XCamReturn
rk_aiq_uapi_abayertnrV2_GetStrength(const RkAiqAlgoContext *ctx,
                                    rk_aiq_bayertnr_strength_v2_t *pStrength)
{
    Abayertnr_Context_V2_t* pCtx = (Abayertnr_Context_V2_t*)ctx;

    float fStrength = 1.0;
    float fMax = ABAYERTNR_LUMA_TF_STRENGTH_MAX_PERCENT;
    float fpercent = 0.5;

    fStrength = pCtx->stStrength.percent;

    if(fStrength <= 1) {
        fpercent = fStrength * 0.5;
    } else {
        float tmp = 1.0;
        tmp = 1 - 0.5 / fStrength;
        if(abs(tmp - 0.999999) < 0.000001) {
            tmp = 1.0;
        }
        fpercent = tmp;
    }

    *pStrength = pCtx->stStrength;
    pStrength->percent = fpercent;

    return XCAM_RETURN_NO_ERROR;
}


#endif

