/*
 * Copyright (c) 2022 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqResourceTranslatorV32.h"

#include "common/rkisp32-config.h"
#include "isp20/Isp20Evts.h"
#include "isp20/Isp20StatsBuffer.h"

//#define AE_STATS_DEBUG
#define MAX_10BITS (1 << 10) - 1
#define MAX_12BITS (1 << 12) - 1
#define MAX_29BITS (1 << 29) - 1
#define MAX_32BITS  0xffffffff

namespace RkCam {

RkAiqResourceTranslatorV32::RkAiqResourceTranslatorV32() {}

XCamReturn RkAiqResourceTranslatorV32::translateAecStats(const SmartPtr<VideoBuffer>& from,
        SmartPtr<RkAiqAecStatsProxy>& to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
  
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    // 0) blc awb cfg

    struct isp32_isp_meas_cfg* isp_params = &_ispParams.meas;
    uint8_t AeSwapMode, AeSelMode, AfUseAeHW;
    AeSwapMode             = isp_params->rawae0.rawae_sel;
    AeSelMode              = isp_params->rawae3.rawae_sel;
    AfUseAeHW              = isp_params->rawaf.ae_mode;
    unsigned int meas_type = 0;

    // ae_stats = (ae_ori_stats_u12/10 - ob_offset_u9 - bls1_val_u12) * awb1_gain_u16 * range_ratio
    struct isp32_bls_cfg* bls_cfg = &_ispParams.bls_cfg;
    struct isp32_awb_gain_cfg* awb_gain_cfg = &_ispParams.awb_gain_cfg;
    struct isp2x_bls_fixed_val bls1_val;  // bls1_val = blc1_ori_val * awb * range_ratio

    u16 isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain;
    u16 awb1_gain_r, awb1_gain_gr, awb1_gain_gb, awb1_gain_b;
    u32 pixel_num      = 0;
    u8  rawhist_mode   = 0;
    u8  index0, index1, index2 = 0;
    bool is_hdr = (getWorkingMode() > 0) ? true : false;

    isp_ob_offset_rb = MAX(bls_cfg->isp_ob_offset >> 2, 0);
    isp_ob_offset_g  = bls_cfg->isp_ob_offset;
    isp_ob_predgain =  MAX(bls_cfg->isp_ob_predgain >> 8, 1);

    if (bls_cfg->bls1_en && !is_hdr) {
        bls1_val.r  = (bls_cfg->bls1_val.r / isp_ob_predgain) >> 2;
        bls1_val.gr = bls_cfg->bls1_val.gr / isp_ob_predgain;
        bls1_val.gb = bls_cfg->bls1_val.gb / isp_ob_predgain;
        bls1_val.b  = (bls_cfg->bls1_val.b / isp_ob_predgain) >> 2;
    } else {
        bls1_val.r  = 0;
        bls1_val.gr = 0;
        bls1_val.gb = 0;
        bls1_val.b  = 0;
    }
    //awb1_gain have adapted to the range of bls1_lvl
    awb1_gain_r  = MAX(256, awb_gain_cfg->awb1_gain_r);
    awb1_gain_gr = MAX(256, awb_gain_cfg->awb1_gain_gr);
    awb1_gain_gb = MAX(256, awb_gain_cfg->awb1_gain_gb);
    awb1_gain_b  = MAX(256, awb_gain_cfg->awb1_gain_b);

#ifdef AE_STATS_DEBUG
    LOGE("bls1[%d-%d-%d-%d]", bls1_val.r, bls1_val.gr, bls1_val.gb, bls1_val.b);
    LOGE("isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain [%d-%d-%d]",
         isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain);
    LOGE("awb1_gain[%d-%d-%d-%d]", awb1_gain_r, awb1_gain_gr, awb1_gain_gb, awb1_gain_b);
#endif

    u16 bls_r, bls_g, bls_b;
    bls_r = ((isp_ob_offset_rb + bls1_val.r) * awb1_gain_r + 128) / 256;
    bls_g = ((isp_ob_offset_g + bls1_val.gr) * awb1_gain_gr + 128) / 256;
    bls_b = ((isp_ob_offset_rb + bls1_val.b) * awb1_gain_b + 128) / 256;

#endif  

#if defined(ISP_HW_V32)

    const SmartPtr<Isp20StatsBuffer> buf = from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp32_isp_stat_buffer* stats;
    SmartPtr<RkAiqAecStats> statsInt = to->data();

    stats = (struct rkisp32_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", mCamPhyId, stats->frame_id,
                  stats->meas_type);

    SmartPtr<RkAiqIrisParamsProxy> irisParams = buf->get_iris_params();

    // ae stats v3.2
    statsInt->frame_id = stats->frame_id;

    switch (AeSwapMode) {
    case AEC_RAWSWAP_MODE_S_LITE:
        meas_type = ((stats->meas_type >> 7) & (0x01)) & ((stats->meas_type >> 11) & (0x01));
        index0       = 0;
        index1       = 1;
        index2       = 2;
        rawhist_mode = isp_params->rawhist0.mode;
        break;
    case AEC_RAWSWAP_MODE_M_LITE:
        meas_type = ((stats->meas_type >> 8) & (0x01)) & ((stats->meas_type >> 12) & (0x01));
        index0       = 1;
        index1       = 0;
        index2       = 2;
        rawhist_mode = isp_params->rawhist1.mode;
        break;
    default:
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
        break;
    }

    statsInt->aec_stats_valid = (meas_type & 0x01) ? true : false;
    if (!statsInt->aec_stats_valid) return XCAM_RETURN_BYPASS;

    //AE-LITE (RAWAE0)
    if(is_hdr || AeSwapMode == AEC_RAWSWAP_MODE_S_LITE) {
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.chn[index0].rawae_lite.channelr_xy[i] = CLIP(
                        (int)(stats->params.rawae0.data[i].channelr_xy * awb1_gain_r / 256 - bls_r), 0, MAX_10BITS);
            statsInt->aec_stats.ae_data.chn[index0].rawae_lite.channelg_xy[i] = CLIP(
                        (int)(stats->params.rawae0.data[i].channelg_xy * awb1_gain_gr / 256 - bls_g), 0, MAX_12BITS);
            statsInt->aec_stats.ae_data.chn[index0].rawae_lite.channelb_xy[i] = CLIP(
                        (int)(stats->params.rawae0.data[i].channelb_xy * awb1_gain_b / 256 - bls_b), 0, MAX_10BITS);
        }
    }

    //AE-BIG (RAWAE1)
    if(is_hdr || AeSwapMode == AEC_RAWSWAP_MODE_M_LITE) {

        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {

            statsInt->aec_stats.ae_data.chn[index1].rawae_big.channelr_xy[i] = CLIP(
                        (int)(stats->params.rawae1_0.data[i].channelr_xy  * awb1_gain_r / 256 - bls_r), 0, MAX_10BITS);
            statsInt->aec_stats.ae_data.chn[index1].rawae_big.channelg_xy[i] = CLIP(
                        (int)(stats->params.rawae1_0.data[i].channelg_xy * awb1_gain_gr / 256 - bls_g), 0, MAX_12BITS);
            statsInt->aec_stats.ae_data.chn[index1].rawae_big.channelb_xy[i] = CLIP(
                        (int)(stats->params.rawae1_0.data[i].channelb_xy * awb1_gain_b / 256 - bls_b), 0, MAX_10BITS);
        }

        for (int i = 0; i < ISP3X_RAWAEBIG_SUBWIN_NUM; i++) {
            pixel_num = isp_params->rawae1.subwin[i].h_size * isp_params->rawae1.subwin[i].v_size;
            statsInt->aec_stats.ae_data.chn[index1].rawae_big.wndx_sumr[i] = CLIP(
                        (s64)((u64)stats->params.rawae1_1.sumr[i] * awb1_gain_r / 256 - (pixel_num >> 2) * bls_r), 0, MAX_29BITS);
            statsInt->aec_stats.ae_data.chn[index1].rawae_big.wndx_sumg[i] = CLIP(
                        (s64)((u64)stats->params.rawae1_1.sumg[i] * awb1_gain_gr / 256 - (pixel_num >> 1) * bls_g), 0, MAX_32BITS);
            statsInt->aec_stats.ae_data.chn[index1].rawae_big.wndx_sumb[i] = CLIP(
                        (s64)((u64)stats->params.rawae1_1.sumb[i] * awb1_gain_b / 256 - (pixel_num >> 2) * bls_b), 0, MAX_29BITS);
        }
    }

    //HIST 0/1

    if (bls_cfg->bls1_en && !is_hdr) {

        memset(statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins, 0,
               ISP3X_HIST_BIN_N_MAX * sizeof(u32));
        memset(statsInt->aec_stats.ae_data.chn[index1].rawhist_big.bins, 0,
               ISP3X_HIST_BIN_N_MAX * sizeof(u32));

        s16 bls1, bls;
        u16 ob, awb1_gain, div_part, round_part;
        int tmp;

        switch (rawhist_mode) {
        case 2:
            ob = isp_ob_offset_rb >> 2;
            bls1 = bls1_val.r >> 2;
            awb1_gain = awb1_gain_r;
            bls = (ob + bls1) * awb1_gain;
            div_part = 256;
            round_part = 128;
            break;
        case 3:
            ob = isp_ob_offset_g >> 4;
            bls1 = bls1_val.gr >> 4;
            awb1_gain = awb1_gain_gr;
            bls = (ob + bls1) * awb1_gain;
            div_part = 256;
            round_part = 128;
            break;
        case 4:
            ob = isp_ob_offset_rb >> 2;
            bls1 = bls1_val.b >> 2;
            awb1_gain = awb1_gain_b;
            bls = (ob + bls1) * awb1_gain;
            div_part = 256;
            round_part = 128;
            break;
        case 5:
        default:
            ob = (u16)((isp_ob_offset_g >> 4) * 587 + (isp_ob_offset_rb >> 2) * 299 + (isp_ob_offset_rb >> 2) * 114 + 500) / 1000;
            bls1 = (s16)((bls1_val.gr >> 4) * 587 + (bls1_val.r >> 2) * 299 + (bls1_val.b >> 2) * 114 + 500) / 1000;
            awb1_gain = 100;
            bls = (ob + bls1) * awb1_gain;
            div_part = 7655 / awb1_gain_r + 15027 / awb1_gain_gr + 2919 / awb1_gain_b;
            round_part = div_part / 2;
            break;
        }


        for (int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {

            tmp = (i - ob - bls1 > 0) ? (i * awb1_gain - bls + round_part) / div_part : 0;
            tmp = (tmp > ISP3X_HIST_BIN_N_MAX - 1) ? (ISP3X_HIST_BIN_N_MAX - 1) : tmp;

            statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins[tmp] +=
                stats->params.rawhist0.hist_bin[i];
            statsInt->aec_stats.ae_data.chn[index1].rawhist_big.bins[tmp] +=
                stats->params.rawhist1.hist_bin[i];

        }
    } else {

        if(is_hdr || AeSwapMode == AEC_RAWSWAP_MODE_S_LITE) {
            memcpy(statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins,
                   stats->params.rawhist0.hist_bin, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
        }

        if(is_hdr || AeSwapMode == AEC_RAWSWAP_MODE_M_LITE) {
            memcpy(statsInt->aec_stats.ae_data.chn[index1].rawhist_big.bins,
                   stats->params.rawhist1.hist_bin, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
        }
    }


    //AE-BIG (RAWAE3)

    statsInt->af_prior = (AfUseAeHW == 0) ? false : true;

    if (!AfUseAeHW) {
        switch (AeSelMode) {
        case AEC_RAWSEL_MODE_CHN_0:
        case AEC_RAWSEL_MODE_CHN_1:

            for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
                statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.channelr_xy[i] = CLIP(
                            (int)(stats->params.rawae3_0.data[i].channelr_xy * awb1_gain_r / 256 - bls_r), 0, MAX_10BITS);
                statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.channelg_xy[i] = CLIP(
                            (int)(stats->params.rawae3_0.data[i].channelg_xy * awb1_gain_gr / 256 - bls_g), 0, MAX_12BITS);
                statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.channelb_xy[i] = CLIP(
                            (int)(stats->params.rawae3_0.data[i].channelb_xy * awb1_gain_b / 256 - bls_b), 0, MAX_10BITS);
            }
            for (int i = 0; i < ISP3X_RAWAEBIG_SUBWIN_NUM; i++) {

                pixel_num = isp_params->rawae3.subwin[i].h_size * isp_params->rawae3.subwin[i].v_size;
                statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumr[i] = CLIP(
                            (s64)((u64)stats->params.rawae3_1.sumr[i] * awb1_gain_r / 256 - (pixel_num >> 2) * bls_r), 0, MAX_29BITS);
                statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumg[i] = CLIP(
                            (s64)((u64)stats->params.rawae3_1.sumg[i] * awb1_gain_gr / 256 - (pixel_num >> 1) * bls_g), 0, MAX_32BITS);
                statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumb[i] = CLIP(
                            (s64)((u64)stats->params.rawae3_1.sumb[i] * awb1_gain_b / 256 - (pixel_num >> 2) * bls_b), 0, MAX_29BITS);
            }

            if (bls_cfg->bls1_en && !is_hdr) {
                memset(statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins, 0,
                       ISP3X_HIST_BIN_N_MAX * sizeof(u32));

                u16 ob;
                s16 bls1, bls;
                int tmp;
                u16 awb1_gain, div_part, round_part;

                switch (isp_params->rawhist3.mode) {
                case 2:
                    ob = isp_ob_offset_rb >> 2;
                    bls1 = bls1_val.r >> 2;
                    awb1_gain = awb1_gain_r;
                    bls = (ob + bls1) * awb1_gain;
                    round_part = 128;
                    div_part = 256;
                    break;
                case 3:
                    ob = isp_ob_offset_g >> 4;
                    bls1 = bls1_val.gr >> 4;
                    awb1_gain = awb1_gain_gr;
                    bls = (ob + bls1) * awb1_gain;
                    round_part = 128;
                    div_part = 256;
                    break;
                case 4:
                    ob = isp_ob_offset_rb >> 2;
                    bls1 = bls1_val.b >> 2;
                    awb1_gain = awb1_gain_b;
                    bls = (ob + bls1) * awb1_gain;
                    round_part = 128;
                    div_part = 256;
                    break;
                case 5:
                default:
                    ob = (u16)((isp_ob_offset_g >> 4) * 587 + (isp_ob_offset_rb >> 2) * 299 + (isp_ob_offset_rb >> 2) * 114 + 500) / 1000;
                    bls1 = (s16)((bls1_val.gr >> 4) * 587 + (bls1_val.r >> 2) * 299 + (bls1_val.b >> 2) * 114 + 500) / 1000;
                    awb1_gain = 100;
                    bls = (ob + bls1) * awb1_gain;
                    div_part = 7655 / awb1_gain_r + 15027 / awb1_gain_gr + 2919 / awb1_gain_b;
                    round_part = div_part / 2;
                    break;
                }


                for (int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
                    tmp = (i - ob - bls1 > 0) ? (i * awb1_gain - bls + round_part) / div_part : 0;
                    tmp = (tmp > ISP3X_HIST_BIN_N_MAX - 1) ? (ISP3X_HIST_BIN_N_MAX - 1) : tmp;

                    statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins[tmp] +=
                        stats->params.rawhist3.hist_bin[i];
                }
            } else {
                memcpy(statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins,
                       stats->params.rawhist3.hist_bin, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
            }
            break;
        case AEC_RAWSEL_MODE_TMO:
            for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
                statsInt->aec_stats.ae_data.extra.rawae_big.channelr_xy[i] =
                    stats->params.rawae3_0.data[i].channelr_xy;
                statsInt->aec_stats.ae_data.extra.rawae_big.channelg_xy[i] =
                    stats->params.rawae3_0.data[i].channelg_xy;
                statsInt->aec_stats.ae_data.extra.rawae_big.channelb_xy[i] =
                    stats->params.rawae3_0.data[i].channelb_xy;
            }
            for (int i = 0; i < ISP3X_RAWAEBIG_SUBWIN_NUM; i++) {
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumr[i] =
                    stats->params.rawae3_1.sumr[i];
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumg[i] =
                    stats->params.rawae3_1.sumg[i];
                statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumb[i] =
                    stats->params.rawae3_1.sumb[i];
            }
            memcpy(statsInt->aec_stats.ae_data.extra.rawhist_big.bins,
                   stats->params.rawhist3.hist_bin, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
            break;

        default:
            LOGE("wrong AeSelMode=%d\n", AeSelMode);
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

#ifdef AE_STATS_DEBUG
    if (AeSwapMode != 0) {
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i * 15 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i * 15 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i * 15 + j]);
            }
        }
        printf("====================sub-win-result======================\n");

        for (int i = 0; i < 4; i++)
            printf("chn0_subwin[%d]:sumr 0x%08" PRIx64", sumg 0x%08" PRIx64", sumb 0x%08" PRIx64"\n", i,
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i],
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i],
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i]);

        printf("====================hist_result========================\n");

        for (int i = 0; i < 256; i++)
            printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins[i]);

    } else {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelr_xy[i * 5 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelg_xy[i * 5 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelb_xy[i * 5 + j]);
            }
        }
        printf("====================hist_result========================\n");
        for (int i = 0; i < 256; i++)
            printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[0].rawhist_lite.bins[i]);
    }

#endif

    /*
     *         unsigned long chn0_mean = 0, chn1_mean = 0;
     *         for(int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
     *             chn0_mean += stats->params.rawae1.data[i].channelg_xy;
     *             chn1_mean += stats->params.rawae3.data[i].channelg_xy;
     *         }
     *
     *
     *         printf("frame[%d]: chn[0-1]_g_mean_xy: %ld-%ld\n",
     *                 stats->frame_id, chn0_mean/ISP3X_RAWAEBIG_MEAN_NUM,
     *                 chn1_mean/ISP3X_RAWAEBIG_MEAN_NUM);
     */

    // expsoure params
    if (_expParams.ptr()) {
        statsInt->aec_stats.ae_exp = _expParams->data()->aecExpInfo;
        /*printf("frame[%d],gain=%d,time=%d\n", stats->frame_id,
               expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
               expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);*/

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

    // iris params
    if (irisParams.ptr()) {
        float sof_time   = (float)irisParams->data()->sofTime / 1000000000.0f;
        float start_time = (float)irisParams->data()->PIris.StartTim.tv_sec +
                           (float)irisParams->data()->PIris.StartTim.tv_usec / 1000000.0f;
        float end_time = (float)irisParams->data()->PIris.EndTim.tv_sec +
                         (float)irisParams->data()->PIris.EndTim.tv_usec / 1000000.0f;
        float frm_intval = 1 / (statsInt->aec_stats.ae_exp.pixel_clock_freq_mhz * 1000000.0f /
                                (float)statsInt->aec_stats.ae_exp.line_length_pixels /
                                (float)statsInt->aec_stats.ae_exp.frame_length_lines);

        /*printf("%s: step=%d,last-step=%d,start-tim=%f,end-tim=%f,sof_tim=%f\n",
            __func__,
            statsInt->aec_stats.ae_exp.Iris.PIris.step,
            irisParams->data()->PIris.laststep,start_time,end_time,sof_time);
        */

        if (sof_time < end_time + frm_intval)
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->data()->PIris.laststep;
        else
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->data()->PIris.step;
    }

    to->set_sequence(stats->frame_id);
#endif

#if defined(ISP_HW_V32_LITE)

    const SmartPtr<Isp20StatsBuffer> buf = from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp32_lite_stat_buffer* stats;
    SmartPtr<RkAiqAecStats> statsInt = to->data();

    stats = (struct rkisp32_lite_stat_buffer*)(buf->get_v4l2_userptr());
    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", mCamPhyId, stats->frame_id,
                  stats->meas_type);

    SmartPtr<RkAiqIrisParamsProxy> irisParams = buf->get_iris_params();

    // ae stats v3.2
    statsInt->frame_id = stats->frame_id;

    switch (AeSwapMode) {
    case AEC_RAWSWAP_MODE_S_LITE:
        meas_type = ((stats->meas_type >> 7) & (0x01)) & ((stats->meas_type >> 11) & (0x01));
        index0       = 0;
        index1       = 1;
        index2       = 2;
        rawhist_mode = isp_params->rawhist0.mode;
        break;
    case AEC_RAWSWAP_MODE_M_LITE:
        meas_type = ((stats->meas_type >> 8) & (0x01)) & ((stats->meas_type >> 12) & (0x01));
        index0       = 1;
        index1       = 0;
        index2       = 2;
        rawhist_mode = isp_params->rawhist0.mode;
        break;
    default:
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
        break;
    }

    statsInt->aec_stats_valid = (meas_type & 0x01) ? true : false;
    if (!statsInt->aec_stats_valid) return XCAM_RETURN_BYPASS;

    statsInt->af_prior = (isp_params->rawaf.ae_sel == 0) ? true : false; /*for isp32-lite, ae_sel=0 use BIG, ae_sel=1 use LITE*/

    if(is_hdr && statsInt->af_prior) {
        if(AeSelMode != AEC_RAWSEL_MODE_CHN_1) {
            LOGE("wrong AeSelMode=%d\n", AeSelMode);
            return XCAM_RETURN_ERROR_PARAM;
        }
    } else if(!is_hdr && statsInt->af_prior) {
        if(AeSelMode != AEC_RAWSEL_MODE_CHN_0) {
            LOGE("wrong AeSelMode=%d\n", AeSelMode);
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

    //AE-LITE (RAWAE0)
    if(!statsInt->af_prior) {
        if(is_hdr || AeSwapMode == AEC_RAWSWAP_MODE_S_LITE) {
            for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {

                statsInt->aec_stats.ae_data.chn[index0].rawae_lite.channelr_xy[i] = CLIP(
                            (int)(stats->params.rawae0.data[i].channelr_xy * awb1_gain_r / 256 - bls_r), 0, MAX_10BITS);
                statsInt->aec_stats.ae_data.chn[index0].rawae_lite.channelg_xy[i] = CLIP(
                            (int)(stats->params.rawae0.data[i].channelg_xy * awb1_gain_gr / 256 - bls_g), 0, MAX_12BITS);
                statsInt->aec_stats.ae_data.chn[index0].rawae_lite.channelb_xy[i] = CLIP(
                            (int)(stats->params.rawae0.data[i].channelb_xy * awb1_gain_b / 256 - bls_b), 0, MAX_10BITS);
            }
        }
    }

    //HIST-LITE (RAWHIST0)
    if (bls_cfg->bls1_en && !is_hdr) {

        memset(statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins, 0,
               ISP3X_HIST_BIN_N_MAX * sizeof(u32));

        s16 bls1, bls;
        u16 ob, awb1_gain, div_part, round_part;
        int tmp;

        switch (rawhist_mode) {
        case 2:
            ob = isp_ob_offset_rb >> 2;
            bls1 = bls1_val.r >> 2;
            awb1_gain = awb1_gain_r;
            bls = (ob + bls1) * awb1_gain;
            div_part = 256;
            round_part = 128;
            break;
        case 3:
            ob = isp_ob_offset_g >> 4;
            bls1 = bls1_val.gr >> 4;
            awb1_gain = awb1_gain_gr;
            bls = (ob + bls1) * awb1_gain;
            div_part = 256;
            round_part = 128;
            break;
        case 4:
            ob = isp_ob_offset_rb >> 2;
            bls1 = bls1_val.b >> 2;
            awb1_gain = awb1_gain_b;
            bls = (ob + bls1) * awb1_gain;
            div_part = 256;
            round_part = 128;
            break;
        case 5:
        default:
            ob = (u16)((isp_ob_offset_g >> 4) * 587 + (isp_ob_offset_rb >> 2) * 299 + (isp_ob_offset_rb >> 2) * 114 + 500) / 1000;
            bls1 = (s16)((bls1_val.gr >> 4) * 587 + (bls1_val.r >> 2) * 299 + (bls1_val.b >> 2) * 114 + 500) / 1000;
            awb1_gain = 100;
            bls = (ob + bls1) * awb1_gain;
            div_part = 7655 / awb1_gain_r + 15027 / awb1_gain_gr + 2919 / awb1_gain_b;
            round_part = div_part / 2;
            break;
        }

        int oneBinWidth = 256 / ISP32L_HIST_LITE_BIN_N_MAX;

        for (int i = 0; i < ISP32L_HIST_LITE_BIN_N_MAX; i++) {

            tmp = oneBinWidth * (i + 0.5);
            tmp = (tmp - ob - bls1 > 0) ? (tmp * awb1_gain - bls + round_part) / div_part : 0;
            tmp = (tmp > oneBinWidth * (ISP32L_HIST_LITE_BIN_N_MAX - 0.5)) ?
                  oneBinWidth * (ISP32L_HIST_LITE_BIN_N_MAX - 0.5) : tmp;

            tmp = tmp / oneBinWidth;

            statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins[tmp] +=
                stats->params.rawhist0.hist_bin[i];
        }
    } else {

        if(is_hdr || AeSwapMode == AEC_RAWSWAP_MODE_S_LITE) {
            memcpy(statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins,
                   stats->params.rawhist0.hist_bin, ISP32L_HIST_LITE_BIN_N_MAX * sizeof(u32));
        }

    }

    // AE/HIST-BIG (RAWAE3/RAWHIST3)
    switch (AeSelMode) {
    case AEC_RAWSEL_MODE_CHN_0:
    case AEC_RAWSEL_MODE_CHN_1:

        //RAWAE3 15x15 block
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.channelr_xy[i] = CLIP(
                        (int)(stats->params.rawae3.data[i].channelr_xy * awb1_gain_r / 256 - bls_r), 0, MAX_10BITS);
            statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.channelg_xy[i] = CLIP(
                        (int)(stats->params.rawae3.data[i].channelg_xy * awb1_gain_gr / 256 - bls_g), 0, MAX_12BITS);
            statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.channelb_xy[i] = CLIP(
                        (int)(stats->params.rawae3.data[i].channelb_xy * awb1_gain_b / 256 - bls_b), 0, MAX_10BITS);
        }

        //RAWAE3 independent block, only one for isp32-lite
        pixel_num = MAX(1, isp_params->rawae3.subwin[0].h_size * isp_params->rawae3.subwin[0].v_size);

        statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumr[0] = CLIP(
                    (s64)((u64)stats->params.rawae3.sumr * awb1_gain_r / 256 - (pixel_num >> 2) * bls_r), 0, MAX_29BITS);
        statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumg[0] = CLIP(
                    (s64)((u64)stats->params.rawae3.sumg * awb1_gain_gr / 256 - (pixel_num >> 1) * bls_g), 0, MAX_32BITS);
        statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumb[0] = CLIP(
                    (s64)((u64)stats->params.rawae3.sumb * awb1_gain_b / 256 - (pixel_num >> 2) * bls_b), 0, MAX_29BITS);

        statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_channelr[0] =
            statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumr[0] / pixel_num;

        statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_channelg[0] =
            statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumg[0] / pixel_num;

        statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_channelb[0] =
            statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big.wndx_sumb[0] / pixel_num;

        //RAWHIST 3
        if (bls_cfg->bls1_en && !is_hdr) {
            memset(statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins, 0,
                   ISP3X_HIST_BIN_N_MAX * sizeof(u32));

            u16 ob;
            s16 bls1, bls;
            int tmp;
            u16 awb1_gain, div_part, round_part;

            switch (isp_params->rawhist3.mode) {
            case 2:
                ob = isp_ob_offset_rb >> 2;
                bls1 = bls1_val.r >> 2;
                awb1_gain = awb1_gain_r;
                bls = (ob + bls1) * awb1_gain;
                round_part = 128;
                div_part = 256;
                break;
            case 3:
                ob = isp_ob_offset_g >> 4;
                bls1 = bls1_val.gr >> 4;
                awb1_gain = awb1_gain_gr;
                bls = (ob + bls1) * awb1_gain;
                round_part = 128;
                div_part = 256;
                break;
            case 4:
                ob = isp_ob_offset_rb >> 2;
                bls1 = bls1_val.b >> 2;
                awb1_gain = awb1_gain_b;
                bls = (ob + bls1) * awb1_gain;
                round_part = 128;
                div_part = 256;
                break;
            case 5:
            default:
                ob = (u16)((isp_ob_offset_g >> 4) * 587 + (isp_ob_offset_rb >> 2) * 299 + (isp_ob_offset_rb >> 2) * 114 + 500) / 1000;
                bls1 = (s16)((bls1_val.gr >> 4) * 587 + (bls1_val.r >> 2) * 299 + (bls1_val.b >> 2) * 114 + 500) / 1000;
                awb1_gain = 100;
                bls = (ob + bls1) * awb1_gain;
                div_part = 7655 / awb1_gain_r + 15027 / awb1_gain_gr + 2919 / awb1_gain_b;
                round_part = div_part / 2;
                break;
            }


            for (int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
                tmp = (i - ob - bls1 > 0) ? (i * awb1_gain - bls + round_part) / div_part : 0;
                tmp = (tmp > ISP3X_HIST_BIN_N_MAX - 1) ? (ISP3X_HIST_BIN_N_MAX - 1) : tmp;

                statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins[tmp] +=
                    stats->params.rawhist3.hist_bin[i];
            }
        } else {
            memcpy(statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins,
                   stats->params.rawhist3.hist_bin, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
        }
        break;
    case AEC_RAWSEL_MODE_TMO:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            statsInt->aec_stats.ae_data.extra.rawae_big.channelr_xy[i] =
                stats->params.rawae3.data[i].channelr_xy;
            statsInt->aec_stats.ae_data.extra.rawae_big.channelg_xy[i] =
                stats->params.rawae3.data[i].channelg_xy;
            statsInt->aec_stats.ae_data.extra.rawae_big.channelb_xy[i] =
                stats->params.rawae3.data[i].channelb_xy;
        }

        statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumr[0] =
            stats->params.rawae3.sumr;
        statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumg[0] =
            stats->params.rawae3.sumg;
        statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumb[0] =
            stats->params.rawae3.sumb;

        pixel_num = MAX(1, isp_params->rawae3.subwin[0].h_size * isp_params->rawae3.subwin[0].v_size);

        statsInt->aec_stats.ae_data.extra.rawae_big.wndx_channelr[0] =
            statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumr[0] / pixel_num;

        statsInt->aec_stats.ae_data.extra.rawae_big.wndx_channelg[0] =
            statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumg[0] / pixel_num;

        statsInt->aec_stats.ae_data.extra.rawae_big.wndx_channelb[0] =
            statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumb[0] / pixel_num;

        memcpy(statsInt->aec_stats.ae_data.extra.rawhist_big.bins,
               stats->params.rawhist3.hist_bin, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
        break;

    default:
        LOGE("wrong AeSelMode=%d\n", AeSelMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

#ifdef AE_STATS_DEBUG
    if(is_hdr) {
        if(AeSelMode != 1) {
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                           statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelr_xy[i * 5 + j],
                           statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelg_xy[i * 5 + j],
                           statsInt->aec_stats.ae_data.chn[1].rawae_lite.channelb_xy[i * 5 + j]);
                }
            }
            printf("====================hist_result========================\n");
            for (int i = 0; i < ISP32L_HIST_LITE_BIN_N_MAX; i++)
                printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[1].rawhist_lite.bins[i]);
        } else {

            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 15; j++) {
                    printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                           statsInt->aec_stats.ae_data.chn[1].rawae_big.channelr_xy[i * 15 + j],
                           statsInt->aec_stats.ae_data.chn[1].rawae_big.channelg_xy[i * 15 + j],
                           statsInt->aec_stats.ae_data.chn[1].rawae_big.channelb_xy[i * 15 + j]);
                }
            }
            printf("====================sub-win-result======================\n");

            printf("chn0_subwin:sumr 0x%08" PRIx64", sumg 0x%08" PRIx64", sumb 0x%08" PRIx64"\n",
                   statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumr[0],
                   statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumg[0],
                   statsInt->aec_stats.ae_data.chn[1].rawae_big.wndx_sumb[0]);

            printf("====================hist_result========================\n");

            for (int i = 0; i < 256; i++)
                printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[1].rawhist_big.bins[i]);
        }
    } else {
        if (AeSelMode == 0) {
            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 15; j++) {
                    printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                           statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i * 15 + j],
                           statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i * 15 + j],
                           statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i * 15 + j]);
                }
            }
            printf("====================sub-win-result======================\n");

            printf("chn0_subwin:sumr 0x%08" PRIx64", sumg 0x%08" PRIx64", sumb 0x%08" PRIx64"\n",
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[0],
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[0],
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[0]);

            printf("====================hist_result========================\n");

            for (int i = 0; i < 256; i++)
                printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins[i]);

        } else {
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                           statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelr_xy[i * 5 + j],
                           statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelg_xy[i * 5 + j],
                           statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelb_xy[i * 5 + j]);
                }
            }
            printf("====================hist_result========================\n");
            for (int i = 0; i < ISP32L_HIST_LITE_BIN_N_MAX; i++)
                printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[0].rawhist_lite.bins[i]);
        }
    }

#endif

    /*
     *         unsigned long chn0_mean = 0, chn1_mean = 0;
     *         for(int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
     *             chn0_mean += stats->params.rawae1.data[i].channelg_xy;
     *             chn1_mean += stats->params.rawae3.data[i].channelg_xy;
     *         }
     *
     *
     *         printf("frame[%d]: chn[0-1]_g_mean_xy: %ld-%ld\n",
     *                 stats->frame_id, chn0_mean/ISP3X_RAWAEBIG_MEAN_NUM,
     *                 chn1_mean/ISP3X_RAWAEBIG_MEAN_NUM);
     */

    // expsoure params
    if (_expParams.ptr()) {
        statsInt->aec_stats.ae_exp = _expParams->data()->aecExpInfo;
        /*printf("frame[%d],gain=%d,time=%d\n", stats->frame_id,
               expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
               expParams->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);*/

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

    // iris params
    if (irisParams.ptr()) {
        float sof_time   = (float)irisParams->data()->sofTime / 1000000000.0f;
        float start_time = (float)irisParams->data()->PIris.StartTim.tv_sec +
                           (float)irisParams->data()->PIris.StartTim.tv_usec / 1000000.0f;
        float end_time = (float)irisParams->data()->PIris.EndTim.tv_sec +
                         (float)irisParams->data()->PIris.EndTim.tv_usec / 1000000.0f;
        float frm_intval = 1 / (statsInt->aec_stats.ae_exp.pixel_clock_freq_mhz * 1000000.0f /
                                (float)statsInt->aec_stats.ae_exp.line_length_pixels /
                                (float)statsInt->aec_stats.ae_exp.frame_length_lines);

        /*printf("%s: step=%d,last-step=%d,start-tim=%f,end-tim=%f,sof_tim=%f\n",
            __func__,
            statsInt->aec_stats.ae_exp.Iris.PIris.step,
            irisParams->data()->PIris.laststep,start_time,end_time,sof_time);
        */

        if (sof_time < end_time + frm_intval)
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->data()->PIris.laststep;
        else
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->data()->PIris.step;
    }

    to->set_sequence(stats->frame_id);
#endif

    return ret;
}

XCamReturn RkAiqResourceTranslatorV32::translateAwbStats(const SmartPtr<VideoBuffer>& from,
        SmartPtr<RkAiqAwbStatsProxy>& to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
#if defined(ISP_HW_V32)
    struct rkisp32_isp_stat_buffer *stats;
    stats = (struct rkisp32_isp_stat_buffer*)(buf->get_v4l2_userptr());
#elif defined(ISP_HW_V32_LITE)
    struct rkisp32_lite_stat_buffer *stats;
    stats = (struct rkisp32_lite_stat_buffer*)(buf->get_v4l2_userptr());
#endif

    SmartPtr<RkAiqAwbStats> statsInt = to->data();

    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("awb stats: camId:%d, frame_id: %d,  meas_type; 0x%x",
                  mCamPhyId, stats->frame_id, stats->meas_type);

    statsInt->awb_stats_valid = stats->meas_type >> 5 & 1;
    if (!statsInt->awb_stats_valid) {
        LOGE_ANALYZER("AWB stats invalid, ignore");
        return XCAM_RETURN_BYPASS;
    }
    memset(&statsInt->awb_stats_v32, 0, sizeof(statsInt->awb_stats_v32));

    if (stats->params.info2ddr.owner == RKISP_INFO2DRR_OWNER_AWB) {
        statsInt->awb_stats_v32.dbginfo_fd = stats->params.info2ddr.buf_fd;
    } else {
        statsInt->awb_stats_v32.dbginfo_fd = -1;
    }
    statsInt->awb_stats_v32.awb_cfg_effect_v32.blkMeasureMode = _ispParams.awb_cfg_v32.blkMeasureMode;
    statsInt->awb_stats_v32.awb_cfg_effect_v32.lightNum = _ispParams.awb_cfg_v32.lightNum;
    statsInt->awb_stats_v32.awb_cfg_effect_v32.groupIllIndxCurrent = _ispParams.awb_cfg_v32.groupIllIndxCurrent;
    memcpy(statsInt->awb_stats_v32.awb_cfg_effect_v32.IllIndxSetCurrent, _ispParams.awb_cfg_v32.IllIndxSetCurrent,
           sizeof(statsInt->awb_stats_v32.awb_cfg_effect_v32.IllIndxSetCurrent));
    memcpy(statsInt->awb_stats_v32.awb_cfg_effect_v32.timeSign, _ispParams.awb_cfg_v32.timeSign,
           sizeof(statsInt->awb_stats_v32.awb_cfg_effect_v32.timeSign));
    statsInt->awb_cfg_effect_valid = true;
    statsInt->frame_id = stats->frame_id;
    for(int i = 0; i < statsInt->awb_stats_v32.awb_cfg_effect_v32.lightNum; i++) {
        statsInt->awb_stats_v32.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].RgainValue =
            stats->params.rawawb.sum[i].rgain_nor;
        statsInt->awb_stats_v32.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].BgainValue =
            stats->params.rawawb.sum[i].bgain_nor;
        statsInt->awb_stats_v32.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].WpNo =
            stats->params.rawawb.sum[i].wp_num_nor;
        statsInt->awb_stats_v32.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].RgainValue =
            stats->params.rawawb.sum[i].rgain_big;
        statsInt->awb_stats_v32.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].BgainValue =
            stats->params.rawawb.sum[i].bgain_big;
        statsInt->awb_stats_v32.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].WpNo =
            stats->params.rawawb.sum[i].wp_num_big;
    }
#if defined(ISP_HW_V32)
    for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        statsInt->awb_stats_v32.blockResult[i].Rvalue = stats->params.rawawb.ramdata[i].r;
        statsInt->awb_stats_v32.blockResult[i].Gvalue = stats->params.rawawb.ramdata[i].g;
        statsInt->awb_stats_v32.blockResult[i].Bvalue = stats->params.rawawb.ramdata[i].b;
        statsInt->awb_stats_v32.blockResult[i].WpNo = stats->params.rawawb.ramdata[i].wp;
    }
#elif defined(ISP_HW_V32_LITE)
    for(int i = 0; i < ISP32L_RAWAWB_RAMDATA_RGB_NUM; i++) {
        statsInt->awb_stats_v32.blockResult[i].Rvalue = stats->params.rawawb.ramdata_r[i];
        statsInt->awb_stats_v32.blockResult[i].Gvalue = stats->params.rawawb.ramdata_g[i];
        statsInt->awb_stats_v32.blockResult[i].Bvalue = stats->params.rawawb.ramdata_b[i];
        if(i%2==0){
            statsInt->awb_stats_v32.blockResult[i].WpNo = stats->params.rawawb.ramdata_wpnum0[i/2];
        }else{
            statsInt->awb_stats_v32.blockResult[i].WpNo = stats->params.rawawb.ramdata_wpnum1[i/2];
        }
    }
#endif
    for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
        statsInt->awb_stats_v32.WpNoHist[i] = stats->params.rawawb.yhist_bin[i];
        // move the shift code here to make WpNoHist merged by several cameras easily
        if( stats->params.rawawb.yhist_bin[i]  & 0x8000 ) {
            statsInt->awb_stats_v32.WpNoHist[i] = stats->params.rawawb.yhist_bin[i] & 0x7FFF;
            statsInt->awb_stats_v32.WpNoHist[i] *=    (1 << 3);
        }
    }

    for(int i = 0; i < statsInt->awb_stats_v32.awb_cfg_effect_v32.lightNum; i++) {
        statsInt->awb_stats_v32.WpNo2[i] =  stats->params.rawawb.sum[i].wp_num2;
    }
    for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
        statsInt->awb_stats_v32.excWpRangeResult[i].RgainValue = stats->params.rawawb.sum_exc[i].rgain_exc;
        statsInt->awb_stats_v32.excWpRangeResult[i].BgainValue = stats->params.rawawb.sum_exc[i].bgain_exc;
        statsInt->awb_stats_v32.excWpRangeResult[i].WpNo =    stats->params.rawawb.sum_exc[i].wp_num_exc;

    }

    to->set_sequence(stats->frame_id);
#endif
    return ret;
}



XCamReturn RkAiqResourceTranslatorV32::translateAfStats(const SmartPtr<VideoBuffer>& from,
        SmartPtr<RkAiqAfStatsProxy>& to) {

    struct isp32_rawaf_luma_data {
        u32 channelg_xy: 12;
        u32 highlit_cnt: 16;
        u32 dummy: 4;
    } __attribute__ ((packed));

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V32)
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp32_isp_stat_buffer *stats;
    SmartPtr<RkAiqAfStats> statsInt = to->data();

    stats = (struct rkisp32_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE_AF("fail to get stats, ignore");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x",
                  stats->frame_id, stats->meas_type);

    statsInt->af_stats_valid =
        (stats->meas_type >> 6) & (0x01) ? true : false;
    if (!statsInt->af_stats_valid)
        return XCAM_RETURN_BYPASS;

    SmartPtr<RkAiqAfInfoProxy> afParams = buf->get_af_params();
    struct isp32_bls_cfg* bls_cfg = &_ispParams.bls_cfg;
    u8 from_awb = _ispParams.meas.rawaf.from_awb;
    u8 from_ynr = _ispParams.meas.rawaf.from_ynr;
    bool is_hdr = (getWorkingMode() > 0) ? true : false;
    int temp_luma, comp_bls = 0;
    u16 max_val = (1 << 12) - 1;

    if (bls_cfg->bls1_en && !is_hdr && !from_awb && !from_ynr) {
        comp_bls = (bls_cfg->bls1_val.gr + bls_cfg->bls1_val.gb) / 2  - bls_cfg->isp_ob_offset;
        comp_bls = MAX(comp_bls, 0);
    }

#if 0
    struct isp32_awb_gain_cfg* awb_gain_cfg = &_ispParams.awb_gain_cfg;

    LOGE("bls0[%d-%d]", bls_cfg->fixed_val.gr, bls_cfg->fixed_val.gb);
    LOGE("bls1[%d-%d]", bls_cfg->bls1_val.gr, bls_cfg->bls1_val.gb);
    LOGE("isp_ob_offset, isp_ob_max, isp_ob_predgain [%d-%d-%d]",
         bls_cfg->isp_ob_offset, bls_cfg->isp_ob_max, bls_cfg->isp_ob_predgain);
    LOGE("awb0_gain[%d-%d], awb1_gain[%d-%d]",
         awb_gain_cfg->gain0_green_b, awb_gain_cfg->gain0_green_r,
         awb_gain_cfg->gain1_green_b, awb_gain_cfg->gain1_green_r);
    LOGE("comp_bls %d", comp_bls);
#endif

    memset(&statsInt->af_stats_v3x, 0, sizeof(rk_aiq_isp_af_stats_v3x_t));
    statsInt->frame_id = stats->frame_id;

    //af
    {
        statsInt->af_stats_v3x.comp_bls = comp_bls >> 2;
        statsInt->af_stats_v3x.wndb_luma = stats->params.rawaf.afm_lum_b;
        statsInt->af_stats_v3x.wndb_sharpness = stats->params.rawaf.afm_sum_b;
        statsInt->af_stats_v3x.winb_highlit_cnt = stats->params.rawaf.highlit_cnt_winb;
        for (int i = 0; i < RKAIQ_RAWAF_SUMDATA_NUM; i++) {
            statsInt->af_stats_v3x.wnda_fv_v1[i] = stats->params.rawaf.ramdata[i].v1;
            statsInt->af_stats_v3x.wnda_fv_v2[i] = stats->params.rawaf.ramdata[i].v2;
            statsInt->af_stats_v3x.wnda_fv_h1[i] = stats->params.rawaf.ramdata[i].h1;
            statsInt->af_stats_v3x.wnda_fv_h2[i] = stats->params.rawaf.ramdata[i].h2;
        }

        struct isp32_rawaf_luma_data *luma = (struct isp32_rawaf_luma_data *)&stats->params.rawae3_0.data[0];
        for (int i = 0; i < RKAIQ_RAWAF_SUMDATA_NUM; i++) {
            temp_luma = (luma[i].channelg_xy - comp_bls) * max_val / (max_val - comp_bls);
            statsInt->af_stats_v3x.wnda_luma[i] = MAX(temp_luma, 0);
            statsInt->af_stats_v3x.wina_highlit_cnt[i] = luma[i].highlit_cnt;
        }

        if(afParams.ptr()) {
            statsInt->af_stats_v3x.focusCode = afParams->data()->focusCode;
            statsInt->af_stats_v3x.zoomCode = afParams->data()->zoomCode;
            statsInt->af_stats_v3x.focus_endtim = afParams->data()->focusEndTim;
            statsInt->af_stats_v3x.focus_starttim = afParams->data()->focusStartTim;
            statsInt->af_stats_v3x.zoom_endtim = afParams->data()->zoomEndTim;
            statsInt->af_stats_v3x.zoom_starttim = afParams->data()->zoomStartTim;
            statsInt->af_stats_v3x.sof_tim = afParams->data()->sofTime;
            statsInt->af_stats_v3x.focusCorrection = afParams->data()->focusCorrection;
            statsInt->af_stats_v3x.zoomCorrection = afParams->data()->zoomCorrection;
            statsInt->af_stats_v3x.angleZ = afParams->data()->angleZ;
        }

        if (_expParams.ptr())
            statsInt->aecExpInfo = _expParams->data()->aecExpInfo;
    }

#endif

#if defined(ISP_HW_V32_LITE)
    const SmartPtr<Isp20StatsBuffer> buf =
        from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp32_lite_stat_buffer *stats_lite;
    SmartPtr<RkAiqAfStats> statsInt = to->data();

    stats_lite = (struct rkisp32_lite_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats_lite == NULL) {
        LOGE_AF("fail to get stats, ignore");
        return XCAM_RETURN_BYPASS;
    }
    LOGD_AF("stats_lite: frame_id: %d,  meas_type; 0x%x",
            stats_lite->frame_id, stats_lite->meas_type);

    statsInt->af_stats_valid =
        (stats_lite->meas_type >> 6) & (0x01) ? true : false;
    if (!statsInt->af_stats_valid)
        return XCAM_RETURN_BYPASS;

    SmartPtr<RkAiqAfInfoProxy> afParams = buf->get_af_params();
    struct isp32_bls_cfg* bls_cfg = &_ispParams.bls_cfg;
    u8 from_awb = _ispParams.meas.rawaf.from_awb;
    u8 from_ynr = _ispParams.meas.rawaf.from_ynr;
    bool is_hdr = (getWorkingMode() > 0) ? true : false;
    int temp_luma, comp_bls = 0;
    u16 max_val = (1 << 12) - 1;

    if (bls_cfg->bls1_en && !is_hdr && !from_awb && !from_ynr) {
        comp_bls = (bls_cfg->bls1_val.gr + bls_cfg->bls1_val.gb) / 2  - bls_cfg->isp_ob_offset;
        comp_bls = MAX(comp_bls, 0);
    }

    memset(&statsInt->af_stats_v3x, 0, sizeof(rk_aiq_isp_af_stats_v3x_t));
    statsInt->frame_id = stats_lite->frame_id;

    //af
    {
        statsInt->af_stats_v3x.comp_bls = comp_bls >> 2;
        statsInt->af_stats_v3x.wndb_luma = stats_lite->params.rawaf.afm_lum_b;
        statsInt->af_stats_v3x.wndb_sharpness = stats_lite->params.rawaf.afm_sum_b;
        statsInt->af_stats_v3x.winb_highlit_cnt = stats_lite->params.rawaf.highlit_cnt_winb;
        for (int i = 0; i < ISP32L_RAWAF_WND_DATA; i++) {
            statsInt->af_stats_v3x.wnda_fv_v1[i] = stats_lite->params.rawaf.ramdata.viir_wnd_data[i];
            statsInt->af_stats_v3x.wnda_fv_h1[i] = stats_lite->params.rawaf.ramdata.hiir_wnd_data[i];
            statsInt->af_stats_v3x.wnda_fv_v2[i] = statsInt->af_stats_v3x.wnda_fv_v1[i];
            statsInt->af_stats_v3x.wnda_fv_h2[i] = statsInt->af_stats_v3x.wnda_fv_h1[i];
        }

        for (int i = 0; i < ISP32L_RAWAF_WND_DATA; i++) {
            temp_luma = (stats_lite->params.rawae0.data[i].channelg_xy - comp_bls) * max_val / (max_val - comp_bls);
            statsInt->af_stats_v3x.wnda_luma[i] = MAX(temp_luma, 0);
            statsInt->af_stats_v3x.wina_highlit_cnt[i] =
                ((stats_lite->params.rawae0.data[i].channelr_xy & 0x3FF) << 10) | (stats_lite->params.rawae0.data[i].channelb_xy & 0x3FF);
        }

        if(afParams.ptr()) {
            statsInt->af_stats_v3x.focusCode = afParams->data()->focusCode;
            statsInt->af_stats_v3x.zoomCode = afParams->data()->zoomCode;
            statsInt->af_stats_v3x.focus_endtim = afParams->data()->focusEndTim;
            statsInt->af_stats_v3x.focus_starttim = afParams->data()->focusStartTim;
            statsInt->af_stats_v3x.zoom_endtim = afParams->data()->zoomEndTim;
            statsInt->af_stats_v3x.zoom_starttim = afParams->data()->zoomStartTim;
            statsInt->af_stats_v3x.sof_tim = afParams->data()->sofTime;
            statsInt->af_stats_v3x.focusCorrection = afParams->data()->focusCorrection;
            statsInt->af_stats_v3x.zoomCorrection = afParams->data()->zoomCorrection;
            statsInt->af_stats_v3x.angleZ = afParams->data()->angleZ;
        }

        if (_expParams.ptr())
            statsInt->aecExpInfo = _expParams->data()->aecExpInfo;
    }

#endif

    return ret;

}

#if RKAIQ_HAVE_DEHAZE_V12
XCamReturn RkAiqResourceTranslatorV32::translateAdehazeStats(const SmartPtr<VideoBuffer>& from,
        SmartPtr<RkAiqAdehazeStatsProxy>& to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V32)
    const SmartPtr<Isp20StatsBuffer> buf = from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp32_isp_stat_buffer* stats;
    SmartPtr<RkAiqAdehazeStats> statsInt = to->data();

    stats = (struct rkisp32_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", stats->frame_id, stats->meas_type);

    // dehaze
    statsInt->adehaze_stats_valid = stats->meas_type >> 17 & 1;
    if (!statsInt->adehaze_stats_valid) return XCAM_RETURN_BYPASS;

    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_adp_air_base =
        stats->params.dhaz.dhaz_adp_air_base;
    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_adp_wt     = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_adp_gratio = stats->params.dhaz.dhaz_adp_gratio;
    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_adp_wt     = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_pic_sumh   = stats->params.dhaz.dhaz_pic_sumh;
    for (int i = 0; i < ISP3X_DHAZ_HIST_IIR_NUM; i++)
        statsInt->adehaze_stats.dehaze_stats_v12.h_rgb_iir[i] = stats->params.dhaz.h_rgb_iir[i];
#endif

#if defined(ISP_HW_V32_LITE)
    const SmartPtr<Isp20StatsBuffer> buf = from.dynamic_cast_ptr<Isp20StatsBuffer>();
    struct rkisp32_lite_stat_buffer* stats;
    SmartPtr<RkAiqAdehazeStats> statsInt = to->data();

    stats = (struct rkisp32_lite_stat_buffer*)(buf->get_v4l2_userptr());
    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", stats->frame_id, stats->meas_type);

    // dehaze
    statsInt->adehaze_stats_valid = stats->meas_type >> 17 & 1;
    if (!statsInt->adehaze_stats_valid) return XCAM_RETURN_BYPASS;

    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_adp_air_base =
        stats->params.dhaz.dhaz_adp_air_base;
    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_adp_wt     = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_adp_gratio = stats->params.dhaz.dhaz_adp_gratio;
    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_adp_wt     = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v12.dhaz_pic_sumh   = stats->params.dhaz.dhaz_pic_sumh;
    for (int i = 0; i < ISP3X_DHAZ_HIST_IIR_NUM; i++)
        statsInt->adehaze_stats.dehaze_stats_v12.h_rgb_iir[i] = stats->params.dhaz.h_rgb_iir[i];

#endif

    return ret;
}
#endif

}  // namespace RkCam
