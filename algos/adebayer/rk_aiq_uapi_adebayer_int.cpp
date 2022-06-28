#include "rk_aiq_uapi_adebayer_int.h"
#include "rk_aiq_algo_adebayer.h"

XCamReturn
rk_aiq_uapi_adebayer_SetAttrib
(
    RkAiqAlgoContext* ctx,
    adebayer_attrib_t attr,
    bool need_sync
)
{
    if(ctx == NULL) {
        LOGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;
    pAdebayerCtx->full_param.enable = attr.enable;
    pAdebayerCtx->mode = attr.mode;
    if (attr.mode == RK_AIQ_DEBAYER_MODE_AUTO) {
        pAdebayerCtx->full_param.thed0 = attr.stAuto.high_freq_thresh;
        pAdebayerCtx->full_param.thed1 = attr.stAuto.low_freq_thresh;
        memcpy(pAdebayerCtx->full_param.sharp_strength, attr.stAuto.sharp_strength,
               sizeof(attr.stAuto.sharp_strength));
    } else if (attr.mode == RK_AIQ_DEBAYER_MODE_MANUAL) {
        memcpy(&pAdebayerCtx->manualAttrib, &attr.stManual, sizeof(attr.stManual));
    } else {
        LOGE("Invalid mode: %s\n", attr.mode == RK_AIQ_DEBAYER_MODE_AUTO ? "auto" : "manual");
        return XCAM_RETURN_ERROR_PARAM;
    }

    pAdebayerCtx->full_param.updated = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_adebayer_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    adebayer_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;
    attr->enable = pAdebayerCtx->full_param.enable;
    attr->mode = pAdebayerCtx->mode;
    attr->stAuto.high_freq_thresh = pAdebayerCtx->full_param.thed0;
    attr->stAuto.low_freq_thresh = pAdebayerCtx->full_param.thed1;
    memcpy(attr->stAuto.sharp_strength, pAdebayerCtx->full_param.sharp_strength,
           sizeof(attr->stAuto.sharp_strength));

    memcpy(&attr->stManual, &pAdebayerCtx->manualAttrib, sizeof(pAdebayerCtx->manualAttrib));

    return XCAM_RETURN_NO_ERROR;
}

