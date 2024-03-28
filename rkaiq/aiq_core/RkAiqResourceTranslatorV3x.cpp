/*
 * RkAiqConfigTranslatorV3x.cpp
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
#include "common/rkisp3-config.h"
#include "RkAiqResourceTranslatorV3x.h"

//#define AE_STATS_DEBUG
#define MAX_8BITS ((1 << 8) - 1)
#define MAX_10BITS ((1 << 10) - 1)
#define MAX_12BITS ((1 << 12) - 1)
#define MAX_29BITS ((1 << 29) - 1)
#define MAX_32BITS  0xffffffff

namespace RkCam {

RkAiqResourceTranslatorV3x::RkAiqResourceTranslatorV3x() : mIsMultiIsp(false) {
    memset(&pic_rect_, 0, sizeof(RkAiqResourceTranslatorV3x::Rectangle));
    memset(&left_isp_rect_, 0, sizeof(RkAiqResourceTranslatorV3x::Rectangle));
    memset(&right_isp_rect_, 0, sizeof(RkAiqResourceTranslatorV3x::Rectangle));
    mIspUnitedMode = RK_AIQ_ISP_UNITED_MODE_NORMAL;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetMultiIspMode(bool isMultiIsp) {
    mIsMultiIsp = isMultiIsp;
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetIspUnitedMode(RkAiqIspUnitedMode mode) {
    mIspUnitedMode = mode;
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetPicInfo(RkAiqResourceTranslatorV3x::Rectangle& pic_rect) {
    pic_rect_ = pic_rect;
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetLeftIspRect(RkAiqResourceTranslatorV3x::Rectangle& left_isp_rect) {
    left_isp_rect_ = left_isp_rect;
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetRightIspRect(
    RkAiqResourceTranslatorV3x::Rectangle& right_isp_rect) {
    right_isp_rect_ = right_isp_rect;
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetPicInfo(RkAiqResourceTranslatorV3x::Rectangle&& pic_rect) {
    pic_rect_ = std::move(pic_rect);
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetLeftIspRect(RkAiqResourceTranslatorV3x::Rectangle&& left_isp_rect) {
    left_isp_rect_ = std::move(left_isp_rect);
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetRightIspRect(
    RkAiqResourceTranslatorV3x::Rectangle&& right_isp_rect) {
    right_isp_rect_ = std::move(right_isp_rect);
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetBottomLeftIspRect(RkAiqResourceTranslatorV3x::Rectangle& bottom_left_isp_rect) {
    bottom_left_isp_rect_ = bottom_left_isp_rect;
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetBottomRightIspRect(
    RkAiqResourceTranslatorV3x::Rectangle& bottom_right_isp_rect) {
    bottom_right_isp_rect_ = bottom_right_isp_rect;
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetBottomLeftIspRect(RkAiqResourceTranslatorV3x::Rectangle&& bottom_left_isp_rect) {
    bottom_left_isp_rect_ = std::move(bottom_left_isp_rect);
    return *this;
}

RkAiqResourceTranslatorV3x& RkAiqResourceTranslatorV3x::SetBottomRightIspRect(
    RkAiqResourceTranslatorV3x::Rectangle&& bottom_right_isp_rect) {
    bottom_right_isp_rect_ = std::move(bottom_right_isp_rect);
    return *this;
}

bool RkAiqResourceTranslatorV3x::IsMultiIspMode() const {
    return mIsMultiIsp;
}

RkAiqIspUnitedMode RkAiqResourceTranslatorV3x::GetIspUnitedMode() {
    return mIspUnitedMode;
}

RkAiqResourceTranslatorV3x::Rectangle RkAiqResourceTranslatorV3x::GetPicInfo() {
    return pic_rect_;
}

RkAiqResourceTranslatorV3x::Rectangle RkAiqResourceTranslatorV3x::GetLeftIspRect() {
    return left_isp_rect_;
}

RkAiqResourceTranslatorV3x::Rectangle RkAiqResourceTranslatorV3x::GetRightIspRect() {
    return right_isp_rect_;
}

RkAiqResourceTranslatorV3x::Rectangle RkAiqResourceTranslatorV3x::GetBottomLeftIspRect() {
    return bottom_left_isp_rect_;
}

RkAiqResourceTranslatorV3x::Rectangle RkAiqResourceTranslatorV3x::GetBottomRightIspRect() {
    return bottom_right_isp_rect_;
}


#if defined(RKAIQ_HAVE_MULTIISP) && defined(ISP_HW_V30)

void JudgeWinLocation(
    struct isp2x_window* ori_win,
    WinSplitMode& mode,
    RkAiqResourceTranslatorV3x::Rectangle left_isp_rect_,
    RkAiqResourceTranslatorV3x::Rectangle right_isp_rect_
) {

    if (ori_win->h_offs + ori_win->h_size <= left_isp_rect_.w) {
        mode = LEFT_MODE;
    } else if(ori_win->h_offs >= right_isp_rect_.x) {
        mode = RIGHT_MODE;
    } else {
        if ((ori_win->h_offs + ori_win->h_size / 2) <= left_isp_rect_.w
                && right_isp_rect_.x <= (ori_win->h_offs + ori_win->h_size / 2)) {
            mode = LEFT_AND_RIGHT_MODE;
        }
        else {

            if ((ori_win->h_offs + ori_win->h_size / 2) < right_isp_rect_.x) {

                u16 h_size_tmp1 = left_isp_rect_.w - ori_win->h_offs;
                u16 h_size_tmp2 = (right_isp_rect_.x - ori_win->h_offs) * 2;

                if (abs(ori_win->h_size - h_size_tmp1) < abs(ori_win->h_size - h_size_tmp2))
                    mode = LEFT_MODE;
                else
                    mode = LEFT_AND_RIGHT_MODE;
            }
            else {

                u16 h_size_tmp1 = ori_win->h_offs + ori_win->h_size - right_isp_rect_.x;
                u16 h_size_tmp2 = (ori_win->h_offs + ori_win->h_size - left_isp_rect_.w) * 2;

                if (abs(ori_win->h_size - h_size_tmp1) < abs(ori_win->h_size - h_size_tmp2))
                    mode = RIGHT_MODE;
                else
                    mode = LEFT_AND_RIGHT_MODE;
            }
        }
    }
}

void MergeAecWinLiteStats(
    struct isp2x_rawaelite_stat*    left_stats,
    struct isp2x_rawaelite_stat*    right_stats,
    rawaelite_stat_t*               merge_stats,
    uint16_t*                       raw_mean,
    unsigned char*                  weight,
    int8_t                          stats_chn_sel,
    int8_t                          y_range_mode,
    WinSplitMode                    mode,
    struct isp2x_bls_fixed_val      bls1_val,
    float*                          bls_ratio
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

    u8 wnd_num = sqrt(ISP3X_RAWAELITE_MEAN_NUM);
    for(int i = 0; i < wnd_num; i++) {
        for(int j = 0; j < wnd_num; j++) {

            // step1 copy stats
            switch(mode) {
            case LEFT_MODE:
                merge_stats->channelr_xy[i * wnd_num + j] = left_stats->data[i * wnd_num + j].channelr_xy;
                merge_stats->channelg_xy[i * wnd_num + j] = left_stats->data[i * wnd_num + j].channelg_xy;
                merge_stats->channelb_xy[i * wnd_num + j] = left_stats->data[i * wnd_num + j].channelb_xy;
                break;
            case RIGHT_MODE:
                merge_stats->channelr_xy[i * wnd_num + j] = right_stats->data[i * wnd_num + j].channelr_xy;
                merge_stats->channelg_xy[i * wnd_num + j] = right_stats->data[i * wnd_num + j].channelg_xy;
                merge_stats->channelb_xy[i * wnd_num + j] = right_stats->data[i * wnd_num + j].channelb_xy;
                break;
            case LEFT_AND_RIGHT_MODE:

                if(j < wnd_num / 2) {
                    merge_stats->channelr_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + j * 2].channelr_xy + left_stats->data[i * wnd_num + j * 2 + 1].channelr_xy) / 2;
                    merge_stats->channelg_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + j * 2].channelg_xy + left_stats->data[i * wnd_num + j * 2 + 1].channelg_xy) / 2;
                    merge_stats->channelb_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + j * 2].channelb_xy + left_stats->data[i * wnd_num + j * 2 + 1].channelb_xy) / 2;
                } else if(j > wnd_num / 2) {
                    merge_stats->channelr_xy[i * wnd_num + j] = (right_stats->data[i * wnd_num + j * 2 - wnd_num].channelr_xy + right_stats->data[i * wnd_num + j * 2 - wnd_num + 1].channelr_xy) / 2;
                    merge_stats->channelg_xy[i * wnd_num + j] = (right_stats->data[i * wnd_num + j * 2 - wnd_num].channelg_xy + right_stats->data[i * wnd_num + j * 2 - wnd_num + 1].channelg_xy) / 2;
                    merge_stats->channelb_xy[i * wnd_num + j] = (right_stats->data[i * wnd_num + j * 2 - wnd_num].channelb_xy + right_stats->data[i * wnd_num + j * 2 - wnd_num + 1].channelb_xy) / 2;
                } else {
                    merge_stats->channelr_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + wnd_num - 1].channelr_xy + right_stats->data[i * wnd_num + 0].channelr_xy) / 2;
                    merge_stats->channelg_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + wnd_num - 1].channelg_xy + right_stats->data[i * wnd_num + 0].channelg_xy) / 2;
                    merge_stats->channelb_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + wnd_num - 1].channelb_xy + right_stats->data[i * wnd_num + 0].channelb_xy) / 2;
                }
                break;
            default:
                break;
            }

            // step2 subtract bls1
            merge_stats->channelr_xy[i * wnd_num + j] = (u16)CLIP((merge_stats->channelr_xy[i * wnd_num + j] - bls1_val.r) * bls_ratio[0], 0, MAX_10BITS);
            merge_stats->channelg_xy[i * wnd_num + j] = (u16)CLIP((merge_stats->channelg_xy[i * wnd_num + j] - bls1_val.gr) * bls_ratio[1], 0, MAX_12BITS);
            merge_stats->channelb_xy[i * wnd_num + j] = (u16)CLIP((merge_stats->channelb_xy[i * wnd_num + j] - bls1_val.b) * bls_ratio[2], 0, MAX_10BITS);

        }
    }

    switch (stats_chn_sel) {
    case RAWSTATS_CHN_Y_EN:
    case RAWSTATS_CHN_ALL_EN:
    default:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            merge_stats->channely_xy[i] = CLIP(round(rcc * (float)(merge_stats->channelr_xy[i] >> 2) +
                                               gcc * (float)(merge_stats->channelg_xy[i] >> 4) +
                                               bcc * (float)(merge_stats->channelb_xy[i] >> 2) + off), 0, MAX_8BITS);
            sum_xy += (merge_stats->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_R_EN:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            sum_xy += ((merge_stats->channelr_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_G_EN:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            merge_stats->channelg_xy[i] = merge_stats->channelg_xy[i];
            sum_xy += ((merge_stats->channelg_xy[i] >> 4) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_B_EN:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            sum_xy += ((merge_stats->channelb_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_RGB_EN:
        break;
    }

}

void MergeAecWinBigStats(
    struct isp2x_rawaebig_stat*     left_stats,
    struct isp2x_rawaebig_stat*     right_stats,
    rawaebig_stat_t*                merge_stats,
    uint16_t*                       raw_mean,
    unsigned char*                  weight,
    int8_t                          stats_chn_sel,
    int8_t                          y_range_mode,
    WinSplitMode                    mode,
    struct isp2x_bls_fixed_val      bls1_val,
    float*                          bls_ratio
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

    u8 wnd_num = sqrt(ISP3X_RAWAEBIG_MEAN_NUM);
    for(int i = 0; i < wnd_num; i++) {
        for(int j = 0; j < wnd_num; j++) {

            // step1 copy stats
            switch(mode) {
            case LEFT_MODE:
                merge_stats->channelr_xy[i * wnd_num + j] = left_stats->data[i * wnd_num + j].channelr_xy;
                merge_stats->channelg_xy[i * wnd_num + j] = left_stats->data[i * wnd_num + j].channelg_xy;
                merge_stats->channelb_xy[i * wnd_num + j] = left_stats->data[i * wnd_num + j].channelb_xy;
                break;
            case RIGHT_MODE:
                merge_stats->channelr_xy[i * wnd_num + j] = right_stats->data[i * wnd_num + j].channelr_xy;
                merge_stats->channelg_xy[i * wnd_num + j] = right_stats->data[i * wnd_num + j].channelg_xy;
                merge_stats->channelb_xy[i * wnd_num + j] = right_stats->data[i * wnd_num + j].channelb_xy;
                break;
            case LEFT_AND_RIGHT_MODE:
                if(j < wnd_num / 2) {
                    merge_stats->channelr_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + j * 2].channelr_xy + left_stats->data[i * wnd_num + j * 2 + 1].channelr_xy) / 2;
                    merge_stats->channelg_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + j * 2].channelg_xy + left_stats->data[i * wnd_num + j * 2 + 1].channelg_xy) / 2;
                    merge_stats->channelb_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + j * 2].channelb_xy + left_stats->data[i * wnd_num + j * 2 + 1].channelb_xy) / 2;
                } else if(j > wnd_num / 2) {
                    merge_stats->channelr_xy[i * wnd_num + j] = (right_stats->data[i * wnd_num + j * 2 - wnd_num].channelr_xy + right_stats->data[i * wnd_num + j * 2 - wnd_num + 1].channelr_xy) / 2;
                    merge_stats->channelg_xy[i * wnd_num + j] = (right_stats->data[i * wnd_num + j * 2 - wnd_num].channelg_xy + right_stats->data[i * wnd_num + j * 2 - wnd_num + 1].channelg_xy) / 2;
                    merge_stats->channelb_xy[i * wnd_num + j] = (right_stats->data[i * wnd_num + j * 2 - wnd_num].channelb_xy + right_stats->data[i * wnd_num + j * 2 - wnd_num + 1].channelb_xy) / 2;
                } else {
                    merge_stats->channelr_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + wnd_num - 1].channelr_xy + right_stats->data[i * wnd_num + 0].channelr_xy) / 2;
                    merge_stats->channelg_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + wnd_num - 1].channelg_xy + right_stats->data[i * wnd_num + 0].channelg_xy) / 2;
                    merge_stats->channelb_xy[i * wnd_num + j] = (left_stats->data[i * wnd_num + wnd_num - 1].channelb_xy + right_stats->data[i * wnd_num + 0].channelb_xy) / 2;
                }
                break;
            default:
                break;
            }

            // step2 subtract bls1
            merge_stats->channelr_xy[i * wnd_num + j] = (u16)CLIP((merge_stats->channelr_xy[i * wnd_num + j] - bls1_val.r) * bls_ratio[0], 0, MAX_10BITS);
            merge_stats->channelg_xy[i * wnd_num + j] = (u16)CLIP((merge_stats->channelg_xy[i * wnd_num + j] - bls1_val.gr) * bls_ratio[1], 0, MAX_12BITS);
            merge_stats->channelb_xy[i * wnd_num + j] = (u16)CLIP((merge_stats->channelb_xy[i * wnd_num + j] - bls1_val.b) * bls_ratio[2], 0, MAX_10BITS);

        }
    }

    switch (stats_chn_sel) {
    case RAWSTATS_CHN_Y_EN:
    case RAWSTATS_CHN_ALL_EN:
    default:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            merge_stats->channely_xy[i] = CLIP(round(rcc * (float)(merge_stats->channelr_xy[i] >> 2) +
                                               gcc * (float)(merge_stats->channelg_xy[i] >> 4) +
                                               bcc * (float)(merge_stats->channelb_xy[i] >> 2) + off), 0, MAX_8BITS);
            sum_xy += (merge_stats->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_R_EN:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            sum_xy += ((merge_stats->channelr_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_G_EN:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            merge_stats->channelg_xy[i] = merge_stats->channelg_xy[i];
            sum_xy += ((merge_stats->channelg_xy[i] >> 4) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_B_EN:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            sum_xy += ((merge_stats->channelb_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_RGB_EN:
        break;
    }

}

void MergeAecSubWinStats(
    struct isp2x_rawaebig_stat*     left_stats,
    struct isp2x_rawaebig_stat*     right_stats,
    rawaebig_stat_t*                merge_stats,
    u8*                             left_en,
    u8*                             right_en,
    struct isp2x_bls_fixed_val      bls1_val,
    float*                          bls_ratio,
    u32*                            pixel_num
) {
    for(int i = 0; i < ISP3X_RAWAEBIG_SUBWIN_NUM; i++) {
        // step1 copy stats
        merge_stats->wndx_sumr[i] = ((left_en[i]) ? left_stats->sumr[i] : 0) + ((right_en[i]) ? right_stats->sumr[i] : 0);
        merge_stats->wndx_sumg[i] = ((left_en[i]) ? left_stats->sumg[i] : 0) + ((right_en[i]) ? right_stats->sumg[i] : 0);
        merge_stats->wndx_sumb[i] = ((left_en[i]) ? left_stats->sumb[i] : 0) + ((right_en[i]) ? right_stats->sumb[i] : 0);

        // step2 subtract bls1
        if(left_en[i] == 1 || right_en[i] == 1) {
            merge_stats->wndx_sumr[i] = (u32)CLIP((merge_stats->wndx_sumr[i] - (pixel_num[i] >> 2) * bls1_val.r) * bls_ratio[0], 0, MAX_29BITS);
            merge_stats->wndx_sumg[i] = (u32)CLIP((merge_stats->wndx_sumg[i] - (pixel_num[i] >> 1) * bls1_val.gr) * bls_ratio[1], 0, MAX_32BITS);
            merge_stats->wndx_sumb[i] = (u32)CLIP((merge_stats->wndx_sumb[i] - (pixel_num[i] >> 2) * bls1_val.b) * bls_ratio[2], 0, MAX_29BITS);
        }
    }
}

void MergeAecHistBinStats(
    u32*                        left_stats,
    u32*                        right_stats,
    u32*                        merge_stats,
    WinSplitMode                mode,
    u8                          hist_mode,
    struct isp2x_bls_fixed_val  bls1_val,
    float*                      bls_ratio,
    bool                        bls1_en
) {

    if (bls1_en) {

        memset(merge_stats, 0, ISP3X_HIST_BIN_N_MAX * sizeof(u32));

        int tmp;
        s16 hist_bls1;
        float hist_bls_ratio;

        switch (hist_mode) {
        case 2:
            hist_bls1 = (bls1_val.r >> 2);
            hist_bls_ratio = bls_ratio[0];
            break;
        case 3:
            hist_bls1 = (bls1_val.gr >> 4);
            hist_bls_ratio = bls_ratio[1];
            break;
        case 4:
            hist_bls1 = (bls1_val.b >> 2);
            hist_bls_ratio = bls_ratio[2];
            break;
        case 5:
        default:
            hist_bls1 = (s16)((bls1_val.gr >> 4) * 0.587 + (bls1_val.r >> 2) * 0.299 + (bls1_val.b >> 2) * 0.114 + 0.5);
            hist_bls_ratio = (float)((1 << 8) - 1) / ((1 << 8) - 1 - hist_bls1);
            break;
        }

        switch(mode) {
        case LEFT_MODE:
            for(int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
                tmp = (i - hist_bls1 >= 0) ? (i - hist_bls1) * hist_bls_ratio + 0.5 : 0;
                tmp = (tmp > ISP3X_HIST_BIN_N_MAX - 1) ? (ISP3X_HIST_BIN_N_MAX - 1) : tmp;
                merge_stats[tmp] += left_stats[i];
            }
            break;
        case RIGHT_MODE:
            for(int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
                tmp = (i - hist_bls1 >= 0) ? (i - hist_bls1) * hist_bls_ratio + 0.5 : 0;
                tmp = (tmp > ISP3X_HIST_BIN_N_MAX - 1) ? (ISP3X_HIST_BIN_N_MAX - 1) : tmp;
                merge_stats[tmp] += right_stats[i];
            }
            break;
        case LEFT_AND_RIGHT_MODE:
            for(int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
                tmp = (i - hist_bls1 >= 0) ? (i - hist_bls1) * hist_bls_ratio + 0.5 : 0;
                tmp = (tmp > ISP3X_HIST_BIN_N_MAX - 1) ? (ISP3X_HIST_BIN_N_MAX - 1) : tmp;
                merge_stats[tmp] += left_stats[i] + right_stats[i];
            }
            break;
        }

    } else {

        memset(merge_stats, 0, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
        switch(mode) {
        case LEFT_MODE:
            for(int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
                merge_stats[i] += left_stats[i];
            }
            break;
        case RIGHT_MODE:
            for(int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
                merge_stats[i] += right_stats[i];
            }
            break;
        case LEFT_AND_RIGHT_MODE:
            for(int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
                merge_stats[i] += left_stats[i] + right_stats[i];
            }
            break;
        }
    }

}

void MergeAwbWinStats(
    rk_aiq_awb_stat_wp_res_light_v201_t *merge_stats,
    struct isp3x_rawawb_meas_stat *left_stats,
    struct isp3x_rawawb_meas_stat *right_stats,
    int lightNum,
    WinSplitMode mode
) {
    switch(mode) {
    case LEFT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].RgainValue = left_stats->ro_rawawb_sum_rgain_nor[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].BgainValue = left_stats->ro_rawawb_sum_bgain_nor[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].WpNo = left_stats->ro_rawawb_wp_num_nor[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].RgainValue = left_stats->ro_rawawb_sum_rgain_big[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].BgainValue = left_stats->ro_rawawb_sum_bgain_big[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].WpNo = left_stats->ro_rawawb_wp_num_big[i];
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].RgainValue = right_stats->ro_rawawb_sum_rgain_nor[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].BgainValue = right_stats->ro_rawawb_sum_bgain_nor[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].WpNo = right_stats->ro_rawawb_wp_num_nor[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].RgainValue = right_stats->ro_rawawb_sum_rgain_big[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].BgainValue = right_stats->ro_rawawb_sum_bgain_big[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].WpNo = right_stats->ro_rawawb_wp_num_big[i];
        }
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < lightNum; i++) {
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].RgainValue =
                left_stats->ro_rawawb_sum_rgain_nor[i] + right_stats->ro_rawawb_sum_rgain_nor[i] ;
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].BgainValue =
                left_stats->ro_rawawb_sum_bgain_nor[i] +  right_stats->ro_rawawb_sum_bgain_nor[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].WpNo =
                left_stats->ro_rawawb_wp_num_nor[i] +  right_stats->ro_rawawb_wp_num_nor[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].RgainValue =
                left_stats->ro_rawawb_sum_rgain_big[i] +  right_stats->ro_rawawb_sum_rgain_big[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].BgainValue =
                left_stats->ro_rawawb_sum_bgain_big[i] +  right_stats->ro_rawawb_sum_bgain_big[i];
            merge_stats[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].WpNo =
                left_stats->ro_rawawb_wp_num_big[i] +  right_stats->ro_rawawb_wp_num_big[i];
        }
        break;
    default:
        break;
    }
}

void MergeAwbBlkStats(
    struct isp2x_window* ori_win,
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    rk_aiq_awb_stat_blk_res_v201_t *merge_stats,
    struct isp3x_rawawb_meas_stat *left_stats,
    struct isp3x_rawawb_meas_stat *right_stats,
    WinSplitMode mode
) {
    u8 wnd_num = sqrt(RK_AIQ_AWB_GRID_NUM_TOTAL);

    switch(mode) {
    case LEFT_MODE:
        for(int i = 0; i < wnd_num; i++) {
            for(int j = 0; j < wnd_num; j++) {
                merge_stats[i * wnd_num + j].Rvalue = left_stats->ramdata[i * wnd_num + j].r;
                merge_stats[i * wnd_num + j].Gvalue = left_stats->ramdata[i * wnd_num + j].g;
                merge_stats[i * wnd_num + j].Bvalue = left_stats->ramdata[i * wnd_num + j].b;
                merge_stats[i * wnd_num + j].WpNo = left_stats->ramdata[i * wnd_num + j].wp;
            }
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < wnd_num; i++) {
            for(int j = 0; j < wnd_num; j++) {
                merge_stats[i * wnd_num + j].Rvalue = right_stats->ramdata[i * wnd_num + j].r;
                merge_stats[i * wnd_num + j].Gvalue = right_stats->ramdata[i * wnd_num + j].g;
                merge_stats[i * wnd_num + j].Bvalue = right_stats->ramdata[i * wnd_num + j].b;
                merge_stats[i * wnd_num + j].WpNo = right_stats->ramdata[i * wnd_num + j].wp;
            }
        }
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < wnd_num; i++) {
            for(int j = 0; j < wnd_num; j++) {
                if(j < wnd_num / 2) {
                    merge_stats[i * wnd_num + j].Rvalue = left_stats->ramdata[i * wnd_num + j * 2].r + left_stats->ramdata[i * wnd_num + j * 2 + 1].r;
                    merge_stats[i * wnd_num + j].Gvalue = left_stats->ramdata[i * wnd_num + j * 2].g + left_stats->ramdata[i * wnd_num + j * 2 + 1].g;
                    merge_stats[i * wnd_num + j].Bvalue = left_stats->ramdata[i * wnd_num + j * 2].b +  left_stats->ramdata[i * wnd_num + j * 2 + 1].b;
                    merge_stats[i * wnd_num + j].WpNo = left_stats->ramdata[i * wnd_num + j * 2].wp + left_stats->ramdata[i * wnd_num + j * 2 + 1].wp;
                } else if(j > wnd_num / 2) {
                    merge_stats[i * wnd_num + j].Rvalue = right_stats->ramdata[i * wnd_num + j * 2 - wnd_num].r + right_stats->ramdata[i * wnd_num + j * 2 - wnd_num + 1].r;
                    merge_stats[i * wnd_num + j].Gvalue = right_stats->ramdata[i * wnd_num + j * 2 - wnd_num].g + right_stats->ramdata[i * wnd_num + j * 2 - wnd_num + 1].g;
                    merge_stats[i * wnd_num + j].Bvalue = right_stats->ramdata[i * wnd_num + j * 2 - wnd_num].b + right_stats->ramdata[i * wnd_num + j * 2 - wnd_num + 1].b;
                    merge_stats[i * wnd_num + j].WpNo = right_stats->ramdata[i * wnd_num + j * 2 - wnd_num].wp + right_stats->ramdata[i * wnd_num + j * 2 - wnd_num + 1].wp;
                } else {
                    merge_stats[i * wnd_num + j].Rvalue = left_stats->ramdata[i * wnd_num + wnd_num - 1].r + right_stats->ramdata[i * wnd_num + 0].r;
                    merge_stats[i * wnd_num + j].Gvalue = left_stats->ramdata[i * wnd_num + wnd_num - 1].g + right_stats->ramdata[i * wnd_num + 0].g;
                    merge_stats[i * wnd_num + j].Bvalue = left_stats->ramdata[i * wnd_num + wnd_num - 1].b + right_stats->ramdata[i * wnd_num + 0].b;
                    merge_stats[i * wnd_num + j].WpNo = left_stats->ramdata[i * wnd_num + wnd_num - 1].wp + right_stats->ramdata[i * wnd_num + 0].wp;
                }
            }
        }
        break;
    default:
        break;
    }

}

void MergeAwbHistBinStats(
    unsigned int *merge_stats,
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

void AwbStatOverflowCheckandFixed(struct isp2x_window* win, rk_aiq_awb_blk_stat_mode_v201_t blkMeasureMode, bool blkStatisticsWithLumaWeightEn, rk_aiq_awb_xy_type_v201_t xyRangeTypeForWpHist,
                                  int lightNum, struct isp3x_rawawb_meas_stat *stats)
{
    int w, h;
    w = win->h_size;
    h = win->v_size;
    float factor1 = (float)((1 << (RK_AIQ_AWB_WP_WEIGHT_BIS_V201 + 1)) - 1) / ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1);
    if(w * h > RK_AIQ_AWB_STAT_MAX_AREA) {
        LOGD_AWB("%s ramdata and ro_wp_num2 is fixed", __FUNCTION__);
        for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
            stats->ramdata[i].wp = (float)stats->ramdata[i].wp * factor1 + 0.5 ;
            stats->ramdata[i].r = (float)stats->ramdata[i].r * factor1 + 0.5 ;
            stats->ramdata[i].g = (float)stats->ramdata[i].g * factor1 + 0.5 ;
            stats->ramdata[i].b = (float)stats->ramdata[i].b * factor1 + 0.5 ;
        }
        if(xyRangeTypeForWpHist == RK_AIQ_AWB_XY_TYPE_BIG_V201) {
            for(int i = 0; i < lightNum; i++) {
                stats->ro_wp_num2[i] = stats->ro_rawawb_wp_num_big[i] >> RK_AIQ_WP_GAIN_FRAC_BIS;
            }
        } else {
            for(int i = 0; i < lightNum; i++) {
                stats->ro_wp_num2[i] = stats->ro_rawawb_wp_num_nor[i] >> RK_AIQ_WP_GAIN_FRAC_BIS;
            }
        }
    } else {
        if(RK_AIQ_AWB_BLK_STAT_MODE_REALWP_V201 == blkMeasureMode && blkStatisticsWithLumaWeightEn) {
            for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
                stats->ramdata[i].wp = (float)stats->ramdata[i].wp * factor1 + 0.5 ;
                stats->ramdata[i].r = (float)stats->ramdata[i].r * factor1 + 0.5 ;
                stats->ramdata[i].g = (float)stats->ramdata[i].g * factor1 + 0.5 ;
                stats->ramdata[i].b = (float)stats->ramdata[i].b * factor1 + 0.5 ;
            }
        }
    }
}

void MergeAwbMultiWinStats(
    rk_aiq_isp_awb_stats_v3x_t *merge_stats,
    struct isp3x_rawawb_meas_stat *left_stats,
    struct isp3x_rawawb_meas_stat *right_stats
) {
    for(int i = 0; i < RK_AIQ_AWB_MULTIWINDOW_NUM_V201; i++) {

        merge_stats->multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].RgainValue =
            left_stats->ro_sum_r_nor_multiwindow[i] + right_stats->ro_sum_r_nor_multiwindow[i];
        merge_stats->multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].BgainValue =
            left_stats->ro_sum_b_nor_multiwindow[i] + right_stats->ro_sum_b_nor_multiwindow[i];
        merge_stats->multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].WpNo =
            left_stats->ro_wp_nm_nor_multiwindow[i] + right_stats->ro_wp_nm_nor_multiwindow[i];
        merge_stats->multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].RgainValue =
            left_stats->ro_sum_r_big_multiwindow[i] + right_stats->ro_sum_r_big_multiwindow[i];
        merge_stats->multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].BgainValue =
            left_stats->ro_sum_b_big_multiwindow[i] + right_stats->ro_sum_b_big_multiwindow[i];
        merge_stats->multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].WpNo =
            left_stats->ro_wp_nm_big_multiwindow[i] + right_stats->ro_wp_nm_big_multiwindow[i];

    }
}

void MergeAwbExcWpStats(
    rk_aiq_awb_stat_wp_res_v201_t*merge_stats,
    isp3x_rawawb_meas_stat *left_stats,
    isp3x_rawawb_meas_stat *right_stats,
    WinSplitMode mode
) {
    switch(mode) {
    case LEFT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
            merge_stats[i].RgainValue = left_stats->ro_sum_r_exc[i];
            merge_stats[i].BgainValue = left_stats->ro_sum_b_exc[i];
            merge_stats[i].WpNo = left_stats->ro_wp_nm_exc[i];
        }
        break;
    case RIGHT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
            merge_stats[i].RgainValue = right_stats->ro_sum_r_exc[i];
            merge_stats[i].BgainValue = right_stats->ro_sum_b_exc[i];
            merge_stats[i].WpNo = right_stats->ro_wp_nm_exc[i];
        }
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
            merge_stats[i].RgainValue = left_stats->ro_sum_r_exc[i] + right_stats->ro_sum_r_exc[i];
            merge_stats[i].BgainValue = left_stats->ro_sum_b_exc[i] + right_stats->ro_sum_b_exc[i];
            merge_stats[i].WpNo = left_stats->ro_wp_nm_exc[i] + right_stats->ro_sum_b_exc[i];
        }
        break;
    }
}

XCamReturn
RkAiqResourceTranslatorV3x::translateMultiAecStats(const SmartPtr<VideoBuffer>& from, SmartPtr<RkAiqAecStatsProxy>& to)
{

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();

    SmartPtr<RkAiqAecStats> statsInt = to->data();

    struct rkisp3x_isp_stat_buffer *left_stats;
    struct rkisp3x_isp_stat_buffer *right_stats;

    left_stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if(left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    right_stats = left_stats + 1;
    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    if (left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
    } else {
        LOGD_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", mCamPhyId, left_stats->frame_id, left_stats->meas_type);
    }

    SmartPtr<RkAiqIrisParamsProxy> irisParams = buf->get_iris_params();
    SmartPtr<RkAiqSensorExpParamsProxy> expParams = nullptr;
    rkisp_effect_params_v20 ispParams;
    memset(&ispParams, 0, sizeof(ispParams));
    if (buf->getEffectiveExpParams(left_stats->frame_id, expParams) < 0)
        LOGE("fail to get expParams");
    if (buf->getEffectiveIspParams(left_stats->frame_id, ispParams) < 0) {
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    /*rawae stats*/
    uint8_t AeSwapMode, AeSelMode, AfUseAeBig;
    AeSwapMode = ispParams.isp_params_v3x[0].meas.rawae0.rawae_sel;
    AeSelMode = ispParams.isp_params_v3x[0].meas.rawae3.rawae_sel;
    AfUseAeBig = ispParams.isp_params_v3x[0].meas.rawaf.ae_mode;

    WinSplitMode AeWinSplitMode[4] = {LEFT_AND_RIGHT_MODE}; //0:rawae0 1:rawae1 2:rawae2 3:rawae3
    WinSplitMode HistWinSplitMode[4] = {LEFT_AND_RIGHT_MODE}; //0:rawhist0 1:rawhist1 2:rawhist2 3:rawhist3

    JudgeWinLocation(&ispParams.isp_params_v3x[0].meas.rawae0.win, AeWinSplitMode[0], left_isp_rect_, right_isp_rect_);
    JudgeWinLocation(&ispParams.isp_params_v3x[0].meas.rawae1.win, AeWinSplitMode[1], left_isp_rect_, right_isp_rect_);
    JudgeWinLocation(&ispParams.isp_params_v3x[0].meas.rawae2.win, AeWinSplitMode[2], left_isp_rect_, right_isp_rect_);
    JudgeWinLocation(&ispParams.isp_params_v3x[0].meas.rawae3.win, AeWinSplitMode[3], left_isp_rect_, right_isp_rect_);

    JudgeWinLocation(&ispParams.isp_params_v3x[0].meas.rawhist0.win, HistWinSplitMode[0], left_isp_rect_, right_isp_rect_);
    JudgeWinLocation(&ispParams.isp_params_v3x[0].meas.rawhist1.win, HistWinSplitMode[1], left_isp_rect_, right_isp_rect_);
    JudgeWinLocation(&ispParams.isp_params_v3x[0].meas.rawhist2.win, HistWinSplitMode[2], left_isp_rect_, right_isp_rect_);
    JudgeWinLocation(&ispParams.isp_params_v3x[0].meas.rawhist3.win, HistWinSplitMode[3], left_isp_rect_, right_isp_rect_);

    struct isp21_bls_cfg *bls_cfg = &ispParams.isp_params_v3x[0].others.bls_cfg;
    struct isp2x_bls_fixed_val bls1_val; //bls1_val = blc1_ori_val * awb * range_ratio
    float bls_ratio[3] = {1, 1, 1};
    u32 pixel_num[ISP3X_RAWAEBIG_SUBWIN_NUM] = {0};
    bool is_bls1_en = bls_cfg->bls1_en;

    if(is_bls1_en) {
        bls1_val.r = bls_cfg->bls1_val.r >> 2;
        bls1_val.gr = bls_cfg->bls1_val.gr;
        bls1_val.gb = bls_cfg->bls1_val.gb;
        bls1_val.b = bls_cfg->bls1_val.b >> 2;

        bls_ratio[0] = (float)((1 << 10) - 1) / ((1 << 10) - 1 - bls_cfg->bls1_val.r);
        bls_ratio[1] = (float)((1 << 12) - 1) / ((1 << 12) - 1 - bls_cfg->bls1_val.gr);
        bls_ratio[2] = (float)((1 << 10) - 1) / ((1 << 10) - 1 - bls_cfg->bls1_val.b);

    } else {
        bls1_val.r = 0;
        bls1_val.gr = 0;
        bls1_val.gb = 0;
        bls1_val.b = 0;
    }

#ifdef AE_STATS_DEBUG
    LOGE("bls1[%d-%d-%d-%d]", bls1_val.r, bls1_val.gr, bls1_val.gb, bls1_val.b);
    LOGE("bls_ratio[%f-%f-%f]", bls_ratio[0], bls_ratio[1], bls_ratio[2]);

    _aeAlgoStatsCfg.UpdateStats = true;
    _aeAlgoStatsCfg.RawStatsChnSel = RAWSTATS_CHN_ALL_EN;
#endif

    unsigned int meas_type = 0;
    u8 index0, index1, index2;
    u8 rawhist_mode = 0, rawhist3_mode = 0;

    switch (AeSwapMode) {
    case AEC_RAWSWAP_MODE_S_LITE:
        meas_type = ((left_stats->meas_type >> 7) & (0x01)) & ((left_stats->meas_type >> 11) & (0x01));
        index0 = 0;
        index1 = 1;
        index2 = 2;
        rawhist_mode = ispParams.isp_params_v3x[0].meas.rawhist0.mode;
        break;
    case AEC_RAWSWAP_MODE_M_LITE:
        meas_type = ((left_stats->meas_type >> 8) & (0x01)) & ((left_stats->meas_type >> 12) & (0x01));
        index0 = 1;
        index1 = 0;
        index2 = 2;
        rawhist_mode = ispParams.isp_params_v3x[0].meas.rawhist1.mode;
        break;
    case AEC_RAWSWAP_MODE_L_LITE:
        meas_type = ((left_stats->meas_type >> 9) & (0x01)) & ((left_stats->meas_type >> 13) & (0x01));
        index0 = 2;
        index1 = 1;
        index2 = 0;
        rawhist_mode = ispParams.isp_params_v3x[0].meas.rawhist2.mode;
        break;
    default:
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
        break;
    }

    //ae stats v3.x
    statsInt->frame_id = left_stats->frame_id;
    statsInt->af_prior = (AfUseAeBig == 0) ? false : true;
    statsInt->aec_stats_valid = (meas_type & 0x01) ? true : false;
    if (!statsInt->aec_stats_valid)
        return XCAM_RETURN_BYPASS;

    MergeAecHistBinStats(left_stats->params.rawhist0.hist_bin,
                         right_stats->params.rawhist0.hist_bin,
                         statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins,
                         HistWinSplitMode[0], rawhist_mode,
                         bls1_val, bls_ratio, is_bls1_en);

    MergeAecHistBinStats(left_stats->params.rawhist1.hist_bin,
                         right_stats->params.rawhist1.hist_bin,
                         statsInt->aec_stats.ae_data.chn[index1].rawhist_big.bins,
                         HistWinSplitMode[1], rawhist_mode,
                         bls1_val, bls_ratio, is_bls1_en);

    MergeAecHistBinStats(left_stats->params.rawhist2.hist_bin,
                         right_stats->params.rawhist2.hist_bin,
                         statsInt->aec_stats.ae_data.chn[index2].rawhist_big.bins,
                         HistWinSplitMode[2], rawhist_mode, bls1_val,
                         bls_ratio, is_bls1_en);

    if (!AfUseAeBig && (AeSelMode <= AEC_RAWSEL_MODE_CHN_2)) {
        rawhist3_mode = ispParams.isp_params_v3x[0].meas.rawhist3.mode;
        MergeAecHistBinStats(left_stats->params.rawhist3.hist_bin,
                             right_stats->params.rawhist3.hist_bin,
                             statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins,
                             HistWinSplitMode[3], rawhist3_mode,
                             bls1_val, bls_ratio, is_bls1_en);
    }

    // calc ae stats run flag
    uint64_t SumHistPix[3] = { 0, 0, 0 };
    uint64_t SumHistBin[3] = { 0, 0, 0 };
    uint16_t HistMean[3] = { 0, 0, 0 };
    u32* hist_bin[3];

    hist_bin[index0] = statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins;
    hist_bin[index1] = statsInt->aec_stats.ae_data.chn[index1].rawhist_big.bins;
    hist_bin[index2] = statsInt->aec_stats.ae_data.chn[index2].rawhist_big.bins;
    /*if (!AfUseAeBig && (AeSelMode <= AEC_RAWSEL_MODE_CHN_2)) {
        hist_bin[AeSelMode] = statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins;
    }*/

    for (int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
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
        //chn index0 => rawae0 rawhist0
        MergeAecWinLiteStats(&left_stats->params.rawae0, &right_stats->params.rawae0,
                             &statsInt->aec_stats.ae_data.chn[index0].rawae_lite,
                             &statsInt->aec_stats.ae_data.raw_mean[index0],
                             _aeAlgoStatsCfg.LiteWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                             AeWinSplitMode[0], bls1_val, bls_ratio);

        //chn index1 => rawae1 rawhist1
        MergeAecWinBigStats(&left_stats->params.rawae1, &right_stats->params.rawae1,
                            &statsInt->aec_stats.ae_data.chn[index1].rawae_big,
                            &statsInt->aec_stats.ae_data.raw_mean[index1],
                            _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                            AeWinSplitMode[1], bls1_val, bls_ratio);

        pixel_num[0] = ispParams.isp_params_v3x[0].meas.rawae1.subwin[0].h_size * ispParams.isp_params_v3x[0].meas.rawae1.subwin[0].v_size;
        pixel_num[1] = ispParams.isp_params_v3x[0].meas.rawae1.subwin[1].h_size * ispParams.isp_params_v3x[0].meas.rawae1.subwin[1].v_size;
        pixel_num[2] = ispParams.isp_params_v3x[0].meas.rawae1.subwin[2].h_size * ispParams.isp_params_v3x[0].meas.rawae1.subwin[2].v_size;
        pixel_num[3] = ispParams.isp_params_v3x[0].meas.rawae1.subwin[3].h_size * ispParams.isp_params_v3x[0].meas.rawae1.subwin[3].v_size;
        MergeAecSubWinStats(&left_stats->params.rawae1,
                            &right_stats->params.rawae1,
                            &statsInt->aec_stats.ae_data.chn[index1].rawae_big,
                            ispParams.isp_params_v3x[1].meas.rawae1.subwin_en, ispParams.isp_params_v3x[2].meas.rawae1.subwin_en,
                            bls1_val, bls_ratio, pixel_num);

        //chn index2 => rawae2 rawhist2
        MergeAecWinBigStats(&left_stats->params.rawae2, &right_stats->params.rawae2,
                            &statsInt->aec_stats.ae_data.chn[index2].rawae_big,
                            &statsInt->aec_stats.ae_data.raw_mean[index2],
                            _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                            AeWinSplitMode[2], bls1_val, bls_ratio);

        pixel_num[0] = ispParams.isp_params_v3x[0].meas.rawae2.subwin[0].h_size * ispParams.isp_params_v3x[0].meas.rawae2.subwin[0].v_size;
        pixel_num[1] = ispParams.isp_params_v3x[0].meas.rawae2.subwin[1].h_size * ispParams.isp_params_v3x[0].meas.rawae2.subwin[1].v_size;
        pixel_num[2] = ispParams.isp_params_v3x[0].meas.rawae2.subwin[2].h_size * ispParams.isp_params_v3x[0].meas.rawae2.subwin[2].v_size;
        pixel_num[3] = ispParams.isp_params_v3x[0].meas.rawae2.subwin[3].h_size * ispParams.isp_params_v3x[0].meas.rawae2.subwin[3].v_size;
        MergeAecSubWinStats(&left_stats->params.rawae2,
                            &right_stats->params.rawae2,
                            &statsInt->aec_stats.ae_data.chn[index2].rawae_big,
                            ispParams.isp_params_v3x[1].meas.rawae2.subwin_en, ispParams.isp_params_v3x[2].meas.rawae2.subwin_en,
                            bls1_val, bls_ratio, pixel_num);

        if(!AfUseAeBig) {
            switch(AeSelMode) {
            case AEC_RAWSEL_MODE_CHN_0:
            case AEC_RAWSEL_MODE_CHN_1:
            case AEC_RAWSEL_MODE_CHN_2:
                //chn [AeSelMode] => rawae3 rawhist3
                MergeAecWinBigStats(&left_stats->params.rawae3, &right_stats->params.rawae3,
                                    &statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big,
                                    &statsInt->aec_stats.ae_data.raw_mean[AeSelMode],
                                    _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                                    AeWinSplitMode[3], bls1_val, bls_ratio);

                pixel_num[0] = ispParams.isp_params_v3x[0].meas.rawae3.subwin[0].h_size * ispParams.isp_params_v3x[0].meas.rawae3.subwin[0].v_size;
                pixel_num[1] = ispParams.isp_params_v3x[0].meas.rawae3.subwin[1].h_size * ispParams.isp_params_v3x[0].meas.rawae3.subwin[1].v_size;
                pixel_num[2] = ispParams.isp_params_v3x[0].meas.rawae3.subwin[2].h_size * ispParams.isp_params_v3x[0].meas.rawae3.subwin[2].v_size;
                pixel_num[3] = ispParams.isp_params_v3x[0].meas.rawae3.subwin[3].h_size * ispParams.isp_params_v3x[0].meas.rawae3.subwin[3].v_size;
                MergeAecSubWinStats(&left_stats->params.rawae3,
                                    &right_stats->params.rawae3,
                                    &statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big,
                                    ispParams.isp_params_v3x[1].meas.rawae3.subwin_en, ispParams.isp_params_v3x[2].meas.rawae3.subwin_en,
                                    bls1_val, bls_ratio, pixel_num);

                break;

            case AEC_RAWSEL_MODE_TMO:
                bls1_val.r = 0;
                bls1_val.gr = 0;
                bls1_val.gb = 0;
                bls1_val.b = 0;

                bls_ratio[0] = 1;
                bls_ratio[1] = 1;
                bls_ratio[2] = 1;

                //extra => rawae3 rawhist3
                MergeAecWinBigStats(&left_stats->params.rawae3, &right_stats->params.rawae3,
                                    &statsInt->aec_stats.ae_data.extra.rawae_big,
                                    &statsInt->aec_stats.ae_data.raw_mean[AeSelMode],
                                    _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                                    AeWinSplitMode[3], bls1_val, bls_ratio);

                pixel_num[0] = ispParams.isp_params_v3x[0].meas.rawae3.subwin[0].h_size * ispParams.isp_params_v3x[0].meas.rawae3.subwin[0].v_size;
                pixel_num[1] = ispParams.isp_params_v3x[0].meas.rawae3.subwin[1].h_size * ispParams.isp_params_v3x[0].meas.rawae3.subwin[1].v_size;
                pixel_num[2] = ispParams.isp_params_v3x[0].meas.rawae3.subwin[2].h_size * ispParams.isp_params_v3x[0].meas.rawae3.subwin[2].v_size;
                pixel_num[3] = ispParams.isp_params_v3x[0].meas.rawae3.subwin[3].h_size * ispParams.isp_params_v3x[0].meas.rawae3.subwin[3].v_size;
                MergeAecSubWinStats(&left_stats->params.rawae3,
                                    &right_stats->params.rawae3,
                                    &statsInt->aec_stats.ae_data.extra.rawae_big,
                                    ispParams.isp_params_v3x[1].meas.rawae3.subwin_en, ispParams.isp_params_v3x[2].meas.rawae3.subwin_en,
                                    bls1_val, bls_ratio, pixel_num);

                rawhist3_mode = ispParams.isp_params_v3x[0].meas.rawhist3.mode;
                MergeAecHistBinStats(left_stats->params.rawhist3.hist_bin,
                                     right_stats->params.rawhist3.hist_bin,
                                     statsInt->aec_stats.ae_data.extra.rawhist_big.bins,
                                     HistWinSplitMode[3], rawhist3_mode,
                                     bls1_val, bls_ratio, false);

                break;

            default:
                LOGE("wrong AeSelMode=%d\n", AeSelMode);
                return XCAM_RETURN_ERROR_PARAM;
            }
        }
        _lastAeStats = statsInt->aec_stats.ae_data;
    } else {
        statsInt->aec_stats.ae_data = _lastAeStats;
    }

#ifdef AE_STATS_DEBUG
    if(AeSwapMode != 0) {
        for(int i = 0; i < 15; i++) {
            for(int j = 0; j < 15; j++) {
                printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i * 15 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i * 15 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i * 15 + j]);
            }
        }
        printf("====================sub-win-result======================\n");

        for(int i = 0; i < 4; i++)
            printf("chn0_subwin[%d]:sumr 0x%08lx, sumg 0x%08lx, sumb 0x%08lx\n", i,
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i],
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i],
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i]);

        printf("====================hist_result========================\n");
        for(int i = 0; i < 256; i++)
            printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins[i]);

    } else {
        for(int i = 0; i < 5; i++) {
            for(int j = 0; j < 5; j++) {
                printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelr_xy[i * 5 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelg_xy[i * 5 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelb_xy[i * 5 + j]);
            }
        }

        printf("====================hist_result========================\n");
        for(int i = 0; i < 256; i++)
            printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[0].rawhist_lite.bins[i]);

    }
#endif

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

    to->set_sequence(left_stats->frame_id);

    return ret;

}

XCamReturn RkAiqResourceTranslatorV3x::translateMultiAwbStats(const SmartPtr<VideoBuffer>& from, SmartPtr<RkAiqAwbStatsProxy>& to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();

    SmartPtr<RkAiqAwbStats> statsInt = to->data();

    struct rkisp3x_isp_stat_buffer *left_stats;
    struct rkisp3x_isp_stat_buffer *right_stats;

    left_stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if(left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    right_stats = left_stats + 1;
    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    if (left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
    } else {
        LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", left_stats->frame_id, left_stats->meas_type);
    }


    statsInt->awb_stats_valid = left_stats->meas_type >> 5 & 1;
    if (!statsInt->awb_stats_valid) {
        LOGE_ANALYZER("AWB stats invalid, ignore");
        return XCAM_RETURN_BYPASS;
    }

    rkisp_effect_params_v20 ispParams;
    memset(&ispParams, 0, sizeof(ispParams));
    if (buf->getEffectiveIspParams(left_stats->frame_id, ispParams) < 0) {
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    rk_aiq_isp_blc_t *bls_cfg = &ispParams.blc_cfg.v0;
    statsInt->blc_cfg_effect = ispParams.blc_cfg.v0;
    //awb30
    statsInt->awb_stats_v3x.awb_cfg_effect_v201 = ispParams.awb_cfg_v3x;
    statsInt->awb_cfg_effect_valid = true;
    statsInt->frame_id = left_stats->frame_id;

    WinSplitMode AwbWinSplitMode = LEFT_AND_RIGHT_MODE;

    struct isp2x_window ori_win;
    ori_win.h_offs = ispParams.isp_params_v3x[0].meas.rawawb.sw_rawawb_h_offs;
    ori_win.h_size = ispParams.isp_params_v3x[0].meas.rawawb.sw_rawawb_h_size;
    ori_win.v_offs = ispParams.isp_params_v3x[0].meas.rawawb.sw_rawawb_v_offs;
    ori_win.v_size = ispParams.isp_params_v3x[0].meas.rawawb.sw_rawawb_v_size;

    JudgeWinLocation(&ori_win, AwbWinSplitMode, left_isp_rect_, right_isp_rect_);

    MergeAwbWinStats(statsInt->awb_stats_v3x.light, &left_stats->params.rawawb, &right_stats->params.rawawb,
                     statsInt->awb_stats_v3x.awb_cfg_effect_v201.lightNum, AwbWinSplitMode);

    struct isp2x_window left_win;
    left_win.h_offs = ispParams.isp_params_v3x[1].meas.rawawb.sw_rawawb_h_offs;
    left_win.h_size = ispParams.isp_params_v3x[1].meas.rawawb.sw_rawawb_h_size;
    left_win.v_offs = ispParams.isp_params_v3x[1].meas.rawawb.sw_rawawb_v_offs;
    left_win.v_size = ispParams.isp_params_v3x[1].meas.rawawb.sw_rawawb_v_size;

    struct isp2x_window right_win;
    right_win.h_offs = ispParams.isp_params_v3x[2].meas.rawawb.sw_rawawb_h_offs;
    right_win.h_size = ispParams.isp_params_v3x[2].meas.rawawb.sw_rawawb_h_size;
    right_win.v_offs = ispParams.isp_params_v3x[2].meas.rawawb.sw_rawawb_v_offs;
    right_win.v_size = ispParams.isp_params_v3x[2].meas.rawawb.sw_rawawb_v_size;

    // to fixed the bug in ic design that some egisters maybe overflow
    AwbStatOverflowCheckandFixed(&left_win,
                                 (rk_aiq_awb_blk_stat_mode_v201_t)ispParams.isp_params_v3x[1].meas.rawawb.sw_rawawb_blk_measure_mode,
                                 ispParams.isp_params_v3x[1].meas.rawawb.sw_rawawb_blk_with_luma_wei_en,
                                 (rk_aiq_awb_xy_type_v201_t)ispParams.isp_params_v3x[1].meas.rawawb.sw_rawawb_wp_hist_xytype,
                                 ispParams.isp_params_v3x[1].meas.rawawb.sw_rawawb_light_num, &left_stats->params.rawawb);
    AwbStatOverflowCheckandFixed(&right_win,
                                 (rk_aiq_awb_blk_stat_mode_v201_t)ispParams.isp_params_v3x[2].meas.rawawb.sw_rawawb_blk_measure_mode,
                                 ispParams.isp_params_v3x[2].meas.rawawb.sw_rawawb_blk_with_luma_wei_en,
                                 (rk_aiq_awb_xy_type_v201_t)ispParams.isp_params_v3x[2].meas.rawawb.sw_rawawb_wp_hist_xytype,
                                 ispParams.isp_params_v3x[2].meas.rawawb.sw_rawawb_light_num, &right_stats->params.rawawb);


    MergeAwbBlkStats(&ori_win, &left_win, &right_win, statsInt->awb_stats_v3x.blockResult, &left_stats->params.rawawb, &right_stats->params.rawawb, AwbWinSplitMode);

    MergeAwbHistBinStats(statsInt->awb_stats_v3x.WpNoHist, left_stats->params.rawawb.ro_yhist_bin, right_stats->params.rawawb.ro_yhist_bin, AwbWinSplitMode);

#if defined(ISP_HW_V30)
    switch(AwbWinSplitMode) {
    case LEFT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v3x.awb_cfg_effect_v201.lightNum; i++)
            statsInt->awb_stats_v3x.WpNo2[i] = left_stats->params.rawawb.ro_wp_num2[i];
        break;
    case RIGHT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v3x.awb_cfg_effect_v201.lightNum; i++)
            statsInt->awb_stats_v3x.WpNo2[i] = right_stats->params.rawawb.ro_wp_num2[i];
        break;
    case LEFT_AND_RIGHT_MODE:
        for(int i = 0; i < statsInt->awb_stats_v3x.awb_cfg_effect_v201.lightNum; i++)
            statsInt->awb_stats_v3x.WpNo2[i] = left_stats->params.rawawb.ro_wp_num2[i] + right_stats->params.rawawb.ro_wp_num2[i];
        break;
    default:
        break;
    }

    MergeAwbMultiWinStats(&statsInt->awb_stats_v3x, &left_stats->params.rawawb, &right_stats->params.rawawb);
    MergeAwbExcWpStats( statsInt->awb_stats_v3x.excWpRangeResult, &left_stats->params.rawawb, &right_stats->params.rawawb, AwbWinSplitMode);

#endif

    LOG1_AWB("bls_cfg %p", bls_cfg);
    if(bls_cfg) {
        LOG1_AWB("bls1_enalbe: %d, b r gb gr:[ %d %d %d %d]", bls_cfg->blc1_enable, bls_cfg->blc1_b,
                 bls_cfg->blc1_r, bls_cfg->blc1_gb, bls_cfg->blc1_gr);
    }
    if(bls_cfg && bls_cfg->blc1_enable && (bls_cfg->blc1_b > 0 || bls_cfg->blc1_r > 0 || bls_cfg->blc1_gb > 0 || bls_cfg->blc1_gr > 0)) {

        for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
            statsInt->awb_stats_v3x.blockResult[i].Rvalue -=
                (long long)(statsInt->awb_stats_v3x.blockResult[i].WpNo * bls_cfg->blc1_r *
                            statsInt->awb_stats_v3x.awb_cfg_effect_v201.pre_wbgain_inv_r + 2048) >> 12 ;
            statsInt->awb_stats_v3x.blockResult[i].Gvalue -=
                (long long)(statsInt->awb_stats_v3x.blockResult[i].WpNo * (bls_cfg->blc1_gr + bls_cfg->blc1_gb) *
                            statsInt->awb_stats_v3x.awb_cfg_effect_v201.pre_wbgain_inv_g + 4096) >> 13 ;
            statsInt->awb_stats_v3x.blockResult[i].Bvalue -=
                (long long)(statsInt->awb_stats_v3x.blockResult[i].WpNo * bls_cfg->blc1_b  *
                            statsInt->awb_stats_v3x.awb_cfg_effect_v201.pre_wbgain_inv_b + 2048) >> 12 ; ;
        }
    }

    //statsInt->awb_stats_valid = ISP2X_STAT_RAWAWB(left_stats->meas_type)? true:false;
    statsInt->awb_stats_valid = left_stats->meas_type >> 5 & 1;
    to->set_sequence(left_stats->frame_id);
    return ret;

}

XCamReturn
RkAiqResourceTranslatorV3x::translateMultiAfStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAfStatsProxy> &to)
{
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

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
    struct rkisp3x_isp_stat_buffer *left_stats, *right_stats;
    SmartPtr<RkAiqAfStats> statsInt = to->data();
    SmartPtr<RkAiqAfInfoProxy> afParams = buf->get_af_params();

    left_stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if (left_stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    right_stats = left_stats + 1;
    if (right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    if (left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
    } else {
        LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", left_stats->frame_id, left_stats->meas_type);
    }

    rkisp_effect_params_v20 ispParams;
    memset(&ispParams, 0, sizeof(ispParams));
    if (buf->getEffectiveIspParams(left_stats->frame_id, ispParams) < 0) {
        LOGE("fail to get ispParams ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    SmartPtr<RkAiqSensorExpParamsProxy> expParams = nullptr;
    if (buf->getEffectiveExpParams(left_stats->frame_id, expParams) < 0)
        LOGE("fail to get expParams");
    if (expParams.ptr())
        statsInt->aecExpInfo = expParams->data()->aecExpInfo;

    statsInt->frame_id = left_stats->frame_id;

    struct isp3x_rawaf_meas_cfg &org_af = ispParams.isp_params_v3x[0].meas.rawaf;
    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t x_st, x_ed, l_blknum, r_blknum, ov_w, blk_w, r_skip_blknum;
    struct AfSplitInfo af_split_info;
    int32_t i, j, k, dst_idx, l_idx, r_idx, l_lht, r_lht, lht0, lht1;

    memset(&af_split_info, 0, sizeof(af_split_info));
    ov_w = left_isp_rect_.w + left_isp_rect_.x - right_isp_rect_.x;
    x_st = org_af.win[0].h_offs;
    x_ed = x_st + org_af.win[0].h_size;
    l_isp_st = left_isp_rect_.x;
    l_isp_ed = left_isp_rect_.x + left_isp_rect_.w;
    r_isp_st = right_isp_rect_.x;
    r_isp_ed = right_isp_rect_.x + right_isp_rect_.w;
    LOGD_AF("wina.x_st %d, wina.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            x_st, x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    //// winA ////
    af_split_info.wina_l_ratio = 0;
    af_split_info.wina_r_ratio = 0;
    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        af_split_info.wina_side_info = LEFT_AND_RIGHT_MODE;
        // af win < one isp width
        if (org_af.win[0].h_size < left_isp_rect_.w) {
            blk_w = org_af.win[0].h_size / ISP2X_RAWAF_SUMDATA_ROW;
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
        else if (org_af.win[0].h_size < left_isp_rect_.w * 3 / 2) {
            l_win_st = x_st;
            l_win_ed = l_isp_ed - 2;
            blk_w = (l_win_ed - l_win_st) / (ISP2X_RAWAF_SUMDATA_ROW + 1);
            l_win_st = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum = ((l_win_ed - l_win_st) * ISP2X_RAWAF_SUMDATA_ROW + org_af.win[0].h_size - 1) / org_af.win[0].h_size;
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
        r_win_st = x_st - right_isp_rect_.x;
        r_win_ed = x_ed - right_isp_rect_.x;
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
    x_st = org_af.win[1].h_offs;
    x_ed = x_st + org_af.win[1].h_size;
    LOGD_AF("winb.x_st %d, winb.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            x_st, x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        af_split_info.winb_side_info = LEFT_AND_RIGHT_MODE;
        l_win_st = x_st;
        l_win_ed = l_isp_ed - 2;
        r_win_st = ov_w - 2;
        r_win_ed = x_ed - right_isp_rect_.x;
        // blend winB by width of left isp winB and right isp winB
        af_split_info.winb_l_ratio = (float)(l_win_ed - l_win_st) / (float)(x_ed - x_st);
        af_split_info.winb_r_ratio = 1 - af_split_info.winb_l_ratio;
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        af_split_info.winb_side_info = RIGHT_MODE;
        af_split_info.winb_l_ratio = 0;
        af_split_info.winb_r_ratio = 1;
        r_win_st = x_st - right_isp_rect_.x;
        r_win_ed = x_ed - right_isp_rect_.x;
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

    //af
    {
        if (((left_stats->meas_type >> 6) & (0x01)) && ((left_stats->meas_type >> 6) & (0x01)))
            statsInt->af_stats_valid = true;
        else
            statsInt->af_stats_valid = false;

        if (af_split_info.winb_side_info == LEFT_AND_RIGHT_MODE) {
            statsInt->af_stats_v3x.wndb_luma = left_stats->params.rawaf.afm_lum_b * af_split_info.winb_l_ratio +
                                               right_stats->params.rawaf.afm_lum_b * af_split_info.winb_r_ratio;
            statsInt->af_stats_v3x.wndb_sharpness = left_stats->params.rawaf.afm_sum_b * af_split_info.winb_l_ratio +
                                                    right_stats->params.rawaf.afm_sum_b * af_split_info.winb_r_ratio;
            statsInt->af_stats_v3x.winb_highlit_cnt = left_stats->params.rawaf.highlit_cnt_winb +
                    right_stats->params.rawaf.highlit_cnt_winb;
        } else if (af_split_info.winb_side_info == LEFT_MODE) {
            statsInt->af_stats_v3x.wndb_luma = left_stats->params.rawaf.afm_lum_b;
            statsInt->af_stats_v3x.wndb_sharpness = left_stats->params.rawaf.afm_sum_b;
            statsInt->af_stats_v3x.winb_highlit_cnt = left_stats->params.rawaf.highlit_cnt_winb;
        } else {
            statsInt->af_stats_v3x.wndb_luma = right_stats->params.rawaf.afm_lum_b;
            statsInt->af_stats_v3x.wndb_sharpness = right_stats->params.rawaf.afm_sum_b;
            statsInt->af_stats_v3x.winb_highlit_cnt = right_stats->params.rawaf.highlit_cnt_winb;
        }

        if (af_split_info.wina_side_info == FULL_MODE) {
            for (i = 0; i < ISP2X_RAWAF_SUMDATA_ROW; i++) {
                for (j = 0; j < ISP2X_RAWAF_SUMDATA_COLUMN; j++) {
                    dst_idx = i * ISP2X_RAWAF_SUMDATA_ROW + j;
                    if (j == 0) {
                        l_idx = i * ISP2X_RAWAF_SUMDATA_ROW + j;
                        statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] = left_stats->params.rawaf.ramdata[l_idx].v1;
                        statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] = left_stats->params.rawaf.ramdata[l_idx].v2;
                        statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] = left_stats->params.rawaf.ramdata[l_idx].h1;
                        statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] = left_stats->params.rawaf.ramdata[l_idx].h2;
                        statsInt->af_stats_v3x.wnda_luma[dst_idx] = left_stats->params.rawae3.data[l_idx].channelg_xy;
                        statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] =
                            ((left_stats->params.rawae3.data[l_idx].channelr_xy & 0x3F) << 10) | left_stats->params.rawae3.data[l_idx].channelb_xy;
                    } else if (j >= 1 && j <= 7) {
                        l_idx = i * ISP2X_RAWAF_SUMDATA_ROW + 2 * (j - 1) + 1;
                        statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] =
                            left_stats->params.rawaf.ramdata[l_idx].v1 + left_stats->params.rawaf.ramdata[l_idx + 1].v1;
                        statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] =
                            left_stats->params.rawaf.ramdata[l_idx].v2 + left_stats->params.rawaf.ramdata[l_idx + 1].v2;
                        statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] =
                            left_stats->params.rawaf.ramdata[l_idx].h1 + left_stats->params.rawaf.ramdata[l_idx + 1].h1;
                        statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] =
                            left_stats->params.rawaf.ramdata[l_idx].h2 + left_stats->params.rawaf.ramdata[l_idx + 1].h2;
                        statsInt->af_stats_v3x.wnda_luma[dst_idx] =
                            (left_stats->params.rawae3.data[l_idx].channelg_xy + left_stats->params.rawae3.data[l_idx + 1].channelg_xy) >> 1;
                        lht0 = ((left_stats->params.rawae3.data[l_idx].channelr_xy & 0x3F) << 10) | left_stats->params.rawae3.data[l_idx].channelb_xy;
                        lht1 = ((left_stats->params.rawae3.data[l_idx + 1].channelr_xy & 0x3F) << 10) | left_stats->params.rawae3.data[l_idx + 1].channelb_xy;
                        statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] = lht0 + lht1;
                    } else {
                        r_idx = i * ISP2X_RAWAF_SUMDATA_ROW + 2 * (j - 8) + 1;
                        statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] =
                            right_stats->params.rawaf.ramdata[r_idx].v1 + right_stats->params.rawaf.ramdata[r_idx + 1].v1;
                        statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] =
                            right_stats->params.rawaf.ramdata[r_idx].v2 + right_stats->params.rawaf.ramdata[r_idx + 1].v2;
                        statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] =
                            right_stats->params.rawaf.ramdata[r_idx].h1 + right_stats->params.rawaf.ramdata[r_idx + 1].h1;
                        statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] =
                            right_stats->params.rawaf.ramdata[r_idx].h2 + right_stats->params.rawaf.ramdata[r_idx + 1].h2;
                        statsInt->af_stats_v3x.wnda_luma[dst_idx] =
                            (right_stats->params.rawae3.data[r_idx].channelg_xy + right_stats->params.rawae3.data[r_idx + 1].channelg_xy) >> 1;
                        lht0 = ((right_stats->params.rawae3.data[r_idx].channelr_xy & 0x3F) << 10) | right_stats->params.rawae3.data[r_idx].channelb_xy;
                        lht1 = ((right_stats->params.rawae3.data[r_idx + 1].channelr_xy & 0x3F) << 10) | right_stats->params.rawae3.data[r_idx + 1].channelb_xy;
                        statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] = lht0 + lht1;
                    }
                }
            }
        }
        else if (af_split_info.wina_side_info == LEFT_AND_RIGHT_MODE) {
            for (i = 0; i < ISP2X_RAWAF_SUMDATA_ROW; i++) {
                j = ISP2X_RAWAF_SUMDATA_ROW - af_split_info.wina_l_blknum;
                for (k = 0; k < af_split_info.wina_l_blknum; j++, k++) {
                    dst_idx = i * ISP2X_RAWAF_SUMDATA_ROW + k;
                    l_idx = i * ISP2X_RAWAF_SUMDATA_ROW + j;
                    statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] = left_stats->params.rawaf.ramdata[l_idx].v1;
                    statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] = left_stats->params.rawaf.ramdata[l_idx].v2;
                    statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] = left_stats->params.rawaf.ramdata[l_idx].h1;
                    statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] = left_stats->params.rawaf.ramdata[l_idx].h2;
                    statsInt->af_stats_v3x.wnda_luma[dst_idx] = left_stats->params.rawae3.data[l_idx].channelg_xy;
                    statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] =
                        ((left_stats->params.rawae3.data[l_idx].channelr_xy & 0x3F) << 10) | left_stats->params.rawae3.data[l_idx].channelb_xy;
                }
            }

            for (i = 0; i < ISP2X_RAWAF_SUMDATA_ROW; i++) {
                j = af_split_info.wina_r_skip_blknum;
                for (k = 0; k < af_split_info.wina_r_blknum; j++, k++) {
                    dst_idx = i * ISP2X_RAWAF_SUMDATA_ROW + k + af_split_info.wina_l_blknum;
                    r_idx = i * ISP2X_RAWAF_SUMDATA_ROW + j;
                    statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] = right_stats->params.rawaf.ramdata[r_idx].v1;
                    statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] = right_stats->params.rawaf.ramdata[r_idx].v2;
                    statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] = right_stats->params.rawaf.ramdata[r_idx].h1;
                    statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] = right_stats->params.rawaf.ramdata[r_idx].h2;

                    statsInt->af_stats_v3x.wnda_luma[dst_idx] = right_stats->params.rawae3.data[r_idx].channelg_xy;
                    statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] =
                        ((right_stats->params.rawae3.data[r_idx].channelr_xy & 0x3F) << 10) | right_stats->params.rawae3.data[r_idx].channelb_xy;
                }
            }

            if (af_split_info.wina_r_skip_blknum == 0) {
                for (j = 0; j < ISP2X_RAWAF_SUMDATA_COLUMN; j++) {
                    dst_idx = j * ISP2X_RAWAF_SUMDATA_ROW + (af_split_info.wina_l_blknum - 1);
                    l_idx = j * ISP2X_RAWAF_SUMDATA_ROW + (ISP2X_RAWAF_SUMDATA_COLUMN - 1);
                    r_idx = j * ISP2X_RAWAF_SUMDATA_ROW;
                    statsInt->af_stats_v3x.wnda_fv_v1[dst_idx] =
                        left_stats->params.rawaf.ramdata[l_idx].v1 * af_split_info.wina_l_ratio +
                        right_stats->params.rawaf.ramdata[r_idx].v1 * af_split_info.wina_r_ratio;
                    statsInt->af_stats_v3x.wnda_fv_v2[dst_idx] =
                        left_stats->params.rawaf.ramdata[l_idx].v2 * af_split_info.wina_l_ratio +
                        right_stats->params.rawaf.ramdata[r_idx].v2 * af_split_info.wina_r_ratio;
                    statsInt->af_stats_v3x.wnda_fv_h1[dst_idx] =
                        left_stats->params.rawaf.ramdata[l_idx].h1 * af_split_info.wina_l_ratio +
                        right_stats->params.rawaf.ramdata[r_idx].h1 * af_split_info.wina_r_ratio;
                    statsInt->af_stats_v3x.wnda_fv_h2[dst_idx] =
                        left_stats->params.rawaf.ramdata[l_idx].h2 * af_split_info.wina_l_ratio +
                        right_stats->params.rawaf.ramdata[r_idx].h2 * af_split_info.wina_r_ratio;

                    statsInt->af_stats_v3x.wnda_luma[dst_idx] =
                        left_stats->params.rawae3.data[l_idx].channelg_xy * af_split_info.wina_l_ratio +
                        right_stats->params.rawae3.data[r_idx].channelg_xy * af_split_info.wina_r_ratio;
                    l_lht = ((left_stats->params.rawae3.data[l_idx].channelr_xy & 0x3F) << 10) | left_stats->params.rawae3.data[l_idx].channelb_xy;
                    r_lht = ((right_stats->params.rawae3.data[r_idx].channelr_xy & 0x3F) << 10) | right_stats->params.rawae3.data[r_idx].channelb_xy;
                    statsInt->af_stats_v3x.wina_highlit_cnt[dst_idx] = l_lht + r_lht;
                }
            }
        } else if (af_split_info.wina_side_info == LEFT_MODE) {
            for (i = 0; i < RKAIQ_RAWAF_SUMDATA_NUM; i++) {
                statsInt->af_stats_v3x.wnda_fv_v1[i] = left_stats->params.rawaf.ramdata[i].v1;
                statsInt->af_stats_v3x.wnda_fv_v2[i] = left_stats->params.rawaf.ramdata[i].v2;
                statsInt->af_stats_v3x.wnda_fv_h1[i] = left_stats->params.rawaf.ramdata[i].h1;
                statsInt->af_stats_v3x.wnda_fv_h2[i] = left_stats->params.rawaf.ramdata[i].h2;

                statsInt->af_stats_v3x.wnda_luma[i] = left_stats->params.rawae3.data[i].channelg_xy;
                statsInt->af_stats_v3x.wina_highlit_cnt[i] =
                    ((left_stats->params.rawae3.data[i].channelr_xy & 0x3F) << 10) | left_stats->params.rawae3.data[i].channelb_xy;
            }
        } else {
            for (i = 0; i < RKAIQ_RAWAF_SUMDATA_NUM; i++) {
                statsInt->af_stats_v3x.wnda_fv_v1[i] = right_stats->params.rawaf.ramdata[i].v1;
                statsInt->af_stats_v3x.wnda_fv_v2[i] = right_stats->params.rawaf.ramdata[i].v2;
                statsInt->af_stats_v3x.wnda_fv_h1[i] = right_stats->params.rawaf.ramdata[i].h1;
                statsInt->af_stats_v3x.wnda_fv_h2[i] = right_stats->params.rawaf.ramdata[i].h2;

                statsInt->af_stats_v3x.wnda_luma[i] = right_stats->params.rawae3.data[i].channelg_xy;
                statsInt->af_stats_v3x.wina_highlit_cnt[i] =
                    ((right_stats->params.rawae3.data[i].channelr_xy & 0x3F) << 10) | right_stats->params.rawae3.data[i].channelb_xy;
            }
        }

        LOGD_AF("af_split_info.wina: %d, %d, %d, %d, %f, %f",
                af_split_info.wina_side_info, af_split_info.wina_l_blknum, af_split_info.wina_r_blknum,
                af_split_info.wina_r_skip_blknum, af_split_info.wina_l_ratio, af_split_info.wina_r_ratio);
        LOGD_AF("af_split_info.winb: %d, %f, %f",
                af_split_info.winb_side_info, af_split_info.winb_l_ratio, af_split_info.winb_r_ratio);

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
        }
    }

    return ret;
}

XCamReturn
RkAiqResourceTranslatorV3x::translateMultiAdehazeStats(const SmartPtr<VideoBuffer>& from, SmartPtr<RkAiqAdehazeStatsProxy>& to)
{

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();

    SmartPtr<RkAiqAdehazeStats> statsInt = to->data();

    struct rkisp3x_isp_stat_buffer *left_stats;
    struct rkisp3x_isp_stat_buffer *right_stats;

    left_stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if(left_stats == NULL) {
        LOGE("fail to get left stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOG1_ADEHAZE(
        "%s left adehaze_stats_valid:%d dhaz_adp_air_base:%d dhaz_adp_wt:%d dhaz_adp_gratio:%d "
        "dhaz_adp_tmax:%d dhaz_pic_sumh:%d\n",
        __func__, left_stats->meas_type >> 17 & 1, left_stats->params.dhaz.dhaz_adp_air_base,
        left_stats->params.dhaz.dhaz_adp_wt, left_stats->params.dhaz.dhaz_adp_gratio,
        left_stats->params.dhaz.dhaz_adp_tmax, left_stats->params.dhaz.dhaz_pic_sumh);
    LOG1_ADEHAZE(
        "%s left h_rgb_iir[0~23]: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d %d %d\n",
        __func__, left_stats->params.dhaz.h_rgb_iir[0], left_stats->params.dhaz.h_rgb_iir[1],
        left_stats->params.dhaz.h_rgb_iir[2], left_stats->params.dhaz.h_rgb_iir[3],
        left_stats->params.dhaz.h_rgb_iir[4], left_stats->params.dhaz.h_rgb_iir[5],
        left_stats->params.dhaz.h_rgb_iir[6], left_stats->params.dhaz.h_rgb_iir[7],
        left_stats->params.dhaz.h_rgb_iir[8], left_stats->params.dhaz.h_rgb_iir[9],
        left_stats->params.dhaz.h_rgb_iir[10], left_stats->params.dhaz.h_rgb_iir[11],
        left_stats->params.dhaz.h_rgb_iir[12], left_stats->params.dhaz.h_rgb_iir[13],
        left_stats->params.dhaz.h_rgb_iir[14], left_stats->params.dhaz.h_rgb_iir[15],
        left_stats->params.dhaz.h_rgb_iir[16], left_stats->params.dhaz.h_rgb_iir[17],
        left_stats->params.dhaz.h_rgb_iir[18], left_stats->params.dhaz.h_rgb_iir[19],
        left_stats->params.dhaz.h_rgb_iir[20], left_stats->params.dhaz.h_rgb_iir[21],
        left_stats->params.dhaz.h_rgb_iir[22], left_stats->params.dhaz.h_rgb_iir[23]);

    right_stats = left_stats + 1;
    if(right_stats == NULL) {
        LOGE("fail to get right stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOG1_ADEHAZE(
        "%s right adehaze_stats_valid:%d dhaz_adp_air_base:%d dhaz_adp_wt:%d dhaz_adp_gratio:%d "
        "dhaz_adp_tmax:%d dhaz_pic_sumh:%d\n",
        __func__, right_stats->meas_type >> 17 & 1, right_stats->params.dhaz.dhaz_adp_air_base,
        right_stats->params.dhaz.dhaz_adp_wt, right_stats->params.dhaz.dhaz_adp_gratio,
        right_stats->params.dhaz.dhaz_adp_tmax, right_stats->params.dhaz.dhaz_pic_sumh);
    LOG1_ADEHAZE(
        "%s right h_rgb_iir[0~23]: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d %d %d\n",
        __func__, right_stats->params.dhaz.h_rgb_iir[0], right_stats->params.dhaz.h_rgb_iir[1],
        right_stats->params.dhaz.h_rgb_iir[2], right_stats->params.dhaz.h_rgb_iir[3],
        right_stats->params.dhaz.h_rgb_iir[4], right_stats->params.dhaz.h_rgb_iir[5],
        right_stats->params.dhaz.h_rgb_iir[6], right_stats->params.dhaz.h_rgb_iir[7],
        right_stats->params.dhaz.h_rgb_iir[8], right_stats->params.dhaz.h_rgb_iir[9],
        right_stats->params.dhaz.h_rgb_iir[10], right_stats->params.dhaz.h_rgb_iir[11],
        right_stats->params.dhaz.h_rgb_iir[12], right_stats->params.dhaz.h_rgb_iir[13],
        right_stats->params.dhaz.h_rgb_iir[14], right_stats->params.dhaz.h_rgb_iir[15],
        right_stats->params.dhaz.h_rgb_iir[16], right_stats->params.dhaz.h_rgb_iir[17],
        right_stats->params.dhaz.h_rgb_iir[18], right_stats->params.dhaz.h_rgb_iir[19],
        right_stats->params.dhaz.h_rgb_iir[20], right_stats->params.dhaz.h_rgb_iir[21],
        right_stats->params.dhaz.h_rgb_iir[22], right_stats->params.dhaz.h_rgb_iir[23]);

    if(left_stats->frame_id != right_stats->frame_id || left_stats->meas_type != right_stats->meas_type) {
        LOGE_ANALYZER("status params(frmid or meas_type) of left isp and right isp are different");
        return XCAM_RETURN_ERROR_PARAM;
    } else {
        LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", left_stats->frame_id, left_stats->meas_type);
    }

    //adehaze stats v3.x
    statsInt->adehaze_stats_valid = left_stats->meas_type >> 17 & 1;
    statsInt->frame_id = left_stats->frame_id;

    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_adp_air_base = (left_stats->params.dhaz.dhaz_adp_air_base + right_stats->params.dhaz.dhaz_adp_air_base) / 2;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_adp_wt = (left_stats->params.dhaz.dhaz_adp_wt + right_stats->params.dhaz.dhaz_adp_wt) / 2;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_adp_gratio = (left_stats->params.dhaz.dhaz_adp_gratio + right_stats->params.dhaz.dhaz_adp_gratio) / 2;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_adp_tmax = (left_stats->params.dhaz.dhaz_adp_tmax + right_stats->params.dhaz.dhaz_adp_tmax) / 2;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_pic_sumh_left = left_stats->params.dhaz.dhaz_pic_sumh;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_pic_sumh_right = right_stats->params.dhaz.dhaz_pic_sumh;

    unsigned int ro_pic_sumh_left = left_stats->params.dhaz.dhaz_pic_sumh;
    if (!ro_pic_sumh_left) {
        ro_pic_sumh_left = ISP3X_DHAZ_PIC_SUM_MIN;
        LOGE_ADEHAZE("%s(%d) left ro_pic_sumh is zero, set to %d !!!\n", __func__, __LINE__,
                     ISP3X_DHAZ_PIC_SUM_MIN);
    }
    unsigned int ro_pic_sumh_right = right_stats->params.dhaz.dhaz_pic_sumh;
    if (!ro_pic_sumh_right) {
        ro_pic_sumh_right = ISP3X_DHAZ_PIC_SUM_MIN;
        LOGE_ADEHAZE("%s(%d) right ro_pic_sumh is zero, set to %d !!!\n", __func__, __LINE__,
                     ISP3X_DHAZ_PIC_SUM_MIN);
    }

    unsigned int tmp = 0;
    for (int i = 0; i < ISP3X_DHAZ_HIST_IIR_NUM; i++) {
        tmp = (left_stats->params.dhaz.h_rgb_iir[i] * ro_pic_sumh_left + right_stats->params.dhaz.h_rgb_iir[i] * ro_pic_sumh_right)
              / (ro_pic_sumh_left + ro_pic_sumh_right);
        statsInt->adehaze_stats.dehaze_stats_v11_duo.h_rgb_iir[i] = tmp > ISP3X_DHAZ_HIST_IIR_MAX ? ISP3X_DHAZ_HIST_IIR_MAX : tmp;
    }

    return ret;
}

#endif

void calcAecLiteWinStatsV3X(
    isp2x_rawaelite_stat *      stats_in,
    rawaelite_stat_t*           stats_out,
    uint16_t*                   raw_mean,
    unsigned char*              weight,
    int8_t                      stats_chn_sel,
    int8_t                      y_range_mode,
    struct isp2x_bls_fixed_val  bls1_val,
    float *                     bls_ratio
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
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channely_xy[i] = CLIP(round(rcc * (float)((int)((stats_in->data[i].channelr_xy - bls1_val.r) * bls_ratio[0]) >> 2) +
                                                   gcc * (float)((int)((stats_in->data[i].channelg_xy - bls1_val.gr) * bls_ratio[1]) >> 4) +
                                                   bcc * (float)((int)((stats_in->data[i].channelb_xy - bls1_val.b) * bls_ratio[2]) >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_R_EN:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = CLIP((int)((stats_in->data[i].channelr_xy - bls1_val.r) * bls_ratio[0]), 0, MAX_10BITS);
            sum_xy += ((stats_out->channelr_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_G_EN:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelg_xy[i] = CLIP((int)((stats_in->data[i].channelg_xy - bls1_val.gr) * bls_ratio[1]), 0, MAX_12BITS);
            sum_xy += ((stats_out->channelg_xy[i] >> 4) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_B_EN:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelb_xy[i] = CLIP((int)((stats_in->data[i].channelb_xy - bls1_val.b) * bls_ratio[2]), 0, MAX_10BITS);
            sum_xy += ((stats_out->channelb_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_RGB_EN:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = CLIP((int)((stats_in->data[i].channelr_xy - bls1_val.r) * bls_ratio[0]), 0, MAX_10BITS);
            stats_out->channelg_xy[i] = CLIP((int)((stats_in->data[i].channelg_xy - bls1_val.gr) * bls_ratio[1]), 0, MAX_12BITS);
            stats_out->channelb_xy[i] = CLIP((int)((stats_in->data[i].channelb_xy - bls1_val.b) * bls_ratio[2]), 0, MAX_10BITS);
        }
        break;

    case RAWSTATS_CHN_ALL_EN:
    default:
        for (int i = 0; i < ISP3X_RAWAELITE_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = CLIP((int)((stats_in->data[i].channelr_xy - bls1_val.r) * bls_ratio[0]), 0, MAX_10BITS);
            stats_out->channelg_xy[i] = CLIP((int)((stats_in->data[i].channelg_xy - bls1_val.gr) * bls_ratio[1]), 0, MAX_12BITS);
            stats_out->channelb_xy[i] = CLIP((int)((stats_in->data[i].channelb_xy - bls1_val.b) * bls_ratio[2]), 0, MAX_10BITS);
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

void calcAecBigWinStatsV3X(
    isp2x_rawaebig_stat *       stats_in,
    rawaebig_stat_t*            stats_out,
    uint16_t*                   raw_mean,
    unsigned char*              weight,
    int8_t                      stats_chn_sel,
    int8_t                      y_range_mode,
    struct isp2x_bls_fixed_val  bls1_val,
    float *                     bls_ratio,
    u32 *                       pixel_num
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
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channely_xy[i] = CLIP(round(rcc * (float)((int)((stats_in->data[i].channelr_xy - bls1_val.r) * bls_ratio[0]) >> 2) +
                                                   gcc * (float)((int)((stats_in->data[i].channelg_xy - bls1_val.gr) * bls_ratio[1]) >> 4) +
                                                   bcc * (float)((int)((stats_in->data[i].channelb_xy - bls1_val.b) * bls_ratio[2]) >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_R_EN:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = CLIP((int)((stats_in->data[i].channelr_xy - bls1_val.r) * bls_ratio[0]), 0, MAX_10BITS);
            sum_xy += ((stats_out->channelr_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_G_EN:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelg_xy[i] = CLIP((int)((stats_in->data[i].channelg_xy - bls1_val.gr) * bls_ratio[1]), 0, MAX_12BITS);
            sum_xy += ((stats_out->channelg_xy[i] >> 4) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_B_EN:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelb_xy[i] = CLIP((int)((stats_in->data[i].channelb_xy - bls1_val.b) * bls_ratio[2]), 0, MAX_10BITS);
            sum_xy += ((stats_out->channelb_xy[i] >> 2) * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;

    case RAWSTATS_CHN_RGB_EN:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = CLIP((int)((stats_in->data[i].channelr_xy - bls1_val.r) * bls_ratio[0]), 0, MAX_10BITS);
            stats_out->channelg_xy[i] = CLIP((int)((stats_in->data[i].channelg_xy - bls1_val.gr) * bls_ratio[1]), 0, MAX_12BITS);
            stats_out->channelb_xy[i] = CLIP((int)((stats_in->data[i].channelb_xy - bls1_val.b) * bls_ratio[2]), 0, MAX_10BITS);
        }
        break;

    case RAWSTATS_CHN_ALL_EN:
    default:
        for (int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
            stats_out->channelr_xy[i] = CLIP((int)((stats_in->data[i].channelr_xy - bls1_val.r) * bls_ratio[0]), 0, MAX_10BITS);
            stats_out->channelg_xy[i] = CLIP((int)((stats_in->data[i].channelg_xy - bls1_val.gr) * bls_ratio[1]), 0, MAX_12BITS);
            stats_out->channelb_xy[i] = CLIP((int)((stats_in->data[i].channelb_xy - bls1_val.b) * bls_ratio[2]), 0, MAX_10BITS);
            stats_out->channely_xy[i] = CLIP(round(rcc * (float)(stats_out->channelr_xy[i] >> 2) +
                                                   gcc * (float)(stats_out->channelg_xy[i] >> 4) +
                                                   bcc * (float)(stats_out->channelb_xy[i] >> 2) + off), 0, MAX_8BITS);
            sum_xy += (stats_out->channely_xy[i] * weight[i]);
            sum_weight += weight[i];
        }
        *raw_mean = round(256.0f * (float)sum_xy / (float)sum_weight);
        break;
    }

    for(int i = 0; i < ISP3X_RAWAEBIG_SUBWIN_NUM; i++) {
        stats_out->wndx_sumr[i] = CLIP((s64)((stats_in->sumr[i] - (pixel_num[i] >> 2) * bls1_val.r) * bls_ratio[0]), 0, MAX_29BITS);
        stats_out->wndx_sumg[i] = CLIP((s64)((stats_in->sumg[i] - (pixel_num[i] >> 1) * bls1_val.gr) * bls_ratio[1]), 0, MAX_32BITS);
        stats_out->wndx_sumb[i] = CLIP((s64)((stats_in->sumb[i] - (pixel_num[i] >> 2) * bls1_val.b) * bls_ratio[2]), 0, MAX_29BITS);
    }

}

void calcAecHistBinStatsV3X(
    u32 *                        hist_bin_in,
    u32 *                        hist_bin_out,
    u8                           hist_mode,
    struct isp2x_bls_fixed_val   bls1_val,
    float *                      bls_ratio,
    bool                         bls1_en
) {

    if (bls1_en) {
        memset(hist_bin_out, 0, ISP3X_HIST_BIN_N_MAX * sizeof(u32));

        int tmp;
        s16 hist_bls1;
        float hist_bls_ratio;
        switch(hist_mode) {
        case 2:
            hist_bls1 = bls1_val.r >> 2;
            hist_bls_ratio = bls_ratio[0];
            break;
        case 3:
            hist_bls1 = bls1_val.gr >> 4;
            hist_bls_ratio = bls_ratio[1];
            break;
        case 4:
            hist_bls1 = bls1_val.b >> 2;
            hist_bls_ratio = bls_ratio[2];
            break;
        case 5:
        default:
            hist_bls1 = (s16)((bls1_val.gr >> 4) * 0.587 + (bls1_val.r >> 2) * 0.299 + (bls1_val.b >> 2) * 0.114 + 0.5);
            hist_bls_ratio = (float)((1 << 8) - 1) / ((1 << 8) - 1 - hist_bls1);
            break;
        }

        for(int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
            tmp = (i - hist_bls1 > 0) ? (i - hist_bls1) * hist_bls_ratio + 0.5 : 0;
            tmp = (tmp > ISP3X_HIST_BIN_N_MAX - 1) ? (ISP3X_HIST_BIN_N_MAX - 1) : tmp;
            hist_bin_out[tmp] += hist_bin_in[i];
        }

    } else {

        memcpy(hist_bin_out, hist_bin_in, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
    }

}

void rotationAecLiteBlkStats(
    rawaelite_stat_t*       rawae_stat,
    int                     degree,
    int8_t                  stats_chn_sel
) {
    uint8_t colnum = sqrt(ISP3X_RAWAELITE_MEAN_NUM);

    rawaelite_stat_t* temp = (rawaelite_stat_t*)calloc(1, sizeof(rawaelite_stat_t));
    if(degree == 1/*RK_PS_SrcOverlapPosition_90*/) {
        // clockwise 90
        switch (stats_chn_sel) {
        case RAWSTATS_CHN_Y_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channely_xy[row * colnum + col] = rawae_stat->channely_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat->channely_xy, temp->channely_xy, sizeof(RAWAELITE_WIN_NUM));
            break;

        case RAWSTATS_CHN_R_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat->channelr_xy, temp->channelr_xy, sizeof(RAWAELITE_WIN_NUM));
            break;

        case RAWSTATS_CHN_G_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat->channelg_xy, temp->channelg_xy, sizeof(RAWAELITE_WIN_NUM));
            break;

        case RAWSTATS_CHN_B_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat->channelb_xy, temp->channelb_xy, sizeof(RAWAELITE_WIN_NUM));
            break;

        case RAWSTATS_CHN_RGB_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[(colnum - 1 - col) * colnum + row];
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[(colnum - 1 - col) * colnum + row];
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat, temp, sizeof(rawaelite_stat_t));
            break;

        case RAWSTATS_CHN_ALL_EN:
        default:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[(colnum - 1 - col) * colnum + row];
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[(colnum - 1 - col) * colnum + row];
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[(colnum - 1 - col) * colnum + row];
                    temp->channely_xy[row * colnum + col] = rawae_stat->channely_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat, temp, sizeof(rawaelite_stat_t));
            break;
        }

    } else if(degree == 3/*RK_PS_SrcOverlapPosition_270*/) {
        // counter clockwise 90
        switch (stats_chn_sel) {
        case RAWSTATS_CHN_Y_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channely_xy[row * colnum + col] = rawae_stat->channely_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat->channely_xy, temp->channely_xy, sizeof(RAWAELITE_WIN_NUM));
            break;

        case RAWSTATS_CHN_R_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat->channelr_xy, temp->channelr_xy, sizeof(RAWAELITE_WIN_NUM));
            break;

        case RAWSTATS_CHN_G_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat->channelg_xy, temp->channelg_xy, sizeof(RAWAELITE_WIN_NUM));
            break;

        case RAWSTATS_CHN_B_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat->channelb_xy, temp->channelb_xy, sizeof(RAWAELITE_WIN_NUM));
            break;

        case RAWSTATS_CHN_RGB_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[col * colnum + (colnum - 1 - row)];
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[col * colnum + (colnum - 1 - row)];
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat, temp, sizeof(rawaelite_stat_t));
            break;

        case RAWSTATS_CHN_ALL_EN:
        default:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[col * colnum + (colnum - 1 - row)];
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[col * colnum + (colnum - 1 - row)];
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[col * colnum + (colnum - 1 - row)];
                    temp->channely_xy[row * colnum + col] = rawae_stat->channely_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat, temp, sizeof(rawaelite_stat_t));
            break;
        }

    } else {
        LOGW("not support mModuleRotation %d", degree);
    }

    if(temp) free(temp);

}

void rotationAecBigBlkStats(
    rawaebig_stat_t*        rawae_stat,
    int                     degree,
    int8_t                  stats_chn_sel
) {
    uint8_t colnum = sqrt(ISP3X_RAWAEBIG_MEAN_NUM);

    rawaebig_stat_t* temp = (rawaebig_stat_t*)calloc(1, sizeof(rawaebig_stat_t));
    if(degree == 1/*RK_PS_SrcOverlapPosition_90*/) {
        // clockwise 90
        switch (stats_chn_sel) {
        case RAWSTATS_CHN_Y_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channely_xy[row * colnum + col] = rawae_stat->channely_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat->channely_xy, temp->channely_xy, sizeof(RAWAEBIG_WIN_NUM));
            break;

        case RAWSTATS_CHN_R_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat->channelr_xy, temp->channelr_xy, sizeof(RAWAEBIG_WIN_NUM));
            break;

        case RAWSTATS_CHN_G_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat->channelg_xy, temp->channelg_xy, sizeof(RAWAEBIG_WIN_NUM));
            break;

        case RAWSTATS_CHN_B_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat->channelb_xy, temp->channelb_xy, sizeof(RAWAEBIG_WIN_NUM));
            break;

        case RAWSTATS_CHN_RGB_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[(colnum - 1 - col) * colnum + row];
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[(colnum - 1 - col) * colnum + row];
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            memcpy(rawae_stat, temp, sizeof(rawaebig_stat_t));
            break;

        case RAWSTATS_CHN_ALL_EN:
        default:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[(colnum - 1 - col) * colnum + row];
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[(colnum - 1 - col) * colnum + row];
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[(colnum - 1 - col) * colnum + row];
                    temp->channely_xy[row * colnum + col] = rawae_stat->channely_xy[(colnum - 1 - col) * colnum + row];
                }
            }
            // TODO: wndx_channel & wndx_sum
            memcpy(rawae_stat, temp, sizeof(rawaebig_stat_t));
            break;
        }

    } else if(degree == 3/*RK_PS_SrcOverlapPosition_270*/) {
        // counter clockwise 90
        switch (stats_chn_sel) {
        case RAWSTATS_CHN_Y_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channely_xy[row * colnum + col] = rawae_stat->channely_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat->channely_xy, temp->channely_xy, sizeof(RAWAEBIG_WIN_NUM));
            break;

        case RAWSTATS_CHN_R_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat->channelr_xy, temp->channelr_xy, sizeof(RAWAEBIG_WIN_NUM));
            break;

        case RAWSTATS_CHN_G_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat->channelg_xy, temp->channelg_xy, sizeof(RAWAEBIG_WIN_NUM));
            break;

        case RAWSTATS_CHN_B_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat->channelb_xy, temp->channelb_xy, sizeof(RAWAEBIG_WIN_NUM));
            break;

        case RAWSTATS_CHN_RGB_EN:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[col * colnum + (colnum - 1 - row)];
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[col * colnum + (colnum - 1 - row)];
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            memcpy(rawae_stat, temp, sizeof(rawaebig_stat_t));
            break;

        case RAWSTATS_CHN_ALL_EN:
        default:
            for(int row = 0; row < colnum; row++) {
                for(int col = 0; col < colnum; col++) {
                    temp->channelr_xy[row * colnum + col] = rawae_stat->channelr_xy[col * colnum + (colnum - 1 - row)];
                    temp->channelg_xy[row * colnum + col] = rawae_stat->channelg_xy[col * colnum + (colnum - 1 - row)];
                    temp->channelb_xy[row * colnum + col] = rawae_stat->channelb_xy[col * colnum + (colnum - 1 - row)];
                    temp->channely_xy[row * colnum + col] = rawae_stat->channely_xy[col * colnum + (colnum - 1 - row)];
                }
            }
            // TODO: wndx_channel & wndx_sum
            memcpy(rawae_stat, temp, sizeof(rawaebig_stat_t));
            break;
        }

    } else {
        LOGW("not support mModuleRotation %d", degree);
    }

    if(temp) free(temp);

}

void RotationDegAwbBlkStas(rk_aiq_awb_stat_blk_res_v201_t *blockResult, int degree)
{

    rk_aiq_awb_stat_blk_res_v201_t blockResult_old[RK_AIQ_AWB_GRID_NUM_TOTAL];
    if(degree == 3 /*RK_PS_SrcOverlapPosition_270*/) {
        memcpy(blockResult_old, blockResult, sizeof(rk_aiq_awb_stat_blk_res_v201_t)*RK_AIQ_AWB_GRID_NUM_TOTAL);
        for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_VERHOR; i++) {
            for(int j = 0; j < RK_AIQ_AWB_GRID_NUM_VERHOR; j++) {
                memcpy(&blockResult[(RK_AIQ_AWB_GRID_NUM_VERHOR - j - 1)*RK_AIQ_AWB_GRID_NUM_VERHOR + i], &blockResult_old[i * RK_AIQ_AWB_GRID_NUM_VERHOR + j], sizeof(rk_aiq_awb_stat_blk_res_v201_t));
            }
        }
    } else if(degree == 1 /*RK_PS_SrcOverlapPosition_90*/) {
        memcpy(blockResult_old, blockResult, sizeof(rk_aiq_awb_stat_blk_res_v201_t)*RK_AIQ_AWB_GRID_NUM_TOTAL);
        for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_VERHOR; i++) {
            for(int j = 0; j < RK_AIQ_AWB_GRID_NUM_VERHOR; j++) {
                memcpy(&blockResult[j * RK_AIQ_AWB_GRID_NUM_VERHOR + (RK_AIQ_AWB_GRID_NUM_VERHOR - i - 1)], &blockResult_old[i * RK_AIQ_AWB_GRID_NUM_VERHOR + j], sizeof(rk_aiq_awb_stat_blk_res_v201_t));
            }
        }
    } else {
        LOGW_AWBGROUP("not support mModuleRotation %d, abandon to rotate awb blk stas !!!!", degree);
    }
}


XCamReturn
RkAiqResourceTranslatorV3x::translateAecStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAecStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V30)

    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
    struct rkisp3x_isp_stat_buffer *stats;
    SmartPtr<RkAiqAecStats> statsInt = to->data();

#if defined(RKAIQ_HAVE_MULTIISP)
    if (mIsMultiIsp) {
        return translateMultiAecStats(from, to);
    }
#endif

    stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("camId: %d, aec stats: frame_id: %d,  meas_type; 0x%x",
                  mCamPhyId, stats->frame_id, stats->meas_type);

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

    /*rawae stats*/
    struct isp3x_isp_meas_cfg* isp_params = &ispParams.isp_params_v3x[0].meas;
    uint8_t AeSwapMode, AeSelMode, AfUseAeBig;
    AeSwapMode = isp_params->rawae0.rawae_sel;
    AeSelMode = isp_params->rawae3.rawae_sel;
    AfUseAeBig = isp_params->rawaf.ae_mode;

    struct isp21_bls_cfg *bls_cfg = &ispParams.isp_params_v3x[0].others.bls_cfg;
    struct isp2x_bls_fixed_val bls1_val; //bls1_val = blc1_ori_val * awb * range_ratio
    float bls_ratio[3] = {1, 1, 1};
    u32 pixel_num[ISP3X_RAWAEBIG_SUBWIN_NUM] = { 0 };
    bool is_bls1_en = bls_cfg->bls1_en;

    if(is_bls1_en) {
        bls1_val.r = bls_cfg->bls1_val.r >> 2;
        bls1_val.gr = bls_cfg->bls1_val.gr;
        bls1_val.gb = bls_cfg->bls1_val.gb;
        bls1_val.b = bls_cfg->bls1_val.b >> 2;

        bls_ratio[0] = (float)((1 << 10) - 1) / ((1 << 10) - 1 - bls_cfg->bls1_val.r);
        bls_ratio[1] = (float)((1 << 12) - 1) / ((1 << 12) - 1 - bls_cfg->bls1_val.gr);
        bls_ratio[2] = (float)((1 << 10) - 1) / ((1 << 10) - 1 - bls_cfg->bls1_val.b);

    } else {
        bls1_val.r = 0;
        bls1_val.gr = 0;
        bls1_val.gb = 0;
        bls1_val.b = 0;
    }

#ifdef AE_STATS_DEBUG
    LOGE("bls1[%d-%d-%d-%d]", bls1_val.r, bls1_val.gr, bls1_val.gb, bls1_val.b);
    LOGE("bls_ratio[%f-%f-%f]", bls_ratio[0], bls_ratio[1], bls_ratio[2]);

    _aeAlgoStatsCfg.UpdateStats = true;
    _aeAlgoStatsCfg.RawStatsChnSel = RAWSTATS_CHN_ALL_EN;
#endif

    unsigned int meas_type = 0;
    u8 index0, index1, index2;
    u8 rawhist_mode = 0, rawhist3_mode = 0;
    s16   hist_bls1;
    float hist_bls_ratio;

    switch (AeSwapMode) {
    case AEC_RAWSWAP_MODE_S_LITE:
        meas_type = ((stats->meas_type >> 7) & (0x01)) & ((stats->meas_type >> 11) & (0x01));
        index0 = 0;
        index1 = 1;
        index2 = 2;
        rawhist_mode = isp_params->rawhist0.mode;
        break;
    case AEC_RAWSWAP_MODE_M_LITE:
        meas_type = ((stats->meas_type >> 8) & (0x01)) & ((stats->meas_type >> 12) & (0x01));
        index0 = 1;
        index1 = 0;
        index2 = 2;
        rawhist_mode = isp_params->rawhist1.mode;
        break;
    case AEC_RAWSWAP_MODE_L_LITE:
        meas_type = ((stats->meas_type >> 9) & (0x01)) & ((stats->meas_type >> 13) & (0x01));
        index0 = 2;
        index1 = 1;
        index2 = 0;
        rawhist_mode = isp_params->rawhist2.mode;
        break;
    default:
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
        break;
    }

    //ae stats v3.x
    statsInt->frame_id = stats->frame_id;
    statsInt->af_prior = (AfUseAeBig == 0) ? false : true;
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
    if (!AfUseAeBig && (AeSelMode <= AEC_RAWSEL_MODE_CHN_2)) {
        hist_bin[AeSelMode] = stats->params.rawhist3.hist_bin;
    }

    for (int i = 0; i < ISP3X_HIST_BIN_N_MAX; i++) {
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
        calcAecLiteWinStatsV3X(&stats->params.rawae0,
                               &statsInt->aec_stats.ae_data.chn[index0].rawae_lite,
                               &statsInt->aec_stats.ae_data.raw_mean[index0],
                               _aeAlgoStatsCfg.LiteWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                               bls1_val, bls_ratio);

        pixel_num[0] = isp_params->rawae1.subwin[0].h_size * isp_params->rawae1.subwin[0].v_size;
        pixel_num[1] = isp_params->rawae1.subwin[1].h_size * isp_params->rawae1.subwin[1].v_size;
        pixel_num[2] = isp_params->rawae1.subwin[2].h_size * isp_params->rawae1.subwin[2].v_size;
        pixel_num[3] = isp_params->rawae1.subwin[3].h_size * isp_params->rawae1.subwin[3].v_size;
        calcAecBigWinStatsV3X(&stats->params.rawae1,
                              &statsInt->aec_stats.ae_data.chn[index1].rawae_big,
                              &statsInt->aec_stats.ae_data.raw_mean[index1],
                              _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                              bls1_val, bls_ratio, pixel_num);

        pixel_num[0] = isp_params->rawae2.subwin[0].h_size * isp_params->rawae2.subwin[0].v_size;
        pixel_num[1] = isp_params->rawae2.subwin[1].h_size * isp_params->rawae2.subwin[1].v_size;
        pixel_num[2] = isp_params->rawae2.subwin[2].h_size * isp_params->rawae2.subwin[2].v_size;
        pixel_num[3] = isp_params->rawae2.subwin[3].h_size * isp_params->rawae2.subwin[3].v_size;
        calcAecBigWinStatsV3X(&stats->params.rawae2,
                              &statsInt->aec_stats.ae_data.chn[index2].rawae_big,
                              &statsInt->aec_stats.ae_data.raw_mean[index2],
                              _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                              bls1_val, bls_ratio, pixel_num);

        calcAecHistBinStatsV3X(stats->params.rawhist0.hist_bin,
                               statsInt->aec_stats.ae_data.chn[index0].rawhist_lite.bins,
                               rawhist_mode, bls1_val, bls_ratio, is_bls1_en);

        calcAecHistBinStatsV3X(stats->params.rawhist1.hist_bin,
                               statsInt->aec_stats.ae_data.chn[index1].rawhist_big.bins,
                               rawhist_mode, bls1_val, bls_ratio, is_bls1_en);

        calcAecHistBinStatsV3X(stats->params.rawhist2.hist_bin,
                               statsInt->aec_stats.ae_data.chn[index2].rawhist_big.bins,
                               rawhist_mode, bls1_val, bls_ratio, is_bls1_en);

        if(!AfUseAeBig) {
            switch(AeSelMode) {
            case AEC_RAWSEL_MODE_CHN_0:
            case AEC_RAWSEL_MODE_CHN_1:
            case AEC_RAWSEL_MODE_CHN_2:

                pixel_num[0] = isp_params->rawae3.subwin[0].h_size * isp_params->rawae3.subwin[0].v_size;
                pixel_num[1] = isp_params->rawae3.subwin[1].h_size * isp_params->rawae3.subwin[1].v_size;
                pixel_num[2] = isp_params->rawae3.subwin[2].h_size * isp_params->rawae3.subwin[2].v_size;
                pixel_num[3] = isp_params->rawae3.subwin[3].h_size * isp_params->rawae3.subwin[3].v_size;
                calcAecBigWinStatsV3X(&stats->params.rawae3,
                                      &statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big,
                                      &statsInt->aec_stats.ae_data.raw_mean[AeSelMode],
                                      _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                                      bls1_val, bls_ratio, pixel_num);

                rawhist3_mode = isp_params->rawhist3.mode;
                calcAecHistBinStatsV3X(stats->params.rawhist3.hist_bin,
                                       statsInt->aec_stats.ae_data.chn[AeSelMode].rawhist_big.bins,
                                       rawhist3_mode, bls1_val, bls_ratio, is_bls1_en);
                break;

            case AEC_RAWSEL_MODE_TMO:
                bls1_val.r = 0;
                bls1_val.gr = 0;
                bls1_val.gb = 0;
                bls1_val.b = 0;

                bls_ratio[0] = 1;
                bls_ratio[1] = 1;
                bls_ratio[2] = 1;

                calcAecBigWinStatsV3X(&stats->params.rawae3,
                                      &statsInt->aec_stats.ae_data.extra.rawae_big,
                                      &statsInt->aec_stats.ae_data.raw_mean[AeSelMode],
                                      _aeAlgoStatsCfg.BigWeight, _aeAlgoStatsCfg.RawStatsChnSel, _aeAlgoStatsCfg.YRangeMode,
                                      bls1_val, bls_ratio, pixel_num);

                memcpy(statsInt->aec_stats.ae_data.extra.rawhist_big.bins, stats->params.rawhist3.hist_bin, ISP3X_HIST_BIN_N_MAX * sizeof(u32));
                break;

            default:
                LOGE("wrong AeSelMode=%d\n", AeSelMode);
                return XCAM_RETURN_ERROR_PARAM;
            }
        }

        //rotate stats for group ae
        if(mIsGroupMode) {
            rotationAecLiteBlkStats(&statsInt->aec_stats.ae_data.chn[index0].rawae_lite, mModuleRotation, _aeAlgoStatsCfg.RawStatsChnSel);
            rotationAecBigBlkStats(&statsInt->aec_stats.ae_data.chn[index1].rawae_big, mModuleRotation, _aeAlgoStatsCfg.RawStatsChnSel);
            rotationAecBigBlkStats(&statsInt->aec_stats.ae_data.chn[index2].rawae_big, mModuleRotation, _aeAlgoStatsCfg.RawStatsChnSel);
            if(!AfUseAeBig) {
                if (AeSelMode <= AEC_RAWSEL_MODE_CHN_2) {
                    rotationAecBigBlkStats(&statsInt->aec_stats.ae_data.chn[AeSelMode].rawae_big, mModuleRotation, _aeAlgoStatsCfg.RawStatsChnSel);
                } else {
                    rotationAecBigBlkStats(&statsInt->aec_stats.ae_data.extra.rawae_big, mModuleRotation, _aeAlgoStatsCfg.RawStatsChnSel);
                }
            }
        }
        _lastAeStats = statsInt->aec_stats.ae_data;
    } else {
        statsInt->aec_stats.ae_data = _lastAeStats;
    }

#ifdef AE_STATS_DEBUG
    if(AeSwapMode != 0) {
        for(int i = 0; i < 15; i++) {
            for(int j = 0; j < 15; j++) {
                printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelr_xy[i * 15 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelg_xy[i * 15 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_big.channelb_xy[i * 15 + j]);
            }
        }
        printf("====================sub-win-result======================\n");

        for(int i = 0; i < 4; i++)
            printf("chn0_subwin[%d]:sumr 0x%08lx, sumg 0x%08lx, sumb 0x%08lx\n", i,
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumr[i],
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumg[i],
                   statsInt->aec_stats.ae_data.chn[0].rawae_big.wndx_sumb[i]);

        printf("====================hist_result========================\n");

        for(int i = 0; i < 256; i++)
            printf("bin[%d]= 0x%08x\n", i, statsInt->aec_stats.ae_data.chn[0].rawhist_big.bins[i]);

    } else {
        for(int i = 0; i < 5; i++) {
            for(int j = 0; j < 5; j++) {
                printf("chn0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelr_xy[i * 5 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelg_xy[i * 5 + j],
                       statsInt->aec_stats.ae_data.chn[0].rawae_lite.channelb_xy[i * 5 + j]);
            }
        }
        printf("====================hist_result========================\n");
        for(int i = 0; i < 256; i++)
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

    //expsoure params
    if (expParams.ptr()) {

        statsInt->aec_stats.ae_exp = expParams->data()->aecExpInfo;
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

    //iris params
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
RkAiqResourceTranslatorV3x::translateAwbStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAwbStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V30)
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
    struct rkisp3x_isp_stat_buffer *stats;
    SmartPtr<RkAiqAwbStats> statsInt = to->data();

#if defined(RKAIQ_HAVE_MULTIISP)
    if (mIsMultiIsp) {
        return translateMultiAwbStats(from, to);
    }
#endif

    stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("camId: %d, awb stats: frame_id: %d,  meas_type; 0x%x",
                  mCamPhyId, stats->frame_id, stats->meas_type);

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
    //awb30
    statsInt->awb_stats_v3x.awb_cfg_effect_v201 = ispParams.awb_cfg_v3x;
    rk_aiq_isp_blc_t *bls_cfg = &ispParams.blc_cfg.v0;
    statsInt->blc_cfg_effect = ispParams.blc_cfg.v0;
    statsInt->awb_cfg_effect_valid = true;
    statsInt->frame_id = stats->frame_id;
    for(int i = 0; i < statsInt->awb_stats_v3x.awb_cfg_effect_v201.lightNum; i++) {
        statsInt->awb_stats_v3x.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].RgainValue =
            stats->params.rawawb.ro_rawawb_sum_rgain_nor[i];
        statsInt->awb_stats_v3x.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].BgainValue =
            stats->params.rawawb.ro_rawawb_sum_bgain_nor[i];
        statsInt->awb_stats_v3x.light[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_nor[i];
        statsInt->awb_stats_v3x.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].RgainValue =
            stats->params.rawawb.ro_rawawb_sum_rgain_big[i];
        statsInt->awb_stats_v3x.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].BgainValue =
            stats->params.rawawb.ro_rawawb_sum_bgain_big[i];
        statsInt->awb_stats_v3x.light[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].WpNo =
            stats->params.rawawb.ro_rawawb_wp_num_big[i];

    }

    for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
        statsInt->awb_stats_v3x.blockResult[i].Rvalue = stats->params.rawawb.ramdata[i].r;
        statsInt->awb_stats_v3x.blockResult[i].Gvalue = stats->params.rawawb.ramdata[i].g;
        statsInt->awb_stats_v3x.blockResult[i].Bvalue = stats->params.rawawb.ramdata[i].b;
        statsInt->awb_stats_v3x.blockResult[i].WpNo = stats->params.rawawb.ramdata[i].wp;
    }

    for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
        statsInt->awb_stats_v3x.WpNoHist[i] = stats->params.rawawb.ro_yhist_bin[i];
        // move the shift code here to make WpNoHist merged by several cameras easily
        if( stats->params.rawawb.ro_yhist_bin[i]  & 0x8000 ) {
            statsInt->awb_stats_v3x.WpNoHist[i] = stats->params.rawawb.ro_yhist_bin[i] & 0x7FFF;
            statsInt->awb_stats_v3x.WpNoHist[i] *=    (1 << 3);
        }
    }

#if defined(ISP_HW_V30)
    for(int i = 0; i < statsInt->awb_stats_v3x.awb_cfg_effect_v201.lightNum; i++) {
        statsInt->awb_stats_v3x.WpNo2[i] =  stats->params.rawawb.ro_wp_num2[i];
    }
    for(int i = 0; i < RK_AIQ_AWB_MULTIWINDOW_NUM_V201; i++) {
        statsInt->awb_stats_v3x.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].RgainValue =
            stats->params.rawawb.ro_sum_r_nor_multiwindow[i];

        statsInt->awb_stats_v3x.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].BgainValue =
            stats->params.rawawb.ro_sum_b_nor_multiwindow[i];
        statsInt->awb_stats_v3x.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_NORMAL_V201].WpNo =
            stats->params.rawawb.ro_wp_nm_nor_multiwindow[i];
        statsInt->awb_stats_v3x.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].RgainValue =
            stats->params.rawawb.ro_sum_r_big_multiwindow[i];
        statsInt->awb_stats_v3x.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].BgainValue =
            stats->params.rawawb.ro_sum_b_big_multiwindow[i];
        statsInt->awb_stats_v3x.multiwindowLightResult[i].xYType[RK_AIQ_AWB_XY_TYPE_BIG_V201].WpNo =
            stats->params.rawawb.ro_wp_nm_big_multiwindow[i];
    }

    for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
        statsInt->awb_stats_v3x.excWpRangeResult[i].RgainValue = stats->params.rawawb.ro_sum_r_exc[i];
        statsInt->awb_stats_v3x.excWpRangeResult[i].BgainValue = stats->params.rawawb.ro_sum_b_exc[i];
        statsInt->awb_stats_v3x.excWpRangeResult[i].WpNo =    stats->params.rawawb.ro_wp_nm_exc[i];

    }

    // to fixed the bug in ic design that some egisters maybe overflow
    if(!mIsMultiIsp) {
        int w, h;
        w = statsInt->awb_stats_v3x.awb_cfg_effect_v201.windowSet[2];
        h = statsInt->awb_stats_v3x.awb_cfg_effect_v201.windowSet[3];
        float factor1 = (float)((1 << (RK_AIQ_AWB_WP_WEIGHT_BIS_V201 + 1)) - 1) / ((1 << RK_AIQ_AWB_WP_WEIGHT_BIS_V201) - 1);
        //float factor1 = (float)(1<<(RK_AIQ_AWB_WP_WEIGHT_BIS_V201+1))/((1<<RK_AIQ_AWB_WP_WEIGHT_BIS_V201)-1);
        if(w * h > RK_AIQ_AWB_STAT_MAX_AREA) {
            LOGD_AWB("%s ramdata and ro_wp_num2 is fixed", __FUNCTION__);
            for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
                statsInt->awb_stats_v3x.blockResult[i].WpNo = (float)statsInt->awb_stats_v3x.blockResult[i].WpNo * factor1 + 0.5 ;
                statsInt->awb_stats_v3x.blockResult[i].Rvalue = (float)statsInt->awb_stats_v3x.blockResult[i].Rvalue * factor1 + 0.5 ;
                statsInt->awb_stats_v3x.blockResult[i].Gvalue = (float)statsInt->awb_stats_v3x.blockResult[i].Gvalue * factor1 + 0.5 ;
                statsInt->awb_stats_v3x.blockResult[i].Bvalue = (float)statsInt->awb_stats_v3x.blockResult[i].Bvalue * factor1 + 0.5 ;
            }
            rk_aiq_awb_xy_type_v201_t typ = statsInt->awb_stats_v3x.awb_cfg_effect_v201.xyRangeTypeForWpHist;
            for(int i = 0; i < statsInt->awb_stats_v3x.awb_cfg_effect_v201.lightNum; i++) {
                statsInt->awb_stats_v3x.WpNo2[i] = statsInt->awb_stats_v3x.light[i].xYType[typ].WpNo >> RK_AIQ_WP_GAIN_FRAC_BIS;
            }
        } else {
            if(statsInt->awb_stats_v3x.awb_cfg_effect_v201.blkMeasureMode == RK_AIQ_AWB_BLK_STAT_MODE_REALWP_V201
                    && statsInt->awb_stats_v3x.awb_cfg_effect_v201.blkStatisticsWithLumaWeightEn) {
                for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
                    statsInt->awb_stats_v3x.blockResult[i].WpNo = (float)statsInt->awb_stats_v3x.blockResult[i].WpNo * factor1 + 0.5 ;
                    statsInt->awb_stats_v3x.blockResult[i].Rvalue = (float)statsInt->awb_stats_v3x.blockResult[i].Rvalue * factor1 + 0.5 ;
                    statsInt->awb_stats_v3x.blockResult[i].Gvalue = (float)statsInt->awb_stats_v3x.blockResult[i].Gvalue * factor1 + 0.5 ;
                    statsInt->awb_stats_v3x.blockResult[i].Bvalue = (float)statsInt->awb_stats_v3x.blockResult[i].Bvalue * factor1 + 0.5 ;
                }
            }
        }
    }

#endif
    LOG1_AWB("bls_cfg %p", bls_cfg);
    if(bls_cfg) {
        LOG1_AWB("bls1_enalbe: %d, b r gb gr:[ %d %d %d %d]", bls_cfg->blc1_enable, bls_cfg->blc1_b,
                 bls_cfg->blc1_r, bls_cfg->blc1_gb, bls_cfg->blc1_gr);
    }
    if(bls_cfg && bls_cfg->blc1_enable && (bls_cfg->blc1_b > 0 || bls_cfg->blc1_r > 0 || bls_cfg->blc1_gb > 0 || bls_cfg->blc1_gr > 0)) {

        for(int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
            statsInt->awb_stats_v3x.blockResult[i].Rvalue -=
                (long long)(statsInt->awb_stats_v3x.blockResult[i].WpNo * bls_cfg->blc1_r *
                            statsInt->awb_stats_v3x.awb_cfg_effect_v201.pre_wbgain_inv_r + 2048) >> 12 ;
            statsInt->awb_stats_v3x.blockResult[i].Gvalue -=
                (long long)(statsInt->awb_stats_v3x.blockResult[i].WpNo * (bls_cfg->blc1_gr + bls_cfg->blc1_gb) *
                            statsInt->awb_stats_v3x.awb_cfg_effect_v201.pre_wbgain_inv_g + 4096) >> 13 ;
            statsInt->awb_stats_v3x.blockResult[i].Bvalue -=
                (long long)(statsInt->awb_stats_v3x.blockResult[i].WpNo * bls_cfg->blc1_b  *
                            statsInt->awb_stats_v3x.awb_cfg_effect_v201.pre_wbgain_inv_b + 2048) >> 12 ;
        }
    }
    LOGV_AWBGROUP("mIsGroupMode %d, mCamPhyId %d,mModuleRotation %d", mIsGroupMode, mCamPhyId, mModuleRotation);
    if(mIsGroupMode) {
        RotationDegAwbBlkStas(statsInt->awb_stats_v3x.blockResult, mModuleRotation);
    }
    //statsInt->awb_stats_valid = ISP2X_STAT_RAWAWB(stats->meas_type)? true:false;
    statsInt->awb_stats_valid = stats->meas_type >> 5 & 1;
    to->set_sequence(stats->frame_id);
#endif
    return ret;
}

XCamReturn
RkAiqResourceTranslatorV3x::translateAfStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAfStatsProxy> &to)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V30)
    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
    struct rkisp3x_isp_stat_buffer *stats;
    SmartPtr<RkAiqAfStats> statsInt = to->data();

#if defined(RKAIQ_HAVE_MULTIISP)
    if (mIsMultiIsp) {
        return translateMultiAfStats(from, to);
    }
#endif

    stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("camId: %d, af stats: frame_id: %d,  meas_type; 0x%x",
                  mCamPhyId, stats->frame_id, stats->meas_type);

    SmartPtr<RkAiqAfInfoProxy> afParams = buf->get_af_params();

    memset(&statsInt->af_stats_v3x, 0, sizeof(rk_aiq_isp_af_stats_v3x_t));
    memset(&statsInt->stat_motor, 0, sizeof(rk_aiq_af_algo_motor_stat_t));
    statsInt->frame_id = stats->frame_id;

    SmartPtr<RkAiqSensorExpParamsProxy> expParams = nullptr;
    if (buf->getEffectiveExpParams(stats->frame_id, expParams) < 0)
        LOGE("fail to get expParams");
    if (expParams.ptr())
        statsInt->aecExpInfo = expParams->data()->aecExpInfo;

    //af
    {
        statsInt->af_stats_valid =
            (stats->meas_type >> 6) & (0x01) ? true : false;

        statsInt->af_stats_v3x.wndb_luma = stats->params.rawaf.afm_lum_b;
        statsInt->af_stats_v3x.wndb_sharpness = stats->params.rawaf.afm_sum_b;
        statsInt->af_stats_v3x.winb_highlit_cnt = stats->params.rawaf.highlit_cnt_winb;
        for (int i = 0; i < RKAIQ_RAWAF_SUMDATA_NUM; i++) {
            statsInt->af_stats_v3x.wnda_fv_v1[i] = stats->params.rawaf.ramdata[i].v1;
            statsInt->af_stats_v3x.wnda_fv_v2[i] = stats->params.rawaf.ramdata[i].v2;
            statsInt->af_stats_v3x.wnda_fv_h1[i] = stats->params.rawaf.ramdata[i].h1;
            statsInt->af_stats_v3x.wnda_fv_h2[i] = stats->params.rawaf.ramdata[i].h2;

            statsInt->af_stats_v3x.wnda_luma[i] = stats->params.rawae3.data[i].channelg_xy;
            statsInt->af_stats_v3x.wina_highlit_cnt[i] =
                ((stats->params.rawae3.data[i].channelr_xy & 0x3F) << 10) | stats->params.rawae3.data[i].channelb_xy;
        }

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
        }
    }
#endif

    return ret;
}

#if RKAIQ_HAVE_DEHAZE_V11_DUO
XCamReturn
RkAiqResourceTranslatorV3x::translateAdehazeStats (const SmartPtr<VideoBuffer> &from, SmartPtr<RkAiqAdehazeStatsProxy> &to)
{

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    Isp20StatsBuffer* buf =
        from.get_cast_ptr<Isp20StatsBuffer>();
    struct rkisp3x_isp_stat_buffer *stats;
    SmartPtr<RkAiqAdehazeStats> statsInt = to->data();

#if defined(RKAIQ_HAVE_MULTIISP)
    if (mIsMultiIsp) {
        return translateMultiAdehazeStats(from, to);
    }
#endif

    stats = (struct rkisp3x_isp_stat_buffer*)(buf->get_v4l2_userptr());
    if(stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("camId: %d, dehaze stats: frame_id: %d,  meas_type; 0x%x",
                  mCamPhyId, stats->frame_id, stats->meas_type);
    //dehaze
    statsInt->adehaze_stats_valid = stats->meas_type >> 17 & 1;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_adp_air_base =
        stats->params.dhaz.dhaz_adp_air_base;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_adp_wt = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_adp_gratio =
        stats->params.dhaz.dhaz_adp_gratio;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_adp_wt = stats->params.dhaz.dhaz_adp_wt;
    statsInt->adehaze_stats.dehaze_stats_v11_duo.dhaz_pic_sumh_left =
        stats->params.dhaz.dhaz_pic_sumh;
    for(int i = 0; i < ISP3X_DHAZ_HIST_IIR_NUM; i++)
        statsInt->adehaze_stats.dehaze_stats_v11_duo.h_rgb_iir[i] = stats->params.dhaz.h_rgb_iir[i];

    to->set_sequence(stats->frame_id);

    return ret;
}
#endif

} //namespace RkCam
