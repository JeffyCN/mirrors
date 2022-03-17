#include "rk_aiq_uapi_adebayer_int.h"
#include "rk_aiq_types_algo_adebayer_prvt.h"

XCamReturn
rk_aiq_uapi_adebayer_SetAttrib
(
    RkAiqAlgoContext* ctx,
    adebayer_attrib_t attr,
    bool need_sync
)
{
    if(ctx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;
    pAdebayerCtx->full_param.enable = attr.enable;
    pAdebayerCtx->full_param.thed0 = attr.high_freq_thresh;
    pAdebayerCtx->full_param.thed1 = attr.low_freq_thresh;
    memcpy(pAdebayerCtx->full_param.sharp_strength, attr.enhance_strength, sizeof(attr.enhance_strength));
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
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;
    attr->enable = pAdebayerCtx->full_param.enable;
    attr->high_freq_thresh = pAdebayerCtx->full_param.thed0;
    attr->low_freq_thresh = pAdebayerCtx->full_param.thed1;
    memcpy(attr->enhance_strength, pAdebayerCtx->full_param.sharp_strength, sizeof(attr->enhance_strength));

    return XCAM_RETURN_NO_ERROR;
}

