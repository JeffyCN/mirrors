#include "again2/rk_aiq_uapi_again_int_v2.h"
#include "again2/rk_aiq_types_again_algo_prvt_v2.h"
#include "again2/rk_aiq_again_algo_gain_v2.h"


XCamReturn
rk_aiq_uapi_againV2_SetAttrib(RkAiqAlgoContext *ctx,
                              rk_aiq_gain_attrib_v2_t *attr,
                              bool need_sync)
{

    Again_Context_V2_t* pCtx = (Again_Context_V2_t*)ctx;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == AGAINV2_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == AGAINV2_OP_MODE_MANUAL) {
        pCtx->stManual.stSelect = attr->stManual.stSelect;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_againV2_GetAttrib(const RkAiqAlgoContext *ctx,
                              rk_aiq_gain_attrib_v2_t *attr)
{

    Again_Context_V2_t* pCtx = (Again_Context_V2_t*)ctx;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}


