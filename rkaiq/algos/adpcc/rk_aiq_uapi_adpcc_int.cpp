#include "rk_aiq_uapi_adpcc_int.h"
#include "adpcc/rk_aiq_types_adpcc_algo_prvt.h"

XCamReturn
rk_aiq_uapi_adpcc_SetAttrib(RkAiqAlgoContext *ctx,
                            rk_aiq_dpcc_attrib_V20_t *attr,
                            bool need_sync)
{
    AdpccContext_t* pAdpccCtx = (AdpccContext_t*)ctx;

    pAdpccCtx->eMode = attr->eMode;
    pAdpccCtx->stAuto = attr->stAuto;
    pAdpccCtx->stManual = attr->stManual;
    //memcpy(&pAdpccCtx->stTool, &attr->stTool, sizeof(CalibDbV2_Dpcc_t));
    pAdpccCtx->isReCal_ = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_adpcc_GetAttrib(const RkAiqAlgoContext *ctx,
                            rk_aiq_dpcc_attrib_V20_t *attr)
{

    AdpccContext_t* pAdpccCtx = (AdpccContext_t*)ctx;

    attr->eMode = pAdpccCtx->eMode;
    memcpy(&attr->stAuto, &pAdpccCtx->stAuto, sizeof(Adpcc_Auto_Attr_t));
    memcpy(&attr->stManual, &pAdpccCtx->stManual, sizeof(Adpcc_Manual_Attr_t));
    //memcpy(&attr->stTool, &pAdpccCtx->stTool, sizeof(CalibDbV2_Dpcc_t));

    return XCAM_RETURN_NO_ERROR;
}



