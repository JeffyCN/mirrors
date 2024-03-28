#include "asharp4/rk_aiq_uapi_camgroup_asharp_int_v4.h"
#include "rk_aiq_types_camgroup_asharp_prvt_v4.h"

XCamReturn
rk_aiq_uapi_camgroup_asharpV4_SetAttrib(RkAiqAlgoContext *ctx,
                                        rk_aiq_sharp_attrib_v4_t *attr,
                                        bool need_sync)
{
    LOGD_ASHARP("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AsharpV4_Contex_t *pGroupCtx = (CamGroup_AsharpV4_Contex_t *)ctx;
    Asharp_Context_V4_t* pCtx = pGroupCtx->asharp_contex_v4;

    pCtx->eMode = attr->eMode;
    if (pCtx->eMode == ASHARP4_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if (pCtx->eMode == ASHARP4_OP_MODE_MANUAL) {
        pCtx->stManual.stSelect = attr->stManual.stSelect;
    } else if (pCtx->eMode == ASHARP4_OP_MODE_REG_MANUAL) {
        pCtx->stManual.stFix = attr->stManual.stFix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_asharpV4_GetAttrib(const RkAiqAlgoContext *ctx,
                                        rk_aiq_sharp_attrib_v4_t *attr)
{
    LOGD_ASHARP("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AsharpV4_Contex_t *pGroupCtx = (CamGroup_AsharpV4_Contex_t *)ctx;
    Asharp_Context_V4_t* pCtx = pGroupCtx->asharp_contex_v4;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_asharpV4_SetStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_sharp_strength_v4_t *pStrength)
{
    LOGD_ASHARP("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AsharpV4_Contex_t *pGroupCtx = (CamGroup_AsharpV4_Contex_t *)ctx;
    Asharp_Context_V4_t* pCtx = pGroupCtx->asharp_contex_v4;

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
rk_aiq_uapi_camgroup_asharpV4_GetStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_sharp_strength_v4_t *pStrength)
{
    LOGD_ASHARP("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AsharpV4_Contex_t *pGroupCtx = (CamGroup_AsharpV4_Contex_t *)ctx;
    Asharp_Context_V4_t* pCtx = pGroupCtx->asharp_contex_v4;

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
rk_aiq_uapi_camgroup_asharpV4_GetInfo(const RkAiqAlgoContext *ctx,
                                      rk_aiq_sharp_info_v4_t *pInfo)
{
    LOGD_ASHARP("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AsharpV4_Contex_t *pGroupCtx = (CamGroup_AsharpV4_Contex_t *)ctx;
    Asharp_Context_V4_t* pCtx = pGroupCtx->asharp_contex_v4;

    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pCtx->stExpInfo;

    return XCAM_RETURN_NO_ERROR;

}

