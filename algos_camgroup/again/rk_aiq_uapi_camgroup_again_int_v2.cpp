#include "again2/rk_aiq_uapi_camgroup_again_int_v2.h"
#include "rk_aiq_types_camgroup_again_prvt.h"

#if 1
#define ACNRV2_CHROMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_camgroup_againV2_SetAttrib(RkAiqAlgoContext *ctx,
                                       rk_aiq_gain_attrib_v2_t *attr,
                                       bool need_sync)
{
    CamGroup_Again_Contex_t *pGroupCtx = (CamGroup_Again_Contex_t *)ctx;
    Again_Context_V2_t* pCtx = pGroupCtx->again_contex_v2;

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
rk_aiq_uapi_camgroup_againV2_GetAttrib(const RkAiqAlgoContext *ctx,
                                       rk_aiq_gain_attrib_v2_t *attr)
{

    CamGroup_Again_Contex_t *pGroupCtx = (CamGroup_Again_Contex_t *)ctx;
    Again_Context_V2_t* pCtx = pGroupCtx->again_contex_v2;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}



#endif

