/*
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

#include "hwi_c/isp39/aiq_isp39ParamsCvt.h"

#include "hwi_c/aiq_ispParamsCvt.h"
#include "include/algos/awb/fixfloat.h"
#include "include/algos/awb/rk_aiq_types_awb_algo_int.h"
#include "xcore/base/xcam_log.h"

#define ISP2X_WBGAIN_FIXSCALE_BIT 8
#define ISP3X_WBGAIN_INTSCALE_BIT 8
#define LOG2(x)                   (log((double)x) / log((double)2))

void convertAiqAeToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    rk_aiq_ae25_stats_cfg_cvt(pBase->_data, &pCvt->isp_params);
    pCvt->mAeParams               = (aiq_params_base_t*)pBase;
    pCvt->mLatestMeasCfg.rawae3 = pCvt->isp_params.isp_cfg->meas.rawae3;
    pCvt->mLatestMeasCfg.rawae0 = pCvt->isp_params.isp_cfg->meas.rawae0;
    pCvt->mLatestMeasCfg.rawhist3 = pCvt->isp_params.isp_cfg->meas.rawhist3;
    pCvt->mLatestMeasCfg.rawhist0 = pCvt->isp_params.isp_cfg->meas.rawhist0;
}

#if ISP_HW_V39
static void AwbGainOtpAdjust(rk_aiq_wb_gain_v32_t* awb_gain_out,
                             const struct rkmodule_awb_inf* otp_awb) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (otp_awb != NULL && otp_awb->flag == 0) {
        LOGV_AWB("%s: otp_awb is null \n", __FUNCTION__);
        return;
    } else {
        LOGV_AWB("%s: otp_awb flag(%d),golden(%d,%d,%d),current(%d,%d,%d) \n", __FUNCTION__,
                 otp_awb->flag, otp_awb->golden_r_value, otp_awb->golden_gr_value,
                 otp_awb->golden_b_value, otp_awb->r_value, otp_awb->gr_value, otp_awb->b_value);
    }
    awb_gain_out->bgain =
        awb_gain_out->bgain * (float)otp_awb->golden_b_value / (float)otp_awb->b_value;
    awb_gain_out->rgain =
        awb_gain_out->rgain * (float)otp_awb->golden_r_value / (float)otp_awb->r_value;
    awb_gain_out->gbgain = awb_gain_out->gbgain;
    awb_gain_out->grgain =
        awb_gain_out->grgain * (float)otp_awb->golden_gr_value / (float)otp_awb->gr_value;

    LOGD_AWB("%s awb gain(multiplied by otp)  = [%f, %f, %f, %f] \n", __FUNCTION__,
             awb_gain_out->rgain, awb_gain_out->grgain, awb_gain_out->gbgain, awb_gain_out->bgain);
}

// call afte ablc_res_v32 got
static void ConfigWbgainBaseOnBlc(rk_aiq_wb_gain_v32_t* awb_gain, const blc_res_cvt_t* blc,
                                  float dgain) {
    short int mainPieplineBLC[AWB_CHANNEL_MAX];
    if (blc == NULL || !blc->en) {
        return;
    }
    memset(mainPieplineBLC, 0, sizeof(mainPieplineBLC[0]) * AWB_CHANNEL_MAX);
    if (blc->en) {
        mainPieplineBLC[AWB_CHANNEL_R] += blc->obcPreTnr.hw_blcC_obR_val;
        mainPieplineBLC[AWB_CHANNEL_GR] += blc->obcPreTnr.hw_blcC_obGr_val;
        mainPieplineBLC[AWB_CHANNEL_B] += blc->obcPreTnr.hw_blcC_obB_val;
        mainPieplineBLC[AWB_CHANNEL_GB] += blc->obcPreTnr.hw_blcC_obGb_val;
    }
    // if (awb_gain->applyPosition == IN_AWBGAIN1 &&
    //     blc->obcPostTnr.sw_blcT_obcPostTnr_mode == blc_manualOBCPostTnr_mode) {
    //     mainPieplineBLC[AWB_CHANNEL_R] += blc->obcPostTnr.hw_blcT_manualOBR_val * dgain;
    //     mainPieplineBLC[AWB_CHANNEL_GR] += blc->obcPostTnr.hw_blcT_manualOBGr_val * dgain;
    //     mainPieplineBLC[AWB_CHANNEL_B] += blc->obcPostTnr.hw_blcT_manualOBB_val * dgain;
    //     mainPieplineBLC[AWB_CHANNEL_GB] += blc->obcPostTnr.hw_blcT_manualOBGb_val * dgain;
    // }
    for (int i = 0; i < AWB_CHANNEL_MAX; i++) {
        if (mainPieplineBLC[i] > 4094) {
            mainPieplineBLC[i] = 4094;
            LOGE_AWB("mainPieplineBLC[%d] = %d is too large!!!!", i, mainPieplineBLC[i]);
        }
    }
    float maxg1 = 0;
    if (mainPieplineBLC[AWB_CHANNEL_R] > 0) {
        awb_gain->rgain *= 4095.0 / (4095 - mainPieplineBLC[AWB_CHANNEL_R]);  // max_value=4095
    }
    if (maxg1 < awb_gain->rgain) {
        maxg1 = awb_gain->rgain;
    }
    if (mainPieplineBLC[AWB_CHANNEL_B] > 0) {
        awb_gain->bgain *= 4095.0 / (4095 - mainPieplineBLC[AWB_CHANNEL_B]);  // max_value=4095
    }
    if (maxg1 < awb_gain->bgain) {
        maxg1 = awb_gain->bgain;
    }
    if (mainPieplineBLC[AWB_CHANNEL_GR] > 0) {
        awb_gain->grgain *= 4095.0 / (4095 - mainPieplineBLC[AWB_CHANNEL_GR]);  // max_value=4095
    }
    if (maxg1 < awb_gain->grgain) {
        maxg1 = awb_gain->grgain;
    }
    if (mainPieplineBLC[AWB_CHANNEL_GB] > 0) {
        awb_gain->gbgain *= 4095.0 / (4095 - mainPieplineBLC[AWB_CHANNEL_GB]);  // max_value=4095
    }
    if (maxg1 < awb_gain->gbgain) {
        maxg1 = awb_gain->gbgain;
    }

    if (maxg1 > 8) {  // max_wbgain=8.0
        float scale = 8 / maxg1;
        awb_gain->gbgain *= scale;
        awb_gain->grgain *= scale;
        awb_gain->bgain *= scale;
        awb_gain->rgain *= scale;
        LOGW_AWB("max wbgain is %f, maybe error case", maxg1);
    }
    LOGD_AWB("%s awb gain  = [%f, %f, %f, %f] \n", __FUNCTION__, awb_gain->rgain, awb_gain->grgain,
             awb_gain->gbgain, awb_gain->bgain);
}

static void convertAiqAwbGainToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    LOG1_AWB("%s enter", __FUNCTION__);
    if (1) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP39_MODULE_AWB_GAIN;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_AWB_GAIN;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP39_MODULE_AWB_GAIN;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP39_MODULE_AWB_GAIN;
        pCvt->isp_params.isp_cfg->module_en_update &= ~ISP39_MODULE_AWB_GAIN;
        return;
    }

    pCvt->awb_gain_final     = *(rk_aiq_wb_gain_v32_t*)pBase->_data;
    const blc_res_cvt_t* blc = &pCvt->mCommonCvtInfo.blc_res;

    // 1)//check awb gain apply position
    pCvt->awb_gain_final.applyPosition =
        ((rk_aiq_working_mode_t)(pCvt->_working_mode) == RK_AIQ_WORKING_MODE_NORMAL) ? IN_AWBGAIN1
        : IN_AWBGAIN0;
    // 2) //otp
    AwbGainOtpAdjust(&pCvt->awb_gain_final, &pCvt->mCommonCvtInfo.otp_awb);
    // 3)//ConfigWbgainBaseOnBlc
    float dgain = pCvt->mCommonCvtInfo.frameDGain[0] > 1 ? pCvt->mCommonCvtInfo.frameDGain[0] : 1;
    ConfigWbgainBaseOnBlc(&pCvt->awb_gain_final, blc, dgain);
    // iqmap
    // 4)//set to reg
    struct isp32_awb_gain_cfg* cfg = &pCvt->isp_params.isp_cfg->others.awb_gain_cfg;
    uint16_t max_wb_gain           = (1 << (ISP2X_WBGAIN_FIXSCALE_BIT + 3)) - 1;
    uint16_t R  = (uint16_t)(0.5 + pCvt->awb_gain_final.rgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t B  = (uint16_t)(0.5 + pCvt->awb_gain_final.bgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t Gr = (uint16_t)(0.5 + pCvt->awb_gain_final.grgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    uint16_t Gb = (uint16_t)(0.5 + pCvt->awb_gain_final.gbgain * (1 << ISP2X_WBGAIN_FIXSCALE_BIT));
    if (pCvt->awb_gain_final.applyPosition == IN_AWBGAIN0) {
        cfg->gain0_red     = R > max_wb_gain ? max_wb_gain : R;
        cfg->gain0_blue    = B > max_wb_gain ? max_wb_gain : B;
        cfg->gain0_green_r = Gr > max_wb_gain ? max_wb_gain : Gr;
        cfg->gain0_green_b = Gb > max_wb_gain ? max_wb_gain : Gb;
        cfg->gain1_red     = R > max_wb_gain ? max_wb_gain : R;
        cfg->gain1_blue    = B > max_wb_gain ? max_wb_gain : B;
        cfg->gain1_green_r = Gr > max_wb_gain ? max_wb_gain : Gr;
        cfg->gain1_green_b = Gb > max_wb_gain ? max_wb_gain : Gb;
        cfg->gain2_red     = R > max_wb_gain ? max_wb_gain : R;
        cfg->gain2_blue    = B > max_wb_gain ? max_wb_gain : B;
        cfg->gain2_green_r = Gr > max_wb_gain ? max_wb_gain : Gr;
        cfg->gain2_green_b = Gb > max_wb_gain ? max_wb_gain : Gb;
        cfg->awb1_gain_r   = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->awb1_gain_b   = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->awb1_gain_gr  = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->awb1_gain_gb  = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
    } else {
        cfg->gain0_red     = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain0_blue    = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain0_green_r = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain0_green_b = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain1_red     = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain1_blue    = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain1_green_r = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain1_green_b = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain2_red     = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain2_blue    = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain2_green_r = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->gain2_green_b = (1 << ISP2X_WBGAIN_FIXSCALE_BIT);
        cfg->awb1_gain_r   = R > max_wb_gain ? max_wb_gain : R;
        cfg->awb1_gain_b   = B > max_wb_gain ? max_wb_gain : B;
        cfg->awb1_gain_gr  = Gr > max_wb_gain ? max_wb_gain : Gr;
        cfg->awb1_gain_gb  = Gb > max_wb_gain ? max_wb_gain : Gb;
    }

    pCvt->mLatestWbGainCfg = *cfg;
}

static void WriteAwbReg(struct isp39_rawawb_meas_cfg* awb_cfg_v32) {
#if 0
    static int count = 0;
    if(count > 20) return;
    char fName[100];
    sprintf(fName, "./tmp/awb_reg_%d.txt", count);
    count++;
    LOGE_AWB( "%s", fName);

    FILE* fp = fopen(fName, "wb");
    if(fp == NULL) {
        return;
    }
    //fprintf(fp, "\t\tsw_rawawb_en = 0x%0x (%d)\n",                              awb_cfg_v32->en                            ,awb_cfg_v32->en);
    fprintf(fp, "\t\tsw_drc2awb_sel = 0x%0x (%d)\n",                                  awb_cfg_v32->drc2awb_sel, awb_cfg_v32->drc2awb_sel);
    fprintf(fp, "\t\tsw_bnr2awb_sel = 0x%0x (%d)\n",                                  awb_cfg_v32->bnr2awb_sel, awb_cfg_v32->bnr2awb_sel);
    fprintf(fp, "\t\tsw_rawawb_sel = 0x%0x (%d)\n",                                   awb_cfg_v32->rawawb_sel, awb_cfg_v32->rawawb_sel);
    fprintf(fp, "\t\tsw_rawawb_uv_en0 = 0x%0x (%d)\n",                            awb_cfg_v32->uv_en0, awb_cfg_v32->uv_en0);
    fprintf(fp, "\t\tsw_rawawb_xy_en0 = 0x%0x (%d)\n",                            awb_cfg_v32->xy_en0, awb_cfg_v32->xy_en0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_en0 = 0x%0x (%d)\n",                         awb_cfg_v32->yuv3d_en0, awb_cfg_v32->yuv3d_en0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx0 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx0, awb_cfg_v32->yuv3d_ls_idx0);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx1 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx1, awb_cfg_v32->yuv3d_ls_idx1);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx2 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx2, awb_cfg_v32->yuv3d_ls_idx2);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_ls_idx3 = 0x%0x (%d)\n",                     awb_cfg_v32->yuv3d_ls_idx3, awb_cfg_v32->yuv3d_ls_idx3);
    fprintf(fp, "\t\tsw_rawawb_in_rshift_to_12bit_en = 0x%0x (%d)\n",             awb_cfg_v32->in_rshift_to_12bit_en, awb_cfg_v32->in_rshift_to_12bit_en);
    fprintf(fp, "\t\tsw_rawawb_in_overexposure_check_en = 0x%0x (%d)\n",          awb_cfg_v32->in_overexposure_check_en, awb_cfg_v32->in_overexposure_check_en);
    fprintf(fp, "\t\tsw_rawawb_wind_size = 0x%0x (%d)\n",                         awb_cfg_v32->wind_size, awb_cfg_v32->wind_size);
    fprintf(fp, "\t\tsw_rawlsc_bypass_en = 0x%0x (%d)\n",                         awb_cfg_v32->rawlsc_bypass_en, awb_cfg_v32->rawlsc_bypass_en);
    fprintf(fp, "\t\tsw_rawawb_light_num = 0x%0x (%d)\n",                         awb_cfg_v32->light_num, awb_cfg_v32->light_num);
    //fprintf(fp, "\t\tsw_rawawb_2ddr_path_en = 0x%0x (%d)\n",                      awb_cfg_v32->2ddr_path_en                  ,awb_cfg_v32->2ddr_path_en);
    fprintf(fp, "\t\tsw_rawawb_uv_en1 = 0x%0x (%d)\n",                            awb_cfg_v32->uv_en1, awb_cfg_v32->uv_en1);
    fprintf(fp, "\t\tsw_rawawb_xy_en1 = 0x%0x (%d)\n",                            awb_cfg_v32->xy_en1, awb_cfg_v32->xy_en1);
    fprintf(fp, "\t\tsw_rawawb_yuv3d_en1 = 0x%0x (%d)\n",                         awb_cfg_v32->yuv3d_en1, awb_cfg_v32->yuv3d_en1);
    //fprintf(fp, "\t\tsw_rawawb_2ddr_path_sel = 0x%0x (%d)\n",                     awb_cfg_v32->2ddr_path_sel                 ,awb_cfg_v32->2ddr_path_sel);
    fprintf(fp, "\t\tsw_rawawbin_low12bit_val = 0x%0x (%d)\n",                    awb_cfg_v32->low12bit_val, awb_cfg_v32->low12bit_val);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_en = 0x%0x (%d)\n",                    awb_cfg_v32->blk_measure_enable, awb_cfg_v32->blk_measure_enable);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_mode = 0x%0x (%d)\n",                  awb_cfg_v32->blk_measure_mode, awb_cfg_v32->blk_measure_mode);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_xytype = 0x%0x (%d)\n",                awb_cfg_v32->blk_measure_xytype, awb_cfg_v32->blk_measure_xytype);
    fprintf(fp, "\t\tsw_rawawb_blk_rtdw_measure_en = 0x%0x (%d)\n",               awb_cfg_v32->blk_rtdw_measure_en, awb_cfg_v32->blk_rtdw_measure_en);
    fprintf(fp, "\t\tsw_rawawb_blk_measure_illu_idx = 0x%0x (%d)\n",              awb_cfg_v32->blk_measure_illu_idx, awb_cfg_v32->blk_measure_illu_idx);
    fprintf(fp, "\t\tsw_rawawb_ds16x8_mode_en = 0x%0x (%d)\n",                    awb_cfg_v32->ds16x8_mode_en, awb_cfg_v32->ds16x8_mode_en);
    fprintf(fp, "\t\tsw_rawawb_blk_with_luma_wei_en = 0x%0x (%d)\n",              awb_cfg_v32->blk_with_luma_wei_en, awb_cfg_v32->blk_with_luma_wei_en);
    fprintf(fp, "\t\tsw_rawawb_in_overexposure_threshold = 0x%0x (%d)\n",         awb_cfg_v32->in_overexposure_threshold, awb_cfg_v32->in_overexposure_threshold);
    fprintf(fp, "\t\tsw_rawawb_h_offs = 0x%0x (%d)\n",                            awb_cfg_v32->h_offs, awb_cfg_v32->h_offs);
    fprintf(fp, "\t\tsw_rawawb_v_offs = 0x%0x (%d)\n",                            awb_cfg_v32->v_offs, awb_cfg_v32->v_offs);
    fprintf(fp, "\t\tsw_rawawb_h_size = 0x%0x (%d)\n",                            awb_cfg_v32->h_size, awb_cfg_v32->h_size);
    fprintf(fp, "\t\tsw_rawawb_v_size = 0x%0x (%d)\n",                            awb_cfg_v32->v_size, awb_cfg_v32->v_size);
    fprintf(fp, "\t\tsw_rawawb_r_max = 0x%0x (%d)\n",                             awb_cfg_v32->r_max, awb_cfg_v32->r_max);
    fprintf(fp, "\t\tsw_rawawb_g_max = 0x%0x (%d)\n",                             awb_cfg_v32->g_max, awb_cfg_v32->g_max);
    fprintf(fp, "\t\tsw_rawawb_b_max = 0x%0x (%d)\n",                             awb_cfg_v32->b_max, awb_cfg_v32->b_max);
    fprintf(fp, "\t\tsw_rawawb_y_max = 0x%0x (%d)\n",                             awb_cfg_v32->y_max, awb_cfg_v32->y_max);
    fprintf(fp, "\t\tsw_rawawb_r_min = 0x%0x (%d)\n",                             awb_cfg_v32->r_min, awb_cfg_v32->r_min);
    fprintf(fp, "\t\tsw_rawawb_g_min = 0x%0x (%d)\n",                             awb_cfg_v32->g_min, awb_cfg_v32->g_min);
    fprintf(fp, "\t\tsw_rawawb_b_min = 0x%0x (%d)\n",                             awb_cfg_v32->b_min, awb_cfg_v32->b_min);
    fprintf(fp, "\t\tsw_rawawb_y_min = 0x%0x (%d)\n",                             awb_cfg_v32->y_min, awb_cfg_v32->y_min);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_wei_en0 = 0x%0x (%d)\n",                   awb_cfg_v32->wp_luma_wei_en0, awb_cfg_v32->wp_luma_wei_en0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_wei_en1 = 0x%0x (%d)\n",                   awb_cfg_v32->wp_luma_wei_en1, awb_cfg_v32->wp_luma_wei_en1);
    fprintf(fp, "\t\tsw_rawawb_wp_blk_wei_en0 = 0x%0x (%d)\n",                    awb_cfg_v32->wp_blk_wei_en0, awb_cfg_v32->wp_blk_wei_en0);
    fprintf(fp, "\t\tsw_rawawb_wp_blk_wei_en1 = 0x%0x (%d)\n",                    awb_cfg_v32->wp_blk_wei_en1, awb_cfg_v32->wp_blk_wei_en1);
    fprintf(fp, "\t\tsw_rawawb_wp_hist_xytype = 0x%0x (%d)\n",                    awb_cfg_v32->wp_hist_xytype, awb_cfg_v32->wp_hist_xytype);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y0 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y0, awb_cfg_v32->wp_luma_weicurve_y0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y1 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y1, awb_cfg_v32->wp_luma_weicurve_y1);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y2 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y2, awb_cfg_v32->wp_luma_weicurve_y2);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y3 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y3, awb_cfg_v32->wp_luma_weicurve_y3);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y4 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y4, awb_cfg_v32->wp_luma_weicurve_y4);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y5 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y5, awb_cfg_v32->wp_luma_weicurve_y5);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y6 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y6, awb_cfg_v32->wp_luma_weicurve_y6);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y7 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y7, awb_cfg_v32->wp_luma_weicurve_y7);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_y8 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_y8, awb_cfg_v32->wp_luma_weicurve_y8);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w0 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w0, awb_cfg_v32->wp_luma_weicurve_w0);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w1 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w1, awb_cfg_v32->wp_luma_weicurve_w1);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w2 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w2, awb_cfg_v32->wp_luma_weicurve_w2);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w3 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w3, awb_cfg_v32->wp_luma_weicurve_w3);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w4 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w4, awb_cfg_v32->wp_luma_weicurve_w4);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w5 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w5, awb_cfg_v32->wp_luma_weicurve_w5);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w6 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w6, awb_cfg_v32->wp_luma_weicurve_w6);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w7 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w7, awb_cfg_v32->wp_luma_weicurve_w7);
    fprintf(fp, "\t\tsw_rawawb_wp_luma_weicurve_w8 = 0x%0x (%d)\n",               awb_cfg_v32->wp_luma_weicurve_w8, awb_cfg_v32->wp_luma_weicurve_w8);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_r = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_r, awb_cfg_v32->pre_wbgain_inv_r);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_g = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_g, awb_cfg_v32->pre_wbgain_inv_g);
    fprintf(fp, "\t\tsw_rawawb_pre_wbgain_inv_b = 0x%0x (%d)\n",                  awb_cfg_v32->pre_wbgain_inv_b, awb_cfg_v32->pre_wbgain_inv_b);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_0, awb_cfg_v32->vertex0_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_0, awb_cfg_v32->vertex0_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_0, awb_cfg_v32->vertex1_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_0, awb_cfg_v32->vertex1_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_0, awb_cfg_v32->vertex2_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_0, awb_cfg_v32->vertex2_v_0);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_0, awb_cfg_v32->vertex3_u_0);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_0 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_0, awb_cfg_v32->vertex3_v_0);
    fprintf(fp, "\t\tsw_rawawb_islope01_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_0, awb_cfg_v32->islope01_0);
    fprintf(fp, "\t\tsw_rawawb_islope12_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_0, awb_cfg_v32->islope12_0);
    fprintf(fp, "\t\tsw_rawawb_islope23_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_0, awb_cfg_v32->islope23_0);
    fprintf(fp, "\t\tsw_rawawb_islope30_0 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_0, awb_cfg_v32->islope30_0);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_1, awb_cfg_v32->vertex0_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_1, awb_cfg_v32->vertex0_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_1, awb_cfg_v32->vertex1_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_1, awb_cfg_v32->vertex1_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_1, awb_cfg_v32->vertex2_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_1, awb_cfg_v32->vertex2_v_1);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_1, awb_cfg_v32->vertex3_u_1);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_1 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_1, awb_cfg_v32->vertex3_v_1);
    fprintf(fp, "\t\tsw_rawawb_islope01_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_1, awb_cfg_v32->islope01_1);
    fprintf(fp, "\t\tsw_rawawb_islope12_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_1, awb_cfg_v32->islope12_1);
    fprintf(fp, "\t\tsw_rawawb_islope23_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_1, awb_cfg_v32->islope23_1);
    fprintf(fp, "\t\tsw_rawawb_islope30_1 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_1, awb_cfg_v32->islope30_1);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_2, awb_cfg_v32->vertex0_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_2, awb_cfg_v32->vertex0_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_2, awb_cfg_v32->vertex1_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_2, awb_cfg_v32->vertex1_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_2, awb_cfg_v32->vertex2_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_2, awb_cfg_v32->vertex2_v_2);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_2, awb_cfg_v32->vertex3_u_2);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_2 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_2, awb_cfg_v32->vertex3_v_2);
    fprintf(fp, "\t\tsw_rawawb_islope01_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_2, awb_cfg_v32->islope01_2);
    fprintf(fp, "\t\tsw_rawawb_islope12_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_2, awb_cfg_v32->islope12_2);
    fprintf(fp, "\t\tsw_rawawb_islope23_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_2, awb_cfg_v32->islope23_2);
    fprintf(fp, "\t\tsw_rawawb_islope30_2 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_2, awb_cfg_v32->islope30_2);
    fprintf(fp, "\t\tsw_rawawb_vertex0_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_u_3, awb_cfg_v32->vertex0_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex0_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex0_v_3, awb_cfg_v32->vertex0_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex1_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_u_3, awb_cfg_v32->vertex1_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex1_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex1_v_3, awb_cfg_v32->vertex1_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex2_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_u_3, awb_cfg_v32->vertex2_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex2_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex2_v_3, awb_cfg_v32->vertex2_v_3);
    fprintf(fp, "\t\tsw_rawawb_vertex3_u_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_u_3, awb_cfg_v32->vertex3_u_3);
    fprintf(fp, "\t\tsw_rawawb_vertex3_v_3 = 0x%0x (%d)\n",                       awb_cfg_v32->vertex3_v_3, awb_cfg_v32->vertex3_v_3);
    fprintf(fp, "\t\tsw_rawawb_islope01_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope01_3, awb_cfg_v32->islope01_3);
    fprintf(fp, "\t\tsw_rawawb_islope12_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope12_3, awb_cfg_v32->islope12_3);
    fprintf(fp, "\t\tsw_rawawb_islope23_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope23_3, awb_cfg_v32->islope23_3);
    fprintf(fp, "\t\tsw_rawawb_islope30_3 = 0x%0x (%d)\n",                        awb_cfg_v32->islope30_3, awb_cfg_v32->islope30_3);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_y, awb_cfg_v32->rgb2ryuvmat0_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_y, awb_cfg_v32->rgb2ryuvmat1_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_y = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_y, awb_cfg_v32->rgb2ryuvmat2_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_y = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_y, awb_cfg_v32->rgb2ryuvofs_y);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_u, awb_cfg_v32->rgb2ryuvmat0_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_u, awb_cfg_v32->rgb2ryuvmat1_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_u = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_u, awb_cfg_v32->rgb2ryuvmat2_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_u = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_u, awb_cfg_v32->rgb2ryuvofs_u);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat0_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat0_v, awb_cfg_v32->rgb2ryuvmat0_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat1_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat1_v, awb_cfg_v32->rgb2ryuvmat1_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvmat2_v = 0x%0x (%d)\n",                    awb_cfg_v32->rgb2ryuvmat2_v, awb_cfg_v32->rgb2ryuvmat2_v);
    fprintf(fp, "\t\tsw_rawawb_rgb2ryuvofs_v = 0x%0x (%d)\n",                     awb_cfg_v32->rgb2ryuvofs_v, awb_cfg_v32->rgb2ryuvofs_v);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_y, awb_cfg_v32->coor_x1_ls0_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_y, awb_cfg_v32->vec_x21_ls0_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_u, awb_cfg_v32->coor_x1_ls0_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_u, awb_cfg_v32->vec_x21_ls0_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls0_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls0_v, awb_cfg_v32->coor_x1_ls0_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls0_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls0_v, awb_cfg_v32->vec_x21_ls0_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls0 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls0, awb_cfg_v32->dis_x1x2_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls0, awb_cfg_v32->rotu0_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls0, awb_cfg_v32->rotu1_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls0, awb_cfg_v32->rotu2_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls0, awb_cfg_v32->rotu3_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls0, awb_cfg_v32->rotu4_ls0);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls0 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls0, awb_cfg_v32->rotu5_ls0);
    fprintf(fp, "\t\tsw_rawawb_th0_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls0, awb_cfg_v32->th0_ls0);
    fprintf(fp, "\t\tsw_rawawb_th1_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls0, awb_cfg_v32->th1_ls0);
    fprintf(fp, "\t\tsw_rawawb_th2_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls0, awb_cfg_v32->th2_ls0);
    fprintf(fp, "\t\tsw_rawawb_th3_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls0, awb_cfg_v32->th3_ls0);
    fprintf(fp, "\t\tsw_rawawb_th4_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls0, awb_cfg_v32->th4_ls0);
    fprintf(fp, "\t\tsw_rawawb_th5_ls0 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls0, awb_cfg_v32->th5_ls0);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_y, awb_cfg_v32->coor_x1_ls1_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_y, awb_cfg_v32->vec_x21_ls1_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_u, awb_cfg_v32->coor_x1_ls1_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_u, awb_cfg_v32->vec_x21_ls1_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls1_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls1_v, awb_cfg_v32->coor_x1_ls1_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls1_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls1_v, awb_cfg_v32->vec_x21_ls1_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls1 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls1, awb_cfg_v32->dis_x1x2_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls1, awb_cfg_v32->rotu0_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls1, awb_cfg_v32->rotu1_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls1, awb_cfg_v32->rotu2_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls1, awb_cfg_v32->rotu3_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls1, awb_cfg_v32->rotu4_ls1);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls1 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls1, awb_cfg_v32->rotu5_ls1);
    fprintf(fp, "\t\tsw_rawawb_th0_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls1, awb_cfg_v32->th0_ls1);
    fprintf(fp, "\t\tsw_rawawb_th1_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls1, awb_cfg_v32->th1_ls1);
    fprintf(fp, "\t\tsw_rawawb_th2_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls1, awb_cfg_v32->th2_ls1);
    fprintf(fp, "\t\tsw_rawawb_th3_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls1, awb_cfg_v32->th3_ls1);
    fprintf(fp, "\t\tsw_rawawb_th4_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls1, awb_cfg_v32->th4_ls1);
    fprintf(fp, "\t\tsw_rawawb_th5_ls1 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls1, awb_cfg_v32->th5_ls1);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_y, awb_cfg_v32->coor_x1_ls2_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_y, awb_cfg_v32->vec_x21_ls2_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_u, awb_cfg_v32->coor_x1_ls2_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_u, awb_cfg_v32->vec_x21_ls2_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls2_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls2_v, awb_cfg_v32->coor_x1_ls2_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls2_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls2_v, awb_cfg_v32->vec_x21_ls2_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls2 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls2, awb_cfg_v32->dis_x1x2_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls2, awb_cfg_v32->rotu0_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls2, awb_cfg_v32->rotu1_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls2, awb_cfg_v32->rotu2_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls2, awb_cfg_v32->rotu3_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls2, awb_cfg_v32->rotu4_ls2);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls2 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls2, awb_cfg_v32->rotu5_ls2);
    fprintf(fp, "\t\tsw_rawawb_th0_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls2, awb_cfg_v32->th0_ls2);
    fprintf(fp, "\t\tsw_rawawb_th1_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls2, awb_cfg_v32->th1_ls2);
    fprintf(fp, "\t\tsw_rawawb_th2_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls2, awb_cfg_v32->th2_ls2);
    fprintf(fp, "\t\tsw_rawawb_th3_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls2, awb_cfg_v32->th3_ls2);
    fprintf(fp, "\t\tsw_rawawb_th4_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls2, awb_cfg_v32->th4_ls2);
    fprintf(fp, "\t\tsw_rawawb_th5_ls2 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls2, awb_cfg_v32->th5_ls2);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_y = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_y, awb_cfg_v32->coor_x1_ls3_y);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_y = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_y, awb_cfg_v32->vec_x21_ls3_y);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_u = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_u, awb_cfg_v32->coor_x1_ls3_u);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_u = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_u, awb_cfg_v32->vec_x21_ls3_u);
    fprintf(fp, "\t\tsw_rawawb_coor_x1_ls3_v = 0x%0x (%d)\n",                     awb_cfg_v32->coor_x1_ls3_v, awb_cfg_v32->coor_x1_ls3_v);
    fprintf(fp, "\t\tsw_rawawb_vec_x21_ls3_v = 0x%0x (%d)\n",                     awb_cfg_v32->vec_x21_ls3_v, awb_cfg_v32->vec_x21_ls3_v);
    fprintf(fp, "\t\tsw_rawawb_dis_x1x2_ls3 = 0x%0x (%d)\n",                      awb_cfg_v32->dis_x1x2_ls3, awb_cfg_v32->dis_x1x2_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu0_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu0_ls3, awb_cfg_v32->rotu0_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu1_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu1_ls3, awb_cfg_v32->rotu1_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu2_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu2_ls3, awb_cfg_v32->rotu2_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu3_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu3_ls3, awb_cfg_v32->rotu3_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu4_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu4_ls3, awb_cfg_v32->rotu4_ls3);
    fprintf(fp, "\t\tsw_rawawb_rotu5_ls3 = 0x%0x (%d)\n",                         awb_cfg_v32->rotu5_ls3, awb_cfg_v32->rotu5_ls3);
    fprintf(fp, "\t\tsw_rawawb_th0_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th0_ls3, awb_cfg_v32->th0_ls3);
    fprintf(fp, "\t\tsw_rawawb_th1_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th1_ls3, awb_cfg_v32->th1_ls3);
    fprintf(fp, "\t\tsw_rawawb_th2_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th2_ls3, awb_cfg_v32->th2_ls3);
    fprintf(fp, "\t\tsw_rawawb_th3_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th3_ls3, awb_cfg_v32->th3_ls3);
    fprintf(fp, "\t\tsw_rawawb_th4_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th4_ls3, awb_cfg_v32->th4_ls3);
    fprintf(fp, "\t\tsw_rawawb_th5_ls3 = 0x%0x (%d)\n",                           awb_cfg_v32->th5_ls3, awb_cfg_v32->th5_ls3);
    fprintf(fp, "\t\tsw_rawawb_wt0 = 0x%0x (%d)\n",                               awb_cfg_v32->wt0, awb_cfg_v32->wt0);
    fprintf(fp, "\t\tsw_rawawb_wt1 = 0x%0x (%d)\n",                               awb_cfg_v32->wt1, awb_cfg_v32->wt1);
    fprintf(fp, "\t\tsw_rawawb_wt2 = 0x%0x (%d)\n",                               awb_cfg_v32->wt2, awb_cfg_v32->wt2);
    fprintf(fp, "\t\tsw_rawawb_mat0_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat0_x, awb_cfg_v32->mat0_x);
    fprintf(fp, "\t\tsw_rawawb_mat0_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat0_y, awb_cfg_v32->mat0_y);
    fprintf(fp, "\t\tsw_rawawb_mat1_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat1_x, awb_cfg_v32->mat1_x);
    fprintf(fp, "\t\tsw_rawawb_mat1_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat1_y, awb_cfg_v32->mat1_y);
    fprintf(fp, "\t\tsw_rawawb_mat2_x = 0x%0x (%d)\n",                            awb_cfg_v32->mat2_x, awb_cfg_v32->mat2_x);
    fprintf(fp, "\t\tsw_rawawb_mat2_y = 0x%0x (%d)\n",                            awb_cfg_v32->mat2_y, awb_cfg_v32->mat2_y);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_0, awb_cfg_v32->nor_x0_0);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_0, awb_cfg_v32->nor_x1_0);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_0, awb_cfg_v32->nor_y0_0);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_0, awb_cfg_v32->nor_y1_0);
    fprintf(fp, "\t\tsw_rawawb_big_x0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_0, awb_cfg_v32->big_x0_0);
    fprintf(fp, "\t\tsw_rawawb_big_x1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_0, awb_cfg_v32->big_x1_0);
    fprintf(fp, "\t\tsw_rawawb_big_y0_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_0, awb_cfg_v32->big_y0_0);
    fprintf(fp, "\t\tsw_rawawb_big_y1_0 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_0, awb_cfg_v32->big_y1_0);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_1, awb_cfg_v32->nor_x0_1);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_1, awb_cfg_v32->nor_x1_1);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_1, awb_cfg_v32->nor_y0_1);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_1, awb_cfg_v32->nor_y1_1);
    fprintf(fp, "\t\tsw_rawawb_big_x0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_1, awb_cfg_v32->big_x0_1);
    fprintf(fp, "\t\tsw_rawawb_big_x1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_1, awb_cfg_v32->big_x1_1);
    fprintf(fp, "\t\tsw_rawawb_big_y0_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_1, awb_cfg_v32->big_y0_1);
    fprintf(fp, "\t\tsw_rawawb_big_y1_1 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_1, awb_cfg_v32->big_y1_1);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_2, awb_cfg_v32->nor_x0_2);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_2, awb_cfg_v32->nor_x1_2);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_2, awb_cfg_v32->nor_y0_2);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_2, awb_cfg_v32->nor_y1_2);
    fprintf(fp, "\t\tsw_rawawb_big_x0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_2, awb_cfg_v32->big_x0_2);
    fprintf(fp, "\t\tsw_rawawb_big_x1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_2, awb_cfg_v32->big_x1_2);
    fprintf(fp, "\t\tsw_rawawb_big_y0_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_2, awb_cfg_v32->big_y0_2);
    fprintf(fp, "\t\tsw_rawawb_big_y1_2 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_2, awb_cfg_v32->big_y1_2);
    fprintf(fp, "\t\tsw_rawawb_nor_x0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x0_3, awb_cfg_v32->nor_x0_3);
    fprintf(fp, "\t\tsw_rawawb_nor_x1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_x1_3, awb_cfg_v32->nor_x1_3);
    fprintf(fp, "\t\tsw_rawawb_nor_y0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y0_3, awb_cfg_v32->nor_y0_3);
    fprintf(fp, "\t\tsw_rawawb_nor_y1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->nor_y1_3, awb_cfg_v32->nor_y1_3);
    fprintf(fp, "\t\tsw_rawawb_big_x0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x0_3, awb_cfg_v32->big_x0_3);
    fprintf(fp, "\t\tsw_rawawb_big_x1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_x1_3, awb_cfg_v32->big_x1_3);
    fprintf(fp, "\t\tsw_rawawb_big_y0_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y0_3, awb_cfg_v32->big_y0_3);
    fprintf(fp, "\t\tsw_rawawb_big_y1_3 = 0x%0x (%d)\n",                          awb_cfg_v32->big_y1_3, awb_cfg_v32->big_y1_3);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region0_excen, awb_cfg_v32->exc_wp_region0_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region0_measen, awb_cfg_v32->exc_wp_region0_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region0_domain, awb_cfg_v32->exc_wp_region0_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_weight = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region0_weight, awb_cfg_v32->exc_wp_region0_weight);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region1_excen, awb_cfg_v32->exc_wp_region1_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region1_measen, awb_cfg_v32->exc_wp_region1_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region1_domain, awb_cfg_v32->exc_wp_region1_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_weight = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region1_weight, awb_cfg_v32->exc_wp_region1_weight);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region2_excen, awb_cfg_v32->exc_wp_region2_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region2_measen, awb_cfg_v32->exc_wp_region2_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region2_domain, awb_cfg_v32->exc_wp_region2_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_weight = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region3_weight, awb_cfg_v32->exc_wp_region2_weight);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region3_excen, awb_cfg_v32->exc_wp_region3_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_measen = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region3_measen, awb_cfg_v32->exc_wp_region3_measen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region3_domain, awb_cfg_v32->exc_wp_region3_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_weight = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region3_weight, awb_cfg_v32->exc_wp_region3_weight);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region4_excen, awb_cfg_v32->exc_wp_region4_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region4_domain, awb_cfg_v32->exc_wp_region4_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_weight = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region4_weight, awb_cfg_v32->exc_wp_region4_weight);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region5_excen, awb_cfg_v32->exc_wp_region5_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region5_domain, awb_cfg_v32->exc_wp_region5_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_weight = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region5_weight, awb_cfg_v32->exc_wp_region5_weight);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_excen = 0x%0x (%d)\n",              awb_cfg_v32->exc_wp_region6_excen, awb_cfg_v32->exc_wp_region6_excen);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_domain = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region6_domain, awb_cfg_v32->exc_wp_region6_domain);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_weight = 0x%0x (%d)\n",             awb_cfg_v32->exc_wp_region6_weight, awb_cfg_v32->exc_wp_region6_weight);
    fprintf(fp, "\t\tsw_rawawb_multiwindow_en = 0x%0x (%d)\n",                    awb_cfg_v32->multiwindow_en, awb_cfg_v32->multiwindow_en);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_h_offs, awb_cfg_v32->multiwindow0_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_v_offs, awb_cfg_v32->multiwindow0_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow0_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow0_h_size, awb_cfg_v32->multiwindow0_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_size, awb_cfg_v32->multiwindow1_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_h_offs, awb_cfg_v32->multiwindow1_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_offs, awb_cfg_v32->multiwindow1_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_h_size, awb_cfg_v32->multiwindow1_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow1_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow1_v_size, awb_cfg_v32->multiwindow1_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_h_offs, awb_cfg_v32->multiwindow2_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_v_offs, awb_cfg_v32->multiwindow2_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_h_size, awb_cfg_v32->multiwindow2_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow2_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow2_v_size, awb_cfg_v32->multiwindow2_v_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_h_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_h_offs, awb_cfg_v32->multiwindow3_h_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_v_offs = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_v_offs, awb_cfg_v32->multiwindow3_v_offs);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_h_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_h_size, awb_cfg_v32->multiwindow3_h_size);
    fprintf(fp, "\t\tsw_rawawb_multiwindow3_v_size = 0x%0x (%d)\n",               awb_cfg_v32->multiwindow3_v_size, awb_cfg_v32->multiwindow3_v_size);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_xu0, awb_cfg_v32->exc_wp_region0_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_xu1, awb_cfg_v32->exc_wp_region0_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_yv0, awb_cfg_v32->exc_wp_region0_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region0_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region0_yv1, awb_cfg_v32->exc_wp_region0_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_xu0, awb_cfg_v32->exc_wp_region1_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_xu1, awb_cfg_v32->exc_wp_region1_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_yv0, awb_cfg_v32->exc_wp_region1_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region1_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region1_yv1, awb_cfg_v32->exc_wp_region1_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_xu0, awb_cfg_v32->exc_wp_region2_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_xu1, awb_cfg_v32->exc_wp_region2_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_yv0, awb_cfg_v32->exc_wp_region2_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region2_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region2_yv1, awb_cfg_v32->exc_wp_region2_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_xu0, awb_cfg_v32->exc_wp_region3_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_xu1, awb_cfg_v32->exc_wp_region3_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_yv0, awb_cfg_v32->exc_wp_region3_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region3_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region3_yv1, awb_cfg_v32->exc_wp_region3_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_xu0, awb_cfg_v32->exc_wp_region4_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_xu1, awb_cfg_v32->exc_wp_region4_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_yv0, awb_cfg_v32->exc_wp_region4_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region4_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region4_yv1, awb_cfg_v32->exc_wp_region4_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_xu0, awb_cfg_v32->exc_wp_region5_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_xu1, awb_cfg_v32->exc_wp_region5_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_yv0, awb_cfg_v32->exc_wp_region5_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region5_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region5_yv1, awb_cfg_v32->exc_wp_region5_yv1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_xu0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_xu0, awb_cfg_v32->exc_wp_region6_xu0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_xu1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_xu1, awb_cfg_v32->exc_wp_region6_xu1);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_yv0 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_yv0, awb_cfg_v32->exc_wp_region6_yv0);
    fprintf(fp, "\t\tsw_rawawb_exc_wp_region6_yv1 = 0x%0x (%d)\n",                awb_cfg_v32->exc_wp_region6_yv1, awb_cfg_v32->exc_wp_region6_yv1);
    fprintf(fp, "\t\tbls2 en(%d),val_rggb(%d,%d,%d,%d))\n",             awb_cfg_v32->bls2_en, awb_cfg_v32->bls2_val.r,
            awb_cfg_v32->bls2_val.gr, awb_cfg_v32->bls2_val.gb, awb_cfg_v32->bls2_val.b);

    fprintf(fp, "awb_cfg_v32->wp_blk_wei_w:");
    for(int i = 0; i < 225; i++) {
        if (i % 15 == 0) {
            fprintf(fp, "\n");
        }
        fprintf(fp, "0x%03x (%3d),", awb_cfg_v32->wp_blk_wei_w[i], awb_cfg_v32->wp_blk_wei_w[i]);

    }
    fprintf(fp, "\n");
    fclose(fp);
#endif
}

static void WriteDataForIcCmodel(struct isp39_rawawb_meas_cfg* wpDetectPara)
{
#if 0
    static int count = 0;
    if(count > 20) return;
    char fName[100];
    sprintf(fName, "./tmp/config_rkisp_demo_s_%d.txt", count);
    count++;
    LOGV_AWB( "%s", fName);
    FILE* fp = fopen(fName, "wb");
    if(fp != NULL)
    {
        //fprintf(fp, "g_awb_para.sw_img_width         =%d;\n",wpDetectPara-> );
        //fprintf(fp, "g_awb_para.sw_img_height        =%d;\n",wpDetectPara-> );
        //fprintf(fp, "g_awb_para.sw_img_depth         =%d;\n",wpDetectPara-> );
        //fprintf(fp, "g_awb_para.rawBayerPattern      =%d;\n",wpDetectPara-> );

        fprintf(fp, "g_awb_para.sw_rawawb_en                 =%d;\n", 1                 );
        fprintf(fp, "g_awb_para.sw_rawawb_xy_en0             =%d;\n", wpDetectPara->xy_en0             );
        fprintf(fp, "g_awb_para.sw_rawawb_uv_en0             =%d;\n", wpDetectPara->uv_en0             );
        fprintf(fp, "g_awb_para.sw_rawawb_3dyuv_en0          =%d;\n", wpDetectPara->yuv3d_en0          );
        fprintf(fp, "g_awb_para.sw_rawawb_xy_en1             =%d;\n", wpDetectPara->xy_en1             );
        fprintf(fp, "g_awb_para.sw_rawawb_uv_en1             =%d;\n", wpDetectPara->uv_en1             );
        fprintf(fp, "g_awb_para.sw_rawawb_3dyuv_en1          =%d;\n", wpDetectPara->yuv3d_en1          );
        fprintf(fp, "g_awb_para.sw_rawawb_wp_blk_wei_en0     =%d;\n", wpDetectPara->wp_blk_wei_en0     );
        fprintf(fp, "g_awb_para.sw_rawawb_wp_blk_wei_en1     =%d;\n", wpDetectPara->wp_blk_wei_en1     );
        fprintf(fp, "g_awb_para.sw_3a_rawawb_sel             =%d;\n", wpDetectPara->rawawb_sel             );
        fprintf(fp, "g_awb_para.sw_rawawb_blk_measure_enable =%d;\n", wpDetectPara->blk_measure_enable );
        fprintf(fp, "g_awb_para.sw_rawawb_blk_measure_mode   =%d;\n", wpDetectPara->blk_measure_mode   );
        fprintf(fp, "g_awb_para.sw_rawawb_blk_measure_xytype =%d;\n", wpDetectPara->blk_measure_xytype );
        fprintf(fp, "g_awb_para.sw_rawawb_blk_measure_illu_idx  =%d;\n", wpDetectPara->blk_measure_illu_idx );
        fprintf(fp, "g_awb_para.sw_rawawb_blk_with_luma_wei_en  =%d;\n", wpDetectPara->blk_with_luma_wei_en);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_wei_en0   =%d;\n", wpDetectPara->wp_luma_wei_en0 );
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_wei_en1   = %d;\n", wpDetectPara->wp_luma_wei_en1);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_hist_xytype    =%d;\n", wpDetectPara->wp_hist_xytype);
        fprintf(fp, "g_awb_para.sw_rawawb_3dyuv_ls_idx0    = %d;\n", wpDetectPara->yuv3d_ls_idx0);
        fprintf(fp, "g_awb_para.sw_rawawb_3dyuv_ls_idx1    = %d;\n", wpDetectPara->yuv3d_ls_idx1);
        fprintf(fp, "g_awb_para.sw_rawawb_3dyuv_ls_idx2    = %d;\n", wpDetectPara->yuv3d_ls_idx2);
        fprintf(fp, "g_awb_para.sw_rawawb_3dyuv_ls_idx3    = %d;\n", wpDetectPara->yuv3d_ls_idx3);
        fprintf(fp, "g_awb_para.sw_rawawb_meas_flag        = %d;\n", 0);
        fprintf(fp, "g_awb_para.sw_rawawb_light_num        = %d;\n", wpDetectPara->light_num);
        fprintf(fp, "g_awb_para.sw_rawawb_rd_ram_en        = %d;\n", 0);
        fprintf(fp, "g_awb_para.sw_rawawb_ram_offs         = %d;\n", 0);
        fprintf(fp, "g_awb_para.sw_rawawb_h_offs           = %d;\n", wpDetectPara->h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_v_offs           = %d;\n", wpDetectPara->v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_h_size           = %d;\n", wpDetectPara->h_size);
        fprintf(fp, "g_awb_para.sw_rawawb_v_size           = %d;\n", wpDetectPara->v_size);
        fprintf(fp, "g_awb_para.sw_rawawb_wind_size      = %d;\n", wpDetectPara->wind_size);
        fprintf(fp, "g_awb_para.sw_rawawb_ds16x8_mode_en = %d;\n", wpDetectPara->ds16x8_mode_en);
        fprintf(fp, "g_awb_para.sw_rawawb_r_max          = %d;\n", wpDetectPara->r_max);
        fprintf(fp, "g_awb_para.sw_rawawb_g_max          = %d;\n", wpDetectPara->g_max);
        fprintf(fp, "g_awb_para.sw_rawawb_b_max          = %d;\n", wpDetectPara->b_max);
        fprintf(fp, "g_awb_para.sw_rawawb_y_max          = %d;\n", wpDetectPara->y_max);
        fprintf(fp, "g_awb_para.sw_rawawb_r_min          = %d;\n", wpDetectPara->r_min);
        fprintf(fp, "g_awb_para.sw_rawawb_g_min          = %d;\n", wpDetectPara->g_min);
        fprintf(fp, "g_awb_para.sw_rawawb_b_min          = %d;\n", wpDetectPara->b_min);
        fprintf(fp, "g_awb_para.sw_rawawb_y_min          = %d;\n", wpDetectPara->y_min);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex0_u_0    =%d;\n", wpDetectPara->vertex0_u_0);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex0_v_0    =%d;\n", wpDetectPara->vertex0_v_0);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex1_u_0    =%d;\n", wpDetectPara->vertex1_u_0);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex1_v_0    =%d;\n", wpDetectPara->vertex1_v_0);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex2_u_0    =%d;\n", wpDetectPara->vertex2_u_0);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex2_v_0    =%d;\n", wpDetectPara->vertex2_v_0);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex3_u_0    =%d;\n", wpDetectPara->vertex3_u_0);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex3_v_0    =%d;\n", wpDetectPara->vertex3_v_0);
        fprintf(fp, "g_awb_para.sw_rawawb_islope01_0      =%d;\n", c2trval(31, wpDetectPara->islope01_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope12_0      =%d;\n", c2trval(31, wpDetectPara->islope12_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope23_0      =%d;\n", c2trval(31, wpDetectPara->islope23_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope30_0      =%d;\n", c2trval(31, wpDetectPara->islope30_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_vertex0_u_1     =%d;\n", wpDetectPara->vertex0_u_1);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex0_v_1     =%d;\n", wpDetectPara->vertex0_v_1);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex1_u_1     =%d;\n", wpDetectPara->vertex1_u_1);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex1_v_1     =%d;\n", wpDetectPara->vertex1_v_1);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex2_u_1     =%d;\n", wpDetectPara->vertex2_u_1);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex2_v_1     =%d;\n", wpDetectPara->vertex2_v_1);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex3_u_1     =%d;\n", wpDetectPara->vertex3_u_1);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex3_v_1     =%d;\n", wpDetectPara->vertex3_v_1);
        fprintf(fp, "g_awb_para.sw_rawawb_islope01_1      =%d;\n", c2trval(31, wpDetectPara->islope01_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope12_1      =%d;\n", c2trval(31, wpDetectPara->islope12_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope23_1      =%d;\n", c2trval(31, wpDetectPara->islope23_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope30_1      =%d;\n", c2trval(31, wpDetectPara->islope30_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_vertex0_u_2    =%d;\n", wpDetectPara->vertex0_u_2 );
        fprintf(fp, "g_awb_para.sw_rawawb_vertex0_v_2    =%d;\n", wpDetectPara->vertex0_v_2 );
        fprintf(fp, "g_awb_para.sw_rawawb_vertex1_u_2    =%d;\n", wpDetectPara->vertex1_u_2 );
        fprintf(fp, "g_awb_para.sw_rawawb_vertex1_v_2    =%d;\n", wpDetectPara->vertex1_v_2 );
        fprintf(fp, "g_awb_para.sw_rawawb_vertex2_u_2    =%d;\n", wpDetectPara->vertex2_u_2 );
        fprintf(fp, "g_awb_para.sw_rawawb_vertex2_v_2    =%d;\n", wpDetectPara->vertex2_v_2 );
        fprintf(fp, "g_awb_para.sw_rawawb_vertex3_u_2    =%d;\n", wpDetectPara->vertex3_u_2 );
        fprintf(fp, "g_awb_para.sw_rawawb_vertex3_v_2    =%d;\n", wpDetectPara->vertex3_v_2 );
        fprintf(fp, "g_awb_para.sw_rawawb_islope01_2      =%d;\n", c2trval(31, wpDetectPara->islope01_2   ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope12_2      =%d;\n", c2trval(31, wpDetectPara->islope12_2  ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope23_2      =%d;\n", c2trval(31, wpDetectPara->islope23_2  ));
        fprintf(fp, "g_awb_para.sw_rawawb_islope30_2      =%d;\n", c2trval(31, wpDetectPara->islope30_2  ));
        fprintf(fp, "g_awb_para.sw_rawawb_vertex0_u_3    = %d;\n", wpDetectPara->vertex0_u_3);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex0_v_3    = %d;\n", wpDetectPara->vertex0_v_3);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex1_u_3    = %d;\n", wpDetectPara->vertex1_u_3);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex1_v_3    = %d;\n", wpDetectPara->vertex1_v_3);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex2_u_3    = %d;\n", wpDetectPara->vertex2_u_3);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex2_v_3    = %d;\n", wpDetectPara->vertex2_v_3);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex3_u_3    = %d;\n", wpDetectPara->vertex3_u_3);
        fprintf(fp, "g_awb_para.sw_rawawb_vertex3_v_3    = %d;\n", wpDetectPara->vertex3_v_3);
        fprintf(fp, "g_awb_para.sw_rawawb_islope01_3      = %d;\n", c2trval(31, wpDetectPara->islope01_3));
        fprintf(fp, "g_awb_para.sw_rawawb_islope12_3      = %d;\n", c2trval(31, wpDetectPara->islope12_3));
        fprintf(fp, "g_awb_para.sw_rawawb_islope23_3      = %d;\n", c2trval(31, wpDetectPara->islope23_3));
        fprintf(fp, "g_awb_para.sw_rawawb_islope30_3      = %d;\n", c2trval(31, wpDetectPara->islope30_3));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat0_u = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat0_u));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat1_u = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat1_u));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat2_u = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat2_u));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvofs_u = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvofs_u));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat0_v = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat0_v));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat1_v = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat1_v));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat2_v = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat2_v));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvofs_v = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvofs_v));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat0_y = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat0_y));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat1_y = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat1_y));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvmat2_y = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvmat2_y));
        fprintf(fp, "g_awb_para.sw_rawawb_rgb2ryuvofs_y = %d;\n", c2trval(15, wpDetectPara->rgb2ryuvofs_y));
        fprintf(fp, "g_awb_para.sw_rawawb_rotu0_ls0 =%d;\n", wpDetectPara->rotu0_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu1_ls0 =%d;\n", wpDetectPara->rotu1_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu2_ls0 =%d;\n", wpDetectPara->rotu2_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu3_ls0 =%d;\n", wpDetectPara->rotu3_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu4_ls0 =%d;\n", wpDetectPara->rotu4_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu5_ls0 =%d;\n", wpDetectPara->rotu5_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_th0_ls0 =%d;\n", wpDetectPara->th0_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_th1_ls0 =%d;\n", wpDetectPara->th1_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_th2_ls0 =%d;\n", wpDetectPara->th2_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_th3_ls0 =%d;\n", wpDetectPara->th3_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_th4_ls0 =%d;\n", wpDetectPara->th4_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_th5_ls0 =%d;\n", wpDetectPara->th5_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls0_u =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls0_u));
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls0_v =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls0_v));
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls0_y =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls0_y));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls0_u =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls0_u));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls0_v =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls0_v));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls0_y =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls0_y));
        fprintf(fp, "g_awb_para.sw_rawawb_dis_x1x2_ls0 =%d;\n", wpDetectPara->dis_x1x2_ls0);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu0_ls1 =%d;\n", wpDetectPara->rotu0_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_rotu1_ls1 =%d;\n", wpDetectPara->rotu1_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_rotu2_ls1 =%d;\n", wpDetectPara->rotu2_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_rotu3_ls1 =%d;\n", wpDetectPara->rotu3_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_rotu4_ls1 =%d;\n", wpDetectPara->rotu4_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_rotu5_ls1 =%d;\n", wpDetectPara->rotu5_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_th0_ls1 =%d;\n", wpDetectPara->th0_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_th1_ls1 =%d;\n", wpDetectPara->th1_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_th2_ls1 =%d;\n", wpDetectPara->th2_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_th3_ls1 =%d;\n", wpDetectPara->th3_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_th4_ls1 =%d;\n", wpDetectPara->th4_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_th5_ls1 =%d;\n", wpDetectPara->th5_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls1_u =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls1_u ));
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls1_v =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls1_v ));
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls1_y =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls1_y ));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls1_u =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls1_u ));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls1_v =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls1_v ));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls1_y =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls1_y ));
        fprintf(fp, "g_awb_para.sw_rawawb_dis_x1x2_ls1 =%d;\n", wpDetectPara->dis_x1x2_ls1 );
        fprintf(fp, "g_awb_para.sw_rawawb_rotu0_ls2 =%d;\n", wpDetectPara->rotu0_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu1_ls2 =%d;\n", wpDetectPara->rotu1_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu2_ls2 =%d;\n", wpDetectPara->rotu2_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu3_ls2 =%d;\n", wpDetectPara->rotu3_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu4_ls2 =%d;\n", wpDetectPara->rotu4_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu5_ls2 =%d;\n", wpDetectPara->rotu5_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_th0_ls2 =%d;\n", wpDetectPara->th0_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_th1_ls2 =%d;\n", wpDetectPara->th1_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_th2_ls2 =%d;\n", wpDetectPara->th2_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_th3_ls2 =%d;\n", wpDetectPara->th3_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_th4_ls2 =%d;\n", wpDetectPara->th4_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_th5_ls2 =%d;\n", wpDetectPara->th5_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls2_u =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls2_u));
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls2_v =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls2_v));
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls2_y =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls2_y));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls2_u =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls2_u));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls2_v =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls2_v));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls2_y =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls2_y));
        fprintf(fp, "g_awb_para.sw_rawawb_dis_x1x2_ls2 =%d;\n", wpDetectPara->dis_x1x2_ls2);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu0_ls3 =%d;\n", wpDetectPara->rotu0_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu1_ls3 =%d;\n", wpDetectPara->rotu1_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu2_ls3 =%d;\n", wpDetectPara->rotu2_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu3_ls3 =%d;\n", wpDetectPara->rotu3_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu4_ls3 =%d;\n", wpDetectPara->rotu4_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_rotu5_ls3 =%d;\n", wpDetectPara->rotu5_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_th0_ls3 =%d;\n", wpDetectPara->th0_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_th1_ls3 =%d;\n", wpDetectPara->th1_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_th2_ls3 =%d;\n", wpDetectPara->th2_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_th3_ls3 =%d;\n", wpDetectPara->th3_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_th4_ls3 =%d;\n", wpDetectPara->th4_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_th5_ls3 =%d;\n", wpDetectPara->th5_ls3);
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls3_u =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls3_u));
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls3_v =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls3_v));
        fprintf(fp, "g_awb_para.sw_rawawb_coor_x1_ls3_y =%d;\n", c2trval(15, wpDetectPara->coor_x1_ls3_y));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls3_u =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls3_u));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls3_v =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls3_v));
        fprintf(fp, "g_awb_para.sw_rawawb_vec_x21_ls3_y =%d;\n", c2trval(15, wpDetectPara->vec_x21_ls3_y));
        fprintf(fp, "g_awb_para.sw_rawawb_dis_x1x2_ls3 =%d;\n", c2trval(7, wpDetectPara->dis_x1x2_ls3));
        fprintf(fp, "g_awb_para.sw_rawawb_wt0            =%d;\n", wpDetectPara->wt0      );
        fprintf(fp, "g_awb_para.sw_rawawb_wt1            =%d;\n", wpDetectPara->wt1      );
        fprintf(fp, "g_awb_para.sw_rawawb_wt2            =%d;\n", wpDetectPara->wt2      );
        fprintf(fp, "g_awb_para.sw_rawawb_mat0_x         =%d;\n", c2trval(15, wpDetectPara->mat0_x   ));
        fprintf(fp, "g_awb_para.sw_rawawb_mat1_x         =%d;\n", c2trval(15, wpDetectPara->mat1_x   ));
        fprintf(fp, "g_awb_para.sw_rawawb_mat2_x         =%d;\n", c2trval(15, wpDetectPara->mat2_x   ));
        fprintf(fp, "g_awb_para.sw_rawawb_mat0_y         =%d;\n", c2trval(15, wpDetectPara->mat0_y   ));
        fprintf(fp, "g_awb_para.sw_rawawb_mat1_y         =%d;\n", c2trval(15, wpDetectPara->mat1_y   ));
        fprintf(fp, "g_awb_para.sw_rawawb_mat2_y         =%d;\n", c2trval(15, wpDetectPara->mat2_y   ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_x0_0       =%d;\n", c2trval(15, wpDetectPara->nor_x0_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_x1_0       =%d;\n", c2trval(15, wpDetectPara->nor_x1_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_y0_0       =%d;\n", c2trval(15, wpDetectPara->nor_y0_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_y1_0       =%d;\n", c2trval(15, wpDetectPara->nor_y1_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_x0_0       =%d;\n", c2trval(15, wpDetectPara->big_x0_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_x1_0       =%d;\n", c2trval(15, wpDetectPara->big_x1_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_y0_0       =%d;\n", c2trval(15, wpDetectPara->big_y0_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_y1_0       =%d;\n", c2trval(15, wpDetectPara->big_y1_0 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_x0_1       =%d;\n", c2trval(15, wpDetectPara->nor_x0_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_x1_1       =%d;\n", c2trval(15, wpDetectPara->nor_x1_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_y0_1       =%d;\n", c2trval(15, wpDetectPara->nor_y0_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_y1_1       =%d;\n", c2trval(15, wpDetectPara->nor_y1_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_x0_1       =%d;\n", c2trval(15, wpDetectPara->big_x0_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_x1_1       =%d;\n", c2trval(15, wpDetectPara->big_x1_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_y0_1       =%d;\n", c2trval(15, wpDetectPara->big_y0_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_y1_1       =%d;\n", c2trval(15, wpDetectPara->big_y1_1 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_x0_2       =%d;\n", c2trval(15, wpDetectPara->nor_x0_2 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_x1_2       =%d;\n", c2trval(15, wpDetectPara->nor_x1_2 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_y0_2       =%d;\n", c2trval(15, wpDetectPara->nor_y0_2 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_y1_2       =%d;\n", c2trval(15, wpDetectPara->nor_y1_2 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_x0_2       =%d;\n", c2trval(15, wpDetectPara->big_x0_2 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_x1_2       =%d;\n", c2trval(15, wpDetectPara->big_x1_2 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_y0_2       =%d;\n", c2trval(15, wpDetectPara->big_y0_2 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_y1_2       =%d;\n", c2trval(15, wpDetectPara->big_y1_2 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_x0_3       =%d;\n", c2trval(15, wpDetectPara->nor_x0_3 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_x1_3       =%d;\n", c2trval(15, wpDetectPara->nor_x1_3 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_y0_3       =%d;\n", c2trval(15, wpDetectPara->nor_y0_3 ));
        fprintf(fp, "g_awb_para.sw_rawawb_nor_y1_3       =%d;\n", c2trval(15, wpDetectPara->nor_y1_3 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_x0_3       =%d;\n", c2trval(15, wpDetectPara->big_x0_3 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_x1_3       =%d;\n", c2trval(15, wpDetectPara->big_x1_3 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_y0_3       =%d;\n", c2trval(15, wpDetectPara->big_y0_3 ));
        fprintf(fp, "g_awb_para.sw_rawawb_big_y1_3       =%d;\n", c2trval(15, wpDetectPara->big_y1_3 ));
        fprintf(fp, "g_awb_para.sw_rawawb_pre_wbgain_inv_r =%d;\n", wpDetectPara->pre_wbgain_inv_r);
        fprintf(fp, "g_awb_para.sw_rawawb_pre_wbgain_inv_g =%d;\n", wpDetectPara->pre_wbgain_inv_g);
        fprintf(fp, "g_awb_para.sw_rawawb_pre_wbgain_inv_b =%d;\n", wpDetectPara->pre_wbgain_inv_b);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow_en =%d;\n", wpDetectPara->multiwindow_en);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow0_h_offs =%d;\n", wpDetectPara->multiwindow0_h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow0_v_offs =%d;\n", wpDetectPara->multiwindow0_v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow0_h_size =%d;//note:diff with chip\n", wpDetectPara->multiwindow0_h_size - wpDetectPara->multiwindow0_h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow0_v_size =%d;\n", wpDetectPara->multiwindow0_v_size - wpDetectPara->multiwindow0_v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow1_h_offs =%d;\n", wpDetectPara->multiwindow1_h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow1_v_offs =%d;\n", wpDetectPara->multiwindow1_v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow1_h_size =%d;\n", wpDetectPara->multiwindow1_h_size - wpDetectPara->multiwindow1_h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow1_v_size =%d;\n", wpDetectPara->multiwindow1_v_size - wpDetectPara->multiwindow1_v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow2_h_offs =%d;\n", wpDetectPara->multiwindow2_h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow2_v_offs =%d;\n", wpDetectPara->multiwindow2_v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow2_h_size =%d;\n", wpDetectPara->multiwindow2_h_size - wpDetectPara->multiwindow2_h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow2_v_size =%d;\n", wpDetectPara->multiwindow2_v_size - wpDetectPara->multiwindow2_v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow3_h_offs =%d;\n", wpDetectPara->multiwindow3_h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow3_v_offs =%d;\n", wpDetectPara->multiwindow3_v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow3_h_size =%d;\n", wpDetectPara->multiwindow3_h_size - wpDetectPara->multiwindow3_h_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_multiwindow3_v_size =%d;\n", wpDetectPara->multiwindow3_v_size - wpDetectPara->multiwindow3_v_offs);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_excen0 =%d;\n", wpDetectPara->exc_wp_region0_excen & 0x1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_excen1 =%d;\n", wpDetectPara->exc_wp_region0_excen & 0x2 >> 1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_measen =%d;\n", wpDetectPara->exc_wp_region0_measen);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_domain =%d;\n", wpDetectPara->exc_wp_region0_domain);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_weight =%d;\n", wpDetectPara->exc_wp_region0_weight);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_xu0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region0_xu0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_xu1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region0_xu1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_yv0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region0_yv0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region0_yv1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region0_yv1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_excen0 =%d;\n", wpDetectPara->exc_wp_region1_excen & 0x1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_excen1 =%d;\n", wpDetectPara->exc_wp_region1_excen & 0x2 >> 1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_measen =%d;\n", wpDetectPara->exc_wp_region1_measen);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_domain =%d;\n", wpDetectPara->exc_wp_region1_domain);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_weight =%d;\n", wpDetectPara->exc_wp_region1_weight);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_xu0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region1_xu0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_xu1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region1_xu1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_yv0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region1_yv0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region1_yv1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region1_yv1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_excen0 =%d;\n", wpDetectPara->exc_wp_region2_excen & 0x1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_excen1 =%d;\n", wpDetectPara->exc_wp_region2_excen & 0x2 >> 1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_measen =%d;\n", wpDetectPara->exc_wp_region2_measen);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_domain =%d;\n", wpDetectPara->exc_wp_region2_domain);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_weight =%d;\n", wpDetectPara->exc_wp_region2_weight);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_xu0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region2_xu0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_xu1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region2_xu1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_yv0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region2_yv0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region2_yv1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region2_yv1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_excen0 =%d;\n", wpDetectPara->exc_wp_region3_excen & 0x1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_excen1 =%d;\n", wpDetectPara->exc_wp_region3_excen & 0x2 >> 1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_measen =%d;\n", wpDetectPara->exc_wp_region3_measen);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_domain =%d;\n", wpDetectPara->exc_wp_region3_domain);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_weight =%d;\n", wpDetectPara->exc_wp_region3_weight);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_xu0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region3_xu0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_xu1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region3_xu1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_yv0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region3_yv0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region3_yv1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region3_yv1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_excen0 =%d;\n", wpDetectPara->exc_wp_region4_excen & 0x1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_excen1 =%d;\n", wpDetectPara->exc_wp_region4_excen & 0x2 >> 1);
        //fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_measen =%d;\n", wpDetectPara->exc_wp_region4_measen);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_domain =%d;\n", wpDetectPara->exc_wp_region4_domain);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_weight =%d;\n", wpDetectPara->exc_wp_region4_weight);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_xu0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region4_xu0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_xu1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region4_xu1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_yv0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region4_yv0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region4_yv1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region4_yv1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_excen0 =%d;\n", wpDetectPara->exc_wp_region5_excen & 0x1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_excen1 =%d;\n", wpDetectPara->exc_wp_region5_excen & 0x2 >> 1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_weight =%d;\n", wpDetectPara->exc_wp_region5_weight);
        //fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_measen =%d;\n", wpDetectPara->exc_wp_region5_measen);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_domain =%d;\n", wpDetectPara->exc_wp_region5_domain);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_xu0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region5_xu0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_xu1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region5_xu1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_yv0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region5_yv0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region5_yv1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region5_yv1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_excen0 =%d;\n", wpDetectPara->exc_wp_region6_excen & 0x1);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_excen1 =%d;\n", wpDetectPara->exc_wp_region6_excen & 0x2 >> 1);
        //fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_measen =%d;\n", wpDetectPara->exc_wp_region6_measen);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_domain =%d;\n", wpDetectPara->exc_wp_region6_domain);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_weight =%d;\n", wpDetectPara->exc_wp_region6_weight);
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_xu0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region6_xu0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_xu1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region6_xu1));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_yv0 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region6_yv0));
        fprintf(fp, "g_awb_para.sw_rawawb_exc_wp_region6_yv1 =%d;\n", c2trval(15, wpDetectPara->exc_wp_region6_yv1));

        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y0 =%d;\n", wpDetectPara->wp_luma_weicurve_y0);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y1 =%d;\n", wpDetectPara->wp_luma_weicurve_y1);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y2 =%d;\n", wpDetectPara->wp_luma_weicurve_y2);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y3 =%d;\n", wpDetectPara->wp_luma_weicurve_y3);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y4 =%d;\n", wpDetectPara->wp_luma_weicurve_y4);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y5 =%d;\n", wpDetectPara->wp_luma_weicurve_y5);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y6 =%d;\n", wpDetectPara->wp_luma_weicurve_y6);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y7 =%d;\n", wpDetectPara->wp_luma_weicurve_y7);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_y8 =%d;\n", wpDetectPara->wp_luma_weicurve_y8);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w0 =%d;\n", wpDetectPara->wp_luma_weicurve_w0);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w1 =%d;\n", wpDetectPara->wp_luma_weicurve_w1);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w2 =%d;\n", wpDetectPara->wp_luma_weicurve_w2);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w3 =%d;\n", wpDetectPara->wp_luma_weicurve_w3);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w4 =%d;\n", wpDetectPara->wp_luma_weicurve_w4);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w5 =%d;\n", wpDetectPara->wp_luma_weicurve_w5);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w6 =%d;\n", wpDetectPara->wp_luma_weicurve_w6);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w7 =%d;\n", wpDetectPara->wp_luma_weicurve_w7);
        fprintf(fp, "g_awb_para.sw_rawawb_wp_luma_weicurve_w8 =%d;\n", wpDetectPara->wp_luma_weicurve_w8);

        for (int i = 0; i < ISP39_RAWAWB_WEIGHT_NUM; i++) {
            fprintf(fp, "g_awb_para.sw_rawawb_wp_blk_wei_w[%d]    =%d;\n", i, wpDetectPara->wp_blk_wei_w[i] );
        }

        fprintf(fp, "g_awb_para.sw_rawawb_blk_rtdw_measure_en =%d;\n", wpDetectPara->blk_rtdw_measure_en);
        fprintf(fp, "g_awb_para.sw_rawawb_rshift_to_12bit_en =%d;\n", wpDetectPara->in_rshift_to_12bit_en);
        fprintf(fp, "g_awb_para.sw_rawawb_in_overexposure_check_en = %d;\n", wpDetectPara->in_overexposure_check_en);
        fprintf(fp, "g_awb_para.sw_rawawb_in_overexposure_thr =%d;\n", wpDetectPara->in_overexposure_threshold);
        fprintf(fp, "g_awb_para.esw_rawawbin_low12bit_val =%d;\n", wpDetectPara->low12bit_val);
        fprintf(fp, "g_awb_para.sw_drc2awb_sel_en =%d;\n", wpDetectPara->drc2awb_sel);
        fprintf(fp, "g_awb_para.sw_bnr2awb_sel_en =%d;\n", wpDetectPara->bnr2awb_sel);
        fprintf(fp, "g_awb_para.sw_3a_rawawb_sel =%d;\n", wpDetectPara->rawawb_sel);
        fprintf(fp, "g_awb_para.sw_bls_en =%d;\n", wpDetectPara->bls2_en);
        fprintf(fp, "g_awb_para.sw_bls_a_fixed =%d;\n", wpDetectPara->bls2_val.r);
        fprintf(fp, "g_awb_para.sw_bls_b_fixed =%d;\n", wpDetectPara->bls2_val.gr);
        fprintf(fp, "g_awb_para.sw_bls_c_fixed =%d;\n", wpDetectPara->bls2_val.gb);
        fprintf(fp, "g_awb_para.sw_bls_d_fixed =%d;\n", wpDetectPara->bls2_val.b);


        fclose(fp);
    }

#endif
}

static void calcInputBitIs12Bit(uint8_t* inputBitIs12Bit, const awbStats_cfg_t* awb_meas,
                                int working_mode, bool ablc_en, float isp_ob_predgain) {
    // inputShiftEnable is true for 20bit ipnut ,for CALIB_AWB_INPUT_BAYERNR + (hdr case  or
    // dgainInAwbGain) is enbale case
    // to do  inputShiftEnable = !inputBitIs12Bit;
    bool selectBayerNrData = (awb_meas->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode);
    bool dgainEn           = ablc_en && isp_ob_predgain > 1;
    if (selectBayerNrData &&
            (dgainEn || (rk_aiq_working_mode_t)working_mode != RK_AIQ_WORKING_MODE_NORMAL)) {
        *inputBitIs12Bit = false;
    } else {
        *inputBitIs12Bit = true;
    }
}

static void ConfigBlc2(const blc_res_cvt_t* ablc, const awbStats_cfg_t* awb_meas, float isp_dgain,
                       float isp_ob_predgain, int working_mode,
                       struct isp39_rawawb_meas_cfg* awb_cfg_v32) {
    awb_cfg_v32->bls2_en     = false;
    awb_cfg_v32->bls2_val.r  = 0;
    awb_cfg_v32->bls2_val.gr = 0;
    awb_cfg_v32->bls2_val.gb = 0;
    awb_cfg_v32->bls2_val.b  = 0;
    if (awb_meas->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode ||
            (awb_meas->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode &&
             working_mode != RK_AIQ_WORKING_MODE_NORMAL)) {
        LOGW_AWB("The BLC bias of the awbStats pathway is not effective in the current mode and is consistent with the main pathway");
        // don't support to use blc2
        return;
    }
    awb_cfg_v32->bls2_en   = true;
    awbStats_blc_t awbBls2 = awb_meas->blc;
    if (awb_meas->blc.hw_awbCfg_blc_en == false) {
        memset(&awbBls2, 0, sizeof(awbBls2));
    }
    float blc1[AWB_CHANNEL_MAX] = {0, 0, 0, 0};
    float dgain2                = 1.0;
    // sw_blcT_obcPostTnr_mode == blc_autoOBCPostTnr_mode : use oboffset only
    // sw_blcT_obcPostTnr_mode == blc_manualOBCPostTnr_mode : use blc1 only
    if (ablc && ablc->en) {
        if (isp_ob_predgain > 1) {
            dgain2 = isp_ob_predgain;
        }
        // if (ablc->obcPostTnr.sw_blcT_obcPostTnr_en &&
        //     ablc->obcPostTnr.sw_blcT_obcPostTnr_mode == blc_manualOBCPostTnr_mode) {
        //     blc1[AWB_CHANNEL_R] =
        //         (float)ablc->obcPostTnr
        //             .hw_blcT_manualOBR_val;  // check blc1 was already multiplied by isp_dgain?
        //     blc1[AWB_CHANNEL_GR] = (float)ablc->obcPostTnr.hw_blcT_manualOBGr_val;
        //     blc1[AWB_CHANNEL_GB] = (float)ablc->obcPostTnr.hw_blcT_manualOBGb_val;
        //     blc1[AWB_CHANNEL_B]  = (float)ablc->obcPostTnr.hw_blcT_manualOBB_val;
        // }
    }
    // 2 blc2 recalc base on ablc
    if (awb_meas->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode) {
        // update by (offset +blc1)*dgain
        // working_mode = normal,so   applyPosition = IN_AWBGAIN1
        awb_cfg_v32->bls2_val.r =
            (awbBls2.hw_awbCfg_obR_val + blc1[AWB_CHANNEL_R]) * isp_dgain * dgain2 + 0.5;
        awb_cfg_v32->bls2_val.b =
            (awbBls2.hw_awbCfg_obB_val + blc1[AWB_CHANNEL_B]) * isp_dgain * dgain2 + 0.5;
        awb_cfg_v32->bls2_val.gr =
            (awbBls2.hw_awbCfg_obGr_val + blc1[AWB_CHANNEL_GR]) * isp_dgain * dgain2 + 0.5;
        awb_cfg_v32->bls2_val.gb =
            (awbBls2.hw_awbCfg_obGb_val + blc1[AWB_CHANNEL_GB]) * isp_dgain * dgain2 + 0.5;
    } else {  // select raw
        // update by offset +blc1
        awb_cfg_v32->bls2_val.r  = (awbBls2.hw_awbCfg_obR_val + blc1[AWB_CHANNEL_R]) + 0.5;
        awb_cfg_v32->bls2_val.b  = (awbBls2.hw_awbCfg_obB_val + blc1[AWB_CHANNEL_B]) + 0.5;
        awb_cfg_v32->bls2_val.gr = (awbBls2.hw_awbCfg_obGr_val + blc1[AWB_CHANNEL_GR]) + 0.5;
        awb_cfg_v32->bls2_val.gb = (awbBls2.hw_awbCfg_obGb_val + blc1[AWB_CHANNEL_GB]) + 0.5;
    }
}

// call after blc2 calc
static XCamReturn ConfigOverexposureValue(const blc_res_cvt_t* ablc, float hdrmge_gain0_1,
        float isp_ob_predgain, int working_mode,
        const awbStats_cfg_t* awb_meas,
        struct isp39_rawawb_meas_cfg* awb_cfg_v32) {
    XCamReturn ret                        = XCAM_RETURN_NO_ERROR;
    awb_cfg_v32->in_overexposure_check_en = true;
    float dgain                           = 1.0;
    int max_blc                           = 0;
    short ob                              = 0;
    int overexposure_value                = 254;
    /*if (ablc)
        ob = ablc->isp_ob_offset;*/
    if (awb_meas->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode) {
        awb_cfg_v32->in_overexposure_threshold = overexposure_value;
    } else if (awb_meas->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode) {
        if (ablc && ablc->en && isp_ob_predgain > 1) {
            dgain = isp_ob_predgain;
        }
        if (working_mode != RK_AIQ_WORKING_MODE_NORMAL) {
            // hdr mode,awbgain0 will reset the out data to full range
            LOGV_AWB("hdrmge_gain0_1 %f", hdrmge_gain0_1);
            awb_cfg_v32->in_overexposure_threshold = overexposure_value * hdrmge_gain0_1 * dgain;
        } else {
            // need check
            short int blc[AWB_CHANNEL_MAX] = {0, 0, 0, 0};
            if (ablc && ablc->en) {
                blc[AWB_CHANNEL_R]  = ablc->obcPreTnr.hw_blcC_obR_val * dgain - ob;
                blc[AWB_CHANNEL_GR] = ablc->obcPreTnr.hw_blcC_obGr_val * dgain - ob;
                blc[AWB_CHANNEL_GB] = ablc->obcPreTnr.hw_blcC_obGb_val * dgain - ob;
                blc[AWB_CHANNEL_B]  = ablc->obcPreTnr.hw_blcC_obB_val * dgain - ob;
            }
            for (int i = 0; i < AWB_CHANNEL_MAX; i++) {
                if (blc[i] < 0) {
                    blc[i] = 0;
                }
            }
            blc[AWB_CHANNEL_B] += awb_cfg_v32->bls2_val.b + ob;
            blc[AWB_CHANNEL_R] += awb_cfg_v32->bls2_val.r + ob;
            blc[AWB_CHANNEL_GB] += awb_cfg_v32->bls2_val.gb + ob;
            blc[AWB_CHANNEL_GR] += awb_cfg_v32->bls2_val.gr + ob;
            for (int i = 0; i < AWB_CHANNEL_MAX; i++) {
                if (blc[i] > max_blc) {
                    max_blc = blc[i];
                }
            }
            if (max_blc < 0) {
                max_blc = 0;
            }
            awb_cfg_v32->in_overexposure_threshold = overexposure_value * dgain - max_blc / 16;
        }

    } else {
        // raw
        // need check
        short int blc[AWB_CHANNEL_MAX] = {0, 0, 0, 0};
        if (ablc && ablc->en) {
            blc[AWB_CHANNEL_R]  = ablc->obcPreTnr.hw_blcC_obR_val - ob;
            blc[AWB_CHANNEL_GR] = ablc->obcPreTnr.hw_blcC_obGr_val - ob;
            blc[AWB_CHANNEL_GB] = ablc->obcPreTnr.hw_blcC_obGb_val - ob;
            blc[AWB_CHANNEL_B]  = ablc->obcPreTnr.hw_blcC_obB_val - ob;
        }

        for (int i = 0; i < AWB_CHANNEL_MAX; i++) {
            if (blc[i] < 0) {
                blc[i] = 0;
            }
        }
        blc[AWB_CHANNEL_B] += awb_cfg_v32->bls2_val.b + ob;
        blc[AWB_CHANNEL_R] += awb_cfg_v32->bls2_val.r + ob;
        blc[AWB_CHANNEL_GB] += awb_cfg_v32->bls2_val.gb + ob;
        blc[AWB_CHANNEL_GR] += awb_cfg_v32->bls2_val.gr + ob;
        for (int i = 0; i < AWB_CHANNEL_MAX; i++) {
            if (blc[i] > max_blc) {
                max_blc = blc[i];
            }
        }
        if (max_blc < 0) {
            max_blc = 0;
        }
        awb_cfg_v32->in_overexposure_threshold = overexposure_value - max_blc / 16;
    }
    return (ret);
}

static void UvParaFixed32(const awbStats_uvRegion_t* wpRegion, unsigned short pu_region[5],
                          unsigned short pv_region[5], int slope_inv[4]) {
    // unsigned short pu_region[5];//12 bit
    // unsigned short pv_region[5];
    // int slope_inv[4];//10+10 signed 1/k
    for (int j = 0; j < 5; j++) {
        if (j == 4) {
            pu_region[4] = pu_region[0];
            pv_region[4] = pv_region[0];
        } else {
            pu_region[j] = (uint16_t)(wpRegion->regionVtx[j].hw_awbT_vtxU_val * 16 + 0.5);
            pv_region[j] = (uint16_t)(wpRegion->regionVtx[j].hw_awbT_vtxV_val * 16 + 0.5);
        }
        if (j > 0 && j < 5) {
            int tmp = pv_region[j] - pv_region[j - 1];
            if (tmp != 0) {
                float tmp2       = (float)(pu_region[j] - pu_region[j - 1]) / (float)tmp;
                slope_inv[j - 1] = tmp2 > 0
                                   ? (int32_t)(tmp2 * (1 << AWB_UV_RANGE_SLOVE_FRACTION) + 0.5)
                                   : (int32_t)(tmp2 * (1 << AWB_UV_RANGE_SLOVE_FRACTION) - 0.5);
            } else {
                slope_inv[j - 1] =
                    (1 << (AWB_UV_RANGE_SLOVE_FRACTION + AWB_UV_RANGE_SLOVE_SINTER - 1)) - 1;
            }
        }
    }
}

static void ConfigPreWbgain3(struct isp39_rawawb_meas_cfg* awb_cfg_v32,
                             const awbStats_cfg_priv_t* awb_meas,
                             const rk_aiq_wb_gain_v32_t* awb_gain,
                             const struct rkmodule_awb_inf* otp_awb) {
    // higher priority than ConfigPreWbgain2
    // call after wbgain  and frameChoose calculation,
    // 1) awb statistics before the awbgain application:
    //    prewbgain = iqMap2MainCam.wbgain
    // 2) awb statistics after the awbgain application:
    //    prewbgain =1/stat3aAwbGainOut *iqMap2MainCam.wbgain
    float preWbgainSw[4];
    if (awb_meas->mode == AWB_CFG_MODE_RK &&
            (awb_meas->preWbgainSw[0] * awb_meas->preWbgainSw[1] * awb_meas->preWbgainSw[2] *
             awb_meas->preWbgainSw[3] >
             0.0001)) {
        preWbgainSw[0] = awb_meas->preWbgainSw[0];
        preWbgainSw[1] = awb_meas->preWbgainSw[1];
        preWbgainSw[2] = awb_meas->preWbgainSw[2];
        preWbgainSw[3] = awb_meas->preWbgainSw[3];
    } else {
        if (awb_meas->mode == AWB_CFG_MODE_RK) {
            LOGE_AWB("%s, wrong awb_meas->preWbgainSw!!! ", __FUNCTION__);
        }
        preWbgainSw[0] = 1;
        preWbgainSw[1] = 1;
        preWbgainSw[2] = 1;
        preWbgainSw[3] = 1;
    }

    // pre_wbgain_inv_b is updating  for awb0-gain enable+select = bnr/hdrc case
    if ((awb_gain->rgain * awb_gain->grgain * awb_gain->bgain > 0.0001) &&
            (awb_meas->com.hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode ||
             (awb_gain->applyPosition == IN_AWBGAIN0 &&
              awb_meas->com.hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode))) {
        // for awb statistics after the awbgain application
        awb_cfg_v32->pre_wbgain_inv_r =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) / awb_gain->rgain * preWbgainSw[0] + 0.5;
        awb_cfg_v32->pre_wbgain_inv_g =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) / awb_gain->grgain * preWbgainSw[1] + 0.5;
        awb_cfg_v32->pre_wbgain_inv_b =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) / awb_gain->bgain * preWbgainSw[3] + 0.5;

    } else {
        if ((awb_meas->com.hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode ||
                (awb_gain->applyPosition == IN_AWBGAIN0 &&
                 awb_meas->com.hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode))) {
            LOGE_AWB("%s, wrong awb_gain!!! ", __FUNCTION__);
        }
        // for awb statistics before the awbgain application
        awb_cfg_v32->pre_wbgain_inv_r =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * preWbgainSw[0] + 0.5;
        awb_cfg_v32->pre_wbgain_inv_g =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * preWbgainSw[1] + 0.5;
        awb_cfg_v32->pre_wbgain_inv_b =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * preWbgainSw[3] + 0.5;
    }

    if (otp_awb != NULL && otp_awb->flag) {
        LOGD_AWB("%s before otp pre_wbgain_inv = [%d, %d, %d];", __FUNCTION__,
                 awb_cfg_v32->pre_wbgain_inv_r, awb_cfg_v32->pre_wbgain_inv_g,
                 awb_cfg_v32->pre_wbgain_inv_b);
        awb_cfg_v32->pre_wbgain_inv_r = (float)awb_cfg_v32->pre_wbgain_inv_r *
                                        (float)otp_awb->golden_r_value /
                                        (float)otp_awb->r_value +
                                        0.5;
        awb_cfg_v32->pre_wbgain_inv_g = (float)awb_cfg_v32->pre_wbgain_inv_g *
                                        (float)(otp_awb->golden_gr_value + 1024) /
                                        (float)(otp_awb->gr_value + 1024) +
                                        0.5;
        awb_cfg_v32->pre_wbgain_inv_b = (float)awb_cfg_v32->pre_wbgain_inv_b *
                                        (float)otp_awb->golden_b_value /
                                        (float)otp_awb->b_value +
                                        0.5;
        LOGD_AWB("otp pre_wbgain_inv = [%d, %d, %d] \n", awb_cfg_v32->pre_wbgain_inv_r,
                 awb_cfg_v32->pre_wbgain_inv_g, awb_cfg_v32->pre_wbgain_inv_b);
    }
}

static bool isAwbCfgVaild(const rk_aiq_isp_awb_meas_cfg_v39_t* awb_meas_priv,
                          const common_cvt_info_t* mCommonCvtInfo) {
    const awbStats_cfg_t* wpDetectPara = (awbStats_cfg_t*)awb_meas_priv;
    if (wpDetectPara->mainWin.hw_awbCfg_win_x > mCommonCvtInfo->rawWidth ||
            wpDetectPara->mainWin.hw_awbCfg_win_x + wpDetectPara->mainWin.hw_awbCfg_win_width >
            mCommonCvtInfo->rawWidth ||
            wpDetectPara->mainWin.hw_awbCfg_win_y > mCommonCvtInfo->rawHeight ||
            wpDetectPara->mainWin.hw_awbCfg_win_y + wpDetectPara->mainWin.hw_awbCfg_win_height >
            mCommonCvtInfo->rawHeight) {
        LOGE_AWB("mainWin[%d,%d,%d,%d]is invaild!!!\n", wpDetectPara->mainWin.hw_awbCfg_win_x,
                 wpDetectPara->mainWin.hw_awbCfg_win_y, wpDetectPara->mainWin.hw_awbCfg_win_width,
                 wpDetectPara->mainWin.hw_awbCfg_win_height);
        return (false);
    }
    // do do dsmode correct + mainwin divisible by 16*8
    return (true);
}

static bool srcChooseCheck(awbStats_src_mode_t srcChoose, int working_mode) {
    bool flag = true;
    if ((rk_aiq_working_mode_t)working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        if (srcChoose == awbStats_chl1DegamOut_mode || srcChoose == awbStats_chl2DegamOut_mode) {
            flag = false;
        }
    } else if ((rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
               (rk_aiq_working_mode_t)working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3) {
        if (srcChoose == awbStats_chl2DegamOut_mode) {
            flag = false;
        }
    }
    if (flag == false) {
        LOGE_AWB("srcChoose %d is invaild!!!\n", srcChoose);
    }
    return flag;
}

static int filtOutFixed(float fval, awb_wpSpace_mode_t hw_awbT_wpSpace_mode)
{
    if(hw_awbT_wpSpace_mode == awbStats_xyWp_mode)
        return UtlFloatToFix_S0310(fval);
    else
        return (uint16_t)(fval * 16 + 0.5);
}

static void convertAiqAwbToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    LOG1_AWB("%s enter", __FUNCTION__);
    pCvt->mAwbParams = (aiq_params_base_t*)pBase;
    const rk_aiq_isp_awb_meas_cfg_v39_t* awb_meas_priv =
        (rk_aiq_isp_awb_meas_cfg_v39_t*)pBase->_data;
    const awbStats_cfg_t* awb_meas = &awb_meas_priv->com;
    if (isAwbCfgVaild(awb_meas_priv, &pCvt->mCommonCvtInfo) == false ||
            srcChooseCheck(awb_meas->hw_awbCfg_statsSrc_mode, pCvt->_working_mode) == false) {
        return;
    }

    // TODO: update by awb_cfg_udpate+awb_gain_update+isp_dgain+hdrmge_gain0_1+blc_res+otp
    if (1) {
        if (awb_meas->hw_awbCfg_stats_en) {
            pCvt->isp_params.isp_cfg->module_ens |= ISP39_MODULE_RAWAWB;
            pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_RAWAWB;
            pCvt->isp_params.isp_cfg->module_en_update |= ISP39_MODULE_RAWAWB;
        }
    } else {
        return;
    }
    // to do :
    // config blc2 important
    // pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_BLS;
    // pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_BLS;
    // pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_BLS;
    struct isp39_rawawb_meas_cfg* awb_cfg_v32 = &pCvt->isp_params.isp_cfg->meas.rawawb;
    RKAiqAecExpInfo_t* ae_exp                 = pCvt->mCommonCvtInfo.ae_exp;
    float isp_dgain                           = 1;
    float hdrmge_gain0_1                      = 1;
    if (ae_exp != NULL) {
        if (pCvt->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            isp_dgain = MAX(1.0f, ae_exp->LinearExp.exp_real_params.isp_dgain);
        } else {
            isp_dgain   = MAX(1.0f, ae_exp->HdrExp[0].exp_real_params.isp_dgain);
            float sExpo = ae_exp->HdrExp[0].exp_real_params.analog_gain *
                          ae_exp->HdrExp[0].exp_real_params.digital_gain *
                          ae_exp->HdrExp[0].exp_real_params.integration_time;

            float lExpo = ae_exp->HdrExp[1].exp_real_params.analog_gain *
                          ae_exp->HdrExp[1].exp_real_params.digital_gain *
                          ae_exp->HdrExp[1].exp_real_params.integration_time;
            if (sExpo > 0.001) {
                hdrmge_gain0_1 = lExpo / sExpo;
            }
        }
        LOGD_AWB("isp_dgain(%f),hdrmge_gain0_1(%f)", isp_dgain, hdrmge_gain0_1);
    } else {
        LOGW_AWB("%s ae_exp is null", __FUNCTION__);
    }

    const blc_res_cvt_t* blc = &pCvt->mCommonCvtInfo.blc_res;
    if (blc == NULL) {
        LOGW_AWB("%s blc_res is null", __FUNCTION__);
    }
    float isp_ob_predgain = pCvt->mCommonCvtInfo.preDGain;
    ConfigBlc2(blc, awb_meas, isp_dgain, isp_ob_predgain, pCvt->_working_mode, awb_cfg_v32);
    LOGD_AWB("blc2_cfg %d %d,%d,%d,%d", awb_cfg_v32->bls2_en, awb_cfg_v32->bls2_val.r,
             awb_cfg_v32->bls2_val.gr, awb_cfg_v32->bls2_val.gb, awb_cfg_v32->bls2_val.b);

    awb_cfg_v32->rawawb_sel = 0;
    if (awb_meas->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode) {
        awb_cfg_v32->drc2awb_sel = 1;
    } else {
        awb_cfg_v32->drc2awb_sel = 0;
        if (awb_meas->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode) {
            awb_cfg_v32->bnr2awb_sel = 1;
        } else {
            awb_cfg_v32->bnr2awb_sel = 0;
            awb_cfg_v32->rawawb_sel  = awb_meas->hw_awbCfg_statsSrc_mode;
            if (pCvt->_working_mode == RK_AIQ_WORKING_MODE_NORMAL && awb_cfg_v32->rawawb_sel > 0) {
                awb_cfg_v32->rawawb_sel = 0;
            }
        }
    }
    bool ablc_en = blc && blc->en;
    calcInputBitIs12Bit(&awb_cfg_v32->low12bit_val, awb_meas, pCvt->_working_mode, ablc_en,
                        isp_ob_predgain);
    awb_cfg_v32->in_rshift_to_12bit_en = !awb_cfg_v32->low12bit_val;
    // awb_cfg_v32->ddr_path_en = awb_meas->write2ddrEnable;
    // awb_cfg_v32->ddr_path_sel = awb_meas->write2ddrSelc;
    ConfigOverexposureValue(blc, hdrmge_gain0_1, isp_ob_predgain, pCvt->_working_mode, awb_meas,
                            awb_cfg_v32);
    awb_cfg_v32->xy_en0           = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_xyDct_en;
    awb_cfg_v32->uv_en0           = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_uvDct_en;
    awb_cfg_v32->yuv3d_en0        = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_rotYuvDct_en;
    awb_cfg_v32->xy_en1           = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_xyDct_en;
    awb_cfg_v32->uv_en1           = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_uvDct_en;
    awb_cfg_v32->yuv3d_en1        = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_rotYuvDct_en;
    awb_cfg_v32->wp_blk_wei_en0   = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_zoneWgt_en;
    awb_cfg_v32->wp_blk_wei_en1   = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_zoneWgt_en;
    awb_cfg_v32->rawlsc_bypass_en = awb_meas->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode
                                    ? true
                                    : (!awb_meas->hw_awbCfg_lsc_en);
    awb_cfg_v32->blk_measure_enable = awb_meas->pixEngine.hw_awbCfg_stats_en;
    awb_cfg_v32->blk_measure_mode =
        awb_meas->pixEngine.hw_awbCfg_zoneStatsSrc_mode > awbStats_pixAll_mode;
    awb_cfg_v32->blk_measure_xytype =
        awb_meas->pixEngine.hw_awbCfg_zoneStatsSrc_mode >= awbStats_bigWpLs0_mode;
    awb_cfg_v32->blk_measure_illu_idx = awb_meas->pixEngine.hw_awbCfg_zoneStatsSrc_mode % 0x10;
    awb_cfg_v32->blk_with_luma_wei_en = awb_meas->pixEngine.hw_awbCfg_lumaWgt_en;
    awb_cfg_v32->wp_luma_wei_en0      = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_lumaWgt_en;
    awb_cfg_v32->wp_luma_wei_en1      = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_lumaWgt_en;
    awb_cfg_v32->wp_hist_xytype       = awb_meas->wpEngine.hw_awbCfg_wpHistSrc_mode;
    awb_cfg_v32->light_num            = awb_meas->wpEngine.hw_awbCfg_lightSrcNum_val;
    awb_cfg_v32->h_offs               = awb_meas->mainWin.hw_awbCfg_win_x;
    awb_cfg_v32->v_offs               = awb_meas->mainWin.hw_awbCfg_win_y;
    awb_cfg_v32->h_size               = awb_meas->mainWin.hw_awbCfg_win_width;
    awb_cfg_v32->v_size               = awb_meas->mainWin.hw_awbCfg_win_height;

    if (awb_meas->hw_awbCfg_ds_mode == awbStats_ds_4x4 ||
            awb_meas->hw_awbCfg_ds_mode == awbStats_ds_8x8) {
        awb_cfg_v32->wind_size      = awb_meas->hw_awbCfg_ds_mode;
        awb_cfg_v32->ds16x8_mode_en = 0;
    } else if (awb_meas->hw_awbCfg_ds_mode == awbStats_ds_16x8) {
        awb_cfg_v32->ds16x8_mode_en = 1;
        awb_cfg_v32->wind_size      = awbStats_ds_8x8;
    }
    awb_cfg_v32->r_max = 16 * awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMaxR_thred + 0.5;
    awb_cfg_v32->g_max = 16 * awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMaxG_thred + 0.5;
    awb_cfg_v32->b_max = 16 * awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMaxB_thred + 0.5;
    awb_cfg_v32->y_max = 16 * awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMaxY_thred + 0.5;
    awb_cfg_v32->r_min = 16 * awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMinR_thred + 0.5;
    awb_cfg_v32->g_min = 16 * awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMinG_thred + 0.5;
    awb_cfg_v32->b_min = 16 * awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMinB_thred + 0.5;
    awb_cfg_v32->y_min = 16 * awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMinY_thred + 0.5;

    unsigned short pu_region[5];
    unsigned short pv_region[5];
    int slope_inv[4];
    UvParaFixed32(&awb_meas->wpEngine.wpDct_uvSpace.wpRegion[0], pu_region, pv_region, slope_inv);
    awb_cfg_v32->vertex0_u_0 = pu_region[0];
    awb_cfg_v32->vertex0_v_0 = pv_region[0];
    awb_cfg_v32->vertex1_u_0 = pu_region[1];
    awb_cfg_v32->vertex1_v_0 = pv_region[1];
    awb_cfg_v32->vertex2_u_0 = pu_region[2];
    awb_cfg_v32->vertex2_v_0 = pv_region[2];
    awb_cfg_v32->vertex3_u_0 = pu_region[3];
    awb_cfg_v32->vertex3_v_0 = pv_region[3];
    awb_cfg_v32->islope01_0  = slope_inv[0];
    awb_cfg_v32->islope12_0  = slope_inv[1];
    awb_cfg_v32->islope23_0  = slope_inv[2];
    awb_cfg_v32->islope30_0  = slope_inv[3];
    UvParaFixed32(&awb_meas->wpEngine.wpDct_uvSpace.wpRegion[1], pu_region, pv_region, slope_inv);
    awb_cfg_v32->vertex0_u_1 = pu_region[0];
    awb_cfg_v32->vertex0_v_1 = pv_region[0];
    awb_cfg_v32->vertex1_u_1 = pu_region[1];
    awb_cfg_v32->vertex1_v_1 = pv_region[1];
    awb_cfg_v32->vertex2_u_1 = pu_region[2];
    awb_cfg_v32->vertex2_v_1 = pv_region[2];
    awb_cfg_v32->vertex3_u_1 = pu_region[3];
    awb_cfg_v32->vertex3_v_1 = pv_region[3];
    awb_cfg_v32->islope01_1  = slope_inv[0];
    awb_cfg_v32->islope12_1  = slope_inv[1];
    awb_cfg_v32->islope23_1  = slope_inv[2];
    awb_cfg_v32->islope30_1  = slope_inv[3];
    UvParaFixed32(&awb_meas->wpEngine.wpDct_uvSpace.wpRegion[2], pu_region, pv_region, slope_inv);
    awb_cfg_v32->vertex0_u_2 = pu_region[0];
    awb_cfg_v32->vertex0_v_2 = pv_region[0];
    awb_cfg_v32->vertex1_u_2 = pu_region[1];
    awb_cfg_v32->vertex1_v_2 = pv_region[1];
    awb_cfg_v32->vertex2_u_2 = pu_region[2];
    awb_cfg_v32->vertex2_v_2 = pv_region[2];
    awb_cfg_v32->vertex3_u_2 = pu_region[3];
    awb_cfg_v32->vertex3_v_2 = pv_region[3];
    awb_cfg_v32->islope01_2  = slope_inv[0];
    awb_cfg_v32->islope12_2  = slope_inv[1];
    awb_cfg_v32->islope23_2  = slope_inv[2];
    awb_cfg_v32->islope30_2  = slope_inv[3];
    UvParaFixed32(&awb_meas->wpEngine.wpDct_uvSpace.wpRegion[3], pu_region, pv_region, slope_inv);
    awb_cfg_v32->vertex0_u_3 = pu_region[0];
    awb_cfg_v32->vertex0_v_3 = pv_region[0];
    awb_cfg_v32->vertex1_u_3 = pu_region[1];
    awb_cfg_v32->vertex1_v_3 = pv_region[1];
    awb_cfg_v32->vertex2_u_3 = pu_region[2];
    awb_cfg_v32->vertex2_v_3 = pv_region[2];
    awb_cfg_v32->vertex3_u_3 = pu_region[3];
    awb_cfg_v32->vertex3_v_3 = pv_region[3];
    awb_cfg_v32->islope01_3  = slope_inv[0];
    awb_cfg_v32->islope12_3  = slope_inv[1];
    awb_cfg_v32->islope23_3  = slope_inv[2];
    awb_cfg_v32->islope30_3  = slope_inv[3];

    short icrgb2RYuv_matrix[12];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float tmp = awb_meas->wpEngine.wpDct_rotYuvSpace.hw_awbCfg_rgb2RotYuv_coeff[i * 4 + j];
            icrgb2RYuv_matrix[i * 4 + j] =
                tmp > 0 ? (int16_t)(tmp * (1 << RK_AIQ_AWB_YUV2_MATRIX_FRAC_BIT) + 0.5)
                : (int16_t)(tmp * (1 << RK_AIQ_AWB_YUV2_MATRIX_FRAC_BIT) - 0.5);
        }
        float tmp = awb_meas->wpEngine.wpDct_rotYuvSpace.hw_awbCfg_rgb2RotYuv_coeff[i * 4 + 3];
        icrgb2RYuv_matrix[i * 4 + 3] =
            tmp > 0 ? (int16_t)(tmp * (1 << RK_AIQ_AWB_YUV2_OFFSET_FRAC_BIT) + 0.5)
            : (int16_t)(tmp * (1 << RK_AIQ_AWB_YUV2_OFFSET_FRAC_BIT) - 0.5);
    }
    awb_cfg_v32->rgb2ryuvmat0_u             = icrgb2RYuv_matrix[0];
    awb_cfg_v32->rgb2ryuvmat1_u             = icrgb2RYuv_matrix[1];
    awb_cfg_v32->rgb2ryuvmat2_u             = icrgb2RYuv_matrix[2];
    awb_cfg_v32->rgb2ryuvofs_u              = icrgb2RYuv_matrix[3];
    awb_cfg_v32->rgb2ryuvmat0_v             = icrgb2RYuv_matrix[4];
    awb_cfg_v32->rgb2ryuvmat1_v             = icrgb2RYuv_matrix[5];
    awb_cfg_v32->rgb2ryuvmat2_v             = icrgb2RYuv_matrix[6];
    awb_cfg_v32->rgb2ryuvofs_v              = icrgb2RYuv_matrix[7];
    awb_cfg_v32->rgb2ryuvmat0_y             = icrgb2RYuv_matrix[8];
    awb_cfg_v32->rgb2ryuvmat1_y             = icrgb2RYuv_matrix[9];
    awb_cfg_v32->rgb2ryuvmat2_y             = icrgb2RYuv_matrix[10];
    awb_cfg_v32->rgb2ryuvofs_y              = icrgb2RYuv_matrix[11];
    const awbStats_rotYuvRegion_t* wpRegion = &awb_meas->wpEngine.wpDct_rotYuvSpace.wpRegion[0];
    awb_cfg_v32->rotu0_ls0                  = wpRegion->hw_awbT_u2WpDistTh_curve.idx[0];
    awb_cfg_v32->rotu1_ls0                  = wpRegion->hw_awbT_u2WpDistTh_curve.idx[1];
    awb_cfg_v32->rotu2_ls0                  = wpRegion->hw_awbT_u2WpDistTh_curve.idx[2];
    awb_cfg_v32->rotu3_ls0                  = wpRegion->hw_awbT_u2WpDistTh_curve.idx[3];
    awb_cfg_v32->rotu4_ls0                  = wpRegion->hw_awbT_u2WpDistTh_curve.idx[4];
    awb_cfg_v32->rotu5_ls0                  = wpRegion->hw_awbT_u2WpDistTh_curve.idx[5];
    awb_cfg_v32->th0_ls0                    = (uint16_t)(
                wpRegion->hw_awbT_u2WpDistTh_curve.val[0] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th1_ls0 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[1] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th2_ls0 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[2] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th3_ls0 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[3] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th4_ls0 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[4] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th5_ls0 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[5] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls0_u = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpU_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls0_v = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpV_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls0_y = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpY_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    float vecP1P2[3];
    vecP1P2[0] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpU_val - wpRegion->lsVect.edp[0].hw_awbT_edpU_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    vecP1P2[1] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpV_val - wpRegion->lsVect.edp[0].hw_awbT_edpV_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    vecP1P2[2] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpY_val - wpRegion->lsVect.edp[0].hw_awbT_edpY_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    awb_cfg_v32->vec_x21_ls0_u = FloatToS16(vecP1P2[0]);
    awb_cfg_v32->vec_x21_ls0_v = FloatToS16(vecP1P2[1]);
    awb_cfg_v32->vec_x21_ls0_y = FloatToS16(vecP1P2[2]);
    awb_cfg_v32->dis_x1x2_ls0  = (uint8_t)(
                                     LOG2(vecP1P2[0] * vecP1P2[0] + vecP1P2[1] * vecP1P2[1] + vecP1P2[2] * vecP1P2[2]) + 0.5 -
                                     RK_AIQ_AWB_YUV2_OUT_FRAC_BIT * 2);
    wpRegion               = &awb_meas->wpEngine.wpDct_rotYuvSpace.wpRegion[1];
    awb_cfg_v32->rotu0_ls1 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[0];
    awb_cfg_v32->rotu1_ls1 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[1];
    awb_cfg_v32->rotu2_ls1 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[2];
    awb_cfg_v32->rotu3_ls1 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[3];
    awb_cfg_v32->rotu4_ls1 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[4];
    awb_cfg_v32->rotu5_ls1 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[5];
    awb_cfg_v32->th0_ls1   = (uint16_t)(
                                 wpRegion->hw_awbT_u2WpDistTh_curve.val[0] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th1_ls1 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[1] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th2_ls1 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[2] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th3_ls1 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[3] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th4_ls1 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[4] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th5_ls1 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[5] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls1_u = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpU_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls1_v = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpV_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls1_y = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpY_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    vecP1P2[0] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpU_val - wpRegion->lsVect.edp[0].hw_awbT_edpU_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    vecP1P2[1] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpV_val - wpRegion->lsVect.edp[0].hw_awbT_edpV_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    vecP1P2[2] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpY_val - wpRegion->lsVect.edp[0].hw_awbT_edpY_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    awb_cfg_v32->vec_x21_ls1_u = FloatToS16(vecP1P2[0]);
    awb_cfg_v32->vec_x21_ls1_v = FloatToS16(vecP1P2[1]);
    awb_cfg_v32->vec_x21_ls1_y = FloatToS16(vecP1P2[2]);
    awb_cfg_v32->dis_x1x2_ls1  = (uint8_t)(
                                     LOG2(vecP1P2[0] * vecP1P2[0] + vecP1P2[1] * vecP1P2[1] + vecP1P2[2] * vecP1P2[2]) + 0.5 -
                                     RK_AIQ_AWB_YUV2_OUT_FRAC_BIT * 2);
    wpRegion               = &awb_meas->wpEngine.wpDct_rotYuvSpace.wpRegion[2];
    awb_cfg_v32->rotu0_ls2 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[0];
    awb_cfg_v32->rotu1_ls2 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[1];
    awb_cfg_v32->rotu2_ls2 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[2];
    awb_cfg_v32->rotu3_ls2 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[3];
    awb_cfg_v32->rotu4_ls2 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[4];
    awb_cfg_v32->rotu5_ls2 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[5];
    awb_cfg_v32->th0_ls2   = (uint16_t)(
                                 wpRegion->hw_awbT_u2WpDistTh_curve.val[0] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th1_ls2 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[1] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th2_ls2 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[2] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th3_ls2 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[3] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th4_ls2 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[4] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th5_ls2 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[5] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls2_u = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpU_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls2_v = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpV_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls2_y = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpY_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    vecP1P2[0] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpU_val - wpRegion->lsVect.edp[0].hw_awbT_edpU_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    vecP1P2[1] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpV_val - wpRegion->lsVect.edp[0].hw_awbT_edpV_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    vecP1P2[2] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpY_val - wpRegion->lsVect.edp[0].hw_awbT_edpY_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    awb_cfg_v32->vec_x21_ls2_u = FloatToS16(vecP1P2[0]);
    awb_cfg_v32->vec_x21_ls2_v = FloatToS16(vecP1P2[1]);
    awb_cfg_v32->vec_x21_ls2_y = FloatToS16(vecP1P2[2]);
    awb_cfg_v32->dis_x1x2_ls2  = (uint8_t)(
                                     LOG2(vecP1P2[0] * vecP1P2[0] + vecP1P2[1] * vecP1P2[1] + vecP1P2[2] * vecP1P2[2]) + 0.5 -
                                     RK_AIQ_AWB_YUV2_OUT_FRAC_BIT * 2);
    wpRegion               = &awb_meas->wpEngine.wpDct_rotYuvSpace.wpRegion[3];
    awb_cfg_v32->rotu0_ls3 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[0];
    awb_cfg_v32->rotu1_ls3 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[1];
    awb_cfg_v32->rotu2_ls3 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[2];
    awb_cfg_v32->rotu3_ls3 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[3];
    awb_cfg_v32->rotu4_ls3 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[4];
    awb_cfg_v32->rotu5_ls3 = wpRegion->hw_awbT_u2WpDistTh_curve.idx[5];
    awb_cfg_v32->th0_ls3   = (uint16_t)(
                                 wpRegion->hw_awbT_u2WpDistTh_curve.val[0] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th1_ls3 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[1] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th2_ls3 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[2] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th3_ls3 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[3] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th4_ls3 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[4] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->th5_ls3 = (uint16_t)(
                               wpRegion->hw_awbT_u2WpDistTh_curve.val[5] * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls3_u = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpU_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls3_v = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpV_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    awb_cfg_v32->coor_x1_ls3_y = (uint16_t)(
                                     wpRegion->lsVect.edp[0].hw_awbT_edpY_val * (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT) + 0.5);
    vecP1P2[0] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpU_val - wpRegion->lsVect.edp[0].hw_awbT_edpU_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    vecP1P2[1] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpV_val - wpRegion->lsVect.edp[0].hw_awbT_edpV_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    vecP1P2[2] =
        (wpRegion->lsVect.edp[1].hw_awbT_edpY_val - wpRegion->lsVect.edp[0].hw_awbT_edpY_val) *
        (1 << RK_AIQ_AWB_YUV2_OUT_FRAC_BIT);
    awb_cfg_v32->vec_x21_ls3_u = FloatToS16(vecP1P2[0]);
    awb_cfg_v32->vec_x21_ls3_v = FloatToS16(vecP1P2[1]);
    awb_cfg_v32->vec_x21_ls3_y = FloatToS16(vecP1P2[2]);
    awb_cfg_v32->dis_x1x2_ls3  = (uint8_t)(
                                     LOG2(vecP1P2[0] * vecP1P2[0] + vecP1P2[1] * vecP1P2[1] + vecP1P2[2] * vecP1P2[2]) + 0.5 -
                                     RK_AIQ_AWB_YUV2_OUT_FRAC_BIT * 2);
    awb_cfg_v32->wt0 =
        UtlFloatToFix_U0012(awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_rgb2xy_coeff[0]);
    awb_cfg_v32->wt1 =
        UtlFloatToFix_U0012(awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_rgb2xy_coeff[1]);
    awb_cfg_v32->wt2 =
        UtlFloatToFix_U0012(awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_rgb2xy_coeff[2]);
    awb_cfg_v32->mat0_x = UtlFloatToFix_S0312(
                              awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_xyTransMatrix_coeff[0]);
    awb_cfg_v32->mat1_x = UtlFloatToFix_S0312(
                              awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_xyTransMatrix_coeff[1]);
    awb_cfg_v32->mat2_x = UtlFloatToFix_S0312(
                              awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_xyTransMatrix_coeff[2]);
    awb_cfg_v32->mat0_y = UtlFloatToFix_S0312(
                              awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_xyTransMatrix_coeff[3]);
    awb_cfg_v32->mat1_y = UtlFloatToFix_S0312(
                              awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_xyTransMatrix_coeff[4]);
    awb_cfg_v32->mat2_y = UtlFloatToFix_S0312(
                              awb_meas->wpEngine.wpDct_xySpace.rgb2xy.hw_awbCfg_xyTransMatrix_coeff[5]);
    const awbStats_xyRegion_t* norWpRegion = &awb_meas->wpEngine.wpDct_xySpace.norWpRegion[0];
    const awbStats_xyRegion_t* bigWpRegion = &awb_meas->wpEngine.wpDct_xySpace.bigWpRegion[0];
    awb_cfg_v32->nor_x0_0 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->nor_x1_0 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->nor_y0_0 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->nor_y1_0 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->big_x0_0 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->big_x1_0 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->big_y0_0 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->big_y1_0 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxY_val);
    norWpRegion           = &awb_meas->wpEngine.wpDct_xySpace.norWpRegion[1];
    bigWpRegion           = &awb_meas->wpEngine.wpDct_xySpace.bigWpRegion[1];
    awb_cfg_v32->nor_x0_1 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->nor_x1_1 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->nor_y0_1 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->nor_y1_1 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->big_x0_1 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->big_x1_1 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->big_y0_1 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->big_y1_1 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxY_val);
    norWpRegion           = &awb_meas->wpEngine.wpDct_xySpace.norWpRegion[2];
    bigWpRegion           = &awb_meas->wpEngine.wpDct_xySpace.bigWpRegion[2];
    awb_cfg_v32->nor_x0_2 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->nor_x1_2 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->nor_y0_2 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->nor_y1_2 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->big_x0_2 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->big_x1_2 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->big_y0_2 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->big_y1_2 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxY_val);
    norWpRegion           = &awb_meas->wpEngine.wpDct_xySpace.norWpRegion[3];
    bigWpRegion           = &awb_meas->wpEngine.wpDct_xySpace.bigWpRegion[3];
    awb_cfg_v32->nor_x0_3 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->nor_x1_3 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->nor_y0_3 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->nor_y1_3 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->big_x0_3 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->big_x1_3 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg_v32->big_y0_3 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg_v32->big_y1_3 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxY_val);
    ConfigPreWbgain3(awb_cfg_v32, awb_meas_priv, &pCvt->awb_gain_final,
                     &pCvt->mCommonCvtInfo.otp_awb);
    awb_cfg_v32->multiwindow_en      = awb_meas->mainWin.hw_awbCfg_nonROI_en;
    awb_cfg_v32->multiwindow0_h_offs = awb_meas->mainWin.nonROI[0].hw_awbCfg_nonROI_x;
    awb_cfg_v32->multiwindow0_v_offs = awb_meas->mainWin.nonROI[0].hw_awbCfg_nonROI_y;
    awb_cfg_v32->multiwindow0_h_size = awb_meas->mainWin.nonROI[0].hw_awbCfg_nonROI_x +
                                       awb_meas->mainWin.nonROI[0].hw_awbCfg_nonROI_width;
    awb_cfg_v32->multiwindow0_v_size = awb_meas->mainWin.nonROI[0].hw_awbCfg_nonROI_y +
                                       awb_meas->mainWin.nonROI[0].hw_awbCfg_nonROI_height;
    awb_cfg_v32->multiwindow1_h_offs = awb_meas->mainWin.nonROI[1].hw_awbCfg_nonROI_x;
    awb_cfg_v32->multiwindow1_v_offs = awb_meas->mainWin.nonROI[1].hw_awbCfg_nonROI_y;
    awb_cfg_v32->multiwindow1_h_size = awb_meas->mainWin.nonROI[1].hw_awbCfg_nonROI_x +
                                       awb_meas->mainWin.nonROI[1].hw_awbCfg_nonROI_width;
    awb_cfg_v32->multiwindow1_v_size = awb_meas->mainWin.nonROI[1].hw_awbCfg_nonROI_y +
                                       awb_meas->mainWin.nonROI[1].hw_awbCfg_nonROI_height;
    awb_cfg_v32->multiwindow2_h_offs = awb_meas->mainWin.nonROI[2].hw_awbCfg_nonROI_x;
    awb_cfg_v32->multiwindow2_v_offs = awb_meas->mainWin.nonROI[2].hw_awbCfg_nonROI_y;
    awb_cfg_v32->multiwindow2_h_size = awb_meas->mainWin.nonROI[2].hw_awbCfg_nonROI_x +
                                       awb_meas->mainWin.nonROI[2].hw_awbCfg_nonROI_width;
    awb_cfg_v32->multiwindow2_v_size = awb_meas->mainWin.nonROI[2].hw_awbCfg_nonROI_y +
                                       awb_meas->mainWin.nonROI[2].hw_awbCfg_nonROI_height;
    awb_cfg_v32->multiwindow3_h_offs = awb_meas->mainWin.nonROI[3].hw_awbCfg_nonROI_x;
    awb_cfg_v32->multiwindow3_v_offs = awb_meas->mainWin.nonROI[3].hw_awbCfg_nonROI_y;
    awb_cfg_v32->multiwindow3_h_size = awb_meas->mainWin.nonROI[3].hw_awbCfg_nonROI_x +
                                       awb_meas->mainWin.nonROI[3].hw_awbCfg_nonROI_width;
    awb_cfg_v32->multiwindow3_v_size = awb_meas->mainWin.nonROI[3].hw_awbCfg_nonROI_y +
                                       awb_meas->mainWin.nonROI[3].hw_awbCfg_nonROI_height;

    int exc_wp_region0_excen0 = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_wpFiltOut_en[0];
    int exc_wp_region0_excen1 = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_wpFiltOut_en[0];
    awb_cfg_v32->exc_wp_region0_excen =
        ((exc_wp_region0_excen1 << 1) + exc_wp_region0_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region0_measen =
        awb_meas->wpEngine.wpFiltOut_fullEntity[0].hw_awbCfg_wpStats_en;
    awb_cfg_v32->exc_wp_region0_domain =
        awb_meas->wpEngine.wpFiltOut_fullEntity[0].hw_awbT_wpSpace_mode;
    awb_cfg_v32->exc_wp_region0_xu0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[0].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_fullEntity[0].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region0_xu1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[0].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_fullEntity[0].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region0_yv0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[0].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_fullEntity[0].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region0_yv1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[0].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_fullEntity[0].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region0_weight =
        (uint8_t)(awb_meas->wpEngine.wpFiltOut_fullEntity[0].hw_awbT_stats_wgt *
                  ((1 << RK_AIQ_WP_INCLUDE_BIS) - 1) +
                  0.5);
    int exc_wp_region1_excen0 = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_wpFiltOut_en[1];
    int exc_wp_region1_excen1 = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_wpFiltOut_en[1];
    awb_cfg_v32->exc_wp_region1_excen =
        ((exc_wp_region1_excen1 << 1) + exc_wp_region1_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region1_measen =
        awb_meas->wpEngine.wpFiltOut_fullEntity[1].hw_awbCfg_wpStats_en;
    awb_cfg_v32->exc_wp_region1_domain =
        awb_meas->wpEngine.wpFiltOut_fullEntity[1].hw_awbT_wpSpace_mode;
    awb_cfg_v32->exc_wp_region1_xu0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[1].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_fullEntity[1].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region1_xu1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[1].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_fullEntity[1].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region1_yv0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[1].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_fullEntity[1].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region1_yv1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[1].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_fullEntity[1].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region1_weight =
        (uint8_t)(awb_meas->wpEngine.wpFiltOut_fullEntity[1].hw_awbT_stats_wgt *
                  ((1 << RK_AIQ_WP_INCLUDE_BIS) - 1) +
                  0.5);
    int exc_wp_region2_excen0 = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_wpFiltOut_en[2];
    int exc_wp_region2_excen1 = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_wpFiltOut_en[2];
    awb_cfg_v32->exc_wp_region2_excen =
        ((exc_wp_region2_excen1 << 1) + exc_wp_region2_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region2_measen =
        awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbCfg_wpStats_en;
    awb_cfg_v32->exc_wp_region2_domain =
        awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode;
    awb_cfg_v32->exc_wp_region2_xu0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[2].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region2_xu1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[2].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region2_yv0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[2].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region2_yv1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[2].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region2_weight =
        (uint8_t)(awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_stats_wgt *
                  ((1 << RK_AIQ_WP_INCLUDE_BIS) - 1) +
                  0.5);
    int exc_wp_region3_excen0 = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_wpFiltOut_en[3];
    int exc_wp_region3_excen1 = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_wpFiltOut_en[3];
    awb_cfg_v32->exc_wp_region3_excen =
        ((exc_wp_region3_excen1 << 1) + exc_wp_region3_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region3_measen =
        awb_meas->wpEngine.wpFiltOut_fullEntity[3].hw_awbCfg_wpStats_en;
    awb_cfg_v32->exc_wp_region3_domain =
        awb_meas->wpEngine.wpFiltOut_fullEntity[3].hw_awbT_wpSpace_mode;
    awb_cfg_v32->exc_wp_region3_xu0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[3].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region3_xu1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[3].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region3_yv0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[3].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region3_yv1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_fullEntity[3].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_fullEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region3_weight =
        (uint8_t)(awb_meas->wpEngine.wpFiltOut_fullEntity[3].hw_awbT_stats_wgt *
                  ((1 << RK_AIQ_WP_INCLUDE_BIS) - 1) +
                  0.5);
    int exc_wp_region4_excen0 = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_wpFiltOut_en[4];
    int exc_wp_region4_excen1 = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_wpFiltOut_en[4];
    awb_cfg_v32->exc_wp_region4_excen =
        ((exc_wp_region4_excen1 << 1) + exc_wp_region4_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region4_domain =
        awb_meas->wpEngine.wpFiltOut_smpEntity[0].hw_awbT_wpSpace_mode;
    awb_cfg_v32->exc_wp_region4_xu0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[0].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_smpEntity[0].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region4_xu1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[0].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_smpEntity[0].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region4_yv0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[0].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_smpEntity[0].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region4_yv1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[0].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_smpEntity[0].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region4_weight =
        (uint8_t)(awb_meas->wpEngine.wpFiltOut_smpEntity[0].hw_awbT_stats_wgt *
                  ((1 << RK_AIQ_WP_INCLUDE_BIS) - 1) +
                  0.5);
    int exc_wp_region5_excen0 = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_wpFiltOut_en[5];
    int exc_wp_region5_excen1 = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_wpFiltOut_en[5];
    awb_cfg_v32->exc_wp_region5_excen =
        ((exc_wp_region5_excen1 << 1) + exc_wp_region5_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region5_domain =
        awb_meas->wpEngine.wpFiltOut_smpEntity[1].hw_awbT_wpSpace_mode;
    awb_cfg_v32->exc_wp_region5_xu0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[1].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_smpEntity[1].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region5_xu1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[1].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_smpEntity[1].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region5_yv0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[1].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_smpEntity[1].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region5_yv1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[1].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_smpEntity[1].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region5_weight =
        (uint8_t)(awb_meas->wpEngine.wpFiltOut_smpEntity[1].hw_awbT_stats_wgt *
                  ((1 << RK_AIQ_WP_INCLUDE_BIS) - 1) +
                  0.5);
    int exc_wp_region6_excen0 = awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_wpFiltOut_en[6];
    int exc_wp_region6_excen1 = awb_meas->wpEngine.bigWpStatsCfg.hw_awbCfg_wpFiltOut_en[6];
    awb_cfg_v32->exc_wp_region6_excen =
        ((exc_wp_region6_excen1 << 1) + exc_wp_region6_excen0) & 0x3;
    awb_cfg_v32->exc_wp_region6_domain =
        awb_meas->wpEngine.wpFiltOut_smpEntity[2].hw_awbT_wpSpace_mode;
    awb_cfg_v32->exc_wp_region6_xu0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[2].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_smpEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region6_xu1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[2].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_meas->wpEngine.wpFiltOut_smpEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region6_yv0 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[2].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_smpEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region6_yv1 =
        filtOutFixed(awb_meas->wpEngine.wpFiltOut_smpEntity[2].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_meas->wpEngine.wpFiltOut_smpEntity[2].hw_awbT_wpSpace_mode);
    awb_cfg_v32->exc_wp_region6_weight =
        (uint8_t)(awb_meas->wpEngine.wpFiltOut_smpEntity[2].hw_awbT_stats_wgt *
                  ((1 << RK_AIQ_WP_INCLUDE_BIS) - 1) +
                  0.5);
    awb_cfg_v32->wp_luma_weicurve_y0 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[0];
    awb_cfg_v32->wp_luma_weicurve_y1 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[1];
    awb_cfg_v32->wp_luma_weicurve_y2 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[2];
    awb_cfg_v32->wp_luma_weicurve_y3 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[3];
    awb_cfg_v32->wp_luma_weicurve_y4 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[4];
    awb_cfg_v32->wp_luma_weicurve_y5 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[5];
    awb_cfg_v32->wp_luma_weicurve_y6 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[6];
    awb_cfg_v32->wp_luma_weicurve_y7 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[7];
    awb_cfg_v32->wp_luma_weicurve_y8 = awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.idx[8];
    awb_cfg_v32->wp_luma_weicurve_w0 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[0] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    awb_cfg_v32->wp_luma_weicurve_w1 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[1] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    awb_cfg_v32->wp_luma_weicurve_w2 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[2] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    awb_cfg_v32->wp_luma_weicurve_w3 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[3] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    awb_cfg_v32->wp_luma_weicurve_w4 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[4] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    awb_cfg_v32->wp_luma_weicurve_w5 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[5] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    awb_cfg_v32->wp_luma_weicurve_w6 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[6] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    awb_cfg_v32->wp_luma_weicurve_w7 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[7] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    awb_cfg_v32->wp_luma_weicurve_w8 = (uint8_t)(awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[8] *
                                       ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1) +
                                       0.5);
    for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        awb_cfg_v32->wp_blk_wei_w[i] = awb_meas->wpEngine.hw_awbCfg_zone_wgt[i];
    }
    awb_cfg_v32->blk_rtdw_measure_en = 0;

    WriteAwbReg(awb_cfg_v32);
    WriteDataForIcCmodel(awb_cfg_v32);
}
#endif
#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33

static void ConfigAfBlc(const blc_res_cvt_t *ablc, const afStats_cfg_t* af_data, float isp_dgain,
                        float isp_ob_predgain, void *isp_rawaf)
{
#if ISP_HW_V39
    struct isp39_rawaf_meas_cfg* rawaf = (struct isp39_rawaf_meas_cfg*)isp_rawaf;
#elif ISP_HW_V35
    struct isp35_rawaf_meas_cfg* rawaf = (struct isp35_rawaf_meas_cfg*)isp_rawaf;
#endif
    if (af_data->hw_afCfg_statsSrc_mode == afStats_dmIn_mode ||
            af_data->hw_afCfg_statsSrc_mode == afStats_ynrOut_mode) {
        //don't support to use blc2
        rawaf->bls_en = 0;
        rawaf->bls_offset = 0;
        return;
    }

    float bls_offset = 0;
    float dgain2 = 1.0;
    //sw_blcT_obcPostTnr_mode == blc_autoOBCPostTnr_mode : use oboffset only
    //sw_blcT_obcPostTnr_mode == blc_manualOBCPostTnr_mode : use blc1 only
    //now only support blc_autoOBCPostTnr_mode
    if (ablc && ablc->en) {
        if (isp_ob_predgain > 1) {
            dgain2 = isp_ob_predgain;
        }
        if (ablc->obcPostTnr.sw_blcT_obcPostTnr_en)
            bls_offset = ablc->obcPostTnr.sw_blcT_autoOB_offset;
    }
    // 2 blc2 recalc base on ablc
    if(af_data->hw_afCfg_statsSrc_mode == afStats_btnrOut_mode) {
        //update by (offset +blc1)*dgain
        // working_mode = normal,so   applyPosition = IN_AWBGAIN1
        rawaf->bls_offset = bls_offset * isp_dgain * dgain2 + 0.5;
    } else {//select raw
        //update by offset +blc1
        rawaf->bls_offset = bls_offset + 0.5;
    }
    rawaf->bls_en = 1;
}

void convertAiqAfToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    const afStats_cfg_t* afStats_cfg = (afStats_cfg_t*)pBase->_data;
#if ISP_HW_V39
    struct isp39_rawaf_meas_cfg* rawaf = &pCvt->isp_params.isp_cfg->meas.rawaf;
#elif ISP_HW_V35
    struct isp35_rawaf_meas_cfg* rawaf = &pCvt->isp_params.isp_cfg->meas.rawaf;
#endif
    int i;

    if (afStats_cfg->hw_afCfg_stats_en) pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_RAWAF;
    pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_RAWAF;
    pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_RAWAF;

    rawaf->ldg_en = afStats_cfg->hw_afCfg_ldg_en;
    switch (afStats_cfg->hw_afCfg_statsSrc_mode) {
    case afStats_vicapOut_mode:
        rawaf->bnr2af_sel = 0;
        rawaf->from_ynr   = 1;
        rawaf->rawaf_sel  = 0;
        break;
    case afStats_ynrOut_mode:
        rawaf->bnr2af_sel = 0;
        rawaf->from_ynr   = 1;
        rawaf->rawaf_sel  = 0;
        break;
    case afStats_btnrOut_mode:
        rawaf->bnr2af_sel = 1;
        rawaf->from_ynr   = 0;
        rawaf->rawaf_sel  = 0;
        break;
    case afStats_dmIn_mode:
        rawaf->bnr2af_sel = 0;
        rawaf->from_ynr   = 0;
        rawaf->rawaf_sel  = 3;
        break;
    case afStats_chl2Wb0Out_mode:
        rawaf->bnr2af_sel = 0;
        rawaf->from_ynr   = 0;
        rawaf->rawaf_sel  = 2;
        break;
    case afStats_chl1Wb0Out_mode:
        rawaf->bnr2af_sel = 0;
        rawaf->from_ynr   = 0;
        rawaf->rawaf_sel  = 1;
        break;
    default:
        rawaf->bnr2af_sel = 0;
        rawaf->from_ynr   = 0;
        rawaf->rawaf_sel  = 0;
        break;
    }

    rawaf->tnrin_shift   = afStats_cfg->hw_afCfg_statsBtnrOut_shift;
    rawaf->win[0].h_offs = afStats_cfg->mainWin.hw_afCfg_win_x;
    rawaf->win[0].v_offs = afStats_cfg->mainWin.hw_afCfg_win_y;
    rawaf->win[0].h_size = afStats_cfg->mainWin.hw_afCfg_win_width;
    rawaf->win[0].v_size = afStats_cfg->mainWin.hw_afCfg_win_height;
    rawaf->win[1].h_offs = afStats_cfg->subWin.hw_afCfg_win_x;
    rawaf->win[1].v_offs = afStats_cfg->subWin.hw_afCfg_win_y;
    rawaf->win[1].h_size = afStats_cfg->subWin.hw_afCfg_win_width;
    rawaf->win[1].v_size = afStats_cfg->subWin.hw_afCfg_win_height;
    rawaf->aehgl_en      = 1;
    rawaf->ae_mode       = 0;
#if ISP_HW_V39
    rawaf->ae_config_use = 1;
#endif
    rawaf->vldg_sel      = 0;
    rawaf->y_mode        = 0;
    rawaf->num_afm_win   = 2;

    RKAiqAecExpInfo_t *ae_exp = pCvt->mCommonCvtInfo.ae_exp;
    float isp_dgain = 1;
    if (ae_exp != NULL) {
        if(pCvt->_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
            isp_dgain = MAX(1.0f, ae_exp->LinearExp.exp_real_params.isp_dgain);
        else
            isp_dgain = MAX(1.0f, ae_exp->HdrExp[0].exp_real_params.isp_dgain);

        LOGD_AF("isp_dgain(%f)", isp_dgain);
    } else {
        LOGW_AF("%s ae_exp is null", __FUNCTION__);
    }

    const blc_res_cvt_t *blc = &pCvt->mCommonCvtInfo.blc_res;
    if (blc == NULL) {
        LOGW_AF("%s blc_res is null", __FUNCTION__);
    }
    ConfigAfBlc(blc, afStats_cfg, isp_dgain, pCvt->mCommonCvtInfo.preDGain, rawaf);
    LOGD_AF("%s: bls_en %d, bls_offset %d", __FUNCTION__, rawaf->bls_en, rawaf->bls_offset);

    switch (afStats_cfg->hw_afCfg_ds_mode) {
    case afStats_ds_2x2_mode:
        rawaf->avg_ds_en    = 0;
        rawaf->avg_ds_mode = 0;
        rawaf->v_dnscl_mode = 2;
        break;
    case afStats_ds_4x4_mode:
        rawaf->avg_ds_en   = 1;
        rawaf->avg_ds_mode = 0;
        rawaf->v_dnscl_mode = 1;
        break;
    case afStats_ds_8x8_mode:
        rawaf->avg_ds_en   = 1;
        rawaf->avg_ds_mode = 1;
        rawaf->v_dnscl_mode = 1;
        break;
    default:
        rawaf->avg_ds_en    = 0;
        rawaf->avg_ds_mode = 0;
        rawaf->v_dnscl_mode = 1;
        break;
    }

    rawaf->gamma_en = afStats_cfg->gamma.hw_afCfg_gamma_en;
    for (i = 0; i < 17; i++) rawaf->gamma_y[i] = afStats_cfg->gamma.hw_afCfg_gamma_val[i] * 0x3FF;
    rawaf->gaus_en = afStats_cfg->preFilt.hw_afCfg_gaus_en;
    for (i = 0; i < 9; i++) rawaf->gaus_coe[i] = afStats_cfg->preFilt.hw_afCfg_gaus_coeff[i] * 128;
    rawaf->highlit_thresh = afStats_cfg->hw_afCfg_hLumaCnt_thred * 0x3FF;

    rawaf->hiir_en               = 1;
    rawaf->hiir_left_border_mode = afStats_cfg->hw_afCfg_hFiltLnBnd_mode;
    for (i = 0; i < 6; i++) rawaf->h1iir1_coe[i] = afStats_cfg->hFilt1.hw_afCfg_iirStep1_coeff[i];
    for (i = 0; i < 6; i++) rawaf->h1iir2_coe[i] = afStats_cfg->hFilt1.hw_afCfg_iirStep2_coeff[i];
    switch (afStats_cfg->hFilt1.hw_afCfg_fvFmt_mode) {
    case afStats_outPeak_sumLinear_mode:
        rawaf->h1_fv_mode  = 1;
        rawaf->h1_acc_mode = 1;
        break;
    case afStats_outNorm_sumLinear_mode:
        rawaf->h1_fv_mode  = 1;
        rawaf->h1_acc_mode = 0;
        break;
    case afStats_outPeak_sumSqu_mode:
        rawaf->h1_fv_mode  = 0;
        rawaf->h1_acc_mode = 1;
        break;
    default:
        rawaf->h1_fv_mode  = 0;
        rawaf->h1_acc_mode = 0;
        break;
    }
    rawaf->h1iir_shift_wina = afStats_cfg->hFilt1.hw_afCfg_accMainWin_shift;
    rawaf->h1iir_shift_winb = afStats_cfg->hFilt1.hw_afCfg_accSubWin_shift;

    for (i = 0; i < 6; i++) rawaf->h2iir1_coe[i] = afStats_cfg->hFilt2.hw_afCfg_iirStep1_coeff[i];
    for (i = 0; i < 6; i++) rawaf->h2iir2_coe[i] = afStats_cfg->hFilt2.hw_afCfg_iirStep2_coeff[i];
    switch (afStats_cfg->hFilt2.hw_afCfg_fvFmt_mode) {
    case afStats_outPeak_sumLinear_mode:
        rawaf->h2_fv_mode  = 1;
        rawaf->h2_acc_mode = 1;
        break;
    case afStats_outNorm_sumLinear_mode:
        rawaf->h2_fv_mode  = 1;
        rawaf->h2_acc_mode = 0;
        break;
    case afStats_outPeak_sumSqu_mode:
        rawaf->h2_fv_mode  = 0;
        rawaf->h2_acc_mode = 1;
        break;
    default:
        rawaf->h2_fv_mode  = 0;
        rawaf->h2_acc_mode = 0;
        break;
    }
    rawaf->h2iir_shift_wina = afStats_cfg->hFilt2.hw_afCfg_accMainWin_shift;
    rawaf->h2iir_shift_winb = afStats_cfg->hFilt2.hw_afCfg_accSubWin_shift;

    rawaf->h_fv_thresh          = afStats_cfg->hFilt_coring.hw_afCfg_coring_thred * 0xFFF;
    rawaf->h_fv_slope           = afStats_cfg->hFilt_coring.hw_afCfg_slope_val * 0x100;
    rawaf->h_fv_limit           = afStats_cfg->hFilt_coring.hw_afCfg_coring_maxLimit * 0x3FF;
    rawaf->curve_h[0].ldg_lumth = afStats_cfg->hFilt_ldg.hw_afCfg_lumaL_thred * 0xFF;
    rawaf->curve_h[0].ldg_gain  = afStats_cfg->hFilt_ldg.hw_afCfg_gainL_val * 0xFF;
    rawaf->curve_h[0].ldg_gslp  = afStats_cfg->hFilt_ldg.hw_afCfg_slopeL_val * 0x100;
    rawaf->curve_h[1].ldg_lumth = afStats_cfg->hFilt_ldg.hw_afCfg_lumaR_thred * 0xFF;
    rawaf->curve_h[1].ldg_gain  = afStats_cfg->hFilt_ldg.hw_afCfg_gainR_val * 0xFF;
    rawaf->curve_h[1].ldg_gslp  = afStats_cfg->hFilt_ldg.hw_afCfg_slopeR_val * 0x100;
    rawaf->hldg_dilate_num      = afStats_cfg->hFilt_ldg.hw_afCfg_maxOptBndL_len;

    rawaf->viir_en = 1;
    for (i = 0; i < 3; i++) rawaf->v1iir_coe[i] = afStats_cfg->vFilt1.hw_afCfg_iirStep1_coeff[i];
    for (i = 0; i < 3; i++) rawaf->v1fir_coe[i] = afStats_cfg->vFilt1.hw_afCfg_firStep2_coeff[i];
    switch (afStats_cfg->vFilt1.hw_afCfg_fvFmt_mode) {
    case afStats_outPeak_sumLinear_mode:
        rawaf->v1_fv_mode  = 1;
        rawaf->v1_acc_mode = 1;
        break;
    case afStats_outNorm_sumLinear_mode:
        rawaf->v1_fv_mode  = 1;
        rawaf->v1_acc_mode = 0;
        break;
    case afStats_outPeak_sumSqu_mode:
        rawaf->v1_fv_mode  = 0;
        rawaf->v1_acc_mode = 1;
        break;
    default:
        rawaf->v1_fv_mode  = 0;
        rawaf->v1_acc_mode = 0;
        break;
    }
    rawaf->v1iir_shift_wina = afStats_cfg->vFilt1.hw_afCfg_accMainWin_shift;
    rawaf->v1iir_shift_winb = afStats_cfg->vFilt1.hw_afCfg_accSubWin_shift;

    for (i = 0; i < 3; i++) rawaf->v2iir_coe[i] = afStats_cfg->vFilt2.hw_afCfg_iirStep1_coeff[i];
    for (i = 0; i < 3; i++) rawaf->v2fir_coe[i] = afStats_cfg->vFilt2.hw_afCfg_firStep2_coeff[i];
    switch (afStats_cfg->vFilt2.hw_afCfg_fvFmt_mode) {
    case afStats_outPeak_sumLinear_mode:
        rawaf->v2_fv_mode  = 1;
        rawaf->v2_acc_mode = 1;
        break;
    case afStats_outNorm_sumLinear_mode:
        rawaf->v2_fv_mode  = 1;
        rawaf->v2_acc_mode = 0;
        break;
    case afStats_outPeak_sumSqu_mode:
        rawaf->v2_fv_mode  = 0;
        rawaf->v2_acc_mode = 1;
        break;
    default:
        rawaf->v2_fv_mode  = 0;
        rawaf->v2_acc_mode = 0;
        break;
    }
    rawaf->v2iir_shift_wina = afStats_cfg->vFilt2.hw_afCfg_accMainWin_shift;
    rawaf->v2iir_shift_winb = afStats_cfg->vFilt2.hw_afCfg_accSubWin_shift;

    rawaf->v_fv_thresh          = afStats_cfg->vFilt_coring.hw_afCfg_coring_thred * 0xFFF;
    rawaf->v_fv_slope           = afStats_cfg->vFilt_coring.hw_afCfg_slope_val * 0x100;
    rawaf->v_fv_limit           = afStats_cfg->vFilt_coring.hw_afCfg_coring_maxLimit * 0x3FF;
    rawaf->curve_v[0].ldg_lumth = afStats_cfg->vFilt_ldg.hw_afCfg_lumaL_thred * 0xFF;
    rawaf->curve_v[0].ldg_gain  = afStats_cfg->vFilt_ldg.hw_afCfg_gainL_val * 0xFF;
    rawaf->curve_v[0].ldg_gslp  = afStats_cfg->vFilt_ldg.hw_afCfg_slopeL_val * 0x100;
    rawaf->curve_v[1].ldg_lumth = afStats_cfg->vFilt_ldg.hw_afCfg_lumaR_thred * 0xFF;
    rawaf->curve_v[1].ldg_gain  = afStats_cfg->vFilt_ldg.hw_afCfg_gainR_val * 0xFF;
    rawaf->curve_v[1].ldg_gslp  = afStats_cfg->vFilt_ldg.hw_afCfg_slopeR_val * 0x100;
}
#endif

#if RKAIQ_HAVE_CAC_V11
static void convertAiqCacToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase,
                                       struct isp39_isp_params_cfg* isp_cfg,
                                       struct isp39_isp_params_cfg* isp_cfg_right,
                                       bool is_multi_isp) {
    if (pBase->en) {
        isp_cfg->module_en_update |= ISP3X_MODULE_CAC;
        isp_cfg->module_ens |= ISP3X_MODULE_CAC;
        isp_cfg->module_cfg_update |= ISP3X_MODULE_CAC;
    } else {
        isp_cfg->module_en_update |= (ISP3X_MODULE_CAC);
        isp_cfg->module_ens &= ~(ISP3X_MODULE_CAC);
        isp_cfg->module_cfg_update &= ~(ISP3X_MODULE_CAC);
        return;
    }

    isp_cfg->others.cac_cfg.bypass_en = pBase->bypass;
    if(is_multi_isp)
        isp_cfg_right->others.cac_cfg.bypass_en = pBase->bypass;
    rk_aiq_cac21_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->isp_params, &pCvt->mCacInfo, &pCvt->mCommonCvtInfo);
}
#endif

#if (RKAIQ_HAVE_BAYERTNR_V30)
static void convertAiqBtnrToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        // bayer3dnr enable  bayer2dnr must enable at the same time
        pCvt->isp_params.isp_cfg->module_ens |= ISP3X_MODULE_BAY3D;
        pCvt->isp_params.isp_cfg->module_ens |= ISP3X_MODULE_BAYNR;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_BAY3D;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP3X_MODULE_BAY3D;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_BAYNR;
    } else {
        // tnr can't open/close in runtime.
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP3X_MODULE_BAY3D;
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP3X_MODULE_BAYNR;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_BAY3D;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_BAYNR;
        return;
    }

    pCvt->isp_params.isp_cfg->others.bay3d_cfg.bypass_en = pBase->bypass;
    pCvt->mBtnrInfo.btnr_attrib = pCvt->btnr_attrib;
    rk_aiq_btnr40_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo, &pCvt->mBtnrInfo, &pCvt->mergeLuma2Wgt);
}
#endif

#if RKAIQ_HAVE_YNR_V24
static void convertAiqYnrToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP3X_MODULE_YNR;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_YNR;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP3X_MODULE_YNR;
    }
    else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP3X_MODULE_YNR;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_YNR;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP3X_MODULE_YNR;
        return;
    }

    if(pBase->bypass) {
        ynr_param_t* ynr_param = (ynr_param_t*)pBase->_data;
        ynr_param->dyn.hw_ynrT_loNr_en = 0;
        ynr_param->dyn.hiNr_filtProc.hw_ynrT_nlmFilt_en = 0;
        LOGW_ANR("Bypass ynr instead by ynr.dyn.loNr_en and ynr.dyn.hiNr_filtProc.nlmFilt_en.");
    }

    rk_aiq_ynr34_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
}
#endif

#if (RKAIQ_HAVE_CNR_V31)
static void convertAiqCnrToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP3X_MODULE_CNR;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_CNR;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP3X_MODULE_CNR;
    }
    else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP3X_MODULE_CNR;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_CNR;
        return;
    }

    if (pBase->bypass) {
        LOGW_ANR("The cnr doesn't support bypass feature, so you can set cnr.hiNr_bifilt.bifiltOut_alpha instead");
    }

    rk_aiq_cnr34_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
}
#endif

#if RKAIQ_HAVE_DEBAYER_V3
static void convertAiqDmToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_DEBAYER;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_DEBAYER;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_DEBAYER;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_DEBAYER;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_DEBAYER;
        return;
    }

#ifdef ISP_HW_V39
    rk_aiq_dm23_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
#endif
}
#endif

#if RKAIQ_HAVE_MERGE_V12
void convertAiqMergeToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_en_update |= 1LL << RK_ISP2X_HDRMGE_ID;
        pCvt->isp_params.isp_cfg->module_ens |= 1LL << RK_ISP2X_HDRMGE_ID;
        pCvt->isp_params.isp_cfg->module_cfg_update |= 1LL << RK_ISP2X_HDRMGE_ID;
    } else {
        pCvt->isp_params.isp_cfg->module_en_update |= 1LL << RK_ISP2X_HDRMGE_ID;
        pCvt->isp_params.isp_cfg->module_ens &= ~(1LL << RK_ISP2X_HDRMGE_ID);
        pCvt->isp_params.isp_cfg->module_cfg_update &= ~(1LL << RK_ISP2X_HDRMGE_ID);
        return;
    }

    rk_aiq_merge22_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo, &pCvt->mergeLuma2Wgt);
}
#endif

#if RKAIQ_HAVE_DEHAZE_V14
static void convertAiqDehazeToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    pCvt->mCommonCvtInfo.dehaze_en = pBase->en;
    if (pCvt->mCommonCvtInfo.histeq_en || pBase->en) {
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_DHAZ;
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_DHAZ;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_DHAZ;
    } else {
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_DHAZ;
        pCvt->isp_params.isp_cfg->module_ens &= ~(ISP2X_MODULE_DHAZ);
        pCvt->isp_params.isp_cfg->module_cfg_update &= ~(ISP2X_MODULE_DHAZ);
        return;
    }
    rk_aiq_dehaze23_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
    rk_aiq_dehazeHisteq23_sigma_params_cvt(&pCvt->isp_params, &pCvt->mCommonCvtInfo);

    if (pBase->bypass) pCvt->isp_params.isp_cfg->others.dhaz_cfg.dc_en = 0;
}

static void convertAiqHisteqToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    pCvt->mCommonCvtInfo.histeq_en = pBase->en;
    if (pCvt->mCommonCvtInfo.dehaze_en || pBase->en) {
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_DHAZ;
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_DHAZ;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_DHAZ;
    } else {
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_DHAZ;
        pCvt->isp_params.isp_cfg->module_ens &= ~(ISP2X_MODULE_DHAZ);
        pCvt->isp_params.isp_cfg->module_cfg_update &= ~(ISP2X_MODULE_DHAZ);
        return;
    }
    rk_aiq_histeq23_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
    rk_aiq_dehazeHisteq23_sigma_params_cvt(&pCvt->isp_params, &pCvt->mCommonCvtInfo);

    if (pBase->bypass) pCvt->isp_params.isp_cfg->others.dhaz_cfg.hist_en = 0;
}
#endif

#if RKAIQ_HAVE_DRC_V20
void convertAiqDrcToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pCvt->mCommonCvtInfo.frameNum > 1 && !pBase->en) {
        LOGW_ATMO("Drc must be on  when isp is HDR mode. Please turn on by drc.en!");
    }
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        pCvt->isp_params.isp_cfg->module_ens |= 1LL << Rk_ISP21_DRC_ID;
        pCvt->isp_params.isp_cfg->module_cfg_update |= 1LL << Rk_ISP21_DRC_ID;
    } else {
        pCvt->isp_params.isp_cfg->module_en_update |= 1LL << Rk_ISP21_DRC_ID;
        pCvt->isp_params.isp_cfg->module_ens &= ~(1LL << Rk_ISP21_DRC_ID);
        // need to update cmps_byp_en when drc en off
        pCvt->isp_params.isp_cfg->module_cfg_update |= (1LL << Rk_ISP21_DRC_ID);
    }
    pCvt->isp_params.isp_cfg->others.drc_cfg.bypass_en = pBase->bypass;
    rk_aiq_drc40_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo, pBase->en);
}
#endif

#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
void convertAiqRgbirToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP39_MODULE_RGBIR;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP39_MODULE_RGBIR;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_RGBIR;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP39_MODULE_RGBIR;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP39_MODULE_RGBIR;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_RGBIR;
        return;
    }

    rk_aiq_rgbir10_params_cvt(pBase->_data, &pCvt->isp_params);
}
#endif

#if RKAIQ_HAVE_CCM_V3
void convertAiqCcmToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP3X_MODULE_CCM;
    }
    pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_CCM;
    pCvt->isp_params.isp_cfg->module_cfg_update |= ISP3X_MODULE_CCM;

    rk_aiq_ccm22_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
}
#endif

#if (RKAIQ_HAVE_SHARP_V34)
static void convertAiqSharpToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP3X_MODULE_SHARP;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_SHARP;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP3X_MODULE_SHARP;
    }
    else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP3X_MODULE_SHARP;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_SHARP;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP3X_MODULE_SHARP;
        return;
    }
    pCvt->isp_params.isp_cfg->others.sharp_cfg.bypass = pBase->bypass;

    rk_aiq_sharp34_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
}
#endif

void convertAiqBlcToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_BLS;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_BLS;
    }
    pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_BLS;
    pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_BLS;

    rk_aiq_blc30_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo, &pCvt->mBlcInfo, &pCvt->awb_gain_final);

    LOGD_ABLC("frame_id:%d, blc_module_en:%d,fixed_val.rggb(%d,%d,%d,%d),bls1_en:(%d),isp_ob_offset:%d,bls1_val.rggb(%d,%d,%d,%d),isp_ob_predgain(%d),isp_ob_max(%d)",
              pCvt->isp_params.isp_cfg->frame_id, pBase->en,
              pCvt->isp_params.isp_cfg->others.bls_cfg.fixed_val.r, pCvt->isp_params.isp_cfg->others.bls_cfg.fixed_val.gr,
              pCvt->isp_params.isp_cfg->others.bls_cfg.fixed_val.gb, pCvt->isp_params.isp_cfg->others.bls_cfg.fixed_val.b,
              pCvt->isp_params.isp_cfg->others.bls_cfg.bls1_en, pCvt->isp_params.isp_cfg->others.bls_cfg.isp_ob_offset,
              pCvt->isp_params.isp_cfg->others.bls_cfg.bls1_val.r, pCvt->isp_params.isp_cfg->others.bls_cfg.bls1_val.gr,
              pCvt->isp_params.isp_cfg->others.bls_cfg.bls1_val.gb, pCvt->isp_params.isp_cfg->others.bls_cfg.bls1_val.b,
              pCvt->isp_params.isp_cfg->others.bls_cfg.isp_ob_predgain, pCvt->isp_params.isp_cfg->others.bls_cfg.isp_ob_max
             );
    /*if (!pBase->en) {
    #if ISP_HW_V35
        struct isp35_bls_cfg* blcCfg = &pCvt->isp_params.isp_cfg->others.bls_cfg;
    #else
        struct isp32_bls_cfg* blcCfg = &pCvt->isp_params.isp_cfg->others.bls_cfg;
    #endif
            blcCfg->fixed_val.r = 0;
            blcCfg->fixed_val.gr = 0;
            blcCfg->fixed_val.gb = 0;
            blcCfg->fixed_val.b = 0;
    }*/
    if (RK_AIQ_HDR_IS_SENSOR_BUILTIN(pCvt->_working_mode)) {
    #if ISP_HW_V35
            struct isp35_bls_cfg* blcCfg = &pCvt->isp_params.isp_cfg->others.bls_cfg;
    #else
            struct isp32_bls_cfg* blcCfg = &pCvt->isp_params.isp_cfg->others.bls_cfg;
    #endif
            if (pBase->en) {
                    blcCfg->bls1_en = true;
                    blcCfg->bls1_val.r=blcCfg->fixed_val.r;
                    blcCfg->bls1_val.gr=blcCfg->fixed_val.gr;
                    blcCfg->bls1_val.gb=blcCfg->fixed_val.gb;
                    blcCfg->bls1_val.b=blcCfg->fixed_val.b;
                    blcCfg->isp_ob_offset = 0;
                    blcCfg->fixed_val.r = 0;
                    blcCfg->fixed_val.gr = 0;
                    blcCfg->fixed_val.gb = 0;
                    blcCfg->fixed_val.b = 0;
                }
    }
    pCvt->mLatestBlsCfg = pCvt->isp_params.isp_cfg->others.bls_cfg;

}

#if RKAIQ_HAVE_LDC
static void convertAiqAldchToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase,
        bool is_multi_isp) {
    ldc_param_t* ldc_param = (ldc_param_t*)(pBase->_data);

    LOGD_ALDC("LDCH %s in params CVT", ldc_param->sta.ldchCfg.en ? "on" : "off");

    if (ldc_param->sta.ldchCfg.en) {
        pCvt->isp_params.isp_cfg->module_en_update |= ISP39_MODULE_LDCH;
        pCvt->isp_params.isp_cfg->module_ens |= ISP39_MODULE_LDCH;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_LDCH;
    } else {
        pCvt->isp_params.isp_cfg->module_en_update |= (ISP39_MODULE_LDCH);
        pCvt->isp_params.isp_cfg->module_ens &= ~(ISP39_MODULE_LDCH);
        pCvt->isp_params.isp_cfg->module_cfg_update &= ~(ISP39_MODULE_LDCH);
        return;
    }

    rk_aiq_ldch22_params_cvt(&pCvt->mCommonCvtInfo, &ldc_param->sta.ldchCfg, &pCvt->isp_params,
                             &pCvt->isp_params, is_multi_isp);
}

#if RKAIQ_HAVE_LDCV
static void convertAiqAldcvToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase,
        bool is_multi_isp) {
    ldc_param_t* ldc_param = (ldc_param_t*)(pBase->_data);

    LOGD_ALDC("LDCV %s in params CVT", ldc_param->sta.ldcvCfg.en ? "on" : "off");

    if (ldc_param->sta.ldcvCfg.en) {
        pCvt->isp_params.isp_cfg->module_en_update |= ISP39_MODULE_LDCV;
        pCvt->isp_params.isp_cfg->module_ens |= ISP39_MODULE_LDCV;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_LDCV;
    } else {
        pCvt->isp_params.isp_cfg->module_en_update |= (ISP39_MODULE_LDCV);
        pCvt->isp_params.isp_cfg->module_ens &= ~(ISP39_MODULE_LDCV);
        pCvt->isp_params.isp_cfg->module_cfg_update &= ~(ISP39_MODULE_LDCV);
        return;
    }

    rk_aiq_ldcv22_params_cvt(&pCvt->mCommonCvtInfo, &ldc_param->sta.ldcvCfg, &pCvt->isp_params,
                             &pCvt->isp_params, is_multi_isp);
}
#endif

void convertAiqAldcToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase,
                                 bool is_multi_isp) {
    convertAiqAldchToIsp39Params(pCvt, pBase, is_multi_isp);
#if RKAIQ_HAVE_LDCV
    convertAiqAldcvToIsp39Params(pCvt, pBase, is_multi_isp);
#endif
}
#endif

void convertAiqExpIspDgainToIsp39Params(AiqIspParamsCvt_t* pCvt,
                                        RKAiqAecExpInfo_t* ae_exp) {
    struct isp32_awb_gain_cfg* cfg      = &pCvt->mLatestWbGainCfg;
    struct isp32_awb_gain_cfg* dest_cfg = &pCvt->isp_params.isp_cfg->others.awb_gain_cfg;
    uint16_t max_wb_gain = (1 << (ISP2X_WBGAIN_FIXSCALE_BIT + ISP3X_WBGAIN_INTSCALE_BIT)) - 1;

    if (pCvt->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        float isp_dgain = MAX(1.0f, ae_exp->LinearExp.exp_real_params.isp_dgain);

        if (fabs(isp_dgain - pCvt->mLatestIspDgain) < FLT_EPSILON &&
                fabs(isp_dgain - 1.0f) < FLT_EPSILON)
            return;

        pCvt->mLatestIspDgain = isp_dgain;

        dest_cfg->gain0_red     = MIN(cfg->gain0_red * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain0_green_r = MIN(cfg->gain0_green_r * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain0_green_b = MIN(cfg->gain0_green_b * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain0_blue    = MIN(cfg->gain0_blue * isp_dgain + 0.5, max_wb_gain);

        dest_cfg->gain1_red     = MIN(cfg->gain1_red * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain1_green_r = MIN(cfg->gain1_green_r * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain1_green_b = MIN(cfg->gain1_green_b * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain1_blue    = MIN(cfg->gain1_blue * isp_dgain + 0.5, max_wb_gain);

        dest_cfg->gain2_red     = MIN(cfg->gain2_red * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain2_green_r = MIN(cfg->gain2_green_r * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain2_green_b = MIN(cfg->gain2_green_b * isp_dgain + 0.5, max_wb_gain);
        dest_cfg->gain2_blue    = MIN(cfg->gain2_blue * isp_dgain + 0.5, max_wb_gain);

        dest_cfg->awb1_gain_r  = cfg->awb1_gain_r;
        dest_cfg->awb1_gain_gr = cfg->awb1_gain_gr;
        dest_cfg->awb1_gain_b  = cfg->awb1_gain_b;
        dest_cfg->awb1_gain_gb = cfg->awb1_gain_gb;

        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_AWB_GAIN;
    } else {
        float isp_dgain0 = MAX(1.0f, ae_exp->HdrExp[0].exp_real_params.isp_dgain);
        float isp_dgain1 = MAX(1.0f, ae_exp->HdrExp[1].exp_real_params.isp_dgain);
        float isp_dgain2 = MAX(1.0f, ae_exp->HdrExp[2].exp_real_params.isp_dgain);

        float isp_dgain = isp_dgain0 + isp_dgain1 + isp_dgain2;
        if (fabs(isp_dgain - pCvt->mLatestIspDgain) < FLT_EPSILON &&
                fabs(isp_dgain - 3.0f) < FLT_EPSILON)
            return;
        pCvt->mLatestIspDgain = isp_dgain;

        if (pCvt->mCommonCvtInfo._airms_en || RK_AIQ_HDR_IS_SENSOR_BUILTIN(pCvt->_working_mode)) {
            uint16_t fixedGain1x = 1 << ISP2X_WBGAIN_FIXSCALE_BIT;
            dest_cfg->gain0_red     = MIN(fixedGain1x * isp_dgain0 + 0.5, max_wb_gain);
            dest_cfg->gain0_green_r = MIN(fixedGain1x * isp_dgain0 + 0.5, max_wb_gain);
            dest_cfg->gain0_green_b = MIN(fixedGain1x * isp_dgain0 + 0.5, max_wb_gain);
            dest_cfg->gain0_blue    = MIN(fixedGain1x * isp_dgain0 + 0.5, max_wb_gain);

            dest_cfg->gain1_red     = MIN(fixedGain1x * isp_dgain1 + 0.5, max_wb_gain);
            dest_cfg->gain1_green_r = MIN(fixedGain1x * isp_dgain1 + 0.5, max_wb_gain);
            dest_cfg->gain1_green_b = MIN(fixedGain1x * isp_dgain1 + 0.5, max_wb_gain);
            dest_cfg->gain1_blue    = MIN(fixedGain1x * isp_dgain1 + 0.5, max_wb_gain);

            dest_cfg->gain2_red     = MIN(fixedGain1x * isp_dgain2 + 0.5, max_wb_gain);
            dest_cfg->gain2_green_r = MIN(fixedGain1x * isp_dgain2 + 0.5, max_wb_gain);
            dest_cfg->gain2_green_b = MIN(fixedGain1x * isp_dgain2 + 0.5, max_wb_gain);
            dest_cfg->gain2_blue    = MIN(fixedGain1x * isp_dgain2 + 0.5, max_wb_gain);

            dest_cfg->awb1_gain_r  = fixedGain1x;
            dest_cfg->awb1_gain_gr = fixedGain1x;
            dest_cfg->awb1_gain_b  = fixedGain1x;
            dest_cfg->awb1_gain_gb = fixedGain1x;
        } else {
            dest_cfg->gain0_red     = MIN(cfg->gain0_red * isp_dgain0 + 0.5, max_wb_gain);
            dest_cfg->gain0_green_r = MIN(cfg->gain0_green_r * isp_dgain0 + 0.5, max_wb_gain);
            dest_cfg->gain0_green_b = MIN(cfg->gain0_green_b * isp_dgain0 + 0.5, max_wb_gain);
            dest_cfg->gain0_blue    = MIN(cfg->gain0_blue * isp_dgain0 + 0.5, max_wb_gain);

            dest_cfg->gain1_red     = MIN(cfg->gain1_red * isp_dgain1 + 0.5, max_wb_gain);
            dest_cfg->gain1_green_r = MIN(cfg->gain1_green_r * isp_dgain1 + 0.5, max_wb_gain);
            dest_cfg->gain1_green_b = MIN(cfg->gain1_green_b * isp_dgain1 + 0.5, max_wb_gain);
            dest_cfg->gain1_blue    = MIN(cfg->gain1_blue * isp_dgain1 + 0.5, max_wb_gain);

            dest_cfg->gain2_red     = MIN(cfg->gain2_red * isp_dgain2 + 0.5, max_wb_gain);
            dest_cfg->gain2_green_r = MIN(cfg->gain2_green_r * isp_dgain2 + 0.5, max_wb_gain);
            dest_cfg->gain2_green_b = MIN(cfg->gain2_green_b * isp_dgain2 + 0.5, max_wb_gain);
            dest_cfg->gain2_blue    = MIN(cfg->gain2_blue * isp_dgain2 + 0.5, max_wb_gain);

            dest_cfg->awb1_gain_r  = cfg->awb1_gain_r;
            dest_cfg->awb1_gain_gr = cfg->awb1_gain_gr;
            dest_cfg->awb1_gain_b  = cfg->awb1_gain_b;
            dest_cfg->awb1_gain_gb = cfg->awb1_gain_gb;
        }

        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_AWB_GAIN;
    }
}

void convertAiqCsmToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_CSM;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_CSM;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_CSM;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_CSM;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_CSM;
        return;
    }

    rk_aiq_csm21_params_cvt(pBase->_data, &pCvt->isp_params);
}
#if RKAIQ_HAVE_3DLUT_V1
static void convertAiq3dlutToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_3DLUT;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_3DLUT;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_3DLUT;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_3DLUT;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_3DLUT;
        return;
    }
    pCvt->isp_params.isp_cfg->others.isp3dlut_cfg.bypass_en = pBase->bypass;
    rk_aiq_lut3d20_params_cvt(pBase->_data, &pCvt->isp_params);
}
#endif
#if (RKAIQ_HAVE_YUVME_V1)
static void convertAiqYmeToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP39_MODULE_YUVME;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP39_MODULE_YUVME;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP39_MODULE_YUVME;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP39_MODULE_YUVME;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP39_MODULE_YUVME;
        return;
    }
    pCvt->isp_params.isp_cfg->others.yuvme_cfg.bypass = pBase->bypass;
    rk_aiq_yme10_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
}
#endif

#if RKAIQ_HAVE_DPCC_V2
void convertAiqDpccToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    // hwi_api_dpcc20_params_check(&dpcc_attr->result);

    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_DPCC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_DPCC;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_DPCC;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_DPCC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_DPCC;
        return;
    }

    rk_aiq_dpcc21_params_cvt(pBase->_data, &pCvt->isp_params);
    // hwi_api_dpcc20_params_cvt(&dpcc_attr->result, &pCvt->isp_params.isp_cfg);
    // hwi_api_dpcc20_params_dump(&dpcc_attr->result, &pCvt->isp_params.isp_cfg->others.dpcc_cfg);
}
#endif

#if RKAIQ_HAVE_GAMMA_V11
void convertAiqGammaToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_GOC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_GOC;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_GOC;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_GOC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_GOC;
        return;
    }

    if (pBase->bypass) {
        LOGW_AGAMMA(
            "The gamma doesn't support bypass feature, so you can set gamma coeff to 1.0 instead");
    }

    rk_aiq_gamma21_params_cvt(pBase->_data, &pCvt->isp_params);
}
#endif

#if RKAIQ_HAVE_LSC_V3
void convertAiqLscToIsp39Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_LSC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_LSC;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_LSC;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_LSC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_LSC;
        return;
    }

    rk_aiq_lsc21_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);
}
#endif

#if RKAIQ_HAVE_GIC_V2
static void convertAiqGicToIsp21Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_GIC;
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_GIC;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_GIC;
    } else {
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_GIC;
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_GIC;
        pCvt->isp_params.isp_cfg->module_cfg_update &= ~ISP2X_MODULE_GIC;
        return;
    }

    pCvt->isp_params.isp_cfg->others.gic_cfg.bypass_en = pBase->bypass;
    rk_aiq_gic21_params_cvt(pBase->_data, &pCvt->isp_params.isp_cfg->others.gic_cfg);
}
#endif

#if RKAIQ_HAVE_CGC_V1
void convertAiqCgcToIsp21Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_CGC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_CGC;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_CGC;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_CGC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_CGC;
        return;
    }

    rk_aiq_cgc10_params_cvt(pBase->_data, &pCvt->isp_params);
}
#endif

#if RKAIQ_HAVE_GAIN_V2
void convertAiqGainToIsp3xParams(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en && pCvt->mCommonCvtInfo.cnr_path_en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP3X_MODULE_GAIN;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP3X_MODULE_GAIN;
    }

    pCvt->isp_params.isp_cfg->module_en_update |= ISP3X_MODULE_GAIN;

    rk_aiq_gain20_params_cvt(pBase->_data, &pCvt->isp_params, &pCvt->mCommonCvtInfo);

    if (memcmp(&pCvt->mLatestGainCfg, &pCvt->isp_params.isp_cfg->others.gain_cfg, sizeof(pCvt->mLatestGainCfg)) != 0) {
        pCvt->mLatestGainCfg = pCvt->isp_params.isp_cfg->others.gain_cfg;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP3X_MODULE_GAIN;
    }
}
#endif

#if RKAIQ_HAVE_ACP_V10
void convertAiqCpToIsp20Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_CPROC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_CPROC;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_CPROC;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_CPROC;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_CPROC;
        return;
    }
    rk_aiq_cp10_params_cvt(pBase->_data, &pCvt->isp_params);
}
#endif

#if RKAIQ_HAVE_AIE_V10
void convertAiqIeToIsp20Params(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase) {
    if (pBase->en) {
        pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_IE;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_IE;
        pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_IE;
    } else {
        pCvt->isp_params.isp_cfg->module_ens &= ~ISP2X_MODULE_IE;
        pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_IE;
    }
}
#endif

#define CVT_INFO(_type, _func) \
    [_type] = {                \
        .type = _type,         \
        .name = #_type,        \
        .func = _func,         \
    }

struct params_cvt_info {
    int32_t type;
    const char* const name;
    void (*func)(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase);
};

static const struct params_cvt_info params_cvts[] = {
#if RKAIQ_HAVE_DEBAYER_V3
    CVT_INFO(RESULT_TYPE_DEBAYER_PARAM, convertAiqDmToIsp39Params),
#endif
    CVT_INFO(RESULT_TYPE_AESTATS_PARAM, convertAiqAeToIsp39Params),
#if ISP_HW_V39
    CVT_INFO(RESULT_TYPE_AWB_PARAM, convertAiqAwbToIsp39Params),
    CVT_INFO(RESULT_TYPE_AWBGAIN_PARAM, convertAiqAwbGainToIsp39Params),
#endif
    CVT_INFO(RESULT_TYPE_CCM_PARAM, convertAiqCcmToIsp39Params),
#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
    CVT_INFO(RESULT_TYPE_AF_PARAM, convertAiqAfToIsp39Params),
#endif
#if (RKAIQ_HAVE_BAYERTNR_V30)
    CVT_INFO(RESULT_TYPE_TNR_PARAM, convertAiqBtnrToIsp39Params),
#endif
#if RKAIQ_HAVE_YNR_V24
    CVT_INFO(RESULT_TYPE_YNR_PARAM, convertAiqYnrToIsp39Params),
#endif
#if (RKAIQ_HAVE_CNR_V31)
    CVT_INFO(RESULT_TYPE_UVNR_PARAM, convertAiqCnrToIsp39Params),
#endif
#if RKAIQ_HAVE_MERGE_V12
    CVT_INFO(RESULT_TYPE_MERGE_PARAM, convertAiqMergeToIsp39Params),
#endif
#if RKAIQ_HAVE_DEHAZE_V14
    CVT_INFO(RESULT_TYPE_DEHAZE_PARAM, convertAiqDehazeToIsp39Params),
    CVT_INFO(RESULT_TYPE_HISTEQ_PARAM, convertAiqHisteqToIsp39Params),
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
    CVT_INFO(RESULT_TYPE_RGBIR_PARAM, convertAiqRgbirToIsp39Params),
#endif
#if (RKAIQ_HAVE_SHARP_V34)
    CVT_INFO(RESULT_TYPE_SHARPEN_PARAM, convertAiqSharpToIsp39Params),
#endif
    CVT_INFO(RESULT_TYPE_BLC_PARAM, convertAiqBlcToIsp39Params),
    CVT_INFO(RESULT_TYPE_CSM_PARAM, convertAiqCsmToIsp39Params),
#if (RKAIQ_HAVE_YUVME_V1)
    CVT_INFO(RESULT_TYPE_MOTION_PARAM, convertAiqYmeToIsp39Params),
#endif
    CVT_INFO(RESULT_TYPE_DPCC_PARAM, convertAiqDpccToIsp39Params),
    CVT_INFO(RESULT_TYPE_AGAMMA_PARAM, convertAiqGammaToIsp39Params),
    CVT_INFO(RESULT_TYPE_LSC_PARAM, convertAiqLscToIsp39Params),
#if RKAIQ_HAVE_DRC_V20
    CVT_INFO(RESULT_TYPE_DRC_PARAM, convertAiqDrcToIsp39Params),
#endif
#if RKAIQ_HAVE_GIC_V2
    CVT_INFO(RESULT_TYPE_GIC_PARAM, convertAiqGicToIsp21Params),
#endif
    CVT_INFO(RESULT_TYPE_CGC_PARAM, convertAiqCgcToIsp21Params),
    CVT_INFO(RESULT_TYPE_CP_PARAM, convertAiqCpToIsp20Params),
    CVT_INFO(RESULT_TYPE_IE_PARAM, convertAiqIeToIsp20Params),
    CVT_INFO(RESULT_TYPE_GAIN_PARAM, convertAiqGainToIsp3xParams),
#if RKAIQ_HAVE_3DLUT_V1
    CVT_INFO(RESULT_TYPE_LUT3D_PARAM, convertAiq3dlutToIsp39Params),
#endif
};

static bool params_cvt_is_known(int32_t type) {
    if (type >= RESULT_TYPE_MAX_PARAM) return false;
    return params_cvts[type].type == type;
}

bool Convert3aResultsToIsp39Cfg(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* pBase,
                                void* isp_cfg_p, bool is_multi_isp) {
    struct isp39_isp_params_cfg* isp_cfg = (struct isp39_isp_params_cfg*)isp_cfg_p;

    switch (pBase->type) {
    case RESULT_TYPE_EXPOSURE_PARAM: {
        AiqSensorExpInfo_t* exp_info = (AiqSensorExpInfo_t*)pBase;
        convertAiqExpIspDgainToIsp39Params(pCvt, &exp_info->aecExpInfo);
    }
    break;
    case RESULT_TYPE_CAC_PARAM: {
#if RKAIQ_HAVE_CAC_V11
        struct isp39_isp_params_cfg* isp_cfg = (struct isp39_isp_params_cfg*)isp_cfg_p;
        struct isp39_isp_params_cfg* isp_cfg_right = isp_cfg + 1;
        convertAiqCacToIsp39Params(pCvt, pBase, isp_cfg, isp_cfg_right, is_multi_isp);
#endif
    }
    break;
#if RKAIQ_HAVE_LDC
    case RESULT_TYPE_LDC_PARAM:
        convertAiqAldcToIsp39Params(pCvt, pBase, is_multi_isp);
        break;
#endif
    default:
        if (params_cvt_is_known(pBase->type)) {
            const struct params_cvt_info* info = &params_cvts[pBase->type];
            info->func(pCvt, pBase);
            return true;
        } else {
            LOGE_CAMHW("unknown param type: 0x%x!", pBase->type);
            return false;
        }
    }
    return true;
}

#if RKAIQ_HAVE_DUMPSYS
static void UpdateModEn(struct isp39_isp_params_cfg* src, struct isp39_isp_params_cfg* dst) {
    LOG1_CAMHW("%s seq:%d, module_en_update:0x%llx\n", __func__, src->frame_id,
               src->module_en_update);

    for (int i = 0; i <= ISP2X_ID_MAX; i++) {
        if (src->module_en_update & BIT_ULL(i)) dst->module_ens |= src->module_ens & (1LL << i);
    }
}

static void UpdateMeasModCfg(struct isp39_isp_params_cfg* src, struct isp39_isp_params_cfg* dst) {
    u64 module_cfg_update = src->module_cfg_update;

    LOG1_CAMHW("%s seq:%d, module_cfg_update:0x%llx\n", __func__, src->frame_id,
               src->module_cfg_update);

    if (module_cfg_update & ISP39_MODULE_RAWAF) dst->meas.rawaf = src->meas.rawaf;

    if (module_cfg_update & ISP39_MODULE_RAWAE0) dst->meas.rawae0 = src->meas.rawae0;

    if (module_cfg_update & ISP39_MODULE_RAWAE3) dst->meas.rawae3 = src->meas.rawae3;

    if (module_cfg_update & ISP39_MODULE_RAWHIST0) dst->meas.rawhist0 = src->meas.rawhist0;

    if (module_cfg_update & ISP39_MODULE_RAWHIST3) dst->meas.rawhist3 = src->meas.rawhist3;

    if (module_cfg_update & ISP39_MODULE_RAWAWB) dst->meas.rawawb = src->meas.rawawb;
}

static void UpdateOthersModCfg(struct isp39_isp_params_cfg* src, struct isp39_isp_params_cfg* dst) {
    u64 module_cfg_update = src->module_cfg_update;

    LOG1_CAMHW("%s seq:%d, module_cfg_update:0x%llx\n", __func__, src->frame_id,
               src->module_cfg_update);

    if (module_cfg_update & ISP39_MODULE_LSC) dst->others.lsc_cfg = src->others.lsc_cfg;

    if (module_cfg_update & ISP39_MODULE_DPCC) dst->others.dpcc_cfg = src->others.dpcc_cfg;

    if (module_cfg_update & ISP39_MODULE_BLS) dst->others.bls_cfg = src->others.bls_cfg;

    if (module_cfg_update & ISP39_MODULE_SDG) dst->others.sdg_cfg = src->others.sdg_cfg;

    if (module_cfg_update & ISP39_MODULE_AWB_GAIN)
        dst->others.awb_gain_cfg = src->others.awb_gain_cfg;

    if (module_cfg_update & ISP39_MODULE_DEBAYER) dst->others.debayer_cfg = src->others.debayer_cfg;

    if (module_cfg_update & ISP39_MODULE_CCM) dst->others.ccm_cfg = src->others.ccm_cfg;

    if (module_cfg_update & ISP39_MODULE_GOC) dst->others.gammaout_cfg = src->others.gammaout_cfg;

    if (module_cfg_update & ISP39_MODULE_CSM) dst->others.csm_cfg = src->others.csm_cfg;

    if (module_cfg_update & ISP39_MODULE_CGC) dst->others.cgc_cfg = src->others.cgc_cfg;

    if (module_cfg_update & ISP39_MODULE_CPROC) dst->others.cproc_cfg = src->others.cproc_cfg;

    if (module_cfg_update & ISP39_MODULE_HDRMGE) dst->others.hdrmge_cfg = src->others.hdrmge_cfg;

    if (module_cfg_update & ISP39_MODULE_DRC) dst->others.drc_cfg = src->others.drc_cfg;

    if (module_cfg_update & ISP39_MODULE_GIC) dst->others.gic_cfg = src->others.gic_cfg;

    if (module_cfg_update & ISP39_MODULE_DHAZ) dst->others.dhaz_cfg = src->others.dhaz_cfg;

    if (module_cfg_update & ISP39_MODULE_3DLUT) dst->others.isp3dlut_cfg = src->others.isp3dlut_cfg;

    if (module_cfg_update & ISP39_MODULE_LDCH) dst->others.ldch_cfg = src->others.ldch_cfg;

    if (module_cfg_update & ISP39_MODULE_LDCV) dst->others.ldcv_cfg = src->others.ldcv_cfg;

    if (module_cfg_update & ISP39_MODULE_YNR) dst->others.ynr_cfg = src->others.ynr_cfg;

    if (module_cfg_update & ISP39_MODULE_CNR) dst->others.cnr_cfg = src->others.cnr_cfg;

    if (module_cfg_update & ISP39_MODULE_SHARP) dst->others.sharp_cfg = src->others.sharp_cfg;

    if (module_cfg_update & ISP39_MODULE_BAY3D) dst->others.bay3d_cfg = src->others.bay3d_cfg;

    if (module_cfg_update & ISP39_MODULE_YUVME) dst->others.yuvme_cfg = src->others.yuvme_cfg;

    if (module_cfg_update & ISP39_MODULE_RGBIR) dst->others.rgbir_cfg = src->others.rgbir_cfg;

    if (module_cfg_update & ISP39_MODULE_CAC) dst->others.cac_cfg = src->others.cac_cfg;

    if (module_cfg_update & ISP39_MODULE_GAIN) dst->others.gain_cfg = src->others.gain_cfg;
}

void AiqIspParamsCvt_updIsp39Params(void* src, void* dst) {
    if (!src || !dst) return;

    struct isp39_isp_params_cfg* pSrc = (struct isp39_isp_params_cfg*)src;
    struct isp39_isp_params_cfg* pDst = (struct isp39_isp_params_cfg*)dst;

    pDst->frame_id          = pSrc->frame_id;
    pDst->module_en_update  = pSrc->module_en_update;
    pDst->module_cfg_update = pSrc->module_cfg_update;

    UpdateModEn(pSrc, pDst);
    UpdateMeasModCfg(pSrc, pDst);
    UpdateOthersModCfg(pSrc, pDst);
}
#endif
