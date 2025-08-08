/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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

#include "aiq_stats_translator.h"
#include "xcore_c/aiq_v4l2_buffer.h"
#include "hwi_c/aiq_sensorHw.h"
#include "hwi_c/aiq_lensHw.h"

// #define AE_STATS_DEBUG
#define MAX_8BITS  ((1 << 8) - 1)
#define MAX_10BITS (1 << 10) - 1
#define MAX_12BITS (1 << 12) - 1
#define MAX_29BITS (1 << 29) - 1
#define MAX_32BITS 0xffffffff


void JudgeWinLocation39(
    struct isp2x_window* ori_win,
    WinSplitMode* mode,
    Rectangle_t left_isp_rect_,
    Rectangle_t right_isp_rect_
) {

    if (ori_win->h_offs + ori_win->h_size <= left_isp_rect_.w) {
        *mode = LEFT_MODE;
    } else if(ori_win->h_offs >= right_isp_rect_.x) {
        *mode = RIGHT_MODE;
    } else {
        if ((ori_win->h_offs + ori_win->h_size / 2) <= left_isp_rect_.w
                && right_isp_rect_.x <= (ori_win->h_offs + ori_win->h_size / 2)) {
            *mode = LEFT_AND_RIGHT_MODE;
        }
        else {

            if ((ori_win->h_offs + ori_win->h_size / 2) < right_isp_rect_.x) {

                u16 h_size_tmp1 = left_isp_rect_.w - ori_win->h_offs;
                u16 h_size_tmp2 = (right_isp_rect_.x - ori_win->h_offs) * 2;

                if (abs(ori_win->h_size - h_size_tmp1) < abs(ori_win->h_size - h_size_tmp2))
                    *mode = LEFT_MODE;
                else
                    *mode = LEFT_AND_RIGHT_MODE;
            }
            else {

                u16 h_size_tmp1 = ori_win->h_offs + ori_win->h_size - right_isp_rect_.x;
                u16 h_size_tmp2 = (ori_win->h_offs + ori_win->h_size - left_isp_rect_.w) * 2;

                if (abs(ori_win->h_size - h_size_tmp1) < abs(ori_win->h_size - h_size_tmp2))
                    *mode = RIGHT_MODE;
                else
                    *mode = LEFT_AND_RIGHT_MODE;
            }
        }
    }
}

static void MergeAecBigWinStatsV39(
    struct isp39_rawae_stat* left,
    struct isp39_rawae_stat* right,
    struct isp39_rawae_stat* out,
    WinSplitMode             mode
) {
    switch (mode)
    {
    case LEFT_MODE:
        memcpy(out, left, sizeof(struct isp39_rawae_stat));
        break;
    case RIGHT_MODE:
        memcpy(out, right, sizeof(struct isp39_rawae_stat));
        break;
    case LEFT_AND_RIGHT_MODE:
        for (int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
            for (int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                if (j < ISP39_MEAN_BLK_X_NUM / 2) {
                    out->blk_y[i].blk_x[j].r = (left->blk_y[i].blk_x[2 * j].r + left->blk_y[i].blk_x[2 * j + 1].r) / 2;
                    out->blk_y[i].blk_x[j].g = (left->blk_y[i].blk_x[2 * j].g + left->blk_y[i].blk_x[2 * j + 1].g) / 2;
                    out->blk_y[i].blk_x[j].b = (left->blk_y[i].blk_x[2 * j].b + left->blk_y[i].blk_x[2 * j + 1].b) / 2;
                } else if (j > ISP39_MEAN_BLK_X_NUM / 2) {
                    out->blk_y[i].blk_x[j].r = (right->blk_y[i].blk_x[2 * j - ISP39_MEAN_BLK_X_NUM].r +
                                               right->blk_y[i].blk_x[2 * j + 1 - ISP39_MEAN_BLK_X_NUM].r) / 2;
                    out->blk_y[i].blk_x[j].g = (right->blk_y[i].blk_x[2 * j - ISP39_MEAN_BLK_X_NUM].g +
                                               right->blk_y[i].blk_x[2 * j + 1 - ISP39_MEAN_BLK_X_NUM].g) / 2;
                    out->blk_y[i].blk_x[j].b = (right->blk_y[i].blk_x[2 * j - ISP39_MEAN_BLK_X_NUM].b +
                                               right->blk_y[i].blk_x[2 * j + 1 - ISP39_MEAN_BLK_X_NUM].b) / 2;
                } else {
                    out->blk_y[i].blk_x[j].r = (left->blk_y[i].blk_x[ISP39_MEAN_BLK_X_NUM - 1].r + right->blk_y[i].blk_x[0].r) / 2;
                    out->blk_y[i].blk_x[j].g = (left->blk_y[i].blk_x[ISP39_MEAN_BLK_X_NUM - 1].g + right->blk_y[i].blk_x[0].g) / 2;
                    out->blk_y[i].blk_x[j].b = (left->blk_y[i].blk_x[ISP39_MEAN_BLK_X_NUM - 1].b + right->blk_y[i].blk_x[0].b) / 2;
                }
            }
        }
        break;
    default:
        break;
    }
}

void MergeAecHistBinStats(
    u32*                        left_stats,
    u32*                        right_stats,
    u32*                        merge_stats,
    WinSplitMode                mode,
    u8                          hist_mode,
    u16                         ob_offset_rb,
    u16                         ob_offset_g,
    struct isp2x_bls_fixed_val  bls1_val,
    struct isp2x_bls_fixed_val  awb1_gain,
    bool                        is_hdr
) {

    if (!is_hdr) {
        //need to consider bls1 & ob
        memset(merge_stats, 0, ISP32_HIST_BIN_N_MAX * sizeof(u32));

        s16 bls1, bls_part;
        u16 ob_part, awb_part, div_part, round_part;
        int tmp;

        switch (hist_mode) {
        case 2:
            ob_part = ob_offset_rb >> 2;
            bls1 = bls1_val.r >> 2;
            awb_part = awb1_gain.r;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 3:
            ob_part = ob_offset_g >> 4;
            bls1 = bls1_val.gr >> 4;
            awb_part = awb1_gain.gr;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 4:
            ob_part = ob_offset_rb >> 2;
            bls1 = bls1_val.b >> 2;
            awb_part = awb1_gain.b;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 5:
        default:
            ob_part = (u16)((ob_offset_g >> 4) * 587 + (ob_offset_rb >> 2) * 299 + (ob_offset_rb >> 2) * 114 + 500) / 1000;
            bls1 = (s16)((bls1_val.gr >> 4) * 587 + (bls1_val.r >> 2) * 299 + (bls1_val.b >> 2) * 114 + 500) / 1000;
            awb_part = 100;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 7655 / awb1_gain.r + 15027 / awb1_gain.gr + 2919 / awb1_gain.b;
            round_part = div_part / 2;
            break;
        }

        switch(mode) {
        case LEFT_MODE:
            for(int i = 0; i < ISP32_HIST_BIN_N_MAX; i++) {
                tmp = (i - ob_part - bls1 > 0) ? (i * awb_part - bls_part + round_part) / div_part : 0;
                tmp = (tmp > ISP32_HIST_BIN_N_MAX - 1) ? (ISP32_HIST_BIN_N_MAX - 1) : tmp;
                merge_stats[tmp] += left_stats[i];
            }
            break;
        case RIGHT_MODE:
            for(int i = 0; i < ISP32_HIST_BIN_N_MAX; i++) {
                tmp = (i - ob_part - bls1 > 0) ? (i * awb_part - bls_part + round_part) / div_part : 0;
                tmp = (tmp > ISP32_HIST_BIN_N_MAX - 1) ? (ISP32_HIST_BIN_N_MAX - 1) : tmp;
                merge_stats[tmp] += right_stats[i];
            }
            break;
        case LEFT_AND_RIGHT_MODE:
            for(int i = 0; i < ISP32_HIST_BIN_N_MAX; i++) {
                tmp = (i - ob_part - bls1 > 0) ? (i * awb_part - bls_part + round_part) / div_part : 0;
                tmp = (tmp > ISP32_HIST_BIN_N_MAX - 1) ? (ISP32_HIST_BIN_N_MAX - 1) : tmp;
                merge_stats[tmp] += left_stats[i] + right_stats[i];
            }
            break;
        }

    } else {

        switch(mode) {
        case LEFT_MODE:
            for(int i = 0; i < ISP32_HIST_BIN_N_MAX; i++) {
                merge_stats[i] = left_stats[i];
            }
            break;
        case RIGHT_MODE:
            for(int i = 0; i < ISP32_HIST_BIN_N_MAX; i++) {
                merge_stats[i] = right_stats[i];
            }
            break;
        case LEFT_AND_RIGHT_MODE:
            for(int i = 0; i < ISP32_HIST_BIN_N_MAX; i++) {
                merge_stats[i] = left_stats[i] + right_stats[i];
            }
            break;
        }
    }

}


#if defined(ISP_HW_V39) & defined(USE_NEWSTRUCT)

/* for bnr20bit, rawae0 meas LSB(0-9bit) rawae3 meas MSB(10-19bit) */
/* for bnr20bit, rawhist0 meas LSB(4-11bit) rawhist3 meas MSB(12-19bit) */

XCamReturn Aec20BitGirdMerge
(
    struct isp39_rawae_stat LSB,
    struct isp39_rawae_stat MSB,
    aeStats_mainWinStats_t* dst
)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //merge as original 20bit and than scale as bit_num

    for(int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
        //r b channel
        dst->hw_ae_meanBayerR_val[i] = (LSB.blk_y[i / 15].blk_x[i % 15].r | MSB.blk_y[i / 15].blk_x[i % 15].r << 10) >> (20 - 10);
        dst->hw_ae_meanBayerB_val[i] = (LSB.blk_y[i / 15].blk_x[i % 15].b | MSB.blk_y[i / 15].blk_x[i % 15].b << 10) >> (20 - 10);
        // g channel
        dst->hw_ae_meanBayerGrGb_val[i] = (LSB.blk_y[i / 15].blk_x[i % 15].g >> 2 | MSB.blk_y[i / 15].blk_x[i % 15].g << 8) >> (20 - 12);
    }


    return ret;
}

XCamReturn Aec20BitHistMerge(u32* LSB, u32* MSB, u32* dst)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //merge as 256*2 bin, and then scale as 256 bin
    for(int i = 0; i < ISP3X_HIST_BIN_N_MAX / 2; i++)
        dst[i] = LSB[i * 2] + LSB[i * 2 + 1];

    for(int i = ISP3X_HIST_BIN_N_MAX / 2; i < ISP3X_HIST_BIN_N_MAX; i++)
        dst[i] = MSB[i * 2 - ISP3X_HIST_BIN_N_MAX] + MSB[i * 2 - ISP3X_HIST_BIN_N_MAX + 1];

    return ret;
}


void calcAecBigWinStatsV39(
    struct isp39_rawae_stat*    stats_in,
    aeStats_mainWinStats_t*     stats_out,
    struct isp2x_bls_fixed_val  bls1_val,
    struct isp2x_bls_fixed_val  awb1_gain
) {

    //copy all channel
    for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
        stats_out->hw_ae_meanBayerR_val[i] = CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].r * awb1_gain.r / 256 - bls1_val.r), 0, MAX_10BITS);
        stats_out->hw_ae_meanBayerGrGb_val[i] = CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].g * awb1_gain.gr / 256 - bls1_val.gr), 0, MAX_12BITS);
        stats_out->hw_ae_meanBayerB_val[i] = CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].b * awb1_gain.b / 256 - bls1_val.b), 0, MAX_10BITS);
    }

    //For isp39, no sub win

}

void calcAecHistBinStatsV39(
    u32*                        hist_bin_in,
    u32*                        hist_bin_out,
    u8                          hist_mode,
    u16                         ob_offset_rb,
    u16                         ob_offset_g,
    struct isp2x_bls_fixed_val  bls1_val,
    struct isp2x_bls_fixed_val  awb1_gain,
    bool                        is_hdr
) {

    if (!is_hdr) {

        memset(hist_bin_out, 0, ISP39_HIST_BIN_N_MAX * sizeof(u32));

        s16 bls1, bls_part;
        u16 ob_part, awb_part, div_part, round_part;
        int tmp;
        switch (hist_mode) {
        case 2:
            ob_part = ob_offset_rb >> 2;
            bls1 = bls1_val.r >> 2;
            awb_part = awb1_gain.r;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 3:
            ob_part = ob_offset_g >> 4;
            bls1 = bls1_val.gr >> 4;
            awb_part = awb1_gain.gr;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 4:
            ob_part = ob_offset_rb >> 2;
            bls1 = bls1_val.b >> 2;
            awb_part = awb1_gain.b;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 5:
        default:
            ob_part = (u16)((ob_offset_g >> 4) * 587 + (ob_offset_rb >> 2) * 299 + (ob_offset_rb >> 2) * 114 + 500) / 1000;
            bls1 = (s16)((bls1_val.gr >> 4) * 587 + (bls1_val.r >> 2) * 299 + (bls1_val.b >> 2) * 114 + 500) / 1000;
            awb_part = 100;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 7655.0f / (float)awb1_gain.r + 15027.0f / (float)awb1_gain.gr + 2919.0f / (float)awb1_gain.b;
            round_part = div_part / 2;
            break;
        }

        for (int i = 0; i < ISP39_HIST_BIN_N_MAX; i++) {
            tmp = (i - ob_part - bls1 > 0) ? (i * awb_part - bls_part + round_part) / div_part : 0;
            tmp = (tmp > ISP39_HIST_BIN_N_MAX - 1) ? (ISP39_HIST_BIN_N_MAX - 1) : tmp;
            hist_bin_out[tmp] += hist_bin_in[i];
        }


    } else {

        memcpy(hist_bin_out, hist_bin_in, ISP39_HIST_BIN_N_MAX * sizeof(u32));
    }

}

static XCamReturn
translateMultiAecStatsV39(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                          aiq_stats_base_t* to) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39) && defined(RKAIQ_HAVE_MULTIISP)
    struct isp39_isp_meas_cfg* isp_params = &pStatsTrans->_ispParams->meas;
    uint8_t AeSwapMode, AeSelMode, Bnr20bitEn;
    AeSwapMode             = isp_params->rawae0.rawae_sel & 0x0f;
    AeSelMode              = isp_params->rawae3.rawae_sel & 0x0f;
    Bnr20bitEn             = isp_params->rawae0.rawae_sel & 0xf0;
    unsigned int meas_type = 0;

    WinSplitMode AeWinSplitMode[2] = {LEFT_AND_RIGHT_MODE}; //0:rawae0 1:rawae3
    WinSplitMode HistWinSplitMode[2] = {LEFT_AND_RIGHT_MODE}; //0:rawhist0 1:rawhist3

    JudgeWinLocation39(&isp_params->rawae0.win, &AeWinSplitMode[0], pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);
    JudgeWinLocation39(&isp_params->rawae3.win, &AeWinSplitMode[1], pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);

    JudgeWinLocation39(&isp_params->rawhist0.win, &HistWinSplitMode[0], pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);
    JudgeWinLocation39(&isp_params->rawhist3.win, &HistWinSplitMode[1], pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);

    // ae_stats = (ae_ori_stats_u12/10 - ob_offset_u9 - bls1_val_u12) * awb1_gain_u16 * range_ratio
    struct isp32_bls_cfg* bls_cfg           = &pStatsTrans->_ispParams->bls_cfg;
    struct isp32_awb_gain_cfg* awb_gain_cfg = &pStatsTrans->_ispParams->awb_gain_cfg;

    struct isp2x_bls_fixed_val bls1_ori_val;
    struct isp2x_bls_fixed_val bls1_val;  // bls1_val = blc1_ori_val * awb * range_ratio
    struct isp2x_bls_fixed_val awb1_gain;

    u16 isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain;
    u32 pixel_num      = 0;
    u8  rawhist_mode   = 0;
    bool is_hdr = (pStatsTrans->mWorkingMode > 0) ? true : false;
    bool is_bls1_en = bls_cfg->bls1_en && !is_hdr;

    isp_ob_offset_rb = bls_cfg->isp_ob_offset >> 2;
    isp_ob_offset_g  = bls_cfg->isp_ob_offset;
    isp_ob_predgain =  MAX(bls_cfg->isp_ob_predgain >> 8, 1);

    if (is_bls1_en) {
        bls1_ori_val.r  = (bls_cfg->bls1_val.r / isp_ob_predgain) >> 2;
        bls1_ori_val.gr = bls_cfg->bls1_val.gr / isp_ob_predgain;
        bls1_ori_val.gb = bls_cfg->bls1_val.gb / isp_ob_predgain;
        bls1_ori_val.b  = (bls_cfg->bls1_val.b / isp_ob_predgain) >> 2;
    } else {
        bls1_ori_val.r  = 0;
        bls1_ori_val.gr = 0;
        bls1_ori_val.gb = 0;
        bls1_ori_val.b  = 0;
    }

    //awb1_gain have adapted to the range of bls1_lvl
    awb1_gain.r = MAX(256, awb_gain_cfg->awb1_gain_r);
    awb1_gain.gr = MAX(256, awb_gain_cfg->awb1_gain_gr);
    awb1_gain.gb = MAX(256, awb_gain_cfg->awb1_gain_gb);
    awb1_gain.b = MAX(256, awb_gain_cfg->awb1_gain_b);

#ifdef AE_STATS_DEBUG
    LOGE("bls1[%d-%d-%d-%d]", bls1_ori_val.r, bls1_ori_val.gr, bls1_ori_val.gb, bls1_ori_val.b);
    LOGE("isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain [%d-%d-%d]",
         isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain);
    LOGE("awb1_gain[%d-%d-%d-%d]", awb1_gain.r, awb1_gain.gr, awb1_gain.gb, awb1_gain.b);
#endif

    // bls1_val = (bls1_ori_val + ob) * awb * range_ratio
    bls1_val.r = ((isp_ob_offset_rb + bls1_ori_val.r) * awb1_gain.r + 128) / 256;
    bls1_val.gr = ((isp_ob_offset_g + bls1_ori_val.gr) * awb1_gain.gr + 128) / 256;
    bls1_val.gb = ((isp_ob_offset_g + bls1_ori_val.gb) * awb1_gain.gb + 128) / 256;
    bls1_val.b = ((isp_ob_offset_rb + bls1_ori_val.b) * awb1_gain.b + 128) / 256;

    aiq_ae_stats_wrapper_t* statsInt = (aiq_ae_stats_wrapper_t*)to->_data;

    struct rkisp39_stat_buffer* left_stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t bufLen = AiqV4l2Buffer_getV4lBufLength((AiqV4l2Buffer_t*)from);
    struct rkisp39_stat_buffer* right_stats = (struct rkisp39_stat_buffer*)((char*)left_stats + bufLen / 2);

    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  left_stats->frame_id, left_stats->meas_type);

    if(left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
        return XCAM_RETURN_ERROR_PARAM;
    }

    AiqIrisInfoWrapper_t irisParamsTmp;
    AiqIrisInfoWrapper_t* irisParams = NULL;
    if (pStatsTrans->mIrishw) {
        XCamReturn ret = AiqLensHw_getIrisInfoParams(pStatsTrans->mIrishw, to->frame_id, &irisParamsTmp);
        if (ret == XCAM_RETURN_NO_ERROR) irisParams = &irisParamsTmp;
    }

    if (AeSwapMode == AEC_RAWSWAP_MODE_S_LITE) {
        meas_type = ((left_stats->meas_type >> 7) & (0x01)) & ((left_stats->meas_type >> 11) & (0x01))\
                    & ((right_stats->meas_type >> 7) & (0x01)) & ((right_stats->meas_type >> 11) & (0x01));
    } else {
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    to->bValid = (meas_type & 0x01) ? true : false;
    if (!to->bValid) {
        LOGE("aec_stats_valid=false,meas_type=0x%x", left_stats->meas_type);
        return XCAM_RETURN_BYPASS;
    }

    // 1.0) RAWAE0 RAWHIST0
    statsInt->aec_stats_v25.ae_data.hw_ae_entityGroup_mode = pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode;

    if(pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
            pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity0_wkOnly_mode) {

        //RAWAE0 BIG-MODE
        if (AeWinSplitMode[0] != LEFT_MODE)
            MergeAecBigWinStatsV39(&left_stats->stat.rawae0, &right_stats->stat.rawae0, &left_stats->stat.rawae0, AeWinSplitMode[0]);

        //RAWAE0 BIG-MODE
        calcAecBigWinStatsV39(&left_stats->stat.rawae0, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin,
                              bls1_val, awb1_gain);

        //RAWHIST0 BIG-MODE
        rawhist_mode = isp_params->rawhist0.mode;
        MergeAecHistBinStats(left_stats->stat.rawhist0.bin,
                             right_stats->stat.rawhist0.bin,
                             statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val,
                             HistWinSplitMode[0], rawhist_mode,
                             isp_ob_offset_rb, isp_ob_offset_g,
                             bls1_ori_val, awb1_gain, is_hdr);
    }

    // 2.0) RAWAE3 RAWHIST3
    if(pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
            pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity3_wkOnly_mode) {

        switch (AeSelMode) {
        case AEC_RAWSEL_MODE_CHN_0:
        case AEC_RAWSEL_MODE_CHN_1:
            break;

        case AEC_RAWSEL_MODE_TMO:

            bls1_val.r = 0;
            bls1_val.gr = 0;
            bls1_val.gb = 0;
            bls1_val.b = 0;

            awb1_gain.r = 256;
            awb1_gain.gr = 256;
            awb1_gain.gb = 256;
            awb1_gain.b = 256;

            break;

        default:
            LOGE("wrong AeSelMode=%d\n", AeSelMode);
            return XCAM_RETURN_ERROR_PARAM;
        }

        if (AeWinSplitMode[0] != LEFT_MODE)
            MergeAecBigWinStatsV39(&left_stats->stat.rawae3, &right_stats->stat.rawae3, &left_stats->stat.rawae3, AeWinSplitMode[0]);

        calcAecBigWinStatsV39(&left_stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin,
                              bls1_val, awb1_gain);


        rawhist_mode = isp_params->rawhist3.mode;
        MergeAecHistBinStats(left_stats->stat.rawhist3.bin,
                             right_stats->stat.rawhist3.bin,
                             statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val,
                             HistWinSplitMode[1], rawhist_mode,
                             isp_ob_offset_rb, isp_ob_offset_g,
                             bls1_ori_val, awb1_gain, is_hdr);
    }

    if (Bnr20bitEn > 0 && pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

        if (AeWinSplitMode[0] != LEFT_MODE) {
            MergeAecBigWinStatsV39(&left_stats->stat.rawae0, &right_stats->stat.rawae0, &left_stats->stat.rawae0, AeWinSplitMode[0]);
            MergeAecBigWinStatsV39(&left_stats->stat.rawae3, &right_stats->stat.rawae3, &left_stats->stat.rawae3, AeWinSplitMode[0]);
        }
        Aec20BitGirdMerge(left_stats->stat.rawae0, left_stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin);

        u32* bin = (u32*)calloc(ISP3X_HIST_BIN_N_MAX, sizeof(u32));
        if (AeWinSplitMode[0] != LEFT_MODE) {
            MergeAecHistBinStats(left_stats->stat.rawhist0.bin, right_stats->stat.rawhist0.bin,
                                 left_stats->stat.rawhist0.bin, AeWinSplitMode[0],
                                 rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                 bls1_ori_val, awb1_gain, true);
            MergeAecHistBinStats(left_stats->stat.rawhist3.bin, right_stats->stat.rawhist3.bin,
                                 left_stats->stat.rawhist3.bin, AeWinSplitMode[0],
                                 rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                 bls1_ori_val, awb1_gain, true);
        }

        Aec20BitHistMerge(left_stats->stat.rawhist0.bin, left_stats->stat.rawhist3.bin, bin);

        for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
            statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerR_val[i] =
                CLIP((int)(statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerR_val[i] * awb1_gain.r / 256 - bls1_val.r), 0, MAX_10BITS);
            statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerGrGb_val[i] =
                CLIP((int)(statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerGrGb_val[i] / 256 - bls1_val.gr), 0, MAX_12BITS);
            statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerB_val[i] =
                CLIP((int)(statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerB_val[i] * awb1_gain.b / 256 - bls1_val.b), 0, MAX_10BITS);
        }

        rawhist_mode = pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.coWkEntity03.hist.hw_aeCfg_pix_mode;
        calcAecHistBinStatsV39(bin, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val, \
                               rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g, bls1_ori_val, awb1_gain, is_hdr);
        if(bin) free(bin);

    }

#ifdef AE_STATS_DEBUG

    if(Bnr20bitEn) {

        if(pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 15; j++) {
                    printf("coWkEnt03[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                           statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                           statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                           statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
                }
            }

            printf("====================hist_result========================\n");

            for (int i = 0; i < 256; i++)
                printf("coWkEnt03 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.hist.hw_ae_histBin_val[i]);
        } else {
            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 15; j++) {
                    printf("entity0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
                }
            }

            printf("====================hist_result========================\n");

            for (int i = 0; i < 256; i++)
                printf("entity0 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i]);

        }

    } else {

        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                printf("entity0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
            }
        }

        printf("====================hist_result========================\n");

        for (int i = 0; i < 256; i++)
            printf("entity0 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i]);

        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                printf("entity3[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
            }
        }

        printf("====================hist_result========================\n");

        for (int i = 0; i < 256; i++)
            printf("entity3 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val[i]);

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
    if (pStatsTrans->_expParams) {
        statsInt->aec_stats_v25.ae_exp = pStatsTrans->_expParams->aecExpInfo;
        /*printf("frame[%d],gain=%d,time=%d\n", stats->frame_id,
               expParams->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
               expParams->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);*/

        /*
         * printf("%s: L: [0x%x-0x%x], M: [0x%x-0x%x], S: [0x%x-0x%x]\n",
         *        __func__,
         *        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
         *        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
         *        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global);
         */
    }

    // iris params
    if (irisParams) {
        float sof_time   = (float)irisParams->sofTime / 1000000000.0f;
        float start_time = (float)irisParams->PIris.StartTim.tv_sec +
                           (float)irisParams->PIris.StartTim.tv_usec / 1000000.0f;
        float end_time = (float)irisParams->PIris.EndTim.tv_sec +
                         (float)irisParams->PIris.EndTim.tv_usec / 1000000.0f;
        float frm_intval = 1 / (statsInt->aec_stats_v25.ae_exp.pixel_clock_freq_mhz * 1000000.0f /
                                (float)statsInt->aec_stats_v25.ae_exp.line_length_pixels /
                                (float)statsInt->aec_stats_v25.ae_exp.frame_length_lines);

        /*printf("%s: step=%d,last-step=%d,start-tim=%f,end-tim=%f,sof_tim=%f\n",
            __func__,
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step,
            irisParams->data()->PIris.laststep,start_time,end_time,sof_time);
        */

        if (sof_time < end_time + frm_intval)
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step = irisParams->PIris.laststep;
        else
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step = irisParams->PIris.step;
    }

    to->frame_id = left_stats->frame_id;
#endif
    return ret;

}

XCamReturn translateAecStatsV39(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)

    if (pStatsTrans->mIsMultiIsp && pStatsTrans->mIspUniteMode)
        return translateMultiAecStatsV39(pStatsTrans, from, to);

    // 0) blc awb cfg
    struct isp39_isp_meas_cfg* isp_params = &pStatsTrans->_ispParams->meas;
    uint8_t AeSwapMode, AeSelMode, Bnr20bitEn;
    AeSwapMode             = isp_params->rawae0.rawae_sel & 0x0f;
    AeSelMode              = isp_params->rawae3.rawae_sel & 0x0f;
    Bnr20bitEn             = isp_params->rawae0.rawae_sel & 0xf0;
    unsigned int meas_type = 0;

    // ae_stats = (ae_ori_stats_u12/10 - ob_offset_u9 - bls1_val_u12) * awb1_gain_u16 * range_ratio
    struct isp32_bls_cfg* bls_cfg           = &pStatsTrans->_ispParams->bls_cfg;
    struct isp32_awb_gain_cfg* awb_gain_cfg = &pStatsTrans->_ispParams->awb_gain_cfg;

    struct isp2x_bls_fixed_val bls1_ori_val;
    struct isp2x_bls_fixed_val bls1_val;  // bls1_val = blc1_ori_val * awb * range_ratio
    struct isp2x_bls_fixed_val awb1_gain;

    u16 isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain;
    u32 pixel_num      = 0;
    u8  rawhist_mode   = 0;
    bool is_hdr = ((pStatsTrans->mWorkingMode > 0) ? true : false) | (Bnr20bitEn > 0 ? true : false);
    bool is_bls1_en = bls_cfg->bls1_en && !is_hdr;

    isp_ob_offset_rb = bls_cfg->isp_ob_offset >> 2;
    isp_ob_offset_g  = bls_cfg->isp_ob_offset;
    isp_ob_predgain =  MAX(bls_cfg->isp_ob_predgain >> 8, 1);

    if (is_bls1_en) {
        bls1_ori_val.r  = (bls_cfg->bls1_val.r / isp_ob_predgain) >> 2;
        bls1_ori_val.gr = bls_cfg->bls1_val.gr / isp_ob_predgain;
        bls1_ori_val.gb = bls_cfg->bls1_val.gb / isp_ob_predgain;
        bls1_ori_val.b  = (bls_cfg->bls1_val.b / isp_ob_predgain) >> 2;
    } else {
        bls1_ori_val.r  = 0;
        bls1_ori_val.gr = 0;
        bls1_ori_val.gb = 0;
        bls1_ori_val.b  = 0;
    }

    //awb1_gain have adapted to the range of bls1_lvl
    awb1_gain.r = MAX(256, awb_gain_cfg->awb1_gain_r);
    awb1_gain.gr = MAX(256, awb_gain_cfg->awb1_gain_gr);
    awb1_gain.gb = MAX(256, awb_gain_cfg->awb1_gain_gb);
    awb1_gain.b = MAX(256, awb_gain_cfg->awb1_gain_b);

#ifdef AE_STATS_DEBUG
    LOGE("bls1[%d-%d-%d-%d]", bls1_ori_val.r, bls1_ori_val.gr, bls1_ori_val.gb, bls1_ori_val.b);
    LOGE("isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain [%d-%d-%d]",
         isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain);
    LOGE("awb1_gain[%d-%d-%d-%d]", awb1_gain.r, awb1_gain.gr, awb1_gain.gb, awb1_gain.b);
#endif

    // bls1_val = (bls1_ori_val + ob) * awb * range_ratio
    bls1_val.r = ((isp_ob_offset_rb + bls1_ori_val.r) * awb1_gain.r + 128) / 256;
    bls1_val.gr = ((isp_ob_offset_g + bls1_ori_val.gr) * awb1_gain.gr + 128) / 256;
    bls1_val.gb = ((isp_ob_offset_g + bls1_ori_val.gb) * awb1_gain.gb + 128) / 256;
    bls1_val.b = ((isp_ob_offset_rb + bls1_ori_val.b) * awb1_gain.b + 128) / 256;

    aiq_ae_stats_wrapper_t* statsInt = (aiq_ae_stats_wrapper_t*)to->_data;
    struct rkisp39_stat_buffer* stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  stats->frame_id, stats->meas_type);

    AiqIrisInfoWrapper_t irisParamsTmp;
    AiqIrisInfoWrapper_t* irisParams = NULL;
    if (pStatsTrans->mIrishw) {
        XCamReturn ret = AiqLensHw_getIrisInfoParams(pStatsTrans->mIrishw, stats->frame_id, &irisParamsTmp);
        if (ret == XCAM_RETURN_NO_ERROR) irisParams = &irisParamsTmp;
    }

    // ae stats v3.9
    to->frame_id = stats->frame_id;

    if (AeSwapMode == AEC_RAWSWAP_MODE_S_LITE) {
        meas_type = ((stats->meas_type >> 7) & (0x01)) & ((stats->meas_type >> 11) & (0x01));
    } else {
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    to->bValid = (meas_type & 0x01) ? true : false;
    if (!to->bValid) {
        LOGE("aec_stats_valid=false,meas_type=0x%x", stats->meas_type);
        return XCAM_RETURN_BYPASS;
    }

    statsInt->aec_stats_v25.ae_data.hw_ae_entityGroup_mode = pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode;

    if(pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
            pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity0_wkOnly_mode) {

        //RAWAE0 BIG-MODE
        calcAecBigWinStatsV39(&stats->stat.rawae0, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin,
                              bls1_val, awb1_gain);
        //RAWHIST0 BIG-MODE
        rawhist_mode = isp_params->rawhist0.mode;

        calcAecHistBinStatsV39(stats->stat.rawhist0.bin,
                               statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val,
                               rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                               bls1_ori_val, awb1_gain, is_hdr);
    }

    if(pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
            pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity3_wkOnly_mode) {

        switch (AeSelMode) {
        case AEC_RAWSEL_MODE_CHN_0:
        case AEC_RAWSEL_MODE_CHN_1:

            //RAWAE3 BIG-MODE
            calcAecBigWinStatsV39(&stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin,
                                  bls1_val, awb1_gain);
            //RAWHIST3 BIG-MODE
            rawhist_mode = isp_params->rawhist3.mode;
            calcAecHistBinStatsV39(stats->stat.rawhist3.bin,
                                   statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val,
                                   rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                   bls1_ori_val, awb1_gain, is_hdr);
            break;
        case AEC_RAWSEL_MODE_TMO:

            bls1_val.r = 0;
            bls1_val.gr = 0;
            bls1_val.gb = 0;
            bls1_val.b = 0;

            awb1_gain.r = 256;
            awb1_gain.gr = 256;
            awb1_gain.gb = 256;
            awb1_gain.b = 256;

            calcAecBigWinStatsV39(&stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin,
                                  bls1_val, awb1_gain);

            memcpy(statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val,
                   stats->stat.rawhist3.bin, ISP39_HIST_BIN_N_MAX * sizeof(u32));

            break;

        default:
            LOGE("wrong AeSelMode=%d\n", AeSelMode);
            return XCAM_RETURN_ERROR_PARAM;
        }
    }


    if(Bnr20bitEn > 0 && pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

        Aec20BitGirdMerge(stats->stat.rawae0, stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin);
        u32* bin = (u32*)aiq_mallocz(ISP3X_HIST_BIN_N_MAX * sizeof(u32));
        Aec20BitHistMerge(stats->stat.rawhist0.bin, stats->stat.rawhist3.bin, bin);

        for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
            statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerR_val[i] = \
                    CLIP((int)(statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerR_val[i] * awb1_gain.r / 256 - bls1_val.r), 0, MAX_10BITS);
            statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerGrGb_val[i] = \
                    CLIP((int)(statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerGrGb_val[i] / 256 - bls1_val.gr), 0, MAX_12BITS);
            statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerB_val[i] = \
                    CLIP((int)(statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerB_val[i] * awb1_gain.b / 256 - bls1_val.b), 0, MAX_10BITS);
        }

        rawhist_mode = pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.coWkEntity03.hist.hw_aeCfg_pix_mode;
        calcAecHistBinStatsV39(bin, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val, \
                               rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g, bls1_ori_val, awb1_gain, is_hdr);
        if(bin) free(bin);
    }


#ifdef AE_STATS_DEBUG

    if(Bnr20bitEn) {

        if(pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 15; j++) {
                    printf("coWkEnt03[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                           statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                           statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                           statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
                }
            }

            printf("====================hist_result========================\n");

            for (int i = 0; i < 256; i++)
                printf("coWkEnt03 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.coWkEnt03.hist.hw_ae_histBin_val[i]);
        } else {
            for (int i = 0; i < 15; i++) {
                for (int j = 0; j < 15; j++) {
                    printf("entity0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
                }
            }

            printf("====================hist_result========================\n");

            for (int i = 0; i < 256; i++)
                printf("entity0 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i]);

        }

    } else {

        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                printf("entity0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
            }
        }

        printf("====================hist_result========================\n");

        for (int i = 0; i < 256; i++)
            printf("entity0 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i]);

        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                printf("entity3[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                       statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
            }
        }

        printf("====================hist_result========================\n");

        for (int i = 0; i < 256; i++)
            printf("entity3 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val[i]);

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
    if (pStatsTrans->_expParams) {
        statsInt->aec_stats_v25.ae_exp = pStatsTrans->_expParams->aecExpInfo;
        /*printf("frame[%d],gain=%d,time=%d\n", stats->frame_id,
               expParams->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
               expParams->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);*/

        /*
         * printf("%s: L: [0x%x-0x%x], M: [0x%x-0x%x], S: [0x%x-0x%x]\n",
         *        __func__,
         *        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
         *        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
         *        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global);
         */
    }

    // iris params
    if (irisParams) {
        float sof_time   = (float)irisParams->sofTime / 1000000000.0f;
        float start_time = (float)irisParams->PIris.StartTim.tv_sec +
                           (float)irisParams->PIris.StartTim.tv_usec / 1000000.0f;
        float end_time = (float)irisParams->PIris.EndTim.tv_sec +
                         (float)irisParams->PIris.EndTim.tv_usec / 1000000.0f;
        float frm_intval = 1 / (statsInt->aec_stats_v25.ae_exp.pixel_clock_freq_mhz * 1000000.0f /
                                (float)statsInt->aec_stats_v25.ae_exp.line_length_pixels /
                                (float)statsInt->aec_stats_v25.ae_exp.frame_length_lines);

        /*printf("%s: step=%d,last-step=%d,start-tim=%f,end-tim=%f,sof_tim=%f\n",
            __func__,
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step,
            irisParams->data()->PIris.laststep,start_time,end_time,sof_time);
        */

        if (sof_time < end_time + frm_intval)
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step = irisParams->PIris.laststep;
        else
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step = irisParams->PIris.step;
    }

    to->frame_id = stats->frame_id;
#endif

    return ret;
}

#else
/* for bnr20bit, rawae0 meas LSB(0-9bit) rawae3 meas MSB(10-19bit) */
/* for bnr20bit, rawhist0 meas LSB(4-11bit) rawhist3 meas MSB(12-19bit) */

XCamReturn Aec20BitGirdClip(u16* LSB, u16* MSB, int bit_num) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // need to use LSB 12BIT data as long frame

    if (bit_num == 10) {  // r b channel

        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            if (MSB[i] > 0b11)
                LSB[i] = (1 << bit_num) - 1;
            else if (0 < MSB[i] && MSB[i] <= 0b11)
                LSB[i] = (LSB[i] | (MSB[i] << bit_num)) >> 2;
            else
                LSB[i] = LSB[i] >> 2;
        }

    } else if (bit_num == 12) {  // g channel

        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            if (MSB[i] > 0b1100)
                LSB[i] = (1 << bit_num) - 1;
            else if (0 < MSB[i] && MSB[i] <= 0b1100)
                LSB[i] = (LSB[i] >> 2 | (MSB[i] << 8));
            else
                LSB[i] = LSB[i] >> 2;
        }
    }

    return ret;
}

static void calcAecBigWinStatsV39(struct isp39_rawae_stat* stats_in, rawaebig_stat_t* stats_out,
                                  uint16_t* raw_mean, unsigned char* weight, int8_t stats_chn_sel,
                                  int8_t y_range_mode, struct isp2x_bls_fixed_val bls1_val,
                                  struct isp2x_bls_fixed_val awb1_gain) {
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
        off = 16;  // 8bit
    }

    switch (stats_chn_sel) {
    case RAWSTATS_CHN_Y_EN:
        for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channely_xy[i] = CLIP(round(
                                                 rcc * (float)((stats_in->blk_y[i / 15].blk_x[i % 15].r * awb1_gain.r / 256 -
                                                         bls1_val.r) >> 2) +
                                                 gcc * (float)((stats_in->blk_y[i / 15].blk_x[i % 15].g * awb1_gain.gr / 256 -
                                                         bls1_val.gr) >> 4) +
                                                 bcc * (float)((stats_in->blk_y[i / 15].blk_x[i % 15].b * awb1_gain.b / 256 -
                                                         bls1_val.b) >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_R_EN:
        for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].r * awb1_gain.r / 256 - bls1_val.r), 0, MAX_10BITS);

            sum_xy += ((stats_out->channelr_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_G_EN:
        for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelg_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].g * awb1_gain.gr / 256 - bls1_val.gr), 0, MAX_12BITS);
            sum_xy += ((stats_out->channelg_xy[i] >> 4) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_B_EN:
        for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelb_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].b * awb1_gain.b / 256 - bls1_val.b), 0, MAX_10BITS);
            sum_xy += ((stats_out->channelb_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_RGB_EN:
        for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].r * awb1_gain.r / 256 - bls1_val.r), 0, MAX_10BITS);
            stats_out->channelg_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].g * awb1_gain.gr / 256 - bls1_val.gr), 0, MAX_12BITS);
            stats_out->channelb_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].b * awb1_gain.b / 256 - bls1_val.b), 0, MAX_10BITS);
        }
        break;

    case RAWSTATS_CHN_ALL_EN:
    default:
        for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].r * awb1_gain.r / 256 - bls1_val.r), 0, MAX_10BITS);
            stats_out->channelg_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].g * awb1_gain.gr / 256 - bls1_val.gr), 0, MAX_12BITS);
            stats_out->channelb_xy[i] =
                CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].b * awb1_gain.b / 256 - bls1_val.b), 0, MAX_10BITS);
            stats_out->channely_xy[i] =
                CLIP(round(rcc * (float)(stats_out->channelr_xy[i] >> 2) +
                           gcc * (float)(stats_out->channelg_xy[i] >> 4) +
                           bcc * (float)(stats_out->channelb_xy[i] >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;
    }

    // For isp39, no sub win
}

static void calcAecHistBinStatsV39(u32* hist_bin_in, u32* hist_bin_out, u8 hist_mode,
                                   u16 ob_offset_rb, u16 ob_offset_g,
                                   struct isp2x_bls_fixed_val bls1_val,
                                   struct isp2x_bls_fixed_val awb1_gain, bool is_hdr) {
    if (!is_hdr) {
        memset(hist_bin_out, 0, ISP39_HIST_BIN_N_MAX * sizeof(u32));

        s16 bls1, bls_part;
        u16 ob_part, awb_part, div_part, round_part;
        int tmp;

        switch (hist_mode) {
        case 2:
            ob_part    = ob_offset_rb >> 2;
            bls1       = bls1_val.r >> 2;
            awb_part   = awb1_gain.r;
            bls_part   = (ob_part + bls1) * awb_part;
            div_part   = 256;
            round_part = 128;
            break;
        case 3:
            ob_part    = ob_offset_g >> 4;
            bls1       = bls1_val.gr >> 4;
            awb_part   = awb1_gain.gr;
            bls_part   = (ob_part + bls1) * awb_part;
            div_part   = 256;
            round_part = 128;
            break;
        case 4:
            ob_part    = ob_offset_rb >> 2;
            bls1       = bls1_val.b >> 2;
            awb_part   = awb1_gain.b;
            bls_part   = (ob_part + bls1) * awb_part;
            div_part   = 256;
            round_part = 128;
            break;
        case 5:
        default:
            ob_part = (u16)((ob_offset_g >> 4) * 587 + (ob_offset_rb >> 2) * 299 +
                            (ob_offset_rb >> 2) * 114 + 500) / 1000;
            bls1 = (s16)((bls1_val.gr >> 4) * 587 + (bls1_val.r >> 2) * 299 +
                         (bls1_val.b >> 2) * 114 + 500) / 1000;
            awb_part   = 100;
            bls_part   = (ob_part + bls1) * awb_part;
            div_part   = 7655 / awb1_gain.r + 15027 / awb1_gain.gr + 2919 / awb1_gain.b;
            round_part = div_part / 2;
            break;
        }

        for (int i = 0; i < ISP39_HIST_BIN_N_MAX; i++) {
            tmp = (i - ob_part - bls1 > 0) ? (i * awb_part - bls_part + round_part) / div_part : 0;
            tmp = (tmp > ISP39_HIST_BIN_N_MAX - 1) ? (ISP39_HIST_BIN_N_MAX - 1) : tmp;
            hist_bin_out[tmp] += hist_bin_in[i];
        }

    } else {
        memcpy(hist_bin_out, hist_bin_in, ISP39_HIST_BIN_N_MAX * sizeof(u32));
    }
}

XCamReturn translateAecStatsV39(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)

    // 0) blc awb cfg
    if (!pStatsTrans->_ispParams) {
        return XCAM_RETURN_ERROR_FAILED;
    }

    struct isp39_isp_meas_cfg* isp_params = &pStatsTrans->_ispParams->meas;
    uint8_t AeSwapMode, AeSelMode, Bnr20bitEn;
    AeSwapMode             = isp_params->rawae0.rawae_sel & 0x0f;
    AeSelMode              = isp_params->rawae3.rawae_sel & 0x0f;
    Bnr20bitEn             = isp_params->rawae0.rawae_sel & 0xf0;
    unsigned int meas_type = 0;

    // ae_stats = (ae_ori_stats_u12/10 - ob_offset_u9 - bls1_val_u12) * awb1_gain_u16 * range_ratio
    struct isp32_bls_cfg* bls_cfg           = &pStatsTrans->_ispParams->bls_cfg;
    struct isp32_awb_gain_cfg* awb_gain_cfg = &pStatsTrans->_ispParams->awb_gain_cfg;

    struct isp2x_bls_fixed_val bls1_ori_val;
    struct isp2x_bls_fixed_val bls1_val;  // bls1_val = blc1_ori_val * awb * range_ratio
    struct isp2x_bls_fixed_val awb1_gain;

    u16 isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain;
    u32 pixel_num   = 0;
    u8 rawhist_mode = 0;
    bool is_hdr     = ((pStatsTrans->mWorkingMode > 0) ? true : false) | (Bnr20bitEn > 0 ? true : false);
    bool is_bls1_en = bls_cfg->bls1_en && !is_hdr;

    isp_ob_offset_rb = bls_cfg->isp_ob_offset >> 2;
    isp_ob_offset_g  = bls_cfg->isp_ob_offset;
    isp_ob_predgain  = MAX(bls_cfg->isp_ob_predgain >> 8, 1);

    if (is_bls1_en) {
        bls1_ori_val.r  = (bls_cfg->bls1_val.r / isp_ob_predgain) >> 2;
        bls1_ori_val.gr = bls_cfg->bls1_val.gr / isp_ob_predgain;
        bls1_ori_val.gb = bls_cfg->bls1_val.gb / isp_ob_predgain;
        bls1_ori_val.b  = (bls_cfg->bls1_val.b / isp_ob_predgain) >> 2;
    } else {
        bls1_ori_val.r  = 0;
        bls1_ori_val.gr = 0;
        bls1_ori_val.gb = 0;
        bls1_ori_val.b  = 0;
    }

    // awb1_gain have adapted to the range of bls1_lvl
    awb1_gain.r  = MAX(256, awb_gain_cfg->awb1_gain_r);
    awb1_gain.gr = MAX(256, awb_gain_cfg->awb1_gain_gr);
    awb1_gain.gb = MAX(256, awb_gain_cfg->awb1_gain_gb);
    awb1_gain.b  = MAX(256, awb_gain_cfg->awb1_gain_b);

#ifdef AE_STATS_DEBUG
    LOGE("bls1[%d-%d-%d-%d]", bls1_ori_val.r, bls1_ori_val.gr, bls1_ori_val.gb, bls1_ori_val.b);
    LOGE("isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain [%d-%d-%d]", isp_ob_offset_rb,
         isp_ob_offset_g, isp_ob_predgain);
    LOGE("awb1_gain[%d-%d-%d-%d]", awb1_gain.r, awb1_gain.gr, awb1_gain.gb, awb1_gain.b);

    pStatsTrans->_aeAlgoStatsCfg.UpdateStats    = true;
    pStatsTrans->_aeAlgoStatsCfg.RawStatsChnSel = RAWSTATS_CHN_ALL_EN;
#endif

    // bls1_val = (bls1_ori_val + ob) * awb * range_ratio
    bls1_val.r  = ((isp_ob_offset_rb + bls1_ori_val.r) * awb1_gain.r + 128) / 256;
    bls1_val.gr = ((isp_ob_offset_g + bls1_ori_val.gr) * awb1_gain.gr + 128) / 256;
    bls1_val.gb = ((isp_ob_offset_g + bls1_ori_val.gb) * awb1_gain.gb + 128) / 256;
    bls1_val.b  = ((isp_ob_offset_rb + bls1_ori_val.b) * awb1_gain.b + 128) / 256;

    aiq_ae_stats_wrapper_t* statsInt = (aiq_ae_stats_wrapper_t*)to->_data;
    struct rkisp39_stat_buffer* stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  stats->frame_id, stats->meas_type);

    AiqIrisInfoWrapper_t irisParamsTmp;
    AiqIrisInfoWrapper_t* irisParams = NULL;
    if (pStatsTrans->mIrishw) {
        XCamReturn ret = AiqLensHw_getIrisInfoParams(pStatsTrans->mIrishw, stats->frame_id, &irisParamsTmp);
        if (ret == XCAM_RETURN_NO_ERROR) irisParams = &irisParamsTmp;
    }
    // ae stats v3.9
    to->frame_id = stats->frame_id;

    if (AeSwapMode == AEC_RAWSWAP_MODE_S_LITE) {
        meas_type    = ((stats->meas_type >> 7) & (0x01)) & ((stats->meas_type >> 11) & (0x01));
        rawhist_mode = isp_params->rawhist0.mode;
    } else {
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    to->bValid = (meas_type & 0x01) ? true : false;
    if (!to->bValid) return XCAM_RETURN_BYPASS;

    // calc ae stats run flag
    uint64_t SumHistPix[2] = {0, 0};
    uint64_t SumHistBin[2] = {0, 0};
    uint16_t HistMean[2]   = {0, 0};
    u32* hist_bin[2];

    hist_bin[0] = stats->stat.rawhist0.bin;
    hist_bin[1] = stats->stat.rawhist3.bin;
    /*if (AeSelMode <= AEC_RAWSEL_MODE_CHN_0) {
        hist_bin[0] = stats->stat.rawhist3.bin;
    }*/

    for (int i = 0; i < ISP32_HIST_BIN_N_MAX; i++) {
        SumHistPix[0] += hist_bin[0][i];
        SumHistBin[0] += (float)(hist_bin[0][i] * (i + 1));

        SumHistPix[1] += hist_bin[1][i];
        SumHistBin[1] += (float)(hist_bin[1][i] * (i + 1));
    }
    HistMean[0]   = (uint16_t)(SumHistBin[0] / MAX(SumHistPix[0], 1));
    HistMean[1]   = (uint16_t)(SumHistBin[1] / MAX(SumHistPix[1], 1));
    bool run_flag = AiqStatsTranslator_getAeStatsRunFlag(pStatsTrans, HistMean);
    run_flag |= pStatsTrans->_aeAlgoStatsCfg.UpdateStats;

    if (run_flag) {
        // RAWAE0 BIG-MODE
        calcAecBigWinStatsV39(&stats->stat.rawae0, &statsInt->aec_stats.ae_data.chn[0].rawae_big,
                              &statsInt->aec_stats.ae_data.raw_mean[0],
                              pStatsTrans->_aeAlgoStatsCfg.BigWeight,
                              pStatsTrans->_aeAlgoStatsCfg.RawStatsChnSel,
                              pStatsTrans->_aeAlgoStatsCfg.YRangeMode, bls1_val, awb1_gain);
        // RAWHIST0 BIG-MODE
        calcAecHistBinStatsV39(stats->stat.rawhist0.bin,
                               statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins, rawhist_mode,
                               isp_ob_offset_rb, isp_ob_offset_g, bls1_ori_val, awb1_gain, is_hdr);

        // AE-BIG (RAWAE3)
        if (Bnr20bitEn > 0) AeSelMode = AEC_RAWSEL_MODE_TMO;

        switch (AeSelMode) {
        case AEC_RAWSEL_MODE_CHN_0:
        case AEC_RAWSEL_MODE_CHN_1:

            // RAWAE3 BIG-MODE
            calcAecBigWinStatsV39(&stats->stat.rawae3,
                                  &statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big,
                                  &statsInt->aec_stats.ae_data.raw_mean[AeSelMode],
                                  pStatsTrans->_aeAlgoStatsCfg.BigWeight,
                                  pStatsTrans->_aeAlgoStatsCfg.RawStatsChnSel,
                                  pStatsTrans->_aeAlgoStatsCfg.YRangeMode, bls1_val, awb1_gain);
            // RAWHIST3 BIG-MODE
            calcAecHistBinStatsV39(stats->stat.rawhist3.bin,
                                   statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins,
                                   rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                   bls1_ori_val, awb1_gain, is_hdr);
            break;
        case AEC_RAWSEL_MODE_TMO:

            bls1_val.r  = 0;
            bls1_val.gr = 0;
            bls1_val.gb = 0;
            bls1_val.b  = 0;

            awb1_gain.r  = 256;
            awb1_gain.gr = 256;
            awb1_gain.gb = 256;
            awb1_gain.b  = 256;

            calcAecBigWinStatsV39(&stats->stat.rawae3,
                                  &statsInt->aec_stats.ae_data.extra.rawae_big,
                                  &statsInt->aec_stats.ae_data.raw_mean[AeSelMode],
                                  pStatsTrans->_aeAlgoStatsCfg.BigWeight,
                                  pStatsTrans->_aeAlgoStatsCfg.RawStatsChnSel,
                                  pStatsTrans->_aeAlgoStatsCfg.YRangeMode, bls1_val, awb1_gain);

            memcpy(statsInt->aec_stats.ae_data.extra.rawhist_big.bins, stats->stat.rawhist3.bin,
                   ISP39_HIST_BIN_N_MAX * sizeof(u32));

            break;

        default:
            LOGE("wrong AeSelMode=%d\n", AeSelMode);
            return XCAM_RETURN_ERROR_PARAM;
        }
        pStatsTrans->_lastAeStats = statsInt->aec_stats.ae_data;
    } else {
        statsInt->aec_stats.ae_data = pStatsTrans->_lastAeStats;
    }

#ifdef AE_STATS_DEBUG
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
        printf("chn0_subwin[%d]:sumr 0x%08" PRIx64 ", sumg 0x%08" PRIx64 ", sumb 0x%08" PRIx64 "\n",
               i, statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i],
               statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i],
               statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i]);

    printf("====================hist_result========================\n");

    for (int i = 0; i < 256; i++)
        printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins[i]);

    if (Bnr20bitEn) {
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 15; j++) {
                printf("extra[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats.ae_data.extra.rawae_big.channelr_xy[i * 15 + j],
                       statsInt->aec_stats.ae_data.extra.rawae_big.channelg_xy[i * 15 + j],
                       statsInt->aec_stats.ae_data.extra.rawae_big.channelb_xy[i * 15 + j]);
            }
        }
        printf("====================sub-win-result======================\n");

        for (int i = 0; i < 4; i++)
            printf("extra_subwin[%d]:sumr 0x%08" PRIx64 ", sumg 0x%08" PRIx64 ", sumb 0x%08" PRIx64
                   "\n",
                   i, statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumr[i],
                   statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumg[i],
                   statsInt->aec_stats.ae_data.extra.rawae_big.wndx_sumb[i]);

        printf("====================hist_result========================\n");

        for (int i = 0; i < 256; i++)
            printf("extra bin[%d]= 0x%08x\n", i,
                   statsInt->aec_stats.ae_data.extra.rawhist_big.bins[i]);
    }

#endif

    if (Bnr20bitEn > 0) {
        uint32_t sum_xy = 0, sum_weight = 0;
        float rcc = 0, gcc = 0, bcc = 0, off = 0;

        if (pStatsTrans->_aeAlgoStatsCfg.YRangeMode <= CAM_YRANGEV2_MODE_FULL) {
            rcc = 0.299;
            gcc = 0.587;
            bcc = 0.114;
            off = 0;
        } else {
            rcc = 0.25;
            gcc = 0.5;
            bcc = 0.1094;
            off = 16;  // 8bit
        }

        switch (pStatsTrans->_aeAlgoStatsCfg.RawStatsChnSel) {
        case RAWSTATS_CHN_R_EN:
            Aec20BitGirdClip(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy,
                             statsInt->aec_stats.ae_data.extra.rawae_big.channelr_xy, 10);
            for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
                sum_xy += ((statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] >> 2) *
                           pStatsTrans->_aeAlgoStatsCfg.BigWeight[i]);
                sum_weight += pStatsTrans->_aeAlgoStatsCfg.BigWeight[i];
            }
            statsInt->aec_stats.ae_data.raw_mean[0] =
                round(256.0f * (float)sum_xy / (float)sum_weight);
            break;

        case RAWSTATS_CHN_G_EN:
            Aec20BitGirdClip(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy,
                             statsInt->aec_stats.ae_data.extra.rawae_big.channelg_xy, 12);
            for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
                sum_xy += ((statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] >> 4) *
                           pStatsTrans->_aeAlgoStatsCfg.BigWeight[i]);
                sum_weight += pStatsTrans->_aeAlgoStatsCfg.BigWeight[i];
            }
            statsInt->aec_stats.ae_data.raw_mean[0] =
                round(256.0f * (float)sum_xy / (float)sum_weight);
            break;

        case RAWSTATS_CHN_B_EN:
            Aec20BitGirdClip(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy,
                             statsInt->aec_stats.ae_data.extra.rawae_big.channelb_xy, 10);
            for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
                sum_xy += ((statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] >> 2) *
                           pStatsTrans->_aeAlgoStatsCfg.BigWeight[i]);
                sum_weight += pStatsTrans->_aeAlgoStatsCfg.BigWeight[i];
            }
            statsInt->aec_stats.ae_data.raw_mean[0] =
                round(256.0f * (float)sum_xy / (float)sum_weight);
            break;

        case RAWSTATS_CHN_RGB_EN:
        case RAWSTATS_CHN_ALL_EN:
        case RAWSTATS_CHN_Y_EN:
        default:
            Aec20BitGirdClip(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy,
                             statsInt->aec_stats.ae_data.extra.rawae_big.channelr_xy, 10);
            Aec20BitGirdClip(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy,
                             statsInt->aec_stats.ae_data.extra.rawae_big.channelg_xy, 12);
            Aec20BitGirdClip(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy,
                             statsInt->aec_stats.ae_data.extra.rawae_big.channelb_xy, 10);
            for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
                statsInt->aec_stats.ae_data.chn[0].rawae_big.channely_xy[i] =
                    CLIP(round(rcc * (float)(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i] >> 2) +
                               gcc * (float)(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i] >> 4) +
                               bcc * (float)(statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i] >> 2) +
                               off), 0, MAX_8BITS);
                sum_xy += (statsInt->aec_stats.ae_data.chn[0].rawae_big.channely_xy[i] * pStatsTrans->_aeAlgoStatsCfg.BigWeight[i]);
                sum_weight += pStatsTrans->_aeAlgoStatsCfg.BigWeight[i];
            }
            statsInt->aec_stats.ae_data.raw_mean[0] = round(256.0f * (float)sum_xy / (float)sum_weight);
            break;
        }

        uint32_t sumpixel = 0;

        for (int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
            sumpixel += statsInt->aec_stats.ae_data.extra.rawhist_big.bins[i];
        }

        statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins[ISP3X_HIST_BIN_N_MAX - 1] += sumpixel;
    }

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
    if (pStatsTrans->_expParams) {
        statsInt->aec_stats.ae_exp = pStatsTrans->_expParams->aecExpInfo;
        /*printf("frame[%d],gain=%d,time=%d\n", stats->frame_id,
               expParams->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
               expParams->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);*/

        /*
         * printf("%s: L: [0x%x-0x%x], M: [0x%x-0x%x], S: [0x%x-0x%x]\n",
         *        __func__,
         *        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
         *        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
         *        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global);
         */
    }

    // iris params
    if (irisParams) {
        float sof_time   = (float)irisParams->sofTime / 1000000000.0f;
        float start_time = (float)irisParams->PIris.StartTim.tv_sec +
                           (float)irisParams->PIris.StartTim.tv_usec / 1000000.0f;
        float end_time = (float)irisParams->PIris.EndTim.tv_sec +
                         (float)irisParams->PIris.EndTim.tv_usec / 1000000.0f;
        float frm_intval = 1 / (statsInt->aec_stats.ae_exp.pixel_clock_freq_mhz * 1000000.0f /
                                (float)statsInt->aec_stats.ae_exp.line_length_pixels /
                                (float)statsInt->aec_stats.ae_exp.frame_length_lines);

        /*printf("%s: step=%d,last-step=%d,start-tim=%f,end-tim=%f,sof_tim=%f\n",
            __func__,
            statsInt->aec_stats.ae_exp.Iris.PIris.step,
            irisParams->PIris.laststep,start_time,end_time,sof_time);
        */

        if (sof_time < end_time + frm_intval)
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->PIris.laststep;
        else
            statsInt->aec_stats.ae_exp.Iris.PIris.step = irisParams->PIris.step;
    }

    to->frame_id = stats->frame_id;
#endif

    return ret;
}

#endif

XCamReturn translateMultiAdehazeStatsV39(AiqStatsTranslator_t* pStatsTrans,
        const aiq_VideoBuffer_t* from, aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct rkisp39_stat_buffer* left_stats;
    struct rkisp39_stat_buffer* right_stats;
    left_stats = (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));
    if (left_stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t bufLen = AiqV4l2Buffer_getV4lBufLength((AiqV4l2Buffer_t*)from);
    right_stats = (struct rkisp39_stat_buffer*)((char*)left_stats + bufLen / 2);
    if (left_stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    if (left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
        return XCAM_RETURN_ERROR_PARAM;
    } else {
        LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", left_stats->frame_id, left_stats->meas_type);
    }

    rkisp_adehaze_stats_t* statsInt = (rkisp_adehaze_stats_t*)to->_data;

    // dehaze
    to->bValid = left_stats->meas_type & ISP39_STAT_DHAZ;
    if (!to->bValid) return XCAM_RETURN_BYPASS;

    statsInt->dehaze_stats_v14.adp_wt       =
        (left_stats->stat.dhaz.adp_wt + right_stats->stat.dhaz.adp_wt) / 2;
    statsInt->dehaze_stats_v14.adp_air_base =
        (left_stats->stat.dhaz.adp_air_base + right_stats->stat.dhaz.adp_air_base) / 2;
    statsInt->dehaze_stats_v14.adp_tmax     =
        (left_stats->stat.dhaz.adp_tmax + right_stats->stat.dhaz.adp_tmax) / 2;
    for (int j = 0; j < ISP39_DHAZ_HIST_IIR_NUM; j++)
        for (int i = 0; i < ISP39_DHAZ_HIST_IIR_BLK_MAX; i++)
            statsInt->dehaze_stats_v14.hist_iir[i][j] =
                (left_stats->stat.dhaz.hist_iir[i][j] + right_stats->stat.dhaz.hist_iir[i][j]);

    return ret;
}

static void MergeAwbWinStats(
    awbStats_wpEngineStats_t *merge_stats,
    struct isp39_rawawb_stat *left_stats,
    struct isp39_rawawb_stat *right_stats,
    int lightNum,
    WinSplitMode mode
) {
    switch(mode) {
    case LEFT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats->norWp[i].hw_awbCfg_rGainSum_val =
                left_stats->sum[i].rgain_nor;
            merge_stats->norWp[i].hw_awbCfg_bGainSum_val =
                left_stats->sum[i].bgain_nor;
            merge_stats->norWp[i].hw_awbCfg_statsWp_count =
                left_stats->sum[i].wp_num_nor;
            merge_stats->bigWp[i].hw_awbCfg_rGainSum_val =
                left_stats->sum[i].rgain_big;
            merge_stats->bigWp[i].hw_awbCfg_bGainSum_val =
                left_stats->sum[i].bgain_big;
            merge_stats->bigWp[i].hw_awbCfg_statsWp_count =
                left_stats->sum[i].wp_num_big;
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats->norWp[i].hw_awbCfg_rGainSum_val =
                right_stats->sum[i].rgain_nor;
            merge_stats->norWp[i].hw_awbCfg_bGainSum_val =
                right_stats->sum[i].bgain_nor;
            merge_stats->norWp[i].hw_awbCfg_statsWp_count =
                right_stats->sum[i].wp_num_nor;
            merge_stats->bigWp[i].hw_awbCfg_rGainSum_val =
                right_stats->sum[i].rgain_big;
            merge_stats->bigWp[i].hw_awbCfg_bGainSum_val =
                right_stats->sum[i].bgain_big;
            merge_stats->bigWp[i].hw_awbCfg_statsWp_count =
                right_stats->sum[i].wp_num_big;
        }
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats->norWp[i].hw_awbCfg_rGainSum_val =
                left_stats->sum[i].rgain_nor + right_stats->sum[i].rgain_nor;
            merge_stats->norWp[i].hw_awbCfg_bGainSum_val =
                left_stats->sum[i].bgain_nor + right_stats->sum[i].bgain_nor;
            merge_stats->norWp[i].hw_awbCfg_statsWp_count =
                left_stats->sum[i].wp_num_nor + right_stats->sum[i].wp_num_nor;
            merge_stats->bigWp[i].hw_awbCfg_rGainSum_val =
                left_stats->sum[i].rgain_big + right_stats->sum[i].rgain_big;
            merge_stats->bigWp[i].hw_awbCfg_bGainSum_val =
                left_stats->sum[i].bgain_big + right_stats->sum[i].bgain_big;
            merge_stats->bigWp[i].hw_awbCfg_statsWp_count =
                left_stats->sum[i].wp_num_big + right_stats->sum[i].wp_num_big;
        }
        break;
    default:
        break;
    }
}

static void MergeAwbBlkStats(
    awbStats_pixStats_t *merge_stats,
    struct isp39_rawawb_stat *left_stats,
    struct isp39_rawawb_stat *right_stats,
    WinSplitMode mode
) {
    int index = 0;

    switch(mode) {
    case LEFT_MODE:
        for(int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
            for(int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                index = i * ISP39_MEAN_BLK_Y_NUM + j;
                merge_stats[index].hw_awbCfg_rSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[j].r;
                merge_stats[index].hw_awbCfg_gSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[j].g;
                merge_stats[index].hw_awbCfg_bSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[j].b;
                merge_stats[index].hw_awbCfg_statsPix_count = left_stats->ramdata_blk_y[i].ramdata_blk_x[j].wp;
            }
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
            for(int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                index = i * ISP39_MEAN_BLK_Y_NUM + j;
                merge_stats[index].hw_awbCfg_rSum_val = right_stats->ramdata_blk_y[i].ramdata_blk_x[j].r;
                merge_stats[index].hw_awbCfg_gSum_val = right_stats->ramdata_blk_y[i].ramdata_blk_x[j].g;
                merge_stats[index].hw_awbCfg_bSum_val = right_stats->ramdata_blk_y[i].ramdata_blk_x[j].b;
                merge_stats[index].hw_awbCfg_statsPix_count = right_stats->ramdata_blk_y[i].ramdata_blk_x[j].wp;
            }
        }
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
            for(int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                index = i * ISP39_MEAN_BLK_Y_NUM + j;
                if(j < ISP39_MEAN_BLK_X_NUM / 2) {
                    merge_stats[index].hw_awbCfg_rSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2].r + left_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 + 1].r;
                    merge_stats[index].hw_awbCfg_gSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2].g + left_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 + 1].g;
                    merge_stats[index].hw_awbCfg_bSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2].b +  left_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 + 1].b;
                    merge_stats[index].hw_awbCfg_statsPix_count = left_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2].wp + left_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 + 1].wp;
                } else if(j > ISP39_MEAN_BLK_X_NUM / 2) {
                    merge_stats[index].hw_awbCfg_rSum_val = right_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 - ISP39_MEAN_BLK_X_NUM].r + right_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 - ISP39_MEAN_BLK_X_NUM + 1].r;
                    merge_stats[index].hw_awbCfg_gSum_val = right_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 - ISP39_MEAN_BLK_X_NUM].g + right_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 - ISP39_MEAN_BLK_X_NUM + 1].g;
                    merge_stats[index].hw_awbCfg_bSum_val = right_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 - ISP39_MEAN_BLK_X_NUM].b + right_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 - ISP39_MEAN_BLK_X_NUM + 1].b;
                    merge_stats[index].hw_awbCfg_statsPix_count = right_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 - ISP39_MEAN_BLK_X_NUM].wp + right_stats->ramdata_blk_y[i].ramdata_blk_x[j * 2 - ISP39_MEAN_BLK_X_NUM + 1].wp;
                } else {
                    merge_stats[index].hw_awbCfg_rSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[ISP39_MEAN_BLK_X_NUM - 1].r + right_stats->ramdata_blk_y[i].ramdata_blk_x[0].r;
                    merge_stats[index].hw_awbCfg_gSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[ISP39_MEAN_BLK_X_NUM - 1].g + right_stats->ramdata_blk_y[i].ramdata_blk_x[0].g;
                    merge_stats[index].hw_awbCfg_bSum_val = left_stats->ramdata_blk_y[i].ramdata_blk_x[ISP39_MEAN_BLK_X_NUM - 1].b + right_stats->ramdata_blk_y[i].ramdata_blk_x[0].b;
                    merge_stats[index].hw_awbCfg_statsPix_count = left_stats->ramdata_blk_y[i].ramdata_blk_x[ISP39_MEAN_BLK_X_NUM - 1].wp + right_stats->ramdata_blk_y[i].ramdata_blk_x[0].wp;
                }
            }
        }
        break;
    default:
        break;
    }
}

static void MergeAwbHistBinStats(
    uint32_t *merge_stats,
    u16 *left_stats,
    u16 *right_stats,
    WinSplitMode mode
) {
    u32 tmp1, tmp2;
    switch(mode) {
    case LEFT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
            tmp1 = left_stats[i];
            if(left_stats[i] & 0x8000) {
                tmp1 = left_stats[i] & 0x7FFF;
                tmp1 *=    (1 << 3);
            }
            merge_stats[i] = tmp1;
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
            tmp2 = right_stats[i];
            if(right_stats[i] & 0x8000) {
                tmp2 = right_stats[i] & 0x7FFF;
                tmp2 *=    (1 << 3);
            }
            merge_stats[i] = tmp2;
        }

        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
            tmp1 = left_stats[i];
            if(left_stats[i] & 0x8000) {
                tmp1 = left_stats[i] & 0x7FFF;
                tmp1 *=    (1 << 3);
            }
            tmp2 = right_stats[i];
            if(right_stats[i] & 0x8000) {
                tmp2 = right_stats[i] & 0x7FFF;
                tmp2 *=    (1 << 3);
            }
            merge_stats[i] = tmp1 + tmp2;

        }
        break;
    }
}

static void MergeAwbExcWpStats(
    awbStats_wpStats_t*merge_stats,
    struct isp39_rawawb_stat *left_stats,
    struct isp39_rawawb_stat *right_stats,
    WinSplitMode mode
) {
    switch(mode) {
    case LEFT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
            merge_stats[i].hw_awbCfg_rGainSum_val = left_stats->sum_exc[i].rgain_exc;
            merge_stats[i].hw_awbCfg_bGainSum_val = left_stats->sum_exc[i].bgain_exc;
            merge_stats[i].hw_awbCfg_statsWp_count = left_stats->sum_exc[i].wp_num_exc;
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
            merge_stats[i].hw_awbCfg_rGainSum_val = right_stats->sum_exc[i].rgain_exc;
            merge_stats[i].hw_awbCfg_bGainSum_val = right_stats->sum_exc[i].bgain_exc;
            merge_stats[i].hw_awbCfg_statsWp_count = right_stats->sum_exc[i].wp_num_exc;
        }
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
            merge_stats[i].hw_awbCfg_rGainSum_val = left_stats->sum_exc[i].rgain_exc + right_stats->sum_exc[i].rgain_exc;
            merge_stats[i].hw_awbCfg_bGainSum_val = left_stats->sum_exc[i].bgain_exc + right_stats->sum_exc[i].bgain_exc;
            merge_stats[i].hw_awbCfg_statsWp_count = left_stats->sum_exc[i].wp_num_exc + right_stats->sum_exc[i].wp_num_exc;
        }
        break;
    }
}

XCamReturn translateMultiAwbStatsV39(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                     aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V39)
    struct rkisp39_stat_buffer* left_stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    aiq_awb_stats_wrapper_t* statsInt = (aiq_awb_stats_wrapper_t*)to->_data;

    if(left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t bufLen = AiqV4l2Buffer_getV4lBufLength((AiqV4l2Buffer_t*)from);
    struct rkisp39_stat_buffer* right_stats = (struct rkisp39_stat_buffer*)((char*)left_stats + bufLen / 2);
    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    if(left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
        return XCAM_RETURN_ERROR_PARAM;
    }

    LOGI_ANALYZER("awb stats: camId:%d, frame_id: %d,  meas_type; 0x%x",
                  pStatsTrans->mCamPhyId, left_stats->frame_id, left_stats->meas_type);

    to->bValid = left_stats->meas_type >> 5 & 1;
    if (!to->bValid) {
        LOGE_ANALYZER("AWB stats invalid, ignore");
        return XCAM_RETURN_BYPASS;
    }
    memset(&statsInt->awb_stats_v39, 0, sizeof(statsInt->awb_stats_v39));

    if (left_stats->stat.info2ddr.owner == RKISP_INFO2DRR_OWNER_AWB) {
        statsInt->awb_stats_v32.dbginfo_fd = left_stats->stat.info2ddr.buf_fd;
    } else {
        statsInt->awb_stats_v32.dbginfo_fd = -1;
    }
    statsInt->awb_stats_v39.awb_cfg_effect.blkMeasureMode =
        pStatsTrans->_ispParams->awb_cfg_v39.com.pixEngine.hw_awbCfg_zoneStatsSrc_mode > awbStats_pixAll_mode;
    statsInt->awb_stats_v39.awb_cfg_effect.mode = pStatsTrans->_ispParams->awb_cfg_v39.mode;
    statsInt->awb_stats_v39.awb_cfg_effect.lightNum = pStatsTrans->_ispParams->awb_cfg_v39.com.wpEngine.hw_awbCfg_lightSrcNum_val;
    statsInt->awb_stats_v39.awb_cfg_effect.groupIllIndxCurrent = pStatsTrans->_ispParams->awb_cfg_v39.groupIllIndxCurrent;
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent, pStatsTrans->_ispParams->awb_cfg_v39.IllIndxSetCurrent,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.timeSign, pStatsTrans->_ispParams->awb_cfg_v39.timeSign,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.timeSign));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.preWbgainSw, pStatsTrans->_ispParams->awb_cfg_v39.preWbgainSw,
           sizeof(pStatsTrans->_ispParams->awb_cfg_v39.preWbgainSw));
    statsInt->awb_cfg_effect_valid = true;

    awbStats_stats_t *awb_stats_v39 = &statsInt->awb_stats_v39.com;

    WinSplitMode AwbWinSplitMode = LEFT_AND_RIGHT_MODE;

    struct isp2x_window ori_win;
    ori_win.h_offs = pStatsTrans->_ispParams->meas.rawawb.h_offs;
    ori_win.h_size = pStatsTrans->_ispParams->meas.rawawb.h_size;
    ori_win.v_offs = pStatsTrans->_ispParams->meas.rawawb.v_offs;
    ori_win.v_size = pStatsTrans->_ispParams->meas.rawawb.v_size;

    JudgeWinLocation39(&ori_win, &AwbWinSplitMode, pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);
    MergeAwbWinStats( &awb_stats_v39->wpEngine, &left_stats->stat.rawawb, &right_stats->stat.rawawb,
                      statsInt->awb_stats_v39.awb_cfg_effect.lightNum, AwbWinSplitMode);

    struct isp2x_window left_win;
    left_win.h_offs = pStatsTrans->_ispParams->isp_params_v39[0].meas.rawawb.h_offs;
    left_win.h_size = pStatsTrans->_ispParams->isp_params_v39[0].meas.rawawb.h_size;
    left_win.v_offs = pStatsTrans->_ispParams->isp_params_v39[0].meas.rawawb.v_offs;
    left_win.v_size = pStatsTrans->_ispParams->isp_params_v39[0].meas.rawawb.v_size;

    struct isp2x_window right_win;
    right_win.h_offs = pStatsTrans->_ispParams->isp_params_v39[1].meas.rawawb.h_offs;
    right_win.h_size = pStatsTrans->_ispParams->isp_params_v39[1].meas.rawawb.h_size;
    right_win.v_offs = pStatsTrans->_ispParams->isp_params_v39[1].meas.rawawb.v_offs;
    right_win.v_size = pStatsTrans->_ispParams->isp_params_v39[1].meas.rawawb.v_size;

    MergeAwbBlkStats(awb_stats_v39->pixEngine.zonePix, &left_stats->stat.rawawb,
                     &right_stats->stat.rawawb, AwbWinSplitMode);

    memset(&statsInt->awb_stats_v39.sumBlkRGB, 0, sizeof(statsInt->awb_stats_v39.sumBlkRGB));
    int index = 0;
    for (int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
        for (int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
            statsInt->awb_stats_v39.sumBlkRGB.hw_awbCfg_rSum_val += statsInt->awb_stats_v39.com.pixEngine.zonePix[index].hw_awbCfg_rSum_val;
            statsInt->awb_stats_v39.sumBlkRGB.hw_awbCfg_gSum_val += statsInt->awb_stats_v39.com.pixEngine.zonePix[index].hw_awbCfg_gSum_val;
            statsInt->awb_stats_v39.sumBlkRGB.hw_awbCfg_bSum_val += statsInt->awb_stats_v39.com.pixEngine.zonePix[index].hw_awbCfg_bSum_val;
            index++;
        }
    }

    MergeAwbHistBinStats(awb_stats_v39->wpEngine.hw_awb_wpHistBin_val, left_stats->stat.rawawb.yhist, right_stats->stat.rawawb.yhist, AwbWinSplitMode);

    switch(AwbWinSplitMode) {
    case LEFT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++)
            awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = left_stats->stat.rawawb.sum[i].wp_num2;
        break;
    case RIGHT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++)
            awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = right_stats->stat.rawawb.sum[i].wp_num2;
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++)
            awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = left_stats->stat.rawawb.sum[i].wp_num2 + right_stats->stat.rawawb.sum[i].wp_num2;
        break;
    default:
        break;
    }

    MergeAwbExcWpStats(awb_stats_v39->wpFltOutFullEngine.fltPix, &left_stats->stat.rawawb, &right_stats->stat.rawawb, AwbWinSplitMode);

    to->frame_id = left_stats->frame_id;
#endif
    return ret;
}

XCamReturn translateAwbStatsV39(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V39)

    if (pStatsTrans->mIsMultiIsp && pStatsTrans->mIspUniteMode)
        return translateMultiAwbStatsV39(pStatsTrans, from, to);

    aiq_awb_stats_wrapper_t* statsInt = (aiq_awb_stats_wrapper_t*)to->_data;
    struct rkisp39_stat_buffer* stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("awb stats: camId:%d, frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  stats->frame_id, stats->meas_type);

    to->bValid = stats->meas_type >> 5 & 1;
    if (!to->bValid) {
        LOGE_ANALYZER("AWB stats invalid, ignore");
        return XCAM_RETURN_BYPASS;
    }
    memset(&statsInt->awb_stats_v39, 0, sizeof(statsInt->awb_stats_v39));

    if (stats->stat.info2ddr.owner == RKISP_INFO2DRR_OWNER_AWB) {
        statsInt->awb_stats_v39.dbginfo_fd = stats->stat.info2ddr.buf_fd;
    } else {
        statsInt->awb_stats_v39.dbginfo_fd = -1;
    }
    statsInt->awb_stats_v39.awb_cfg_effect.blkMeasureMode =
        pStatsTrans->_ispParams->awb_cfg_v39.com.pixEngine.hw_awbCfg_zoneStatsSrc_mode > awbStats_pixAll_mode;
    statsInt->awb_stats_v39.awb_cfg_effect.mode = pStatsTrans->_ispParams->awb_cfg_v39.mode;
    statsInt->awb_stats_v39.awb_cfg_effect.lightNum = pStatsTrans->_ispParams->awb_cfg_v39.com.wpEngine.hw_awbCfg_lightSrcNum_val;
    statsInt->awb_stats_v39.awb_cfg_effect.groupIllIndxCurrent = pStatsTrans->_ispParams->awb_cfg_v39.groupIllIndxCurrent;
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent, pStatsTrans->_ispParams->awb_cfg_v39.IllIndxSetCurrent,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.timeSign, pStatsTrans->_ispParams->awb_cfg_v39.timeSign,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.timeSign));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.preWbgainSw, pStatsTrans->_ispParams->awb_cfg_v39.preWbgainSw,
           sizeof(pStatsTrans->_ispParams->awb_cfg_v39.preWbgainSw));
    statsInt->awb_cfg_effect_valid = true;
    to->frame_id             = stats->frame_id;

    awbStats_stats_t *awb_stats_v39 = &statsInt->awb_stats_v39.com ;

    for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
        awb_stats_v39->wpEngine.norWp[i].hw_awbCfg_rGainSum_val =
            stats->stat.rawawb.sum[i].rgain_nor;
        awb_stats_v39->wpEngine.norWp[i].hw_awbCfg_bGainSum_val =
            stats->stat.rawawb.sum[i].bgain_nor;
        awb_stats_v39->wpEngine.norWp[i].hw_awbCfg_statsWp_count =
            stats->stat.rawawb.sum[i].wp_num_nor;
        awb_stats_v39->wpEngine.bigWp[i].hw_awbCfg_rGainSum_val =
            stats->stat.rawawb.sum[i].rgain_big;
        awb_stats_v39->wpEngine.bigWp[i].hw_awbCfg_bGainSum_val =
            stats->stat.rawawb.sum[i].bgain_big;
        awb_stats_v39->wpEngine.bigWp[i].hw_awbCfg_statsWp_count =
            stats->stat.rawawb.sum[i].wp_num_big;
    }
    memset(&statsInt->awb_stats_v39.sumBlkRGB, 0, sizeof(statsInt->awb_stats_v39.sumBlkRGB));
    int index = 0;
    for (int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
        for (int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
            awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_rSum_val = stats->stat.rawawb.ramdata_blk_y[i].ramdata_blk_x[j].r;
            awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_gSum_val = stats->stat.rawawb.ramdata_blk_y[i].ramdata_blk_x[j].g;
            awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_bSum_val = stats->stat.rawawb.ramdata_blk_y[i].ramdata_blk_x[j].b;
            awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_statsPix_count = stats->stat.rawawb.ramdata_blk_y[i].ramdata_blk_x[j].wp;
            statsInt->awb_stats_v39.sumBlkRGB.hw_awbCfg_rSum_val += awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_rSum_val;
            statsInt->awb_stats_v39.sumBlkRGB.hw_awbCfg_gSum_val += awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_gSum_val;
            statsInt->awb_stats_v39.sumBlkRGB.hw_awbCfg_bSum_val += awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_bSum_val;
            index++;
        }
    }

    for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
        awb_stats_v39->wpEngine.hw_awb_wpHistBin_val[i] = stats->stat.rawawb.yhist[i];
        // move the shift code here to make WpNoHist merged by several cameras easily
        if( stats->stat.rawawb.yhist[i]  & 0x8000 ) {
            awb_stats_v39->wpEngine.hw_awb_wpHistBin_val[i] = stats->stat.rawawb.yhist[i] & 0x7FFF;
            awb_stats_v39->wpEngine.hw_awb_wpHistBin_val[i] *=    (1 << 3);
        }
    }

    for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
        awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] =  stats->stat.rawawb.sum[i].wp_num2;
    }
    for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
        awb_stats_v39->wpFltOutFullEngine.fltPix[i].hw_awbCfg_rGainSum_val = stats->stat.rawawb.sum_exc[i].rgain_exc;
        awb_stats_v39->wpFltOutFullEngine.fltPix[i].hw_awbCfg_bGainSum_val = stats->stat.rawawb.sum_exc[i].bgain_exc;
        awb_stats_v39->wpFltOutFullEngine.fltPix[i].hw_awbCfg_statsWp_count =    stats->stat.rawawb.sum_exc[i].wp_num_exc;

    }
    to->frame_id = stats->frame_id;
#endif
    return ret;
}

XCamReturn translateMultiAfStatsV39(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                    aiq_stats_base_t* to) {
    typedef enum WinSplitMode_s {
        LEFT_AND_RIGHT_MODE = 0,
        LEFT_MODE,
        RIGHT_MODE,
        FULL_MODE,
    } SplitMode;

    struct AfSplitInfo {
        SplitMode wina_side_info;
        int32_t wina_l_blknum;
        int32_t wina_r_blknum;
        int32_t wina_r_skip_blknum;
        float wina_l_ratio;
        float wina_r_ratio;

        SplitMode winb_side_info;
        float winb_l_ratio;
        float winb_r_ratio;
    };

    struct isp32_rawaf_luma_data {
        u32 channelg_xy: 12;
        u32 highlit_cnt: 16;
        u32 dummy: 4;
    } __attribute__ ((packed));

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiq_af_stats_wrapper_t* statsInt = (aiq_af_stats_wrapper_t*)to->_data;

    struct rkisp39_stat_buffer* left_stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t bufLen = AiqV4l2Buffer_getV4lBufLength((AiqV4l2Buffer_t*)from);
    struct rkisp39_stat_buffer* right_stats = (struct rkisp39_stat_buffer*)((char*)left_stats + bufLen / 2);

    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  left_stats->frame_id, left_stats->meas_type);

    if(left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
        return XCAM_RETURN_ERROR_PARAM;
    }

    struct isp39_rawaf_meas_cfg* org_af = &pStatsTrans->_ispParams->meas.rawaf;
    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t x_st, x_ed, l_blknum, r_blknum, ov_w, blk_w, r_skip_blknum;
    struct AfSplitInfo af_split_info;
    int32_t i, j, k, dst_idx, l_idx, r_idx, l_lht, r_lht, lht0, lht1;

    struct isp32_bls_cfg* bls_cfg = &pStatsTrans->_ispParams->bls_cfg;
    u8 bnr2af_sel = pStatsTrans->_ispParams->meas.rawaf.bnr2af_sel;
    u8 from_ynr = org_af->from_ynr;
    bool is_hdr = (pStatsTrans->mWorkingMode > 0) ? true : false;
    int temp_luma, comp_bls = 0;
    u16 max_val = (1 << 12) - 1;

    if (bls_cfg->bls1_en && !is_hdr && !from_ynr && !bnr2af_sel) {
        comp_bls = (bls_cfg->bls1_val.gr + bls_cfg->bls1_val.gb) / 2  - bls_cfg->isp_ob_offset;
        comp_bls = MAX(comp_bls, 0);
    }

    memset(&af_split_info, 0, sizeof(af_split_info));
    ov_w = pStatsTrans->left_isp_rect_.w + pStatsTrans->left_isp_rect_.x - pStatsTrans->right_isp_rect_.x;
    x_st = org_af->win[0].h_offs;
    x_ed = x_st + org_af->win[0].h_size;
    l_isp_st = pStatsTrans->left_isp_rect_.x;
    l_isp_ed = pStatsTrans->left_isp_rect_.x + pStatsTrans->left_isp_rect_.w;
    r_isp_st = pStatsTrans->right_isp_rect_.x;
    r_isp_ed = pStatsTrans->right_isp_rect_.x + pStatsTrans->right_isp_rect_.w;
    LOGD_AF("wina.x_st %d, wina.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            x_st, x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    //// winA ////
    af_split_info.wina_l_ratio = 0;
    af_split_info.wina_r_ratio = 0;
    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        af_split_info.wina_side_info = LEFT_AND_RIGHT_MODE;
        // af win < one isp width
        if (org_af->win[0].h_size < pStatsTrans->left_isp_rect_.w) {
            blk_w = org_af->win[0].h_size / ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum = (l_isp_ed - x_st + blk_w - 1) / blk_w;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW - l_blknum;
            l_win_ed = l_isp_ed - 2;
            l_win_st = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            if (blk_w < ov_w) {
                r_skip_blknum = ov_w / blk_w;
                r_win_st = ov_w - r_skip_blknum * blk_w;
                r_win_ed = ov_w + (ISP2X_RAWAF_SUMDATA_ROW - r_skip_blknum) * blk_w;
                af_split_info.wina_r_skip_blknum = r_skip_blknum;
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;

                // blend last block of left isp and first block of right isp
                af_split_info.wina_r_skip_blknum = 0;
                af_split_info.wina_l_ratio = (float)ov_w / (float)blk_w;
                af_split_info.wina_r_ratio = 1 - af_split_info.wina_l_ratio;
            }
        }
        // af win <= one isp width * 1.5
        else if (org_af->win[0].h_size < pStatsTrans->left_isp_rect_.w * 3 / 2) {
            l_win_st = x_st;
            l_win_ed = l_isp_ed - 2;
            blk_w = (l_win_ed - l_win_st) / (ISP2X_RAWAF_SUMDATA_ROW + 1);
            l_win_st = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum = ((l_win_ed - l_win_st) * ISP2X_RAWAF_SUMDATA_ROW + org_af->win[0].h_size - 1) / org_af->win[0].h_size;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW - l_blknum;
            if (blk_w < ov_w) {
                r_skip_blknum = ov_w / blk_w;
                r_win_st = ov_w - r_skip_blknum * blk_w;
                r_win_ed = ov_w + (ISP2X_RAWAF_SUMDATA_ROW - r_skip_blknum) * blk_w;
                af_split_info.wina_r_skip_blknum = r_skip_blknum;
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;
                // blend last block of left isp and first block of right isp
                af_split_info.wina_r_skip_blknum = 0;
                af_split_info.wina_l_ratio = (float)ov_w / (float)blk_w;
                af_split_info.wina_r_ratio = 1 - af_split_info.wina_l_ratio;
            }
        }
        else {
            l_win_st = x_st;
            l_win_ed = l_isp_ed - 2;
            blk_w = (l_win_ed - l_win_st) / ISP2X_RAWAF_SUMDATA_ROW;
            l_win_st = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            r_win_st = 2;
            r_win_ed = r_win_st + blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            af_split_info.wina_side_info = FULL_MODE;
            l_blknum = ISP2X_RAWAF_SUMDATA_ROW;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        }
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        af_split_info.wina_side_info = RIGHT_MODE;
        l_blknum = 0;
        r_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        r_win_st = x_st - pStatsTrans->right_isp_rect_.x;
        r_win_ed = x_ed - pStatsTrans->right_isp_rect_.x;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        af_split_info.wina_side_info = LEFT_MODE;
        l_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        r_blknum = 0;
        l_win_st = x_st;
        l_win_ed = x_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    af_split_info.wina_l_blknum = l_blknum;
    af_split_info.wina_r_blknum = r_blknum;

    //// winB ////
    af_split_info.winb_l_ratio = 0;
    af_split_info.winb_r_ratio = 0;
    x_st = org_af->win[1].h_offs;
    x_ed = x_st + org_af->win[1].h_size;
    LOGD_AF("winb.x_st %d, winb.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            x_st, x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        af_split_info.winb_side_info = LEFT_AND_RIGHT_MODE;
        l_win_st = x_st;
        l_win_ed = l_isp_ed - 2;
        r_win_st = ov_w - 2;
        r_win_ed = x_ed - pStatsTrans->right_isp_rect_.x;
        // blend winB by width of left isp winB and right isp winB
        af_split_info.winb_l_ratio = (float)(l_win_ed - l_win_st) / (float)(x_ed - x_st);
        af_split_info.winb_r_ratio = 1 - af_split_info.winb_l_ratio;
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        af_split_info.winb_side_info = RIGHT_MODE;
        af_split_info.winb_l_ratio = 0;
        af_split_info.winb_r_ratio = 1;
        r_win_st = x_st - pStatsTrans->right_isp_rect_.x;
        r_win_ed = x_ed - pStatsTrans->right_isp_rect_.x;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        af_split_info.winb_side_info = LEFT_MODE;
        af_split_info.winb_l_ratio = 1;
        af_split_info.winb_r_ratio = 0;
        l_win_st = x_st;
        l_win_ed = x_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    if (!(left_stats->meas_type & ISP39_STAT_RAWAF)) {
        to->bValid = false;
        LOGD_AF("af stat is invalid, ignore");
        return XCAM_RETURN_BYPASS;
    } else {
        to->bValid = true;
    }

    AiqAfInfoWrapper_t afParamsTmp;
    AiqAfInfoWrapper_t* afParams = NULL;
    if (pStatsTrans->mFocusLensHw) {
        XCamReturn ret = AiqLensHw_getAfInfoParams(pStatsTrans->mFocusLensHw, left_stats->frame_id, &afParamsTmp);
        if (ret == XCAM_RETURN_NO_ERROR) afParams = &afParamsTmp;
    }

    memset(&statsInt->af_stats_v3x, 0, sizeof(rk_aiq_isp_af_stats_v3x_t));
    memset(&statsInt->stat_motor, 0, sizeof(rk_aiq_af_algo_motor_stat_t));
    to->frame_id = left_stats->frame_id;

    //af
    {
#ifdef USE_NEWSTRUCT
        statsInt->stat_motor.comp_bls = 0;
        if (af_split_info.winb_side_info == LEFT_AND_RIGHT_MODE) {
            statsInt->afStats_stats.subWin.hw_af_luma_val     = left_stats->stat.rawaf.sumy_winb * af_split_info.winb_l_ratio +
                    right_stats->stat.rawaf.sumy_winb * af_split_info.winb_r_ratio;
            statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val = left_stats->stat.rawaf.highlit_cnt_winb +
                    right_stats->stat.rawaf.highlit_cnt_winb;
            statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val = left_stats->stat.rawaf.v1iir_sumb * af_split_info.winb_l_ratio +
                    right_stats->stat.rawaf.v1iir_sumb * af_split_info.winb_r_ratio;
            statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val = left_stats->stat.rawaf.v2iir_sumb * af_split_info.winb_l_ratio +
                    right_stats->stat.rawaf.v2iir_sumb * af_split_info.winb_r_ratio;
            statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val = left_stats->stat.rawaf.h1iir_sumb * af_split_info.winb_l_ratio +
                    right_stats->stat.rawaf.h1iir_sumb * af_split_info.winb_r_ratio;
            statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val = left_stats->stat.rawaf.h2iir_sumb * af_split_info.winb_l_ratio +
                    right_stats->stat.rawaf.h2iir_sumb * af_split_info.winb_r_ratio;
        } else if (af_split_info.winb_side_info == LEFT_MODE) {
            statsInt->afStats_stats.subWin.hw_af_luma_val     = left_stats->stat.rawaf.sumy_winb;
            statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val = left_stats->stat.rawaf.highlit_cnt_winb;
            statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val = left_stats->stat.rawaf.v1iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val = left_stats->stat.rawaf.v2iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val = left_stats->stat.rawaf.h1iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val = left_stats->stat.rawaf.h2iir_sumb;
        } else {
            statsInt->afStats_stats.subWin.hw_af_luma_val     = right_stats->stat.rawaf.sumy_winb;
            statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val = right_stats->stat.rawaf.highlit_cnt_winb;
            statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val = right_stats->stat.rawaf.v1iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val = right_stats->stat.rawaf.v2iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val = right_stats->stat.rawaf.h1iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val = right_stats->stat.rawaf.h2iir_sumb;
        }

        if (af_split_info.wina_side_info == FULL_MODE) {
            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    if (j == 0) {
                        l_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                        statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                        statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                        temp_luma = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                        statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                    } else if (j >= 1 && j <= 7) {
                        l_idx = i * ISP39_MEAN_BLK_Y_NUM + 2 * (j - 1) + 1;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                            left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 1) + 1].v1 + left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * j].v1;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                            left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 1) + 1].v2 + left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * j].v2;
                        statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                            left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 1) + 1].h1 + left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * j].h1;
                        statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                            left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 1) + 1].h2 + left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * j].h2;

                        temp_luma = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 1) + 1].average + left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * j].average;
                        statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] =
                            left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 1) + 1].highlight + left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * j].highlight;
                    } else {
                        r_idx = i * ISP39_MEAN_BLK_Y_NUM + 2 * (j - 8) + 1;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                            right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 8) + 1].v1 + right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 7)].v1;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                            right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 8) + 1].v2 + right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 7)].v2;
                        statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                            right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 8) + 1].h1 + right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 7)].h1;
                        statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                            right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 8) + 1].h2 + right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 7)].h2;

                        temp_luma =
                            right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 8) + 1].average + right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 7)].average;
                        statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] =
                            right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 8) + 1].highlight + right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 7)].highlight;
                    }
                }
            }
        }
        else if (af_split_info.wina_side_info == LEFT_AND_RIGHT_MODE) {
            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                j = ISP39_MEAN_BLK_X_NUM - af_split_info.wina_l_blknum;
                for (k = 0; k < af_split_info.wina_l_blknum; j++, k++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + k;
                    l_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                    temp_luma = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                    statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                    statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                }
            }

            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                j = af_split_info.wina_r_skip_blknum;
                for (k = 0; k < af_split_info.wina_r_blknum; j++, k++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + k + af_split_info.wina_l_blknum;
                    r_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] = right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] = right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] = right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] = right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                    temp_luma = right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                    statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                    statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] = right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                }
            }

            if (af_split_info.wina_r_skip_blknum == 0) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = j * ISP39_MEAN_BLK_Y_NUM + (af_split_info.wina_l_blknum - 1);
                    l_idx = j * ISP39_MEAN_BLK_Y_NUM + (ISP39_MEAN_BLK_X_NUM - 1);
                    r_idx = j * ISP39_MEAN_BLK_Y_NUM;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                        left_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[ISP39_MEAN_BLK_X_NUM - 1].v1 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[0].v1 * af_split_info.wina_r_ratio;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                        left_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[ISP39_MEAN_BLK_X_NUM - 1].v2 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[0].v2 * af_split_info.wina_r_ratio;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                        left_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[ISP39_MEAN_BLK_X_NUM - 1].h1 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[0].h1 * af_split_info.wina_r_ratio;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                        left_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[ISP39_MEAN_BLK_X_NUM - 1].h2 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[0].h2 * af_split_info.wina_r_ratio;

                    temp_luma = left_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[ISP39_MEAN_BLK_X_NUM - 1].average +
                                right_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[0].average;
                    statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                    statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] =
                        left_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[ISP39_MEAN_BLK_X_NUM - 1].highlight +
                        right_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[0].highlight;
                }
            }
        } else if (af_split_info.wina_side_info == LEFT_MODE) {
            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                    temp_luma = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                    statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                    statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                }
            }
        } else {
            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] = right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] = right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] = right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] = right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                    temp_luma = right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                    statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                    statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] = right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                }
            }
        }
#else
        statsInt->stat_motor.comp_bls = comp_bls >> 2;
        if (af_split_info.winb_side_info == LEFT_AND_RIGHT_MODE) {
            statsInt->af_stats_v3x.wndb_luma = left_stats->stat.rawaf.sumy_winb * af_split_info.winb_l_ratio +
                                               right_stats->stat.rawaf.sumy_winb * af_split_info.winb_r_ratio;
            statsInt->af_stats_v3x.winb_highlit_cnt = left_stats->stat.rawaf.highlit_cnt_winb +
                    right_stats->stat.rawaf.highlit_cnt_winb;
            statsInt->af_stats_v3x.wndb_fv_v1 = left_stats->stat.rawaf.v1iir_sumb * af_split_info.winb_l_ratio +
                                                right_stats->stat.rawaf.v1iir_sumb * af_split_info.winb_r_ratio;
            statsInt->af_stats_v3x.wndb_fv_v2 = left_stats->stat.rawaf.v2iir_sumb * af_split_info.winb_l_ratio +
                                                right_stats->stat.rawaf.v2iir_sumb * af_split_info.winb_r_ratio;
            statsInt->af_stats_v3x.wndb_fv_h1 = left_stats->stat.rawaf.h1iir_sumb * af_split_info.winb_l_ratio +
                                                right_stats->stat.rawaf.h1iir_sumb * af_split_info.winb_r_ratio;
            statsInt->af_stats_v3x.wndb_fv_h2 = left_stats->stat.rawaf.h2iir_sumb * af_split_info.winb_l_ratio +
                                                right_stats->stat.rawaf.h2iir_sumb * af_split_info.winb_r_ratio;
        } else if (af_split_info.winb_side_info == LEFT_MODE) {
            statsInt->af_stats_v3x.wndb_luma = left_stats->stat.rawaf.sumy_winb;
            statsInt->af_stats_v3x.winb_highlit_cnt = left_stats->stat.rawaf.highlit_cnt_winb;
            statsInt->af_stats_v3x.wndb_fv_v1 = left_stats->stat.rawaf.v1iir_sumb;
            statsInt->af_stats_v3x.wndb_fv_v2 = left_stats->stat.rawaf.v2iir_sumb;
            statsInt->af_stats_v3x.wndb_fv_h1 = left_stats->stat.rawaf.h1iir_sumb;
            statsInt->af_stats_v3x.wndb_fv_h2 = left_stats->stat.rawaf.h2iir_sumb;
        } else {
            statsInt->af_stats_v3x.wndb_luma = right_stats->stat.rawaf.sumy_winb;
            statsInt->af_stats_v3x.winb_highlit_cnt = right_stats->stat.rawaf.highlit_cnt_winb;
            statsInt->af_stats_v3x.wndb_fv_v1 = right_stats->stat.rawaf.v1iir_sumb;
            statsInt->af_stats_v3x.wndb_fv_v2 = right_stats->stat.rawaf.v2iir_sumb;
            statsInt->af_stats_v3x.wndb_fv_h1 = right_stats->stat.rawaf.h1iir_sumb;
            statsInt->af_stats_v3x.wndb_fv_h2 = right_stats->stat.rawaf.h2iir_sumb;
        }

        if (af_split_info.wina_side_info == FULL_MODE) {
            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    if (j == 0) {
                        l_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                        statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                        statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                        statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                        statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                        temp_luma = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                        temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                        statsInt->af_stats_v3x.wnda_luma[dst_idx] = MAX(temp_luma, 0);
                        statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                    } else if (j >= 1 && j <= 7) {
                        l_idx = i * ISP39_MEAN_BLK_Y_NUM + 2 * (j - 1) + 1;
                        statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] =
                            left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 1) + 1].v1 + left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * j].v1;
                        statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] =
                            left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 1) + 1].v2 + left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * j].v2;
                        statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] =
                            left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 1) + 1].h1 + left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * j].h1;
                        statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] =
                            left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 1) + 1].h2 + left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * j].h2;

                        temp_luma = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 1) + 1].average + left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * j].average;
                        temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                        statsInt->af_stats_v3x.wnda_luma[dst_idx] = MAX(temp_luma, 0);
                        statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] =
                            left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 1) + 1].highlight + left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * j].highlight;
                    } else {
                        r_idx = i * ISP39_MEAN_BLK_Y_NUM + 2 * (j - 8) + 1;
                        statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] =
                            right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 8) + 1].v1 + right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 7)].v1;
                        statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] =
                            right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 8) + 1].v2 + right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 7)].v2;
                        statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] =
                            right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 8) + 1].h1 + right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 7)].h1;
                        statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] =
                            right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 8) + 1].h2 + right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 7)].h2;

                        temp_luma =
                            right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 8) + 1].average + right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 7)].average;
                        temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                        statsInt->af_stats_v3x.wnda_luma[dst_idx] = MAX(temp_luma, 0);
                        statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] =
                            right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 8) + 1].highlight + right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 7)].highlight;
                    }
                }
            }
        }
        else if (af_split_info.wina_side_info == LEFT_AND_RIGHT_MODE) {
            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                j = ISP39_MEAN_BLK_X_NUM - af_split_info.wina_l_blknum;
                for (k = 0; k < af_split_info.wina_l_blknum; j++, k++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + k;
                    l_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                    statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                    statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                    statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                    temp_luma = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                    temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                    statsInt->af_stats_v3x.wnda_luma[dst_idx] = MAX(temp_luma, 0);
                    statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                }
            }

            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                j = af_split_info.wina_r_skip_blknum;
                for (k = 0; k < af_split_info.wina_r_blknum; j++, k++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + k + af_split_info.wina_l_blknum;
                    r_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] = right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                    statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] = right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                    statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] = right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                    statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] = right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                    temp_luma = right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                    temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                    statsInt->af_stats_v3x.wnda_luma[dst_idx] = MAX(temp_luma, 0);
                    statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] = right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                }
            }

            if (af_split_info.wina_r_skip_blknum == 0) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = j * ISP39_MEAN_BLK_Y_NUM + (af_split_info.wina_l_blknum - 1);
                    l_idx = j * ISP39_MEAN_BLK_Y_NUM + (ISP39_MEAN_BLK_X_NUM - 1);
                    r_idx = j * ISP39_MEAN_BLK_Y_NUM;
                    statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] =
                        left_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[ISP39_MEAN_BLK_X_NUM - 1].v1 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[0].v1 * af_split_info.wina_r_ratio;
                    statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] =
                        left_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[ISP39_MEAN_BLK_X_NUM - 1].v2 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[0].v2 * af_split_info.wina_r_ratio;
                    statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] =
                        left_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[ISP39_MEAN_BLK_X_NUM - 1].h1 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[0].h1 * af_split_info.wina_r_ratio;
                    statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] =
                        left_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[ISP39_MEAN_BLK_X_NUM - 1].h2 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[0].h2 * af_split_info.wina_r_ratio;

                    temp_luma = left_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[ISP39_MEAN_BLK_X_NUM - 1].average +
                                right_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[0].average;
                    temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                    statsInt->af_stats_v3x.wnda_luma[dst_idx] = MAX(temp_luma, 0);
                    statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] =
                        left_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[ISP39_MEAN_BLK_X_NUM - 1].highlight +
                        right_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[0].highlight;
                }
            }
        } else if (af_split_info.wina_side_info == LEFT_MODE) {
            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                    statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] = left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                    statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                    statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] = left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                    temp_luma = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                    temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                    statsInt->af_stats_v3x.wnda_luma[dst_idx] = MAX(temp_luma, 0);
                    statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] = left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                }
            }
        } else {
            for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                    statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] = right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                    statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] = right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                    statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] = right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                    statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] = right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                    temp_luma = right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                    temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                    statsInt->af_stats_v3x.wnda_luma[dst_idx] = MAX(temp_luma, 0);
                    statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] = right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                }
            }
        }

        LOGD_AF("af_split_info.wina: %d, %d, %d, %d, %f, %f",
                af_split_info.wina_side_info, af_split_info.wina_l_blknum, af_split_info.wina_r_blknum,
                af_split_info.wina_r_skip_blknum, af_split_info.wina_l_ratio, af_split_info.wina_r_ratio);
        LOGD_AF("af_split_info.winb: %d, %f, %f",
                af_split_info.winb_side_info, af_split_info.winb_l_ratio, af_split_info.winb_r_ratio);
#endif

        if (afParams) {
            statsInt->stat_motor.focusCode       = afParams->focusCode;
            statsInt->stat_motor.zoomCode        = afParams->zoomCode;
            statsInt->stat_motor.focus_endtim    = afParams->focusEndTim;
            statsInt->stat_motor.focus_starttim  = afParams->focusStartTim;
            statsInt->stat_motor.zoom_endtim     = afParams->zoomEndTim;
            statsInt->stat_motor.zoom_starttim   = afParams->zoomStartTim;
            statsInt->stat_motor.sof_tim         = afParams->sofTime;
            statsInt->stat_motor.focusCorrection = afParams->focusCorrection;
            statsInt->stat_motor.zoomCorrection  = afParams->zoomCorrection;
            statsInt->stat_motor.angleZ          = afParams->angleZ;
        }

        if (pStatsTrans->_expParams) statsInt->aecExpInfo = pStatsTrans->_expParams->aecExpInfo;
    }

    return ret;
}

XCamReturn translateAfStatsV39(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                               aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)

    if (pStatsTrans->mIsMultiIsp && pStatsTrans->mIspUniteMode)
        return translateMultiAfStatsV39(pStatsTrans, from, to);

    aiq_af_stats_wrapper_t* statsInt = (aiq_af_stats_wrapper_t*)to->_data;
    struct rkisp39_stat_buffer* stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));
    if (stats == NULL) {
        LOGE_AF("fail to get stats, ignore");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", stats->frame_id, stats->meas_type);

    if (!(stats->meas_type & ISP39_STAT_RAWAF)) {
        to->bValid = false;
        LOGD_AF("af stat is invalid, ignore");
        return XCAM_RETURN_BYPASS;
    } else {
        to->bValid = true;
    }

    AiqAfInfoWrapper_t afParamsTmp;
    AiqAfInfoWrapper_t* afParams = NULL;
    if (pStatsTrans->mFocusLensHw) {
        XCamReturn ret = AiqLensHw_getAfInfoParams(pStatsTrans->mFocusLensHw, stats->frame_id, &afParamsTmp);
        if (ret == XCAM_RETURN_NO_ERROR) afParams = &afParamsTmp;
    }

    struct isp32_bls_cfg* bls_cfg = &pStatsTrans->_ispParams->bls_cfg;
    u8 bnr2af_sel                 = pStatsTrans->_ispParams->meas.rawaf.bnr2af_sel;
    u8 from_ynr                   = pStatsTrans->_ispParams->meas.rawaf.from_ynr;
    bool is_hdr                   = (pStatsTrans->mWorkingMode > 0) ? true : false;
    int temp_luma, comp_bls = 0;
    u16 max_val = (1 << 12) - 1;

    if (bls_cfg->bls1_en && !is_hdr && !from_ynr && !bnr2af_sel) {
        comp_bls = (bls_cfg->bls1_val.gr + bls_cfg->bls1_val.gb) / 2 - bls_cfg->isp_ob_offset;
        comp_bls = MAX(comp_bls, 0);
    }

    memset(&statsInt->af_stats_v3x, 0, sizeof(rk_aiq_isp_af_stats_v3x_t));
    memset(&statsInt->stat_motor, 0, sizeof(rk_aiq_af_algo_motor_stat_t));
    to->frame_id = stats->frame_id;

    // af
    {
        int index                     = 0;

#ifdef USE_NEWSTRUCT
        statsInt->stat_motor.comp_bls = 0;
        statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val = stats->stat.rawaf.h1iir_sumb;
        statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val = stats->stat.rawaf.h2iir_sumb;
        statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val = stats->stat.rawaf.v1iir_sumb;
        statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val = stats->stat.rawaf.v2iir_sumb;
        statsInt->afStats_stats.subWin.hw_af_luma_val     = stats->stat.rawaf.sumy_winb;
        statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val = stats->stat.rawaf.highlit_cnt_winb;
        for (int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
            for (int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[index] =
                    stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[index] =
                    stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[index] =
                    stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[index] =
                    stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                temp_luma = stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                statsInt->afStats_stats.mainWin.hw_af_luma_val[index] = MAX(temp_luma, 0);
                statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[index] =
                    stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                index++;
            }
        }
#else
        statsInt->af_stats_v3x.wndb_luma        = stats->stat.rawaf.sumy_winb;
        statsInt->af_stats_v3x.winb_highlit_cnt = stats->stat.rawaf.highlit_cnt_winb;
        statsInt->af_stats_v3x.wndb_fv_v1       = stats->stat.rawaf.v1iir_sumb;
        statsInt->af_stats_v3x.wndb_fv_v2       = stats->stat.rawaf.v2iir_sumb;
        statsInt->af_stats_v3x.wndb_fv_h1       = stats->stat.rawaf.h1iir_sumb;
        statsInt->af_stats_v3x.wndb_fv_h2       = stats->stat.rawaf.h2iir_sumb;
        for (int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
            for (int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                statsInt->af_stats_v3x.wnda_fv_v1[index] =
                    stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                statsInt->af_stats_v3x.wnda_fv_v2[index] =
                    stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                statsInt->af_stats_v3x.wnda_fv_h1[index] =
                    stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                statsInt->af_stats_v3x.wnda_fv_h2[index] =
                    stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                temp_luma = stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                temp_luma = (temp_luma - comp_bls) * max_val / (max_val - comp_bls);
                statsInt->af_stats_v3x.wnda_luma[index] = MAX(temp_luma, 0);
                statsInt->af_stats_v3x.wina_highlit_cnt[index] =
                    stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                index++;
            }
        }
#endif

        if (afParams) {
            statsInt->stat_motor.focusCode       = afParams->focusCode;
            statsInt->stat_motor.zoomCode        = afParams->zoomCode;
            statsInt->stat_motor.focus_endtim    = afParams->focusEndTim;
            statsInt->stat_motor.focus_starttim  = afParams->focusStartTim;
            statsInt->stat_motor.zoom_endtim     = afParams->zoomEndTim;
            statsInt->stat_motor.zoom_starttim   = afParams->zoomStartTim;
            statsInt->stat_motor.sof_tim         = afParams->sofTime;
            statsInt->stat_motor.focusCorrection = afParams->focusCorrection;
            statsInt->stat_motor.zoomCorrection  = afParams->zoomCorrection;
            statsInt->stat_motor.angleZ          = afParams->angleZ;
        }

        if (pStatsTrans->_expParams) statsInt->aecExpInfo = pStatsTrans->_expParams->aecExpInfo;
    }
#endif

    return ret;
}

#if RKAIQ_HAVE_DEHAZE_V14
XCamReturn translateAdehazeStatsV14(AiqStatsTranslator_t* pStatsTrans,
                                    const aiq_VideoBuffer_t* from, aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rkisp_adehaze_stats_t* statsInt = (rkisp_adehaze_stats_t*)to->_data;
    struct rkisp39_stat_buffer* stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));
    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", stats->frame_id, stats->meas_type);

    // dehaze
    to->bValid = stats->meas_type & ISP39_STAT_DHAZ;
    if (!to->bValid) return XCAM_RETURN_BYPASS;

    statsInt->dehaze_stats_v14.adp_wt       = stats->stat.dhaz.adp_wt;
    statsInt->dehaze_stats_v14.adp_air_base = stats->stat.dhaz.adp_air_base;
    statsInt->dehaze_stats_v14.adp_tmax     = stats->stat.dhaz.adp_tmax;
    for (int j = 0; j < ISP39_DHAZ_HIST_IIR_NUM; j++)
        for (int i = 0; i < ISP39_DHAZ_HIST_IIR_BLK_MAX; i++)
            statsInt->dehaze_stats_v14.hist_iir[i][j] = stats->stat.dhaz.hist_iir[i][j];

    return ret;
}

#endif

#if defined(RKAIQ_HAVE_BAYERTNR_V30)
XCamReturn translateBay3dStatsV30(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                  AiqList_t* to, AiqPool_t* pool, AiqMutex_t* lock) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct rkisp39_stat_buffer* stats =
        (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (stats == NULL) {
        LOGE_ANALYZER("Fail to get isp stats, ignore");
        return XCAM_RETURN_BYPASS;
    }

    LOGI_ANALYZER("camId: %d, isp stats: frame_id: %d, meas_type; 0x%x", pStatsTrans->mCamPhyId, stats->frame_id,
                  stats->meas_type);

    if (!(stats->meas_type & ISP39_STAT_BAY3D)) {
        LOGE_ANALYZER("bay3d stat is invalid, ignore");
        return XCAM_RETURN_BYPASS;
    }

    AiqPoolItem_t* pItem = aiqPool_getFree(pool);
    if (!pItem) {
        LOGE_ANALYZER("fid:%d no free dhazStats buf", stats->frame_id);
        return XCAM_RETURN_BYPASS;
    }

    aiq_stats_base_t* pbase        = (aiq_stats_base_t*)pItem->_pData;
    rk_aiq_bay3d_stat_t* bay3dStat = (rk_aiq_bay3d_stat_t*)pbase->_data;

    pbase->bValid   = true;
    pbase->frame_id = stats->frame_id;
    *bay3dStat      = stats->stat.bay3d;

    LOGV_ANALYZER(
        "camId: %d, tnr stats: frame_id: %d, sigma count: %d, sigma_calc[0-3]: %d, %d, %d, %d",
        pStatsTrans->mCamPhyId, stats->frame_id, bay3dStat->tnr_auto_sigma_count,
        bay3dStat->tnr_auto_sigma_calc[0], bay3dStat->tnr_auto_sigma_calc[1],
        bay3dStat->tnr_auto_sigma_calc[2], bay3dStat->tnr_auto_sigma_calc[3]);

    aiqMutex_lock(lock);
    if (aiqList_size(to) > 4) {
        AiqListItem_t* pItem    = aiqList_get_item(to, NULL);
        aiq_stats_base_t* pStat = *(aiq_stats_base_t**)pItem->_pData;
        AIQ_REF_BASE_UNREF(&pStat->_ref_base);
        aiqList_erase_item_locked(to, pItem);
    }

    if (aiqList_push(to, &pbase)) {
        LOGE_ANALYZER("push err !");
        AIQ_REF_BASE_UNREF(&pbase->_ref_base);
    }
    aiqMutex_unlock(lock);

    return ret;
}
#endif

#if defined(ISP_HW_V39)
XCamReturn AiqStatsTranslator_init(AiqStatsTranslator_t* pStatsTrans) {
    pStatsTrans->translateAecStats          = translateAecStatsV39;
    pStatsTrans->translateAwbStats          = translateAwbStatsV39;
    pStatsTrans->translateAfStats           = translateAfStatsV39;
#if RKAIQ_HAVE_DEHAZE_V14
    pStatsTrans->translateAdehazeStats      = translateAdehazeStatsV14;
#endif
    pStatsTrans->translateAgainStats        = NULL;
#if RKAIQ_HAVE_PDAF
    pStatsTrans->translatePdafStats         = AiqStatsTranslator_translatePdafStats;
#endif
#if defined(RKAIQ_HAVE_BAYERTNR_V30)
    pStatsTrans->translateBay3dStats        = translateBay3dStatsV30;
#endif
#if defined(RKAIQ_HAVE_MULTIISP)
    pStatsTrans->translateMultiAecStats     = NULL;
    pStatsTrans->translateMultiAwbStats     = NULL;
    pStatsTrans->translateMultiAfStats      = NULL;
    pStatsTrans->translateMultiAdehazeStats = NULL;
#endif
    return XCAM_RETURN_NO_ERROR;
}

void AiqStatsTranslator_deinit(AiqStatsTranslator_t* pStatsTrans) {
    return;
}

#endif
