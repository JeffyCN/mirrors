#include "acnr2/rk_aiq_uapi_camgroup_acnr_int_v2.h"
#include "rk_aiq_types_camgroup_acnr_prvt_v2.h"

#if 1
#define ACNRV2_CHROMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_camgroup_acnrV2_SetAttrib(RkAiqAlgoContext *ctx,
                                      rk_aiq_cnr_attrib_v2_t *attr,
                                      bool need_sync)
{
    CamGroup_AcnrV2_Contex_t *pGroupCtx = (CamGroup_AcnrV2_Contex_t *)ctx;
    Acnr_Context_V2_t* pCtx = pGroupCtx->acnr_contex_v2;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == ACNRV2_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == ACNRV2_OP_MODE_MANUAL) {
        pCtx->stManual.stSelect = attr->stManual.stSelect;
    } else if(pCtx->eMode == ACNRV2_OP_MODE_REG_MANUAL) {
        pCtx->stManual.stFix = attr->stManual.stFix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_acnrV2_GetAttrib(const RkAiqAlgoContext *ctx,
                                      rk_aiq_cnr_attrib_v2_t *attr)
{

    CamGroup_AcnrV2_Contex_t *pGroupCtx = (CamGroup_AcnrV2_Contex_t *)ctx;
    Acnr_Context_V2_t* pCtx = pGroupCtx->acnr_contex_v2;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_uapi_camgroup_acnrV2_SetChromaSFStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_cnr_strength_v2_t *pStrength)
{
    CamGroup_AcnrV2_Contex_t *pGroupCtx = (CamGroup_AcnrV2_Contex_t *)ctx;
    Acnr_Context_V2_t* pCtx = pGroupCtx->acnr_contex_v2;

    float fStrength = 1.0f;
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
rk_aiq_uapi_camgroup_acnrV2_GetChromaSFStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_cnr_strength_v2_t *pStrength)
{
    CamGroup_AcnrV2_Contex_t *pGroupCtx = (CamGroup_AcnrV2_Contex_t *)ctx;
    Acnr_Context_V2_t* pCtx = pGroupCtx->acnr_contex_v2;

    float fStrength = 1.0f;
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

XCamReturn
rk_aiq_uapi_camgroup_acnrV2_GetInfo(const RkAiqAlgoContext *ctx,
                                    rk_aiq_cnr_info_v2_t *pInfo)
{
    CamGroup_AcnrV2_Contex_t *pGroupCtx = (CamGroup_AcnrV2_Contex_t *)ctx;
    Acnr_Context_V2_t* pCtx = pGroupCtx->acnr_contex_v2;

    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pCtx->stExpInfo;

    return XCAM_RETURN_NO_ERROR;
}

#endif

