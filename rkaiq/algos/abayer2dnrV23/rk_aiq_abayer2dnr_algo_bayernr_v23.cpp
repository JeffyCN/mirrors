

#include "rk_aiq_abayer2dnr_algo_bayernr_v23.h"

RKAIQ_BEGIN_DECLARE


Abayer2dnr_result_V23_t bayer2dnr_select_params_by_ISO_V23(RK_Bayer2dnr_Params_V23_t *pParams, RK_Bayer2dnrV23_Params_Select_t *pSelect, Abayer2dnr_ExpInfo_V23_t *pExpInfo)
{
    Abayer2dnr_result_V23_t res = ABAYER2DNR_V23_RET_SUCCESS;
    int iso = 50;

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    LOGD_ANR("%s:%d iso:%d \n", __FUNCTION__, __LINE__, iso);

    int isoGainStd[RK_BAYER2DNR_V23_MAX_ISO_NUM];
    int isoGain = MAX(int(iso / 50), 1);
    int isoGainLow = 0;
    int isoGainHig = 0;
    int isoLevelLow = 0;
    int isoLevelHig = 0;
    int i;
    float tmpf;

#ifndef RK_SIMULATOR_HW
    for(int i = 0; i < RK_BAYER2DNR_V23_MAX_ISO_NUM; i++) {
        isoGainStd[i] = pParams->iso[i] / 50;
    }
#else
    for(int i = 0; i < RK_BAYER2DNR_V23_MAX_ISO_NUM; i++) {
        isoGainStd[i] = 1 * (1 << i);
    }
#endif

    for (i = 0; i < RK_BAYER2DNR_V23_MAX_ISO_NUM - 1; i++)
    {
        if (isoGain >= isoGainStd[i] && isoGain <= isoGainStd[i + 1])
        {
            isoGainLow = isoGainStd[i];
            isoGainHig = isoGainStd[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
        }
    }

    pExpInfo->isoLevelLow = isoLevelLow;
    pExpInfo->isoLevelHig = isoLevelHig;

    RK_Bayer2dnrV23_Params_Select_t *pLowISO = &pParams->Bayer2dnrParamsISO[isoLevelLow];
    RK_Bayer2dnrV23_Params_Select_t *pHighISO = &pParams->Bayer2dnrParamsISO[isoLevelHig];

    LOGD_ANR("%s:%d isoGain:%d isoGainHig:%d isoGainLow:%d\n", __FUNCTION__, __LINE__, isoGain,
             isoGainHig, isoGainLow);

    pSelect->enable = pParams->enable;
    pSelect->hdrdgain_ctrl_en = pParams->hdrdgain_ctrl_en;

    float ratio = 0;
    ratio = float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow);

    pSelect->filter_strength = ratio * (pHighISO->filter_strength - pLowISO->filter_strength) + pLowISO->filter_strength;

    for (i = 0; i < 16; i++) {
        pSelect->sigma[i] = ratio * (pHighISO->sigma[i] - pLowISO->sigma[i]) + pLowISO->sigma[i];
        pSelect->gain_adj[i] = ratio * (pHighISO->gain_adj[i] - pLowISO->gain_adj[i]) + pLowISO->gain_adj[i];
    }

    pSelect->edgesofts = ratio * (pHighISO->edgesofts - pLowISO->edgesofts) + pLowISO->edgesofts;
    pSelect->ratio = ratio * (pHighISO->ratio - pLowISO->ratio) + pLowISO->ratio;
    pSelect->weight = ratio * (pHighISO->weight - pLowISO->weight) + pLowISO->weight;
    pSelect->pix_diff = ratio * (pHighISO->pix_diff - pLowISO->pix_diff) + pLowISO->pix_diff;
    pSelect->diff_thld = ratio * (pHighISO->diff_thld - pLowISO->diff_thld) + pLowISO->diff_thld;

    pSelect->gain_scale = ratio * (pHighISO->gain_scale - pLowISO->gain_scale) + pLowISO->gain_scale;
    pSelect->hdr_dgain_scale_s = ratio * (pHighISO->hdr_dgain_scale_s - pLowISO->hdr_dgain_scale_s) + pLowISO->hdr_dgain_scale_s;
    pSelect->hdr_dgain_scale_m = ratio * (pHighISO->hdr_dgain_scale_m - pLowISO->hdr_dgain_scale_m) + pLowISO->hdr_dgain_scale_m;

    if((isoGain - isoGainLow) <= (isoGainHig - isoGain) ) {
        pSelect->gauss_guide = pLowISO->gauss_guide;
        pSelect->gain_bypass = pLowISO->gain_bypass;
        pSelect->trans_mode = pLowISO->trans_mode;
        pSelect->trans_offset = pLowISO->trans_offset;
        pSelect->itrans_offset = pLowISO->itrans_offset;
        pSelect->trans_datmax = pLowISO->trans_datmax;
        for (i = 0; i < 16; i++) {
            pSelect->gain_lumapoint[i] = pLowISO->gain_lumapoint[i];
            pSelect->lumapoint[i] = pLowISO->lumapoint[i];
        }
    } else {
        pSelect->gauss_guide = pHighISO->gauss_guide;
        pSelect->gain_bypass = pHighISO->gain_bypass;
        pSelect->trans_mode = pHighISO->trans_mode;
        pSelect->trans_offset = pHighISO->trans_offset;
        pSelect->itrans_offset = pHighISO->itrans_offset;
        pSelect->trans_datmax = pHighISO->trans_datmax;
        for (i = 0; i < 16; i++) {
            pSelect->gain_lumapoint[i] = pHighISO->gain_lumapoint[i];
            pSelect->lumapoint[i] = pHighISO->lumapoint[i];
        }
    }

    return res;
}


unsigned short bayer2dnr_get_trans_V23(int tmpfix)
{
    int logtablef[65] = {0, 1465, 2909, 4331, 5731, 7112, 8472, 9813, 11136, 12440,
                         13726, 14995, 16248, 17484, 18704, 19908, 21097, 22272, 23432, 24578, 25710,
                         26829, 27935, 29028, 30109, 31177, 32234, 33278, 34312, 35334, 36345, 37346,
                         38336, 39315, 40285, 41245, 42195, 43136, 44068, 44990, 45904, 46808, 47704,
                         48592, 49472, 50343, 51207, 52062, 52910, 53751, 54584, 55410, 56228, 57040,
                         57844, 58642, 59433, 60218, 60996, 61768, 62534, 63293, 64047, 64794, 65536
                        };
    int logprecision = 6;
    int logfixbit = 16;
    int logtblbit = 16;
    int logscalebit = 12;
    int logfixmul = (1 << logfixbit);
    long long x8, one = 1;
    long long gx, n = 0, ix1, ix2, dp;
    long long lt1, lt2, dx, fx;
    int i, j = 1;

    x8 = tmpfix + (1 << 8);
    // find highest bit
    for (i = 0; i < 32; i++)
    {
        if (x8 & j)
        {
            n = i;
        }
        j = j << 1;
    }

    gx = x8 - (one << n);
    gx = gx * (one << logprecision) * logfixmul;
    gx = gx / (one << n);

    ix1 = gx >> logfixbit;
    dp = gx - ix1 * logfixmul;

    ix2 = ix1 + 1;

    lt1 = logtablef[ix1];
    lt2 = logtablef[ix2];

    dx = lt1 * (logfixmul - dp) + lt2 * dp;

    fx = dx + (n - 8) * (one << (logfixbit + logtblbit));
    fx = fx + (one << (logfixbit + logtblbit - logscalebit - 1));
    fx = fx >> (logfixbit + logtblbit - logscalebit);

    return fx;
}

float bayer2dnr_wgt_sig_tab_V23(int index, int len, int* exp_x, float*exp_y)
{
    int i;
    float res;
    float ratio;
    int *luma_point = exp_x;
    float *interpval = exp_y;

    for(i = 0; i < len; i++) {
        if(index < luma_point[i])
            break;
    }

    if(i <= 0)
        res = interpval[0];
    else if(i > len - 1)
        res = interpval[len - 1];
    else {
        ratio = (index - luma_point[i - 1]) * (interpval[i] - interpval[i - 1]);
        ratio = ratio / (luma_point[i] - luma_point[i - 1]);
        res = interpval[i - 1] + ratio;
    }

    return res;
}


Abayer2dnr_result_V23_t bayer2dnr_fix_transfer_V23(RK_Bayer2dnrV23_Params_Select_t* pSelect, RK_Bayer2dnr_Fix_V23_t *pFix, rk_aiq_bayer2dnr_strength_v23_t* pStrength, Abayer2dnr_ExpInfo_V23_t *pExpInfo)
{
    //--------------------------- v23 params ----------------------------//
    float frameiso[3];
    float frameEt[3];
    float fdGain[3];
    int dGain[3] = {0};
    int i = 0;
    int ypos[8] = {4, 4, 4, 3, 3, 2, 2, 1};
    int xpos[8] = {4, 2, 0, 3, 1, 2, 0, 1};
    float tmp1, tmp2, edgesofts;
    int bayernr_sw_bil_gauss_weight[16];
    int tmp;

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    if (pStrength->strength_enable)
        fStrength = pStrength->percent;

    if(fStrength <= 0.0f) {
        fStrength = 0.000001;
    }

    LOGD_ANR("strength_enable:%d fStrength: %f \n", pStrength->strength_enable, fStrength);

    // hdr gain
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
        dGain[i] = int(fdGain[i] * (1 << FIXNLMCALC));
        LOGD_ANR("bayernrv23 idx[%d] fdgain:%f dgain:%d \n",
                 i, fdGain[i], dGain[i]);
    }

    //ISP_BAYNR_3A00_CTRL
    if(pExpInfo->bayertnr_en && !pSelect->gain_bypass) {
        pFix->bay3d_gain_en = 1;
    } else {
        pFix->bay3d_gain_en = 0;
    }
    pFix->lg2_mode = pSelect->trans_mode;
    pFix->gauss_en = pSelect->gauss_guide;
    pFix->log_bypass = 0;
    pFix->bayer_en = pSelect->enable;
    if(pExpInfo->bayertnr_en) {
        pFix->bayer_en = 1;
    }



    // ISP_BAYNR_3A00_DGAIN0-2
    for(i = 0; i < framenum; i++) {
        tmp = dGain[i] / ( 1 << (FIXNLMCALC - FIXGAINBIT));
        pFix->dgain[i] = CLIP(tmp, 0, 0xffff);
    }

#if 1 //predgain not use yet
    if(framenum == 1) {
        //in leaner mode ,predgain open, dgain should usr merge mode ,dgain[0],dgain[1],dgain[2]should confige 1x.
        if(pExpInfo->blc_ob_predgain > 0.0f) {
            pFix->dgain[0] = 0;
            pFix->dgain[1] = 0;
            pFix->dgain[2] = 0;
        }
    }
#endif

    if(pSelect->hdrdgain_ctrl_en) {
        //lc
        if(framenum == 2) {
            LOGD_ANR("lc before bayernr dgain:%d\n", pFix->dgain[0]);
            tmp = pFix->dgain[0] * pSelect->hdr_dgain_scale_s;
            pFix->dgain[0] = CLIP(tmp, 0, 0xffff);
            LOGD_ANR("lc after bayernr dgain:%d sacale_s:%f\n ",
                     pFix->dgain[0], pSelect->hdr_dgain_scale_s);
        }

        if(framenum == 3) {
            LOGD_ANR("lc before bayernr dgain:%d %d\n",
                     pFix->dgain[0],
                     pFix->dgain[1]);
            tmp = pFix->dgain[0] * pSelect->hdr_dgain_scale_s;
            pFix->dgain[0] = CLIP(tmp, 0, 0xffff);

            tmp = pFix->dgain[1] * pSelect->hdr_dgain_scale_m;
            pFix->dgain[1] = CLIP(tmp, 0, 0xffff);

            LOGD_ANR("lc after bayernr dgain:%d %d scale:%f %f\n ",
                     pFix->dgain[0], pFix->dgain[1],
                     pSelect->hdr_dgain_scale_s, pSelect->hdr_dgain_scale_m);
        }
    }

    // ISP_BAYNR_3A00_PIXDIFF
    tmp = pSelect->pix_diff;
    pFix->pix_diff = CLIP(tmp, 0, 0x3fff);

    // ISP_BAYNR_3A00_THLD
    tmp = pSelect->diff_thld;
    pFix->diff_thld = CLIP(tmp, 0, 0x3ff);
    tmp = (int)(pSelect->ratio / pSelect->filter_strength / fStrength * (1 << 10));
    pFix->softthld = CLIP(tmp, 0, 0x3ff);

    // ISP_BAYNR_3A00_W1_STRENG
    tmp                 = 0;  //(int)(pSelect->filter_strength * fStrength * (1 << FIXBILSTRG));
    pFix->bltflt_streng = CLIP(tmp, 0, 0xfff);
    tmp = (int)(pSelect->weight * fStrength * (1 << 10));
    pFix->reg_w1 = CLIP(tmp, 0, 0x3ff);

    // ISP_BAYNR_3A00_SIGMAX0-15   ISP_BAYNR_3A00_SIGMAY0-15
    for(i = 0; i < 16; i++) {
        //pFix->sigma_x[i] = bayernr_get_trans_V23(pSelect->bayernrv23_filter_lumapoint[i]);
        tmp = pSelect->lumapoint[i];
        pFix->sigma_x[i] = CLIP(tmp, 0, 0xffff);
        tmp = pSelect->sigma[i] * pSelect->filter_strength;
        LOGD_ANR("filter_strength:%f\n sigma[%d]:%d", pSelect->filter_strength, i,
                 pSelect->sigma[i]);
        pFix->sigma_y[i] = CLIP(tmp, 0, 0xffff);
    }

    // ISP_BAYNR_3A00_WRIT_D
    edgesofts = pSelect->edgesofts * fStrength;
    if (edgesofts > 16.0) {
        edgesofts = 16.0;
    }
    for(i = 0; i < 8; i++)
    {
        tmp1 = (float)(ypos[i] * ypos[i] + xpos[i] * xpos[i]);
        tmp1 = tmp1 / (2 * edgesofts * edgesofts);
        tmp2 = expf(-tmp1);
        bayernr_sw_bil_gauss_weight[i] = (int)(tmp1 * (EXP2RECISION_FIX / (1 << 7)));
        bayernr_sw_bil_gauss_weight[i + 8] = (int)(tmp2 * (1 << FIXVSTINV));
    }

    if(pExpInfo->gray_mode) {
        //gray mode
        tmp = bayernr_sw_bil_gauss_weight[12];
        pFix->weit_d[0] = CLIP(tmp, 0, 0x3ff);
        tmp = bayernr_sw_bil_gauss_weight[10];
        pFix->weit_d[1] = CLIP(tmp, 0, 0x3ff);
        tmp = bayernr_sw_bil_gauss_weight[11];
        pFix->weit_d[2] = CLIP(tmp, 0, 0x3ff);

    } else {
        tmp = bayernr_sw_bil_gauss_weight[13];
        pFix->weit_d[0] = CLIP(tmp, 0, 0x3ff);
        tmp = bayernr_sw_bil_gauss_weight[14];
        pFix->weit_d[1] = CLIP(tmp, 0, 0x3ff);
        tmp = bayernr_sw_bil_gauss_weight[15];
        pFix->weit_d[2] = CLIP(tmp, 0, 0x3ff);
    }



    tmp             = pSelect->trans_offset;
    pFix->lg2_off   = CLIP(tmp, 0, 0x1fff);
    tmp             = pSelect->itrans_offset;
    pFix->lg2_lgoff = CLIP(tmp, 0, 0xffff);
    tmp             = pSelect->trans_datmax;
    pFix->dat_max = CLIP(tmp, 0, 0xfffff);



    pFix->rgain_off = 0;
    pFix->bgain_off = 0;

    for (i = 0; i < 16; i++) {
        tmp             = pSelect->gain_lumapoint[i];
        pFix->gain_x[i] = CLIP(tmp, 0, 0xff);
        tmp             = pSelect->gain_adj[i] * pSelect->gain_scale * fStrength ;
        pFix->gain_y[i] = CLIP(tmp, 0, 0xfff);
    }
    bayer2dnr_fix_printf_V23(pFix);

    return ABAYER2DNR_V23_RET_SUCCESS;

}

Abayer2dnr_result_V23_t bayer2dnr_fix_printf_V23(RK_Bayer2dnr_Fix_V23_t * pFix)
{
    //FILE *fp = fopen("bayernr_regsiter.dat", "wb+");
    Abayer2dnr_result_V23_t res = ABAYER2DNR_V23_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    LOGD_ANR("%s:(%d) ############# bayernr2D enter######################## \n", __FUNCTION__, __LINE__);
    //ISP_BAYNR_3A00_CTRL(0x0000)
    LOGD_ANR(
        "(0x0000) sw_bay3d_gain_en:0x%x lg2_mode:0x%x gauss_en:0x%x log_bypass:0x%x en:0x%x \n",
        pFix->bay3d_gain_en, pFix->lg2_mode, pFix->gauss_en, pFix->log_bypass, pFix->bayer_en);

    // ISP_BAYNR_3A00_DGAIN0-2 (0x0004 - 0x0008)
    for(int i = 0; i < 3; i++) {
        LOGD_ANR("(0x0004 - 0x0008) dgain[%d]:0x%x \n",
                 i, pFix->dgain[i]);
    }

    // ISP_BAYNR_3A00_PIXDIFF(0x000c)
    LOGD_ANR("(0x000c) pix_diff:0x%x \n",
             pFix->pix_diff);

    // ISP_BAYNR_3A00_THLD(0x0010)
    LOGD_ANR("(0x000d) diff_thld:0x%x softthld:0x%x \n",
             pFix->diff_thld,
             pFix->softthld);

    // ISP_BAYNR_3A00_W1_STRENG(0x0014)
    LOGD_ANR("(0x0014) bltflt_streng:0x%x reg_w1:0x%x \n",
             pFix->bltflt_streng,
             pFix->reg_w1);

    // ISP_BAYNR_3A00_SIGMAX0-15(0x0018 - 0x0034)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x0018 - 0x0034) sig_x[%d]:0x%x \n",
                 i, pFix->sigma_x[i]);
    }

    // ISP_BAYNR_3A00_SIGMAY0-15(0x0038 - 0x0054)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x0038 - 0x0054) sig_y[%d]:0x%x \n",
                 i, pFix->sigma_y[i]);
    }

    // ISP_BAYNR_3A00_WRIT_D(0x0058)
    LOGD_ANR("(0x0058) weit_d[0]:0x%x weit_d[1]:0x%x weit_d[2]:0x%x\n",
             pFix->weit_d[0],
             pFix->weit_d[1],
             pFix->weit_d[2]);

    // ISP_BAYNR_3A00_LG_OFF(0x005c)
    LOGD_ANR("(0x005c) lg2_lgoff:0x%x lg2_off:0x%x \n",
             pFix->lg2_lgoff,
             pFix->lg2_off);

    // ISP_BAYNR_3A00_DATMAX(0x0060)
    LOGD_ANR("(0x0060) dat_max:0x%x \n",
             pFix->dat_max);

    // ISP_BAYNR_3A00_LG_OFF(0x0064)
    LOGD_ANR("(0x0064) bgain_off:0x%x rgain_off:0x%x \n",
             pFix->bgain_off,
             pFix->rgain_off);

    // ISP_BAYNR_3A00_GAINX0-15(0x0068 - 0x0074)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x0068 - 0x0074) gain_x[%d]:0x%x \n", i, pFix->gain_x[i]);
    }

    // ISP_BAYNR_3A00_GAINY0-15(0x0078 - 0x0094)
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x0078 - 0x0094) gain_y[%d]:0x%x \n", i, pFix->gain_y[i]);
    }

    LOGD_ANR("%s:(%d) ############# bayernr2D exit ######################## \n", __FUNCTION__, __LINE__);
    return res;
}


Abayer2dnr_result_V23_t bayer2dnr_get_setting_by_name_json_V23(CalibDbV2_Bayer2dnrV23_t* pCalibdb, char *name, int *calib_idx, int *tuning_idx)
{

    int i = 0;
    Abayer2dnr_result_V23_t res = ABAYER2DNR_V23_RET_SUCCESS;

    if(pCalibdb == NULL || name == NULL || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
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




Abayer2dnr_result_V23_t bayer2dnr_init_params_json_V23(RK_Bayer2dnr_Params_V23_t *pParams, CalibDbV2_Bayer2dnrV23_t* pCalibdb, int calib_idx, int tuning_idx)
{
    Abayer2dnr_result_V23_t res = ABAYER2DNR_V23_RET_SUCCESS;
    CalibDbV2_Bayer2dnrV23_C_ISO_t *pCalibIso = NULL;
    CalibDbV2_Bayer2dnrV23_T_ISO_t *pTuningISO = NULL;
    //CalibDbV2_BayerTnr_V23_TuningPara_Setting_ISO_t *pTnrIso = NULL;

    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL || pCalibdb == NULL || calib_idx < 0 || tuning_idx < 0) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }
    pParams->enable = pCalibdb->TuningPara.enable;
    pParams->hdrdgain_ctrl_en = pCalibdb->TuningPara.hdrdgain_ctrl_en;

    if(calib_idx < pCalibdb->CalibPara.Setting_len) {
        for(int i = 0; i < pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO_len && i < RK_BAYER2DNR_V23_MAX_ISO_NUM; i++) {
            pCalibIso = &pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO[i];
            pParams->iso[i] = pCalibIso->iso;
            for(int k = 0; k < 16; k++) {
                pParams->Bayer2dnrParamsISO[i].lumapoint[k] = pCalibIso->lumapoint[k];
                pParams->Bayer2dnrParamsISO[i].sigma[k] = pCalibIso->sigma[k];
            }
        }
    }

    if(tuning_idx < pCalibdb->TuningPara.Setting_len) {
        for(int i = 0; i < pCalibdb->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_BAYER2DNR_V23_MAX_ISO_NUM; i++) {
            pTuningISO = &pCalibdb->TuningPara.Setting[tuning_idx].Tuning_ISO[i];
            pParams->iso[i] = pTuningISO->iso;
            pParams->Bayer2dnrParamsISO[i].filter_strength = pTuningISO->filter_strength;
            pParams->Bayer2dnrParamsISO[i].edgesofts = pTuningISO->edgesofts;
            pParams->Bayer2dnrParamsISO[i].weight = pTuningISO->weight;
            pParams->Bayer2dnrParamsISO[i].ratio = pTuningISO->ratio;
            pParams->Bayer2dnrParamsISO[i].gauss_guide = pTuningISO->gauss_guide;

            pParams->Bayer2dnrParamsISO[i].gain_bypass = pTuningISO->gain_bypass;
            pParams->Bayer2dnrParamsISO[i].gain_scale = pTuningISO->gain_scale;
            for(int k = 0; k < 16; k++) {
                pParams->Bayer2dnrParamsISO[i].gain_lumapoint[k] = pTuningISO->gain_adj.gain_lumapoint[k];
                pParams->Bayer2dnrParamsISO[i].gain_adj[k] = pTuningISO->gain_adj.gain_adj[k];
            }

            pParams->Bayer2dnrParamsISO[i].pix_diff = pTuningISO->pix_diff;
            pParams->Bayer2dnrParamsISO[i].diff_thld = pTuningISO->diff_thld;

            pParams->Bayer2dnrParamsISO[i].trans_mode = pTuningISO->trans_mode;
            pParams->Bayer2dnrParamsISO[i].trans_offset = pTuningISO->trans_offset;
            pParams->Bayer2dnrParamsISO[i].itrans_offset = pTuningISO->itrans_offset;
            pParams->Bayer2dnrParamsISO[i].trans_datmax = pTuningISO->trans_datmax;
            pParams->Bayer2dnrParamsISO[i].hdr_dgain_scale_s = pTuningISO->hdr_dgain_scale_s;
            pParams->Bayer2dnrParamsISO[i].hdr_dgain_scale_m = pTuningISO->hdr_dgain_scale_m;
        }
    }

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!   \n", __FUNCTION__, __LINE__);

    return res;
}

Abayer2dnr_result_V23_t bayer2dnr_config_setting_param_json_V23(RK_Bayer2dnr_Params_V23_t *pParams, CalibDbV2_Bayer2dnrV23_t* pCalibdbV23, char* param_mode, char * snr_name)
{
    Abayer2dnr_result_V23_t res = ABAYER2DNR_V23_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;


    if(pParams == NULL || pCalibdbV23 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYER2DNR_V23_RET_NULL_POINTER;
    }

    res = bayer2dnr_get_setting_by_name_json_V23(pCalibdbV23, snr_name, &calib_idx, &tuning_idx);
    if(res != ABAYER2DNR_V23_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
    }

    res = bayer2dnr_init_params_json_V23(pParams, pCalibdbV23, calib_idx, tuning_idx);

    return res;

}

RKAIQ_END_DECLARE

