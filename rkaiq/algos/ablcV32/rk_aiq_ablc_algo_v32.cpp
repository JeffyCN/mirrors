/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "rk_aiq_ablc_algo_v32.h"

#include "rk_aiq_ablc_algo_itf_v32.h"

AblcResult_V32_t AblcJsonParamInit_V32(AblcParams_V32_t* pParams,
                                       AblcV32ParaV2_t* pBlcCalibParams) {
    AblcResult_V32_t res = ABLC_V32_RET_SUCCESS;

    if (pParams == NULL || pBlcCalibParams == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    pParams->enable = pBlcCalibParams->enable;
    for (int i = 0; i < pParams->len; i++) {
        pParams->iso[i]    = pBlcCalibParams->BLC_Data.ISO[i];
        pParams->blc_r[i]  = pBlcCalibParams->BLC_Data.R_Channel[i];
        pParams->blc_gr[i] = pBlcCalibParams->BLC_Data.Gr_Channel[i];
        pParams->blc_gb[i] = pBlcCalibParams->BLC_Data.Gb_Channel[i];
        pParams->blc_b[i]  = pBlcCalibParams->BLC_Data.B_Channel[i];

        LOGD_ABLC("%s(%d): Ablc en:%d iso:%f blc:%f %f %f %f \n", __FUNCTION__, __LINE__,
                  pParams->enable, pParams->iso[i], pParams->blc_r[i], pParams->blc_gr[i],
                  pParams->blc_gb[i], pParams->blc_b[i]);
    }

    LOG1_ABLC("%s(%d)\n", __FUNCTION__, __LINE__);
    return res;
}

AblcResult_V32_t AblcOBJsonParamInit_V32(AblcOBParams_V32_t* pParams,
        AblcV32OBPara_t* pBlcOBCalibParams) {
    AblcResult_V32_t res = ABLC_V32_RET_SUCCESS;

    if (pParams == NULL || pBlcOBCalibParams == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    pParams->enable = pBlcOBCalibParams->enable;
    for (int i = 0; i < pParams->len; i++) {
        pParams->iso[i]         = pBlcOBCalibParams->BLC_OB_Data.ISO[i];
        pParams->ob_offset[i]   = pBlcOBCalibParams->BLC_OB_Data.isp_ob_Offset[i];
        pParams->ob_predgain[i] = pBlcOBCalibParams->BLC_OB_Data.isp_ob_preDgain[i];

        LOGD_ABLC("%s(%d): blc_OB en:%d iso:%f ob_offset:%f ,ob_predgain: %f \n", __FUNCTION__,
                  __LINE__, pParams->enable, pParams->iso[i], pParams->ob_offset[i],
                  pParams->ob_predgain[i]);
    }

    LOGD_ABLC("%s(%d)\n", __FUNCTION__, __LINE__);
    return res;
}

AblcResult_V32_t AblcRefJsonParamInit_V32(AblcRefParams_V32_t* pBlcRefPara, CalibDbV2_Bayer2dnrV23_Calib_t* stBayer2dnrCalib) {
    LOG1_ABLC("%s:enter!\n", __FUNCTION__);

    AblcResult_V32_t ret = ABLC_V32_RET_SUCCESS;
#if (RKAIQ_HAVE_BAYER2DNR_V23)
    // initial checks
    DCT_ASSERT(pBlcRefPara != NULL);
    DCT_ASSERT(stBayer2dnrCalib != NULL);

    if (pBlcRefPara->len != stBayer2dnrCalib->Blc_Ref_len) {

        if (pBlcRefPara->iso) free(pBlcRefPara->iso);
        if (pBlcRefPara->Reference_r) free(pBlcRefPara->Reference_r);
        if (pBlcRefPara->Reference_gr) free(pBlcRefPara->Reference_gr);
        if (pBlcRefPara->Reference_gb) free(pBlcRefPara->Reference_gb);
        if (pBlcRefPara->Reference_b) free(pBlcRefPara->Reference_b);

        pBlcRefPara->len = stBayer2dnrCalib->Blc_Ref_len;
        pBlcRefPara->iso = (float*)malloc(sizeof(float) * (stBayer2dnrCalib->Blc_Ref_len));
        pBlcRefPara->Reference_r   =
            (float*)malloc(sizeof(float) * (stBayer2dnrCalib->Blc_Ref_len));
        pBlcRefPara->Reference_gr   =
            (float*)malloc(sizeof(float) * (stBayer2dnrCalib->Blc_Ref_len));
        pBlcRefPara->Reference_gb   =
            (float*)malloc(sizeof(float) * (stBayer2dnrCalib->Blc_Ref_len));
        pBlcRefPara->Reference_b   =
            (float*)malloc(sizeof(float) * (stBayer2dnrCalib->Blc_Ref_len));
    }

    for (int j = 0; j < pBlcRefPara->len; j++) {
        pBlcRefPara->iso[j] = stBayer2dnrCalib->Blc_Ref[j].iso;
        pBlcRefPara->Reference_r[j] = stBayer2dnrCalib->Blc_Ref[j].Reference_r;
        pBlcRefPara->Reference_gr[j] = stBayer2dnrCalib->Blc_Ref[j].Reference_gr;
        pBlcRefPara->Reference_gb[j] = stBayer2dnrCalib->Blc_Ref[j].Reference_gb;
        pBlcRefPara->Reference_b[j] = stBayer2dnrCalib->Blc_Ref[j].Reference_b;
    }
    LOGD_ABLC("%s(%d): blc ref init done \n", __FUNCTION__, __LINE__);
#endif
    LOG1_ABLC("%s:exit!\n", __FUNCTION__);
    return ret;
}

AblcResult_V32_t AblcV32_IQParams_Check(AblcParams_V32_t* pBLC0Params, AblcOBParams_V32_t* pBLCOBParams, AblcRefParams_V32_t* pBlcRef) {
    AblcResult_V32_t ret = ABLC_V32_RET_SUCCESS;

    if (pBLC0Params == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pBLCOBParams == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pBlcRef == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    float blc_r_caculate;
    float blc_gr_caculate;
    float blc_gb_caculate;
    float blc_b_caculate;
    float diff;

    if (pBlcRef->len) {
        for (int i = 0; i < pBLC0Params->len; i++) {
            blc_r_caculate = pBLC0Params->blc_r[i] - pBLCOBParams->ob_offset[i];
            blc_gr_caculate = pBLC0Params->blc_gr[i] - pBLCOBParams->ob_offset[i];
            blc_gb_caculate = pBLC0Params->blc_gb[i] - pBLCOBParams->ob_offset[i];
            blc_b_caculate = pBLC0Params->blc_b[i] - pBLCOBParams->ob_offset[i];

            diff = ((pBlcRef->Reference_r[i] - blc_r_caculate) > 0 ) ? (pBlcRef->Reference_r[i] - blc_r_caculate) : ( blc_r_caculate - pBlcRef->Reference_r[i]);
            if (diff > 0.01) {
                LOGE_ABLC("BLC0_r-ob_offset not equal to the blc Reference_r in iso(%d) Reference_r=%.4f, blc_r_caculate=%.4f\n",
                          int(pBlcRef->iso[i]), pBlcRef->Reference_r[i], blc_r_caculate);
            }

            diff = ((pBlcRef->Reference_gr[i] - blc_gr_caculate) > 0 ) ? (pBlcRef->Reference_gr[i] - blc_gr_caculate) : ( blc_gr_caculate - pBlcRef->Reference_gr[i]);
            if (diff > 0.01) {
                LOGE_ABLC("BLC0_gr-ob_offset not equal to the blc Reference_gr in iso(%d) Reference_gr=%.4f, blc_gr_caculate=%.4f\n, cur-blc-gr=%.4f",
                          int(pBlcRef->iso[i]), pBlcRef->Reference_gr[i], blc_gr_caculate, pBLC0Params->blc_gr[i]);
            }

            diff = ((pBlcRef->Reference_gb[i] - blc_gb_caculate) > 0 ) ? (pBlcRef->Reference_gb[i] - blc_gb_caculate) : ( blc_gb_caculate - pBlcRef->Reference_gb[i]);
            if (diff > 0.01) {
                LOGE_ABLC("BLC0_gb-ob_offset not equal to the blc Reference_gb in iso(%d) Reference_gb=%.4f, blc_gb_caculate=%.4f\n",
                          int(pBlcRef->iso[i]), pBlcRef->Reference_gb[i], blc_gb_caculate);
            }

            diff = ((pBlcRef->Reference_b[i] - blc_b_caculate) > 0 ) ? (pBlcRef->Reference_b[i] - blc_b_caculate) : ( blc_b_caculate - pBlcRef->Reference_b[i]);
            if (diff > 0.01) {
                LOGE_ABLC("BLC0_b-ob_offset not equal to the blc Reference_b in iso(%d) Reference_b=%.4f, blc_b_caculate=%.4f\n",
                          int(pBlcRef->iso[i]), pBlcRef->Reference_b[i], blc_b_caculate);
            }
        }
    }

    return ret;
}
AblcResult_V32_t Ablc_Select_Params_By_ISO_V32(AblcParams_V32_t* pParams, AblcSelect_V32_t* pSelect,
        AblcExpInfo_V32_t* pExpInfo) {
    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    int isoLowlevel  = 0;
    int isoHighlevel = 0;
    int lowIso       = 0;
    int highIso      = 0;
    float ratio      = 0.0f;
    int isoValue     = 50;
    int i            = 0;

    if (pParams == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pSelect == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pExpInfo == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pParams->len < 1) {
        LOGE_ABLC("%s(%d): param len is less than 1!\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    isoValue = pExpInfo->arIso[pExpInfo->hdr_mode];
    for (i = 0; i < pParams->len - 1; i++) {
        if (isoValue >= pParams->iso[i] && isoValue <= pParams->iso[i + 1]) {
            isoLowlevel  = i;
            isoHighlevel = i + 1;
            lowIso       = pParams->iso[i];
            highIso      = pParams->iso[i + 1];
            ratio        = (isoValue - lowIso) / (float)(highIso - lowIso);

            LOG1_ABLC("%s:%d iso: %d %d isovalue:%d ratio:%f \n", __FUNCTION__, __LINE__, lowIso,
                      highIso, isoValue, ratio);
            break;
        }
    }

    if (i == pParams->len - 1) {
        if (isoValue < pParams->iso[0]) {
            isoLowlevel  = 0;
            isoHighlevel = 1;
            ratio        = 0;
        }

        if (isoValue > pParams->iso[pParams->len - 1]) {
            isoLowlevel  = pParams->len - 1;
            isoHighlevel = pParams->len - 1;
            ratio        = 0;
        }
    }

    pExpInfo->isoLevelHig = isoHighlevel;
    pExpInfo->isoLevelLow = isoLowlevel;

    pSelect->enable = pParams->enable;

    pSelect->blc_r =
        (short int)(ratio * (pParams->blc_r[isoHighlevel] - pParams->blc_r[isoLowlevel]) +
                    pParams->blc_r[isoLowlevel]);
    pSelect->blc_gr =
        (short int)(ratio * (pParams->blc_gr[isoHighlevel] - pParams->blc_gr[isoLowlevel]) +
                    pParams->blc_gr[isoLowlevel]);
    pSelect->blc_gb =
        (short int)(ratio * (pParams->blc_gb[isoHighlevel] - pParams->blc_gb[isoLowlevel]) +
                    pParams->blc_gb[isoLowlevel]);
    pSelect->blc_b =
        (short int)(ratio * (pParams->blc_b[isoHighlevel] - pParams->blc_b[isoLowlevel]) +
                    pParams->blc_b[isoLowlevel]);

    LOGD_ABLC("%s:(%d) Ablc En:%d  ISO:%d  isoLowlevel:%d isoHighlevel:%d  rggb: %d %d %d %d  \n",
              __FUNCTION__, __LINE__, pSelect->enable, isoValue, isoLowlevel, isoHighlevel,
              pSelect->blc_r, pSelect->blc_gr, pSelect->blc_gb, pSelect->blc_b);

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_V32_RET_SUCCESS;
}

AblcResult_V32_t Ablc_Select_OBParams_By_ISO_V32(AblcOBParams_V32_t* pParams,
        AblcOBSelect_V32_t* pSelect,
        AblcExpInfo_V32_t* pExpInfo) {
    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    int isoLowlevel  = 0;
    int isoHighlevel = 0;
    int lowIso       = 0;
    int highIso      = 0;
    float ratio      = 0.0f;
    int isoValue     = 50;
    int i            = 0;

    if (pParams == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pSelect == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pExpInfo == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pParams->len < 1) {
        LOGE_ABLC("%s(%d): param len is less than 1!\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    isoValue = pExpInfo->arIso[pExpInfo->hdr_mode];
    for (i = 0; i < pParams->len - 1; i++) {
        if (isoValue >= pParams->iso[i] && isoValue <= pParams->iso[i + 1]) {
            isoLowlevel  = i;
            isoHighlevel = i + 1;
            lowIso       = pParams->iso[i];
            highIso      = pParams->iso[i + 1];
            ratio        = (isoValue - lowIso) / (float)(highIso - lowIso);

            LOG1_ABLC("%s:%d iso: %d %d isovalue:%d ratio:%f \n", __FUNCTION__, __LINE__, lowIso,
                      highIso, isoValue, ratio);
            break;
        }
    }

    if (i == pParams->len - 1) {
        if (isoValue < pParams->iso[0]) {
            isoLowlevel  = 0;
            isoHighlevel = 1;
            ratio        = 0;
        }

        if (isoValue > pParams->iso[pParams->len - 1]) {
            isoLowlevel  = pParams->len - 1;
            isoHighlevel = pParams->len - 1;
            ratio        = 0;
        }
    }

    if (pExpInfo->hdr_mode == 0) {
        LOGD_ABLC("%s:(%d) LINE MODE blc_ob enable: hdr_mode = %", __FUNCTION__, __LINE__,
                  pExpInfo->hdr_mode);
        pSelect->enable      = pParams->enable;
        pSelect->ob_offset   = (short int)(ratio * (pParams->ob_offset[isoHighlevel] -
                                           pParams->ob_offset[isoLowlevel]) +
                                           pParams->ob_offset[isoLowlevel]);
        pSelect->ob_predgain = (float)(ratio * (pParams->ob_predgain[isoHighlevel] -
                                                pParams->ob_predgain[isoLowlevel]) +
                                       pParams->ob_predgain[isoLowlevel]);
    } else {
        LOGD_ABLC("%s:(%d) HDR MODE blc_ob disable: hdr_mode = %d", __FUNCTION__, __LINE__,
                  pExpInfo->hdr_mode);
        pSelect->enable      = false;
        pSelect->ob_offset   = 0;
        pSelect->ob_predgain = 0;
    }

    LOGD_ABLC(
        "%s:(%d) ISO:%d  isoLowlevel:%d isoHighlevel:%d  isp_ob_offset: %f isp_ob_predgian %f  \n",
        __FUNCTION__, __LINE__, isoValue, isoLowlevel, isoHighlevel, pSelect->ob_offset,
        pSelect->ob_predgain);

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_V32_RET_SUCCESS;
}

/******************************************************************************
 * BlcOBNewMalloc()
 ***************************************************************************/
void BlcOBNewMalloc(AblcOBParams_V32_t* pBlcOBPara, AblcV32OBPara_t* pBlcOBCalibParams) {
    LOG1_ABLC("%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pBlcOBPara != NULL);
    DCT_ASSERT(pBlcOBCalibParams != NULL);

    if (pBlcOBPara->len != pBlcOBCalibParams->BLC_OB_Data.ISO_len) {
        if (pBlcOBPara->iso) free(pBlcOBPara->iso);

        if (pBlcOBPara->ob_offset) free(pBlcOBPara->ob_offset);

        if (pBlcOBPara->ob_predgain) free(pBlcOBPara->ob_predgain);

        pBlcOBPara->len = pBlcOBCalibParams->BLC_OB_Data.ISO_len;
        pBlcOBPara->iso = (float*)malloc(sizeof(float) * (pBlcOBCalibParams->BLC_OB_Data.ISO_len));
        pBlcOBPara->ob_offset =
            (float*)malloc(sizeof(float) * (pBlcOBCalibParams->BLC_OB_Data.isp_ob_Offset_len));
        pBlcOBPara->ob_predgain =
            (float*)malloc(sizeof(float) * (pBlcOBCalibParams->BLC_OB_Data.isp_ob_preDgain_len));
    }

    LOG1_ABLC("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * BlcNewMalloc()
 ***************************************************************************/
void BlcNewMalloc(AblcParams_V32_t* pBlcPara, AblcV32ParaV2_t* pBlcCalibParams) {
    LOG1_ABLC("%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pBlcPara != NULL);
    DCT_ASSERT(pBlcCalibParams != NULL);

    if (pBlcPara->len != pBlcCalibParams->BLC_Data.ISO_len) {
        if (pBlcPara->iso) free(pBlcPara->iso);

        if (pBlcPara->blc_b) free(pBlcPara->blc_b);

        if (pBlcPara->blc_gb) free(pBlcPara->blc_gb);

        if (pBlcPara->blc_gr) free(pBlcPara->blc_gr);

        if (pBlcPara->blc_r) free(pBlcPara->blc_r);

        pBlcPara->len   = pBlcCalibParams->BLC_Data.ISO_len;
        pBlcPara->iso   = (float*)malloc(sizeof(float) * (pBlcCalibParams->BLC_Data.ISO_len));
        pBlcPara->blc_r = (float*)malloc(sizeof(float) * (pBlcCalibParams->BLC_Data.R_Channel_len));
        pBlcPara->blc_gr =
            (float*)malloc(sizeof(float) * (pBlcCalibParams->BLC_Data.Gr_Channel_len));
        pBlcPara->blc_gb =
            (float*)malloc(sizeof(float) * (pBlcCalibParams->BLC_Data.Gb_Channel_len));
        pBlcPara->blc_b = (float*)malloc(sizeof(float) * (pBlcCalibParams->BLC_Data.B_Channel_len));
    }

    LOG1_ABLC("%s:exit!\n", __FUNCTION__);
}

AblcResult_V32_t AblcV32ParamsUpdate(AblcContext_V32_t* pAblcCtx, CalibDbV2_Blc_V32_t* pCalibDb) {
    LOGD_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    AblcResult_V32_t ret = ABLC_V32_RET_SUCCESS;

    if (pAblcCtx == NULL || pCalibDb == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    // blc0
    BlcNewMalloc(&pAblcCtx->stBlc0Params, &pCalibDb->Blc0TuningPara);
    AblcJsonParamInit_V32(&pAblcCtx->stBlc0Params, &pCalibDb->Blc0TuningPara);

    // bls1
    if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        BlcNewMalloc(&pAblcCtx->stBlc1Params, &pCalibDb->Blc1TuningPara);
        AblcJsonParamInit_V32(&pAblcCtx->stBlc1Params, &pCalibDb->Blc1TuningPara);
    }

    // blc_ob
    if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE()) {
        BlcOBNewMalloc(&pAblcCtx->stBlcOBParams, &pCalibDb->BlcObPara);
        AblcOBJsonParamInit_V32(&pAblcCtx->stBlcOBParams, &pCalibDb->BlcObPara);
    }
    // blc_ref
    if (CHECK_ISP_HW_V32() ||  CHECK_ISP_HW_V32_LITE()) {
#if (RKAIQ_HAVE_BAYER2DNR_V23)
        AblcRefJsonParamInit_V32(&pAblcCtx->stBlcRefParams, &pAblcCtx->stBayer2dnrCalib);
#endif
    }

    return ret;
    LOGD_ABLC("%s:exit!\n", __FUNCTION__);
}

AblcResult_V32_t AblcV32Init(AblcContext_V32_t** ppAblcCtx, CamCalibDbV2Context_t* pCalibDb) {
    AblcContext_V32_t* pAblcCtx;

    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    pAblcCtx = (AblcContext_V32_t*)malloc(sizeof(AblcContext_V32_t));
    if (pAblcCtx == NULL) {
        LOGE_ABLC("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    memset(pAblcCtx, 0x00, sizeof(AblcContext_V32_t));
    pAblcCtx->eState = ABLC_V32_STATE_INITIALIZED;

    *ppAblcCtx = pAblcCtx;

    // init params for algo work
    pAblcCtx->eMode = ABLC_V32_OP_MODE_AUTO;
    pAblcCtx->isReCalculate |= 1;
    pAblcCtx->isUpdateParam = true;

    CalibDbV2_Blc_V32_t* ablc_calib =
        (CalibDbV2_Blc_V32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, ablcV32_calib));
#if (RKAIQ_HAVE_BAYER2DNR_V23)
    CalibDbV2_Bayer2dnrV23_t *bayernr_v23 = (CalibDbV2_Bayer2dnrV23_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, bayer2dnr_v23));
    pAblcCtx->stBayer2dnrCalib = bayernr_v23->CalibPara;
#endif
    pAblcCtx->stBlcCalib = *ablc_calib;
    AblcV32ParamsUpdate(pAblcCtx, ablc_calib);

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_V32_RET_SUCCESS;
}

AblcResult_V32_t AblcV32Release(AblcContext_V32_t* pAblcCtx) {
    LOG1_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    if (pAblcCtx == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }
    // BLC0
    if (pAblcCtx->stBlc0Params.iso) free(pAblcCtx->stBlc0Params.iso);

    if (pAblcCtx->stBlc0Params.blc_b) free(pAblcCtx->stBlc0Params.blc_b);

    if (pAblcCtx->stBlc0Params.blc_gb) free(pAblcCtx->stBlc0Params.blc_gb);

    if (pAblcCtx->stBlc0Params.blc_gr) free(pAblcCtx->stBlc0Params.blc_gr);

    if (pAblcCtx->stBlc0Params.blc_r) free(pAblcCtx->stBlc0Params.blc_r);

    // BLC1
    if (pAblcCtx->stBlc1Params.iso) free(pAblcCtx->stBlc1Params.iso);

    if (pAblcCtx->stBlc1Params.blc_b) free(pAblcCtx->stBlc1Params.blc_b);

    if (pAblcCtx->stBlc1Params.blc_gb) free(pAblcCtx->stBlc1Params.blc_gb);

    if (pAblcCtx->stBlc1Params.blc_gr) free(pAblcCtx->stBlc1Params.blc_gr);

    if (pAblcCtx->stBlc1Params.blc_r) free(pAblcCtx->stBlc1Params.blc_r);

    // BLC_OB
    if (pAblcCtx->stBlcOBParams.iso) free(pAblcCtx->stBlcOBParams.iso);

    if (pAblcCtx->stBlcOBParams.ob_offset) free(pAblcCtx->stBlcOBParams.ob_offset);

    if (pAblcCtx->stBlcOBParams.ob_predgain) free(pAblcCtx->stBlcOBParams.ob_predgain);

    // BLC_REF
    if (pAblcCtx->stBlcRefParams.iso) free(pAblcCtx->stBlcRefParams.iso);

    if (pAblcCtx->stBlcRefParams.Reference_b) free(pAblcCtx->stBlcRefParams.Reference_b);

    if (pAblcCtx->stBlcRefParams.Reference_gb) free(pAblcCtx->stBlcRefParams.Reference_gb);

    if (pAblcCtx->stBlcRefParams.Reference_gr) free(pAblcCtx->stBlcRefParams.Reference_gr);

    if (pAblcCtx->stBlcRefParams.Reference_r) free(pAblcCtx->stBlcRefParams.Reference_r);

    memset(pAblcCtx, 0x00, sizeof(AblcContext_V32_t));
    free(pAblcCtx);

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);
    return ABLC_V32_RET_SUCCESS;
}

AblcResult_V32_t AblcV32Process(AblcContext_V32_t* pAblcCtx, AblcExpInfo_V32_t* pExpInfo) {
    LOGD_ABLC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    AblcResult_V32_t ret = ABLC_V32_RET_SUCCESS;
    float tmp;

    if (pAblcCtx == NULL) {
        LOGE_ABLC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    if (pExpInfo == NULL) {
        LOGE_ABLC("%s(%d): null pointer \n", __FUNCTION__, __LINE__);
        return ABLC_V32_RET_NULL_POINTER;
    }

    memcpy(&pAblcCtx->stExpInfo, pExpInfo, sizeof(AblcExpInfo_V32_t));

    if (pAblcCtx->eMode == ABLC_V32_OP_MODE_AUTO) {
        LOGD_ABLC("%s:(%d) Ablc auto !!! \n", __FUNCTION__, __LINE__);
        ret = Ablc_Select_Params_By_ISO_V32(&pAblcCtx->stBlc0Params, &pAblcCtx->stBlc0Select,
                                            pExpInfo);
        if (pAblcCtx->stBlc0Select.enable) {

            pAblcCtx->ProcRes.enable = pAblcCtx->stBlc0Select.enable;
            pAblcCtx->ProcRes.blc_r  = pAblcCtx->stBlc0Select.blc_r;
            pAblcCtx->ProcRes.blc_gr = pAblcCtx->stBlc0Select.blc_gr;
            pAblcCtx->ProcRes.blc_gb = pAblcCtx->stBlc0Select.blc_gb;
            pAblcCtx->ProcRes.blc_b  = pAblcCtx->stBlc0Select.blc_b;
        } else {
            pAblcCtx->ProcRes.enable = false;
            pAblcCtx->ProcRes.blc_r  = 0;
            pAblcCtx->ProcRes.blc_gr = 0;
            pAblcCtx->ProcRes.blc_gb = 0;
            pAblcCtx->ProcRes.blc_b  = 0;
        }

        ret = Ablc_Select_Params_By_ISO_V32(&pAblcCtx->stBlc1Params, &pAblcCtx->stBlc1Select,
                                            pExpInfo);
        if (pAblcCtx->stBlc1Select.enable) {
            //hdr won't use blc1
            pAblcCtx->ProcRes.blc1_enable = pAblcCtx->stBlc1Select.enable;
            pAblcCtx->ProcRes.blc1_r      = pAblcCtx->stBlc1Select.blc_r * pExpInfo->isp_dgain[0];
            pAblcCtx->ProcRes.blc1_gr     = pAblcCtx->stBlc1Select.blc_gr * pExpInfo->isp_dgain[0];
            pAblcCtx->ProcRes.blc1_gb     = pAblcCtx->stBlc1Select.blc_gb * pExpInfo->isp_dgain[0];
            pAblcCtx->ProcRes.blc1_b      = pAblcCtx->stBlc1Select.blc_b * pExpInfo->isp_dgain[0];
        } else {
            pAblcCtx->ProcRes.blc1_enable = false;
            pAblcCtx->ProcRes.blc1_r      = 0;
            pAblcCtx->ProcRes.blc1_gr     = 0;
            pAblcCtx->ProcRes.blc1_gb     = 0;
            pAblcCtx->ProcRes.blc1_b      = 0;
        }

        ret = Ablc_Select_OBParams_By_ISO_V32(&pAblcCtx->stBlcOBParams,
                                              &pAblcCtx->stBlcOBSelect, pExpInfo);
        if (pAblcCtx->stBlcOBSelect.enable) {
            pAblcCtx->ProcRes.blc_ob_enable = pAblcCtx->stBlcOBSelect.enable;
            // isp_ob_offset algo result
            pAblcCtx->ProcRes.isp_ob_offset = pAblcCtx->stBlcOBSelect.ob_offset;
            // isp_ob_predgain algo result
            pAblcCtx->ProcRes.isp_ob_predgain = pAblcCtx->stBlcOBSelect.ob_predgain;
            // isp_ob_max algo result
            pAblcCtx->ProcRes.isp_ob_max = (int)(4096 * pAblcCtx->ProcRes.isp_ob_predgain) - pAblcCtx->ProcRes.isp_ob_offset;
        } else {
            pAblcCtx->ProcRes.blc_ob_enable   = false;
            pAblcCtx->ProcRes.isp_ob_offset   = 0;
            pAblcCtx->ProcRes.isp_ob_predgain = 1;
            pAblcCtx->ProcRes.isp_ob_max      = 0xfffff;
        }
    } else if (pAblcCtx->eMode == ABLC_V32_OP_MODE_MANUAL) {
        LOGE_ABLC("%s:(%d) Ablc manual !!! \n", __FUNCTION__, __LINE__);
        pAblcCtx->ProcRes.enable = pAblcCtx->stBlc0Manual.enable;
        pAblcCtx->ProcRes.blc_r  = pAblcCtx->stBlc0Manual.blc_r;
        pAblcCtx->ProcRes.blc_gr = pAblcCtx->stBlc0Manual.blc_gr;
        pAblcCtx->ProcRes.blc_gb = pAblcCtx->stBlc0Manual.blc_gb;
        pAblcCtx->ProcRes.blc_b  = pAblcCtx->stBlc0Manual.blc_b;

        pAblcCtx->ProcRes.blc1_enable = pAblcCtx->stBlc1Manual.enable;
        pAblcCtx->ProcRes.blc1_r      = pAblcCtx->stBlc1Manual.blc_r * pExpInfo->isp_dgain[0];
        pAblcCtx->ProcRes.blc1_gr     = pAblcCtx->stBlc1Manual.blc_gr * pExpInfo->isp_dgain[0];
        pAblcCtx->ProcRes.blc1_gb     = pAblcCtx->stBlc1Manual.blc_gb * pExpInfo->isp_dgain[0];
        pAblcCtx->ProcRes.blc1_b      = pAblcCtx->stBlc1Manual.blc_b * pExpInfo->isp_dgain[0];

        if (pAblcCtx->stBlcOBManual.enable) {
            pAblcCtx->ProcRes.blc_ob_enable = pAblcCtx->stBlcOBManual.enable;
            // isp_ob_offset
            pAblcCtx->ProcRes.isp_ob_offset = pAblcCtx->stBlcOBManual.ob_offset;
            // isp_ob_predgain
            pAblcCtx->ProcRes.isp_ob_predgain = pAblcCtx->stBlcOBManual.ob_predgain;
            // isp_ob_max
            pAblcCtx->ProcRes.isp_ob_max = (int)(4096 * pAblcCtx->stBlcOBManual.ob_predgain) - pAblcCtx->stBlcOBManual.ob_offset;
        } else {
            pAblcCtx->ProcRes.blc_ob_enable   = false;
            pAblcCtx->ProcRes.isp_ob_offset   = 0;
            pAblcCtx->ProcRes.isp_ob_predgain = 1;
            pAblcCtx->ProcRes.isp_ob_max      = 0xfffff;
        }
    } else {
        LOGE_ABLC("%s(%d): not support mode:%d!\n", __FUNCTION__, __LINE__, pAblcCtx->eMode);
    }

    LOGD_ABLC("%s(%d): Ablc en:%d blc:%d %d %d %d \n", __FUNCTION__, __LINE__,
              pAblcCtx->ProcRes.enable, pAblcCtx->ProcRes.blc_r, pAblcCtx->ProcRes.blc_gr,
              pAblcCtx->ProcRes.blc_gb, pAblcCtx->ProcRes.blc_b);

    LOGD_ABLC("%s(%d): Ablc1 en:%d blc:%d %d %d %d \n", __FUNCTION__, __LINE__,
              pAblcCtx->ProcRes.blc1_enable, pAblcCtx->ProcRes.blc1_r, pAblcCtx->ProcRes.blc1_gr,
              pAblcCtx->ProcRes.blc1_gb, pAblcCtx->ProcRes.blc1_b);
    LOGD_ABLC("%s(%d): AblcOB en:%d ob_offset: %d ob_predgain: %f ob_max: %d \n", __FUNCTION__,
              __LINE__, pAblcCtx->ProcRes.blc_ob_enable, pAblcCtx->ProcRes.isp_ob_offset,
              pAblcCtx->ProcRes.isp_ob_predgain, pAblcCtx->ProcRes.isp_ob_max);

    LOG1_ABLC("%s(%d): exit!\n", __FUNCTION__, __LINE__);

    return ret;
}
