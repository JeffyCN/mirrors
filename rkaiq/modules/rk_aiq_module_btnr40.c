#include "rk_aiq_isp39_modules.h"
#include "interpolation.h"

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

void bayertnr_luma2sigmax_config_v30(btnr_trans_params_t *pTransParams, blc_res_cvt_t *pBlc, float preDgain)
{
    int pix_max;
    int i, tmp, tmp0, tmp1;
    int sigbins = 20;

    bool transf_bypass_en = pTransParams->isTransfBypass;

    pix_max = transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans((1 << 12) - 1, pTransParams);
    pix_max = pTransParams->isHdrMode ? bayertnr_logtrans((1 << 20) - 1, pTransParams) : pix_max;
    if(pTransParams->isHdrMode) {
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 1;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 10;
        // hdr long bins
        int lgbins = 10;
        for(i = 0; i < lgbins; i++) {
            tmp = 128 * (i + 1);
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pix_max);
        }
        pTransParams->tnr_luma_sigma_x[lgbins - 1] = pix_max;
        // hdr short bins
        for(i = lgbins; i < lgbins + 6; i++) {
            tmp = 128 * (i - lgbins + 1)  + pTransParams->tnr_luma_sigma_x[lgbins - 1];
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pix_max);
        }
        for(i = lgbins + 6; i < sigbins; i++) {
            tmp = 256 * (i - lgbins - 6 + 1)  + pTransParams->tnr_luma_sigma_x[lgbins + 6 - 1];
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pix_max);
        }
        pTransParams->tnr_luma_sigma_x[sigbins - 1] = pix_max;

    } else if(!transf_bypass_en)   {
#if 0
        int segs = 6;
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 0;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 0;
        for(i = 0; i < segs; i++) {
            pTransParams->tnr_luma_sigma_x[i] = 32 * (i + 1);
        }
        for(i = segs; i < sigbins; i++) {
            pTransParams->tnr_luma_sigma_x[i] = 64 * (i - segs + 1) + 32 * segs;
        }
        pTransParams->tnr_luma_sigma_x[sigbins - 1] = pix_max;
#else
        uint32_t blc_remain = 0;
        if(pBlc->obcPostTnr.sw_blcT_autoOB_offset && pBlc->obcPostTnr.sw_blcT_obcPostTnr_en) {
            blc_remain = pBlc->obcPostTnr.sw_blcT_autoOB_offset;
        }
        int log_pix_max = bayertnr_logtrans(((1 << 12) * preDgain - 1), pTransParams);
        int log_ob_offset = bayertnr_logtrans(blc_remain, pTransParams);
        pTransParams->tnr_luma_sigma_x[0] = (log_ob_offset + 128);
#if 0
        printf("tnr sigmax ob offset:%d, log_ob_offset:%d  log_pix_max:%d step:128 finnal x0:%d\n",
               blc_remain, log_ob_offset, log_pix_max,  pTransParams->tnr_luma_sigma_x[0]);
#endif
        int step = 0;
        for(i = 1; i < sigbins; i++) {
            step = (log_pix_max - pTransParams->tnr_luma_sigma_x[i - 1]) / (sigbins - i);
            if(i != sigbins - 1) {
                step = 1 << (ROUND_F(LOG2(step)));
            } else {
                step = 1 << (FLOOR(LOG2(step)));
            }
            tmp = pTransParams->tnr_luma_sigma_x[i - 1] + step;
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, log_pix_max);
#if 0
            printf(" tnr sigmax idx:%d step:%d stepfinnal:%d x[%d]=%d clip:%u\n",
                   i, (log_pix_max - pTransParams->tnr_luma_sigma_x[i - 1]) / (sigbins - i), step, i, tmp,  pTransParams->tnr_luma_sigma_x[i]);
#endif
        }
#endif
    }
    else
    {
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 0;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 0;
        for(i = 0; i < sigbins / 2; i++) {
            pTransParams->tnr_luma_sigma_x[i] = 128 * (i + 1);
            pTransParams->tnr_luma_sigma_x[i + sigbins / 2] = 256 * (i + 1) + 128 * 10;
        }
        pTransParams->tnr_luma_sigma_x[sigbins - 1] = pix_max;
    }

}

void rk_aiq_btnr40_params_logtrans(struct isp39_bay3d_cfg *pCfg)
{
    uint8_t is15bit = pCfg->transf_mode_scale;
    uint8_t offsetbit = bayertnr_find_top_one_pos(pCfg->transf_mode_offset);

#define LOGTRANSF_VAR(a) a = isp39_logtransf(a, is15bit, offsetbit)
    LOGTRANSF_VAR(pCfg->cur_spnr_sigma_offset);
    LOGTRANSF_VAR(pCfg->cur_spnr_sigma_hdr_sht_offset);
    LOGTRANSF_VAR(pCfg->cur_spnr_pix_diff_max_limit);
    LOGTRANSF_VAR(pCfg->cur_spnr_wgt_cal_offset);
    LOGTRANSF_VAR(pCfg->pre_spnr_sigma_offset);
    LOGTRANSF_VAR(pCfg->pre_spnr_sigma_hdr_sht_offset);
    LOGTRANSF_VAR(pCfg->pre_spnr_pix_diff_max_limit);
    LOGTRANSF_VAR(pCfg->pre_spnr_wgt_cal_offset);
#undef LOGTRANSF_VAR
}



void rk_aiq_btnr40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo, btnr_cvt_info_t* pBtnrInfo, mergeLuma2Wgt_t* pMergeLumaWgt)
{
    btnr_trans_params_t *pTransParams = &pBtnrInfo->mBtnrTransParams;
    btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[0];

    btnr_api_attrib_t *btnr_attrib = pBtnrInfo->btnr_attrib;
    btnr_param_auto_t *paut = &btnr_attrib->stAuto;
    rk_aiq_op_mode_t opMode = btnr_attrib->opMode;

    if (cvtinfo->isFirstFrame) {
        // save stats_buffer_cnt and stats_delay_cnt
        uint16_t bufCnt = pBtnrInfo->stats_buffer_cnt;
        uint16_t dlyCnt = pBtnrInfo->stats_delay_cnt;
        memset(pBtnrInfo, 0, sizeof(btnr_cvt_info_t));
        pBtnrInfo->stats_buffer_cnt = bufCnt;
        pBtnrInfo->stats_delay_cnt  = dlyCnt;
    } else {
        btnr_stats = bayertnr_get_stats(pBtnrInfo, cvtinfo->frameId);
        if (cvtinfo->frameId - pBtnrInfo->stats_delay_cnt != btnr_stats->id) {
            pBtnrInfo->btnr_stats_miss_cnt ++;
            if ((pBtnrInfo->btnr_stats_miss_cnt > 10) && (pBtnrInfo->btnr_stats_miss_cnt % 30 == 0)) {
                LOGE_ANR("Btnr stats miss match! frameId: %d stats [%d %d %d]", cvtinfo->frameId,
                         pBtnrInfo->mBtnrStats[0].id, pBtnrInfo->mBtnrStats[1].id, pBtnrInfo->mBtnrStats[2].id);
            }
        } else {
            pBtnrInfo->btnr_stats_miss_cnt = 0;
        }
    }

    int i, j;
    int tmp, tmp0, tmp1;
    int pix_max, sigbins = 20;
    struct isp39_bay3d_cfg *pFix = &isp_params->isp_cfg->others.bay3d_cfg;

    btnr_param_t *btnr_param = (btnr_param_t *) attr;
    btnr_params_static_t* psta = &btnr_param->sta;
    btnr_other_dyn_t * pdyn = &btnr_param->dyn;
    btnr_md_dyn_t* pmdDyn = &btnr_param->mdDyn;

    bool bayertnr_default_noise_curve_use = false;
    int bayertnr_iso_cur = cvtinfo->frameIso[cvtinfo->frameNum - 1];
    bayertnr_default_noise_curve_use = bayertnr_find_top_one_pos(bayertnr_iso_cur) != bayertnr_find_top_one_pos(pTransParams->bayertnr_iso_pre);
    pTransParams->bayertnr_iso_pre = bayertnr_iso_cur;
    pTransParams->isFirstFrame = cvtinfo->isFirstFrame;
    pTransParams->isHdrMode = cvtinfo->frameNum == 2;

    float frameiso[3];
    float frameEt[3];
    float fdGain[3];

    int framenum = cvtinfo->frameNum;
    frameiso[0] = cvtinfo->frameIso[0];
    frameiso[1] = cvtinfo->frameIso[1];
    frameiso[2] = cvtinfo->frameIso[2];

    frameEt[0] = cvtinfo->frameEt[0];
    frameEt[1] = cvtinfo->frameEt[1];
    frameEt[2] = cvtinfo->frameEt[2];
    for (i = 0; i < framenum; i++) {
        fdGain[i] = frameiso[i] * frameEt[i];
    }
    for (i = 0; i < framenum; i++) {
        fdGain[i] = fdGain[framenum - 1] / fdGain[i];
    }

    // BAY3D_CTRL0 0x2c00
    //pFix->bypass_en = bypass;
    pFix->iirsparse_en = cvtinfo->use_aiisp ? 1 : 0;


    if(psta->hw_btnrCfg_pixDomain_mode != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_pixDomain_mode
            || psta->transCfg.hw_btnr_trans_mode != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_mode
            || psta->transCfg.hw_btnr_trans_offset != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_offset) {
        LOGW_ANR("Btnr run in pixLog2Domain is %d, trans_mode is %d, trans_offset is %d.\n"
                 "But calib pixLog2Domain_mode is %d, trans_mode is %d, trans_offset is %d.\n",
                 psta->hw_btnrCfg_pixDomain_mode, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_pixDomain_mode,
                 psta->transCfg.hw_btnr_trans_mode, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_mode,
                 psta->transCfg.hw_btnr_trans_offset, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_offset);
    }

    if (cvtinfo->frameNum > 1) {
        if (psta->hw_btnrCfg_pixDomain_mode != btnr_pixLog2Domain_mode) {
            LOGW_ANR("Btnr must run in pixLog2Domain(ori mode is %d) when isp is HDR mode(framenum=%d), btnr_pixLog2Domain_mode is be forcibly set to hw_btnrCfg_pixDomain_mode in HWI\n"
                     "You can set by btnr.static.hw_btnrCfg_pixDomain_mode, but change hw_btnrCfg_pixDomain_mode in running time may cause abnormal image transitions\n",
                     psta->hw_btnrCfg_pixDomain_mode, cvtinfo->frameNum, psta->hw_btnrCfg_pixDomain_mode);
            psta->hw_btnrCfg_pixDomain_mode = btnr_pixLog2Domain_mode;
        }
        if (psta->transCfg.hw_btnr_trans_mode != btnr_pixInBw20b_mode) {
            LOGE_ANR("hw_btnrCfg_trans_mode == %s(0x%x) is error. When isp is in HDR mode, btnr must run in 'btnr_pixInBw20b_mode'. "
                     "The trans_mode will be forcibly set to 'btnr_pixInBw20b_mode' in HWI.",
                     trans_mode2str(psta->transCfg.hw_btnr_trans_mode), psta->transCfg.hw_btnr_trans_mode);
            psta->transCfg.hw_btnr_trans_mode = btnr_pixInBw20b_mode;
        }
    }

    if(cvtinfo->frameNum == 1 && cvtinfo->preDGain > 1.0) {
        if (psta->hw_btnrCfg_pixDomain_mode != btnr_pixLog2Domain_mode) {
            LOGW_ANR("Btnr must run in pixLog2Domain(ori mode is %d) when predgain > 1.0, btnr_pixLog2Domain_mode is be forcibly set to hw_btnrCfg_pixDomain_mode in HWI\n",
                     psta->hw_btnrCfg_pixDomain_mode);
            psta->hw_btnrCfg_pixDomain_mode = btnr_pixLog2Domain_mode;
        }
    }

    if (!cvtinfo->isFirstFrame) {
        if (pBtnrInfo->hw_btnrCfg_pixDomain_mode != (int)psta->hw_btnrCfg_pixDomain_mode) {
            LOGW_ANR("change hw_btnrCfg_pixDomain_mode in running time may cause abnormal image transitions");
        }
    }
    pBtnrInfo->hw_btnrCfg_pixDomain_mode = (int)psta->hw_btnrCfg_pixDomain_mode;

    //BAY3D_CTRL1 0x2c04
    switch (psta->hw_btnrCfg_pixDomain_mode) {
    case btnr_pixLog2Domain_mode :
        pFix->transf_bypass_en = 0;
        pFix->transf_mode  = 0;
        break;
    // case btnr_pixSqrtDomain_mode:
    //     pFix->transf_bypass_en = 0;
    //     pFix->transf_mode  = 1;
    //     break;
    case btnr_pixLinearDomain_mode:
        pFix->transf_bypass_en = 1;
        break;
    }

    if (cvtinfo->preDGain > 1.0 || cvtinfo->frameNum > 1) {
        if (pFix->transf_bypass_en) {
            LOGW_ANR("pFix->transf_bypass_en error !");
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
    pFix->lpf_hi_bypass_en = !pmdDyn->loAsRatio_hiByMskMd.hw_btnrT_hfLpf_en;
    pFix->lo_diff_vfilt_bypass_en = !pmdDyn->subLoMd0_diffCh.hw_btnrT_vFilt_en;
    pFix->lpf_lo_bypass_en = !pmdDyn->subLoMd1.hw_btnrT_lpf_en;
    pFix->lo_wgt_hfilt_en = pmdDyn->subLoMd1.hw_btnrT_hFilt_en;
    pFix->lo_diff_hfilt_en = pmdDyn->subLoMd0_diffCh.hw_btnrT_hFilt_en;
    pFix->sig_hfilt_en = pmdDyn->subLoMd0_sgmCh.hw_btnrT_hFilt_en;
    //pFix->gkalman_en = pSelect->hw_btnr_gKalman_en;
    pFix->spnr_pre_sigma_use_en = (pdyn->preFrmSpNr.hw_btnrT_sigma_mode == btnr_kalPkSgm_mode);

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

    switch (pmdDyn->loMd.hw_btnrT_loMd_mode) {
    case btnr_subLoMd01Mix_mode:
        pFix->lo_detection_mode = 0;
        break;
    case btnr_subLoMd0Only_mode:
        pFix->lo_detection_mode = 1;
        break;
    case btnr_subLoMd1Only_mode:
        pFix->lo_detection_mode = 2;
        break;
    }

    pFix->lomdwgt_dbg_en = 0;
    switch (pmdDyn->hw_btnrT_md_mode) {
    case btnr_loAsRatio_hiByMsk_mode:
        pFix->wgt_cal_mode = 0;
        break;
    case btnr_loAsBias_hiBySgm_mode:
        pFix->wgt_cal_mode = 1;
        break;
    case btnr_loOnly_dbg_mode:
        pFix->lomdwgt_dbg_en = 1;
        pFix->wgt_cal_mode = 1;
        break;
    case btnr_hiByMskOnly_dbg_mode:
        pFix->wgt_cal_mode = 0;
        pFix->lo_detection_mode = 3;
        break;
    case btnr_hiBySgmOnly_dbg_mode:
        pFix->wgt_cal_mode = 1;
        pFix->lo_detection_mode = 3;
        pFix->lomdwgt_dbg_en = 0;
        break;
    }

    pFix->noisebal_mode = pdyn->hw_btnrT_noiseBal_mode;

    if (cvtinfo->frameNum > 1) {
        if (pFix->cur_spnr_sigma_curve_double_en == 0 || pFix->pre_spnr_sigma_curve_double_en == 0) {
            if (cvtinfo->btnr_warning_count < 5) {
                LOGW_ANR("When isp is HDR mode, hw_btnrT_sigmaCurve_mode recommends using btnr_midSegmInterpOff_mode. "
                         "You can set by dyn.curFrmSpNr.hw_btnrT_sigmaCurve_mode\n");
            }
            else if (cvtinfo->btnr_warning_count % 300 == 0) {
                LOGW_ANR("When isp is HDR mode, hw_btnrT_sigmaCurve_mode recommends using btnr_midSegmInterpOff_mode. "
                         "You can set by dyn.curFrmSpNr.hw_btnrT_sigmaCurve_mode\n");
            }
            cvtinfo->btnr_warning_count++;
        }
    }

    // BAY3D_CTRL2 0x2c08
    pFix->mge_wgt_ds_mode = 0;
    pFix->kalman_wgt_ds_mode = 0;
    pFix->mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;
    pFix->sigma_calc_mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;

    // BAY3D_TRANS0 0x2c0c
    if (psta->transCfg.hw_btnr_trans_mode == btnr_pixInBw15b_mode)
        pFix->transf_mode_scale = 1;
    else
        pFix->transf_mode_scale = 0;

    tmp = psta->transCfg.hw_btnr_trans_offset;
    int n  = bayertnr_find_top_one_pos(tmp);
    if(pFix->transf_mode_scale == 1) {
        n = 8;
    } else {
        if(n < 8)
            n = 8;
        if(n > 12)
            n = 12;
    }
    tmp = (1 << n);
    pFix->transf_mode_offset = CLIP(tmp, 0, 0x1fff);



    if (pFix->transf_mode_scale == 1) {
        pFix->itransf_mode_offset = (1 << 9) * n;
    } else {
        pFix->itransf_mode_offset = (1 << 8) * n;
    }

    // BAY3D_TRANS1 0x2c10
    // mode scale只有0和1两种选择，对应log域的小数精度，
    // 0对应8bit小数，4bit整数， 4bit整数导致最大线性输入数据位宽为20bit->transData_maxlimit为2的20次方-1
    // 1对应9bit小数，3bit整数， 3bit整数限制最大线性输入数据位宽为15bit->transData_maxlimit为2的15次方-1
    if(pFix->transf_bypass_en == 1) {
        pFix->transf_data_max_limit = ((1 << 12) - 1);
    } else {
        if (pFix->transf_mode_scale == 1) {
            pFix->transf_data_max_limit = ((1 << 15) - 1);
        }
        else {
            pFix->transf_data_max_limit = ((1 << 20) - 1);
        }
    }


    if(pFix->transf_bypass_en) {
        pFix->tnr_pix_max = ((1 << 12) - 1);
    } else {
        if(pFix->transf_mode_scale) {
            pFix->tnr_pix_max = bayertnr_logtrans(((1 << 15) - 1), pTransParams);
        } else {
            pFix->tnr_pix_max = bayertnr_logtrans(((1 << 20) - 1), pTransParams);;
        }
    }

    // BAY3D_CURDGAIN 0x2c14
    tmp = pdyn->curFrmSpNr.hw_btnrT_sigmaHdrS_scale * (1 << FIXBITDGAIN);
    pFix->cur_spnr_sigma_hdr_sht_scale = CLIP(tmp, 0, 0xffff);
    tmp = pdyn->curFrmSpNr.hw_btnrT_sigma_scale * (1 << FIXBITDGAIN);
    pFix->cur_spnr_sigma_scale = CLIP(tmp, 0, 0xffff);

    // tnr sigma curve must calculate before spnr sigma
    if(cvtinfo->isFirstFrame || (pTransParams->transf_mode != pFix->transf_mode) ||
            (pTransParams->transf_mode_scale != pFix->transf_mode_scale)) {
        bayertnr_logtrans_init(pFix->transf_mode, pFix->transf_mode_scale, pTransParams);
    }
    pTransParams->transf_mode = pFix->transf_mode;
    pTransParams->transf_mode_scale = pFix->transf_mode_scale;
    pTransParams->transf_mode_offset = pFix->transf_mode_offset;
    pTransParams->transf_data_max_limit = pFix->transf_data_max_limit;
    pTransParams->itransf_mode_offset = pFix->itransf_mode_offset;

    pTransParams->isTransfBypass = pFix->transf_bypass_en;
    bayertnr_luma2sigmax_config_v30(pTransParams, &cvtinfo->blc_res, cvtinfo->preDGain);
    for(i = 0; i < sigbins; i++) {
        pFix->tnr_luma_sigma_x[i] = pTransParams->tnr_luma_sigma_x[i];
    }

    //printf("oyyf tnr sigmax[] :");
    if(pTransParams->isTransfBypass  || opMode == RK_AIQ_OP_MODE_MANUAL) {
        for(i = 0; i < sigbins; i++) {
            pFix->tnr_luma_sigma_x[i] = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.idx[i];
            //printf("%d ", pCfg->tnr_luma2sigma_x[i]);
        }
    }
    //printf("\n");

    //x_step must be 2^n
    int x_step = 0;
    for(i = 1; i < sigbins; i++) {
        x_step = pFix->tnr_luma_sigma_x[i] - pFix->tnr_luma_sigma_x[i - 1];
        x_step = 1 << (FLOOR(LOG2(x_step)));
        tmp = pFix->tnr_luma_sigma_x[i - 1] + x_step;
        pFix->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pFix->tnr_pix_max);
    }



    float kcoef0 = 1.0;
    float kcoef1 = 1.0;
    int max_sig = ((1 << 12) - 1);
    if(psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_manualSigma_mode || cvtinfo->isFirstFrame || bayertnr_default_noise_curve_use) {
        pTransParams->bayertnr_auto_sig_count_en = 0;
        if(pTransParams->isTransfBypass || opMode == RK_AIQ_OP_MODE_MANUAL) {
            for(i = 0; i < sigbins; i++) {
                pTransParams->tnr_luma_sigma_y[i] = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.val[i];
            }
        } else if(pTransParams->isHdrMode) {
            uint16_t shortY[sigbins];
            int short_iso = cvtinfo->frameIso[1];
            int short_ilow = 0, short_ihigh = 0;
            float short_iso_ratio = 0.0f;

            pre_interp(short_iso, NULL, 0, &short_ilow, &short_ihigh, &short_iso_ratio);

            uint16_t* pSigmaX = pFix->tnr_luma_sigma_x;
            uint16_t* pSigmaY = pTransParams->tnr_luma_sigma_y;
            uint16_t* pCalibX = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pLongY = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pShortLowY = btnr_attrib->stAuto.dyn[short_ilow].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pShortHighY = btnr_attrib->stAuto.dyn[short_ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.val;

            for(i = 0; i < sigbins; i++) {
                shortY[i] = pShortLowY[i] + short_iso_ratio * (pShortHighY[i] - pShortLowY[i]);
            }
            btnr_hdr_sigma_calc(pSigmaX, pSigmaY, pCalibX, shortY, pLongY, pMergeLumaWgt, pTransParams, sigbins, fdGain[0]);
        } else {
            int iso = cvtinfo->frameIso[0];
            int ilow = 0, ihigh = 0;
            float iso_ratio = 0.0f;
            float x_ratio = 0.0f;
            int y_lowISO = 0;
            int y_highISO = 0;
            pre_interp(iso, NULL, 0, &ilow, &ihigh, &iso_ratio);
            // interpolate y in low iso
            uint16_t* pLowY = btnr_attrib->stAuto.dyn[ilow].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pHighY = btnr_attrib->stAuto.dyn[ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pLowX = btnr_attrib->stAuto.dyn[ilow].sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pHighX = btnr_attrib->stAuto.dyn[ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pSigmaX = pFix->tnr_luma_sigma_x;
            uint16_t* pSigmaY = pTransParams->tnr_luma_sigma_y;
            bayertnr_sigmaY_interpolate(pSigmaX, pSigmaY, pLowX, pHighX, pLowY, pHighY, sigbins, iso_ratio);
        }
    } else {
        pTransParams->bayertnr_auto_sig_count_en = 1;
        pTransParams->bayertnr_auto_sig_count_filt_wgt = pdyn->sigmaEnv.sw_btnrT_autoSgmIIR_alpha * (1 << 10);
        pTransParams->bayertnr_auto_sig_count_max = cvtinfo->rawWidth * cvtinfo->rawHeight / 3;
        bayertnr_autosigma_config(btnr_stats, pTransParams, &cvtinfo->blc_res);
    }

    kcoef0 = 1.0;
    max_sig = ((1 << 12) - 1);
    for(i = 0; i < sigbins; i++) {
        pFix->tnr_luma_sigma_y[i] = CLIP((int)(pTransParams->tnr_luma_sigma_y[i] * kcoef0), 0, max_sig);
    }

    int spnrsigbins = 16;
    bool auto_sig_curve_spnruse = psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_autoSigma_mode;
    pix_max = pFix->transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans((1 << 12) - 1, pTransParams);
    pix_max = cvtinfo->frameNum == 2 ? bayertnr_logtrans((1 << 20) - 1, pTransParams) : pix_max;
    if(cvtinfo->frameNum == 2) {
        int lgbins = 8;
        // hdr long bins
        for(i = 0; i < lgbins; i++) {
            pFix->cur_spnr_luma_sigma_x[i] = 128 * (i + 1);
            pFix->pre_spnr_luma_sigma_x[i] = 128 * (i + 1);
        }
        // hdr short bins, max gain 256
        for(i = lgbins; i < spnrsigbins ; i++) {
            pFix->cur_spnr_luma_sigma_x[i] = 256 * (i - lgbins + 1) + pFix->cur_spnr_luma_sigma_x[lgbins - 1];
            pFix->pre_spnr_luma_sigma_x[i] = 256 * (i - lgbins + 1) + pFix->pre_spnr_luma_sigma_x[lgbins - 1];
        }
        pFix->cur_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;
        pFix->pre_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;
    }
    else if(!pFix->transf_bypass_en) {
        // sensor linear mode, tnr use log domain
#if 0
        for(i = 0; i < spnrsigbins; i++)
        {
            pFix->cur_spnr_luma_sigma_x[i] = 64 * (i + 1);
            pFix->pre_spnr_luma_sigma_x[i] = 64 * (i + 1);
        }
        pFix->cur_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;
        pFix->pre_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;
#else
        uint32_t blc_remain = 0;
        if(cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset && cvtinfo->blc_res.obcPostTnr.sw_blcT_obcPostTnr_en) {
            blc_remain = cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset;
        }
        int log_pix_max = bayertnr_logtrans(((1 << 12) * cvtinfo->preDGain - 1), pTransParams);
        int log_ob_offset = bayertnr_logtrans(blc_remain, pTransParams);

        pFix->pre_spnr_luma_sigma_x[0] = (log_ob_offset + 128);
        int step = 0;
        for(i = 1; i < spnrsigbins; i++) {
            step = (log_pix_max - pFix->pre_spnr_luma_sigma_x[i - 1]) / (spnrsigbins - i);
            if(i != spnrsigbins - 1) {
                step = 1 << (ROUND_F(LOG2(step)));
            } else {
                step = 1 << (FLOOR(LOG2(step)));
            }
            tmp = pFix->pre_spnr_luma_sigma_x[i - 1] + step;
            pFix->pre_spnr_luma_sigma_x[i] = CLIP(tmp, 0, log_pix_max);
            pFix->cur_spnr_luma_sigma_x[i] = pFix->pre_spnr_luma_sigma_x[i];
#if 0
            printf("spnr sigmax idx:%d step:%d stepfinnal:%d x[%d]=%d clip:%u\n",
                   i, (log_pix_max - pFix->pre_spnr_luma2sigma_x[i - 1]) / (spnrsigbins - i), step, i, tmp,  pFix->pre_spnr_luma2sigma_x[i]);
#endif
        }
#endif
    } else {
        for(i = 0; i < spnrsigbins; i++)
        {
            pFix->cur_spnr_luma_sigma_x[i] = 256 * (i + 1);
            pFix->pre_spnr_luma_sigma_x[i] = 256 * (i + 1);
        }
        pFix->cur_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;
        pFix->pre_spnr_luma_sigma_x[spnrsigbins - 1] = pix_max;
    }


    if(!auto_sig_curve_spnruse || cvtinfo->isFirstFrame) {
        if(pTransParams->isTransfBypass || opMode == RK_AIQ_OP_MODE_MANUAL) {
            for(i = 0; i < 16; i++) {
                pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_curSpNrSgm_curve.val[i] * kcoef0), 0, max_sig);
                pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val[i] * kcoef1), 0, max_sig);
                pFix->cur_spnr_luma_sigma_x[i] = pdyn->sigmaEnv.hw_btnrC_curSpNrSgm_curve.idx[i];
                pFix->pre_spnr_luma_sigma_x[i] = pdyn->sigmaEnv.hw_btnrC_iirSpNrSgm_curve.idx[i];
            }
        } else if(pTransParams->isHdrMode) {
            uint16_t shortY[spnrsigbins];
            int short_iso = cvtinfo->frameIso[1];
            int short_ilow = 0, short_ihigh = 0;
            float short_iso_ratio = 0.0f, merge_luma2Wgt = 0.0f;

            pre_interp(short_iso, NULL, 0, &short_ilow, &short_ihigh, &short_iso_ratio);

            uint16_t* pSigmaX = pFix->cur_spnr_luma_sigma_x;
            uint16_t* pSigmaY = pFix->cur_spnr_luma_sigma_y;
            uint16_t *pLongY = pdyn->sigmaEnv.hw_btnrC_curSpNrSgm_curve.val;
            uint16_t* pCalibX = pdyn->sigmaEnv.hw_btnrC_curSpNrSgm_curve.idx;
            uint16_t* pShortLowY = btnr_attrib->stAuto.dyn[short_ilow].sigmaEnv.hw_btnrC_curSpNrSgm_curve.val;
            uint16_t* pShortHighY = btnr_attrib->stAuto.dyn[short_ihigh].sigmaEnv.hw_btnrC_curSpNrSgm_curve.val;
            for(i = 0; i < spnrsigbins; i++) {
                shortY[i] = pShortLowY[i] + short_iso_ratio * (pShortHighY[i] - pShortLowY[i]);
            }
            btnr_hdr_sigma_calc(pSigmaX, pSigmaY, pCalibX, shortY, pLongY, pMergeLumaWgt, pTransParams, spnrsigbins, fdGain[0]);

            pSigmaX = pFix->pre_spnr_luma_sigma_x;
            pSigmaY = pFix->pre_spnr_luma_sigma_y;
            pLongY = pdyn->sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val;
            pCalibX = pdyn->sigmaEnv.hw_btnrC_iirSpNrSgm_curve.idx;
            pShortLowY = btnr_attrib->stAuto.dyn[short_ilow].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val;
            pShortHighY = btnr_attrib->stAuto.dyn[short_ihigh].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val;
            for(i = 0; i < spnrsigbins; i++) {
                shortY[i] = pShortLowY[i] + short_iso_ratio * (pShortHighY[i] - pShortLowY[i]);
            }
            btnr_hdr_sigma_calc(pSigmaX, pSigmaY, pCalibX, shortY, pLongY, pMergeLumaWgt, pTransParams, spnrsigbins, fdGain[0]);
        } else {
            int iso = cvtinfo->frameIso[0];
            int ilow = 0, ihigh = 0;
            float iso_ratio = 0.0f;
            float x_ratio = 0.0f;
            int y_lowISO = 0;
            int y_highISO = 0;
            pre_interp(iso, NULL, 0, &ilow, &ihigh, &iso_ratio);
            // interpolate y in low iso
            uint16_t* pLowY = btnr_attrib->stAuto.dyn[ilow].sigmaEnv.hw_btnrC_curSpNrSgm_curve.val;
            uint16_t* pHighY = btnr_attrib->stAuto.dyn[ihigh].sigmaEnv.hw_btnrC_curSpNrSgm_curve.val;
            uint16_t* pLowX = btnr_attrib->stAuto.dyn[ilow].sigmaEnv.hw_btnrC_curSpNrSgm_curve.idx;
            uint16_t* pHighX = btnr_attrib->stAuto.dyn[ihigh].sigmaEnv.hw_btnrC_curSpNrSgm_curve.idx;
            uint16_t* pSigmaX = pFix->cur_spnr_luma_sigma_x;
            uint16_t* pSigmaY = pFix->cur_spnr_luma_sigma_y;
            bayertnr_sigmaY_interpolate(pSigmaX, pSigmaY, pLowX, pHighX, pLowY, pHighY, spnrsigbins, iso_ratio);

            pLowY = btnr_attrib->stAuto.dyn[ilow].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val;
            pHighY = btnr_attrib->stAuto.dyn[ihigh].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val;
            pLowX = btnr_attrib->stAuto.dyn[ilow].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.idx;
            pHighX = btnr_attrib->stAuto.dyn[ihigh].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.idx;
            pSigmaX = pFix->pre_spnr_luma_sigma_x;
            pSigmaY = pFix->pre_spnr_luma_sigma_y;
            bayertnr_sigmaY_interpolate(pSigmaX, pSigmaY, pLowX, pHighX, pLowY, pHighY, spnrsigbins, iso_ratio);
        }
    }
    else {
        for(i = 0; i < 16; i++) {
            tmp = bayertnr_tnr_noise_curve(pFix->cur_spnr_luma_sigma_x[i], 0, pTransParams);
            pFix->cur_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef0), 0, max_sig);
            tmp = bayertnr_tnr_noise_curve(pFix->pre_spnr_luma_sigma_x[i], 0, pTransParams);
            pFix->pre_spnr_luma_sigma_y[i] = CLIP((int)(tmp * kcoef1), 0, max_sig);
        }
    }

    //x step must 2^n
    for(i = 1; i < spnrsigbins; i++) {
        x_step = pFix->cur_spnr_luma_sigma_x[i] - pFix->cur_spnr_luma_sigma_x[i - 1];
        x_step = 1 << (FLOOR(LOG2(x_step)));
        tmp = pFix->cur_spnr_luma_sigma_x[i - 1] + x_step;
        pFix->cur_spnr_luma_sigma_x[i] = CLIP(tmp, 0, pFix->tnr_pix_max);
    }

    //x step must 2^n
    for(i = 1; i < spnrsigbins; i++) {
        x_step = pFix->pre_spnr_luma_sigma_x[i] - pFix->pre_spnr_luma_sigma_x[i - 1];
        x_step = 1 << (FLOOR(LOG2(x_step)));
        tmp = pFix->pre_spnr_luma_sigma_x[i - 1] + x_step;
        pFix->pre_spnr_luma_sigma_x[i] = CLIP(tmp, 0, pFix->tnr_pix_max);
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
    if (pmdDyn->loAsRatio_hiByMskMd.sw_btnrT_hfLpfCfg_mode == btnr_cfgByFiltStrg_mode) {
        bay_gauss7x5_filter_coeff(pmdDyn->loAsRatio_hiByMskMd.sw_btnrT_hfLpf_strg, 5, 7, 7, pFix->tnr_lpf_hi_coeff);
    } else {
        for(i = 0; i < 9; i++) {
            tmp = pmdDyn->loAsRatio_hiByMskMd.hw_btnrT_hfLpfSpatial_wgt[i] * (1 << 10);
            pFix->tnr_lpf_hi_coeff[i] = CLIP(tmp, 0, 0x3ff);
        }
    }

    /* BAY3D_TNRLOW*/
    if (pmdDyn->subLoMd1.sw_btnrT_lpfCfg_mode == btnr_cfgByFiltStrg_mode) {
        bay_gauss7x5_filter_coeff(pmdDyn->subLoMd1.sw_btnrT_lpf_strg, 5, 7, 7, pFix->tnr_lpf_lo_coeff);
    } else {
        for(i = 0; i < 9; i++) {
            tmp = pmdDyn->subLoMd1.hw_btnrT_lpfSpatial_wgt[i] * (1 << 10);
            pFix->tnr_lpf_lo_coeff[i] = CLIP(tmp, 0, 0x3ff);
        }
    }

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
                 __FUNCTION__, __LINE__,
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
    tmp = pmdDyn->loAsRatio_hiByMskMd.hw_btnrT_hiMdWgt_scale * (1 << 10);
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
    tmp = pmdDyn->loAsRatio_hiByMskMd.hw_btnrT_loWgtStat_scale * (1 << 10);
    pFix->tnr_mode0_lo_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsRatio_hiByMskMd.hw_btnrT_loWgtStatHdrS_scale * (1 << 10);
    pFix->tnr_mode0_lo_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1SCL1 */
    tmp = pmdDyn->loAsBias_hiBySgmMd.hw_btnrT_loWgtStat_scale * (1 << 10);
    pFix->tnr_mode1_lo_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsBias_hiBySgmMd.hw_btnrT_loWgtStatHdrS_scale * (1 << 10);
    pFix->tnr_mode1_lo_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1SCL2 */
    tmp = pmdDyn->loAsBias_hiBySgmMd.hw_btnrT_mdWgt_scale * (1 << 10);
    pFix->tnr_mode1_wgt_scale  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsBias_hiBySgmMd.hw_btnrT_mdWgtHdrS_scale  * (1 << 10);
    pFix->tnr_mode1_wgt_hdr_sht_scale  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGTOFF */
    tmp = pmdDyn->loAsBias_hiBySgmMd.hw_btnrT_loWgtStat_negOff * (1 << 10);
    pFix->tnr_mode1_lo_wgt_offset  = CLIP(tmp, 0, 0x3fff);
    tmp = pmdDyn->loAsBias_hiBySgmMd.hw_btnrT_loWgtStatHdrS_negOff * (1 << 10);
    pFix->tnr_mode1_lo_wgt_hdr_sht_offset  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWGT1OFF */
    tmp = pdyn->sigmaEnv.hw_btnrT_statsPixAlpha_thred * (1 << 10);
    pFix->tnr_auto_sigma_count_wgt_thred  = CLIP(tmp, 0, 0x3ff);
    tmp = pmdDyn->loAsBias_hiBySgmMd.hw_btnrT_mdWgt_minLimit * (1 << 10);
    pFix->tnr_mode1_wgt_min_limit  = CLIP(tmp, 0, 0x3ff);
    tmp = pmdDyn->loAsBias_hiBySgmMd.hw_btnrT_mdWgt_offset * (1 << 10);
    pFix->tnr_mode1_wgt_offset  = CLIP(tmp, 0, 0xfff);

    /* BAY3D_TNRWLO_THL */
    tmp =  pmdDyn->frmAlpha.hw_btnrT_loAlpha_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pmdDyn->frmAlpha.hw_btnrT_loAlpha_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_min_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pmdDyn->frmAlpha.hw_btnrT_loAlphaHdrS_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pmdDyn->frmAlpha.hw_btnrT_loAlphaHdrS_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_hdr_sht_min_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWLO_THH */
    tmp =  pmdDyn->frmAlpha.hw_btnrT_loAlpha_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pmdDyn->frmAlpha.hw_btnrT_loAlpha_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_max_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pmdDyn->frmAlpha.hw_btnrT_loAlphaHdrS_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pmdDyn->frmAlpha.hw_btnrT_loAlphaHdrS_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_lo_wgt_clip_hdr_sht_max_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWHI_THL */
    tmp =  pmdDyn->frmAlpha.hw_btnrT_hiAlpha_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pmdDyn->frmAlpha.hw_btnrT_hiAlpha_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_min_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pmdDyn->frmAlpha.hw_btnrT_hiAlphaHdrS_minLimit == 0  ?  0 : (int)((1.0 - 1.0 / pmdDyn->frmAlpha.hw_btnrT_hiAlphaHdrS_minLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_hdr_sht_min_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRWHI_THH */
    tmp =  pmdDyn->frmAlpha.hw_btnrT_hiAlpha_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pmdDyn->frmAlpha.hw_btnrT_hiAlpha_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_max_limit  = CLIP(tmp, 0, 0x3fff);
    tmp =  pmdDyn->frmAlpha.hw_btnrT_hiAlphaHdrS_maxLimit == 0  ?  0 : (int)((1.0 - 1.0 / pmdDyn->frmAlpha.hw_btnrT_hiAlphaHdrS_maxLimit) * (1 << FIXTNRWWW));
    pFix->tnr_hi_wgt_clip_hdr_sht_max_limit  = CLIP(tmp, 0, 0x3fff);

    /* BAY3D_TNRKEEP */
    tmp =  pdyn->noiseBal_byHiAlpha.hw_btnrT_curHiOrg_alpha * (1 << FIXBITWFWGT);
    pFix->tnr_cur_spnr_hi_wgt_min_limit  = CLIP(tmp, 0, 0xff);
    tmp =  pdyn->noiseBal_byHiAlpha.hw_btnrT_iirHiOrg_alpha * (1 << FIXBITWFWGT);
    pFix->tnr_pre_spnr_hi_wgt_min_limit  = CLIP(tmp, 0, 0xff);


    tmp = psta->sigmaEnv.hw_btnrCfg_statsPixCnt_thred;
    pFix->tnr_auto_sigma_count_th = CLIP(tmp, 0, 0xfffff);

    tmp = pmdDyn->subLoMd1.hw_btnrT_mdWgtFstLnNegOff_en;
    pFix->lowgt_ctrl = CLIP(tmp, 0, 3);
    tmp = pmdDyn->subLoMd1.hw_btnrT_mdWgtFstLn_negOff;
    pFix->lowgt_offint = CLIP(tmp, 0, 0x3ff);

    tmp = MIN(((float)1.0 / (float)(pdyn->noiseBal_byLoAlpha.hw_btnrT_noiseBal_strg) * (1 << FIXTNRWGT)), (1 << FIXTNRWGT)); //(1<<FIXTNRWGT);
    pFix->tnr_motion_nr_strg = CLIP(tmp, 0, 0x7ff);
    tmp = MIN((pdyn->locSgmStrg.hw_bnrT_locSgmStrg_maxLimit * (1 << FIXGAINOUT)), ((1 << FIXGAINOUT) - 1));
    pFix->tnr_gain_max = CLIP(tmp, 0, 0xff);


    pTransParams->bayertnr_lo_wgt_clip_min_limit = pFix->tnr_lo_wgt_clip_min_limit;
    pTransParams->bayertnr_lo_wgt_clip_max_limit = pFix->tnr_lo_wgt_clip_max_limit;
    pFix->tnr_out_sigma_sq = bayertnr_update_sq(pTransParams);

    if (!pFix->transf_bypass_en) {
        rk_aiq_btnr40_params_logtrans(pFix);
    }
    return;
}
