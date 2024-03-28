#include "rk_aiq_uapi_acp_int.h"
#include "rk_aiq_types_algo_acp_prvt.h"

XCamReturn rk_aiq_uapi_acp_SetAttrib(RkAiqAlgoContext* ctx,
                                    const acp_attrib_t* attr,
                                    bool need_sync)
{
    if(ctx == NULL) {
        LOGE_ACP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AcpContext_t* pAcpCtx = &ctx->acpCtx;
    pAcpCtx->params.brightness = attr->brightness;
    pAcpCtx->params.contrast = attr->contrast;
    pAcpCtx->params.saturation = attr->saturation;
    pAcpCtx->params.hue = attr->hue;
    pAcpCtx->isReCal_ = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_acp_GetAttrib(RkAiqAlgoContext*  ctx,
                                    acp_attrib_t* attr)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ACP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AcpContext_t* pAcpCtx = &ctx->acpCtx;
    attr->brightness = pAcpCtx->params.brightness;
    attr->contrast = pAcpCtx->params.contrast;
    attr->saturation = pAcpCtx->params.saturation;
    attr->hue = pAcpCtx->params.hue;
    return XCAM_RETURN_NO_ERROR;
}

