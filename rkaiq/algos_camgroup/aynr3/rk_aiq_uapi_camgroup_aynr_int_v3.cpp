#include "aynr3/rk_aiq_uapi_camgroup_aynr_int_v3.h"
#include "rk_aiq_types_camgroup_aynr_prvt_v3.h"

#if 1


XCamReturn
rk_aiq_uapi_camgroup_aynrV3_SetAttrib(RkAiqAlgoContext *ctx,
                                      rk_aiq_ynr_attrib_v3_t *attr,
                                      bool need_sync)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AynrV3_Contex_t *pGroupCtx = (CamGroup_AynrV3_Contex_t *)ctx;
    Aynr_Context_V3_t* pCtx = pGroupCtx->aynr_contex_v3;

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
rk_aiq_uapi_camgroup_aynrV3_GetAttrib(const RkAiqAlgoContext *ctx,
                                      rk_aiq_ynr_attrib_v3_t *attr)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AynrV3_Contex_t *pGroupCtx = (CamGroup_AynrV3_Contex_t *)ctx;
    Aynr_Context_V3_t* pCtx = pGroupCtx->aynr_contex_v3;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_uapi_camgroup_aynrV3_SetLumaSFStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_ynr_strength_v3_t *pStrength)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
    CamGroup_AynrV3_Contex_t *pGroupCtx = (CamGroup_AynrV3_Contex_t *)ctx;
    Aynr_Context_V3_t* pCtx = pGroupCtx->aynr_contex_v3;

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
rk_aiq_uapi_camgroup_aynrV3_GetLumaSFStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_ynr_strength_v3_t *pStrength)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
    CamGroup_AynrV3_Contex_t *pGroupCtx = (CamGroup_AynrV3_Contex_t *)ctx;
    Aynr_Context_V3_t* pCtx = pGroupCtx->aynr_contex_v3;

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
rk_aiq_uapi_camgroup_aynrV3_GetInfo(const RkAiqAlgoContext *ctx,
                                    rk_aiq_ynr_info_v3_t *pInfo)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
    CamGroup_AynrV3_Contex_t *pGroupCtx = (CamGroup_AynrV3_Contex_t *)ctx;
    Aynr_Context_V3_t* pCtx = pGroupCtx->aynr_contex_v3;

    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pCtx->stExpInfo;
    return XCAM_RETURN_NO_ERROR;
}

#endif

