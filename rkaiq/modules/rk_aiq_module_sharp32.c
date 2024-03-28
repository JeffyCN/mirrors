#include "rk_aiq_isp32_modules.h"

bool rk_aiq_sharp32_check_attrib(sharp_api_attrib_t *attr)
{
    return true;
}

bool rk_aiq_sharp32_check_param(sharp_param_t *param)
{
    return true;
}

#define RK_SHARP_V33_SHARP_RATIO_FIX_BITS 2
#define RK_SHARP_V33_GAUS_RATIO_FIX_BITS  7
#define RK_SHARP_V33_BF_RATIO_FIX_BITS    7
#define RK_SHARP_V33_PBFCOEFF_FIX_BITS    7
#define RK_SHARP_V33_RFCOEFF_FIX_BITS     7
#define RK_SHARP_V33_HBFCOEFF_FIX_BITS    7
#define RK_SHARP_V33_GLOBAL_GAIN_FIX_BITS       4
#define RK_SHARP_V33_GLOBAL_GAIN_ALPHA_FIX_BITS 3
#define RK_SHARP_V33_LOCAL_GAIN_FIX_BITS        4
#define RK_SHARP_V33_LOCAL_GAIN_SACLE_FIX_BITS  7
#define RK_SHARP_V33_ADJ_GAIN_FIX_BITS          10
#define RK_SHARP_V33_STRENGTH_TABLE_FIX_BITS    7
#define RK_SHARP_V33_LUMA_POINT_NUM (8)

void rk_aiq_sharp32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo)
{
    int i;
    int tmp;
    int pbf_sigma_shift     = 0;
    int bf_sigma_shift      = 0;
    struct isp32_sharp_cfg *phwcfg = &isp_cfg->others.sharp_cfg ;
    sharp_param_t *sharp_param = (sharp_param_t *) attr;
    sharp_params_dyn_t* pdyn = &sharp_param->dyn;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;

    if (cols > 3072 && rows > 1728) {
        phwcfg->radius_ds_mode = 1;
    } else {
        phwcfg->radius_ds_mode = 0;
    }

    phwcfg->noiseclip_mode  = pdyn->textureDct.hw_sharpT_noiseSgmLimit_mode == sharp_setManual_mode;
    phwcfg->exgain_bypass   = pdyn->inPixSgm.hw_sharpT_inPixSgm_mode == sharp_inPixSgmGlb_mode;
    //phwcfg->bypass          = bypass;
    tmp                     = (int)ROUND_F(pdyn->sharpOpt.hw_sharpT_hfGlbShpWgt_val *
                                           (1 << RK_SHARP_V33_SHARP_RATIO_FIX_BITS));
    phwcfg->sharp_ratio     = CLIP(tmp, 0, 127);
    tmp                     = (int)ROUND_F(pdyn->bifilt.hw_sharpT_biFiltOut_alpha *
                                           (1 << RK_SHARP_V33_BF_RATIO_FIX_BITS));
    phwcfg->bf_ratio        = CLIP(tmp, 0, 0x80);
    tmp                     = (int)ROUND_F(pdyn->gausFilt.hw_sharpT_gausFiltOut_alpha *
                                           (1 << RK_SHARP_V33_GAUS_RATIO_FIX_BITS));
    phwcfg->gaus_ratio      = CLIP(tmp, 0, 0x80);
    tmp                     = (int)ROUND_F(pdyn->preBifilt.hw_sharpT_biFiltOut_alpha *
                                           (1 << RK_SHARP_V33_BF_RATIO_FIX_BITS));
    phwcfg->pbf_ratio       = CLIP(tmp, 0, 0x80);

    for (int i = 0; i < ISP3X_SHARP_X_NUM; i++) {
        tmp                 = (int16_t)LOG2(pdyn->preBifilt.hw_sharpC_luma2RSigma_curve.idx[i + 1] -
                                               pdyn->preBifilt.hw_sharpC_luma2RSigma_curve.idx[i]);
        phwcfg->luma_dx[i]  = CLIP(tmp, 0, 15);
    }

    int sigma_deci_bits = 9;
    int sigma_inte_bits = 1;
    int max_val         = 0;
    int min_val         = 65536;
    int shf_bits        = 0;
    short sigma_bits[3];
    for (int i = 0; i < ISP3X_SHARP_Y_NUM; i++) {
        int cur_sigma = FLOOR((pdyn->preBifilt.hw_sharpC_luma2RSigma_curve.val[i] *
                               pdyn->preBifilt.sw_sharpT_rsigma_scale +
                               pdyn->preBifilt.hw_sharpT_rsigma_offset));
        if (max_val < cur_sigma) max_val = cur_sigma;
        if (min_val > cur_sigma) min_val = cur_sigma;
    }
    sigma_bits[0]   = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1]   = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2]   = sigma_deci_bits + sigma_bits[0];
    pbf_sigma_shift = sigma_bits[2] - 5;
    for (i = 0; i < ISP3X_SHARP_Y_NUM; i++) {
        tmp = (int16_t)ROUND_F((float)1 / (pdyn->preBifilt.hw_sharpC_luma2RSigma_curve.val[i] *
                                           pdyn->preBifilt.sw_sharpT_rsigma_scale +
                                           pdyn->preBifilt.hw_sharpT_rsigma_offset) * (1 << sigma_bits[2]));
        phwcfg->pbf_sigma_inv[i] = CLIP(tmp, 0, 4095);
    }

    sigma_deci_bits = 9;
    sigma_inte_bits = 1;
    max_val         = 0;
    min_val         = 65536;
    shf_bits        = 0;
    for (int i = 0; i < RK_SHARP_V33_LUMA_POINT_NUM; i++) {
        int cur_sigma = FLOOR((pdyn->bifilt.hw_sharpC_luma2RSigma_curve.val[i] *
                               pdyn->bifilt.sw_sharpT_rsigma_scale +
                               pdyn->bifilt.hw_sharpT_rsigma_offset));
        if (max_val < cur_sigma) max_val = cur_sigma;
        if (min_val > cur_sigma) min_val = cur_sigma;
    }
    sigma_bits[0]  = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1]  = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2]  = sigma_deci_bits + sigma_bits[0];
    bf_sigma_shift = sigma_bits[2] - 5;
    for (i = 0; i < ISP3X_SHARP_Y_NUM; i++) {
        tmp = (int16_t)ROUND_F((float)1 / (pdyn->bifilt.hw_sharpC_luma2RSigma_curve.val[i] *
                                           pdyn->bifilt.sw_sharpT_rsigma_scale +
                                           pdyn->bifilt.hw_sharpT_rsigma_offset) * (1 << sigma_bits[2]));
        phwcfg->bf_sigma_inv[i] = CLIP(tmp, 0, 4095);
    }
    phwcfg->pbf_sigma_shift = CLIP(pbf_sigma_shift, 0, 15);;
    phwcfg->bf_sigma_shift  = CLIP(bf_sigma_shift, 0, 15);;

    for (int i = 0; i < ISP3X_SHARP_Y_NUM; i++) {
        tmp = (int)(pdyn->sharpOpt.hw_sharpT_luma2hfShpLimit_val[i]);
        phwcfg->clip_hf[i] = CLIP(tmp, 0, 1023);
    }

    float pre_bila_filter[3];
    if (pdyn->preBifilt.sw_sharpT_filtCfg_mode == sharp_cfgByFiltStrg_mode) {
        float dis_table_3x3[3] = {0.0, 1.0, 2.0};
        double e               = 2.71828182845905;
        float sigma            = pdyn->preBifilt.sw_sharpT_filtSpatial_strg;
        float sum_gauss_coeff  = 0.0;
        for (int i = 0; i < 3; i++) {
            float tmpf         = pow(e, -dis_table_3x3[i] / 2.0 / sigma / sigma);
            pre_bila_filter[i] = tmpf;
        }
        sum_gauss_coeff = pre_bila_filter[0] + 4 * pre_bila_filter[1] + 4 * pre_bila_filter[2];
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pre_bila_filter[i] / sum_gauss_coeff;
        }
    } else {
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pdyn->preBifilt.hw_sharpT_filtSpatial_wgt[i];
        }
    }
    tmp = (int)ROUND_F(pre_bila_filter[0] * (1 << RK_SHARP_V33_PBFCOEFF_FIX_BITS));
    phwcfg->pbf_coef0 = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pre_bila_filter[1] * (1 << RK_SHARP_V33_PBFCOEFF_FIX_BITS));
    phwcfg->pbf_coef1 = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(pre_bila_filter[2] * (1 << RK_SHARP_V33_PBFCOEFF_FIX_BITS));
    phwcfg->pbf_coef2 = CLIP(tmp, 0, 127);

    float bila_filter[3];
    if (pdyn->bifilt.sw_sharpT_filtCfg_mode == sharp_cfgByFiltStrg_mode) {
        float dis_table_3x3[3] = {0.0, 1.0, 2.0};
        double e               = 2.71828182845905;
        float sigma            = pdyn->bifilt.sw_sharpT_filtSpatial_strg;
        float sum_gauss_coeff  = 0.0;
        for (int i = 0; i < 3; i++) {
            float tmpf         = pow(e, -dis_table_3x3[i] / 2.0 / sigma / sigma);
            bila_filter[i] = tmpf;
        }
        sum_gauss_coeff = bila_filter[0] + 4 * bila_filter[1] + 4 * bila_filter[2];
        for (int i = 0; i < 3; i++) {
            bila_filter[i] = bila_filter[i] / sum_gauss_coeff;
        }
    } else {
        for (int i = 0; i < 3; i++) {
            bila_filter[i] = pdyn->bifilt.hw_sharpT_filtSpatial_wgt[i];
        }
    }
    tmp = (int)ROUND_F(bila_filter[0] * (1 << RK_SHARP_V33_RFCOEFF_FIX_BITS));
    phwcfg->bf_coef0 = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(bila_filter[1] * (1 << RK_SHARP_V33_RFCOEFF_FIX_BITS));
    phwcfg->bf_coef1 = CLIP(tmp, 0, 127);
    tmp = (int)ROUND_F(bila_filter[2] * (1 << RK_SHARP_V33_RFCOEFF_FIX_BITS));
    phwcfg->bf_coef2 = CLIP(tmp, 0, 127);

    float range_filter[6];
    if (pdyn->gausFilt.sw_sharpT_filtCfg_mode == sharp_cfgByFiltStrg_mode) {
        float gauss_dis_table_5x5[6] = {0.0, 1.0, 2.0, 4.0, 5.0, 8.0};
        float gauss_dis_table_3x3[6] = {0.0, 1.0, 2.0, 1000, 1000, 1000};
        double e                     = 2.71828182845905;
        float sigma                  = pdyn->gausFilt.sw_sharpT_gausFilt_Strg;
        float sum_gauss_coeff        = 0.0;
        if (pdyn->gausFilt.sw_sharpT_filtSpatialRadi_mode == sharp_filt5_mode) {
            for (int i = 0; i < 6; i++) {
                float tmp       = pow(e, -gauss_dis_table_5x5[i] / 2.0 / sigma / sigma);
                range_filter[i] = tmp;
            }
        } else {
            for (int i = 0; i < 6; i++) {
                float tmp       = pow(e, -gauss_dis_table_3x3[i] / 2.0 / sigma / sigma);
                range_filter[i] = tmp;
            }
        }

        sum_gauss_coeff = range_filter[0] + 4 * range_filter[1] + 4 * range_filter[2] +
                          4 * range_filter[3] + 8 * range_filter[4] + 4 * range_filter[5];
        for (int i = 0; i < 6; i++) {
            range_filter[i] = range_filter[i] / sum_gauss_coeff;
        }
    } else {
        for (int i = 0; i < 6; i++) {
            range_filter[i] = pdyn->gausFilt.hw_sharpT_gausFilt_coeff[i];
        }
    }
    for (int i = 0; i < ISP3X_SHARP_GAUS_COEF_NUM; i++) {
        tmp = (int)ROUND_F(range_filter[i] * (1 << RK_SHARP_V33_HBFCOEFF_FIX_BITS));
        phwcfg->gaus_coef[i] = CLIP(tmp, 0, 127);
    }

    tmp                       = pdyn->inPixSgm.hw_sharpT_glbPixSgm_val *
                                    (1 << RK_SHARP_V33_GLOBAL_GAIN_FIX_BITS);
    phwcfg->global_gain       = CLIP(tmp, 0, 1023);
    tmp                       = pdyn->inPixSgm.hw_sharpT_glbPixSgm_alpha *
                                    (1 << RK_SHARP_V33_GLOBAL_GAIN_ALPHA_FIX_BITS);
    phwcfg->global_gain_alpha = CLIP(tmp, 0, 8);
    tmp                       = pdyn->inPixSgm.hw_sharpT_localPixSgm_scale *
                                    (1 << RK_SHARP_V33_LOCAL_GAIN_SACLE_FIX_BITS);
    phwcfg->local_gainscale   = CLIP(tmp, 0, 128);
    for (int i = 0; i < 14; i++) {
        tmp                   = ROUND_F(pdyn->sharpOpt.hw_sharpT_pixSgm2ShpWgt_val[i] *
                                          (1 << RK_SHARP_V33_ADJ_GAIN_FIX_BITS));
        phwcfg->gain_adj[i] = CLIP(tmp, 0, 16384);
    }

    tmp = cols / 2;
    phwcfg->center_wid = CLIP(tmp, 0, 8191);
    tmp = rows / 2;
    phwcfg->center_het = CLIP(tmp, 0, 8191);

    for (int i = 0; i < 22; i++) {
        tmp                     = ROUND_F(pdyn->sharpOpt.hw_sharpT_radiDist2ShpWgt_val[i] *
                                          (1 << RK_SHARP_V33_STRENGTH_TABLE_FIX_BITS));
        phwcfg->strength[i] = CLIP(tmp, 0, 128);
    }

    tmp                        = ROUND_F(pdyn->textureDct.hw_sharpT_noiseSgm_scale *
                                         (1 << RK_SHARP_V33_ADJ_GAIN_FIX_BITS));
    phwcfg->noise_strength = CLIP(tmp, 0, 16383);
    tmp                        = (pdyn->sharpOpt.hw_sharpT_text2ShpWgt_norizeMax);
    phwcfg->enhance_bit    = CLIP(tmp, 0, 9);
    tmp                        = (pdyn->textureDct.hw_sharpT_noiseSgm_maxLimit);
    phwcfg->noise_sigma    = CLIP(tmp, 0, 1023);

    for (int i = 0; i < 8; i++) {
        phwcfg->ehf_th[i]   = 0;
        phwcfg->clip_neg[i] = 0;
    }
    return;
}
