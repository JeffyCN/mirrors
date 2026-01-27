#include "rk_aiq_isp39_modules.h"
#include "interpolation.h"

#define FIXTNRWGT       10
#define FIXTNRKAL       8
#define FIXTNRWWW       12

int bayertnr_find_top_one_pos(int data) {
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

void bayertnr_logtrans_init(int bayertnr_trans_mode, int bayertnr_trans_mode_scale, btnr_trans_params_t *pTransPrarms)
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
            pTransPrarms->bayertnr_logtablef[i] = 2 * bayertnr_logtblmul;
            pTransPrarms->bayertnr_logtablei[i] = 4 * bayertnr_logtblmul;
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
            pTransPrarms->bayertnr_logtablef[i] = 1 * bayertnr_logtblmul;
            pTransPrarms->bayertnr_logtablei[i] = 2 * bayertnr_logtblmul;
        }
        LOGI_ANR("log domain\n");
    }

    LOGI_ANR("oyyf log demoain init: mode:%d scale%d\n", bayertnr_trans_mode, bayertnr_trans_mode_scale);
    pTransPrarms->bayertnr_logprecision = bayertnr_logprecision;
    pTransPrarms->bayertnr_logfixbit = bayertnr_logfixbit;
    pTransPrarms->bayertnr_logtblbit = bayertnr_logtblbit;
    pTransPrarms->bayertnr_logscalebit = bayertnr_logscalebit;
    pTransPrarms->bayertnr_logfixmul = bayertnr_logfixmul;
    pTransPrarms->bayertnr_logtblmul = bayertnr_logtblmul;
}

int bayertnr_logtrans(uint32_t tmpfix, btnr_trans_params_t *pTransPrarms)
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


    if(pTransPrarms->transf_mode)
    {
        long long dn;

        x8 = MIN((tmpfix + pTransPrarms->transf_mode_offset), pTransPrarms->transf_data_max_limit);

        // find highest bit
        n  = (long long)bayertnr_find_top_one_pos((int)x8);
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

        fx = fx - pTransPrarms->itransf_mode_offset;
    }
    else
    {

        x8 = MIN((tmpfix + pTransPrarms->transf_mode_offset), pTransPrarms->transf_data_max_limit);

        // find highest bit
        n = (long long)bayertnr_find_top_one_pos((int)x8);

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

        fx = fx - pTransPrarms->itransf_mode_offset;

    }

    return (int)fx;
}

int bayertnr_logiitrans(int tmpData, btnr_trans_params_t *pTransPrarms)
{
    long long iii, ss, n, dn, s;
    long long one = 1;
    long long ix1, ix2, dp;
    long long lt1, lt2, fx;
    long long stepbit, step, yy;
    long long  tempdat;
    int bayertnr_logprecision = pTransPrarms->bayertnr_logprecision;
    int bayertnr_logfixbit = pTransPrarms->bayertnr_logfixbit;
    int bayertnr_logtblbit = pTransPrarms->bayertnr_logtblbit;
    int bayertnr_logscalebit = pTransPrarms->bayertnr_logscalebit;
    int bayertnr_logfixmul = pTransPrarms->bayertnr_logfixmul;
    int bayertnr_logtblmul = pTransPrarms->bayertnr_logtblmul;
    uint16_t bayertnr_trans_mode_offset = pTransPrarms->transf_mode_offset;
    uint16_t bayertnr_itrans_mode_offset = pTransPrarms->itransf_mode_offset;
    uint8_t bayertnr_trans_mode = pTransPrarms->transf_mode;
    uint32_t bayertnr_trans_data_max = pTransPrarms->transf_data_max_limit;

    if(bayertnr_trans_mode)
    {
        fx = tmpData;
        fx = fx + bayertnr_itrans_mode_offset;

        n  = (long long)bayertnr_find_top_one_pos((int)fx);
        dn = n - bayertnr_logscalebit;

        s = fx - (one << (bayertnr_logscalebit + dn));
        s = s * bayertnr_logtblmul / (one << (bayertnr_logscalebit + dn));
        stepbit = bayertnr_logtblbit - bayertnr_logprecision;
        step = (one << stepbit);

        ix1 = s / step;
        dp = s - ix1 * step;

        ix2 = ix1 + 1;

        lt1 = pTransPrarms->bayertnr_logtablei[ix1];
        lt2 = pTransPrarms->bayertnr_logtablei[ix2];

        ss = lt1 * (step - dp) + lt2 * dp;

        ss = ss + (one << (bayertnr_logtblbit + stepbit - dn * 2 - 1));
        yy = ss >> (bayertnr_logtblbit + stepbit - dn * 2);
        yy = yy - bayertnr_trans_mode_offset;
        tempdat = MIN(yy, bayertnr_trans_data_max);
    }
    else
    {
        fx = tmpData;
        fx = fx + bayertnr_itrans_mode_offset;

        iii = fx / (one << bayertnr_logscalebit);
        ss = fx - iii * (one << bayertnr_logscalebit);

        stepbit = bayertnr_logscalebit - bayertnr_logprecision;
        step = (one << stepbit);

        ix1 = ss / step;
        dp = ss - ix1 * step;

        ix2 = ix1 + 1;

        lt1 = pTransPrarms->bayertnr_logtablei[ix1];
        lt2 = pTransPrarms->bayertnr_logtablei[ix2];

        ss = lt1 * (step - dp) + lt2 * dp;

        yy = (one << iii) * ss;
        yy = yy / (one << (bayertnr_logtblbit + stepbit));
        yy = yy - bayertnr_trans_mode_offset;
        tempdat = MIN(yy, bayertnr_trans_data_max);
    }
    return (int)tempdat;
}

int rk_autoblc_gen_tbl(unsigned int* bayertnr_itransf_tbl, uint16_t bayertnr_pixlog_max, btnr_trans_params_t *pTransPrarms)
{

    for (int j = 0; j <= bayertnr_pixlog_max; j++)
    {
        bayertnr_itransf_tbl[j] = pTransPrarms->isTransfBypass == 0 ? bayertnr_logiitrans(j, pTransPrarms) : j;
    }
    for (int j = bayertnr_pixlog_max + 1; j < 4096; j++)
    {
        bayertnr_itransf_tbl[j] = pTransPrarms->isTransfBypass == 0 ? bayertnr_logiitrans(bayertnr_pixlog_max, pTransPrarms) : j;
    }
    return 0;
}

void bayertnr_save_stats(void *stats_buffer, btnr_cvt_info_t *pBtnrInfo)
{
    if (stats_buffer == NULL)
        return;

#if RKAIQ_HAVE_BAYERTNR_V30
    struct rkisp39_stat_buffer* stats = stats_buffer;
    if (stats->meas_type & ISP39_STAT_BAY3D) {
        uint8_t min_idx = 0;
        for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
            if (pBtnrInfo->mBtnrStats[min_idx].id > pBtnrInfo->mBtnrStats[i].id)
                min_idx = i;
        }
        btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[min_idx];
        btnr_stats->id = stats->frame_id;
        btnr_stats->sigma_num = stats->stat.bay3d.tnr_auto_sigma_count;
        for (uint8_t i = 0; i < 20; i++) {
            btnr_stats->sigma_y[i] = stats->stat.bay3d.tnr_auto_sigma_calc[i];
        }
    }
#elif RKAIQ_HAVE_BAYERTNR_V41
    {
        struct rkisp33_stat_buffer* stats = stats_buffer;
        if (stats->meas_type & ISP33_STAT_BAY3D) {
            uint8_t min_idx = 0;
            for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
                if (pBtnrInfo->mBtnrStats[min_idx].id > pBtnrInfo->mBtnrStats[i].id)
                    min_idx = i;
            }
            btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[min_idx];
            btnr_stats->id = stats->frame_id;
            btnr_stats->sigma_num = stats->stat.bay3d.sigma_num;
            for (uint8_t i = 0; i < 20; i++) {
                btnr_stats->sigma_y[i] = stats->stat.bay3d.sigma_y[i];
            }
#if 0
            printf("btnr sigma stats [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
                   btnr_stats->sigma_y[0],
                   btnr_stats->sigma_y[1],
                   btnr_stats->sigma_y[2],
                   btnr_stats->sigma_y[3],
                   btnr_stats->sigma_y[4],
                   btnr_stats->sigma_y[5],
                   btnr_stats->sigma_y[6],
                   btnr_stats->sigma_y[7],
                   btnr_stats->sigma_y[8],
                   btnr_stats->sigma_y[9],
                   btnr_stats->sigma_y[10],
                   btnr_stats->sigma_y[11],
                   btnr_stats->sigma_y[12],
                   btnr_stats->sigma_y[13],
                   btnr_stats->sigma_y[14],
                   btnr_stats->sigma_y[15],
                   btnr_stats->sigma_y[16],
                   btnr_stats->sigma_y[17],
                   btnr_stats->sigma_y[18],
                   btnr_stats->sigma_y[19]);
#endif
        }
    }
#elif RKAIQ_HAVE_BAYERTNR_V42
    {
        struct rkisp35_stat_buffer* stats = stats_buffer;
        if (stats->meas_type & ISP35_STAT_BAY3D) {
            uint8_t min_idx = 0;
            for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
                if (pBtnrInfo->mBtnrStats[min_idx].id > pBtnrInfo->mBtnrStats[i].id)
                    min_idx = i;
            }
            btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[min_idx];
            btnr_stats->id = stats->frame_id;
            btnr_stats->sigma_num = stats->stat.bay3d.sigma_num;
            for (uint8_t i = 0; i < 20; i++) {
                btnr_stats->sigma_y[i] = stats->stat.bay3d.sigma_y[i];
            }
#if 0
            printf("btnr sigma stats [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
                   btnr_stats->sigma_y[0],
                   btnr_stats->sigma_y[1],
                   btnr_stats->sigma_y[2],
                   btnr_stats->sigma_y[3],
                   btnr_stats->sigma_y[4],
                   btnr_stats->sigma_y[5],
                   btnr_stats->sigma_y[6],
                   btnr_stats->sigma_y[7],
                   btnr_stats->sigma_y[8],
                   btnr_stats->sigma_y[9],
                   btnr_stats->sigma_y[10],
                   btnr_stats->sigma_y[11],
                   btnr_stats->sigma_y[12],
                   btnr_stats->sigma_y[13],
                   btnr_stats->sigma_y[14],
                   btnr_stats->sigma_y[15],
                   btnr_stats->sigma_y[16],
                   btnr_stats->sigma_y[17],
                   btnr_stats->sigma_y[18],
                   btnr_stats->sigma_y[19]);
#endif
        }

        if (stats->meas_type & ISP35_STAT_BAY3D_L2) {
            uint8_t min_idx = 0;
            for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
                if (pBtnrInfo->mBtnr2Stats[min_idx].id > pBtnrInfo->mBtnr2Stats[i].id)
                    min_idx = i;
            }
            btnr_stats_t *btnr2_stats = &pBtnrInfo->mBtnr2Stats[min_idx];
            btnr2_stats->id = stats->frame_id;
            btnr2_stats->sigma_num = stats->stat.bay3d.sigma_num;
            for (uint8_t i = 0; i < 20; i++) {
                btnr2_stats->sigma_y[i] = stats->stat.bay3d.sigma_y[i];
            }
#if 0
            printf("btnr sigma stats [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
                   btnr2_stats->sigma_y[0],
                   btnr2_stats->sigma_y[1],
                   btnr2_stats->sigma_y[2],
                   btnr2_stats->sigma_y[3],
                   btnr2_stats->sigma_y[4],
                   btnr2_stats->sigma_y[5],
                   btnr2_stats->sigma_y[6],
                   btnr2_stats->sigma_y[7],
                   btnr2_stats->sigma_y[8],
                   btnr2_stats->sigma_y[9],
                   btnr2_stats->sigma_y[10],
                   btnr2_stats->sigma_y[11],
                   btnr2_stats->sigma_y[12],
                   btnr2_stats->sigma_y[13],
                   btnr2_stats->sigma_y[14],
                   btnr2_stats->sigma_y[15],
                   btnr2_stats->sigma_y[16],
                   btnr2_stats->sigma_y[17],
                   btnr2_stats->sigma_y[18],
                   btnr2_stats->sigma_y[19]);
#endif
        }
    }
#endif

#if defined(RKAIQ_HAVE_SHARP_V40)
    {
        struct rkisp33_stat_buffer* stats = stats_buffer;
        if (stats->meas_type & ISP33_STAT_SHARP) {
            uint8_t min_idx = 0;
            for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
                if (pBtnrInfo->mSharpStats[min_idx].id > pBtnrInfo->mSharpStats[i].id)
                    min_idx = i;
            }
            sharp_stats_t *sharp_stats = &pBtnrInfo->mSharpStats[min_idx];
            sharp_stats->id = stats->frame_id;
            for (uint8_t i = 0; i < 17; i++) {
                sharp_stats->noise_curve[i] = stats->stat.sharp.noise_curve[i];
            }
        }
    }
#elif defined(RKAIQ_HAVE_SHARP_V41)
    {
        struct rkisp35_stat_buffer* stats = stats_buffer;
        if (stats->meas_type & ISP35_STAT_SHARP) {
            uint8_t min_idx = 0;
            for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
                if (pBtnrInfo->mSharpStats[min_idx].id > pBtnrInfo->mSharpStats[i].id)
                    min_idx = i;
            }
            sharp_stats_t *sharp_stats = &pBtnrInfo->mSharpStats[min_idx];
            sharp_stats->id = stats->frame_id;
            for (uint8_t i = 0; i < 17; i++) {
                sharp_stats->noise_curve[i] = stats->stat.sharp.noise_curve[i];
            }
        }
    }
#endif
    //printf("mSharpStatsId [%d %d %d]\n", pBtnrInfo->mSharpStats[0].id, pBtnrInfo->mSharpStats[1].id, pBtnrInfo->mSharpStats[2].id);
    //printf("mBtnrStatsId [%d %d %d]\n", pBtnrInfo->mBtnrStats[0].id, pBtnrInfo->mBtnrStats[1].id, pBtnrInfo->mBtnrStats[2].id);
}

btnr_stats_t *bayertnr_get_stats(btnr_cvt_info_t *pBtnrInfo, uint32_t frameId)
{
    uint8_t idx = 0;

    for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
        if (pBtnrInfo->mBtnrStats[i].id == frameId - pBtnrInfo->stats_delay_cnt)
            idx = i;
    }
    return &pBtnrInfo->mBtnrStats[idx];
}

btnr_stats_t *bayertnr2_get_stats(btnr_cvt_info_t *pBtnrInfo, uint32_t frameId)
{
    uint8_t idx = 0;

    for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
        if (pBtnrInfo->mBtnr2Stats[i].id == frameId - pBtnrInfo->stats_delay_cnt)
            idx = i;
    }
    return &pBtnrInfo->mBtnr2Stats[idx];
}

#if defined(RKAIQ_HAVE_SHARP_V40) || defined(RKAIQ_HAVE_SHARP_V41)
sharp_stats_t *sharp_get_stats(btnr_cvt_info_t *pBtnrInfo, uint32_t frameId)
{
    uint8_t idx = 0;

    for (uint8_t i = 0; i < pBtnrInfo->stats_buffer_cnt; i++) {
        if (pBtnrInfo->mSharpStats[i].id == frameId - pBtnrInfo->stats_delay_cnt)
            idx = i;
    }
    return &pBtnrInfo->mSharpStats[idx];
}
#endif
int bayertnr_kalm_bitcut(int datain, int bitsrc, int bitdst)
{
    int out;
    out = bitsrc == bitdst ? datain : ((datain + (1 << (bitsrc - bitdst - 1))) >> (bitsrc - bitdst));
    return out;
}

int bayertnr_wgt_sqrt_tab(int index)
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


int bayertnr_update_sq(btnr_trans_params_t *pTransPrarms)
{
    int tmp, tmp0, tmp1;

    /* BAY3D_TNRSIGORG */
    // noise balance update info
    int pre_wk_stat, pre_pk_stat, wk_stat;
    int sigorg, sigsta, wsta;
    long long pkp1_sq, tmpL0, tmpL1;
    sigorg = 256;
    if(pTransPrarms->isFirstFrame)
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
        wk_stat = CLIP(wk_stat, pTransPrarms->bayertnr_lo_wgt_clip_min_limit, pTransPrarms->bayertnr_lo_wgt_clip_max_limit);

        tmp0 = bayertnr_kalm_bitcut(wk_stat, FIXTNRWWW, FIXTNRKAL);
        tmp1  = bayertnr_wgt_sqrt_tab((1 << FIXTNRKAL) - tmp0);
        tmp1  = (tmp1 * sigorg) >> FIXTNRKAL;
        tmp1  = CLIP(tmp1, 1, (1 << FIXTNRKAL));

        pTransPrarms->bayertnr_wgt_stat = wk_stat;
        pTransPrarms->bayertnr_pk_stat  = tmp1;
    }

    sigorg = 256;
    sigsta = pre_pk_stat;
    wsta   = bayertnr_kalm_bitcut(pTransPrarms->bayertnr_wgt_stat, FIXTNRWWW, FIXTNRWGT);
    pkp1_sq  = (long long)(sigsta * sigsta);
    tmpL0 = (long long)((wsta * wsta) / (1 << FIXTNRWGT));
    tmpL0 = (long long)(tmpL0 * pkp1_sq);
    tmpL1 = (long long)((((1 << FIXTNRWGT) - wsta) * ((1 << FIXTNRWGT) - wsta)) / (1 << FIXTNRWGT));
    tmp = tmpL0 + tmpL1 * sigorg * sigorg;
    tmp =  CLIP(tmp, 0, 0x3ffffff);

    //printf("%s:, baytnr_out_sigma_sq 0x%x\n", __func__, tmp);
    return tmp;

}
void bay_gauss5x5_filter_coeff(float sigma, int halftaby, int halftabx, int strdtabx, int *gstab, int coefsum)
{
    int halfx = halftabx;
    int halfy = halftaby;
    int strdx = strdtabx;
    int stridex = strdx / 2;
    int centerx = halfx / 2;
    int centery = halfy / 2;
    int gausstab[5 * 5];
    int i, j, sumc;
    float tmpf0, tmpf1;
    float tmpf2, gausstabf[5 * 5];
    int gstabidx[5 * 5] =
    {   5, 4, 3, 4, 5,
        4, 2, 1, 2, 4,
        3, 1, 0, 1, 3,
        4, 2, 1, 2, 4,
        5, 4, 3, 4, 5,
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
            gausstab[i * halfx + j] = (int)(gausstabf[i * halfx + j] / tmpf2 * coefsum);
            sumc = sumc + gausstab[i * halfx + j];
        }
    }
    gausstab[halfy / 2 * halfx + halfx / 2] += (coefsum - sumc);

    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gstab[gstabidx[i * halfx + j]] = gausstab[i * halfx + j];
        }
    }
}

int bayertnr_autosigma_config(btnr_stats_t *pStats, btnr_trans_params_t *pTransPrarms, blc_res_cvt_t* pBlc)
{
    // update auto sigma curve
    int sigma_bins = 20;
    uint16_t tmp = 0;
    uint16_t *sigmay_data = pStats->sigma_y;
    uint16_t *sigmay_curve = pTransPrarms->tnr_luma_sigma_y;
    int iir_wgt  = pTransPrarms->bayertnr_auto_sig_count_filt_wgt;
    int filt_coef[5] = {0, 0, 1, 0, 0};
    int sigmay_calc[24 + 2 + 2], sigmay_tmp[24];
    int i = 0, j = 0, tmp0 = 0, tmp1 = 0, coefacc = 0;

    if (pTransPrarms->bayertnr_auto_sig_count_en == 0)
        return 0;

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
            if(i >= sigma_bins) {
                pTransPrarms->bayertnr_auto_sig_count_valid = 0;
                break;
            }
        }
    }

    // sigma iir
    if(pTransPrarms->isTransfBypass) {
        if(pBlc->obcPostTnr.sw_blcT_obcPostTnr_en && pBlc->obcPostTnr.sw_blcT_autoOB_offset) {
            for (j = 2; j < sigma_bins; j++)
            {
                sigmay_tmp[j] = MAX(sigmay_tmp[j], sigmay_tmp[j - 1]);
            }
        } else {
            for (j = 1; j < sigma_bins; j++)
            {
                sigmay_tmp[j] = MAX(sigmay_tmp[j], sigmay_tmp[j - 1]);
            }
        }
    }

    if(pStats->sigma_num < pTransPrarms->bayertnr_auto_sig_count_max)
    {
        pTransPrarms->bayertnr_auto_sig_count_valid = 0;
    }
    else
        pTransPrarms->bayertnr_auto_sig_count_valid = 1;

    if(pTransPrarms->bayertnr_auto_sig_count_valid == 0) {
        iir_wgt = 1024;
    }

    LOGD_ANR("btnr num:%d num_thred:%d auto_valid:%d,iir_wgt:%d\n",
             pStats->sigma_num,
             pTransPrarms->bayertnr_auto_sig_count_max,
             pTransPrarms->bayertnr_auto_sig_count_valid,
             iir_wgt);

    for (j = 0; j < sigma_bins; j++)
    {
        tmp = (iir_wgt * sigmay_curve[j] + (1024 - iir_wgt) * sigmay_tmp[j]) >> 10;
        sigmay_curve[j] = tmp;
    }

#if 0
    printf("btnr sigmay_tmp[%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           sigmay_tmp[0],
           sigmay_tmp[1],
           sigmay_tmp[2],
           sigmay_tmp[3],
           sigmay_tmp[4],
           sigmay_tmp[5],
           sigmay_tmp[6],
           sigmay_tmp[7],
           sigmay_tmp[8],
           sigmay_tmp[9],
           sigmay_tmp[10],
           sigmay_tmp[11],
           sigmay_tmp[12],
           sigmay_tmp[13],
           sigmay_tmp[14],
           sigmay_tmp[15],
           sigmay_tmp[16],
           sigmay_tmp[17],
           sigmay_tmp[18],
           sigmay_tmp[19]);

    printf("btnr sigmay_curve iir finnal [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           sigmay_curve[0],
           sigmay_curve[1],
           sigmay_curve[2],
           sigmay_curve[3],
           sigmay_curve[4],
           sigmay_curve[5],
           sigmay_curve[6],
           sigmay_curve[7],
           sigmay_curve[8],
           sigmay_curve[9],
           sigmay_curve[10],
           sigmay_curve[11],
           sigmay_curve[12],
           sigmay_curve[13],
           sigmay_curve[14],
           sigmay_curve[15],
           sigmay_curve[16],
           sigmay_curve[17],
           sigmay_curve[18],
           sigmay_curve[19]);
#endif

    return 0;
}

int bayertnr_tnr_noise_curve(int data, int isHdrShort, btnr_trans_params_t *pTransPrarms)
{
    int sigbins = 20;
    int dbl_en  = pTransPrarms->bayertnr_tnr_sigma_curve_double_en;
    int dbl_pos = pTransPrarms->bayertnr_tnr_sigma_curve_double_pos;
    int i, sigma;
    int ratio;

    for(i = 0; i < sigbins; i++)
    {
        if(data < pTransPrarms->tnr_luma_sigma_x[i])
            break;
    }

    if(isHdrShort && dbl_en && i <= dbl_pos)
        sigma = pTransPrarms->tnr_luma_sigma_y[dbl_pos];
    else if(dbl_en && i == dbl_pos)
        sigma = pTransPrarms->tnr_luma_sigma_y[dbl_pos];
    else if(i <= 0)
        sigma = pTransPrarms->tnr_luma_sigma_y[0];
    else if(i > (sigbins - 1))
        sigma = pTransPrarms->tnr_luma_sigma_y[sigbins - 1];
    else
    {
        ratio = (data - pTransPrarms->tnr_luma_sigma_x[i - 1]) * (pTransPrarms->tnr_luma_sigma_y[i] - pTransPrarms->tnr_luma_sigma_y[i - 1]);
        ratio = ratio / (pTransPrarms->tnr_luma_sigma_x[i] - pTransPrarms->tnr_luma_sigma_x[i - 1]);

        sigma = pTransPrarms->tnr_luma_sigma_y[i - 1] + ratio;
    }

    return sigma;
}

void bayertnr_sigmaY_interpolate(u16 *pSigX, u16 *pSigY, u16 *pLowX, u16 *pHighX, u16 *pLowY, u16 *pHighY,  int sigbins, float iso_ratio)
{
    int i, j, tmp;
    float x_ratio = 0.0;
    u16 y_lowISO = 0, y_highISO = 0;

    for(i = 0; i < sigbins; i++) {
        tmp = pSigX[i];
        for(j = 0; j < sigbins - 1; j++) {
            if(tmp >= pLowX[j] &&  tmp <= pLowX[j + 1]) {
                x_ratio = (float)(tmp - pLowX[j]) / (pLowX[j + 1] - pLowX[j]);
                break;
            }
        }
        if(j == sigbins - 1) {
            if(tmp < pLowX[0]) {
                j = 0;
                x_ratio = 0;
            }
            if(tmp > pLowX[sigbins - 1] ) {
                j = sigbins - 2;
                x_ratio = 1;
            }
        }
        y_lowISO = x_ratio * (pLowY[j + 1] - pLowY[j]) + pLowY[j];
#if 0
        printf("tnr low: x:%u xlow:%u xHigh:%u xratio:%f ylow:%u yhigh:%u y:%u\n",
               tmp, pLowX[j], pLowX[j + 1], x_ratio, pLowY[j], pLowY[j + 1], y_lowISO);
#endif

        //interpolate y in high iso
        for(j = 0; j < sigbins - 1; j++) {
            if(tmp >= pHighX[j] &&  tmp <= pHighX[j + 1]) {
                x_ratio = (float)(tmp - pHighX[j]) / (pHighX[j + 1] - pHighX[j]);
                break;
            }
        }
        if(j == sigbins - 1) {
            if(tmp < pHighX[0]) {
                j = 0;
                x_ratio = 0;
            }
            if(tmp > pHighX[sigbins - 1] ) {
                j = sigbins - 2;
                x_ratio = 1;
            }
        }
        y_highISO = x_ratio * (pHighY[j + 1] - pHighY[j]) + pHighY[j];

        // intepolate y between two iso
        pSigY[i]  = iso_ratio * (y_highISO - y_lowISO) + y_lowISO;
#if 0
        printf("tnr high: x:%u xlow:%u xHigh:%u xratio:%f ylow:%u yhigh:%u y:%u  finnalY:%u\n",
               tmp, pHighX[j], pHighX[j + 1], x_ratio, pHighY[j], pHighY[j + 1], y_highISO, pSigY[i]);
#endif
    }
}

uint16_t btnr_hdr_sigmaY_interpolate(int xData, uint16_t* sigcur_idx, uint16_t*sigcur_val, int sigbins)
{
    int i;
    uint16_t sigma;
    int ratio;

    for(i = 0; i < sigbins; i++) {
        if(xData < sigcur_idx[i])
            break;
    }

    if(i <= 0) {
        sigma = sigcur_val[0];
    } else if(i >= sigbins - 1) {
        sigma = sigcur_val[sigbins - 1];
    } else {
        ratio = (xData - sigcur_idx[i - 1]) * (sigcur_val[i] - sigcur_val[i - 1]);
        ratio = ratio / (sigcur_idx[i] - sigcur_idx[i - 1]);

        sigma = sigcur_val[i - 1] + ratio;
    }

    return sigma;
}

float btnr_hdr_mergeWgt_interpolate(int xData, int* luma_idx, float*luma2wgt_val)
{
    int lumabins = 17;
    int i;
    float wgt;
    int ratio;

    for(i = 0; i < lumabins; i++) {
        if(xData < luma_idx[i])
            break;
    }

    if(i <= 0) {
        wgt = luma2wgt_val[0];
    } else if(i >= lumabins - 1) {
        wgt = luma2wgt_val[lumabins - 1];
    } else {
        ratio = (xData - luma_idx[i - 1]) * (luma2wgt_val[i] - luma2wgt_val[i - 1]);
        ratio = ratio / (luma_idx[i] - luma_idx[i - 1]);

        wgt = luma2wgt_val[i - 1] + ratio;
    }

    return wgt;
}


void btnr_hdr_sigma_calc(uint16_t* pSigmaX, uint16_t* pSigmaY, uint16_t* pCalibX, uint16_t*pShortY, uint16_t* pLongY, mergeLuma2Wgt_t* pMerge_luma2wgt, btnr_trans_params_t *pTransParams, int sigbins, float hdr_ratio)
{
    float short_iso_ratio = 0.0f, merge_luma2Wgt = 0.0f;
    int i, tmp0, tmp1, tmp2, tmp3, short_edge_idx = 0;
    int long_pixLuma, short_pixLuma;
    int pixmax_long =  bayertnr_logtrans(((1 << 12) - 1), pTransParams);
	int* pMerge_luma_idx = NULL;
	float* pMerge_luma_wgt = NULL;

	#if MGE_WGT_USE_OLD_STRUCT
		pMerge_luma_idx = pMerge_luma2wgt->luma_idx;
		pMerge_luma_wgt = pMerge_luma2wgt->luma_wgt;
	#else
		pMerge_luma_idx = pMerge_luma2wgt->wgtCurve[0].idx;
		pMerge_luma_wgt = pMerge_luma2wgt->wgtCurve[0].val;
	#endif

    for(i = 0; i < sigbins; i++) {
        if(pSigmaX[i] <= pixmax_long) {
            short_edge_idx = i;
        }
    }

    for(i = 0; i < short_edge_idx; i++) {
        tmp1 = 0;
		tmp2 = 0;
		tmp3 = 0;
        merge_luma2Wgt = 0.0f;
        tmp0 = btnr_hdr_sigmaY_interpolate(pSigmaX[i], pCalibX, pLongY, sigbins);
        long_pixLuma = bayertnr_logiitrans(pSigmaX[i], pTransParams);
        if(long_pixLuma >= pMerge_luma_idx[0] * 4) {
            short_pixLuma = (float)long_pixLuma / hdr_ratio;
            short_pixLuma = bayertnr_logtrans(short_pixLuma, pTransParams);
            tmp1 = btnr_hdr_sigmaY_interpolate(MAX(short_pixLuma, 0), pCalibX, pShortY, sigbins);
            tmp2 = tmp1 + bayertnr_logtrans(hdr_ratio, pTransParams);
            tmp3 = tmp1 * sqrt(hdr_ratio);
			tmp1 = 0.7*tmp2 + 0.3*tmp3;
            merge_luma2Wgt = btnr_hdr_mergeWgt_interpolate(long_pixLuma / 4, pMerge_luma_idx, pMerge_luma_wgt);
            long_pixLuma = bayertnr_logiitrans(pSigmaX[i + 1], pTransParams);
            merge_luma2Wgt = (merge_luma2Wgt + btnr_hdr_mergeWgt_interpolate(long_pixLuma / 4, pMerge_luma_idx, pMerge_luma_wgt)) / 2;
        }

        pSigmaY[i] = tmp0 * (1.0f - merge_luma2Wgt) + merge_luma2Wgt * tmp1;
        //printf("long: [%d]: x:%d tmp0:%d  short: long_pixLuma:%d merge_luma2Wgt:%f tmp1:%d finnal:%hu\n", i, pSigmaX[i], tmp0, long_pixLuma, merge_luma2Wgt, tmp1, pSigmaY[i]);
    }

    for(i = short_edge_idx; i < sigbins; i++) {
        tmp1 = 0;
		tmp2 = 0;
		tmp3 = 0;
        merge_luma2Wgt = 1.0;
        long_pixLuma = bayertnr_logiitrans(pSigmaX[i], pTransParams);
        short_pixLuma = (float)long_pixLuma / hdr_ratio;
        short_pixLuma = bayertnr_logtrans(short_pixLuma, pTransParams);
        tmp0 = btnr_hdr_sigmaY_interpolate(MAX(short_pixLuma, 0), pCalibX, pShortY, sigbins);
        tmp2 = tmp1 + bayertnr_logtrans(hdr_ratio, pTransParams);
        tmp3 = tmp1 * sqrt(hdr_ratio);
		tmp1 = 0.7*tmp2 + 0.3*tmp3;
        if( long_pixLuma <= pMerge_luma_idx[16] * 4) {
            tmp1 = btnr_hdr_sigmaY_interpolate(pSigmaX[i], pCalibX, pLongY, sigbins);
            merge_luma2Wgt = btnr_hdr_mergeWgt_interpolate(long_pixLuma / 4, pMerge_luma_idx, pMerge_luma_wgt);
            long_pixLuma = bayertnr_logiitrans(pSigmaX[i + 1], pTransParams);
            merge_luma2Wgt = (merge_luma2Wgt + btnr_hdr_mergeWgt_interpolate(long_pixLuma / 4, pMerge_luma_idx, pMerge_luma_wgt)) / 2;
        }
        pSigmaY[i] = tmp0 * merge_luma2Wgt + (1.0 - merge_luma2Wgt) * tmp1;
        //printf("iitrans pSigmaX:%d short luma:%d %f tmp0:%d, merge_luma2Wgt:%f tmp1:%d finnal:%hu \n", pSigmaX[i], long_pixLuma, long_pixLuma/fdGain[0], tmp0, merge_luma2Wgt, tmp1, pSigmaY[i]);
    }

}


void btnr_sensor_hdr_sigma_calc(uint16_t* pSigmaX, uint16_t* pSigmaY, common_cvt_info_t* cvtinfo, btnr_cvt_info_t* pBtnrInfo, mergeLuma2Wgt_t* pMergeLumaWgt, int sigbins, btnr_trans_params_t *pTransParams)
{
    int frame_num = cvtinfo->frameNum;
    int i = 0, j = 0;
    int isSensorHdr = RK_AIQ_HDR_IS_SENSOR_BUILTIN(pBtnrInfo->_working_mode);
    int iso[frame_num];
    float exp_ratio[frame_num];
    uint16_t **PPIsoCalibSigmaY;
    uint16_t **PPIsoCalibSigmaX;
    float iso_ratio;
    int iso_ilow = 0, iso_ihigh = 0;
    CalibDb_Cis_Tnr_t *btnr_cis_info = (CalibDb_Cis_Tnr_t *)pBtnrInfo->btnr_cis_info;
    int sigma_x_linear[frame_num];
    int sigma_x_log[frame_num];
    int sigma_y[frame_num];
    int sigma_y_tmp[frame_num];
    float weight[frame_num - 1];
    static int hdr_cnt = 0;
    int blc = 0;
	
    if(cvtinfo->blc_res.en) {
        blc = (cvtinfo->blc_res.obcPreTnr.hw_blcC_obGr_val + cvtinfo->blc_res.obcPreTnr.hw_blcC_obGb_val) / 2;
        if((hdr_cnt % 300) == 0) {
            printf("blc:%d\n", blc);
        }
    }

	if((hdr_cnt % 300) == 0) {
		for(i=0; i<frame_num -1; i++){
			printf("frame idx %d : mergeLumaIdx: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
				i,
				pMergeLumaWgt->wgtCurve[i].idx[0],
				pMergeLumaWgt->wgtCurve[i].idx[1],
				pMergeLumaWgt->wgtCurve[i].idx[2],
				pMergeLumaWgt->wgtCurve[i].idx[3],
				pMergeLumaWgt->wgtCurve[i].idx[4],
				pMergeLumaWgt->wgtCurve[i].idx[5],
				pMergeLumaWgt->wgtCurve[i].idx[6],
				pMergeLumaWgt->wgtCurve[i].idx[7],
				pMergeLumaWgt->wgtCurve[i].idx[8],
				pMergeLumaWgt->wgtCurve[i].idx[9],
				pMergeLumaWgt->wgtCurve[i].idx[10],
				pMergeLumaWgt->wgtCurve[i].idx[11],
				pMergeLumaWgt->wgtCurve[i].idx[12],
				pMergeLumaWgt->wgtCurve[i].idx[13],
				pMergeLumaWgt->wgtCurve[i].idx[14],
				pMergeLumaWgt->wgtCurve[i].idx[15],
				pMergeLumaWgt->wgtCurve[i].idx[16]);
			
        	printf("frame idx %d : mergeWgt: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
				i,
				pMergeLumaWgt->wgtCurve[i].val[0],
				pMergeLumaWgt->wgtCurve[i].val[1],
				pMergeLumaWgt->wgtCurve[i].val[2],
				pMergeLumaWgt->wgtCurve[i].val[3],
				pMergeLumaWgt->wgtCurve[i].val[4],
				pMergeLumaWgt->wgtCurve[i].val[5],
				pMergeLumaWgt->wgtCurve[i].val[6],
				pMergeLumaWgt->wgtCurve[i].val[7],
				pMergeLumaWgt->wgtCurve[i].val[8],
				pMergeLumaWgt->wgtCurve[i].val[9],
				pMergeLumaWgt->wgtCurve[i].val[10],
				pMergeLumaWgt->wgtCurve[i].val[11],
				pMergeLumaWgt->wgtCurve[i].val[12],
				pMergeLumaWgt->wgtCurve[i].val[13],
				pMergeLumaWgt->wgtCurve[i].val[14],
				pMergeLumaWgt->wgtCurve[i].val[15],
				pMergeLumaWgt->wgtCurve[i].val[16]);
		}
    }

    PPIsoCalibSigmaX = (uint16_t **)malloc(frame_num * sizeof(uint16_t *));
    PPIsoCalibSigmaY = (uint16_t **)malloc(frame_num * sizeof(uint16_t *));
    for(i = 0; i < frame_num; i++) {
        PPIsoCalibSigmaX[i] = (uint16_t *)malloc(sigbins * sizeof(uint16_t));
        memset(PPIsoCalibSigmaX[i], 0x00, sigbins * sizeof(uint16_t));
        PPIsoCalibSigmaY[i] = (uint16_t *)malloc(sigbins * sizeof(uint16_t));
        memset(PPIsoCalibSigmaY[i], 0x00, sigbins * sizeof(uint16_t));
    }

    for(i = 0; i < frame_num; i++) {
        if(isSensorHdr) {
            if(i == frame_num - 1) {
                exp_ratio[i] = 1;
            } else {
                exp_ratio[i] = cvtinfo->ae_exp->HdrExp[frame_num - 1].exp_real_params.cis_total_gain * cvtinfo->ae_exp->HdrExp[frame_num - 1].exp_real_params.integration_time
                               / (cvtinfo->ae_exp->HdrExp[i].exp_real_params.cis_total_gain * cvtinfo->ae_exp->HdrExp[i].exp_real_params.integration_time);
            }
            iso[i] = cvtinfo->frameIso[i];
        }

        if((hdr_cnt % 300) == 0) {
            printf("frame_idx:%d iso:%d time:%f again:%f cis_gain:%f, conversionGain:%f exp_ratio:%f\n",
                   i, iso[i],
                   cvtinfo->ae_exp->HdrExp[i].exp_real_params.integration_time,
                   cvtinfo->ae_exp->HdrExp[i].exp_real_params.analog_gain,
                   cvtinfo->ae_exp->HdrExp[i].exp_real_params.cis_total_gain,
                   cvtinfo->ae_exp->HdrExp[i].exp_real_params.cis_total_gain / cvtinfo->ae_exp->HdrExp[i].exp_real_params.analog_gain,
                   exp_ratio[i]);
        }

        iso_ilow = 0;
        iso_ihigh = 0;
        iso_ratio = 0.0;
        pre_interp(iso[i], NULL, 0, &iso_ilow, &iso_ihigh, &iso_ratio);

        //printf("iso:%d iso_low:%d iso_high:%d iso_ratio:%f\n", iso[i], iso_ilow, iso_ihigh, iso_ratio);
        for(j = 0; j < sigbins; j++) {
            uint16_t sigmaLowY =  btnr_cis_info->sigmaCurve_dyn[iso_ilow].sigmaCurve[i].val[j];
            uint16_t sigmaHighY =  btnr_cis_info->sigmaCurve_dyn[iso_ihigh].sigmaCurve[i].val[j];
            PPIsoCalibSigmaX[i][j] =  btnr_cis_info->sigmaCurve_dyn[iso_ilow].sigmaCurve[i].idx[j];
            PPIsoCalibSigmaY[i][j] = ( sigmaLowY + iso_ratio * (sigmaHighY - sigmaLowY));
            //printf("frame_idx:%d sigmaY[%d]=%d \n", i, j, PPIsoCalibSigmaY[i][j]);
        }

        if((hdr_cnt % 300) == 0) {
            printf("frame_idx:%d bins:%d sigmaY=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                   i, sigbins,
                   PPIsoCalibSigmaY[i][0],
                   PPIsoCalibSigmaY[i][1],
                   PPIsoCalibSigmaY[i][2],
                   PPIsoCalibSigmaY[i][3],
                   PPIsoCalibSigmaY[i][4],
                   PPIsoCalibSigmaY[i][5],
                   PPIsoCalibSigmaY[i][6],
                   PPIsoCalibSigmaY[i][7],
                   PPIsoCalibSigmaY[i][8],
                   PPIsoCalibSigmaY[i][9],
                   PPIsoCalibSigmaY[i][10],
                   PPIsoCalibSigmaY[i][11],
                   PPIsoCalibSigmaY[i][12],
                   PPIsoCalibSigmaY[i][13],
                   PPIsoCalibSigmaY[i][14],
                   PPIsoCalibSigmaY[i][15],
                   PPIsoCalibSigmaY[i][16],
                   PPIsoCalibSigmaY[i][17]);
        }

    }


    for(j = 0; j < sigbins; j++) {
        sigma_x_log[frame_num - 1] = pSigmaX[j];
        sigma_x_linear[frame_num - 1] = bayertnr_logiitrans(sigma_x_log[frame_num - 1], pTransParams);
        sigma_y[frame_num - 1] = btnr_hdr_sigmaY_interpolate(sigma_x_log[frame_num - 1], PPIsoCalibSigmaX[frame_num - 1], PPIsoCalibSigmaY[frame_num - 1], sigbins);
        sigma_y_tmp[frame_num - 1] = sigma_y[frame_num - 1];
        for(i = 0; i < frame_num - 1; i++) {
            sigma_x_linear[i] =  sigma_x_linear[frame_num - 1] / exp_ratio[i];
            sigma_x_log[i] = bayertnr_logtrans(sigma_x_linear[i], pTransParams);
            sigma_y[i] = btnr_hdr_sigmaY_interpolate(MAX(sigma_x_log[i], 0), PPIsoCalibSigmaX[i], PPIsoCalibSigmaY[i], sigbins);

        }

        for(i = 0; i < frame_num - 1; i++) {
			#if MGE_WGT_USE_OLD_STRUCT
			weight[i] = btnr_hdr_mergeWgt_interpolate(MAX((sigma_x_linear[i + 1]  -  blc / 2), 0), pMergeLumaWgt[i].luma_idx, pMergeLumaWgt[i].luma_wgt);
            if( 0 ) {
                int pre_x = bayertnr_logiitrans(pSigmaX[j - 1], pTransParams) / exp_ratio[i];
                weight[i] = (weight[i] + btnr_hdr_mergeWgt_interpolate(MAX((pre_x), 0), pMergeLumaWgt[i].luma_idx, pMergeLumaWgt[i].luma_wgt)) / 2;
            }
			#else
            weight[i] = btnr_hdr_mergeWgt_interpolate(MAX((sigma_x_linear[i + 1]  -  blc / 2), 0), pMergeLumaWgt->wgtCurve[i].idx, pMergeLumaWgt->wgtCurve[i].val);
            if( 0 ) {
                int pre_x = bayertnr_logiitrans(pSigmaX[j - 1], pTransParams) / exp_ratio[i];
                weight[i] = (weight[i] + btnr_hdr_mergeWgt_interpolate(MAX((pre_x), 0), pMergeLumaWgt->wgtCurve[i].idx, pMergeLumaWgt->wgtCurve[i].val)) / 2;
            }
			#endif
            if(sigma_x_linear[i] >= blc / 2 || weight[i] >= 0.3) {
                int tmp_y1 = sigma_y[i] * sqrt(exp_ratio[i]);
                int tmp_y2 = sigma_y[i] + bayertnr_logtrans(exp_ratio[i], pTransParams);
                sigma_y[i] = (0.3 * tmp_y1 + 0.7 * tmp_y2);
            }
            sigma_y_tmp[i] = sigma_y[i];
            sigma_y[i + 1] = (sigma_y[i]) * weight[i] + (1.0 - weight[i]) * (sigma_y[i + 1]);
        }

        pSigmaY[j] = sigma_y[frame_num - 1];


        if((hdr_cnt % 300) == 0) {
            printf("linear x:%d %d %d Log x:%d %d %d  log Y:%d %d %d weight:%f %f finnal:%d\n",
                   sigma_x_linear[2], sigma_x_linear[1], sigma_x_linear[0],
                   sigma_x_log[2], sigma_x_log[1], sigma_x_log[0],
                   sigma_y_tmp[2], sigma_y_tmp[1], sigma_y_tmp[0],
                   weight[1], weight[0], pSigmaY[j]);
        }
    }



    for(j = 1; j < sigbins - 1; j++) {
        pSigmaY[j] = ( 0.2 * pSigmaY[j - 1] + 0.6 * pSigmaY[j] + 0.2 * pSigmaY[j + 1]);
    }

    if((hdr_cnt % 300) == 0) {
        printf("finnal sigmaY=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
               pSigmaY[0],
               pSigmaY[1],
               pSigmaY[2],
               pSigmaY[3],
               pSigmaY[4],
               pSigmaY[5],
               pSigmaY[6],
               pSigmaY[7],
               pSigmaY[8],
               pSigmaY[9],
               pSigmaY[10],
               pSigmaY[11],
               pSigmaY[12],
               pSigmaY[13],
               pSigmaY[14],
               pSigmaY[15],
               pSigmaY[16],
               pSigmaY[17]);
    }

    for(i = 0; i < frame_num; i++) {
        free(PPIsoCalibSigmaX[i]);
        free(PPIsoCalibSigmaY[i]);
    }
    free(PPIsoCalibSigmaX);
    free(PPIsoCalibSigmaY);
    hdr_cnt++;
}

