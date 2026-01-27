#include "rk_aiq_uapi_adrc_int.h"
#include "adrc/rk_aiq_types_adrc_algo_prvt.h"
#if RKAIQ_HAVE_DRC_V10
#include "adrc/rk_aiq_adrc_algo_v10.h"
#endif
#if RKAIQ_HAVE_DRC_V11
#include "adrc/rk_aiq_adrc_algo_v11.h"
#endif
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE
#include "adrc/rk_aiq_adrc_algo_v12.h"
#endif
#if RKAIQ_HAVE_DRC_V20
#include "adrc/rk_aiq_adrc_algo_v20.h"
#endif

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
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV10.stAuto, sizeof(CalibDbV2_drc_V10_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV10.stManual, sizeof(mdrcAttr_V10_t));
    // drc info
    AdrcV10Params2Api(pAdrcCtx, &attr->Info);

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
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV11.stAuto, sizeof(CalibDbV2_drc_V11_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV11.stManual, sizeof(mdrcAttr_V11_t));
    // drc info
    AdrcV11Params2Api(pAdrcCtx, &attr->Info);

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
        AdrcV12ClipStAutoParams(pAdrcCtx);
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
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV12.stAuto, sizeof(CalibDbV2_drc_V12_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV12.stManual, sizeof(mdrcAttr_V12_t));
    // drc info
    AdrcV12Params2Api(pAdrcCtx, &attr->Info);

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
        AdrcV12ClipStAutoParams(pAdrcCtx);
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
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV12.stAuto, sizeof(CalibDbV2_drc_v12_lite_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV12.stManual, sizeof(mdrcAttr_v12_lite_t));
    // drc info
    AdrcV12LiteParams2Api(pAdrcCtx, &attr->Info);

    LOG1_ATMO("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_DRC_V20
XCamReturn rk_aiq_uapi_adrc_v20_SetAttrib(RkAiqAlgoContext* ctx, const drcAttrV20_t* attr,
                                          bool need_sync) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)(ctx);
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;

    pAdrcCtx->drcAttrV20.opMode = attr->opMode;
    if (attr->opMode == DRC_OPMODE_AUTO) {
        memcpy(&pAdrcCtx->drcAttrV20.stAuto, &attr->stAuto, sizeof(CalibDbV2_drc_V20_t));
        AdrcV20ClipStAutoParams(pAdrcCtx);
        pAdrcCtx->ifReCalcStAuto = true;
    }
    if (attr->opMode == DRC_OPMODE_MANUAL) {
        memcpy(&pAdrcCtx->drcAttrV20.stManual, &attr->stManual, sizeof(mdrcAttr_V20_t));
        pAdrcCtx->ifReCalcStManual = true;
    }

    LOG1_ATMO("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn rk_aiq_uapi_adrc_v20_GetAttrib(RkAiqAlgoContext* ctx, drcAttrV20_t* attr) {
    LOG1_ATMO("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)ctx;

    attr->opMode = pAdrcCtx->drcAttrV20.opMode;
    memcpy(&attr->stAuto, &pAdrcCtx->drcAttrV20.stAuto, sizeof(CalibDbV2_drc_V20_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttrV20.stManual, sizeof(mdrcAttr_V20_t));
    // drc info
    AdrcV20Params2Api(pAdrcCtx, &attr->Info);

    LOG1_ATMO("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
#endif
