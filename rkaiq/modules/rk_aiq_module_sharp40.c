#include "rk_aiq_isp33_modules.h"

static void SharpCreateKernelCoeffs(int radius, int max_radius, float rsigma, int *kernel_coeffs, int fix_bits, int dim)
{
    // calcute distance
    int i = 0;
    int j = 0;
    int k = 0;

    double e = 2.71828182845905;
    float gaus_table    [10] = { 0 };
    float distance_table[10] = { 0 };
    float sumTable = 0;
    float tmp;

    float coeffScale_table1D[10] = { 1, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
    float coeffScale_table2D[10] = { 1, 4, 4, 4, 8, 4, 4, 8, 8, 4 };

    int coeffNums     = (radius     + 1) * (radius     + 2) / 2;
    int coeffNums_max = (max_radius + 1) * (max_radius + 2) / 2;

    if (dim == 1)
    {
        coeffNums = radius + 1;
    }

    // calc distance
    if (dim == 2)
    {
        for (i = 0; i <= radius; i++)
        {
            for (j = 0; j <= i; j++)
            {
                distance_table[i * (i + 1) / 2 + j] = pow(i, 2) + pow(j, 2);
            }
        }
    }
    else
    {
        for (i = 0; i <= radius; i++)
        {
            distance_table[i] = pow(i, 2);
        }
    }

    // calc coeff
    for (k = 0; k < coeffNums; k++)
    {
        tmp = pow(e, -distance_table[k] / 2.0 / rsigma / rsigma);
        gaus_table[k] = tmp;

        if (dim == 2)
        {
            sumTable += coeffScale_table2D[k] * gaus_table[k];
        }
        else
        {
            sumTable += coeffScale_table1D[k] * gaus_table[k];
        }
    }

    for (k = 0; k < coeffNums_max; k++)
    {
        gaus_table[k] = gaus_table[k] / sumTable;
        kernel_coeffs[k] = FLOOR(gaus_table[k] * (1 << fix_bits));
    }
}

static void SharpKernelCoeffsNormalization(int *kernel_coeffs, int radius, int coeffs_sum, int dim)
{
    int i = 0;
    int offset    = 0;
    int sum_coeff = 0;
    int coeffScale_table1D[10] = { 1, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
    int coeffScale_table2D[10] = { 1, 4, 4, 4, 8, 4, 4, 8, 8, 4 };
    int kernel_coeffs_counts   = (radius + 1) * (radius + 2) / 2;
    if (dim == 1)
    {
        kernel_coeffs_counts = radius + 1;
        for (i = 0; i < kernel_coeffs_counts; i++)
        {
            sum_coeff += kernel_coeffs[i] * coeffScale_table1D[i];
        }
    }
    else if (dim == 2)
    {
        for (i = 0; i < kernel_coeffs_counts; i++)
        {
            sum_coeff += kernel_coeffs[i] * coeffScale_table2D[i];
        }
    }

    offset = coeffs_sum - sum_coeff;
    kernel_coeffs[0] = kernel_coeffs[0] + offset;
}

void noiseCurveInterp(uint16_t *noise_curve_stats, int *noise_curve_ext, uint16_t* last_noise_curve)
{
    int32_t noise_curve_bak[17] = { 0 };
    int zero_sum = 0;
    for(int k = 0; k < 17; k++) {
        noise_curve_bak[k] = noise_curve_stats[k];
        if(noise_curve_stats[k] == 0) {
            zero_sum++;
        }
    }

    if(zero_sum == 17) {
        LOGD_ASHARP("warning: sharp noise stats are all zero! use last noise curve instead.\n");
        for (int i = 0; i < 17; i++) {
            noise_curve_ext[i] = last_noise_curve[i];
        }
        return;
    }

    for (int cur = 0; cur < 17; cur++)
    {
        if (noise_curve_bak[cur] == 0)
        {
            int left_idx = 0;
            int right_idx = 0;
            int valid_idx = 0;
            if (cur == 0)
            {
                for (int i = 1; i < 17; i++)
                {
                    if (noise_curve_bak[i])
                    {
                        valid_idx = i;
                        break;
                    }
                }
                noise_curve_bak[cur] = noise_curve_bak[valid_idx];
            }
            else if (cur == 17 - 1)
            {
                for (int i = 17 - 1; i >= 0; i--)
                {
                    if (noise_curve_bak[i])
                    {
                        valid_idx = i;
                        break;
                    }
                }
                noise_curve_bak[17 - 1] = noise_curve_bak[valid_idx];
            }
            else
            {
                for (int i = cur; i >= 0; i--)
                {
                    if (noise_curve_bak[i])
                    {
                        left_idx = i;
                        break;
                    }
                }
                for (int i = cur; i < 17; i++)
                {
                    if (noise_curve_bak[i])
                    {
                        right_idx = i;
                        break;
                    }
                }
                int wgt_left  = right_idx - cur;
                int wgt_right = cur - left_idx;
                int val_interp;
                if (right_idx == 0)
                {
                    val_interp = noise_curve_bak[left_idx];
                } else {
                    val_interp = ROUND_F((wgt_left * noise_curve_bak[left_idx] + wgt_right * noise_curve_bak[right_idx]) / (right_idx - left_idx));
                }
                noise_curve_bak[cur] = val_interp;
            }
        }
    }
    for (int i = 0; i < 17; i++)
    {
        //int noise_curve_config = pSelect->noiseCurve_ext[i];
        int noise_curve_modify = noise_curve_bak[i];
        /*
        if (pSelect->noiseExtConfig_mode == 1) {
            noise_curve_ext[i] = noise_curve_modify;
        } else {
            noise_curve_ext[i] = noise_curve_config;
        }
        */
        noise_curve_ext[i] = noise_curve_modify;
    }

#if 0
    printf("sharp stats:%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
           noise_curve_ext[0],
           noise_curve_ext[1],
           noise_curve_ext[2],
           noise_curve_ext[3],
           noise_curve_ext[4],
           noise_curve_ext[5],
           noise_curve_ext[6],
           noise_curve_ext[7],
           noise_curve_ext[8],
           noise_curve_ext[9],
           noise_curve_ext[10],
           noise_curve_ext[11],
           noise_curve_ext[12],
           noise_curve_ext[13],
           noise_curve_ext[14],
           noise_curve_ext[15],
           noise_curve_ext[16]);
#endif
}

void rk_aiq_sharp40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo, btnr_cvt_info_t* pBtnrInfo)
{
    int i, tmp;
    float fPercent = 1.0f;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;
    struct isp33_sharp_cfg *pCfg = &isp_params->isp_cfg->others.sharp_cfg;
    sharp_param_t *sharp_param = (sharp_param_t *) attr;
    sharp_params_static_t* psta = &sharp_param->sta;
    sharp_params_dyn_t* pdyn = &sharp_param->dyn;
    texEst_param_t *texEst_param = &cvtinfo->texEst_param;
    texEst_params_dyn_t *pTexDyn = &texEst_param->dyn;

    //////////////////////////////////////////////////////////////////////////
    // coeff clip : coef0->7bit, coef1~coef2->5bit, coef3 ~ coef9->4bit
    int kernel_coeffs_maxLimit[10] = { 127, 31, 31, 15, 15, 15, 15, 15, 15, 15 };

    // REG: CTRL
    //pCfg->bypass = pFix->sharp_bypass;
    if (pdyn->locShpStrg.motionStrg.hw_shpT_locSgmStrg_mode == shp_locGlbSgmStrgMix_mode) {
        pCfg->local_gain_bypass = 0;
    } else {
        pCfg->local_gain_bypass = 1;
    }
    if (pTexDyn->texEst.hw_texEstT_texEst_mode == texEst_texEst5x5_mode) {
        pCfg->tex_est_mode = 0;
    } else {
        pCfg->tex_est_mode = 1;
    }
    if (pdyn->edgeShp.shootReduction.sw_shpT_maxMinFlt_mode == shp_maxMinFiltRadius5_mode) {
        pCfg->max_min_flt_mode = 0;
    } else {
        pCfg->max_min_flt_mode = 1;
    }
    if (pdyn->detailShp.detailFusion.hw_shpT_detailAlpha_mode == shp_tex2AlphaPosCorr_mode) {
        pCfg->detail_fusion_wgt_mode = 0;
    } else {
        pCfg->detail_fusion_wgt_mode = 1;
    }
    if (pTexDyn->noiseEst.hw_texEstT_noiseEst_mode == texEst_noiseEst3x3_mode) {
        pCfg->noise_calc_mode = 0;
    } else {
        pCfg->noise_calc_mode = 1;
    }
    if (psta->locShpStrg_radiDist.hw_shpT_radiusStep_mode == shp_step_128_mode) {
        pCfg->radius_step_mode = 0;
    } else {
        pCfg->radius_step_mode = 1;
    }
    if (pTexDyn->noiseEst.hw_texEstT_nsEstTexThd_mode == texEst_baseNoiseStats_mode) {
        pCfg->noise_curve_mode = 0;
    } else {
        pCfg->noise_curve_mode = 1;
    }
    if (pdyn->locShpStrg.motionStrg.hw_shpT_locSgmStrg2Mot_mode == shp_toMotionStrg1_mode) {
        pCfg->gain_wgt_mode = 0;
    } else {
        pCfg->gain_wgt_mode = 1;
    }
    if (psta->debug.sw_shpT_dbgOut_en) {
        switch (psta->debug.hw_shpT_dbgOut_mode) {
        case shp_texEst_mode:
            pCfg->debug_mode = 1;
            break;
        case shp_detailLocShpStrg_mode:
            pCfg->debug_mode = 2;
            break;
        case shp_edgeShpStrg_mode:
            pCfg->debug_mode = 3;
            break;
        case shp_contrastDetailPosStrg_mode:
            pCfg->debug_mode = 4;
            break;
        case shp_detailPosLimit_mode:
            pCfg->debug_mode = 5;
            break;
        }
    } else {
        pCfg->debug_mode = 0;
    }
    if (psta->lowPowerCfg.detailShpLP.hw_shpCfg_lp_en) {
        pCfg->detail_lp_en = 1;
    } else {
        pCfg->detail_lp_en = 0;
    }

    // method0: pos sum of d_kernel(derivative kernel) = 22 * 16 / 22, pos sum of n_kernel(noise kernel) = 2
    // method1: pos sum of d_kernel(derivative kernel) = 16,           pos sum of n_kernel(noise kernel) = 2
    // only consider pos coeffs: derivative = 1023 * 16 * 2, calc noise = 1023 * 2
    // noise_norm_scale = 16
    // Range: fixed num 16 * scale range[0, 63.9] = 1023, 10bit
    int32_t noise_norm_scale = 16;
    // REG: TEXTURE0
    tmp = noise_norm_scale * (pTexDyn->texEst.hw_texEstT_nsEstDf1_scale) * (1 << 6);
    pCfg->fst_noise_scale = CLIP(tmp, 0, 0xffff);
    tmp = noise_norm_scale * (pTexDyn->texEst.hw_texEstT_sigmaDf1_scale) * (1 << 6);
    pCfg->fst_sigma_scale = CLIP(tmp, 0, 0xffff);
    // REG: TEXTURE1
    tmp = (pTexDyn->texEst.hw_texEstT_sigmaDf1_offset) * (1 << 0);
    pCfg->fst_sigma_offset = CLIP(tmp, 0, 0xffff);
    tmp = (pTexDyn->texEst.hw_texEstT_texEstDf1_scale) * (1 << 10);
    pCfg->fst_wgt_scale = CLIP(tmp, 0, 0x7fff);
    // REG: TEXTURE2
    pCfg->tex_wgt_mode = CLIP(pTexDyn->texEst.hw_texEstT_wgtOpt_mode, 0, 0x3);
    tmp = (pTexDyn->noiseEst.hw_texEstT_nsEstMean_alpha) * (1 << 5);
    pCfg->noise_est_alpha = CLIP(tmp, 0, 0x3f);
    // REG: TEXTURE3
    tmp = noise_norm_scale * (pTexDyn->texEst.hw_texEstT_nsEstDf2_scale) * (1 << 6);
    pCfg->sec_noise_scale = CLIP(tmp, 0, 0xffff);
    tmp = noise_norm_scale * (pTexDyn->texEst.hw_texEstT_sigmaDf2_scale) * (1 << 6);
    pCfg->sec_sigma_scale = CLIP(tmp, 0, 0xffff);
    // REG: TEXTURE4
    tmp = (pTexDyn->texEst.hw_texEstT_sigmaDf2_offset) * (1 << 0);
    pCfg->sec_sigma_offset = CLIP(tmp, 0, 0xffff);
    tmp = (pTexDyn->texEst.hw_texEstT_texEstDf2_scale) * (1 << 10);
    pCfg->sec_wgt_scale = CLIP(tmp, 0, 0x7fff);

    {
        int coeff[6];
        if (pdyn->eHfDetailShp.detailExtra_hpf.sw_shpT_filtCfg_mode == shp_cfgByFiltStrg_mode) {
            int tmp_hpf_coeff0[6] = { 0 };
            int tmp_hpf_coeff1[6] = { 0 };
            float rsigma = pdyn->eHfDetailShp.detailExtra_hpf.sw_shpT_filtSpatial_strg;
            SharpCreateKernelCoeffs(1, 1, rsigma,           &tmp_hpf_coeff0[0], 7, 2);
            SharpCreateKernelCoeffs(2, 2, rsigma * sqrt(2), &tmp_hpf_coeff1[0], 7, 2);
            for (i = 0; i < 6; i++) {
                if (i == 0) {
                    coeff[i] = (1 << 7) - (2 * tmp_hpf_coeff0[i] - tmp_hpf_coeff1[i]);
                } else {
                    coeff[i] = 0 - (2 * tmp_hpf_coeff0[i] - tmp_hpf_coeff1[i]);
                }
            }
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = ROUND_F(pdyn->eHfDetailShp.detailExtra_hpf.hw_shpT_filtSpatial_wgt[i] * (1 << 7));
        }

        for (int k = 0; k < 6; k++) {
            int coeff_maxLimit = kernel_coeffs_maxLimit[k];
            coeff[k] = MIN(coeff[k],  coeff_maxLimit);
            coeff[k] = MAX(coeff[k], -coeff_maxLimit);
        }

        SharpKernelCoeffsNormalization(coeff, 2, 0, 2);

        //oyyf add for negative reg value
        for(int k = 1; k < 3; k++) {
            if(coeff[k] < 0) {
                tmp = coeff[k];
                coeff[k] =  tmp & ((1 << 6) - 1);
            }
        }
        //oyyf add for negative reg value
        for(int k = 3; k < 6; k++) {
            if(coeff[k] < 0) {
                tmp = coeff[k];
                coeff[k] =  tmp & ((1 << 5) - 1);
            }
        }

        // REG: HPF_KERNEL0
        pCfg->img_hpf_coeff[0] = CLIP(coeff[0], 0, 0xff);
        // REG: HPF_KERNEL1
        pCfg->img_hpf_coeff[1] = CLIP(coeff[1], 0, 0x3f);
        pCfg->img_hpf_coeff[2] = CLIP(coeff[2], 0, 0x3f);
        pCfg->img_hpf_coeff[3] = CLIP(coeff[3], 0, 0x1f);
        pCfg->img_hpf_coeff[4] = CLIP(coeff[4], 0, 0x1f);
        pCfg->img_hpf_coeff[5] = CLIP(coeff[5], 0, 0x1f);
    }

    {
        int coeff[3];
        if (pTexDyn->texEstFlt.sw_texEstT_filtCfg_mode == texEst_cfgByFiltStrg_mode) {
            float rsigma = pTexDyn->texEstFlt.sw_texEstT_filtSpatial_strg;
            SharpCreateKernelCoeffs(1, 1, rsigma, coeff, 6, 2);
        } else {
            for (i = 0; i < 3; i++)
                coeff[i] = pTexDyn->texEstFlt.sw_texEstT_filtSpatial_wgt[i] * (1 << 6);
        }

        SharpKernelCoeffsNormalization(coeff, 1, (1 << 6), 2);

        // REG: TEXFLT_KERNEL
        pCfg->texWgt_flt_coeff0 = CLIP(coeff[0], 0, 0x3f);
        pCfg->texWgt_flt_coeff1 = CLIP(coeff[1], 0, 0x3f);
        pCfg->texWgt_flt_coeff2 = CLIP(coeff[2], 0, 0x3f);
    }

    // REG: DETAIL0
    tmp = (pdyn->detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcMf_alpha) * (1 << 5);
    pCfg->detail_in_alpha = CLIP(tmp, 0, 0x3f);
    tmp = (pdyn->detailShp.detailExtra_preBifilt.hw_shpT_rgeWgt_slope) * (1 << 8);
    pCfg->pre_bifilt_slope_fix = CLIP(tmp, 0, 0x7ff);
    tmp = (pdyn->detailShp.detailExtra_lpfSrc.hw_shpT_detailSrcHf_alpha) * (1 << 5);
    pCfg->pre_bifilt_alpha = CLIP(tmp, 0, 0x3f);
    pCfg->fusion_wgt_min_limit = pdyn->detailShp.detailFusion.hw_shpT_hiDetailAlpha_minLimit;
    pCfg->fusion_wgt_max_limit = pdyn->detailShp.detailFusion.hw_shpT_hiDetailAlpha_maxLimit;

    {
        int16_t luma_dx[7];
        float *luma_point = psta->lumaLutCfg.hw_shpT_lumaLutIdx_val;
        for(i = 0; i < 7; i++) {
            luma_dx[i] = (int16_t)LOG2(luma_point[i + 1] - luma_point[i]);
        }
        // REG: LUMA_DX
        pCfg->luma_dx[0] = CLIP(luma_dx[0], 0, 0xf);
        pCfg->luma_dx[1] = CLIP(luma_dx[1], 0, 0xf);
        pCfg->luma_dx[2] = CLIP(luma_dx[2], 0, 0xf);
        pCfg->luma_dx[3] = CLIP(luma_dx[3], 0, 0xf);
        pCfg->luma_dx[4] = CLIP(luma_dx[4], 0, 0xf);
        pCfg->luma_dx[5] = CLIP(luma_dx[5], 0, 0xf);
        pCfg->luma_dx[6] = CLIP(luma_dx[6], 0, 0xf);
    }

    {
        float cur_sigma;
        float constant_coeff                    = sqrt(log2(exp(1)));
        uint16_t *vsigma = pdyn->detailShp.sgmEnv.sw_shpC_luma2Sigma_curve.val;
        float preBifilt_scale = pdyn->detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_scale;
        uint16_t preBifilt_offset = pdyn->detailShp.detailExtra_preBifilt.sw_shpT_rgeSgm_offset;
        float preBifilt_slope = pdyn->detailShp.detailExtra_preBifilt.hw_shpT_rgeWgt_slope;
        int preBifilt_vsigma_inv[8];
        for(int i = 0; i < 8; i++) {
            cur_sigma = MAX(0, vsigma[i]) * preBifilt_scale + preBifilt_offset;
            preBifilt_vsigma_inv[i] = MIN(0x3fff, (int16_t)ROUND_F((float)constant_coeff * preBifilt_slope / cur_sigma * (1 << 14)));
        }
        // REG: PBF_VSIGMA0
        pCfg->pre_bifilt_vsigma_inv[0] = CLIP(preBifilt_vsigma_inv[0], 0, 0x3fff);
        pCfg->pre_bifilt_vsigma_inv[1] = CLIP(preBifilt_vsigma_inv[1], 0, 0x3fff);
        // REG: PBF_VSIGMA1
        pCfg->pre_bifilt_vsigma_inv[2] = CLIP(preBifilt_vsigma_inv[2], 0, 0x3fff);
        pCfg->pre_bifilt_vsigma_inv[3] = CLIP(preBifilt_vsigma_inv[3], 0, 0x3fff);
        // REG: PBF_VSIGMA2
        pCfg->pre_bifilt_vsigma_inv[4] = CLIP(preBifilt_vsigma_inv[4], 0, 0x3fff);
        pCfg->pre_bifilt_vsigma_inv[5] = CLIP(preBifilt_vsigma_inv[5], 0, 0x3fff);
        // REG: PBF_VSIGMA3
        pCfg->pre_bifilt_vsigma_inv[6] = CLIP(preBifilt_vsigma_inv[6], 0, 0x3fff);
        pCfg->pre_bifilt_vsigma_inv[7] = CLIP(preBifilt_vsigma_inv[7], 0, 0x3fff);
    }

    {
        int coeff[3];
        if (pdyn->detailShp.detailExtra_preBifilt.sw_shpT_filtCfg_mode == shp_cfgByFiltStrg_mode) {
            float rsigma = pdyn->detailShp.detailExtra_preBifilt.sw_shpT_filtSpatial_strg;
            SharpCreateKernelCoeffs(1, 1, rsigma, coeff, 6, 2);
        } else {
            for (i = 0; i < 3; i++)
                coeff[i] = ROUND_F(pdyn->detailShp.detailExtra_preBifilt.hw_shpT_filtSpatial_wgt[i] * (1 << 6));
        }

        SharpKernelCoeffsNormalization(coeff, 1, (1 << 6), 2);

        // REG: PBF_KERNEL
        pCfg->pre_bifilt_coeff0 = CLIP(coeff[0], 0, 0x3f);
        pCfg->pre_bifilt_coeff1 = CLIP(coeff[1], 0, 0x3f);
        pCfg->pre_bifilt_coeff2 = CLIP(coeff[2], 0, 0x3f);
    }

    {
        int coeff[6];
        int radius = pdyn->detailShp.hiDetailExtra_lpf.sw_shpT_filtRadius_mode;
        if (pdyn->detailShp.hiDetailExtra_lpf.sw_shpT_filtCfg_mode == shp_cfgByFiltStrg_mode) {
            float rsigma = pdyn->detailShp.hiDetailExtra_lpf.sw_shpT_filtSpatial_strg;
            SharpCreateKernelCoeffs(radius, 5 / 2, rsigma, coeff, 7, 2);
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = ROUND_F(pdyn->detailShp.hiDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i] * (1 << 7));
        }

        for (int k = 0; k < 6; k++) {
            int coeff_maxLimit = kernel_coeffs_maxLimit[k];
            coeff[k] = MIN(coeff[k],  coeff_maxLimit);
        }
        SharpKernelCoeffsNormalization(coeff, 5 / 2, (1 << 7), 2);

        // REG: DETAIL_KERNEL0
        pCfg->hi_detail_lpf_coeff[0] = CLIP(coeff[0], 0, 0x7f);
        pCfg->hi_detail_lpf_coeff[1] = CLIP(coeff[1], 0, 0x1f);
        pCfg->hi_detail_lpf_coeff[2] = CLIP(coeff[2], 0, 0x1f);
        pCfg->hi_detail_lpf_coeff[3] = CLIP(coeff[3], 0, 0xf);
        // REG: DETAIL_KERNEL1
        pCfg->hi_detail_lpf_coeff[4] = CLIP(coeff[4], 0, 0xf);
        pCfg->hi_detail_lpf_coeff[5] = CLIP(coeff[5], 0, 0xf);
    }

    {
        int coeff[6];
        int radius = pdyn->detailShp.midDetailExtra_lpf.sw_shpT_filtRadius_mode;
        if (pdyn->detailShp.midDetailExtra_lpf.sw_shpT_filtCfg_mode == shp_cfgByFiltStrg_mode) {
            float rsigma = pdyn->detailShp.midDetailExtra_lpf.sw_shpT_filt_strg;
            SharpCreateKernelCoeffs(radius, 5 / 2, rsigma, coeff, 7, 2);
        } else {
            for (i = 0; i < 6; i++)
                coeff[i] = ROUND_F(pdyn->detailShp.midDetailExtra_lpf.hw_shpT_filtSpatial_wgt[i] * (1 << 7));
        }

        for (int k = 0; k < 6; k++) {
            int coeff_maxLimit = kernel_coeffs_maxLimit[k];
            coeff[k] = MIN(coeff[k],  coeff_maxLimit);
        }
        SharpKernelCoeffsNormalization(coeff, 5 / 2, (1 << 7), 2);

        pCfg->mi_detail_lpf_coeff[0] = CLIP(coeff[0], 0, 0x7f);
        pCfg->mi_detail_lpf_coeff[1] = CLIP(coeff[1], 0, 0x1f);
        // REG: DETAIL_KERNEL2
        pCfg->mi_detail_lpf_coeff[2] = CLIP(coeff[2], 0, 0x1f);
        pCfg->mi_detail_lpf_coeff[3] = CLIP(coeff[3], 0, 0xf);
        pCfg->mi_detail_lpf_coeff[4] = CLIP(coeff[4], 0, 0xf);
        pCfg->mi_detail_lpf_coeff[5] = CLIP(coeff[5], 0, 0xf);
    }

    // REG: GAIN
    tmp = (pdyn->locShpStrg.motionStrg.hw_shpT_glbSgmStrg_val) * (1 << 4);
    pCfg->global_gain = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->locShpStrg.motionStrg.hw_shpT_glbSgmStrg_alpha) * (1 << 3);
    pCfg->gain_merge_alpha = CLIP(tmp, 0, 0xf);
    tmp = (pdyn->locShpStrg.motionStrg.hw_shpT_localSgmStrg_scale) * (1 << 7);
    pCfg->local_gain_scale = CLIP(tmp, 0, 0xff);


    {
        float slope, x_offset, minLimit, maxLimit, staticRegion_thred, motionRegion_thred;
        shp_motionStrg1_t *motionStrg1 = NULL;

        // REG: GAIN_ADJ
        motionStrg1 = &pdyn->edgeShp.locShpStrg_motionStrg1;
        minLimit = MIN(motionStrg1->hw_shpT_motRegionShp_strg, motionStrg1->hw_shpT_statRegionShp_strg);
        maxLimit = MAX(motionStrg1->hw_shpT_motRegionShp_strg, motionStrg1->hw_shpT_statRegionShp_strg);
        staticRegion_thred = motionStrg1->sw_shpT_locSgmStrgStat_maxThred;
        motionRegion_thred = motionStrg1->sw_shpT_locSgmStrgMot_minThred;
        if (motionStrg1->hw_shpT_statRegionShp_strg < motionStrg1->hw_shpT_motRegionShp_strg) {
            slope = (maxLimit - minLimit) / (motionRegion_thred - staticRegion_thred);
            x_offset = staticRegion_thred;
        } else {
            slope = -1.0f * (maxLimit - minLimit) / (motionRegion_thred - staticRegion_thred);
            x_offset = motionRegion_thred;
        }
        tmp = (maxLimit) * (1 << 7);
        pCfg->edge_gain_max_limit = CLIP(tmp, 0, 0xff);
        tmp = (minLimit) * (1 << 7);
        pCfg->edge_gain_min_limit = CLIP(tmp, 0, 0xff);
        tmp = (slope) * (1 << 3);
        pCfg->edge_gain_slope = CLIP(tmp, -127, 127);
        tmp = (x_offset) * (1 << 4);
        pCfg->edge_gain_offset = CLIP(tmp, 0, 0x3ff);

        motionStrg1 = &pdyn->detailShp.locShpStrg_motionStrg1;
        minLimit = MIN(motionStrg1->hw_shpT_motRegionShp_strg, motionStrg1->hw_shpT_statRegionShp_strg);
        maxLimit = MAX(motionStrg1->hw_shpT_motRegionShp_strg, motionStrg1->hw_shpT_statRegionShp_strg);
        staticRegion_thred = motionStrg1->sw_shpT_locSgmStrgStat_maxThred;
        motionRegion_thred = motionStrg1->sw_shpT_locSgmStrgMot_minThred;
        if (motionStrg1->hw_shpT_statRegionShp_strg < motionStrg1->hw_shpT_motRegionShp_strg) {
            slope = (maxLimit - minLimit) / (motionRegion_thred - staticRegion_thred);
            x_offset = staticRegion_thred;
        } else {
            slope = -1.0f * (maxLimit - minLimit) / (motionRegion_thred - staticRegion_thred);
            x_offset = motionRegion_thred;
        }
        tmp = (maxLimit) * (1 << 7);
        pCfg->detail_gain_max_limit = CLIP(tmp, 0, 0xff);
        tmp = (minLimit) * (1 << 7);
        pCfg->detail_gain_min_limit = CLIP(tmp, 0, 0xff);
        tmp = (slope) * (1 << 3);
        pCfg->detail_gain_slope = CLIP(tmp, -127, 127);
        tmp = (x_offset) * (1 << 4);
        pCfg->detail_gain_offset = CLIP(tmp, 0, 0x3ff);

        motionStrg1 = &pdyn->dHfDetailShp.locShpStrg_motionStrg1;
        minLimit = MIN(motionStrg1->hw_shpT_motRegionShp_strg, motionStrg1->hw_shpT_statRegionShp_strg);
        maxLimit = MAX(motionStrg1->hw_shpT_motRegionShp_strg, motionStrg1->hw_shpT_statRegionShp_strg);
        staticRegion_thred = motionStrg1->sw_shpT_locSgmStrgStat_maxThred;
        motionRegion_thred = motionStrg1->sw_shpT_locSgmStrgMot_minThred;
        if (motionStrg1->hw_shpT_statRegionShp_strg < motionStrg1->hw_shpT_motRegionShp_strg) {
            slope = (maxLimit - minLimit) / (motionRegion_thred - staticRegion_thred);
            x_offset = staticRegion_thred;
        } else {
            slope = -1.0f * (maxLimit - minLimit) / (motionRegion_thred - staticRegion_thred);
            x_offset = motionRegion_thred;
        }
        tmp = (maxLimit) * (1 << 7);
        pCfg->hitex_gain_max_limit = CLIP(tmp, 0, 0xff);
        tmp = (minLimit) * (1 << 7);
        pCfg->hitex_gain_min_limit = CLIP(tmp, 0, 0xff);
        tmp = (slope) * (1 << 3);
        pCfg->hitex_gain_slope = CLIP(tmp, -127, 127);
        tmp = (x_offset) * (1 << 4);
        pCfg->hitex_gain_offset = CLIP(tmp, 0, 0x3ff);
    }

    // REG: GAIN_ADJ4
    tmp = (pdyn->edgeShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma) * (1 << 11);
    pCfg->edge_gain_sigma = CLIP(tmp, 0, 0xfff);
    tmp = (pdyn->detailShp.locShpStrg_motionStrg2.hw_shpT_motionStrg_sigma) * (1 << 11);
    pCfg->detail_gain_sigma = CLIP(tmp, 0, 0xfff);
    // REG: EDGE0
    tmp = (pdyn->edgeShp.locShpStrg_edge.hw_shpT_edgePosIdx_scale) * (1 << 2);
    pCfg->pos_edge_wgt_scale = CLIP(tmp, 0, 0xfff);
    tmp = (pdyn->edgeShp.locShpStrg_edge.hw_shpT_edgeNegIdx_scale) * (1 << 2);
    pCfg->neg_edge_wgt_scale = CLIP(tmp, 0, 0xfff);
    // REG: EDGE1
    tmp = (pdyn->edgeShp.glbShpStrg.hw_shpT_edgePos_strg) * (1 << 2);
    pCfg->pos_edge_strg = CLIP(tmp, 0, 0x3f);
    tmp = (pdyn->edgeShp.glbShpStrg.hw_shpT_edgeNeg_strg) * (1 << 2);
    pCfg->neg_edge_strg = CLIP(tmp, 0, 0x3f);
    tmp = (pdyn->edgeShp.shootReduction.sw_shpT_overShoot_alpha) * (1 << 5);
    pCfg->overshoot_alpha = CLIP(tmp, 0, 0x3f);
    tmp = (pdyn->edgeShp.shootReduction.sw_shpT_underShoot_alpha) * (1 << 5);
    pCfg->undershoot_alpha = CLIP(tmp, 0, 0x3f);

    {
        int coeff[10];
        int radius = pdyn->edgeShp.edgeExtra.sw_shpT_filtRadius_mode;
        if (pdyn->edgeShp.edgeExtra.sw_shpT_filtCfg_mode == shp_cfgByFiltStrg_mode) {
            float rsigma = pdyn->edgeShp.edgeExtra.sw_shpT_filtSpatial_strg;
            SharpCreateKernelCoeffs(radius, 7 / 2, rsigma, coeff, 7, 2);
        } else {
            for (i = 0; i < 10; i++)
                coeff[i] = ROUND_F(pdyn->edgeShp.edgeExtra.hw_shpT_filtSpatial_wgt[i] * (1 << 7));
        }

        for (int k = 0; k < 10; k++) {
            int coeff_maxLimit = kernel_coeffs_maxLimit[k];
            coeff[k] = MIN(coeff[k],  coeff_maxLimit);
        }
        SharpKernelCoeffsNormalization(coeff, 7 / 2, (1 << 7), 2);

        // REG: EDGE_KERNEL0
        pCfg->edge_lpf_coeff[0] = CLIP(coeff[0], 0, 0x7f);
        pCfg->edge_lpf_coeff[1] = CLIP(coeff[1], 0, 0x1f);
        pCfg->edge_lpf_coeff[2] = CLIP(coeff[2], 0, 0x1f);
        pCfg->edge_lpf_coeff[3] = CLIP(coeff[3], 0, 0xf);
        // REG: EDGE_KERNEL1
        pCfg->edge_lpf_coeff[4] = CLIP(coeff[4], 0, 0xf);
        pCfg->edge_lpf_coeff[5] = CLIP(coeff[5], 0, 0xf);
        pCfg->edge_lpf_coeff[6] = CLIP(coeff[6], 0, 0xf);
        pCfg->edge_lpf_coeff[7] = CLIP(coeff[7], 0, 0xf);
        // REG: EDGE_KERNEL2
        pCfg->edge_lpf_coeff[8] = CLIP(coeff[8], 0, 0xf);
        pCfg->edge_lpf_coeff[9] = CLIP(coeff[9], 0, 0xf);
    }

    {
        int edgeWgt_val[17];
        if (pdyn->edgeShp.locShpStrg_edge.sw_shpT_edgeStrgCurve_mode == shp_cfgCurveDirect_mode) {
            for (i = 0; i < 17; i++) {
                edgeWgt_val[i] = pdyn->edgeShp.locShpStrg_edge.hw_shpT_edge2ShpStrg_val[i];
            }
        } else {
            float power                         = pdyn->edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_curvePower_val;
            float edgeWgt_minLimit              = pdyn->edgeShp.locShpStrg_edge.edgeStrgCurveCtrl.sw_shpT_edgeStrg_minLimit;
            for (i = 0; i < 17; i++) {
                edgeWgt_val[i] = MIN(1023, edgeWgt_minLimit + ROUND_F(1024 * (1 - pow(1 - pow(i * 0.0625f, power), power))));
            }
        }
        // REG: EDGE_WGT_VAL0
        pCfg->edge_wgt_val[0] = CLIP(edgeWgt_val[0], 0, 0x3ff);
        pCfg->edge_wgt_val[1] = CLIP(edgeWgt_val[1], 0, 0x3ff);
        pCfg->edge_wgt_val[2] = CLIP(edgeWgt_val[2], 0, 0x3ff);
        // REG: EDGE_WGT_VAL1
        pCfg->edge_wgt_val[3] = CLIP(edgeWgt_val[3], 0, 0x3ff);
        pCfg->edge_wgt_val[4] = CLIP(edgeWgt_val[4], 0, 0x3ff);
        pCfg->edge_wgt_val[5] = CLIP(edgeWgt_val[5], 0, 0x3ff);
        // REG: EDGE_WGT_VAL2
        pCfg->edge_wgt_val[6] = CLIP(edgeWgt_val[6], 0, 0x3ff);
        pCfg->edge_wgt_val[7] = CLIP(edgeWgt_val[7], 0, 0x3ff);
        pCfg->edge_wgt_val[8] = CLIP(edgeWgt_val[8], 0, 0x3ff);
        // REG: EDGE_WGT_VAL3
        pCfg->edge_wgt_val[9] = CLIP(edgeWgt_val[9], 0, 0x3ff);
        pCfg->edge_wgt_val[10] = CLIP(edgeWgt_val[10], 0, 0x3ff);
        pCfg->edge_wgt_val[11] = CLIP(edgeWgt_val[11], 0, 0x3ff);
        // REG: EDGE_WGT_VAL4
        pCfg->edge_wgt_val[12] = CLIP(edgeWgt_val[12], 0, 0x3ff);
        pCfg->edge_wgt_val[13] = CLIP(edgeWgt_val[13], 0, 0x3ff);
        pCfg->edge_wgt_val[14] = CLIP(edgeWgt_val[14], 0, 0x3ff);
        // REG: EDGE_WGT_VAL5
        pCfg->edge_wgt_val[15] = CLIP(edgeWgt_val[15], 0, 0x3ff);
        pCfg->edge_wgt_val[16] = CLIP(edgeWgt_val[16], 0, 0x3ff);
    }

    {
        int luma_adj_strg[8];
        if(pdyn->locShpStrg.luma.sw_shpT_luma_en) {
            for (i = 0; i < 8; i++) {
                luma_adj_strg[i] = ROUND_F(pdyn->locShpStrg.luma.hw_shpT_luma2ShpStrg_val[i] * (1 << 7));
            }
        } else {
            for (i = 0; i < 8; i++) {
                luma_adj_strg[i] = ROUND_F(1 * (1 << 7));
            }
        }
        // REG: LUMA_ADJ_STRG0
        pCfg->luma2strg[0] = CLIP(luma_adj_strg[0], 0, 0xff);
        pCfg->luma2strg[1] = CLIP(luma_adj_strg[1], 0, 0xff);
        pCfg->luma2strg[2] = CLIP(luma_adj_strg[2], 0, 0xff);
        pCfg->luma2strg[3] = CLIP(luma_adj_strg[3], 0, 0xff);
        // REG: LUMA_ADJ_STRG1
        pCfg->luma2strg[4] = CLIP(luma_adj_strg[4], 0, 0xff);
        pCfg->luma2strg[5] = CLIP(luma_adj_strg[5], 0, 0xff);
        pCfg->luma2strg[6] = CLIP(luma_adj_strg[6], 0, 0xff);
        pCfg->luma2strg[7] = CLIP(luma_adj_strg[7], 0, 0xff);
    }

    {
        int center_x = convert_coordinate(psta->locShpStrg_radiDist.hw_shpCfg_opticCenter_x, cols);
        int center_y = convert_coordinate(psta->locShpStrg_radiDist.hw_shpCfg_opticCenter_y, rows);
        // REG: CENTER
        pCfg->center_x = CLIP(center_x, 0, 0x1fff);
        pCfg->center_y = CLIP(center_y, 0, 0x1fff);
    }

    {
        int tex_wgt_table_x[4];
        int tex_x_inv_fix[3];
        int flat_maxLimit = pdyn->locShpStrg.texRegion_clsfBaseTex.hw_shpT_flatRegion_maxThred;
        int edge_minLimit = pdyn->locShpStrg.texRegion_clsfBaseTex.hw_shpT_edgeRegion_minThred;

        flat_maxLimit = MAX(flat_maxLimit, 1);
        flat_maxLimit = MIN(flat_maxLimit, 0x3fe);
        edge_minLimit = MAX(edge_minLimit, flat_maxLimit + 1);
        edge_minLimit = MIN(edge_minLimit, 0x3ff);
        tex_wgt_table_x[0] = 0;
        tex_wgt_table_x[1] = flat_maxLimit;
        tex_wgt_table_x[2] = edge_minLimit;
        tex_wgt_table_x[3] = (1 << 10);

        for (i = 0; i < 3; i++) {
            tex_x_inv_fix[i] = (1 << 22) / (tex_wgt_table_x[i + 1] - tex_wgt_table_x[i]);
        }

        // REG: OUT_LIMIT
        pCfg->flat_max_limit = CLIP(flat_maxLimit, 0, 0x3ff);
        pCfg->edge_min_limit = CLIP(edge_minLimit, 0, 0x3ff);

        // REG: TEX_X_INV_FIX0
        pCfg->tex_x_inv_fix0 = CLIP(tex_x_inv_fix[0], 0, 0x7fffff);
        // REG: TEX_X_INV_FIX1
        pCfg->tex_x_inv_fix1 = CLIP(tex_x_inv_fix[1], 0, 0x7fffff);
        // REG: TEX_X_INV_FIX2
        pCfg->tex_x_inv_fix2 = CLIP(tex_x_inv_fix[2], 0, 0x7fffff);

        // REG: DETAIL1
        pCfg->detail_fusion_slope_fix = CLIP(tex_x_inv_fix[1], 0, 0x7fffff);
    }

    {
        int local_strg_detail[4];
        int local_strg_hiTex[4];
        local_strg_detail[0] = ROUND_F(pdyn->detailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg * (1 << 10));
        local_strg_detail[1] = ROUND_F(pdyn->detailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg * (1 << 10));
        local_strg_detail[2] = ROUND_F(pdyn->detailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg * (1 << 10));
        local_strg_detail[3] = ROUND_F(pdyn->detailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg * (1 << 10));
        local_strg_hiTex[0] = ROUND_F(pdyn->dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionL_strg * (1 << 10));
        local_strg_hiTex[1] = ROUND_F(pdyn->dHfDetailShp.locShpStrg_texRegion.hw_shpT_flatRegionR_strg * (1 << 10));
        local_strg_hiTex[2] = ROUND_F(pdyn->dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionL_strg * (1 << 10));
        local_strg_hiTex[3] = ROUND_F(pdyn->dHfDetailShp.locShpStrg_texRegion.hw_shpT_edgeRegionR_strg * (1 << 10));

        // REG: LOCAL_STRG0
        pCfg->tex2detail_strg[0] = CLIP(local_strg_detail[0], 0, 0x3ff);
        pCfg->tex2detail_strg[1] = CLIP(local_strg_detail[1], 0, 0x3ff);
        pCfg->tex2detail_strg[2] = CLIP(local_strg_detail[2], 0, 0x3ff);
        // REG: LOCAL_STRG1
        pCfg->tex2detail_strg[3] = CLIP(local_strg_detail[3], 0, 0x3ff);
        pCfg->tex2loss_tex_in_hinr_strg[0] = CLIP(local_strg_hiTex[0], 0, 0x3ff);
        pCfg->tex2loss_tex_in_hinr_strg[1] = CLIP(local_strg_hiTex[1], 0, 0x3ff);
        // REG: LOCAL_STRG2
        pCfg->tex2loss_tex_in_hinr_strg[2] = CLIP(local_strg_hiTex[2], 0, 0x3ff);
        pCfg->tex2loss_tex_in_hinr_strg[3] = CLIP(local_strg_hiTex[3], 0, 0x3ff);
    }

    {
        int detail_pos_scale[9];
        int detail_neg_scale[9];
        for (i = 0; i < 9; i++) {
            detail_pos_scale[i] = ROUND_F(pdyn->detailShp.locShpStrg_contrast.hw_shpT_contrast2posStrg_val[i] * (1 << 7));
            detail_neg_scale[i] = ROUND_F(pdyn->detailShp.locShpStrg_contrast.hw_shpT_contrast2negStrg_val[i] * (1 << 7));
        }
        // REG: DETAIL_SCALE_TAB0
        pCfg->contrast2pos_strg[0] = CLIP(detail_pos_scale[0], 0, 0xff);
        pCfg->contrast2pos_strg[1] = CLIP(detail_pos_scale[1], 0, 0xff);
        pCfg->contrast2pos_strg[2] = CLIP(detail_pos_scale[2], 0, 0xff);
        pCfg->contrast2pos_strg[3] = CLIP(detail_pos_scale[3], 0, 0xff);
        // REG: DETAIL_SCALE_TAB1
        pCfg->contrast2pos_strg[4] = CLIP(detail_pos_scale[4], 0, 0xff);
        pCfg->contrast2pos_strg[5] = CLIP(detail_pos_scale[5], 0, 0xff);
        pCfg->contrast2pos_strg[6] = CLIP(detail_pos_scale[6], 0, 0xff);
        pCfg->contrast2pos_strg[7] = CLIP(detail_pos_scale[7], 0, 0xff);
        // REG: DETAIL_SCALE_TAB2
        pCfg->contrast2pos_strg[8] = CLIP(detail_pos_scale[8], 0, 0xff);
        tmp = (pdyn->detailShp.glbShpStrg.hw_shpT_detailPos_strg) * (1 << 2);
        pCfg->pos_detail_strg = CLIP(tmp, 0, 0x3f);
        // REG: DETAIL_SCALE_TAB3
        pCfg->contrast2neg_strg[0] = CLIP(detail_neg_scale[0], 0, 0xff);
        pCfg->contrast2neg_strg[1] = CLIP(detail_neg_scale[1], 0, 0xff);
        pCfg->contrast2neg_strg[2] = CLIP(detail_neg_scale[2], 0, 0xff);
        pCfg->contrast2neg_strg[3] = CLIP(detail_neg_scale[3], 0, 0xff);
        // REG: DETAIL_SCALE_TAB4
        pCfg->contrast2neg_strg[4] = CLIP(detail_neg_scale[4], 0, 0xff);
        pCfg->contrast2neg_strg[5] = CLIP(detail_neg_scale[5], 0, 0xff);
        pCfg->contrast2neg_strg[6] = CLIP(detail_neg_scale[6], 0, 0xff);
        pCfg->contrast2neg_strg[7] = CLIP(detail_neg_scale[7], 0, 0xff);
        // REG: DETAIL_SCALE_TAB5
        pCfg->contrast2neg_strg[8] = CLIP(detail_neg_scale[8], 0, 0xff);
        tmp = (pdyn->detailShp.glbShpStrg.hw_shpT_detailNeg_strg) * (1 << 2);
        pCfg->neg_detail_strg = CLIP(tmp, 0, 0x3f);
    }

    {
        int detail_posClip_byTex[9];
        int detail_negClip_byTex[9];
        for (i = 0; i < 9; i++) {
            detail_posClip_byTex[i] = ROUND_F(pdyn->detailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i]);
            detail_negClip_byTex[i] = ROUND_F(pdyn->detailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i]);
        }
        // REG: DETAIL_TEX_CLIP0
        pCfg->tex2detail_pos_clip[0] = CLIP(detail_posClip_byTex[0], 0, 0x3ff);
        pCfg->tex2detail_pos_clip[1] = CLIP(detail_posClip_byTex[1], 0, 0x3ff);
        pCfg->tex2detail_pos_clip[2] = CLIP(detail_posClip_byTex[2], 0, 0x3ff);
        // REG: DETAIL_TEX_CLIP1
        pCfg->tex2detail_pos_clip[3] = CLIP(detail_posClip_byTex[3], 0, 0x3ff);
        pCfg->tex2detail_pos_clip[4] = CLIP(detail_posClip_byTex[4], 0, 0x3ff);
        pCfg->tex2detail_pos_clip[5] = CLIP(detail_posClip_byTex[5], 0, 0x3ff);
        // REG: DETAIL_TEX_CLIP2
        pCfg->tex2detail_pos_clip[6] = CLIP(detail_posClip_byTex[6], 0, 0x3ff);
        pCfg->tex2detail_pos_clip[7] = CLIP(detail_posClip_byTex[7], 0, 0x3ff);
        pCfg->tex2detail_pos_clip[8] = CLIP(detail_posClip_byTex[8], 0, 0x3ff);
        // REG: DETAIL_TEX_CLIP3
        pCfg->tex2detail_neg_clip[0] = CLIP(detail_negClip_byTex[0], 0, 0x3ff);
        pCfg->tex2detail_neg_clip[1] = CLIP(detail_negClip_byTex[1], 0, 0x3ff);
        pCfg->tex2detail_neg_clip[2] = CLIP(detail_negClip_byTex[2], 0, 0x3ff);
        // REG: DETAIL_TEX_CLIP4
        pCfg->tex2detail_neg_clip[3] = CLIP(detail_negClip_byTex[3], 0, 0x3ff);
        pCfg->tex2detail_neg_clip[4] = CLIP(detail_negClip_byTex[4], 0, 0x3ff);
        pCfg->tex2detail_neg_clip[5] = CLIP(detail_negClip_byTex[5], 0, 0x3ff);
        // REG: DETAIL_TEX_CLIP5
        pCfg->tex2detail_neg_clip[6] = CLIP(detail_negClip_byTex[6], 0, 0x3ff);
        pCfg->tex2detail_neg_clip[7] = CLIP(detail_negClip_byTex[7], 0, 0x3ff);
        pCfg->tex2detail_neg_clip[8] = CLIP(detail_negClip_byTex[8], 0, 0x3ff);
    }

    {
        int hiestf_posClip_byTex[9];
        int hiestf_negClip_byTex[9];
        for (i = 0; i < 9; i++) {
            hiestf_posClip_byTex[i] = ROUND_F(pdyn->eHfDetailShp.shootReduction.hw_shpT_tex2DetailPosClip_val[i]);
            hiestf_negClip_byTex[i] = ROUND_F(pdyn->eHfDetailShp.shootReduction.hw_shpT_tex2DetailNegClip_val[i]);
        }
        // REG: GRAIN_TEX_CLIP0
        pCfg->tex2grain_pos_clip[0] = CLIP(hiestf_posClip_byTex[0], 0, 0x3ff);
        pCfg->tex2grain_pos_clip[1] = CLIP(hiestf_posClip_byTex[1], 0, 0x3ff);
        pCfg->tex2grain_pos_clip[2] = CLIP(hiestf_posClip_byTex[2], 0, 0x3ff);
        // REG: GRAIN_TEX_CLIP1
        pCfg->tex2grain_pos_clip[3] = CLIP(hiestf_posClip_byTex[3], 0, 0x3ff);
        pCfg->tex2grain_pos_clip[4] = CLIP(hiestf_posClip_byTex[4], 0, 0x3ff);
        pCfg->tex2grain_pos_clip[5] = CLIP(hiestf_posClip_byTex[5], 0, 0x3ff);
        // REG: GRAIN_TEX_CLIP2
        pCfg->tex2grain_pos_clip[6] = CLIP(hiestf_posClip_byTex[6], 0, 0x3ff);
        pCfg->tex2grain_pos_clip[7] = CLIP(hiestf_posClip_byTex[7], 0, 0x3ff);
        pCfg->tex2grain_pos_clip[8] = CLIP(hiestf_posClip_byTex[8], 0, 0x3ff);
        // REG: GRAIN_TEX_CLIP3
        pCfg->tex2grain_neg_clip[0] = CLIP(hiestf_negClip_byTex[0], 0, 0x3ff);
        pCfg->tex2grain_neg_clip[1] = CLIP(hiestf_negClip_byTex[1], 0, 0x3ff);
        pCfg->tex2grain_neg_clip[2] = CLIP(hiestf_negClip_byTex[2], 0, 0x3ff);
        // REG: GRAIN_TEX_CLIP4
        pCfg->tex2grain_neg_clip[3] = CLIP(hiestf_negClip_byTex[3], 0, 0x3ff);
        pCfg->tex2grain_neg_clip[4] = CLIP(hiestf_negClip_byTex[4], 0, 0x3ff);
        pCfg->tex2grain_neg_clip[5] = CLIP(hiestf_negClip_byTex[5], 0, 0x3ff);
        // REG: GRAIN_TEX_CLIP5
        pCfg->tex2grain_neg_clip[6] = CLIP(hiestf_negClip_byTex[6], 0, 0x3ff);
        pCfg->tex2grain_neg_clip[7] = CLIP(hiestf_negClip_byTex[7], 0, 0x3ff);
        pCfg->tex2grain_neg_clip[8] = CLIP(hiestf_negClip_byTex[8], 0, 0x3ff);
    }

    {
        int detail_posClip_byLuma[8];
        int detail_negClip_byLuma[8];
        for (i = 0; i < 8; i++) {
            detail_posClip_byLuma[i] = ROUND_F(pdyn->detailShp.shootReduction.hw_shpT_luma2DetailPosClip_val[i]);
            detail_negClip_byLuma[i] = ROUND_F(pdyn->detailShp.shootReduction.hw_shpT_luma2DetailNegClip_val[i]);
        }
        // REG: DETAIL_LUMA_CLIP0
        pCfg->luma2detail_pos_clip[0] = CLIP(detail_posClip_byLuma[0], 0, 0x3ff);
        pCfg->luma2detail_pos_clip[1] = CLIP(detail_posClip_byLuma[1], 0, 0x3ff);
        pCfg->luma2detail_pos_clip[2] = CLIP(detail_posClip_byLuma[2], 0, 0x3ff);
        // REG: DETAIL_LUMA_CLIP1
        pCfg->luma2detail_pos_clip[3] = CLIP(detail_posClip_byLuma[3], 0, 0x3ff);
        pCfg->luma2detail_pos_clip[4] = CLIP(detail_posClip_byLuma[4], 0, 0x3ff);
        pCfg->luma2detail_pos_clip[5] = CLIP(detail_posClip_byLuma[5], 0, 0x3ff);
        // REG: DETAIL_LUMA_CLIP2
        pCfg->luma2detail_pos_clip[6] = CLIP(detail_posClip_byLuma[6], 0, 0x3ff);
        pCfg->luma2detail_pos_clip[7] = CLIP(detail_posClip_byLuma[7], 0, 0x3ff);
        // REG: DETAIL_LUMA_CLIP3
        pCfg->luma2detail_neg_clip[0] = CLIP(detail_negClip_byLuma[0], 0, 0x3ff);
        pCfg->luma2detail_neg_clip[1] = CLIP(detail_negClip_byLuma[1], 0, 0x3ff);
        pCfg->luma2detail_neg_clip[2] = CLIP(detail_negClip_byLuma[2], 0, 0x3ff);
        // REG: DETAIL_LUMA_CLIP4
        pCfg->luma2detail_neg_clip[3] = CLIP(detail_negClip_byLuma[3], 0, 0x3ff);
        pCfg->luma2detail_neg_clip[4] = CLIP(detail_negClip_byLuma[4], 0, 0x3ff);
        pCfg->luma2detail_neg_clip[5] = CLIP(detail_negClip_byLuma[5], 0, 0x3ff);
        // REG: DETAIL_LUMA_CLIP5
        pCfg->luma2detail_neg_clip[6] = CLIP(detail_negClip_byLuma[6], 0, 0x3ff);
        pCfg->luma2detail_neg_clip[7] = CLIP(detail_negClip_byLuma[7], 0, 0x3ff);
    }

    // REG: GRAIN_STRG
    tmp = (pdyn->eHfDetailShp.glbShpStrg.hw_shpT_eHfDetail_strg) * (1 << 2);
    pCfg->grain_strg = CLIP(tmp, 0, 0x3f);

    {
        int hue_adj_strg[9];
        if(pdyn->locShpStrg.hue.sw_shpT_hue_en) {
            for (i = 0; i < 9; i++) {
                hue_adj_strg[i] = ROUND_F(pdyn->locShpStrg.hue.hw_shpT_hue2ShpStrg_val[i] * (1 << 9));
            }
        } else {
            for (i = 0; i < 9; i++) {
                hue_adj_strg[i] = ROUND_F(1 * (1 << 9));
            }
        }
        // REG: HUE_ADJ_TAB0
        pCfg->hue2strg[0] = CLIP(hue_adj_strg[0], 0, 0x3ff);
        pCfg->hue2strg[1] = CLIP(hue_adj_strg[1], 0, 0x3ff);
        pCfg->hue2strg[2] = CLIP(hue_adj_strg[2], 0, 0x3ff);
        // REG: HUE_ADJ_TAB1
        pCfg->hue2strg[3] = CLIP(hue_adj_strg[3], 0, 0x3ff);
        pCfg->hue2strg[4] = CLIP(hue_adj_strg[4], 0, 0x3ff);
        pCfg->hue2strg[5] = CLIP(hue_adj_strg[5], 0, 0x3ff);
        // REG: HUE_ADJ_TAB2
        pCfg->hue2strg[6] = CLIP(hue_adj_strg[6], 0, 0x3ff);
        pCfg->hue2strg[7] = CLIP(hue_adj_strg[7], 0, 0x3ff);
        pCfg->hue2strg[8] = CLIP(hue_adj_strg[8], 0, 0x3ff);
    }

    {
        int distance_adj_strg[11];
        if(pdyn->locShpStrg.radiDist.sw_shpT_radiDist_en) {
            for (i = 0; i < 11; i++) {
                distance_adj_strg[i] = ROUND_F(pdyn->locShpStrg.radiDist.hw_shpT_radiDist2ShpStrg_val[i] * (1 << 7));
            }
        } else {
            for (i = 0; i < 11; i++) {
                distance_adj_strg[i] = ROUND_F(1 * (1 << 7));
            }
        }
        // REG: DISATANCE_ADJ0
        pCfg->distance2strg[0] = CLIP(distance_adj_strg[0], 0, 0xff);
        pCfg->distance2strg[1] = CLIP(distance_adj_strg[1], 0, 0xff);
        pCfg->distance2strg[2] = CLIP(distance_adj_strg[2], 0, 0xff);
        pCfg->distance2strg[3] = CLIP(distance_adj_strg[3], 0, 0xff);
        // REG: DISATANCE_ADJ1
        pCfg->distance2strg[4] = CLIP(distance_adj_strg[4], 0, 0xff);
        pCfg->distance2strg[5] = CLIP(distance_adj_strg[5], 0, 0xff);
        pCfg->distance2strg[6] = CLIP(distance_adj_strg[6], 0, 0xff);
        pCfg->distance2strg[7] = CLIP(distance_adj_strg[7], 0, 0xff);
        // REG: DISATANCE_ADJ2
        pCfg->distance2strg[8] = CLIP(distance_adj_strg[8], 0, 0xff);
        pCfg->distance2strg[9] = CLIP(distance_adj_strg[9], 0, 0xff);
        pCfg->distance2strg[10] = CLIP(distance_adj_strg[10], 0, 0xff);
    }

    int noise_curve_ext[17] = {0};
    {
        int hi_tex_threshold[9];
        for (i = 0; i < 17; i++) {
            float ynr_lo_noise_sigma = cvtinfo->ynr_sigma[i] * (1 << 3);
            float noiseSigma_scale = pdyn->dHfDetailShp.detailExtra.hw_shp_noiseThred_scale;
            int hi_tex_thred = ROUND_F(ynr_lo_noise_sigma * noiseSigma_scale / (1 << 3));
            if ((i & 1) == 0) {
                hi_tex_threshold[i >> 1] = MIN(0x3ff, hi_tex_thred);
            }
            if (cvtinfo->isFirstFrame) {
                noise_curve_ext[i] = MIN(2046, ROUND_INT(((int)ynr_lo_noise_sigma * 2), 3));
            }
        }
        // REG: NOISE_SIGMA0
        pCfg->hi_tex_threshold[0] = CLIP(hi_tex_threshold[0], 0, 0x3ff);
        pCfg->hi_tex_threshold[1] = CLIP(hi_tex_threshold[1], 0, 0x3ff);
        // REG: NOISE_SIGMA1
        pCfg->hi_tex_threshold[2] = CLIP(hi_tex_threshold[2], 0, 0x3ff);
        pCfg->hi_tex_threshold[3] = CLIP(hi_tex_threshold[3], 0, 0x3ff);
        // REG: NOISE_SIGMA2
        pCfg->hi_tex_threshold[4] = CLIP(hi_tex_threshold[4], 0, 0x3ff);
        pCfg->hi_tex_threshold[5] = CLIP(hi_tex_threshold[5], 0, 0x3ff);
        // REG: NOISE_SIGMA3
        pCfg->hi_tex_threshold[6] = CLIP(hi_tex_threshold[6], 0, 0x3ff);
        pCfg->hi_tex_threshold[7] = CLIP(hi_tex_threshold[7], 0, 0x3ff);
        // REG: NOISE_SIGMA4
        pCfg->hi_tex_threshold[8] = CLIP(hi_tex_threshold[8], 0, 0x3ff);
    }

    // REG: LOSSTEXINHINR_STRG
    tmp = (pdyn->dHfDetailShp.glbShpStrg.hw_shpT_dHiDetail_strg) * (1 << 2);
    pCfg->loss_tex_in_hinr_strg = CLIP(tmp, 0, 0x3f);

    // REG: NOISE_CURVE8
    tmp = (pTexDyn->noiseEst.hw_texEstT_nsStatsCntThd_ratio) * (1 << 10);
    pCfg->noise_count_thred_ratio = CLIP(tmp, 0, 0xff);
    tmp = (pTexDyn->noiseEst.hw_texEstT_nsEstTexThd_scale) * (1 << 4);
    pCfg->noise_clip_scale = CLIP(tmp, 0, 0xff);
    // REG: NOISE_CLIP
    tmp = (pTexDyn->noiseEst.hw_texEstT_nsEstTexThd_minLimit) * (1 << 0);
    pCfg->noise_clip_min_limit = CLIP(tmp, 0, 0x7ff);
    tmp = (pTexDyn->noiseEst.hw_texEstT_nsEstTexThd_maxLimit) * (1 << 0);
    pCfg->noise_clip_max_limit = CLIP(tmp, 0, 0x7ff);

    if (!cvtinfo->isFirstFrame) {
        sharp_stats_t *sharp_stats = &pBtnrInfo->mSharpStats[0];
        sharp_stats = sharp_get_stats(pBtnrInfo, cvtinfo->frameId);
        if (cvtinfo->frameId - pBtnrInfo->stats_delay_cnt != sharp_stats->id) {
            pBtnrInfo->sharp_stats_miss_cnt ++;
            if ((pBtnrInfo->sharp_stats_miss_cnt > 10) && (pBtnrInfo->sharp_stats_miss_cnt % 30 == 0)) {
                LOGE_ANR("Sharp stats miss match! frameId: %d stats [%d %d %d]", cvtinfo->frameId,
                         pBtnrInfo->mSharpStats[0].id, pBtnrInfo->mSharpStats[1].id, pBtnrInfo->mSharpStats[2].id);
                for (i = 0; i < 17; i++) {
                    noise_curve_ext[i] = pBtnrInfo->sharp_noise_curve_pre[i];
                }

            }
        } else {
            noiseCurveInterp(sharp_stats->noise_curve, noise_curve_ext, pBtnrInfo->sharp_noise_curve_pre);
        }
    }
    for (i = 0; i < 17; i++) {
        pBtnrInfo->sharp_noise_curve_pre[i] = noise_curve_ext[i];
    }

    // cal noise curve for gic
    if (pBtnrInfo->gic_noise_auto_mode && !cvtinfo->isFirstFrame) {
        struct isp33_gic_cfg* gic_cfg = &isp_params->isp_cfg->others.gic_cfg;
        for (i = 0; i < 17; i++) {
            gic_cfg->bfflt_vsigma_y[i] = CLIP(noise_curve_ext[i], 0, 0x3ff);
        }
    }

    if (pTexDyn->noiseEst.hw_texEstT_nsEstTexThd_mode != texEst_baseNoiseStats_mode) {
        for (i = 0; i < 17; i++) {
            noise_curve_ext[i] = pTexDyn->noiseEst.hw_texEstT_nsEstTexManual_thred[i];
        }
    }

    // REG: NOISE_CURVE0
    pCfg->noise_curve_ext[0] = CLIP(noise_curve_ext[0], 0, 0x7ff);
    pCfg->noise_curve_ext[1] = CLIP(noise_curve_ext[1], 0, 0x7ff);
    // REG: NOISE_CURVE1
    pCfg->noise_curve_ext[2] = CLIP(noise_curve_ext[2], 0, 0x7ff);
    pCfg->noise_curve_ext[3] = CLIP(noise_curve_ext[3], 0, 0x7ff);
    // REG: NOISE_CURVE2
    pCfg->noise_curve_ext[4] = CLIP(noise_curve_ext[4], 0, 0x7ff);
    pCfg->noise_curve_ext[5] = CLIP(noise_curve_ext[5], 0, 0x7ff);
    // REG: NOISE_CURVE3
    pCfg->noise_curve_ext[6] = CLIP(noise_curve_ext[6], 0, 0x7ff);
    pCfg->noise_curve_ext[7] = CLIP(noise_curve_ext[7], 0, 0x7ff);
    // REG: NOISE_CURVE4
    pCfg->noise_curve_ext[8] = CLIP(noise_curve_ext[8], 0, 0x7ff);
    pCfg->noise_curve_ext[9] = CLIP(noise_curve_ext[9], 0, 0x7ff);
    // REG: NOISE_CURVE5
    pCfg->noise_curve_ext[10] = CLIP(noise_curve_ext[10], 0, 0x7ff);
    pCfg->noise_curve_ext[11] = CLIP(noise_curve_ext[11], 0, 0x7ff);
    // REG: NOISE_CURVE6
    pCfg->noise_curve_ext[12] = CLIP(noise_curve_ext[12], 0, 0x7ff);
    pCfg->noise_curve_ext[13] = CLIP(noise_curve_ext[13], 0, 0x7ff);
    // REG: NOISE_CURVE7
    pCfg->noise_curve_ext[14] = CLIP(noise_curve_ext[14], 0, 0x7ff);
    pCfg->noise_curve_ext[15] = CLIP(noise_curve_ext[15], 0, 0x7ff);
    // REG: NOISE_CURVE8
    pCfg->noise_curve_ext[16] = CLIP(noise_curve_ext[16], 0, 0x7ff);

    return;
}
