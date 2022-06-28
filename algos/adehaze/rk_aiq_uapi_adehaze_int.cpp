#include "rk_aiq_uapi_adehaze_int.h"
#include "rk_aiq_types_adehaze_algo_prvt.h"
#include "xcam_log.h"

XCamReturn
rk_aiq_uapi_adehaze_SetAttrib(RkAiqAlgoContext *ctx,
                              adehaze_sw_V2_t attr,
                              bool need_sync)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t * AdehazeHandle = (AdehazeHandle_t *)ctx;

    AdehazeHandle->AdehazeAtrr.mode = attr.mode;
    if(attr.mode == DEHAZE_API_MANUAL)
        memcpy(&AdehazeHandle->AdehazeAtrr.stManual, &attr.stManual, sizeof(mDehazeAttr_t));
    else if(attr.mode == DEHAZE_API_DEHAZE_MANUAL)
        memcpy(&AdehazeHandle->AdehazeAtrr.stDehazeManu, &attr.stDehazeManu, sizeof(DehazeManuAttr_t));
    else if(attr.mode == DEHAZE_API_ENHANCE_MANUAL)
        memcpy(&AdehazeHandle->AdehazeAtrr.stEnhanceManu, &attr.stEnhanceManu, sizeof(EnhanceManuAttr_t));

    return ret;
}

XCamReturn
rk_aiq_uapi_adehaze_GetAttrib(RkAiqAlgoContext *ctx, adehaze_sw_V2_t *attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t * AdehazeHandle = (AdehazeHandle_t *)ctx;

    attr->mode = AdehazeHandle->AdehazeAtrr.mode;
    memcpy(&attr->stManual, &AdehazeHandle->AdehazeAtrr.stManual, sizeof(mDehazeAttr_t));
    memcpy(&attr->stDehazeManu, &AdehazeHandle->AdehazeAtrr.stDehazeManu, sizeof(DehazeManuAttr_t));
    memcpy(&attr->stEnhanceManu, &AdehazeHandle->AdehazeAtrr.stEnhanceManu, sizeof(EnhanceManuAttr_t));

    return ret;
}

