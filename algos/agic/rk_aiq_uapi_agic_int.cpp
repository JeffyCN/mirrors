#include "rk_aiq_uapi_agic_int.h"
#include "rk_aiq_types_algo_agic_prvt.h"

XCamReturn
rk_aiq_uapi_agic_SetAttrib
(
    RkAiqAlgoContext* ctx,
    agic_attrib_t attr,
    bool need_sync
)
{
    if(ctx == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AgicContext_t* pAgicCtx = (AgicContext_t*)&ctx->agicCtx;
    pAgicCtx->attr.opMode = attr.opMode;
    memcpy(&pAgicCtx->attr.stAuto, &attr.stAuto, sizeof(agicAttr_t));
    memcpy(&pAgicCtx->attr.stManual, &attr.stManual, sizeof(mgicAttr_t));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_agic_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    agic_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_AGIC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AgicContext_t* pAgicCtx = (AgicContext_t*)&ctx->agicCtx;

    attr->opMode = pAgicCtx->attr.opMode;
    memcpy(&attr->stAuto, &pAgicCtx->attr.stAuto, sizeof(agicAttr_t));
    memcpy(&attr->stManual, &pAgicCtx->attr.stManual, sizeof(mgicAttr_t));

    return XCAM_RETURN_NO_ERROR;
}

