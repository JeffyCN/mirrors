#include "rk_aiq_uapi_ablc_int.h"
#include "ablc/rk_aiq_types_ablc_algo_prvt.h"


bool
IfBlcDataEqu
(
    Blc_data_t* pInput
)
{
    bool equ = true;
    int pInputLen[5];
    pInputLen[0] = pInput->ISO_len;
    pInputLen[1] = pInput->R_Channel_len;
    pInputLen[2] = pInput->Gr_Channel_len;
    pInputLen[3] = pInput->Gb_Channel_len;
    pInputLen[4] = pInput->B_Channel_len;

    for(int i = 0; i < 4; i++)
        if(pInputLen[i] != pInputLen[i + 1])
            equ = false;

    return equ;
}

XCamReturn
rk_aiq_uapi_ablc_SetTool
(
    CalibDbV2_Ablc_t* pStore,
    CalibDbV2_Ablc_t* pInput
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pStore->BlcTuningPara.enable = pInput->BlcTuningPara.enable;

    bool Ifequ = IfBlcDataEqu(&pInput->BlcTuningPara.BLC_Data);
    if(!Ifequ) {
        LOGE_ABLC("%s: Input BLC Data lens is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }

    if(pStore->BlcTuningPara.BLC_Data.ISO_len != pInput->BlcTuningPara.BLC_Data.ISO_len) {
        free(pStore->BlcTuningPara.BLC_Data.ISO);
        free(pStore->BlcTuningPara.BLC_Data.R_Channel);
        free(pStore->BlcTuningPara.BLC_Data.Gr_Channel);
        free(pStore->BlcTuningPara.BLC_Data.Gb_Channel);
        free(pStore->BlcTuningPara.BLC_Data.B_Channel);
        pStore->BlcTuningPara.BLC_Data.ISO = (float *) malloc(sizeof(float) * pInput->BlcTuningPara.BLC_Data.ISO_len);
        pStore->BlcTuningPara.BLC_Data.R_Channel = (float *) malloc(sizeof(float) * pInput->BlcTuningPara.BLC_Data.ISO_len);
        pStore->BlcTuningPara.BLC_Data.Gr_Channel = (float *) malloc(sizeof(float) * pInput->BlcTuningPara.BLC_Data.ISO_len);
        pStore->BlcTuningPara.BLC_Data.Gb_Channel = (float *) malloc(sizeof(float) * pInput->BlcTuningPara.BLC_Data.ISO_len);
        pStore->BlcTuningPara.BLC_Data.B_Channel = (float *) malloc(sizeof(float) * pInput->BlcTuningPara.BLC_Data.ISO_len);
    }
    pStore->BlcTuningPara.BLC_Data.ISO_len = pInput->BlcTuningPara.BLC_Data.ISO_len;
    pStore->BlcTuningPara.BLC_Data.R_Channel_len = pInput->BlcTuningPara.BLC_Data.ISO_len;
    pStore->BlcTuningPara.BLC_Data.Gr_Channel_len = pInput->BlcTuningPara.BLC_Data.ISO_len;
    pStore->BlcTuningPara.BLC_Data.Gb_Channel_len = pInput->BlcTuningPara.BLC_Data.ISO_len;
    pStore->BlcTuningPara.BLC_Data.B_Channel_len = pInput->BlcTuningPara.BLC_Data.ISO_len;
    memcpy(pStore->BlcTuningPara.BLC_Data.ISO, pInput->BlcTuningPara.BLC_Data.ISO, sizeof(float)*pInput->BlcTuningPara.BLC_Data.ISO_len);
    memcpy(pStore->BlcTuningPara.BLC_Data.R_Channel, pInput->BlcTuningPara.BLC_Data.R_Channel, sizeof(float)*pInput->BlcTuningPara.BLC_Data.ISO_len);
    memcpy(pStore->BlcTuningPara.BLC_Data.Gr_Channel, pInput->BlcTuningPara.BLC_Data.Gr_Channel, sizeof(float)*pInput->BlcTuningPara.BLC_Data.ISO_len);
    memcpy(pStore->BlcTuningPara.BLC_Data.Gb_Channel, pInput->BlcTuningPara.BLC_Data.Gb_Channel, sizeof(float)*pInput->BlcTuningPara.BLC_Data.ISO_len);
    memcpy(pStore->BlcTuningPara.BLC_Data.B_Channel, pInput->BlcTuningPara.BLC_Data.B_Channel, sizeof(float)*pInput->BlcTuningPara.BLC_Data.ISO_len);


    return ret;
}

XCamReturn
rk_aiq_uapi_ablc_SetAttrib(RkAiqAlgoContext *ctx,
                           rk_aiq_blc_attrib_t *attr,
                           bool need_sync)
{
    AblcContext_t* pAblcCtx = (AblcContext_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    pAblcCtx->attr.eMode = attr->eMode;
    if(attr->eMode == ABLC_OP_MODE_API_MANUAL)
        pAblcCtx->attr.stManual = attr->stManual;
    if(attr->eMode == ABLC_OP_MODE_API_TOOL)
        ret = rk_aiq_uapi_ablc_SetTool(&pAblcCtx->attr.stTool, &attr->stTool);

    pAblcCtx->isReCalculate |= 1;
    return ret;
}

XCamReturn
rk_aiq_uapi_ablc_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_blc_attrib_t *attr)
{

    AblcContext_t* pAblcCtx = (AblcContext_t*)ctx;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    attr->eMode = pAblcCtx->attr.eMode;
    memcpy(&attr->stManual, &pAblcCtx->attr.stManual, sizeof(AblcManualAttr_t));
    ret = rk_aiq_uapi_ablc_SetTool(&attr->stTool, &pAblcCtx->attr.stTool);

    return ret;
}



