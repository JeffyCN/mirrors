#include "rk_aiq_uapi_aie_int.h"
#include "rk_aiq_types_algo_aie_prvt.h"

XCamReturn
rk_aiq_uapi_aie_SetAttrib(RkAiqAlgoContext* ctx,
                            const aie_attrib_t* attr,
                            bool need_sync)
{
    if(ctx == NULL) {
        LOGE_AIE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    ctx->params.mode = attr->mode;
    ctx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_uapi_aie_GetAttrib(RkAiqAlgoContext*  ctx,
                                    aie_attrib_t* attr)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_AIE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    attr->mode = ctx->params.mode;

    return XCAM_RETURN_NO_ERROR;
}

