#include "rk_aiq_uapi_argbir_int.h"

#include "argbir/rk_aiq_types_argbir_algo_prvt.h"

#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
XCamReturn rk_aiq_uapi_argbir_v10_SetAttrib(RkAiqAlgoContext* ctx, const RgbirAttrV10_t* attr,
                                            bool need_sync) {
    LOG1_ARGBIR("ENTER: %s \n", __func__);
    aRgbirContext_t* pArgbirCtx = (aRgbirContext_t*)(ctx);
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;

    pArgbirCtx->RgbirAttrV10.opMode = attr->opMode;
    if (attr->opMode == RGBIR_OPMODE_AUTO) {
        memcpy(&pArgbirCtx->RgbirAttrV10.stAuto, &attr->stAuto, sizeof(CalibDbV2_rgbir_v10_t));
        pArgbirCtx->ifReCalcStAuto = true;
    }
    if (attr->opMode == RGBIR_OPMODE_MANUAL) {
        memcpy(&pArgbirCtx->RgbirAttrV10.stManual, &attr->stManual, sizeof(mRgbirAttr_v10_t));
        pArgbirCtx->ifReCalcStManual = true;
    }

    LOGV_ARGBIR("%s: update api params, opMode:%d ifReCalcStAuto:%d ifReCalcStManual:%d\n",
                __FUNCTION__, pArgbirCtx->RgbirAttrV10.opMode, pArgbirCtx->ifReCalcStAuto,
                pArgbirCtx->ifReCalcStManual);
    LOG1_ARGBIR("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn rk_aiq_uapi_argbir_v10_GetAttrib(RkAiqAlgoContext* ctx, RgbirAttrV10_t* attr) {
    LOG1_ARGBIR("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aRgbirContext_t* pArgbirCtx = (aRgbirContext_t*)ctx;

    attr->opMode = pArgbirCtx->RgbirAttrV10.opMode;
    memcpy(&attr->stAuto, &pArgbirCtx->RgbirAttrV10.stAuto, sizeof(CalibDbV2_rgbir_v10_t));
    memcpy(&attr->stManual, &pArgbirCtx->RgbirAttrV10.stManual, sizeof(mRgbirAttr_v10_t));
    // get info
    attr->info.iso         = pArgbirCtx->NextData.iso;
    attr->info.ValidParams = pArgbirCtx->NextData.HandleData;

    LOG1_ARGBIR("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
#endif