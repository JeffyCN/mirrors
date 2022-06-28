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
#include "isp20/rkispp-config.h"
#include "RkAiqResourceTranslator.h"
#include "PdafStreamProcUnit.h"
#ifdef PDAF_RAW_DUMP
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif


namespace RkCam {

XCamReturn RkAiqResourceTranslator::translateIspStats(
    const SmartPtr<VideoBuffer>& from, SmartPtr<RkAiqIspStatsIntProxy>& to,
    const SmartPtr<RkAiqAecStatsProxy>& aecStat, const SmartPtr<RkAiqAwbStatsProxy>& awbStat,
    const SmartPtr<RkAiqAfStatsProxy>& afStat, const SmartPtr<RkAiqAtmoStatsProxy>& tmoStat,
    const SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat) {
    XCamReturn ret                       = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf = from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp2x_stat_buffer* stats;
    SmartPtr<RkAiqIspStats> statsInt = to->data();

    stats = (struct rkisp_isp2x_stat_buffer*)(buf->get_v4l2_userptr());
    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGD_ANALYZER("stats frame_id(%d), meas_type; 0x%x, buf sequence(%d)", stats->frame_id,
                  stats->meas_type, buf->get_sequence());

    statsInt->frame_id = stats->frame_id;

    statsInt->AecStatsProxy     = aecStat;
    statsInt->AwbStatsProxy     = awbStat;
    statsInt->AfStatsProxy      = afStat;
    statsInt->AtmoStatsProxy    = tmoStat;
    statsInt->AdehazeStatsProxy = dehazeStat;

    to->set_sequence(stats->frame_id);

    return ret;
}

XCamReturn
RkAiqResourceTranslator::translateAecStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAecStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
#if defined(ISP_HW_V21)
    struct rkisp_isp21_stat_buffer *stats;
#else
    struct rkisp_isp2x_stat_buffer *stats;
#endif
    SmartPtr<RkAiqAecStats> statsInt = to->data();

#if defined(ISP_HW_V21)
    stats = (struct rkisp_isp21_stat_buffer *)(buf->get_v4l2_userptr());
#else
    stats = (struct rkisp_isp2x_stat_buffer *)(buf->get_v4l2_userptr());
#endif
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOGD_ANALYZER("stats frame_id(%d), meas_type; 0x%x, buf sequence(%d)",
                  stats->frame_id, stats->meas_type, buf->get_sequence());

    SmartPtr<RkAiqIrisParamsProxy> irisParams = buf->get_iris_params();
    SmartPtr<RkAiqExpParamsProxy> expParams = nullptr;
    rkisp_effect_params_v20 ispParams = {0};
    if (buf->getEffectiveExpParams(stats->frame_id, expParams) < 0)
        LOGE("fail to get expParams");
    if (buf->getEffectiveIspParams(stats->frame_id, ispParams) < 0) {
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    statsInt->frame_id = stats->frame_id;
    //ae

    /*rawae stats*/
    uint8_t AeSwapMode, AeSelMode;
#if defined(ISP_HW_V21)
    AeSwapMode = ispParams.isp_params_v21.meas.rawae0.rawae_sel;
    AeSelMode = ispParams.isp_params_v21.meas.rawae3.rawae_sel;
#else
    AeSwapMode = ispParams.isp_params.meas.rawae0.rawae_sel;
    AeSelMode = ispParams.isp_params.meas.rawae3.rawae_sel;
#endif
    unsigned int meas_type = 0;

    switch(AeSwapMode) {
    case AEC_RAWSWAP_MODE_S_LITE:

        meas_type = ((stats->meas_type >> 7) & (0x01)) & ((stats->meas_type >> 11) & (0x01));
        statsInt->aec_stats_valid = (meas_type & 0x01) ? true : false;

        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;
            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelr_xy[i] = stats->params.rawae2.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelg_xy[i] = stats->params.rawae2.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelb_xy[i] = stats->params.rawae2.data[i].channelb_xy;
            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumr[i] = stats->params.rawae2.sumr[i];
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumg[i] = stats->params.rawae2.sumg[i];
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumb[i] = stats->params.rawae2.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[2].rawhist_big.bins, stats->params.rawhist2.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    case AEC_RAWSWAP_MODE_M_LITE:

        meas_type = ((stats->meas_type >> 8) & (0x01)) & ((stats->meas_type >> 12) & (0x01));
        statsInt->aec_stats_valid = (meas_type & 0x01) ? true : false;

        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;
            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelr_xy[i] = stats->params.rawae2.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelg_xy[i] = stats->params.rawae2.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelb_xy[i] = stats->params.rawae2.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumr[i] = stats->params.rawae2.sumr[i];
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumg[i] = stats->params.rawae2.sumg[i];
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumb[i] = stats->params.rawae2.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[2].rawhist_big.bins, stats->params.rawhist2.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    case AEC_RAWSWAP_MODE_L_LITE:

        meas_type = ((stats->meas_type >> 9) & (0x01)) & ((stats->meas_type >> 13) & (0x01));
        statsInt->aec_stats_valid = (meas_type & 0x01) ? true : false;

        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            if(i < ISP2X_RAWAELITE_MEAN_NUM) {
                statsInt->aec_stats.ae_data.chn[2].rawae_lite.channelr_xy[i] = stats->params.rawae0.data[i].channelr_xy;
                statsInt->aec_stats.ae_data.chn[2].rawae_lite.channelg_xy[i] = stats->params.rawae0.data[i].channelg_xy;
                statsInt->aec_stats.ae_data.chn[2].rawae_lite.channelb_xy[i] = stats->params.rawae0.data[i].channelb_xy;
            }
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] = stats->params.rawae2.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] = stats->params.rawae2.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] = stats->params.rawae2.data[i].channelb_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelr_xy[i] = stats->params.rawae1.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelg_xy[i] = stats->params.rawae1.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[1].rawae_big.channelb_xy[i] = stats->params.rawae1.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i] = stats->params.rawae2.sumr[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i] = stats->params.rawae2.sumg[i];
                statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i] = stats->params.rawae2.sumb[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumr[i] = stats->params.rawae1.sumr[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumg[i] = stats->params.rawae1.sumg[i];
                statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumb[i] = stats->params.rawae1.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins, stats->params.rawhist2.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[1].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[2].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
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

    case AEC_RAWSEL_MODE_CHN_2:
        for(int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelr_xy[i] = stats->params.rawae3.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelg_xy[i] = stats->params.rawae3.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.chn[2].rawae_big.channelb_xy[i] = stats->params.rawae3.data[i].channelb_xy;

            if(i < ISP2X_RAWAEBIG_SUBWIN_NUM) {
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumr[i] = stats->params.rawae3.sumr[i];
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumg[i] = stats->params.rawae3.sumg[i];
                statsInt->aec_stats.ae_data.chn[2].rawae_big.wndx_sumb[i] = stats->params.rawae3.sumb[i];
            }
        }
        memcpy(statsInt->aec_stats.ae_data.chn[2].rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
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

    //yuvae
    for(int i = 0; i < ISP2X_YUVAE_MEAN_NUM; i++) {
        statsInt->aec_stats.ae_data.yuvae.mean[i] = stats->params.yuvae.mean[i];
        if(i < ISP2X_YUVAE_SUBWIN_NUM)
            statsInt->aec_stats.ae_data.yuvae.ro_yuvae_sumy[i] = stats->params.yuvae.ro_yuvae_sumy[i];
    }
    memcpy(statsInt->aec_stats.ae_data.sihist.bins, stats->params.sihst.win_stat[0].hist_bins, ISP2X_SIHIST_WIN_NUM * sizeof(u32));

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
RkAiqResourceTranslator::translateAwbStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAwbStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp2x_stat_buffer *stats;
    SmartPtr<RkAiqAwbStats> statsInt = to->data();

    stats = (struct rkisp_isp2x_stat_buffer *)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGD_ANALYZER("stats frame_id(%d), meas_type; 0x%x, buf sequence(%d)",
                  stats->frame_id, stats->meas_type, buf->get_sequence());

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

    statsInt->frame_id = stats->frame_id;
    //awb2.0

    statsInt->awb_stats.awb_cfg_effect_v200 = ispParams.awb_cfg;
    statsInt->awb_cfg_effect_valid = true;

    for(int i = 0; i < statsInt->awb_stats.awb_cfg_effect_v200.lightNum; i++) {
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Rvalue =
            stats->params.rawawb.ro_rawawb_sum_r_nor[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Gvalue =
            stats->params.rawawb.ro_rawawb_sum_g_nor[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Bvalue =
            stats->params.rawawb.ro_rawawb_sum_b_nor[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_nor[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Rvalue =
            stats->params.rawawb.ro_rawawb_sum_r_big[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Gvalue =
            stats->params.rawawb.ro_rawawb_sum_g_big[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Bvalue =
            stats->params.rawawb.ro_rawawb_sum_b_big[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_big[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Rvalue =
            stats->params.rawawb.ro_rawawb_sum_r_sma[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Gvalue =
            stats->params.rawawb.ro_rawawb_sum_g_sma[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Bvalue =
            stats->params.rawawb.ro_rawawb_sum_b_sma[i];
        statsInt->awb_stats.light[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_sma[i];
    }
    for(int i = 0; i < statsInt->awb_stats.awb_cfg_effect_v200.lightNum; i++) {
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Rvalue =
            stats->params.rawawb.ro_sum_r_nor_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Gvalue =
            stats->params.rawawb.ro_sum_g_nor_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].Bvalue =
            stats->params.rawawb.ro_sum_b_nor_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V200].WpNo =
            stats->params.rawawb.ro_wp_nm_nor_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Rvalue =
            stats->params.rawawb.ro_sum_r_big_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Gvalue =
            stats->params.rawawb.ro_sum_g_big_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].Bvalue =
            stats->params.rawawb.ro_sum_b_big_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V200].WpNo =
            stats->params.rawawb.ro_wp_nm_big_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Rvalue =
            stats->params.rawawb.ro_sum_r_sma_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Gvalue =
            stats->params.rawawb.ro_sum_g_sma_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].Bvalue =
            stats->params.rawawb.ro_sum_b_sma_multiwindow[i];
        statsInt->awb_stats.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_SMALL_V200].WpNo =
            stats->params.rawawb.ro_wp_nm_sma_multiwindow[i];
    }
    for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V200; i++) {
        statsInt->awb_stats.excWpRangeResult[i].Rvalue = stats->params.rawawb.ro_sum_r_exc[i];
        statsInt->awb_stats.excWpRangeResult[i].Gvalue = stats->params.rawawb.ro_sum_g_exc[i];
        statsInt->awb_stats.excWpRangeResult[i].Bvalue = stats->params.rawawb.ro_sum_b_exc[i];
        statsInt->awb_stats.excWpRangeResult[i].WpNo =    stats->params.rawawb.ro_wp_nm_exc[i];

    }
    for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        statsInt->awb_stats.blockResult[i].Rvalue = stats->params.rawawb.ramdata[i].r;
        statsInt->awb_stats.blockResult[i].Gvalue = stats->params.rawawb.ramdata[i].g;
        statsInt->awb_stats.blockResult[i].Bvalue = stats->params.rawawb.ramdata[i].b;
        statsInt->awb_stats.blockResult[i].isWP[2] = stats->params.rawawb.ramdata[i].wp & 0x1;
        statsInt->awb_stats.blockResult[i].isWP[1] = (stats->params.rawawb.ramdata[i].wp >> 1) & 0x1;
        statsInt->awb_stats.blockResult[i].isWP[0] = (stats->params.rawawb.ramdata[i].wp >> 2) & 0x1;
    }
    //statsInt->awb_stats_valid = ISP2X_STAT_RAWAWB(stats->meas_type)? true:false;
    statsInt->awb_stats_valid = stats->meas_type >> 5 & 1;

    to->set_sequence(stats->frame_id);

    return ret;
}

XCamReturn
RkAiqResourceTranslator::translateAtmoStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAtmoStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp2x_stat_buffer *stats;
    SmartPtr<RkAiqAtmoStats> statsInt = to->data();

    stats = (struct rkisp_isp2x_stat_buffer *)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOGD_ANALYZER("stats frame_id(%d), meas_type; 0x%x, buf sequence(%d)",
                  stats->frame_id, stats->meas_type, buf->get_sequence());

    statsInt->frame_id = stats->frame_id;

    //ahdr
    statsInt->atmo_stats_valid = stats->meas_type >> 16 & 1;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_lglow = stats->params.hdrtmo.lglow;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_lgmin = stats->params.hdrtmo.lgmin;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_lghigh = stats->params.hdrtmo.lghigh;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_lgmax = stats->params.hdrtmo.lgmax;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_weightkey = stats->params.hdrtmo.weightkey;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_lgmean = stats->params.hdrtmo.lgmean;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_lgrange0 = stats->params.hdrtmo.lgrange0;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_lgrange1 = stats->params.hdrtmo.lgrange1;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_palpha = stats->params.hdrtmo.palpha;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_lgavgmax = stats->params.hdrtmo.lgavgmax;
    statsInt->atmo_stats.tmo_stats.ro_hdrtmo_linecnt = stats->params.hdrtmo.linecnt;
    for(int i = 0; i < 32; i++)
        statsInt->atmo_stats.tmo_stats.ro_array_min_max[i] = stats->params.hdrtmo.min_max[i];

    to->set_sequence(stats->frame_id);

    return ret;
}

XCamReturn
RkAiqResourceTranslator::translateAdehazeStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAdehazeStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp_isp2x_stat_buffer *stats;
    SmartPtr<RkAiqAdehazeStats> statsInt = to->data();

    stats = (struct rkisp_isp2x_stat_buffer *)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOGD_ANALYZER("stats frame_id(%d), meas_type; 0x%x, buf sequence(%d)",
                  stats->frame_id, stats->meas_type, buf->get_sequence());

    statsInt->frame_id = stats->frame_id;

    //dehaze
    statsInt->adehaze_stats_valid = stats->meas_type >> 17 & 1;
    statsInt->adehaze_stats.dehaze_stats_v20.dhaz_adp_air_base = stats->params.dhaz.dhaz_adp_air_base;
    statsInt->adehaze_stats.dehaze_stats_v20.dhaz_adp_wt = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v20.dhaz_adp_gratio = stats->params.dhaz.dhaz_adp_gratio;
    statsInt->adehaze_stats.dehaze_stats_v20.dhaz_adp_wt = stats->params.dhaz.dhaz_adp_wt;
    for(int i = 0; i < 64; i++) {
        statsInt->adehaze_stats.dehaze_stats_v20.h_b_iir[i] = stats->params.dhaz.h_b_iir[i];
        statsInt->adehaze_stats.dehaze_stats_v20.h_g_iir[i] = stats->params.dhaz.h_g_iir[i];
        statsInt->adehaze_stats.dehaze_stats_v20.h_r_iir[i] = stats->params.dhaz.h_r_iir[i];
    }

    to->set_sequence(stats->frame_id);

    return ret;
}

XCamReturn
RkAiqResourceTranslator::translateAfStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAfStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
#if defined(ISP_HW_V21)
    struct rkisp_isp21_stat_buffer *stats;
#else
    struct rkisp_isp2x_stat_buffer *stats;
#endif
    SmartPtr<RkAiqAfStats> statsInt = to->data();

#if defined(ISP_HW_V21)
    stats = (struct rkisp_isp21_stat_buffer *)(buf->get_v4l2_userptr());
#else
    stats = (struct rkisp_isp2x_stat_buffer *)(buf->get_v4l2_userptr());
#endif
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOGD_ANALYZER("stats frame_id(%d), meas_type; 0x%x, buf sequence(%d)",
                  stats->frame_id, stats->meas_type, buf->get_sequence());

    SmartPtr<RkAiqAfInfoProxy> afParams = buf->get_af_params();

    statsInt->frame_id = stats->frame_id;

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
            statsInt->af_stats.focusCode = afParams->data()->focusCode;
            statsInt->af_stats.zoomCode = afParams->data()->zoomCode;
            statsInt->af_stats.focus_endtim = afParams->data()->focusEndTim;
            statsInt->af_stats.focus_starttim = afParams->data()->focusStartTim;
            statsInt->af_stats.zoom_endtim = afParams->data()->zoomEndTim;
            statsInt->af_stats.zoom_starttim = afParams->data()->zoomStartTim;
            statsInt->af_stats.sof_tim = afParams->data()->sofTime;
            statsInt->af_stats.lowpass_id = afParams->data()->lowPassId;
            statsInt->af_stats.focusCorrection = afParams->data()->focusCorrection;
            statsInt->af_stats.zoomCorrection = afParams->data()->zoomCorrection;
            memcpy(statsInt->af_stats.lowpass_fv4_4,
                   afParams->data()->lowPassFv4_4, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
            memcpy(statsInt->af_stats.lowpass_fv8_8,
                   afParams->data()->lowPassFv8_8, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
            memcpy(statsInt->af_stats.lowpass_highlht,
                   afParams->data()->lowPassHighLht, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
            memcpy(statsInt->af_stats.lowpass_highlht2,
                   afParams->data()->lowPassHighLht2, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));

            statsInt->af_stats.angleZ = afParams->data()->angleZ;
        }
    }

    to->set_sequence(stats->frame_id);

    return ret;
}

XCamReturn
RkAiqResourceTranslator::translatePdafStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqPdafStatsProxy> &to, bool sns_mirror)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<PdafBufferProxy> buf =
        from.dynamic_cast_ptr<PdafBufferProxy>();
    void *pdafstats = (void *)(buf->get_v4l2_userptr());
    SmartPtr<RkAiqPdafStats> statsInt = to->data();

    if (pdafstats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    rk_aiq_isp_pdaf_meas_t* pdaf = &buf->pdaf_meas;
    uint16_t *pdLData, *pdRData, *pdData;
    bool pdMirrorInCalib = pdaf->pdMirrorInCalib;
    uint32_t i, j, pixelperline;
    uint64_t pdMean;

    pdLData = statsInt->pdaf_stats.pdLData;
    pdRData = statsInt->pdaf_stats.pdRData;
    pdData = (uint16_t *)pdafstats;

//#define PDAF_RAW_DUMP
#ifdef PDAF_RAW_DUMP
    {
        FILE* fp;
        char name[64];
        int frame_id = buf->get_sequence() % 10;

        memset(name, 0, sizeof(name));
        sprintf(name, "/tmp/pdaf_raw_%d.raw", frame_id);
        fp = fopen(name, "wb");
        fwrite(pdData, 1016 * 760, 2, fp);
        fflush(fp);
        fclose(fp);
    }
#endif

    pdMean = 0;
    pixelperline = 2 * pdaf->pdWidth;
    if (pdMirrorInCalib) {
        for (j = 0; j < pdaf->pdHeight; j++) {
            for (i = 0; i < pixelperline; i+=2) {
                pdMean += pdData[j * pixelperline + (pixelperline - 1 - i)];
                *pdRData++ = pdData[j * pixelperline + (pixelperline - 1 - i)];
                pdMean += pdData[j * pixelperline + (pixelperline - 1 - (i+1))];
                *pdLData++ = pdData[j * pixelperline + (pixelperline - 1 - (i+1))];
            }
        }
    } else {
        for (j = 0; j < pdaf->pdHeight; j++) {
            for (i = 0; i < pixelperline; i+=2) {
                pdMean += pdData[j * pixelperline + i];
                *pdLData++ = pdData[j * pixelperline + i];
                pdMean += pdData[j * pixelperline + (i+1)];
                *pdRData++ = pdData[j * pixelperline + (i+1)];
            }
        }
    }
    pdMean /= pixelperline * pdaf->pdHeight;

#ifdef PDAF_RAW_DUMP
    {
        FILE* fp;

        fp = fopen("/tmp/pdaf_L.raw", "wb");
        fwrite(statsInt->pdaf_stats.pdLData, pdaf->pdWidth * pdaf->pdHeight, 2, fp);
        fflush(fp);
        fclose(fp);

        fp = fopen("/tmp/pdaf_R.raw", "wb");
        fwrite(statsInt->pdaf_stats.pdRData, pdaf->pdWidth * pdaf->pdHeight, 2, fp);
        fflush(fp);
        fclose(fp);
    }
#endif

    statsInt->pdaf_stats_valid = true;
    statsInt->frame_id = buf->get_sequence();
    statsInt->pdaf_stats.pdWidth = pdaf->pdWidth;
    statsInt->pdaf_stats.pdHeight = pdaf->pdHeight;
    statsInt->pdaf_stats.pdMirror = sns_mirror;
    statsInt->pdaf_stats.pdMean = (uint32_t)pdMean;

    return ret;
}

XCamReturn
RkAiqResourceTranslator::translateOrbStats (const SmartPtr<VideoBuffer> &from,
        SmartPtr<RkAiqOrbStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    const SmartPtr<V4l2BufferProxy> nr_stats_buf =
        from.dynamic_cast_ptr<V4l2BufferProxy>();
    SmartPtr<RkAiqOrbStats> statsInt = to->data();

    struct rkispp_stats_nrbuf *stats = NULL;
    stats = (struct rkispp_stats_nrbuf *)(nr_stats_buf->get_v4l2_userptr());

    // orbStats->data()->valid = true;//(stats->meas_type >> 4) & (0x01) ? true : false;
    statsInt->orb_stats.frame_id = stats->frame_id;
    statsInt->orb_stats.num_points = stats->total_num;
    if (stats->total_num > 0 && stats->total_num <= ORB_DATA_NUM)
        memcpy(statsInt->orb_stats.points, stats->data, stats->total_num * sizeof(stats->data[0]));

    to->set_sequence(stats->frame_id);

    return ret;
}

} //namespace RkCam
