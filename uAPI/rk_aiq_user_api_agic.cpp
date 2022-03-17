#include "rk_aiq_user_api_agic.h"
#include "RkAiqHandleInt.h"

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn
rk_aiq_user_api_agic_SetAttrib(rk_aiq_sys_ctx_t* sys_ctx, agic_attrib_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGIC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAgicHandleInt* algo_handle =
        algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

    if (algo_handle) {
        return algo_handle->setAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_agic_GetAttrib(rk_aiq_sys_ctx_t* sys_ctx, agic_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    RkAiqAgicHandleInt* algo_handle =
        algoHandle<RkAiqAgicHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGIC);

    if (algo_handle) {
        return algo_handle->getAttrib(attr);
    }

    return XCAM_RETURN_NO_ERROR;
}

