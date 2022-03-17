#include "rk_aiq_uapi_acsm.h"
#include "rk_aiq_types_algo_acsm_prvt.h"

XCamReturn
rk_aiq_uapi_acsm_SetAttrib
(
    RkAiqAlgoContext* ctx,
    rk_aiq_uapi_acsm_attrib_t attr
)
{
    if(ctx == NULL) {
        LOGE_ACSM("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AcsmContext_t* pAcsmCtx = &ctx->acsmCtx;
    pAcsmCtx->params = attr.param; 
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_acsm_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    rk_aiq_uapi_acsm_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ACSM("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AcsmContext_t* pAcsmCtx = &ctx->acsmCtx;
    attr->param = pAcsmCtx->params;
    return XCAM_RETURN_NO_ERROR;
}

