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

void JudgeWinLocation35(
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

void JudgeWinLocationVertical35(
    struct isp2x_window* ori_win,
    WinSplitMode* mode,
    Rectangle_t left_isp_rect_,
    Rectangle_t right_isp_rect_
) {

    if (ori_win->v_offs + ori_win->v_size <= left_isp_rect_.h) {
        *mode = TOP_MODE;
    } else if(ori_win->v_offs >= right_isp_rect_.y) {
        *mode = BOTTOM_MODE;
    } else {
        if ((ori_win->v_offs + ori_win->v_size / 2) <= left_isp_rect_.h
                && right_isp_rect_.y <= (ori_win->v_offs + ori_win->v_size / 2)) {
            *mode = TOP_AND_BOTTOM_MODE;
        }
        else {

            if ((ori_win->v_offs + ori_win->v_size / 2) < right_isp_rect_.y) {

                u16 v_size_tmp1 = left_isp_rect_.h - ori_win->v_offs;
                u16 v_size_tmp2 = (right_isp_rect_.y - ori_win->v_offs) * 2;

                if (abs(ori_win->v_size - v_size_tmp1) < abs(ori_win->v_size - v_size_tmp2))
                    *mode = TOP_MODE;
                else
                    *mode = TOP_AND_BOTTOM_MODE;
            }
            else {

                u16 v_size_tmp1 = ori_win->v_offs + ori_win->v_size - right_isp_rect_.y;
                u16 v_size_tmp2 = (ori_win->v_offs + ori_win->v_size - left_isp_rect_.h) * 2;

                if (abs(ori_win->v_size - v_size_tmp1) < abs(ori_win->v_size - v_size_tmp2))
                    *mode = BOTTOM_MODE;
                else
                    *mode = TOP_AND_BOTTOM_MODE;
            }
        }
    }
}

static void MergeAecBigWinStatsV35(
    struct isp33_rawae_stat* left,
    struct isp33_rawae_stat* right,
    struct isp33_rawae_stat* out,
    WinSplitMode             mode
) {
    switch (mode)
    {
    case LEFT_MODE:
        memcpy(out, left, sizeof(struct isp33_rawae_stat));
        break;
    case RIGHT_MODE:
        memcpy(out, right, sizeof(struct isp33_rawae_stat));
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

static void MergeAecBigWinStatsVerticalV35(
    struct isp33_rawae_stat* left,
    struct isp33_rawae_stat* right,
    struct isp33_rawae_stat* out,
    WinSplitMode             mode
) {
    switch (mode)
    {
    case TOP_MODE:
        memcpy(out, left, sizeof(struct isp33_rawae_stat));
        break;
    case BOTTOM_MODE:
        memcpy(out, right, sizeof(struct isp33_rawae_stat));
        break;
    case TOP_AND_BOTTOM_MODE:
        for (int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
            for (int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                if (j < ISP39_MEAN_BLK_Y_NUM / 2) {
                    out->blk_y[j].blk_x[i].r = (left->blk_y[2 * j].blk_x[i].r + left->blk_y[2 * j + 1].blk_x[i].r) / 2;
                    out->blk_y[j].blk_x[i].g = (left->blk_y[2 * j].blk_x[i].g + left->blk_y[2 * j + 1].blk_x[i].g) / 2;
                    out->blk_y[j].blk_x[i].b = (left->blk_y[2 * j].blk_x[i].b + left->blk_y[2 * j + 1].blk_x[i].b) / 2;
                } else if (j > ISP39_MEAN_BLK_Y_NUM / 2) {
                    out->blk_y[j].blk_x[i].r = (right->blk_y[2 * j - ISP39_MEAN_BLK_Y_NUM].blk_x[i].r +
                                               right->blk_y[2 * j + 1 - ISP39_MEAN_BLK_Y_NUM].blk_x[i].r) / 2;
                    out->blk_y[j].blk_x[i].g = (right->blk_y[2 * j - ISP39_MEAN_BLK_Y_NUM].blk_x[i].g +
                                               right->blk_y[2 * j + 1 - ISP39_MEAN_BLK_Y_NUM].blk_x[i].g) / 2;
                    out->blk_y[j].blk_x[i].b = (right->blk_y[2 * j - ISP39_MEAN_BLK_Y_NUM].blk_x[i].b +
                                               right->blk_y[2 * j + 1 - ISP39_MEAN_BLK_Y_NUM].blk_x[i].b) / 2;
                } else {
                    out->blk_y[j].blk_x[i].r = (left->blk_y[ISP39_MEAN_BLK_X_NUM - 1].blk_x[i].r + right->blk_y[0].blk_x[i].r) / 2;
                    out->blk_y[j].blk_x[i].g = (left->blk_y[ISP39_MEAN_BLK_X_NUM - 1].blk_x[i].g + right->blk_y[0].blk_x[i].g) / 2;
                    out->blk_y[j].blk_x[i].b = (left->blk_y[ISP39_MEAN_BLK_X_NUM - 1].blk_x[i].b + right->blk_y[0].blk_x[i].b) / 2;
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

#if defined(ISP_HW_V35) && defined(USE_NEWSTRUCT)

/* for bnr20bit, rawae0 meas LSB(0-9bit) rawae3 meas MSB(10-19bit) */
/* for bnr20bit, rawhist0 meas LSB(4-11bit) rawhist3 meas MSB(12-19bit) */

XCamReturn Aec20BitGirdMerge
(
    struct isp33_rawae_stat LSB,
    struct isp33_rawae_stat MSB,
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


void calcAecBigWinStatsV35(
    struct isp33_rawae_stat*    stats_in,
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

}

void calcAecHistBinStatsV35(
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
translateMultiAecStatsV35(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                          aiq_stats_base_t* to) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V35) && defined(RKAIQ_HAVE_MULTIISP)
    struct isp35_isp_meas_cfg* isp_params = &pStatsTrans->_ispParams->meas;
    uint8_t AeSwapMode, AeSelMode, Bnr20bitEn;
    AeSwapMode             = isp_params->rawae0.rawae_sel & 0x0f;
    AeSelMode              = isp_params->rawae3.rawae_sel & 0x0f;
    Bnr20bitEn             = isp_params->rawae0.rawae_sel & 0xf0;
    unsigned int meas_type = 0;
    struct isp2x_window ori_win;

    RkAiqIspUniteMode ispUniteMode = pStatsTrans->mIspUniteMode;

    WinSplitMode AeWinSplitMode[2] = {LEFT_AND_RIGHT_MODE}; //0:rawae0 1:rawae3
    WinSplitMode HistWinSplitMode[2] = {LEFT_AND_RIGHT_MODE}; //0:rawhist0 1:rawhist3

    WinSplitMode AeWinSplitModeV[2] = {LEFT_AND_RIGHT_MODE}; //0:rawae0 1:rawae3
    WinSplitMode HistWinSplitModeV[2] = {LEFT_AND_RIGHT_MODE}; //0:rawhist0 1:rawhist3

    ori_win.h_offs = isp_params->rawae0.win0_h_offset;
    ori_win.v_offs = isp_params->rawae0.win0_v_offset;
    ori_win.h_size = isp_params->rawae0.win0_h_size;
    ori_win.v_size = isp_params->rawae0.win0_v_size;
    JudgeWinLocation35(&ori_win, &AeWinSplitMode[0], pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);

    ori_win.h_offs = isp_params->rawae3.win0_h_offset;
    ori_win.v_offs = isp_params->rawae3.win0_v_offset;
    ori_win.h_size = isp_params->rawae3.win0_h_size;
    ori_win.v_size = isp_params->rawae3.win0_v_size;
    JudgeWinLocation35(&ori_win, &AeWinSplitMode[1], pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);

    ori_win.h_offs = isp_params->rawhist0.h_offset;
    ori_win.v_offs = isp_params->rawhist0.v_offset;
    ori_win.h_size = isp_params->rawhist0.h_size;
    ori_win.v_size = isp_params->rawhist0.v_size;
    JudgeWinLocation35(&ori_win, &HistWinSplitMode[0], pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);

    ori_win.h_offs = isp_params->rawhist3.h_offset;
    ori_win.v_offs = isp_params->rawhist3.v_offset;
    ori_win.h_size = isp_params->rawhist3.h_size;
    ori_win.v_size = isp_params->rawhist3.v_size;
    JudgeWinLocation35(&ori_win, &HistWinSplitMode[1], pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);

    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        ori_win.h_offs = isp_params->rawae0.win0_h_offset;
        ori_win.v_offs = isp_params->rawae0.win0_v_offset;
        ori_win.h_size = isp_params->rawae0.win0_h_size;
        ori_win.v_size = isp_params->rawae0.win0_v_size;
        JudgeWinLocationVertical35(&ori_win, &AeWinSplitModeV[0], pStatsTrans->left_isp_rect_, pStatsTrans->bottom_left_isp_rect_);

        ori_win.h_offs = isp_params->rawae3.win0_h_offset;
        ori_win.v_offs = isp_params->rawae3.win0_v_offset;
        ori_win.h_size = isp_params->rawae3.win0_h_size;
        ori_win.v_size = isp_params->rawae3.win0_v_size;
        JudgeWinLocationVertical35(&ori_win, &AeWinSplitModeV[1], pStatsTrans->left_isp_rect_, pStatsTrans->bottom_left_isp_rect_);

        ori_win.h_offs = isp_params->rawhist0.h_offset;
        ori_win.v_offs = isp_params->rawhist0.v_offset;
        ori_win.h_size = isp_params->rawhist0.h_size;
        ori_win.v_size = isp_params->rawhist0.v_size;
        JudgeWinLocationVertical35(&ori_win, &HistWinSplitModeV[0], pStatsTrans->left_isp_rect_, pStatsTrans->bottom_left_isp_rect_);

        ori_win.h_offs = isp_params->rawhist3.h_offset;
        ori_win.v_offs = isp_params->rawhist3.v_offset;
        ori_win.h_size = isp_params->rawhist3.h_size;
        ori_win.v_size = isp_params->rawhist3.v_size;
        JudgeWinLocationVertical35(&ori_win, &HistWinSplitModeV[1], pStatsTrans->left_isp_rect_, pStatsTrans->bottom_left_isp_rect_);
    } else {
        for (int i = 0; i < 2; i++) {
            AeWinSplitModeV[i] = TOP_MODE;
            HistWinSplitModeV[i] = TOP_MODE;
        }
    }

    // ae_stats = (ae_ori_stats_u12/10 - ob_offset_u9 - bls1_val_u12) * awb1_gain_u16 * range_ratio
    struct isp35_bls_cfg* bls_cfg           = &pStatsTrans->_ispParams->bls_cfg;
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

    struct rkisp35_stat_buffer* left_stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    uint32_t bufLen = AiqV4l2Buffer_getV4lBufLength((AiqV4l2Buffer_t*)from);
    uint32_t bufCnt = bufLen / sizeof(struct rkisp35_stat_buffer) > 4 ? 4 : bufLen / sizeof(struct rkisp35_stat_buffer);
    uint32_t bufSize = bufLen / bufCnt;
    struct rkisp35_stat_buffer* right_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize);

    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    struct rkisp35_stat_buffer* bottom_left_stats = NULL;
    struct rkisp35_stat_buffer* bottom_right_stats = NULL;
    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        bottom_left_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize * 2);
        if (bottom_left_stats == NULL) {
            LOGE("fail to get stats ,ignore\n");
            return XCAM_RETURN_BYPASS;
        }

        bottom_right_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize * 3);
        if(bottom_right_stats == NULL) {
            LOGE("fail to get right stats ,ignore\n");
            return XCAM_RETURN_BYPASS;
        }
    } else {
        bottom_left_stats = left_stats;
        bottom_right_stats = right_stats;
    }

    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  left_stats->frame_id, left_stats->meas_type);

    if(left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type ||
       bottom_left_stats->frame_id != bottom_right_stats->frame_id || bottom_left_stats->meas_type != bottom_right_stats->meas_type ||
       left_stats->frame_id != bottom_left_stats->frame_id || left_stats->meas_type != bottom_left_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
        LOGE_ANALYZER("top left id %d meas %x right id %d meas %x bottom left id %d meas %x right id %d meas %x mode %d",
                      left_stats->frame_id, left_stats->meas_type,
                      right_stats->frame_id, right_stats->meas_type,
                      bottom_left_stats->frame_id, bottom_left_stats->meas_type,
                      bottom_right_stats->frame_id, bottom_right_stats->meas_type,
                      ispUniteMode);
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
            MergeAecBigWinStatsV35(&left_stats->stat.rawae0, &right_stats->stat.rawae0, &left_stats->stat.rawae0, AeWinSplitMode[0]);

        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
            if (AeWinSplitMode[0] != LEFT_MODE)
                MergeAecBigWinStatsV35(&bottom_left_stats->stat.rawae0, &bottom_right_stats->stat.rawae0, &bottom_left_stats->stat.rawae0, AeWinSplitMode[0]);

            if (AeWinSplitModeV[0] != TOP_MODE)
                MergeAecBigWinStatsVerticalV35(&left_stats->stat.rawae0, &bottom_left_stats->stat.rawae0, &left_stats->stat.rawae0, AeWinSplitModeV[0]);
        }
        //RAWAE0 BIG-MODE
        calcAecBigWinStatsV35(&left_stats->stat.rawae0, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin,
                              bls1_val, awb1_gain);

        //RAWHIST0 BIG-MODE
        rawhist_mode = isp_params->rawhist0.mode;
        MergeAecHistBinStats(left_stats->stat.rawhist0.bin,
                             right_stats->stat.rawhist0.bin,
                             statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val,
                             HistWinSplitMode[0], rawhist_mode,
                             isp_ob_offset_rb, isp_ob_offset_g,
                             bls1_ori_val, awb1_gain, is_hdr);

        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
            aeStats_histStats_t bottom_stats;
            MergeAecHistBinStats(bottom_left_stats->stat.rawhist0.bin, bottom_right_stats->stat.rawhist0.bin,
                                 bottom_stats.hw_ae_histBin_val,
                                 HistWinSplitModeV[0], rawhist_mode,
                                 isp_ob_offset_rb, isp_ob_offset_g,
                                 bls1_ori_val, awb1_gain, is_hdr);

            switch (HistWinSplitModeV[0])
            {
            case TOP_MODE:
                break;
            case BOTTOM_MODE:
                for (int i = 0; i < ISP32L_HIST_LITE_BIN_N_MAX; i++)
                    statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i] = bottom_stats.hw_ae_histBin_val[i];
                break;
            case TOP_AND_BOTTOM_MODE:
                for (int i = 0; i < ISP32L_HIST_LITE_BIN_N_MAX; i++)
                    statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i] += bottom_stats.hw_ae_histBin_val[i];
                break;
            default:
                break;
            }
        }
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
            MergeAecBigWinStatsV35(&left_stats->stat.rawae3, &right_stats->stat.rawae3, &left_stats->stat.rawae3, AeWinSplitMode[0]);

        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
            if (AeWinSplitMode[0] != LEFT_MODE)
                MergeAecBigWinStatsV35(&bottom_left_stats->stat.rawae3, &bottom_right_stats->stat.rawae3, &bottom_left_stats->stat.rawae3, AeWinSplitMode[0]);

            if (AeWinSplitModeV[0] != TOP_MODE)
                MergeAecBigWinStatsVerticalV35(&left_stats->stat.rawae3, &bottom_left_stats->stat.rawae3, &left_stats->stat.rawae3, AeWinSplitModeV[0]);
        }

        calcAecBigWinStatsV35(&left_stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin,
                              bls1_val, awb1_gain);


        rawhist_mode = isp_params->rawhist3.mode;
        MergeAecHistBinStats(left_stats->stat.rawhist3.bin,
                             right_stats->stat.rawhist3.bin,
                             statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val,
                             HistWinSplitMode[1], rawhist_mode,
                             isp_ob_offset_rb, isp_ob_offset_g,
                             bls1_ori_val, awb1_gain, is_hdr);

        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
            aeStats_histStats_t bottom_stats;
            MergeAecHistBinStats(bottom_left_stats->stat.rawhist3.bin, bottom_right_stats->stat.rawhist3.bin,
                                 bottom_stats.hw_ae_histBin_val,
                                 HistWinSplitModeV[1], rawhist_mode,
                                 isp_ob_offset_rb, isp_ob_offset_g,
                                 bls1_ori_val, awb1_gain, is_hdr);

            switch (HistWinSplitModeV[1])
            {
            case TOP_MODE:
                break;
            case BOTTOM_MODE:
                for (int i = 0; i < ISP32L_HIST_LITE_BIN_N_MAX; i++)
                    statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i] = bottom_stats.hw_ae_histBin_val[i];
                break;
            case TOP_AND_BOTTOM_MODE:
                for (int i = 0; i < ISP32L_HIST_LITE_BIN_N_MAX; i++)
                    statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i] += bottom_stats.hw_ae_histBin_val[i];
                break;
            default:
                break;
            }
        }
    }

    if (Bnr20bitEn > 0 && pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_coWk_mode) {

        if (AeWinSplitMode[0] != LEFT_MODE) {
            MergeAecBigWinStatsV35(&left_stats->stat.rawae0, &right_stats->stat.rawae0, &left_stats->stat.rawae0, AeWinSplitMode[0]);
            MergeAecBigWinStatsV35(&left_stats->stat.rawae3, &right_stats->stat.rawae3, &left_stats->stat.rawae3, AeWinSplitMode[0]);
        }
        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
            if (AeWinSplitMode[0] != LEFT_MODE) {
                MergeAecBigWinStatsV35(&bottom_left_stats->stat.rawae0, &bottom_right_stats->stat.rawae0, &bottom_left_stats->stat.rawae0, AeWinSplitMode[0]);
                MergeAecBigWinStatsV35(&bottom_left_stats->stat.rawae3, &bottom_right_stats->stat.rawae3, &bottom_left_stats->stat.rawae3, AeWinSplitMode[0]);
            }

            if (AeWinSplitModeV[0] != TOP_MODE) {
                MergeAecBigWinStatsVerticalV35(&left_stats->stat.rawae0, &bottom_left_stats->stat.rawae0, &left_stats->stat.rawae0, AeWinSplitModeV[0]);
                MergeAecBigWinStatsVerticalV35(&left_stats->stat.rawae3, &bottom_left_stats->stat.rawae3, &left_stats->stat.rawae3, AeWinSplitModeV[0]);
            }
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

        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
            if (AeWinSplitMode[0] != LEFT_MODE) {
                MergeAecHistBinStats(bottom_left_stats->stat.rawhist0.bin, bottom_right_stats->stat.rawhist0.bin,
                                     bottom_left_stats->stat.rawhist0.bin, AeWinSplitMode[0],
                                     rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                     bls1_ori_val, awb1_gain, true);
                MergeAecHistBinStats(bottom_left_stats->stat.rawhist3.bin, bottom_right_stats->stat.rawhist3.bin,
                                     bottom_left_stats->stat.rawhist3.bin, AeWinSplitMode[0],
                                     rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                     bls1_ori_val, awb1_gain, true);
            }

            if (AeWinSplitModeV[0] != TOP_MODE) {
                MergeAecHistBinStats(left_stats->stat.rawhist0.bin, bottom_left_stats->stat.rawhist0.bin,
                                     left_stats->stat.rawhist0.bin, AeWinSplitModeV[0],
                                     rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                     bls1_ori_val, awb1_gain, true);
                MergeAecHistBinStats(left_stats->stat.rawhist3.bin, bottom_left_stats->stat.rawhist3.bin,
                                     left_stats->stat.rawhist3.bin, AeWinSplitModeV[0],
                                     rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                     bls1_ori_val, awb1_gain, true);
            }
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
        calcAecHistBinStatsV35(bin, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val, \
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

XCamReturn translateAecStatsV35(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V35)

    if (pStatsTrans->mIsMultiIsp && pStatsTrans->mIspUniteMode) {
        return translateMultiAecStatsV35(pStatsTrans, from, to);
    }

    // 0) blc awb cfg
    struct isp35_isp_meas_cfg* isp_params = &pStatsTrans->_ispParams->meas;
    uint8_t AeSwapMode, AeSelMode, Bnr20bitEn;
    AeSwapMode             = isp_params->rawae0.rawae_sel & 0x0f;
    AeSelMode              = isp_params->rawae3.rawae_sel & 0x0f;
    Bnr20bitEn             = isp_params->rawae0.rawae_sel & 0xf0;
    unsigned int meas_type = 0;

    // ae_stats = (ae_ori_stats_u12/10 - ob_offset_u9 - bls1_val_u12) * awb1_gain_u16 * range_ratio
    struct isp35_bls_cfg* bls_cfg           = &pStatsTrans->_ispParams->bls_cfg;
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
    struct rkisp35_stat_buffer* stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

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

    // ae stats v3.5
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
        calcAecBigWinStatsV35(&stats->stat.rawae0, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin,
                              bls1_val, awb1_gain);

        pixel_num = pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_width \
                    * pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_height;

        //For isp35, only has one sub win
        statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerR_val = CLIP((s64)((u64)stats->stat.rawae0.wnd1_sumr * awb1_gain.r / 256 - (pixel_num >> 2) * bls1_val.r), 0, MAX_29BITS);
        statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerGrGb_val = CLIP((s64)((u64)stats->stat.rawae0.wnd1_sumg * awb1_gain.gr / 256 - (pixel_num >> 1) * bls1_val.gb), 0, MAX_32BITS);
        statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerB_val = CLIP((s64)((u64)stats->stat.rawae0.wnd1_sumb * awb1_gain.b / 256 - (pixel_num >> 2) * bls1_val.b), 0, MAX_29BITS);

        //RAWHIST0 BIG-MODE
        rawhist_mode = isp_params->rawhist0.mode;

        calcAecHistBinStatsV35(stats->stat.rawhist0.bin,
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
            calcAecBigWinStatsV35(&stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin,
                                  bls1_val, awb1_gain);

            pixel_num = pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_width \
                        * pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_height;

            //For isp35, only has one sub win
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerR_val = CLIP((s64)((u64)stats->stat.rawae3.wnd1_sumr * awb1_gain.r / 256 - (pixel_num >> 2) * bls1_val.r), 0, MAX_29BITS);
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerGrGb_val = CLIP((s64)((u64)stats->stat.rawae3.wnd1_sumg * awb1_gain.gr / 256 - (pixel_num >> 1) * bls1_val.gb), 0, MAX_32BITS);
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerB_val = CLIP((s64)((u64)stats->stat.rawae3.wnd1_sumb * awb1_gain.b / 256 - (pixel_num >> 2) * bls1_val.b), 0, MAX_29BITS);

            //RAWHIST3 BIG-MODE
            rawhist_mode = isp_params->rawhist3.mode;
            calcAecHistBinStatsV35(stats->stat.rawhist3.bin,
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

            calcAecBigWinStatsV35(&stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin,
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
        calcAecHistBinStatsV35(bin, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val, \
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

#endif

static void MergeAwbWinStats(
    awbStats_wpEngineStats_t *merge_stats,
    struct isp33_rawawb_stat *left_stats,
    struct isp33_rawawb_stat *right_stats,
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

static void MergeAwbWinStatsV(
    awbStats_wpEngineStats_t *merge_stats,
    awbStats_wpEngineStats_t *left_stats,
    awbStats_wpEngineStats_t *right_stats,
    int lightNum,
    WinSplitMode mode
) {
    switch(mode) {
    case LEFT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats->norWp[i].hw_awbCfg_rGainSum_val =
                left_stats->norWp[i].hw_awbCfg_rGainSum_val;
            merge_stats->norWp[i].hw_awbCfg_bGainSum_val =
                left_stats->norWp[i].hw_awbCfg_bGainSum_val;
            merge_stats->norWp[i].hw_awbCfg_statsWp_count =
                left_stats->norWp[i].hw_awbCfg_statsWp_count;
            merge_stats->bigWp[i].hw_awbCfg_rGainSum_val =
                left_stats->bigWp[i].hw_awbCfg_rGainSum_val;
            merge_stats->bigWp[i].hw_awbCfg_bGainSum_val =
                left_stats->bigWp[i].hw_awbCfg_bGainSum_val;
            merge_stats->bigWp[i].hw_awbCfg_statsWp_count =
                left_stats->bigWp[i].hw_awbCfg_statsWp_count;
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats->norWp[i].hw_awbCfg_rGainSum_val =
                right_stats->norWp[i].hw_awbCfg_rGainSum_val;
            merge_stats->norWp[i].hw_awbCfg_bGainSum_val =
                right_stats->norWp[i].hw_awbCfg_bGainSum_val;
            merge_stats->norWp[i].hw_awbCfg_statsWp_count =
                right_stats->norWp[i].hw_awbCfg_statsWp_count;
            merge_stats->bigWp[i].hw_awbCfg_rGainSum_val =
                right_stats->bigWp[i].hw_awbCfg_rGainSum_val;
            merge_stats->bigWp[i].hw_awbCfg_bGainSum_val =
                right_stats->bigWp[i].hw_awbCfg_bGainSum_val;
            merge_stats->bigWp[i].hw_awbCfg_statsWp_count =
                right_stats->bigWp[i].hw_awbCfg_statsWp_count;
        }
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats->norWp[i].hw_awbCfg_rGainSum_val =
                left_stats->norWp[i].hw_awbCfg_rGainSum_val + right_stats->norWp[i].hw_awbCfg_rGainSum_val;
            merge_stats->norWp[i].hw_awbCfg_bGainSum_val =
                left_stats->norWp[i].hw_awbCfg_bGainSum_val + right_stats->norWp[i].hw_awbCfg_bGainSum_val;
            merge_stats->norWp[i].hw_awbCfg_statsWp_count =
                left_stats->norWp[i].hw_awbCfg_statsWp_count + right_stats->norWp[i].hw_awbCfg_statsWp_count;
            merge_stats->bigWp[i].hw_awbCfg_rGainSum_val =
                left_stats->bigWp[i].hw_awbCfg_rGainSum_val + right_stats->bigWp[i].hw_awbCfg_rGainSum_val;
            merge_stats->bigWp[i].hw_awbCfg_bGainSum_val =
                left_stats->bigWp[i].hw_awbCfg_bGainSum_val + right_stats->bigWp[i].hw_awbCfg_bGainSum_val;
            merge_stats->bigWp[i].hw_awbCfg_statsWp_count =
                left_stats->bigWp[i].hw_awbCfg_statsWp_count + right_stats->bigWp[i].hw_awbCfg_statsWp_count;
        }
        break;
    default:
        break;
    }
}

static void MergeAwbBlkStats(
    awbStats_pixStats_t *merge_stats,
    struct isp33_rawawb_stat *left_stats,
    struct isp33_rawawb_stat *right_stats,
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

static void MergeAwbBlkStatsV(
    awbStats_pixStats_t *merge_stats,
    awbStats_pixStats_t *left_stats,
    awbStats_pixStats_t *right_stats,
    WinSplitMode mode
) {
    int index = 0;
    int line_idx = 0;

    switch(mode) {
    case LEFT_MODE:
        memcpy(merge_stats, left_stats, sizeof(awbStats_pixStats_t) * ISP39_MEAN_BLK_Y_NUM * ISP39_MEAN_BLK_X_NUM);
        break;
    case RIGHT_MODE:
        memcpy(merge_stats, right_stats, sizeof(awbStats_pixStats_t) * ISP39_MEAN_BLK_Y_NUM * ISP39_MEAN_BLK_X_NUM);
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
            for(int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                index = i * ISP39_MEAN_BLK_Y_NUM + j;
                line_idx = i * ISP39_MEAN_BLK_X_NUM;
                if (j < ISP39_MEAN_BLK_X_NUM / 2) {
                    merge_stats[index].hw_awbCfg_rSum_val = left_stats[line_idx + j * 2].hw_awbCfg_rSum_val + left_stats[line_idx + j * 2 + 1].hw_awbCfg_rSum_val;
                    merge_stats[index].hw_awbCfg_gSum_val = left_stats[line_idx + j * 2].hw_awbCfg_gSum_val + left_stats[line_idx + j * 2 + 1].hw_awbCfg_gSum_val;
                    merge_stats[index].hw_awbCfg_bSum_val = left_stats[line_idx + j * 2].hw_awbCfg_bSum_val + left_stats[line_idx + j * 2 + 1].hw_awbCfg_bSum_val;
                    merge_stats[index].hw_awbCfg_statsPix_count = left_stats[line_idx + j * 2].hw_awbCfg_statsPix_count + left_stats[line_idx + j * 2 + 1].hw_awbCfg_statsPix_count;
                } else if (j > ISP39_MEAN_BLK_X_NUM / 2) {
                    merge_stats[index].hw_awbCfg_rSum_val = right_stats[line_idx + j * 2 - ISP39_MEAN_BLK_X_NUM].hw_awbCfg_rSum_val + right_stats[line_idx + j * 2 - ISP39_MEAN_BLK_X_NUM + 1].hw_awbCfg_rSum_val;
                    merge_stats[index].hw_awbCfg_gSum_val = right_stats[line_idx + j * 2 - ISP39_MEAN_BLK_X_NUM].hw_awbCfg_gSum_val + right_stats[line_idx + j * 2 - ISP39_MEAN_BLK_X_NUM + 1].hw_awbCfg_gSum_val;
                    merge_stats[index].hw_awbCfg_bSum_val = right_stats[line_idx + j * 2 - ISP39_MEAN_BLK_X_NUM].hw_awbCfg_bSum_val + right_stats[line_idx + j * 2 - ISP39_MEAN_BLK_X_NUM + 1].hw_awbCfg_bSum_val;
                    merge_stats[index].hw_awbCfg_statsPix_count = right_stats[line_idx + j * 2 - ISP39_MEAN_BLK_X_NUM].hw_awbCfg_statsPix_count + right_stats[line_idx + j * 2 - ISP39_MEAN_BLK_X_NUM + 1].hw_awbCfg_statsPix_count;
                } else {
                    merge_stats[index].hw_awbCfg_rSum_val = left_stats[line_idx + ISP39_MEAN_BLK_X_NUM - 1].hw_awbCfg_rSum_val + right_stats[line_idx].hw_awbCfg_rSum_val;
                    merge_stats[index].hw_awbCfg_gSum_val = left_stats[line_idx + ISP39_MEAN_BLK_X_NUM - 1].hw_awbCfg_gSum_val + right_stats[line_idx].hw_awbCfg_gSum_val;
                    merge_stats[index].hw_awbCfg_bSum_val = left_stats[line_idx + ISP39_MEAN_BLK_X_NUM - 1].hw_awbCfg_bSum_val + right_stats[line_idx].hw_awbCfg_bSum_val;
                    merge_stats[index].hw_awbCfg_statsPix_count = left_stats[line_idx + ISP39_MEAN_BLK_X_NUM - 1].hw_awbCfg_statsPix_count + right_stats[line_idx].hw_awbCfg_statsPix_count;
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

static void MergeAwbHistBinStatsV(
    uint32_t *merge_stats,
    uint32_t *top_stats,
    uint32_t *bottom_stats,
    WinSplitMode mode
) {
    u32 tmp1, tmp2;
    switch(mode) {
    case TOP_MODE:
        memcpy(merge_stats, top_stats, sizeof(uint32_t) * RK_AIQ_AWB_WP_HIST_BIN_NUM);
        break;
    case BOTTOM_MODE:
        memcpy(merge_stats, bottom_stats, sizeof(uint32_t) * RK_AIQ_AWB_WP_HIST_BIN_NUM);
        break;
    case TOP_AND_BOTTOM_MODE:
        for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
            merge_stats[i] = top_stats[i] + bottom_stats[i];
        }
        break;
    }
}

static void MergeAwbExcWpStats(
    awbStats_wpStats_t*merge_stats,
    struct isp33_rawawb_stat *left_stats,
    struct isp33_rawawb_stat *right_stats,
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

static void MergeAwbExcWpStatsV(
    awbStats_wpStats_t *merge_stats,
    awbStats_wpStats_t *left_stats,
    awbStats_wpStats_t *right_stats,
    WinSplitMode mode
) {
    switch(mode) {
    case LEFT_MODE:
        memcpy(merge_stats, left_stats, sizeof(awbStats_wpStats_t) * RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201);
        break;
    case RIGHT_MODE:
        memcpy(merge_stats, right_stats, sizeof(awbStats_wpStats_t) * RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201);
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
            merge_stats[i].hw_awbCfg_rGainSum_val = left_stats[i].hw_awbCfg_rGainSum_val + right_stats[i].hw_awbCfg_rGainSum_val;
            merge_stats[i].hw_awbCfg_bGainSum_val = left_stats[i].hw_awbCfg_bGainSum_val + right_stats[i].hw_awbCfg_bGainSum_val;
            merge_stats[i].hw_awbCfg_statsWp_count = left_stats[i].hw_awbCfg_statsWp_count + right_stats[i].hw_awbCfg_statsWp_count;
        }
        break;
    }
}

XCamReturn translateMultiAwbStatsV35(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                     aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V35)
    struct rkisp35_stat_buffer* left_stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    aiq_awb_stats_wrapper_t* statsInt = (aiq_awb_stats_wrapper_t*)to->_data;

    if(left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    RkAiqIspUniteMode ispUniteMode = pStatsTrans->mIspUniteMode;

    uint32_t bufLen = AiqV4l2Buffer_getV4lBufLength((AiqV4l2Buffer_t*)from);
    uint32_t bufCnt = bufLen / sizeof(struct rkisp35_stat_buffer) > 4 ? 4 : bufLen / sizeof(struct rkisp35_stat_buffer);
    uint32_t bufSize = bufLen / bufCnt;
    struct rkisp35_stat_buffer* right_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize);
    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    awbStats_stats_t *bottom_awb_stats_v39 = NULL;
    struct rkisp35_stat_buffer* bottom_left_stats = NULL;
    struct rkisp35_stat_buffer* bottom_right_stats = NULL;
    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        bottom_left_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize * 2);
        if (bottom_left_stats == NULL) {
            LOGE("fail to get stats ,ignore\n");
            return XCAM_RETURN_BYPASS;
        }

        bottom_right_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize * 3);
        if(bottom_right_stats == NULL) {
            LOGE("fail to get right stats ,ignore\n");
            return XCAM_RETURN_BYPASS;
        }

        if (pStatsTrans->_bottom_mge_awb_stats == NULL) {
            pStatsTrans->_bottom_mge_awb_stats = aiq_mallocz(sizeof(awbStats_stats_t));
            if (pStatsTrans->_bottom_mge_awb_stats == NULL) {
                LOGE_ANALYZER("fail to malloc pStatsTrans->_bottom_mge_awb_stats");
                return XCAM_RETURN_BYPASS;
            }
        }
        bottom_awb_stats_v39 = (awbStats_stats_t*)pStatsTrans->_bottom_mge_awb_stats;
        memset(bottom_awb_stats_v39, 0, sizeof(awbStats_stats_t));
    } else {
        bottom_left_stats = left_stats;
        bottom_right_stats = right_stats;
    }

    if(left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type ||
       bottom_left_stats->frame_id != bottom_right_stats->frame_id || bottom_left_stats->meas_type != bottom_right_stats->meas_type ||
       left_stats->frame_id != bottom_left_stats->frame_id || left_stats->meas_type != bottom_left_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
        LOGE_ANALYZER("top left id %d meas %x right id %d meas %x bottom left id %d meas %x right id %d meas %x mode %d",
                      left_stats->frame_id, left_stats->meas_type,
                      right_stats->frame_id, right_stats->meas_type,
                      bottom_left_stats->frame_id, bottom_left_stats->meas_type,
                      bottom_right_stats->frame_id, bottom_right_stats->meas_type,
                      ispUniteMode);
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
        pStatsTrans->_ispParams->awb_cfg_v35.com.pixEngine.hw_awbCfg_zoneStatsSrc_mode>awbStats_pixAll_mode;
    statsInt->awb_stats_v39.awb_cfg_effect.mode = pStatsTrans->_ispParams->awb_cfg_v35.mode;
    statsInt->awb_stats_v39.awb_cfg_effect.lightNum = pStatsTrans->_ispParams->awb_cfg_v35.com.wpEngine.hw_awbCfg_lightSrcNum_val;
    statsInt->awb_stats_v39.awb_cfg_effect.groupIllIndxCurrent = pStatsTrans->_ispParams->awb_cfg_v35.groupIllIndxCurrent;
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent, pStatsTrans->_ispParams->awb_cfg_v35.IllIndxSetCurrent,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.timeSign, pStatsTrans->_ispParams->awb_cfg_v35.timeSign,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.timeSign));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.preWbgainSw, pStatsTrans->_ispParams->awb_cfg_v35.preWbgainSw,
           sizeof(pStatsTrans->_ispParams->awb_cfg_v35.preWbgainSw));
    statsInt->awb_cfg_effect_valid = true;

    awbStats_stats_t *awb_stats_v39 = &statsInt->awb_stats_v39.com;

    WinSplitMode AwbWinSplitMode = LEFT_AND_RIGHT_MODE;
    WinSplitMode AwbWinSplitModeV = TOP_AND_BOTTOM_MODE;

    struct isp2x_window ori_win;
    ori_win.h_offs = pStatsTrans->_ispParams->meas.rawawb.h_offs;
    ori_win.h_size = pStatsTrans->_ispParams->meas.rawawb.h_size;
    ori_win.v_offs = pStatsTrans->_ispParams->meas.rawawb.v_offs;
    ori_win.v_size = pStatsTrans->_ispParams->meas.rawawb.v_size;

    JudgeWinLocation35(&ori_win, &AwbWinSplitMode, pStatsTrans->left_isp_rect_, pStatsTrans->right_isp_rect_);
    JudgeWinLocationVertical35(&ori_win, &AwbWinSplitModeV, pStatsTrans->left_isp_rect_, pStatsTrans->bottom_left_isp_rect_);
    MergeAwbWinStats( &awb_stats_v39->wpEngine, &left_stats->stat.rawawb, &right_stats->stat.rawawb,
                     statsInt->awb_stats_v39.awb_cfg_effect.lightNum, AwbWinSplitMode);

    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        MergeAwbWinStats(&bottom_awb_stats_v39->wpEngine, &bottom_left_stats->stat.rawawb, &bottom_right_stats->stat.rawawb,
                         statsInt->awb_stats_v39.awb_cfg_effect.lightNum, AwbWinSplitMode);

        MergeAwbWinStatsV(&awb_stats_v39->wpEngine, &awb_stats_v39->wpEngine, &bottom_awb_stats_v39->wpEngine,
                          statsInt->awb_stats_v39.awb_cfg_effect.lightNum, AwbWinSplitModeV);
    }

    MergeAwbBlkStats(awb_stats_v39->pixEngine.zonePix, &left_stats->stat.rawawb,
                     &right_stats->stat.rawawb, AwbWinSplitMode);

    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        MergeAwbBlkStats(bottom_awb_stats_v39->pixEngine.zonePix, &bottom_left_stats->stat.rawawb,
                         &bottom_right_stats->stat.rawawb, AwbWinSplitMode);

        MergeAwbBlkStatsV(awb_stats_v39->pixEngine.zonePix, awb_stats_v39->pixEngine.zonePix,
                          bottom_awb_stats_v39->pixEngine.zonePix, AwbWinSplitModeV);
    }

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
    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        MergeAwbHistBinStats(bottom_awb_stats_v39->wpEngine.hw_awb_wpHistBin_val, bottom_left_stats->stat.rawawb.yhist, bottom_right_stats->stat.rawawb.yhist, AwbWinSplitMode);
        MergeAwbHistBinStatsV(awb_stats_v39->wpEngine.hw_awb_wpHistBin_val, awb_stats_v39->wpEngine.hw_awb_wpHistBin_val,
                              bottom_awb_stats_v39->wpEngine.hw_awb_wpHistBin_val, AwbWinSplitModeV);
    }

    switch(AwbWinSplitMode) {
    case LEFT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
            awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = left_stats->stat.rawawb.sum[i].wp_num2;
            if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                bottom_awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = bottom_left_stats->stat.rawawb.sum[i].wp_num2;
            }
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
            awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = right_stats->stat.rawawb.sum[i].wp_num2;
            if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                bottom_awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = bottom_right_stats->stat.rawawb.sum[i].wp_num2;
            }
        }
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
            awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = left_stats->stat.rawawb.sum[i].wp_num2 + right_stats->stat.rawawb.sum[i].wp_num2;
            if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                bottom_awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = bottom_left_stats->stat.rawawb.sum[i].wp_num2 + bottom_right_stats->stat.rawawb.sum[i].wp_num2;
            }
        }
        break;
    default:
        break;
    }

    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        switch(AwbWinSplitModeV) {
        case TOP_MODE:
            break;
        case BOTTOM_MODE:
            for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
                awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] = bottom_awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i];
            }
            break;
        case TOP_AND_BOTTOM_MODE:
            for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
                awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] += bottom_awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i];
            }
            break;
        default:
            break;
        }
    }

    MergeAwbExcWpStats(awb_stats_v39->wpFltOutFullEngine.fltPix, &left_stats->stat.rawawb, &right_stats->stat.rawawb, AwbWinSplitMode);
    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        MergeAwbExcWpStats(bottom_awb_stats_v39->wpFltOutFullEngine.fltPix, &bottom_left_stats->stat.rawawb, &bottom_right_stats->stat.rawawb, AwbWinSplitMode);
        MergeAwbExcWpStatsV(awb_stats_v39->wpFltOutFullEngine.fltPix, awb_stats_v39->wpFltOutFullEngine.fltPix,
                            bottom_awb_stats_v39->wpFltOutFullEngine.fltPix, AwbWinSplitModeV);
    }

    to->frame_id = left_stats->frame_id;
#endif
    return ret;
}

XCamReturn translateAwbStatsV35(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V35)

    if (pStatsTrans->mIsMultiIsp && pStatsTrans->mIspUniteMode) {
        return translateMultiAwbStatsV35(pStatsTrans, from, to);
    }

    aiq_awb_stats_wrapper_t* statsInt = (aiq_awb_stats_wrapper_t*)to->_data;
    struct rkisp35_stat_buffer* stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

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
        pStatsTrans->_ispParams->awb_cfg_v35.com.pixEngine.hw_awbCfg_zoneStatsSrc_mode > awbStats_pixAll_mode;
    statsInt->awb_stats_v39.awb_cfg_effect.mode = pStatsTrans->_ispParams->awb_cfg_v35.mode;
    statsInt->awb_stats_v39.awb_cfg_effect.lightNum = pStatsTrans->_ispParams->awb_cfg_v35.com.wpEngine.hw_awbCfg_lightSrcNum_val;
    statsInt->awb_stats_v39.awb_cfg_effect.groupIllIndxCurrent = pStatsTrans->_ispParams->awb_cfg_v35.groupIllIndxCurrent;
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent, pStatsTrans->_ispParams->awb_cfg_v35.IllIndxSetCurrent,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.timeSign, pStatsTrans->_ispParams->awb_cfg_v35.timeSign,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.timeSign));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.preWbgainSw, pStatsTrans->_ispParams->awb_cfg_v35.preWbgainSw,
           sizeof(pStatsTrans->_ispParams->awb_cfg_v35.preWbgainSw));
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

XCamReturn translateMultiAfStatsV35(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
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
    afStats_stats_t *b_afStats_stats = NULL;

    struct rkisp35_stat_buffer* left_stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    RkAiqIspUniteMode ispUniteMode = pStatsTrans->mIspUniteMode;

    uint32_t bufLen = AiqV4l2Buffer_getV4lBufLength((AiqV4l2Buffer_t*)from);
    uint32_t bufCnt = bufLen / sizeof(struct rkisp35_stat_buffer) > 4 ? 4 : bufLen / sizeof(struct rkisp35_stat_buffer);
    uint32_t bufSize = bufLen / bufCnt;
    struct rkisp35_stat_buffer* right_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize);

    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    struct rkisp35_stat_buffer* bottom_left_stats = NULL;
    struct rkisp35_stat_buffer* bottom_right_stats = NULL;
    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        bottom_left_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize * 2);
        if (bottom_left_stats == NULL) {
            LOGE("fail to get stats ,ignore\n");
            return XCAM_RETURN_BYPASS;
        }

        bottom_right_stats = (struct rkisp35_stat_buffer*)((char*)left_stats + bufSize * 3);
        if(bottom_right_stats == NULL) {
            LOGE("fail to get right stats ,ignore\n");
            return XCAM_RETURN_BYPASS;
        }

        if (pStatsTrans->_bottom_mge_af_stats == NULL) {
            pStatsTrans->_bottom_mge_af_stats = aiq_mallocz(sizeof(afStats_stats_t));
            if (pStatsTrans->_bottom_mge_af_stats == NULL) {
                LOGE_ANALYZER("fail to malloc _bottom_mge_af_stats");
                return XCAM_RETURN_BYPASS;
            }
        }
        b_afStats_stats = (afStats_stats_t *)pStatsTrans->_bottom_mge_af_stats;
        memset(b_afStats_stats, 0, sizeof(afStats_stats_t));
    } else {
        bottom_left_stats = left_stats;
        bottom_right_stats = right_stats;
    }

    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  left_stats->frame_id, left_stats->meas_type);

    if(left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type ||
       bottom_left_stats->frame_id != bottom_right_stats->frame_id || bottom_left_stats->meas_type != bottom_right_stats->meas_type ||
       left_stats->frame_id != bottom_left_stats->frame_id || left_stats->meas_type != bottom_left_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
        LOGE_ANALYZER("top left id %d meas %x right id %d meas %x bottom left id %d meas %x right id %d meas %x mode %d",
                      left_stats->frame_id, left_stats->meas_type,
                      right_stats->frame_id, right_stats->meas_type,
                      bottom_left_stats->frame_id, bottom_left_stats->meas_type,
                      bottom_right_stats->frame_id, bottom_right_stats->meas_type,
                      ispUniteMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    struct isp35_rawaf_meas_cfg* org_af = &pStatsTrans->_ispParams->meas.rawaf;
    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t t_isp_st, t_isp_ed, b_isp_st, b_isp_ed;
    int32_t t_win_st, t_win_ed, b_win_st, b_win_ed;
    int32_t x_st, x_ed, l_blknum, r_blknum, ov_w, blk_w, r_skip_blknum;
    int32_t y_st, y_ed, t_blknum, b_blknum, ov_h, blk_h, b_skip_blknum;
    struct AfSplitInfo af_split_info;
    struct AfSplitInfo af_split_info_v;
    int32_t i, j, k, dst_idx, l_idx, r_idx, l_lht, r_lht, lht0, lht1;
    int32_t t_idx, b_idx, t_lht, b_lht;

    struct isp35_bls_cfg* bls_cfg = &pStatsTrans->_ispParams->bls_cfg;
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

    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        // judge vertical win mode
        memset(&af_split_info_v, 0, sizeof(af_split_info_v));
        ov_h = pStatsTrans->left_isp_rect_.h + pStatsTrans->left_isp_rect_.y - pStatsTrans->bottom_left_isp_rect_.y;
        y_st = org_af->win[0].h_offs;
        y_ed = y_st + org_af->win[0].h_size;
        t_isp_st = pStatsTrans->left_isp_rect_.y;
        t_isp_ed = pStatsTrans->left_isp_rect_.y + pStatsTrans->left_isp_rect_.h;
        b_isp_st = pStatsTrans->bottom_left_isp_rect_.y;
        b_isp_ed = pStatsTrans->bottom_left_isp_rect_.y + pStatsTrans->bottom_left_isp_rect_.h;

        //// winA ////
        af_split_info_v.wina_l_ratio = 0;
        af_split_info_v.wina_r_ratio = 0;
        // af win in both side
        if ((y_st < b_isp_st) && (y_ed > t_isp_ed)) {
            af_split_info_v.wina_side_info = LEFT_AND_RIGHT_MODE;
            // af win < one isp width
            if (org_af->win[0].v_size < pStatsTrans->left_isp_rect_.h) {
                blk_w = org_af->win[0].v_size / ISP2X_RAWAF_SUMDATA_ROW;
                t_blknum = (t_isp_ed - y_st + blk_w - 1) / blk_w;
                b_blknum = ISP2X_RAWAF_SUMDATA_ROW - t_blknum;
                t_win_ed = t_isp_ed - 4;
                t_win_st = t_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
                if (blk_w < ov_h) {
                    r_skip_blknum = ov_h / blk_w;
                    b_win_st = ov_h - r_skip_blknum * blk_w;
                    b_win_ed = ov_h + (ISP2X_RAWAF_SUMDATA_ROW - r_skip_blknum) * blk_w;
                    af_split_info_v.wina_r_skip_blknum = r_skip_blknum;
                }
                else {
                    r_skip_blknum = 0;
                    b_win_st = 2;
                    b_win_ed = b_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;

                    // blend last block of left isp and first block of right isp
                    af_split_info_v.wina_r_skip_blknum = 0;
                    af_split_info_v.wina_l_ratio = (float)ov_h / (float)blk_w;
                    af_split_info_v.wina_r_ratio = 1 - af_split_info_v.wina_l_ratio;
                }
            }
            // af win <= one isp width * 1.5
            else if (org_af->win[0].v_size < pStatsTrans->left_isp_rect_.h * 3 / 2) {
                t_win_st = y_st;
                t_win_ed = t_isp_ed - 4;
                blk_w = (t_win_ed - t_win_st) / (ISP2X_RAWAF_SUMDATA_ROW + 1);
                t_win_st = t_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
                t_blknum = ((t_win_ed - t_win_st) * ISP2X_RAWAF_SUMDATA_ROW + org_af->win[0].v_size - 1) / org_af->win[0].v_size;
                b_blknum = ISP2X_RAWAF_SUMDATA_ROW - t_blknum;
                if (blk_w < ov_h) {
                    r_skip_blknum = ov_h / blk_w;
                    b_win_st = ov_h - r_skip_blknum * blk_w;
                    b_win_ed = ov_h + (ISP2X_RAWAF_SUMDATA_ROW - r_skip_blknum) * blk_w;
                    af_split_info_v.wina_r_skip_blknum = r_skip_blknum;
                }
                else {
                    r_skip_blknum = 0;
                    b_win_st = 2;
                    b_win_ed = b_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;
                    // blend last block of left isp and first block of right isp
                    af_split_info_v.wina_r_skip_blknum = 0;
                    af_split_info_v.wina_l_ratio = (float)ov_h / (float)blk_w;
                    af_split_info_v.wina_r_ratio = 1 - af_split_info_v.wina_l_ratio;
                }
            }
            else {
                t_win_st = y_st;
                t_win_ed = t_isp_ed - 4;
                blk_w = (t_win_ed - t_win_st) / ISP2X_RAWAF_SUMDATA_ROW;
                t_win_st = t_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
                b_win_st = 2;
                b_win_ed = b_win_st + blk_w * ISP2X_RAWAF_SUMDATA_ROW;
                af_split_info_v.wina_side_info = FULL_MODE;
                t_blknum = ISP2X_RAWAF_SUMDATA_ROW;
                b_blknum = ISP2X_RAWAF_SUMDATA_ROW;
            }
        }
        // af win in right side
        else if ((y_st >= b_isp_st) && (y_ed > t_isp_ed)) {
            af_split_info_v.wina_side_info = RIGHT_MODE;
            t_blknum = 0;
            b_blknum = ISP2X_RAWAF_SUMDATA_ROW;
            b_win_st = y_st - pStatsTrans->bottom_left_isp_rect_.y;
            b_win_ed = y_ed - pStatsTrans->bottom_left_isp_rect_.y;
            t_win_st = b_win_st;
            t_win_ed = b_win_ed;
        }
        // af win in left side
        else {
            af_split_info_v.wina_side_info = LEFT_MODE;
            t_blknum = ISP2X_RAWAF_SUMDATA_ROW;
            b_blknum = 0;
            t_win_st = y_st;
            t_win_ed = y_ed;
            b_win_st = t_win_st;
            b_win_ed = t_win_ed;
        }

        af_split_info_v.wina_l_blknum = t_blknum;
        af_split_info_v.wina_r_blknum = b_blknum;

        //// winB ////
        af_split_info_v.winb_l_ratio = 0;
        af_split_info_v.winb_r_ratio = 0;
        y_st = org_af->win[1].v_offs;
        y_ed = y_st + org_af->win[1].v_size;
        LOGD_AF("winb.y_st %d, winb.y_ed %d, t_isp_st %d, t_isp_ed %d, b_isp_st %d, b_isp_ed %d",
                y_st, y_ed, t_isp_st, t_isp_ed, b_isp_st, b_isp_ed);

        // af win in both side
        if ((y_st < b_isp_st) && (y_ed > t_isp_ed)) {
            af_split_info_v.winb_side_info = LEFT_AND_RIGHT_MODE;
            t_win_st = y_st;
            t_win_ed = t_isp_ed - 2;
            b_win_st = ov_h - 2;
            b_win_ed = y_ed - pStatsTrans->bottom_left_isp_rect_.y;
            // blend winB by width of left isp winB and right isp winB
            af_split_info_v.winb_l_ratio = (float)(t_win_ed - t_win_st) / (float)(y_ed - y_st);
            af_split_info_v.winb_r_ratio = 1 - af_split_info_v.winb_l_ratio;
        }
        // af win in right side
        else if ((y_st >= b_isp_st) && (y_ed > t_isp_ed)) {
            af_split_info_v.winb_side_info = RIGHT_MODE;
            af_split_info_v.winb_l_ratio = 0;
            af_split_info_v.winb_r_ratio = 1;
            b_win_st = y_st - pStatsTrans->bottom_left_isp_rect_.y;
            b_win_ed = y_ed - pStatsTrans->bottom_left_isp_rect_.y;
            t_win_st = b_win_st;
            t_win_ed = b_win_ed;
        }
        // af win in left side
        else {
            af_split_info_v.winb_side_info = LEFT_MODE;
            af_split_info_v.winb_l_ratio = 1;
            af_split_info_v.winb_r_ratio = 0;
            t_win_st = y_st;
            t_win_ed = y_ed;
            b_win_st = t_win_st;
            b_win_ed = t_win_ed;
        }
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

            if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                b_afStats_stats->subWin.hw_af_luma_val     = bottom_left_stats->stat.rawaf.sumy_winb * af_split_info_v.winb_l_ratio +
                                                             bottom_right_stats->stat.rawaf.sumy_winb * af_split_info_v.winb_r_ratio;
                b_afStats_stats->subWin.hw_af_hLumaCnt_val = bottom_left_stats->stat.rawaf.highlit_cnt_winb +
                                                             bottom_right_stats->stat.rawaf.highlit_cnt_winb;
                b_afStats_stats->subWin.hw_af_vFilt1Fv_val = bottom_left_stats->stat.rawaf.v1iir_sumb * af_split_info_v.winb_l_ratio +
                                                             bottom_right_stats->stat.rawaf.v1iir_sumb * af_split_info_v.winb_r_ratio;
                b_afStats_stats->subWin.hw_af_vFilt2Fv_val = bottom_left_stats->stat.rawaf.v2iir_sumb * af_split_info_v.winb_l_ratio +
                                                             bottom_right_stats->stat.rawaf.v2iir_sumb * af_split_info_v.winb_r_ratio;
                b_afStats_stats->subWin.hw_af_hFilt1Fv_val = bottom_left_stats->stat.rawaf.h1iir_sumb * af_split_info_v.winb_l_ratio +
                                                             bottom_right_stats->stat.rawaf.h1iir_sumb * af_split_info_v.winb_r_ratio;
                b_afStats_stats->subWin.hw_af_hFilt2Fv_val = bottom_left_stats->stat.rawaf.h2iir_sumb * af_split_info_v.winb_l_ratio +
                                                             bottom_right_stats->stat.rawaf.h2iir_sumb * af_split_info_v.winb_r_ratio;
            }
        } else if (af_split_info.winb_side_info == LEFT_MODE) {
            statsInt->afStats_stats.subWin.hw_af_luma_val     = left_stats->stat.rawaf.sumy_winb;
            statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val = left_stats->stat.rawaf.highlit_cnt_winb;
            statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val = left_stats->stat.rawaf.v1iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val = left_stats->stat.rawaf.v2iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val = left_stats->stat.rawaf.h1iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val = left_stats->stat.rawaf.h2iir_sumb;

            if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                b_afStats_stats->subWin.hw_af_luma_val     = bottom_left_stats->stat.rawaf.sumy_winb;
                b_afStats_stats->subWin.hw_af_hLumaCnt_val = bottom_left_stats->stat.rawaf.highlit_cnt_winb;
                b_afStats_stats->subWin.hw_af_vFilt1Fv_val = bottom_left_stats->stat.rawaf.v1iir_sumb;
                b_afStats_stats->subWin.hw_af_vFilt2Fv_val = bottom_left_stats->stat.rawaf.v2iir_sumb;
                b_afStats_stats->subWin.hw_af_hFilt1Fv_val = bottom_left_stats->stat.rawaf.h1iir_sumb;
                b_afStats_stats->subWin.hw_af_hFilt2Fv_val = bottom_left_stats->stat.rawaf.h2iir_sumb;
            }
        } else {
            statsInt->afStats_stats.subWin.hw_af_luma_val     = right_stats->stat.rawaf.sumy_winb;
            statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val = right_stats->stat.rawaf.highlit_cnt_winb;
            statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val = right_stats->stat.rawaf.v1iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val = right_stats->stat.rawaf.v2iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val = right_stats->stat.rawaf.h1iir_sumb;
            statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val = right_stats->stat.rawaf.h2iir_sumb;

            if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                b_afStats_stats->subWin.hw_af_luma_val     = bottom_right_stats->stat.rawaf.sumy_winb;
                b_afStats_stats->subWin.hw_af_hLumaCnt_val = bottom_right_stats->stat.rawaf.highlit_cnt_winb;
                b_afStats_stats->subWin.hw_af_vFilt1Fv_val = bottom_right_stats->stat.rawaf.v1iir_sumb;
                b_afStats_stats->subWin.hw_af_vFilt2Fv_val = bottom_right_stats->stat.rawaf.v2iir_sumb;
                b_afStats_stats->subWin.hw_af_hFilt1Fv_val = bottom_right_stats->stat.rawaf.h1iir_sumb;
                b_afStats_stats->subWin.hw_af_hFilt2Fv_val = bottom_right_stats->stat.rawaf.h2iir_sumb;
            }
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

                        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                            b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                            b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                            b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                            b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                            temp_luma = bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                            b_afStats_stats->mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                            b_afStats_stats->mainWin.hw_af_hLumaCnt_val[dst_idx] = bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                        }
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

                        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                            b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                                bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 1) + 1].v1 + bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * j].v1;
                            b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                                bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 1) + 1].v2 + bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * j].v2;
                            b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                                bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 1) + 1].h1 + bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * j].h1;
                            b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                                bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 1) + 1].h2 + bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * j].h2;

                            temp_luma = bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 1) + 1].average + bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * j].average;
                            b_afStats_stats->mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                            b_afStats_stats->mainWin.hw_af_hLumaCnt_val[dst_idx] =
                                bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 1) + 1].highlight + bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * j].highlight;
                        }
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

                        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                            b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                                bottom_right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 8) + 1].v1 + bottom_right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 7)].v1;
                            b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                                bottom_right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 8) + 1].v2 + bottom_right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[2 * (j - 7)].v2;
                            b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                                bottom_right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 8) + 1].h1 + bottom_right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 7)].h1;
                            b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                                bottom_right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 8) + 1].h2 + bottom_right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[2 * (j - 7)].h2;

                            temp_luma =
                                bottom_right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 8) + 1].average + bottom_right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 7)].average;
                            b_afStats_stats->mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                            b_afStats_stats->mainWin.hw_af_hLumaCnt_val[dst_idx] =
                                bottom_right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 8) + 1].highlight + bottom_right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[2 * (j - 7)].highlight;
                        }
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

                    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                        b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                        b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                        b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                        b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                        temp_luma = bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                        b_afStats_stats->mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        b_afStats_stats->mainWin.hw_af_hLumaCnt_val[dst_idx] = bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                    }
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

                    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                        b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[dst_idx] = bottom_right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                        b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[dst_idx] = bottom_right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                        b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[dst_idx] = bottom_right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                        b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[dst_idx] = bottom_right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                        temp_luma = bottom_right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                        b_afStats_stats->mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        b_afStats_stats->mainWin.hw_af_hLumaCnt_val[dst_idx] = bottom_right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                    }
                }
            }

            if (af_split_info.wina_r_skip_blknum == 0) {
                for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                    dst_idx = j * ISP39_MEAN_BLK_Y_NUM + (af_split_info.wina_l_blknum - 1);
                    l_idx = j * ISP39_MEAN_BLK_Y_NUM + (ISP39_MEAN_BLK_X_NUM - 1);
                    r_idx = j * ISP39_MEAN_BLK_Y_NUM;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                        left_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[ISP39_MEAN_BLK_X_NUM -1].v1 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[0].v1 * af_split_info.wina_r_ratio;
                    statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                        left_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[ISP39_MEAN_BLK_X_NUM -1].v2 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[0].v2 * af_split_info.wina_r_ratio;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                        left_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[ISP39_MEAN_BLK_X_NUM -1].h1 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[0].h1 * af_split_info.wina_r_ratio;
                    statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                        left_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[ISP39_MEAN_BLK_X_NUM -1].h2 * af_split_info.wina_l_ratio +
                        right_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[0].h2 * af_split_info.wina_r_ratio;

                    temp_luma = left_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[ISP39_MEAN_BLK_X_NUM -1].average +
                                right_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[0].average;
                    statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                    statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] =
                        left_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[ISP39_MEAN_BLK_X_NUM -1].highlight +
                        right_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[0].highlight;

                    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                        b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                            bottom_left_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[ISP39_MEAN_BLK_X_NUM -1].v1 * af_split_info.wina_l_ratio +
                            bottom_right_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[0].v1 * af_split_info.wina_r_ratio;
                        b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                            bottom_left_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[ISP39_MEAN_BLK_X_NUM -1].v2 * af_split_info.wina_l_ratio +
                            bottom_right_stats->stat.rawaf.viir_blk_y[j].viir_blk_x[0].v2 * af_split_info.wina_r_ratio;
                        b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                            bottom_left_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[ISP39_MEAN_BLK_X_NUM -1].h1 * af_split_info.wina_l_ratio +
                            bottom_right_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[0].h1 * af_split_info.wina_r_ratio;
                        b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                            bottom_left_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[ISP39_MEAN_BLK_X_NUM -1].h2 * af_split_info.wina_l_ratio +
                            bottom_right_stats->stat.rawaf.hiir_blk_y[j].hiir_blk_x[0].h2 * af_split_info.wina_r_ratio;

                        temp_luma = bottom_left_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[ISP39_MEAN_BLK_X_NUM -1].average +
                                    bottom_right_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[0].average;
                        b_afStats_stats->mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        b_afStats_stats->mainWin.hw_af_hLumaCnt_val[dst_idx] =
                            bottom_left_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[ISP39_MEAN_BLK_X_NUM -1].highlight +
                            bottom_right_stats->stat.rawaf.aehgl_blk_y[j].aehgl_blk_x[0].highlight;
                    }
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

                    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                        b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                        b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                        b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                        b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[dst_idx] = bottom_left_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                        temp_luma = bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                        b_afStats_stats->mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        b_afStats_stats->mainWin.hw_af_hLumaCnt_val[dst_idx] = bottom_left_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                    }
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

                    if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                        b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[dst_idx] = bottom_right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v1;
                        b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[dst_idx] = bottom_right_stats->stat.rawaf.viir_blk_y[i].viir_blk_x[j].v2;
                        b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[dst_idx] = bottom_right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h1;
                        b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[dst_idx] = bottom_right_stats->stat.rawaf.hiir_blk_y[i].hiir_blk_x[j].h2;

                        temp_luma = bottom_right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].average;
                        b_afStats_stats->mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        b_afStats_stats->mainWin.hw_af_hLumaCnt_val[dst_idx] = bottom_right_stats->stat.rawaf.aehgl_blk_y[i].aehgl_blk_x[j].highlight;
                    }
                }
            }
        }

        if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
            if (af_split_info_v.winb_side_info == LEFT_AND_RIGHT_MODE) {
                statsInt->afStats_stats.subWin.hw_af_luma_val = statsInt->afStats_stats.subWin.hw_af_luma_val * af_split_info_v.winb_l_ratio +
                                                                b_afStats_stats->subWin.hw_af_luma_val * af_split_info_v.winb_r_ratio;
                statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val = statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val +
                                                                    b_afStats_stats->subWin.hw_af_hLumaCnt_val;
                statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val = statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val * af_split_info_v.winb_l_ratio +
                                                                    b_afStats_stats->subWin.hw_af_vFilt1Fv_val * af_split_info_v.winb_r_ratio;
                statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val = statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val * af_split_info_v.winb_l_ratio +
                                                                    b_afStats_stats->subWin.hw_af_vFilt2Fv_val * af_split_info_v.winb_r_ratio;
                statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val = statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val * af_split_info_v.winb_l_ratio +
                                                                    b_afStats_stats->subWin.hw_af_hFilt1Fv_val * af_split_info_v.winb_r_ratio;
                statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val = statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val * af_split_info_v.winb_l_ratio +
                                                                    b_afStats_stats->subWin.hw_af_hFilt2Fv_val * af_split_info_v.winb_r_ratio;
            } else if (af_split_info_v.winb_side_info == RIGHT_MODE) {
                statsInt->afStats_stats.subWin.hw_af_luma_val     = b_afStats_stats->subWin.hw_af_luma_val;
                statsInt->afStats_stats.subWin.hw_af_hLumaCnt_val = b_afStats_stats->subWin.hw_af_hLumaCnt_val;
                statsInt->afStats_stats.subWin.hw_af_vFilt1Fv_val = b_afStats_stats->subWin.hw_af_vFilt1Fv_val;
                statsInt->afStats_stats.subWin.hw_af_vFilt2Fv_val = b_afStats_stats->subWin.hw_af_vFilt2Fv_val;
                statsInt->afStats_stats.subWin.hw_af_hFilt1Fv_val = b_afStats_stats->subWin.hw_af_hFilt1Fv_val;
                statsInt->afStats_stats.subWin.hw_af_hFilt2Fv_val = b_afStats_stats->subWin.hw_af_hFilt2Fv_val;
            }

            if (af_split_info_v.wina_side_info == FULL_MODE) {
                for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                    for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                        dst_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                        if (j == 0) {
                            l_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                            // statsInt->afStats_stats = statsInt->afStats_stats
                            continue;
                        } else if (j >= 1 && j <= 7) {
                            l_idx = i * ISP39_MEAN_BLK_Y_NUM + 2 * (j - 1) + 1;
                            statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                                statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[l_idx] + statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[l_idx + 1];
                            statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                                statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[l_idx] + statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[l_idx + 1];
                            statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                                statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[l_idx] + statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[l_idx + 1];
                            statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                                statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[l_idx] + statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[l_idx + 1];

                            temp_luma = statsInt->afStats_stats.mainWin.hw_af_luma_val[l_idx] + statsInt->afStats_stats.mainWin.hw_af_luma_val[l_idx + 1];
                            statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                            statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] =
                                statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[l_idx] + statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[l_idx + 1];
                        } else {
                            r_idx = i * ISP39_MEAN_BLK_Y_NUM + 2 * (j - 8) + 1;
                            statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                                b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[r_idx] + b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[r_idx + 1];
                            statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                                b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[r_idx] + b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[r_idx + 1];
                            statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                                b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[r_idx] + b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[r_idx + 1];
                            statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                                b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[r_idx] + b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[r_idx + 1];

                            temp_luma = b_afStats_stats->mainWin.hw_af_luma_val[r_idx] + b_afStats_stats->mainWin.hw_af_luma_val[r_idx + 1];
                            statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                            statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] =
                                b_afStats_stats->mainWin.hw_af_hLumaCnt_val[r_idx] + b_afStats_stats->mainWin.hw_af_hLumaCnt_val[r_idx + 1];
                        }
                    }
                }
            }
            else if (af_split_info_v.wina_side_info == LEFT_AND_RIGHT_MODE) {
                for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                    j = ISP39_MEAN_BLK_X_NUM - af_split_info.wina_l_blknum;
                    for (k = 0; k < af_split_info.wina_l_blknum; j++, k++) {
                        dst_idx = i * ISP39_MEAN_BLK_Y_NUM + k;
                        l_idx = i * ISP39_MEAN_BLK_Y_NUM + j;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] = statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[l_idx];
                        statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] = statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[l_idx];
                        statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] = statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[l_idx];
                        statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] = statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[l_idx];

                        temp_luma = statsInt->afStats_stats.mainWin.hw_af_luma_val[l_idx];
                        statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] = b_afStats_stats->mainWin.hw_af_hLumaCnt_val[l_idx];
                    }
                }

                for (i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
                    j = af_split_info.wina_r_skip_blknum;
                    for (k = 0; k < af_split_info.wina_r_blknum; j++, k++) {
                        dst_idx = i * ISP39_MEAN_BLK_Y_NUM + k + af_split_info.wina_l_blknum;
                        r_idx = i * ISP39_MEAN_BLK_Y_NUM + j;

                        statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] = b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[r_idx];
                        statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] = b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[r_idx];
                        statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] = b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[r_idx];
                        statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] = b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[r_idx];

                        temp_luma = b_afStats_stats->mainWin.hw_af_luma_val[r_idx];
                        statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] = b_afStats_stats->mainWin.hw_af_hLumaCnt_val[r_idx];
                    }
                }

                if (af_split_info.wina_r_skip_blknum == 0) {
                    for (j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
                        dst_idx = j * ISP39_MEAN_BLK_Y_NUM + (af_split_info.wina_l_blknum - 1);
                        l_idx = j * ISP39_MEAN_BLK_Y_NUM + (ISP39_MEAN_BLK_X_NUM - 1);
                        r_idx = j * ISP39_MEAN_BLK_Y_NUM;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] =
                            statsInt->afStats_stats.mainWin.hw_af_vFilt1Fv_val[dst_idx] * af_split_info_v.winb_l_ratio +
                            b_afStats_stats->mainWin.hw_af_vFilt1Fv_val[r_idx] * af_split_info.winb_r_ratio;
                        statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] =
                            statsInt->afStats_stats.mainWin.hw_af_vFilt2Fv_val[dst_idx] * af_split_info_v.winb_l_ratio +
                            b_afStats_stats->mainWin.hw_af_vFilt2Fv_val[r_idx] * af_split_info.winb_r_ratio;
                        statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] =
                            statsInt->afStats_stats.mainWin.hw_af_hFilt1Fv_val[dst_idx] * af_split_info_v.winb_l_ratio +
                            b_afStats_stats->mainWin.hw_af_hFilt1Fv_val[r_idx] * af_split_info.winb_r_ratio;
                        statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] =
                            statsInt->afStats_stats.mainWin.hw_af_hFilt2Fv_val[dst_idx] * af_split_info_v.winb_l_ratio +
                            b_afStats_stats->mainWin.hw_af_hFilt2Fv_val[r_idx] * af_split_info.winb_r_ratio;

                        temp_luma = statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] * af_split_info_v.winb_l_ratio +
                                    b_afStats_stats->mainWin.hw_af_luma_val[r_idx] * af_split_info.winb_r_ratio;
                        statsInt->afStats_stats.mainWin.hw_af_luma_val[dst_idx] = MAX(temp_luma, 0);
                        statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] =
                            statsInt->afStats_stats.mainWin.hw_af_hLumaCnt_val[dst_idx] * af_split_info_v.winb_l_ratio +
                            b_afStats_stats->mainWin.hw_af_hLumaCnt_val[r_idx] * af_split_info.winb_r_ratio;
                    }
                }
            }
        }

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


XCamReturn translateAfStatsV35(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                               aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V35)

    if (pStatsTrans->mIsMultiIsp && pStatsTrans->mIspUniteMode) {
        return translateMultiAfStatsV35(pStatsTrans, from, to);
    }

    aiq_af_stats_wrapper_t* statsInt = (aiq_af_stats_wrapper_t*)to->_data;
    struct rkisp35_stat_buffer* stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));
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

    struct isp35_bls_cfg* bls_cfg = &pStatsTrans->_ispParams->bls_cfg;
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
    struct rkisp35_stat_buffer* stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));
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

    struct rkisp35_stat_buffer* stats =
        (struct rkisp35_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

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

#if defined(ISP_HW_V35)
XCamReturn AiqStatsTranslator_init(AiqStatsTranslator_t* pStatsTrans) {
    pStatsTrans->translateAecStats          = translateAecStatsV35;
    pStatsTrans->translateAwbStats          = translateAwbStatsV35;
    pStatsTrans->translateAfStats           = translateAfStatsV35;
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

    pStatsTrans->_bottom_mge_af_stats       = NULL;
    pStatsTrans->_bottom_mge_awb_stats       = NULL;

    return XCAM_RETURN_NO_ERROR;
}

void AiqStatsTranslator_deinit(AiqStatsTranslator_t* pStatsTrans) {

    if (pStatsTrans->_bottom_mge_af_stats) {
        aiq_free(pStatsTrans->_bottom_mge_af_stats);
        pStatsTrans->_bottom_mge_af_stats = NULL;
    }

    if (pStatsTrans->_bottom_mge_awb_stats) {
        aiq_free(pStatsTrans->_bottom_mge_awb_stats);
        pStatsTrans->_bottom_mge_awb_stats = NULL;
    }

    return;
}

#endif
