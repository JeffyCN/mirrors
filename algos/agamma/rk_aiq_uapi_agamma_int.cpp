#include "rk_aiq_uapi_agamma_int.h"
#include "agamma/rk_aiq_types_agamma_algo_prvt.h"


void
rk_aiq_uapi_agamma_SetTool
(
    CalibDbV2_gamma_t* pStore,
    CalibDbV2_gamma_t* pInput
)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    pStore->GammaTuningPara.Gamma_en = pInput->GammaTuningPara.Gamma_en;
    pStore->GammaTuningPara.Gamma_out_offset = pInput->GammaTuningPara.Gamma_out_offset;
    pStore->GammaTuningPara.Gamma_out_segnum = pInput->GammaTuningPara.Gamma_out_segnum;
    for(int i = 0; i < 45; i++)
        pStore->GammaTuningPara.Gamma_curve[i] = pInput->GammaTuningPara.Gamma_curve[i];

    LOG1_AGAMMA("EXIT: %s \n", __func__);

}

XCamReturn
rk_aiq_uapi_agamma_SetAttrib(RkAiqAlgoContext *ctx,
                             rk_aiq_gamma_attrib_V2_t attr,
                             bool need_sync)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    AgammaHandle_t *gamma_handle = (AgammaHandle_t *)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    gamma_handle->agammaAttr.mode = attr.mode;
    if(attr.mode == RK_AIQ_GAMMA_MODE_MANUAL)
        memcpy(&gamma_handle->agammaAttr.stManual, &attr.stManual, sizeof(Agamma_api_manual_t));
    if(attr.mode == RK_AIQ_GAMMA_MODE_TOOL)
        rk_aiq_uapi_agamma_SetTool(&gamma_handle->agammaAttr.stTool, &attr.stTool);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn
rk_aiq_uapi_agamma_GetAttrib(const RkAiqAlgoContext *ctx,
                             rk_aiq_gamma_attrib_V2_t *attr)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    AgammaHandle_t* gamma_handle = (AgammaHandle_t*)ctx;

    attr->mode = gamma_handle->agammaAttr.mode;
    memcpy(&attr->stManual, &gamma_handle->agammaAttr.stManual, sizeof(Agamma_api_manual_t));
    rk_aiq_uapi_agamma_SetTool(&attr->stTool, &gamma_handle->agammaAttr.stTool);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}




