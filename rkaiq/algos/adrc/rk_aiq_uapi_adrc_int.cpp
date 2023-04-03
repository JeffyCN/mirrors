#include "rk_aiq_uapi_adrc_int.h"
#include "adrc/rk_aiq_types_adrc_algo_prvt.h"

#if RKAIQ_HAVE_DRC_V10
XCamReturn rk_aiq_uapi_adrc_v10_SetAttrib(RkAiqAlgoContext* ctx, const drcAttrV10_t* attr,
                                          bool need_sync) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)(ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pAdrcCtx->drcAttrV10.opMode = attr->opMode;
    if (attr->opMode == DRC_OPMODE_AUTO) {
        memcpy(&pAdrcCtx->drcAttrV10.stAuto, &attr->stAuto, sizeof(CalibDbV2_drc_V10_t));
        pAdrcCtx->ifReCalcStAuto = true;
    }
    if (attr->opMode == DRC_OPMODE_MANUAL) {
        memcpy(&pAdrcCtx->drcAttrV10.stManual, &attr->stManual, sizeof(mdrcAttr_V10_t));
        pAdrcCtx->ifReCalcStManual = true;
    }

    LOG1_ATMO("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn rk_aiq_uapi_adrc_v10_GetAttrib(RkAiqAlgoContext* ctx, drcAttrV10_t* attr) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)ctx;

    attr->opMode = pAdrcCtx->drcAttrV10.opMode;
    memcpy(&attr->Info, &pAdrcCtx->drcAttrV10.Info, sizeof(DrcInfoV10_t));
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV10.stAuto, sizeof(CalibDbV2_drc_V10_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV10.stManual, sizeof(mdrcAttr_V10_t));

    LOG1_ATMO("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_DRC_V11
XCamReturn rk_aiq_uapi_adrc_v11_SetAttrib(RkAiqAlgoContext* ctx, const drcAttrV11_t* attr,
                                          bool need_sync) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)(ctx);
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;

    pAdrcCtx->drcAttrV11.opMode = attr->opMode;
    if (attr->opMode == DRC_OPMODE_AUTO) {
        memcpy(&pAdrcCtx->drcAttrV11.stAuto, &attr->stAuto, sizeof(CalibDbV2_drc_V11_t));
        pAdrcCtx->ifReCalcStAuto = true;
    }
    if (attr->opMode == DRC_OPMODE_MANUAL) {
        memcpy(&pAdrcCtx->drcAttrV11.stManual, &attr->stManual, sizeof(mdrcAttr_V11_t));
        pAdrcCtx->ifReCalcStManual = true;
    }

    LOG1_ATMO("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn rk_aiq_uapi_adrc_v11_GetAttrib(RkAiqAlgoContext* ctx, drcAttrV11_t* attr) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)ctx;

    attr->opMode = pAdrcCtx->drcAttrV11.opMode;
    memcpy(&attr->Info, &pAdrcCtx->drcAttrV11.Info, sizeof(DrcInfoV11_t));
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV11.stAuto, sizeof(CalibDbV2_drc_V11_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV11.stManual, sizeof(mdrcAttr_V11_t));

    LOG1_ATMO("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_DRC_V12
XCamReturn rk_aiq_uapi_adrc_v12_SetAttrib(RkAiqAlgoContext* ctx, const drcAttrV12_t* attr,
                                          bool need_sync) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)(ctx);
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;

    pAdrcCtx->drcAttrV12.opMode = attr->opMode;
    if (attr->opMode == DRC_OPMODE_AUTO) {
        memcpy(&pAdrcCtx->drcAttrV12.stAuto, &attr->stAuto, sizeof(CalibDbV2_drc_V12_t));
        pAdrcCtx->ifReCalcStAuto = true;
    }
    if (attr->opMode == DRC_OPMODE_MANUAL) {
        memcpy(&pAdrcCtx->drcAttrV12.stManual, &attr->stManual, sizeof(mdrcAttr_V12_t));
        pAdrcCtx->ifReCalcStManual = true;
    }

    LOG1_ATMO("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn rk_aiq_uapi_adrc_v12_GetAttrib(RkAiqAlgoContext* ctx, drcAttrV12_t* attr) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)ctx;

    attr->opMode = pAdrcCtx->drcAttrV12.opMode;
    memcpy(&attr->Info, &pAdrcCtx->drcAttrV12.Info, sizeof(DrcInfoV12_t));
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV12.stAuto, sizeof(CalibDbV2_drc_V12_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV12.stManual, sizeof(mdrcAttr_V12_t));

    LOG1_ATMO("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
XCamReturn rk_aiq_uapi_adrc_v12_lite_SetAttrib(RkAiqAlgoContext* ctx, const drcAttrV12Lite_t* attr,
                                               bool need_sync) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)(ctx);
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;

    pAdrcCtx->drcAttrV12.opMode = attr->opMode;
    if (attr->opMode == DRC_OPMODE_AUTO) {
        memcpy(&pAdrcCtx->drcAttrV12.stAuto, &attr->stAuto, sizeof(CalibDbV2_drc_v12_lite_t));
        pAdrcCtx->ifReCalcStAuto = true;
    }
    if (attr->opMode == DRC_OPMODE_MANUAL) {
        memcpy(&pAdrcCtx->drcAttrV12.stManual, &attr->stManual, sizeof(mdrcAttr_v12_lite_t));
        pAdrcCtx->ifReCalcStManual = true;
    }

    LOG1_ATMO("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn rk_aiq_uapi_adrc_v12_lite_GetAttrib(RkAiqAlgoContext* ctx, drcAttrV12Lite_t* attr) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)ctx;

    attr->opMode = pAdrcCtx->drcAttrV12.opMode;
    memcpy(&attr->Info, &pAdrcCtx->drcAttrV12.Info, sizeof(DrcInfoV12Lite_t));
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV12.stAuto, sizeof(CalibDbV2_drc_v12_lite_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV12.stManual, sizeof(mdrcAttr_v12_lite_t));

    LOG1_ATMO("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
#endif
