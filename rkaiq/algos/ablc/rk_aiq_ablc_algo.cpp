#include "rk_aiq_algo_ablc_itf.h"
#include "rk_aiq_ablc_algo.h"

AblcResult_t AblcJsonParamInit(AblcParams_t *pParams, AblcParaV2_t* pBlcCalibParams)
{
    AblcResult_t res = ABLC_RET_SUCCESS;

    if(pParams == NULL || pBlcCalibParams == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    pParams->enable = pBlcCalibParams->enable;
    for(int i = 0; i < ABLC_MAX_ISO_LEVEL; i++) {
        pParams->iso[i] = pBlcCalibParams->BLC_Data.ISO[i];
        pParams->blc_r[i] = pBlcCalibParams->BLC_Data.R_Channel[i];
        pParams->blc_gr[i] = pBlcCalibParams->BLC_Data.Gr_Channel[i];
        pParams->blc_gb[i] = pBlcCalibParams->BLC_Data.Gb_Channel[i];
        pParams->blc_b[i] = pBlcCalibParams->BLC_Data.B_Channel[i];

        LOGD_ABLC("%s(%d): Ablc en:%d iso:%d blc:%f %f %f %f \n",
                  __FUNCTION__, __LINE__,
                  pParams->enable,
                  pParams->iso[i],
                  pParams->blc_r[i],
                  pParams->blc_gr[i],
                  pParams->blc_gb[i],
                  pParams->blc_b[i]);
    }

    LOG1_ABLC("%s(%d)\n", __FUNCTION__, __LINE__);
    return res;
}


AblcResult_t Ablc_Select_Params_By_ISO(AblcParams_t *pParams, AblcSelect_t *pSelect, AblcExpInfo_t *pExpInfo)
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
    for(i = 0; i < ABLC_MAX_ISO_LEVEL - 1; i++)
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

    if(i == ABLC_MAX_ISO_LEVEL - 1) {
        if(isoValue < pParams->iso[0])
        {
            isoLowlevel = 0;
            isoHighlevel = 1;
            ratio = 0;
        }

        if(isoValue > pParams->iso[ABLC_MAX_ISO_LEVEL - 1])
        {
            isoLowlevel = ABLC_MAX_ISO_LEVEL - 2;
            isoHighlevel = ABLC_MAX_ISO_LEVEL - 1;
            ratio = 0;
        }
    }

    pExpInfo->isoHigh = pParams->iso[isoHighlevel];
    pExpInfo->isoLow = pParams->iso[isoLowlevel];

    pSelect->enable = pParams->enable;

    pSelect->blc_r = (ratio * (pParams->blc_r[isoHighlevel] - pParams->blc_r[isoLowlevel])
                      + pParams->blc_r[isoLowlevel]);
    pSelect->blc_gr = (ratio * (pParams->blc_gr[isoHighlevel] - pParams->blc_gr[isoLowlevel])
                       + pParams->blc_gr[isoLowlevel]);
    pSelect->blc_gb = (ratio * (pParams->blc_gb[isoHighlevel] - pParams->blc_gb[isoLowlevel])
                       + pParams->blc_gb[isoLowlevel]);
    pSelect->blc_b = (ratio * (pParams->blc_b[isoHighlevel] - pParams->blc_b[isoLowlevel])
                      + pParams->blc_b[isoLowlevel]);

    LOGD_ABLC("%s:(%d) Ablc En:%d  ISO:%d  isoLowlevel:%d isoHighlevel:%d  rggb: %f %f %f %f  \n",
              __FUNCTION__, __LINE__,
              pSelect->enable, isoValue,
              isoLowlevel, isoHighlevel,
              pSelect->blc_r, pSelect->blc_gr,
              pSelect->blc_gb, pSelect->blc_b);

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_RET_SUCCESS;
}

AblcResult_t AblcParamsUpdate(AblcContext_t *pAblcCtx, CalibDbV2_Ablc_t *pCalibDb)
{
    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    AblcResult_t ret = ABLC_RET_SUCCESS;

    if(pAblcCtx == NULL || pCalibDb == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_RET_NULL_POINTER;
    }

    //blc0
    AblcJsonParamInit(&pAblcCtx->stBlc0Params, &pCalibDb->BlcTuningPara);

    //blc1
    if (CHECK_ISP_HW_V3X()) {
        AblcJsonParamInit(&pAblcCtx->stBlc1Params, &pCalibDb->Blc1TuningPara);
    }

    return ret;
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
    pAblcCtx->eMode = ABLC_OP_MODE_AUTO;
    pAblcCtx->isReCalculate |= 1;
    pAblcCtx->isUpdateParam = true;


    CalibDbV2_Ablc_t* ablc_calib =
        (CalibDbV2_Ablc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, ablc_calib));

    memcpy(&pAblcCtx->stBlcCalib, ablc_calib, sizeof(pAblcCtx->stBlcCalib));
    AblcParamsUpdate(pAblcCtx, ablc_calib);

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

    if(pAblcCtx->eMode == ABLC_OP_MODE_AUTO) {
        LOGD_ABLC("%s:(%d) Ablc auto !!! \n", __FUNCTION__, __LINE__);
        ret = Ablc_Select_Params_By_ISO(&pAblcCtx->stBlc0Params, &pAblcCtx->stBlc0Select, pExpInfo);
        pAblcCtx->ProcRes.enable = pAblcCtx->stBlc0Select.enable;
        pAblcCtx->ProcRes.blc_r = (uint16_t)(pAblcCtx->stBlc0Select.blc_r + 0.5);
        pAblcCtx->ProcRes.blc_gr = (uint16_t)(pAblcCtx->stBlc0Select.blc_gr + 0.5);
        pAblcCtx->ProcRes.blc_gb = (uint16_t)(pAblcCtx->stBlc0Select.blc_gb + 0.5);
        pAblcCtx->ProcRes.blc_b = (uint16_t)(pAblcCtx->stBlc0Select.blc_b + 0.5);

        if (CHECK_ISP_HW_V3X()) {
            if(pExpInfo->hdr_mode == 0) {
                //normal mode allow blc1 enable
                if(pAblcCtx->stBlc1Params.enable) {
                    ret = Ablc_Select_Params_By_ISO(&pAblcCtx->stBlc1Params, &pAblcCtx->stBlc1Select, pExpInfo);
                }
                pAblcCtx->stBlc1Select.enable = pAblcCtx->stBlc1Params.enable;
                pAblcCtx->ProcRes.blc1_enable = pAblcCtx->stBlc1Select.enable;
                pAblcCtx->ProcRes.blc1_r = pAblcCtx->stBlc1Select.blc_r;
                pAblcCtx->ProcRes.blc1_gr = pAblcCtx->stBlc1Select.blc_gr;
                pAblcCtx->ProcRes.blc1_gb = pAblcCtx->stBlc1Select.blc_gb;
                pAblcCtx->ProcRes.blc1_b = pAblcCtx->stBlc1Select.blc_b;
            } else {
                //hdr mode blc1 must disable
                pAblcCtx->stBlc1Select.enable = 0;
                pAblcCtx->ProcRes.blc1_enable = 0;
                pAblcCtx->ProcRes.blc1_r = 0;
                pAblcCtx->ProcRes.blc1_gr = 0;
                pAblcCtx->ProcRes.blc1_gb = 0;
                pAblcCtx->ProcRes.blc1_b = 0;
            }
        }
    } else if(pAblcCtx->eMode == ABLC_OP_MODE_MANUAL) {
        LOGD_ABLC("%s:(%d) Ablc manual !!! \n", __FUNCTION__, __LINE__);
        pAblcCtx->ProcRes.enable = pAblcCtx->stBlc0Manual.enable;
        pAblcCtx->ProcRes.blc_r = (uint16_t)(pAblcCtx->stBlc0Manual.blc_r + 0.5);
        pAblcCtx->ProcRes.blc_gr = (uint16_t)(pAblcCtx->stBlc0Manual.blc_gr + 0.5);
        pAblcCtx->ProcRes.blc_gb = (uint16_t)(pAblcCtx->stBlc0Manual.blc_gb + 0.5);
        pAblcCtx->ProcRes.blc_b = (uint16_t)(pAblcCtx->stBlc0Manual.blc_b + 0.5);


        if (CHECK_ISP_HW_V3X()) {
            if(pExpInfo->hdr_mode == 0) {
                //normal mode allow blc1 enable
                pAblcCtx->ProcRes.blc1_enable = pAblcCtx->stBlc1Manual.enable;
                pAblcCtx->ProcRes.blc1_r = pAblcCtx->stBlc1Manual.blc_r;
                pAblcCtx->ProcRes.blc1_gr = pAblcCtx->stBlc1Manual.blc_gr;
                pAblcCtx->ProcRes.blc1_gb = pAblcCtx->stBlc1Manual.blc_gb;
                pAblcCtx->ProcRes.blc1_b = pAblcCtx->stBlc1Manual.blc_b;
            } else {
                //hdr mode blc1 must disable
                pAblcCtx->ProcRes.blc1_enable = 0;
                pAblcCtx->ProcRes.blc1_r = 0;
                pAblcCtx->ProcRes.blc1_gr = 0;
                pAblcCtx->ProcRes.blc1_gb = 0;
                pAblcCtx->ProcRes.blc1_b = 0;
            }
        }
    } else {
        LOGE_ABLC("%s(%d): not support mode:%d!\n", __FUNCTION__, __LINE__, pAblcCtx->eMode);
    }


    memcpy(&pAblcCtx->stExpInfo, pExpInfo, sizeof(AblcExpInfo_t));

    LOGD_ABLC("%s(%d): Ablc en:%d blc:%d %d %d %d \n",
              __FUNCTION__, __LINE__,
              pAblcCtx->ProcRes.enable,
              pAblcCtx->ProcRes.blc_r,
              pAblcCtx->ProcRes.blc_gr,
              pAblcCtx->ProcRes.blc_gb,
              pAblcCtx->ProcRes.blc_b);

    if (CHECK_ISP_HW_V3X()) {
        LOGD_ABLC("%s(%d): Ablc1 en:%d blc:%f %f %f %f \n",
                  __FUNCTION__, __LINE__,
                  pAblcCtx->ProcRes.blc1_enable,
                  pAblcCtx->ProcRes.blc1_r,
                  pAblcCtx->ProcRes.blc1_gr,
                  pAblcCtx->ProcRes.blc1_gb,
                  pAblcCtx->ProcRes.blc1_b);
    }

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_RET_SUCCESS;
}


