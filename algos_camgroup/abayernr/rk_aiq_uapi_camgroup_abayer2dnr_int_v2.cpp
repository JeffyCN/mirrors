#include "abayer2dnr2/rk_aiq_uapi_camgroup_abayer2dnr_int_v2.h"
#include "rk_aiq_types_camgroup_abayernr_prvt.h"


#if 1

#define RAWNR_LUMA_TF_STRENGTH_MAX_PERCENT (100.0)
#define RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_camgroup_abayer2dnrV2_SetAttrib(RkAiqAlgoContext *ctx,
        rk_aiq_bayer2dnr_attrib_v2_t *attr,
        bool need_sync)
{
    CamGroup_Abayernr_Contex_t *pGroupCtx = (CamGroup_Abayernr_Contex_t *)ctx;
    Abayer2dnr_Context_V2_t* pCtx = pGroupCtx->abayernr_contex_v2;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == ABAYER2DNR_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == ABAYER2DNR_OP_MODE_MANUAL) {
        pCtx->stManual.st2DSelect = attr->stManual.st2DSelect;
    } else if(pCtx->eMode == ABAYER2DNR_OP_MODE_REG_MANUAL) {
        pCtx->stManual.st2Dfix = attr->stManual.st2Dfix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_abayer2dnrV2_GetAttrib(const RkAiqAlgoContext *ctx,
        rk_aiq_bayer2dnr_attrib_v2_t *attr)
{

    CamGroup_Abayernr_Contex_t *pGroupCtx = (CamGroup_Abayernr_Contex_t *)ctx;
    Abayer2dnr_Context_V2_t* pCtx = pGroupCtx->abayernr_contex_v2;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_camgroup_abayer2dnrV2_SetStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_bayer2dnr_strength_v2_t *pStrength)
{
    CamGroup_Abayernr_Contex_t *pGroupCtx = (CamGroup_Abayernr_Contex_t *)ctx;
    Abayer2dnr_Context_V2_t* pCtx = pGroupCtx->abayernr_contex_v2;

    float fStrength = 1.0f;
    float fMax = RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT;
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
rk_aiq_uapi_camgroup_abayer2dnrV2_GetStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_bayer2dnr_strength_v2_t *pStrength)
{
    CamGroup_Abayernr_Contex_t *pGroupCtx = (CamGroup_Abayernr_Contex_t *)ctx;
    Abayer2dnr_Context_V2_t* pCtx = pGroupCtx->abayernr_contex_v2;

    float fStrength = 1.0f;
    float fMax = RAWNR_LUMA_SF_STRENGTH_MAX_PERCENT;
    float fPercent = 0.5;

    fStrength = pCtx->stStrength.percent;

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

    *pStrength = pCtx->stStrength;
    pStrength->percent = fPercent;

    return XCAM_RETURN_NO_ERROR;
}



#endif

