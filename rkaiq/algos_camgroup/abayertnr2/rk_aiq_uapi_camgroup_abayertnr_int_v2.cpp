#include "abayertnr2/rk_aiq_uapi_camgroup_abayertnr_int_v2.h"
#include "rk_aiq_types_camgroup_abayertnr_prvt_v2.h"


XCamReturn
rk_aiq_uapi_camgroup_abayertnrV2_SetAttrib(RkAiqAlgoContext *ctx,
        rk_aiq_bayertnr_attrib_v2_t *attr,
        bool need_sync)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AbayertnrV2_Contex_t *pGroupCtx = (CamGroup_AbayertnrV2_Contex_t *)ctx;
    Abayertnr_Context_V2_t* pCtx = pGroupCtx->abayertnr_contex_v2;

    pCtx->eMode = attr->eMode;
    if (pCtx->eMode == ABAYERTNRV2_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if (pCtx->eMode == ABAYERTNRV2_OP_MODE_MANUAL) {
        pCtx->stManual.st3DSelect = attr->stManual.st3DSelect;
    } else if (pCtx->eMode == ABAYERTNRV2_OP_MODE_REG_MANUAL) {
        pCtx->stManual.st3DFix = attr->stManual.st3DFix;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_abayertnrV2_GetAttrib(const RkAiqAlgoContext *ctx,
        rk_aiq_bayertnr_attrib_v2_t *attr)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AbayertnrV2_Contex_t *pGroupCtx = (CamGroup_AbayertnrV2_Contex_t *)ctx;
    Abayertnr_Context_V2_t* pCtx = pGroupCtx->abayertnr_contex_v2;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_abayertnrV2_SetStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_bayertnr_strength_v2_t *pStrength)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AbayertnrV2_Contex_t *pGroupCtx = (CamGroup_AbayertnrV2_Contex_t *)ctx;
    Abayertnr_Context_V2_t* pCtx = pGroupCtx->abayertnr_contex_v2;

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
rk_aiq_uapi_camgroup_abayertnrV2_GetStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_bayertnr_strength_v2_t *pStrength)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AbayertnrV2_Contex_t *pGroupCtx = (CamGroup_AbayertnrV2_Contex_t *)ctx;
    Abayertnr_Context_V2_t* pCtx = pGroupCtx->abayertnr_contex_v2;

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
rk_aiq_uapi_camgroup_abayertnrV2_GetInfo(const RkAiqAlgoContext *ctx,
        rk_aiq_bayertnr_info_v2_t *pInfo)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AbayertnrV2_Contex_t *pGroupCtx = (CamGroup_AbayertnrV2_Contex_t *)ctx;
    Abayertnr_Context_V2_t* pCtx = pGroupCtx->abayertnr_contex_v2;

    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pCtx->stExpInfo;

    return XCAM_RETURN_NO_ERROR;
}


