/*
 * RkAiqConfigTranslator.cpp
 *
 *  Copyright (c) 2019 Rockchip Corporation
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

#include "isp20/Isp20Evts.h"
#include "isp20/Isp20StatsBuffer.h"
#include "isp20/rkisp2-config.h"
#include "isp21/rkisp21-config.h"
#include "RkAiqResourceTranslatorV21.h"

namespace RkCam {

XCamReturn
RkAiqResourceTranslatorV21::translateIspStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqIspStatsIntProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp21_stat_buffer *stats;
    SmartPtr<RkAiqIspStats> statsInt = to->data();

    SmartPtr<RkAiqExpParamsProxy> expParams = nullptr;
	rkisp_effect_params_v20 ispParams = {0};

    SmartPtr<RkAiqAfInfoProxy> afParams = buf->get_af_params();
    SmartPtr<RkAiqIrisParamsProxy> irisParams = buf->get_iris_params();

    stats = (struct rkisp_isp21_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x",
                  stats->frame_id, stats->meas_type);

    if (buf->getEffectiveExpParams(stats->frame_id, expParams) < 0)
        LOGE("fail to get expParams");
    if (buf->getEffectiveIspParams(stats->frame_id, ispParams) < 0) {
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    statsInt->frame_id = stats->frame_id;

    // TODO: do awb/ae/af/hdr stats convert

    //dehaze
    statsInt->adehaze_stats_valid = stats->meas_type >> 17 & 1;
    statsInt->adehaze_stats.dehaze_stats_v21.dhaz_adp_air_base = stats->params.dhaz.dhaz_adp_air_base;
    statsInt->adehaze_stats.dehaze_stats_v21.dhaz_adp_wt = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v21.dhaz_adp_gratio = stats->params.dhaz.dhaz_adp_gratio;
    statsInt->adehaze_stats.dehaze_stats_v21.dhaz_adp_wt = stats->params.dhaz.dhaz_adp_wt;
    for(int i = 0; i < 64; i++)
        statsInt->adehaze_stats.dehaze_stats_v21.h_rgb_iir[i] = stats->params.dhaz.h_rgb_iir[i];


    //ae stats v2.1

    statsInt->aec_stats_valid = (stats->meas_type >> 11) & (0x01) ? true : false;

    /*rawae stats*/
    uint8_t AeSwapMode, AeSelMode;
#if 0
    AeSwapMode = ispParams->aec_meas.rawae0.rawae_sel;
    AeSelMode = ispParams->aec_meas.rawae3.rawae_sel;
#else
    AeSwapMode = ispParams.isp_params_v21.meas.rawae0.rawae_sel;
    AeSelMode = ispParams.isp_params_v21.meas.rawae3.rawae_sel;
#endif

    switch(AeSwapMode) {
    case AEC_RAWSWAP_MODE_S_LITE:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    case AEC_RAWSWAP_MODE_M_LITE:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    default:
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
        break;
    }

    switch(AeSelMode) {
    case AEC_RAWSEL_MODE_CHN_0:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    case AEC_RAWSEL_MODE_CHN_1:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    case AEC_RAWSEL_MODE_TMO:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.extra.rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.extra.rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.extra.rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.extra.rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    default:
        LOGE("wrong AeSelMode=%d\n", AeSelMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    /*yuvae stats*/
    for(int i = 0; i < ISP2X_YUVAE_MEAN_NUM; i++) {
        statsInt->aec_stats.ae_data.yuvae.mean[i] = stats->params.yuvae.mean[i];
        if(i < ISP2X_YUVAE_SUBWIN_NUM)
            statsInt->aec_stats.ae_data.yuvae.ro_yuvae_sumy[i] = stats->params.yuvae.ro_yuvae_sumy[i];
    }

    /*
     *         unsigned long chn0_mean = 0, chn1_mean = 0;
     *         for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
     *             chn0_mean += stats->params.rawae1.data[i].channelg_xy;
     *             chn1_mean += stats->params.rawae3.data[i].channelg_xy;
     *         }
     *
     *
     *         printf("frame[%d]: chn[0-1]_g_mean_xy: %ld-%ld\n",
     *                 stats->frame_id, chn0_mean/ISP2X_RAWAEBIG_MEAN_NUM,
     *                 chn1_mean/ISP2X_RAWAEBIG_MEAN_NUM);
     */

    //expsoure params
    if (expParams.ptr()) {

        statsInt->aec_stats.ae_exp = expParams->data()->aecExpInfo;
        /*
         * printf("%s: L: [0x%x-0x%x], M: [0x%x-0x%x], S: [0x%x-0x%x]\n",
         *        __func__,
         *        expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
         *        expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
         *        expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global);
         */
    }

    //iris params
    if (irisParams.ptr()) {

        float sof_time = (float)irisParams->data()->sofTime / 1000000000.0f;
        float start_time = (float)irisParams->data()->PIris.StartTim.tv_sec + (float)irisParams->data()->PIris.StartTim.tv_usec / 1000000.0f;
        float end_time = (float)irisParams->data()->PIris.EndTim.tv_sec + (float)irisParams->data()->PIris.EndTim.tv_usec / 1000000.0f;
        float frm_intval = 1 / (statsInt->aec_stats.ae_exp.pixel_clock_freq_mhz * 1000000.0f /
                                (float)statsInt->aec_stats.ae_exp.line_length_pixels / (float)statsInt->aec_stats.ae_exp.frame_length_lines);

        /*printf("%s: step=%d,last-step=%d,start-tim=%f,end-tim=%f,sof_tim=%f\n",
            __func__,
            statsInt->aec_stats.ae_exp.Iris.PIris.step,
            irisParams->data()->PIris.laststep,start_time,end_time,sof_time);
        */

        if(sof_time < end_time + frm_intval)
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->data()->PIris.laststep;
        else
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->data()->PIris.step;
    }

    //af
    {
        statsInt->af_stats_valid =
            (stats->meas_type >> 6) & (0x01) ? true : false;
        statsInt->af_stats.roia_luminance =
            stats->params.rawaf.afm_lum[0];
        statsInt->af_stats.roib_sharpness =
            stats->params.rawaf.afm_sum[1];
        statsInt->af_stats.roib_luminance =
            stats->params.rawaf.afm_lum[1];
        memcpy(statsInt->af_stats.global_sharpness,
               stats->params.rawaf.ramdata, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));

        statsInt->af_stats.roia_sharpness = 0LL;
        for (int i = 0; i < ISP2X_RAWAF_SUMDATA_NUM; i++)
            statsInt->af_stats.roia_sharpness += stats->params.rawaf.ramdata[i];

        if(afParams.ptr()) {
            statsInt->af_stats.focus_endtim = afParams->data()->focusEndTim;
            statsInt->af_stats.focus_starttim = afParams->data()->focusStartTim;
            statsInt->af_stats.zoom_endtim = afParams->data()->zoomEndTim;
            statsInt->af_stats.zoom_starttim = afParams->data()->zoomStartTim;
            statsInt->af_stats.sof_tim = afParams->data()->sofTime;
        }
    }

    to->set_sequence(stats->frame_id);

    return ret;
}

XCamReturn
RkAiqResourceTranslatorV21::translateAecStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAecStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp21_stat_buffer *stats;
    SmartPtr<RkAiqAecStats> statsInt = to->data();

    stats = (struct rkisp_isp21_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x",
                  stats->frame_id, stats->meas_type);

    SmartPtr<RkAiqIrisParamsProxy> irisParams = buf->get_iris_params();
    SmartPtr<RkAiqExpParamsProxy> expParams = nullptr;
	rkisp_effect_params_v20 ispParams = {0};
    if (buf->getEffectiveExpParams(stats->frame_id, expParams) < 0)
        LOGE("fail to get expParams");
    if (buf->getEffectiveIspParams(stats->frame_id, ispParams) < 0) {
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    //ae stats v2.1

    statsInt->aec_stats_valid = (stats->meas_type >> 11) & (0x01) ? true : false;
    statsInt->frame_id = stats->frame_id;

    /*rawae stats*/
    uint8_t AeSwapMode, AeSelMode;
    AeSwapMode = ispParams.isp_params_v21.meas.rawae0.rawae_sel;
    AeSelMode = ispParams.isp_params_v21.meas.rawae3.rawae_sel;

    switch(AeSwapMode) {
    case AEC_RAWSWAP_MODE_S_LITE:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    case AEC_RAWSWAP_MODE_M_LITE:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    default:
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
        break;
    }

    switch(AeSelMode) {
    case AEC_RAWSEL_MODE_CHN_0:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    case AEC_RAWSEL_MODE_CHN_1:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    case AEC_RAWSEL_MODE_TMO:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.extra.rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.extra.rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.extra.rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.extra.rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    default:
        LOGE("wrong AeSelMode=%d\n", AeSelMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    /*yuvae stats*/
    for(int i = 0; i < ISP2X_YUVAE_MEAN_NUM; i++) {
        statsInt->aec_stats.ae_data.yuvae.mean[i] = stats->params.yuvae.mean[i];
        if(i < ISP2X_YUVAE_SUBWIN_NUM)
            statsInt->aec_stats.ae_data.yuvae.ro_yuvae_sumy[i] = stats->params.yuvae.ro_yuvae_sumy[i];
    }

    /*
     *         unsigned long chn0_mean = 0, chn1_mean = 0;
     *         for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
     *             chn0_mean += stats->params.rawae1.data[i].channelg_xy;
     *             chn1_mean += stats->params.rawae3.data[i].channelg_xy;
     *         }
     *
     *
     *         printf("frame[%d]: chn[0-1]_g_mean_xy: %ld-%ld\n",
     *                 stats->frame_id, chn0_mean/ISP2X_RAWAEBIG_MEAN_NUM,
     *                 chn1_mean/ISP2X_RAWAEBIG_MEAN_NUM);
     */

    //expsoure params
    if (expParams.ptr()) {

        statsInt->aec_stats.ae_exp = expParams->data()->aecExpInfo;
        /*
         * printf("%s: L: [0x%x-0x%x], M: [0x%x-0x%x], S: [0x%x-0x%x]\n",
         *        __func__,
         *        expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
         *        expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
         *        expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time,
         *        expParams->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global);
         */
    }

    //iris params
    if (irisParams.ptr()) {

        float sof_time = (float)irisParams->data()->sofTime / 1000000000.0f;
        float start_time = (float)irisParams->data()->PIris.StartTim.tv_sec + (float)irisParams->data()->PIris.StartTim.tv_usec / 1000000.0f;
        float end_time = (float)irisParams->data()->PIris.EndTim.tv_sec + (float)irisParams->data()->PIris.EndTim.tv_usec / 1000000.0f;
        float frm_intval = 1 / (statsInt->aec_stats.ae_exp.pixel_clock_freq_mhz * 1000000.0f /
                                (float)statsInt->aec_stats.ae_exp.line_length_pixels / (float)statsInt->aec_stats.ae_exp.frame_length_lines);

        /*printf("%s: step=%d,last-step=%d,start-tim=%f,end-tim=%f,sof_tim=%f\n",
            __func__,
            statsInt->aec_stats.ae_exp.Iris.PIris.step,
            irisParams->data()->PIris.laststep,start_time,end_time,sof_time);
        */

        if(sof_time < end_time + frm_intval)
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->data()->PIris.laststep;
        else
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->data()->PIris.step;
    }

    to->set_sequence(stats->frame_id);

    return ret;
}

XCamReturn
RkAiqResourceTranslatorV21::translateAwbStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAwbStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp21_stat_buffer *stats;
    SmartPtr<RkAiqAwbStats> statsInt = to->data();

    stats = (struct rkisp_isp21_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x",
                  stats->frame_id, stats->meas_type);

    statsInt->awb_stats_valid = stats->meas_type >> 5 & 1;
    if (!statsInt->awb_stats_valid) {
        LOGE_ANALYZER("AWB stats invalid, ignore");
        return XCAM_RETURN_BYPASS;
    }

	rkisp_effect_params_v20 ispParams = {0};
    if (buf->getEffectiveIspParams(stats->frame_id, ispParams) < 0) {
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    // TODO: do awb/ae/af/hdr stats convert

    //awb2.1

    statsInt->awb_stats_v201.awb_cfg_effect_v201 = ispParams.awb_cfg_v201;
    statsInt->awb_cfg_effect_valid = true;
    statsInt->frame_id = stats->frame_id;

    for(int i = 0; i < statsInt->awb_stats_v201.awb_cfg_effect_v201.lightNum; i++) {
        statsInt->awb_stats_v201.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].RgainValue =
            stats->params.rawawb.ro_rawawb_sum_rgain_nor[i];
        statsInt->awb_stats_v201.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].BgainValue =
            stats->params.rawawb.ro_rawawb_sum_bgain_nor[i];
        statsInt->awb_stats_v201.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_nor[i];
        statsInt->awb_stats_v201.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].RgainValue =
            stats->params.rawawb.ro_rawawb_sum_rgain_big[i];
        statsInt->awb_stats_v201.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].BgainValue =
            stats->params.rawawb.ro_rawawb_sum_bgain_big[i];
        statsInt->awb_stats_v201.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_big[i];

    }

    for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        statsInt->awb_stats_v201.blockResult[i].Rvalue = stats->params.rawawb.ramdata[i].r;
        statsInt->awb_stats_v201.blockResult[i].Gvalue = stats->params.rawawb.ramdata[i].g;
        statsInt->awb_stats_v201.blockResult[i].Bvalue = stats->params.rawawb.ramdata[i].b;
        statsInt->awb_stats_v201.blockResult[i].WpNo = stats->params.rawawb.ramdata[i].wp;
    }

    for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
        statsInt->awb_stats_v201.WpNoHist[i] = stats->params.rawawb.ro_yhist_bin[i];

    }
    //statsInt->awb_stats_valid = ISP2X_STAT_RAWAWB(stats->meas_type)? true:false;
    statsInt->awb_stats_valid = stats->meas_type >> 5 & 1;

    to->set_sequence(stats->frame_id);

    return ret;
}

XCamReturn
RkAiqResourceTranslatorV21::translateAfStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAfStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp21_stat_buffer *stats;
    SmartPtr<RkAiqAfStats> statsInt = to->data();

    stats = (struct rkisp_isp21_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x",
                  stats->frame_id, stats->meas_type);

    SmartPtr<RkAiqAfInfoProxy> afParams = buf->get_af_params();

    //af
    statsInt->af_stats_valid =
        (stats->meas_type >> 6) & (0x01) ? true : false;
    statsInt->af_stats.roia_luminance =
        stats->params.rawaf.afm_lum[0];
    statsInt->af_stats.roib_sharpness =
        stats->params.rawaf.afm_sum[1];
    statsInt->af_stats.roib_luminance =
        stats->params.rawaf.afm_lum[1];
    memcpy(statsInt->af_stats.global_sharpness,
            stats->params.rawaf.ramdata, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
    statsInt->frame_id = stats->frame_id;

    statsInt->af_stats.roia_sharpness = 0LL;
    for (int i = 0; i < ISP2X_RAWAF_SUMDATA_NUM; i++)
        statsInt->af_stats.roia_sharpness += stats->params.rawaf.ramdata[i];

    if(afParams.ptr()) {
        statsInt->af_stats.focus_endtim = afParams->data()->focusEndTim;
        statsInt->af_stats.focus_starttim = afParams->data()->focusStartTim;
        statsInt->af_stats.zoom_endtim = afParams->data()->zoomEndTim;
        statsInt->af_stats.zoom_starttim = afParams->data()->zoomStartTim;
        statsInt->af_stats.sof_tim = afParams->data()->sofTime;
    }

    return ret;
}

XCamReturn
RkAiqResourceTranslatorV21::translateAdehazeStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAdehazeStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp21_stat_buffer *stats;
    SmartPtr<RkAiqAdehazeStats> statsInt = to->data();

    stats = (struct rkisp_isp21_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x",
                  stats->frame_id, stats->meas_type);

    //dehaze
    statsInt->adehaze_stats_valid = stats->meas_type >> 17 & 1;
    statsInt->adehaze_stats.dehaze_stats_v21.dhaz_adp_air_base = stats->params.dhaz.dhaz_adp_air_base;
    statsInt->adehaze_stats.dehaze_stats_v21.dhaz_adp_wt = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v21.dhaz_adp_gratio = stats->params.dhaz.dhaz_adp_gratio;
    statsInt->adehaze_stats.dehaze_stats_v21.dhaz_adp_wt = stats->params.dhaz.dhaz_adp_wt;
    for(int i = 0; i < 64; i++)
        statsInt->adehaze_stats.dehaze_stats_v21.h_rgb_iir[i] = stats->params.dhaz.h_rgb_iir[i];

    to->set_sequence(stats->frame_id);

    return ret;
}

} //namespace RkCam
