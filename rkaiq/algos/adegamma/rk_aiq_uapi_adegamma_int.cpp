#include "rk_aiq_uapi_adegamma_int.h"
#include "adegamma/rk_aiq_types_adegamma_algo_prvt.h"


void
rk_aiq_uapi_adegama_SetTool
(
    CalibDbV2_Adegmma_t* pStore,
    CalibDbV2_Adegmma_t* pInput
)
{
    LOG1_ADEGAMMA(" %s: Enter\n", __func__);

    pStore->DegammaTuningPara.degamma_en = pInput->DegammaTuningPara.degamma_en;
    for(int i = 0; i < 17; i++) {
        pStore->DegammaTuningPara.X_axis[i] = pInput->DegammaTuningPara.X_axis[i];
        pStore->DegammaTuningPara.curve_R[i] = pInput->DegammaTuningPara.curve_R[i];
        pStore->DegammaTuningPara.curve_G[i] = pInput->DegammaTuningPara.curve_G[i];
        pStore->DegammaTuningPara.curve_B[i] = pInput->DegammaTuningPara.curve_B[i];
    }

    LOG1_ADEGAMMA(" %s: Exit\n", __func__);
}

XCamReturn
rk_aiq_uapi_adegamma_SetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_degamma_attrib_t attr,
                               bool need_sync)
{
    LOG1_ADEGAMMA(" %s: Enter\n", __func__);

    AdegammaHandle_t *degamma_handle = (AdegammaHandle_t *)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    degamma_handle->adegammaAttr.mode = attr.mode;
    if(attr.mode == RK_AIQ_DEGAMMA_MODE_MANUAL)
        memcpy(&degamma_handle->adegammaAttr.stManual, &attr.stManual, sizeof(Adegamma_api_manual_t));
    if(attr.mode == RK_AIQ_DEGAMMA_MODE_TOOL)
        rk_aiq_uapi_adegama_SetTool(&degamma_handle->adegammaAttr.stTool, &attr.stTool);
    degamma_handle->isReCal_ = true;

    LOG1_ADEGAMMA(" %s: Exit\n", __func__);
    return ret;
}

XCamReturn
rk_aiq_uapi_adegamma_GetAttrib(const RkAiqAlgoContext *ctx,
                               rk_aiq_degamma_attrib_t *attr)
{
    LOG1_ADEGAMMA(" %s: Enter\n", __func__);

    AdegammaHandle_t* degamma_handle = (AdegammaHandle_t*)ctx;

    attr->mode = degamma_handle->adegammaAttr.mode;
    attr->Scene_mode = degamma_handle->adegammaAttr.Scene_mode;
    memcpy(&attr->stManual, &degamma_handle->adegammaAttr.stManual, sizeof(Adegamma_api_manual_t));
    rk_aiq_uapi_adegama_SetTool(&attr->stTool, &degamma_handle->adegammaAttr.stTool);

    LOG1_ADEGAMMA(" %s: Exit\n", __func__);
    return XCAM_RETURN_NO_ERROR;
}




