#include "rk_aiq_isp39_modules.h"

//////////////////////////////////////////////////////////////////////////
#define     RKCNR_V31_MAX_DOWN_SCALE            8
// iir params
#define     RKCNR_V31_FIX_BIT_BF_RATIO          10
#define     RKCNR_V31_FIX_BIT_IIR_WGT           6//10
#define     RKCNR_V31_FIX_BIT_INV_SIGMA         15
#define     RKCNR_V31_FIX_BIT_SLOPE             7
#define     RKCNR_V31_FIX_BIT_WGT_RATIO         3
#define     RKCNR_V31_FIX_BIT_INT_TO_FLOAT      7
// gain params
#define     RKCNR_V31_SIGMA_FIX_BIT             8
#define     RKCNR_V31_LOCAL_GAIN_FIX_BITS       4
#define     RKCNR_V31_G_GAIN_FIX_BITS           4
#define     RKCNR_V31_G_GAIN_ALPHA_FIX_BITS     3
#define     RKCNR_V31_GAIN_ISO_FIX_BITS         7
// 3x3 bf params
#define     RKCNR_V31_FIX_BIT_GAUS_RATIO        10
#define     RKCNR_V31_sgmRatio                  6
#define     RKCNR_V31_log2e                     6
#define     RKCNR_V31_uvgain                    4
#define     RKCNR_V31_exp2_lut_y                7
#define     RKCNR_V31_bfRatio                   RKCNR_V31_exp2_lut_y
// saturation adj param fix bit
#define     RKCNR_V31_FIX_BIT_SATURATION        10
// global alpha fix bit
#define     RKCNR_V31_FIX_BIT_GLOBAL_ALPHA      10
// div sum wgt fix bit
#define     RKCNR_V31_FIX_BIT_DIV_SUM_WGT       22//
#define     RKCNR_V31_FIX_BIT_DIST_SCALE        7
//////////////////////////////////////////////////////////////////////////
// params
#define     RKCNR_V31_THUMB_BF_RADIUS           3
#define     RKCNR_V31_IIR_RADIUSY               1
#define     RKCNR_V31_IIR_RADIUSX               3//1
#define     RKCNR_V31_BF_RADIUS                 1
#define     RKCNR_V31_IIR_COEFF_NUM             10
#define     RKCNR_V31_SGM_ADJ_TABLE_LEN         13
// extend of down scale yuv, 3x3 IIR, 3x3 bf
#define     RKCNR_V31_DS_EXTEND_Y               (RKCNR_V31_IIR_RADIUSY)
#define     RKCNR_V31_DS_EXTEND_X               (RKCNR_V31_IIR_RADIUSX)
#define     RKCNR_V31_GAUS_RADIUS               2
#define     RKCNR_V31_CLIP_RADIUS               1
#define     cnr_422_5x5_gaus                    0 // 422: 5x3 or 5x5 gaussian filter

#define Math_LOG2(x)    (log((double)x)   / log((double)2))

void rk_aiq_cnr34_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo)
{
    float fStrength = 1.0;
    int i, tmp;
    struct isp39_cnr_cfg *pFix = &isp_cfg->others.cnr_cfg;
    cnr_param_t *cnr_param = (cnr_param_t *) attr;
    cnr_params_dyn_t* pdyn = &cnr_param->dyn;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;

    // CNR_CTRL
    pFix->hiflt_wgt0_mode = pdyn->hiNr_bifilt.hw_cnrT_filtWgtZero_mode;
    if (pdyn->loNrGuide_preProc.hw_cnrT_ds_mode == cnr_ds_4x4_mode )  {
        pFix->thumb_mode = 1;
    } else {
        pFix->thumb_mode = 3;
    }
    pFix->yuv422_mode = 0;
    //pFix->exgain_bypass = pSelect->hw_cnrT_exgain_bypass;
	pFix->loflt_coeff = (pdyn->loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[0]& 0x01 << 0)
					| ((pdyn->loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[1] & 0x01) << 1)
					| ((pdyn->loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[2] & 0x01) << 2)
					| ((pdyn->loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[2] & 0x01) << 3)
					| ((pdyn->loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[1] & 0x01) << 4)
					| ((pdyn->loNrGuide_iirFilt.hw_cnrT_filtSpatial_wgt[0] & 0x01) << 5);

    // CNR_EXGAIN
    tmp = pdyn->locSgmStrg.hw_cnrT_locSgmStrg_scale * (1 << RKCNR_V31_GAIN_ISO_FIX_BITS);
    pFix->local_gain_scale = CLIP(tmp, 0, 0xff);
    tmp = pdyn->locSgmStrg.hw_cnrT_glbSgmStrg_alpha * (1 << RKCNR_V31_G_GAIN_ALPHA_FIX_BITS);
    pFix->global_gain_alpha = CLIP(tmp, 0, 0x8);
    tmp = pdyn->locSgmStrg.hw_cnrT_glbSgmStrg_val * (1 << RKCNR_V31_G_GAIN_FIX_BITS);
    pFix->global_gain = CLIP(tmp, 0, 0x3ff);

    /* CNR_THUMB1 */
    int yuvBit = 10;
    int scale  = (1 << yuvBit) - 1;
    int log2e = (int)(0.8493f * (1 << (RKCNR_V31_log2e + RKCNR_V31_SIGMA_FIX_BIT)));
    int rkcnr_sw_cnrT_loFltUV_gain = ROUND_F((1 << RKCNR_V31_uvgain) * pdyn->loNrGuide_preProc.hw_cnrT_uvEdg_strg);
    int thumbBFilterSigma = ROUND_F(pdyn->loNrGuide_bifilt.sw_cnrT_rgeSgm_val * scale * fStrength);
    thumbBFilterSigma = (int)(0.8493f * (1 << RKCNR_V31_FIX_BIT_INV_SIGMA) / thumbBFilterSigma);
    tmp = thumbBFilterSigma * ((1 << RKCNR_V31_uvgain) - rkcnr_sw_cnrT_loFltUV_gain * 2);
    tmp                 = ROUND_INT(tmp, 6);
    pFix->lobfflt_vsigma_y = CLIP(tmp, 0, 0x3fff);
    tmp                 = thumbBFilterSigma * rkcnr_sw_cnrT_loFltUV_gain;
    tmp = ROUND_INT(tmp, 6); // (diff * sigma) >> tmp = (diff * (sigma >> 6))
    pFix->lobfflt_vsigma_uv = CLIP(tmp, 0, 0x3fff);

    /* CNR_THUMB_BF_RATIO */
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_BF_RATIO) * pdyn->loNrGuide_bifilt.hw_cnrT_bifiltOut_alpha * fStrength);
    pFix->lobfflt_alpha = CLIP(tmp, 0, 0x7ff);

    /* CNR_LBF_WEITD */
    if (pdyn->loNrGuide_bifilt.sw_cnrT_filtCfg_mode == cnr_cfgByFiltStrg_mode) {
    } else {
        tmp = ROUND_F(pdyn->loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[0] * (1 << RKCNR_V31_exp2_lut_y));
        pFix->thumb_bf_coeff0 = CLIP(tmp, 0, 0xff);
        tmp = ROUND_F(pdyn->loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[1] * (1 << RKCNR_V31_exp2_lut_y));
        pFix->thumb_bf_coeff1 = CLIP(tmp, 0, 0xff);
        tmp = ROUND_F(pdyn->loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[2] * (1 << RKCNR_V31_exp2_lut_y));
        pFix->thumb_bf_coeff2 = CLIP(tmp, 0, 0xff);
        tmp = ROUND_F(pdyn->loNrGuide_bifilt.hw_cnrT_filtSpatial_wgt[3] * (1 << RKCNR_V31_exp2_lut_y));
        pFix->thumb_bf_coeff3 = CLIP(tmp, 0, 0xff);
    }

    /* CNR_IIR_PARA1 */
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_SLOPE) * pdyn->loNrGuide_iirFilt.hw_cnrT_sgm2NhoodWgt_slope);
    pFix->loflt_wgt_slope = CLIP(tmp, 0, 0x3ff);

    tmp = ROUND_F(1.2011 * (1 << RKCNR_V31_FIX_BIT_INV_SIGMA) / (pdyn->loNrGuide_iirFilt.sw_cnrT_rgeSgm_val * scale * fStrength));
    int tmptmp = tmp * pFix->loflt_wgt_slope;
    int shiftBit = Math_LOG2(tmptmp) - RKCNR_V31_FIX_BIT_INT_TO_FLOAT;
    shiftBit = MAX(shiftBit, 0);
    tmp = RKCNR_V31_FIX_BIT_INV_SIGMA - shiftBit;
    if (tmp < 0) {
        tmp = ABS(tmp) + (1 << 5);
    }
    pFix->exp_x_shift_bit = CLIP(tmp, 0, 0x3f);

    tmp = ROUND_F((float)tmptmp / (1 << shiftBit));
    pFix->loflt_vsigma = CLIP(tmp, 0, 0xff);

    tmp = ROUND_F((1 << RKCNR_V31_uvgain) * pdyn->loNrGuide_preProc.hw_cnrT_uvEdg_strg);
    pFix->loflt_uv_gain = CLIP(tmp, 0, 0xf);

    /* CNR_IIR_PARA2 */
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_IIR_WGT) * pdyn->loNrGuide_iirFilt.hw_cnrT_nhoodWgtZero_thred);
    pFix->loflt_wgt_min_thred = CLIP(tmp, 0, 0x3f);
    tmp = ROUND_F((1 << 3) * pdyn->loNrGuide_iirFilt. hw_cnrT_iirFiltStrg_maxLimit);
    pFix->loflt_wgt_max_limit = CLIP(tmp, 0, 0x7f);


    /* CNR_GAUS_COE */
    float gaus_table[6];
    uint8_t gaus_reg[6];
    if(pdyn->hiNr_preLpf.sw_cnrT_filtCfg_mode == cnr_cfgByFiltStrg_mode) {
        float dis_table_5x5[6] = { 0.0, 1.0, 2.0, 4.0, 5.0, 8.0 };
        float dis_table_5x3[6] = { 0.0, 1.0, 4.0, 1.0, 2.0, 5.0 };
        float sigma = pdyn->hiNr_preLpf.sw_cnrT_filtSpatial_strg;
        double e = 2.71828182845905;
        float sumTable = 0;
        if (0 == !pFix->yuv422_mode) {
            for (int k = 0; k < 6; k++) {
                float tmp = pow(e, -dis_table_5x5[k] / 2.0 / sigma / sigma);
                gaus_table[k] = tmp;
            }
        } else {
            for (int k = 0; k < 6; k++) {
                float tmp = pow(e, -dis_table_5x3[k] / 2.0 / sigma / sigma);
                gaus_table[k] = tmp;
            }
        }
    } else {
        for(int k = 0; k < 6; k++) {
            gaus_table[k] = pdyn->hiNr_preLpf.hw_cnrT_filtSpatial_wgt[k];
        }
    }

    //check gaus params
    float sumTable = 0;
    if (0 == !pFix->yuv422_mode) {
        sumTable = gaus_table[0]
                   + 4 * gaus_table[1]
                   + 4 * gaus_table[2]
                   + 4 * gaus_table[3]
                   + 8 * gaus_table[4]
                   + 4 * gaus_table[5];
    } else {
        sumTable = gaus_table[0]
                   + 2 * gaus_table[1]
                   + 2 * gaus_table[2]
                   + 2 * gaus_table[3]
                   + 4 * gaus_table[4]
                   + 4 * gaus_table[5];
    }

    for (int k = 0; k < 6; k++) {
        gaus_table[k] = gaus_table[k] / sumTable;
        tmp = ROUND_F(gaus_table[k] * (1 << 8));
        gaus_reg[k] = CLIP(tmp, 0, 0x7f);
    }

    int sum_coeff = 0;
    if (0 == !pFix->yuv422_mode) {
        sum_coeff = gaus_reg[0]
                    + 4 * gaus_reg[1]
                    + 4 * gaus_reg[2]
                    + 4 * gaus_reg[3]
                    + 8 * gaus_reg[4]
                    + 4 * gaus_reg[5];
    } else {
        sum_coeff = gaus_reg[0]
                    + 2 * gaus_reg[1]
                    + 2 * gaus_reg[2]
                    + 2 * gaus_reg[3]
                    + 4 * gaus_reg[4]
                    + 4 * gaus_reg[5];
    }
    int offset = (1 << 8) - sum_coeff;
    gaus_reg[0] = gaus_reg[0] + offset;

    for(int i = 0; i < 6; i++) {
        tmp = gaus_reg[5 - i];
        pFix->gaus_flt_coeff[i] = CLIP(tmp, 0, 0x7f);
    }

    /* CNR_GAUS_RATIO */
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_GAUS_RATIO) * pdyn->hiNr_preLpf.hw_cnrT_lpfOut_alpha);
    pFix->gaus_flt_alpha = CLIP(tmp, 0, 0x7ff);
    tmp = ROUND_F((1 << RKCNR_V31_exp2_lut_y) * pdyn->hiNr_bifilt.hw_cnrT_nhoodWgt_minLimit);
    pFix->hiflt_wgt_min_limit = CLIP(tmp, 0, 0xff);
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_GLOBAL_ALPHA) * pdyn->hiNr_bifilt.hw_cnrT_bifiltOut_alpha);
    pFix->hiflt_alpha = CLIP(tmp, 0, 0x7ff);

    /* CNR_BF_PARA1 */
    tmp = ROUND_F((1 << RKCNR_V31_uvgain) * pdyn->hiNr_bifilt.hw_cnrT_uvEdg_strg);
    pFix->hiflt_uv_gain = CLIP(tmp, 0, 0x7f);

    /* CNR_BF_PARA2 */
    tmp =  pdyn->hiNr_bifilt.hw_cnrT_satAdj_offset;
    pFix->adj_offset = CLIP(tmp, 0, 0x1ff);
    tmp = ROUND_F((1 << RKCNR_V31_FIX_BIT_SATURATION) * pdyn->hiNr_bifilt.hw_cnrT_satAdj_scale);
    pFix->adj_scale = CLIP(tmp, 0, 0x7fff);

    /* CNR_SIGMA */
    for(int i = 0; i < 13; i++) {
        tmp = ROUND_F(pdyn->hiNr_bifilt.hw_cnrT_locSgmStrg2SgmRat_val[i] * (1 << RKCNR_V31_sgmRatio));
        pFix->sgm_ratio[i] = CLIP(tmp, 0, 0xff);
    }

    /* CNR_IIR_GLOBAL_GAIN */
    tmp = pdyn->loNrGuide_iirFilt.hw_cnrT_glbSgmRatio_alpha * (1 << RKCNR_V31_G_GAIN_ALPHA_FIX_BITS);
    pFix->loflt_global_sgm_ratio_alpha = CLIP(tmp, 0, 0x8);
    tmp = pdyn->loNrGuide_iirFilt.hw_cnrT_glbSgm_ratio * (1 << RKCNR_V31_sgmRatio);
    pFix->loflt_global_sgm_ratio = CLIP(tmp, 0, 0xff);

    /* CNR_SIGMA */
    for(int i = 0; i < 13; i++) {
        tmp = ROUND_F(pdyn->hiNr_bifilt.hw_cnrT_locSgmStrg2CtrPix_wgt[i] * (1 << RKCNR_V31_bfRatio));
        pFix->cur_wgt[i] = CLIP(tmp, 0, 0xff);
    }

    /* CNR_SIGMA */
    for(int i = 0; i < 8; i++) {
        tmp = ROUND_F(pdyn->hw_cnrC_luma2HiNrSgm_curve.idx[i]);
        pFix->hiflt_vsigma_idx[i] = CLIP(tmp, 0, 0x3ff);
        tmp = log2e / (pdyn->hw_cnrC_luma2HiNrSgm_curve.val[i] * scale);
        pFix->hiflt_vsigma[i] = CLIP(tmp, 0, 0x3fff);
    }
    return;
}
