#include "rk_aiq_uapi_adebayer_int.h"
#if RKAIQ_HAVE_DEBAYER_V1
#include "adebayer/rk_aiq_adebayer_algo_v1.h"
#endif
#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE
#include "adebayer/rk_aiq_adebayer_algo_v2.h"
#endif
#if RKAIQ_HAVE_DEBAYER_V3
#include "adebayer/rk_aiq_adebayer_algo_v3.h"
#endif

#if RKAIQ_HAVE_DEBAYER_V1
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
    pAdebayerCtx->mode = attr.mode;
    if (attr.mode == RK_AIQ_DEBAYER_MODE_AUTO) {
        pAdebayerCtx->full_param.thed0 = attr.stAuto.high_freq_thresh;
        pAdebayerCtx->full_param.thed1 = attr.stAuto.low_freq_thresh;
        memcpy(pAdebayerCtx->full_param.sharp_strength, attr.stAuto.sharp_strength,
               sizeof(attr.stAuto.sharp_strength));
    } else if (attr.mode == RK_AIQ_DEBAYER_MODE_MANUAL) {
        memcpy(&pAdebayerCtx->manualAttrib, &attr.stManual, sizeof(attr.stManual));
    } else {
        LOGE_ADEBAYER("Invalid mode: %s\n", attr.mode == RK_AIQ_DEBAYER_MODE_AUTO ? "auto" : "manual");
        return XCAM_RETURN_ERROR_PARAM;
    }

    pAdebayerCtx->is_reconfig = true;

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
    attr->mode = pAdebayerCtx->mode;
    attr->stAuto.high_freq_thresh = pAdebayerCtx->full_param.thed0;
    attr->stAuto.low_freq_thresh = pAdebayerCtx->full_param.thed1;
    memcpy(attr->stAuto.sharp_strength, pAdebayerCtx->full_param.sharp_strength,
           sizeof(attr->stAuto.sharp_strength));

    memcpy(&attr->stManual, &pAdebayerCtx->manualAttrib, sizeof(pAdebayerCtx->manualAttrib));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_DEBAYER_V2
XCamReturn
rk_aiq_uapi_adebayer_v2_SetAttrib
(
    RkAiqAlgoContext* ctx,
    adebayer_v2_attrib_t attr,
    bool need_sync
) {
    if(ctx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;

    pAdebayerCtx->mode = attr.mode;

    if (attr.mode == RK_AIQ_DEBAYER_MODE_AUTO) {
        pAdebayerCtx->full_param_v2 = attr.stAuto;

    } else if (attr.mode == RK_AIQ_DEBAYER_MODE_MANUAL) {
        pAdebayerCtx->select_param_v2 = attr.stManual;
    } else {
        LOGE_ADEBAYER("Invalid mode: %d\n", attr.mode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pAdebayerCtx->is_reconfig = true;

    return XCAM_RETURN_NO_ERROR;
}
XCamReturn
rk_aiq_uapi_adebayer_v2_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    adebayer_v2_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;

    attr->mode = pAdebayerCtx->mode;
    attr->stAuto = pAdebayerCtx->full_param_v2;
    attr->stManual = pAdebayerCtx->select_param_v2;

    return XCAM_RETURN_NO_ERROR;
}

#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
XCamReturn
rk_aiq_uapi_adebayer_v2lite_SetAttrib
(
    RkAiqAlgoContext* ctx,
    adebayer_v2lite_attrib_t attr,
    bool need_sync
) {
    if(ctx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;

    pAdebayerCtx->mode = attr.mode;

    if (attr.mode == RK_AIQ_DEBAYER_MODE_AUTO) {
        pAdebayerCtx->full_param_v2_lite = attr.stAuto;

    } else if (attr.mode == RK_AIQ_DEBAYER_MODE_MANUAL) {
        pAdebayerCtx->select_param_v2_lite = attr.stManual;
    } else {
        LOGE_ADEBAYER("Invalid mode: %d\n", attr.mode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pAdebayerCtx->is_reconfig = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_adebayer_v2lite_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    adebayer_v2lite_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;

    attr->mode = pAdebayerCtx->mode;
    attr->stAuto = pAdebayerCtx->full_param_v2_lite;
    attr->stManual = pAdebayerCtx->select_param_v2_lite;

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_DEBAYER_V3
XCamReturn
rk_aiq_uapi_adebayer_v3_SetAttrib
(
    RkAiqAlgoContext* ctx,
    adebayer_v3_attrib_t attr,
    bool need_sync
) {
    if(ctx == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;

    pAdebayerCtx->mode = attr.mode;

    if (attr.mode == RK_AIQ_DEBAYER_MODE_AUTO) {
        pAdebayerCtx->full_param_v3 = attr.stAuto;

    } else if (attr.mode == RK_AIQ_DEBAYER_MODE_MANUAL) {
        pAdebayerCtx->select_param_v3 = attr.stManual;
    } else {
        LOGE_ADEBAYER("Invalid mode: %d\n", attr.mode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pAdebayerCtx->is_reconfig = true;

    return XCAM_RETURN_NO_ERROR;
}
XCamReturn
rk_aiq_uapi_adebayer_v3_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    adebayer_v3_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ADEBAYER("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&ctx->adebayerCtx;

    attr->mode = pAdebayerCtx->mode;
    attr->stAuto = pAdebayerCtx->full_param_v3;
    attr->stManual = pAdebayerCtx->select_param_v3;

    return XCAM_RETURN_NO_ERROR;
}

#endif

