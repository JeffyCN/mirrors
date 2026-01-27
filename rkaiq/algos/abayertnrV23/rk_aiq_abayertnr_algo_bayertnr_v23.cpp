

#include "rk_aiq_abayertnr_algo_bayertnr_v23.h"


RKAIQ_BEGIN_DECLARE

Abayertnr_result_V23_t bayertnr_select_params_by_ISO_V23(void* pParams_v, void* pSelect_v,
        Abayertnr_ExpInfo_V23_t* pExpInfo) {
    Abayertnr_result_V23_t res = ABAYERTNRV23_RET_SUCCESS;
    int iso = 50;

#if (RKAIQ_HAVE_BAYERTNR_V23)
    RK_Bayertnr_Params_V23_t* pParams        = (RK_Bayertnr_Params_V23_t*)pParams_v;
    RK_Bayertnr_Params_V23_Select_t* pSelect = (RK_Bayertnr_Params_V23_Select_t*)pSelect_v;
#else
    RK_Bayertnr_Params_V23L_t* pParams       = (RK_Bayertnr_Params_V23L_t*)pParams_v;
    RK_Bayertnr_Param_V23L_Select_t* pSelect = (RK_Bayertnr_Param_V23L_Select_t*)pSelect_v;
#endif

    if(pParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    iso = pExpInfo->arIso[pExpInfo->hdr_mode];

    LOGD_ANR("%s:%d iso:%d \n", __FUNCTION__, __LINE__, iso);

    int isoGainStd[RK_BAYERNR_V23_MAX_ISO_NUM];
    int isoGain = MAX(int(iso / 50), 1);
    int isoGainLow = 0;
    int isoGainHig = 0;
    int isoLevelLow = 0;
    int isoLevelHig = 0;
    int i;
    float tmpf;

#ifndef RK_SIMULATOR_HW
    for(int i = 0; i < RK_BAYERNR_V23_MAX_ISO_NUM; i++) {
        isoGainStd[i] = pParams->iso[i] / 50;
    }
#else
    for(int i = 0; i < RK_BAYERNR_V23_MAX_ISO_NUM; i++) {
        isoGainStd[i] = 1 * (1 << i);
    }
#endif

    for (i = 0; i < RK_BAYERNR_V23_MAX_ISO_NUM - 1; i++) {
        if (isoGain >= isoGainStd[i] && isoGain <= isoGainStd[i + 1]) {
            isoGainLow = isoGainStd[i];
            isoGainHig = isoGainStd[i + 1];
            isoLevelLow = i;
            isoLevelHig = i + 1;
            break;
        }
    }

    if(iso > pParams->iso[RK_BAYERNR_V23_MAX_ISO_NUM - 1]) {
        isoGainLow = isoGainStd[RK_BAYERNR_V23_MAX_ISO_NUM - 2];
        isoGainHig = isoGainStd[RK_BAYERNR_V23_MAX_ISO_NUM - 1];
        isoLevelLow = RK_BAYERNR_V23_MAX_ISO_NUM - 2;
        isoLevelHig = RK_BAYERNR_V23_MAX_ISO_NUM - 1;
    }

    if(iso < pParams->iso[0]) {
        isoGainLow = isoGainStd[0];
        isoGainHig = isoGainStd[1];
        isoLevelLow = 0;
        isoLevelHig = 1;
    }

    LOGD_ANR("%s:%d iso:%d high:%d low:%d\n",
             __FUNCTION__, __LINE__,
             isoGain, isoGainHig, isoGainLow);

    pExpInfo->isoLevelLow = isoLevelLow;
    pExpInfo->isoLevelHig = isoLevelHig;
#if (RKAIQ_HAVE_BAYERTNR_V23)
    RK_Bayertnr_Params_V23_Select_t* pLowISO = &pParams->bayertnrParamISO[isoLevelLow];
    RK_Bayertnr_Params_V23_Select_t* pHighISO = &pParams->bayertnrParamISO[isoLevelHig];
#else
    RK_Bayertnr_Param_V23L_Select_t* pLowISO   = &pParams->bayertnrParamISO[isoLevelLow];
    RK_Bayertnr_Param_V23L_Select_t* pHighISO  = &pParams->bayertnrParamISO[isoLevelHig];
#endif

    pSelect->enable = pParams->enable;

    pSelect->thumbds_w = pLowISO->thumbds_w;
    pSelect->thumbds_h = pLowISO->thumbds_h;

    float ratio = 0;
    ratio = float(isoGain - isoGainLow) / float(isoGainHig - isoGainLow);

    pSelect->lo_filter_strength =
        ratio * (pHighISO->lo_filter_strength - pLowISO->lo_filter_strength) + pLowISO->lo_filter_strength;

    pSelect->hi_filter_strength =
        ratio * (pHighISO->hi_filter_strength - pLowISO->hi_filter_strength) + pLowISO->hi_filter_strength;

    pSelect->soft_threshold_ratio =
        ratio * (pHighISO->soft_threshold_ratio - pLowISO->soft_threshold_ratio) + pLowISO->soft_threshold_ratio;


    for (i = 0; i < 16; i++) {
        pSelect->sigma[i] =
            ratio * (pHighISO->sigma[i] - pLowISO->sigma[i]) + pLowISO->sigma[i];
        pSelect->lo_sigma[i] =
            ratio * (pHighISO->lo_sigma[i] - pLowISO->lo_sigma[i]) + pLowISO->lo_sigma[i];
        pSelect->hi_sigma[i] =
            ratio * (pHighISO->hi_sigma[i] - pLowISO->hi_sigma[i]) + pLowISO->hi_sigma[i];
    }

    pSelect->hidif_th  =
        ratio * (pHighISO->hidif_th - pLowISO->hidif_th) + pLowISO->hidif_th;

#if (RKAIQ_HAVE_BAYERTNR_V23_LITE)
    pSelect->wgtmin =
        ratio * (pHighISO->wgtmin - pLowISO->wgtmin) + pLowISO->wgtmin;
#endif

    if((isoGain - isoGainLow) <= (isoGainHig - isoGain)) {
        pSelect->trans_en = pLowISO->trans_en;
        pSelect->lo_enable = pLowISO->lo_enable;
        pSelect->hi_enable = pLowISO->hi_enable;

        pSelect->lo_gsbay_en = pLowISO->lo_gsbay_en;
        pSelect->lo_gslum_en = pLowISO->lo_gslum_en;
        pSelect->hi_gslum_en = pLowISO->hi_gslum_en;

        pSelect->wgt_use_mode = pLowISO->wgt_use_mode;
        pSelect->wgt_mge_mode = pLowISO->wgt_mge_mode;

        pSelect->global_pk_en = pLowISO->global_pk_en;
        pSelect->global_pksq = pLowISO->global_pksq;

        pSelect->hi_wgt_comp = pLowISO->hi_wgt_comp;
        pSelect->lo_clipwgt = pLowISO->lo_clipwgt;

        for (i = 0; i < 16; i++) {
            pSelect->lumapoint[i] = pLowISO->lumapoint[i];
            pSelect->lumapoint2[i] = pLowISO->lumapoint2[i];
        }

        pSelect->lo_filter_rat0 = pLowISO->lo_filter_rat0;
        pSelect->lo_filter_thed0 = pLowISO->lo_filter_thed0;

        pSelect->hi_filter_rat0 = pLowISO->hi_filter_rat0;
        pSelect->hi_filter_thed0 = pLowISO->hi_filter_thed0;
        pSelect->hi_filter_rat1 = pLowISO->hi_filter_rat1;
        pSelect->hi_filter_thed1 = pLowISO->hi_filter_thed1;

        pSelect->guass_guide_coeff0 = pLowISO->guass_guide_coeff0;
        pSelect->guass_guide_coeff1 = pLowISO->guass_guide_coeff1;
        pSelect->guass_guide_coeff2 = pLowISO->guass_guide_coeff2;

        pSelect->hi_filter_abs_ctrl = pLowISO->hi_filter_abs_ctrl;
#if (RKAIQ_HAVE_BAYERTNR_V23)
        pSelect->lo_med_en = pLowISO->lo_med_en;
        pSelect->hi_med_en = pLowISO->hi_med_en;

        pSelect->hi_guass = pLowISO->hi_guass;
        pSelect->kl_guass = pLowISO->kl_guass;

        pSelect->hi_filter_filt_avg = pLowISO->hi_filter_filt_avg;
        pSelect->hi_filter_filt_bay = pLowISO->hi_filter_filt_bay;
        pSelect->hi_filter_filt_mode = pLowISO->hi_filter_filt_mode;

        pSelect->guass_guide_coeff3 = pLowISO->guass_guide_coeff3;
#endif

#if (RKAIQ_HAVE_BAYERTNR_V23_LITE)
        pSelect->wgtmm_opt_en = pLowISO->wgtmm_opt_en;
        pSelect->wgtmm_sel_en = pLowISO->wgtmm_sel_en;
#endif
    } else {
        pSelect->trans_en = pHighISO->trans_en;
        pSelect->lo_enable = pHighISO->lo_enable;
        pSelect->hi_enable = pHighISO->hi_enable;

        pSelect->lo_gsbay_en = pHighISO->lo_gsbay_en;
        pSelect->lo_gslum_en = pHighISO->lo_gslum_en;
        pSelect->hi_gslum_en = pHighISO->hi_gslum_en;

        pSelect->wgt_use_mode = pHighISO->wgt_use_mode;
        pSelect->wgt_mge_mode = pHighISO->wgt_mge_mode;

        pSelect->global_pk_en = pHighISO->global_pk_en;
        pSelect->global_pksq = pHighISO->global_pksq;

        pSelect->hi_filter_abs_ctrl = pHighISO->hi_filter_abs_ctrl;

        pSelect->hi_wgt_comp = pHighISO->hi_wgt_comp;
        pSelect->lo_clipwgt = pHighISO->lo_clipwgt;

        for (i = 0; i < 16; i++) {
            pSelect->lumapoint[i] = pHighISO->lumapoint[i];
            pSelect->lumapoint2[i] = pHighISO->lumapoint2[i];
        }

        pSelect->lo_filter_rat0 = pHighISO->lo_filter_rat0;
        pSelect->lo_filter_thed0 = pHighISO->lo_filter_thed0;

        pSelect->hi_filter_rat0 = pHighISO->hi_filter_rat0;
        pSelect->hi_filter_thed0 = pHighISO->hi_filter_thed0;
        pSelect->hi_filter_rat1 = pHighISO->hi_filter_rat1;
        pSelect->hi_filter_thed1 = pHighISO->hi_filter_thed1;

        pSelect->guass_guide_coeff0 = pHighISO->guass_guide_coeff0;
        pSelect->guass_guide_coeff1 = pHighISO->guass_guide_coeff1;
        pSelect->guass_guide_coeff2 = pHighISO->guass_guide_coeff2;

#if (RKAIQ_HAVE_BAYERTNR_V23)
        pSelect->lo_med_en = pHighISO->lo_med_en;
        pSelect->hi_med_en = pHighISO->hi_med_en;

        pSelect->hi_guass = pHighISO->hi_guass;
        pSelect->kl_guass = pHighISO->kl_guass;

        pSelect->hi_filter_filt_avg = pHighISO->hi_filter_filt_avg;
        pSelect->hi_filter_filt_bay = pHighISO->hi_filter_filt_bay;
        pSelect->hi_filter_filt_mode = pHighISO->hi_filter_filt_mode;

        pSelect->guass_guide_coeff3 = pHighISO->guass_guide_coeff3;
#endif

#if (RKAIQ_HAVE_BAYERTNR_V23_LITE)
        pSelect->wgtmm_opt_en = pHighISO->wgtmm_opt_en;
        pSelect->wgtmm_sel_en = pHighISO->wgtmm_sel_en;
#endif
    }

    return res;
}

unsigned short bayertnr_get_trans_V23(int tmpfix)
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

Abayertnr_result_V23_t bayertnr_fix_transfer_V23(void* pSelect_v, RK_Bayertnr_Fix_V23_t* pFix,
        rk_aiq_bayertnr_strength_v23_t* pStrength,
        Abayertnr_ExpInfo_V23_t* pExpInfo) {
    int i = 0;
    int tmp;

#if (RKAIQ_HAVE_BAYERTNR_V23)
    RK_Bayertnr_Params_V23_Select_t* pSelect = (RK_Bayertnr_Params_V23_Select_t*)pSelect_v;
#else
    RK_Bayertnr_Param_V23L_Select_t* pSelect   = (RK_Bayertnr_Param_V23L_Select_t*)pSelect_v;
#endif

    if(pSelect == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    if(pStrength == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    float fStrength = 1.0;

    if(pStrength->strength_enable)
        fStrength = pStrength->percent;

    if(fStrength <= 0.0f) {
        fStrength = 0.000001;
    }

    LOGD_ANR("strength_enable:%d fStrength: %f \n", pStrength->strength_enable, fStrength);

    pSelect->lo_enable = 1;

    // BAY3D_BAY3D_CTRL 0x2c00
    pFix->soft_st = 0;
    pFix->soft_mode = 0;
    pFix->bwsaving_en = pSelect->trans_en != 0;
    pFix->loswitch_protect = 0;
    pFix->glbpk_en = pSelect->global_pk_en;
    pFix->logaus3_bypass_en = !pSelect->lo_gslum_en;
    pFix->logaus5_bypass_en = !pSelect->lo_gsbay_en;
#if (RKAIQ_HAVE_BAYERTNR_V23)
    pFix->lomed_bypass_en = !pSelect->lo_med_en;
#else
    pFix->lomed_bypass_en                      = 0;
#endif
#if (RKAIQ_HAVE_BAYERTNR_V23)
    pFix->hichnsplit_en = (pSelect->hi_filter_filt_bay == 0) ? (pSelect->lo_enable == 0) : (pSelect->hi_filter_filt_bay > 1);
#else
    pFix->hichnsplit_en                        = 0;
#endif
    pFix->hiabs_possel = (pSelect->hi_filter_abs_ctrl == 0) ? (pSelect->lo_enable == 0) : (pSelect->hi_filter_abs_ctrl > 1);
    pFix->higaus_bypass_en = !pSelect->hi_gslum_en;
#if (RKAIQ_HAVE_BAYERTNR_V23)
    pFix->himed_bypass_en = !pSelect->hi_med_en;
#else
    pFix->himed_bypass_en                      = 0;
#endif
    pFix->lobypass_en = !pSelect->lo_enable;
    pFix->hibypass_en = !pSelect->hi_enable;
    pFix->bypass_en = !pSelect->enable;;
    pFix->bay3d_en = pSelect->enable;


    // BAY3D_BAY3D_KALRATIO 0x2c04
    tmp = (int)(pSelect->soft_threshold_ratio * (1 << 10) );
    pFix->softwgt = CLIP(tmp, 0, 0x3ff);
    tmp = (int)pSelect->hidif_th;
    pFix->hidif_th = CLIP(tmp, 0, 0xffff);

    // BAY3D_BAY3D_GLBPK2 0x2c08
    tmp = pSelect->global_pksq;
    pFix->glbpk2 = CLIP(tmp, 0, 0xfffffff);


    // BAY3D_BAY3D_CTRL1 0x2c0c
    pFix->hiwgt_opt_en = pSelect->wgt_mge_mode;
#if (RKAIQ_HAVE_BAYERTNR_V23)
    pFix->hichncor_en = (pSelect->hi_filter_filt_avg == 0) ? (pSelect->lo_enable == 0) : (pSelect->hi_filter_filt_avg > 1);
#else
    pFix->hichncor_en                          = 0;
#endif
    pFix->bwopt_gain_dis = 0;
    pFix->lo4x8_en = 1;
    pFix->lo4x4_en = 0;
    if(pSelect->thumbds_w == 4  && pSelect->thumbds_h == 4) {
        pFix->lo4x8_en = 0;
        pFix->lo4x4_en = 1;
    } else if(pSelect->thumbds_w == 8  && pSelect->thumbds_h == 4) {
        pFix->lo4x8_en = 1;
        pFix->lo4x4_en = 0;
    } else if(pSelect->thumbds_w == 8  && pSelect->thumbds_h == 8) {
        pFix->lo4x8_en = 0;
        pFix->lo4x4_en = 0;
    } else {
        pFix->lo4x8_en = 1;
        pFix->lo4x4_en = 0;
    }
#if (RKAIQ_HAVE_BAYERTNR_V23)
    pFix->hisig_ind_sel = pSelect->hi_guass;
    pFix->pksig_ind_sel = pSelect->kl_guass;
#else
    pFix->hisig_ind_sel                        = 0;
    pFix->pksig_ind_sel                        = 0;
#endif
    pFix->iirwr_rnd_en = 1;
    pFix->curds_high_en = 0;
#if (RKAIQ_HAVE_BAYERTNR_V23)
    tmp = pSelect->hi_filter_filt_mode;
    tmp = CLIP(tmp, 0, 4);
    pFix->higaus3_mode = 0;
    pFix->higaus5x5_en = 0;
    if(tmp == 0)
        pFix->higaus3_mode = 0;
    else if(tmp == 1)
        pFix->higaus3_mode = 2;
    else if(tmp == 2)
        pFix->higaus3_mode = 1;
    else if(tmp == 3)
        pFix->higaus5x5_en = 1;
    else if(tmp == 4)
        pFix->higaus5x5_en = 0;
#else
    pFix->higaus3_mode                         = 0;
    pFix->higaus5x5_en                         = 0;
#endif
    pFix->wgtmix_opt_en = pSelect->wgt_use_mode != 0;

#if (RKAIQ_HAVE_BAYERTNR_V23_LITE)
    pFix->wgtmm_opt_en = pSelect->wgtmm_opt_en;
    pFix->wgtmm_sel_en = pSelect->wgtmm_sel_en;
#endif

    // BAY3D_BAY3D_WGTLMT 0x2c10
    tmp = (int)(((float)1 - pSelect->lo_clipwgt) * (1 << FIXTNRWGT));
    pFix->wgtlmt = CLIP(tmp, 0, 0x3ff);
    tmp = pSelect->hi_enable ? (int)(pSelect->hi_wgt_comp * (1 << 10)) : 0;
    pFix->wgtratio = CLIP(tmp, 0, 0x3ff);


    for(i = 0; i < 16; i++) {
        // BAY3D_BAY3D_SIG_X0  0x2c14 - 0x2c30
        //no need transfer to log domain
        //tmp = bayertnr_get_trans_V23(pSelect->bayertnrv23_tnr_luma_point[i]);
        tmp = pSelect->lumapoint[i];
        pFix->sig0_x[i] = CLIP(tmp, 0, 0xffff);

        // BAY3D_BAY3D_SIG0_Y0 0x2c34 - 0x2c50
        tmp = pSelect->sigma[i];
        pFix->sig0_y[i] = CLIP(tmp, 0, 0x3fff);

        // BAY3D_BAY3D_SIG_X0  0x2c54 - 0x2c70
        tmp = pSelect->lumapoint2[i];
        pFix->sig1_x[i] = CLIP(tmp, 0, 0xffff);

        // BAY3D_BAY3D_SIG1_Y0 0x2c74 - 0x2c90
        tmp = pSelect->hi_sigma[i] * pSelect->hi_filter_strength * fStrength;
        pFix->sig1_y[i] = CLIP(tmp, 0, (1 << 14) - 1);

        // BAY3D_BAY3D_SIG2_Y0 0x2c94 - 0x2cb0
        tmp = pSelect->lo_sigma[i] * pSelect->lo_filter_strength * fStrength;
        pFix->sig2_y[i] = CLIP(tmp, 0, (1 << 10) - 1);
    }

#if (RKAIQ_HAVE_BAYERTNR_V23_LITE)
    tmp          = pSelect->wgtmin * (1 << 10);
    pFix->wgtmin = CLIP(tmp, 0, (1 << 10) - 1);
#endif

    // BAY3D_BAY3D_HISIGRAT 0x2ce4
    tmp = (int)(pSelect->hi_filter_rat0 * (1 << 8));
    pFix->hisigrat0 = CLIP(tmp, 0, 0xfff);
    tmp = (int)(pSelect->hi_filter_rat1 * (1 << 8));
    pFix->hisigrat1 = CLIP(tmp, 0, 0xfff);

    // BAY3D_BAY3D_HISIGOFF 0x2ce8
    tmp = (int)(pSelect->hi_filter_thed0);
    pFix->hisigoff0 = CLIP(tmp, 0, 0xfff);
    tmp = (int)(pSelect->hi_filter_thed1);
    pFix->hisigoff1 = CLIP(tmp, 0, 0xfff);

    // BAY3D_BAY3D_LOSIG 0x2cec
    tmp = (int)(pSelect->lo_filter_thed0);
    pFix->losigoff = CLIP(tmp, 0, 0xfff);
    tmp = (int)(pSelect->lo_filter_rat0 * (1 << 8));
    pFix->losigrat = CLIP(tmp, 0, 0xfff);

    // BAY3D_BAY3D_LOSIG 0x2cf0
    tmp = (int)(0);
    pFix->rgain_off = CLIP(tmp, 0, 0x3fff);
    tmp = (int)(0);
    pFix->bgain_off = CLIP(tmp, 0, 0x3fff);

    // BAY3D_BAY3D_SIGGAUS 0x2cf4
    tmp = (int)(pSelect->guass_guide_coeff0);
    pFix->siggaus0 = CLIP(tmp, 0, 0x3f);
    tmp = (int)(pSelect->guass_guide_coeff1);
    pFix->siggaus1 = CLIP(tmp, 0, 0x3f);
    tmp = (int)(pSelect->guass_guide_coeff2);
    pFix->siggaus2 = CLIP(tmp, 0, 0x3f);
#if (RKAIQ_HAVE_BAYERTNR_V23)
    tmp = (int)(pSelect->guass_guide_coeff3);
    pFix->siggaus3 = CLIP(tmp, 0, 0x3f);
#else
    pFix->siggaus3                             = 0;
#endif

    bayertnr_fix_printf_V23(pFix);

    return ABAYERTNRV23_RET_SUCCESS;
}

Abayertnr_result_V23_t bayertnr_fix_printf_V23(RK_Bayertnr_Fix_V23_t * pFix)
{
    //FILE *fp = fopen("bayernr_regsiter.dat", "wb+");
    Abayertnr_result_V23_t res = ABAYERTNRV23_RET_SUCCESS;

    if(pFix == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    LOGD_ANR("%s:(%d) ############# bayertnr enter######################## \n", __FUNCTION__, __LINE__);


    // BAY3D_BAY3D_CTRL (0x002c00)
    LOGD_ANR("(0x2c00) soft_st:0x%x soft_mode:0x%x bwsaving_en:0x%x loswitch_protect:0x%x glbpk_en:0x%x\n",
             pFix->soft_st,
             pFix->soft_mode,
             pFix->bwsaving_en,
             pFix->loswitch_protect,
             pFix->glbpk_en);

    LOGD_ANR("(0x2c00) logaus3_bypass_en:0x%x logaus5_bypass_en:0x%x lomed_bypass_en:0x%x hichnsplit_en:0x%x hiabs_possel:0x%x\n",
             pFix->logaus3_bypass_en,
             pFix->logaus5_bypass_en,
             pFix->lomed_bypass_en,
             pFix->hichnsplit_en,
             pFix->hiabs_possel);

    LOGD_ANR("(0x2c00) higaus_bypass_en:0x%x himed_bypass_en:0x%x lobypass_en,:0x%x hibypass_en:0x%x bypass_en:0x%x en:0x%x\n",
             pFix->higaus_bypass_en,
             pFix->himed_bypass_en,
             pFix->lobypass_en,
             pFix->hibypass_en,
             pFix->bypass_en,
             pFix->bay3d_en);

    // BAY3D_BAY3D_KALRATIO 0x2c04
    LOGD_ANR("(0x2c04) softwgt:0x%x hidif_th:0x%x \n",
             pFix->softwgt,
             pFix->hidif_th);

    // BAY3D_BAY3D_GLBPK2 0x2c08
    LOGD_ANR("(0x2c08) glbpk2:0x%x \n",
             pFix->glbpk2);


    // BAY3D_BAY3D_CTRL1 0x2c0c
    LOGD_ANR("(0x2c0c) hiwgt_opt_en:0x%x hichncor_en:0x%x bwopt_gain_dis,:0x%x lo4x8_en:0x%x lo4x4_en:0x%x hisig_ind_sel:0x%x\n",
             pFix->hiwgt_opt_en,
             pFix->hichncor_en,
             pFix->bwopt_gain_dis,
             pFix->lo4x8_en,
             pFix->lo4x4_en,
             pFix->hisig_ind_sel);

    LOGD_ANR("(0x2c0c) pksig_ind_sel:0x%x iirwr_rnd_en:0x%x curds_high_en,:0x%x higaus3_mode:0x%x higaus5x5_en:0x%x wgtmix_opt_en:0x%x\n",
             pFix->pksig_ind_sel,
             pFix->iirwr_rnd_en,
             pFix->curds_high_en,
             pFix->higaus3_mode,
             pFix->higaus5x5_en,
             pFix->wgtmix_opt_en);

    // BAY3D_BAY3D_WGTLMT 0x2c10
    LOGD_ANR("(0x2c10) wgtlmt:0x%x wgtratio:0x%x \n",
             pFix->wgtlmt,
             pFix->wgtratio);

    // BAY3D_BAY3D_SIG_X0  0x2c14 - 0x2c30
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2c14 - 0x2c30) sig0_x[%d]:0x%x \n",
                 i, pFix->sig0_x[i]);
    }

    // BAY3D_BAY3D_SIG0_Y0 0x2c34 - 0x2c50
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2c34 - 0x2c50) sig0_y[%d]:0x%x \n",
                 i, pFix->sig0_y[i]);
    }

    // BAY3D_BAY3D_SIG_X0  0x2c54 - 0x2c70
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2c54 - 0x2c70) sig1_x[%d]:0x%x \n",
                 i, pFix->sig1_x[i]);
    }

    // // BAY3D_BAY3D_SIG1_Y0 0x2c74 - 0x2c90
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2c74 - 0x2c90) sig1_y[%d]:0x%x \n",
                 i, pFix->sig1_y[i]);
    }

    // BAY3D_BAY3D_SIG2_Y0 0x2c94 - 0x2cb0
    for(int i = 0; i < 16; i++) {
        LOGD_ANR("(0x2c94 - 0x2cb0) sig2_y[%d]:0x%x \n",
                 i, pFix->sig2_y[i]);
    }


    // BAY3D_BAY3D_HISIGRAT 0x2ce4
    LOGD_ANR("(0x2ce4) hisigrat0:0x%x hisigrat1:0x%x \n",
             pFix->hisigrat0,
             pFix->hisigrat1);

    // BAY3D_BAY3D_HISIGOFF 0x2ce8
    LOGD_ANR("(0x2ce8) hisigoff0:0x%x hisigoff1:0x%x \n",
             pFix->hisigoff0,
             pFix->hisigoff1);


    // BAY3D_BAY3D_LOSIG 0x2cec
    LOGD_ANR("(0x2cec) losigoff:0x%x losigrat:0x%x \n",
             pFix->losigoff,
             pFix->losigrat);


    // BAY3D_BAY3D_LOSIG 0x2cf0
    LOGD_ANR("(0x2cf0) rgain_off:0x%x bgain_off:0x%x \n",
             pFix->rgain_off,
             pFix->bgain_off);


    // BAY3D_BAY3D_SIGGAUS 0x2cf4
    LOGD_ANR("(0x2cf4) siggaus:0x%x 0x%x 0x%x 0x%x\n",
             pFix->siggaus0,
             pFix->siggaus1,
             pFix->siggaus2,
             pFix->siggaus3);


    LOGD_ANR("%s:(%d) ############# bayertnr exit ######################## \n", __FUNCTION__, __LINE__);
    return res;
}

Abayertnr_result_V23_t bayertnr_get_setting_by_name_json_V23(void* pCalibdb_v, char* name,
        int* calib_idx, int* tuning_idx) {
    int i = 0;
    Abayertnr_result_V23_t res = ABAYERTNRV23_RET_SUCCESS;

#if (RKAIQ_HAVE_BAYERTNR_V23)
    CalibDbV2_BayerTnrV23_t* pCalibdb = (CalibDbV2_BayerTnrV23_t*)pCalibdb_v;
#else
    CalibDbV2_BayerTnrV23Lite_t* pCalibdb      = (CalibDbV2_BayerTnrV23Lite_t*)pCalibdb_v;
#endif

    if(pCalibdb == NULL || name == NULL || calib_idx == NULL || tuning_idx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
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

Abayertnr_result_V23_t bayertnr_init_params_json_V23(void* pParams_v, void* pCalibdb_v,
        int calib_idx, int tuning_idx) {
    Abayertnr_result_V23_t res = ABAYERTNRV23_RET_SUCCESS;
    CalibDbV2_BayerTnrV23_C_ISO_t *pCalibIso = NULL;
#if (RKAIQ_HAVE_BAYERTNR_V23)
    CalibDbV2_BayerTnrV23_t* pCalibdb         = (CalibDbV2_BayerTnrV23_t*)pCalibdb_v;
    RK_Bayertnr_Params_V23_t* pParams         = (RK_Bayertnr_Params_V23_t*)pParams_v;
    CalibDbV2_BayerTnrV23_T_ISO_t* pTuningIso = NULL;
#else
    CalibDbV2_BayerTnrV23Lite_t* pCalibdb      = (CalibDbV2_BayerTnrV23Lite_t*)pCalibdb_v;
    RK_Bayertnr_Params_V23L_t* pParams         = (RK_Bayertnr_Params_V23L_t*)pParams_v;
    CalibDbV2_BayerTnrV23L_T_ISO_t* pTuningIso = NULL;
#endif

#if 1
    LOGI_ANR("%s:(%d) oyyf bayerner xml config start\n", __FUNCTION__, __LINE__);
    if(pParams == NULL || pCalibdb == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    pParams->enable = pCalibdb->TuningPara.enable;
    for(int i = 0; i < pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO_len && i < RK_BAYERNR_V23_MAX_ISO_NUM; i++) {
        pCalibIso = &pCalibdb->CalibPara.Setting[calib_idx].Calib_ISO[i];
        pParams->iso[i] = pCalibIso->iso;
        for(int k = 0; k < 16; k++) {
            pParams->bayertnrParamISO[i].lumapoint[k] = pCalibIso->lumapoint[k];
            pParams->bayertnrParamISO[i].sigma[k] = pCalibIso->sigma[k];
            pParams->bayertnrParamISO[i].lumapoint2[k] = pCalibIso->lumapoint2[k];
            pParams->bayertnrParamISO[i].lo_sigma[k] = pCalibIso->lo_sigma[k];
            pParams->bayertnrParamISO[i].hi_sigma[k] = pCalibIso->hi_sigma[k];
        }
    }

    for(int i = 0; i < pCalibdb->TuningPara.Setting[tuning_idx].Tuning_ISO_len && i < RK_BAYERNR_V23_MAX_ISO_NUM; i++) {
        pTuningIso = &pCalibdb->TuningPara.Setting[tuning_idx].Tuning_ISO[i];
        pParams->iso[i] = pTuningIso->iso;
        pParams->bayertnrParamISO[i].thumbds_w = pCalibdb->TuningPara.thumbds_w;
        pParams->bayertnrParamISO[i].thumbds_h = pCalibdb->TuningPara.thumbds_h;
        pParams->bayertnrParamISO[i].lo_enable = pCalibdb->TuningPara.lo_enable;
        pParams->bayertnrParamISO[i].hi_enable = pTuningIso->hi_enable;
        pParams->bayertnrParamISO[i].lo_filter_strength = pTuningIso->lo_filter_strength;
        pParams->bayertnrParamISO[i].hi_filter_strength = pTuningIso->hi_filter_strength;
        pParams->bayertnrParamISO[i].soft_threshold_ratio = pTuningIso->soft_threshold_ratio;


        pParams->bayertnrParamISO[i].lo_clipwgt = pTuningIso->lo_clipwgt;
#if (RKAIQ_HAVE_BAYERTNR_V23)
        pParams->bayertnrParamISO[i].lo_med_en = pTuningIso->lo_med_en;
#endif
        pParams->bayertnrParamISO[i].lo_gsbay_en = pTuningIso->lo_gsbay_en;
        pParams->bayertnrParamISO[i].lo_gslum_en = pTuningIso->lo_gslum_en;
#if (RKAIQ_HAVE_BAYERTNR_V23)
        pParams->bayertnrParamISO[i].hi_med_en = pTuningIso->hi_med_en;
#endif
        pParams->bayertnrParamISO[i].hi_gslum_en = pTuningIso->hi_gslum_en;
        pParams->bayertnrParamISO[i].global_pk_en = pTuningIso->global_pk_en;
        pParams->bayertnrParamISO[i].global_pksq = pTuningIso->global_pksq;

        pParams->bayertnrParamISO[i].hidif_th = pTuningIso->hidif_th;
        pParams->bayertnrParamISO[i].hi_wgt_comp = pTuningIso->hi_wgt_comp;

        pParams->bayertnrParamISO[i].lo_filter_rat0 = pTuningIso->lo_filter_rat0;
        pParams->bayertnrParamISO[i].lo_filter_thed0 = pTuningIso->lo_filter_thed0;


        pParams->bayertnrParamISO[i].hi_filter_abs_ctrl = pTuningIso->hi_filter_abs_ctrl;
#if (RKAIQ_HAVE_BAYERTNR_V23)
        pParams->bayertnrParamISO[i].hi_filter_filt_avg = pTuningIso->hi_filter_filt_avg;
        pParams->bayertnrParamISO[i].hi_filter_filt_bay = pTuningIso->hi_filter_filt_bay;
        pParams->bayertnrParamISO[i].hi_filter_filt_mode = pTuningIso->hi_filter_filt_mode;
#endif

        pParams->bayertnrParamISO[i].hi_filter_rat0 = pTuningIso->hi_filter_rat0;
        pParams->bayertnrParamISO[i].hi_filter_thed0 = pTuningIso->hi_filter_thed0;
        pParams->bayertnrParamISO[i].hi_filter_rat1 = pTuningIso->hi_filter_rat1;
        pParams->bayertnrParamISO[i].hi_filter_thed1 = pTuningIso->hi_filter_thed1;


        pParams->bayertnrParamISO[i].guass_guide_coeff0 = pTuningIso->guass_guide_coeff0;
        pParams->bayertnrParamISO[i].guass_guide_coeff1 = pTuningIso->guass_guide_coeff1;
        pParams->bayertnrParamISO[i].guass_guide_coeff2 = pTuningIso->guass_guide_coeff2;
#if (RKAIQ_HAVE_BAYERTNR_V23)
        pParams->bayertnrParamISO[i].guass_guide_coeff3 = pTuningIso->guass_guide_coeff3;
#endif
        pParams->bayertnrParamISO[i].wgt_use_mode = pTuningIso->wgt_use_mode;
        pParams->bayertnrParamISO[i].wgt_mge_mode = pTuningIso->wgt_mge_mode;
#if (RKAIQ_HAVE_BAYERTNR_V23)
        pParams->bayertnrParamISO[i].hi_guass = pTuningIso->hi_guass;
        pParams->bayertnrParamISO[i].kl_guass = pTuningIso->kl_guass;
#endif

#if (RKAIQ_HAVE_BAYERTNR_V23_LITE)
        pParams->bayertnrParamISO[i].wgtmm_opt_en = pTuningIso->wgtmm_opt_en;
        pParams->bayertnrParamISO[i].wgtmm_sel_en = pTuningIso->wgtmm_sel_en;
        pParams->bayertnrParamISO[i].wgtmin       = pTuningIso->wgtmin;
#endif

        pParams->bayertnrParamISO[i].trans_en = pCalibdb->TuningPara.trans_en;

    }

    LOGI_ANR("%s:(%d) oyyf bayerner xml config end!   \n", __FUNCTION__, __LINE__);
#endif
    return res;
}

Abayertnr_result_V23_t bayertnr_config_setting_param_json_V23(void* pParams_v, void* pCalibdbV23_v,
        char* param_mode, char* snr_name) {
    Abayertnr_result_V23_t res = ABAYERTNRV23_RET_SUCCESS;
    int calib_idx = 0;
    int tuning_idx = 0;

#if (RKAIQ_HAVE_BAYERTNR_V23)
    RK_Bayertnr_Params_V23_t* pParams    = (RK_Bayertnr_Params_V23_t*)pParams_v;
    CalibDbV2_BayerTnrV23_t* pCalibdbV23 = (CalibDbV2_BayerTnrV23_t*)pCalibdbV23_v;
#else
    RK_Bayertnr_Params_V23L_t* pParams         = (RK_Bayertnr_Params_V23L_t*)pParams_v;
    CalibDbV2_BayerTnrV23Lite_t* pCalibdbV23   = (CalibDbV2_BayerTnrV23Lite_t*)pCalibdbV23_v;
#endif

    if(pParams == NULL || pCalibdbV23 == NULL
            || param_mode == NULL || snr_name == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ABAYERTNRV23_RET_NULL_POINTER;
    }

    res = bayertnr_get_setting_by_name_json_V23(pCalibdbV23, snr_name, &calib_idx, &tuning_idx);
    if(res != ABAYERTNRV23_RET_SUCCESS) {
        LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);

    }

    res = bayertnr_init_params_json_V23(pParams, pCalibdbV23, calib_idx, tuning_idx);

    return res;
}

RKAIQ_END_DECLARE

