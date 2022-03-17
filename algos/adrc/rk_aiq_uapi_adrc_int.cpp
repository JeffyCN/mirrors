#include "rk_aiq_uapi_adrc_int.h"
#include "adrc/rk_aiq_types_adrc_algo_prvt.h"

bool
IfDrcDataEqu
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
rk_aiq_uapi_adrc_SetTool
(
    CalibDbV2_drc_t* pStore,
    CalibDbV2_drc_t* pInput
)
{
    LOG1_ATMO("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //enbale
    pStore->DrcTuningPara.Enable = pInput->DrcTuningPara.Enable;

    //drc gain
    int DrcGainLen[4];
    DrcGainLen[0] = pInput->DrcTuningPara.DrcGain.EnvLv_len;
    DrcGainLen[1] = pInput->DrcTuningPara.DrcGain.DrcGain_len;
    DrcGainLen[2] = pInput->DrcTuningPara.DrcGain.Alpha_len;
    DrcGainLen[3] = pInput->DrcTuningPara.DrcGain.Clip_len;
    bool DrcGainLenEqu = IfDrcDataEqu(DrcGainLen, 4);
    if(!DrcGainLenEqu) {
        LOGE_ATMO("%s: Input DRC DrcGain Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->DrcTuningPara.DrcGain.EnvLv_len != pInput->DrcTuningPara.DrcGain.EnvLv_len) {
        //free
        free(pStore->DrcTuningPara.DrcGain.EnvLv);
        free(pStore->DrcTuningPara.DrcGain.DrcGain);
        free(pStore->DrcTuningPara.DrcGain.Alpha);
        free(pStore->DrcTuningPara.DrcGain.Clip);
        //malloc
        pStore->DrcTuningPara.DrcGain.EnvLv =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.DrcGain.EnvLv_len);
        pStore->DrcTuningPara.DrcGain.DrcGain =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.DrcGain.EnvLv_len);
        pStore->DrcTuningPara.DrcGain.Alpha =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.DrcGain.EnvLv_len);
        pStore->DrcTuningPara.DrcGain.Clip =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.DrcGain.EnvLv_len);
        //store len
        pStore->DrcTuningPara.DrcGain.EnvLv_len = pInput->DrcTuningPara.DrcGain.EnvLv_len;
        pStore->DrcTuningPara.DrcGain.DrcGain_len = pInput->DrcTuningPara.DrcGain.EnvLv_len;
        pStore->DrcTuningPara.DrcGain.Alpha_len = pInput->DrcTuningPara.DrcGain.EnvLv_len;
        pStore->DrcTuningPara.DrcGain.Clip_len = pInput->DrcTuningPara.DrcGain.EnvLv_len;
    }
    memcpy(pStore->DrcTuningPara.DrcGain.EnvLv,
           pInput->DrcTuningPara.DrcGain.EnvLv, sizeof(float)*pInput->DrcTuningPara.DrcGain.EnvLv_len);
    memcpy(pStore->DrcTuningPara.DrcGain.DrcGain,
           pInput->DrcTuningPara.DrcGain.DrcGain, sizeof(float)*pInput->DrcTuningPara.DrcGain.EnvLv_len);
    memcpy(pStore->DrcTuningPara.DrcGain.Alpha,
           pInput->DrcTuningPara.DrcGain.Alpha, sizeof(float)*pInput->DrcTuningPara.DrcGain.EnvLv_len);
    memcpy(pStore->DrcTuningPara.DrcGain.Clip,
           pInput->DrcTuningPara.DrcGain.Clip, sizeof(float)*pInput->DrcTuningPara.DrcGain.EnvLv_len);

    //Hi Lit
    int HiLitLen[3];
    DrcGainLen[0] = pInput->DrcTuningPara.HiLight.EnvLv_len;
    DrcGainLen[1] = pInput->DrcTuningPara.HiLight.Strength_len;
    bool HiLitLenEqu = IfDrcDataEqu(HiLitLen, 3);
    if(!HiLitLenEqu) {
        LOGE_ATMO("%s: Input DRC HiLit Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->DrcTuningPara.HiLight.EnvLv_len != pInput->DrcTuningPara.HiLight.EnvLv_len) {
        //free
        free(pStore->DrcTuningPara.HiLight.EnvLv);
        free(pStore->DrcTuningPara.HiLight.Strength);
        //malloc
        pStore->DrcTuningPara.HiLight.EnvLv =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.HiLight.EnvLv_len);
        pStore->DrcTuningPara.HiLight.Strength =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.HiLight.EnvLv_len);
        //store len
        pStore->DrcTuningPara.HiLight.EnvLv_len = pInput->DrcTuningPara.HiLight.EnvLv_len;
        pStore->DrcTuningPara.HiLight.Strength_len = pInput->DrcTuningPara.HiLight.EnvLv_len;
    }
    memcpy(pStore->DrcTuningPara.HiLight.EnvLv,
           pInput->DrcTuningPara.HiLight.EnvLv, sizeof(float)*pInput->DrcTuningPara.HiLight.EnvLv_len);
    memcpy(pStore->DrcTuningPara.HiLight.Strength,
           pInput->DrcTuningPara.HiLight.Strength, sizeof(float)*pInput->DrcTuningPara.HiLight.EnvLv_len);

    //local
    int LocalLen[4];
    LocalLen[0] = pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
    LocalLen[1] = pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit_len;
    LocalLen[2] = pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast_len;
    LocalLen[3] = pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast_len;
    bool LocalLenEqu = IfDrcDataEqu(LocalLen, 4);
    if(!LocalLenEqu) {
        LOGE_ATMO("%s: Input DRC Local Data length is NOT EQUAL !!!\n", __FUNCTION__ );
        return XCAM_RETURN_ERROR_FAILED;
    }
    if(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len != pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len) {
        //free
        free(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv);
        free(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit);
        free(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast);
        free(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast);
        //malloc
        pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);
        pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);
        pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);
        pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast =
            (float *) malloc(sizeof(float) * pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);
        //store len
        pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len = pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
        pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit_len = pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
        pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast_len = pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
        pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast_len = pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len;
    }
    memcpy(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv,
           pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv, sizeof(float)*pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);
    memcpy(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit,
           pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit, sizeof(float)*pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);
    memcpy(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast,
           pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast, sizeof(float)*pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);
    memcpy(pStore->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast,
           pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast, sizeof(float)*pInput->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len);

    //local other
    pStore->DrcTuningPara.LocalTMOSetting.curPixWeit = pInput->DrcTuningPara.LocalTMOSetting.curPixWeit;
    pStore->DrcTuningPara.LocalTMOSetting.preFrameWeit = pInput->DrcTuningPara.LocalTMOSetting.preFrameWeit;
    pStore->DrcTuningPara.LocalTMOSetting.Range_force_sgm = pInput->DrcTuningPara.LocalTMOSetting.Range_force_sgm;
    pStore->DrcTuningPara.LocalTMOSetting.Range_sgm_cur = pInput->DrcTuningPara.LocalTMOSetting.Range_sgm_cur;
    pStore->DrcTuningPara.LocalTMOSetting.Range_sgm_pre = pInput->DrcTuningPara.LocalTMOSetting.Range_sgm_pre;
    pStore->DrcTuningPara.LocalTMOSetting.Space_sgm_cur = pInput->DrcTuningPara.LocalTMOSetting.Space_sgm_cur;
    pStore->DrcTuningPara.LocalTMOSetting.Space_sgm_pre = pInput->DrcTuningPara.LocalTMOSetting.Space_sgm_pre;

    //other
    pStore->DrcTuningPara.Edge_Weit = pInput->DrcTuningPara.Edge_Weit;
    pStore->DrcTuningPara.IIR_frame = pInput->DrcTuningPara.IIR_frame;
    pStore->DrcTuningPara.Tolerance = pInput->DrcTuningPara.Tolerance;
    pStore->DrcTuningPara.damp = pInput->DrcTuningPara.damp;
    for(int i = 0; i < ADRC_Y_NUM; i++)
        pStore->DrcTuningPara.Scale_y[i] = pInput->DrcTuningPara.Scale_y[i];

    LOG1_ATMO("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn
rk_aiq_uapi_adrc_SetAttrib(RkAiqAlgoContext *ctx,
                           drc_attrib_t attr,
                           bool need_sync)
{
    LOG1_ATMO("ENTER: %s \n", __func__);
    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)(ctx->AdrcInstConfig.hAdrc);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pAdrcCtx->drcAttr.Enable = attr.Enable;
    pAdrcCtx->drcAttr.opMode = attr.opMode;
    if(attr.opMode == DRC_OPMODE_MANU)
        memcpy(&pAdrcCtx->drcAttr.stManual, &attr.stManual, sizeof(mdrcAttr_t));
    /*if(attr.opMode == DRC_OPMODE_AUTO)
        ret = rk_aiq_uapi_adrc_SetTool(&pAdrcCtx->drcAttr.stAuto, &attr.stAuto);*/
    if(attr.opMode == DRC_OPMODE_DRC_GAIN)
        memcpy(&pAdrcCtx->drcAttr.stManual.DrcGain, &attr.stManual.DrcGain, sizeof(mDrcGain_t));
    if(attr.opMode == DRC_OPMODE_HILIT)
        memcpy(&pAdrcCtx->drcAttr.stManual.HiLit, &attr.stManual.HiLit, sizeof(mDrcHiLit_t));
    if(attr.opMode == DRC_OPMODE_LOCAL_TMO)
        memcpy(&pAdrcCtx->drcAttr.stManual.Local, &attr.stManual.Local, sizeof(mDrcLocal_t));
    if(attr.opMode == DRC_OPMODE_COMPRESS)
        memcpy(&pAdrcCtx->drcAttr.stManual.Compress, &attr.stManual.Compress, sizeof(mDrcCompress_t));

    LOG1_ATMO("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn
rk_aiq_uapi_adrc_GetAttrib(RkAiqAlgoContext *ctx,
                           drc_attrib_t *attr)
{
    LOG1_ATMO("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)ctx->AdrcInstConfig.hAdrc;

    attr->Enable = pAdrcCtx->drcAttr.Enable;
    attr->opMode = pAdrcCtx->drcAttr.opMode;
    memcpy(&attr->Info, &pAdrcCtx->drcAttr.Info, sizeof(DrcInfo_t));
    memcpy(&attr->stManual, &pAdrcCtx->drcAttr.stManual, sizeof(mdrcAttr_t));
    //ret = rk_aiq_uapi_adrc_SetTool(&attr->stAuto, &pAdrcCtx->drcAttr.stAuto);

    LOG1_ATMO("EXIT: %s \n", __func__);
    return XCAM_RETURN_NO_ERROR;
}




