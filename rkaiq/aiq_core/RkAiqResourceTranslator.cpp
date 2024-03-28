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
//#define PDAF_RAW_DUMP

#include "isp20/Isp20Evts.h"
#include "isp20/Isp20StatsBuffer.h"
#include "isp20/rkispp-config.h"
#include "RkAiqResourceTranslator.h"
#include "PdafStreamProcUnit.h"
#include <fcntl.h>
#include <unistd.h>
#ifdef ANDROID_OS
#include <cutils/properties.h>
#endif

#ifdef __ARM_NEON
#define NEON_OPT
#endif
#ifdef NEON_OPT
#include <arm_neon.h>
#endif

#define DEFAULT_PD_RAW_PATH "/data/pdaf/frm%04u_pdAll.raw"
#define DEFAULT_PD_LRAW_PATH "/data/pdaf/frm%04u_pdLeft.raw"
#define DEFAULT_PD_RRAW_PATH "/data/pdaf/frm%04u_pdRight.raw"

#define MAX_8BITS ((1 << 8) - 1)

void calcAecLiteWinStats(
    isp2x_rawaelite_stat*       stats_in,
    rawaelite_stat_t*           stats_out,
    uint16_t*                   raw_mean,
    unsigned char*              weight,
    int8_t                      stats_chn_sel,
    int8_t                      y_range_mode
) {

    // NOTE: R/G/B/Y channel stats (10/12/10/8bits)
    uint32_t sum_xy = 0, sum_weight = 0;
    float rcc = 0, gcc = 0, bcc = 0, off = 0;

    if (y_range_mode <= CAM_YRANGEV2_MODE_FULL) {
        rcc = 0.299;
        gcc = 0.587;
        bcc = 0.114;
        off = 0;
    } else {
        rcc = 0.25;
        gcc = 0.5;
        bcc = 0.1094;
        off = 16;  //8bit
    }

    switch (stats_chn_sel) {
    case RAWSTATS_CHN_Y_EN:
        for (int i = 0; i < ISP2X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channely_xy[i] = CLIP(round(rcc * (float)(stats_in->data[i].channelr_xy >> 2) +
                                                   gcc * (float)(stats_in->data[i].channelg_xy >> 4) +
                                                   bcc * (float)(stats_in->data[i].channelb_xy >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_R_EN:
        for (int i = 0; i < ISP2X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = stats_in->data[i].channelr_xy;
            sum_xy += ((stats_out->channelr_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_G_EN:
        for (int i = 0; i < ISP2X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelg_xy[i] = stats_in->data[i].channelg_xy;
            sum_xy += ((stats_out->channelg_xy[i] >> 4) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_B_EN:
        for (int i = 0; i < ISP2X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelb_xy[i] = stats_in->data[i].channelb_xy;
            sum_xy += ((stats_out->channelb_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_RGB_EN:
        for (int i = 0; i < ISP2X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = stats_in->data[i].channelr_xy;
            stats_out->channelg_xy[i] = stats_in->data[i].channelg_xy;
            stats_out->channelb_xy[i] = stats_in->data[i].channelb_xy;
        }
        break;

    case RAWSTATS_CHN_ALL_EN:
    default:
        for (int i = 0; i < ISP2X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = stats_in->data[i].channelr_xy;
            stats_out->channelg_xy[i] = stats_in->data[i].channelg_xy;
            stats_out->channelb_xy[i] = stats_in->data[i].channelb_xy;
            stats_out->channely_xy[i] = CLIP(round(rcc * (float)(stats_out->channelr_xy[i] >> 2) +
                                                   gcc * (float)(stats_out->channelg_xy[i] >> 4) +
                                                   bcc * (float)(stats_out->channelb_xy[i] >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;
    }

}

void calcAecBigWinStats(
    isp2x_rawaebig_stat*        stats_in,
    rawaebig_stat_t*            stats_out,
    uint16_t*                   raw_mean,
    unsigned char*              weight,
    int8_t                      stats_chn_sel,
    int8_t                      y_range_mode
) {

    // NOTE: R/G/B/Y channel stats (10/12/10/8bits)
    uint32_t sum_xy = 0, sum_weight = 0;
    float rcc = 0, gcc = 0, bcc = 0, off = 0;

    if (y_range_mode <= CAM_YRANGEV2_MODE_FULL) {
        rcc = 0.299;
        gcc = 0.587;
        bcc = 0.114;
        off = 0;
    } else {
        rcc = 0.25;
        gcc = 0.5;
        bcc = 0.1094;
        off = 16;  //8bit
    }

    switch (stats_chn_sel) {
    case RAWSTATS_CHN_Y_EN:
        for (int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channely_xy[i] = CLIP(round(rcc * (float)(stats_in->data[i].channelr_xy >> 2) +
                                                   gcc * (float)(stats_in->data[i].channelg_xy >> 4) +
                                                   bcc * (float)(stats_in->data[i].channelb_xy >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_R_EN:
        for (int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = stats_in->data[i].channelr_xy;
            sum_xy += ((stats_out->channelr_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_G_EN:
        for (int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelg_xy[i] = stats_in->data[i].channelg_xy;
            sum_xy += ((stats_out->channelg_xy[i] >> 4) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_B_EN:
        for (int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelb_xy[i] = stats_in->data[i].channelb_xy;
            sum_xy += ((stats_out->channelb_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_RGB_EN:
        for (int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = stats_in->data[i].channelr_xy;
            stats_out->channelg_xy[i] = stats_in->data[i].channelg_xy;
            stats_out->channelb_xy[i] = stats_in->data[i].channelb_xy;
        }
        break;

    case RAWSTATS_CHN_ALL_EN:
    default:
        for (int i = 0; i < ISP2X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = stats_in->data[i].channelr_xy;
            stats_out->channelg_xy[i] = stats_in->data[i].channelg_xy;
            stats_out->channelb_xy[i] = stats_in->data[i].channelb_xy;
            stats_out->channely_xy[i] = CLIP(round(rcc * (float)(stats_out->channelr_xy[i] >> 2) +
                                                   gcc * (float)(stats_out->channelg_xy[i] >> 4) +
                                                   bcc * (float)(stats_out->channelb_xy[i] >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;
    }

    for (int i = 0; i < ISP2X_RAWAEBIG_SUBWIN_NUM; i++) {
        stats_out->wndx_sumr[i] = stats_in->sumr[i];
        stats_out->wndx_sumg[i] = stats_in->sumg[i];
        stats_out->wndx_sumb[i] = stats_in->sumb[i];
    }
}

namespace RkCam {

XCamReturn RkAiqResourceTranslator::translateIspStats(
    const SmartPtr<VideoBuffer>& from, SmartPtr<RkAiqIspStatsIntProxy>& to,
    const SmartPtr<RkAiqAecStatsProxy>& aecStat, const SmartPtr<RkAiqAwbStatsProxy>& awbStat,
    const SmartPtr<RkAiqAfStatsProxy>& afStat, const SmartPtr<RkAiqAtmoStatsProxy>& tmoStat,
    const SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat) {
    XCamReturn ret                       = XCAM_RETURN_NO_ERROR;
    Isp20StatsBuffer* buf = from.get_cast_ptr<Isp20StatsBuffer>();
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
#if defined(ISP_HW_V20) || defined(ISP_HW_V21)
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
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

    LOGD_ANALYZER("stats frame_id(%u), meas_type; 0x%x, buf sequence(%d)", stats->frame_id,
                  stats->meas_type, buf->get_sequence());

    SmartPtr<RkAiqIrisParamsProxy> irisParams = buf->get_iris_params();
    SmartPtr<RkAiqSensorExpParamsProxy> expParams = nullptr;
    rkisp_effect_params_v20 ispParams;
    memset(&ispParams, 0, sizeof(ispParams));
    if (buf->getEffectiveExpParams(stats->frame_id, expParams) < 0)
        LOGE("fail to get expParams");
    if (buf->getEffectiveIspParams(stats->frame_id, ispParams) < 0) {
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

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

#if defined(ISP_HW_V21)
    // AF need aec channelg_xy stats
    _aeAlgoStatsCfg.UpdateStats = true;
    _aeAlgoStatsCfg.RawStatsChnSel = RAWSTATS_CHN_ALL_EN;
#endif

    unsigned int meas_type = 0;
    u8 index0, index1, index2;

    switch (AeSwapMode) {
    case AEC_RAWSWAP_MODE_S_LITE:
        meas_type = ((stats->meas_type >> 7) & (0x01)) & ((stats->meas_type >> 11) & (0x01));
        index0 = 0;
        index1 = 1;
        index2 = 2;
        break;
    case AEC_RAWSWAP_MODE_M_LITE:
        meas_type = ((stats->meas_type >> 8) & (0x01)) & ((stats->meas_type >> 12) & (0x01));
        index0 = 1;
        index1 = 0;
        index2 = 2;
        break;
    case AEC_RAWSWAP_MODE_L_LITE:
        meas_type = ((stats->meas_type >> 9) & (0x01)) & ((stats->meas_type >> 13) & (0x01));
        index0 = 2;
        index1 = 1;
        index2 = 0;
        break;
    default:
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
        break;
    }

    //ae stats v2.x
    statsInt->frame_id = stats->frame_id;
    statsInt->aec_stats_valid = (meas_type & 0x01) ? true : false;
    if (!statsInt->aec_stats_valid)
        return XCAM_RETURN_BYPASS;

    // calc ae stats run flag
    uint64_t SumHistPix[3] = { 0, 0, 0 };
    uint64_t SumHistBin[3] = { 0, 0, 0 };
    uint16_t HistMean[3] = { 0, 0, 0 };
    u32* hist_bin[3];

    hist_bin[index0] = stats->params.rawhist0.hist_bin;
    hist_bin[index1] = stats->params.rawhist1.hist_bin;
    hist_bin[index2] = stats->params.rawhist2.hist_bin;
    if (AeSelMode <= AEC_RAWSEL_MODE_CHN_2) {
        hist_bin[AeSelMode] = stats->params.rawhist3.hist_bin;
    }

    for (int i = 0; i < ISP2X_HIST_BIN_N_MAX; i++) {
        SumHistPix[index0] += hist_bin[index0][i];
        SumHistBin[index0] += (hist_bin[index0][i] * (i + 1));

        SumHistPix[index1] += hist_bin[index1][i];
        SumHistBin[index1] += (hist_bin[index1][i] * (i + 1));

        SumHistPix[index2] += hist_bin[index2][i];
        SumHistBin[index2] += (hist_bin[index2][i] * (i + 1));
    }

    HistMean[0] = (uint16_t)(SumHistBin[0] / MAX(SumHistPix[0], 1));
    HistMean[1] = (uint16_t)(SumHistBin[1] / MAX(SumHistPix[1], 1));
    HistMean[2] = (uint16_t)(SumHistBin[2] / MAX(SumHistPix[2], 1));
    bool run_flag = getAeStatsRunFlag(HistMean);
    run_flag |= _aeAlgoStatsCfg.UpdateStats;

    if (run_flag) {
        calcAecLiteWinStats(&stats->params.rawae0,
                            &statsInt->aec_stats.ae_data.chn[index0].rawae_lite,
                            &statsInt->aec_stats.ae_data.raw_mean[index0],
                            _aeAlgoStatsCfg.LiteWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode);

        calcAecBigWinStats(&stats->params.rawae1,
                           &statsInt->aec_stats.ae_data.chn[index1].rawae_big,
                           &statsInt->aec_stats.ae_data.raw_mean[index1],
                           _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode);

        calcAecBigWinStats(&stats->params.rawae2,
                           &statsInt->aec_stats.ae_data.chn[index2].rawae_big,
                           &statsInt->aec_stats.ae_data.raw_mean[index2],
                           _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode);

        memcpy(statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins, stats->params.rawhist0.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[index1].rawhist_big.bins, stats->params.rawhist1.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
        memcpy(statsInt->aec_stats.ae_data.chn[index2].rawhist_big.bins, stats->params.rawhist2.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));

        switch (AeSelMode) {
        case AEC_RAWSEL_MODE_CHN_0:
        case AEC_RAWSEL_MODE_CHN_1:
        case AEC_RAWSEL_MODE_CHN_2:

            calcAecBigWinStats(&stats->params.rawae3,
                               &statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big,
                               &statsInt->aec_stats.ae_data.raw_mean[AeSelMode],
                               _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode);

            memcpy(statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP2X_HIST_BIN_N_MAX * sizeof(u32));
            break;

        case AEC_RAWSEL_MODE_TMO:

            calcAecBigWinStats(&stats->params.rawae3,
                               &statsInt->aec_stats.ae_data.extra.rawae_big,
                               &statsInt->aec_stats.ae_data.raw_mean[AeSelMode],
                               _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode);

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
        _lastAeStats =  statsInt->aec_stats.ae_data;

    } else {
        statsInt->aec_stats.ae_data = _lastAeStats;
    }

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
        // float start_time = (float)irisParams->data()->PIris.StartTim.tv_sec + (float)irisParams->data()->PIris.StartTim.tv_usec / 1000000.0f;
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
#endif
    return ret;
}

XCamReturn
RkAiqResourceTranslator::translateAwbStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAwbStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V20)
    Isp20StatsBuffer* buf =
        from.get_cast_ptr <Isp20StatsBuffer>();
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

    rkisp_effect_params_v20 ispParams;
    memset(&ispParams, 0, sizeof(ispParams));
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
#endif
    return ret;
}

XCamReturn
RkAiqResourceTranslator::translateAtmoStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAtmoStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Isp20StatsBuffer* buf =
        from.get_cast_ptr <Isp20StatsBuffer>();
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

#if RKAIQ_HAVE_DEHAZE_V10
XCamReturn
RkAiqResourceTranslator::translateAdehazeStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAdehazeStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Isp20StatsBuffer* buf =
        from.get_cast_ptr <Isp20StatsBuffer>();
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
    statsInt->adehaze_stats.dehaze_stats_v10.dhaz_adp_air_base =
        stats->params.dhaz.dhaz_adp_air_base;
    statsInt->adehaze_stats.dehaze_stats_v10.dhaz_adp_wt     = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v10.dhaz_adp_gratio = stats->params.dhaz.dhaz_adp_gratio;
    statsInt->adehaze_stats.dehaze_stats_v10.dhaz_adp_wt     = stats->params.dhaz.dhaz_adp_wt;
    for(int i = 0; i < 64; i++) {
        statsInt->adehaze_stats.dehaze_stats_v10.h_b_iir[i] = stats->params.dhaz.h_b_iir[i];
        statsInt->adehaze_stats.dehaze_stats_v10.h_g_iir[i] = stats->params.dhaz.h_g_iir[i];
        statsInt->adehaze_stats.dehaze_stats_v10.h_r_iir[i] = stats->params.dhaz.h_r_iir[i];
    }

    to->set_sequence(stats->frame_id);

    return ret;
}
#endif

XCamReturn
RkAiqResourceTranslator::translateAfStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAfStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
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
            statsInt->stat_motor.focusCode = afParams->data()->focusCode;
            statsInt->stat_motor.zoomCode = afParams->data()->zoomCode;
            statsInt->stat_motor.focus_endtim = afParams->data()->focusEndTim;
            statsInt->stat_motor.focus_starttim = afParams->data()->focusStartTim;
            statsInt->stat_motor.zoom_endtim = afParams->data()->zoomEndTim;
            statsInt->stat_motor.zoom_starttim = afParams->data()->zoomStartTim;
            statsInt->stat_motor.sof_tim = afParams->data()->sofTime;
            statsInt->stat_motor.focusCorrection = afParams->data()->focusCorrection;
            statsInt->stat_motor.zoomCorrection = afParams->data()->zoomCorrection;
            statsInt->stat_motor.angleZ = afParams->data()->angleZ;
            statsInt->af_stats.lowpass_id = afParams->data()->lowPassId;
            memcpy(statsInt->af_stats.lowpass_fv4_4,
                   afParams->data()->lowPassFv4_4, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
            memcpy(statsInt->af_stats.lowpass_fv8_8,
                   afParams->data()->lowPassFv8_8, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
            memcpy(statsInt->af_stats.lowpass_highlht,
                   afParams->data()->lowPassHighLht, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
            memcpy(statsInt->af_stats.lowpass_highlht2,
                   afParams->data()->lowPassHighLht2, ISP2X_RAWAF_SUMDATA_NUM * sizeof(u32));
        }

        if (_expParams.ptr())
            statsInt->aecExpInfo = _expParams->data()->aecExpInfo;
    }

    to->set_sequence(stats->frame_id);

    return ret;
}

#if RKAIQ_HAVE_PDAF
bool RkAiqResourceTranslator::getFileValue(const char* path, int* pos) {
    const char* delim = " ";
    char buffer[16]   = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp != -1) {
        if (read(fp, buffer, sizeof(buffer)) <= 0) {
            LOGE_AF("%s read %s failed!", __func__, path);
            goto OUT;
        } else {
            char* p = nullptr;

            p = strtok(buffer, delim);
            if (p != nullptr) {
                *pos = atoi(p);
            }
        }
        close(fp);
        return true;
    }

OUT:
    return false;
}

XCamReturn
RkAiqResourceTranslator::translatePdafStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqPdafStatsProxy> &to, bool sns_mirror)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    PdafBufferProxy* buf =
        from.get_cast_ptr<PdafBufferProxy>();
    void *pdafstats = (void *)(buf->get_v4l2_userptr());
    SmartPtr<RkAiqPdafStats> statsInt = to->data();

    if (pdafstats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    rk_aiq_isp_pdaf_meas_t* pdaf = &buf->pdaf_meas;
    uint16_t *pdLData, *pdRData, *pdData;
    uint32_t i, j, pixelperline;
    unsigned short pdWidth;
    unsigned short pdHeight;
    bool dumppdraw = false;
    uint32_t frame_id;

    pdLData = statsInt->pdaf_stats.pdLData;
    pdRData = statsInt->pdaf_stats.pdRData;
    pdData = (uint16_t *)pdafstats;

    //LOGD_AF("%s: frame_id %d, timestamp %lld, pdLData %p, pdRData %p, pdData %p",
    //    __func__, buf->get_sequence(), buf->get_timestamp(), pdLData, pdRData, pdData);

#ifdef PDAF_RAW_DUMP
    {
        FILE* fp;
        char name[64];
        frame_id = buf->get_sequence() % 10;

        ALOGD("@%s: pdWidthxpdHeight: %dx%d !\n", __FUNCTION__, pdaf->pdWidth, pdaf->pdHeight);
        memset(name, 0, sizeof(name));
        if (frame_id < 3) {
            sprintf(name, DEFAULT_PD_RAW_PATH, frame_id);
            fp = fopen(name, "wb");
            if (fp) {
                fwrite(pdData, pdaf->pdWidth * pdaf->pdHeight, 2, fp);
                fflush(fp);
                fclose(fp);
            } else {
                LOGE_AF("%s: can not write to %s file", __func__, name);
            }
        }
    }
#endif

    if (pdaf->pdafSensorType == PDAF_SENSOR_TYPE3) {
        if (pdaf->pdLRInDiffLine == 0) {
            pdWidth = pdaf->pdWidth >> 1;
            pdHeight = pdaf->pdHeight;

            pixelperline = 2 * pdWidth;
            for (j = 0; j < pdHeight; j++) {
                pdData = (uint16_t *)pdafstats + j * pixelperline;
                for (i = 0; i < pixelperline; i += 2) {
                    *pdLData++ = pdData[i] >> 6;
                    *pdRData++ = pdData[i + 1] >> 6;
                }
            }
        } else {
            pdWidth = pdaf->pdWidth;
            pdHeight = pdaf->pdHeight >> 1;
            pixelperline = pdaf->pdWidth;
            for (j = 0; j < 2 * pdHeight; j += 2) {
                for (i = 0; i < pixelperline; i++) {
                    *pdLData++ = pdData[i] >> 6;
                }
                pdData += pixelperline;
                for (i = 0; i < pixelperline; i++) {
                    *pdRData++ = pdData[i] >> 6;
                }
                pdData += pixelperline;
                pdLData += pixelperline;
                pdRData += pixelperline;
            }
        }
    } else {
        if (pdaf->pdLRInDiffLine == 0) {
            pdWidth = pdaf->pdWidth >> 1;
            pdHeight = pdaf->pdHeight;

#ifdef NEON_OPT
            uint16x8x2_t vld2_data;
            uint16x8_t vrev_data;
            pixelperline = 2 * pdWidth;
            for (j = 0; j < pdHeight; j++) {
                pdData = (uint16_t *)pdafstats + j * pixelperline;
                for (i = 0; i < pixelperline / 16 * 16; i += 16) {
                    vld2_data = vld2q_u16(pdData);
                    vst1q_u16(pdLData, vld2_data.val[0]);
                    vst1q_u16(pdRData, vld2_data.val[1]);
                    pdLData += 8;
                    pdRData += 8;
                    pdData += 16;
                }

                if (pixelperline % 16) {
                    for (i = 0; i < pixelperline % 16; i += 2) {
                        *pdLData++ = pdData[i];
                        *pdRData++ = pdData[i + 1];
                    }
                }
            }
#else
            pixelperline = 2 * pdWidth;
            for (j = 0; j < pdHeight; j++) {
                pdData = (uint16_t *)pdafstats + j * pixelperline;
                for (i = 0; i < pixelperline; i += 2) {
                    *pdLData++ = pdData[i];
                    *pdRData++ = pdData[i + 1];
                }
            }
#endif
        } else {
            pdWidth = pdaf->pdWidth;
            pdHeight = pdaf->pdHeight >> 1;
            pixelperline = pdaf->pdWidth;
            for (j = 0; j < 2 * pdHeight; j += 2) {
                memcpy(pdRData, pdData, pixelperline * sizeof(uint16_t));
                pdData += pixelperline;
                memcpy(pdLData, pdData, pixelperline * sizeof(uint16_t));
                pdData += pixelperline;
                pdLData += pixelperline;
                pdRData += pixelperline;
            }
        }
    }

#ifdef PDAF_RAW_DUMP
    frame_id = buf->get_sequence() % 10;

    if (frame_id < 3) {
        dumppdraw = true;
    }
#endif

    mEnPdDump = false;
#ifndef ANDROID_OS
    char* valueStr = getenv("rkaiq_pddump");
    if (valueStr) {
        mEnPdDump = atoi(valueStr) > 0 ? true : false;
    }
#else
    char property_value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("persist.vendor.rkaiq_pddump", property_value, "-1");
    int val = atoi(property_value);
    if (val != -1)
        mEnPdDump = atoi(property_value) > 0 ? true : false;
#endif

    if (mEnPdDump) {
        int dump_cnt = 0;
        if (getFileValue("/data/.dump_pd", &dump_cnt) == true) {
            if (dump_cnt > 0) {
                mPdafDumpCnt = dump_cnt;
                system("echo 0 > /data/.dump_pd");
            }
        } else {
            mPdafDumpCnt = 0;
        }

        if (mPdafDumpCnt > 0) {
            frame_id = buf->get_sequence();

            LOGI_AF("%s: dump pd raw, mPdafDumpCnt %d, frame_id %d", __func__, mPdafDumpCnt, frame_id);
            mPdafDumpCnt--;
            dumppdraw = true;
        }
    }

    if (dumppdraw) {
        FILE* fp;
        char name[64];

        memset(name, 0, sizeof(name));
        sprintf(name, DEFAULT_PD_LRAW_PATH, frame_id);
        fp = fopen(name, "wb");
        if (fp) {
            fwrite(statsInt->pdaf_stats.pdLData, pdWidth * pdHeight, 2, fp);
            fflush(fp);
            fclose(fp);
        } else {
            LOGE_AF("%s: can not write to %s file", __func__, name);
        }

        memset(name, 0, sizeof(name));
        sprintf(name, DEFAULT_PD_RRAW_PATH, frame_id);
        fp = fopen(name, "wb");
        if (fp) {
            fwrite(statsInt->pdaf_stats.pdRData, pdWidth * pdHeight, 2, fp);
            fflush(fp);
            fclose(fp);
        } else {
            LOGE_AF("%s: can not write to %s file", __func__, name);
        }
    }

    statsInt->pdaf_stats_valid = true;
    statsInt->frame_id = buf->get_sequence();
    statsInt->pdaf_stats.pdWidth = pdWidth;
    statsInt->pdaf_stats.pdHeight = pdHeight;
    statsInt->pdaf_stats.pdMirror = sns_mirror;
    statsInt->pdaf_stats.pdMean = 0;

    return ret;
}
#endif

XCamReturn
RkAiqResourceTranslator::translateOrbStats (const SmartPtr<VideoBuffer> &from,
        SmartPtr<RkAiqOrbStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    V4l2BufferProxy* nr_stats_buf =
        from.get_cast_ptr<V4l2BufferProxy>();
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

XCamReturn
RkAiqResourceTranslator::getParams(const SmartPtr<VideoBuffer>& from)
{
    Isp20StatsBuffer* buf = from.get_cast_ptr<Isp20StatsBuffer>();
#ifdef ISP_HW_V39
    auto stats = (struct rkisp39_stat_buffer*)(buf->get_v4l2_userptr());
#elif ISP_HW_V32_LITE
    auto stats = (struct rkisp32_lite_stat_buffer*)(buf->get_v4l2_userptr());
#elif ISP_HW_V32
    auto stats = (struct rkisp32_isp_stat_buffer*)(buf->get_v4l2_userptr());
#elif defined(ISP_HW_V30)
    auto stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
#elif defined(ISP_HW_V21)
    auto stats = (struct rkisp_isp21_stat_buffer*)(buf->get_v4l2_userptr());
#elif defined(ISP_HW_V20)
    auto stats = (struct rkisp_isp2x_stat_buffer*)(buf->get_v4l2_userptr());
#endif

    //TODO: check if needed
    //memset(&ispParams, 0, sizeof(_expParams));

    if (buf->getEffectiveExpParams(stats->frame_id, _expParams) < 0)
        LOGE("fail to get expParams");
#ifdef ISP_HW_V20
    if (buf->getEffectiveIspParams(stats->frame_id, _ispParams) < 0) {
#else
    if (buf->getEffectiveIspParams(stats->params_id, _ispParams) < 0) {
#endif
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    return XCAM_RETURN_NO_ERROR;
}

void
RkAiqResourceTranslator::releaseParams()
{
    _expParams.release();
}

bool RkAiqResourceTranslator::getAeStatsRunFlag(uint16_t* HistMean)
{
    bool run_flag = false;
    int FrameNum = 0;
    int cur_working_mode = getWorkingMode();

    if (cur_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        FrameNum = 1;
    else if(cur_working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 && cur_working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)
        FrameNum = 2;
    else if(cur_working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)
        FrameNum = 3;
    else
        LOGE("wrong working mode=%d\n", cur_working_mode);

    for(int i = 0; i < FrameNum; i++) {
        if(_lastHistMean[i] != HistMean[i]) {
            run_flag = true;
            _lastHistMean[i] = HistMean[i];
        }
    }

    return run_flag;
}

} //namespace RkCam
