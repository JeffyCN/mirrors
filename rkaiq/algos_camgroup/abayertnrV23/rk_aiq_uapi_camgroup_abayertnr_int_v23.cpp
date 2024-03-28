#include "abayertnrV23/rk_aiq_uapi_camgroup_abayertnr_int_v23.h"
#include "rk_aiq_types_camgroup_abayertnr_prvt_v23.h"

#define ABAYERTNR_LUMA_SF_STRENGTH_MAX_PERCENT (7.0)

XCamReturn rk_aiq_uapi_camgroup_abayertnrV23_SetAttrib(RkAiqAlgoContext* ctx,
                                                       const rk_aiq_bayertnr_attrib_v23_t* attr,
                                                       bool /* need_sync */) {
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
#if (RKAIQ_HAVE_BAYERTNR_V23)

    CamGroup_AbayertnrV23_Contex_t *pGroupCtx = (CamGroup_AbayertnrV23_Contex_t *)ctx;
    Abayertnr_Context_V23_t* pCtx = pGroupCtx->abayertnr_contex_v23;

    pCtx->eMode = attr->eMode;
    if (pCtx->eMode == ABAYERTNRV23_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if (pCtx->eMode == ABAYERTNRV23_OP_MODE_MANUAL) {
        pCtx->stManual.st3DSelect = attr->stManual.st3DSelect;
    } else if (pCtx->eMode == ABAYERTNRV23_OP_MODE_REG_MANUAL) {
        pCtx->stManual.st3DFix = attr->stManual.st3DFix;
    }
    pCtx->isReCalculate |= 1;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_camgroup_abayertnrV23_GetAttrib(const RkAiqAlgoContext* ctx,
                                                       rk_aiq_bayertnr_attrib_v23_t* attr) {
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

#if (RKAIQ_HAVE_BAYERTNR_V23)
    CamGroup_AbayertnrV23_Contex_t *pGroupCtx = (CamGroup_AbayertnrV23_Contex_t *)ctx;
    Abayertnr_Context_V23_t* pCtx = pGroupCtx->abayertnr_contex_v23;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_camgroup_abayertnrV23Lite_SetAttrib(RkAiqAlgoContext* ctx,
                                                           const rk_aiq_bayertnr_attrib_v23L_t* attr,
                                                           bool /* need_sync */) {
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
#if (RKAIQ_HAVE_BAYERTNR_V23_LITE)
    CamGroup_AbayertnrV23_Contex_t* pGroupCtx = (CamGroup_AbayertnrV23_Contex_t*)ctx;
    Abayertnr_Context_V23_t* pCtx             = pGroupCtx->abayertnr_contex_v23;

    pCtx->eMode = attr->eMode;
    if (pCtx->eMode == ABAYERTNRV23_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if (pCtx->eMode == ABAYERTNRV23_OP_MODE_MANUAL) {
        pCtx->stManual.st3DSelect = attr->stManual.st3DSelect;
    } else if (pCtx->eMode == ABAYERTNRV23_OP_MODE_REG_MANUAL) {
        pCtx->stManual.st3DFix = attr->stManual.st3DFix;
    }
    pCtx->isReCalculate |= 1;
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_camgroup_abayertnrV23Lite_GetAttrib(const RkAiqAlgoContext* ctx,
                                                           rk_aiq_bayertnr_attrib_v23L_t* attr) {
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
#if (RKAIQ_HAVE_BAYERTNR_V23)
    CamGroup_AbayertnrV23_Contex_t* pGroupCtx = (CamGroup_AbayertnrV23_Contex_t*)ctx;
    Abayertnr_Context_V23_t* pCtx             = pGroupCtx->abayertnr_contex_v23;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_abayertnrV23_SetStrength(const RkAiqAlgoContext *ctx,
        const rk_aiq_bayertnr_strength_v23_t* pStrength)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AbayertnrV23_Contex_t *pGroupCtx = (CamGroup_AbayertnrV23_Contex_t *)ctx;
    Abayertnr_Context_V23_t* pCtx = pGroupCtx->abayertnr_contex_v23;

    float fStrength = 1.0f;
    float fslope = ABAYERTNR_LUMA_SF_STRENGTH_MAX_PERCENT;
    float fPercent = 0.5f;

    fPercent = pStrength->percent;

    if(fPercent <= 0.5) {
        fStrength =  fPercent / 0.5;
    } else {
        if(fPercent >= 0.999999)
            fPercent = 0.999999;
        fStrength = 0.5 * fslope / (1.0 - fPercent) - fslope + 1;
    }

    pCtx->stStrength = *pStrength;
    pCtx->stStrength.percent = fStrength;
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_abayertnrV23_GetStrength(const RkAiqAlgoContext *ctx,
        rk_aiq_bayertnr_strength_v23_t* pStrength)
{
    LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);

    CamGroup_AbayertnrV23_Contex_t *pGroupCtx = (CamGroup_AbayertnrV23_Contex_t *)ctx;
    Abayertnr_Context_V23_t* pCtx = pGroupCtx->abayertnr_contex_v23;

    float fStrength = 1.0f;
    float fslope = ABAYERTNR_LUMA_SF_STRENGTH_MAX_PERCENT;
    float fPercent = 0.5f;

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
rk_aiq_uapi_camgroup_abayertnrV23_GetInfo(const RkAiqAlgoContext* ctx,
        rk_aiq_bayertnr_info_v23_t* pInfo) {

    CamGroup_AbayertnrV23_Contex_t *pGroupCtx = (CamGroup_AbayertnrV23_Contex_t *)ctx;
    Abayertnr_Context_V23_t* pCtx = pGroupCtx->abayertnr_contex_v23;

    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];


    pInfo->expo_info = pCtx->stExpInfo;
    return XCAM_RETURN_NO_ERROR;
}



