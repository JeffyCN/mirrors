#include "rk_aiq_uapi_agamma_int.h"
#include "agamma/rk_aiq_types_agamma_algo_prvt.h"

XCamReturn
rk_aiq_uapi_agamma_SetAttrib(RkAiqAlgoContext *ctx,
                             rk_aiq_gamma_attrib_V2_t attr,
                             bool need_sync)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    AgammaHandle_t *gamma_handle = (AgammaHandle_t *)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(CHECK_ISP_HW_V21()) {
        gamma_handle->agammaAttr.atrrV21.mode = attr.atrrV21.mode;
        if(attr.atrrV21.mode == RK_AIQ_GAMMA_MODE_MANUAL)
            memcpy(&gamma_handle->agammaAttr.atrrV21.stManual, &attr.atrrV21.stManual, sizeof(Agamma_api_manualV21_t));
        if(attr.atrrV21.mode == RK_AIQ_GAMMA_MODE_FAST)
            memcpy(&gamma_handle->agammaAttr.atrrV21.stFast, &attr.atrrV21.stFast, sizeof(Agamma_api_Fast_t));
    }
    else if(CHECK_ISP_HW_V30()) {
        gamma_handle->agammaAttr.atrrV30.mode = attr.atrrV30.mode;
        if(attr.atrrV30.mode == RK_AIQ_GAMMA_MODE_MANUAL)
            memcpy(&gamma_handle->agammaAttr.atrrV30.stManual, &attr.atrrV30.stManual, sizeof(Agamma_api_manualV30_t));
        if(attr.atrrV30.mode == RK_AIQ_GAMMA_MODE_FAST)
            memcpy(&gamma_handle->agammaAttr.atrrV30.stFast, &attr.atrrV30.stFast, sizeof(Agamma_api_Fast_t));
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn
rk_aiq_uapi_agamma_GetAttrib(const RkAiqAlgoContext *ctx,
                             rk_aiq_gamma_attrib_V2_t *attr)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    AgammaHandle_t* gamma_handle = (AgammaHandle_t*)ctx;

    if(CHECK_ISP_HW_V21()) {
        attr->atrrV21.mode = gamma_handle->agammaAttr.atrrV21.mode;
        memcpy(&attr->atrrV21.stManual, &gamma_handle->agammaAttr.atrrV21.stManual, sizeof(Agamma_api_manualV21_t));
        memcpy(&attr->atrrV21.stFast, &gamma_handle->agammaAttr.atrrV21.stFast, sizeof(Agamma_api_Fast_t));
    }
    else if(CHECK_ISP_HW_V30()) {
        attr->atrrV30.mode = gamma_handle->agammaAttr.atrrV30.mode;
        memcpy(&attr->atrrV30.stManual, &gamma_handle->agammaAttr.atrrV30.stManual, sizeof(Agamma_api_manualV30_t));
        memcpy(&attr->atrrV30.stFast, &gamma_handle->agammaAttr.atrrV30.stFast, sizeof(Agamma_api_Fast_t));
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}
