/*
 * rk_aiq_algo_debayer_itf.c
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

#include "rk_aiq_algo_types.h"
#include "dehaze_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqHandle.h"

#include "interpolation.h"

//RKAIQ_BEGIN_DECLARE
#if RKAIQ_HAVE_DEHAZE_V12
XCamReturn DehazeSelectParam(DehazeContext_t *pDehazeCtx, dehaze_param_t* out, int iso, bool stats_true,
    dehaze_stats_v12_t* pStats);
static XCamReturn TransferHistWrSemiAtuos2Res(dehaze_param_t* out,
                                        dehaze_mhist_wr_semiauto_t* pSemiAutoCurve,
                                        dehaze_stats_v12_t* pStats, bool stats_true);
static void calc_cdf(int* hist, int* cdf, int bin_num, float clim, uint16_t vmin, uint16_t vmax);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
XCamReturn DehazeSelectParam(DehazeContext_t *pDehazeCtx, dehaze_param_t* out, int iso);
#endif
static XCamReturn
create_context
(
    RkAiqAlgoContext** context,
    const AlgoCtxInstanceCfg* cfg
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    DehazeContext_t *ctx = new DehazeContext_t();
    if (ctx == NULL) {
        LOGE_ADEHAZE( "%s: create Dehaze context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->dehaze_attrib =
        (dehaze_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, dehaze));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ADEHAZE("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context
(
    RkAiqAlgoContext* context
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DehazeContext_t* pDehazeCtx = (DehazeContext_t*)context;
    delete pDehazeCtx;
    return result;
}

static XCamReturn
prepare
(
    RkAiqAlgoCom* params
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    DehazeContext_t* pDehazeCtx = (DehazeContext_t*)params->ctx;

    if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pDehazeCtx->dehaze_attrib =
                (dehaze_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, dehaze));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pDehazeCtx->working_mode      = params->u.prepare.working_mode;
    pDehazeCtx->width             = params->u.prepare.sns_op_width;
    pDehazeCtx->height            = params->u.prepare.sns_op_height;
    pDehazeCtx->dehaze_attrib =
        (dehaze_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, dehaze));
    pDehazeCtx->prepare_params = &params->u.prepare;
    pDehazeCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DehazeContext_t* pDehazeCtx = (DehazeContext_t*)inparams->ctx;
    dehaze_api_attrib_t* dehaze_attrib = pDehazeCtx->dehaze_attrib;
    RkAiqAlgoProcResDehaze* pDehazeProcResParams = (RkAiqAlgoProcResDehaze*)outparams;
    RkAiqAlgoProcDehaze* dehaze_proc_param = (RkAiqAlgoProcDehaze*)inparams;

    LOGV_ADEHAZE("%s: (enter)\n", __FUNCTION__ );

    if (!dehaze_attrib) {
        LOGE_ADEHAZE("dehaze_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    int iso = inparams->u.proc.iso;
    if (pDehazeCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL&&
        dehaze_proc_param->blc_ob_enable &&
        dehaze_proc_param->isp_ob_predgain >= ISP_PREDGAIN_DEFAULT)
        iso = iso * dehaze_proc_param->isp_ob_predgain;
    bool init = inparams->u.proc.init;
    float delta_iso = (float)abs(iso - pDehazeCtx->iso);
    delta_iso /= (float)pDehazeCtx->iso;
    // get ynr res
    for (int i = 0; i < YNR_ISO_CURVE_POINT_NUM; i++)
        pDehazeCtx->YnrProcRes_sigma[i] = dehaze_proc_param->sigma[i];
    
    outparams->cfg_update = false;

    if (inparams->u.proc.is_bw_sensor) {
        dehaze_attrib->en = false;
        outparams->cfg_update = init ? true : false;
        return XCAM_RETURN_NO_ERROR;
    }

    if (dehaze_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ADEHAZE("mode is %d, not auto mode, ignore", dehaze_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pDehazeCtx->isReCal_ = true;
    }

    if (delta_iso >= pDehazeCtx->dehaze_attrib->stAuto.sta.sw_dehaze_byPass_thred) {
        pDehazeCtx->isReCal_ = true;
    }

    if (pDehazeCtx->isReCal_) {
#if RKAIQ_HAVE_DEHAZE_V12
        DehazeSelectParam(pDehazeCtx, pDehazeProcResParams->dehazeRes, iso, dehaze_proc_param->stats_true, dehaze_proc_param->dehaze_stats_v12);
#endif
#if RKAIQ_HAVE_DEHAZE_V14
        DehazeSelectParam(pDehazeCtx, pDehazeProcResParams->dehazeRes, iso);
#endif
        outparams->cfg_update = true;
        outparams->en = dehaze_attrib->en;
        outparams->bypass = dehaze_attrib->bypass;
        LOGI_ADEHAZE("dehaze en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pDehazeCtx->iso = iso;
    pDehazeCtx->isReCal_ = false;

    LOGV_ADEHAZE("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_DEHAZE_V12
XCamReturn DehazeSelectParam
(
    DehazeContext_t *pDehazeCtx,
    dehaze_param_t* out, int iso, bool stats_true,
    dehaze_stats_v12_t* pStats)
{
    LOGI_ADEHAZE("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pDehazeCtx == NULL) {
        LOGE_ADEHAZE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    dehaze_param_auto_t* paut = &pDehazeCtx->dehaze_attrib->stAuto;

    if (paut->sta.dehaze_setting.en && paut->sta.enhance_setting.en) {
        out->sta.dc_en      = FUNCTION_ENABLE;
        out->sta.enhance_en = FUNCTION_ENABLE;
    } else if (paut->sta.dehaze_setting.en && !paut->sta.enhance_setting.en) {
        out->sta.dc_en      = FUNCTION_ENABLE;
        out->sta.enhance_en = FUNCTION_DISABLE;
    } else if (!paut->sta.dehaze_setting.en && paut->sta.enhance_setting.en) {
        out->sta.dc_en      = FUNCTION_ENABLE;
        out->sta.enhance_en = FUNCTION_ENABLE;
    } else {
        out->sta.dc_en      = FUNCTION_DISABLE;
        out->sta.enhance_en = FUNCTION_DISABLE;
    }
    if (paut->sta.hist_setting.en)
        out->sta.hist_en = FUNCTION_ENABLE;
    else
        out->sta.hist_en = FUNCTION_DISABLE;

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);

    out->sta.cfg_alpha = paut->sta.cfg_alpha;
    out->sta.round_en = FUNCTION_ENABLE;
    // dehaze setting
    if (out->sta.dc_en){
        out->dyn.DehazeData.dc_min_th = interpolation_f32(
            paut->dyn[ilow].DehazeData.dc_min_th, paut->dyn[ihigh].DehazeData.dc_min_th, ratio);
        out->dyn.DehazeData.dc_max_th = interpolation_f32(
            paut->dyn[ilow].DehazeData.dc_max_th, paut->dyn[ihigh].DehazeData.dc_max_th, ratio);
        out->dyn.DehazeData.yhist_th = interpolation_f32(
            paut->dyn[ilow].DehazeData.yhist_th, paut->dyn[ihigh].DehazeData.yhist_th, ratio);
        out->dyn.DehazeData.yblk_th = interpolation_f32(
            paut->dyn[ilow].DehazeData.yblk_th, paut->dyn[ihigh].DehazeData.yblk_th, ratio) *
            ((pDehazeCtx->width + 15) / 16) * ((pDehazeCtx->height + 15) / 16);
        out->dyn.DehazeData.dark_th = interpolation_f32(
            paut->dyn[ilow].DehazeData.dark_th, paut->dyn[ihigh].DehazeData.dark_th, ratio);
        out->dyn.DehazeData.bright_min = interpolation_f32(
            paut->dyn[ilow].DehazeData.bright_min, paut->dyn[ihigh].DehazeData.bright_min, ratio);
        out->dyn.DehazeData.bright_max = interpolation_f32(
            paut->dyn[ilow].DehazeData.bright_max, paut->dyn[ihigh].DehazeData.bright_max, ratio);
        out->dyn.DehazeData.wt_max = interpolation_f32(
            paut->dyn[ilow].DehazeData.wt_max, paut->dyn[ihigh].DehazeData.wt_max, ratio);
        out->dyn.DehazeData.air_min = interpolation_f32(
            paut->dyn[ilow].DehazeData.air_min, paut->dyn[ihigh].DehazeData.air_min, ratio);
        out->dyn.DehazeData.air_max = interpolation_f32(
            paut->dyn[ilow].DehazeData.air_max, paut->dyn[ihigh].DehazeData.air_max, ratio);
        out->dyn.DehazeData.tmax_base = interpolation_f32(
            paut->dyn[ilow].DehazeData.tmax_base, paut->dyn[ihigh].DehazeData.tmax_base, ratio);
        out->dyn.DehazeData.tmax_off = interpolation_f32(
            paut->dyn[ilow].DehazeData.tmax_off, paut->dyn[ihigh].DehazeData.tmax_off, ratio);
        out->dyn.DehazeData.tmax_max = interpolation_f32(
            paut->dyn[ilow].DehazeData.tmax_max, paut->dyn[ihigh].DehazeData.tmax_max, ratio);
        out->dyn.DehazeData.cfg_wt = interpolation_f32(
            paut->dyn[ilow].DehazeData.cfg_wt, paut->dyn[ihigh].DehazeData.cfg_wt, ratio);
        out->dyn.DehazeData.cfg_air = interpolation_f32(
            paut->dyn[ilow].DehazeData.cfg_air, paut->dyn[ihigh].DehazeData.cfg_air, ratio);
        out->dyn.DehazeData.cfg_tmax = interpolation_f32(
            paut->dyn[ilow].DehazeData.cfg_tmax, paut->dyn[ihigh].DehazeData.cfg_tmax, ratio);
        out->dyn.DehazeData.range_sigma = interpolation_f32(
            paut->dyn[ilow].DehazeData.range_sigma, paut->dyn[ihigh].DehazeData.range_sigma, ratio);
        out->dyn.DehazeData.space_sigma_cur = interpolation_f32(
            paut->dyn[ilow].DehazeData.space_sigma_cur, paut->dyn[ihigh].DehazeData.space_sigma_cur, ratio);
        out->dyn.DehazeData.space_sigma_pre = interpolation_f32(
            paut->dyn[ilow].DehazeData.space_sigma_pre, paut->dyn[ihigh].DehazeData.space_sigma_pre, ratio);
        out->dyn.DehazeData.bf_weight = interpolation_f32(
            paut->dyn[ilow].DehazeData.bf_weight, paut->dyn[ihigh].DehazeData.bf_weight, ratio);
        out->dyn.DehazeData.dc_weitcur = interpolation_f32(
            paut->dyn[ilow].DehazeData.dc_weitcur, paut->dyn[ihigh].DehazeData.dc_weitcur, ratio);
        out->sta.dehaze_setting.air_lc_en =
            paut->sta.dehaze_setting.air_lc_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        out->sta.dehaze_setting.stab_fnum = paut->sta.dehaze_setting.stab_fnum;
        out->sta.dehaze_setting.iir_sigma = paut->sta.dehaze_setting.iir_sigma;
        out->sta.dehaze_setting.iir_wt_sigma = paut->sta.dehaze_setting.iir_wt_sigma;
        out->sta.dehaze_setting.iir_air_sigma = paut->sta.dehaze_setting.iir_air_sigma;
        out->sta.dehaze_setting.iir_tmax_sigma = paut->sta.dehaze_setting.iir_tmax_sigma;
        out->sta.dehaze_setting.iir_pre_wet = paut->sta.dehaze_setting.iir_pre_wet;
        out->sta.dehaze_setting.gaus_h0 = paut->sta.dehaze_setting.gaus_h0;
        out->sta.dehaze_setting.gaus_h1 = paut->sta.dehaze_setting.gaus_h1;
        out->sta.dehaze_setting.gaus_h2 = paut->sta.dehaze_setting.gaus_h2;
    }

    // enhance setting
    if (out->sta.dc_en) {
        //pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
        out->dyn.EnhanceData.enhance_value = interpolation_f32(
            paut->dyn[ilow].EnhanceData.enhance_value, paut->dyn[ihigh].EnhanceData.enhance_value, ratio);
        out->dyn.EnhanceData.enhance_chroma = interpolation_f32(
            paut->dyn[ilow].EnhanceData.enhance_chroma, paut->dyn[ihigh].EnhanceData.enhance_chroma, ratio);
        for (int j = 0; j < DHAZ_ENHANCE_CURVE_KNOTS_NUM; j++) {
            out->dyn.EnhanceData.enhance_curve[j] = interpolation_f32(
                paut->dyn[ilow].EnhanceData.enhance_curve[j], paut->dyn[ihigh].EnhanceData.enhance_curve[j], ratio);
        }
        for (int j = 0; j < DHAZ_ENHANCE_CURVE_KNOTS_NUM; j++) {
            out->dyn.EnhanceData.enh_luma[j] = interpolation_f32(
                paut->dyn[ilow].EnhanceData.enh_luma[j], paut->dyn[ihigh].EnhanceData.enh_luma[j], ratio);
        }
        // dehaze v12 add
        out->sta.enhance_setting.color_deviate_en =
            paut->sta.enhance_setting.color_deviate_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        out->sta.enhance_setting.enh_luma_en =
            paut->sta.enhance_setting.enh_luma_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    }

    // hist setting
    if (out->sta.hist_en) {
        //pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
        out->dyn.HistData.hist_gratio = interpolation_f32(
                paut->dyn[ilow].HistData.hist_gratio, paut->dyn[ihigh].HistData.hist_gratio, ratio);
        out->dyn.HistData.hist_th_off = interpolation_f32(
                paut->dyn[ilow].HistData.hist_th_off, paut->dyn[ihigh].HistData.hist_th_off, ratio);
        out->dyn.HistData.hist_k = interpolation_f32(
                paut->dyn[ilow].HistData.hist_k, paut->dyn[ihigh].HistData.hist_k, ratio);
        out->dyn.HistData.hist_min = interpolation_f32(
                paut->dyn[ilow].HistData.hist_min, paut->dyn[ihigh].HistData.hist_min, ratio);
        out->dyn.HistData.cfg_gratio = interpolation_f32(
                paut->dyn[ilow].HistData.cfg_gratio, paut->dyn[ihigh].HistData.cfg_gratio, ratio);
        out->dyn.HistData.hist_scale = interpolation_f32(
                paut->dyn[ilow].HistData.hist_scale, paut->dyn[ihigh].HistData.hist_scale, ratio);
        out->sta.hist_setting.hist_para_en =
            paut->sta.hist_setting.hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        // clip hpara_en
        out->sta.hist_setting.hist_para_en = out->sta.dc_en ? out->sta.hist_setting.hist_para_en
                                                   : FUNCTION_ENABLE;
        if (paut->sta.hist_setting.hist_sta_wr.mode == dehaze_HIST_WR_MANUAL) {
            out->sta.soft_wr_en = FUNCTION_ENABLE;
        } else if (paut->sta.hist_setting.hist_sta_wr.mode == dehaze_HIST_WR_AUTO) {
            out->sta.soft_wr_en = FUNCTION_DISABLE;
        } else if (paut->sta.hist_setting.hist_sta_wr.mode == dehaze_HIST_WR_SEMIAUTO) {
            dehaze_mhist_wr_semiauto_t semi_auto_curve;
            //pre_interp(iso, NULL, 0, ilow, ihigh, ratio);
            semi_auto_curve.clim0 = interpolation_f32(
                paut->dyn[ilow].hist_dyn_wr.semiauto_curve.clim0, paut->dyn[ihigh].hist_dyn_wr.semiauto_curve.clim0, ratio);
            semi_auto_curve.clim1 = interpolation_f32(
                paut->dyn[ilow].hist_dyn_wr.semiauto_curve.clim1, paut->dyn[ihigh].hist_dyn_wr.semiauto_curve.clim1, ratio);
            semi_auto_curve.dark_th = interpolation_f32(
                paut->dyn[ilow].hist_dyn_wr.semiauto_curve.dark_th, paut->dyn[ihigh].hist_dyn_wr.semiauto_curve.dark_th, ratio);
            XCamReturn ret;
            if (stats_true)
                ret = TransferHistWrSemiAtuos2Res(out, &semi_auto_curve, pStats, stats_true);
            else
                ret = XCAM_RETURN_ERROR_PARAM;

            if (ret == XCAM_RETURN_NO_ERROR)
                out->sta.soft_wr_en = FUNCTION_ENABLE;
            else
                out->sta.soft_wr_en = FUNCTION_DISABLE;
        }
    }

    // get sigma_idx
    for (int i = 0; i < DHAZ_SIGMA_IDX_NUM; i++)
        out->sta.sigma_idx[i] = (i + 1) * YNR_CURVE_STEP;
    // get sigma_lut
    float sigam_total = 0.0f;
    for (int i = 0; i < DHAZ_SIGMA_IDX_NUM; i++) sigam_total += pDehazeCtx->YnrProcRes_sigma[i];
    if (sigam_total < FLT_EPSILON) {
        for (int i = 0; i < DHAZ_SIGMA_IDX_NUM; i++) out->sta.sigma_lut[i] = 0x200;
    } else {
        for (int i = 0; i < DHAZ_SIGMA_LUT_NUM; i++) {
            out->sta.sigma_lut[i] = pDehazeCtx->YnrProcRes_sigma[i];
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn TransferHistWrSemiAtuos2Res(dehaze_param_t* out,
                                       dehaze_mhist_wr_semiauto_t* pSemiAutoCurve,
                                       dehaze_stats_v12_t* pStats, bool stats_true) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    if (!stats_true) return XCAM_RETURN_BYPASS;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int i;
    int hist[64];
    int cdf_out[64];
    int dark_idx = 1;
    int mean     = 0;
    static bool first_time = true;

    if (first_time) {
        for (i = 0; i < 63; ++i) {
            out->sta.hist_setting.hist_sta_wr.hist_wr[i] = (i + 1) * 16;
        }
        out->sta.hist_setting.hist_sta_wr.hist_wr[i] = 1023;
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
        hist[i] = LIMIT_VALUE_UNSIGNED(pStats->h_rgb_iir[i] - pStats->h_rgb_iir[i - 1], 1023);
        mean += (i + 1) * 16 * hist[i];
    }

    hist[0] = pStats->h_rgb_iir[0];
    mean += 16 * hist[0];
    mean = mean >> 10;
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
        out->sta.hist_setting.hist_sta_wr.hist_wr[i] = 
            (7 * out->sta.hist_setting.hist_sta_wr.hist_wr[i] + cdf_out[i] + 4) / 8;
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

static void calc_cdf(int* hist, int* cdf, int bin_num, float clim, uint16_t vmin, uint16_t vmax) {
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
#endif
#if RKAIQ_HAVE_DEHAZE_V14
XCamReturn DehazeSelectParam
(
    DehazeContext_t* pDehazeCtx,
    dehaze_param_t* out, int iso)
{
    LOGI_ADEHAZE("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pDehazeCtx == NULL) {
        LOGE_ADEHAZE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    dehaze_param_auto_t* paut = &pDehazeCtx->dehaze_attrib->stAuto;

    if (paut->sta.hw_dehaze_setting.en && paut->sta.hw_enhance_setting.en) {
        out->sta.dc_en = FUNCTION_ENABLE;
        out->sta.enhance_en = FUNCTION_ENABLE;
    } else if (paut->sta.hw_dehaze_setting.en && !paut->sta.hw_enhance_setting.en) {
        out->sta.dc_en      = FUNCTION_ENABLE;
        out->sta.enhance_en = FUNCTION_DISABLE;
    } else if (!paut->sta.hw_dehaze_setting.en && paut->sta.hw_enhance_setting.en) {
        out->sta.dc_en      = FUNCTION_ENABLE;
        out->sta.enhance_en = FUNCTION_ENABLE;
    } else {
        out->sta.dc_en      = FUNCTION_DISABLE;
        out->sta.enhance_en = FUNCTION_DISABLE;
    }
    if (paut->sta.hw_hist_setting.en)
        out->sta.hist_en = FUNCTION_ENABLE;
    else
        out->sta.hist_en = FUNCTION_DISABLE;

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    pre_interp(iso, NULL, 0, ilow, ihigh, ratio);

    if (paut->sta.hw_dehaze_setting.en || paut->sta.hw_enhance_setting.en
        || paut->sta.hw_hist_setting.en) {
        out->sta.mem_mode = 0x0;
        out->sta.mem_force = 0x0;
        out->sta.round_en  = FUNCTION_ENABLE;

        out->dyn.hw_commom_setting.hw_contrast_ThumbFlt_invVsigma = interpolation_f32(
            paut->dyn[ilow].hw_commom_setting.hw_contrast_ThumbFlt_invVsigma,
            paut->dyn[ihigh].hw_commom_setting.hw_contrast_ThumbFlt_invVsigma, ratio);
        out->dyn.hw_commom_setting.hw_contrast_curThumbFlt_invRsgm = interpolation_f32(
            paut->dyn[ilow].hw_commom_setting.hw_contrast_curThumbFlt_invRsgm,
            paut->dyn[ihigh].hw_commom_setting.hw_contrast_curThumbFlt_invRsgm, ratio);
        out->dyn.hw_commom_setting.hw_contrast_preThumbFlt_invRsgm =interpolation_f32(
            paut->dyn[ilow].hw_commom_setting.hw_contrast_preThumbFlt_invRsgm,
            paut->dyn[ihigh].hw_commom_setting.hw_contrast_preThumbFlt_invRsgm, ratio);
        out->dyn.hw_commom_setting.hw_contrast_thumbMerge_wgt =interpolation_f32(
            paut->dyn[ilow].hw_commom_setting.hw_contrast_thumbMerge_wgt,
            paut->dyn[ihigh].hw_commom_setting.hw_contrast_thumbMerge_wgt, ratio);
        out->dyn.hw_commom_setting.hw_contrast_thumbFlt_curWgt =interpolation_f32(
            paut->dyn[ilow].hw_commom_setting.hw_contrast_thumbFlt_curWgt,
            paut->dyn[ihigh].hw_commom_setting.hw_contrast_thumbFlt_curWgt, ratio);
        out->dyn.hw_commom_setting.hw_dehaze_user_gainFuse = interpolation_f32(
            paut->dyn[ilow].hw_commom_setting.hw_dehaze_user_gainFuse,
            paut->dyn[ihigh].hw_commom_setting.hw_dehaze_user_gainFuse, ratio);
        out->dyn.hw_commom_setting.sw_contrast_paramTflt_curWgt =interpolation_u16(
            paut->dyn[ilow].hw_commom_setting.sw_contrast_paramTflt_curWgt,
            paut->dyn[ihigh].hw_commom_setting.sw_contrast_paramTflt_curWgt, ratio);
        out->dyn.hw_commom_setting.sw_contrast_thumbTflt_curWgt =interpolation_u16(
            paut->dyn[ilow].hw_commom_setting.sw_contrast_thumbTflt_curWgt,
            paut->dyn[ihigh].hw_commom_setting.sw_contrast_thumbTflt_curWgt, ratio);

        out->sta.gaus_h0 = DEHAZE_GAUS_H4;
        out->sta.gaus_h1 = DEHAZE_GAUS_H1;
        out->sta.gaus_h2 = DEHAZE_GAUS_H0;
    }

    if (out->sta.hw_dehaze_setting.en) {
        out->sta.hw_dehaze_setting.hw_dehaze_luma_mode =
            paut->sta.hw_dehaze_setting.hw_dehaze_luma_mode ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        out->dyn.hw_dehaze_params.hw_dehaze_darkCh_minThed = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_darkCh_minThed,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_darkCh_minThed, ratio);
        out->dyn.hw_dehaze_params.hw_dehaze_darkCh_maxThed = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_darkCh_maxThed,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_darkCh_maxThed, ratio);
        out->dyn.hw_dehaze_params.hw_dehaze_lumaCount_maxThed = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_lumaCount_maxThed,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_lumaCount_maxThed, ratio);
        out->dyn.hw_dehaze_params.sw_dehaze_lumaCount_minRatio =interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.sw_dehaze_lumaCount_minRatio,
            paut->dyn[ihigh].hw_dehaze_params.sw_dehaze_lumaCount_minRatio, ratio) *
            ((pDehazeCtx->width + 15) / 16) * ((pDehazeCtx->height + 15) / 16);;
        out->dyn.hw_dehaze_params.hw_dehaze_darkArea_thed = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_darkArea_thed,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_darkArea_thed, ratio);
        out->dyn.hw_dehaze_params.hw_dehaze_bright_minLimit = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_bright_minLimit,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_bright_minLimit, ratio);
        out->dyn.hw_dehaze_params.hw_dehaze_bright_maxLimit = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_bright_maxLimit,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_bright_maxLimit, ratio);
        out->dyn.hw_dehaze_params.sw_dehaze_invContrast_scale = interpolation_f32(
            paut->dyn[ilow].hw_dehaze_params.sw_dehaze_invContrast_scale,
            paut->dyn[ihigh].hw_dehaze_params.sw_dehaze_invContrast_scale, ratio);
        out->dyn.hw_dehaze_params.hw_dehaze_airLight_minLimit = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_airLight_minLimit,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_airLight_minLimit, ratio);
        out->dyn.hw_dehaze_params.hw_dehaze_airLight_maxLimit = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_airLight_maxLimit,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_airLight_maxLimit, ratio);
        out->dyn.hw_dehaze_params.sw_dehaze_airLight_scale = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.sw_dehaze_airLight_scale,
            paut->dyn[ihigh].hw_dehaze_params.sw_dehaze_airLight_scale, ratio);
        out->dyn.hw_dehaze_params.sw_dehaze_transRatio_offset = interpolation_f32(
            paut->dyn[ilow].hw_dehaze_params.sw_dehaze_transRatio_offset,
            paut->dyn[ihigh].hw_dehaze_params.sw_dehaze_transRatio_offset, ratio);
        out->dyn.hw_dehaze_params.sw_dehaze_transRatio_maxLimit = interpolation_f32(
            paut->dyn[ilow].hw_dehaze_params.sw_dehaze_transRatio_maxLimit,
            paut->dyn[ihigh].hw_dehaze_params.sw_dehaze_transRatio_maxLimit, ratio);
        out->dyn.hw_dehaze_params.sw_dehaze_paramMerge_alpha = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.sw_dehaze_paramMerge_alpha,
            paut->dyn[ihigh].hw_dehaze_params.sw_dehaze_paramMerge_alpha, ratio);
        out->dyn.hw_dehaze_params.sw_dehaze_userInvContrast = interpolation_f32(
            paut->dyn[ilow].hw_dehaze_params.sw_dehaze_userInvContrast,
            paut->dyn[ihigh].hw_dehaze_params.sw_dehaze_userInvContrast, ratio);
        out->dyn.hw_dehaze_params.hw_dehaze_userAirLight = interpolation_u8(
            paut->dyn[ilow].hw_dehaze_params.hw_dehaze_userAirLight,
            paut->dyn[ihigh].hw_dehaze_params.hw_dehaze_userAirLight, ratio);
        out->dyn.hw_dehaze_params.sw_dehaze_userTransRatio = interpolation_f32(
            paut->dyn[ilow].hw_dehaze_params.sw_dehaze_userTransRatio,
            paut->dyn[ihigh].hw_dehaze_params.sw_dehaze_userTransRatio, ratio);
        out->sta.hw_dehaze_setting.sw_dhaz_invContrastTflt_invSgm =
            paut->sta.hw_dehaze_setting.sw_dhaz_invContrastTflt_invSgm;
        out->sta.hw_dehaze_setting.sw_dhaz_airLightTflt_invSgm =
            paut->sta.hw_dehaze_setting.sw_dhaz_airLightTflt_invSgm;
        out->sta.hw_dehaze_setting.sw_dhaz_transRatioTflt_invSgm =
            paut->sta.hw_dehaze_setting.sw_dhaz_transRatioTflt_invSgm;
    }

    if (out->sta.hw_enhance_setting.en) {
        // color_deviate_en
        out->sta.hw_enhance_setting.hw_enhance_cProtect_en =
            paut->sta.hw_enhance_setting.hw_enhance_cProtect_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        // enh_luma_en
        out->sta.hw_enhance_setting.hw_enhance_luma2strg_en =
            paut->sta.hw_enhance_setting.hw_enhance_luma2strg_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        out->dyn.hw_enhance_params.sw_enhance_contrast_strg = interpolation_f32(
            paut->dyn[ilow].hw_enhance_params.sw_enhance_contrast_strg,
            paut->dyn[ihigh].hw_enhance_params.sw_enhance_contrast_strg, ratio);
        // enhance_chroma
        out->dyn.hw_enhance_params.sw_enhance_saturate_strg = interpolation_f32(
            paut->dyn[ilow].hw_enhance_params.sw_enhance_saturate_strg,
            paut->dyn[ihigh].hw_enhance_params.sw_enhance_saturate_strg, ratio);
        // enh_curve
        for (int j = 0; j < DHAZ_ENHANCE_CURVE_KNOTS_NUM; j++) {
            out->dyn.hw_enhance_params.hw_enhance_loLumaConvert_val[j] =interpolation_f32(
                paut->dyn[ilow].hw_enhance_params.hw_enhance_loLumaConvert_val[j],
                paut->dyn[ihigh].hw_enhance_params.hw_enhance_loLumaConvert_val[j], ratio);
        }
        // enh_luma
        for (int j = 0; j < DHAZ_ENHANCE_CURVE_KNOTS_NUM; j++) {
            out->dyn.hw_enhance_params.sw_enhance_luma2strg_val[j] = interpolation_f32(
                paut->dyn[ilow].hw_enhance_params.sw_enhance_luma2strg_val[j],
                paut->dyn[ihigh].hw_enhance_params.sw_enhance_luma2strg_val[j], ratio);
        }
    }

    if (out->sta.hw_hist_setting.en) {
        // sw_hist_MapTflt_invSigma
        out->sta.hw_hist_setting.sw_hist_MapTflt_invSigma = paut->sta.hw_hist_setting.sw_hist_MapTflt_invSigma;
        // map_mode
        out->sta.hw_hist_setting.hw_hist_imgMap_mode = paut->sta.hw_hist_setting.hw_hist_imgMap_mode;
        // thumb_col
        out->sta.hw_hist_setting.hw_histc_blocks_cols = paut->sta.hw_hist_setting.hw_histc_blocks_cols;
        // thumb_row
        out->sta.hw_hist_setting.hw_histc_blocks_rows = paut->sta.hw_hist_setting.hw_histc_blocks_rows;
        // blk_het
        out->sta.hw_hist_setting.blk_het = pDehazeCtx->height / out->sta.hw_hist_setting.hw_histc_blocks_rows;
        // blk_wid
        out->sta.hw_hist_setting.blk_wid = pDehazeCtx->width / out->sta.hw_hist_setting.hw_histc_blocks_cols;
        // hist_th_off
        out->dyn.hw_hist_params.hw_histc_noiseCount_offset = interpolation_u8(
                paut->dyn[ilow].hw_hist_params.hw_histc_noiseCount_offset,
                paut->dyn[ihigh].hw_hist_params.hw_histc_noiseCount_offset, ratio);
        // hist_k
        out->dyn.hw_hist_params.sw_histc_noiseCount_scale = interpolation_f32(
                paut->dyn[ilow].hw_hist_params.sw_histc_noiseCount_scale,
                paut->dyn[ihigh].hw_hist_params.sw_histc_noiseCount_scale, ratio);
        // hist_min
        out->dyn.hw_hist_params.sw_histc_countWgt_minLimit = interpolation_f32(
                paut->dyn[ilow].hw_hist_params.sw_histc_countWgt_minLimit,
                paut->dyn[ihigh].hw_hist_params.sw_histc_countWgt_minLimit, ratio);
        // cfg_k
        out->dyn.hw_hist_params.sw_hist_mapUserSet = interpolation_f32(
                paut->dyn[ilow].hw_hist_params.sw_hist_mapUserSet,
                paut->dyn[ihigh].hw_hist_params.sw_hist_mapUserSet, ratio);
        // cfg_k_alpha
        out->dyn.hw_hist_params.sw_hist_mapMerge_alpha = interpolation_f32(
                paut->dyn[ilow].hw_hist_params.sw_hist_mapMerge_alpha,
                paut->dyn[ihigh].hw_hist_params.sw_hist_mapMerge_alpha, ratio);
        // k_gain
        out->dyn.hw_hist_params.sw_hist_mapCount_scale = interpolation_f32(
                paut->dyn[ilow].hw_hist_params.sw_hist_mapCount_scale,
                paut->dyn[ihigh].hw_hist_params.sw_hist_mapCount_scale, ratio);
        // blend_wet
        for (int j = 0; i < DHAZ_BLEND_WET_NUM; i++) {
            out->dyn.hw_hist_params.sw_dehaze_outputMerge_alpha[i] = interpolation_f32(
                paut->dyn[ilow].hw_hist_params.sw_dehaze_outputMerge_alpha[i],
                paut->dyn[ihigh].hw_hist_params.sw_dehaze_outputMerge_alpha[i], ratio);
        }
    }

    // get sigma_idx
    for (int i = 0; i < DHAZ_SIGMA_IDX_NUM; i++)
        out->sta.sigma_idx[i] = (i + 1) * YNR_CURVE_STEP;
    // get sigma_lut
    float sigam_total = 0.0f;
    for (int i = 0; i < DHAZ_SIGMA_IDX_NUM; i++) sigam_total += pDehazeCtx->YnrProcRes_sigma[i];
    if (sigam_total < FLT_EPSILON) {
        for (int i = 0; i < DHAZ_SIGMA_IDX_NUM; i++) out->sta.sigma_lut[i] = 0x200;
    } else {
        for (int i = 0; i < DHAZ_SIGMA_LUT_NUM; i++) {
            out->sta.sigma_lut[i] = pDehazeCtx->YnrProcRes_sigma[i];
        }
    }
    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn
algo_dehaze_SetAttrib
(
    RkAiqAlgoContext* ctx,
    dehaze_api_attrib_t *attr
) {
    if(ctx == NULL || attr == NULL) {
        LOGE_ADEHAZE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DehazeContext_t* pDehazeCtx = (DehazeContext_t*)ctx;
    dehaze_api_attrib_t* dehaze_attrib = pDehazeCtx->dehaze_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ADEHAZE("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    dehaze_attrib->opMode = attr->opMode;
    dehaze_attrib->en = attr->en;
    dehaze_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        dehaze_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        dehaze_attrib->stMan = attr->stMan;
    else {
        LOGW_ADEHAZE("wrong mode: %d\n", attr->opMode);
    }

    pDehazeCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_dehaze_GetAttrib
(
    RkAiqAlgoContext* ctx,
    dehaze_api_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ADEHAZE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    DehazeContext_t* pDehazeCtx = (DehazeContext_t*)ctx;
    dehaze_api_attrib_t* dehaze_attrib = pDehazeCtx->dehaze_attrib;

#if 0
    if (dehaze_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ADEHAZE("not auto mode: %d", dehaze_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = dehaze_attrib->opMode;
    attr->en = dehaze_attrib->en;
    attr->bypass = dehaze_attrib->bypass;
    memcpy(&attr->stAuto, &dehaze_attrib->stAuto, sizeof(dehaze_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_DEHAZE_VERSION     "v0.1.0"
#define RKISP_ALGO_DEHAZE_VENDOR      "Rockchip"
#define RKISP_ALGO_DEHAZE_DESCRIPTION "Rockchip dehaze algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescDehaze = {
    .common = {
        .version = RKISP_ALGO_DEHAZE_VERSION,
        .vendor  = RKISP_ALGO_DEHAZE_VENDOR,
        .description = RKISP_ALGO_DEHAZE_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADHAZ,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

//RKAIQ_END_DECLARE
