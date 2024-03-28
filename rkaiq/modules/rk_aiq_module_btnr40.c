#include "rk_aiq_isp39_modules.h"

// fix define
#define FIXTNRSQRT      30
#define FIXTNRWGT       10
#define FIXTNRKAL       8
#define FIXTNRWWW       12
#define FIXGAINOUT      8
#define FIXTNRSFT       10
#define FIXMERGEWG      10      // HDR merge weight
#define MERGEWGBIT      6       // HDR merge weight cut bits
#define FIXGAINBIT      8
#define FIXTNRCALC      10
#define FIXBILCALC      10
#define FIXDIFMAX       ((long long)1<<(14))     // DIFF USE 12bits
#define FIXBITIIRSPR    8
#define FIXBITWFWGT     8
#define FIXBITDGAIN     8

void bayertnr_logtrans_init_V30(int bayertnr_trans_mode,  int bayertnr_trans_mode_scale, btnr_trans_params_v30_t *pTransPrarms)
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
                pTransPrarms->bayertnr_logtablef[i] = (int)(tmp * bayertnr_logtblmul);

                tmp = i;
                tmp = 1 + tmp / tmp1;
                tmp = pow(tmp, 2);
                pTransPrarms->bayertnr_logtablei[i] = (int)(tmp * bayertnr_logtblmul);
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
                pTransPrarms->bayertnr_logtablef[i] = (int)(tmp * bayertnr_logtblmul);
                tmp = i;
                tmp = tmp / tmp1;
                tmp = pow(2, tmp);
                pTransPrarms->bayertnr_logtablei[i] = (int)(tmp * bayertnr_logtblmul);
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

int  bayertnr_logtrans_V30(uint32_t tmpfix, struct isp39_bay3d_cfg* pFix, btnr_trans_params_v30_t *pTransPrarms)
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

int bayertnr_tnr_noise_curve_V30(int data, int isHdrShort, struct isp39_bay3d_cfg* pFix, btnr_trans_params_v30_t *pTransPrarms)
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

static void bay_bifilt13x9_filter_coeff(int graymode, float sigma, int halftaby, int halftabx, uint16_t*rbtab, uint16_t *ggtab)
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

int bayertnr_autosigma_config_V30(bool isFirstFrame,
                                  struct isp39_bay3d_stat  *pStats,
                                  btnr_trans_params_v30_t *pTransPrarms)
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

void rk_aiq_btnr40_update_sq(struct isp39_isp_params_cfg* isp_cfg, btnr_cvt_info_t *pBtnrInfo, uint32_t frameId)
{
    int tmp, tmp0, tmp1;
    struct isp39_bay3d_cfg *pFix = &isp_cfg->others.bay3d_cfg;
    btnr_trans_params_v30_t *pTransPrarms = &pBtnrInfo->mBtnrTransParams;

    /* BAY3D_TNRSIGORG */
    // noise balance update info
    int pre_wk_stat, pre_pk_stat, wk_stat;
    int sigorg, sigsta, wsta;
    long long pkp1_sq, tmpL0, tmpL1;
    sigorg = 256;
    if(frameId == 0)
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

    //LOGK("rk_aiq_btnr40_update_sq %d, tnr_out_sigma_sq 0x%lx", frameId, pFix->tnr_out_sigma_sq);
}

void rk_aiq_btnr40_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo, btnr_cvt_info_t *pBtnrInfo)
{
    btnr_trans_params_v30_t *pTransPrarms = &pBtnrInfo->mBtnrTransParams;
    struct isp39_bay3d_stat *btnr_stats = &pBtnrInfo->mBtnrStats;
    bool bypass = pBtnrInfo->bypass;

    int i;
    int tmp, tmp0, tmp1;
    struct isp39_bay3d_cfg *pFix = &isp_cfg->others.bay3d_cfg;

    btnr_param_t *btnr_param = (btnr_param_t *) attr;
    btnr_params_static_t* psta = &btnr_param->sta;
    btnr_other_dyn_t * pdyn = &btnr_param->dyn;
    btnr_md_dyn_t* pmdDyn = &btnr_param->mdDyn;

    bool bayertnr_default_noise_curve_use = false;
    int bayertnr_iso_cur = cvtinfo->frameIso[cvtinfo->frameNum- 1];
    bayertnr_default_noise_curve_use = bayertnr_find_top_one_pos_V30(bayertnr_iso_cur) != bayertnr_find_top_one_pos_V30(pTransPrarms->bayertnr_iso_pre);

    // BAY3D_CTRL0 0x2c00
    pFix->bypass_en = bypass;
    pFix->iirsparse_en = 0;

    //BAY3D_CTRL1 0x2c04
    switch (psta->hw_btnrCfg_pixDomain_mode) {
    case btnr_pixLog2Domain_mode :
        pFix->transf_bypass_en = 0;
        pFix->transf_mode  = 0;
        break;
    case btnr_pixSqrtDomain_mode:
        pFix->transf_bypass_en = 0;
        pFix->transf_mode  = 1;
        break;
    case btnr_pixLinearDomain_mode:
        pFix->transf_bypass_en = 1;
        break;
    }

    if (cvtinfo->preDGain > 1.0 || cvtinfo->frameNum > 1) {
        if (pFix->transf_bypass_en) {
            LOGE_ANR("%s:: pFix->transf_bypass_en error !\n");
            pFix->transf_bypass_en = 0;
        }
    }

    pFix->sigma_curve_double_en =  (cvtinfo->frameNum > 1) ? 1 : 0;
    pFix->cur_spnr_bypass_en = !pdyn->curFrmSpNr.hw_btnrT_spNr_en;
    pFix->cur_spnr_sigma_idxfilt_bypass_en = !pdyn->curFrmSpNr.hw_btnrT_sigmaIdxLpf_en;
    pFix->cur_spnr_sigma_curve_double_en = (pdyn->curFrmSpNr.hw_btnrT_sigmaCurve_mode == btnr_midSegmInterpOff_mode);
    pFix->pre_spnr_bypass_en = !pdyn->preFrmSpNr.hw_btnrT_spNr_en;
    pFix->pre_spnr_sigma_idxfilt_bypass_en = !pdyn->preFrmSpNr.hw_btnrT_sigmaIdxLpf_en;
    pFix->pre_spnr_sigma_curve_double_en = (pdyn->preFrmSpNr.hw_btnrT_sigmaCurve_mode == btnr_midSegmInterpOff_mode);
    pFix->lpf_hi_bypass_en = !pmdDyn->loAsHiRatioMd.hw_btnrT_hfLpf_en;
    pFix->lo_diff_vfilt_bypass_en = !pmdDyn->subLoMd0_diffCh.hw_btnrT_vFilt_en;
    pFix->lpf_lo_bypass_en = !pmdDyn->subLoMd1.hw_btnrT_lpf_en;
    pFix->lo_wgt_hfilt_en = pmdDyn->subLoMd1.hw_btnrT_hFilt_en;
    pFix->lo_diff_hfilt_en = pmdDyn->subLoMd0_diffCh.hw_btnrT_hFilt_en;
    pFix->sig_hfilt_en = pmdDyn->subLoMd0_sgmCh.hw_btnrT_hFilt_en;
    //pFix->gkalman_en = pSelect->hw_btnr_gKalman_en;
    pFix->spnr_pre_sigma_use_en = (pdyn->preFrmSpNr.hw_btnrT_sigma_mode == btnr_kalPkSgm_mode);

    if (pmdDyn->loMd.hw_btnrT_loMd_en)
        pFix->lo_detection_mode = pmdDyn->loMd.hw_btnrT_loMd_mode;
    else
        pFix->lo_detection_mode = 3;

    pFix->md_bypass_en = !pmdDyn->hw_btnrT_md_en;
    if (psta->debug.sw_btnrT_dbgOut_en) {
        switch (psta->debug.hw_btnrT_dbgOut_mode) {
        case btnr_dbgOut_iirSpNr_mode:
            pFix->iirspnr_out_en = 1;
            pFix->curdbg_out_en = 0;
            pFix->md_wgt_out_en = 0;
            break;
        case btnr_dbgOut_curSpNr_mode :
            pFix->iirspnr_out_en = 0;
            pFix->curdbg_out_en = 1;
            pFix->md_wgt_out_en = 0;
            break;
        case btnr_dbgOut_mdWgt_mode:
            pFix->iirspnr_out_en = 0;
            pFix->curdbg_out_en = 0;
            pFix->md_wgt_out_en = 1;
            break;
        }
    } else {
        pFix->iirspnr_out_en = 0;
        pFix->curdbg_out_en = 0;
        pFix->md_wgt_out_en = 0;
    }
    if (pmdDyn->hw_btnrT_md_mode == btnr_loMdOnly_mode)
        pFix->lomdwgt_dbg_en = 1;
    pFix->noisebal_mode = pdyn->hw_btnrT_frmAlpha_mode;

    if (cvtinfo->frameNum > 1) {
        if (pFix->cur_spnr_sigma_curve_double_en == 0) {
            LOGE_ANR("%s:: cur_spnr_sigma_curve_double_en error !\n");
            pFix->cur_spnr_sigma_curve_double_en = 1;
        }
        if (pFix->pre_spnr_sigma_curve_double_en == 0) {
            LOGE_ANR("%s:: pre_spnr_sigma_curve_double_en error !\n");
            pFix->pre_spnr_sigma_curve_double_en  = 1;
        }
    }

    // BAY3D_CTRL2 0x2c08
    int last_transf_mode = pFix->transf_mode;
    int last_transf_mode_scale = pFix->transf_mode_scale;
    if (pmdDyn->hw_btnrT_md_mode == btnr_loAsRatioForHi_mode)
        pFix->wgt_cal_mode = 0;
    if (pmdDyn->hw_btnrT_md_mode == btnr_loAsBiasForHi_mode)
        pFix->wgt_cal_mode = 1;
    pFix->mge_wgt_ds_mode = 0;
    pFix->kalman_wgt_ds_mode = 0;
    pFix->mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;
    pFix->sigma_calc_mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;

    // BAY3D_TRANS0 0x2c0c
    tmp = psta->transCfg.hw_btnr_trans_offset;
    pFix->transf_mode_offset = CLIP(tmp, 0, 0x1fff);

    tmp = psta->transCfg.hw_btnr_trans_scale;
    pFix->transf_mode_scale = CLIP(tmp, 0, 0x01);
    if (cvtinfo->frameNum > 1)
        pFix->transf_mode_scale = 0;

    if (pFix->transf_mode_scale == 0 && pFix->transf_bypass_en == 0) {
        if (pFix->transf_mode_offset == 512)
            pFix->itransf_mode_offset = 4096;
        else if (pFix->transf_mode_offset == 1024)
            pFix->itransf_mode_offset = 8192;
        else
            pFix->itransf_mode_offset = 2048;
    } else {
        pFix->itransf_mode_offset = 2048;
    }

    // BAY3D_TRANS1 0x2c10
    tmp = psta->transCfg.hw_btnr_transData_maxLimit;
    pFix->transf_data_max_limit = 1048575;


    // BAY3D_CURDGAIN 0x2c14
    tmp = pdyn->curFrmSpNr.hw_btnrT_sigmaHdrS_scale * (1 << FIXBITDGAIN);
    pFix->cur_spnr_sigma_hdr_sht_scale = CLIP(tmp, 0, 0xffff);
    tmp = pdyn->curFrmSpNr.hw_btnrT_sigma_scale * (1 << FIXBITDGAIN);
    pFix->cur_spnr_sigma_scale = CLIP(tmp, 0, 0xffff);

    // tnr sigma curve must calculate before spnr sigma
    float kcoef0, kcoef1;
    int max_sig, pix_max;
    if(cvtinfo->isFirstFrame|| (last_transf_mode != pFix->transf_mode) || (last_transf_mode_scale != pFix->transf_mode_scale)) {
        bayertnr_logtrans_init_V30(pFix->transf_mode, pFix->transf_mode_scale, pTransPrarms);
    }

    int sigbins = 20;
    pix_max = pFix->transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans_V30((1 << 12) - 1, pFix, pTransPrarms);
    if(cvtinfo->frameNum == 2 ) {
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
            if(cvtinfo->frameDGain[0] <= (1 << i))
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
    if((psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_autoSigma_mode ) && !(cvtinfo->isFirstFrame)) {
        pTransPrarms->bayertnr_auto_sig_count_en = 1;
        pTransPrarms->bayertnr_auto_sig_count_filt_wgt = pdyn->sigmaEnv.sw_btnrT_autoSgmIIR_alpha * (1 << 10);
        pTransPrarms->bayertnr_auto_sig_count_max = cvtinfo->rawWidth * cvtinfo->rawHeight / 3;
        bayertnr_autosigma_config_V30(cvtinfo->isFirstFrame, btnr_stats, pTransPrarms);
    }

    if(psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_manualSigma_mode || cvtinfo->isFirstFrame || bayertnr_default_noise_curve_use) {
        kcoef0 = 1.0;
        max_sig = ((1 << 12) - 1);
        for(i = 0; i < sigbins; i++) {
            pFix->tnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.val[i] * kcoef0), 0, max_sig);
            pTransPrarms->tnr_luma_sigma_y[i] = pFix->tnr_luma_sigma_y[i];
        }
    } else {
        kcoef0 = 1.0;
        max_sig = ((1 << 12) - 1);
        for(i = 0; i < sigbins; i++)
            pFix->tnr_luma_sigma_y[i] = CLIP((int)(pTransPrarms->tnr_luma_sigma_y[i] * kcoef0), 0, max_sig);
    }

    int spnrsigbins = 16;
    bool auto_sig_curve_spnruse = psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_autoSigma_mode;
    pix_max = pFix->transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans_V30((1 << 12) - 1, pFix, pTransPrarms);
    if(cvtinfo->frameNum == 2) {
        //pFix->cur_spnr_sigma_curve_double_en = 1;
        //pFix->pre_spnr_sigma_curve_double_en = 1;
        //pTransPrarms->bayertnr_cur_spnr_sigma_curve_double_pos = 8;
        //pTransPrarms->bayertnr_pre_spnr_sigma_curve_double_pos = 8;
        int lgbins = 8;
        float kcoef0 = 1.0;
        float kcoef1 = 1.0;
        int max_sig = ((1 << 12) - 1);
        // hdr long bins
        for(i = 0; i < lgbins; i++) {
            pFix->cur_spnr_luma_sigma_x[i] = 128 * (i + 1);
            pFix->pre_spnr_luma_sigma_x[i] = 128 * (i + 1);
        }
        for(i = 0; i < lgbins; i++) {
            if(!auto_sig_curve_spnruse || cvtinfo->isFirstFrame) {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_curSpNrSgm_curve.val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val[i] * kcoef1), 0, max_sig);
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
            if(cvtinfo->frameDGain[0] <= (1 << i))
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
            if(!auto_sig_curve_spnruse || cvtinfo->isFirstFrame)
            {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_curSpNrSgm_curve.val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val[i] * kcoef1), 0, max_sig);
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
        float kcoef0 = 1.0;
        float kcoef1 = 1.0;
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
            if(!auto_sig_curve_spnruse || cvtinfo->isFirstFrame)
            {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_curSpNrSgm_curve.val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val[i] * kcoef1), 0, max_sig);
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
        int lgbins = 0;
        float kcoef0 = 1.0;
        float kcoef1 = 1.0;
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
            if(!auto_sig_curve_spnruse || cvtinfo->isFirstFrame )
            {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_curSpNrSgm_curve.val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val[i] * kcoef1), 0, max_sig);
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
    tmp = pdyn->curFrmSpNr.hw_btnrT_sigmaHdrS_offset * (1 << FIXBITDGAIN) ;
    pFix->cur_spnr_sigma_hdr_sht_offset = CLIP(tmp, 0, 0xfff);
    tmp = pdyn->curFrmSpNr.hw_btnrT_sigma_offset * (1 << FIXBITDGAIN);
    pFix->cur_spnr_sigma_offset = CLIP(tmp, 0, 0xfff);

    /* BAY3D_CURWTH */
    tmp = pdyn->curFrmSpNr.hw_btnrT_pixDiff_maxLimit;
    pFix->cur_spnr_pix_diff_max_limit = tmp;
    tmp = pdyn->curFrmSpNr.hw_btnrT_pixDiff_negOff * (1 << 10);
    pFix->cur_spnr_wgt_cal_offset = CLIP(tmp, 0, 0xfff);

    /* BAY3D_CURBFALP */
    tmp = (cvtinfo->isFirstFrame) ? 0 : pdyn->curFrmSpNr.hw_btnrT_spNrOut_alpha * (1 << 10);
    pFix->cur_spnr_wgt = CLIP(tmp, 0, 0x3ff);
    tmp = pdyn->preFrmSpNr.hw_btnrT_spNrOut_alpha  * (1 << 10);
    pFix->pre_spnr_wgt = CLIP(tmp, 0, 0x3ff);

    /* BAY3D_CURWDC */
    bay_bifilt13x9_filter_coeff(cvtinfo->isGrayMode, pdyn->curFrmSpNr.sw_btnrT_filtSpatial_strg, 5, 7, pFix->cur_spnr_space_rb_wgt, pFix->cur_spnr_space_gg_wgt);

    /* BAY3D_IIRDGAIN */
    tmp = pdyn->preFrmSpNr.hw_btnrT_sigmaHdrS_scale * (1 << FIXBITDGAIN);
    pFix->pre_spnr_sigma_hdr_sht_scale = CLIP(tmp, 0, 0xfff);
    tmp = pdyn->preFrmSpNr.hw_btnrT_sigma_scale * (1 << FIXBITDGAIN);
    pFix->pre_spnr_sigma_scale = CLIP(tmp, 0, 0xfff);

    /* BAY3D_IIRGAIN_OFF */
    pFix->pre_spnr_sigma_rgain_offset = 0;
    pFix->pre_spnr_sigma_bgain_offset = 0;

    /* BAY3D_IIRSIG_OFF */
    tmp = pdyn->preFrmSpNr.hw_btnrT_sigmaHdrS_offset * (1 << FIXBITDGAIN) ;
    pFix->pre_spnr_sigma_hdr_sht_offset = CLIP(tmp, 0, 0xfff);
    tmp = pdyn->preFrmSpNr.hw_btnrT_sigma_offset  * (1 << FIXBITDGAIN);
    pFix->pre_spnr_sigma_offset = CLIP(tmp, 0, 0xfff);


    /* BAY3D_IIRWTH */
    tmp = pdyn->preFrmSpNr.hw_btnrT_pixDiff_maxLimit;
    pFix->pre_spnr_pix_diff_max_limit = tmp;
    tmp = pdyn->preFrmSpNr.hw_btnrT_pixDiff_negOff * (1 << 10);
    pFix->pre_spnr_wgt_cal_offset = CLIP(tmp, 0, 0xfff);

    /* BAY3D_IIRWDC */
    /* BAY3D_IIRWDY */
    //float sigmaiir = 25;
    bay_bifilt13x9_filter_coeff(cvtinfo->isGrayMode, pdyn->preFrmSpNr.sw_btnrT_filtSpatial_strg, 5, 7, pFix->pre_spnr_space_rb_wgt, pFix->pre_spnr_space_gg_wgt);

    /* BAY3D_BFCOEF */
    tmp = pdyn->curFrmSpNr.hw_btnrT_pixDiff_scale * (1 << 10);
    pFix->cur_spnr_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);
    tmp = pdyn->preFrmSpNr.hw_btnrT_pixDiff_scale * (1 << 10);
    pFix->pre_spnr_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRSIG_X */
    /* BAY3D_TNRSIG_Y */
    //tnr sigma curve must calculate befor spnr sigma curve

    /* BAY3D_TNRHIW */
    bay_gauss7x5_filter_coeff(pmdDyn->loAsHiRatioMd.sw_btnrT_hfLpf_strg, 5, 7, 7, pFix->tnr_lpf_hi_coeff);

    /* BAY3D_TNRLOW*/
    bay_gauss7x5_filter_coeff(pmdDyn->subLoMd1.sw_btnrT_lpf_strg, 5, 7, 7, pFix->tnr_lpf_lo_coeff);

    /* BAY3D_TNRGF3 */
    tmp = pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[0] * (1 << 6);
    pFix->tnr_wgt_filt_coeff0 = CLIP(tmp, 0, 0x3ff);
    tmp = pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[1] * (1 << 6);
    pFix->tnr_wgt_filt_coeff1 = CLIP(tmp, 0, 0x3ff);
    tmp = pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[2] * (1 << 6);
    pFix->tnr_wgt_filt_coeff2 = CLIP(tmp, 0, 0x3ff);
    tmp =  1024 - pFix->tnr_wgt_filt_coeff1 * 4 - pFix->tnr_wgt_filt_coeff2 * 4;
    if(tmp < 0) {
        LOGE_ANR("%s:%d: hw_tnrWgtFltCoef:%f + 4*%f + 4*%f = %f should less than 16, use[4,2,1]instead, please check iq params!\n",
                 pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[0], pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[1], pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[2],
                 pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[0] + 4 * pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[1] + 4 * pmdDyn->mdWgtPost.hw_btnr_lpfSpatial_wgt[2]);
        pFix->tnr_wgt_filt_coeff0 = 4 * (1 << 6);
        pFix->tnr_wgt_filt_coeff1 = 2 * (1 << 6);
        pFix->tnr_wgt_filt_coeff2 = 1 * (1 << 6);
    } else {
        pFix->tnr_wgt_filt_coeff0 = CLIP(tmp, 0, 0x3ff);
    }

    /* BAY3D_TNRSIGSCL */
    tmp = pmdDyn->mdSigma.hw_btnrT_sigma_scale * (1 << 10);
    pFix->tnr_sigma_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->mdSigma.hw_btnrT_sigmaHdrS_scale * (1 << 10);
    pFix->tnr_sigma_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRVIIR */
    tmp = pmdDyn->subLoMd0_sgmCh.hw_btnrT_vIIRFilt_strg * (1 << 4);
    pFix->tnr_sig_vfilt_wgt  = CLIP(tmp, 0, 0xf);
    tmp = pmdDyn->subLoMd0_diffCh.hw_btnrT_vIIRFilt_strg * (1 << 4);
    pFix->tnr_lo_diff_vfilt_wgt  = CLIP(tmp, 0, 0xf);
    tmp = pmdDyn->subLoMd1.hw_btnrT_vIIRFilt_strg * (1 << 4);
    pFix->tnr_lo_wgt_vfilt_wgt  = CLIP(tmp, 0, 0xf);
    tmp = pmdDyn->subLoMd0_sgmCh.hw_btnrT_vIIRFstLn_scale * (1 << 4);
    pFix->tnr_sig_first_line_scale = CLIP(tmp, 0, 0x1f);
    tmp = pmdDyn->subLoMd0_diffCh.hw_btnrT_vIIRFstLn_scale * (1 << 4);
    pFix->tnr_lo_diff_first_line_scale = CLIP(tmp, 0, 0x1f);

    /* BAY3D_TNRLFSCL */
    tmp = pmdDyn->subLoMd1.hw_btnrT_mdWgt_negOff * (1 << 10);
    pFix->tnr_lo_wgt_cal_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->subLoMd1.hw_btnrT_mdWgt_scale * (1 << 10);
    pFix->tnr_lo_wgt_cal_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRLFSCLTH */
    tmp = pmdDyn->subLoMd1.hw_btnrT_mdWgt_maxLimit * (1 << 10);
    pFix->tnr_low_wgt_cal_max_limit  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsHiRatioMd.hw_btnrT_hiMdWgt_scale * (1 << 10);
    pFix->tnr_mode0_base_ratio  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRDSWGTSCL */
    tmp = pmdDyn->subLoMd0_wgtOpt.hw_btnrT_mdWgt_negOff * (1 << 10);
    pFix->tnr_lo_diff_wgt_cal_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->subLoMd0_wgtOpt.hw_btnrT_mdWgt_scale * (1 << 10);
    pFix->tnr_lo_diff_wgt_cal_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWLSTSCL */
    tmp = pmdDyn->loMd.hw_btnrT_preWgtMge_offset * (1 << 10);
    pFix->tnr_lo_mge_pre_wgt_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loMd.hw_btnrT_preWgtMge_scale * (1 << 10);
    pFix->tnr_lo_mge_pre_wgt_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT0SCL0 */
    tmp = pmdDyn->loAsHiRatioMd.hw_btnrT_loWgtStat_scale * (1 << 10);
    pFix->tnr_mode0_lo_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsHiRatioMd.hw_btnrT_loWgtStatHdrS_scale * (1 << 10);
    pFix->tnr_mode0_lo_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1SCL1 */
    tmp = pmdDyn->loAsHiBiasMd.hw_btnrT_loWgtStat_scale * (1 << 10);
    pFix->tnr_mode1_lo_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsHiBiasMd.hw_btnrT_loWgtStatHdrS_scale * (1 << 10);
    pFix->tnr_mode1_lo_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1SCL2 */
    tmp = pmdDyn->loAsHiBiasMd.hw_btnrT_mdWgt_scale * (1 << 10);
    pFix->tnr_mode1_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsHiBiasMd.hw_btnrT_mdWgtHdrS_scale  * (1 << 10);
    pFix->tnr_mode1_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGTOFF */
    tmp = pmdDyn->loAsHiBiasMd.hw_btnrT_loWgtStat_negOff * (1 << 10);
    pFix->tnr_mode1_lo_wgt_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsHiBiasMd.hw_btnrT_loWgtStatHdrS_negOff * (1 << 10);
    pFix->tnr_mode1_lo_wgt_hdr_sht_offset  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1OFF */
    tmp = pdyn->sigmaEnv.hw_btnrT_statsPixAlpha_thred * (1 << 10);
    pFix->tnr_auto_sigma_count_wgt_thred  = CLIP(tmp, 0, 0x3ff);
    tmp = pmdDyn->loAsHiBiasMd.hw_btnrT_loWgtStat_minLimit * (1 << 10);
    pFix->tnr_mode1_wgt_min_limit  = CLIP(tmp, 0, 0x3ff);
    tmp = pmdDyn->loAsHiBiasMd.hw_btnrT_loWgtStat_offset * (1 << 10);
    pFix->tnr_mode1_wgt_offset  = CLIP(tmp, 0, 0xfff);

    /* BAY3D_TNRWLO_THL */
    tmp =  pdyn->frmAlpha_lo.hw_btnrT_loAlpha_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pdyn->frmAlpha_lo.hw_btnrT_loAlpha_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_min_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pdyn->frmAlpha_lo.hw_btnrT_loAlphaHdrS_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pdyn->frmAlpha_lo.hw_btnrT_loAlphaHdrS_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_hdr_sht_min_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWLO_THH */
    tmp =  pdyn->frmAlpha_lo.hw_btnrT_loAlpha_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pdyn->frmAlpha_lo.hw_btnrT_loAlpha_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_max_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pdyn->frmAlpha_lo.hw_btnrT_loAlphaHdrS_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pdyn->frmAlpha_lo.hw_btnrT_loAlphaHdrS_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_hdr_sht_max_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWHI_THL */
    tmp =  pdyn->frmAlpha_hiByHi.hw_btnrT_hiAlpha_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pdyn->frmAlpha_hiByHi.hw_btnrT_hiAlpha_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_min_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pdyn->frmAlpha_hiByHi.hw_btnrT_hiAlphaHdrS_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pdyn->frmAlpha_hiByHi.hw_btnrT_hiAlphaHdrS_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_hdr_sht_min_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWHI_THH */
    tmp =  pdyn->frmAlpha_hiByHi.hw_btnrT_hiAlpha_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pdyn->frmAlpha_hiByHi.hw_btnrT_hiAlpha_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_max_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pdyn->frmAlpha_hiByHi.hw_btnrT_hiAlphaHdrS_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pdyn->frmAlpha_hiByHi.hw_btnrT_hiAlphaHdrS_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_hdr_sht_max_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRKEEP */
    tmp =  pdyn->frmAlpha_hiByHi.hw_btnrT_curHiOrg_alpha * (1 << FIXBITWFWGT);
    pFix->tnr_cur_spnr_hi_wgt_min_limit  = CLIP(tmp, 0, 0xff);
    tmp =  pdyn->frmAlpha_hiByHi.hw_btnrT_iirHiOrg_alpha * (1 << FIXBITWFWGT);
    pFix->tnr_pre_spnr_hi_wgt_min_limit  = CLIP(tmp, 0, 0xff);

    if(pFix->transf_bypass_en) {
        pFix->tnr_pix_max = 0xfff;
    } else {
        pFix->tnr_pix_max = 0xc00;
    }

    tmp = psta->sigmaEnv.hw_btnrCfg_statsPixCnt_thred;
    pFix->tnr_auto_sigma_count_th = CLIP(tmp, 0, 0xfffff);

    tmp = pmdDyn->subLoMd1.hw_btnrT_mdWgtFstLnNegOff_en;
    pFix->lowgt_ctrl = CLIP(tmp, 0, 3);
    tmp = pmdDyn->subLoMd1.hw_btnrT_mdWgtFstLn_negOff;
    pFix->lowgt_offint = CLIP(tmp, 0, 0x3ff);

    tmp = MIN(((float)1.0 / (float)(pdyn->frmAlpha_hiByLo.hw_btnrT_hiMotionNr_strg) * (1 << FIXTNRWGT)), (1 << FIXTNRWGT)); //(1<<FIXTNRWGT);
    pFix->tnr_motion_nr_strg = CLIP(tmp, 0, 0x7ff);
    tmp = MIN((pdyn->locSgmStrg.hw_bnrT_locSgmStrg_maxLimit * (1 << FIXGAINOUT)), ((1 << FIXGAINOUT) - 1));
    pFix->tnr_gain_max = CLIP(tmp, 0, 0xff);

    return;
}
