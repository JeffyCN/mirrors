#include "rk_aiq_isp33_modules.h"
#include "interpolation.h"

// fix define
#define FIXTNRSQRT      40
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

void bay_gauss5x5_spnr_coeff(float sigma, int halftaby, int halftabx, int strdtabx, int* gstab)
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
            tmpf0 = tmpf0 / (sigma * sigma);
            tmpf1 = expf(-tmpf0);
            tmpf2 = tmpf2 + tmpf1;
            gausstabf[i * halfx + j] = tmpf1;
        }
    }
    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gausstab[i * halfx + j] = (int)(gausstabf[i * halfx + j] * (1 << 6));
            sumc = sumc + gausstab[i * halfx + j];
        }
    }

    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gstab[gstabidx[i * halfx + j]] = gausstab[i * halfx + j];
        }
    }
}

void bay_gauss7x5_filter_coeff(float sigma, int halftaby, int halftabx, int strdtabx, int *gstab)
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
    gausstab[halfy / 2 * halfx + halfx / 2] += ((1 << 10) - sumc);

    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gstab[gstabidx[i * halfx + j]] = gausstab[i * halfx + j];
        }
    }
}

int bayertnr_kalm_bitcut_V40(int datain, int bitsrc, int bitdst)
{
    int out;
    out = bitsrc == bitdst ? datain : ((datain + (1 << (bitsrc - bitdst - 1))) >> (bitsrc - bitdst));
    return out;
}

int bayertnr_wgt_sqrt_tab_V40(int index)
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

void bayertnr_luma2sigmax_config_v41(btnr_trans_params_t *pTransParams, blc_res_cvt_t *pBlc, float preDgain)
{
    // tnr sigma curve
    float kcoef0, kcoef1;
    int i, tmp, tmp0, tmp1;
    int max_sig, pix_max, pix_max_long;
    int sigbins = 20;
    bool transf_bypass_en = pTransParams->isTransfBypass;
    if(preDgain < 1.0) {
        preDgain = 1.0;
    }

    pix_max = transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans((1 << 12) - 1, pTransParams);
    pix_max = pTransParams->isHdrMode ? bayertnr_logtrans((1 << 20) - 1, pTransParams) : pix_max;
    if(pTransParams->isHdrMode)
    {
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 1;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 10;
        // hdr long bins
        int lgbins = pTransParams->bayertnr_tnr_sigma_curve_double_pos;
        for(i = 0; i < lgbins; i++) {
            tmp = 128 * (i + 1); //pSelect->bayertnr_tnr_lum[i];
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pix_max);
        }
        // hdr short bins
        int shbins = sigbins - lgbins;
        for(i = lgbins; i < lgbins + 6; i++) {
            tmp = 128 * (i - lgbins + 1)  + pTransParams->tnr_luma_sigma_x[lgbins - 1]; //pParser->bayertnr_tnr_lum[i];
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pix_max);
        }
        for(i = lgbins + 6; i < sigbins; i++) {
            tmp = 256 * (i - lgbins - 6 + 1)  + pTransParams->tnr_luma_sigma_x[lgbins + 6 - 1]; //pParser->bayertnr_tnr_lum[i];
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pix_max);
        }
        pTransParams->tnr_luma_sigma_x[sigbins - 1] = pix_max;

    } else if(!transf_bypass_en)  {
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 0;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 0;

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
    }
    else
    {
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 0;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 0;
        float blc_remain = 0;
        /*
            #ifdef supportManualOBC
            if(pBlc->obcPostTnr.sw_blcT_obcPostTnr_mode == blc_manualOBCPostTnr_mode) {
                blc_remain = MAX(blc_remain, pBlc->obcPostTnr.hw_blcT_manualOBR_val);
                blc_remain = MAX(blc_remain, pBlc->obcPostTnr.hw_blcT_manualOBGr_val);
                blc_remain = MAX(blc_remain, pBlc->obcPostTnr.hw_blcT_manualOBGb_val);
                blc_remain = MAX(blc_remain, pBlc->obcPostTnr.hw_blcT_manualOBB_val);
            }
           #endif
        */


        if(pBlc->obcPostTnr.sw_blcT_autoOB_offset && pBlc->obcPostTnr.sw_blcT_obcPostTnr_en) {
            blc_remain += pBlc->obcPostTnr.sw_blcT_autoOB_offset;
        }

        if(blc_remain != 0) {
            pTransParams->tnr_luma_sigma_x[0] = blc_remain;
        } else {
            pTransParams->tnr_luma_sigma_x[0] = 128;
        }

        for(i = 1; i < 8; i++) {
            tmp = 128 * (i + 0) + pTransParams->tnr_luma_sigma_x[0];
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pix_max);
        }
        for(i = 8; i < sigbins; i++) {
            tmp = 256 * (i - 8 + 1)  + pTransParams->tnr_luma_sigma_x[ 8 - 1];
            pTransParams->tnr_luma_sigma_x[i] = CLIP(tmp, 0, pix_max);
        }
        pTransParams->tnr_luma_sigma_x[sigbins - 1] = pix_max;
    }

#if 0
    printf("btnr sigmax ob:%u predgain:%f [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           pBlc->obcPostTnr.sw_blcT_autoOB_offset,
           preDgain,
           pTransParams->tnr_luma_sigma_x[0],
           pTransParams->tnr_luma_sigma_x[1],
           pTransParams->tnr_luma_sigma_x[2],
           pTransParams->tnr_luma_sigma_x[3],
           pTransParams->tnr_luma_sigma_x[4],
           pTransParams->tnr_luma_sigma_x[5],
           pTransParams->tnr_luma_sigma_x[6],
           pTransParams->tnr_luma_sigma_x[7],
           pTransParams->tnr_luma_sigma_x[8],
           pTransParams->tnr_luma_sigma_x[9],
           pTransParams->tnr_luma_sigma_x[10],
           pTransParams->tnr_luma_sigma_x[11],
           pTransParams->tnr_luma_sigma_x[12],
           pTransParams->tnr_luma_sigma_x[13],
           pTransParams->tnr_luma_sigma_x[14],
           pTransParams->tnr_luma_sigma_x[15],
           pTransParams->tnr_luma_sigma_x[16],
           pTransParams->tnr_luma_sigma_x[17],
           pTransParams->tnr_luma_sigma_x[18],
           pTransParams->tnr_luma_sigma_x[19]);
#endif
}

void rk_aiq_btnr42_params_logtrans(struct isp35_bay3d_cfg *pCfg)
{
    uint8_t is15bit = pCfg->transf_mode_scale;
    uint8_t offsetbit = bayertnr_find_top_one_pos(pCfg->transf_mode_offset);

#define LOGTRANSF_VAR(a) a = isp39_logtransf(a, is15bit, offsetbit)
    LOGTRANSF_VAR(pCfg->lo_wgt_vfilt_offset);
    LOGTRANSF_VAR(pCfg->lo_pre_soft_thresh_max_limit);
    LOGTRANSF_VAR(pCfg->lo_pre_soft_thresh_min_limit);
    LOGTRANSF_VAR(pCfg->pre_spnr_sigma_offset);
    LOGTRANSF_VAR(pCfg->pre_spnr_sigma_hdr_sht_offset);
#undef LOGTRANSF_VAR
}

static void bay_hozi_filter_coeff(float sigma, int *hfilter_coeff)
{
    float tmpf0, tmpf1, coeff[5];
    int sumc = 0;

    tmpf1 = 0;
    for (int j = 0; j < 5; j++)
    {
        tmpf0 = (float)((j - 2) * (j - 2));
        tmpf0 = tmpf0 / (2 * sigma * sigma);
        coeff[j] = expf(-tmpf0);
        tmpf1 = tmpf1 + coeff[j];
    }
    for (int j = 0; j < 5; j++)
    {
        hfilter_coeff[j] = (int)(coeff[j] / tmpf1 * (1 << 5));
        sumc = sumc + hfilter_coeff[j];
    }
    hfilter_coeff[2] += ((1 << 5) - sumc);
}



void rk_aiq_btnr42_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo, btnr_cvt_info_t* pBtnrInfo, mergeLuma2Wgt_t* pMergeLumaWgt)
{
    btnr_trans_params_t *pTransParams = &pBtnrInfo->mBtnrTransParams;
    btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[0];

    btnr_api_attrib_t *btnr_attrib = pBtnrInfo->btnr_attrib;
    btnr_param_auto_t *paut = &btnr_attrib->stAuto;
    rk_aiq_op_mode_t opMode = btnr_attrib->opMode;
#if 0
    printf("hw_btnrT_sigma_scale %f %f\n",
           paut->mdMeDyn[0].mdSigma.hw_btnrT_sigma_scale, paut->mdMeDyn[1].mdSigma.hw_btnrT_sigma_scale);
#endif


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
    blc_cvt_info_t* pBlcInfo = (blc_cvt_info_t*)cvtinfo->pBlcInfo;
    float sigma_ratio = ((float)btnr_stats->sigma_num) / (cvtinfo->rawWidth * cvtinfo->rawHeight);
    pBlcInfo->sigma_ratio = sigma_ratio;

    float tmpf;
    int i, j, tmp, tmp0, tmp1, sigbins, halfx;
    struct isp35_bay3d_cfg *pCfg = &isp_params->isp_cfg->others.bay3d_cfg;

    btnr_param_t *btnr_param = (btnr_param_t *) attr;
    btnr_params_static_t* psta = &btnr_param->sta;
    btnr_other_dyn_t* pdyn = &btnr_param->spNrDyn;
    btnr_mdMe_dyn_t* pmdDyn = &btnr_param->mdMeDyn;

    bool bayertnr_default_noise_curve_use = false;
    int bayertnr_iso_cur = cvtinfo->frameIso[cvtinfo->frameNum - 1];
    bayertnr_default_noise_curve_use = bayertnr_find_top_one_pos(bayertnr_iso_cur) != bayertnr_find_top_one_pos(pTransParams->bayertnr_iso_pre);
    pTransParams->bayertnr_iso_pre = bayertnr_iso_cur;
    pTransParams->isFirstFrame = cvtinfo->isFirstFrame;
    pTransParams->isHdrMode = cvtinfo->frameNum == 2;

    if(psta->hw_btnrCfg_pixDomain_mode != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_pixDomain_mode
            || psta->transCfg.hw_btnrCfg_trans_mode != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_mode
            || psta->transCfg.hw_btnrCfg_trans_offset != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_offset) {
        LOGW_ANR("Btnr run in pixLog2Domain is %d, trans_mode is %d, trans_offset is %d.\n"
                 "But calib pixLog2Domain_mode is %d, trans_mode is %d, trans_offset is %d.\n",
                 psta->hw_btnrCfg_pixDomain_mode, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_pixDomain_mode,
                 psta->transCfg.hw_btnrCfg_trans_mode, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_mode,
                 psta->transCfg.hw_btnrCfg_trans_offset, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_offset);
    }

    if (cvtinfo->frameNum > 1) {
        if (psta->hw_btnrCfg_pixDomain_mode != btnr_pixLog2Domain_mode) {
            LOGW_ANR("Btnr must run in pixLog2Domain(ori mode is %d) when isp is HDR mode(framenum=%d), btnr_pixLog2Domain_mode is be forcibly set to hw_btnrCfg_pixDomain_mode in HWI\n"
                     "You can set by btnr.static.hw_btnrCfg_pixDomain_mode, but change hw_btnrCfg_pixDomain_mode in running time may cause abnormal image transitions\n",
                     psta->hw_btnrCfg_pixDomain_mode, cvtinfo->frameNum, psta->hw_btnrCfg_pixDomain_mode);
            psta->hw_btnrCfg_pixDomain_mode = btnr_pixLog2Domain_mode;
        }
        if (psta->transCfg.hw_btnrCfg_trans_mode != btnr_pixInBw20b_mode) {
            LOGE_ANR("hw_btnrCfg_trans_mode == %s(0x%x) is error. When isp is in HDR mode, btnr must run in 'btnr_pixInBw20b_mode'. "
                     "The trans_mode will be forcibly set to 'btnr_pixInBw20b_mode' in HWI.",
                     trans_mode2str(psta->transCfg.hw_btnrCfg_trans_mode), psta->transCfg.hw_btnrCfg_trans_mode);
            psta->transCfg.hw_btnrCfg_trans_mode = btnr_pixInBw20b_mode;
        }
    }

    if(cvtinfo->frameNum == 1 && cvtinfo->preDGain > 1.0) {
        if (psta->hw_btnrCfg_pixDomain_mode != btnr_pixLog2Domain_mode) {
            LOGW_ANR("Btnr must run in pixLog2Domain(ori mode is %d) when predgain > 1.0, btnr_pixLog2Domain_mode is be forcibly set to hw_btnrCfg_pixDomain_mode in HWI\n",
                     psta->hw_btnrCfg_pixDomain_mode);
            psta->hw_btnrCfg_pixDomain_mode = btnr_pixLog2Domain_mode;
        }
    }

    if (cvtinfo->use_aiisp) {
        if (psta->transCfg.hw_btnrCfg_trans_mode != btnr_pixInBw20b_mode) {
            LOGE_ANR("hw_btnrCfg_trans_mode == %s(0x%x) is error. When isp is in Aiisp mode, btnr must run in 'btnr_pixInBw20b_mode'. "
                     "The trans_mode will be forcibly set to 'btnr_pixInBw20b_mode' in HWI.",
                     trans_mode2str(psta->transCfg.hw_btnrCfg_trans_mode), psta->transCfg.hw_btnrCfg_trans_mode);
            psta->transCfg.hw_btnrCfg_trans_mode = btnr_pixInBw20b_mode;
        }
    }

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
        //printf("exp idx%d: time:%f, gain:%f, all:%f\n", i, frameEt[i], frameiso[i], fdGain[i]);
    }
    for (i = 0; i < framenum; i++) {
        fdGain[i] = fdGain[framenum - 1] / fdGain[i];
    }

    //printf("fdGain[0]:%f\n", fdGain[0]);
    // REG: BAY3D_CTRL0
    if (pdyn->outFrmFusion.sw_btnrT_outFrmBase_mode == btnr_curBaseOut_mode) {
        pCfg->out_use_pre_mode = pdyn->outFrmFusion.noiseBal_curBaseOut.sw_btnrT_noiseBal_mode;
    } else {
        pCfg->out_use_pre_mode = pdyn->outFrmFusion.noiseBal_preBaseOut.sw_btnrT_noiseBal_mode;
    }
    //pCfg->out_use_pre_mode = CLIP(pdyn->hw_btnrT_noiseBal_mode, 0, 0x7);
    pCfg->motion_est_en = cvtinfo->frameId > 1 ? pmdDyn->memc.hw_btnrT_memc_en : 0;
    pCfg->btnr_ldc_en = psta->swMemc.sw_btnr_swMemc_en;
#if 1
    // TODO: for aiisp
    pCfg->iir_wr_src = 0;
    pCfg->iir_rw_fmt = 2;

    if (psta->hw_btnrCfg_pixDomain_mode == btnr_pixLog2Domain_mode &&
            !cvtinfo->use_aiisp)
        pCfg->iir_rw_fmt = 0; // could be 0~3, preferred to be 0 to save bandwidth

    if (psta->hw_btnrCfg_pixDomain_mode == btnr_pixLinearDomain_mode &&
            cvtinfo->frameNum == 1)
        pCfg->iir_rw_fmt = 2; // should be 2 by hardware restriction

    if (pCfg->btnr_ldc_en)//TODO
        pCfg->iir_rw_fmt = 3; // should be 3 by hardware restriction

    // check conflictions
    if (pCfg->btnr_ldc_en && cvtinfo->use_aiisp && (cvtinfo->frameNum > 1))
        LOGE("btnr_ldc_en and aiisp_hdr_en can't be enabled simultaneously");
#endif

    // REG: BAY3D_CTRL1
    switch (psta->hw_btnrCfg_pixDomain_mode) {
    case btnr_pixLog2Domain_mode :
        pCfg->transf_bypass_en = 0;
        pCfg->transf_mode  = 0;
        break;
    case btnr_pixLinearDomain_mode:
        pCfg->transf_bypass_en = 1;
        pCfg->transf_mode  = 0;
        break;
    }
    switch (pmdDyn->loMd.hw_btnrT_loMd_mode) {
    case btnr_allSubLoMdMix_mode:
        pCfg->lo_mge_wgt_mode = 0;
        break;
    case btnr_subLoMd0Only_mode:
        pCfg->lo_mge_wgt_mode = 1;
        break;
    case btnr_subLoMd1Only_mode:
        pCfg->lo_mge_wgt_mode = 2;
        break;
    case btnr_subDeepLoMdOnly_mode:
        pCfg->lo_mge_wgt_mode = 3;
        break;
    }
    if (psta->debug.sw_btnrT_dbgOut_en) {
        switch (psta->debug.hw_btnrT_dbgOut_mode) {
        case btnr_dbgOut_preSpNr_mode:
            pCfg->cur_spnr_out_en = 0;
            pCfg->pre_spnr_out_en = 1;
            pCfg->md_wgt_out_en = 0;
            break;
        case btnr_dbgOut_curSpNr_mode :
            pCfg->cur_spnr_out_en = 1;
            pCfg->pre_spnr_out_en = 0;
            pCfg->md_wgt_out_en = 0;
            break;
        case btnr_dbgOut_mdWgt_mode:
            pCfg->cur_spnr_out_en = 0;
            pCfg->pre_spnr_out_en = 0;
            pCfg->md_wgt_out_en = 1;
            break;
        }
    } else {
        pCfg->md_wgt_out_en = 0;
        pCfg->pre_spnr_out_en = 0;
        pCfg->cur_spnr_out_en = 0;
    }

    if (pmdDyn->subDeepLoMd.sw_btnrT_dLoSrc_mode == btnr_subLoMd1_mode)
        pCfg->md_large_lo_use_mode = 0;
    else
        pCfg->md_large_lo_use_mode = 1;

    pCfg->tnrsigma_curve_double_en =  (cvtinfo->frameNum > 1) ? 1 : 0;
    pCfg->md_large_lo_min_filter_bypass_en = !pmdDyn->subDeepLoMd.hw_btnrT_minFilt_en;
    pCfg->md_large_lo_gauss_filter_bypass_en = !pmdDyn->subDeepLoMd.hw_btnrT_lpf_en;
    pCfg->md_large_lo_md_wgt_bypass_en = !pmdDyn->subDeepLoMd.hw_btnrT_dLoMd_en;
    pCfg->pre_pix_out_mode = pmdDyn->frmFusion.hw_btnrT_frmFusion_mode;
    pCfg->lpf_hi_bypass_en = !pmdDyn->hiMd.hw_btnrT_hfLpf_en;
    pCfg->lo_diff_vfilt_bypass_en = !pmdDyn->subLoMd0.diffCh.hw_btnrT_vFilt_en;
    pCfg->lpf_lo_bypass_en = !pmdDyn->subLoMd1.hw_btnrT_lpf_en;
    pCfg->lo_wgt_hfilt_en = pmdDyn->subLoMd1.hw_btnrT_hFilt_en;
    pCfg->lo_diff_hfilt_en = pmdDyn->subLoMd0.diffCh.hw_btnrT_hFilt_en;
    pCfg->sig_hfilt_en = pmdDyn->subLoMd0.sgmCh.hw_btnrT_hFilt_en;
    pCfg->motion_detect_bypass_en = !pmdDyn->hw_btnrT_md_en;
    pCfg->md_only_lo_en = !pmdDyn->loAsBias_hiMd1.hw_btnrT_hiMd_en;
    pCfg->lo_detection_bypass_en = !pmdDyn->loMd.hw_btnrT_loMd_en;

    // REG: BAY3D_CTRL2
    if (pdyn->preSpNr.sigma.hw_btnrT_sigma_mode == btnr_lutSgmOnly_mode)
        pCfg->spnr_presigma_use_en = 0;
    else
        pCfg->spnr_presigma_use_en = 1;

    if (pdyn->preSpNr.sigma.hw_btnrT_sigmaCurve_mode == btnr_midSegmInterpOn_mode)
        pCfg->pre_spnr_sigma_curve_double_en = 0;
    else
        pCfg->pre_spnr_sigma_curve_double_en = 1;
    if (cvtinfo->frameNum > 1) {
        if (pCfg->pre_spnr_sigma_curve_double_en == 0) {
            if (cvtinfo->btnr_warning_count < 5) {
                LOGW_ANR("When isp is HDR mode, hw_btnrT_sigmaCurve_mode recommends using btnr_midSegmInterpOff_mode. "
                         "You can set by dyn.curFrmSpNr.hw_btnrT_sigmaCurve_mode");
            }
            else if (cvtinfo->btnr_warning_count % 300 == 0) {
                LOGW_ANR("When isp is HDR mode, hw_btnrT_sigmaCurve_mode recommends using btnr_midSegmInterpOff_mode. "
                         "You can set by dyn.curFrmSpNr.hw_btnrT_sigmaCurve_mode");
            }
            cvtinfo->btnr_warning_count++;
            // pCfg->pre_spnr_sigma_curve_double_en  = 1;
        }
    }

    if (pdyn->preSpNr.sigma.hw_btnrT_idxLpfStrg_mode == btnr_lpfStrgH_mode)
        pCfg->pre_spnr_sigma_idx_filt_mode = 0;
    else
        pCfg->pre_spnr_sigma_idx_filt_mode = 1;

    switch (pdyn->preSpNr.hiNr.sw_btnrT_rgeWgt_mode) {
    case btnr_bayerAvg_mode:
        pCfg->pre_spnr_hi_filter_wgt_mode = 0;
        pCfg->pre_spnr_hi_filter_rb_wgt_mode = 0;
        break;
    case btnr_gIndividual_rbAvg_mode :
        pCfg->pre_spnr_hi_filter_wgt_mode = 1;
        pCfg->pre_spnr_hi_filter_rb_wgt_mode = 0;
        break;
    case btnr_bayerIndividual_mode :
        pCfg->pre_spnr_hi_filter_wgt_mode = 1;
        pCfg->pre_spnr_hi_filter_rb_wgt_mode = 1;
        break;
    }
    switch (pdyn->preSpNr.loNr.sw_btnrT_rgeWgt_mode) {
    case btnr_bayerAvg_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 0;
        pCfg->pre_spnr_lo_filter_rb_wgt_mode = 0;
        break;
    case btnr_gIndividual_rbAvg_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 1;
        pCfg->pre_spnr_lo_filter_rb_wgt_mode = 0;
        break;
    case btnr_bayerIndividual_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 1;
        pCfg->pre_spnr_lo_filter_rb_wgt_mode = 1;
        break;
    }

    switch (pdyn->preSpNr.hiNr.hw_btnrT_gic_mode) {
    case btnr_gicDisable_mode :
        pCfg->pre_spnr_hi_filter_gic_en = 0;
        pCfg->pre_spnr_hi_filter_gic_enhance_en = 0;
        break;
    case btnr_gicStrgL_mode :
        pCfg->pre_spnr_hi_filter_gic_en = 1;
        pCfg->pre_spnr_hi_filter_gic_enhance_en = 0;
        break;
    case btnr_gicStrgH_mode :
        pCfg->pre_spnr_hi_filter_gic_en = 1;
        pCfg->pre_spnr_hi_filter_gic_enhance_en = 1;
        break;
    }
    pCfg->cur_spnr_filter_bypass_en = !(pdyn->curSpNr.hw_btnrT_spNr_en && pdyn->curSpNr.hw_btnrT_hiFilter_en);
    pCfg->pre_spnr_lo_filter_bypass_en = !(pdyn->preSpNr.loNr.hw_btnrT_loNr_en && pdyn->preSpNr.sw_btnrT_spNr_en);
    pCfg->pre_spnr_hi_filter_bypass_en = !(pdyn->preSpNr.hiNr.hw_btnrT_hiNr_en && pdyn->preSpNr.sw_btnrT_spNr_en);
    pCfg->pre_spnr_hi_guide_filter_bypass_en = !pdyn->preSpNr.hiNr.hw_btnrT_guideLpf_en;
    pCfg->pre_spnr_sigma_idx_filt_bypass_en = !pdyn->preSpNr.sigma.hw_btnrT_sigmaIdxLpf_en;
    pCfg->pre_spnr_hi_noise_ctrl_en = pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_en;
    pCfg->pre_hi_gic_lp_en =  psta->lowPowerCfg.preSpnrLP.hiNrLP.hw_btnrCfg_gicLP_en;
    pCfg->pre_hi_bf_lp_en =  psta->lowPowerCfg.preSpnrLP.hiNrLP.hw_btnrCfg_bfLP_en;
    pCfg->pre_lo_avg_lp_en =  psta->lowPowerCfg.preSpnrLP.loNrLP.hw_btnrCfg_avgRgeWgtLP_en;
    pCfg->pre_spnr_dpc_flt_en =  pdyn->preDpc.hw_btnrT_dpc_en;
    pCfg->pre_spnr_dpc_flt_mode = CLIP(pdyn->preDpc.hw_btnrT_dpc_mode, 1, 1); //ONLY SUPPORT btnr_nsBalSoftThd_mode
    pCfg->pre_spnr_dpc_nr_bal_mode = pCfg->pre_spnr_dpc_flt_mode ?
                                     pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_mode : pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_preWgt_mode;
    pCfg->pre_spnr_dpc_flt_prewgt_en = pCfg->pre_spnr_dpc_flt_mode ?
                                       pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_en : pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_preWgt_en;

    // REG: BAY3D_CTRL3
    if (pmdDyn->hiMd.hw_btnrT_md_mode == btnr_loAsRatio_hi0_mode) {
        pCfg->wgt_cal_mode = 0;
    } else {
        pCfg->wgt_cal_mode = 1;
    }

    if (psta->hw_btnrCfg_megWgtDs_mode == btnr_megWgtDs_avg_mode)
        pCfg->mge_wgt_ds_mode = 0;
    else
        pCfg->mge_wgt_ds_mode = 1;

    pCfg->transf_mode = 0;
    pCfg->kalman_wgt_ds_mode = CLIP(psta->hw_btnrCfg_kalWgtDs_mode, 0, 0x3);
    pCfg->mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;
    pCfg->sigma_calc_mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;

    // REG: BAY3D_TRANS0
    if (psta->transCfg.hw_btnrCfg_trans_mode == btnr_pixInBw15b_mode)
        pCfg->transf_mode_scale = 1;
    else
        pCfg->transf_mode_scale = 0;

    tmp = psta->transCfg.hw_btnrCfg_trans_offset;
    int n  = bayertnr_find_top_one_pos(tmp);
    if(pCfg->transf_mode_scale == 1) {
        n = 8;
    } else {
        if(n < 8)
            n = 8;
        if(n > 12)
            n = 12;
    }
    tmp = (1 << n);
    pCfg->transf_mode_offset = CLIP(tmp, 0, 0x1fff);

    if (pCfg->transf_mode_scale == 1) {
        pCfg->itransf_mode_offset = (1 << 9) * n;
    } else {
        pCfg->itransf_mode_offset = (1 << 8) * n;
    }

    // REG: BAY3D_TRANS1
    // mode scale只有0�?两种选择，对应log域的小数精度�?    // 0对应8bit小数�?bit整数�?4bit整数导致最大线性输入数据位宽为20bit->transData_maxlimit�?�?0次方-1
    // 1对应9bit小数�?bit整数�?3bit整数限制最大线性输入数据位宽为15bit->transData_maxlimit�?�?5次方-1
    if(pCfg->transf_bypass_en == 1) {
        pCfg->transf_data_max_limit = ((1 << 12) - 1);
    } else {
        if (pCfg->transf_mode_scale == 1) {
            pCfg->transf_data_max_limit = ((1 << 15) - 1);
        }
        else {
            pCfg->transf_data_max_limit = ((1 << 20) - 1);
        }
    }

    // REG: BAY3D_PREHI_SIGSCL
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_preSigma_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_ctrl_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PREHI_SIGOF
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideLpf3_alpha) * (1 << 6);
    pCfg->pre_spnr_hi_guide_out_wgt = CLIP(tmp, 0, 0x7f);

    {
        int coeff[6];
        if (pdyn->curSpNr.sw_btnrT_filtCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_filter_coeff(pdyn->curSpNr.sw_btnrT_filtSpatial_strg, 5, 5, 5, coeff, (1 << 8));
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = pdyn->curSpNr.sw_btnrT_filtSpatial_wgt[i];
        }
        // REG: BAY3D_CURHISPW0
        pCfg->cur_spnr_filter_coeff[0] = CLIP(coeff[0], 0, 0xff);
        pCfg->cur_spnr_filter_coeff[1] = CLIP(coeff[1], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[2] = CLIP(coeff[2], 0, 0x3f);
        // REG: BAY3D_CURHISPW1
        pCfg->cur_spnr_filter_coeff[3] = CLIP(coeff[3], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[4] = CLIP(coeff[4], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[5] = CLIP(coeff[5], 0, 0x3f);
    }


    // REG: BAY3D_PREHI_SIGSCL
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_rgeSgm_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_hi_sigma_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PREHI_WSCL
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_diffSgmRatio_scale) * (1 << 6);
    pCfg->pre_spnr_hi_wgt_calc_scale = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_PREHIWMM
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_filtWgt_minLimit) * (1 << 8);
    pCfg->pre_spnr_hi_filter_wgt_min_limit = CLIP(tmp, 1, 0xff);

    pCfg->pre_spnr_hi_wgt_calc_offset = 0;
    // REG: BAY3D_PREHISIGOF
    //tmp = MAX((1 - pdyn->preSpNr.hiNr.hw_btnrT_hiNrOut_alpha) * (1 << 7), 0);
    tmp = 0;
    pCfg->pre_spnr_hi_filter_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigma_offset) * (1 << 0);
    pCfg->pre_spnr_sigma_offset = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigmaHdrS_offset) * (1 << 0);
    pCfg->pre_spnr_sigma_hdr_sht_offset = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_PREHISIGSCL
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigma_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigmaHdrS_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);

    {
        int coeff[6];
        if (pdyn->preSpNr.hiNr.sw_btnrT_filtCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_spnr_coeff(pdyn->preSpNr.hiNr.sw_btnrT_filtSpatial_strg, 5, 5, 5, coeff);
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = pdyn->preSpNr.hiNr.sw_btnrT_filtSpatial_wgt[i];
        }
        // REG: BAY3D_PREHISPW0
        pCfg->pre_spnr_hi_filter_coeff[0] = CLIP(coeff[0], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[1] = CLIP(coeff[1], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[2] = CLIP(coeff[2], 0, 0x3f);
        // REG: BAY3D_PREHISPW1
        pCfg->pre_spnr_hi_filter_coeff[3] = CLIP(coeff[3], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[4] = CLIP(coeff[4], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[5] = CLIP(coeff[5], 0, 0x3f);
    }

    // REG: BAY3D_PRELOSIGCSL
    tmp                           = (pdyn->preSpNr.loNr.hw_btnrT_rgeSgm_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_lo_sigma_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PRELOSIGOF
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_rgeWgt_scale) * (1 << 6);
    pCfg->pre_spnr_lo_wgt_calc_scale = CLIP(tmp, 0, 0xff);

    pCfg->pre_spnr_lo_wgt_calc_offset = 0;
    // REG: BAY3D_PREHI_NRCT
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_scale) * (1 << 7);
    pCfg->pre_spnr_hi_noise_ctrl_scale = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_offset) * (1 << 7);
    pCfg->pre_spnr_hi_noise_ctrl_offset = CLIP(tmp, 0, 0x7f);


    {
        int coeff[9];
        if (pmdDyn->hiMd.sw_btnrT_hfLpfCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss7x5_filter_coeff(pmdDyn->hiMd.sw_btnrT_hfLpf_strg, 5, 7, 7, coeff);
        } else {
            for (i = 0; i < 9; i++)
                coeff[i] = pmdDyn->hiMd.hw_btnrT_hfLpfSpatial_wgt[i];
        }
        // REG: BAY3D_HIWD0
        pCfg->lpf_hi_coeff[0] = CLIP(coeff[0], 0, 0x3ff);
        pCfg->lpf_hi_coeff[1] = CLIP(coeff[1], 0, 0x3ff);
        pCfg->lpf_hi_coeff[2] = CLIP(coeff[2], 0, 0x3ff);
        // REG: BAY3D_HIWD3
        pCfg->lpf_hi_coeff[3] = CLIP(coeff[3], 0, 0x3ff);
        pCfg->lpf_hi_coeff[4] = CLIP(coeff[4], 0, 0x3ff);
        pCfg->lpf_hi_coeff[5] = CLIP(coeff[5], 0, 0x3ff);
        // REG: BAY3D_HIWD6
        pCfg->lpf_hi_coeff[6] = CLIP(coeff[6], 0, 0x3ff);
        pCfg->lpf_hi_coeff[7] = CLIP(coeff[7], 0, 0x3ff);
        pCfg->lpf_hi_coeff[8] = CLIP(coeff[8], 0, 0x3ff);
    }

    {
        int coeff[9];
        if (pmdDyn->subLoMd1.sw_btnrT_lpfCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss7x5_filter_coeff(pmdDyn->subLoMd1.sw_btnrT_lpf_strg, 5, 7, 7, coeff);
        } else {
            for (i = 0; i < 9; i++)
                coeff[i] = pmdDyn->subLoMd1.hw_btnrT_lpfSpatial_wgt[i];
        }
        // REG: BAY3D_HIWD0
        pCfg->lpf_lo_coeff[0] = CLIP(coeff[0], 0, 0x3ff);
        pCfg->lpf_lo_coeff[1] = CLIP(coeff[1], 0, 0x3ff);
        pCfg->lpf_lo_coeff[2] = CLIP(coeff[2], 0, 0x3ff);
        // REG: BAY3D_HIWD3
        pCfg->lpf_lo_coeff[3] = CLIP(coeff[3], 0, 0x3ff);
        pCfg->lpf_lo_coeff[4] = CLIP(coeff[4], 0, 0x3ff);
        pCfg->lpf_lo_coeff[5] = CLIP(coeff[5], 0, 0x3ff);
        // REG: BAY3D_HIWD6
        pCfg->lpf_lo_coeff[6] = CLIP(coeff[6], 0, 0x3ff);
        pCfg->lpf_lo_coeff[7] = CLIP(coeff[7], 0, 0x3ff);
        pCfg->lpf_lo_coeff[8] = CLIP(coeff[8], 0, 0x3ff);
    }

    {
        int coeff[6];
        if (pmdDyn->mdSigma.sw_btnrT_sigmaIdxFltCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_filter_coeff(pmdDyn->mdSigma.hw_btnrT_sigmaIdxFilt_strg, 5, 5, 5, coeff, (1 << 8));
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = pmdDyn->mdSigma.hw_btnrT_sigmaIdxFilt_coeff[i];
        }
        // REG: BAY3D_GF3
        pCfg->sigma_idx_filt_coeff[0] = CLIP(coeff[0], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[1] = CLIP(coeff[1], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[2] = CLIP(coeff[2], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[3] = CLIP(coeff[3], 0, 0xff);
        // REG: BAY3D_GF4
        pCfg->sigma_idx_filt_coeff[4] = CLIP(coeff[4], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[5] = CLIP(coeff[5], 0, 0xff);
    }
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgtFstLn_negOff) * (1 << 8);
    pCfg->lo_wgt_cal_first_line_sigma_scale = CLIP(tmp, 0, 0xfff);

    // REG: BAY3D_VIIR
    float loWgtVfilt_wgt = 1.0 - (1.0 / pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRFilt_strg);
    tmp = (loWgtVfilt_wgt) * (1 << 4);
    pCfg->lo_diff_vfilt_wgt = CLIP(tmp, 0, 0x1f);
    loWgtVfilt_wgt = 1.0 - (1.0 / pmdDyn->subLoMd1.hw_btnrT_vIIRFilt_strg);
    tmp = (loWgtVfilt_wgt) * (1 << 4);
    pCfg->lo_wgt_vfilt_wgt = CLIP(tmp, 0, 0x1f);
    tmp = (pmdDyn->subLoMd0.sgmCh.hw_btnrT_vIIRFstLn_scale) * (1 << 4);
    pCfg->sig_first_line_scale = CLIP(tmp, 0, 0x3f);
    tmp = (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRFstLn_scale) * (1 << 4);
    pCfg->lo_diff_first_line_scale = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_LFSCL
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_negOff) * (1 << 10);
    pCfg->lo_wgt_cal_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->lo_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_LFSCLTH
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_maxLimit) * (1 << 10);
    pCfg->lo_wgt_cal_max_limit = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_hiNsBase_ratio) * (1 << 10);
    pCfg->mode0_base_ratio = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_DSWGTSCL
    tmp = (pmdDyn->subLoMd0.wgtOpt.hw_btnrT_mdWgt_negOff) * (1 << 10);
    pCfg->lo_diff_wgt_cal_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->subLoMd0.wgtOpt.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->lo_diff_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTlastSCL
    tmp = (pmdDyn->loMd.hw_btnrT_preWgtMge_offset) * (1 << 10);
    pCfg->lo_mge_pre_wgt_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loMd.hw_btnrT_preWgtMge_scale) * (1 << 10);
    pCfg->lo_mge_pre_wgt_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL0
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_loWgtStat_scale) * (1 << 10);
    pCfg->mode0_lo_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_loWgtStatHdrS_scale) * (1 << 10);
    pCfg->mode0_lo_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL1
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStat_scale) * (1 << 10);
    pCfg->mode1_lo_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStatHdrS_scale) * (1 << 10);
    pCfg->mode1_lo_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL2
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->mode1_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgtHdrS_scale) * (1 << 10);
    pCfg->mode1_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTOFF
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStat_negOff) * (1 << 10);
    pCfg->mode1_lo_wgt_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStatHdrS_negOff) * (1 << 10);
    pCfg->mode1_lo_wgt_hdr_sht_offset = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGT1OFF
    tmp = (pdyn->sigmaEnv.hw_btnrT_statsPixAlpha_thred) * (1 << 10);
    pCfg->auto_sigma_count_wgt_thred = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_minLimit) * (1 << 10);
    pCfg->mode1_wgt_min_limit = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_offset) * (1 << 10);
    pCfg->mode1_wgt_offset = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_WGTLO_L
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusion_minLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_min_limit = CLIP(tmp, 0, 0x3fff);
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusionHdrS_minLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_hdr_sht_min_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTLO_H
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusion_maxLimit;
#if 1
    // if iso change, then ob_offset change, the tnr is disable to void the redish image
    static int pre_ob = 0;
    if(cvtinfo->isFirstFrame) {
        pre_ob = cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset;
    }
    if(psta->frmFusion.isoSwitch.sw_btnrT_fusionIso_mode == btnr_limitAdj_mode) {
        int delta_ob = ABS(cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset - pre_ob);
        if(delta_ob >= psta->frmFusion.isoSwitch.sw_btnrT_limitAdj_deltaOB) {
            tmpf = 1;
        }
        if(delta_ob > 0) {
            LOGD_ANR("iso:%d pre_ob:%d cur_ob:%d delta_ob:%d\n", cvtinfo->frameIso[0], pre_ob, cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset, delta_ob);
        }
    }
    pre_ob = cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset;
#endif
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_max_limit = CLIP(tmp, 0, 0x3fff);
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusionHdrS_maxLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_hdr_sht_max_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_STH_SCL
    tmp = (pmdDyn->frmFusion.hw_btnrT_loGgSoftThd_scale) * (1 << 8);
    pCfg->lo_pre_gg_soft_thresh_scale = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->frmFusion.hw_btnrT_loRbSoftThd_scale) * (1 << 8);
    pCfg->lo_pre_rb_soft_thresh_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_STH_LIMIT
    tmp = (pmdDyn->frmFusion.hw_btnrT_loSoftThd_maxLimit);
    pCfg->lo_pre_soft_thresh_max_limit = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->frmFusion.hw_btnrT_loSoftThd_minLimit);
    pCfg->lo_pre_soft_thresh_min_limit = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_HIKEEP
    tmp = (pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_curHiOrg_alpha) * (1 << FIXBITWFWGT);
    pCfg->cur_spnr_hi_wgt_min_limit = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_iirHiOrg_alpha) * (1 << FIXBITWFWGT);
    pCfg->pre_spnr_hi_wgt_min_limit = CLIP(tmp, 0, 0xff);
    tmp = (pmdDyn->memc.hw_btnrT_mcLoWgt_thred) * (1 << 10);
    pCfg->motion_est_lo_wgt_thred = CLIP(tmp, 0, 0x3ff);
    // REG: BAY3D_SIGNUMTH
    tmp = (psta->sigmaEnv.hw_btnrCfg_statsPixCnt_thred);
    pCfg->sigma_num_th = CLIP(tmp, 0, 0xfffff);
    // REG: BAY3D_MONR
    if (pdyn->outFrmFusion.sw_btnrT_outFrmBase_mode == btnr_curBaseOut_mode) {
        float hi_scale = pdyn->outFrmFusion.noiseBal_curBaseOut.sw_btnrT_hiNoiseBalNr_scale;                                                //
        float md_scale = pdyn->outFrmFusion.noiseBal_curBaseOut.sw_btnrT_midNoiseBalNr_scale;                                               //
        float hi_offset = pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_hiNoiseBalNr_off * (1 << 10);
        float md_offset = pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_midNoiseBalNr_off * (1 << 10);
        tmp0 = MIN((sqrt((float)1.0 / pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_hiNoiseBal_strg) * (1 << 10)), (1 << 10));
        tmp1 = MIN((sqrt((float)1.0 / pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_midNoiseBal_strg) * (1 << 10)), (1 << 10));
        pCfg->out_use_hi_noise_bal_nr_strg = MIN((tmp0 + hi_offset) * hi_scale, 2.0 * (1 << 10));
        pCfg->out_use_md_noise_bal_nr_strg = MIN((tmp1 + md_offset) * md_scale, 2.0 * (1 << 10));
        pCfg->out_use_hi_noise_bal_nr_off = hi_offset;
        pCfg->out_use_md_noise_bal_nr_off = md_offset;
    }
    else {
        float hi_scale = pdyn->outFrmFusion.noiseBal_preBaseOut.sw_btnrT_hiNoiseBalNr_scale;                                                //
        float md_scale = pdyn->outFrmFusion.noiseBal_preBaseOut.sw_btnrT_midNoiseBalNr_scale;                                               //
        float hi_offset = pdyn->outFrmFusion.noiseBal_preBaseOut.hw_btnrT_hiNoiseBalNr_off * (1 << 10);
        float md_offset = pdyn->outFrmFusion.noiseBal_preBaseOut.hw_btnrT_midNoiseBalNr_off * (1 << 10);
        tmp0 = MIN((sqrt((float)1.0 / pdyn->outFrmFusion.noiseBal_preBaseOut.hw_btnrT_hiNoiseBal_strg) * (1 << 10)), (1 << 10));
        tmp1 = MIN((sqrt((float)1.0 / pdyn->outFrmFusion.noiseBal_preBaseOut.hw_btnrT_midNoiseBal_strg) * (1 << 10)), (1 << 10));
        pCfg->out_use_hi_noise_bal_nr_strg = MIN((tmp0 + hi_offset) * hi_scale, 2.0 * (1 << 10));
        pCfg->out_use_md_noise_bal_nr_strg = MIN((tmp1 + md_offset) * md_scale, 2.0 * (1 << 10));
        pCfg->out_use_hi_noise_bal_nr_off = hi_offset;
        pCfg->out_use_md_noise_bal_nr_off = md_offset;
    }
    tmp = (pdyn->locSgmStrg.hw_bnrT_locSgmStrg_maxLimit) * (1 << 8);
    pCfg->gain_out_max_limit = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_SIGSCL
    tmp = (pmdDyn->mdSigma.hw_btnrT_sigma_scale) * (1 << 10);
    pCfg->sigma_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->mdSigma.hw_btnrT_sigmaHdrS_scale) * (1 << 10);
    pCfg->sigma_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_DSOFF
    tmp = (pmdDyn->subLoMd1.hw_btnrT_vIIRWgt_offset) * (1 << 10);
    pCfg->lo_wgt_vfilt_offset = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_offset);
    pCfg->lo_diff_vfilt_offset = CLIP(tmp, 0, 0xfff);
    tmp = (0) * (1 << 4);
    pCfg->lo_wgt_cal_first_line_vfilt_wgt = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_DSSCL
    tmp = (pmdDyn->subLoMd1.hw_btnrT_vIIRWgt_scale) * (1 << 4);
    pCfg->lo_wgt_vfilt_scale = CLIP(tmp, 0, 0x3f);
    {
        // calc vfilt scale
        int bayertnr_lodiff_vfilt_scale_bit;
        int bayertnr_lodiff_vfilt_scale;
        for (i = 8; i >= 2; i--) {
            if (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale < (float)1.0 / (1 << i)) {
                bayertnr_lodiff_vfilt_scale_bit = (i + 1) - 2;
                bayertnr_lodiff_vfilt_scale = MIN((int)(pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale * (1 << (i + 1)) + 0.5), 63); //U2.4
                break;
            }
        }
        if(i == 1) {
            bayertnr_lodiff_vfilt_scale_bit = 2 - 2;
            bayertnr_lodiff_vfilt_scale = MIN((int)(pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale * (1 << 2) + 0.5), 63);  //U2.4
        }
        pCfg->lo_diff_vfilt_scale_bit = CLIP(bayertnr_lodiff_vfilt_scale_bit, 0, 0x7);
        pCfg->lo_diff_vfilt_scale = CLIP(bayertnr_lodiff_vfilt_scale, 0, 0x7f);
    }
    tmp = (0) * (1 << 4);
    pCfg->lo_diff_first_line_vfilt_wgt = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_ME0
    tmp = (pmdDyn->memc.hw_btnrT_upMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->memc.hw_btnrT_upMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->memc.hw_btnrT_sadFiltSpatial_wgt[0]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt0 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_ME1
    tmp = (pmdDyn->memc.hw_btnrT_upLeftMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_left_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->memc.hw_btnrT_upLeftMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_left_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->memc.hw_btnrT_sadFiltSpatial_wgt[1]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt1 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_ME2
    tmp = (pmdDyn->memc.hw_btnrT_upRightMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_right_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->memc.hw_btnrT_upRightMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_right_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->memc.hw_btnrT_sadFiltSpatial_wgt[2]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt2 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_WGTMAX
    tmpf = pmdDyn->frmFusion.hw_btnrT_loMotion_maxLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_motion_max_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGT1MAX
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_maxLimit) * (1 << 10);
    pCfg->mode1_wgt_max_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTM0
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_mdWgt_maxLimit) * (1 << 10);
    pCfg->mode0_wgt_out_max_limit = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->mdWgtFilt.hw_btnrT_mdWgt_offset) * (1 << 10);
    pCfg->mode0_wgt_out_offset = CLIP(tmp, 0, 0x3ff);
    if(pmdDyn->hiMd.hw_btnrT_md_mode == btnr_loAsBias_hi1_mode) {
        pCfg->mode0_wgt_out_offset = 0;
    }
    /* BAY3D_LOCOEF0 */
    {
        int coeff[5];
        if (pmdDyn->subLoMd1.sw_btnrT_hFiltCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_hozi_filter_coeff(pmdDyn->subLoMd1.hw_btnrT_hFilt_strg, coeff);
        } else {
            for (i = 0; i < 5; i++)
                coeff[i] = pmdDyn->subLoMd1.hw_btnrT_hFilt_coeff[i];
        }
        pCfg->lo_wgt_hflt_coeff0 = CLIP(coeff[2], 0, 0x1f);
        pCfg->lo_wgt_hflt_coeff1 = CLIP(coeff[1], 0, 0x1f);
        pCfg->lo_wgt_hflt_coeff2 = CLIP(coeff[0], 0, 0x1f);
    }

    {
        int coeff[5];
        if (pmdDyn->subLoMd0.sgmCh.sw_btnrT_hFiltCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_hozi_filter_coeff(pmdDyn->subLoMd0.sgmCh.hw_btnrT_hFilt_strg, coeff);
        } else {
            for (i = 0; i < 5; i++)
                coeff[i] = pmdDyn->subLoMd0.sgmCh.hw_btnrT_hFilt_coeff[i];
        }
        pCfg->sig_hflt_coeff0 = CLIP(coeff[2], 0, 0x1f);
        pCfg->sig_hflt_coeff1 = CLIP(coeff[1], 0, 0x1f);
        pCfg->sig_hflt_coeff2 = CLIP(coeff[0], 0, 0x1f);
    }

    /* BAY3D_LOCOEF1 */
    {
        int coeff[5];
        if (pmdDyn->subLoMd0.diffCh.sw_btnrT_hFiltCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_hozi_filter_coeff(pmdDyn->subLoMd0.diffCh.hw_btnrT_hFilt_strg, coeff);
        } else {
            for (i = 0; i < 5; i++)
                coeff[i] = pmdDyn->subLoMd0.diffCh.hw_btnrT_hFilt_coeff[i];
        }
        pCfg->lo_dif_hflt_coeff0 = CLIP(coeff[2], 0, 0x1f);
        pCfg->lo_dif_hflt_coeff1 = CLIP(coeff[1], 0, 0x1f);
        pCfg->lo_dif_hflt_coeff2 = CLIP(coeff[0], 0, 0x1f);
    }
    /* BAY3D_DPC0 */
    pCfg->pre_spnr_dpc_bright_str = CLIP(pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_brightDpc_strg, 0, 4);
    pCfg->pre_spnr_dpc_dark_str = CLIP(pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_darkDpc_strg, 0, 4);
    pCfg->pre_spnr_dpc_str = CLIP(pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_dpc_strg, 0, 4);
    tmp = (pCfg->pre_spnr_dpc_flt_mode ? pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_scale :
           pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_preWgt_scale) * (1 << 4);
    pCfg->pre_spnr_dpc_wk_scale = CLIP(tmp, 0, 0xff);
    tmp = (pCfg->pre_spnr_dpc_flt_mode ? pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_offset :
           pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_preWgt_offset) * (1 << 10);
    pCfg->pre_spnr_dpc_wk_offset = CLIP(tmp, 0, 0xff);
    /* BAY3D_DPC1 */
    tmp = pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_noiseBal_strg * (1 << 10);
    pCfg->pre_spnr_dpc_nr_bal_str = pCfg->pre_spnr_dpc_flt_mode ? pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_offset * (1 << 10) : tmp;
    tmp = (pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_softThd_scale) * (1 << 8);
    pCfg->pre_spnr_dpc_soft_thr_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PRELOWGT
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_ratAvgRgeWgt_alpha) * (1 << 7);
    pCfg->pre_spnr_lo_val_wgt_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = MAX((1 - pdyn->preSpNr.loNr.hw_btnrT_loNrOut_alpha) * (1 << 7), 0);
    pCfg->pre_spnr_lo_filter_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_filtWgt_minLimit) * (1 << 8);
    pCfg->pre_spnr_lo_filter_wgt_min = CLIP(tmp, 1, 0xff);
    // REG: BAY3D_MIDBIG0
    tmp = (pmdDyn->subDeepLoMd.hw_btnrT_wgt_offset) * (1 << 8);
    pCfg->md_large_lo_md_wgt_offset = CLIP(tmp, 0, 0xff);
    tmp = (pmdDyn->subDeepLoMd.hw_btnrT_wgt_scale) * (1 << 8);
    pCfg->md_large_lo_md_wgt_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_MIDBIG1
    tmp = (pmdDyn->subDeepLoMd.sw_btnrT_wgt2FusionLmt_negOff) * (1 << 8);
    pCfg->md_large_lo_wgt_cut_offset = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->subDeepLoMd.hw_btnrT_wgt2FussionLmt_offset) * (1 << 12);
    pCfg->md_large_lo_wgt_add_offset = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_MIDBIG2
    tmp = (pmdDyn->subDeepLoMd.sw_btnrT_wgt2FusionLmt_scale) * (1 << 8);
    pCfg->md_large_lo_wgt_scale = CLIP(tmp, 0, 0xfff);

    // tnr sigma curve must calculate before spnr sigma
    if(cvtinfo->isFirstFrame || (pTransParams->transf_mode != pCfg->transf_mode)
            || (pTransParams->transf_mode_scale != pCfg->transf_mode_scale)) {
        bayertnr_logtrans_init(pCfg->transf_mode, pCfg->transf_mode_scale, pTransParams);
    }

    bool update_itransf_tbl = false;
    if ((pTransParams->transf_mode != pCfg->transf_mode) ||
            (pTransParams->transf_mode_scale != pCfg->transf_mode_scale) ||
            (pTransParams->isTransfBypass != pCfg->transf_bypass_en) ||
            (pTransParams->transf_data_max_limit != pCfg->transf_data_max_limit) ||
            (pTransParams->itransf_mode_offset != pCfg->itransf_mode_offset) ||
            (pTransParams->transf_mode_offset != pCfg->transf_mode_offset)) {
        update_itransf_tbl = true;
    }

    pTransParams->transf_mode = pCfg->transf_mode;
    pTransParams->transf_mode_scale = pCfg->transf_mode_scale;
    pTransParams->transf_mode_offset = pCfg->transf_mode_offset;
    pTransParams->transf_data_max_limit = pCfg->transf_data_max_limit;
    pTransParams->itransf_mode_offset = pCfg->itransf_mode_offset;

    // REG: BAY3D_PIXMAX
    if(pCfg->transf_bypass_en) {
        pCfg->pix_max_limit  = ((1 << 12) - 1);
    } else {
        if(pCfg->transf_mode_scale) {
            pCfg->pix_max_limit  = bayertnr_logtrans(((1 << 15) - 1), pTransParams);
        } else {
            pCfg->pix_max_limit  = bayertnr_logtrans(((1 << 20) - 1), pTransParams);
        }
    }

    pTransParams->isTransfBypass = pCfg->transf_bypass_en;
    if (cvtinfo->isFirstFrame || update_itransf_tbl) {
        rk_autoblc_gen_tbl(pBlcInfo->bayertnr_itransf_tbl, pCfg->pix_max_limit, pTransParams);
    }

    bayertnr_luma2sigmax_config_v41(pTransParams, &cvtinfo->blc_res, cvtinfo->preDGain);


    bool auto_sig_curve_spnruse = psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_autoSigma_mode;
    sigbins = 20;

    for(i = 0; i < sigbins; i++) {
        pCfg->tnr_luma2sigma_x[i] = CLIP(pTransParams->tnr_luma_sigma_x[i], 0, 0xfff);
    }

    //printf("oyyf tnr sigmax[] :");
    if(pTransParams->isTransfBypass  || opMode == RK_AIQ_OP_MODE_MANUAL) {
        for(i = 0; i < sigbins; i++) {
            pCfg->tnr_luma2sigma_x[i] = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.idx[i];
            //printf("%d ", pCfg->tnr_luma2sigma_x[i]);
        }
    }
    //printf("\n");

    //x_step must be 2^n
    int x_step = 0;
    for(i = 1; i < sigbins; i++) {
        x_step = pCfg->tnr_luma2sigma_x[i] - pCfg->tnr_luma2sigma_x[i - 1];
        x_step = 1 << (FLOOR(LOG2(x_step)));
        tmp = pCfg->tnr_luma2sigma_x[i - 1] + x_step;
        pCfg->tnr_luma2sigma_x[i] = CLIP(tmp, 0, pCfg->pix_max_limit);
    }
    pCfg->tnr_luma2sigma_x[sigbins - 1] =  pCfg->pix_max_limit;



    if(psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_manualSigma_mode || cvtinfo->isFirstFrame || bayertnr_default_noise_curve_use) {
        pTransParams->bayertnr_auto_sig_count_en = 0;

        if( pTransParams->isTransfBypass || opMode == RK_AIQ_OP_MODE_MANUAL) {
            for(i = 0; i < sigbins; i++) {
                pTransParams->tnr_luma_sigma_y[i] = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.val[i];
            }
        } else if(pTransParams->isHdrMode) {
            uint16_t shortY[sigbins];
            int short_iso = cvtinfo->frameIso[1];
            int short_ilow = 0, short_ihigh = 0;
            float short_iso_ratio = 0.0f;

            pre_interp(short_iso, NULL, 0, &short_ilow, &short_ihigh, &short_iso_ratio);

            uint16_t* pSigmaX = pCfg->tnr_luma2sigma_x;
            uint16_t* pSigmaY = pTransParams->tnr_luma_sigma_y;
            uint16_t* pCalibX = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pLongY = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pShortLowY = btnr_attrib->stAuto.spNrDyn[short_ilow].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pShortHighY = btnr_attrib->stAuto.spNrDyn[short_ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.val;

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
            uint16_t* pLowY = btnr_attrib->stAuto.spNrDyn[ilow].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pHighY = btnr_attrib->stAuto.spNrDyn[ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pLowX = btnr_attrib->stAuto.spNrDyn[ilow].sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pHighX = btnr_attrib->stAuto.spNrDyn[ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pSigmaX = pCfg->tnr_luma2sigma_x;
            uint16_t* pSigmaY = pTransParams->tnr_luma_sigma_y;
            bayertnr_sigmaY_interpolate(pSigmaX, pSigmaY, pLowX, pHighX, pLowY, pHighY, sigbins, iso_ratio);
        }
    } else {
        pTransParams->bayertnr_auto_sig_count_en = 1;
        pTransParams->bayertnr_auto_sig_count_filt_wgt = pdyn->sigmaEnv.sw_btnrT_autoSgmIIR_alpha * (1 << 10);
        pTransParams->bayertnr_auto_sig_count_max = cvtinfo->rawWidth * cvtinfo->rawHeight / 3;
        bayertnr_autosigma_config(btnr_stats, pTransParams, &cvtinfo->blc_res);
    }

#if 0
    printf("btnr stats sigmaY [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
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
    int max_sig = ((1 << 12) - 1);
    for(i = 0; i < sigbins; i++) {
        pCfg->tnr_luma2sigma_y[i] = CLIP((int)(pTransParams->tnr_luma_sigma_y[i]), 0, max_sig);
    }

#if 0
    printf("btnr reg sigmaY [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           pCfg->tnr_luma2sigma_y[0],
           pCfg->tnr_luma2sigma_y[1],
           pCfg->tnr_luma2sigma_y[2],
           pCfg->tnr_luma2sigma_y[3],
           pCfg->tnr_luma2sigma_y[4],
           pCfg->tnr_luma2sigma_y[5],
           pCfg->tnr_luma2sigma_y[6],
           pCfg->tnr_luma2sigma_y[7],
           pCfg->tnr_luma2sigma_y[8],
           pCfg->tnr_luma2sigma_y[9],
           pCfg->tnr_luma2sigma_y[10],
           pCfg->tnr_luma2sigma_y[11],
           pCfg->tnr_luma2sigma_y[12],
           pCfg->tnr_luma2sigma_y[13],
           pCfg->tnr_luma2sigma_y[14],
           pCfg->tnr_luma2sigma_y[15],
           pCfg->tnr_luma2sigma_y[16],
           pCfg->tnr_luma2sigma_y[17],
           pCfg->tnr_luma2sigma_y[18],
           pCfg->tnr_luma2sigma_y[19]);
#endif

    // spnr sigma curve, cur bil and iir bil strength
    int spnrsigbins = 16;

    int pix_max = pCfg->transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans((1 << 12) - 1, pTransParams);
    pix_max = cvtinfo->frameNum == 2 ? bayertnr_logtrans((1 << 20) - 1, pTransParams) : pix_max;
    if(cvtinfo->frameNum == 2) {
        // hdr long bins
        int lgbins = 8;
        for(i = 0; i < lgbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 128 * (i + 1);
        }
        // hdr short bins, max gain 256
        for(i = lgbins; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 256 * (i - lgbins + 1) + pCfg->pre_spnr_luma2sigma_x[lgbins - 1];
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;

    } else if (!pCfg->transf_bypass_en) {
#if 0
        for(i = 0; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 64 * (i + 1) ;
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;
#else
        uint32_t blc_remain = 0;
        if(cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset && cvtinfo->blc_res.obcPostTnr.sw_blcT_obcPostTnr_en) {
            blc_remain = cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset;
        }
        int log_pix_max = bayertnr_logtrans(((1 << 12) * cvtinfo->preDGain - 1), pTransParams);
        int log_ob_offset = bayertnr_logtrans(blc_remain, pTransParams);

        pCfg->pre_spnr_luma2sigma_x[0] = (log_ob_offset + 128);
        int step = 0;
        for(i = 1; i < spnrsigbins; i++) {
            step = (log_pix_max - pCfg->pre_spnr_luma2sigma_x[i - 1]) / (spnrsigbins - i);
            if(i != spnrsigbins - 1) {
                step = 1 << (ROUND_F(LOG2(step)));
            } else {
                step = 1 << (FLOOR(LOG2(step)));
            }
            tmp = pCfg->pre_spnr_luma2sigma_x[i - 1] + step;
            pCfg->pre_spnr_luma2sigma_x[i] = CLIP(tmp, 0, log_pix_max);
#if 0
            printf("spnr sigmax idx:%d step:%d stepfinnal:%d x[%d]=%d clip:%u\n",
                   i, (log_pix_max - pCfg->pre_spnr_luma2sigma_x[i - 1]) / (spnrsigbins - i), step, i, tmp,  pCfg->pre_spnr_luma2sigma_x[i]);
#endif
        }
#endif
    } else {
        for(i = 0; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 256 * (i + 1) ;
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;
    }

    //x step must 2^n
    for(i = 1; i < spnrsigbins; i++) {
        x_step = pCfg->pre_spnr_luma2sigma_x[i] - pCfg->pre_spnr_luma2sigma_x[i - 1];
        x_step = 1 << (FLOOR(LOG2(x_step)));
        tmp = pCfg->pre_spnr_luma2sigma_x[i - 1] + x_step;
        pCfg->pre_spnr_luma2sigma_x[i] = CLIP(tmp, 0, pCfg->pix_max_limit);
    }
    pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;
#if 0
    printf("spnr sigmax[%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           pCfg->pre_spnr_luma2sigma_x[0],
           pCfg->pre_spnr_luma2sigma_x[1],
           pCfg->pre_spnr_luma2sigma_x[2],
           pCfg->pre_spnr_luma2sigma_x[3],
           pCfg->pre_spnr_luma2sigma_x[4],
           pCfg->pre_spnr_luma2sigma_x[5],
           pCfg->pre_spnr_luma2sigma_x[6],
           pCfg->pre_spnr_luma2sigma_x[7],
           pCfg->pre_spnr_luma2sigma_x[8],
           pCfg->pre_spnr_luma2sigma_x[9],
           pCfg->pre_spnr_luma2sigma_x[10],
           pCfg->pre_spnr_luma2sigma_x[11],
           pCfg->pre_spnr_luma2sigma_x[12],
           pCfg->pre_spnr_luma2sigma_x[13],
           pCfg->pre_spnr_luma2sigma_x[14],
           pCfg->pre_spnr_luma2sigma_x[15]);
#endif

    if(!auto_sig_curve_spnruse || cvtinfo->isFirstFrame) {
        if(pTransParams->isTransfBypass || opMode == RK_AIQ_OP_MODE_MANUAL) {
            for(i = 0; i < spnrsigbins; i++) {
                pCfg->pre_spnr_luma2sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.val[i]), 0, max_sig);
                pCfg->pre_spnr_luma2sigma_x[i] = pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.idx[i];
            }
        } else if(pTransParams->isHdrMode ) {
            uint16_t shortY[spnrsigbins];
            int short_iso = cvtinfo->frameIso[1];
            int short_ilow = 0, short_ihigh = 0;
            float short_iso_ratio = 0.0f, merge_luma2Wgt = 0.0f;

            pre_interp(short_iso, NULL, 0, &short_ilow, &short_ihigh, &short_iso_ratio);

            uint16_t* pSigmaX = pCfg->pre_spnr_luma2sigma_x;
            uint16_t* pSigmaY = pCfg->pre_spnr_luma2sigma_y;
            uint16_t* pLongY = pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
            uint16_t* pCalibX = pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.idx;
            uint16_t* pShortLowY = btnr_attrib->stAuto.spNrDyn[short_ilow].sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
            uint16_t* pShortHighY = btnr_attrib->stAuto.spNrDyn[short_ihigh].sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
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
            uint16_t* pLowY = btnr_attrib->stAuto.spNrDyn[ilow].sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
            uint16_t* pHighY = btnr_attrib->stAuto.spNrDyn[ihigh].sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
            uint16_t* pLowX = btnr_attrib->stAuto.spNrDyn[ilow].sigmaEnv.hw_btnrC_preSpNrSgm_curve.idx;
            uint16_t* pHighX = btnr_attrib->stAuto.spNrDyn[ihigh].sigmaEnv.hw_btnrC_preSpNrSgm_curve.idx;
            uint16_t* pSigmaX = pCfg->pre_spnr_luma2sigma_x;
            uint16_t* pSigmaY = pCfg->pre_spnr_luma2sigma_y;
            bayertnr_sigmaY_interpolate(pSigmaX, pSigmaY, pLowX, pHighX, pLowY, pHighY, spnrsigbins, iso_ratio);
        }
    } else {
        for(i = 0; i < spnrsigbins; i++) {
            tmp0 = bayertnr_tnr_noise_curve(pCfg->pre_spnr_luma2sigma_x[i], 0, pTransParams);
            pCfg->pre_spnr_luma2sigma_y[i] = CLIP((int)(tmp0), 0, max_sig);
        }
    }


#if 0
    printf("spnr btnr reg sigmaY [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           pCfg->pre_spnr_luma2sigma_y[0],
           pCfg->pre_spnr_luma2sigma_y[1],
           pCfg->pre_spnr_luma2sigma_y[2],
           pCfg->pre_spnr_luma2sigma_y[3],
           pCfg->pre_spnr_luma2sigma_y[4],
           pCfg->pre_spnr_luma2sigma_y[5],
           pCfg->pre_spnr_luma2sigma_y[6],
           pCfg->pre_spnr_luma2sigma_y[7],
           pCfg->pre_spnr_luma2sigma_y[8],
           pCfg->pre_spnr_luma2sigma_y[9],
           pCfg->pre_spnr_luma2sigma_y[10],
           pCfg->pre_spnr_luma2sigma_y[11],
           pCfg->pre_spnr_luma2sigma_y[12],
           pCfg->pre_spnr_luma2sigma_y[13],
           pCfg->pre_spnr_luma2sigma_y[14],
           pCfg->pre_spnr_luma2sigma_y[15]);
#endif

    // REG: BAY3D_SIGORG
    pTransParams->bayertnr_lo_wgt_clip_min_limit = pCfg->lo_wgt_clip_min_limit ;
    pTransParams->bayertnr_lo_wgt_clip_max_limit = pCfg->lo_wgt_clip_max_limit;
    pCfg->tnr_out_sigma_sq  = bayertnr_update_sq(pTransParams);

    if (!pCfg->transf_bypass_en) {
        rk_aiq_btnr42_params_logtrans(pCfg);
    }

    // test
    if (pCfg->btnr_ldc_en) {
        pCfg->btnr_ldc_en = 1;

        pCfg->iir_rw_fmt = 3;
        pCfg->b3dldcv_map13p3_en=0;
        pCfg->b3dldcv_force_map_en=0;
        /* B3DLDC_ADR_STS */
        pCfg->b3dldch_en=1;
        pCfg->b3dldch_map13p3_en=0;
        pCfg->b3dldch_force_map_en=0;
        /* B3DLDC_EXTBOUND1 */
        pCfg->btnr_ldcltp_mode=1;
        pCfg->btnr_ldc_wrap_ext_bound_offset=0x0;
    } else {
        pCfg->btnr_ldc_en = 0;
        pCfg->b3dldch_en = 0;
    }

    return;
}

void rk_aiq_btnr42_l2_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo, btnr_cvt_info_t* pBtnrInfo, mergeLuma2Wgt_t* pMergeLumaWgt)
{
    btnr_trans_params_t *pTransParams = &pBtnrInfo->mBtnrTransParams;
    btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[0];

    btnr_api_attrib_t *btnr_attrib = pBtnrInfo->btnr_attrib;
    btnr_param_auto_t *paut = &btnr_attrib->stAuto;
    rk_aiq_op_mode_t opMode = btnr_attrib->opMode;
#if 0
    printf("hw_btnrT_sigma_scale %f %f\n",
           paut->mdMeDyn[0].mdSigma.hw_btnrT_sigma_scale, paut->mdMeDyn[1].mdSigma.hw_btnrT_sigma_scale);
#endif


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
    blc_cvt_info_t* pBlcInfo = (blc_cvt_info_t*)cvtinfo->pBlcInfo;
    float sigma_ratio = ((float)btnr_stats->sigma_num) / (cvtinfo->rawWidth * cvtinfo->rawHeight);
    pBlcInfo->sigma_ratio = sigma_ratio;

    float tmpf;
    int i, j, tmp, tmp0, tmp1, sigbins, halfx;
    struct isp35_bay3d_cfg *pCfg = &isp_params->isp_cfg->others.bay3d_l2_cfg;

    btnr_param_t *btnr_param = (btnr_param_t *) attr;
    btnr_params_static_t* psta = &btnr_param->sta;
    btnr_other_dyn_t* pdyn = &btnr_param->spNrDyn;
    btnr_mdMe_dyn_t* pmdDyn = &btnr_param->mdMeDyn;

    bool bayertnr_default_noise_curve_use = false;
    int bayertnr_iso_cur = cvtinfo->frameIso[cvtinfo->frameNum - 1];
    bayertnr_default_noise_curve_use = bayertnr_find_top_one_pos(bayertnr_iso_cur) != bayertnr_find_top_one_pos(pTransParams->bayertnr_iso_pre);
    pTransParams->bayertnr_iso_pre = bayertnr_iso_cur;
    pTransParams->isFirstFrame = cvtinfo->isFirstFrame;
    pTransParams->isHdrMode = cvtinfo->frameNum == 2;

    if(psta->hw_btnrCfg_pixDomain_mode != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_pixDomain_mode
            || psta->transCfg.hw_btnrCfg_trans_mode != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_mode
            || psta->transCfg.hw_btnrCfg_trans_offset != pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_offset) {
        LOGW_ANR("Btnr run in pixLog2Domain is %d, trans_mode is %d, trans_offset is %d.\n"
                 "But calib pixLog2Domain_mode is %d, trans_mode is %d, trans_offset is %d.\n",
                 psta->hw_btnrCfg_pixDomain_mode, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_pixDomain_mode,
                 psta->transCfg.hw_btnrCfg_trans_mode, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_mode,
                 psta->transCfg.hw_btnrCfg_trans_offset, pdyn->sigmaEnv.hw_btnrC_sigmaAttrib.hw_btnrCfg_trans_offset);
    }

    if (cvtinfo->frameNum > 1) {
        if (psta->hw_btnrCfg_pixDomain_mode != btnr_pixLog2Domain_mode) {
            LOGW_ANR("Btnr must run in pixLog2Domain(ori mode is %d) when isp is HDR mode(framenum=%d), btnr_pixLog2Domain_mode is be forcibly set to hw_btnrCfg_pixDomain_mode in HWI\n"
                     "You can set by btnr.static.hw_btnrCfg_pixDomain_mode, but change hw_btnrCfg_pixDomain_mode in running time may cause abnormal image transitions\n",
                     psta->hw_btnrCfg_pixDomain_mode, cvtinfo->frameNum, psta->hw_btnrCfg_pixDomain_mode);
            psta->hw_btnrCfg_pixDomain_mode = btnr_pixLog2Domain_mode;
        }
        if (psta->transCfg.hw_btnrCfg_trans_mode != btnr_pixInBw20b_mode) {
            LOGE_ANR("hw_btnrCfg_trans_mode == %s(0x%x) is error. When isp is in HDR mode, btnr must run in 'btnr_pixInBw20b_mode'. "
                     "The trans_mode will be forcibly set to 'btnr_pixInBw20b_mode' in HWI.",
                     trans_mode2str(psta->transCfg.hw_btnrCfg_trans_mode), psta->transCfg.hw_btnrCfg_trans_mode);
            psta->transCfg.hw_btnrCfg_trans_mode = btnr_pixInBw20b_mode;
        }
    }

    if(cvtinfo->frameNum == 1 && cvtinfo->preDGain > 1.0) {
        if (psta->hw_btnrCfg_pixDomain_mode != btnr_pixLog2Domain_mode) {
            LOGW_ANR("Btnr must run in pixLog2Domain(ori mode is %d) when predgain > 1.0, btnr_pixLog2Domain_mode is be forcibly set to hw_btnrCfg_pixDomain_mode in HWI\n",
                     psta->hw_btnrCfg_pixDomain_mode);
            psta->hw_btnrCfg_pixDomain_mode = btnr_pixLog2Domain_mode;
        }
    }

    if (cvtinfo->use_aiisp) {
        if (psta->transCfg.hw_btnrCfg_trans_mode != btnr_pixInBw20b_mode) {
            LOGE_ANR("hw_btnrCfg_trans_mode == %s(0x%x) is error. When isp is in Aiisp mode, btnr must run in 'btnr_pixInBw20b_mode'. "
                     "The trans_mode will be forcibly set to 'btnr_pixInBw20b_mode' in HWI.",
                     trans_mode2str(psta->transCfg.hw_btnrCfg_trans_mode), psta->transCfg.hw_btnrCfg_trans_mode);
            psta->transCfg.hw_btnrCfg_trans_mode = btnr_pixInBw20b_mode;
        }
    }

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
        //printf("exp idx%d: time:%f, gain:%f, all:%f\n", i, frameEt[i], frameiso[i], fdGain[i]);
    }
    for (i = 0; i < framenum; i++) {
        fdGain[i] = fdGain[framenum - 1] / fdGain[i];
    }

    //printf("fdGain[0]:%f\n", fdGain[0]);
    // REG: BAY3D_CTRL0
    if (pdyn->outFrmFusion.sw_btnrT_outFrmBase_mode == btnr_curBaseOut_mode) {
        pCfg->out_use_pre_mode = pdyn->outFrmFusion.noiseBal_curBaseOut.sw_btnrT_noiseBal_mode;
    } else {
        pCfg->out_use_pre_mode = pdyn->outFrmFusion.noiseBal_preBaseOut.sw_btnrT_noiseBal_mode;
    }
    //pCfg->out_use_pre_mode = CLIP(pdyn->hw_btnrT_noiseBal_mode, 0, 0x7);
    pCfg->motion_est_en = cvtinfo->frameId > 1 ? pmdDyn->memc.hw_btnrT_memc_en : 0;
#if 1
    // TODO: for aiisp
    pCfg->iir_wr_src = 0;
    pCfg->iir_rw_fmt = 2;

    if (psta->hw_btnrCfg_pixDomain_mode == btnr_pixLog2Domain_mode &&
            !cvtinfo->use_aiisp)
        pCfg->iir_rw_fmt = 0; // could be 0~3, preferred to be 0 to save bandwidth

    if (psta->hw_btnrCfg_pixDomain_mode == btnr_pixLinearDomain_mode &&
            cvtinfo->frameNum == 1)
        pCfg->iir_rw_fmt = 2; // should be 2 by hardware restriction

    if (pCfg->btnr_ldc_en)//TODO
        pCfg->iir_rw_fmt = 3; // should be 3 by hardware restriction

    // check conflictions
    if (pCfg->btnr_ldc_en && cvtinfo->use_aiisp && (cvtinfo->frameNum > 1))
        LOGE("btnr_ldc_en and aiisp_hdr_en can't be enabled simultaneously");
#endif

    // REG: BAY3D_CTRL1
    switch (psta->hw_btnrCfg_pixDomain_mode) {
    case btnr_pixLog2Domain_mode :
        pCfg->transf_bypass_en = 0;
        pCfg->transf_mode  = 0;
        break;
    case btnr_pixLinearDomain_mode:
        pCfg->transf_bypass_en = 1;
        pCfg->transf_mode  = 0;
        break;
    }
    switch (pmdDyn->loMd.hw_btnrT_loMd_mode) {
    case btnr_allSubLoMdMix_mode:
        pCfg->lo_mge_wgt_mode = 0;
        break;
    case btnr_subLoMd0Only_mode:
        pCfg->lo_mge_wgt_mode = 1;
        break;
    case btnr_subLoMd1Only_mode:
        pCfg->lo_mge_wgt_mode = 2;
        break;
    case btnr_subDeepLoMdOnly_mode:
        pCfg->lo_mge_wgt_mode = 3;
        break;
    }
    if (psta->debug.sw_btnrT_dbgOut_en) {
        switch (psta->debug.hw_btnrT_dbgOut_mode) {
        case btnr_dbgOut_preSpNr_mode:
            pCfg->cur_spnr_out_en = 0;
            pCfg->pre_spnr_out_en = 1;
            pCfg->md_wgt_out_en = 0;
            break;
        case btnr_dbgOut_curSpNr_mode :
            pCfg->cur_spnr_out_en = 1;
            pCfg->pre_spnr_out_en = 0;
            pCfg->md_wgt_out_en = 0;
            break;
        case btnr_dbgOut_mdWgt_mode:
            pCfg->cur_spnr_out_en = 0;
            pCfg->pre_spnr_out_en = 0;
            pCfg->md_wgt_out_en = 1;
            break;
        }
    } else {
        pCfg->md_wgt_out_en = 0;
        pCfg->pre_spnr_out_en = 0;
        pCfg->cur_spnr_out_en = 0;
    }

    if (pmdDyn->subDeepLoMd.sw_btnrT_dLoSrc_mode == btnr_subLoMd1_mode)
        pCfg->md_large_lo_use_mode = 0;
    else
        pCfg->md_large_lo_use_mode = 1;

    pCfg->tnrsigma_curve_double_en =  (cvtinfo->frameNum > 1) ? 1 : 0;
    pCfg->md_large_lo_min_filter_bypass_en = !pmdDyn->subDeepLoMd.hw_btnrT_minFilt_en;
    pCfg->md_large_lo_gauss_filter_bypass_en = !pmdDyn->subDeepLoMd.hw_btnrT_lpf_en;
    pCfg->md_large_lo_md_wgt_bypass_en = !pmdDyn->subDeepLoMd.hw_btnrT_dLoMd_en;
    pCfg->pre_pix_out_mode = pmdDyn->frmFusion.hw_btnrT_frmFusion_mode;
    pCfg->lpf_hi_bypass_en = !pmdDyn->hiMd.hw_btnrT_hfLpf_en;
    pCfg->lo_diff_vfilt_bypass_en = !pmdDyn->subLoMd0.diffCh.hw_btnrT_vFilt_en;
    pCfg->lpf_lo_bypass_en = !pmdDyn->subLoMd1.hw_btnrT_lpf_en;
    pCfg->lo_wgt_hfilt_en = pmdDyn->subLoMd1.hw_btnrT_hFilt_en;
    pCfg->lo_diff_hfilt_en = pmdDyn->subLoMd0.diffCh.hw_btnrT_hFilt_en;
    pCfg->sig_hfilt_en = pmdDyn->subLoMd0.sgmCh.hw_btnrT_hFilt_en;
    pCfg->motion_detect_bypass_en = !pmdDyn->hw_btnrT_md_en;
    pCfg->md_only_lo_en = !pmdDyn->loAsBias_hiMd1.hw_btnrT_hiMd_en;
    pCfg->lo_detection_bypass_en = !pmdDyn->loMd.hw_btnrT_loMd_en;

    // REG: BAY3D_CTRL2
    if (pdyn->preSpNr.sigma.hw_btnrT_sigma_mode == btnr_lutSgmOnly_mode)
        pCfg->spnr_presigma_use_en = 0;
    else
        pCfg->spnr_presigma_use_en = 1;

    if (pdyn->preSpNr.sigma.hw_btnrT_sigmaCurve_mode == btnr_midSegmInterpOn_mode)
        pCfg->pre_spnr_sigma_curve_double_en = 0;
    else
        pCfg->pre_spnr_sigma_curve_double_en = 1;
    if (cvtinfo->frameNum > 1) {
        if (pCfg->pre_spnr_sigma_curve_double_en == 0) {
            if (cvtinfo->btnr_warning_count < 5) {
                LOGW_ANR("When isp is HDR mode, hw_btnrT_sigmaCurve_mode recommends using btnr_midSegmInterpOff_mode. "
                         "You can set by dyn.curFrmSpNr.hw_btnrT_sigmaCurve_mode");
            }
            else if (cvtinfo->btnr_warning_count % 300 == 0) {
                LOGW_ANR("When isp is HDR mode, hw_btnrT_sigmaCurve_mode recommends using btnr_midSegmInterpOff_mode. "
                         "You can set by dyn.curFrmSpNr.hw_btnrT_sigmaCurve_mode");
            }
            cvtinfo->btnr_warning_count++;
            // pCfg->pre_spnr_sigma_curve_double_en  = 1;
        }
    }

    if (pdyn->preSpNr.sigma.hw_btnrT_idxLpfStrg_mode == btnr_lpfStrgH_mode)
        pCfg->pre_spnr_sigma_idx_filt_mode = 0;
    else
        pCfg->pre_spnr_sigma_idx_filt_mode = 1;

    switch (pdyn->preSpNr.hiNr.sw_btnrT_rgeWgt_mode) {
    case btnr_bayerAvg_mode:
        pCfg->pre_spnr_hi_filter_wgt_mode = 0;
        pCfg->pre_spnr_hi_filter_rb_wgt_mode = 0;
        break;
    case btnr_gIndividual_rbAvg_mode :
        pCfg->pre_spnr_hi_filter_wgt_mode = 1;
        pCfg->pre_spnr_hi_filter_rb_wgt_mode = 0;
        break;
    case btnr_bayerIndividual_mode :
        pCfg->pre_spnr_hi_filter_wgt_mode = 1;
        pCfg->pre_spnr_hi_filter_rb_wgt_mode = 1;
        break;
    }
    switch (pdyn->preSpNr.loNr.sw_btnrT_rgeWgt_mode) {
    case btnr_bayerAvg_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 0;
        pCfg->pre_spnr_lo_filter_rb_wgt_mode = 0;
        break;
    case btnr_gIndividual_rbAvg_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 1;
        pCfg->pre_spnr_lo_filter_rb_wgt_mode = 0;
        break;
    case btnr_bayerIndividual_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 1;
        pCfg->pre_spnr_lo_filter_rb_wgt_mode = 1;
        break;
    }

    switch (pdyn->preSpNr.hiNr.hw_btnrT_gic_mode) {
    case btnr_gicDisable_mode :
        pCfg->pre_spnr_hi_filter_gic_en = 0;
        pCfg->pre_spnr_hi_filter_gic_enhance_en = 0;
        break;
    case btnr_gicStrgL_mode :
        pCfg->pre_spnr_hi_filter_gic_en = 1;
        pCfg->pre_spnr_hi_filter_gic_enhance_en = 0;
        break;
    case btnr_gicStrgH_mode :
        pCfg->pre_spnr_hi_filter_gic_en = 1;
        pCfg->pre_spnr_hi_filter_gic_enhance_en = 1;
        break;
    }
    pCfg->cur_spnr_filter_bypass_en = !(pdyn->curSpNr.hw_btnrT_spNr_en && pdyn->curSpNr.hw_btnrT_hiFilter_en);
    pCfg->pre_spnr_lo_filter_bypass_en = !(pdyn->preSpNr.loNr.hw_btnrT_loNr_en && pdyn->preSpNr.sw_btnrT_spNr_en);
    pCfg->pre_spnr_hi_filter_bypass_en = !(pdyn->preSpNr.hiNr.hw_btnrT_hiNr_en && pdyn->preSpNr.sw_btnrT_spNr_en);
    pCfg->pre_spnr_hi_guide_filter_bypass_en = !pdyn->preSpNr.hiNr.hw_btnrT_guideLpf_en;
    pCfg->pre_spnr_sigma_idx_filt_bypass_en = !pdyn->preSpNr.sigma.hw_btnrT_sigmaIdxLpf_en;
    pCfg->pre_spnr_hi_noise_ctrl_en = pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_en;
    pCfg->pre_hi_gic_lp_en =  psta->lowPowerCfg.preSpnrLP.hiNrLP.hw_btnrCfg_gicLP_en;
    pCfg->pre_hi_bf_lp_en =  psta->lowPowerCfg.preSpnrLP.hiNrLP.hw_btnrCfg_bfLP_en;
    pCfg->pre_lo_avg_lp_en =  psta->lowPowerCfg.preSpnrLP.loNrLP.hw_btnrCfg_avgRgeWgtLP_en;
    pCfg->pre_spnr_dpc_flt_en =  pdyn->preDpc.hw_btnrT_dpc_en;
    pCfg->pre_spnr_dpc_flt_mode = CLIP(pdyn->preDpc.hw_btnrT_dpc_mode, 1, 1); //ONLY SUPPORT btnr_nsBalSoftThd_mode
    pCfg->pre_spnr_dpc_nr_bal_mode = pCfg->pre_spnr_dpc_flt_mode ?
                                     pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_mode : pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_preWgt_mode;
    pCfg->pre_spnr_dpc_flt_prewgt_en = pCfg->pre_spnr_dpc_flt_mode ?
                                       pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_en : pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_preWgt_en;

    // REG: BAY3D_CTRL3
    if (pmdDyn->hiMd.hw_btnrT_md_mode == btnr_loAsRatio_hi0_mode) {
        pCfg->wgt_cal_mode = 0;
    } else {
        pCfg->wgt_cal_mode = 1;
    }

    if (psta->hw_btnrCfg_megWgtDs_mode == btnr_megWgtDs_avg_mode)
        pCfg->mge_wgt_ds_mode = 0;
    else
        pCfg->mge_wgt_ds_mode = 1;

    pCfg->transf_mode = 0;
    pCfg->kalman_wgt_ds_mode = CLIP(psta->hw_btnrCfg_kalWgtDs_mode, 0, 0x3);
    pCfg->mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;
    pCfg->sigma_calc_mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;

    // REG: BAY3D_TRANS0
    if (psta->transCfg.hw_btnrCfg_trans_mode == btnr_pixInBw15b_mode)
        pCfg->transf_mode_scale = 1;
    else
        pCfg->transf_mode_scale = 0;

    tmp = psta->transCfg.hw_btnrCfg_trans_offset;
    int n  = bayertnr_find_top_one_pos(tmp);
    if(pCfg->transf_mode_scale == 1) {
        n = 8;
    } else {
        if(n < 8)
            n = 8;
        if(n > 12)
            n = 12;
    }
    tmp = (1 << n);
    pCfg->transf_mode_offset = CLIP(tmp, 0, 0x1fff);

    if (pCfg->transf_mode_scale == 1) {
        pCfg->itransf_mode_offset = (1 << 9) * n;
    } else {
        pCfg->itransf_mode_offset = (1 << 8) * n;
    }

    // REG: BAY3D_TRANS1
    // mode scale只有0�?两种选择，对应log域的小数精度�?    // 0对应8bit小数�?bit整数�?4bit整数导致最大线性输入数据位宽为20bit->transData_maxlimit�?�?0次方-1
    // 1对应9bit小数�?bit整数�?3bit整数限制最大线性输入数据位宽为15bit->transData_maxlimit�?�?5次方-1
    if(pCfg->transf_bypass_en == 1) {
        pCfg->transf_data_max_limit = ((1 << 12) - 1);
    } else {
        if (pCfg->transf_mode_scale == 1) {
            pCfg->transf_data_max_limit = ((1 << 15) - 1);
        }
        else {
            pCfg->transf_data_max_limit = ((1 << 20) - 1);
        }
    }

    // REG: BAY3D_PREHI_SIGSCL
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_preSigma_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_ctrl_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PREHI_SIGOF
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideLpf3_alpha) * (1 << 6);
    pCfg->pre_spnr_hi_guide_out_wgt = CLIP(tmp, 0, 0x7f);

    {
        int coeff[6];
        if (pdyn->curSpNr.sw_btnrT_filtCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_filter_coeff(pdyn->curSpNr.sw_btnrT_filtSpatial_strg, 5, 5, 5, coeff, (1 << 8));
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = pdyn->curSpNr.sw_btnrT_filtSpatial_wgt[i];
        }
        // REG: BAY3D_CURHISPW0
        pCfg->cur_spnr_filter_coeff[0] = CLIP(coeff[0], 0, 0xff);
        pCfg->cur_spnr_filter_coeff[1] = CLIP(coeff[1], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[2] = CLIP(coeff[2], 0, 0x3f);
        // REG: BAY3D_CURHISPW1
        pCfg->cur_spnr_filter_coeff[3] = CLIP(coeff[3], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[4] = CLIP(coeff[4], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[5] = CLIP(coeff[5], 0, 0x3f);
    }


    // REG: BAY3D_PREHI_SIGSCL
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_rgeSgm_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_hi_sigma_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PREHI_WSCL
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_diffSgmRatio_scale) * (1 << 6);
    pCfg->pre_spnr_hi_wgt_calc_scale = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_PREHIWMM
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_filtWgt_minLimit) * (1 << 8);
    pCfg->pre_spnr_hi_filter_wgt_min_limit = CLIP(tmp, 1, 0xff);

    pCfg->pre_spnr_hi_wgt_calc_offset = 0;
    // REG: BAY3D_PREHISIGOF
    //tmp = MAX((1 - pdyn->preSpNr.hiNr.hw_btnrT_hiNrOut_alpha) * (1 << 7), 0);
    tmp = 0;
    pCfg->pre_spnr_hi_filter_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigma_offset) * (1 << 0);
    pCfg->pre_spnr_sigma_offset = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigmaHdrS_offset) * (1 << 0);
    pCfg->pre_spnr_sigma_hdr_sht_offset = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_PREHISIGSCL
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigma_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigmaHdrS_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);

    {
        int coeff[6];
        if (pdyn->preSpNr.hiNr.sw_btnrT_filtCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_spnr_coeff(pdyn->preSpNr.hiNr.sw_btnrT_filtSpatial_strg, 5, 5, 5, coeff);
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = pdyn->preSpNr.hiNr.sw_btnrT_filtSpatial_wgt[i];
        }
        // REG: BAY3D_PREHISPW0
        pCfg->pre_spnr_hi_filter_coeff[0] = CLIP(coeff[0], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[1] = CLIP(coeff[1], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[2] = CLIP(coeff[2], 0, 0x3f);
        // REG: BAY3D_PREHISPW1
        pCfg->pre_spnr_hi_filter_coeff[3] = CLIP(coeff[3], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[4] = CLIP(coeff[4], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[5] = CLIP(coeff[5], 0, 0x3f);
    }

    // REG: BAY3D_PRELOSIGCSL
    tmp                           = (pdyn->preSpNr.loNr.hw_btnrT_rgeSgm_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_lo_sigma_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PRELOSIGOF
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_rgeWgt_scale) * (1 << 6);
    pCfg->pre_spnr_lo_wgt_calc_scale = CLIP(tmp, 0, 0xff);

    pCfg->pre_spnr_lo_wgt_calc_offset = 0;
    // REG: BAY3D_PREHI_NRCT
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_scale) * (1 << 7);
    pCfg->pre_spnr_hi_noise_ctrl_scale = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_offset) * (1 << 7);
    pCfg->pre_spnr_hi_noise_ctrl_offset = CLIP(tmp, 0, 0x7f);


    {
        int coeff[9];
        if (pmdDyn->hiMd.sw_btnrT_hfLpfCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss7x5_filter_coeff(pmdDyn->hiMd.sw_btnrT_hfLpf_strg, 5, 7, 7, coeff);
        } else {
            for (i = 0; i < 9; i++)
                coeff[i] = pmdDyn->hiMd.hw_btnrT_hfLpfSpatial_wgt[i];
        }
        // REG: BAY3D_HIWD0
        pCfg->lpf_hi_coeff[0] = CLIP(coeff[0], 0, 0x3ff);
        pCfg->lpf_hi_coeff[1] = CLIP(coeff[1], 0, 0x3ff);
        pCfg->lpf_hi_coeff[2] = CLIP(coeff[2], 0, 0x3ff);
        // REG: BAY3D_HIWD3
        pCfg->lpf_hi_coeff[3] = CLIP(coeff[3], 0, 0x3ff);
        pCfg->lpf_hi_coeff[4] = CLIP(coeff[4], 0, 0x3ff);
        pCfg->lpf_hi_coeff[5] = CLIP(coeff[5], 0, 0x3ff);
        // REG: BAY3D_HIWD6
        pCfg->lpf_hi_coeff[6] = CLIP(coeff[6], 0, 0x3ff);
        pCfg->lpf_hi_coeff[7] = CLIP(coeff[7], 0, 0x3ff);
        pCfg->lpf_hi_coeff[8] = CLIP(coeff[8], 0, 0x3ff);
    }

    {
        int coeff[9];
        if (pmdDyn->subLoMd1.sw_btnrT_lpfCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss7x5_filter_coeff(pmdDyn->subLoMd1.sw_btnrT_lpf_strg, 5, 7, 7, coeff);
        } else {
            for (i = 0; i < 9; i++)
                coeff[i] = pmdDyn->subLoMd1.hw_btnrT_lpfSpatial_wgt[i];
        }
        // REG: BAY3D_HIWD0
        pCfg->lpf_lo_coeff[0] = CLIP(coeff[0], 0, 0x3ff);
        pCfg->lpf_lo_coeff[1] = CLIP(coeff[1], 0, 0x3ff);
        pCfg->lpf_lo_coeff[2] = CLIP(coeff[2], 0, 0x3ff);
        // REG: BAY3D_HIWD3
        pCfg->lpf_lo_coeff[3] = CLIP(coeff[3], 0, 0x3ff);
        pCfg->lpf_lo_coeff[4] = CLIP(coeff[4], 0, 0x3ff);
        pCfg->lpf_lo_coeff[5] = CLIP(coeff[5], 0, 0x3ff);
        // REG: BAY3D_HIWD6
        pCfg->lpf_lo_coeff[6] = CLIP(coeff[6], 0, 0x3ff);
        pCfg->lpf_lo_coeff[7] = CLIP(coeff[7], 0, 0x3ff);
        pCfg->lpf_lo_coeff[8] = CLIP(coeff[8], 0, 0x3ff);
    }

    {
        int coeff[6];
        if (pmdDyn->mdSigma.sw_btnrT_sigmaIdxFltCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_filter_coeff(pmdDyn->mdSigma.hw_btnrT_sigmaIdxFilt_strg, 5, 5, 5, coeff, (1 << 8));
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = pmdDyn->mdSigma.hw_btnrT_sigmaIdxFilt_coeff[i];
        }
        // REG: BAY3D_GF3
        pCfg->sigma_idx_filt_coeff[0] = CLIP(coeff[0], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[1] = CLIP(coeff[1], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[2] = CLIP(coeff[2], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[3] = CLIP(coeff[3], 0, 0xff);
        // REG: BAY3D_GF4
        pCfg->sigma_idx_filt_coeff[4] = CLIP(coeff[4], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[5] = CLIP(coeff[5], 0, 0xff);
    }
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgtFstLn_negOff) * (1 << 8);
    pCfg->lo_wgt_cal_first_line_sigma_scale = CLIP(tmp, 0, 0xfff);

    // REG: BAY3D_VIIR
    float loWgtVfilt_wgt = 1.0 - (1.0 / pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRFilt_strg);
    tmp = (loWgtVfilt_wgt) * (1 << 4);
    pCfg->lo_diff_vfilt_wgt = CLIP(tmp, 0, 0x1f);
    loWgtVfilt_wgt = 1.0 - (1.0 / pmdDyn->subLoMd1.hw_btnrT_vIIRFilt_strg);
    tmp = (loWgtVfilt_wgt) * (1 << 4);
    pCfg->lo_wgt_vfilt_wgt = CLIP(tmp, 0, 0x1f);
    tmp = (pmdDyn->subLoMd0.sgmCh.hw_btnrT_vIIRFstLn_scale) * (1 << 4);
    pCfg->sig_first_line_scale = CLIP(tmp, 0, 0x3f);
    tmp = (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRFstLn_scale) * (1 << 4);
    pCfg->lo_diff_first_line_scale = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_LFSCL
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_negOff) * (1 << 10);
    pCfg->lo_wgt_cal_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->lo_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_LFSCLTH
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_maxLimit) * (1 << 10);
    pCfg->lo_wgt_cal_max_limit = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_hiNsBase_ratio) * (1 << 10);
    pCfg->mode0_base_ratio = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_DSWGTSCL
    tmp = (pmdDyn->subLoMd0.wgtOpt.hw_btnrT_mdWgt_negOff) * (1 << 10);
    pCfg->lo_diff_wgt_cal_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->subLoMd0.wgtOpt.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->lo_diff_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTlastSCL
    tmp = (pmdDyn->loMd.hw_btnrT_preWgtMge_offset) * (1 << 10);
    pCfg->lo_mge_pre_wgt_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loMd.hw_btnrT_preWgtMge_scale) * (1 << 10);
    pCfg->lo_mge_pre_wgt_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL0
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_loWgtStat_scale) * (1 << 10);
    pCfg->mode0_lo_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_loWgtStatHdrS_scale) * (1 << 10);
    pCfg->mode0_lo_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL1
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStat_scale) * (1 << 10);
    pCfg->mode1_lo_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStatHdrS_scale) * (1 << 10);
    pCfg->mode1_lo_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL2
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->mode1_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgtHdrS_scale) * (1 << 10);
    pCfg->mode1_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTOFF
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStat_negOff) * (1 << 10);
    pCfg->mode1_lo_wgt_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStatHdrS_negOff) * (1 << 10);
    pCfg->mode1_lo_wgt_hdr_sht_offset = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGT1OFF
    tmp = (pdyn->sigmaEnv.hw_btnrT_statsPixAlpha_thred) * (1 << 10);
    pCfg->auto_sigma_count_wgt_thred = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_minLimit) * (1 << 10);
    pCfg->mode1_wgt_min_limit = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_offset) * (1 << 10);
    pCfg->mode1_wgt_offset = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_WGTLO_L
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusion_minLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_min_limit = CLIP(tmp, 0, 0x3fff);
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusionHdrS_minLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_hdr_sht_min_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTLO_H
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusion_maxLimit;
#if 1
    // if iso change, then ob_offset change, the tnr is disable to void the redish image
    static int pre_ob = 0;
    if(cvtinfo->isFirstFrame) {
        pre_ob = cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset;
    }
    if(psta->frmFusion.isoSwitch.sw_btnrT_fusionIso_mode == btnr_limitAdj_mode) {
        int delta_ob = ABS(cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset - pre_ob);
        if(delta_ob >= psta->frmFusion.isoSwitch.sw_btnrT_limitAdj_deltaOB) {
            tmpf = 1;
        }
        if(delta_ob > 0) {
            LOGD_ANR("iso:%d pre_ob:%d cur_ob:%d delta_ob:%d\n", cvtinfo->frameIso[0], pre_ob, cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset, delta_ob);
        }
    }
    pre_ob = cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset;
#endif
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_max_limit = CLIP(tmp, 0, 0x3fff);
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusionHdrS_maxLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_hdr_sht_max_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_STH_SCL
    tmp = (pmdDyn->frmFusion.hw_btnrT_loGgSoftThd_scale) * (1 << 8);
    pCfg->lo_pre_gg_soft_thresh_scale = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->frmFusion.hw_btnrT_loRbSoftThd_scale) * (1 << 8);
    pCfg->lo_pre_rb_soft_thresh_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_STH_LIMIT
    tmp = (pmdDyn->frmFusion.hw_btnrT_loSoftThd_maxLimit);
    pCfg->lo_pre_soft_thresh_max_limit = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->frmFusion.hw_btnrT_loSoftThd_minLimit);
    pCfg->lo_pre_soft_thresh_min_limit = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_HIKEEP
    tmp = (pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_curHiOrg_alpha) * (1 << FIXBITWFWGT);
    pCfg->cur_spnr_hi_wgt_min_limit = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_iirHiOrg_alpha) * (1 << FIXBITWFWGT);
    pCfg->pre_spnr_hi_wgt_min_limit = CLIP(tmp, 0, 0xff);
    tmp = (pmdDyn->memc.hw_btnrT_mcLoWgt_thred) * (1 << 10);
    pCfg->motion_est_lo_wgt_thred = CLIP(tmp, 0, 0x3ff);
    // REG: BAY3D_SIGNUMTH
    tmp = (psta->sigmaEnv.hw_btnrCfg_statsPixCnt_thred);
    pCfg->sigma_num_th = CLIP(tmp, 0, 0xfffff);
    // REG: BAY3D_MONR
    if (pdyn->outFrmFusion.sw_btnrT_outFrmBase_mode == btnr_curBaseOut_mode) {
        float hi_scale = pdyn->outFrmFusion.noiseBal_curBaseOut.sw_btnrT_hiNoiseBalNr_scale;                                                //
        float md_scale = pdyn->outFrmFusion.noiseBal_curBaseOut.sw_btnrT_midNoiseBalNr_scale;                                               //
        float hi_offset = pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_hiNoiseBalNr_off * (1 << 10);
        float md_offset = pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_midNoiseBalNr_off * (1 << 10);
        tmp0 = MIN((sqrt((float)1.0 / pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_hiNoiseBal_strg) * (1 << 10)), (1 << 10));
        tmp1 = MIN((sqrt((float)1.0 / pdyn->outFrmFusion.noiseBal_curBaseOut.hw_btnrT_midNoiseBal_strg) * (1 << 10)), (1 << 10));
        pCfg->out_use_hi_noise_bal_nr_strg = MIN((tmp0 + hi_offset) * hi_scale, 2.0 * (1 << 10));
        pCfg->out_use_md_noise_bal_nr_strg = MIN((tmp1 + md_offset) * md_scale, 2.0 * (1 << 10));
        pCfg->out_use_hi_noise_bal_nr_off = hi_offset;
        pCfg->out_use_md_noise_bal_nr_off = md_offset;
    }
    else {
        float hi_scale = pdyn->outFrmFusion.noiseBal_preBaseOut.sw_btnrT_hiNoiseBalNr_scale;                                                //
        float md_scale = pdyn->outFrmFusion.noiseBal_preBaseOut.sw_btnrT_midNoiseBalNr_scale;                                               //
        float hi_offset = pdyn->outFrmFusion.noiseBal_preBaseOut.hw_btnrT_hiNoiseBalNr_off * (1 << 10);
        float md_offset = pdyn->outFrmFusion.noiseBal_preBaseOut.hw_btnrT_midNoiseBalNr_off * (1 << 10);
        tmp0 = MIN((sqrt((float)1.0 / pdyn->outFrmFusion.noiseBal_preBaseOut.hw_btnrT_hiNoiseBal_strg) * (1 << 10)), (1 << 10));
        tmp1 = MIN((sqrt((float)1.0 / pdyn->outFrmFusion.noiseBal_preBaseOut.hw_btnrT_midNoiseBal_strg) * (1 << 10)), (1 << 10));
        pCfg->out_use_hi_noise_bal_nr_strg = MIN((tmp0 + hi_offset) * hi_scale, 2.0 * (1 << 10));
        pCfg->out_use_md_noise_bal_nr_strg = MIN((tmp1 + md_offset) * md_scale, 2.0 * (1 << 10));
        pCfg->out_use_hi_noise_bal_nr_off = hi_offset;
        pCfg->out_use_md_noise_bal_nr_off = md_offset;
    }
    tmp = (pdyn->locSgmStrg.hw_bnrT_locSgmStrg_maxLimit) * (1 << 8);
    pCfg->gain_out_max_limit = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_SIGSCL
    tmp = (pmdDyn->mdSigma.hw_btnrT_sigma_scale) * (1 << 10);
    pCfg->sigma_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->mdSigma.hw_btnrT_sigmaHdrS_scale) * (1 << 10);
    pCfg->sigma_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_DSOFF
    tmp = (pmdDyn->subLoMd1.hw_btnrT_vIIRWgt_offset) * (1 << 10);
    pCfg->lo_wgt_vfilt_offset = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_offset);
    pCfg->lo_diff_vfilt_offset = CLIP(tmp, 0, 0xfff);
    tmp = (0) * (1 << 4);
    pCfg->lo_wgt_cal_first_line_vfilt_wgt = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_DSSCL
    tmp = (pmdDyn->subLoMd1.hw_btnrT_vIIRWgt_scale) * (1 << 4);
    pCfg->lo_wgt_vfilt_scale = CLIP(tmp, 0, 0x3f);
    {
        // calc vfilt scale
        int bayertnr_lodiff_vfilt_scale_bit;
        int bayertnr_lodiff_vfilt_scale;
        for (i = 8; i >= 2; i--) {
            if (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale < (float)1.0 / (1 << i)) {
                bayertnr_lodiff_vfilt_scale_bit = (i + 1) - 2;
                bayertnr_lodiff_vfilt_scale = MIN((int)(pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale * (1 << (i + 1)) + 0.5), 63); //U2.4
                break;
            }
        }
        if(i == 1) {
            bayertnr_lodiff_vfilt_scale_bit = 2 - 2;
            bayertnr_lodiff_vfilt_scale = MIN((int)(pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale * (1 << 2) + 0.5), 63);  //U2.4
        }
        pCfg->lo_diff_vfilt_scale_bit = CLIP(bayertnr_lodiff_vfilt_scale_bit, 0, 0x7);
        pCfg->lo_diff_vfilt_scale = CLIP(bayertnr_lodiff_vfilt_scale, 0, 0x7f);
    }
    tmp = (0) * (1 << 4);
    pCfg->lo_diff_first_line_vfilt_wgt = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_ME0
    tmp = (pmdDyn->memc.hw_btnrT_upMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->memc.hw_btnrT_upMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->memc.hw_btnrT_sadFiltSpatial_wgt[0]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt0 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_ME1
    tmp = (pmdDyn->memc.hw_btnrT_upLeftMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_left_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->memc.hw_btnrT_upLeftMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_left_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->memc.hw_btnrT_sadFiltSpatial_wgt[1]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt1 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_ME2
    tmp = (pmdDyn->memc.hw_btnrT_upRightMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_right_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->memc.hw_btnrT_upRightMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_right_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->memc.hw_btnrT_sadFiltSpatial_wgt[2]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt2 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_WGTMAX
    tmpf = pmdDyn->frmFusion.hw_btnrT_loMotion_maxLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_motion_max_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGT1MAX
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_maxLimit) * (1 << 10);
    pCfg->mode1_wgt_max_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTM0
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_mdWgt_maxLimit) * (1 << 10);
    pCfg->mode0_wgt_out_max_limit = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->mdWgtFilt.hw_btnrT_mdWgt_offset) * (1 << 10);
    pCfg->mode0_wgt_out_offset = CLIP(tmp, 0, 0x3ff);
    if(pmdDyn->hiMd.hw_btnrT_md_mode == btnr_loAsBias_hi1_mode) {
        pCfg->mode0_wgt_out_offset = 0;
    }
    /* BAY3D_LOCOEF0 */
    {
        int coeff[5];
        if (pmdDyn->subLoMd1.sw_btnrT_hFiltCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_hozi_filter_coeff(pmdDyn->subLoMd1.hw_btnrT_hFilt_strg, coeff);
        } else {
            for (i = 0; i < 5; i++)
                coeff[i] = pmdDyn->subLoMd1.hw_btnrT_hFilt_coeff[i];
        }
        pCfg->lo_wgt_hflt_coeff0 = CLIP(coeff[2], 0, 0x1f);
        pCfg->lo_wgt_hflt_coeff1 = CLIP(coeff[1], 0, 0x1f);
        pCfg->lo_wgt_hflt_coeff2 = CLIP(coeff[0], 0, 0x1f);
    }

    {
        int coeff[5];
        if (pmdDyn->subLoMd0.sgmCh.sw_btnrT_hFiltCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_hozi_filter_coeff(pmdDyn->subLoMd0.sgmCh.hw_btnrT_hFilt_strg, coeff);
        } else {
            for (i = 0; i < 5; i++)
                coeff[i] = pmdDyn->subLoMd0.sgmCh.hw_btnrT_hFilt_coeff[i];
        }
        pCfg->sig_hflt_coeff0 = CLIP(coeff[2], 0, 0x1f);
        pCfg->sig_hflt_coeff1 = CLIP(coeff[1], 0, 0x1f);
        pCfg->sig_hflt_coeff2 = CLIP(coeff[0], 0, 0x1f);
    }

    /* BAY3D_LOCOEF1 */
    {
        int coeff[5];
        if (pmdDyn->subLoMd0.diffCh.sw_btnrT_hFiltCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_hozi_filter_coeff(pmdDyn->subLoMd0.diffCh.hw_btnrT_hFilt_strg, coeff);
        } else {
            for (i = 0; i < 5; i++)
                coeff[i] = pmdDyn->subLoMd0.diffCh.hw_btnrT_hFilt_coeff[i];
        }
        pCfg->lo_dif_hflt_coeff0 = CLIP(coeff[2], 0, 0x1f);
        pCfg->lo_dif_hflt_coeff1 = CLIP(coeff[1], 0, 0x1f);
        pCfg->lo_dif_hflt_coeff2 = CLIP(coeff[0], 0, 0x1f);
    }
    /* BAY3D_DPC0 */
    pCfg->pre_spnr_dpc_bright_str = CLIP(pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_brightDpc_strg, 0, 4);
    pCfg->pre_spnr_dpc_dark_str = CLIP(pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_darkDpc_strg, 0, 4);
    pCfg->pre_spnr_dpc_str = CLIP(pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_dpc_strg, 0, 4);
    tmp = (pCfg->pre_spnr_dpc_flt_mode ? pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_scale :
           pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_preWgt_scale) * (1 << 4);
    pCfg->pre_spnr_dpc_wk_scale = CLIP(tmp, 0, 0xff);
    tmp = (pCfg->pre_spnr_dpc_flt_mode ? pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_offset :
           pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_preWgt_offset) * (1 << 10);
    pCfg->pre_spnr_dpc_wk_offset = CLIP(tmp, 0, 0xff);
    /* BAY3D_DPC1 */
    tmp = pdyn->preDpc.dpc_nsBalNsStrg.hw_btnrT_noiseBal_strg * (1 << 10);
    pCfg->pre_spnr_dpc_nr_bal_str = pCfg->pre_spnr_dpc_flt_mode ? pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_preWgt_offset * (1 << 10) : tmp;
    tmp = (pdyn->preDpc.dpc_nsBalSfThd.hw_btnrT_softThd_scale) * (1 << 8);
    pCfg->pre_spnr_dpc_soft_thr_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PRELOWGT
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_ratAvgRgeWgt_alpha) * (1 << 7);
    pCfg->pre_spnr_lo_val_wgt_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = MAX((1 - pdyn->preSpNr.loNr.hw_btnrT_loNrOut_alpha) * (1 << 7), 0);
    pCfg->pre_spnr_lo_filter_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_filtWgt_minLimit) * (1 << 8);
    pCfg->pre_spnr_lo_filter_wgt_min = CLIP(tmp, 1, 0xff);
    // REG: BAY3D_MIDBIG0
    tmp = (pmdDyn->subDeepLoMd.hw_btnrT_wgt_offset) * (1 << 8);
    pCfg->md_large_lo_md_wgt_offset = CLIP(tmp, 0, 0xff);
    tmp = (pmdDyn->subDeepLoMd.hw_btnrT_wgt_scale) * (1 << 8);
    pCfg->md_large_lo_md_wgt_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_MIDBIG1
    tmp = (pmdDyn->subDeepLoMd.sw_btnrT_wgt2FusionLmt_negOff) * (1 << 8);
    pCfg->md_large_lo_wgt_cut_offset = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->subDeepLoMd.hw_btnrT_wgt2FussionLmt_offset) * (1 << 12);
    pCfg->md_large_lo_wgt_add_offset = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_MIDBIG2
    tmp = (pmdDyn->subDeepLoMd.sw_btnrT_wgt2FusionLmt_scale) * (1 << 8);
    pCfg->md_large_lo_wgt_scale = CLIP(tmp, 0, 0xfff);

    // tnr sigma curve must calculate before spnr sigma
    if(cvtinfo->isFirstFrame || (pTransParams->transf_mode != pCfg->transf_mode)
            || (pTransParams->transf_mode_scale != pCfg->transf_mode_scale)) {
        bayertnr_logtrans_init(pCfg->transf_mode, pCfg->transf_mode_scale, pTransParams);
    }

    bool update_itransf_tbl = false;
    if ((pTransParams->transf_mode != pCfg->transf_mode) ||
            (pTransParams->transf_mode_scale != pCfg->transf_mode_scale) ||
            (pTransParams->isTransfBypass != pCfg->transf_bypass_en) ||
            (pTransParams->transf_data_max_limit != pCfg->transf_data_max_limit) ||
            (pTransParams->itransf_mode_offset != pCfg->itransf_mode_offset) ||
            (pTransParams->transf_mode_offset != pCfg->transf_mode_offset)) {
        update_itransf_tbl = true;
    }

    pTransParams->transf_mode = pCfg->transf_mode;
    pTransParams->transf_mode_scale = pCfg->transf_mode_scale;
    pTransParams->transf_mode_offset = pCfg->transf_mode_offset;
    pTransParams->transf_data_max_limit = pCfg->transf_data_max_limit;
    pTransParams->itransf_mode_offset = pCfg->itransf_mode_offset;

    // REG: BAY3D_PIXMAX
    if(pCfg->transf_bypass_en) {
        pCfg->pix_max_limit  = ((1 << 12) - 1);
    } else {
        if(pCfg->transf_mode_scale) {
            pCfg->pix_max_limit  = bayertnr_logtrans(((1 << 15) - 1), pTransParams);
        } else {
            pCfg->pix_max_limit  = bayertnr_logtrans(((1 << 20) - 1), pTransParams);
        }
    }

    pTransParams->isTransfBypass = pCfg->transf_bypass_en;
    if (cvtinfo->isFirstFrame || update_itransf_tbl) {
        rk_autoblc_gen_tbl(pBlcInfo->bayertnr_itransf_tbl, pCfg->pix_max_limit, pTransParams);
    }

    bayertnr_luma2sigmax_config_v41(pTransParams, &cvtinfo->blc_res, cvtinfo->preDGain);


    bool auto_sig_curve_spnruse = psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_autoSigma_mode;
    sigbins = 20;

    for(i = 0; i < sigbins; i++) {
        pCfg->tnr_luma2sigma_x[i] = CLIP(pTransParams->tnr_luma_sigma_x[i], 0, 0xfff);
    }

    //printf("oyyf tnr sigmax[] :");
    if(pTransParams->isTransfBypass  || opMode == RK_AIQ_OP_MODE_MANUAL) {
        for(i = 0; i < sigbins; i++) {
            pCfg->tnr_luma2sigma_x[i] = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.idx[i];
            //printf("%d ", pCfg->tnr_luma2sigma_x[i]);
        }
    }
    //printf("\n");

    //x_step must be 2^n
    int x_step = 0;
    for(i = 1; i < sigbins; i++) {
        x_step = pCfg->tnr_luma2sigma_x[i] - pCfg->tnr_luma2sigma_x[i - 1];
        x_step = 1 << (FLOOR(LOG2(x_step)));
        tmp = pCfg->tnr_luma2sigma_x[i - 1] + x_step;
        pCfg->tnr_luma2sigma_x[i] = CLIP(tmp, 0, pCfg->pix_max_limit);
    }
    pCfg->tnr_luma2sigma_x[sigbins - 1] =  pCfg->pix_max_limit;



    if(psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_manualSigma_mode || cvtinfo->isFirstFrame || bayertnr_default_noise_curve_use) {
        pTransParams->bayertnr_auto_sig_count_en = 0;

        if( pTransParams->isTransfBypass || opMode == RK_AIQ_OP_MODE_MANUAL) {
            for(i = 0; i < sigbins; i++) {
                pTransParams->tnr_luma_sigma_y[i] = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.val[i];
            }
        } else if(pTransParams->isHdrMode) {
            uint16_t shortY[sigbins];
            int short_iso = cvtinfo->frameIso[1];
            int short_ilow = 0, short_ihigh = 0;
            float short_iso_ratio = 0.0f;

            pre_interp(short_iso, NULL, 0, &short_ilow, &short_ihigh, &short_iso_ratio);

            uint16_t* pSigmaX = pCfg->tnr_luma2sigma_x;
            uint16_t* pSigmaY = pTransParams->tnr_luma_sigma_y;
            uint16_t* pCalibX = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pLongY = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pShortLowY = btnr_attrib->stAuto.spNrDyn[short_ilow].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pShortHighY = btnr_attrib->stAuto.spNrDyn[short_ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.val;

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
            uint16_t* pLowY = btnr_attrib->stAuto.spNrDyn[ilow].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pHighY = btnr_attrib->stAuto.spNrDyn[ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.val;
            uint16_t* pLowX = btnr_attrib->stAuto.spNrDyn[ilow].sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pHighX = btnr_attrib->stAuto.spNrDyn[ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.idx;
            uint16_t* pSigmaX = pCfg->tnr_luma2sigma_x;
            uint16_t* pSigmaY = pTransParams->tnr_luma_sigma_y;
            bayertnr_sigmaY_interpolate(pSigmaX, pSigmaY, pLowX, pHighX, pLowY, pHighY, sigbins, iso_ratio);
        }
    } else {
        pTransParams->bayertnr_auto_sig_count_en = 1;
        pTransParams->bayertnr_auto_sig_count_filt_wgt = pdyn->sigmaEnv.sw_btnrT_autoSgmIIR_alpha * (1 << 10);
        pTransParams->bayertnr_auto_sig_count_max = cvtinfo->rawWidth * cvtinfo->rawHeight / 3;
        bayertnr_autosigma_config(btnr_stats, pTransParams, &cvtinfo->blc_res);
    }

#if 0
    printf("btnr stats sigmaY [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
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
    int max_sig = ((1 << 12) - 1);
    for(i = 0; i < sigbins; i++) {
        pCfg->tnr_luma2sigma_y[i] = CLIP((int)(pTransParams->tnr_luma_sigma_y[i]), 0, max_sig);
    }

#if 0
    printf("btnr reg sigmaY [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           pCfg->tnr_luma2sigma_y[0],
           pCfg->tnr_luma2sigma_y[1],
           pCfg->tnr_luma2sigma_y[2],
           pCfg->tnr_luma2sigma_y[3],
           pCfg->tnr_luma2sigma_y[4],
           pCfg->tnr_luma2sigma_y[5],
           pCfg->tnr_luma2sigma_y[6],
           pCfg->tnr_luma2sigma_y[7],
           pCfg->tnr_luma2sigma_y[8],
           pCfg->tnr_luma2sigma_y[9],
           pCfg->tnr_luma2sigma_y[10],
           pCfg->tnr_luma2sigma_y[11],
           pCfg->tnr_luma2sigma_y[12],
           pCfg->tnr_luma2sigma_y[13],
           pCfg->tnr_luma2sigma_y[14],
           pCfg->tnr_luma2sigma_y[15],
           pCfg->tnr_luma2sigma_y[16],
           pCfg->tnr_luma2sigma_y[17],
           pCfg->tnr_luma2sigma_y[18],
           pCfg->tnr_luma2sigma_y[19]);
#endif

    // spnr sigma curve, cur bil and iir bil strength
    int spnrsigbins = 16;

    int pix_max = pCfg->transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans((1 << 12) - 1, pTransParams);
    pix_max = cvtinfo->frameNum == 2 ? bayertnr_logtrans((1 << 20) - 1, pTransParams) : pix_max;
    if(cvtinfo->frameNum == 2) {
        // hdr long bins
        int lgbins = 8;
        for(i = 0; i < lgbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 128 * (i + 1);
        }
        // hdr short bins, max gain 256
        for(i = lgbins; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 256 * (i - lgbins + 1) + pCfg->pre_spnr_luma2sigma_x[lgbins - 1];
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;

    } else if (!pCfg->transf_bypass_en) {
#if 0
        for(i = 0; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 64 * (i + 1) ;
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;
#else
        uint32_t blc_remain = 0;
        if(cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset && cvtinfo->blc_res.obcPostTnr.sw_blcT_obcPostTnr_en) {
            blc_remain = cvtinfo->blc_res.obcPostTnr.sw_blcT_autoOB_offset;
        }
        int log_pix_max = bayertnr_logtrans(((1 << 12) * cvtinfo->preDGain - 1), pTransParams);
        int log_ob_offset = bayertnr_logtrans(blc_remain, pTransParams);

        pCfg->pre_spnr_luma2sigma_x[0] = (log_ob_offset + 128);
        int step = 0;
        for(i = 1; i < spnrsigbins; i++) {
            step = (log_pix_max - pCfg->pre_spnr_luma2sigma_x[i - 1]) / (spnrsigbins - i);
            if(i != spnrsigbins - 1) {
                step = 1 << (ROUND_F(LOG2(step)));
            } else {
                step = 1 << (FLOOR(LOG2(step)));
            }
            tmp = pCfg->pre_spnr_luma2sigma_x[i - 1] + step;
            pCfg->pre_spnr_luma2sigma_x[i] = CLIP(tmp, 0, log_pix_max);
#if 0
            printf("spnr sigmax idx:%d step:%d stepfinnal:%d x[%d]=%d clip:%u\n",
                   i, (log_pix_max - pCfg->pre_spnr_luma2sigma_x[i - 1]) / (spnrsigbins - i), step, i, tmp,  pCfg->pre_spnr_luma2sigma_x[i]);
#endif
        }
#endif
    } else {
        for(i = 0; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 256 * (i + 1) ;
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;
    }

    //x step must 2^n
    for(i = 1; i < spnrsigbins; i++) {
        x_step = pCfg->pre_spnr_luma2sigma_x[i] - pCfg->pre_spnr_luma2sigma_x[i - 1];
        x_step = 1 << (FLOOR(LOG2(x_step)));
        tmp = pCfg->pre_spnr_luma2sigma_x[i - 1] + x_step;
        pCfg->pre_spnr_luma2sigma_x[i] = CLIP(tmp, 0, pCfg->pix_max_limit);
    }
    pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;
#if 0
    printf("spnr sigmax[%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           pCfg->pre_spnr_luma2sigma_x[0],
           pCfg->pre_spnr_luma2sigma_x[1],
           pCfg->pre_spnr_luma2sigma_x[2],
           pCfg->pre_spnr_luma2sigma_x[3],
           pCfg->pre_spnr_luma2sigma_x[4],
           pCfg->pre_spnr_luma2sigma_x[5],
           pCfg->pre_spnr_luma2sigma_x[6],
           pCfg->pre_spnr_luma2sigma_x[7],
           pCfg->pre_spnr_luma2sigma_x[8],
           pCfg->pre_spnr_luma2sigma_x[9],
           pCfg->pre_spnr_luma2sigma_x[10],
           pCfg->pre_spnr_luma2sigma_x[11],
           pCfg->pre_spnr_luma2sigma_x[12],
           pCfg->pre_spnr_luma2sigma_x[13],
           pCfg->pre_spnr_luma2sigma_x[14],
           pCfg->pre_spnr_luma2sigma_x[15]);
#endif

    if(!auto_sig_curve_spnruse || cvtinfo->isFirstFrame) {
        if(pTransParams->isTransfBypass || opMode == RK_AIQ_OP_MODE_MANUAL) {
            for(i = 0; i < spnrsigbins; i++) {
                pCfg->pre_spnr_luma2sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.val[i]), 0, max_sig);
                pCfg->pre_spnr_luma2sigma_x[i] = pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.idx[i];
            }
        } else if(pTransParams->isHdrMode ) {
            uint16_t shortY[spnrsigbins];
            int short_iso = cvtinfo->frameIso[1];
            int short_ilow = 0, short_ihigh = 0;
            float short_iso_ratio = 0.0f, merge_luma2Wgt = 0.0f;

            pre_interp(short_iso, NULL, 0, &short_ilow, &short_ihigh, &short_iso_ratio);

            uint16_t* pSigmaX = pCfg->pre_spnr_luma2sigma_x;
            uint16_t* pSigmaY = pCfg->pre_spnr_luma2sigma_y;
            uint16_t* pLongY = pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
            uint16_t* pCalibX = pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.idx;
            uint16_t* pShortLowY = btnr_attrib->stAuto.spNrDyn[short_ilow].sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
            uint16_t* pShortHighY = btnr_attrib->stAuto.spNrDyn[short_ihigh].sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
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
            uint16_t* pLowY = btnr_attrib->stAuto.spNrDyn[ilow].sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
            uint16_t* pHighY = btnr_attrib->stAuto.spNrDyn[ihigh].sigmaEnv.hw_btnrC_preSpNrSgm_curve.val;
            uint16_t* pLowX = btnr_attrib->stAuto.spNrDyn[ilow].sigmaEnv.hw_btnrC_preSpNrSgm_curve.idx;
            uint16_t* pHighX = btnr_attrib->stAuto.spNrDyn[ihigh].sigmaEnv.hw_btnrC_preSpNrSgm_curve.idx;
            uint16_t* pSigmaX = pCfg->pre_spnr_luma2sigma_x;
            uint16_t* pSigmaY = pCfg->pre_spnr_luma2sigma_y;
            bayertnr_sigmaY_interpolate(pSigmaX, pSigmaY, pLowX, pHighX, pLowY, pHighY, spnrsigbins, iso_ratio);
        }
    } else {
        for(i = 0; i < spnrsigbins; i++) {
            tmp0 = bayertnr_tnr_noise_curve(pCfg->pre_spnr_luma2sigma_x[i], 0, pTransParams);
            pCfg->pre_spnr_luma2sigma_y[i] = CLIP((int)(tmp0), 0, max_sig);
        }
    }


#if 0
    printf("spnr btnr reg sigmaY [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
           pCfg->pre_spnr_luma2sigma_y[0],
           pCfg->pre_spnr_luma2sigma_y[1],
           pCfg->pre_spnr_luma2sigma_y[2],
           pCfg->pre_spnr_luma2sigma_y[3],
           pCfg->pre_spnr_luma2sigma_y[4],
           pCfg->pre_spnr_luma2sigma_y[5],
           pCfg->pre_spnr_luma2sigma_y[6],
           pCfg->pre_spnr_luma2sigma_y[7],
           pCfg->pre_spnr_luma2sigma_y[8],
           pCfg->pre_spnr_luma2sigma_y[9],
           pCfg->pre_spnr_luma2sigma_y[10],
           pCfg->pre_spnr_luma2sigma_y[11],
           pCfg->pre_spnr_luma2sigma_y[12],
           pCfg->pre_spnr_luma2sigma_y[13],
           pCfg->pre_spnr_luma2sigma_y[14],
           pCfg->pre_spnr_luma2sigma_y[15]);
#endif

    // REG: BAY3D_SIGORG
    pTransParams->bayertnr_lo_wgt_clip_min_limit = pCfg->lo_wgt_clip_min_limit ;
    pTransParams->bayertnr_lo_wgt_clip_max_limit = pCfg->lo_wgt_clip_max_limit;
    pCfg->tnr_out_sigma_sq  = bayertnr_update_sq(pTransParams);

    if (!pCfg->transf_bypass_en) {
        rk_aiq_btnr42_params_logtrans(pCfg);
    }

    //TODO
    pCfg->btnr_ldc_en = 0;
    pCfg->b3dldch_en = 0;


    return;
}

// enable this if you want to test isp39_logtransf
#if 0
void bayertnr_isp39_logtrans_test(struct isp35_bay3d_cfg *pCfg, btnr_trans_params_t *pTransParams)
{
    static bool logtransf_test = true;
    if (logtransf_test) {
        printf("*************** enter logtransf_test ************** \n");
        printf("transf_bypass : %d\n", pCfg->transf_bypass_en);
        printf("data_maxlimit : %d\n", pCfg->transf_data_max_limit);
        printf("is15bit mode  : %d\n", pCfg->transf_mode_scale);
        printf("transf_offset : %d\n", pCfg->transf_mode_offset);

        int data_maxlimit = pCfg->transf_data_max_limit;
        uint8_t is15bit = pCfg->transf_mode_scale;
        uint8_t offsetbit = bayertnr_find_top_one_pos(pCfg->transf_mode_offset);
        printf("----> test correctness ...\n");
        for (int testi = 0; testi < data_maxlimit; testi++) {
            uint16_t test_ret1 = isp39_logtransf(testi, is15bit, offsetbit);
            uint16_t test_ret2 = bayertnr_logtrans(testi, pTransParams);

            if (abs(test_ret1 - test_ret2) > 1) {
                printf("ERROR logtrans missmatch %d %d %d!!!\n", testi, test_ret1, test_ret2);
            }
        }
        printf("----> done!\n");

        struct timespec tp;
        uint64_t test_time_start, test_time_tmp, test_time1, test_time2;
        uint64_t test_sum = 0; // prevent optimization

        printf("----> test execution time ...\n");

        clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
        test_time_start = tp.tv_sec * 1000 * 1000 * 1000 + tp.tv_nsec;
        for (int testi = 0; testi < data_maxlimit; testi++) {
            uint16_t test_ret1 = isp39_logtransf(testi, is15bit, offsetbit);
            test_sum += test_ret1;
        }
        clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
        test_time_tmp = tp.tv_sec * 1000 * 1000 * 1000 + tp.tv_nsec;
        test_time1 = test_time_tmp - test_time_start;
        printf("isp39_logtransf sum %lld, time %lld\n", test_sum, test_time1);

        test_sum = 0;
        clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
        test_time_start = tp.tv_sec * 1000 * 1000 * 1000 + tp.tv_nsec;
        for (int testi = 0; testi < data_maxlimit; testi++) {
            uint16_t test_ret2 = bayertnr_logtrans(testi, pTransParams);
            test_sum += test_ret2;
        }
        clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
        test_time_tmp = tp.tv_sec * 1000 * 1000 * 1000 + tp.tv_nsec;
        test_time2 = test_time_tmp - test_time_start;
        printf("bayertnr_logtrans sum %lld, time %lld\n", test_sum, test_time2);

        float fast_percent = (1.0 * test_time2) / (1.0 * test_time1);
        printf("----> time test done, %.2f%%, faster!\n", fast_percent * 100);

        logtransf_test = false;
        printf("*************** logtransf_test done! ************** \n");
    }
}
#endif
