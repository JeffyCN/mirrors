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
#include "common/rkisp2-config.h"
#include "common/rkisp21-config.h"
#include "RkAiqResourceTranslatorV21.h"
#include "PdafStreamProcUnit.h"
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace RkCam {

XCamReturn
RkAiqResourceTranslatorV21::translateAwbStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAwbStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V21)
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
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

    rkisp_effect_params_v20 ispParams;
    memset(&ispParams, 0, sizeof(ispParams));
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
        // move the shift code here to make WpNoHist merged by several cameras easily
        if( stats->params.rawawb.ro_yhist_bin[i]  & 0x8000 ){
             statsInt->awb_stats_v201.WpNoHist[i] = stats->params.rawawb.ro_yhist_bin[i] & 0x7FFF;
             statsInt->awb_stats_v201.WpNoHist[i] *=    (1 << 3);
        }

    }

    //statsInt->awb_stats_valid = ISP2X_STAT_RAWAWB(stats->meas_type)? true:false;
    statsInt->awb_stats_valid = stats->meas_type >> 5 & 1;

    to->set_sequence(stats->frame_id);
#endif
    return ret;
}

#if RKAIQ_HAVE_DEHAZE_V11
XCamReturn
RkAiqResourceTranslatorV21::translateAdehazeStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAdehazeStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
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
    statsInt->adehaze_stats.dehaze_stats_v11.dhaz_adp_air_base =
        stats->params.dhaz.dhaz_adp_air_base;
    statsInt->adehaze_stats.dehaze_stats_v11.dhaz_adp_wt     = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v11.dhaz_adp_gratio = stats->params.dhaz.dhaz_adp_gratio;
    statsInt->adehaze_stats.dehaze_stats_v11.dhaz_adp_wt     = stats->params.dhaz.dhaz_adp_wt;
    for(int i = 0; i < 64; i++)
        statsInt->adehaze_stats.dehaze_stats_v11.h_rgb_iir[i] = stats->params.dhaz.h_rgb_iir[i];

    to->set_sequence(stats->frame_id);

    return ret;
}
#endif

} //namespace RkCam
