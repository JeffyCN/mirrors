#include "common/rk-aiisp-config.h"
#include "common/rk-isp35-config.h"

#define AIYNR_NAR_MAP_BIT_7     7

#if RKAIQ_HAVE_AIYNR

static void rk_aiq_aiynr_params_print(struct isp35_ai_cfg* ai_cfg)
{
    int i;

    LOGD_AIYNR("aipre:");
    LOGD_AIYNR("aiisp_raw12_msb = 0x%x", ai_cfg->aiisp_raw12_msb);
    LOGD_AIYNR("aiisp_gain_mode = 0x%x", ai_cfg->aiisp_gain_mode);
    LOGD_AIYNR("aiisp_curve_en = 0x%x", ai_cfg->aiisp_curve_en);
    LOGD_AIYNR("aipre_iir_en = 0x%x", ai_cfg->aipre_iir_en);
    LOGD_AIYNR("aipre_iir2ddr_en = 0x%x", ai_cfg->aipre_iir2ddr_en);
    LOGD_AIYNR("aipre_gain_en = 0x%x", ai_cfg->aipre_gain_en);
    LOGD_AIYNR("aipre_gain2ddr_en = 0x%x", ai_cfg->aipre_iir_en);
    LOGD_AIYNR("aipre_luma2gain_dis = 0x%x", ai_cfg->aipre_luma2gain_dis);
    LOGD_AIYNR("aipre_nl_ddr_mode = 0x%x", ai_cfg->aipre_nl_ddr_mode);
    LOGD_AIYNR("aipre_yraw_sel = 0x%x", ai_cfg->aipre_yraw_sel);
    LOGD_AIYNR("aipre_gain_bypass = 0x%x", ai_cfg->aipre_gain_bypass);
    LOGD_AIYNR("aipre_gain_mode = 0x%x", ai_cfg->aipre_gain_mode);
    LOGD_AIYNR("aipre_narmap_inv = 0x%x", ai_cfg->aipre_narmap_inv);

    /*for (i = 0; i < ISP35_AI_SIGMA_NUM; i++) {
        LOGD_AIYNR("aiisp_sigma_y[%d] = 0x%x", i, ai_cfg->aiisp_sigma_y[i]);
    }*/

    LOGD_AIYNR("aipre_scale = 0x%x", ai_cfg->aipre_scale);
    LOGD_AIYNR("aipre_zp = 0x%x", ai_cfg->aipre_zp);
    LOGD_AIYNR("aipre_black_lvl = 0x%x", ai_cfg->aipre_black_lvl);

    LOGD_AIYNR("aipre_gain_alpha = 0x%x", ai_cfg->aipre_gain_alpha);
    LOGD_AIYNR("aipre_global_gain = 0x%x", ai_cfg->aipre_global_gain);
    LOGD_AIYNR("aipre_gain_ratio = 0x%x", ai_cfg->aipre_gain_ratio);

    LOGD_AIYNR("aipre_noise_mot_offset = 0x%x", ai_cfg->aipre_noise_mot_offset);
    LOGD_AIYNR("aipre_noise_mot_gain = 0x%x", ai_cfg->aipre_noise_mot_gain);
    LOGD_AIYNR("aipre_noise_luma_offset = 0x%x", ai_cfg->aipre_noise_luma_offset);

    LOGD_AIYNR("aipre_noise_luma_gain = 0x%x", ai_cfg->aipre_noise_luma_gain);
    LOGD_AIYNR("aipre_noise_luma_clip = 0x%x", ai_cfg->aipre_noise_luma_clip);
    LOGD_AIYNR("aipre_noise_luma_static = 0x%x", ai_cfg->aipre_noise_luma_static);

    LOGD_AIYNR("aipre_nar_manual = 0x%x", ai_cfg->aipre_nar_manual);
    LOGD_AIYNR("aipre_nar_manual_alpha = 0x%x", ai_cfg->aipre_nar_manual_alpha);

    LOGD_AIYNR("pyr_yraw_mode = 0x%x", ai_cfg->pyr_yraw_mode);
    LOGD_AIYNR("pyr_sigma_en = 0x%x", ai_cfg->pyr_sigma_en);
    LOGD_AIYNR("pyr_yraw_sel = 0x%x", ai_cfg->pyr_yraw_sel);
    LOGD_AIYNR("pyr_gain_leftshift = 0x%x", ai_cfg->pyr_gain_leftshift);
    LOGD_AIYNR("pyr_blacklvl_sig = 0x%x", ai_cfg->pyr_blacklvl_sig);
    for (i = 0; i < ISP35_VPSL_SIGMA_NUM; i++) {
        LOGD_AIYNR("pyr_sigma_y[%d] = 0x%x", i, ai_cfg->pyr_sigma_y[i]);
    }
}

void rk_aiq_aiynr_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo, bool bypass)
{
    struct isp35_ai_cfg* ai_cfg = &isp_params->isp_cfg->others.ai_cfg;
    RkAiqAiynrModelInfo_t *pAiynrModelInfo = &cvtinfo->aiynrModelInfo;
    int i;

    rk_aiq_isp_aiynr_params_t *isp_aiynr_param = (rk_aiq_isp_aiynr_params_t *)attr;
    aiynr_param_t *aiynr_param = &isp_aiynr_param->aiynr_param;
    aiynr_params_static_t* psta = &aiynr_param->sta;
    aiynr_param_calib_t *calib = &aiynr_param->dyn.calib;
    aiynr_param_tunning_t *tunning = &aiynr_param->dyn.tunning;

    if (!cvtinfo->use_aiisp) {
        memset(ai_cfg, 0, sizeof(*ai_cfg));
        LOGD_AIYNR("aiisp is disabled");
        return;
    }

    isp_aiynr_param->is_bypass      = bypass;
    isp_aiynr_param->is_hdr         = (cvtinfo->frameNum > 1) ? true : false;
    isp_aiynr_param->sw_nar_inverse = tunning->sw_aiYnrT_narInverse_en;

    isp_aiynr_param->sw_pos_noiselimit = pow(tunning->sw_aiYnrT_noiseAdd_limit, 2) * 32767;
    isp_aiynr_param->sw_neg_noiselimit = (0xFFFF - isp_aiynr_param->sw_pos_noiselimit + 1) | (1 << 15);

    for (int i = 0; i < ISP35_AI_SIGMA_NUM; i++) {
        isp_aiynr_param->sw_out_decomp_y[i] = CLIP(tunning->sw_aiYnrT_decomp_curve[i] * (1 << 8), 0, 0xffff);
    }

    /* CTRL */
    ai_cfg->aiisp_raw12_msb     = 0;
    ai_cfg->aiisp_gain_mode     = 0;
    ai_cfg->aiisp_curve_en      = 0;
    ai_cfg->aipre_iir_en        = 0;
    ai_cfg->aipre_iir2ddr_en    = 0;
    ai_cfg->aipre_gain_en       = 1;
    ai_cfg->aipre_gain2ddr_en   = 1;
    ai_cfg->aipre_luma2gain_dis = (cvtinfo->rawWidth % 8 != 0) ? 1 : 0;
    ai_cfg->aipre_nl_ddr_mode   = 1;
    ai_cfg->aipre_yraw_sel      = 0; // yuv path
    ai_cfg->aipre_gain_bypass   = 0;
    ai_cfg->aipre_gain_mode     = 0;
    ai_cfg->aipre_narmap_inv    = tunning->sw_aiYnrT_narInverse_en;

    /* SIGMA_Y */
    for (i = 0; i < ISP35_AI_SIGMA_NUM; i++)
        ai_cfg->aiisp_sigma_y[i] = 0;

    /* AIPRE_NL_PRE */
    ai_cfg->aipre_scale     = 0;
    ai_cfg->aipre_zp        = 0;
    ai_cfg->aipre_black_lvl = 0;

    /* AIPRE_GAIN_PARA */
    ai_cfg->aipre_gain_alpha    = CLIP(tunning->sw_aiYnrT_gainMerge_alpha * (1 << 3), 0, 0xf);
    ai_cfg->aipre_global_gain   = CLIP(tunning->sw_aiYnrT_globalGain_strg * (1 << 4), 0, 0xff);
    ai_cfg->aipre_gain_ratio    = CLIP(tunning->sw_aiYnrT_noiseAdd_ratio * (1 << AIYNR_NAR_MAP_BIT_7) * 255, 0, 0x7fff);

    /* AIPRE_SIGMA_CURVE */
    for (i = 0; i < ISP35_AI_SIGMA_NUM; i++)
        ai_cfg->aipre_sigma_y[i] = 0;

    /* AIPRE_NOISE0 */
    ai_cfg->aipre_noise_mot_offset  = CLIP(sqrt(1.0 / tunning->sw_aiYnrT_noiseAddMot_offset) * 255.0, 0, 0xff);
    ai_cfg->aipre_noise_mot_gain    = CLIP(tunning->sw_aiYnrT_noiseAddMot_coeff * (1 << 3), -64, 63);
    ai_cfg->aipre_noise_luma_offset = 0;

    /* AIPRE_NOISE1 */
    ai_cfg->aipre_noise_luma_gain   = 0;
    ai_cfg->aipre_noise_luma_clip   = 0;
    ai_cfg->aipre_noise_luma_static = 0;

    /* AIPRE_NOISE2 */
    ai_cfg->aipre_nar_manual        = CLIP(tunning->sw_aiYnrT_narManual_strg * (1 << 7), 0, 0xff);
    ai_cfg->aipre_nar_manual_alpha  = CLIP(tunning->sw_aiYnrT_narManual_alpha * (1 << 4), 0, 0xf);
    /* VPSL_PYR_CTRL */
    ai_cfg->pyr_yraw_mode       = 1;
    ai_cfg->pyr_sigma_en        = 1;
    ai_cfg->pyr_yraw_sel        = 0; // y mode
    ai_cfg->pyr_gain_leftshift  = CLIP(tunning->sw_aiYnrT_gain_leftshift, 0, 7);
    ai_cfg->pyr_blacklvl_sig    = 0;

    /* VPSL_PYR_SIGMA_LUT */
    for (i = 0; i < ISP35_VPSL_SIGMA_NUM; i++) {
        ai_cfg->pyr_sigma_y[i] = CLIP((calib->sw_aiYnrC_sigma_lut[i] * tunning->sw_aiYnrT_sigmaLut_scale), 0, 0x3ff);
    }

#if 0
    ai_cfg->aiisp_raw12_msb = 0;
    ai_cfg->aiisp_gain_mode = 0;
    ai_cfg->aipre_iir_en = 0;
    ai_cfg->aipre_gain_en = 1;
    ai_cfg->aipre_gain2ddr_en = 1;
    ai_cfg->aipre_yraw_sel = 0;

    ai_cfg->pyr_yraw_mode = 1;
    ai_cfg->pyr_sigma_en = 1;
    ai_cfg->pyr_yraw_sel = 0;
    ai_cfg->pyr_gain_leftshift = 0;
    ai_cfg->pyr_blacklvl_sig = 0;

    ai_cfg->aipre_scale = (0x12000080 & 0xff);
    ai_cfg->aipre_zp = (0x12000080 >> 8) & 0xff;
    ai_cfg->aipre_black_lvl = (0x12000080 >> 20) & 0x1ff;

    ai_cfg->aipre_gain_alpha = (0x00400808 & 0xf);
    ai_cfg->aipre_global_gain = (0x00400808 >> 4) & 0xff;
    ai_cfg->aipre_gain_ratio = (0x00400808 >> 12) & 0x7fff;

    ai_cfg->aipre_sigma_y[0] = (0x0a020880 & 0x3ff);
    ai_cfg->aipre_sigma_y[1] = ((0x0a020880 >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[2] = ((0x0a020880 >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[3] = (0x100320be & 0x3ff);
    ai_cfg->aipre_sigma_y[4] = ((0x100320be >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[5] = ((0x100320be >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[6] = (0x16d5a12c & 0x3ff);
    ai_cfg->aipre_sigma_y[7] = ((0x16d5a12c >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[8] = ((0x16d5a12c >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[9] = (0x1845f172 & 0x3ff);
    ai_cfg->aipre_sigma_y[10] = ((0x1845f172 >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[11] = ((0x1845f172 >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[12] = (0x1a464186 & 0x3ff);
    ai_cfg->aipre_sigma_y[13] = ((0x1a464186 >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[14] = ((0x1a464186 >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[15] = (0x1ca6f1b0 & 0x3ff);
    ai_cfg->aipre_sigma_y[16] = ((0x1ca6f1b0 >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[17] = ((0x1ca6f1b0 >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[18] = (0x2007d1ce & 0x3ff);
    ai_cfg->aipre_sigma_y[19] = ((0x2007d1ce >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[20] = ((0x2007d1ce >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[21] = (0x24489a11 & 0x3ff);
    ai_cfg->aipre_sigma_y[22] = ((0x24489a11 >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[23] = ((0x24489a11 >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[24] = (0x29a9ee58 & 0x3ff);
    ai_cfg->aipre_sigma_y[25] = ((0x29a9ee58 >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[26] = ((0x29a9ee58 >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[27] = (0x30cb6aba & 0x3ff);
    ai_cfg->aipre_sigma_y[28] = ((0x30cb6aba >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[29] = ((0x30cb6aba >> 20) & 0x3ff);
    ai_cfg->aipre_sigma_y[30] = (0x3e7e6320 & 0x3ff);
    ai_cfg->aipre_sigma_y[31] = ((0x3e7e6320 >> 10) & 0x3ff);
    ai_cfg->aipre_sigma_y[32] = ((0x3e7e6320 >> 20) & 0x3ff);

    ai_cfg->aipre_noise_mot_offset = (0x00802480 & 0xff);
    ai_cfg->aipre_noise_mot_gain = (0x00802480 >> 8) & 0x7f;
    ai_cfg->aipre_noise_luma_offset = (0x00802480 >> 16) & 0x3ff;

    ai_cfg->aipre_noise_luma_gain = (0x80080023 & 0x7ff);
    ai_cfg->aipre_noise_luma_clip = (0x80080023 >> 12) & 0x3ff;
    ai_cfg->aipre_noise_luma_static = (0x80080023 >> 24) & 0xff;

    ai_cfg->aipre_nar_manual = (0x00000840 & 0xff);
    ai_cfg->aipre_nar_manual_alpha = (0x00000840 >> 8) & 0x1f;

    ai_cfg->pyr_sigma_y[0] = 0xe;
    ai_cfg->pyr_sigma_y[1] = 0x9;
    ai_cfg->pyr_sigma_y[2] = 0x6;
    ai_cfg->pyr_sigma_y[3] = 0x5;
    ai_cfg->pyr_sigma_y[4] = 0x4;
    ai_cfg->pyr_sigma_y[5] = 0x3;
    ai_cfg->pyr_sigma_y[6] = 0x2;
    ai_cfg->pyr_sigma_y[7] = 0x2;
    ai_cfg->pyr_sigma_y[8] = 0x1;
    ai_cfg->pyr_sigma_y[9] = 0x10;
    ai_cfg->pyr_sigma_y[10] = 0xd;
    ai_cfg->pyr_sigma_y[11] = 0xb;
    ai_cfg->pyr_sigma_y[12] = 0xa;
    ai_cfg->pyr_sigma_y[13] = 0x9;
    ai_cfg->pyr_sigma_y[14] = 0x8;
    ai_cfg->pyr_sigma_y[15] = 0x7;
    ai_cfg->pyr_sigma_y[16] = 0x6;
    ai_cfg->pyr_sigma_y[17] = 0x5;
    ai_cfg->pyr_sigma_y[18] = 0x14;
    ai_cfg->pyr_sigma_y[19] = 0x15;
    ai_cfg->pyr_sigma_y[20] = 0x14;
    ai_cfg->pyr_sigma_y[21] = 0x13;
    ai_cfg->pyr_sigma_y[22] = 0x11;
    ai_cfg->pyr_sigma_y[23] = 0xf;
    ai_cfg->pyr_sigma_y[24] = 0xd;
    ai_cfg->pyr_sigma_y[25] = 0xb;
    ai_cfg->pyr_sigma_y[26] = 0x9;
    ai_cfg->pyr_sigma_y[27] = 0x1a;
    ai_cfg->pyr_sigma_y[28] = 0x1f;
    ai_cfg->pyr_sigma_y[29] = 0x1e;
    ai_cfg->pyr_sigma_y[30] = 0x1c;
    ai_cfg->pyr_sigma_y[31] = 0x19;
    ai_cfg->pyr_sigma_y[32] = 0x17;
    ai_cfg->pyr_sigma_y[33] = 0x14;
    ai_cfg->pyr_sigma_y[34] = 0x11;
    ai_cfg->pyr_sigma_y[35] = 0xe;
    ai_cfg->pyr_sigma_y[36] = 0x20;
    ai_cfg->pyr_sigma_y[37] = 0x28;
    ai_cfg->pyr_sigma_y[38] = 0x27;
    ai_cfg->pyr_sigma_y[39] = 0x25;
    ai_cfg->pyr_sigma_y[40] = 0x22;
    ai_cfg->pyr_sigma_y[41] = 0x1e;
    ai_cfg->pyr_sigma_y[42] = 0x1b;
    ai_cfg->pyr_sigma_y[43] = 0x17;
    ai_cfg->pyr_sigma_y[44] = 0x13;
    ai_cfg->pyr_sigma_y[45] = 0x27;
    ai_cfg->pyr_sigma_y[46] = 0x32;
    ai_cfg->pyr_sigma_y[47] = 0x31;
    ai_cfg->pyr_sigma_y[48] = 0x2e;
    ai_cfg->pyr_sigma_y[49] = 0x2a;
    ai_cfg->pyr_sigma_y[50] = 0x26;
    ai_cfg->pyr_sigma_y[51] = 0x21;
    ai_cfg->pyr_sigma_y[52] = 0x1d;
    ai_cfg->pyr_sigma_y[53] = 0x18;
    ai_cfg->pyr_sigma_y[54] = 0x2e;
    ai_cfg->pyr_sigma_y[55] = 0x3c;
    ai_cfg->pyr_sigma_y[56] = 0x3a;
    ai_cfg->pyr_sigma_y[57] = 0x37;
    ai_cfg->pyr_sigma_y[58] = 0x32;
    ai_cfg->pyr_sigma_y[59] = 0x2e;
    ai_cfg->pyr_sigma_y[60] = 0x28;
    ai_cfg->pyr_sigma_y[61] = 0x23;
    ai_cfg->pyr_sigma_y[62] = 0x1d;
    ai_cfg->pyr_sigma_y[63] = 0x35;
    ai_cfg->pyr_sigma_y[64] = 0x46;
    ai_cfg->pyr_sigma_y[65] = 0x44;
    ai_cfg->pyr_sigma_y[66] = 0x40;
    ai_cfg->pyr_sigma_y[67] = 0x3b;
    ai_cfg->pyr_sigma_y[68] = 0x35;
    ai_cfg->pyr_sigma_y[69] = 0x2f;
    ai_cfg->pyr_sigma_y[70] = 0x28;
    ai_cfg->pyr_sigma_y[71] = 0x21;
    ai_cfg->pyr_sigma_y[72] = 0x3c;
    ai_cfg->pyr_sigma_y[73] = 0x50;
    ai_cfg->pyr_sigma_y[74] = 0x4e;
    ai_cfg->pyr_sigma_y[75] = 0x49;
    ai_cfg->pyr_sigma_y[76] = 0x43;
    ai_cfg->pyr_sigma_y[77] = 0x3d;
    ai_cfg->pyr_sigma_y[78] = 0x36;
    ai_cfg->pyr_sigma_y[79] = 0x2e;
    ai_cfg->pyr_sigma_y[80] = 0x26;
#endif

    rk_aiq_aiynr_params_print(ai_cfg);

    return;

}

#endif
