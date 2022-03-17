#include "rk_aiq_uapi_atmo_int.h"
#include "rk_aiq_types_atmo_algo_prvt.h"

bool
IfTmoDataEqu
(
    int* pInput,
    int len
)
{
    bool equ = true;

    for(int i = 0; i < len - 1; i++)
        if(pInput[i] != pInput[i + 1])
            equ = false;

    return equ;
}

XCamReturn
rk_aiq_uapi_atmo_SetTool
(
    CalibDbV2_tmo_t* pStore,
    CalibDbV2_tmo_t* pInput
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //global luma
    int GlobalLumaLen[3];
    GlobalLumaLen[0] = pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    GlobalLumaLen[1] = pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO_len;
    GlobalLumaLen[2] = pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength_len;
    bool GlobalLumaEqu = IfTmoDataEqu(GlobalLumaLen, 3);
    if(!GlobalLumaEqu) {
        LOGE_ATMO("%s: Input TMO Global Luma Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }

    if(pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len != pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len) {
        free(pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv);
        free(pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO);
        free(pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength);
        pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len);
        pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len);
        pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len);
    }
    pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len = pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO_len = pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength_len = pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    memcpy(pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv,
           pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv, sizeof(float)*pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len);
    memcpy(pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO,
           pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO, sizeof(float)*pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len);
    memcpy(pStore->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength,
           pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength, sizeof(float)*pInput->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len);

    //high light
    int HighLightLen[3];
    HighLightLen[0] = pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len;
    HighLightLen[1] = pInput->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv_len;
    HighLightLen[2] = pInput->TmoTuningPara.DetailsHighLight.HighLightData.Strength_len;
    bool HighLightLenEqu = IfTmoDataEqu(HighLightLen, 3);
    if(!HighLightLenEqu) {
        LOGE_ATMO("%s: Input TMO Hight Light Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len != pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len) {
        free(pStore->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf);
        free(pStore->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv);
        free(pStore->TmoTuningPara.DetailsHighLight.HighLightData.Strength);
        pStore->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len);
        pStore->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len);
        pStore->TmoTuningPara.DetailsHighLight.HighLightData.Strength =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len);
    }
    pStore->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len = pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len;
    pStore->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv_len = pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len;
    pStore->TmoTuningPara.DetailsHighLight.HighLightData.Strength_len = pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len;
    memcpy(pStore->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf,
           pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf, sizeof(float)*pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len);
    memcpy(pStore->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv,
           pInput->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv, sizeof(float)*pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len);
    memcpy(pStore->TmoTuningPara.DetailsHighLight.HighLightData.Strength,
           pInput->TmoTuningPara.DetailsHighLight.HighLightData.Strength, sizeof(float)*pInput->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len);

    //low light
    int LowLightLen[4];
    LowLightLen[0] = pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len;
    LowLightLen[1] = pInput->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf_len;
    LowLightLen[2] = pInput->TmoTuningPara.DetailsLowLight.LowLightData.ISO_len;
    LowLightLen[3] = pInput->TmoTuningPara.DetailsLowLight.LowLightData.Strength_len;
    bool LowLightLenEqu = IfTmoDataEqu(LowLightLen, 4);
    if(!LowLightLenEqu) {
        LOGE_ATMO("%s: Input TMO Low Light Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len != pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len) {
        free(pStore->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma);
        free(pStore->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf);
        free(pStore->TmoTuningPara.DetailsLowLight.LowLightData.ISO);
        free(pStore->TmoTuningPara.DetailsLowLight.LowLightData.Strength);
        pStore->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len);
        pStore->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len);
        pStore->TmoTuningPara.DetailsLowLight.LowLightData.ISO =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len);
        pStore->TmoTuningPara.DetailsLowLight.LowLightData.Strength =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len);
    }
    pStore->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len = pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len;
    pStore->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf_len = pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len;
    pStore->TmoTuningPara.DetailsLowLight.LowLightData.ISO_len = pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len;
    pStore->TmoTuningPara.DetailsLowLight.LowLightData.Strength_len = pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len;
    memcpy(pStore->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma,
           pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma, sizeof(float)*pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len);
    memcpy(pStore->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf,
           pInput->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf, sizeof(float)*pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len);
    memcpy(pStore->TmoTuningPara.DetailsLowLight.LowLightData.ISO,
           pInput->TmoTuningPara.DetailsLowLight.LowLightData.ISO, sizeof(float)*pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len);
    memcpy(pStore->TmoTuningPara.DetailsLowLight.LowLightData.Strength,
           pInput->TmoTuningPara.DetailsLowLight.LowLightData.Strength, sizeof(float)*pInput->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len);

    //global tmo
    int GlobalTMOLen[3];
    GlobalTMOLen[0] = pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len;
    GlobalTMOLen[1] = pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv_len;
    GlobalTMOLen[2] = pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength_len;
    bool GlobalTMOLenEqu = IfTmoDataEqu(GlobalTMOLen, 3);
    if(!GlobalTMOLenEqu) {
        LOGE_ATMO("%s: Input TMO GlobalTMO Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len != pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len) {
        free(pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange);
        free(pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv);
        free(pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength);
        pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len);
        pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len);
        pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len);
    }
    pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len = pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len;
    pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv_len = pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len;
    pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength_len = pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len;
    memcpy(pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange,
           pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange, sizeof(float)*pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len);
    memcpy(pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv,
           pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv, sizeof(float)*pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len);
    memcpy(pStore->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength,
           pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength, sizeof(float)*pInput->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len);

    //local tmo
    int LocalTMOLen[3];
    LocalTMOLen[0] = pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len;
    LocalTMOLen[1] = pInput->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv_len;
    LocalTMOLen[2] = pInput->TmoTuningPara.LocalTMO.LocalTmoData.Strength_len;
    bool LocalTMOLenEqu = IfTmoDataEqu(LocalTMOLen, 3);
    if(!LocalTMOLenEqu) {
        LOGE_ATMO("%s: Input TMO LocalTMO Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len != pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len) {
        //free
        free(pStore->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange);
        free(pStore->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv);
        free(pStore->TmoTuningPara.LocalTMO.LocalTmoData.Strength);
        //malloc
        pStore->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len);
        pStore->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len);
        pStore->TmoTuningPara.LocalTMO.LocalTmoData.Strength =
            (float *) malloc(sizeof(float) * pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len);
        //store len
        pStore->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len = pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len;
        pStore->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv_len = pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len;
        pStore->TmoTuningPara.LocalTMO.LocalTmoData.Strength_len = pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len;
    }
    memcpy(pStore->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange,
           pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange, sizeof(float)*pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len);
    memcpy(pStore->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv,
           pInput->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv, sizeof(float)*pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len);
    memcpy(pStore->TmoTuningPara.LocalTMO.LocalTmoData.Strength,
           pInput->TmoTuningPara.LocalTMO.LocalTmoData.Strength, sizeof(float)*pInput->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len);


    pStore->TmoTuningPara.Enable = pInput->TmoTuningPara.Enable;
    pStore->TmoTuningPara.GlobalLuma.Mode = pInput->TmoTuningPara.GlobalLuma.Mode;
    pStore->TmoTuningPara.GlobalLuma.Tolerance = pInput->TmoTuningPara.GlobalLuma.Tolerance;
    pStore->TmoTuningPara.DetailsHighLight.Mode = pInput->TmoTuningPara.DetailsHighLight.Mode;
    pStore->TmoTuningPara.DetailsHighLight.Tolerance = pInput->TmoTuningPara.DetailsHighLight.Tolerance;
    pStore->TmoTuningPara.DetailsLowLight.Mode = pInput->TmoTuningPara.DetailsLowLight.Mode;
    pStore->TmoTuningPara.DetailsLowLight.Tolerance = pInput->TmoTuningPara.DetailsLowLight.Tolerance;
    pStore->TmoTuningPara.LocalTMO.Mode = pInput->TmoTuningPara.LocalTMO.Mode;
    pStore->TmoTuningPara.LocalTMO.Tolerance = pInput->TmoTuningPara.LocalTMO.Tolerance;
    pStore->TmoTuningPara.GlobaTMO.Mode = pInput->TmoTuningPara.GlobaTMO.Mode;
    pStore->TmoTuningPara.GlobaTMO.Tolerance = pInput->TmoTuningPara.GlobaTMO.Tolerance;
    pStore->TmoTuningPara.GlobaTMO.Enable = pInput->TmoTuningPara.GlobaTMO.Enable;
    pStore->TmoTuningPara.GlobaTMO.IIR = pInput->TmoTuningPara.GlobaTMO.IIR;
    pStore->TmoTuningPara.damp = pInput->TmoTuningPara.damp;

    return ret;
}

XCamReturn
rk_aiq_uapi_atmo_SetAttrib
(
    RkAiqAlgoContext* ctx,
    atmo_attrib_t attr,
    bool need_sync
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(ctx == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AtmoContext_t* pAtmoCtx = (AtmoContext_t*)(ctx->AtmoInstConfig.hAtmo);

    //Todo
    pAtmoCtx->tmoAttr.opMode = attr.opMode;
    if(attr.opMode == TMO_OPMODE_SET_LEVEL)
        memcpy(&pAtmoCtx->tmoAttr.stSetLevel, &attr.stSetLevel, sizeof(FastMode_t));
    if(attr.opMode == TMO_OPMODE_DARKAREA)
        memcpy(&pAtmoCtx->tmoAttr.stDarkArea, &attr.stDarkArea, sizeof(DarkArea_t));
    if(attr.opMode == TMO_OPMODE_TOOL)
        ret = rk_aiq_uapi_atmo_SetTool(&pAtmoCtx->tmoAttr.stTool, &attr.stTool);

    if (attr.opMode == TMO_OPMODE_AUTO) {
        pAtmoCtx->tmoAttr.stAuto.bUpdateTmo = attr.stAuto.bUpdateTmo;
        memcpy(&pAtmoCtx->tmoAttr.stAuto.stTmoAuto, &attr.stAuto.stTmoAuto, sizeof(atmoAttr_t));
    }
    else
        pAtmoCtx->tmoAttr.stAuto.bUpdateTmo = false;

    if (attr.opMode == TMO_OPMODE_MANU) {
        pAtmoCtx->tmoAttr.stManual.bUpdateTmo = attr.stManual.bUpdateTmo;
        memcpy(&pAtmoCtx->tmoAttr.stManual.stTmoManual, &attr.stManual.stTmoManual, sizeof(mtmoAttr_t));
    }
    else
        pAtmoCtx->tmoAttr.stManual.bUpdateTmo = false;

    return ret;
}

XCamReturn
rk_aiq_uapi_atmo_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    atmo_attrib_t* attr
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(ctx == NULL || attr == NULL) {
        LOGE_ATMO("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    AtmoContext_t* pAtmoCtx = (AtmoContext_t*)ctx->AtmoInstConfig.hAtmo;

    attr->opMode = pAtmoCtx->tmoAttr.opMode;
    memcpy(&attr->stAuto.stTmoAuto, &pAtmoCtx->tmoAttr.stAuto.stTmoAuto, sizeof(atmoAttr_t));
    memcpy(&attr->stManual.stTmoManual, &pAtmoCtx->tmoAttr.stManual.stTmoManual, sizeof(mtmoAttr_t));
    memcpy(&attr->stSetLevel, &pAtmoCtx->tmoAttr.stSetLevel, sizeof(FastMode_t));
    memcpy(&attr->stDarkArea, &pAtmoCtx->tmoAttr.stDarkArea, sizeof(DarkArea_t));
    ret = rk_aiq_uapi_atmo_SetTool(&attr->stTool, &pAtmoCtx->tmoAttr.stTool);
    memcpy(&attr->stTool, &pAtmoCtx->tmoAttr.stTool, sizeof(CalibDbV2_tmo_t));
    memcpy(&attr->CtlInfo, &pAtmoCtx->tmoAttr.CtlInfo, sizeof(TmoCurrCtlData_t));
    memcpy(&attr->RegInfo, &pAtmoCtx->tmoAttr.RegInfo, sizeof(TmoCurrRegData_t));

    return ret;
}


