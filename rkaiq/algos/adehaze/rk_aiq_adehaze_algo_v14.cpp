/*
 * rk_aiq_adehaze_algo_v14.cpp
 *
 *  Copyright (c) 2024 Rockchip Corporation
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
 *
 */
#include "rk_aiq_adehaze_algo_v14.h"

#include <string.h>

#include "xcam_log.h"

float GetInterpRatioV14(float* pX, int& lo, int& hi, float CtrlValue, int length_max) {
    float ratio = 0.0f;

    if (CtrlValue < pX[0]) {
        lo    = 0;
        hi    = 0;
        ratio = 0.0f;
    } else if (CtrlValue >= pX[length_max - 1]) {
        lo    = length_max - 1;
        hi    = length_max - 1;
        ratio = 0.0f;
    } else {
        for (int i = 0; i < length_max - 1; i++) {
            if (CtrlValue >= pX[i] && CtrlValue < pX[i + 1]) {
                lo    = i;
                hi    = i + 1;
                ratio = (CtrlValue - pX[i]) / (pX[i + 1] - pX[i]);
                break;
            } else
                continue;
        }
    }

    return ratio;
}

int DehazeLinearInterpV14(const float* pX, const float* pY, float posx, int BitInt, int BitFloat,
                          int XSize) {
    int index;
    float yOut     = 0.0;
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    if (posx >= pX[XSize - 1]) {
        yOut = pY[XSize - 1];
    } else if (posx <= pX[0]) {
        yOut = pY[0];
    } else {
        index = 0;
        while ((posx >= pX[index]) && (index < XSize)) {
            index++;
        }
        index -= 1;
        yOut = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index])) +
               pY[index];
    }

    yOutInt = LIMIT_VALUE((int)(yOut * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

void DehazeHistWrTableInterpV14(const HistWr_t* pCurveIn, mManual_curve_t* pCurveOut, float posx) {
    int i       = 0;
    float ratio = 1.0;

    if (posx < pCurveIn->manual_curve[0].CtrlData) {
        for (int i = 0; i < DHAZ_V12_HIST_WR_CURVE_NUM; i++) {
            pCurveOut->curve_x[i] = pCurveIn->manual_curve[0].curve_x[i];
            pCurveOut->curve_y[i] = pCurveIn->manual_curve[0].curve_y[i];
        }
    } else if (posx >= pCurveIn->manual_curve[12].CtrlData) {
        for (int i = 0; i < DHAZ_V12_HIST_WR_CURVE_NUM; i++) {
            pCurveOut->curve_x[i] = pCurveIn->manual_curve[12].curve_x[i];
            pCurveOut->curve_y[i] = pCurveIn->manual_curve[12].curve_y[i];
        }
    } else {
        for (int i = 0; i < DHAZ_CTRL_DATA_STEP_MAX; i++) {
            if (posx >= pCurveIn->manual_curve[i].CtrlData &&
                posx < pCurveIn->manual_curve[i + 1].CtrlData) {
                if ((pCurveIn->manual_curve[i + 1].CtrlData - pCurveIn->manual_curve[i].CtrlData) !=
                    0)
                    ratio = (posx - pCurveIn->manual_curve[i].CtrlData) /
                            (pCurveIn->manual_curve[i + 1].CtrlData -
                             pCurveIn->manual_curve[i].CtrlData);
                else
                    LOGE_ADEHAZE("Dehaze zero in %s(%d) \n", __func__, __LINE__);

                for (int j = 0; j < DHAZ_V12_HIST_WR_CURVE_NUM; j++) {
                    pCurveOut->curve_x[j] = ratio * (pCurveIn->manual_curve[i + 1].curve_x[j] -
                                                     pCurveIn->manual_curve[i].curve_x[j]) +
                                            pCurveIn->manual_curve[i].curve_x[j];
                    pCurveOut->curve_y[j] = ratio * (pCurveIn->manual_curve[i + 1].curve_y[j] -
                                                     pCurveIn->manual_curve[i].curve_y[j]) +
                                            pCurveIn->manual_curve[i].curve_y[j];
                }
                break;
            } else
                continue;
        }
    }

    // check curve_x
    for (int i = 1; i < DHAZ_CTRL_DATA_STEP_MAX; i++) {
        if (!(pCurveOut->curve_x[i] % HSIT_WR_MIN_STEP))
            continue;
        else {
            int orig_x            = pCurveOut->curve_x[i];
            pCurveOut->curve_x[i] = HSIT_WR_MIN_STEP * (pCurveOut->curve_x[i] / HSIT_WR_MIN_STEP);
            if (orig_x != 0) {
                pCurveOut->curve_y[i] = pCurveOut->curve_y[i - 1] +
                                        (pCurveOut->curve_x[i] - pCurveOut->curve_x[i - 1]) *
                                            (pCurveOut->curve_y[i] - pCurveOut->curve_y[i - 1]) /
                                            (orig_x - pCurveOut->curve_x[i - 1]);
            }
            continue;
        }
    }

#if 0
    LOGD_ADEHAZE(
        "%s hist_wr.curve_x[0~11]:0x%x 0x%x 0x%x "
        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        __func__, pCurveOut->curve_x[0], pCurveOut->curve_x[1], pCurveOut->curve_x[2],
        pCurveOut->curve_x[3], pCurveOut->curve_x[4], pCurveOut->curve_x[5], pCurveOut->curve_x[6],
        pCurveOut->curve_x[7], pCurveOut->curve_x[8], pCurveOut->curve_x[9], pCurveOut->curve_x[10],
        pCurveOut->curve_x[11]);
    LOGD_ADEHAZE(
        "%s hist_wr.curve_y[0~11]:0x%x 0x%x 0x%x "
        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
        __func__, pCurveOut->curve_y[0], pCurveOut->curve_y[1], pCurveOut->curve_y[2],
        pCurveOut->curve_y[3], pCurveOut->curve_y[4], pCurveOut->curve_y[5], pCurveOut->curve_y[6],
        pCurveOut->curve_y[7], pCurveOut->curve_y[8], pCurveOut->curve_y[9], pCurveOut->curve_y[10],
        pCurveOut->curve_y[11]);
#endif
}

float LinearInterpV14(const float* pX, const float* pY, float posx, int XSize) {
    int index;
    float yOut = 0;

    if (posx >= pX[XSize - 1]) {
        yOut = pY[XSize - 1];
    } else if (posx <= pX[0]) {
        yOut = pY[0];
    } else {
        index = 0;
        while ((posx >= pX[index]) && (index < XSize)) {
            index++;
        }
        index -= 1;
        yOut = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index])) +
               pY[index];
    }

    return yOut;
}

int ClipFloatValueV14(float posx, int BitInt, int BitFloat) {
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    yOutInt = LIMIT_VALUE((int)(posx * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

unsigned int ClipIntValueV14(unsigned int posx, int BitInt, int BitFloat) {
    unsigned int yOutInt    = 0;
    unsigned int yOutIntMax = (1 << (BitFloat + BitInt)) - 1;

    yOutInt = LIMIT_VALUE_UNSIGNED(posx << BitFloat, yOutIntMax);
    return yOutInt;
}

XCamReturn TransferHistWr2Res(RkAiqAdehazeProcResult_t* pProcRes, mManual_curve_t* pCurve) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    /*
// check curve_x
for (int i = 0; i < DHAZ_V12_HIST_WR_CURVE_NUM - 2; i++) {
    if (!(pCurve->curve_x[i] % HSIT_WR_MIN_STEP) &&
        !(pCurve->curve_x[i + 1] % HSIT_WR_MIN_STEP)) {
        if (pCurve->curve_x[i] < pCurve->curve_x[i + 1])
            continue;
        else {
            LOGE_ADEHAZE("%s hist_wr.manu_curve.curve_x[%d] is samller than curve_x[%d]\n",
                         __func__, i + 1, i);
            return XCAM_RETURN_ERROR_PARAM;
        }
    } else {
        LOGE_ADEHAZE("%s hist_wr.manu_curve.curve_x[%d] can not be divided by 16\n", __func__,
                     i);
        return XCAM_RETURN_ERROR_PARAM;
    }
}
if (!(!(pCurve->curve_x[16] % HSIT_WR_MIN_STEP) || pCurve->curve_x[16] == HSIT_WR_X_MAX)) {
    LOGE_ADEHAZE("%s hist_wr.manu_curve.curve_x[12] can not be divided by 16\n", __func__);
    return XCAM_RETURN_ERROR_PARAM;
}

int step      = 0;
int add_knots = 0;
int k         = 0;
for (int i = 0; i < DHAZ_V12_HIST_WR_CURVE_NUM - 2; i++) {
    pProcRes->ProcResV14.hist_wr[k] = pCurve->curve_y[i];
    pProcRes->ProcResV14.hist_wr[k] =
        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.hist_wr[k], BIT_10_MAX);
    step      = (pCurve->curve_x[i + 1] - pCurve->curve_x[i]) / HSIT_WR_MIN_STEP;
    add_knots = step;
    for (int j = 1; step; step--) {
        pProcRes->ProcResV14.hist_wr[k + j] =
            pCurve->curve_y[i] + HSIT_WR_MIN_STEP * j *
                                     (pCurve->curve_y[i + 1] - pCurve->curve_y[i]) /
                                     (pCurve->curve_x[i + 1] - pCurve->curve_x[i]);
        pProcRes->ProcResV14.hist_wr[k + j] =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.hist_wr[k + j], BIT_10_MAX);
        j++;
    }
    k += add_knots;
}

step = (1024 - pCurve->curve_x[15]) / HSIT_WR_MIN_STEP;
for (int j = 1; step; step--) {
    pProcRes->ProcResV14.hist_wr[k + j] =
        pCurve->curve_y[15] +
        HSIT_WR_MIN_STEP * j * (1024 - pCurve->curve_y[15]) / (1024 - pCurve->curve_x[15]);
    pProcRes->ProcResV14.hist_wr[k + j] =
        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.hist_wr[k + j], BIT_10_MAX);
    j++;
}
*/

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

void calc_cdf(int* hist, int* cdf, int bin_num, float clim, uint16_t vmin, uint16_t vmax) {
    int sum = 0;
    int i;
    int fix_clim;
    int extra = 0;
    int add;

    if (bin_num <= 0) return;

    for (i = 0; i < bin_num; ++i) {
        hist[i] = hist[i] * 8;
        sum += hist[i];
    }

    fix_clim = ((int)(clim * sum)) / bin_num;

    for (i = 0; i < bin_num; ++i) {
        if (hist[i] > fix_clim) {
            extra += hist[i] - fix_clim;
            hist[i] = fix_clim;
        }
    }

    add = (extra + bin_num / 2) / bin_num;

    for (i = 0; i < bin_num; ++i) {
        hist[i] += add;
    }

    cdf[0] = hist[0];

    for (i = 1; i < bin_num; ++i) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    if (cdf[bin_num - 1] > 0) {
        for (i = 0; i < bin_num; ++i) {
            cdf[i] = cdf[i] * (vmax - vmin) / cdf[bin_num - 1] + vmin;
        }
    } else {
        for (i = 0; i < bin_num; ++i) {
            cdf[i] = i * (vmax - vmin) / (bin_num - 1) + vmin;
        }
    }
}

XCamReturn TransferHistWrSemiAtuos2Res(RkAiqAdehazeProcResult_t* pProcRes,
                                       mhist_wr_semiauto_t* pSemiAutoCurve,
                                       dehaze_stats_v14_t* pStats, bool stats_true) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    if (!stats_true) return XCAM_RETURN_BYPASS;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    /*
int i;
int hist[64];
int cdf_out[64];
int dark_idx           = 1;
int mean               = 0;
static bool first_time = true;

if (first_time) {
    for (i = 0; i < 63; ++i) {
        pProcRes->ProcResV14.hist_wr[i] = (i + 1) * 16;
    }
    pProcRes->ProcResV14.hist_wr[i] = 1023;
    first_time                      = false;
    return XCAM_RETURN_NO_ERROR;
}

if (pStats->h_rgb_iir[63] != 1023) {
    LOGW_ADEHAZE("invalid cdf input, using normalize output\n");
    return XCAM_RETURN_BYPASS;
}

for (i = 0; i < 64; ++i) {
    hist[i] = (pStats->h_rgb_iir[i / 2] >> ((i % 2) * 16)) & 0xffff;
}

for (i = 63; i > 0; --i) {
    hist[i] = LIMIT_VALUE(pStats->h_rgb_iir[i] - pStats->h_rgb_iir[i - 1], 1023, 0);
    mean += (i + 1) * 16 * hist[i];
}

hist[0] = pStats->h_rgb_iir[0];
mean += 16 * hist[0];
mean                    = mean >> 10;
pSemiAutoCurve->dark_th = MAX(1024 - mean, pSemiAutoCurve->dark_th);
dark_idx                = (int(pSemiAutoCurve->dark_th + 8.0f) / 16.0f);
dark_idx                = LIMIT_VALUE(dark_idx, 64, 1);

calc_cdf(hist, cdf_out, dark_idx, pSemiAutoCurve->clim0, 0, dark_idx * 16);
calc_cdf(hist + dark_idx, cdf_out + dark_idx, 64 - dark_idx, pSemiAutoCurve->clim1,
         dark_idx * 16, 1023);

for (i = 0; i < 64; ++i) {
    if (i < dark_idx)
        cdf_out[i] = cdf_out[i] < (i + 1) * 16 ? (i + 1) * 16 : cdf_out[i];
    else if (cdf_out[i] < cdf_out[i - 1])
        cdf_out[i] = cdf_out[i - 1];
    pProcRes->ProcResV14.hist_wr[i] =
        (7 * pProcRes->ProcResV14.hist_wr[i] + cdf_out[i] + 4) / 8;
    pProcRes->ProcResV14.hist_wr[i] =
        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.hist_wr[i], 1023);
}
*/

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

void stManuGetCommomParamsV14(mDehazeAttrV14_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                              int rawWidth, int rawHeight, unsigned int MDehazeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV14.mem_mode  = 0x0;
    pProcRes->ProcResV14.mem_force = 0x0;
    pProcRes->ProcResV14.round_en  = FUNCTION_ENABLE;
    pProcRes->ProcResV14.range_sima =
        ClipFloatValueV14(pStManu->hw_commom_setting.hw_contrast_ThumbFlt_invVsigma, 0, 8);
    pProcRes->ProcResV14.space_sigma_cur =
        ClipFloatValueV14(pStManu->hw_commom_setting.hw_contrast_curThumbFlt_invRsgm, 0, 8);
    pProcRes->ProcResV14.space_sigma_pre =
        ClipFloatValueV14(pStManu->hw_commom_setting.hw_contrast_preThumbFlt_invRsgm, 0, 8);
    pProcRes->ProcResV14.bf_weight =
        ClipFloatValueV14(pStManu->hw_commom_setting.hw_contrast_thumbMerge_wgt, 0, 8);
    pProcRes->ProcResV14.dc_weitcur =
        ClipFloatValueV14(pStManu->hw_commom_setting.hw_contrast_thumbFlt_curWgt, 0, 8);
    pProcRes->ProcResV14.gain_fuse_alpha = LIMIT_VALUE_UNSIGNED(
        (unsigned short)(pStManu->hw_commom_setting.hw_dehaze_user_gainFuse * (BIT_8_MAX + 1)),
        BIT_8_MAX + 1);
    pProcRes->ProcResV14.stab_fnum =
        ClipIntValueV14(pStManu->hw_commom_setting.sw_contrast_paramTflt_curWgt, 5, 0);
    pProcRes->ProcResV14.iir_pre_wet =
        ClipIntValueV14(pStManu->hw_commom_setting.sw_contrast_thumbTflt_curWgt, 4, 0);
    pProcRes->ProcResV14.gaus_h0 = DEHAZE_GAUS_H4;
    pProcRes->ProcResV14.gaus_h1 = DEHAZE_GAUS_H1;
    pProcRes->ProcResV14.gaus_h2 = DEHAZE_GAUS_H0;

    LOGD_ADEHAZE(
        "%s: hw_contrast_ThumbFlt_invVsigma:%f hw_contrast_curThumbFlt_invRsgm:%f "
        "hw_contrast_preThumbFlt_invRsgm:%f hw_contrast_thumbMerge_wgt:%f "
        "hw_contrast_thumbFlt_curWgt:%f hw_dehaze_user_gainFuse:%f\n",
        __func__, pProcRes->ProcResV14.range_sima / 255.0f,
        pProcRes->ProcResV14.space_sigma_cur / 255.0f,
        pProcRes->ProcResV14.space_sigma_pre / 255.0f, pProcRes->ProcResV14.bf_weight / 255.0f,
        pProcRes->ProcResV14.dc_weitcur / 255.0f, pProcRes->ProcResV14.gain_fuse_alpha / 256.0f);
    LOGV_ADEHAZE(
        "%s: range_sima:%d space_sigma_cur:%d space_sigma_pre:%d bf_weight:%d dc_weitcur:%d "
        "gain_fuse_alpha:%d\n",
        __func__, pProcRes->ProcResV14.range_sima, pProcRes->ProcResV14.space_sigma_cur,
        pProcRes->ProcResV14.space_sigma_pre, pProcRes->ProcResV14.bf_weight,
        pProcRes->ProcResV14.dc_weitcur, pProcRes->ProcResV14.gain_fuse_alpha);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetDehazeParamsV14(mDehazeAttrV14_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                              int rawWidth, int rawHeight, unsigned int MDehazeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV14.air_lc_en =
        pStManu->hw_dehaze_setting.hw_dehaze_luma_mode ? FUNCTION_ENABLE : FUNCTION_DISABLE;

    pProcRes->ProcResV14.dc_min_th =
        ClipIntValueV14(pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkCh_minThed, 8, 0);
    pProcRes->ProcResV14.dc_max_th =
        ClipIntValueV14(pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkCh_maxThed, 8, 0);
    pProcRes->ProcResV14.yhist_th = ClipIntValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_lumaCount_maxThed, 8, 0);
    pProcRes->ProcResV14.yblk_th =
        pStManu->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_lumaCount_minRatio *
        ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16);
    pProcRes->ProcResV14.dark_th =
        ClipIntValueV14(pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkArea_thed, 8, 0);
    pProcRes->ProcResV14.bright_min = ClipIntValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_bright_minLimit, 8, 0);
    pProcRes->ProcResV14.bright_max = ClipIntValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_bright_maxLimit, 8, 0);
    pProcRes->ProcResV14.wt_max = ClipFloatValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_invContrast_scale, 0, 8);
    pProcRes->ProcResV14.air_min = ClipIntValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_airLight_minLimit, 8, 0);
    pProcRes->ProcResV14.air_max = ClipIntValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_airLight_maxLimit, 8, 0);
    pProcRes->ProcResV14.tmax_base =
        ClipIntValueV14(pStManu->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_airLight_scale, 8, 0);
    pProcRes->ProcResV14.tmax_off = ClipFloatValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_transRatio_offset, 0, 10);
    pProcRes->ProcResV14.tmax_max = ClipFloatValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_transRatio_maxLimit, 0, 10);
    pProcRes->ProcResV14.cfg_alpha = LIMIT_VALUE_UNSIGNED(
        pStManu->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_paramMerge_alpha, BIT_8_MAX);
    pProcRes->ProcResV14.cfg_wt = ClipFloatValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_userInvContrast, 0, 8);
    pProcRes->ProcResV14.cfg_air =
        ClipIntValueV14(pStManu->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_userAirLight, 8, 0);
    pProcRes->ProcResV14.cfg_tmax = ClipFloatValueV14(
        pStManu->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_userTransRatio, 0, 10);
    if (pStManu->hw_dehaze_setting.sw_dhaz_invContrastTflt_invSgm >= 0.0f)
        pProcRes->ProcResV14.iir_wt_sigma = LIMIT_VALUE(
            int(1024.0f /
                (8.0f * pStManu->hw_dehaze_setting.sw_dhaz_invContrastTflt_invSgm + 0.5f)),
            0x7ff, 0);
    else
        pProcRes->ProcResV14.iir_wt_sigma = 0x7ff;
    if (pStManu->hw_dehaze_setting.sw_dhaz_airLightTflt_invSgm)
        pProcRes->ProcResV14.iir_air_sigma = LIMIT_VALUE(
            int(1024.0f / pStManu->hw_dehaze_setting.sw_dhaz_airLightTflt_invSgm), 255, 0);
    else
        pProcRes->ProcResV14.iir_air_sigma = 0x8;
    if (pStManu->hw_dehaze_setting.sw_dhaz_transRatioTflt_invSgm)
        pProcRes->ProcResV14.iir_tmax_sigma = LIMIT_VALUE(
            int(1.0f / pStManu->hw_dehaze_setting.sw_dhaz_transRatioTflt_invSgm), 0x7ff, 0);
    else
        pProcRes->ProcResV14.iir_tmax_sigma = 0x5f;

    // add for rk_aiq_uapi2_setMDehazeStrth
    if (MDehazeStrth != DEHAZE_DEFAULT_LEVEL) {
        pProcRes->ProcResV14.cfg_alpha = BIT_8_MAX;
        unsigned int level_diff        = MDehazeStrth > DEHAZE_DEFAULT_LEVEL
                                      ? (MDehazeStrth - DEHAZE_DEFAULT_LEVEL)
                                      : (DEHAZE_DEFAULT_LEVEL - MDehazeStrth);
        bool level_up = MDehazeStrth > DEHAZE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV14.cfg_wt += level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV14.cfg_air += level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV14.cfg_tmax += level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        } else {
            pProcRes->ProcResV14.cfg_wt =
                level_diff * DEHAZE_DEFAULT_CFG_WT_STEP > pProcRes->ProcResV14.cfg_wt
                    ? 0
                    : (pProcRes->ProcResV14.cfg_wt - level_diff * DEHAZE_DEFAULT_CFG_WT_STEP);
            pProcRes->ProcResV14.cfg_air =
                level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP > pProcRes->ProcResV14.cfg_air
                    ? 0
                    : (pProcRes->ProcResV14.cfg_air - level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP);
            pProcRes->ProcResV14.cfg_tmax =
                level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP > pProcRes->ProcResV14.cfg_tmax
                    ? 0
                    : (pProcRes->ProcResV14.cfg_tmax - level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP);
        }
        pProcRes->ProcResV14.cfg_wt = LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_wt, BIT_8_MAX);
        pProcRes->ProcResV14.cfg_tmax =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_tmax, BIT_10_MAX);
    }

    if (pProcRes->ProcResV14.dc_en && !(pProcRes->ProcResV14.enhance_en)) {
        if (pProcRes->ProcResV14.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:1 cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__,
                         pProcRes->ProcResV14.cfg_air / 1.0f,
                         pProcRes->ProcResV14.cfg_tmax / 1023.0f,
                         pProcRes->ProcResV14.cfg_wt / 255.0f);
            LOGV_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n",
                         __func__, pProcRes->ProcResV14.cfg_air, pProcRes->ProcResV14.cfg_tmax,
                         pProcRes->ProcResV14.cfg_wt);
        } else if (pProcRes->ProcResV14.cfg_alpha == 0) {
            LOGD_ADEHAZE("%s cfg_alpha:0 air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__,
                         pProcRes->ProcResV14.air_max / 1.0f, pProcRes->ProcResV14.air_min / 1.0f,
                         pProcRes->ProcResV14.tmax_base / 1.0f,
                         pProcRes->ProcResV14.wt_max / 255.0f);
            LOGV_ADEHAZE(
                "%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n",
                __func__, pProcRes->ProcResV14.air_max, pProcRes->ProcResV14.air_min,
                pProcRes->ProcResV14.tmax_base, pProcRes->ProcResV14.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetEnhanceParamsV14(mDehazeAttrV14_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                               unsigned int MEnhanceStrth, unsigned int MEnhanceChromeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool level_up           = false;
    unsigned int level_diff = 0;

    pProcRes->ProcResV14.enhance_value = ClipFloatValueV14(
        pStManu->hw_enhance_setting.hw_enhance_params.sw_enhance_contrast_strg, 4, 10);
    pProcRes->ProcResV14.enhance_chroma = ClipFloatValueV14(
        pStManu->hw_enhance_setting.hw_enhance_params.sw_enhance_saturate_strg, 4, 10);

    for (int i = 0; i < DHAZ_V14_ENHANCE_CRUVE_NUM; i++)
        pProcRes->ProcResV14.enh_curve[i] =
            (int)(pStManu->hw_enhance_setting.hw_enhance_params.hw_enhance_loLumaConvert_val[i]);

    // dehaze v12 add
    pProcRes->ProcResV14.color_deviate_en = pStManu->hw_enhance_setting.hw_enhance_cProtect_en;
    pProcRes->ProcResV14.enh_luma_en      = pStManu->hw_enhance_setting.hw_enhance_luma2strg_en;
    for (int i = 0; i < DHAZ_V14_ENH_LUMA_NUM; i++)
        pProcRes->ProcResV14.enh_luma[i] = ClipFloatValueV14(
            pStManu->hw_enhance_setting.hw_enhance_params.sw_enhance_luma2strg_val[i], 4, 6);

    // add for rk_aiq_uapi2_setMEnhanceStrth
    if (MEnhanceStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceStrth);
        level_up = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL;
        if (pProcRes->ProcResV14.enh_luma_en) {
            if (level_up) {
                for (int j = 0; j < DHAZ_V14_ENH_LUMA_NUM; j++) {
                    pProcRes->ProcResV14.enh_luma[j] += level_diff * ENH_LUMA_DEFAULT_STEP;
                    pProcRes->ProcResV14.enh_luma[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.enh_luma[j], BIT_10_MAX);
                }
            } else {
                for (int j = 0; j < DHAZ_V14_ENH_LUMA_NUM; j++) {
                    pProcRes->ProcResV14.enh_luma[j] =
                        level_diff * ENH_LUMA_DEFAULT_STEP > pProcRes->ProcResV14.enh_luma[j]
                            ? 0
                            : (pProcRes->ProcResV14.enh_luma[j] -
                               level_diff * ENH_LUMA_DEFAULT_STEP);
                    pProcRes->ProcResV14.enh_luma[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.enh_luma[j], BIT_10_MAX);
                }
            }
        } else {
            if (level_up) {
                pProcRes->ProcResV14.enhance_value += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
            } else {
                pProcRes->ProcResV14.enhance_value =
                    level_diff * ENHANCE_VALUE_DEFAULT_STEP > pProcRes->ProcResV14.enhance_value
                        ? 0
                        : (pProcRes->ProcResV14.enhance_value -
                           level_diff * ENHANCE_VALUE_DEFAULT_STEP);
            }
            pProcRes->ProcResV14.enhance_value =
                LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.enhance_value, BIT_14_MAX);
        }
    }

    // add for rk_aiq_uapi2_setMEnhanceChromeStrth
    if (MEnhanceChromeStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceChromeStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceChromeStrth);
        level_up = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV14.enhance_chroma += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV14.enhance_chroma =
                level_diff * ENHANCE_VALUE_DEFAULT_STEP > pProcRes->ProcResV14.enhance_chroma
                    ? 0
                    : (pProcRes->ProcResV14.enhance_chroma -
                       level_diff * ENHANCE_VALUE_DEFAULT_STEP);
        }
        pProcRes->ProcResV14.enhance_chroma =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.enhance_chroma, BIT_14_MAX);
    }

    if (pProcRes->ProcResV14.dc_en && pProcRes->ProcResV14.enhance_en) {
        LOGD_ADEHAZE(
            "%s hw_enhance_cProtect_en:%d hw_enhance_luma2strg_en:%d sw_enhance_contrast_strg:%f "
            "sw_enhance_saturate_strg:%f\n",
            __func__, pProcRes->ProcResV14.color_deviate_en, pProcRes->ProcResV14.enh_luma_en,
            pStManu->hw_enhance_setting.hw_enhance_params.sw_enhance_contrast_strg,
            pStManu->hw_enhance_setting.hw_enhance_params.sw_enhance_saturate_strg);
        if (pProcRes->ProcResV14.enh_luma_en) {
            LOGD_ADEHAZE(
                "%s enh_luma[0~7]:%f %f %f %f %f %f %f %f\n", __func__,
                pProcRes->ProcResV14.enh_luma[0] / 64.0f, pProcRes->ProcResV14.enh_luma[1] / 64.0f,
                pProcRes->ProcResV14.enh_luma[2] / 64.0f, pProcRes->ProcResV14.enh_luma[3] / 64.0f,
                pProcRes->ProcResV14.enh_luma[4] / 64.0f, pProcRes->ProcResV14.enh_luma[5] / 64.0f,
                pProcRes->ProcResV14.enh_luma[6] / 64.0f, pProcRes->ProcResV14.enh_luma[7] / 64.0f);
        }
        LOGV_ADEHAZE(
            "%s color_deviate_en:%d enh_luma_en:%d enhance_value_reg:0x%x "
            "enhance_chroma_reg:0x%x\n",
            __func__, pProcRes->ProcResV14.color_deviate_en, pProcRes->ProcResV14.enh_luma_en,
            pProcRes->ProcResV14.enhance_value, pProcRes->ProcResV14.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetHistParamsV14(mDehazeAttrV14_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                            dehaze_stats_v14_t* pStats, bool stats_true, int height, int width) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    unsigned int tmp_int = 0;

    // sw_hist_MapTflt_invSigma
    if (pStManu->hw_hist_setting.sw_hist_MapTflt_invSigma)
        pProcRes->ProcResV14.iir_sigma =
            LIMIT_VALUE(int(256.0f / pStManu->hw_hist_setting.sw_hist_MapTflt_invSigma), 255, 0);
    else
        pProcRes->ProcResV14.iir_sigma = 0x1;
    // map_mode
    pProcRes->ProcResV14.map_mode = pStManu->hw_hist_setting.hw_hist_imgMap_mode;
    // thumb_col
    pProcRes->ProcResV14.thumb_col = pStManu->hw_hist_setting.hw_histc_blocks_cols;
    // thumb_row
    pProcRes->ProcResV14.thumb_row = pStManu->hw_hist_setting.hw_histc_blocks_rows;
    // blk_het
    pProcRes->ProcResV14.blk_het = LIMIT_VALUE(height / pProcRes->ProcResV14.thumb_row,
                                               HIST_BLOCK_HEIGHT_MAX, HSIT_BLOCK_HEIGHT_MIN);
    // blk_wid
    pProcRes->ProcResV14.blk_wid = LIMIT_VALUE(width / pProcRes->ProcResV14.thumb_col,
                                               HIST_BLOCK_WIDTH_MAX, HSIT_BLOCK_WIDTH_MIN);
    // hist_th_off
    pProcRes->ProcResV14.hist_th_off =
        ClipIntValueV14(pStManu->hw_hist_setting.hw_hist_params.hw_histc_noiseCount_offset, 8, 0);
    // hist_k
    pProcRes->ProcResV14.hist_k =
        ClipFloatValueV14(pStManu->hw_hist_setting.hw_hist_params.sw_histc_noiseCount_scale, 3, 2);
    // hist_min
    pProcRes->ProcResV14.hist_min =
        ClipFloatValueV14(pStManu->hw_hist_setting.hw_hist_params.sw_histc_countWgt_minLimit, 1, 8);
    // cfg_k
    pProcRes->ProcResV14.cfg_k =
        ClipFloatValueV14(pStManu->hw_hist_setting.hw_hist_params.sw_hist_mapUserSet, 1, 8);
    pProcRes->ProcResV14.cfg_k = LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_k, BIT_8_MAX + 1);
    // cfg_k_alpha
    pProcRes->ProcResV14.cfg_k_alpha =
        ClipFloatValueV14(pStManu->hw_hist_setting.hw_hist_params.sw_hist_mapMerge_alpha, 1, 8);
    pProcRes->ProcResV14.cfg_k_alpha =
        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_k_alpha, BIT_8_MAX + 1);
    // k_gain
    pProcRes->ProcResV14.k_gain =
        ClipFloatValueV14(pStManu->hw_hist_setting.hw_hist_params.sw_hist_mapCount_scale, 3, 8);
    // blend_wet
    for (int j = 0; j < DHAZ_V14_BLEND_WET_NUM; j++) {
        tmp_int = ClipFloatValueV14(
            pStManu->hw_hist_setting.hw_hist_params.sw_dehaze_outputMerge_alpha[j], 1, 8);
        pProcRes->ProcResV14.blend_wet[j] = LIMIT_VALUE_UNSIGNED(tmp_int, BIT_8_MAX + 1);
    }

    /*
XCamReturn ret = XCAM_RETURN_NO_ERROR;
if (pStManu->hw_hist_setting.hist_wr.mode == HIST_WR_MANUAL) {
    ret = TransferHistWr2Res(pProcRes, &pStManu->hw_hist_setting.hist_wr.manual_curve);
    if (ret == XCAM_RETURN_NO_ERROR)
        pProcRes->ProcResV14.soft_wr_en = FUNCTION_ENABLE;
    else
        pProcRes->ProcResV14.soft_wr_en = FUNCTION_DISABLE;
} else if (pStManu->hw_hist_setting.hist_wr.mode == HIST_WR_AUTO) {
    pProcRes->ProcResV14.soft_wr_en = FUNCTION_DISABLE;
} else if (pStManu->hw_hist_setting.hist_wr.mode == HIST_WR_SEMIAUTO) {
    ret = TransferHistWrSemiAtuos2Res(
        pProcRes, &pStManu->hw_hist_setting.hist_wr.semiauto_curve, pStats, stats_true);
    if (ret == XCAM_RETURN_NO_ERROR)
        pProcRes->ProcResV14.soft_wr_en = FUNCTION_ENABLE;
    else
        pProcRes->ProcResV14.soft_wr_en = FUNCTION_DISABLE;
}
*/

    LOGD_ADEHAZE(
        "%s cfg_alpha:%f hw_histc_noiseCount_offset:%f sw_histc_noiseCount_scale:%f "
        "sw_histc_countWgt_minLimit:%f sw_hist_mapMerge_alpha:%f sw_hist_mapUserSet:%f "
        "sw_hist_mapCount_scale:%f \n",
        __func__, pProcRes->ProcResV14.cfg_alpha / 255.0f, pProcRes->ProcResV14.hist_th_off / 1.0f,
        pProcRes->ProcResV14.hist_k / 4.0f, pProcRes->ProcResV14.hist_min / 256.0f,
        pProcRes->ProcResV14.cfg_k_alpha / 256.0f, pProcRes->ProcResV14.cfg_k / 256.0f,
        pProcRes->ProcResV14.k_gain / 255.0f);
    LOGD_ADEHAZE(
        "%s: sw_dehaze_outputMerge_alpha[0~7]:%f %f %f %f %f %f %f %f\n", __func__,
        pProcRes->ProcResV14.blend_wet[0] / 256.0f, pProcRes->ProcResV14.blend_wet[1] / 256.0f,
        pProcRes->ProcResV14.blend_wet[2] / 256.0f, pProcRes->ProcResV14.blend_wet[3] / 256.0f,
        pProcRes->ProcResV14.blend_wet[4] / 256.0f, pProcRes->ProcResV14.blend_wet[5] / 256.0f,
        pProcRes->ProcResV14.blend_wet[6] / 256.0f, pProcRes->ProcResV14.blend_wet[6] / 256.0f);
    LOGV_ADEHAZE(
        "%s cfg_alpha_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x "
        "hist_min_reg:0x%x cfg_k_alpha:0x%x cfg_k:0x%x k_gain:0x%x\n",
        __func__, pProcRes->ProcResV14.cfg_alpha, pProcRes->ProcResV14.hist_th_off,
        pProcRes->ProcResV14.hist_k, pProcRes->ProcResV14.hist_min,
        pProcRes->ProcResV14.cfg_k_alpha, pProcRes->ProcResV14.cfg_k, pProcRes->ProcResV14.k_gain);
    LOGV_ADEHAZE("%s: blend_wet[0~7]:%d %d %d %d %d %d %d %d\n", __func__,
                 pProcRes->ProcResV14.blend_wet[0], pProcRes->ProcResV14.blend_wet[1],
                 pProcRes->ProcResV14.blend_wet[2], pProcRes->ProcResV14.blend_wet[3],
                 pProcRes->ProcResV14.blend_wet[4], pProcRes->ProcResV14.blend_wet[5],
                 pProcRes->ProcResV14.blend_wet[6], pProcRes->ProcResV14.blend_wet[6]);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetCommomParamsV14(CalibDbDehazeV14_t* pCalibV14, RkAiqAdehazeProcResult_t* pProcRes,
                        int rawWidth, int rawHeight, unsigned int MDehazeStrth, float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    int lo = 0, hi = 0;

    pProcRes->ProcResV14.mem_mode  = 0x0;
    pProcRes->ProcResV14.mem_force = 0x0;
    pProcRes->ProcResV14.round_en  = FUNCTION_ENABLE;

    float ratio = GetInterpRatioV14(pCalibV14->hw_commom_setting.iso, lo, hi, CtrlValue,
                                    DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV14.range_sima = ClipFloatValueV14(
        ratio * (pCalibV14->hw_commom_setting.hw_contrast_ThumbFlt_invVsigma[hi] -
                 pCalibV14->hw_commom_setting.hw_contrast_ThumbFlt_invVsigma[lo]) +
            pCalibV14->hw_commom_setting.hw_contrast_ThumbFlt_invVsigma[lo],
        0, 8);
    pProcRes->ProcResV14.space_sigma_cur = ClipFloatValueV14(
        ratio * (pCalibV14->hw_commom_setting.hw_contrast_curThumbFlt_invRsgm[hi] -
                 pCalibV14->hw_commom_setting.hw_contrast_curThumbFlt_invRsgm[lo]) +
            pCalibV14->hw_commom_setting.hw_contrast_curThumbFlt_invRsgm[lo],
        0, 8);
    pProcRes->ProcResV14.space_sigma_pre = ClipFloatValueV14(
        ratio * (pCalibV14->hw_commom_setting.hw_contrast_preThumbFlt_invRsgm[hi] -
                 pCalibV14->hw_commom_setting.hw_contrast_preThumbFlt_invRsgm[lo]) +
            pCalibV14->hw_commom_setting.hw_contrast_preThumbFlt_invRsgm[lo],
        0, 8);
    pProcRes->ProcResV14.bf_weight =
        ClipFloatValueV14(ratio * (pCalibV14->hw_commom_setting.hw_contrast_thumbMerge_wgt[hi] -
                                   pCalibV14->hw_commom_setting.hw_contrast_thumbMerge_wgt[lo]) +
                              pCalibV14->hw_commom_setting.hw_contrast_thumbMerge_wgt[lo],
                          0, 8);
    pProcRes->ProcResV14.dc_weitcur =
        ClipFloatValueV14(ratio * (pCalibV14->hw_commom_setting.hw_contrast_thumbFlt_curWgt[hi] -
                                   pCalibV14->hw_commom_setting.hw_contrast_thumbFlt_curWgt[lo]) +
                              pCalibV14->hw_commom_setting.hw_contrast_thumbFlt_curWgt[lo],
                          0, 8);
    pProcRes->ProcResV14.gain_fuse_alpha = LIMIT_VALUE_UNSIGNED(
        (unsigned short)((ratio * (pCalibV14->hw_commom_setting.hw_dehaze_user_gainFuse[hi] -
                                   pCalibV14->hw_commom_setting.hw_dehaze_user_gainFuse[lo]) +
                          pCalibV14->hw_commom_setting.hw_dehaze_user_gainFuse[lo]) *
                         (BIT_8_MAX + 1)),
        BIT_8_MAX + 1);
    pProcRes->ProcResV14.stab_fnum =
        ClipIntValueV14(ratio * (pCalibV14->hw_commom_setting.sw_contrast_paramTflt_curWgt[hi] -
                                 pCalibV14->hw_commom_setting.sw_contrast_paramTflt_curWgt[lo]) +
                            pCalibV14->hw_commom_setting.sw_contrast_paramTflt_curWgt[lo],
                        5, 0);
    pProcRes->ProcResV14.iir_pre_wet =
        ClipIntValueV14(ratio * (pCalibV14->hw_commom_setting.sw_contrast_thumbTflt_curWgt[hi] -
                                 pCalibV14->hw_commom_setting.sw_contrast_thumbTflt_curWgt[lo]) +
                            pCalibV14->hw_commom_setting.sw_contrast_thumbTflt_curWgt[lo],
                        4, 0);
    pProcRes->ProcResV14.gaus_h0 = DEHAZE_GAUS_H4;
    pProcRes->ProcResV14.gaus_h1 = DEHAZE_GAUS_H1;
    pProcRes->ProcResV14.gaus_h2 = DEHAZE_GAUS_H0;

    LOGD_ADEHAZE(
        "%s: hw_contrast_ThumbFlt_invVsigma:%f hw_contrast_curThumbFlt_invRsgm:%f "
        "hw_contrast_preThumbFlt_invRsgm:%f hw_contrast_thumbMerge_wgt:%f "
        "hw_contrast_thumbFlt_curWgt:%f hw_dehaze_user_gainFuse:%f\n",
        __func__, pProcRes->ProcResV14.range_sima / 255.0f,
        pProcRes->ProcResV14.space_sigma_cur / 255.0f,
        pProcRes->ProcResV14.space_sigma_pre / 255.0f, pProcRes->ProcResV14.bf_weight / 255.0f,
        pProcRes->ProcResV14.dc_weitcur / 255.0f, pProcRes->ProcResV14.gain_fuse_alpha / 256.0f);
    LOGV_ADEHAZE(
        "%s: range_sima:%d space_sigma_cur:%d space_sigma_pre:%d bf_weight:%d dc_weitcur:%d "
        "gain_fuse_alpha:%d\n",
        __func__, pProcRes->ProcResV14.range_sima, pProcRes->ProcResV14.space_sigma_cur,
        pProcRes->ProcResV14.space_sigma_pre, pProcRes->ProcResV14.bf_weight,
        pProcRes->ProcResV14.dc_weitcur, pProcRes->ProcResV14.gain_fuse_alpha);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeParamsV14(CalibDbDehazeV14_t* pCalibV14, RkAiqAdehazeProcResult_t* pProcRes,
                        int rawWidth, int rawHeight, unsigned int MDehazeStrth, float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    int lo = 0, hi = 0;

    pProcRes->ProcResV14.air_lc_en =
        pCalibV14->hw_dehaze_setting.hw_dehaze_luma_mode ? FUNCTION_ENABLE : FUNCTION_DISABLE;

    float ratio = GetInterpRatioV14(pCalibV14->hw_dehaze_setting.hw_dehaze_params.iso, lo, hi,
                                    CtrlValue, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV14.dc_min_th = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkCh_minThed[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkCh_minThed[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkCh_minThed[lo],
        8, 0);
    pProcRes->ProcResV14.dc_max_th = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkCh_maxThed[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkCh_maxThed[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkCh_maxThed[lo],
        8, 0);
    pProcRes->ProcResV14.yhist_th = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_lumaCount_maxThed[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_lumaCount_maxThed[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_lumaCount_maxThed[lo],
        8, 0);
    pProcRes->ProcResV14.yblk_th =
        (ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_lumaCount_minRatio[hi] -
                  pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_lumaCount_minRatio[lo]) +
         pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_lumaCount_minRatio[lo]) *
        ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16);
    pProcRes->ProcResV14.dark_th = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkArea_thed[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkArea_thed[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_darkArea_thed[lo],
        8, 0);
    pProcRes->ProcResV14.bright_min = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_bright_minLimit[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_bright_minLimit[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_bright_minLimit[lo],
        8, 0);
    pProcRes->ProcResV14.bright_max = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_bright_maxLimit[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_bright_maxLimit[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_bright_maxLimit[lo],
        8, 0);
    pProcRes->ProcResV14.wt_max = ClipFloatValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_invContrast_scale[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_invContrast_scale[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_invContrast_scale[lo],
        0, 8);
    pProcRes->ProcResV14.air_min = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_airLight_minLimit[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_airLight_minLimit[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_airLight_minLimit[lo],
        8, 0);
    pProcRes->ProcResV14.air_max = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_airLight_maxLimit[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_airLight_maxLimit[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_airLight_maxLimit[lo],
        8, 0);
    pProcRes->ProcResV14.tmax_base = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_airLight_scale[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_airLight_scale[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_airLight_scale[lo],
        8, 0);
    pProcRes->ProcResV14.tmax_off = ClipFloatValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_transRatio_offset[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_transRatio_offset[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_transRatio_offset[lo],
        0, 10);
    pProcRes->ProcResV14.tmax_max = ClipFloatValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_transRatio_maxLimit[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_transRatio_maxLimit[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_transRatio_maxLimit[lo],
        0, 10);
    pProcRes->ProcResV14.cfg_alpha = LIMIT_VALUE_UNSIGNED(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_paramMerge_alpha[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_paramMerge_alpha[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_paramMerge_alpha[lo],
        BIT_8_MAX);
    pProcRes->ProcResV14.cfg_wt = ClipFloatValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_userInvContrast[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_userInvContrast[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_userInvContrast[lo],
        0, 8);
    pProcRes->ProcResV14.cfg_air = ClipIntValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_userAirLight[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_userAirLight[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.hw_dehaze_userAirLight[lo],
        8, 0);
    pProcRes->ProcResV14.cfg_tmax = ClipFloatValueV14(
        ratio * (pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_userTransRatio[hi] -
                 pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_userTransRatio[lo]) +
            pCalibV14->hw_dehaze_setting.hw_dehaze_params.sw_dehaze_userTransRatio[lo],
        0, 10);
    if (pCalibV14->hw_dehaze_setting.sw_dhaz_invContrastTflt_invSgm >= 0.0f)
        pProcRes->ProcResV14.iir_wt_sigma = LIMIT_VALUE(
            int(1024.0f /
                (8.0f * pCalibV14->hw_dehaze_setting.sw_dhaz_invContrastTflt_invSgm + 0.5f)),
            0x7ff, 0);
    else
        pProcRes->ProcResV14.iir_wt_sigma = 0x7ff;
    if (pCalibV14->hw_dehaze_setting.sw_dhaz_airLightTflt_invSgm)
        pProcRes->ProcResV14.iir_air_sigma = LIMIT_VALUE(
            int(1024.0f / pCalibV14->hw_dehaze_setting.sw_dhaz_airLightTflt_invSgm), 255, 0);
    else
        pProcRes->ProcResV14.iir_air_sigma = 0x8;
    if (pCalibV14->hw_dehaze_setting.sw_dhaz_transRatioTflt_invSgm)
        pProcRes->ProcResV14.iir_tmax_sigma = LIMIT_VALUE(
            int(1.0f / pCalibV14->hw_dehaze_setting.sw_dhaz_transRatioTflt_invSgm), 0x7ff, 0);
    else
        pProcRes->ProcResV14.iir_tmax_sigma = 0x5f;

    // add for rk_aiq_uapi2_setMDehazeStrth
    if (MDehazeStrth != DEHAZE_DEFAULT_LEVEL) {
        pProcRes->ProcResV14.cfg_alpha = BIT_8_MAX;
        unsigned int level_diff        = MDehazeStrth > DEHAZE_DEFAULT_LEVEL
                                      ? (MDehazeStrth - DEHAZE_DEFAULT_LEVEL)
                                      : (DEHAZE_DEFAULT_LEVEL - MDehazeStrth);
        bool level_up = MDehazeStrth > DEHAZE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV14.cfg_wt += level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV14.cfg_air += level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV14.cfg_tmax += level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        } else {
            pProcRes->ProcResV14.cfg_wt =
                level_diff * DEHAZE_DEFAULT_CFG_WT_STEP > pProcRes->ProcResV14.cfg_wt
                    ? 0
                    : (pProcRes->ProcResV14.cfg_wt - level_diff * DEHAZE_DEFAULT_CFG_WT_STEP);
            pProcRes->ProcResV14.cfg_air =
                level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP > pProcRes->ProcResV14.cfg_air
                    ? 0
                    : (pProcRes->ProcResV14.cfg_air - level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP);
            pProcRes->ProcResV14.cfg_tmax =
                level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP > pProcRes->ProcResV14.cfg_tmax
                    ? 0
                    : (pProcRes->ProcResV14.cfg_tmax - level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP);
        }
        pProcRes->ProcResV14.cfg_wt = LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_wt, BIT_8_MAX);
        pProcRes->ProcResV14.cfg_tmax =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_tmax, BIT_10_MAX);
    }

    if (pProcRes->ProcResV14.dc_en && !(pProcRes->ProcResV14.enhance_en)) {
        if (pProcRes->ProcResV14.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:1 cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__,
                         pProcRes->ProcResV14.cfg_air / 1.0f,
                         pProcRes->ProcResV14.cfg_tmax / 1023.0f,
                         pProcRes->ProcResV14.cfg_wt / 255.0f);
            LOGV_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n",
                         __func__, pProcRes->ProcResV14.cfg_air, pProcRes->ProcResV14.cfg_tmax,
                         pProcRes->ProcResV14.cfg_wt);
        } else if (pProcRes->ProcResV14.cfg_alpha == 0) {
            LOGD_ADEHAZE("%s cfg_alpha:0 air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__,
                         pProcRes->ProcResV14.air_max / 1.0f, pProcRes->ProcResV14.air_min / 1.0f,
                         pProcRes->ProcResV14.tmax_base / 1.0f,
                         pProcRes->ProcResV14.wt_max / 255.0f);
            LOGV_ADEHAZE(
                "%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n",
                __func__, pProcRes->ProcResV14.air_max, pProcRes->ProcResV14.air_min,
                pProcRes->ProcResV14.tmax_base, pProcRes->ProcResV14.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetEnhanceParamsV14(CalibDbDehazeV14_t* pCalibV14, RkAiqAdehazeProcResult_t* pProcRes,
                         unsigned int MEnhanceStrth, unsigned int MEnhanceChromeStrth,
                         float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool level_up           = false;
    unsigned int level_diff = 0;
    float tmp_float         = 0.0f;
    float ratio             = 1.0;

    // color_deviate_en
    pProcRes->ProcResV14.color_deviate_en =
        pCalibV14->hw_enhance_setting.hw_enhance_cProtect_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    // enh_luma_en
    pProcRes->ProcResV14.enh_luma_en =
        pCalibV14->hw_enhance_setting.hw_enhance_luma2strg_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    if (CtrlValue < pCalibV14->hw_enhance_setting.hw_enhance_params[0].iso) {
        // enhance_value
        pProcRes->ProcResV14.enhance_value = ClipFloatValueV14(
            pCalibV14->hw_enhance_setting.hw_enhance_params[0].sw_enhance_contrast_strg, 4, 10);
        // enhance_chroma
        pProcRes->ProcResV14.enhance_chroma = ClipFloatValueV14(
            pCalibV14->hw_enhance_setting.hw_enhance_params[0].sw_enhance_saturate_strg, 4, 10);
        // enh_curve
        for (int j = 0; j < DHAZ_V14_ENHANCE_CRUVE_NUM; j++)
            pProcRes->ProcResV14.enh_curve[j] =
                (int)(pCalibV14->hw_enhance_setting.hw_enhance_params[0]
                          .hw_enhance_loLumaConvert_val[j]);
        // enh_luma
        for (int j = 0; j < DHAZ_V14_ENH_LUMA_NUM; j++)
            pProcRes->ProcResV14.enh_luma[j] = ClipFloatValueV14(
                pCalibV14->hw_enhance_setting.hw_enhance_params[0].sw_enhance_luma2strg_val[j], 4,
                6);
    } else if (CtrlValue >= pCalibV14->hw_enhance_setting.hw_enhance_params[12].iso) {
        // enhance_value
        pProcRes->ProcResV14.enhance_value = ClipFloatValueV14(
            pCalibV14->hw_enhance_setting.hw_enhance_params[12].sw_enhance_contrast_strg, 4, 10);
        // enhance_chroma
        pProcRes->ProcResV14.enhance_chroma = ClipFloatValueV14(
            pCalibV14->hw_enhance_setting.hw_enhance_params[12].sw_enhance_saturate_strg, 4, 10);
        // enh_curve
        for (int j = 0; j < DHAZ_V14_ENHANCE_CRUVE_NUM; j++)
            pProcRes->ProcResV14.enh_curve[j] =
                (int)(pCalibV14->hw_enhance_setting.hw_enhance_params[12]
                          .hw_enhance_loLumaConvert_val[j]);
        // enh_luma
        for (int j = 0; j < DHAZ_V14_ENH_LUMA_NUM; j++)
            pProcRes->ProcResV14.enh_luma[j] = ClipFloatValueV14(
                pCalibV14->hw_enhance_setting.hw_enhance_params[12].sw_enhance_luma2strg_val[j], 4,
                6);
    } else {
        for (int i = 0; i < DHAZ_CTRL_DATA_STEP_MAX; i++) {
            if (CtrlValue >= pCalibV14->hw_enhance_setting.hw_enhance_params[i].iso &&
                CtrlValue < pCalibV14->hw_enhance_setting.hw_enhance_params[i + 1].iso) {
                if ((pCalibV14->hw_enhance_setting.hw_enhance_params[i + 1].iso -
                     pCalibV14->hw_enhance_setting.hw_enhance_params[i].iso) != 0)
                    ratio = (CtrlValue - pCalibV14->hw_enhance_setting.hw_enhance_params[i].iso) /
                            (pCalibV14->hw_enhance_setting.hw_enhance_params[i + 1].iso -
                             pCalibV14->hw_enhance_setting.hw_enhance_params[i].iso);
                else
                    LOGE_ADEHAZE("Dehaze zero in %s(%d) \n", __func__, __LINE__);
                // enhance_value
                tmp_float =
                    ratio * (pCalibV14->hw_enhance_setting.hw_enhance_params[i + 1]
                                 .sw_enhance_contrast_strg -
                             pCalibV14->hw_enhance_setting.hw_enhance_params[i]
                                 .sw_enhance_contrast_strg) +
                    pCalibV14->hw_enhance_setting.hw_enhance_params[i].sw_enhance_contrast_strg;
                pProcRes->ProcResV14.enhance_value = ClipFloatValueV14(tmp_float, 4, 10);
                // enhance_chroma
                tmp_float =
                    ratio * (pCalibV14->hw_enhance_setting.hw_enhance_params[i + 1]
                                 .sw_enhance_saturate_strg -
                             pCalibV14->hw_enhance_setting.hw_enhance_params[i]
                                 .sw_enhance_saturate_strg) +
                    pCalibV14->hw_enhance_setting.hw_enhance_params[i].sw_enhance_saturate_strg;
                pProcRes->ProcResV14.enhance_chroma = ClipFloatValueV14(tmp_float, 4, 10);
                // enh_curve
                for (int j = 0; j < DHAZ_V14_ENHANCE_CRUVE_NUM; j++) {
                    pProcRes->ProcResV14.enh_curve[j] =
                        ratio * (pCalibV14->hw_enhance_setting.hw_enhance_params[i + 1]
                                     .hw_enhance_loLumaConvert_val[j] -
                                 pCalibV14->hw_enhance_setting.hw_enhance_params[i]
                                     .hw_enhance_loLumaConvert_val[j]) +
                        pCalibV14->hw_enhance_setting.hw_enhance_params[i]
                            .hw_enhance_loLumaConvert_val[j];
                }
                // enh_luma
                for (int j = 0; j < DHAZ_V14_ENH_LUMA_NUM; j++) {
                    tmp_float = ratio * (pCalibV14->hw_enhance_setting.hw_enhance_params[i + 1]
                                             .sw_enhance_luma2strg_val[j] -
                                         pCalibV14->hw_enhance_setting.hw_enhance_params[i]
                                             .sw_enhance_luma2strg_val[j]) +
                                pCalibV14->hw_enhance_setting.hw_enhance_params[i]
                                    .sw_enhance_luma2strg_val[j];
                    pProcRes->ProcResV14.enh_luma[j] = ClipFloatValueV14(tmp_float, 4, 6);
                }
                break;
            } else
                continue;
        }
    }

    // add for rk_aiq_uapi2_setMEnhanceStrth
    if (MEnhanceStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceStrth);
        level_up = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL;
        if (pProcRes->ProcResV14.enh_luma_en) {
            if (level_up) {
                for (int j = 0; j < DHAZ_V14_ENH_LUMA_NUM; j++) {
                    pProcRes->ProcResV14.enh_luma[j] += level_diff * ENH_LUMA_DEFAULT_STEP;
                    pProcRes->ProcResV14.enh_luma[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.enh_luma[j], BIT_10_MAX);
                }
            } else {
                for (int j = 0; j < DHAZ_V14_ENH_LUMA_NUM; j++) {
                    pProcRes->ProcResV14.enh_luma[j] =
                        level_diff * ENH_LUMA_DEFAULT_STEP > pProcRes->ProcResV14.enh_luma[j]
                            ? 0
                            : (pProcRes->ProcResV14.enh_luma[j] -
                               level_diff * ENH_LUMA_DEFAULT_STEP);
                    pProcRes->ProcResV14.enh_luma[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.enh_luma[j], BIT_10_MAX);
                }
            }
        } else {
            if (level_up) {
                pProcRes->ProcResV14.enhance_value += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
            } else {
                pProcRes->ProcResV14.enhance_value =
                    level_diff * ENHANCE_VALUE_DEFAULT_STEP > pProcRes->ProcResV14.enhance_value
                        ? 0
                        : (pProcRes->ProcResV14.enhance_value -
                           level_diff * ENHANCE_VALUE_DEFAULT_STEP);
            }
            pProcRes->ProcResV14.enhance_value =
                LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.enhance_value, BIT_14_MAX);
        }
    }

    // add for rk_aiq_uapi2_setMEnhanceChromeStrth
    if (MEnhanceChromeStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceChromeStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceChromeStrth);
        level_up = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV14.enhance_chroma += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV14.enhance_chroma =
                level_diff * ENHANCE_VALUE_DEFAULT_STEP > pProcRes->ProcResV14.enhance_chroma
                    ? 0
                    : (pProcRes->ProcResV14.enhance_chroma -
                       level_diff * ENHANCE_VALUE_DEFAULT_STEP);
        }
        pProcRes->ProcResV14.enhance_chroma =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.enhance_chroma, BIT_14_MAX);
    }

    if (pProcRes->ProcResV14.dc_en && pProcRes->ProcResV14.enhance_en) {
        LOGD_ADEHAZE(
            "%s hw_enhance_cProtect_en:%d hw_enhance_luma2strg_en:%d sw_enhance_contrast_strg:%f "
            "sw_enhance_saturate_strg:%f\n",
            __func__, pProcRes->ProcResV14.color_deviate_en, pProcRes->ProcResV14.enh_luma_en,
            pProcRes->ProcResV14.enhance_value / 1024.0f,
            pProcRes->ProcResV14.enhance_chroma / 1024.0f);
        if (pProcRes->ProcResV14.enh_luma_en) {
            LOGD_ADEHAZE(
                "%s hw_enhance_loLumaConvert_val[0~7]:%f %f %f %f %f %f %f %f\n", __func__,
                pProcRes->ProcResV14.enh_luma[0] / 64.0f, pProcRes->ProcResV14.enh_luma[1] / 64.0f,
                pProcRes->ProcResV14.enh_luma[2] / 64.0f, pProcRes->ProcResV14.enh_luma[3] / 64.0f,
                pProcRes->ProcResV14.enh_luma[4] / 64.0f, pProcRes->ProcResV14.enh_luma[5] / 64.0f,
                pProcRes->ProcResV14.enh_luma[6] / 64.0f, pProcRes->ProcResV14.enh_luma[7] / 64.0f);
        }
        LOGV_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     pProcRes->ProcResV14.enhance_value, pProcRes->ProcResV14.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetHistParamsV14(CalibDbDehazeV14_t* pCalibV14, RkAiqAdehazeProcResult_t* pProcRes,
                      dehaze_stats_v14_t* pStats, bool stats_true, int height, int width,
                      float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    unsigned int tmp_int = 0;

    // sw_hist_MapTflt_invSigma
    if (pCalibV14->hw_hist_setting.sw_hist_MapTflt_invSigma)
        pProcRes->ProcResV14.iir_sigma =
            LIMIT_VALUE(int(256.0f / pCalibV14->hw_hist_setting.sw_hist_MapTflt_invSigma), 255, 0);
    else
        pProcRes->ProcResV14.iir_sigma = 0x1;
    // map_mode
    pProcRes->ProcResV14.map_mode = pCalibV14->hw_hist_setting.hw_hist_imgMap_mode;
    // thumb_col
    pProcRes->ProcResV14.thumb_col = pCalibV14->hw_hist_setting.hw_histc_blocks_cols;
    // thumb_row
    pProcRes->ProcResV14.thumb_row = pCalibV14->hw_hist_setting.hw_histc_blocks_rows;
    // blk_het
    pProcRes->ProcResV14.blk_het = LIMIT_VALUE(height / pProcRes->ProcResV14.thumb_row,
                                               HIST_BLOCK_HEIGHT_MAX, HSIT_BLOCK_HEIGHT_MIN);
    // blk_wid
    pProcRes->ProcResV14.blk_wid = LIMIT_VALUE(width / pProcRes->ProcResV14.thumb_col,
                                               HIST_BLOCK_WIDTH_MAX, HSIT_BLOCK_WIDTH_MIN);

    if (CtrlValue < pCalibV14->hw_hist_setting.hw_hist_params[0].iso) {
        // hist_th_off
        pProcRes->ProcResV14.hist_th_off = ClipIntValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[0].hw_histc_noiseCount_offset, 8, 0);
        // hist_k
        pProcRes->ProcResV14.hist_k = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[0].sw_histc_noiseCount_scale, 3, 2);
        // hist_min
        pProcRes->ProcResV14.hist_min = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[0].sw_histc_countWgt_minLimit, 1, 8);
        // cfg_k
        pProcRes->ProcResV14.cfg_k = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[0].sw_hist_mapUserSet, 1, 8);
        pProcRes->ProcResV14.cfg_k =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_k, BIT_8_MAX + 1);
        // cfg_k_alpha
        pProcRes->ProcResV14.cfg_k_alpha = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[0].sw_hist_mapMerge_alpha, 1, 8);
        pProcRes->ProcResV14.cfg_k_alpha =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_k_alpha, BIT_8_MAX + 1);
        // k_gain
        pProcRes->ProcResV14.k_gain = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[0].sw_hist_mapCount_scale, 3, 8);
        // blend_wet
        for (int j = 0; j < DHAZ_V14_BLEND_WET_NUM; j++) {
            tmp_int = ClipFloatValueV14(
                pCalibV14->hw_hist_setting.hw_hist_params[0].sw_dehaze_outputMerge_alpha[j], 1, 8);
            pProcRes->ProcResV14.blend_wet[j] = LIMIT_VALUE_UNSIGNED(tmp_int, BIT_8_MAX + 1);
        }
    } else if (CtrlValue >= pCalibV14->hw_hist_setting.hw_hist_params[12].iso) {
        // hist_th_off
        pProcRes->ProcResV14.hist_th_off = ClipIntValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[12].hw_histc_noiseCount_offset, 8, 0);
        // hist_k
        pProcRes->ProcResV14.hist_k = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[12].sw_histc_noiseCount_scale, 3, 2);
        // hist_min
        pProcRes->ProcResV14.hist_min = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[12].sw_histc_countWgt_minLimit, 1, 8);
        // cfg_k
        pProcRes->ProcResV14.cfg_k = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[12].sw_hist_mapUserSet, 1, 8);
        pProcRes->ProcResV14.cfg_k =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_k, BIT_8_MAX + 1);
        // cfg_k_alpha
        pProcRes->ProcResV14.cfg_k_alpha = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[12].sw_hist_mapMerge_alpha, 1, 8);
        pProcRes->ProcResV14.cfg_k_alpha =
            LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.cfg_k_alpha, BIT_8_MAX + 1);
        // k_gain
        pProcRes->ProcResV14.k_gain = ClipFloatValueV14(
            pCalibV14->hw_hist_setting.hw_hist_params[12].sw_hist_mapCount_scale, 3, 8);
        // blend_wet
        for (int j = 0; j < DHAZ_V14_BLEND_WET_NUM; j++) {
            tmp_int = ClipFloatValueV14(
                pCalibV14->hw_hist_setting.hw_hist_params[12].sw_dehaze_outputMerge_alpha[j], 1, 8);
            pProcRes->ProcResV14.blend_wet[j] = LIMIT_VALUE_UNSIGNED(tmp_int, BIT_8_MAX + 1);
        }
    } else {
        float tmp_float = 0.0f;
        float ratio     = 1.0f;
        for (int i = 0; i < DHAZ_CTRL_DATA_STEP_MAX; i++) {
            if (CtrlValue >= pCalibV14->hw_hist_setting.hw_hist_params[i].iso &&
                CtrlValue < pCalibV14->hw_hist_setting.hw_hist_params[i + 1].iso) {
                if ((pCalibV14->hw_hist_setting.hw_hist_params[i + 1].iso -
                     pCalibV14->hw_hist_setting.hw_hist_params[i].iso) != 0)
                    ratio = (CtrlValue - pCalibV14->hw_hist_setting.hw_hist_params[i].iso) /
                            (pCalibV14->hw_hist_setting.hw_hist_params[i + 1].iso -
                             pCalibV14->hw_hist_setting.hw_hist_params[i].iso);
                else
                    LOGE_ADEHAZE("Dehaze zero in %s(%d) \n", __func__, __LINE__);
                // hist_th_off
                tmp_int =
                    ratio *
                        (pCalibV14->hw_hist_setting.hw_hist_params[i + 1]
                             .hw_histc_noiseCount_offset -
                         pCalibV14->hw_hist_setting.hw_hist_params[i].hw_histc_noiseCount_offset) +
                    pCalibV14->hw_hist_setting.hw_hist_params[i].hw_histc_noiseCount_offset;
                pProcRes->ProcResV14.hist_th_off = ClipIntValueV14(tmp_int, 8, 0);
                // hist_k
                tmp_float =
                    ratio *
                        (pCalibV14->hw_hist_setting.hw_hist_params[i + 1]
                             .sw_histc_noiseCount_scale -
                         pCalibV14->hw_hist_setting.hw_hist_params[i].sw_histc_noiseCount_scale) +
                    pCalibV14->hw_hist_setting.hw_hist_params[i].sw_histc_noiseCount_scale;
                pProcRes->ProcResV14.hist_k = ClipFloatValueV14(tmp_float, 3, 2);
                // hist_min
                tmp_float =
                    ratio *
                        (pCalibV14->hw_hist_setting.hw_hist_params[i + 1]
                             .sw_histc_countWgt_minLimit -
                         pCalibV14->hw_hist_setting.hw_hist_params[i].sw_histc_countWgt_minLimit) +
                    pCalibV14->hw_hist_setting.hw_hist_params[i].sw_histc_countWgt_minLimit;
                pProcRes->ProcResV14.hist_min = ClipFloatValueV14(tmp_float, 1, 8);
                // cfg_k
                tmp_float =
                    ratio * (pCalibV14->hw_hist_setting.hw_hist_params[i + 1].sw_hist_mapUserSet -
                             pCalibV14->hw_hist_setting.hw_hist_params[i].sw_hist_mapUserSet) +
                    pCalibV14->hw_hist_setting.hw_hist_params[i].sw_hist_mapUserSet;
                pProcRes->ProcResV14.cfg_k = ClipFloatValueV14(tmp_float, 1, 8);
                // cfg_k_alpha
                tmp_float =
                    ratio *
                        (pCalibV14->hw_hist_setting.hw_hist_params[i + 1].sw_hist_mapMerge_alpha -
                         pCalibV14->hw_hist_setting.hw_hist_params[i].sw_hist_mapMerge_alpha) +
                    pCalibV14->hw_hist_setting.hw_hist_params[i].sw_hist_mapMerge_alpha;
                pProcRes->ProcResV14.cfg_k_alpha = ClipFloatValueV14(tmp_float, 1, 8);
                // k_gain
                tmp_float =
                    ratio *
                        (pCalibV14->hw_hist_setting.hw_hist_params[i + 1].sw_hist_mapCount_scale -
                         pCalibV14->hw_hist_setting.hw_hist_params[i].sw_hist_mapCount_scale) +
                    pCalibV14->hw_hist_setting.hw_hist_params[i].sw_hist_mapCount_scale;
                pProcRes->ProcResV14.k_gain = ClipFloatValueV14(tmp_float, 3, 8);
                // blend_wet
                for (int j = 0; j < DHAZ_V14_BLEND_WET_NUM; j++) {
                    tmp_float =
                        ratio * (pCalibV14->hw_hist_setting.hw_hist_params[i + 1]
                                     .sw_dehaze_outputMerge_alpha[j] -
                                 pCalibV14->hw_hist_setting.hw_hist_params[i]
                                     .sw_dehaze_outputMerge_alpha[j]) +
                        pCalibV14->hw_hist_setting.hw_hist_params[i].sw_dehaze_outputMerge_alpha[j];
                    pProcRes->ProcResV14.blend_wet[j] = ClipFloatValueV14(tmp_float, 1, 8);
                    pProcRes->ProcResV14.blend_wet[j] =
                        LIMIT_VALUE_UNSIGNED(pProcRes->ProcResV14.blend_wet[j], BIT_8_MAX + 1);
                }
                break;
            } else
                continue;
        }
    }

    /*
XCamReturn ret = XCAM_RETURN_NO_ERROR;
if (pCalibV14->hw_hist_setting.hist_wr.mode == HIST_WR_MANUAL) {
    mManual_curve_t Curve;
    DehazeHistWrTableInterpV14(&pCalibV14->hw_hist_setting.hist_wr, &Curve, CtrlValue);
    ret = TransferHistWr2Res(pProcRes, &Curve);

    if (ret == XCAM_RETURN_NO_ERROR)
        pProcRes->ProcResV14.soft_wr_en = FUNCTION_ENABLE;
    else
        pProcRes->ProcResV14.soft_wr_en = FUNCTION_DISABLE;
} else if (pCalibV14->hw_hist_setting.hist_wr.mode == HIST_WR_AUTO) {
    pProcRes->ProcResV14.soft_wr_en = FUNCTION_DISABLE;
} else if (pCalibV14->hw_hist_setting.hist_wr.mode == HIST_WR_SEMIAUTO) {
    mhist_wr_semiauto_t semi_auto_curve;
    semi_auto_curve.clim0 =
        LinearInterpV14(pCalibV14->hw_hist_setting.hist_wr.semiauto_curve.iso,
                        pCalibV14->hw_hist_setting.hist_wr.semiauto_curve.clim0, CtrlValue,
                        DHAZ_CTRL_DATA_STEP_MAX);
    semi_auto_curve.clim1 =
        LinearInterpV14(pCalibV14->hw_hist_setting.hist_wr.semiauto_curve.iso,
                        pCalibV14->hw_hist_setting.hist_wr.semiauto_curve.clim1, CtrlValue,
                        DHAZ_CTRL_DATA_STEP_MAX);
    semi_auto_curve.dark_th =
        LinearInterpV14(pCalibV14->hw_hist_setting.hist_wr.semiauto_curve.iso,
                        pCalibV14->hw_hist_setting.hist_wr.semiauto_curve.dark_th, CtrlValue,
                        DHAZ_CTRL_DATA_STEP_MAX);
    ret = TransferHistWrSemiAtuos2Res(pProcRes, &semi_auto_curve, pStats, stats_true);

    if (ret == XCAM_RETURN_NO_ERROR)
        pProcRes->ProcResV14.soft_wr_en = FUNCTION_ENABLE;
    else
        pProcRes->ProcResV14.soft_wr_en = FUNCTION_DISABLE;
}
*/

    LOGD_ADEHAZE(
        "%s: cfg_alpha:%f hw_histc_noiseCount_offset:%f sw_histc_noiseCount_scale:%f "
        "sw_histc_countWgt_minLimit:%f sw_hist_mapMerge_alpha:%f sw_hist_mapUserSet:%f "
        "sw_hist_mapCount_scale:%f \n",
        __func__, pProcRes->ProcResV14.cfg_alpha / 255.0f, pProcRes->ProcResV14.hist_th_off / 1.0f,
        pProcRes->ProcResV14.hist_k / 4.0f, pProcRes->ProcResV14.hist_min / 256.0f,
        pProcRes->ProcResV14.cfg_k_alpha / 256.0f, pProcRes->ProcResV14.cfg_k / 256.0f,
        pProcRes->ProcResV14.k_gain / 255.0f);
    LOGD_ADEHAZE(
        "%s: sw_dehaze_outputMerge_alpha[0~7]:%f %f %f %f %f %f %f %f\n", __func__,
        pProcRes->ProcResV14.blend_wet[0] / 256.0f, pProcRes->ProcResV14.blend_wet[1] / 256.0f,
        pProcRes->ProcResV14.blend_wet[2] / 256.0f, pProcRes->ProcResV14.blend_wet[3] / 256.0f,
        pProcRes->ProcResV14.blend_wet[4] / 256.0f, pProcRes->ProcResV14.blend_wet[5] / 256.0f,
        pProcRes->ProcResV14.blend_wet[6] / 256.0f, pProcRes->ProcResV14.blend_wet[6] / 256.0f);
    LOGV_ADEHAZE(
        "%s: cfg_alpha_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x "
        "hist_min_reg:0x%x cfg_k_alpha:0x%x cfg_k:0x%x k_gain:0x%x\n",
        __func__, pProcRes->ProcResV14.cfg_alpha, pProcRes->ProcResV14.hist_th_off,
        pProcRes->ProcResV14.hist_k, pProcRes->ProcResV14.hist_min,
        pProcRes->ProcResV14.cfg_k_alpha, pProcRes->ProcResV14.cfg_k, pProcRes->ProcResV14.k_gain);
    LOGV_ADEHAZE("%s: blend_wet[0~7]:%d %d %d %d %d %d %d %d\n", __func__,
                 pProcRes->ProcResV14.blend_wet[0], pProcRes->ProcResV14.blend_wet[1],
                 pProcRes->ProcResV14.blend_wet[2], pProcRes->ProcResV14.blend_wet[3],
                 pProcRes->ProcResV14.blend_wet[4], pProcRes->ProcResV14.blend_wet[5],
                 pProcRes->ProcResV14.blend_wet[6], pProcRes->ProcResV14.blend_wet[6]);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

XCamReturn GetDehazeLocalGainSettingV14(RkAiqAdehazeProcResult_t* pProcRes, float* sigma) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // get sigma_idx
    for (int i = 0; i < DHAZ_V14_SIGMA_IDX_NUM; i++)
        pProcRes->ProcResV14.sigma_idx[i] = (i + 1) * YNR_CURVE_STEP;

    // get sigma_lut
    float sigam_total = 0.0f;
    for (int i = 0; i < DHAZ_V14_SIGMA_IDX_NUM; i++) sigam_total += sigma[i];

    if (sigam_total < FLT_EPSILON) {
        for (int i = 0; i < DHAZ_V14_SIGMA_LUT_NUM; i++) pProcRes->ProcResV14.sigma_lut[i] = 0x200;
    } else {
        int tmp = 0;
        for (int i = 0; i < DHAZ_V14_SIGMA_LUT_NUM; i++) {
            tmp                               = LIMIT_VALUE(8.0f * sigma[i], BIT_10_MAX, BIT_MIN);
            pProcRes->ProcResV14.sigma_lut[i] = tmp;
        }
    }

    LOGV_ADEHAZE("%s(%d) ynr sigma(0~5): %f %f %f %f %f %f\n", __func__, __LINE__, sigma[0],
                 sigma[1], sigma[2], sigma[3], sigma[4], sigma[5]);
    LOGV_ADEHAZE("%s(%d) dehaze local gain IDX(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__,
                 __LINE__, pProcRes->ProcResV14.sigma_idx[0], pProcRes->ProcResV14.sigma_idx[1],
                 pProcRes->ProcResV14.sigma_idx[2], pProcRes->ProcResV14.sigma_idx[3],
                 pProcRes->ProcResV14.sigma_idx[4], pProcRes->ProcResV14.sigma_idx[5]);
    LOGV_ADEHAZE("%s(%d) dehaze local gain LUT(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__,
                 __LINE__, pProcRes->ProcResV14.sigma_lut[0], pProcRes->ProcResV14.sigma_lut[1],
                 pProcRes->ProcResV14.sigma_lut[2], pProcRes->ProcResV14.sigma_lut[3],
                 pProcRes->ProcResV14.sigma_lut[4], pProcRes->ProcResV14.sigma_lut[5]);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

#ifdef RKAIQ_ENABLE_CAMGROUP
XCamReturn AdehazeGetCurrDataGroup(AdehazeHandle_t* pAdehazeCtx,
                                   rk_aiq_singlecam_3a_result_t* pCamgrpParams) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // get ynr res
    for (int i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++)
        pAdehazeCtx->YnrProcResV24_sigma[i] = pCamgrpParams->aynr_sigma._aynr_sigma_v24[i];

    if (pCamgrpParams) {
        // get EnvLv
        if (pCamgrpParams->aec._aePreRes) {
            RkAiqAlgoPreResAe* pAEPreRes =
                (RkAiqAlgoPreResAe*)pCamgrpParams->aec._aePreRes->map(pCamgrpParams->aec._aePreRes);

            if (pAEPreRes) {
                switch (pAdehazeCtx->FrameNumber) {
                    case LINEAR_NUM:
                        pAdehazeCtx->CurrDataV14.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
                        break;
                    case HDR_2X_NUM:
                        pAdehazeCtx->CurrDataV14.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                        break;
                    case HDR_3X_NUM:
                        pAdehazeCtx->CurrDataV14.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                        break;
                    default:
                        LOGE_ADEHAZE("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
                        break;
                }

                // Normalize the current envLv for AEC
                pAdehazeCtx->CurrDataV14.EnvLv =
                    (pAdehazeCtx->CurrDataV14.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
                pAdehazeCtx->CurrDataV14.EnvLv =
                    LIMIT_VALUE(pAdehazeCtx->CurrDataV14.EnvLv, ENVLVMAX, ENVLVMIN);
            } else {
                pAdehazeCtx->CurrDataV14.EnvLv = ENVLVMIN;
                LOGW_ADEHAZE("%s:_aePreRes Res is NULL!\n", __FUNCTION__);
            }
        } else {
            pAdehazeCtx->CurrDataV14.EnvLv = ENVLVMIN;
            LOGW_ADEHAZE("%s:_aePreRes Res is NULL!\n", __FUNCTION__);
        }

        // get iso
        if (pAdehazeCtx->FrameNumber == LINEAR_NUM) {
            pAdehazeCtx->CurrDataV14.ISO =
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain * ISOMIN;
            // ablcV32_proc_res
            if (pCamgrpParams->ablc._blcConfig_v32) {
                if (pCamgrpParams->ablc._blcConfig_v32->blc_ob_enable) {
                    if (pCamgrpParams->ablc._blcConfig_v32->isp_ob_predgain >= ISP_PREDGAIN_DEFAULT)
                        pAdehazeCtx->CurrDataV14.ISO *=
                            pCamgrpParams->ablc._blcConfig_v32->isp_ob_predgain;
                    else
                        LOGE_ADEHAZE("%s:BLC ob is ON, and predgain is %f!\n", __FUNCTION__,
                                     pCamgrpParams->ablc._blcConfig_v32->isp_ob_predgain);
                }
            } else {
                LOGW_ADEHAZE("%s: ABLC Res is NULL!\n", __FUNCTION__);
            }
        } else if (pAdehazeCtx->FrameNumber == HDR_2X_NUM ||
                   pAdehazeCtx->FrameNumber == HDR_3X_NUM) {
            pAdehazeCtx->CurrDataV14.ISO =
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.analog_gain *
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.digital_gain *
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else {
        pAdehazeCtx->CurrDataV14.EnvLv = ENVLVMIN;
        pAdehazeCtx->CurrDataV14.ISO   = ISOMIN;
        LOGW_ADEHAZE("%s: camgroupParmasArray[0] Res is NULL!\n", __FUNCTION__);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
    return ret;
}
#endif
XCamReturn AdehazeGetCurrData(AdehazeHandle_t* pAdehazeCtx, RkAiqAlgoProcAdhaz* pProcPara) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // get ynr res
    for (int i = 0; i < YNR_V24_ISO_CURVE_POINT_NUM; i++)
        pAdehazeCtx->YnrProcResV24_sigma[i] = pProcPara->sigma_v24[i];

    // get EnvLv
    XCamVideoBuffer* xCamAePreRes = pProcPara->com.u.proc.res_comb->ae_pre_res;
    if (xCamAePreRes) {
        RkAiqAlgoPreResAe* pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);

        if (pAEPreRes) {
            switch (pAdehazeCtx->FrameNumber) {
                case LINEAR_NUM:
                    pAdehazeCtx->CurrDataV14.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
                    break;
                case HDR_2X_NUM:
                    pAdehazeCtx->CurrDataV14.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                    break;
                case HDR_3X_NUM:
                    pAdehazeCtx->CurrDataV14.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                    break;
                default:
                    LOGE_ADEHAZE("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
                    break;
            }
            // Normalize the current envLv for AEC
            pAdehazeCtx->CurrDataV14.EnvLv =
                (pAdehazeCtx->CurrDataV14.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
            pAdehazeCtx->CurrDataV14.EnvLv =
                LIMIT_VALUE(pAdehazeCtx->CurrDataV14.EnvLv, ENVLVMAX, ENVLVMIN);

        } else {
            pAdehazeCtx->CurrDataV14.EnvLv = ENVLVMIN;
            LOGW_ADEHAZE("%s:PreResBuf is NULL!\n", __FUNCTION__);
        }
    } else {
        pAdehazeCtx->CurrDataV14.EnvLv = ENVLVMIN;
        LOGW_ADEHAZE("%s:PreResBuf is NULL!\n", __FUNCTION__);
    }

    // get ISO
    if (pProcPara->com.u.proc.curExp) {
        if (pAdehazeCtx->FrameNumber == LINEAR_NUM) {
            pAdehazeCtx->CurrDataV14.ISO =
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;

            if (pProcPara->OBResV12.blc_ob_enable &&
                pProcPara->OBResV12.isp_ob_predgain < ISP_PREDGAIN_DEFAULT) {
                LOGE_ADEHAZE("%s: ob_enable ON , and ob_predgain[%f]<1.0f!!!\n", __FUNCTION__,
                             pProcPara->OBResV12.isp_ob_predgain);
            }
            if (pProcPara->OBResV12.blc_ob_enable &&
                pProcPara->OBResV12.isp_ob_predgain >= ISP_PREDGAIN_DEFAULT)
                pAdehazeCtx->CurrDataV14.ISO *= pProcPara->OBResV12.isp_ob_predgain;
        } else if (pAdehazeCtx->FrameNumber == HDR_2X_NUM ||
                   pAdehazeCtx->FrameNumber == HDR_3X_NUM) {
            pAdehazeCtx->CurrDataV14.ISO =
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else {
        pAdehazeCtx->CurrDataV14.ISO = ISOMIN;
        LOGW_ADEHAZE("%s:AE cur expo is NULL!\n", __FUNCTION__);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
    return ret;
}

XCamReturn AdehazeInit(AdehazeHandle_t** pAdehazeCtx, CamCalibDbV2Context_t* pCalib) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* handle = (AdehazeHandle_t*)calloc(1, sizeof(AdehazeHandle_t));

    CalibDbV2_dehaze_v14_t* calibv2_adehaze_calib_V14 =
        (CalibDbV2_dehaze_v14_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, adehaze_calib));
    memcpy(&handle->AdehazeAtrrV14.stAuto, calibv2_adehaze_calib_V14,
           sizeof(CalibDbV2_dehaze_v14_t));  // set defsult stAuto

    handle->PreDataV14.EnvLv   = ENVLVMIN;
    handle->PreDataV14.ApiMode = DEHAZE_API_AUTO;

    // set api default
    handle->AdehazeAtrrV14.mode = DEHAZE_API_AUTO;

    handle->AdehazeAtrrV14.Info.ISO                 = ISOMIN;
    handle->AdehazeAtrrV14.Info.EnvLv               = ENVLVMIN;
    handle->AdehazeAtrrV14.Info.MDehazeStrth        = DEHAZE_DEFAULT_LEVEL;
    handle->AdehazeAtrrV14.Info.MEnhanceStrth       = ENHANCE_DEFAULT_LEVEL;
    handle->AdehazeAtrrV14.Info.MEnhanceChromeStrth = ENHANCE_DEFAULT_LEVEL;

    *pAdehazeCtx = handle;
    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return (ret);
}

XCamReturn AdehazeRelease(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pAdehazeCtx) free(pAdehazeCtx);
    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return (ret);
}

XCamReturn AdehazeProcess(AdehazeHandle_t* pAdehazeCtx, dehaze_stats_v14_t* pStats, bool stats_true,
                          RkAiqAdehazeProcResult_t* pAdehzeProcRes) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__,
                 pAdehzeProcRes->enable,
                 (pAdehzeProcRes->ProcResV14.dc_en & FUNCTION_ENABLE) &&
                     (!(pAdehzeProcRes->ProcResV14.enhance_en & FUNCTION_ENABLE)),
                 (pAdehzeProcRes->ProcResV14.dc_en & FUNCTION_ENABLE) &&
                     (pAdehzeProcRes->ProcResV14.enhance_en & FUNCTION_ENABLE),
                 pAdehzeProcRes->ProcResV14.hist_en);

    if (pAdehazeCtx->AdehazeAtrrV14.mode == DEHAZE_API_AUTO) {
        float CtrlValue = pAdehazeCtx->CurrDataV14.ISO;

        // commom setting
        if (pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_dehaze_setting.en ||
            pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_enhance_setting.en ||
            pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_hist_setting.en)
            GetCommomParamsV14(&pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara, pAdehzeProcRes,
                               pAdehazeCtx->width, pAdehazeCtx->height,
                               pAdehazeCtx->AdehazeAtrrV14.Info.MDehazeStrth, CtrlValue);

        // dehaze setting
        if (pAdehzeProcRes->ProcResV14.dc_en)
            GetDehazeParamsV14(&pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara, pAdehzeProcRes,
                               pAdehazeCtx->width, pAdehazeCtx->height,
                               pAdehazeCtx->AdehazeAtrrV14.Info.MDehazeStrth, CtrlValue);

        // enhance setting
        // enhance curve is effective in dehaze function. when dc_en on, GetEnhanceParamsV14 is on
        if (pAdehzeProcRes->ProcResV14.dc_en)
            GetEnhanceParamsV14(&pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara,
                                pAdehzeProcRes, pAdehazeCtx->AdehazeAtrrV14.Info.MEnhanceStrth,
                                pAdehazeCtx->AdehazeAtrrV14.Info.MEnhanceChromeStrth, CtrlValue);

        // hist setting
        if (pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_hist_setting.en)
            GetHistParamsV14(&pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara, pAdehzeProcRes,
                             pStats, stats_true, pAdehazeCtx->height, pAdehazeCtx->width,
                             CtrlValue);
    } else if (pAdehazeCtx->AdehazeAtrrV14.mode == DEHAZE_API_MANUAL) {
        // commom setting
        if (pAdehazeCtx->AdehazeAtrrV14.stManual.hw_dehaze_setting.en ||
            pAdehazeCtx->AdehazeAtrrV14.stManual.hw_enhance_setting.en ||
            pAdehazeCtx->AdehazeAtrrV14.stManual.hw_hist_setting.en)
            stManuGetCommomParamsV14(&pAdehazeCtx->AdehazeAtrrV14.stManual, pAdehzeProcRes,
                                     pAdehazeCtx->width, pAdehazeCtx->height,
                                     pAdehazeCtx->AdehazeAtrrV14.Info.MDehazeStrth);

        // dehaze setting
        if (pAdehzeProcRes->ProcResV14.dc_en)
            stManuGetDehazeParamsV14(&pAdehazeCtx->AdehazeAtrrV14.stManual, pAdehzeProcRes,
                                     pAdehazeCtx->width, pAdehazeCtx->height,
                                     pAdehazeCtx->AdehazeAtrrV14.Info.MDehazeStrth);

        // enhance setting
        // enhance curve is effective in dehaze function. when dc_en on, stManuGetEnhanceParamsV14 is on
        if (pAdehzeProcRes->ProcResV14.dc_en)
            stManuGetEnhanceParamsV14(&pAdehazeCtx->AdehazeAtrrV14.stManual, pAdehzeProcRes,
                                      pAdehazeCtx->AdehazeAtrrV14.Info.MEnhanceStrth,
                                      pAdehazeCtx->AdehazeAtrrV14.Info.MEnhanceChromeStrth);

        // hist setting
        if (pAdehazeCtx->AdehazeAtrrV14.stManual.hw_hist_setting.en)
            stManuGetHistParamsV14(&pAdehazeCtx->AdehazeAtrrV14.stManual, pAdehzeProcRes, pStats,
                                   stats_true, pAdehazeCtx->height, pAdehazeCtx->width);
    } else
        LOGE_ADEHAZE("%s:Wrong Adehaze API mode!!! \n", __func__);

    // get local gain setting
    ret = GetDehazeLocalGainSettingV14(pAdehzeProcRes, pAdehazeCtx->YnrProcResV24_sigma);

    // store pre data
    pAdehazeCtx->PreDataV14.EnvLv   = pAdehazeCtx->CurrDataV14.EnvLv;
    pAdehazeCtx->PreDataV14.ISO     = pAdehazeCtx->CurrDataV14.ISO;
    pAdehazeCtx->PreDataV14.ApiMode = pAdehazeCtx->CurrDataV14.ApiMode;

    // store api info
    pAdehazeCtx->AdehazeAtrrV14.Info.ISO   = pAdehazeCtx->CurrDataV14.ISO;
    pAdehazeCtx->AdehazeAtrrV14.Info.EnvLv = pAdehazeCtx->CurrDataV14.EnvLv;

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return ret;
}

bool AdehazeByPassProcessing(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool byPassProc = true;

    if (pAdehazeCtx->FrameID <= INIT_CALC_PARAMS_NUM)
        byPassProc = false;
    else if (pAdehazeCtx->AdehazeAtrrV14.mode != pAdehazeCtx->PreDataV14.ApiMode)
        byPassProc = false;
    else if (pAdehazeCtx->AdehazeAtrrV14.mode == DEHAZE_API_MANUAL) {
        byPassProc = !pAdehazeCtx->ifReCalcStManual;
    } else if (pAdehazeCtx->AdehazeAtrrV14.mode == DEHAZE_API_AUTO) {
        float diff = pAdehazeCtx->PreDataV14.ISO - pAdehazeCtx->CurrDataV14.ISO;
        diff /= pAdehazeCtx->PreDataV14.ISO;
        if (diff >= pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.sw_dehaze_byPass_thred ||
            diff <=
                (0.0f - pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.sw_dehaze_byPass_thred))
            byPassProc = false;
        else
            byPassProc = true;
        byPassProc = byPassProc && !pAdehazeCtx->ifReCalcStAuto;
    }

    LOGD_ADEHAZE(
        "%s:FrameID:%d DehazeApiMode:%d ifReCalcStAuto:%d ifReCalcStManual:%d CtrlDataType:%d "
        "EnvLv:%f ISO:%f byPassProc:%d\n",
        __func__, pAdehazeCtx->FrameID, pAdehazeCtx->AdehazeAtrrV14.mode,
        pAdehazeCtx->ifReCalcStAuto, pAdehazeCtx->ifReCalcStManual,
        pAdehazeCtx->CurrDataV14.CtrlDataType, pAdehazeCtx->CurrDataV14.EnvLv,
        pAdehazeCtx->CurrDataV14.ISO, byPassProc);

    pAdehazeCtx->ifReCalcStManual = false;
    pAdehazeCtx->ifReCalcStAuto   = false;

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return byPassProc;
}

/******************************************************************************
 * DehazeEnableSetting()
 *
 *****************************************************************************/
bool DehazeEnableSetting(AdehazeHandle_t* pAdehazeCtx, RkAiqAdehazeProcResult_t* pAdehzeProcRes) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);

    if (pAdehazeCtx->AdehazeAtrrV14.mode == DEHAZE_API_AUTO) {
        pAdehzeProcRes->enable = pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_dehaze_en;

        if (pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_dehaze_en) {
            if (pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_dehaze_setting.en &&
                pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_enhance_setting.en) {
                pAdehzeProcRes->ProcResV14.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV14.enhance_en = FUNCTION_ENABLE;
            } else if (pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_dehaze_setting.en &&
                       !pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_enhance_setting.en) {
                pAdehzeProcRes->ProcResV14.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV14.enhance_en = FUNCTION_DISABLE;
            } else if (!pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_dehaze_setting.en &&
                       pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_enhance_setting.en) {
                pAdehzeProcRes->ProcResV14.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV14.enhance_en = FUNCTION_ENABLE;
            } else {
                pAdehzeProcRes->ProcResV14.dc_en      = FUNCTION_DISABLE;
                pAdehzeProcRes->ProcResV14.enhance_en = FUNCTION_DISABLE;
            }

            if (pAdehazeCtx->AdehazeAtrrV14.stAuto.DehazeTuningPara.hw_hist_setting.en)
                pAdehzeProcRes->ProcResV14.hist_en = FUNCTION_ENABLE;
            else
                pAdehzeProcRes->ProcResV14.hist_en = FUNCTION_DISABLE;
        }
    } else if (pAdehazeCtx->AdehazeAtrrV14.mode == DEHAZE_API_MANUAL) {
        pAdehzeProcRes->enable = pAdehazeCtx->AdehazeAtrrV14.stManual.hw_dehaze_en;

        if (pAdehazeCtx->AdehazeAtrrV14.stManual.hw_dehaze_en) {
            if (pAdehazeCtx->AdehazeAtrrV14.stManual.hw_dehaze_setting.en &&
                pAdehazeCtx->AdehazeAtrrV14.stManual.hw_enhance_setting.en) {
                pAdehzeProcRes->ProcResV14.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV14.enhance_en = FUNCTION_ENABLE;
            } else if (pAdehazeCtx->AdehazeAtrrV14.stManual.hw_dehaze_setting.en &&
                       !pAdehazeCtx->AdehazeAtrrV14.stManual.hw_enhance_setting.en) {
                pAdehzeProcRes->ProcResV14.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV14.enhance_en = FUNCTION_DISABLE;
            } else if (!pAdehazeCtx->AdehazeAtrrV14.stManual.hw_dehaze_setting.en &&
                       pAdehazeCtx->AdehazeAtrrV14.stManual.hw_enhance_setting.en) {
                pAdehzeProcRes->ProcResV14.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV14.enhance_en = FUNCTION_ENABLE;
            } else {
                pAdehzeProcRes->ProcResV14.dc_en      = FUNCTION_DISABLE;
                pAdehzeProcRes->ProcResV14.enhance_en = FUNCTION_DISABLE;
            }

            if (pAdehazeCtx->AdehazeAtrrV14.stManual.hw_hist_setting.en)
                pAdehzeProcRes->ProcResV14.hist_en = FUNCTION_ENABLE;
            else
                pAdehzeProcRes->ProcResV14.hist_en = FUNCTION_DISABLE;
        }
    } else {
        LOGE_ADEHAZE("%s: Dehaze api in WRONG MODE!!!, dehaze by pass!!!\n", __FUNCTION__);
        pAdehzeProcRes->enable = false;
    }

    return pAdehzeProcRes->enable;
    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
}
