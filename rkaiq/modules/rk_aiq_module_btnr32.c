#include "rk_aiq_isp32_modules.h"

bool rk_aiq_btnr32_check_attrib(btnr_api_attrib_t *attr)
{
    return true;
}

bool rk_aiq_btnr32_check_param(btnr_param_t *param)
{
    return true;
}

void rk_aiq_btnr32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg, bool bypass)
{
    int i;
    int tmp;
    struct isp32_bay3d_cfg *phwcfg = &isp_cfg->others.bay3d_cfg;
    btnr_param_t *btnr_param = (btnr_param_t *) attr;
    btnr_params_dyn_t *pdyn = &btnr_param->dyn;
    btnr_params_static_t *psta = &btnr_param->sta;

    if (psta->bwCfg.hw_btnrCfg_bw_mode == bw_compress_mode)
        phwcfg->bwsaving_en = 1;
    else
        phwcfg->bwsaving_en = 0;

    phwcfg->bypass_en = bypass;
    switch (psta->mdCfg.hw_btnrT_md_mode) {
    case loHiMdMix_mode:
        phwcfg->hibypass_en = 0;
        phwcfg->lobypass_en = 0;
        break;
    case hiMdOnly_mode:
        phwcfg->hibypass_en = 0;
        phwcfg->lobypass_en = 1;
        break;
    case loMdOnly_mode:
        phwcfg->hibypass_en = 1;
        phwcfg->lobypass_en = 0;
        break;
    }
    phwcfg->himed_bypass_en   = !pdyn->hiMd.hw_btnrT_hiMedfilt_en;
    phwcfg->lomed_bypass_en   = !pdyn->loMd.hw_btnrT_loMedfilt_en;
    phwcfg->higaus_bypass_en  = !pdyn->hiMd.hw_btnrT_hiFilt_en;
    phwcfg->hiabs_possel      = pdyn->hiMd.hw_btnrT_hiFiltAbs_mode == abs_filt_mode;
    switch (pdyn->hiMd.hw_btnrT_hiFilt_mode) {
    case bayerFilt_gaus_mode:
        phwcfg->hichncor_en = 0;
        phwcfg->higaus3_mode = 0;
        phwcfg->hichnsplit_en = 1;
        break;
    case bayerFilt_gausMean_mode:
        phwcfg->hichncor_en = 1;
        break;
    case lumaFilt_filt3StrgLo_mode:
        phwcfg->hichncor_en = 0;
        phwcfg->higaus3_mode = 2;
        break;
    case lumaFilt_filt3StrgHi_mode:
        phwcfg->hichncor_en = 0;
        phwcfg->higaus3_mode = 1;
        break;
    case lumaFilt_filt5StrgLo_mode:
        phwcfg->hichncor_en = 0;
        phwcfg->higaus3_mode = 0;
        phwcfg->hichnsplit_en = 0;
        phwcfg->higaus5x5_en = 1;
        break;
    case lumaFilt_filt7StrgLo_mode:
        phwcfg->hichncor_en = 0;
        phwcfg->higaus3_mode = 0;
        phwcfg->hichnsplit_en = 0;
        phwcfg->higaus5x5_en = 0;
        break;
    }

    phwcfg->logaus5_bypass_en = !pdyn->loMd.hw_btnrT_loBayerFilt_en;
    phwcfg->logaus3_bypass_en = !pdyn->loMd.hw_btnrT_loLumaFilt_en;
    phwcfg->glbpk_en          = psta->pkCfg.pkSigmaMode.hw_btnrT_pkSigma_mode == btnr_pkSigma_glb_mode;
    phwcfg->loswitch_protect  = 0;

    tmp = (int)(pdyn->frmAlpha.hw_bnrT_softThred_scale * (1 << 10));
    phwcfg->softwgt  = CLIP(tmp, 0, 0x3ff);
    tmp = (int)(pdyn->frmAlphaWgtPk.sw_btnrT_glbPkSigma_scale);
    phwcfg->glbpk2   = CLIP(tmp, 0, 0xfffffff);

    switch (pdyn->frmAlphaWgtMd.sw_btnrT_wgtMdOpt_mode) {
    case loHi_loHiDiff_mode:
        phwcfg->hiwgt_opt_en = 0;
        phwcfg->wgtmix_opt_en = 0;
        break;
    case loHi0_hi1_mode:
        phwcfg->hiwgt_opt_en = 1;
        phwcfg->wgtmix_opt_en = 1;
        break;
    case loHi0_negHi1_mode:
        phwcfg->hiwgt_opt_en = 1;
        phwcfg->wgtmix_opt_en = 0;
        break;
    }

    phwcfg->bwopt_gain_dis = 0;

    switch (psta->mdCfg.hw_btnrT_loDs_mode) {
    case loDs_4x4_mode:
        phwcfg->lo4x4_en = 1;
        phwcfg->lo4x8_en = 0;
        break;
    case loDs_8x4_mode:
        phwcfg->lo4x4_en = 0;
        phwcfg->lo4x8_en = 1;
        break;
    case loDs_8x8_mode:
        phwcfg->lo4x4_en = 0;
        phwcfg->lo4x8_en = 0;
        break;
    }

    phwcfg->hisig_ind_sel = pdyn->hiMd.hw_btnrT_hiSigmaIdxPreFilt_en;
    phwcfg->pksig_ind_sel = pdyn->frmAlphaWgtPk.hw_btnrT_pkSigmaIdxPreFilt_en;
    phwcfg->iirwr_rnd_en = 1;
    phwcfg->curds_high_en = 0;
    tmp = (int)(((float)1 - pdyn->frmAlphaWgtPk.hw_btnrT_wgtPk_minLimit) * (1 << 10));
    phwcfg->wgtlmt = CLIP(tmp, 0, 0x3ff);
    if (phwcfg->hibypass_en == 0)
        tmp = (int)(pdyn->frmAlphaWgtMd.hw_btnrT_loHi0Diff_scale * (1 << 10));
    else
        tmp = 0;
    phwcfg->wgtratio = CLIP(tmp, 0, 0x3ff);

    for (int i = 0; i < ISP3X_BAY3D_XY_NUM; i++) {
        tmp = pdyn->frmAlphaWgtPk.hw_btnrC_luma2PkSigma_Curve.idx[i];
        phwcfg->sig0_x[i] = CLIP(tmp, 0, 0xffff);
        tmp = pdyn->frmAlphaWgtPk.hw_btnrC_luma2PkSigma_Curve.val[i];
        phwcfg->sig0_y[i] = CLIP(tmp, 0, 0x3fff);

        tmp = pdyn->hiMd.hw_btnrC_luma2HiSigma_Curve.idx[i];
        phwcfg->sig1_x[i] = CLIP(tmp, 0, 0xffff);
        tmp = pdyn->hiMd.hw_btnrC_luma2HiSigma_Curve.val[i];
        phwcfg->sig1_y[i] = CLIP(tmp, 0, 0x3fff);

        tmp = pdyn->loMd.hw_btnrC_luma2LoSigma_Curve.idx[i];
        phwcfg->sig2_x[i] = CLIP(tmp, 0, 0xffff);
        tmp = pdyn->loMd.hw_btnrC_luma2LoSigma_Curve.val[i];
        phwcfg->sig2_y[i] = CLIP(tmp, 0, 0x3ff);
    }

    tmp = (int)(pdyn->hiMd.hw_btnrT_hi0Sigma_scale * (1 << 8));
    phwcfg->hisigrat0 = CLIP(tmp, 0, 0xfff);
    tmp = (int)(pdyn->hiMd.hw_btnrT_hi1Sigma_scale * (1 << 8));
    phwcfg->hisigrat1 = CLIP(tmp, 0, 0xfff);

    tmp = (int)(pdyn->hiMd.hw_btnrT_hi0Sigma_offset);
    phwcfg->hisigoff0 = CLIP(tmp, 0, 0xfff);
    tmp = (int)(pdyn->hiMd.hw_btnrT_hi1Sigma_offset);
    phwcfg->hisigoff1 = CLIP(tmp, 0, 0xfff);

    tmp = (int)(pdyn->loMd.hw_btnrT_loSigma_offset);
    phwcfg->losigoff = CLIP(tmp, 0, 0xfff);
    tmp = (int)(pdyn->loMd.hw_btnrT_loSigma_scale * (1 << 8));
    phwcfg->losigrat = CLIP(tmp, 0, 0xfff);

    phwcfg->rgain_off = 0;
    phwcfg->bgain_off = 0;

    tmp = (int)(pdyn->hiMd.hw_btnrT_SigmaIdxPreFilt_coeff[0]);
    phwcfg->siggaus0 =  CLIP(tmp, 0, 0x3f);
    tmp = (int)(pdyn->hiMd.hw_btnrT_SigmaIdxPreFilt_coeff[1]);
    phwcfg->siggaus1 =  CLIP(tmp, 0, 0x3f);
    tmp = (int)(pdyn->hiMd.hw_btnrT_SigmaIdxPreFilt_coeff[2]);
    phwcfg->siggaus2 =  CLIP(tmp, 0, 0x3f);
    tmp = (int)(pdyn->hiMd.hw_btnrT_SigmaIdxPreFilt_coeff[3]);
    phwcfg->siggaus3 =  CLIP(tmp, 0, 0x3f);

    phwcfg->hidif_th = 0xffff;
    return;
}
