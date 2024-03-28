#include "rk_aiq_isp39_modules.h"

#define RK_SHARP_V34_LUMA_POINT_NUM (8)

#define RK_SHARP_V34_PBF_DIAM                 3
#define RK_SHARP_V34_RF_DIAM                  5
#define RK_SHARP_V34_BF_DIAM                  3
#define RK_SHARP_V34_AVG_DIAM                 3
#define RK_SHARP_V34_SHARP_ADJ_GAIN_TABLE_LEN 14
#define RK_SHARP_V34_STRENGTH_TABLE_LEN       22

#define RK_SHARP_V34_SHARP_STRG_FIX_BITS 2
#define RK_SHARP_V34_GAUS_RATIO_FIX_BITS 7
#define RK_SHARP_V34_BF_RATIO_FIX_BITS   7
#define RK_SHARP_V34_PBFCOEFF_FIX_BITS   7
#define RK_SHARP_V34_RFCOEFF_FIX_BITS    7
#define RK_SHARP_V34_HBFCOEFF_FIX_BITS   7

#define RK_SHARP_V34_GLOBAL_GAIN_FIX_BITS       4
#define RK_SHARP_V34_GLOBAL_GAIN_ALPHA_FIX_BITS 3
#define RK_SHARP_V34_LOCAL_GAIN_FIX_BITS        4
#define RK_SHARP_V34_LOCAL_GAIN_SACLE_FIX_BITS  7
#define RK_SHARP_V34_ADJ_GAIN_FIX_BITS          10
#define RK_SHARP_V34_STRENGTH_TABLE_FIX_BITS    7
#define RK_SHARP_V34_TEX_FIX_BITS               10

void rk_aiq_sharp34_params_cvt(void * attr, struct isp39_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo)
{
    int i, tmp;
    float fPercent = 1.0f;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;
    struct isp39_sharp_cfg *pFix = &isp_cfg->others.sharp_cfg;
    sharp_param_t *sharp_param = (sharp_param_t *) attr;
    sharp_params_static_t* psta = &sharp_param->sta;
    sharp_params_dyn_t* pdyn = &sharp_param->dyn;
    int sum_coeff, offset;
    int pbf_sigma_shift     = 0;
    int bf_sigma_shift      = 0;

    // SHARP_EN (0x0000)
    pFix->bypass            = 0;
    // TODO
    //pFix->center_mode       = pSelect->hw_sharp_centerPosition_mode;

    if (pdyn->shpScl_locSgmStrg.hw_sharpT_locSgmStrg_mode == sharp_locGlbSgmStrgMix_mode )
        pFix->local_gain_bypass = 0;
    else
        pFix->local_gain_bypass = 1;

    if (cols > 3072 && rows > 1728) {
        pFix->radius_step_mode = 1;
    } else {
        pFix->radius_step_mode = 0;
    }

    if (pdyn->shpScl_texDetect.hw_sharpT_estNsClip_mode == sharp_preNsSgmStats_mode)
        pFix->noise_clip_mode = 0;
    else
        pFix->noise_clip_mode = 1;

    if (pdyn->shpScl_texDetect.hw_sharpT_estNsFilt_mode == sharp_allFilt_mode )
        pFix->noise_filt_sel  = 0;
    else
        pFix->noise_filt_sel  = 1;

    if (pdyn->sharpOpt.hw_sharpT_shpSrc_mode == sharp_hfExactPreBfOut_mode) {
        pFix->baselmg_sel     = 0;
    }
    if (pdyn->sharpOpt.hw_sharpT_shpSrc_mode == sharp_sharpIn_mode) {
        pFix->baselmg_sel     = 1;
    }
    if (pdyn->sharpOpt.hw_sharpT_shpSrc_mode == sharp_hfExtraLpfOut_mode) {
        pFix->clipldx_sel     = 1;
    }

    pFix->tex2wgt_en      = pdyn->sharpOpt.hw_sharpT_texShpSclRemap_en;
 
    // SHARP_RATIO  (0x0004)
    tmp                       = (int)ROUND_F(pdyn->hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha / fPercent *
                       (1 << RK_SHARP_V34_BF_RATIO_FIX_BITS));
    pFix->pre_bifilt_alpha    = CLIP(tmp, 0, 0x80);
    tmp                       = (int)ROUND_F(pdyn->hfExtra_lpf.hw_sharpT_lpfOut_alpha / fPercent *
                       (1 << RK_SHARP_V34_GAUS_RATIO_FIX_BITS));
    pFix->guide_filt_alpha    = CLIP(tmp, 0, 0x80);
    tmp                       = (int)ROUND_F(pdyn->hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha / fPercent *
                       (1 << RK_SHARP_V34_BF_RATIO_FIX_BITS));
    pFix->detail_bifilt_alpha = CLIP(tmp, 0, 0x80);
    tmp = (int)ROUND_F((pdyn->sharpOpt.sw_sharpT_mfGlbShpScl_val + pdyn->sharpOpt.sw_sharpT_lfGlbShpScl_val) *
                       fPercent * (1 << RK_SHARP_V34_SHARP_STRG_FIX_BITS));
    pFix->global_sharp_strg = CLIP(tmp, 0, 127);

    // SHARP_LUMA_DX (0x0008)
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM - 1; i++) {
        tmp                     = (int16_t)LOG2(pdyn->hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.idx[i + 1] -
                            pdyn->hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.idx[i]);
        pFix->luma2table_idx[i] = CLIP(tmp, 0, 15);
    }

    // SHARP_PBF_SIGMA_INV_0 (0x000c - 0x0014)
    // pre bf sigma inv
    int sigma_deci_bits = 9;
    int sigma_inte_bits = 1;
    int max_val         = 0;
    int min_val         = 65536;
    int shf_bits        = 0;
    short sigma_bits[3];
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        int cur_sigma =
            FLOOR((pdyn->hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i] * pdyn->hfExtra_preBifilt.sw_sharpT_rgeSgm_scale +
                   pdyn->hfExtra_preBifilt.sw_sharpT_rgeSgm_offset) /
                  fPercent);
        if (max_val < cur_sigma) max_val = cur_sigma;
        if (min_val > cur_sigma) min_val = cur_sigma;
    }
    sigma_bits[0]   = FLOOR(log((float)min_val) / log((float)2));
    sigma_bits[1]   = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2]   = sigma_deci_bits + sigma_bits[0];
    pbf_sigma_shift = sigma_bits[2] - 5;
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        tmp = (int16_t)ROUND_F(
            1.0f /
            (pdyn->hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i] * pdyn->hfExtra_preBifilt.sw_sharpT_rgeSgm_scale +
             pdyn->hfExtra_preBifilt.sw_sharpT_rgeSgm_offset) *
            fPercent * (1 << sigma_bits[2]));
        pFix->pbf_sigma_inv[i] = CLIP(tmp, 0, 4095);
    }

    // SHARP_BF_SIGMA_INV_0 (0x0018 -  0x0020)
    // bf sigma inv
    sigma_deci_bits = 9;
    sigma_inte_bits = 1;
    max_val         = 0;
    min_val         = 65536;
    shf_bits        = 0;
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        int cur_sigma =
            FLOOR((pdyn->hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i] * pdyn->hfExtra_hfBifilt.sw_sharpT_rgeSgm_scale +
                   pdyn->hfExtra_hfBifilt.sw_sharpT_rgeSgm_offset) /
                  fPercent);
        if (max_val < cur_sigma) max_val = cur_sigma;
        if (min_val > cur_sigma) min_val = cur_sigma;
    }
    sigma_bits[0]  = FLOOR(log((float)min_val) / log((float)2.0f));
    sigma_bits[1]  = MAX(sigma_inte_bits - sigma_bits[0], 0);
    sigma_bits[2]  = sigma_deci_bits + sigma_bits[0];
    bf_sigma_shift = sigma_bits[2] - 5;
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        tmp = (int16_t)ROUND_F(
            1.0f /
            (pdyn->hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i] * pdyn->hfExtra_hfBifilt.sw_sharpT_rgeSgm_scale +
             pdyn->hfExtra_hfBifilt.sw_sharpT_rgeSgm_offset) *
            fPercent * (1 << sigma_bits[2]));
        pFix->bf_sigma_inv[i] = CLIP(tmp, 0, 4095);
    }

    // SHARP_SIGMA_SHIFT (0x00024)
    pFix->pbf_sigma_shift = CLIP(pbf_sigma_shift, 0, 15);
    pFix->bf_sigma_shift  = CLIP(bf_sigma_shift, 0, 15);

    // EHF_TH (0x0028 -  0x0030)
    for (int i = 0; i < 8; i++)
        pFix->luma2strg_val[i] = CLIP(pdyn->shpScl_hf.hw_sharpT_luma2hfScl_val[i], 0, 1023);

    // SHARP_SHARP_CLIP_HF (0x0034 -  0x003c)
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        tmp                       = (int)(pdyn->sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i] * fPercent);
        pFix->luma2posclip_val[i] = CLIP(tmp, 0, 1023);
    }

    // SHARP_SHARP_PBF_COEF (0x00040)
    // filter coeff
    // bf coeff
    // rk_sharp_V34_pbfCoeff : [4], [1], [0]
    float pre_bila_filter[3];
    if (pdyn->hfExtra_preBifilt.sw_sharpT_filtCfg_mode == sharp_cfgByFiltStrg_mode) {
        float dis_table_3x3[3] = {0.0, 1.0, 2.0};
        double e               = 2.71828182845905;
        float sigma            = pdyn->hfExtra_preBifilt.sw_sharpT_filtSpatial_strg;
        float sum_gauss_coeff  = 0.0;
        for (int i = 0; i < 3; i++) {
            float tmp          = pow(e, -dis_table_3x3[i] / 2.0 / sigma / sigma);
            pre_bila_filter[i] = tmp;
        }
        sum_gauss_coeff = pre_bila_filter[0] + 4 * pre_bila_filter[1] + 4 * pre_bila_filter[2];
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pre_bila_filter[i] / sum_gauss_coeff;
        }
    } else {
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pdyn->hfExtra_preBifilt.hw_sharpT_filtSpatial_wgt[i];
        }
    }
    tmp             = (int)ROUND_F(pre_bila_filter[0] * (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS));
    pFix->pbf_coef0 = CLIP(tmp, 0, 127);
    tmp             = (int)ROUND_F(pre_bila_filter[1] * (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS));
    pFix->pbf_coef1 = CLIP(tmp, 0, 127);
    tmp             = (int)ROUND_F(pre_bila_filter[2] * (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS));
    pFix->pbf_coef2 = CLIP(tmp, 0, 127);

    sum_coeff       = pFix->pbf_coef0 + 4 * pFix->pbf_coef1 + 4 * pFix->pbf_coef2;
    offset          = (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS) - sum_coeff;
    tmp             = (int)(pFix->pbf_coef0 + offset);
    pFix->pbf_coef0 = CLIP(tmp, 0, 127);

    // SHARP_SHARP_BF_COEF (0x00044)
    // rk_sharp_V34_rfCoeff :  [4], [1], [0]
    float bila_filter[3];
    if (pdyn->hfExtra_hfBifilt.sw_sharpT_filtCfg_mode == sharp_cfgByFiltStrg_mode) {
        float dis_table_3x3[3] = {0.0, 1.0, 2.0};
        double e               = 2.71828182845905;
        float sigma            = pdyn->hfExtra_hfBifilt.sw_sharpT_filtSpatial_strg;
        float sum_gauss_coeff  = 0.0;
        for (int i = 0; i < 3; i++) {
            float tmp      = pow(e, -dis_table_3x3[i] / 2.0 / sigma / sigma);
            bila_filter[i] = tmp;
        }
        sum_gauss_coeff = bila_filter[0] + 4 * bila_filter[1] + 4 * bila_filter[2];
        for (int i = 0; i < 3; i++) {
            bila_filter[i] = bila_filter[i] / sum_gauss_coeff;
        }
    } else {
        for (int i = 0; i < 3; i++) {
            pre_bila_filter[i] = pdyn->hfExtra_hfBifilt.hw_sharpT_filtSpatial_wgt[i];
        }
    }
    tmp            = (int)ROUND_F(bila_filter[0] * (1 << RK_SHARP_V34_RFCOEFF_FIX_BITS));
    pFix->bf_coef0 = CLIP(tmp, 0, 127);
    tmp            = (int)ROUND_F(bila_filter[1] * (1 << RK_SHARP_V34_RFCOEFF_FIX_BITS));
    pFix->bf_coef1 = CLIP(tmp, 0, 127);
    tmp            = (int)ROUND_F(bila_filter[2] * (1 << RK_SHARP_V34_RFCOEFF_FIX_BITS));
    pFix->bf_coef2 = CLIP(tmp, 0, 127);

    sum_coeff      = pFix->bf_coef0 + 4 * pFix->bf_coef1 + 4 * pFix->bf_coef2;
    offset         = (1 << RK_SHARP_V34_PBFCOEFF_FIX_BITS) - sum_coeff;
    tmp            = (int)(pFix->bf_coef0 + offset);
    pFix->bf_coef0 = CLIP(tmp, 0, 127);

    float kernel0_ratio = pdyn->sharpOpt.sw_sharpT_mfGlbShpScl_val /
                          (pdyn->sharpOpt.sw_sharpT_mfGlbShpScl_val + pdyn->sharpOpt.sw_sharpT_lfGlbShpScl_val);
    float kernel1_ratio = pdyn->sharpOpt.sw_sharpT_lfGlbShpScl_val /
                          (pdyn->sharpOpt.sw_sharpT_mfGlbShpScl_val + pdyn->sharpOpt.sw_sharpT_lfGlbShpScl_val);
    if (pdyn->hfExtra_lpf.sw_sharpT_filtCfg_mode == sharp_cfgBy2SwLpfStrg_mode) {
        float dis_table_5x5[6] = {0.0f, 1.0f, 2.0f, 4.0f, 5.0f, 8.0f};
        float dis_table_3x3[6] = {0.0f, 1.0f, 2.0f, 1000.0f, 1000.0f, 1000.0f};
        float gaus_table[6];
        float gaus_table1[6];
        float gaus_table_combine[6];

        float sigma  = pdyn->hfExtra_lpf.sw_sharpT_mf_strg;
        float sigma1 = pdyn->hfExtra_lpf.sw_sharpT_lf_strg;
        double e     = 2.71828182845905;
        for (int k = 0; k < 6; k++) {
            float tmp0    = pow(e, -dis_table_3x3[k] / 2.0 / sigma / sigma);
            gaus_table[k] = tmp0;
        }
        for (int k = 0; k < 6; k++) {
            float tmp1     = pow(e, -dis_table_5x5[k] / 2.0 / sigma1 / sigma1);
            gaus_table1[k] = tmp1;
        }
        float sumTable = 0;
        sumTable       = gaus_table[0] + 4 * gaus_table[1] + 4 * gaus_table[2] + 4 * gaus_table[3] +
                   8 * gaus_table[4] + 4 * gaus_table[5];

        float sumTable1 = 0;
        sumTable1 = gaus_table1[0] + 4 * gaus_table1[1] + 4 * gaus_table1[2] + 4 * gaus_table1[3] +
                    8 * gaus_table1[4] + 4 * gaus_table1[5];
        for (int k = 0; k < 6; k++) {
            // gaus_table[k] = gaus_table[k] / sumTable;
            gaus_table_combine[k] = kernel0_ratio * gaus_table[k] / sumTable +
                                    kernel1_ratio * gaus_table1[k] / sumTable1;
            pFix->img_lpf_coeff[k] =
                ROUND_F(gaus_table_combine[k] * (1 << RK_SHARP_V34_HBFCOEFF_FIX_BITS));
        }
    } else {
        for (int k = 0; k < 6; k++) {
            float range_coeff        = pdyn->hfExtra_lpf.hw_sharpT_lpf_wgt[k];
            pFix->img_lpf_coeff[k] =
                ROUND_F(range_coeff * (1 << RK_SHARP_V34_HBFCOEFF_FIX_BITS));
        }
    }

    // gain
    tmp               = pdyn->shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_val * (1 << RK_SHARP_V34_GLOBAL_GAIN_FIX_BITS);
    pFix->global_gain = CLIP(tmp, 0, 1023);
    tmp = pdyn->shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_alpha * (1 << RK_SHARP_V34_GLOBAL_GAIN_ALPHA_FIX_BITS);
    pFix->gain_merge_alpha = CLIP(tmp, 0, 8);
    tmp = pdyn->shpScl_locSgmStrg.hw_sharpT_localSgmStrg_scale * (1 << RK_SHARP_V34_LOCAL_GAIN_SACLE_FIX_BITS);
    pFix->local_gain_scale = CLIP(tmp, 0, 128);

    // gain adjust strength
    for (int i = 0; i < RK_SHARP_V34_SHARP_ADJ_GAIN_TABLE_LEN; i++) {
        tmp = ROUND_F(pdyn->sharpOpt.hw_sharpT_locSgmStrg2ShpScl_val[i] * (1 << RK_SHARP_V34_ADJ_GAIN_FIX_BITS));
        pFix->gain2strg_val[i] = CLIP(tmp, 0, 16384);
    }

    // CENTER
    tmp            = cols / 2;
    pFix->center_x = CLIP(tmp, 0, 8191);
    tmp            = rows / 2;
    pFix->center_y = CLIP(tmp, 0, 8191);

    // gain dis strength
    for (int i = 0; i < RK_SHARP_V34_STRENGTH_TABLE_LEN; i++) {
        tmp                        = ROUND_F(pdyn->sharpOpt.hw_sharpT_radiDist2ShpScl_val[i] *
                      (1 << RK_SHARP_V34_STRENGTH_TABLE_FIX_BITS));
        pFix->distance2strg_val[i] = CLIP(tmp, 0, 128);
    }

    // SHARP_SHARP_CLIP_NEG (0x008c -  0x0094)
    for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
        tmp                        = (int)(pdyn->sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i] * fPercent);
        pFix->luma2neg_clip_val[i] = CLIP(tmp, 0, 1023);
    }

    // TEXTURE0
    tmp                     = ROUND_F(pdyn->shpScl_texDetect.hw_sharpT_estNsManual_maxLimit);
    pFix->noise_max_limit   = CLIP(tmp, 0, 1023);

    // TEXTURE1
    pFix->noise_norm_bit = pdyn->shpScl_texDetect.hw_sharpT_estNsNorize_shift;
    pFix->tex_wgt_mode   = pdyn->sharpOpt.hw_sharpT_shpOpt_mode;

    float tex_coef        = pdyn->sharpOpt.hw_sharpT_tex2ShpScl_scale;
    int tex_reserve_level = 0;
    if (tex_coef >= 16 && tex_coef <= 31)
        tex_reserve_level = 0;
    else if (tex_coef >= 8 && tex_coef < 16)
        tex_reserve_level = 1;
    else if (tex_coef >= 4 && tex_coef < 8)
        tex_reserve_level = 2;
    else if (tex_coef >= 2 && tex_coef < 4)
        tex_reserve_level = 3;
    else if (tex_coef >= 1 && tex_coef < 2)
        tex_reserve_level = 4;
    else if (tex_coef >= 0.5 && tex_coef < 1)
        tex_reserve_level = 5;
    else if (tex_coef >= 0.25 && tex_coef < 0.5)
        tex_reserve_level = 6;
    else if (tex_coef >= 0.125 && tex_coef < 0.25)
        tex_reserve_level = 7;
    else if (tex_coef >= 0.0625 && tex_coef < 0.125)
        tex_reserve_level = 8;
    else if (tex_coef >= 0.03125 && tex_coef < 0.0625)
        tex_reserve_level = 9;
    else if (tex_coef < 0.03125)
        tex_reserve_level = 10;
    pFix->tex_reserve_level = tex_reserve_level;
    tmp = ROUND_F(tex_coef * (1 << (tex_reserve_level + RK_SHARP_V34_TEX_FIX_BITS)));
    pFix->tex_wet_scale = CLIP(tmp, 0, 31 * (1 << RK_SHARP_V34_TEX_FIX_BITS));

    // TEXTURE_LUT
    for (int i = 0; i < 17; i++) pFix->tex2wgt_val[i] = pdyn->sharpOpt.hw_sharpT_texShpSclRemap_val[i];

    // TEXTURE2
    tmp = ROUND_F(pdyn->shpScl_texDetect.hw_sharpT_estNs_scale * (1 << RK_SHARP_V34_ADJ_GAIN_FIX_BITS));
    pFix->noise_strg = CLIP(tmp, 0, 16383);

    for (int i = 0; i < 17; i++) pFix->detail2strg_val[i] = pdyn->shpScl_hf.hw_sharpT_hfScl2ShpScl_val[i];

    return;
}
