

#include "rk_aiq_abayertnr_algo_bayertnr_v30.h"


RKAIQ_BEGIN_DECLARE

#define BTNR_INTERP_V30(x0, x1, ratio) ((ratio) * ((x1) - (x0)) + x0)

Abayertnr_result_V30_t bayertnr_select_params_by_ISO_V30(void* pParams_v, void* pSelect_v,
        Abayertnr_ExpInfo_V30_t* pExpInfo) {
    Abayertnr_result_V30_t res = ABAYERTNRV30_RET_SUCCESS;
    int iso = 50;

    RK_Bayertnr_Params_V30_t* pParams        = (RK_Bayertnr_Params_V30_t*)pParams_v;
    RK_Bayertnr_Params_V30_Select_t* pSelect = (RK_Bayertnr_Params_V30_Select_t*)pSelect_v;


    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    LOGD_ANR("%s:%d iso:%d \n", __FUNCTION__, __LINE__, iso);

    int isoGainStd[RK_BAYERNR_V30_MAX_ISO_NUM];
    int isoGain = MAX(int(iso / 50), 1);
    int isoGainLow = 0;
    int isoGainHig = 0;
    int isoGainCorrect = 1;
    int isoLevelLow = 0;
    int isoLevelHig = 0;
    int isoLevelCorrect = 0;
    int i, j;
    float tmpf;

#ifndef RK_SIMULATOR_HW
    for(int i = 0; i < RK_BAYERNR_V30_MAX_ISO_NUM; i++) {
        isoGainStd[i] = pParams->iso[i] / 50;
    }
#else
    for(int i = 0; i < RK_BAYERNR_V30_MAX_ISO_NUM; i++) {
        isoGainStd[i] = 1 * (1 << i);
    }
#endif

    for (i = 0; i < RK_BAYERNR_V30_MAX_ISO_NUM - 1; i++) {
        if (isoGain >= isoGainStd[i] && isoGain <= isoGainStd[i + 1]) {
            isoGainLow = isoGainStd[i];
            isoGainHig = isoGainStd[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
            isoGainCorrect = ((isoGain - isoGainStd[i]) <= (isoGainStd[i + 1] - isoGain)) ? isoGainStd[i] : isoGainStd[i + 1];
            isoLevelCorrect = ((isoGain - isoGainStd[i]) <= (isoGainStd[i + 1] - isoGain)) ? i : (i + 1);
            break;
        }
    }

    if(iso > pParams->iso[RK_BAYERNR_V30_MAX_ISO_NUM - 1]) {
        isoGainLow = isoGainStd[RK_BAYERNR_V30_MAX_ISO_NUM - 2];
        isoGainHig = isoGainStd[RK_BAYERNR_V30_MAX_ISO_NUM - 1];
        isoLevelLow = RK_BAYERNR_V30_MAX_ISO_NUM - 2;
        isoLevelHig = RK_BAYERNR_V30_MAX_ISO_NUM - 1;
        isoGainCorrect = isoGainStd[RK_BAYERNR_V30_MAX_ISO_NUM - 1];
        isoLevelCorrect = RK_BAYERNR_V30_MAX_ISO_NUM - 1;
    }

    if(iso < pParams->iso[0]) {
        isoGainLow = isoGainStd[0];
        isoGainHig = isoGainStd[1];
        isoLevelLow = 0;
        isoLevelHig = 1;
        isoGainCorrect = isoGainStd[0];
        isoLevelCorrect = 0;
    }

    LOGD_ANR("%s:%d gain:%d high:%d low:%d\n",
             __FUNCTION__, __LINE__,
             isoGain, isoGainHig, isoGainLow);

    pExpInfo->isoLevelLow = isoLevelLow;
    pExpInfo->isoLevelHig = isoLevelHig;

    RK_Bayertnr_Params_V30_Select_t* pLowISO = &pParams->bayertnrParamISO[isoLevelLow];
    RK_Bayertnr_Params_V30_Select_t* pHighISO = &pParams->bayertnrParamISO[isoLevelHig];
    RK_Bayertnr_Params_V30_Select_t* pNearISO = NULL;

    float ratio = 0;
    ratio = float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow);

    pSelect->enable = pParams->enable;

    if((isoGain - isoGainLow) <= (isoGainHig - isoGain)) {
        pNearISO = pLowISO;
    } else {
        pNearISO = pHighISO;
    }

    pSelect->hw_mdBypss_en = pNearISO->hw_mdBypss_en;
    pSelect->hw_iirSpnrOut_en = pNearISO->hw_iirSpnrOut_en;
    pSelect->hw_loMdWgtdbg_en = pNearISO->hw_loMdWgtdbg_en;
    pSelect->hw_mdWgtOut_en = pNearISO->hw_mdWgtOut_en;
    pSelect->hw_loDetection_mode = pNearISO->hw_loDetection_mode;
    pSelect->hw_curFiltOut_en = pNearISO->hw_curFiltOut_en;

    //pSelect->hw_btnr_tnr_en = pNearISO->hw_btnr_tnr_en;
    pSelect->hw_btnr_curSpnr_en = pNearISO->hw_btnr_curSpnr_en;
    pSelect->hw_btnr_preSpnr_en = pNearISO->hw_btnr_preSpnr_en;

    pSelect->sw_btnr_tnrFilt_strg = BTNR_INTERP_V30(pLowISO->sw_btnr_tnrFilt_strg, pHighISO->sw_btnr_tnrFilt_strg, ratio);
    pSelect->sw_btnr_curSpnrFilt_strg = BTNR_INTERP_V30(pLowISO->sw_btnr_curSpnrFilt_strg, pHighISO->sw_btnr_curSpnrFilt_strg, ratio);
    pSelect->sw_btnr_preSpnrFilt_strg = BTNR_INTERP_V30(pLowISO->sw_btnr_preSpnrFilt_strg, pHighISO->sw_btnr_preSpnrFilt_strg, ratio);

    pSelect->hw_btnr_curSpnrSigmaIdxFilt_en = pNearISO->hw_btnr_curSpnrSigmaIdxFilt_en;
    // pSelect->hw_btnr_curSpnrSigmaIdxFilt_mode = pNearISO->hw_btnr_curSpnrSigmaIdxFilt_mode;
    pSelect->hw_btnr_preSpnrSigmaIdxFilt_en = pNearISO->hw_btnr_preSpnrSigmaIdxFilt_en;
    //pSelect->hw_btnr_preSpnrSigmaIdxFilt_mode = pNearISO->hw_btnr_preSpnrSigmaIdxFilt_mode;

    pSelect->hw_btnr_curSpnr_wgt = BTNR_INTERP_V30(pLowISO->hw_btnr_curSpnr_wgt, pHighISO->hw_btnr_curSpnr_wgt, ratio);
    pSelect->hw_btnr_preSpnr_wgt = BTNR_INTERP_V30(pLowISO->hw_btnr_preSpnr_wgt, pHighISO->hw_btnr_preSpnr_wgt, ratio);
    pSelect->hw_btnr_curSpnrWgtCal_scale = pNearISO->hw_btnr_curSpnrWgtCal_scale;
    pSelect->hw_btnr_curSpnrWgtCal_offset = pNearISO->hw_btnr_curSpnrWgtCal_offset;
    pSelect->hw_btnr_preSpnrWgtCal_scale = pNearISO->hw_btnr_preSpnrWgtCal_scale;
    pSelect->hw_btnr_preSpnrWgtCal_offset = pNearISO->hw_btnr_preSpnrWgtCal_offset;

    pSelect->hw_btnr_spnrPresigmaUse_en = pNearISO->hw_btnr_spnrPresigmaUse_en;
    pSelect->hw_btnr_curSpnrSigma_scale = pNearISO->hw_btnr_curSpnrSigma_scale;
    pSelect->hw_btnr_curSpnrSigma_offset = pNearISO->hw_btnr_curSpnrSigma_offset;
    pSelect->hw_btnr_preSpnrSigma_scale = pNearISO->hw_btnr_preSpnrSigma_scale;
    pSelect->hw_btnr_preSpnrSigma_offset = pNearISO->hw_btnr_preSpnrSigma_offset;
    pSelect->hw_btnr_curSpnrSigmaHdrS_scale = pNearISO->hw_btnr_curSpnrSigmaHdrS_scale;
    pSelect->hw_btnr_curSpnrSigmaHdrS_offset = pNearISO->hw_btnr_curSpnrSigmaHdrS_offset;
    pSelect->hw_btnr_preSpnrSigmaHdrS_scale = pNearISO->hw_btnr_preSpnrSigmaHdrS_scale;
    pSelect->hw_btnr_preSpnrSigmaHdrS_offset = pNearISO->hw_btnr_preSpnrSigmaHdrS_offset;

    pSelect->hw_btnr_transf_en = pNearISO->hw_btnr_transf_en;
    pSelect->hw_btnr_transf_mode = pNearISO->hw_btnr_transf_mode;
    pSelect->hw_btnr_transfMode0_scale = pNearISO->hw_btnr_transfMode0_scale;
    pSelect->sw_btnr_transfMode0_offset = pNearISO->sw_btnr_transfMode0_offset;
    pSelect->sw_btnr_itransfMode0_offset = pNearISO->sw_btnr_itransfMode0_offset;
    pSelect->sw_btnr_transfMode1_offset = pNearISO->sw_btnr_transfMode1_offset;
    pSelect->sw_btnr_itransfMode1_offset = pNearISO->sw_btnr_itransfMode1_offset;
    pSelect->hw_btnr_transfData_maxLimit = pNearISO->hw_btnr_transfData_maxLimit;

    pSelect->hw_btnr_gKalman_en = pNearISO->hw_btnr_gKalman_en;
    pSelect->hw_btnr_gKalman_wgt = pNearISO->hw_btnr_gKalman_wgt;

    pSelect->hw_btnr_wgtCal_mode = pNearISO->hw_btnr_wgtCal_mode;
    pSelect->hw_btnr_lpfHi_en = pNearISO->hw_btnr_lpfHi_en;
    pSelect->sw_btnr_lpfHi_sigma = pNearISO->sw_btnr_lpfHi_sigma;
    pSelect->hw_btnr_lpfLo_en = pNearISO->hw_btnr_lpfLo_en;
    pSelect->sw_btnr_lpfLo_sigma = pNearISO->sw_btnr_lpfLo_sigma;
    pSelect->hw_btnr_sigmaIdxFilt_en = pNearISO->hw_btnr_sigmaIdxFilt_en;
    pSelect->hw_btnr_sigmaIdxFilt_mode = pNearISO->hw_btnr_sigmaIdxFilt_mode;
    pSelect->hw_btnr_sigma_scale = pNearISO->hw_btnr_sigma_scale;
    pSelect->hw_btnr_wgtFilt_en = pNearISO->hw_btnr_wgtFilt_en;
    pSelect->hw_btnr_mode0LoWgt_scale = pNearISO->hw_btnr_mode0LoWgt_scale;
    pSelect->hw_btnr_mode0Base_ratio = pNearISO->hw_btnr_mode0Base_ratio;
    pSelect->hw_btnr_mode1LoWgt_scale = pNearISO->hw_btnr_mode1LoWgt_scale;
    pSelect->hw_btnr_mode1LoWgt_offset = pNearISO->hw_btnr_mode1LoWgt_offset;
    pSelect->hw_btnr_mode1Wgt_offset = pNearISO->hw_btnr_mode1Wgt_offset;
    pSelect->hw_btnr_mode1Wgt_minLimit = pNearISO->hw_btnr_mode1Wgt_minLimit;
    pSelect->hw_btnr_mode1Wgt_scale = pNearISO->hw_btnr_mode1Wgt_scale;

    pSelect->hw_btnr_loDetection_en = pNearISO->hw_btnr_loDetection_en;
    pSelect->hw_btnr_loDiffVfilt_en = pNearISO->hw_btnr_loDiffVfilt_en;
    pSelect->hw_btnr_loDiffVfilt_mode = pNearISO->hw_btnr_loDiffVfilt_mode;
    pSelect->hw_btnr_loDiffHfilt_en = pNearISO->hw_btnr_loDiffHfilt_en;
    pSelect->hw_btnr_loDiffWgtCal_scale = pNearISO->hw_btnr_loDiffWgtCal_scale;
    pSelect->hw_btnr_loDiffWgtCal_offset = pNearISO->hw_btnr_loDiffWgtCal_offset;
    pSelect->hw_btnr_loDiffFirstLine_scale = pNearISO->hw_btnr_loDiffFirstLine_scale;
    pSelect->hw_btnr_loDiffVfilt_wgt = pNearISO->hw_btnr_loDiffVfilt_wgt;
    pSelect->hw_btnr_sigmaHfilt_en = pNearISO->hw_btnr_sigmaHfilt_en;
    pSelect->hw_btnr_sigmaFirstLine_scale = pNearISO->hw_btnr_sigmaFirstLine_scale;
    pSelect->hw_btnr_sigmaVfilt_wgt = pNearISO->hw_btnr_sigmaVfilt_wgt;
    pSelect->hw_btnr_loWgtCal_maxLimit = pNearISO->hw_btnr_loWgtCal_maxLimit;
    pSelect->hw_btnr_loWgtCal_scale = pNearISO->hw_btnr_loWgtCal_scale;
    pSelect->hw_btnr_loWgtCal_offset = pNearISO->hw_btnr_loWgtCal_offset;
    pSelect->hw_btnr_loWgtHfilt_en = pNearISO->hw_btnr_loWgtHfilt_en;
    pSelect->hw_btnr_loWgtVfilt_wgt = pNearISO->hw_btnr_loWgtVfilt_wgt;
    pSelect->hw_btnr_loMgePreWgt_scale = pNearISO->hw_btnr_loMgePreWgt_scale;
    pSelect->hw_btnr_loMgePreWgt_offset = pNearISO->hw_btnr_loMgePreWgt_offset;


    pSelect->hw_btnr_sigmaHdrS_scale = pNearISO->hw_btnr_sigmaHdrS_scale;
    pSelect->hw_btnr_mode0LoWgtHdrS_scale = pNearISO->hw_btnr_mode0LoWgtHdrS_scale;
    pSelect->hw_btnr_mode1LoWgtHdrS_scale = pNearISO->hw_btnr_mode1LoWgtHdrS_scale;
    pSelect->hw_btnr_mode1LoWgtHdrS_offset = pNearISO->hw_btnr_mode1LoWgtHdrS_offset;
    pSelect->hw_btnr_mode1WgtHdrS_scale = pNearISO->hw_btnr_mode1WgtHdrS_scale;

    pSelect->sw_btnr_loMgeFrame_maxLimit = pNearISO->sw_btnr_loMgeFrame_maxLimit;
    pSelect->sw_btnr_loMgeFrame_minLimit = pNearISO->sw_btnr_loMgeFrame_minLimit;
    pSelect->sw_btnr_hiMgeFrame_maxLimit = pNearISO->sw_btnr_hiMgeFrame_maxLimit;
    pSelect->sw_btnr_hiMgeFrame_minLimit = pNearISO->sw_btnr_hiMgeFrame_minLimit;
    pSelect->sw_btnr_loMgeFrameHdrS_maxLimit = pNearISO->sw_btnr_loMgeFrameHdrS_maxLimit;
    pSelect->sw_btnr_loMgeFrameHdrS_minLimit = pNearISO->sw_btnr_loMgeFrameHdrS_minLimit;
    pSelect->sw_btnr_hiMgeFrameHdrS_maxLimit = pNearISO->sw_btnr_hiMgeFrameHdrS_maxLimit;
    pSelect->sw_btnr_hiMgeFrameHdrS_minLimit = pNearISO->sw_btnr_hiMgeFrameHdrS_minLimit;

    pSelect->hw_autoSigCount_en = pNearISO->hw_autoSigCount_en;
    pSelect->hw_autoSigCount_thred = BTNR_INTERP_V30(pLowISO->hw_autoSigCount_thred, pHighISO->hw_autoSigCount_thred, ratio);
    pSelect->hw_autoSigCountWgt_thred = BTNR_INTERP_V30(pLowISO->hw_autoSigCountWgt_thred, pHighISO->hw_autoSigCountWgt_thred, ratio);
    pSelect->sw_autoSigCountFilt_wgt = BTNR_INTERP_V30(pLowISO->sw_autoSigCountFilt_wgt, pHighISO->sw_autoSigCountFilt_wgt, ratio);
    pSelect->sw_autoSigCountSpnr_en = pNearISO->sw_autoSigCountSpnr_en;

    pSelect->sw_btnr_curSpnrFilt_strg = BTNR_INTERP_V30(pLowISO->sw_btnr_curSpnrFilt_strg, pHighISO->sw_btnr_curSpnrFilt_strg, ratio);
    pSelect->sw_btnr_preSpnrFilt_strg = BTNR_INTERP_V30(pLowISO->sw_btnr_preSpnrFilt_strg, pHighISO->sw_btnr_preSpnrFilt_strg, ratio);

    pSelect->hw_btnr_curSpnrHiWgt_minLimit = pNearISO->hw_btnr_curSpnrHiWgt_minLimit;
    pSelect->hw_btnr_preSpnrHiWgt_minLimit = pNearISO->hw_btnr_preSpnrHiWgt_minLimit;
    pSelect->hw_btnr_gainOut_maxLimit = pNearISO->hw_btnr_gainOut_maxLimit;
    pSelect->hw_btnr_noiseBal_mode = pNearISO->hw_btnr_noiseBal_mode;
    pSelect->hw_btnr_noiseBalNr_strg = pNearISO->hw_btnr_noiseBalNr_strg;

    LOGD_ANR("hw_btnr_gainOut_maxLimit :%f hw_btnr_noiseBalNr_strg:%f\n", pSelect->hw_btnr_gainOut_maxLimit, pSelect->hw_btnr_noiseBalNr_strg);
    for (i = 0; i < 20; i++) {
        pSelect->bayertnr_tnr_luma2sigma_idx[i] = pNearISO->bayertnr_tnr_luma2sigma_idx[i];
        pSelect->bayertnr_tnr_luma2sigma_val[i] = BTNR_INTERP_V30(pLowISO->bayertnr_tnr_luma2sigma_val[i], pHighISO->bayertnr_tnr_luma2sigma_val[i], ratio);
    }

    for (i = 0; i < 16; i++) {
        pSelect->bayertnr_spnr_luma2sigma_idx[i] = pNearISO->bayertnr_spnr_luma2sigma_idx[i];
        pSelect->bayertnr_spnr_curluma2sigma_val[i] = BTNR_INTERP_V30(pLowISO->bayertnr_spnr_curluma2sigma_val[i], pHighISO->bayertnr_spnr_curluma2sigma_val[i], ratio);
        pSelect->bayertnr_spnr_preluma2sigma_val[i] = BTNR_INTERP_V30(pLowISO->bayertnr_spnr_preluma2sigma_val[i], pHighISO->bayertnr_spnr_preluma2sigma_val[i], ratio);
    }

    pSelect->sw_curSpnrSpaceWgt_sigma = BTNR_INTERP_V30(pLowISO->sw_curSpnrSpaceWgt_sigma, pHighISO->sw_curSpnrSpaceWgt_sigma, ratio);
    pSelect->sw_iirSpnrSpaceWgt_sigma = BTNR_INTERP_V30(pLowISO->sw_iirSpnrSpaceWgt_sigma, pHighISO->sw_iirSpnrSpaceWgt_sigma, ratio);

    for (i = 0; i < 3; i++) {
        pSelect->hw_tnrWgtFltCoef[i] = pNearISO->hw_tnrWgtFltCoef[i];
    }
    pSelect->hw_bay3d_lowgt_ctrl = pNearISO->hw_bay3d_lowgt_ctrl;
    pSelect->hw_bay3d_lowgt_offinit = pNearISO->hw_bay3d_lowgt_offinit;

    return res;
}



void bay_bifilt13x9_filter_coeff(int graymode, float sigma, int halftaby, int halftabx, uint16_t*rbtab, uint16_t *ggtab)
{
    int halfx = halftabx;
    int halfy = halftaby;
    int winsx = halfx * 2 - 1;
    int winsy = halfy * 2 - 1;
    int centerx = halfx - 1;
    int centery = halfy - 1;
    int i, j;
    float tmpf0, tmpf1;
    float tmpf2, gausstabf[7 * 5];
    int bila_spwgt_coef[9][13];
    int rbtabidx[9][13] =
    {   {6, 0, 5, 0, 4, 0, 3, 0, 4, 0, 5, 0, 6},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {7, 0, 4, 0, 2, 0, 1, 0, 2, 0, 4, 0, 7},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {8, 0, 3, 0, 1, 0, 9, 0, 1, 0, 3, 0, 8},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {7, 0, 4, 0, 2, 0, 1, 0, 2, 0, 4, 0, 7},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6, 0, 5, 0, 4, 0, 3, 0, 4, 0, 5, 0, 6}
    };
    int ggtabidx[9][13] =
    {   {0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 6, 0, 4, 0, 4, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 3, 0, 2, 0, 3, 0, 8, 0, 0},
        {0, 7, 0, 4, 0, 1, 0, 1, 0, 4, 0, 7, 0},
        {10, 0, 5, 0, 2, 0, 9, 0, 2, 0, 5, 0, 10},
        {0, 7, 0, 4, 0, 1, 0, 1, 0, 4, 0, 7, 0},
        {0, 0, 8, 0, 3, 0, 2, 0, 3, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 4, 0, 4, 0, 6, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0}
    };
    int yytabidx[9][13] =
    {   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 6, 5, 4, 3, 4, 5, 6, 0, 0, 0},
        {0, 0, 0, 7, 4, 2, 1, 2, 4, 7, 0, 0, 0},
        {0, 0, 0, 8, 3, 1, 9, 1, 3, 8, 0, 0, 0},
        {0, 0, 0, 7, 4, 2, 1, 2, 4, 7, 0, 0, 0},
        {0, 0, 0, 6, 5, 4, 3, 4, 5, 6, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    for (i = 0; i < winsy; i++)
    {
        for (j = 0; j < winsx; j++)
        {
            tmpf0 = (float)((i - centery) * (i - centery) + (j - centerx) * (j - centerx));
            tmpf0 = tmpf0 / (2 * sigma * sigma);
            tmpf1 = expf(-tmpf0);
            bila_spwgt_coef[i][j] = (int)(tmpf1 * (1 << 10));
            if(bila_spwgt_coef[i][j] > 0x3ff)
                bila_spwgt_coef[i][j] = 0x3ff;
            if(bila_spwgt_coef[i][j] < 0)
                bila_spwgt_coef[i][j] = 0;
        }
    }

    for (i = 0; i < winsy; i++)
    {
        for (j = 0; j < winsx; j++)
        {
            if(rbtabidx[i][j] == 9)
                rbtab[0] = bila_spwgt_coef[i][j];
            else if(rbtabidx[i][j])
                rbtab[rbtabidx[i][j]] = bila_spwgt_coef[i][j];
            if(ggtabidx[i][j] == 9)
                ggtab[0] = bila_spwgt_coef[i][j];
            else if(ggtabidx[i][j] && ggtabidx[i][j] != 10) // 10 and rg_8 combine
                ggtab[ggtabidx[i][j]] = bila_spwgt_coef[i][j];
            if(graymode)
            {
                if(yytabidx[i][j] == 9)
                    ggtab[0] = bila_spwgt_coef[i][j];
                else if(yytabidx[i][j] && yytabidx[i][j] != 10) // 10 and rg_8 combine
                    ggtab[yytabidx[i][j]] = bila_spwgt_coef[i][j];
            }
        }
    }
}


void bay_gauss7x5_filter_coeff(float sigma, int halftaby, int halftabx, int strdtabx, uint16_t *gstab)
{
    int halfx = halftabx;
    int halfy = halftaby;
    int strdx = strdtabx;
    int stridex = strdx / 2;
    int centerx = halfx / 2;
    int centery = halfy / 2;
    int gausstab[7 * 5];
    int i, j, sumc;
    float tmpf0, tmpf1;
    float tmpf2, gausstabf[7 * 5];
    int gstabidx[7 * 5] =
    {   6, 5, 4, 3, 4, 5, 6,
        7, 4, 2, 1, 2, 4, 7,
        8, 3, 1, 0, 1, 3, 8,
        7, 4, 2, 1, 2, 4, 7,
        6, 5, 4, 3, 4, 5, 6
    };

    tmpf2 = 0;
    sumc = 0;
    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            tmpf0 = (float)((i - centery) * (i - centery) + (j - centerx) * (j - centerx));
            tmpf0 = tmpf0 / (2 * sigma * sigma);
            tmpf1 = expf(-tmpf0);
            tmpf2 = tmpf2 + tmpf1;
            gausstabf[i * halfx + j] = tmpf1;
        }
    }
    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gausstab[i * halfx + j] = (int)(gausstabf[i * halfx + j] / tmpf2 * (1 << 10));
            sumc = sumc + gausstab[i * halfx + j];
        }
    }
    gausstab[halfy / 2 * halfx + halfx / 2] += (1024 - sumc);

    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gausstab[i * halfx + j]  = CLIP(gausstab[i * halfx + j], 0, 0x3ff);
            gstab[gstabidx[i * halfx + j]] = gausstab[i * halfx + j];
        }
    }
}

void bayertnr_logtrans_init_V30(int bayertnr_trans_mode,  int bayertnr_trans_mode_scale, Abayertnr_trans_params_v30_t *pTransPrarms)
{
    int i, j;
    int bayertnr_logprecision;
    int bayertnr_logfixbit;
    int bayertnr_logtblbit;
    int bayertnr_logscalebit;
    int bayertnr_logfixmul;
    int bayertnr_logtblmul;

    if(bayertnr_trans_mode)
    {
        bayertnr_logprecision = 6;
        bayertnr_logfixbit = 12;
        bayertnr_logtblbit = 12;
        bayertnr_logscalebit = 6;
        bayertnr_logfixmul = (1 << bayertnr_logfixbit);
        bayertnr_logtblmul = (1 << bayertnr_logtblbit);
        {
            double tmp, tmp1;
            for (i = 0; i < (1 << bayertnr_logprecision); i++)
            {
                tmp1 = ( 1 << bayertnr_logprecision);
                tmp = i;
                tmp = 1 + tmp * 3 / tmp1;
                tmp = sqrt(tmp);
                pTransPrarms->bayertnr_logtablef[i] = int(tmp * bayertnr_logtblmul);

                tmp = i;
                tmp = 1 + tmp / tmp1;
                tmp = pow(tmp, 2);
                pTransPrarms->bayertnr_logtablei[i] = int(tmp * bayertnr_logtblmul);
            }
            pTransPrarms->bayertnr_logtablef[i - 1] = 2 * bayertnr_logtblmul;
            pTransPrarms->bayertnr_logtablei[i - 1] = 4 * bayertnr_logtblmul;
        }
    }
    else
    {
        bayertnr_logprecision = 6;
        bayertnr_logfixbit = 12;
        bayertnr_logtblbit = 12;
        bayertnr_logscalebit = bayertnr_trans_mode_scale + 8;
        bayertnr_logfixmul = (1 << bayertnr_logfixbit);
        bayertnr_logtblmul = (1 << bayertnr_logtblbit);
        {
            double tmp, tmp1;
            for (i = 0; i < (1 << bayertnr_logprecision); i++)
            {
                tmp1 = (1 << bayertnr_logprecision);
                tmp = i;
                tmp = 1 + tmp / tmp1;
                tmp = log(tmp)  /  log(2.0);
                pTransPrarms->bayertnr_logtablef[i] = int(tmp * bayertnr_logtblmul);
                tmp = i;
                tmp = tmp / tmp1;
                tmp = pow(2, tmp);
                pTransPrarms->bayertnr_logtablei[i] = int(tmp * bayertnr_logtblmul);
            }
            pTransPrarms->bayertnr_logtablef[i - 1] = 1 * bayertnr_logtblmul;
            pTransPrarms->bayertnr_logtablei[i - 1] = 2 * bayertnr_logtblmul;
        }
    }

    pTransPrarms->bayertnr_logprecision = bayertnr_logprecision;
    pTransPrarms->bayertnr_logfixbit = bayertnr_logfixbit;
    pTransPrarms->bayertnr_logtblbit = bayertnr_logtblbit;
    pTransPrarms->bayertnr_logscalebit = bayertnr_logscalebit;
    pTransPrarms->bayertnr_logfixmul = bayertnr_logfixmul;
    pTransPrarms->bayertnr_logtblmul = bayertnr_logtblmul;
}

int  bayertnr_find_top_one_pos_V30(int data)
{
    int i, j = 1;
    int pos = 0;

    for(i = 0; i < 32; i++)
    {
        if(data & j)
        {
            pos = i;
        }
        j = j << 1;
    }

    return pos;
}

int  bayertnr_logtrans_V30(uint32_t tmpfix, RK_Bayertnr_Fix_V30_t* pFix, Abayertnr_trans_params_v30_t *pTransPrarms)
{
    long long x8, one = 1;
    long long gx, n, ix1, ix2, dp;
    long long lt1, lt2, dx, fx;
    int bayertnr_logprecision = pTransPrarms->bayertnr_logprecision;
    int bayertnr_logfixbit = pTransPrarms->bayertnr_logfixbit;
    int bayertnr_logtblbit = pTransPrarms->bayertnr_logtblbit;
    int bayertnr_logscalebit = pTransPrarms->bayertnr_logscalebit;
    int bayertnr_logfixmul = pTransPrarms->bayertnr_logfixmul;
    int bayertnr_logtblmul = pTransPrarms->bayertnr_logtblmul;

    if(pFix->transf_mode)
    {
        long long dn;

        x8 = MIN((tmpfix + pFix->transf_mode_offset), pFix->transf_data_max_limit);

        // find highest bit
        n  = (long long)bayertnr_find_top_one_pos_V30((int)x8);
        n  = n >> 1;
        dn = n * 2;

        gx = x8 - (one << dn);
        gx = gx * (one << bayertnr_logprecision) * bayertnr_logfixmul;
        gx = gx / (one << dn);
        gx = gx / 3;

        ix1 = gx >> bayertnr_logfixbit;
        dp = gx - ix1 * bayertnr_logfixmul;

        dp = dp / 64;       // opt
        ix2 = ix1 + 1;

        lt1 = pTransPrarms->bayertnr_logtablef[ix1];
        lt2 = pTransPrarms->bayertnr_logtablef[ix2];

        dx = lt1 * (bayertnr_logfixmul / 64 - dp) + lt2 * dp;   // opt
        dp = dp * 64;       // opt

        fx = dx + (one << (bayertnr_logfixbit + bayertnr_logtblbit - bayertnr_logscalebit - n - 1));
        fx = fx >> (bayertnr_logfixbit + bayertnr_logtblbit - bayertnr_logscalebit - n);

        fx = fx - pFix->itransf_mode_offset;
    }
    else
    {
        x8 = MIN((tmpfix + pFix->transf_mode_offset), pFix->transf_data_max_limit);

        // find highest bit
        n = (long long)bayertnr_find_top_one_pos_V30((int)x8);

        gx = x8 - (one << n);
        gx = gx * (one << bayertnr_logprecision) * bayertnr_logfixmul;
        gx = gx / (one << n);

        ix1 = gx >> bayertnr_logfixbit;
        dp = gx - ix1 * bayertnr_logfixmul;

        dp = (dp + 32) / 64;        // opt
        ix2 = ix1 + 1;

        lt1 = pTransPrarms->bayertnr_logtablef[ix1];
        lt2 = pTransPrarms->bayertnr_logtablef[ix2];

        dx = lt1 * (bayertnr_logfixmul / 64 - dp) + lt2 * dp; // opt
        dx = dx * 64;       // opt

        fx = dx + n * (one << (bayertnr_logfixbit + bayertnr_logtblbit));
        fx = fx + (one << (bayertnr_logfixbit + bayertnr_logtblbit - bayertnr_logscalebit - 1));
        fx = fx >> (bayertnr_logfixbit + bayertnr_logtblbit - bayertnr_logscalebit);

        fx = fx - pFix->itransf_mode_offset;
    }

    return (int)fx;
}

int bayertnr_kalm_bitcut_V30(int datain, int bitsrc, int bitdst)
{
    int out;
    out = bitsrc == bitdst ? datain : ((datain + (1 << (bitsrc - bitdst - 1))) >> (bitsrc - bitdst));
    return out;
}

int bayertnr_tnr_noise_curve_V30(int data, int isHdrShort, RK_Bayertnr_Fix_V30_t* pFix, Abayertnr_trans_params_v30_t *pTransPrarms)
{
    int sigbins = 20;
    int dbl_en  = pFix->sigma_curve_double_en;
    int dbl_pos = pTransPrarms->bayertnr_tnr_sigma_curve_double_pos;
    int i, sigma;
    int ratio;

    for(i = 0; i < sigbins; i++)
    {
        if(data < pFix->tnr_luma_sigma_x[i])
            break;
    }

    if(isHdrShort && dbl_en && i <= dbl_pos)
        sigma = pFix->tnr_luma_sigma_y[dbl_pos];
    else if(dbl_en && i == dbl_pos)
        sigma = pFix->tnr_luma_sigma_y[dbl_pos];
    else if(i <= 0)
        sigma = pFix->tnr_luma_sigma_y[0];
    else if(i > (sigbins - 1))
        sigma = pFix->tnr_luma_sigma_y[sigbins - 1];
    else
    {
        ratio = (data - pFix->tnr_luma_sigma_x[i - 1]) * (pFix->tnr_luma_sigma_y[i] - pFix->tnr_luma_sigma_y[i - 1]);
        ratio = ratio / (pFix->tnr_luma_sigma_x[i] - pFix->tnr_luma_sigma_x[i - 1]);

        sigma = pFix->tnr_luma_sigma_y[i - 1] + ratio;
    }

    return sigma;
}

int bayertnr_wgt_sqrt_tab_V30(int index)
{
    int i, res, ratio;
    int len = 10;
    int tab_x[10] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};
    int tab_y[10] = {0, 16, 23, 32, 45, 64, 91, 128, 181, 256};

    for(i = 0; i < len; i++)
    {
        if(index < tab_x[i])
            break;
    }

    if(i <= 0)
        res = tab_y[0];
    else if(i > len - 1)
        res = tab_y[len - 1];
    else
    {
        ratio = (index - tab_x[i - 1]);
        ratio = ratio * (tab_y[i] - tab_y[i - 1]);
        ratio = ratio / (tab_x[i] - tab_x[i - 1]);
        res = (int)(tab_y[i - 1] + ratio);
    }

    return res;
}

int bayertnr_autosigma_config_V30(bool isFirstFrame,
                                  Abayertnr_Stats_V30_t *pStats,
                                  Abayertnr_trans_params_v30_t *pTransPrarms)
{
    bool auto_en = pTransPrarms->bayertnr_auto_sig_count_en;
    // update auto sigma curve
    if(auto_en  && !isFirstFrame)
    {
        int sigma_bins = 20;
        uint16_t tmp = 0;
        uint16_t *sigmay_data = pStats->tnr_auto_sigma_calc;
        uint16_t *sigmay_curve = pTransPrarms->tnr_luma_sigma_y;
        int iir_wgt  = pTransPrarms->bayertnr_auto_sig_count_filt_wgt;
        int filt_coef[5] = {0, 0, 1, 0, 0};
        int sigmay_calc[24 + 2 + 2], sigmay_tmp[24];
        int i = 0, j = 0, tmp0 = 0, tmp1 = 0, coefacc = 0;

        sigmay_calc[0] = sigmay_data[0];
        sigmay_calc[1] = sigmay_data[0];
        for (j = 0; j < sigma_bins; j++)
        {
            sigmay_calc[j + 2] = sigmay_data[j];
        }
        sigmay_calc[2 + sigma_bins + 0] = sigmay_data[sigma_bins - 1];
        sigmay_calc[2 + sigma_bins + 1] = sigmay_data[sigma_bins - 1];

        for (j = 0; j < 5; j++)
            coefacc += filt_coef[j];
        for (j = 2; j < sigma_bins + 2; j++)
        {
            tmp1 = sigmay_calc[j - 2] * filt_coef[0] + sigmay_calc[j - 1] * filt_coef[1];
            tmp1 = tmp1 + sigmay_calc[j] * filt_coef[2] + sigmay_calc[j + 1] * filt_coef[3];
            tmp1 = tmp1 + sigmay_calc[j + 2] * filt_coef[4];
            sigmay_tmp[j - 2] = tmp1 / coefacc;
        }

        // calc special point
        for (j = 0; j < sigma_bins; j++)
        {
            if(sigmay_tmp[j] == 0)
            {
                for (i = 1; i < sigma_bins + 1; i++)
                {
                    tmp0 = CLIP((j - i), 0, sigma_bins);
                    if(sigmay_tmp[tmp0])
                        break;
                    tmp0 = CLIP((j + i), 0, sigma_bins);
                    if(sigmay_tmp[tmp0])
                        break;
                }
                sigmay_tmp[j] = sigmay_tmp[tmp0];
                if(i >= sigma_bins)
                {
                    //printf("sigmay_curve no point use!\n");
                    pTransPrarms->bayertnr_auto_sig_count_valid = 0;
                    break;
                }
            }
        }

        // sigma iir
        for (j = 1; j < sigma_bins; j++)
        {
            sigmay_tmp[j] = MAX(sigmay_tmp[j], sigmay_tmp[j - 1]);
        }

        LOGD_ANR("pStats->framid:%d sigma_curve capture 0x%x point sigma_cout_max:0x%x !\n",
                 pStats->frame_id,
                 pStats->tnr_auto_sigma_count,
                 pTransPrarms->bayertnr_auto_sig_count_max);
        if(pStats->tnr_auto_sigma_count < pTransPrarms->bayertnr_auto_sig_count_max)
        {
            LOGD_ANR("sigma_curve capture 0x%x point not enough, sigma_cout_max:0x%x !\n",
                     pStats->tnr_auto_sigma_count, pTransPrarms->bayertnr_auto_sig_count_max);
            pTransPrarms->bayertnr_auto_sig_count_valid = 0;
        }
        else
            pTransPrarms->bayertnr_auto_sig_count_valid = 1;

        if(pTransPrarms->bayertnr_auto_sig_count_valid == 0)
            iir_wgt = 1024;
        for (j = 0; j < sigma_bins; j++)
        {
            tmp = (iir_wgt * sigmay_curve[j] + (1024 - iir_wgt) * sigmay_tmp[j]) >> 10;

            LOGD_ANR("%s:%d  iir_wgt:%d sigma[%d]: auto:0x%x  pre:0x%x sigmay_tmp:0x%x reg:0x%x\n",
                     __FUNCTION__, __LINE__,
                     iir_wgt, j, pStats->tnr_auto_sigma_calc[j],
                     sigmay_curve[j], sigmay_tmp[j], tmp);

            sigmay_curve[j] = tmp;
        }

    }

    return 0;
}

Abayertnr_result_V30_t bayertnr_fix_transfer_V30(void* pSelect_v, RK_Bayertnr_Fix_V30_t* pFix,
        rk_aiq_bayertnr_strength_v30_t* pStrength,
        Abayertnr_ExpInfo_V30_t* pExpInfo,
        Abayertnr_trans_params_v30_t *pTransPrarms,
        Abayertnr_Stats_V30_t *pStats) {
    int i = 0;
    int tmp, tmp0, tmp1;
    float frameiso[3];
    float frameEt[3];
    float fdGain[3];

    RK_Bayertnr_Params_V30_Select_t* pSelect = (RK_Bayertnr_Params_V30_Select_t*)pSelect_v;


    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    int framenum = pExpInfo->hdr_mode + 1;
    frameiso[0] = pExpInfo->arIso[0];
    frameiso[1] = pExpInfo->arIso[1];
    frameiso[2] = pExpInfo->arIso[2];

    frameEt[0] = pExpInfo->arTime[0];
    frameEt[1] = pExpInfo->arTime[1];
    frameEt[2] = pExpInfo->arTime[2];


    for (i = 0; i < framenum; i++) {
        fdGain[i] = frameiso[i] * frameEt[i];
        LOGD_ANR("bayernrv23 idx[%d] iso:%f time:%f dgain:%f\n",
                 i, frameiso[i], frameEt[i], fdGain[i]);
    }

    for (i = 0; i < framenum; i++) {
        fdGain[i] = fdGain[framenum - 1] / fdGain[i];
    }

    if(pStrength->strength_enable)
        fStrength = pStrength->percent;

    if(fStrength <= 0.0f) {
        fStrength = 0.000001;
    }

    LOGD_ANR("strength_enable:%d fStrength: %f \n", pStrength->strength_enable, fStrength);

    int bayertnr_default_noise_curve_use = 0;
    int bayertnr_iso_cur = pExpInfo->arIso[pExpInfo->hdr_mode];
    bayertnr_default_noise_curve_use = bayertnr_find_top_one_pos_V30(bayertnr_iso_cur) != bayertnr_find_top_one_pos_V30(pTransPrarms->bayertnr_iso_pre);
    LOGD_ANR("preiso:%d cur:%d use:%d  hdrframeNum:%d\n", pTransPrarms->bayertnr_iso_pre, bayertnr_iso_cur, bayertnr_default_noise_curve_use, framenum);

    pTransPrarms->bayertnr_iso_pre = bayertnr_iso_cur;
    // BAY3D_CTRL0 0x2c00
    pFix->bypass_en = !pSelect->enable;
    pFix->iirsparse_en = 0;

    //BAY3D_CTRL1 0x2c04
    pFix->transf_bypass_en =  (framenum > 1 || pExpInfo->blc_ob_predgain > 1.0f) ? 0 : !pSelect->hw_btnr_transf_en;
    pFix->sigma_curve_double_en =  (framenum > 1) ? 1 : 0;
    pFix->cur_spnr_bypass_en = !pSelect->hw_btnr_curSpnr_en ;
    pFix->cur_spnr_sigma_idxfilt_bypass_en = !pSelect->hw_btnr_curSpnrSigmaIdxFilt_en;
    pFix->cur_spnr_sigma_curve_double_en = (framenum > 1) ? 1 : 0;
    pFix->pre_spnr_bypass_en = !pSelect->hw_btnr_preSpnr_en;
    pFix->pre_spnr_sigma_idxfilt_bypass_en = !pSelect->hw_btnr_preSpnrSigmaIdxFilt_en;
    pFix->pre_spnr_sigma_curve_double_en = (framenum > 1) ? 1 : 0;
    pFix->lpf_hi_bypass_en = !pSelect->hw_btnr_lpfHi_en;
    pFix->lo_diff_vfilt_bypass_en = !pSelect->hw_btnr_loDiffVfilt_en;
    pFix->lpf_lo_bypass_en = !pSelect->hw_btnr_lpfLo_en;
    pFix->lo_wgt_hfilt_en = pSelect->hw_btnr_loWgtHfilt_en;
    pFix->lo_diff_hfilt_en = pSelect->hw_btnr_loDiffHfilt_en;
    pFix->sig_hfilt_en = pSelect->hw_btnr_sigmaHfilt_en;
    pFix->gkalman_en = pSelect->hw_btnr_gKalman_en;
    pFix->spnr_pre_sigma_use_en = pSelect->hw_btnr_spnrPresigmaUse_en;
    pFix->lo_detection_mode = pSelect->hw_loDetection_mode;
    if(pSelect->hw_btnr_loDetection_en == 0) {
        pFix->lo_detection_mode = 3;
    }
    pFix->md_wgt_out_en = pSelect->hw_mdWgtOut_en;
    pFix->md_bypass_en = pSelect->hw_mdBypss_en;
    pFix->iirspnr_out_en = pSelect->hw_iirSpnrOut_en;
    pFix->lomdwgt_dbg_en = pSelect->hw_loMdWgtdbg_en;
    pFix->curdbg_out_en = pSelect->hw_curFiltOut_en;
    pFix->noisebal_mode = pSelect->hw_btnr_noiseBal_mode;


    // BAY3D_CTRL2 0x2c08
    int last_transf_mode = pFix->transf_mode;
    int last_transf_mode_scale = pFix->transf_mode_scale;
    pFix->transf_mode = (framenum > 1) ? 0 : pSelect->hw_btnr_transf_mode;
    pFix->wgt_cal_mode = pSelect->hw_btnr_wgtCal_mode;
    pFix->mge_wgt_ds_mode = 0;
    pFix->kalman_wgt_ds_mode = 0;
    pFix->mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;
    pFix->sigma_calc_mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;

    // BAY3D_TRANS0 0x2c0c
    tmp = (pFix->transf_mode) ? pSelect->sw_btnr_transfMode1_offset : pSelect->sw_btnr_transfMode0_offset ;
    pFix->transf_mode_offset = CLIP(tmp, 0, 0x1fff);
    tmp = (framenum > 1) ? 0 : pSelect->hw_btnr_transfMode0_scale;
    pFix->transf_mode_scale = CLIP(tmp, 0, 0x01);
    tmp = (pFix->transf_mode) ? pSelect->sw_btnr_itransfMode1_offset : pSelect->sw_btnr_itransfMode0_offset ;;
    pFix->itransf_mode_offset = CLIP(tmp, 0, 0xffff);

    // BAY3D_TRANS1 0x2c10
    tmp = pSelect->hw_btnr_transfData_maxLimit;
    pFix->transf_data_max_limit = CLIP(tmp, 0, 0xfffff);

    // BAY3D_CURDGAIN 0x2c14
    tmp = pSelect->hw_btnr_curSpnrSigmaHdrS_scale * (1 << FIXBITDGAIN);
    pFix->cur_spnr_sigma_hdr_sht_scale = CLIP(tmp, 0, 0xffff);
    tmp = pSelect->hw_btnr_curSpnrSigma_scale * (1 << FIXBITDGAIN);;
    pFix->cur_spnr_sigma_scale = CLIP(tmp, 0, 0xffff);

    // tnr sigma curve must calculate before spnr sigma
    float kcoef0, kcoef1;
    int max_sig, pix_max;
    if(pExpInfo->isFirstFrame || (last_transf_mode != pFix->transf_mode) || (last_transf_mode_scale != pFix->transf_mode_scale)) {
        bayertnr_logtrans_init_V30(pFix->transf_mode, pFix->transf_mode_scale, pTransPrarms);
    }
    int sigbins = 20;
    pix_max = pFix->transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans_V30((1 << 12) - 1, pFix, pTransPrarms);
    if(framenum == 2 ) {
        //pFix->sigma_curve_double_en = 1;
        pTransPrarms->bayertnr_tnr_sigma_curve_double_pos = 10;
        // hdr long bins
        int lgbins = 10;
        for(i = 0; i < lgbins; i++) {
            pFix->tnr_luma_sigma_x[i] = 128 * (i + 1);
        }

        // hdr short bins
        int shbins = sigbins - lgbins;
        for(i = 0; i < 8; i++) {
            if(fdGain[0] <= (1 << i))
                break;
        }
        i = 8;
        tmp = (1 << (12 + i)) - 1; // luma
        tmp1 = pFix->transf_bypass_en ? tmp : bayertnr_logtrans_V30(tmp, pFix, pTransPrarms);
        tmp = tmp1 - pix_max;
        for(i = lgbins; i < lgbins + 6; i++) {
            tmp = 128 * (i - lgbins + 1)  + pFix->tnr_luma_sigma_x[lgbins - 1]; //pParser->bayertnr_tnr_lum[i];
            pFix->tnr_luma_sigma_x[i] = tmp;
            //printf("tnr sigma x idx:%d 0x%x\n", i,pFix->tnr_luma_sigma_x[i]);
        }
        for(i = lgbins + 6; i < sigbins; i++) {
            tmp = 256 * (i - lgbins - 6 + 1)  + pFix->tnr_luma_sigma_x[lgbins + 6 - 1]; //pParser->bayertnr_tnr_lum[i];
            pFix->tnr_luma_sigma_x[i] = tmp;
            //printf("tnr sigma x idx:%d 0x%x\n", i,pFix->tnr_luma_sigma_x[i]);
        }
        pFix->tnr_luma_sigma_x[sigbins - 1] = tmp1;
        for(i = 0; i < 20; i++) {
            //printf("tnr sigma x idx:%d 0x%x\n", i,pFix->tnr_luma_sigma_x[i]);
        }
    } else if(!pFix->transf_bypass_en)   {
        int segs = 6;
        //pTransPrarms->bayertnr_tnr_sigma_curve_double_en = 0;
        pTransPrarms->bayertnr_tnr_sigma_curve_double_pos = 0;
        for(i = 0; i < segs; i++) {
            pFix->tnr_luma_sigma_x[i] = 32 * (i + 1);
        }
        for(i = segs; i < sigbins; i++) {
            pFix->tnr_luma_sigma_x[i] = 64 * (i - segs + 1) + 32 * segs;
        }
        pFix->tnr_luma_sigma_x[sigbins - 1] = pix_max;
    }
    else
    {
        //pFix->sigma_curve_double_en = 0;
        pTransPrarms->bayertnr_tnr_sigma_curve_double_pos = 0;
        for(i = 0; i < sigbins / 2; i++) {
            pFix->tnr_luma_sigma_x[i] = 128 * (i + 1);
            pFix->tnr_luma_sigma_x[i + sigbins / 2] = 256 * (i + 1) + 128 * 10;
        }
        pFix->tnr_luma_sigma_x[sigbins - 1] = pix_max;
    }

    // if auto sigma count??  use update params?
    if(pSelect->hw_autoSigCount_en == 1 && !pExpInfo->isFirstFrame) {
        pTransPrarms->bayertnr_auto_sig_count_en = pSelect->hw_autoSigCount_en;
        pTransPrarms->bayertnr_auto_sig_count_filt_wgt = pSelect->sw_autoSigCountFilt_wgt * (1 << 10);
        pTransPrarms->bayertnr_auto_sig_count_max = pTransPrarms->rawWidth * pTransPrarms->rawHeight / 3;
        bayertnr_autosigma_config_V30(pExpInfo->isFirstFrame, pStats, pTransPrarms);
    }

    if(pSelect->hw_autoSigCount_en == 0 || pExpInfo->isFirstFrame || bayertnr_default_noise_curve_use)
    {
        kcoef0 = pSelect->sw_btnr_tnrFilt_strg;
        max_sig = ((1 << 12) - 1);
        for(i = 0; i < sigbins; i++) {
            pFix->tnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_tnr_luma2sigma_val[i] * kcoef0), 0, max_sig);
            pTransPrarms->tnr_luma_sigma_y[i] = pFix->tnr_luma_sigma_y[i];
        }
    } else {
        kcoef0 = pSelect->sw_btnr_tnrFilt_strg;
        max_sig = ((1 << 12) - 1);
        for(i = 0; i < sigbins; i++)
            pFix->tnr_luma_sigma_y[i] = CLIP((int)(pTransPrarms->tnr_luma_sigma_y[i] * kcoef0), 0, max_sig);
    }



    int spnrsigbins = 16;
    int auto_sig_curve_spnruse = pSelect->sw_autoSigCountSpnr_en;
    pix_max = pFix->transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans_V30((1 << 12) - 1, pFix, pTransPrarms);
    if(framenum == 2) {
        //pFix->cur_spnr_sigma_curve_double_en = 1;
        //pFix->pre_spnr_sigma_curve_double_en = 1;
        //pTransPrarms->bayertnr_cur_spnr_sigma_curve_double_pos = 8;
        //pTransPrarms->bayertnr_pre_spnr_sigma_curve_double_pos = 8;
        int lgbins = 8;
        float kcoef0 = pSelect->sw_btnr_curSpnrFilt_strg;
        float kcoef1 = pSelect->sw_btnr_preSpnrFilt_strg;
        int max_sig = ((1 << 12) - 1);
        // hdr long bins
        for(i = 0; i < lgbins; i++) {
            pFix->cur_spnr_luma_sigma_x[i] = 128 * (i + 1);
            pFix->pre_spnr_luma_sigma_x[i] = 128 * (i + 1);
        }
        for(i = 0; i < lgbins; i++) {
            if(!auto_sig_curve_spnruse || pExpInfo->isFirstFrame) {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_spnr_curluma2sigma_val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_spnr_preluma2sigma_val[i] * kcoef1), 0, max_sig);
            }
            else {
                tmp = bayertnr_tnr_noise_curve_V30(pFix->cur_spnr_luma_sigma_x[i], 0, pFix, pTransPrarms);
                pFix->cur_spnr_luma_sigma_y[i]  = CLIP((int)(tmp * kcoef0), 0, max_sig);
                tmp = bayertnr_tnr_noise_curve_V30(pFix->pre_spnr_luma_sigma_x[i], 0, pFix, pTransPrarms);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef1), 0, max_sig);
            }
        }

        // hdr short bins, max gain 256
        int shbins = spnrsigbins - lgbins;
        for(i = 0; i < 8; i++) {
            if(fdGain[0] <= (1 << i))
                break;
        }
        i = 8;//MIN(i, 8);
        tmp = (1 << (12 + i)) - 1;
        tmp1 =  pFix->transf_bypass_en ? tmp : bayertnr_logtrans_V30(tmp, pFix, pTransPrarms);
        tmp = tmp1 - pix_max;
        for(i = lgbins; i < spnrsigbins ; i++) {
            pFix->cur_spnr_luma_sigma_x[i] = 256 * (i - lgbins + 1) + pFix->cur_spnr_luma_sigma_x[lgbins - 1];
            pFix->pre_spnr_luma_sigma_x[i] = 256 * (i - lgbins + 1) + pFix->pre_spnr_luma_sigma_x[lgbins - 1];
        }
        pFix->cur_spnr_luma_sigma_x[spnrsigbins - 1] = tmp1;
        pFix->pre_spnr_luma_sigma_x[spnrsigbins - 1] = tmp1;

        for(i = lgbins; i < spnrsigbins; i++)
        {
            if(!auto_sig_curve_spnruse || pExpInfo->isFirstFrame)
            {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_spnr_curluma2sigma_val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_spnr_preluma2sigma_val[i] * kcoef1), 0, max_sig);
            }
            else
            {
                tmp = bayertnr_tnr_noise_curve_V30(pFix->cur_spnr_luma_sigma_x[i], 0, pFix, pTransPrarms);
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef0), 0, max_sig);
                tmp = bayertnr_tnr_noise_curve_V30(pFix->pre_spnr_luma_sigma_x[i], 0, pFix, pTransPrarms);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef1), 0, max_sig);
            }
        }
    }
    else if(!pFix->transf_bypass_en) {
        float kcoef0 = pSelect->sw_btnr_curSpnrFilt_strg;
        float kcoef1 = pSelect->sw_btnr_preSpnrFilt_strg;
        max_sig = ((1 << 12) - 1);
        // hdr long bins
        for(i = 0; i < spnrsigbins; i++)
        {
            pFix->cur_spnr_luma_sigma_x[i] = 64 * (i + 1);
            pFix->pre_spnr_luma_sigma_x[i] = 64 * (i + 1);
        }
        pFix->cur_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;
        pFix->pre_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;

        for(i = 0; i < spnrsigbins; i++)
        {
            if(!auto_sig_curve_spnruse || pExpInfo->isFirstFrame)
            {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_spnr_curluma2sigma_val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_spnr_preluma2sigma_val[i] * kcoef1), 0, max_sig);
            }
            else
            {
                tmp = bayertnr_tnr_noise_curve_V30(pFix->cur_spnr_luma_sigma_x[i], 0, pFix, pTransPrarms);
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef0), 0, max_sig);
                tmp = bayertnr_tnr_noise_curve_V30(pFix->pre_spnr_luma_sigma_x[i], 0, pFix, pTransPrarms);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef1), 0, max_sig);
            }
        }
    } else {
        //pFix->cur_spnr_sigma_curve_double_en = 0;
        // pFix->pre_spnr_sigma_curve_double_en = 0;
        // pTransPrarms->bayertnr_cur_spnr_sigma_curve_double_pos = 0;
        // pTransPrarms->bayertnr_pre_spnr_sigma_curve_double_pos = 0;
        int lgbins = 0;
        float kcoef0 = pSelect->sw_btnr_curSpnrFilt_strg;
        float kcoef1 = pSelect->sw_btnr_preSpnrFilt_strg;
        int max_sig = ((1 << 12) - 1);
        for(i = 0; i < spnrsigbins; i++)
        {
            pFix->cur_spnr_luma_sigma_x[i] = 256 * (i + 1);
            pFix->pre_spnr_luma_sigma_x[i] = 256 * (i + 1);
        }
        pFix->cur_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;
        pFix->pre_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;

        for(i = 0; i < spnrsigbins; i++)
        {
            if(!auto_sig_curve_spnruse || pExpInfo->isFirstFrame )
            {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_spnr_curluma2sigma_val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pSelect->bayertnr_spnr_preluma2sigma_val[i] * kcoef1), 0, max_sig);
            }
            else
            {
                tmp = bayertnr_tnr_noise_curve_V30(pFix->cur_spnr_luma_sigma_x[i], 0, pFix, pTransPrarms);
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef0), 0, max_sig);
                tmp = bayertnr_tnr_noise_curve_V30(pFix->pre_spnr_luma_sigma_x[i], 0, pFix, pTransPrarms);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef1), 0, max_sig);
            }
        }
    }


    /* BAY3D_CURGAIN_OFF 0x2c58 */
    pFix->cur_spnr_sigma_rgain_offset = 0;
    pFix->cur_spnr_sigma_bgain_offset = 0;

    /* BAY3D_CURSIG_OFF */
    tmp = pSelect->hw_btnr_curSpnrSigmaHdrS_offset * (1 << FIXBITDGAIN) ;
    pFix->cur_spnr_sigma_hdr_sht_offset = CLIP(tmp, 0, 0xfff);
    tmp = pSelect->hw_btnr_curSpnrSigma_offset * (1 << FIXBITDGAIN);
    pFix->cur_spnr_sigma_offset = CLIP(tmp, 0, 0xfff);


    /* BAY3D_CURWTH */
    pFix->cur_spnr_pix_diff_max_limit = (1 << 12) - 1;
    tmp = pSelect->hw_btnr_curSpnrWgtCal_offset * (1 << 10);
    pFix->cur_spnr_wgt_cal_offset = CLIP(tmp, 0, 0xfff);

    /* BAY3D_CURBFALP */
    tmp = (pExpInfo->isFirstFrame) ? 0 : pSelect->hw_btnr_curSpnr_wgt * (1 << 10);
    pFix->cur_spnr_wgt = CLIP(tmp, 0, 0x3ff);
    tmp = pSelect->hw_btnr_preSpnr_wgt * (1 << 10);
    pFix->pre_spnr_wgt = CLIP(tmp, 0, 0x3ff);

    /* BAY3D_CURWDC */
    //float sigmacur = 25;
    bay_bifilt13x9_filter_coeff(pExpInfo->gray_mode, pSelect->sw_curSpnrSpaceWgt_sigma, 5, 7, pFix->cur_spnr_space_rb_wgt, pFix->cur_spnr_space_gg_wgt);

    /* BAY3D_IIRDGAIN */
    tmp = pSelect->hw_btnr_preSpnrSigmaHdrS_scale * (1 << FIXBITDGAIN);
    pFix->pre_spnr_sigma_hdr_sht_scale = CLIP(tmp, 0, 0xfff);
    tmp = pSelect->hw_btnr_preSpnrSigma_scale * (1 << FIXBITDGAIN);
    pFix->pre_spnr_sigma_scale = CLIP(tmp, 0, 0xfff);

    /* BAY3D_IIRGAIN_OFF */
    pFix->pre_spnr_sigma_rgain_offset = 0;
    pFix->pre_spnr_sigma_bgain_offset = 0;

    /* BAY3D_IIRSIG_OFF */
    tmp = pSelect->hw_btnr_preSpnrSigmaHdrS_offset * (1 << FIXBITDGAIN) ;
    pFix->pre_spnr_sigma_hdr_sht_offset = CLIP(tmp, 0, 0xfff);
    tmp = pSelect->hw_btnr_preSpnrSigma_offset * (1 << FIXBITDGAIN);
    pFix->pre_spnr_sigma_offset = CLIP(tmp, 0, 0xfff);

    /* BAY3D_IIRWTH */
    pFix->pre_spnr_pix_diff_max_limit = (1 << 12) - 1;
    tmp = pSelect->hw_btnr_preSpnrWgtCal_offset * (1 << 10);
    pFix->pre_spnr_wgt_cal_offset = CLIP(tmp, 0, 0xfff);

    /* BAY3D_IIRWDC */
    /* BAY3D_IIRWDY */
    //float sigmaiir = 25;
    bay_bifilt13x9_filter_coeff(pExpInfo->gray_mode, pSelect->sw_iirSpnrSpaceWgt_sigma, 5, 7, pFix->pre_spnr_space_rb_wgt, pFix->pre_spnr_space_gg_wgt);


    /* BAY3D_BFCOEF */
    tmp = pSelect->hw_btnr_curSpnrWgtCal_scale * (1 << 10) ;
    pFix->cur_spnr_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_preSpnrWgtCal_scale * (1 << 10);
    pFix->pre_spnr_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRSIG_X */
    /* BAY3D_TNRSIG_Y */
    //tnr sigma curve must calculate befor spnr sigma curve

    /* BAY3D_TNRHIW */
    bay_gauss7x5_filter_coeff(pSelect->sw_btnr_lpfHi_sigma, 5, 7, 7, pFix->tnr_lpf_hi_coeff);

    /* BAY3D_TNRLOW*/
    bay_gauss7x5_filter_coeff(pSelect->sw_btnr_lpfLo_sigma, 5, 7, 7, pFix->tnr_lpf_lo_coeff);

    /* BAY3D_TNRGF3 */

    tmp = pSelect->hw_tnrWgtFltCoef[0] * (1 << 6);
    pFix->tnr_wgt_filt_coeff0 = CLIP(tmp, 0, 0x3ff);
    tmp = pSelect->hw_tnrWgtFltCoef[1] * (1 << 6);
    pFix->tnr_wgt_filt_coeff1 = CLIP(tmp, 0, 0x3ff);
    tmp = pSelect->hw_tnrWgtFltCoef[2] * (1 << 6);
    pFix->tnr_wgt_filt_coeff2 = CLIP(tmp, 0, 0x3ff);
    tmp =  1024 - pFix->tnr_wgt_filt_coeff1 * 4 - pFix->tnr_wgt_filt_coeff2 * 4;
    if(tmp < 0) {
        LOGE_ANR("%s:%d: hw_tnrWgtFltCoef:%f + 4*%f + 4*%f = %f should less than 16, use[4,2,1]instead, please check iq params!\n",
                 pSelect->hw_tnrWgtFltCoef[0], pSelect->hw_tnrWgtFltCoef[1], pSelect->hw_tnrWgtFltCoef[2],
                 pSelect->hw_tnrWgtFltCoef[0] + 4 * pSelect->hw_tnrWgtFltCoef[1] + 4 * pSelect->hw_tnrWgtFltCoef[2]);
        pFix->tnr_wgt_filt_coeff0 = 4 * (1 << 6);
        pFix->tnr_wgt_filt_coeff1 = 2 * (1 << 6);
        pFix->tnr_wgt_filt_coeff2 = 1 * (1 << 6);
    } else {
        pFix->tnr_wgt_filt_coeff0 = CLIP(tmp, 0, 0x3ff);
    }


    /* BAY3D_TNRSIGSCL */
    tmp = pSelect->hw_btnr_sigma_scale * fStrength * (1 << 10);
    pFix->tnr_sigma_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_sigmaHdrS_scale * fStrength * (1 << 10);
    pFix->tnr_sigma_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRVIIR */
    tmp = pSelect->hw_btnr_sigmaVfilt_wgt * (1 << 4);
    pFix->tnr_sig_vfilt_wgt  = CLIP(tmp, 0, 0xf);
    tmp = pSelect->hw_btnr_loDiffVfilt_wgt * (1 << 4);
    pFix->tnr_lo_diff_vfilt_wgt  = CLIP(tmp, 0, 0xf);
    tmp = pSelect->hw_btnr_loWgtVfilt_wgt * (1 << 4);
    pFix->tnr_lo_wgt_vfilt_wgt  = CLIP(tmp, 0, 0xf);
    tmp = pSelect->hw_btnr_sigmaFirstLine_scale * (1 << 4);
    pFix->tnr_sig_first_line_scale = CLIP(tmp, 0, 0x1f);
    tmp = pSelect->hw_btnr_loDiffFirstLine_scale * (1 << 4);
    pFix->tnr_lo_diff_first_line_scale = CLIP(tmp, 0, 0x1f);

    /* BAY3D_TNRLFSCL */
    tmp = pSelect->hw_btnr_loWgtCal_offset * (1 << 10);
    pFix->tnr_lo_wgt_cal_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_loWgtCal_scale * (1 << 10);
    pFix->tnr_lo_wgt_cal_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRLFSCLTH */
    tmp = pSelect->hw_btnr_loWgtCal_maxLimit * (1 << 10);
    pFix->tnr_low_wgt_cal_max_limit  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_mode0Base_ratio * (1 << 10);
    pFix->tnr_mode0_base_ratio  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRDSWGTSCL */
    tmp = pSelect->hw_btnr_loDiffWgtCal_offset * (1 << 10);
    pFix->tnr_lo_diff_wgt_cal_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_loDiffWgtCal_scale * (1 << 10);
    pFix->tnr_lo_diff_wgt_cal_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWLSTSCL */
    tmp = pSelect->hw_btnr_loMgePreWgt_offset * (1 << 10);
    pFix->tnr_lo_mge_pre_wgt_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_loMgePreWgt_scale * (1 << 10);
    pFix->tnr_lo_mge_pre_wgt_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT0SCL0 */
    tmp = pSelect->hw_btnr_mode0LoWgt_scale * (1 << 10);
    pFix->tnr_mode0_lo_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_mode0LoWgtHdrS_scale * (1 << 10);
    pFix->tnr_mode0_lo_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1SCL1 */
    tmp = pSelect->hw_btnr_mode1LoWgt_scale * (1 << 10);
    pFix->tnr_mode1_lo_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_mode1LoWgtHdrS_scale * (1 << 10);
    pFix->tnr_mode1_lo_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1SCL2 */
    tmp = pSelect->hw_btnr_mode1Wgt_scale * (1 << 10);
    pFix->tnr_mode1_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_mode1WgtHdrS_scale * (1 << 10);
    pFix->tnr_mode1_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGTOFF */
    tmp = pSelect->hw_btnr_mode1LoWgt_offset * (1 << 10);
    pFix->tnr_mode1_lo_wgt_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pSelect->hw_btnr_mode1LoWgtHdrS_offset * (1 << 10);
    pFix->tnr_mode1_lo_wgt_hdr_sht_offset  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1OFF */
    tmp = pSelect->hw_autoSigCountWgt_thred * (1 << 10);
    pFix->tnr_auto_sigma_count_wgt_thred  = CLIP(tmp, 0, 0x3ff);
    tmp = pSelect->hw_btnr_mode1Wgt_minLimit * (1 << 10);
    pFix->tnr_mode1_wgt_min_limit  = CLIP(tmp, 0, 0x3ff);
    tmp = pSelect->hw_btnr_mode1Wgt_offset * (1 << 10);
    pFix->tnr_mode1_wgt_offset  = CLIP(tmp, 0, 0xfff);

    /* BAY3D_TNRWLO_THL */
    tmp =  pSelect->sw_btnr_loMgeFrame_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pSelect->sw_btnr_loMgeFrame_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_min_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pSelect->sw_btnr_loMgeFrameHdrS_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pSelect->sw_btnr_loMgeFrameHdrS_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_hdr_sht_min_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWLO_THH */
    tmp =  pSelect->sw_btnr_loMgeFrame_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pSelect->sw_btnr_loMgeFrame_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_max_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pSelect->sw_btnr_loMgeFrameHdrS_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pSelect->sw_btnr_loMgeFrameHdrS_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_hdr_sht_max_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWHI_THL */
    tmp =  pSelect->sw_btnr_hiMgeFrame_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pSelect->sw_btnr_hiMgeFrame_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_min_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pSelect->sw_btnr_hiMgeFrameHdrS_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pSelect->sw_btnr_hiMgeFrameHdrS_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_hdr_sht_min_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWHI_THH */
    tmp =  pSelect->sw_btnr_hiMgeFrame_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pSelect->sw_btnr_hiMgeFrame_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_max_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pSelect->sw_btnr_hiMgeFrameHdrS_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pSelect->sw_btnr_hiMgeFrameHdrS_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_hdr_sht_max_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRKEEP */
    tmp =  pSelect->hw_btnr_curSpnrHiWgt_minLimit * (1 << FIXBITWFWGT);
    pFix->tnr_cur_spnr_hi_wgt_min_limit  = CLIP(tmp, 0, 0xff);
    tmp =  pSelect->hw_btnr_preSpnrHiWgt_minLimit * (1 << FIXBITWFWGT);
    pFix->tnr_pre_spnr_hi_wgt_min_limit  = CLIP(tmp, 0, 0xff);

    if(pFix->transf_bypass_en) {
        pFix->tnr_pix_max = 0xfff;
    } else {
        pFix->tnr_pix_max = 0xc00;
    }

    /* BAY3D_TNRSIGORG */
    // noise balance update info
    int pre_wk_stat, pre_pk_stat, wk_stat;
    int sigorg, sigsta, wsta;
    long long pkp1_sq, tmpL0, tmpL1;
    //int tmp0, tmp1;
    sigorg = 256;
    if(pExpInfo->isFirstFrame)
    {
        pTransPrarms->bayertnr_pk_stat  = sigorg;
        pTransPrarms->bayertnr_wgt_stat = 0;
        pre_wk_stat = pTransPrarms->bayertnr_wgt_stat;
        pre_pk_stat = pTransPrarms->bayertnr_pk_stat;
    }
    else
    {
        pre_wk_stat = pTransPrarms->bayertnr_wgt_stat;
        pre_pk_stat = pTransPrarms->bayertnr_pk_stat;

        wk_stat = (1 << FIXTNRWWW) * (1 << FIXTNRWWW) / ((2 << FIXTNRWWW) - pre_wk_stat);
        wk_stat = CLIP(wk_stat, pFix->tnr_lo_wgt_clip_min_limit, pFix->tnr_lo_wgt_clip_max_limit);

        tmp0 = bayertnr_kalm_bitcut_V30(wk_stat, FIXTNRWWW, FIXTNRKAL);
        tmp1  = bayertnr_wgt_sqrt_tab_V30((1 << FIXTNRKAL) - tmp0);
        tmp1  = (tmp1 * sigorg) >> FIXTNRKAL;
        tmp1  = CLIP(tmp1, 1, (1 << FIXTNRKAL));

        pTransPrarms->bayertnr_wgt_stat = wk_stat;
        pTransPrarms->bayertnr_pk_stat  = tmp1;
    }

    sigorg = 256;
    sigsta = pre_pk_stat;
    wsta   = bayertnr_kalm_bitcut_V30(pTransPrarms->bayertnr_wgt_stat, FIXTNRWWW, FIXTNRWGT);
    pkp1_sq  = (long long)(sigsta * sigsta);
    tmpL0 = (long long)((wsta * wsta) / (1 << FIXTNRWGT));
    tmpL0 = (long long)(tmpL0 * pkp1_sq);
    tmpL1 = (long long)((((1 << FIXTNRWGT) - wsta) * ((1 << FIXTNRWGT) - wsta)) / (1 << FIXTNRWGT));
    tmp = tmpL0 + tmpL1 * sigorg * sigorg;
    pFix->tnr_out_sigma_sq = CLIP(tmp, 0, 0x3ffffff);

    tmp = pSelect->hw_autoSigCount_thred;
    pFix->tnr_auto_sigma_count_th = CLIP(tmp, 0, 0xfffff);
    bayertnr_fix_printf_V30(pFix);

    tmp = pSelect->hw_bay3d_lowgt_ctrl;
    pFix->hw_bay3d_lowgt_ctrl = CLIP(tmp, 0, 3);
    tmp = pSelect->hw_bay3d_lowgt_offinit;
    pFix->hw_bay3d_lowgt_offinit = CLIP(tmp, 0, 0x3ff);

    tmp = MIN(((float)1.0 / (float)(pSelect->hw_btnr_noiseBalNr_strg) * (1 << FIXTNRWGT)), (1 << FIXTNRWGT)); //(1<<FIXTNRWGT);
    pFix->tnr_motion_nr_strg = CLIP(tmp, 0, 0x7ff);
    tmp = MIN((pSelect->hw_btnr_gainOut_maxLimit * (1 << FIXGAINOUT)), ((1 << FIXGAINOUT) - 1));
    pFix->tnr_gain_max = CLIP(tmp, 0, 0xff);

    return ABAYERTNRV30_RET_SUCCESS;
}

Abayertnr_result_V30_t bayertnr_fix_printf_V30(RK_Bayertnr_Fix_V30_t * pFix)
{
    //FILE *fp = fopen("bayernr_regsiter.dat", "wb+");
    Abayertnr_result_V30_t res = ABAYERTNRV30_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    LOGD_ANR("%s:(%d) ############# bayertnr enter######################## \n", __FUNCTION__, __LINE__);


    // BAY3D_BAY3D_CTRL (0x2c00)
    LOGD_ANR("(0x2c00) bypass_en:0x%x iirsparse_en:0x%x \n",
             pFix->bypass_en,
             pFix->iirsparse_en);

    // BAY3D_CTRL1 (0x2c04)
    LOGD_ANR("(0x2c04) transf_bypass_en:0x%x sigma_curve_double_en:0x%x cur_spnr_bypass_en:0x%x cur_spnr_sigma_idxfilt_bypass_en:0x%x cur_spnr_sigma_curve_double_en:0x%x\n",
             pFix->transf_bypass_en,
             pFix->sigma_curve_double_en,
             pFix->cur_spnr_bypass_en,
             pFix->cur_spnr_sigma_idxfilt_bypass_en,
             pFix->cur_spnr_sigma_curve_double_en);

    LOGD_ANR("(0x2c04) pre_spnr_bypass_en:0x%x pre_spnr_sigma_idxfilt_bypass_en:0x%x pre_spnr_sigma_curve_double_en,:0x%x lpf_hi_bypass_en:0x%x lo_diff_vfilt_bypass_en:0x%x lpf_lo_bypass_en:0x%x\n",
             pFix->pre_spnr_bypass_en,
             pFix->pre_spnr_sigma_idxfilt_bypass_en,
             pFix->pre_spnr_sigma_curve_double_en,
             pFix->lpf_hi_bypass_en,
             pFix->lo_diff_vfilt_bypass_en,
             pFix->lpf_lo_bypass_en);

    LOGD_ANR("(0x2c04) lo_wgt_hfilt_en:0x%x lo_diff_hfilt_en:0x%x sig_hfilt_en,:0x%x gkalman_en:0x%x spnr_pre_sigma_use_en:0x%x lo_detection_mode:0x%x md_wgt_out_en:0x%x\n",
             pFix->lo_wgt_hfilt_en,
             pFix->lo_diff_hfilt_en,
             pFix->sig_hfilt_en,
             pFix->gkalman_en,
             pFix->spnr_pre_sigma_use_en,
             pFix->lo_detection_mode,
             pFix->md_wgt_out_en);

    LOGD_ANR("(0x2c04) md_bypass_en:0x%x iirspnr_out_en:0x%x lomdwgt_dbg_en:0x%x \n",
             pFix->md_bypass_en,
             pFix->iirspnr_out_en,
             pFix->lomdwgt_dbg_en);

    // BAY3D_CTRL2 0x2c08
    LOGD_ANR("(0x2c08) transf_mode:0x%x wgt_cal_mode:0x%x mge_wgt_ds_mode,:0x%x kalman_wgt_ds_mode:0x%x mge_wgt_hdr_sht_thred:0x%x sigma_calc_mge_wgt_hdr_sht_thred:0x%x\n",
             pFix->transf_mode,
             pFix->wgt_cal_mode,
             pFix->mge_wgt_ds_mode,
             pFix->kalman_wgt_ds_mode,
             pFix->mge_wgt_hdr_sht_thred,
             pFix->sigma_calc_mge_wgt_hdr_sht_thred);


    // BAY3D_TRANS0 0x2c0c
    LOGD_ANR("(0x2c0c) transf_mode_offset:0x%x transf_mode_scale:0x%x itransf_mode_offset:0x%x\n",
             pFix->transf_mode_offset,
             pFix->transf_mode_scale,
             pFix->itransf_mode_offset);

    // BAY3D_TRANS1 0x2c10
    LOGD_ANR("(0x2c10) transf_data_max_limit:0x%x\n",
             pFix->transf_data_max_limit);


    // BAY3D_BAY3D_WGTLMT 0x2c14
    LOGD_ANR("(0x2c14) cur_spnr_sigma_hdr_sht_scale:0x%x cur_spnr_sigma_scale:0x%x \n",
             pFix->cur_spnr_sigma_hdr_sht_scale,
             pFix->cur_spnr_sigma_scale);


    // BAY3D_CURSIG_X  0x2c18 - 0x2c34
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2c18 - 0x2c34) cur_spnr_luma_sigma_x[%d]:0x%x \n",
                 i, pFix->cur_spnr_luma_sigma_x[i]);
    }

    // BAY3D_CURSIG_Y 0x2c38 - 0x2c54
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2c38 - 0x2c54) cur_spnr_luma_sigma_y[%d]:0x%x \n",
                 i, pFix->cur_spnr_luma_sigma_y[i]);
    }

    // BAY3D_CURGAIN_OFF 0x2c58
    LOGD_ANR("(0x2c58) cur_spnr_sigma_rgain_offset:0x%x cur_spnr_sigma_bgain_offset:0x%x \n",
             pFix->cur_spnr_sigma_rgain_offset,
             pFix->cur_spnr_sigma_bgain_offset);

    // BAY3D_CURSIG_OFF 0x2c5c
    LOGD_ANR("(0x2c5c) cur_spnr_sigma_hdr_sht_offset:0x%x cur_spnr_sigma_offset:0x%x \n",
             pFix->cur_spnr_sigma_hdr_sht_offset,
             pFix->cur_spnr_sigma_offset);

    // BAY3D_CURWTH 0x2c60
    LOGD_ANR("(0x2c60) cur_spnr_pix_diff_max_limit:0x%x cur_spnr_wgt_cal_offset:0x%x \n",
             pFix->cur_spnr_pix_diff_max_limit,
             pFix->cur_spnr_wgt_cal_offset);

    // BAY3D_CURBFALP 0x2c64
    LOGD_ANR("(0x2c64) cur_spnr_wgt:0x%x pre_spnr_wgt:0x%x \n",
             pFix->cur_spnr_wgt,
             pFix->pre_spnr_wgt);

    // BAY3D_CURWDC  0x2c68 - 0x2c70
    for(int i = 0; i < 9; i++) {
        LOGD_ANR("(0x2c68 - 0x2c70) cur_spnr_space_rb_wgt[%d]:0x%x \n",
                 i, pFix->cur_spnr_space_rb_wgt[i]);
    }

    // BAY3D_CURWDY 0x2c74 - 0x2c7c
    for(int i = 0; i < 9; i++) {
        LOGD_ANR("(0x2c74 - 0x2c7c) cur_spnr_space_gg_wgt[%d]:0x%x \n",
                 i, pFix->cur_spnr_space_gg_wgt[i]);
    }

    // BAY3D_IIRDGAIN 0x2c80
    LOGD_ANR("(0x2c80) pre_spnr_sigma_hdr_sht_scale:0x%x pre_spnr_sigma_scale:0x%x \n",
             pFix->pre_spnr_sigma_hdr_sht_scale,
             pFix->pre_spnr_sigma_scale);

    // BAY3D_IIRSIG_X 0x2c84 - 0x2ca0
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2c84 - 0x2ca0) pre_spnr_luma_sigma_x[%d]:0x%x \n",
                 i, pFix->pre_spnr_luma_sigma_x[i]);
    }

    // BAY3D_IIRSIG_Y 0x2c84 - 0x2cc0
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2ca4 - 0x2cc0) pre_spnr_luma_sigma_y[%d]:0x%x \n",
                 i, pFix->pre_spnr_luma_sigma_y[i]);
    }

    // BAY3D_IIRGAIN_OFF 0x2cc4
    LOGD_ANR("(0x2cc4) pre_spnr_sigma_rgain_offset:0x%x pre_spnr_sigma_bgain_offset:0x%x \n",
             pFix->pre_spnr_sigma_rgain_offset,
             pFix->pre_spnr_sigma_bgain_offset);

    // BAY3D_IIRSIG_OFF 0x2cc8
    LOGD_ANR("(0x2cc8) pre_spnr_sigma_hdr_sht_offset:0x%x pre_spnr_sigma_offset:0x%x \n",
             pFix->pre_spnr_sigma_hdr_sht_offset,
             pFix->pre_spnr_sigma_offset);

    // BAY3D_IIRWTH 0x2ccc
    LOGD_ANR("(0x2ccc) pre_spnr_pix_diff_max_limit:0x%x pre_spnr_wgt_cal_offset:0x%x \n",
             pFix->pre_spnr_pix_diff_max_limit,
             pFix->pre_spnr_wgt_cal_offset);

    // BAY3D_IIRWDC  0x2cd0 - 0x2cd8
    for(int i = 0; i < 9; i++) {
        LOGD_ANR("(0x2cd0  - 0x2cd8) pre_spnr_space_rb_wgt[%d]:0x%x \n",
                 i, pFix->pre_spnr_space_rb_wgt[i]);
    }

    // BAY3D_IIRWDY 0x2cdc - 0x2ce4
    for(int i = 0; i < 9; i++) {
        LOGD_ANR("(0x2cdc - 0x2ce4) pre_spnr_space_gg_wgt[%d]:0x%x \n",
                 i, pFix->pre_spnr_space_gg_wgt[i]);
    }

    // BAY3D_BFCOEF 0x2ce8
    LOGD_ANR("(0x2ce8) cur_spnr_wgt_cal_scale:0x%x pre_spnr_wgt_cal_scale:0x%x \n",
             pFix->cur_spnr_wgt_cal_scale,
             pFix->pre_spnr_wgt_cal_scale);


    // BAY3D_TNRSIG_X  0x3a00 - 0x3a24
    for(int i = 0; i < 20; i++) {
        LOGD_ANR("(0x3a00  - 0x3a24) tnr_luma_sigma_x[%d]:0x%x \n",
                 i, pFix->tnr_luma_sigma_x[i]);
    }

    // BAY3D_TNRSIG_Y 0x3a28 - 0x3a4c
    for(int i = 0; i < 20; i++) {
        LOGD_ANR("(0x3a28 - 0x3a4c) tnr_luma_sigma_y[%d]:0x%x \n",
                 i, pFix->tnr_luma_sigma_y[i]);
    }

    // BAY3D_IIRWDC  0x3a50 - 0x3a58
    for(int i = 0; i < 9; i++) {
        LOGD_ANR("(0x3a50  - 0x3a58) tnr_lpf_hi_coeff[%d]:0x%x \n",
                 i, pFix->tnr_lpf_hi_coeff[i]);
    }

    // BAY3D_IIRWDY 0x3a5c - 0x3a64
    for(int i = 0; i < 9; i++) {
        LOGD_ANR("(0x3a5c - 0x3a64) tnr_lpf_lo_coeff[%d]:0x%x \n",
                 i, pFix->tnr_lpf_lo_coeff[i]);
    }

    // BAY3D_TNRGF3 0x3a68
    LOGD_ANR("(0x3a68) tnr_wgt_filt_coeff0:0x%x tnr_wgt_filt_coeff1:0x%x tnr_wgt_filt_coeff2:0x%x \n",
             pFix->tnr_wgt_filt_coeff0,
             pFix->tnr_wgt_filt_coeff1,
             pFix->tnr_wgt_filt_coeff2);

    // BAY3D_TNRSIGSCL 0x3a6c
    LOGD_ANR("(0x3a6c) tnr_sigma_scale:0x%x tnr_sigma_hdr_sht_scale:0x%x \n",
             pFix->tnr_sigma_scale,
             pFix->tnr_sigma_hdr_sht_scale);

    // BAY3D_TNRVIIR 0x3a70
    LOGD_ANR("(0x3a70) tnr_sig_vfilt_wgt:0x%x tnr_lo_diff_vfilt_wgt:0x%x tnr_lo_wgt_vfilt_wgt,:0x%x tnr_sig_first_line_scale:0x%x tnr_lo_diff_first_line_scale:0x%x \n",
             pFix->tnr_sig_vfilt_wgt,
             pFix->tnr_lo_diff_vfilt_wgt,
             pFix->tnr_lo_wgt_vfilt_wgt,
             pFix->tnr_sig_first_line_scale,
             pFix->tnr_lo_diff_first_line_scale);

    // BAY3D_TNRLFSCL 0x3a74
    LOGD_ANR("(0x3a74) tnr_lo_wgt_cal_offset:0x%x tnr_lo_wgt_cal_scale:0x%x \n",
             pFix->tnr_lo_wgt_cal_offset,
             pFix->tnr_lo_wgt_cal_scale);

    // BAY3D_TNRLFSCLTH 0x3a78
    LOGD_ANR("(0x3a78) tnr_low_wgt_cal_max_limit:0x%x tnr_mode0_base_ratio:0x%x \n",
             pFix->tnr_low_wgt_cal_max_limit,
             pFix->tnr_mode0_base_ratio);

    // BAY3D_TNRDSWGTSCL 0x3a7c
    LOGD_ANR("(0x3a7c) tnr_lo_diff_wgt_cal_offset:0x%x tnr_lo_diff_wgt_cal_scale:0x%x \n",
             pFix->tnr_lo_diff_wgt_cal_offset,
             pFix->tnr_lo_diff_wgt_cal_scale);

    // BAY3D_TNRWLSTSCL 0x3a80
    LOGD_ANR("(0x3a80) tnr_lo_mge_pre_wgt_offset:0x%x tnr_lo_mge_pre_wgt_scale:0x%x \n",
             pFix->tnr_lo_mge_pre_wgt_offset,
             pFix->tnr_lo_mge_pre_wgt_scale);

    // BAY3D_TNRWGT0SCL0 0x3a84
    LOGD_ANR("(0x3a84) tnr_mode0_lo_wgt_scale:0x%x tnr_mode0_lo_wgt_hdr_sht_scale:0x%x \n",
             pFix->tnr_mode0_lo_wgt_scale,
             pFix->tnr_mode0_lo_wgt_hdr_sht_scale);

    // BAY3D_TNRWGT1SCL1 0x3a88
    LOGD_ANR("(0x3a88) tnr_mode1_lo_wgt_scale:0x%x tnr_mode1_lo_wgt_hdr_sht_scale:0x%x \n",
             pFix->tnr_mode1_lo_wgt_scale,
             pFix->tnr_mode1_lo_wgt_hdr_sht_scale);

    // BAY3D_TNRWGT1SCL2 0x3a8c
    LOGD_ANR("(0x3a8c) tnr_mode1_wgt_scale:0x%x tnr_mode1_wgt_hdr_sht_scale:0x%x \n",
             pFix->tnr_mode1_wgt_scale,
             pFix->tnr_mode1_wgt_hdr_sht_scale);

    // BAY3D_TNRWGTOFF 0x3a90
    LOGD_ANR("(0x3a90) tnr_mode1_lo_wgt_offset:0x%x tnr_mode1_lo_wgt_hdr_sht_offset:0x%x \n",
             pFix->tnr_mode1_lo_wgt_offset,
             pFix->tnr_mode1_lo_wgt_hdr_sht_offset);

    // BAY3D_TNRWGT1OFF 0x3a94
    LOGD_ANR("(0x3a94) tnr_auto_sigma_count_wgt_thred:0x%x tnr_mode1_wgt_min_limit:0x%x tnr_mode1_wgt_offset:0x%x \n",
             pFix->tnr_auto_sigma_count_wgt_thred,
             pFix->tnr_mode1_wgt_min_limit,
             pFix->tnr_mode1_wgt_offset);

    // BAY3D_TNRSIGORG 0x3a98
    LOGD_ANR("(0x3a98) tnr_out_sigma_sq:0x%x\n",
             pFix->tnr_out_sigma_sq);

    // BAY3D_TNRWLO_THL 0x3a9c
    LOGD_ANR("(0x3a9c) tnr_lo_wgt_clip_min_limit:0x%x tnr_lo_wgt_clip_hdr_sht_min_limit:0x%x \n",
             pFix->tnr_lo_wgt_clip_min_limit,
             pFix->tnr_lo_wgt_clip_hdr_sht_min_limit);

    // BAY3D_TNRWLO_THH 0x3aa0
    LOGD_ANR("(0x3aa0) tnr_lo_wgt_clip_max_limit:0x%x tnr_lo_wgt_clip_hdr_sht_max_limit:0x%x \n",
             pFix->tnr_lo_wgt_clip_max_limit,
             pFix->tnr_lo_wgt_clip_hdr_sht_max_limit);

    // BAY3D_TNRWHI_THL 0x3aa4
    LOGD_ANR("(0x3aa4) tnr_hi_wgt_clip_min_limit:0x%x tnr_hi_wgt_clip_hdr_sht_min_limit:0x%x \n",
             pFix->tnr_hi_wgt_clip_min_limit,
             pFix->tnr_hi_wgt_clip_hdr_sht_min_limit);

    // BAY3D_TNRWHI_THH 0x3aa8
    LOGD_ANR("(0x3aa8) tnr_hi_wgt_clip_max_limit:0x%x tnr_hi_wgt_clip_hdr_sht_max_limit:0x%x \n",
             pFix->tnr_hi_wgt_clip_max_limit,
             pFix->tnr_hi_wgt_clip_hdr_sht_max_limit);

    // BAY3D_TNRKEEP 0x3aac
    LOGD_ANR("(0x3aac) tnr_cur_spnr_hi_wgt_min_limit:0x%x tnr_pre_spnr_hi_wgt_min_limit:0x%x \n",
             pFix->tnr_cur_spnr_hi_wgt_min_limit,
             pFix->tnr_pre_spnr_hi_wgt_min_limit);

    // BAY3D_TNRKEEP 0x3ab0
    LOGD_ANR("(0x3ab0) tnr_pix_max:0x%x \n",
             pFix->tnr_pix_max);

    // BAY3D_TNRSIGYO  0x3ab0-0x3ad4
    for(int i = 0; i < 20; i++) {
        LOGD_ANR("(0x3ad8 - 0x3afc) tnr_auto_sigma_calc[%d]:0x%x \n",
                 i, pFix->tnr_auto_sigma_calc[i]);
    }

    // BAY3D_TNRKEEP 0x2c04
    LOGD_ANR("(0x2c04) curdbg_out_en:0x%x noisebal_mode:0x%x \n",
             pFix->curdbg_out_en,
             pFix->noisebal_mode);

    // BAY3D_TNRKEEP 0x3ab8
    LOGD_ANR("(0x3ab8) tnr_motion_nr_strg:0x%x tnr_gain_max:0x%x \n",
             pFix->tnr_motion_nr_strg,
             pFix->tnr_gain_max);
    LOGD_ANR("%s:(%d) ############# bayertnr exit ######################## \n", __FUNCTION__, __LINE__);
    return res;
}

Abayertnr_result_V30_t bayertnr_get_setting_by_name_json_V30(void* pCalibdb_v, char* name,
        int* calib_idx, int* tuning_idx) {
    int i = 0;
    Abayertnr_result_V30_t res = ABAYERTNRV30_RET_SUCCESS;

    CalibDbV2_BayerTnrV30_t* pCalibdb = (CalibDbV2_BayerTnrV30_t*)pCalibdb_v;

    if(pCalibdb == NULL || name == NULL || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    for(i = 0; i < pCalibdb->TuningPara.Setting_len; i++) {
        if(strncmp(name, pCalibdb->TuningPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->TuningPara.Setting_len) {
        *tuning_idx = i;
    } else {
        *tuning_idx = 0;
    }

    for(i = 0; i < pCalibdb->CalibPara.Setting_len; i++) {
        if(strncmp(name, pCalibdb->CalibPara.Setting[i].SNR_Mode, strlen(name)*sizeof(char)) == 0) {
            break;
        }
    }

    if(i < pCalibdb->CalibPara.Setting_len) {
        *calib_idx = i;
    } else {
        *calib_idx = 0;
    }

    LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *calib_idx, i);
    return res;
}

Abayertnr_result_V30_t bayertnr_init_params_json_V30(void* pParams_v, void* pCalibdb_v,
        int calib_idx, int tuning_idx) {
    Abayertnr_result_V30_t res = ABAYERTNRV30_RET_SUCCESS;
    CalibDbV2_BayerTnrV30_C_ISO_t *pCalibIso = NULL;

    CalibDbV2_BayerTnrV30_t* pCalibdb         = (CalibDbV2_BayerTnrV30_t*)pCalibdb_v;
    RK_Bayertnr_Params_V30_t* pParams         = (RK_Bayertnr_Params_V30_t*)pParams_v;
    CalibDbV2_BayerTnrV30_T_ISO_t* pTuningIso = NULL;

#if 1
    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL || pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    pParams->enable = pCalibdb->TuningPara.enable;
    for(int i = 0; i < pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO_len && i < RK_BAYERNR_V30_MAX_ISO_NUM; i++) {
        pCalibIso = &pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO[i];
        pParams->iso[i] = pCalibIso->iso;
        for(int k = 0; k < 20; k++) {
            pParams->bayertnrParamISO[i].bayertnr_tnr_luma2sigma_idx[k] = pCalibIso->bayertnr_tnr_luma2sigma_idx[k];
            pParams->bayertnrParamISO[i].bayertnr_tnr_luma2sigma_val[k] = pCalibIso->bayertnr_tnr_luma2sigma_val[k];
        }
        for(int k = 0; k < 16; k++) {
            pParams->bayertnrParamISO[i].bayertnr_spnr_luma2sigma_idx[k] = pCalibIso->bayertnr_spnr_luma2sigma_idx[k];
            pParams->bayertnrParamISO[i].bayertnr_spnr_curluma2sigma_val[k] = pCalibIso->bayertnr_spnr_curluma2sigma_val[k];
            pParams->bayertnrParamISO[i].bayertnr_spnr_preluma2sigma_val[k] = pCalibIso->bayertnr_spnr_preluma2sigma_val[k];
        }
    }

    for(int i = 0; i < pCalibdb->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_BAYERNR_V30_MAX_ISO_NUM; i++) {
        pTuningIso = &pCalibdb->TuningPara.Setting[tuning_idx].Tuning_ISO[i];
        pParams->iso[i] = pTuningIso->iso;
        pParams->bayertnrParamISO[i].enable = pCalibdb->TuningPara.enable;
        pParams->bayertnrParamISO[i].hw_mdBypss_en = pCalibdb->TuningPara.hw_mdBypss_en;
        pParams->bayertnrParamISO[i].hw_iirSpnrOut_en = pCalibdb->TuningPara.hw_iirSpnrOut_en;
        pParams->bayertnrParamISO[i].hw_loMdWgtdbg_en = pCalibdb->TuningPara.hw_loMdWgtdbg_en;
        pParams->bayertnrParamISO[i].hw_mdWgtOut_en = pCalibdb->TuningPara.hw_mdWgtOut_en;
        pParams->bayertnrParamISO[i].hw_loDetection_mode = pCalibdb->TuningPara.hw_loDetection_mode;
        pParams->bayertnrParamISO[i].hw_curFiltOut_en = pCalibdb->TuningPara.hw_curFiltOut_en;

        pParams->bayertnrParamISO[i].hw_btnr_curSpnr_en = pTuningIso->hw_btnr_curSpnr_en;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnr_en =  pTuningIso->hw_btnr_preSpnr_en;
        pParams->bayertnrParamISO[i].sw_btnr_tnrFilt_strg = pTuningIso->sw_btnr_tnrFilt_strg;
        pParams->bayertnrParamISO[i].sw_btnr_curSpnrFilt_strg = pTuningIso->sw_btnr_curSpnrFilt_strg;
        pParams->bayertnrParamISO[i].sw_btnr_preSpnrFilt_strg = pTuningIso->sw_btnr_preSpnrFilt_strg;

        pParams->bayertnrParamISO[i].hw_btnr_curSpnrSigmaIdxFilt_en = pTuningIso->hw_btnr_curSpnrSigmaIdxFilt_en;
        // pParams->bayertnrParamISO[i].hw_btnr_curSpnrSigmaIdxFilt_mode = pTuningIso->hw_btnr_curSpnrSigmaIdxFilt_mode;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnrSigmaIdxFilt_en = pTuningIso->hw_btnr_preSpnrSigmaIdxFilt_en;
        // pParams->bayertnrParamISO[i].hw_btnr_preSpnrSigmaIdxFilt_mode = pTuningIso->hw_btnr_preSpnrSigmaIdxFilt_mode;
        pParams->bayertnrParamISO[i].hw_btnr_curSpnr_wgt = pTuningIso->hw_btnr_curSpnr_wgt;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnr_wgt = pTuningIso->hw_btnr_preSpnr_wgt;
        pParams->bayertnrParamISO[i].hw_btnr_curSpnrWgtCal_scale = pTuningIso->hw_btnr_curSpnrWgtCal_scale;
        pParams->bayertnrParamISO[i].hw_btnr_curSpnrWgtCal_offset = pTuningIso->hw_btnr_curSpnrWgtCal_offset;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnrWgtCal_scale = pTuningIso->hw_btnr_preSpnrWgtCal_scale;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnrWgtCal_offset = pTuningIso->hw_btnr_preSpnrWgtCal_offset;
        pParams->bayertnrParamISO[i].hw_btnr_spnrPresigmaUse_en = pTuningIso->hw_btnr_spnrPresigmaUse_en;
        pParams->bayertnrParamISO[i].hw_btnr_curSpnrSigma_scale = pTuningIso->hw_btnr_curSpnrSigma_scale;
        pParams->bayertnrParamISO[i].hw_btnr_curSpnrSigma_offset = pTuningIso->hw_btnr_curSpnrSigma_offset;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnrSigma_scale = pTuningIso->hw_btnr_preSpnrSigma_scale;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnrSigma_offset = pTuningIso->hw_btnr_preSpnrSigma_offset;
        pParams->bayertnrParamISO[i].hw_btnr_curSpnrSigmaHdrS_scale = pTuningIso->hw_btnr_curSpnrSigmaHdrS_scale;
        pParams->bayertnrParamISO[i].hw_btnr_curSpnrSigmaHdrS_offset = pTuningIso->hw_btnr_curSpnrSigmaHdrS_offset;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnrSigmaHdrS_scale = pTuningIso->hw_btnr_preSpnrSigmaHdrS_scale;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnrSigmaHdrS_offset = pTuningIso->hw_btnr_preSpnrSigmaHdrS_offset;

        pParams->bayertnrParamISO[i].hw_btnr_transf_en = pTuningIso->hw_btnr_transf_en;
        pParams->bayertnrParamISO[i].hw_btnr_transf_mode = pTuningIso->hw_btnr_transf_mode;
        pParams->bayertnrParamISO[i].hw_btnr_transfMode0_scale = pTuningIso->hw_btnr_transfMode0_scale;
        pParams->bayertnrParamISO[i].sw_btnr_transfMode0_offset = pTuningIso->sw_btnr_transfMode0_offset;
        pParams->bayertnrParamISO[i].sw_btnr_itransfMode0_offset = pTuningIso->sw_btnr_itransfMode0_offset;
        pParams->bayertnrParamISO[i].sw_btnr_transfMode1_offset = pTuningIso->sw_btnr_transfMode1_offset;
        pParams->bayertnrParamISO[i].sw_btnr_itransfMode1_offset = pTuningIso->sw_btnr_itransfMode1_offset;
        pParams->bayertnrParamISO[i].hw_btnr_transfData_maxLimit = pTuningIso->hw_btnr_transfData_maxLimit;

        pParams->bayertnrParamISO[i].hw_btnr_gKalman_en = pTuningIso->hw_btnr_gKalman_en;
        pParams->bayertnrParamISO[i].hw_btnr_gKalman_wgt = pTuningIso->hw_btnr_gKalman_wgt;


        pParams->bayertnrParamISO[i].hw_btnr_wgtCal_mode = pTuningIso->hw_btnr_wgtCal_mode;
        pParams->bayertnrParamISO[i].hw_btnr_lpfHi_en = pTuningIso->hw_btnr_lpfHi_en;
        pParams->bayertnrParamISO[i].sw_btnr_lpfHi_sigma = pTuningIso->sw_btnr_lpfHi_sigma;
        pParams->bayertnrParamISO[i].hw_btnr_lpfLo_en = pTuningIso->hw_btnr_lpfLo_en;
        pParams->bayertnrParamISO[i].sw_btnr_lpfLo_sigma = pTuningIso->sw_btnr_lpfLo_sigma;
        pParams->bayertnrParamISO[i].hw_btnr_sigmaIdxFilt_en = pTuningIso->hw_btnr_sigmaIdxFilt_en;
        pParams->bayertnrParamISO[i].hw_btnr_sigmaIdxFilt_mode = pTuningIso->hw_btnr_sigmaIdxFilt_mode;
        pParams->bayertnrParamISO[i].hw_btnr_sigma_scale = pTuningIso->hw_btnr_sigma_scale;
        pParams->bayertnrParamISO[i].hw_btnr_wgtFilt_en = pTuningIso->hw_btnr_wgtFilt_en;
        pParams->bayertnrParamISO[i].hw_btnr_mode0LoWgt_scale = pTuningIso->hw_btnr_mode0LoWgt_scale;
        pParams->bayertnrParamISO[i].hw_btnr_mode0Base_ratio = pTuningIso->hw_btnr_mode0Base_ratio;
        pParams->bayertnrParamISO[i].hw_btnr_mode1LoWgt_scale = pTuningIso->hw_btnr_mode1LoWgt_scale;
        pParams->bayertnrParamISO[i].hw_btnr_mode1LoWgt_offset = pTuningIso->hw_btnr_mode1LoWgt_offset;
        pParams->bayertnrParamISO[i].hw_btnr_mode1Wgt_offset = pTuningIso->hw_btnr_mode1Wgt_offset;
        pParams->bayertnrParamISO[i].hw_btnr_mode1Wgt_minLimit = pTuningIso->hw_btnr_mode1Wgt_minLimit;
        pParams->bayertnrParamISO[i].hw_btnr_mode1Wgt_scale = pTuningIso->hw_btnr_mode1Wgt_scale;


        pParams->bayertnrParamISO[i].hw_btnr_loDetection_en = pTuningIso->hw_btnr_loDetection_en;
        pParams->bayertnrParamISO[i].hw_btnr_loDiffVfilt_en = pTuningIso->hw_btnr_loDiffVfilt_en;
        pParams->bayertnrParamISO[i].hw_btnr_loDiffVfilt_mode = pTuningIso->hw_btnr_loDiffVfilt_mode;
        pParams->bayertnrParamISO[i].hw_btnr_loDiffHfilt_en = pTuningIso->hw_btnr_loDiffHfilt_en;
        pParams->bayertnrParamISO[i].hw_btnr_loDiffWgtCal_scale = pTuningIso->hw_btnr_loDiffWgtCal_scale;
        pParams->bayertnrParamISO[i].hw_btnr_loDiffWgtCal_offset = pTuningIso->hw_btnr_loDiffWgtCal_offset;
        pParams->bayertnrParamISO[i].hw_btnr_loDiffFirstLine_scale = pTuningIso->hw_btnr_loDiffFirstLine_scale;
        pParams->bayertnrParamISO[i].hw_btnr_loDiffVfilt_wgt = pTuningIso->hw_btnr_loDiffVfilt_wgt;
        pParams->bayertnrParamISO[i].hw_btnr_sigmaHfilt_en = pTuningIso->hw_btnr_sigmaHfilt_en;
        pParams->bayertnrParamISO[i].hw_btnr_sigmaFirstLine_scale = pTuningIso->hw_btnr_sigmaFirstLine_scale;
        pParams->bayertnrParamISO[i].hw_btnr_sigmaVfilt_wgt = pTuningIso->hw_btnr_sigmaVfilt_wgt;
        pParams->bayertnrParamISO[i].hw_btnr_loWgtCal_maxLimit = pTuningIso->hw_btnr_loWgtCal_maxLimit;
        pParams->bayertnrParamISO[i].hw_btnr_loWgtCal_scale = pTuningIso->hw_btnr_loWgtCal_scale;
        pParams->bayertnrParamISO[i].hw_btnr_loWgtCal_offset = pTuningIso->hw_btnr_loWgtCal_offset;
        pParams->bayertnrParamISO[i].hw_btnr_loWgtHfilt_en = pTuningIso->hw_btnr_loWgtHfilt_en;
        pParams->bayertnrParamISO[i].hw_btnr_loWgtVfilt_wgt = pTuningIso->hw_btnr_loWgtVfilt_wgt;
        pParams->bayertnrParamISO[i].hw_btnr_loMgePreWgt_scale = pTuningIso->hw_btnr_loMgePreWgt_scale;
        pParams->bayertnrParamISO[i].hw_btnr_loMgePreWgt_offset = pTuningIso->hw_btnr_loMgePreWgt_offset;

        pParams->bayertnrParamISO[i].hw_btnr_sigmaHdrS_scale = pTuningIso->hw_btnr_sigmaHdrS_scale;
        pParams->bayertnrParamISO[i].hw_btnr_mode0LoWgtHdrS_scale = pTuningIso->hw_btnr_mode0LoWgtHdrS_scale;
        pParams->bayertnrParamISO[i].hw_btnr_mode1LoWgtHdrS_scale = pTuningIso->hw_btnr_mode1LoWgtHdrS_scale;
        pParams->bayertnrParamISO[i].hw_btnr_mode1LoWgtHdrS_offset = pTuningIso->hw_btnr_mode1LoWgtHdrS_offset;
        pParams->bayertnrParamISO[i].hw_btnr_mode1WgtHdrS_scale = pTuningIso->hw_btnr_mode1WgtHdrS_scale;

        pParams->bayertnrParamISO[i].sw_btnr_loMgeFrame_maxLimit = pTuningIso->sw_btnr_loMgeFrame_maxLimit;
        pParams->bayertnrParamISO[i].sw_btnr_loMgeFrame_minLimit = pTuningIso->sw_btnr_loMgeFrame_minLimit;
        pParams->bayertnrParamISO[i].sw_btnr_hiMgeFrame_maxLimit = pTuningIso->sw_btnr_hiMgeFrame_maxLimit;
        pParams->bayertnrParamISO[i].sw_btnr_hiMgeFrame_minLimit = pTuningIso->sw_btnr_hiMgeFrame_minLimit;
        pParams->bayertnrParamISO[i].sw_btnr_loMgeFrameHdrS_maxLimit = pTuningIso->sw_btnr_loMgeFrameHdrS_maxLimit;
        pParams->bayertnrParamISO[i].sw_btnr_loMgeFrameHdrS_minLimit = pTuningIso->sw_btnr_loMgeFrameHdrS_minLimit;
        pParams->bayertnrParamISO[i].sw_btnr_hiMgeFrameHdrS_maxLimit = pTuningIso->sw_btnr_hiMgeFrameHdrS_maxLimit;
        pParams->bayertnrParamISO[i].sw_btnr_hiMgeFrameHdrS_minLimit = pTuningIso->sw_btnr_hiMgeFrameHdrS_minLimit;

        pParams->bayertnrParamISO[i].hw_autoSigCount_en = pTuningIso->hw_autoSigCount_en;
        pParams->bayertnrParamISO[i].hw_autoSigCount_thred = pTuningIso->hw_autoSigCount_thred;
        pParams->bayertnrParamISO[i].hw_autoSigCountWgt_thred = pTuningIso->hw_autoSigCountWgt_thred;
        pParams->bayertnrParamISO[i].sw_autoSigCountFilt_wgt = pTuningIso->sw_autoSigCountFilt_wgt;
        pParams->bayertnrParamISO[i].sw_autoSigCountSpnr_en = pTuningIso->sw_autoSigCountSpnr_en;

        pParams->bayertnrParamISO[i].hw_btnr_curSpnrHiWgt_minLimit = pTuningIso->hw_btnr_curSpnrHiWgt_minLimit;
        pParams->bayertnrParamISO[i].hw_btnr_preSpnrHiWgt_minLimit = pTuningIso->hw_btnr_preSpnrHiWgt_minLimit;
        pParams->bayertnrParamISO[i].hw_btnr_gainOut_maxLimit = pTuningIso->hw_btnr_gainOut_maxLimit;
        pParams->bayertnrParamISO[i].hw_btnr_noiseBal_mode = pTuningIso->hw_btnr_noiseBal_mode;
        pParams->bayertnrParamISO[i].hw_btnr_noiseBalNr_strg = pTuningIso->hw_btnr_noiseBalNr_strg;

        pParams->bayertnrParamISO[i].sw_curSpnrSpaceWgt_sigma = pTuningIso->sw_curSpnrSpaceWgt_sigma;
        pParams->bayertnrParamISO[i].sw_iirSpnrSpaceWgt_sigma = pTuningIso->sw_iirSpnrSpaceWgt_sigma;

        for(int k = 0; k < 3; k++) {
            pParams->bayertnrParamISO[i].hw_tnrWgtFltCoef[k] = pTuningIso->hw_tnrWgtFltCoef[k];
        }

        pParams->bayertnrParamISO[i].hw_bay3d_lowgt_ctrl = pTuningIso->hw_bay3d_lowgt_ctrl;
        pParams->bayertnrParamISO[i].hw_bay3d_lowgt_offinit = pTuningIso->hw_bay3d_lowgt_offinit;
    }

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!   \n", __FUNCTION__, __LINE__);
#endif
    return res;
}

Abayertnr_result_V30_t bayertnr_config_setting_param_json_V30(void* pParams_v, void* pCalibdbV30_v,
        char* param_mode, char* snr_name) {
    Abayertnr_result_V30_t res = ABAYERTNRV30_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;

    RK_Bayertnr_Params_V30_t* pParams    = (RK_Bayertnr_Params_V30_t*)pParams_v;
    CalibDbV2_BayerTnrV30_t* pCalibdbV30 = (CalibDbV2_BayerTnrV30_t*)pCalibdbV30_v;


    if(pParams == NULL || pCalibdbV30 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV30_RET_NULL_POINTER;
    }

    res = bayertnr_get_setting_by_name_json_V30(pCalibdbV30, snr_name, &calib_idx, &tuning_idx);
    if(res != ABAYERTNRV30_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);

    }

    res = bayertnr_init_params_json_V30(pParams, pCalibdbV30, calib_idx, tuning_idx);

    return res;
}

RKAIQ_END_DECLARE

