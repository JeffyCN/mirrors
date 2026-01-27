/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rk_aiq_isp39_modules.h"
#define RATIO_DEFAULT               (1.0f)
#define RATIO_MAX                   (256.0f)
#define SW_HDRMGE_GAIN_FIX          (0x40)
#define SW_HDRMGE_GAIN_INV_FIX      (0xfff)
#define OECURVESMOOTHMAX            (200)
#define EACHOECURVESMOOTHMAX        (50)
#define MDCURVESMOOTHMAX            (200)
#define MDCURVEOFFSETMAX            (100)
#define SW_HDRMGE_LM_DIF_0P9_FIX    (255)
#define SW_HDRMGE_MS_DIF_0P8_FIX    (255)
#define SHORT_MODE_COEF_MAX         (0.001)
#define HDR_LONG_FRMAE_MODE_OECURVE (0)

static int mergeClipValue(float posx, int BitInt, int BitFloat, bool ifBitMax) {
    int yOutInt = 0, yOutIntMin = 0, yOutIntMax = 0;

    if (ifBitMax)
        yOutIntMax = (int)(pow(2, (BitFloat + BitInt)));
    else
        yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    yOutInt = CLIP((int)(posx * pow(2, BitFloat)), yOutIntMin, yOutIntMax);

    return yOutInt;
}

void rk_aiq_merge22_params_dump(void* attr, isp_params_t* isp_params) {
    LOGD_AMERGE("gain0_inv %d", isp_params->isp_cfg->others.hdrmge_cfg.gain0_inv);
    LOGD_AMERGE("gain0 %d", isp_params->isp_cfg->others.hdrmge_cfg.gain0);
    LOGD_AMERGE("gain1_inv %d", isp_params->isp_cfg->others.hdrmge_cfg.gain1_inv);
    LOGD_AMERGE("gain1 %d", isp_params->isp_cfg->others.hdrmge_cfg.gain1);
    LOGD_AMERGE("gain2 %d", isp_params->isp_cfg->others.hdrmge_cfg.gain2);
    LOGD_AMERGE("lm_dif_0p15 %d", isp_params->isp_cfg->others.hdrmge_cfg.lm_dif_0p15);
    LOGD_AMERGE("lm_dif_0p9 %d", isp_params->isp_cfg->others.hdrmge_cfg.lm_dif_0p9);
    LOGD_AMERGE("ms_diff_0p15 %d", isp_params->isp_cfg->others.hdrmge_cfg.ms_diff_0p15);
    LOGD_AMERGE("ms_dif_0p8 %d", isp_params->isp_cfg->others.hdrmge_cfg.ms_dif_0p8);
    LOGD_AMERGE("s_base %d", isp_params->isp_cfg->others.hdrmge_cfg.s_base);
    LOGD_AMERGE("sw_hdrmge_ms_thd0 %d", isp_params->isp_cfg->others.hdrmge_cfg.ms_thd0);
    LOGD_AMERGE("ms_thd0 %d", isp_params->isp_cfg->others.hdrmge_cfg.ms_thd0);
    LOGD_AMERGE("ms_thd1 %d", isp_params->isp_cfg->others.hdrmge_cfg.ms_thd1);
    LOGD_AMERGE("ms_scl %d", isp_params->isp_cfg->others.hdrmge_cfg.ms_scl);
    LOGD_AMERGE("lm_thd0 %d", isp_params->isp_cfg->others.hdrmge_cfg.lm_thd0);
    LOGD_AMERGE("lm_thd1 %d", isp_params->isp_cfg->others.hdrmge_cfg.lm_thd1);
    LOGD_AMERGE("lm_scl %d", isp_params->isp_cfg->others.hdrmge_cfg.lm_scl);
    LOGD_AMERGE("each_raw_en 0x%x each_raw_gain0 0x%x each_raw_gain1 0x%x\n",
                isp_params->isp_cfg->others.hdrmge_cfg.each_raw_en,
                isp_params->isp_cfg->others.hdrmge_cfg.each_raw_gain0,
                isp_params->isp_cfg->others.hdrmge_cfg.each_raw_gain1);
    for (int i = 0; i < 17; i++) {
        LOGD_AMERGE("curve_0[%d] %d", i, isp_params->isp_cfg->others.hdrmge_cfg.curve.curve_0[i]);
        LOGD_AMERGE("curve_1[%d] %d", i, isp_params->isp_cfg->others.hdrmge_cfg.curve.curve_1[i]);
        LOGD_AMERGE("e_y[%d] %d", i, isp_params->isp_cfg->others.hdrmge_cfg.e_y[i]);
    }
}

void rk_aiq_merge22_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo, mergeLuma2Wgt_t* pMergeLuma2Wgt) {
    struct isp32_hdrmge_cfg* phwcfg = &isp_params->isp_cfg->others.hdrmge_cfg;
    mge_param_t* merge_param        = (mge_param_t*)attr;
    mge_params_dyn_t* pdyn          = &merge_param->dyn;
    mge_params_static_t* psta       = &merge_param->sta;
    int luma_idx[17] = {512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024};

    if (psta->expRat.sw_mgeCfg_expRat_mode == mge_expRatSyncAE_mode) {
        psta->expRat.sw_mgeCfg_expRatFix_val = cvtinfo->L2S_Ratio;
    }
    bool LongFrmMode = cvtinfo->ae_exp->HdrExp[cvtinfo->frameNum - 1].exp_real_params.longfrm_mode;
    if (LongFrmMode) {
        psta->expRat.sw_mgeCfg_expRatFix_val = 1.0f;
    }

    phwcfg->mode = cvtinfo->frameNum - 1;
    phwcfg->gain0_inv =
        mergeClipValue(RATIO_DEFAULT / psta->expRat.sw_mgeCfg_expRatFix_val, 0, 12, false);
    phwcfg->gain0 = mergeClipValue(psta->expRat.sw_mgeCfg_expRatFix_val, 8, 6, false);
    ;
    phwcfg->gain1_inv = SW_HDRMGE_GAIN_INV_FIX;
    phwcfg->gain1     = SW_HDRMGE_GAIN_FIX;
    phwcfg->gain2     = SW_HDRMGE_GAIN_FIX;
    phwcfg->ms_diff_0p15 =
        (unsigned char)(pdyn->mdWgt_baseHdrL.hw_mgeT_lumaLutCreate_offset * MDCURVEOFFSETMAX);
    phwcfg->ms_dif_0p8 = SW_HDRMGE_MS_DIF_0P8_FIX;  // psta->expRat.hw_mgeCfg_lumaHdrL_scale;
    if (LongFrmMode) {
        for (int i = 0; i < MGE_OECURVE_LEN; i++) phwcfg->e_y[i] = HDR_LONG_FRMAE_MODE_OECURVE;
    } else {
        if (pdyn->oeWgt.sw_mgeT_oeLut_mode == mge_cfgByCurveDirectly_mode) {
            for (int i = 0; i < MGE_OECURVE_LEN; i++) {
                phwcfg->e_y[i] = 1024.0f * pdyn->oeWgt.sw_mgeT_luma2Wgt_val[i];
                phwcfg->e_y[i] = MIN(phwcfg->e_y[i], 1023);
            }
        } else {
            int step    = 32;
            float curve = 0.0f;
            float k     = 511.0f;
            for (int i = 0; i < MGE_OECURVE_LEN; ++i) {
                curve = 1.0f + exp(-pdyn->oeWgt.sw_mgeT_lutCreate_slope * OECURVESMOOTHMAX *
                                   (k / 1023.0f - pdyn->oeWgt.sw_mgeT_lutCreate_offset / 256.0f));
                curve = 1024.0f / curve;
                phwcfg->e_y[i] = round(curve);
                phwcfg->e_y[i] = MIN(phwcfg->e_y[i], 1023);
                k += step;
            }
        }
    }

#if MGE_WGT_USE_OLD_STRUCT
    for (int i = 0; i < MGE_OECURVE_LEN; i++) {
        pMergeLuma2Wgt->luma_idx[i] = luma_idx[i];
        pMergeLuma2Wgt->luma_wgt[i] = (float)phwcfg->e_y[i] / 1024.0;
    }
#else
    pMergeLuma2Wgt->wgtCurve_num = cvtinfo->frameNum - 1;
    for (int i = 0; i < pMergeLuma2Wgt->wgtCurve_num; i++) {
        for (int j = 0; j < 17; j++) {
            pMergeLuma2Wgt->wgtCurve[i].idx[j] = luma_idx[j];
            pMergeLuma2Wgt->wgtCurve[i].val[j] = (float)phwcfg->e_y[j] / 1024.0f;
        }
    }
    LOGD_AMERGE("wgtCurve_num %d", pMergeLuma2Wgt->wgtCurve_num);
    LOGD_AMERGE("%s: MS_wgt_idx: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pMergeLuma2Wgt->wgtCurve[0].val[0],
                pMergeLuma2Wgt->wgtCurve[0].val[1], pMergeLuma2Wgt->wgtCurve[0].val[2],
                pMergeLuma2Wgt->wgtCurve[0].val[3], pMergeLuma2Wgt->wgtCurve[0].val[4],
                pMergeLuma2Wgt->wgtCurve[0].val[5], pMergeLuma2Wgt->wgtCurve[0].val[6],
                pMergeLuma2Wgt->wgtCurve[0].val[7], pMergeLuma2Wgt->wgtCurve[0].val[8],
                pMergeLuma2Wgt->wgtCurve[0].val[9], pMergeLuma2Wgt->wgtCurve[0].val[10],
                pMergeLuma2Wgt->wgtCurve[0].val[11], pMergeLuma2Wgt->wgtCurve[0].val[12],
                pMergeLuma2Wgt->wgtCurve[0].val[13], pMergeLuma2Wgt->wgtCurve[0].val[14],
                pMergeLuma2Wgt->wgtCurve[0].val[15], pMergeLuma2Wgt->wgtCurve[0].val[16]);
    LOGD_AMERGE("%s: MS_wgt_val: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
                __FUNCTION__, pMergeLuma2Wgt->wgtCurve[0].val[0],
                pMergeLuma2Wgt->wgtCurve[0].val[1], pMergeLuma2Wgt->wgtCurve[0].val[2],
                pMergeLuma2Wgt->wgtCurve[0].val[3], pMergeLuma2Wgt->wgtCurve[0].val[4],
                pMergeLuma2Wgt->wgtCurve[0].val[5], pMergeLuma2Wgt->wgtCurve[0].val[6],
                pMergeLuma2Wgt->wgtCurve[0].val[7], pMergeLuma2Wgt->wgtCurve[0].val[8],
                pMergeLuma2Wgt->wgtCurve[0].val[9], pMergeLuma2Wgt->wgtCurve[0].val[10],
                pMergeLuma2Wgt->wgtCurve[0].val[11], pMergeLuma2Wgt->wgtCurve[0].val[12],
                pMergeLuma2Wgt->wgtCurve[0].val[13], pMergeLuma2Wgt->wgtCurve[0].val[14],
                pMergeLuma2Wgt->wgtCurve[0].val[15], pMergeLuma2Wgt->wgtCurve[0].val[16]);
    LOGD_AMERGE("%s: LM_wgt_idx: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__, pMergeLuma2Wgt->wgtCurve[1].val[0],
                pMergeLuma2Wgt->wgtCurve[1].val[1], pMergeLuma2Wgt->wgtCurve[1].val[2],
                pMergeLuma2Wgt->wgtCurve[1].val[3], pMergeLuma2Wgt->wgtCurve[1].val[4],
                pMergeLuma2Wgt->wgtCurve[1].val[5], pMergeLuma2Wgt->wgtCurve[1].val[6],
                pMergeLuma2Wgt->wgtCurve[1].val[7], pMergeLuma2Wgt->wgtCurve[1].val[8],
                pMergeLuma2Wgt->wgtCurve[1].val[9], pMergeLuma2Wgt->wgtCurve[1].val[10],
                pMergeLuma2Wgt->wgtCurve[1].val[11], pMergeLuma2Wgt->wgtCurve[1].val[12],
                pMergeLuma2Wgt->wgtCurve[1].val[13], pMergeLuma2Wgt->wgtCurve[1].val[14],
                pMergeLuma2Wgt->wgtCurve[1].val[15], pMergeLuma2Wgt->wgtCurve[1].val[16]);
    LOGD_AMERGE("%s: LM_wgt_val: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
                __FUNCTION__, pMergeLuma2Wgt->wgtCurve[1].val[0],
                pMergeLuma2Wgt->wgtCurve[1].val[1], pMergeLuma2Wgt->wgtCurve[1].val[2],
                pMergeLuma2Wgt->wgtCurve[1].val[3], pMergeLuma2Wgt->wgtCurve[1].val[4],
                pMergeLuma2Wgt->wgtCurve[1].val[5], pMergeLuma2Wgt->wgtCurve[1].val[6],
                pMergeLuma2Wgt->wgtCurve[1].val[7], pMergeLuma2Wgt->wgtCurve[1].val[8],
                pMergeLuma2Wgt->wgtCurve[1].val[9], pMergeLuma2Wgt->wgtCurve[1].val[10],
                pMergeLuma2Wgt->wgtCurve[1].val[11], pMergeLuma2Wgt->wgtCurve[1].val[12],
                pMergeLuma2Wgt->wgtCurve[1].val[13], pMergeLuma2Wgt->wgtCurve[1].val[14],
                pMergeLuma2Wgt->wgtCurve[1].val[15], pMergeLuma2Wgt->wgtCurve[1].val[16]);
#endif

    if (pdyn->sw_mgeT_baseFrm_mode == mge_baseHdrL_mode) {
        if (pdyn->mdWgt_baseHdrL.sw_mgeT_mdLut_mode == mge_cfgByCurveDirectly_mode) {
            for (int i = 0; i < MGE_MDCURVE_LEN; i++) {
                phwcfg->curve.curve_0[i] =
                    1024.0f * pdyn->mdWgt_baseHdrL.sw_mgeT_lumaDiff2Wgt_val[i];
                phwcfg->curve.curve_0[i] = MIN(phwcfg->curve.curve_0[i], 1023);
                phwcfg->l_raw0[i] = 1024.0f * pdyn->mdWgt_baseHdrL.sw_mgeT_rawChDiff2Wgt_val[i];
                phwcfg->l_raw0[i] = MIN(phwcfg->l_raw0[i], 1023);
            }
        } else {
            int step     = 16;
            float curve0 = 0.0f;
            float curve1 = 0.0f;
            float k      = 0.0f;
            for (int i = 0; i < MGE_MDCURVE_LEN; ++i) {
                curve0 = 1.0f +
                         exp(-pdyn->mdWgt_baseHdrL.sw_mgeT_lumaLutCreate_slope * MDCURVESMOOTHMAX *
                             (k / 1023.0f - pdyn->mdWgt_baseHdrL.hw_mgeT_lumaLutCreate_offset *
                              MDCURVEOFFSETMAX / 256.0f));
                curve0                   = 1024.0f / curve0;
                phwcfg->curve.curve_0[i] = round(curve0);
                phwcfg->curve.curve_0[i] = MIN(phwcfg->curve.curve_0[i], 1023);
                // phwcfg->curve.curve_1[i] = phwcfg->curve.curve_0[i];
                k += step;
            }
            // merge v12 add
            if (pdyn->sw_mgeT_baseHdrL_mode == mge_oeMdByLuma_rawCh_mode) {
                step   = 64;
                curve0 = 0.0f;
                k      = 0.0f;

                for (int i = 0; i < MGE_MDCURVE_LEN; ++i) {
                    curve0 =
                        1.0f +
                        exp(-pdyn->mdWgt_baseHdrL.sw_mgeT_rawChLutCreate_slope *
                            EACHOECURVESMOOTHMAX *
                            (k / 1023.0f - pdyn->mdWgt_baseHdrL.sw_mgeT_rawChLutCreate_offset));
                    curve0            = 1024.0f / curve0;
                    phwcfg->l_raw0[i] = round(curve0);
                    phwcfg->l_raw0[i] = MIN(phwcfg->l_raw0[i], 1023);
                    phwcfg->l_raw1[i] = phwcfg->l_raw0[i];
                    k += step;
                }
            }
        }
    } else if (pdyn->sw_mgeT_baseFrm_mode == mge_baseHdrS_mode) {
        float step  = 1.0f / 16.0f;
        float curve = 0.0f;
        for (int i = 0; i < MGE_MDCURVE_LEN; ++i) {
            curve                    = 0.01f + pow(i * step, 2.0f);
            curve                    = 1024.0f * pow(i * step, 2.0f) / curve;
            phwcfg->curve.curve_0[i] = round(curve);
            phwcfg->curve.curve_0[i] = MIN(phwcfg->curve.curve_0[i], 1023);
            // phwcfg->curve.curve_1[i] = phwcfg->curve.curve_0[i];
        }
    }

    phwcfg->s_base = pdyn->sw_mgeT_baseFrm_mode;
    if (pdyn->sw_mgeT_baseFrm_mode == mge_baseHdrS_mode) {
        float SGain = cvtinfo->ae_exp->HdrExp[0].exp_real_params.analog_gain *
                      cvtinfo->ae_exp->HdrExp[0].exp_real_params.digital_gain *
                      cvtinfo->ae_exp->HdrExp[0].exp_real_params.isp_dgain;
        float Coef = pdyn->mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg * SHORT_MODE_COEF_MAX;
        float sw_mgeT_wgtMaxTh_strg = pow(100.0f * Coef * SGain, 0.5f);

        // calc md curve
        float sw_hdrmge_ms_thd0 = pdyn->mdWgt_baseHdrS.hw_mgeT_wgtZero_thred;
        float sw_hdrmge_ms_thd1 = sw_mgeT_wgtMaxTh_strg;
        // phwcfg->lm_thd0 = mergeClipValue(pdyn->mdWgt_baseHdrS.hw_mgeT_wgtZero_thred, 0, 10,
        // false); phwcfg->lm_thd1 = pdyn->mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg;
        float sw_hdrmge_ms_scl = (sw_hdrmge_ms_thd0 == sw_hdrmge_ms_thd1)
                                 ? 0.0f
                                 : (1.0f / (sw_hdrmge_ms_thd1 - sw_hdrmge_ms_thd0));
        // float sw_hdrmge_lm_scl = (pdyn->mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg ==
        // pdyn->mdWgt_baseHdrS.hw_mgeT_wgtZero_thred)
        //                                 ? 0.0f
        //                                 : (1.0f / (pdyn->mdWgt_baseHdrS.sw_mgeT_wgtMaxTh_strg -
        //                                 pdyn->mdWgt_baseHdrS.hw_mgeT_wgtZero_thred));
        phwcfg->ms_thd0 = mergeClipValue(sw_hdrmge_ms_thd0, 0, 10, false);
        phwcfg->ms_thd1 = mergeClipValue(sw_hdrmge_ms_thd1, 0, 10, false);
        phwcfg->ms_scl  = (unsigned short)(64.0f * sw_hdrmge_ms_scl);
        // phwcfg->lm_scl = (unsigned short)(64.0f * sw_hdrmge_lm_scl);

        // calc gain0 in mge_baseHdrS_mode
        phwcfg->gain0_inv = phwcfg->gain0_inv * pdyn->mdWgt_baseHdrS.sw_mgeT_lumaDiff_scale;
        phwcfg->gain0_inv = phwcfg->gain0_inv > 0xfff ? 0xfff : phwcfg->gain0_inv;
    }
    // // merge v12 add
    phwcfg->each_raw_en = pdyn->sw_mgeT_baseHdrL_mode;
    if (pdyn->sw_mgeT_baseHdrL_mode == mge_oeMdByLuma_rawCh_mode) {
        phwcfg->each_raw_gain0 = mergeClipValue(psta->expRat.sw_mgeCfg_expRatFix_val, 8, 6, false);
        phwcfg->each_raw_gain1 = SW_HDRMGE_GAIN_FIX;
    }
    rk_aiq_merge22_params_dump(attr, isp_params);
}
