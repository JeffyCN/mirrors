#include "rk_aiq_algo_ablc_itf.h"
#include "rk_aiq_ablc_algo.h"


AblcResult_t Ablc_html_params_init(AblcConfig_t *pParams)
{
    AblcResult_t ret = ABLC_RET_SUCCESS;

    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        ret = ABLC_RET_NULL_POINTER;
        LOGE_ADPCC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ret;
    }

    int isoBase = 50;

    pParams->enable = 1;
    for(int i = 0; i < pParams->len; i++) {
        pParams->iso[i] = isoBase * (1 << i);
        pParams->blc_r[i] = 200;
        pParams->blc_gr[i] = 200;
        pParams->blc_gb[i] = 200;
        pParams->blc_b[i] = 200;
    }

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ret;
}

AblcResult_t AblcConfig(AblcConfig_t *pParams, CalibDbV2_Ablc_t* pBlcCalib)
{
    AblcResult_t res = ABLC_RET_SUCCESS;

    if(pParams == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    if(pBlcCalib == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    pParams->enable = pBlcCalib->BlcTuningPara.enable;
    for(int i = 0; i < pParams->len; i++) {
        pParams->iso[i] = pBlcCalib->BlcTuningPara.BLC_Data.ISO[i];
        pParams->blc_r[i] = pBlcCalib->BlcTuningPara.BLC_Data.R_Channel[i];
        pParams->blc_gr[i] = pBlcCalib->BlcTuningPara.BLC_Data.Gr_Channel[i];
        pParams->blc_gb[i] = pBlcCalib->BlcTuningPara.BLC_Data.Gb_Channel[i];
        pParams->blc_b[i] = pBlcCalib->BlcTuningPara.BLC_Data.B_Channel[i];
    }
    LOGD_ABLC("%s(%d): Ablc en:%d blc:%f %f %f %f \n",
              __FUNCTION__, __LINE__,
              pParams->enable,
              pParams->blc_r[0],
              pParams->blc_gr[0],
              pParams->blc_gb[0],
              pParams->blc_gb[0]);

    LOG1_ABLC("%s(%d)\n", __FUNCTION__, __LINE__);
    return res;
}

AblcResult_t Ablc_Api_Manual_Select_Params(AblcManualAttr_t *pParams, AblcProc_t *pSelect)
{
    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    LOGD_ABLC("%s:(%d) Ablc in Manual api!!! \n", __FUNCTION__, __LINE__);

    if(pParams == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    pSelect->enable = pParams->enable;
    pSelect->blc_b = pParams->blc_b;
    pSelect->blc_gb = pParams->blc_gb;
    pSelect->blc_gr = pParams->blc_gr;
    pSelect->blc_r = pParams->blc_r;

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_RET_SUCCESS;
}

AblcResult_t Ablc_Select_Params_By_ISO(AblcConfig_t *pParams, AblcProc_t *pSelect, AblcExpInfo_t *pExpInfo)
{
    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    int isoLowlevel = 0;
    int isoHighlevel = 0;
    int lowIso = 0;
    int highIso = 0;
    float ratio = 0.0f;
    int isoValue = 50;
    int i = 0;

    if(pParams == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }


    isoValue = pExpInfo->arIso[pExpInfo->hdr_mode];
    for(i = 0; i < pParams->len - 1; i++)
    {
        if(isoValue >= pParams->iso[i] && isoValue <= pParams->iso[i + 1])
        {
            isoLowlevel = i;
            isoHighlevel = i + 1;
            lowIso = pParams->iso[i];
            highIso = pParams->iso[i + 1];
            ratio = (isoValue - lowIso ) / (float)(highIso - lowIso);

            LOG1_ABLC("%s:%d iso: %d %d isovalue:%d ratio:%f \n",
                      __FUNCTION__, __LINE__,
                      lowIso, highIso, isoValue, ratio);
            break;
        }
    }

    if(i == pParams->len - 1) {
        if(isoValue < pParams->iso[0])
        {
            isoLowlevel = 0;
            isoHighlevel = 1;
            ratio = 0;
        }

        if(isoValue > pParams->iso[pParams->len - 1])
        {
            isoLowlevel = pParams->len - 1;
            isoHighlevel = pParams->len - 1;
            ratio = 0;
        }
    }

    pSelect->enable = pParams->enable;

    pSelect->blc_r = (short int)(ratio * (pParams->blc_r[isoHighlevel] - pParams->blc_r[isoLowlevel])
                                 + pParams->blc_r[isoLowlevel]);
    pSelect->blc_gr = (short int)(ratio * (pParams->blc_gr[isoHighlevel] - pParams->blc_gr[isoLowlevel])
                                  + pParams->blc_gr[isoLowlevel]);
    pSelect->blc_gb = (short int)(ratio * (pParams->blc_gb[isoHighlevel] - pParams->blc_gb[isoLowlevel])
                                  + pParams->blc_gb[isoLowlevel]);
    pSelect->blc_b = (short int)(ratio * (pParams->blc_b[isoHighlevel] - pParams->blc_b[isoLowlevel])
                                 + pParams->blc_b[isoLowlevel]);

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_RET_SUCCESS;
}
/******************************************************************************
 * BlcNewMalloc()
 ***************************************************************************/
void BlcNewMalloc
(
    AblcConfig_t*           pBlcPara,
    CalibDbV2_Ablc_t*         pCalibDb
) {
    LOG1_ABLC( "%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pBlcPara != NULL);
    DCT_ASSERT(pCalibDb != NULL);

    if(pBlcPara->len != pCalibDb->BlcTuningPara.BLC_Data.ISO_len) {
        free(pBlcPara->iso);
        free(pBlcPara->blc_b);
        free(pBlcPara->blc_gb);
        free(pBlcPara->blc_gr);
        free(pBlcPara->blc_r);
        pBlcPara->len = pCalibDb->BlcTuningPara.BLC_Data.ISO_len;
        pBlcPara->iso = (float*)malloc(sizeof(float) * (pCalibDb->BlcTuningPara.BLC_Data.ISO_len));
        pBlcPara->blc_r = (float*)malloc(sizeof(float) * (pCalibDb->BlcTuningPara.BLC_Data.R_Channel_len));
        pBlcPara->blc_gr = (float*)malloc(sizeof(float) * (pCalibDb->BlcTuningPara.BLC_Data.Gr_Channel_len));
        pBlcPara->blc_gb = (float*)malloc(sizeof(float) * (pCalibDb->BlcTuningPara.BLC_Data.Gb_Channel_len));
        pBlcPara->blc_b = (float*)malloc(sizeof(float) * (pCalibDb->BlcTuningPara.BLC_Data.B_Channel_len));
    }

    LOG1_ABLC( "%s:exit!\n", __FUNCTION__);
}

AblcResult_t AblcInit(AblcContext_t **ppAblcCtx, CamCalibDbV2Context_t *pCalibDb)
{
    AblcContext_t * pAblcCtx;

    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAblcCtx = (AblcContext_t *)malloc(sizeof(AblcContext_t));
    if(pAblcCtx == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    memset(pAblcCtx, 0x00, sizeof(AblcContext_t));
    pAblcCtx->eState = ABLC_STATE_INITIALIZED;

    *ppAblcCtx = pAblcCtx;

    //init params for algo work
    pAblcCtx->attr.eMode = ABLC_OP_MODE_API_OFF;
    pAblcCtx->isReCalculate |= 1;
    pAblcCtx->isUpdateParam = true;

#if 1
    CalibDbV2_Ablc_t* ablc_calib =
        (CalibDbV2_Ablc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, ablc_calib));
    //xml param
    memcpy(&pAblcCtx->stBlcCalib, ablc_calib, sizeof(CalibDbV2_Ablc_t));
    pAblcCtx->config.len = ablc_calib->BlcTuningPara.BLC_Data.ISO_len;
    pAblcCtx->config.iso = (float*)malloc(sizeof(float) * (ablc_calib->BlcTuningPara.BLC_Data.ISO_len));
    pAblcCtx->config.blc_r = (float*)malloc(sizeof(float) * (ablc_calib->BlcTuningPara.BLC_Data.R_Channel_len));
    pAblcCtx->config.blc_gr = (float*)malloc(sizeof(float) * (ablc_calib->BlcTuningPara.BLC_Data.Gr_Channel_len));
    pAblcCtx->config.blc_gb = (float*)malloc(sizeof(float) * (ablc_calib->BlcTuningPara.BLC_Data.Gb_Channel_len));
    pAblcCtx->config.blc_b = (float*)malloc(sizeof(float) * (ablc_calib->BlcTuningPara.BLC_Data.B_Channel_len));
    AblcConfig(&pAblcCtx->config, &pAblcCtx->stBlcCalib);
#else
    //static init params
    Ablc_html_params_init(&pAblcCtx->stAuto.stParams);
#endif

    LOG1_ABLC("%s(%d): Ablc en:%d blc:%d %d %d %d \n",
              __FUNCTION__, __LINE__,
              pAblcCtx->config.enable,
              pAblcCtx->config.blc_r[0],
              pAblcCtx->config.blc_gr[0],
              pAblcCtx->config.blc_gb[0],
              pAblcCtx->config.blc_gb[0]);


    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_RET_SUCCESS;
}

AblcResult_t AblcRelease(AblcContext_t *pAblcCtx)
{
    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if(pAblcCtx == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    free(pAblcCtx->config.iso);
    free(pAblcCtx->config.blc_r);
    free(pAblcCtx->config.blc_gr);
    free(pAblcCtx->config.blc_gb);
    free(pAblcCtx->config.blc_b);
    memset(pAblcCtx, 0x00, sizeof(AblcContext_t));
    free(pAblcCtx);

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_RET_SUCCESS;

}

AblcResult_t AblcProcess(AblcContext_t *pAblcCtx, AblcExpInfo_t *pExpInfo)
{
    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    AblcResult_t ret = ABLC_RET_SUCCESS;

    if(pAblcCtx == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ABLC("%s(%d): null pointer \n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    memcpy(&pAblcCtx->stExpInfo, pExpInfo, sizeof(AblcExpInfo_t));

    if(pAblcCtx->attr.eMode == ABLC_OP_MODE_API_OFF) {
        LOGD_ABLC("%s:(%d) Ablc in api off !!! \n", __FUNCTION__, __LINE__);
        ret = Ablc_Select_Params_By_ISO(&pAblcCtx->config, &pAblcCtx->ProcRes, pExpInfo);
    }
    else if(pAblcCtx->attr.eMode == ABLC_OP_MODE_API_TOOL) {
        LOGD_ABLC("%s:(%d) Ablc in Tool off !!! \n", __FUNCTION__, __LINE__);
        ret = Ablc_Select_Params_By_ISO(&pAblcCtx->config, &pAblcCtx->ProcRes, pExpInfo);
    }
    else if(pAblcCtx->attr.eMode == ABLC_OP_MODE_API_MANUAL)
        ret = Ablc_Api_Manual_Select_Params(&pAblcCtx->attr.stManual, &pAblcCtx->ProcRes);

    LOGD_ABLC("%s:(%d) Ablc En:%d rggb: %d %d %d %d \n",
              __FUNCTION__, __LINE__, pAblcCtx->ProcRes.enable,
              pAblcCtx->ProcRes.blc_r, pAblcCtx->ProcRes.blc_gr,
              pAblcCtx->ProcRes.blc_gb, pAblcCtx->ProcRes.blc_b);
    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_RET_SUCCESS;
}


