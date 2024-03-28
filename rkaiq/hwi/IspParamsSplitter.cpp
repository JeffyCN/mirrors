/*
 * IspParamsSplitter.h - Split ISP params to Left/Right ISP params
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */
#include "IspParamsSplitter.h"

#include <utility>

#include "common/rk_isp20_hw.h"
#include "common/rkisp2-config.h"
#include "common/rkisp3-config.h"
#include "common/rkisp32-config.h"

#include "algos/ae/rk_aiq_types_ae_hw.h"

using namespace RkCam;

//#define DEBUG

#define MAX(a,b) (((a)>(b))?(a):(b))

namespace {
// Internal implementation to split submodule configs eg:
//  * SplitIsp2xAeLittle
//  * SplitIsp2xAeBig0
//  * SplitIsp3xAeBig0
// Then a module can be an aggregation of submodules
// for example:
// SplitAecParams
//   -> SplitIsp2xAeLittle
//   -> SplitIsp3xAeBig0

/*********************************************/
/*            Aec hwi splitter               */
/*********************************************/

void SplitAecWeight(
    u8* ori_weight,
    u8* left_weight,
    u8* right_weight,
    WinSplitMode mode,
    u8 wnd_num
) {
    switch (mode) {
    case LEFT_AND_RIGHT_MODE:
        for (int i = 0; i < wnd_num; i++) {
            for (int j = 0; j < wnd_num; j++) {
                left_weight[i * wnd_num + j] = ori_weight[i * wnd_num + j / 2];
                right_weight[i * wnd_num + j] = ori_weight[i * wnd_num + j / 2 + j % 2 + wnd_num / 2];
            }
        }
        break;
    case LEFT_MODE:
    case RIGHT_MODE:
        memcpy(left_weight, ori_weight, wnd_num * wnd_num * sizeof(u8));
        memcpy(right_weight, ori_weight, wnd_num * wnd_num * sizeof(u8));
    default:
        break;
    }
}

void SplitAecSubWin(
    u8* subwin_en,
    struct isp2x_window* ori_win,
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    IspParamsSplitter::Rectangle left_isp_rect_,
    IspParamsSplitter::Rectangle right_isp_rect_,
    WinSplitMode* mode
) {

    for (int i = 0; i < ISP2X_RAWAEBIG_SUBWIN_NUM; i++) {
        if (subwin_en[i] == 1) { //only when subwin is enabled, should split hwi params
            if (ori_win[i].h_offs + ori_win[i].h_size <= left_isp_rect_.w) {
#ifdef DEBUG
                printf("win locate in left isp\n");
#endif
                mode[i] = LEFT_MODE;

                left_win[i].h_offs = ori_win[i].h_offs;
                left_win[i].h_size = ori_win[i].h_size;
                left_win[i].v_offs = ori_win[i].v_offs;
                left_win[i].v_size = ori_win[i].v_size;

                right_win[i].h_offs = 0;
                right_win[i].h_size = ori_win[i].h_size;
                right_win[i].v_offs = ori_win[i].v_offs;
                right_win[i].v_size = ori_win[i].v_size;
            }
            else if (ori_win[i].h_offs >= right_isp_rect_.x) {
#ifdef DEBUG
                printf("win locate in right isp\n");
#endif
                mode[i] = RIGHT_MODE;

                //win only locate in right isp, actually stats of left isp would not be used
                left_win[i].h_offs = 0;
                left_win[i].h_size = ori_win[i].h_size;
                left_win[i].v_offs = ori_win[i].v_offs;
                left_win[i].v_size = ori_win[i].v_size;

                right_win[i].h_offs = ori_win[i].h_offs - right_isp_rect_.x;
                right_win[i].h_size = ori_win[i].h_size;
                right_win[i].v_offs = ori_win[i].v_offs;
                right_win[i].v_size = ori_win[i].v_size;

            }
            else {
#ifdef DEBUG
                printf(" win locate at left&right isp\n");
#endif
                mode[i] = LEFT_AND_RIGHT_MODE;

                left_win[i].h_offs = ori_win[i].h_offs;
                left_win[i].h_size = MAX(0, ((long)left_isp_rect_.w - (long)left_win[i].h_offs));
                left_win[i].v_offs = ori_win[i].v_offs;
                left_win[i].v_size = ori_win[i].v_size;


                right_win[i].h_offs = left_win[i].h_offs + left_win[i].h_size - right_isp_rect_.x;
                right_win[i].h_size = MAX(0, ((long)ori_win[i].h_size - (long)left_win[i].h_size));
                right_win[i].v_offs = ori_win[i].v_offs;
                right_win[i].v_size = ori_win[i].v_size;
            }
        }
    }
}

void SplitAecCalcBlockSize(
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    u8 wnd_num,
    IspParamsSplitter::Rectangle right_isp_rect_,
    u16* block_h
) {

    bool loop_en = true;

    while (loop_en && *block_h > 0) {

        left_win->h_size = *block_h * wnd_num;
        right_win->h_offs = (left_win->h_size + left_win->h_offs > right_isp_rect_.x) ?
                            left_win->h_size + left_win->h_offs - right_isp_rect_.x : 0;

        if (u32(right_win->h_offs + *block_h * wnd_num) > right_isp_rect_.w - 1) {

            (*block_h)--;

        }
        else {
            loop_en = false;

            //if (*block_h % 2)
            //(*block_h)--;

            left_win->h_size = *block_h * wnd_num;
            right_win->h_offs = (left_win->h_size + left_win->h_offs > right_isp_rect_.x) ?
                                left_win->h_size + left_win->h_offs - right_isp_rect_.x : 0;
            right_win->h_offs = right_win->h_offs & 0xfffe;
            right_win->h_size = *block_h * wnd_num;
        }
    }
}

void SplitAecWin(
    isp2x_window* ori_win,
    isp2x_window* left_win,
    isp2x_window* right_win,
    u8 wnd_num,
    IspParamsSplitter::Rectangle left_isp_rect_,
    IspParamsSplitter::Rectangle right_isp_rect_,
    WinSplitMode* mode
) {

    //win only locate in left isp, actually stats of right isp would not be used
    if (ori_win->h_offs + ori_win->h_size <= left_isp_rect_.w) {

#ifdef DEBUG
        printf("win locate in left isp\n");
#endif
        *mode = LEFT_MODE;

        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = 0;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = ori_win->v_offs;
        right_win->v_size = ori_win->v_size;
    }
    else if (ori_win->h_offs >= right_isp_rect_.x) {

#ifdef DEBUG
        printf("win locate in right isp\n");
#endif
        *mode = RIGHT_MODE;

        //win only locate in right isp, actually stats of left isp would not be used
        left_win->h_offs = 0;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = ori_win->h_offs - right_isp_rect_.x;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = ori_win->v_offs;
        right_win->v_size = ori_win->v_size;

    }
    else {
        if ((ori_win->h_offs + ori_win->h_size / 2) <= left_isp_rect_.w
                && right_isp_rect_.x <= (ori_win->h_offs + ori_win->h_size / 2)) {
#ifdef DEBUG
            printf(" win locate at left&right isp,and center line locate in overlapping zone!\n");
#endif
            *mode = LEFT_AND_RIGHT_MODE;

            //win center locate at overlap zone
            left_win->h_offs = ori_win->h_offs;
            left_win->v_offs = ori_win->v_offs;
            left_win->v_size = ori_win->v_size;

            right_win->v_offs = ori_win->v_offs;
            right_win->v_size = ori_win->v_size;

            u16 block_h = ori_win->h_size / (2 * wnd_num);
            SplitAecCalcBlockSize(left_win, right_win, wnd_num, right_isp_rect_, &block_h);
        }
        else {
#ifdef DEBUG
            printf(" win locate at left&right isp,but center line not locate in overlapping zone!\n");
#endif
            if ((ori_win->h_offs + ori_win->h_size / 2) < right_isp_rect_.x) {

                left_win->h_offs = ori_win->h_offs;
                left_win->v_offs = ori_win->v_offs;
                left_win->v_size = ori_win->v_size;

                right_win->v_offs = ori_win->v_offs;
                right_win->v_size = ori_win->v_size;

                u16 h_size_tmp1 = left_isp_rect_.w - ori_win->h_offs;
                u16 h_size_tmp2 = (right_isp_rect_.x - ori_win->h_offs) * 2;

                if (abs(ori_win->h_size - h_size_tmp1) < abs(ori_win->h_size - h_size_tmp2)) {
#ifdef DEBUG
                    printf("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp1);
#endif
                    *mode = LEFT_MODE;

                    ori_win->h_size = h_size_tmp1;

                    left_win->h_size = ori_win->h_size;
                    //actually stats of right isp would not be used
                    right_win->h_offs = 0;
                    right_win->h_size = ori_win->h_size;
                }
                else {
#ifdef DEBUG
                    printf("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp2);
#endif
                    *mode = LEFT_AND_RIGHT_MODE;

                    ori_win->h_size = h_size_tmp2;

                    u16 block_h = ori_win->h_size / (2 * wnd_num);
                    SplitAecCalcBlockSize(left_win, right_win, wnd_num, right_isp_rect_, &block_h);
                }
            }
            else {
                left_win->v_offs = ori_win->v_offs;
                left_win->v_size = ori_win->v_size;

                right_win->v_offs = ori_win->v_offs;
                right_win->v_size = ori_win->v_size;

                u16 h_size_tmp1 = ori_win->h_offs + ori_win->h_size - right_isp_rect_.x;
                u16 h_size_tmp2 = (ori_win->h_offs + ori_win->h_size - left_isp_rect_.w) * 2;

                if (abs(ori_win->h_size - h_size_tmp1) < abs(ori_win->h_size - h_size_tmp2)) {

#ifdef DEBUG
                    printf("correct glb.h_off %d to %u\n", ori_win->h_offs, right_isp_rect_.x);
                    printf("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp1);
#endif

                    *mode = RIGHT_MODE;

                    ori_win->h_size = h_size_tmp1;
                    ori_win->h_offs = right_isp_rect_.x;

                    right_win->h_offs = 0;
                    right_win->h_size = ori_win->h_size;

                    //actually stats of left isp would not be used
                    left_win->h_offs = 0;
                    left_win->h_size = ori_win->h_size;
                }
                else {
#ifdef DEBUG
                    printf("correct glb.h_off %d to %d\n", ori_win->h_offs,
                           ori_win->h_offs + ori_win->h_size - (ori_win->h_offs + ori_win->h_size - left_isp_rect_.w) * 2);
                    printf("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp2);
#endif
                    *mode = LEFT_AND_RIGHT_MODE;

                    ori_win->h_offs = ori_win->h_offs + ori_win->h_size - (ori_win->h_offs + ori_win->h_size - left_isp_rect_.w) * 2;
                    ori_win->h_size = h_size_tmp2;
                    left_win->h_offs = ori_win->h_offs;

                    u16 block_h = ori_win->h_size / (2 * wnd_num);
                    SplitAecCalcBlockSize(left_win, right_win, wnd_num, right_isp_rect_, &block_h);
                }
            }
        }
    }
}

/*********************************************/
/*      other module hwi splitter            */
/*********************************************/

void SplitAwbCalcBlockSize(
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    u8 ds_awb,
    u8 wnd_num,
    IspParamsSplitter::Rectangle right_isp_rect_,
    u16* block_h
) {

    bool loop_en = true;

    while (loop_en && *block_h > 0) {

        left_win->h_size = (*block_h  * wnd_num) << ds_awb;

        right_win->h_offs = (left_win->h_size + left_win->h_offs > right_isp_rect_.x) ?
                            left_win->h_size + left_win->h_offs - right_isp_rect_.x : 0;

        if (u32(right_win->h_offs + left_win->h_size) > right_isp_rect_.w ) {
            (*block_h)--;
        }
        else {
            loop_en = false;

            //if (*block_h % 2)
            //    (*block_h)--;

            left_win->h_size = (*block_h  * wnd_num) << ds_awb;
            right_win->h_offs = (left_win->h_size + left_win->h_offs > right_isp_rect_.x) ?
                                left_win->h_size + left_win->h_offs - right_isp_rect_.x : 0;
            right_win->h_offs = right_win->h_offs & 0xfffe;
            right_win->h_size = (*block_h  * wnd_num) << ds_awb;
        }
    }
}

void SplitAwbWin(
    isp2x_window* ori_win,
    isp2x_window* left_win,
    isp2x_window* right_win,
    u8 ds_awb,
    u8 wnd_num,
    IspParamsSplitter::Rectangle left_isp_rect_,
    IspParamsSplitter::Rectangle right_isp_rect_,
    WinSplitMode* mode
) {

    u16 win_ds_hsize = ori_win->h_size >> ds_awb;
    u16 ori_win_hsize_clip = win_ds_hsize << ds_awb;

    //win only locate in left isp, actually stats of right isp would not be used
    if (ori_win->h_offs + ori_win_hsize_clip <= left_isp_rect_.w) {

        LOG1_AWB("win locate in left isp\n");

        *mode = LEFT_MODE;

        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win_hsize_clip;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = 0;
        right_win->h_size = ori_win_hsize_clip;
        right_win->v_offs = ori_win->v_offs;
        right_win->v_size = ori_win->v_size;
    }
    else if (ori_win->h_offs >= right_isp_rect_.x) {

        LOG1_AWB("win locate in right isp\n");

        *mode = RIGHT_MODE;

        //win only locate in right isp, actually stats of left isp would not be used
        left_win->h_offs = 0;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = ori_win->h_offs - right_isp_rect_.x;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = ori_win->v_offs;
        right_win->v_size = ori_win->v_size;

    }
    else {

        if ((ori_win->h_offs + ori_win->h_size / 2) <= left_isp_rect_.w
                && right_isp_rect_.x <= (ori_win->h_offs + ori_win->h_size / 2)) {

            LOG1_AWB(" win locate at left&right isp,and center line locate in overlapping zone!\n");

            *mode = LEFT_AND_RIGHT_MODE;

            //win center locate at overlap zone
            left_win->h_offs = ori_win->h_offs;
            left_win->v_offs = ori_win->v_offs;
            left_win->v_size = ori_win->v_size;

            right_win->v_offs = ori_win->v_offs;
            right_win->v_size = ori_win->v_size;
            // u16 block_h = ori_win->h_size / (2 * wnd_num);

            u16 block_h = win_ds_hsize / (2 * wnd_num);

            left_win->h_size = (block_h  * wnd_num) << ds_awb;

            right_win->h_offs = (left_win->h_size + left_win->h_offs > right_isp_rect_.x) ?
                                left_win->h_size + left_win->h_offs - right_isp_rect_.x : 0;
            right_win->h_offs = right_win->h_offs & 0xfffe;
            right_win->h_size = (win_ds_hsize - block_h * wnd_num) << ds_awb;
            right_win->h_size = right_win->h_offs + right_win->h_size > right_isp_rect_.w ? (right_isp_rect_.w - right_win->h_offs) : right_win->h_size;
        }
        else {

            LOG1_AWB(" win locate at left&right isp,but center line not locate in overlapping zone!\n");

            if ((ori_win->h_offs + ori_win->h_size / 2) < right_isp_rect_.x) {

                left_win->h_offs = ori_win->h_offs;
                left_win->v_offs = ori_win->v_offs;
                left_win->v_size = ori_win->v_size;

                right_win->v_offs = ori_win->v_offs;
                right_win->v_size = ori_win->v_size;

                u16 h_size_tmp1 = left_isp_rect_.w - ori_win->h_offs;
                u16 h_size_tmp2 = (right_isp_rect_.x - ori_win->h_offs) * 2;

                if (abs(ori_win_hsize_clip - h_size_tmp1) < abs(ori_win_hsize_clip - h_size_tmp2)) {

                    LOG1_AWB("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp1);

                    *mode = LEFT_MODE;

                    ori_win->h_size = h_size_tmp1;

                    left_win->h_size = ori_win->h_size;
                    //actually stats of right isp would not be used
                    right_win->h_offs = 0;
                    right_win->h_size = ori_win->h_size;
                }
                else {
                    LOG1_AWB("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp2);
                    *mode = LEFT_AND_RIGHT_MODE;

                    ori_win->h_size = h_size_tmp2;
                    win_ds_hsize = ori_win->h_size >> ds_awb;

                    u16 block_h = win_ds_hsize / (2 * wnd_num);

                    SplitAwbCalcBlockSize(left_win, right_win, ds_awb, wnd_num, right_isp_rect_, &block_h);
                }
            }
            else {
                left_win->v_offs = ori_win->v_offs;
                left_win->v_size = ori_win->v_size;

                right_win->v_offs = ori_win->v_offs;
                right_win->v_size = ori_win->v_size;

                u16 h_size_tmp1 = ori_win->h_offs + ori_win->h_size - right_isp_rect_.x;
                u16 h_size_tmp2 = (ori_win->h_offs + ori_win->h_size - left_isp_rect_.w) * 2;

                if (abs(ori_win_hsize_clip - h_size_tmp1) < abs(ori_win_hsize_clip - h_size_tmp2)) {

                    LOG1_AWB("correct glb.h_off %d to %d\n", ori_win->h_offs, right_isp_rect_.x);
                    LOG1_AWB("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp1);

                    *mode = RIGHT_MODE;

                    ori_win->h_size = h_size_tmp1;
                    ori_win->h_offs = right_isp_rect_.x;

                    right_win->h_offs = 0;
                    right_win->h_size = ori_win->h_size;

                    //actually stats of left isp would not be used
                    left_win->h_offs = 0;
                    left_win->h_size = ori_win->h_size;
                }
                else {

                    LOG1_AWB("correct glb.h_off %d to %d\n", ori_win->h_offs,
                             ori_win->h_offs + ori_win->h_size - (ori_win->h_offs + ori_win->h_size - left_isp_rect_.w) * 2);
                    LOG1_AWB("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp2);

                    *mode = LEFT_AND_RIGHT_MODE;

                    ori_win->h_offs = ori_win->h_offs + ori_win->h_size - (ori_win->h_offs + ori_win->h_size - left_isp_rect_.w) * 2;
                    ori_win->h_size = h_size_tmp2;
                    left_win->h_offs = ori_win->h_offs;

                    win_ds_hsize = ori_win->h_size >> ds_awb;

                    u16 block_h = win_ds_hsize / (2 * wnd_num);

                    SplitAwbCalcBlockSize(left_win, right_win, ds_awb, wnd_num, right_isp_rect_, &block_h);
                }
            }
        }
    }
}

void SplitAwbMultiWin(
    struct isp2x_window* ori_win,
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    struct isp2x_window* main_left_win,
    struct isp2x_window* main_right_win,
    IspParamsSplitter::Rectangle left_isp_rect_,
    IspParamsSplitter::Rectangle right_isp_rect_,
    WinSplitMode* mode
) {
    if (ori_win->h_offs + ori_win->h_size <= main_left_win->h_offs + main_left_win->h_size) {

        LOG1_AWB("win locate in left isp\n");

        *mode = LEFT_MODE;

        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = 0;
        right_win->h_size = 0;
        right_win->v_offs = 0;
        right_win->v_size = 0;
    }
    else if (ori_win->h_offs >= right_isp_rect_.x + main_right_win->h_offs) {

        LOG1_AWB("win locate in right isp\n");

        *mode = RIGHT_MODE;

        //win only locate in right isp, actually stats of left isp would not be used
        left_win->h_offs = 0;
        left_win->h_size = 0;
        left_win->v_offs = 0;
        left_win->v_size = 0;

        right_win->h_offs = MAX((int)main_right_win->h_offs, (int)ori_win->h_offs - (int)right_isp_rect_.x);
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = ori_win->v_offs;
        right_win->v_size = ori_win->v_size;

    }
    else {

        LOG1_AWB(" win locate at left&right isp\n");

        *mode = LEFT_AND_RIGHT_MODE;

        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = MAX(0, ((int)main_left_win->h_offs + (int)main_left_win->h_size - (int)left_win->h_offs));
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win->v_size;


        right_win->h_offs = MAX((int)main_right_win->h_offs, (int)left_win->h_offs + (int)left_win->h_size - (int)right_isp_rect_.x);
        right_win->h_size = MAX(0, ((int)ori_win->h_size - (int)left_win->h_size));
        right_win->v_offs = ori_win->v_offs;
        right_win->v_size = ori_win->v_size;
    }
}

void SplitAecWeightVertical(
    u8* ori_weight,
    u8* left_weight,
    u8* right_weight,
    WinSplitMode mode,
    u8 wnd_num
) {
    switch (mode) {
    case LEFT_AND_RIGHT_MODE:
        for (int i = 0; i < wnd_num; i++) {
            for (int j = 0; j < wnd_num; j++) {
                left_weight[i + j * wnd_num] = ori_weight[i + j / 2 * wnd_num];
                right_weight[i + j * wnd_num] = ori_weight[i + wnd_num * (j / 2 + j % 2 + wnd_num / 2)];
            }
        }
        break;
    case LEFT_MODE:
    case RIGHT_MODE:
        memcpy(left_weight, ori_weight, wnd_num * wnd_num * sizeof(u8));
        memcpy(right_weight, ori_weight, wnd_num * wnd_num * sizeof(u8));
    default:
        break;
    }
}

void SplitAecCalcBlockSizeVertical(
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    u8 wnd_num,
    IspParamsSplitter::Rectangle right_isp_rect_,
    u16* block_v
) {

    bool loop_en = true;

    while (loop_en && *block_v > 0) {

        left_win->v_size = *block_v * wnd_num;
        right_win->v_offs = (left_win->v_size + left_win->v_offs > right_isp_rect_.y) ?
                            left_win->v_size + left_win->v_offs - right_isp_rect_.y : 0;

        if (u32(right_win->v_offs + *block_v * wnd_num) > right_isp_rect_.h - 1) {

            (*block_v)--;

        }
        else {
            loop_en = false;

            //if (*block_h % 2)
            //(*block_h)--;

            left_win->v_size = *block_v * wnd_num;
            right_win->v_offs = (left_win->v_size + left_win->v_offs > right_isp_rect_.y) ?
                                left_win->v_size + left_win->v_offs - right_isp_rect_.y : 0;
            right_win->v_offs = right_win->v_offs & 0xfffe;
            right_win->v_size = *block_v * wnd_num;
        }
    }
}

void SplitAecSubWinVertical(
    u8* subwin_en,
    struct isp2x_window* ori_win,
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    IspParamsSplitter::Rectangle left_isp_rect_,
    IspParamsSplitter::Rectangle right_isp_rect_,
    WinSplitMode* mode
) {

    for (int i = 0; i < ISP2X_RAWAEBIG_SUBWIN_NUM; i++) {
        if (subwin_en[i] == 1) { //only when subwin is enabled, should split hwi params
            if (ori_win[i].v_offs + ori_win[i].v_size <= left_isp_rect_.h) {
#ifdef DEBUG
                printf("win locate in left isp\n");
#endif
                mode[i] = LEFT_MODE;

                left_win[i].h_offs = ori_win[i].h_offs;
                left_win[i].h_size = ori_win[i].h_size;
                left_win[i].v_offs = ori_win[i].v_offs;
                left_win[i].v_size = ori_win[i].v_size;

                right_win[i].h_offs = ori_win[i].h_offs;
                right_win[i].h_size = ori_win[i].h_size;
                right_win[i].v_offs = 0;
                right_win[i].v_size = ori_win[i].v_size;
            }
            else if (ori_win[i].v_offs >= right_isp_rect_.y) {
#ifdef DEBUG
                printf("win locate in right isp\n");
#endif
                mode[i] = RIGHT_MODE;

                //win only locate in right isp, actually stats of left isp would not be used
                left_win[i].h_offs = ori_win[i].h_offs;
                left_win[i].h_size = ori_win[i].h_size;
                left_win[i].v_offs = 0;
                left_win[i].v_size = ori_win[i].v_size;

                right_win[i].h_offs = ori_win[i].h_offs;
                right_win[i].h_size = ori_win[i].h_size;
                right_win[i].v_offs = ori_win[i].v_offs - right_isp_rect_.y;
                right_win[i].v_size = ori_win[i].v_size;

            }
            else {
#ifdef DEBUG
                printf(" win locate at left&right isp\n");
#endif
                mode[i] = LEFT_AND_RIGHT_MODE;

                left_win[i].h_offs = ori_win[i].h_offs;
                left_win[i].h_size = ori_win[i].h_size;
                left_win[i].v_offs = ori_win[i].v_offs;
                left_win[i].v_size = MAX(0, ((long)left_isp_rect_.h - (long)left_win[i].v_offs));

                right_win[i].h_offs = right_win[i].h_offs;
                right_win[i].h_size = right_win[i].h_size;
                right_win[i].v_offs = left_win[i].v_offs + left_win[i].v_size - right_isp_rect_.y;
                right_win[i].v_size = MAX(0, ((long)ori_win[i].v_size - (long)left_win[i].v_size));
            }
        }
    }
}

void SplitAecWinVertical(
    isp2x_window* ori_win,
    isp2x_window* left_win,
    isp2x_window* right_win,
    u8 wnd_num,
    IspParamsSplitter::Rectangle left_isp_rect_,
    IspParamsSplitter::Rectangle right_isp_rect_,
    WinSplitMode* mode
) {

    //win only locate in left isp, actually stats of right isp would not be used
    if (ori_win->v_offs + ori_win->v_size <= left_isp_rect_.h) {

#ifdef DEBUG
        printf("win locate in left isp\n");
#endif
        *mode = LEFT_MODE;

        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = ori_win->h_offs;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = 0;
        right_win->v_size = ori_win->v_size;
    }
    else if (ori_win->v_offs >= right_isp_rect_.y) {

#ifdef DEBUG
        printf("win locate in right isp\n");
#endif
        *mode = RIGHT_MODE;

        //win only locate in right isp, actually stats of left isp would not be used
        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = 0;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = ori_win->h_offs;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = ori_win->v_offs - right_isp_rect_.y;
        right_win->v_size = ori_win->v_size;

    }
    else {
        if ((ori_win->v_offs + ori_win->v_size / 2) <= left_isp_rect_.h
                && right_isp_rect_.y <= (ori_win->v_offs + ori_win->v_size / 2)) {
#ifdef DEBUG
            printf(" win locate at left&right isp,and center line locate in overlapping zone!\n");
#endif
            *mode = LEFT_AND_RIGHT_MODE;

            //win center locate at overlap zone
            left_win->v_offs = ori_win->v_offs;
            left_win->h_offs = ori_win->h_offs;
            left_win->h_size = ori_win->h_size;

            right_win->h_offs = ori_win->h_offs;
            right_win->h_size = ori_win->h_size;

            u16 block_v = ori_win->v_size / (2 * wnd_num);
            SplitAecCalcBlockSizeVertical(left_win, right_win, wnd_num, right_isp_rect_, &block_v);
        }
        else {
#ifdef DEBUG
            printf(" win locate at left&right isp,but center line not locate in overlapping zone!\n");
#endif
            if ((ori_win->v_offs + ori_win->v_size / 2) < right_isp_rect_.y) {

                left_win->v_offs = ori_win->v_offs;
                left_win->h_offs = ori_win->h_offs;
                left_win->h_size = ori_win->h_size;

                right_win->h_offs = ori_win->h_offs;
                right_win->h_size = ori_win->h_size;

                u16 v_size_tmp1 = left_isp_rect_.h - ori_win->v_offs;
                u16 v_size_tmp2 = (right_isp_rect_.y - ori_win->v_offs) * 2;

                if (abs(ori_win->v_size - v_size_tmp1) < abs(ori_win->v_size - v_size_tmp2)) {
#ifdef DEBUG
                    printf("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp1);
#endif
                    *mode = LEFT_MODE;

                    ori_win->v_size = v_size_tmp1;

                    left_win->v_size = ori_win->v_size;
                    //actually stats of right isp would not be used
                    right_win->v_offs = 0;
                    right_win->v_size = ori_win->v_size;
                }
                else {
#ifdef DEBUG
                    printf("correct glb.h_size %d to %d\n", ori_win->h_size, h_size_tmp2);
#endif
                    *mode = LEFT_AND_RIGHT_MODE;

                    ori_win->v_size = v_size_tmp2;

                    u16 block_v = ori_win->v_size / (2 * wnd_num);
                    SplitAecCalcBlockSizeVertical(left_win, right_win, wnd_num, right_isp_rect_, &block_v);
                }
            }
            else {
                left_win->h_offs = ori_win->h_offs;
                left_win->h_size = ori_win->h_size;

                right_win->h_offs = ori_win->h_offs;
                right_win->h_size = ori_win->h_size;

                u16 v_size_tmp1 = ori_win->v_offs + ori_win->v_size - right_isp_rect_.y;
                u16 v_size_tmp2 = (ori_win->v_offs + ori_win->v_size - left_isp_rect_.h) * 2;

                if (abs(ori_win->v_size - v_size_tmp1) < abs(ori_win->v_size - v_size_tmp2)) {

#ifdef DEBUG
                    printf("correct glb.h_off %d to %u\n", ori_win->v_offs, right_isp_rect_.y);
                    printf("correct glb.h_size %d to %d\n", ori_win->v_size, v_size_tmp1);
#endif

                    *mode = RIGHT_MODE;

                    ori_win->v_size = v_size_tmp1;
                    ori_win->v_offs = right_isp_rect_.y;

                    right_win->v_offs = 0;
                    right_win->v_size = ori_win->v_size;

                    //actually stats of left isp would not be used
                    left_win->v_offs = 0;
                    left_win->v_size = ori_win->v_size;
                }
                else {
#ifdef DEBUG
                    printf("correct glb.h_off %d to %d\n", ori_win->v_offs,
                           ori_win->v_offs + ori_win->v_size - (ori_win->v_offs + ori_win->v_size - left_isp_rect_.h) * 2);
                    printf("correct glb.h_size %d to %d\n", ori_win->v_size, v_size_tmp2);
#endif
                    *mode = LEFT_AND_RIGHT_MODE;

                    ori_win->v_offs = ori_win->v_offs + ori_win->v_size - (ori_win->v_offs + ori_win->v_size - left_isp_rect_.h) * 2;
                    ori_win->v_size = v_size_tmp2;
                    left_win->v_offs = ori_win->v_offs;

                    u16 block_v = ori_win->v_size / (2 * wnd_num);
                    SplitAecCalcBlockSizeVertical(left_win, right_win, wnd_num, right_isp_rect_, &block_v);
                }
            }
        }
    }
}

void SplitAwbCalcBlockSizeVertical(
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    u8 ds_awb,
    u8 wnd_num,
    IspParamsSplitter::Rectangle right_isp_rect_,
    u16* block_v
) {

    bool loop_en = true;

    while (loop_en && *block_v > 0) {

        left_win->v_size = (*block_v  * wnd_num) << ds_awb;

        right_win->v_offs = (left_win->v_size + left_win->v_offs > right_isp_rect_.y) ?
                            left_win->v_size + left_win->v_offs - right_isp_rect_.y : 0;

        if (u32(right_win->v_offs + left_win->v_size) > right_isp_rect_.h ) {
            (*block_v)--;
        }
        else {
            loop_en = false;

            //if (*block_v % 2)
            //    (*block_v)--;

            left_win->v_size = (*block_v  * wnd_num) << ds_awb;
            right_win->v_offs = (left_win->v_size + left_win->v_offs > right_isp_rect_.y) ?
                                left_win->v_size + left_win->v_offs - right_isp_rect_.y : 0;
            right_win->v_offs = right_win->v_offs & 0xfffe;
            right_win->v_size = (*block_v  * wnd_num) << ds_awb;
        }
    }
}

void SplitAwbWinVertical(
    isp2x_window* ori_win,
    isp2x_window* left_win,
    isp2x_window* right_win,
    u8 ds_awb,
    u8 wnd_num,
    IspParamsSplitter::Rectangle left_isp_rect_,
    IspParamsSplitter::Rectangle right_isp_rect_,
    WinSplitMode* mode
) {

    u16 win_ds_vsize = ori_win->v_size >> ds_awb;
    u16 ori_win_vsize_clip = win_ds_vsize << ds_awb;

    //win only locate in left isp, actually stats of right isp would not be used
    if (ori_win->v_offs + ori_win_vsize_clip <= left_isp_rect_.h) {

        LOG1_AWB("win locate in left isp\n");

        *mode = LEFT_MODE;

        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win_vsize_clip;

        right_win->h_offs = ori_win->h_offs;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = 0;
        right_win->v_size = ori_win_vsize_clip;
    }
    else if (ori_win->h_offs >= right_isp_rect_.y) {

        LOG1_AWB("win locate in right isp\n");

        *mode = RIGHT_MODE;

        //win only locate in right isp, actually stats of left isp would not be used
        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = 0;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = ori_win->h_offs;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = ori_win->v_offs - right_isp_rect_.y;
        right_win->v_size = ori_win->v_size;

    }
    else {

        if ((ori_win->v_offs + ori_win->v_size / 2) <= left_isp_rect_.h
                && right_isp_rect_.y <= (ori_win->v_offs + ori_win->v_size / 2)) {

            LOG1_AWB(" win locate at left&right isp,and center line locate in overlapping zone!\n");

            *mode = LEFT_AND_RIGHT_MODE;

            //win center locate at overlap zone
            left_win->v_offs = ori_win->v_offs;
            left_win->h_offs = ori_win->h_offs;
            left_win->h_size = ori_win->h_size;

            right_win->h_offs = ori_win->h_offs;
            right_win->h_size = ori_win->h_size;
            // u16 block_v = ori_win->v_size / (2 * wnd_num);

            u16 block_v = win_ds_vsize / (2 * wnd_num);

            left_win->v_size = (block_v  * wnd_num) << ds_awb;

            right_win->v_offs = (left_win->v_size + left_win->v_offs > right_isp_rect_.y) ?
                                left_win->v_size + left_win->v_offs - right_isp_rect_.y : 0;
            right_win->v_offs = right_win->v_offs & 0xfffe;
            right_win->v_size = (win_ds_vsize - block_v * wnd_num) << ds_awb;
            right_win->v_size = right_win->v_offs + right_win->v_size > right_isp_rect_.h ? (right_isp_rect_.h - right_win->v_offs) : right_win->v_size;
        }
        else {

            LOG1_AWB(" win locate at left&right isp,but center line not locate in overlapping zone!\n");

            if ((ori_win->v_offs + ori_win->v_size / 2) < right_isp_rect_.y) {

                left_win->v_offs = ori_win->v_offs;
                left_win->h_offs = ori_win->h_offs;
                left_win->h_size = ori_win->h_size;

                right_win->h_offs = ori_win->h_offs;
                right_win->h_size = ori_win->h_size;

                u16 v_size_tmp1 = left_isp_rect_.h - ori_win->v_offs;
                u16 v_size_tmp2 = (right_isp_rect_.y - ori_win->v_offs) * 2;

                if (abs(ori_win_vsize_clip - v_size_tmp1) < abs(ori_win_vsize_clip - v_size_tmp2)) {

                    LOG1_AWB("correct glb.v_size %d to %d\n", ori_win->v_size, v_size_tmp1);

                    *mode = LEFT_MODE;

                    ori_win->v_size = v_size_tmp1;

                    left_win->v_size = ori_win->v_size;
                    //actually stats of right isp would not be used
                    right_win->v_offs = 0;
                    right_win->v_size = ori_win->v_size;
                }
                else {
                    LOG1_AWB("correct glb.h_size %d to %d\n", ori_win->v_size, v_size_tmp2);
                    *mode = LEFT_AND_RIGHT_MODE;

                    ori_win->v_size = v_size_tmp2;
                    win_ds_vsize = ori_win->v_size >> ds_awb;

                    u16 block_v = win_ds_vsize / (2 * wnd_num);

                    SplitAwbCalcBlockSizeVertical(left_win, right_win, ds_awb, wnd_num, right_isp_rect_, &block_v);
                }
            }
            else {
                left_win->h_offs = ori_win->h_offs;
                left_win->h_size = ori_win->h_size;

                right_win->h_offs = ori_win->h_offs;
                right_win->h_size = ori_win->h_size;

                u16 v_size_tmp1 = ori_win->v_offs + ori_win->v_size - right_isp_rect_.y;
                u16 v_size_tmp2 = (ori_win->v_offs + ori_win->v_size - left_isp_rect_.h) * 2;

                if (abs(ori_win_vsize_clip - v_size_tmp1) < abs(ori_win_vsize_clip - v_size_tmp2)) {

                    LOG1_AWB("correct glb.v_off %d to %d\n", ori_win->v_offs, right_isp_rect_.y);
                    LOG1_AWB("correct glb.v_size %d to %d\n", ori_win->v_size, v_size_tmp1);

                    *mode = RIGHT_MODE;

                    ori_win->v_size = v_size_tmp1;
                    ori_win->v_offs = right_isp_rect_.y;

                    right_win->v_offs = 0;
                    right_win->v_size = ori_win->v_size;

                    //actually stats of left isp would not be used
                    left_win->v_offs = 0;
                    left_win->v_size = ori_win->v_size;
                }
                else {

                    LOG1_AWB("correct glb.v_off %d to %d\n", ori_win->v_offs,
                             ori_win->v_offs + ori_win->v_size - (ori_win->v_offs + ori_win->v_size - left_isp_rect_.h) * 2);
                    LOG1_AWB("correct glb.v_size %d to %d\n", ori_win->v_size, v_size_tmp2);

                    *mode = LEFT_AND_RIGHT_MODE;

                    ori_win->v_offs = ori_win->v_offs + ori_win->v_size - (ori_win->v_offs + ori_win->v_size - left_isp_rect_.h) * 2;
                    ori_win->v_size = v_size_tmp2;
                    left_win->v_offs = ori_win->v_offs;

                    win_ds_vsize = ori_win->v_size >> ds_awb;

                    u16 block_v = win_ds_vsize / (2 * wnd_num);

                    SplitAwbCalcBlockSizeVertical(left_win, right_win, ds_awb, wnd_num, right_isp_rect_, &block_v);
                }
            }
        }
    }
}

void SplitAwbMultiWinVertical(
    struct isp2x_window* ori_win,
    struct isp2x_window* left_win,
    struct isp2x_window* right_win,
    struct isp2x_window* main_left_win,
    struct isp2x_window* main_right_win,
    IspParamsSplitter::Rectangle left_isp_rect_,
    IspParamsSplitter::Rectangle right_isp_rect_,
    WinSplitMode* mode
) {
    if (ori_win->v_offs + ori_win->v_size <= main_left_win->v_offs + main_left_win->v_size) {

        LOG1_AWB("win locate in left isp\n");

        *mode = LEFT_MODE;

        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = ori_win->v_size;

        right_win->h_offs = 0;
        right_win->h_size = 0;
        right_win->v_offs = 0;
        right_win->v_size = 0;
    }
    else if (ori_win->v_offs >= right_isp_rect_.y + main_right_win->v_offs) {

        LOG1_AWB("win locate in right isp\n");

        *mode = RIGHT_MODE;

        //win only locate in right isp, actually stats of left isp would not be used
        left_win->h_offs = 0;
        left_win->h_size = 0;
        left_win->v_offs = 0;
        left_win->v_size = 0;

        right_win->h_offs = ori_win->h_offs;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = MAX((int)main_right_win->v_offs, (int)ori_win->v_offs - (int)right_isp_rect_.y);
        right_win->v_size = ori_win->v_size;

    }
    else {

        LOG1_AWB(" win locate at left&right isp\n");

        *mode = LEFT_AND_RIGHT_MODE;

        left_win->h_offs = ori_win->h_offs;
        left_win->h_size = ori_win->h_size;
        left_win->v_offs = ori_win->v_offs;
        left_win->v_size = MAX(0, ((int)main_left_win->v_offs + (int)main_left_win->v_size - (int)left_win->v_offs));
        
        right_win->h_offs = ori_win->h_offs;
        right_win->h_size = ori_win->h_size;
        right_win->v_offs = MAX((int)main_right_win->v_offs, (int)left_win->v_offs + (int)left_win->v_size - (int)right_isp_rect_.y);
        right_win->v_size = MAX(0, ((int)ori_win->v_size - (int)left_win->v_size));
    }
}

}

template <>
XCamReturn IspParamsSplitter::SplitRawAeLiteParams<struct isp2x_rawaelite_meas_cfg>(
    struct isp2x_rawaelite_meas_cfg* ori, struct isp2x_rawaelite_meas_cfg* left,
    struct isp2x_rawaelite_meas_cfg* right) {

    u8 wnd_num = 0;
    if (ori->wnd_num == 0)
        wnd_num = 1;
    else
        wnd_num = 5;

    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    WinSplitMode sub_mode[4] = {LEFT_AND_RIGHT_MODE};

    SplitAecWin(&ori->win, &left->win, &right->win, wnd_num, left_isp_rect_, right_isp_rect_, &mode);

#ifdef DEBUG
    printf("AeLite left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->win.h_offs, left->win.v_offs,
           left->win.h_size, left->win.v_size, right->win.h_offs, right->win.v_offs,
           right->win.h_size, right->win.v_size);
#endif

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitRawAeBigParams<struct isp2x_rawaebig_meas_cfg>(
    struct isp2x_rawaebig_meas_cfg* ori, struct isp2x_rawaebig_meas_cfg* left,
    struct isp2x_rawaebig_meas_cfg* right) {

    u8 wnd_num = 0;

    if (ori->wnd_num == 0)
        wnd_num = 1;
    else if (ori->wnd_num == 1)
        wnd_num = 5;
    else
        wnd_num = 15;

    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    WinSplitMode sub_mode[4] = {LEFT_AND_RIGHT_MODE};

    SplitAecWin(&ori->win, &left->win, &right->win, wnd_num, left_isp_rect_, right_isp_rect_, &mode);
    SplitAecSubWin(ori->subwin_en, ori->subwin, left->subwin, right->subwin, left_isp_rect_, right_isp_rect_, sub_mode);

    for (int i = 0; i < ISP2X_RAWAEBIG_SUBWIN_NUM; i++) {
        if (ori->subwin_en[i]) {
            switch (sub_mode[i]) {
            case LEFT_AND_RIGHT_MODE:
                left->subwin_en[i] = true;
                right->subwin_en[i] = true;
                break;
            case LEFT_MODE:
                left->subwin_en[i] = true;
                right->subwin_en[i] = false;
                break;
            case RIGHT_MODE:
                left->subwin_en[i] = false;
                right->subwin_en[i] = true;
                break;

            }
        }
    }

#ifdef DEBUG
    printf("AeBig left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->win.h_offs, left->win.v_offs,
           left->win.h_size, left->win.v_size, right->win.h_offs, right->win.v_offs,
           right->win.h_size, right->win.v_size);
#endif

    return XCAM_RETURN_NO_ERROR;

}


template <>
XCamReturn IspParamsSplitter::SplitRawHistLiteParams<struct isp2x_rawhistlite_cfg>(
    struct isp2x_rawhistlite_cfg* ori, struct isp2x_rawhistlite_cfg* left,
    struct isp2x_rawhistlite_cfg* right) {

    u8 wnd_num = 0;
    wnd_num = 5;

    WinSplitMode mode = LEFT_AND_RIGHT_MODE;

    SplitAecWin(&ori->win, &left->win, &right->win, wnd_num, left_isp_rect_, right_isp_rect_, &mode);
    SplitAecWeight(ori->weight, left->weight, right->weight, mode, wnd_num);

#ifdef DEBUG
    printf("HistLite left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->win.h_offs, left->win.v_offs,
           left->win.h_size, left->win.v_size, right->win.h_offs, right->win.v_offs,
           right->win.h_size, right->win.v_size);

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            printf("%d ", left->weight[i * wnd_num + j]);
        printf("\n");
    }
    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            printf("%d ", right->weight[i * wnd_num + j]);
        printf("\n");
    }
#endif

    return XCAM_RETURN_NO_ERROR;

}

template <>
XCamReturn IspParamsSplitter::SplitRawHistBigParams<struct isp2x_rawhistbig_cfg>(
    struct isp2x_rawhistbig_cfg* ori, struct isp2x_rawhistbig_cfg* left,
    struct isp2x_rawhistbig_cfg* right) {

    u8 wnd_num = 0;

    if (ori->wnd_num <= 1)
        wnd_num = 5;
    else
        wnd_num = 15;


    WinSplitMode mode = LEFT_AND_RIGHT_MODE;

    SplitAecWin(&ori->win, &left->win, &right->win, wnd_num, left_isp_rect_, right_isp_rect_, &mode);
    SplitAecWeight(ori->weight, left->weight, right->weight, mode, wnd_num);

#ifdef DEBUG
    printf("HistBig left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->win.h_offs, left->win.v_offs,
           left->win.h_size, left->win.v_size, right->win.h_offs, right->win.v_offs,
           right->win.h_size, right->win.v_size);

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            printf("%d ", left->weight[i * wnd_num + j]);
        printf("\n");
    }
    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            printf("%d ", right->weight[i * wnd_num + j]);
        printf("\n");
    }

#endif

    return XCAM_RETURN_NO_ERROR;

}


template <>
XCamReturn IspParamsSplitter::SplitAecParams<struct isp3x_isp_params_cfg>(
    struct isp3x_isp_params_cfg* ori,
    struct isp3x_isp_params_cfg* left,
    struct isp3x_isp_params_cfg* right) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //RAWAE
    if (ori->module_cfg_update & ISP3X_MODULE_RAWAE0)
        ret = SplitRawAeLiteParams(&ori->meas.rawae0, &left->meas.rawae0, &right->meas.rawae0);
    if (ori->module_cfg_update & ISP3X_MODULE_RAWAE1)
        ret = SplitRawAeBigParams(&ori->meas.rawae1, &left->meas.rawae1, &right->meas.rawae1);
    if (ori->module_cfg_update & ISP3X_MODULE_RAWAE2)
        ret = SplitRawAeBigParams(&ori->meas.rawae2, &left->meas.rawae2, &right->meas.rawae2);
    if (ori->module_cfg_update & ISP3X_MODULE_RAWAE3)
        ret = SplitRawAeBigParams(&ori->meas.rawae3, &left->meas.rawae3, &right->meas.rawae3);

    //RAWHIST
    if (ori->module_cfg_update & ISP3X_MODULE_RAWHIST0)
        ret = SplitRawHistLiteParams(&ori->meas.rawhist0, &left->meas.rawhist0, &right->meas.rawhist0);
    if (ori->module_cfg_update & ISP3X_MODULE_RAWHIST1)
        ret = SplitRawHistBigParams(&ori->meas.rawhist1, &left->meas.rawhist1, &right->meas.rawhist1);
    if (ori->module_cfg_update & ISP3X_MODULE_RAWHIST2)
        ret = SplitRawHistBigParams(&ori->meas.rawhist2, &left->meas.rawhist2, &right->meas.rawhist2);
    if (ori->module_cfg_update & ISP3X_MODULE_RAWHIST3)
        ret = SplitRawHistBigParams(&ori->meas.rawhist3, &left->meas.rawhist3, &right->meas.rawhist3);


    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAecParams<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori,
    struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //RAWAE
    if (ori->module_cfg_update & ISP32_MODULE_RAWAE0)
        ret = SplitRawAeLiteParams(&ori->meas.rawae0, &left->meas.rawae0, &right->meas.rawae0);
    if (ori->module_cfg_update & ISP32_MODULE_RAWAE1)
        ret = SplitRawAeBigParams(&ori->meas.rawae1, &left->meas.rawae1, &right->meas.rawae1);
    if (ori->module_cfg_update & ISP32_MODULE_RAWAE2)
        ret = SplitRawAeBigParams(&ori->meas.rawae2, &left->meas.rawae2, &right->meas.rawae2);
    if (ori->module_cfg_update & ISP32_MODULE_RAWAE3)
        ret = SplitRawAeBigParams(&ori->meas.rawae3, &left->meas.rawae3, &right->meas.rawae3);

    //RAWHIST
    if (ori->module_cfg_update & ISP32_MODULE_RAWHIST0)
        ret = SplitRawHistLiteParams(&ori->meas.rawhist0, &left->meas.rawhist0, &right->meas.rawhist0);
    if (ori->module_cfg_update & ISP32_MODULE_RAWHIST1)
        ret = SplitRawHistBigParams(&ori->meas.rawhist1, &left->meas.rawhist1, &right->meas.rawhist1);
    if (ori->module_cfg_update & ISP32_MODULE_RAWHIST2)
        ret = SplitRawHistBigParams(&ori->meas.rawhist2, &left->meas.rawhist2, &right->meas.rawhist2);
    if (ori->module_cfg_update & ISP32_MODULE_RAWHIST3)
        ret = SplitRawHistBigParams(&ori->meas.rawhist3, &left->meas.rawhist3, &right->meas.rawhist3);


    return XCAM_RETURN_NO_ERROR;
}

// split Awb param

template <>
XCamReturn IspParamsSplitter::SplitAwbParams<struct isp3x_isp_params_cfg>(
    struct isp3x_isp_params_cfg* ori,
    struct isp3x_isp_params_cfg* left,
    struct isp3x_isp_params_cfg* right) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    isp2x_window ori_win;
    isp2x_window left_win;
    isp2x_window right_win;
    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    u8 wnd_num = 15;

    ori_win.h_offs = ori->meas.rawawb.sw_rawawb_h_offs;
    ori_win.h_size = ori->meas.rawawb.sw_rawawb_h_size;
    ori_win.v_offs = ori->meas.rawawb.sw_rawawb_v_offs;
    ori_win.v_size = ori->meas.rawawb.sw_rawawb_v_size;

    memcpy(&left_win, &ori_win, sizeof(ori_win));
    memcpy(&right_win, &ori_win, sizeof(ori_win));

    // Awb measure window
    u8 awb_ds;
    if (ori->meas.rawawb.sw_rawawb_wind_size == 0) {
        awb_ds = 2;
    } else {
        awb_ds = 3;
    }
    u16 min_hsize = wnd_num << awb_ds;

    SplitAwbWin(&ori_win, &left_win, &right_win, awb_ds, wnd_num, left_isp_rect_, right_isp_rect_, &mode);
    if (ori_win.h_size < min_hsize) {
        ori->meas.rawawb.sw_rawawb_blk_measure_enable = 0;
        left->meas.rawawb.sw_rawawb_blk_measure_enable = 0;
        right->meas.rawawb.sw_rawawb_blk_measure_enable = 0;
    }
    else {
        if (mode == LEFT_AND_RIGHT_MODE) {
            if (left_win.h_size < min_hsize)
                left->meas.rawawb.sw_rawawb_blk_measure_enable = 0;
            if (right_win.h_size < min_hsize)
                right->meas.rawawb.sw_rawawb_blk_measure_enable = 0;
        }
    }

    // Awb blk_wei_w
    //SplitAwbWeight(&ori_win, &left_win, &right_win, ori->meas.rawawb.sw_rawawb_wp_blk_wei_w, left->meas.rawawb.sw_rawawb_wp_blk_wei_w, right->meas.rawawb.sw_rawawb_wp_blk_wei_w, mode, wnd_num);
    SplitAecWeight(ori->meas.rawawb.sw_rawawb_wp_blk_wei_w, left->meas.rawawb.sw_rawawb_wp_blk_wei_w, right->meas.rawawb.sw_rawawb_wp_blk_wei_w, mode, wnd_num);

    left->meas.rawawb.sw_rawawb_h_offs = left_win.h_offs;
    left->meas.rawawb.sw_rawawb_h_size = left_win.h_size;
    left->meas.rawawb.sw_rawawb_v_offs = left_win.v_offs;
    left->meas.rawawb.sw_rawawb_v_size = left_win.v_size;

    right->meas.rawawb.sw_rawawb_h_offs = right_win.h_offs;
    right->meas.rawawb.sw_rawawb_h_size = right_win.h_size;
    right->meas.rawawb.sw_rawawb_v_offs = right_win.v_offs;
    right->meas.rawawb.sw_rawawb_v_size = right_win.v_size;

    LOGD_AWB("Awb measure window  left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left_win.h_offs, left_win.v_offs,
             left_win.h_size, left_win.v_size, right_win.h_offs, right_win.v_offs,
             right_win.h_size, right_win.v_size);

    LOGV_AWB("Awb block weight: \n LEFT = { \n");

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            LOGV_AWB("%d ", left->meas.rawawb.sw_rawawb_wp_blk_wei_w[i * wnd_num + j]);
        LOGV_AWB("\n");
    }
    LOGV_AWB("} \n RIGHT = { \n");

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            LOGV_AWB("%d ", right->meas.rawawb.sw_rawawb_wp_blk_wei_w[i * wnd_num + j]);
        LOGV_AWB("\n");
    }
    LOGV_AWB("}  \n");

    // Awb Multi Window
    isp2x_window sub_ori_win;
    isp2x_window sub_left_win;
    isp2x_window sub_right_win;
    u16 sub_win_st = 0;
    u16 sub_win_ed = 0;
    u16 main_win_st = 0;
    u16 main_win_ed = 0;

    if (ori->meas.rawawb.sw_rawawb_multiwindow_en) {
        // Awb Multi window 0
        sub_ori_win.h_offs = ori->meas.rawawb.sw_rawawb_multiwindow0_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.sw_rawawb_multiwindow0_h_size - ori->meas.rawawb.sw_rawawb_multiwindow0_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.sw_rawawb_multiwindow0_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.sw_rawawb_multiwindow0_v_size - ori->meas.rawawb.sw_rawawb_multiwindow0_v_offs;

        sub_win_st = left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed = sub_win_st + sub_ori_win.h_size;
        main_win_st = left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs, sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_0 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, right_isp_rect_, &mode);
        left->meas.rawawb.sw_rawawb_multiwindow0_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.sw_rawawb_multiwindow0_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.sw_rawawb_multiwindow0_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.sw_rawawb_multiwindow0_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.sw_rawawb_multiwindow0_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.sw_rawawb_multiwindow0_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.sw_rawawb_multiwindow0_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.sw_rawawb_multiwindow0_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 0 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 1
        sub_ori_win.h_offs = ori->meas.rawawb.sw_rawawb_multiwindow1_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.sw_rawawb_multiwindow1_h_size - ori->meas.rawawb.sw_rawawb_multiwindow1_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.sw_rawawb_multiwindow1_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.sw_rawawb_multiwindow1_v_size - ori->meas.rawawb.sw_rawawb_multiwindow1_v_offs;

        sub_win_st = left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed = sub_win_st + sub_ori_win.h_size;
        main_win_st = left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs, sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_1 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, right_isp_rect_, &mode);
        left->meas.rawawb.sw_rawawb_multiwindow1_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.sw_rawawb_multiwindow1_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.sw_rawawb_multiwindow1_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.sw_rawawb_multiwindow1_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.sw_rawawb_multiwindow1_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.sw_rawawb_multiwindow1_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.sw_rawawb_multiwindow1_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.sw_rawawb_multiwindow1_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 1 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 2
        sub_ori_win.h_offs = ori->meas.rawawb.sw_rawawb_multiwindow2_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.sw_rawawb_multiwindow2_h_size - ori->meas.rawawb.sw_rawawb_multiwindow2_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.sw_rawawb_multiwindow2_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.sw_rawawb_multiwindow2_v_size - ori->meas.rawawb.sw_rawawb_multiwindow2_v_offs;

        sub_win_st = left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed = sub_win_st + sub_ori_win.h_size;
        main_win_st = left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs, sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_2 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, right_isp_rect_, &mode);
        left->meas.rawawb.sw_rawawb_multiwindow2_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.sw_rawawb_multiwindow2_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.sw_rawawb_multiwindow2_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.sw_rawawb_multiwindow2_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.sw_rawawb_multiwindow2_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.sw_rawawb_multiwindow2_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.sw_rawawb_multiwindow2_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.sw_rawawb_multiwindow2_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 2 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 3
        sub_ori_win.h_offs = ori->meas.rawawb.sw_rawawb_multiwindow3_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.sw_rawawb_multiwindow3_h_size - ori->meas.rawawb.sw_rawawb_multiwindow3_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.sw_rawawb_multiwindow3_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.sw_rawawb_multiwindow3_v_size - ori->meas.rawawb.sw_rawawb_multiwindow3_v_offs ;

        sub_win_st = left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed = sub_win_st + sub_ori_win.h_size;
        main_win_st = left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs, sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_3 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, right_isp_rect_, &mode);
        left->meas.rawawb.sw_rawawb_multiwindow3_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.sw_rawawb_multiwindow3_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.sw_rawawb_multiwindow3_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.sw_rawawb_multiwindow3_v_size = sub_left_win.v_size + sub_left_win.v_offs ;

        right->meas.rawawb.sw_rawawb_multiwindow3_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.sw_rawawb_multiwindow3_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.sw_rawawb_multiwindow3_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.sw_rawawb_multiwindow3_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 3 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);
    }

    return ret;

}

template <>
XCamReturn IspParamsSplitter::SplitAwbParams<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori,
    struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    isp2x_window ori_win;
    isp2x_window left_win;
    isp2x_window right_win;
    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    u8 wnd_num = 15;

    ori_win.h_offs = ori->meas.rawawb.h_offs;
    ori_win.h_size = ori->meas.rawawb.h_size;
    ori_win.v_offs = ori->meas.rawawb.v_offs;
    ori_win.v_size = ori->meas.rawawb.v_size;

    memcpy(&left_win, &ori_win, sizeof(ori_win));
    memcpy(&right_win, &ori_win, sizeof(ori_win));

    // Awb measure window
    u8 awb_ds;
    if (ori->meas.rawawb.wind_size == 0) {
        awb_ds = 2;
    } else {
        awb_ds = 3;
    }
    if (ori->meas.rawawb.ds16x8_mode_en) {
        awb_ds = 4;
    }
    u16 min_hsize = wnd_num << awb_ds;

    SplitAwbWin(&ori_win, &left_win, &right_win, awb_ds, wnd_num, left_isp_rect_, right_isp_rect_, &mode);
    if (ori_win.h_size < min_hsize) {
        ori->meas.rawawb.blk_measure_enable = 0;
        left->meas.rawawb.blk_measure_enable = 0;
        right->meas.rawawb.blk_measure_enable = 0;
    }
    else {
        if (mode == LEFT_AND_RIGHT_MODE) {
            if (left_win.h_size < min_hsize)
                left->meas.rawawb.blk_measure_enable = 0;
            if (right_win.h_size < min_hsize)
                right->meas.rawawb.blk_measure_enable = 0;
        }
    }

    // Awb blk_wei_w
    //SplitAwbWeight(&ori_win, &left_win, &right_win, ori->meas.rawawb.wp_blk_wei_w, left->meas.rawawb.wp_blk_wei_w, right->meas.rawawb.wp_blk_wei_w, mode, wnd_num);
    SplitAecWeight(ori->meas.rawawb.wp_blk_wei_w, left->meas.rawawb.wp_blk_wei_w, right->meas.rawawb.wp_blk_wei_w, mode, wnd_num);

    left->meas.rawawb.h_offs = left_win.h_offs;
    left->meas.rawawb.h_size = left_win.h_size;
    left->meas.rawawb.v_offs = left_win.v_offs;
    left->meas.rawawb.v_size = left_win.v_size;

    right->meas.rawawb.h_offs = right_win.h_offs;
    right->meas.rawawb.h_size = right_win.h_size;
    right->meas.rawawb.v_offs = right_win.v_offs;
    right->meas.rawawb.v_size = right_win.v_size;

    LOGD_AWB("Awb measure window  left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left_win.h_offs, left_win.v_offs,
             left_win.h_size, left_win.v_size, right_win.h_offs, right_win.v_offs,
             right_win.h_size, right_win.v_size);

    LOGV_AWB("Awb block weight: \n LEFT = { \n");

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            LOGV_AWB("%d ", left->meas.rawawb.wp_blk_wei_w[i * wnd_num + j]);
        LOGV_AWB("\n");
    }
    LOGV_AWB("} \n RIGHT = { \n");

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            LOGV_AWB("%d ", right->meas.rawawb.wp_blk_wei_w[i * wnd_num + j]);
        LOGV_AWB("\n");
    }
    LOGV_AWB("}  \n");

    // Awb Multi Window
    isp2x_window sub_ori_win;
    isp2x_window sub_left_win;
    isp2x_window sub_right_win;
    u16 sub_win_st = 0;
    u16 sub_win_ed = 0;
    u16 main_win_st = 0;
    u16 main_win_ed = 0;

    if (ori->meas.rawawb.multiwindow_en) {
        // Awb Multi window 0
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow0_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.multiwindow0_h_size - ori->meas.rawawb.multiwindow0_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow0_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.multiwindow0_v_size - ori->meas.rawawb.multiwindow0_v_offs;

        sub_win_st = left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed = sub_win_st + sub_ori_win.h_size;
        main_win_st = left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs, sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_0 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, right_isp_rect_, &mode);
        left->meas.rawawb.multiwindow0_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow0_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow0_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow0_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow0_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow0_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow0_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow0_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 0 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 1
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow1_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.multiwindow1_h_size - ori->meas.rawawb.multiwindow1_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow1_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.multiwindow1_v_size - ori->meas.rawawb.multiwindow1_v_offs;

        sub_win_st = left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed = sub_win_st + sub_ori_win.h_size;
        main_win_st = left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs, sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_1 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, right_isp_rect_, &mode);
        left->meas.rawawb.multiwindow1_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow1_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow1_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow1_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow1_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow1_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow1_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow1_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 1 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 2
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow2_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.multiwindow2_h_size - ori->meas.rawawb.multiwindow2_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow2_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.multiwindow2_v_size - ori->meas.rawawb.multiwindow2_v_offs;

        sub_win_st = left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed = sub_win_st + sub_ori_win.h_size;
        main_win_st = left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs, sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_2 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, right_isp_rect_, &mode);
        left->meas.rawawb.multiwindow2_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow2_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow2_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow2_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow2_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow2_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow2_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow2_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 2 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 3
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow3_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.multiwindow3_h_size - ori->meas.rawawb.multiwindow3_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow3_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.multiwindow3_v_size - ori->meas.rawawb.multiwindow3_v_offs ;

        sub_win_st = left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed = sub_win_st + sub_ori_win.h_size;
        main_win_st = left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs, sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_3 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, right_isp_rect_, &mode);
        left->meas.rawawb.multiwindow3_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow3_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow3_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow3_v_size = sub_left_win.v_size + sub_left_win.v_offs ;

        right->meas.rawawb.multiwindow3_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow3_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow3_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow3_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 3 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);
    }

    return ret;

}

template <>
XCamReturn IspParamsSplitter::SplitAfParams<struct isp3x_isp_params_cfg>(
    struct isp3x_isp_params_cfg* ori,
    struct isp3x_isp_params_cfg* left,
    struct isp3x_isp_params_cfg* right) {
    struct isp3x_rawaf_meas_cfg org_af = left->meas.rawaf;
    struct isp3x_rawaf_meas_cfg* l_af = &left->meas.rawaf;
    struct isp3x_rawaf_meas_cfg* r_af = &right->meas.rawaf;
    struct isp2x_rawaebig_meas_cfg* l_ae3 = &left->meas.rawae3;
    struct isp2x_rawaebig_meas_cfg* r_ae3 = &right->meas.rawae3;
    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t x_st, x_ed, l_blknum, r_blknum, ov_w, blk_w, r_skip_blknum;

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
    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
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
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;
            }
        }
        // af win < one isp width * 1.5
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
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;
            }
        } else {
            l_win_st = x_st;
            l_win_ed = l_isp_ed - 2;
            blk_w = (l_win_ed - l_win_st) / ISP2X_RAWAF_SUMDATA_ROW;
            l_win_st = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            r_win_st = 2;
            r_win_ed = r_win_st + blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum = ISP2X_RAWAF_SUMDATA_ROW;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW;
            r_skip_blknum = 0;
        }
        LOGD_AF("wina: blk_w %d, ov_w %d, l_blknum %d, r_blknum %d, r_skip_blknum %d",
                blk_w, ov_w, l_blknum, r_blknum, r_skip_blknum);
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        l_blknum = 0;
        r_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        r_win_st = x_st - right_isp_rect_.x;
        r_win_ed = x_ed - right_isp_rect_.x;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        r_blknum = 0;
        l_win_st = x_st;
        l_win_ed = x_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    l_af->win[0].h_offs = l_win_st;
    l_af->win[0].h_size = l_win_ed - l_win_st;
    r_af->win[0].h_offs = r_win_st;
    r_af->win[0].h_size = r_win_ed - r_win_st;

    //// winB ////
    x_st = org_af.win[1].h_offs;
    x_ed = x_st + org_af.win[1].h_size;
    LOGD_AF("winb.x_st %d, winb.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            x_st, x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        l_win_st = x_st;
        l_win_ed = l_isp_ed - 2;
        r_win_st = ov_w - 2;
        r_win_ed = x_ed - right_isp_rect_.x;
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        r_win_st = x_st - right_isp_rect_.x;
        r_win_ed = x_ed - right_isp_rect_.x;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_win_st = x_st;
        l_win_ed = x_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    l_af->win[1].h_offs = l_win_st;
    l_af->win[1].h_size = l_win_ed - l_win_st;
    r_af->win[1].h_offs = r_win_st;
    r_af->win[1].h_size = r_win_ed - r_win_st;

    // rawae3 is used by af now!!!
    if (org_af.ae_mode) {
        l_ae3->win.h_offs = l_af->win[0].h_offs;
        l_ae3->win.v_offs = l_af->win[0].v_offs;
        l_ae3->win.h_size = l_af->win[0].h_size;
        l_ae3->win.v_size = l_af->win[0].v_size;
        r_ae3->win.h_offs = r_af->win[0].h_offs;
        r_ae3->win.v_offs = r_af->win[0].v_offs;
        r_ae3->win.h_size = r_af->win[0].h_size;
        r_ae3->win.v_size = r_af->win[0].v_size;
    }

    LOGD_AF("AfWinA left=%d-%d-%d-%d, right=%d-%d-%d-%d",
            l_af->win[0].h_offs, l_af->win[0].v_offs,
            l_af->win[0].h_size, l_af->win[0].v_size,
            r_af->win[0].h_offs, r_af->win[0].v_offs,
            r_af->win[0].h_size, r_af->win[0].v_size);

    LOGD_AF("AfWinB left=%d-%d-%d-%d, right=%d-%d-%d-%d",
            l_af->win[1].h_offs, l_af->win[1].v_offs,
            l_af->win[1].h_size, l_af->win[1].v_size,
            r_af->win[1].h_offs, r_af->win[1].v_offs,
            r_af->win[1].h_size, r_af->win[1].v_size);

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAfParams<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori,
    struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {
    struct isp32_rawaf_meas_cfg org_af = left->meas.rawaf;
    struct isp32_rawaf_meas_cfg* l_af = &left->meas.rawaf;
    struct isp32_rawaf_meas_cfg* r_af = &right->meas.rawaf;
#if defined(ISP_HW_V32)
    struct isp2x_rawaebig_meas_cfg org_ae3 = left->meas.rawae3;
    struct isp2x_rawaebig_meas_cfg* l_ae3 = &left->meas.rawae3;
    struct isp2x_rawaebig_meas_cfg* r_ae3 = &right->meas.rawae3;
    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t x_st, x_ed, l_blknum, r_blknum, ov_w, blk_w, r_skip_blknum;

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
    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
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
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;
            }
        }
        // af win < one isp width * 1.5
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
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;
            }
        } else {
            l_win_st = x_st;
            l_win_ed = l_isp_ed - 2;
            blk_w = (l_win_ed - l_win_st) / ISP2X_RAWAF_SUMDATA_ROW;
            l_win_st = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            r_win_st = 2;
            r_win_ed = r_win_st + blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum = ISP2X_RAWAF_SUMDATA_ROW;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW;
            r_skip_blknum = 0;
        }
        LOGD_AF("wina: blk_w %d, ov_w %d, l_blknum %d, r_blknum %d, r_skip_blknum %d",
                blk_w, ov_w, l_blknum, r_blknum, r_skip_blknum);
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        l_blknum = 0;
        r_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        r_win_st = x_st - right_isp_rect_.x;
        r_win_ed = x_ed - right_isp_rect_.x;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        r_blknum = 0;
        l_win_st = x_st;
        l_win_ed = x_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    l_af->win[0].h_offs = l_win_st;
    l_af->win[0].h_size = l_win_ed - l_win_st;
    r_af->win[0].h_offs = r_win_st;
    r_af->win[0].h_size = r_win_ed - r_win_st;

    //// winB ////
    x_st = org_af.win[1].h_offs;
    x_ed = x_st + org_af.win[1].h_size;
    LOGD_AF("winb.x_st %d, winb.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            x_st, x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        l_win_st = x_st;
        l_win_ed = l_isp_ed - 2;
        r_win_st = ov_w - 2;
        r_win_ed = x_ed - right_isp_rect_.x;
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        r_win_st = x_st - right_isp_rect_.x;
        r_win_ed = x_ed - right_isp_rect_.x;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_win_st = x_st;
        l_win_ed = x_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    l_af->win[1].h_offs = l_win_st;
    l_af->win[1].h_size = l_win_ed - l_win_st;
    r_af->win[1].h_offs = r_win_st;
    r_af->win[1].h_size = r_win_ed - r_win_st;

    // rawae3 is used by af now!!!
    if (org_af.ae_mode) {
        l_ae3->win.h_offs = l_af->win[0].h_offs;
        l_ae3->win.v_offs = l_af->win[0].v_offs;
        l_ae3->win.h_size = l_af->win[0].h_size;
        l_ae3->win.v_size = l_af->win[0].v_size;
        r_ae3->win.h_offs = r_af->win[0].h_offs;
        r_ae3->win.v_offs = r_af->win[0].v_offs;
        r_ae3->win.h_size = r_af->win[0].h_size;
        r_ae3->win.v_size = r_af->win[0].v_size;
    }
#elif defined(ISP_HW_V32_LITE)
    struct isp2x_rawaelite_meas_cfg org_ae0 = left->meas.rawae0;
    struct isp2x_rawaelite_meas_cfg* l_ae0 = &left->meas.rawae0;
    struct isp2x_rawaelite_meas_cfg* r_ae0 = &right->meas.rawae0;
    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t x_st, x_ed, l_blknum, r_blknum, ov_w, blk_w, r_skip_blknum;
    uint8_t wnd_num;

    wnd_num = sqrt(ISP32L_RAWAF_WND_DATA);

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
    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        // af win < one isp width
        if (org_af.win[0].h_size < left_isp_rect_.w) {
            blk_w = org_af.win[0].h_size / wnd_num;
            l_blknum = (l_isp_ed - x_st + blk_w - 1) / blk_w;
            r_blknum = wnd_num - l_blknum;
            l_win_ed = l_isp_ed - 2;
            l_win_st = l_win_ed - blk_w * wnd_num;
            if (blk_w < ov_w) {
                r_skip_blknum = ov_w / blk_w;
                r_win_st = ov_w - r_skip_blknum * blk_w;
                r_win_ed = ov_w + (wnd_num - r_skip_blknum) * blk_w;
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + wnd_num * blk_w;
            }
        }
        // af win < one isp width * 1.5
        else if (org_af.win[0].h_size < left_isp_rect_.w * 3 / 2) {
            l_win_st = x_st;
            l_win_ed = l_isp_ed - 2;
            blk_w = (l_win_ed - l_win_st) / (wnd_num + 1);
            l_win_st = l_win_ed - blk_w * wnd_num;
            l_blknum = ((l_win_ed - l_win_st) * wnd_num + org_af.win[0].h_size - 1) / org_af.win[0].h_size;
            r_blknum = wnd_num - l_blknum;
            if (blk_w < ov_w) {
                r_skip_blknum = ov_w / blk_w;
                r_win_st = ov_w - r_skip_blknum * blk_w;
                r_win_ed = ov_w + (wnd_num - r_skip_blknum) * blk_w;
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + wnd_num * blk_w;
            }
        } else {
            l_win_st = x_st;
            l_win_ed = l_isp_ed - 2;
            blk_w = (l_win_ed - l_win_st) / wnd_num;
            l_win_st = l_win_ed - blk_w * wnd_num;
            r_win_st = 2;
            r_win_ed = r_win_st + blk_w * wnd_num;
            l_blknum = wnd_num;
            r_blknum = wnd_num;
            r_skip_blknum = 0;
        }
        LOGD_AF("wina: blk_w %d, ov_w %d, l_blknum %d, r_blknum %d, r_skip_blknum %d",
                blk_w, ov_w, l_blknum, r_blknum, r_skip_blknum);
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        l_blknum = 0;
        r_blknum = wnd_num;
        r_win_st = x_st - right_isp_rect_.x;
        r_win_ed = x_ed - right_isp_rect_.x;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_blknum = wnd_num;
        r_blknum = 0;
        l_win_st = x_st;
        l_win_ed = x_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    l_af->win[0].h_offs = l_win_st;
    l_af->win[0].h_size = l_win_ed - l_win_st;
    r_af->win[0].h_offs = r_win_st;
    r_af->win[0].h_size = r_win_ed - r_win_st;

    //// winB ////
    x_st = org_af.win[1].h_offs;
    x_ed = x_st + org_af.win[1].h_size;
    LOGD_AF("winb.x_st %d, winb.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            x_st, x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        l_win_st = x_st;
        l_win_ed = l_isp_ed - 2;
        r_win_st = ov_w - 2;
        r_win_ed = x_ed - right_isp_rect_.x;
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        r_win_st = x_st - right_isp_rect_.x;
        r_win_ed = x_ed - right_isp_rect_.x;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_win_st = x_st;
        l_win_ed = x_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    l_af->win[1].h_offs = l_win_st;
    l_af->win[1].h_size = l_win_ed - l_win_st;
    r_af->win[1].h_offs = r_win_st;
    r_af->win[1].h_size = r_win_ed - r_win_st;

    // rawae3 is used by af now!!!
    if (org_af.ae_mode) {
        l_ae0->win.h_offs = l_af->win[0].h_offs;
        l_ae0->win.v_offs = l_af->win[0].v_offs;
        l_ae0->win.h_size = l_af->win[0].h_size;
        l_ae0->win.v_size = l_af->win[0].v_size;
        r_ae0->win.h_offs = r_af->win[0].h_offs;
        r_ae0->win.v_offs = r_af->win[0].v_offs;
        r_ae0->win.h_size = r_af->win[0].h_size;
        r_ae0->win.v_size = r_af->win[0].v_size;
    }
#endif
    LOGD_AF("AfWinA left=%d-%d-%d-%d, right=%d-%d-%d-%d",
            l_af->win[0].h_offs, l_af->win[0].v_offs,
            l_af->win[0].h_size, l_af->win[0].v_size,
            r_af->win[0].h_offs, r_af->win[0].v_offs,
            r_af->win[0].h_size, r_af->win[0].v_size);

    LOGD_AF("AfWinB left=%d-%d-%d-%d, right=%d-%d-%d-%d",
            l_af->win[1].h_offs, l_af->win[1].v_offs,
            l_af->win[1].h_size, l_af->win[1].v_size,
            r_af->win[1].h_offs, r_af->win[1].v_offs,
            r_af->win[1].h_size, r_af->win[1].v_size);

    return XCAM_RETURN_NO_ERROR;
}

int AlscMatrixScale(unsigned short ori_matrix[], unsigned short left_matrix[],
                    unsigned short right_matrix[], int cols, int rows) {
    int ori_col_index = 0;
    int lef_dst_index = 0;
    int rht_dst_index = 0;
    int mid_col = cols / 2;
    int row_index = 0;

    for (row_index = 0; row_index < rows; row_index++) {
        for (ori_col_index = 0; ori_col_index < cols; ori_col_index++) {
            if (ori_col_index < mid_col) {
                left_matrix[lef_dst_index++] =
                    ori_matrix[row_index * cols + ori_col_index];
                left_matrix[lef_dst_index++] =
                    (ori_matrix[row_index * cols + ori_col_index] +
                     ori_matrix[row_index * cols + ori_col_index + 1]) / 2;
            } else if (ori_col_index == mid_col) {
                left_matrix[lef_dst_index++] =
                    ori_matrix[row_index * cols + ori_col_index];
                right_matrix[rht_dst_index++] =
                    ori_matrix[row_index * cols + ori_col_index];
            } else {
                right_matrix[rht_dst_index++] =
                    (ori_matrix[row_index * cols + ori_col_index] +
                     ori_matrix[row_index * cols + ori_col_index - 1]) / 2;
                right_matrix[rht_dst_index++] =
                    ori_matrix[row_index * cols + ori_col_index];
            }
        }
    }

    return 0;
}

int AlscMatrixSplit(const unsigned short* ori_matrix, int cols, int rows, unsigned short left[],
                    unsigned short right[]) {
    int out_cols                = cols / 2 + cols % 2;
    int out_rows                = rows;
    int left_start_addr         = 0;
    int right_start_addr        = cols - out_cols;
    unsigned short* left_index  = left;
    unsigned short* right_index = right;

    while (out_rows--) {
        memcpy(left_index, ori_matrix + left_start_addr, out_cols * sizeof(unsigned short));
        memcpy(right_index, ori_matrix + right_start_addr, out_cols * sizeof(unsigned short));
        left_index += out_cols;
        right_index += out_cols;
        left_start_addr += cols;
        right_start_addr += cols;
    }

    return 0;
}

int SplitAlscXtable(const unsigned short* in_array, int in_size, int ori_imgw,
                    unsigned short* dst_left, unsigned short* dst_right,
                    int left_w, int right_w) {
    int in_index = 0;
    int left_index = 0;
    int right_index = 0;
    for (in_index = 0; in_index < in_size; in_index++) {
        if (in_index < (in_size / 2)) {
            dst_left[left_index++] =
                ceil(in_array[in_index] * 1.0 / ori_imgw * left_w);
            dst_left[left_index++] =
                floor(in_array[in_index] * 1.0 / ori_imgw * left_w);
        } else {
            dst_right[right_index++] =
                ceil(in_array[in_index] * 1.0 / ori_imgw * right_w);
            dst_right[right_index++] =
                floor(in_array[in_index] * 1.0 / ori_imgw * right_w);
        }
    }

    return 0;
}

int lscGradUpdate(unsigned short xgrad_tbl[],
                  unsigned short ygrad_tbl[],
                  unsigned short x_sect_tbl[],
                  unsigned short y_sect_tbl[],
                  int x_sect_size,
                  int y_sect_size) {
    uint32_t x_size = x_sect_size;
    uint32_t y_size = y_sect_size;

    for (uint32_t i = 0; i < x_size; i++) {
        if (0 < x_sect_tbl[i]) {
            xgrad_tbl[i] = (uint16_t)((double)(1UL << 15) / x_sect_tbl[i] + 0.5);
        } else {
            return XCAM_RETURN_ERROR_PARAM;
        }
    }
    for (uint32_t i = 0; i < y_size; i++) {
        if (0 < y_sect_tbl[i]) {
            ygrad_tbl[i] = (uint16_t)((double)(1UL << 15) / y_sect_tbl[i] + 0.5);
        } else {
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAlscParams<struct isp3x_isp_params_cfg>(
    struct isp3x_isp_params_cfg* ori, struct isp3x_isp_params_cfg* left,
    struct isp3x_isp_params_cfg* right) {
    struct isp3x_lsc_cfg* lsc_cfg_ori = &ori->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_lef = &left->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_rht = &right->others.lsc_cfg;

    memcpy(lsc_cfg_lef->y_size_tbl, lsc_cfg_ori->y_size_tbl,
           sizeof(lsc_cfg_ori->y_size_tbl));
    memcpy(lsc_cfg_rht->y_size_tbl, lsc_cfg_ori->y_size_tbl,
           sizeof(lsc_cfg_ori->y_size_tbl));

    SplitAlscXtable(lsc_cfg_ori->x_size_tbl, ISP3X_LSC_SIZE_TBL_SIZE,
                    pic_rect_.w,
                    lsc_cfg_lef->x_size_tbl,
                    lsc_cfg_rht->x_size_tbl,
                    left_isp_rect_.w,
                    right_isp_rect_.w);

    AlscMatrixScale(lsc_cfg_ori->r_data_tbl,
                    lsc_cfg_lef->r_data_tbl,
                    lsc_cfg_rht->r_data_tbl,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScale(lsc_cfg_ori->gr_data_tbl,
                    lsc_cfg_lef->gr_data_tbl,
                    lsc_cfg_rht->gr_data_tbl,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScale(lsc_cfg_ori->gb_data_tbl,
                    lsc_cfg_lef->gb_data_tbl,
                    lsc_cfg_rht->gb_data_tbl,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScale(lsc_cfg_ori->b_data_tbl,
                    lsc_cfg_lef->b_data_tbl,
                    lsc_cfg_rht->b_data_tbl,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1);

    lscGradUpdate(lsc_cfg_lef->x_grad_tbl, lsc_cfg_lef->y_grad_tbl,
                  lsc_cfg_lef->x_size_tbl, lsc_cfg_lef->y_size_tbl,
                  ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    lscGradUpdate(lsc_cfg_rht->x_grad_tbl, lsc_cfg_rht->y_grad_tbl,
                  lsc_cfg_rht->x_size_tbl, lsc_cfg_rht->y_size_tbl,
                  ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAlscParams<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori, struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {
    struct isp3x_lsc_cfg* lsc_cfg_ori = &ori->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_lef = &left->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_rht = &right->others.lsc_cfg;

    memcpy(lsc_cfg_lef->y_size_tbl, lsc_cfg_ori->y_size_tbl,
           sizeof(lsc_cfg_ori->y_size_tbl));
    memcpy(lsc_cfg_rht->y_size_tbl, lsc_cfg_ori->y_size_tbl,
           sizeof(lsc_cfg_ori->y_size_tbl));

    SplitAlscXtable(lsc_cfg_ori->x_size_tbl, ISP3X_LSC_SIZE_TBL_SIZE,
                    pic_rect_.w,
                    lsc_cfg_lef->x_size_tbl,
                    lsc_cfg_rht->x_size_tbl,
                    left_isp_rect_.w,
                    right_isp_rect_.w);

    AlscMatrixScale(lsc_cfg_ori->r_data_tbl,
                    lsc_cfg_lef->r_data_tbl,
                    lsc_cfg_rht->r_data_tbl,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScale(lsc_cfg_ori->gr_data_tbl,
                    lsc_cfg_lef->gr_data_tbl,
                    lsc_cfg_rht->gr_data_tbl,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScale(lsc_cfg_ori->gb_data_tbl,
                    lsc_cfg_lef->gb_data_tbl,
                    lsc_cfg_rht->gb_data_tbl,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScale(lsc_cfg_ori->b_data_tbl,
                    lsc_cfg_lef->b_data_tbl,
                    lsc_cfg_rht->b_data_tbl,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1,
                    ISP3X_LSC_SIZE_TBL_SIZE + 1);

    lscGradUpdate(lsc_cfg_lef->x_grad_tbl, lsc_cfg_lef->y_grad_tbl,
                  lsc_cfg_lef->x_size_tbl, lsc_cfg_lef->y_size_tbl,
                  ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    lscGradUpdate(lsc_cfg_rht->x_grad_tbl, lsc_cfg_rht->y_grad_tbl,
                  lsc_cfg_rht->x_size_tbl, lsc_cfg_rht->y_size_tbl,
                  ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAynrParams<struct isp3x_isp_params_cfg>(
    struct isp3x_isp_params_cfg* ori, struct isp3x_isp_params_cfg* left,
    struct isp3x_isp_params_cfg* right) {
    struct isp3x_ynr_cfg* ynr_cfg_ori = &ori->others.ynr_cfg;
    struct isp3x_ynr_cfg* ynr_cfg_lef = &left->others.ynr_cfg;
    struct isp3x_ynr_cfg* ynr_cfg_rht = &right->others.ynr_cfg;

    //left half pic center pixel calculate
    ynr_cfg_lef->rnr_center_coorh = pic_rect_.w / 2;
    ynr_cfg_lef->rnr_center_coorv = left_isp_rect_.h / 2;

    //right half pic center pixel calculate
    ynr_cfg_rht->rnr_center_coorh = (right_isp_rect_.w - pic_rect_.w / 2);
    ynr_cfg_rht->rnr_center_coorv = right_isp_rect_.h / 2;

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAynrParams<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori, struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {
    struct isp32_ynr_cfg* ynr_cfg_ori = &ori->others.ynr_cfg;
    struct isp32_ynr_cfg* ynr_cfg_lef = &left->others.ynr_cfg;
    struct isp32_ynr_cfg* ynr_cfg_rht = &right->others.ynr_cfg;

    //left half pic center pixel calculate
    ynr_cfg_lef->rnr_center_coorh = pic_rect_.w / 2;
    ynr_cfg_lef->rnr_center_coorv = left_isp_rect_.h / 2;

    //right half pic center pixel calculate
    ynr_cfg_rht->rnr_center_coorh = (right_isp_rect_.w - pic_rect_.w / 2);
    ynr_cfg_rht->rnr_center_coorv = right_isp_rect_.h / 2;

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAsharpParams<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori, struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {
    struct isp32_sharp_cfg* sharp_cfg_ori = &ori->others.sharp_cfg;
    struct isp32_sharp_cfg* sharp_cfg_lef = &left->others.sharp_cfg;
    struct isp32_sharp_cfg* sharp_cfg_rht = &right->others.sharp_cfg;

    //left half pic center pixel calculate
    sharp_cfg_lef->center_mode = 1;
    sharp_cfg_lef->center_wid = pic_rect_.w / 2;
    sharp_cfg_lef->center_het = left_isp_rect_.h / 2;

    //right half pic center pixel calculate
    sharp_cfg_rht->center_mode = 1;
    sharp_cfg_rht->center_wid = (right_isp_rect_.w - pic_rect_.w / 2);
    sharp_cfg_rht->center_het = right_isp_rect_.h / 2;

    return XCAM_RETURN_NO_ERROR;
}

IspParamsSplitter& IspParamsSplitter::SetPicInfo(IspParamsSplitter::Rectangle&& pic_rect) {
    pic_rect_ = std::move(pic_rect);
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetPicInfo(IspParamsSplitter::Rectangle& pic_rect) {
    pic_rect_ = pic_rect;
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetLeftIspRect(IspParamsSplitter::Rectangle&& left_isp_rect) {
    left_isp_rect_ = std::move(left_isp_rect);
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetLeftIspRect(IspParamsSplitter::Rectangle& left_isp_rect) {
    left_isp_rect_ = left_isp_rect;
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetRightIspRect(
    IspParamsSplitter::Rectangle&& right_isp_rect) {
    right_isp_rect_ = std::move(right_isp_rect);
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetRightIspRect(
    IspParamsSplitter::Rectangle& right_isp_rect) {
    right_isp_rect_ = right_isp_rect;
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetBottomLeftIspRect(IspParamsSplitter::Rectangle&& bottom_left_isp_rect) {
    bottom_left_isp_rect_ = std::move(bottom_left_isp_rect);
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetBottomLeftIspRect(IspParamsSplitter::Rectangle& bottom_left_isp_rect) {
    bottom_left_isp_rect_ = bottom_left_isp_rect;
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetBottomRightIspRect(
    IspParamsSplitter::Rectangle& bottom_right_isp_rect) {
    bottom_right_isp_rect_ = bottom_right_isp_rect;
    return *this;
}

IspParamsSplitter& IspParamsSplitter::SetBottomRightIspRect(
    IspParamsSplitter::Rectangle&& bottom_right_isp_rect) {
    bottom_right_isp_rect_ = std::move(bottom_right_isp_rect);
    return *this;
}

const IspParamsSplitter::Rectangle& IspParamsSplitter::GetPicInfo() const {
    return pic_rect_;
}

const IspParamsSplitter::Rectangle& IspParamsSplitter::GetLeftIspRect() const {
    return left_isp_rect_;
}

const IspParamsSplitter::Rectangle& IspParamsSplitter::GetRightIspRect() const {
    return right_isp_rect_;
}

template <>
XCamReturn IspParamsSplitter::SplitIspParams<struct isp3x_isp_params_cfg>(
    struct isp3x_isp_params_cfg* orig_isp_params,
    struct isp3x_isp_params_cfg* isp_params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct isp3x_isp_params_cfg* left_isp_params  = isp_params;
    struct isp3x_isp_params_cfg* right_isp_params = isp_params + 1;

    struct isp3x_cac_cfg cac_right;
    struct isp2x_ldch_cfg ldch_right;
    memcpy(&cac_right, &right_isp_params->others.cac_cfg, sizeof(struct isp3x_cac_cfg));
    memcpy(&ldch_right, &right_isp_params->others.ldch_cfg, sizeof(struct isp2x_ldch_cfg));

    // Modules that use the same params for both left and right isp
    // will not need to implent split function
    memcpy(right_isp_params, left_isp_params, sizeof(struct isp3x_isp_params_cfg));
    memcpy(&right_isp_params->others.ldch_cfg, &ldch_right, sizeof(struct isp2x_ldch_cfg));

    memcpy(&right_isp_params->others.cac_cfg, &cac_right, sizeof(struct isp3x_cac_cfg));

    ret = SplitAecParams(orig_isp_params, left_isp_params, right_isp_params);
    // Should return failure ?
    if (orig_isp_params->module_cfg_update & ISP3X_MODULE_RAWAWB)
        ret = SplitAwbParams(orig_isp_params, left_isp_params, right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP3X_MODULE_RAWAF)
        ret = SplitAfParams(orig_isp_params, left_isp_params, right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP3X_MODULE_LSC)
        ret = SplitAlscParams(orig_isp_params, left_isp_params, right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP3X_MODULE_SHARP)
        ret = SplitAynrParams(orig_isp_params, left_isp_params, right_isp_params);
    LOGD_CAMHW("Split ISP Params: left %p right %p size %d",
               left_isp_params,
               right_isp_params,
               sizeof(*left_isp_params));

    return ret;
}

template <>
XCamReturn IspParamsSplitter::SplitIspParams<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* orig_isp_params,
    struct isp32_isp_params_cfg* isp_params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct isp32_isp_params_cfg* left_isp_params  = isp_params;
    struct isp32_isp_params_cfg* right_isp_params = isp_params + 1;

    struct isp32_cac_cfg cac_right;
    struct isp32_ldch_cfg ldch_right;
    memcpy(&cac_right, &right_isp_params->others.cac_cfg, sizeof(struct isp32_cac_cfg));
    memcpy(&ldch_right, &right_isp_params->others.ldch_cfg, sizeof(struct isp32_ldch_cfg));

    // Modules that use the same params for both left and right isp
    // will not need to implent split function
    memcpy(right_isp_params, left_isp_params, sizeof(struct isp32_isp_params_cfg));

    memcpy(&right_isp_params->others.cac_cfg, &cac_right, sizeof(struct isp32_cac_cfg));
    memcpy(&right_isp_params->others.ldch_cfg, &ldch_right, sizeof(struct isp32_ldch_cfg));

    ret = SplitAecParams(orig_isp_params, left_isp_params, right_isp_params);
    // Should return failure ?
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_RAWAWB)
        ret = SplitAwbParams(orig_isp_params, left_isp_params, right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_RAWAF)
        ret = SplitAfParams(orig_isp_params, left_isp_params, right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_LSC)
        ret = SplitAlscParams(orig_isp_params, left_isp_params, right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_YNR)
        ret = SplitAynrParams(orig_isp_params, left_isp_params, right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_SHARP)
        ret = SplitAsharpParams(orig_isp_params, left_isp_params, right_isp_params);
    LOGD_CAMHW("Split ISP Params: left %p right %p size %d",
               left_isp_params,
               right_isp_params,
               sizeof(*left_isp_params));

    return ret;
}

template <>
XCamReturn IspParamsSplitter::SplitRawAeLiteParamsVertical<struct isp2x_rawaelite_meas_cfg>(
    struct isp2x_rawaelite_meas_cfg* ori, struct isp2x_rawaelite_meas_cfg* left,
    struct isp2x_rawaelite_meas_cfg* right) {

    u8 wnd_num = 0;
    if (ori->wnd_num == 0)
        wnd_num = 1;
    else
        wnd_num = 5;

    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    WinSplitMode sub_mode[4] = {LEFT_AND_RIGHT_MODE};

    SplitAecWinVertical(&ori->win, &left->win, &right->win, wnd_num, left_isp_rect_, bottom_left_isp_rect_, &mode);

#ifdef DEBUG
    printf("AeLite left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->win.h_offs, left->win.v_offs,
           left->win.h_size, left->win.v_size, right->win.h_offs, right->win.v_offs,
           right->win.h_size, right->win.v_size);
#endif

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitRawAeBigParamsVertical<struct isp2x_rawaebig_meas_cfg>(
    struct isp2x_rawaebig_meas_cfg* ori, struct isp2x_rawaebig_meas_cfg* left,
    struct isp2x_rawaebig_meas_cfg* right) {

    u8 wnd_num = 0;

    if (ori->wnd_num == 0)
        wnd_num = 1;
    else if (ori->wnd_num == 1)
        wnd_num = 5;
    else
        wnd_num = 15;

    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    WinSplitMode sub_mode[4] = {LEFT_AND_RIGHT_MODE};

    SplitAecWinVertical(&ori->win, &left->win, &right->win, wnd_num, left_isp_rect_, bottom_left_isp_rect_, &mode);
    SplitAecSubWinVertical(ori->subwin_en, ori->subwin, left->subwin, right->subwin, left_isp_rect_, bottom_left_isp_rect_, sub_mode);

    for (int i = 0; i < ISP2X_RAWAEBIG_SUBWIN_NUM; i++) {
        if (ori->subwin_en[i]) {
            switch (sub_mode[i]) {
            case LEFT_AND_RIGHT_MODE:
                left->subwin_en[i] = true;
                right->subwin_en[i] = true;
                break;
            case LEFT_MODE:
                left->subwin_en[i] = true;
                right->subwin_en[i] = false;
                break;
            case RIGHT_MODE:
                left->subwin_en[i] = false;
                right->subwin_en[i] = true;
                break;

            }
        }
    }

#ifdef DEBUG
    printf("AeBig left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->win.h_offs, left->win.v_offs,
           left->win.h_size, left->win.v_size, right->win.h_offs, right->win.v_offs,
           right->win.h_size, right->win.v_size);
#endif

    return XCAM_RETURN_NO_ERROR;

}


template <>
XCamReturn IspParamsSplitter::SplitRawHistLiteParamsVertical<struct isp2x_rawhistlite_cfg>(
    struct isp2x_rawhistlite_cfg* ori, struct isp2x_rawhistlite_cfg* left,
    struct isp2x_rawhistlite_cfg* right) {

    u8 wnd_num = 0;
    wnd_num = 5;

    WinSplitMode mode = LEFT_AND_RIGHT_MODE;

    SplitAecWinVertical(&ori->win, &left->win, &right->win, wnd_num, left_isp_rect_, bottom_left_isp_rect_, &mode);
    SplitAecWeightVertical(ori->weight, left->weight, right->weight, mode, wnd_num);

#ifdef DEBUG
    printf("HistLite left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->win.h_offs, left->win.v_offs,
           left->win.h_size, left->win.v_size, right->win.h_offs, right->win.v_offs,
           right->win.h_size, right->win.v_size);

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            printf("%d ", left->weight[i * wnd_num + j]);
        printf("\n");
    }
    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            printf("%d ", right->weight[i * wnd_num + j]);
        printf("\n");
    }
#endif

    return XCAM_RETURN_NO_ERROR;

}

template <>
XCamReturn IspParamsSplitter::SplitRawHistBigParamsVertical<struct isp2x_rawhistbig_cfg>(
    struct isp2x_rawhistbig_cfg* ori, struct isp2x_rawhistbig_cfg* left,
    struct isp2x_rawhistbig_cfg* right) {

    u8 wnd_num = 0;

    if (ori->wnd_num <= 1)
        wnd_num = 5;
    else
        wnd_num = 15;

    WinSplitMode mode = LEFT_AND_RIGHT_MODE;

    SplitAecWinVertical(&ori->win, &left->win, &right->win, wnd_num, left_isp_rect_, bottom_left_isp_rect_, &mode);
    SplitAecWeightVertical(ori->weight, left->weight, right->weight, mode, wnd_num);

#ifdef DEBUG
    printf("HistBig left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->win.h_offs, left->win.v_offs,
           left->win.h_size, left->win.v_size, right->win.h_offs, right->win.v_offs,
           right->win.h_size, right->win.v_size);

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            printf("%d ", left->weight[i * wnd_num + j]);
        printf("\n");
    }
    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            printf("%d ", right->weight[i * wnd_num + j]);
        printf("\n");
    }

#endif

    return XCAM_RETURN_NO_ERROR;

}

template <>
XCamReturn IspParamsSplitter::SplitAecParamsVertical<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori,
    struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //RAWAE
    if (ori->module_cfg_update & ISP32_MODULE_RAWAE0)
        ret = SplitRawAeLiteParamsVertical(&ori->meas.rawae0, &left->meas.rawae0, &right->meas.rawae0);
    if (ori->module_cfg_update & ISP32_MODULE_RAWAE1)
        ret = SplitRawAeBigParamsVertical(&ori->meas.rawae1, &left->meas.rawae1, &right->meas.rawae1);
    if (ori->module_cfg_update & ISP32_MODULE_RAWAE2)
        ret = SplitRawAeBigParamsVertical(&ori->meas.rawae2, &left->meas.rawae2, &right->meas.rawae2);
    if (ori->module_cfg_update & ISP32_MODULE_RAWAE3)
        ret = SplitRawAeBigParamsVertical(&ori->meas.rawae3, &left->meas.rawae3, &right->meas.rawae3);

    //RAWHIST
    if (ori->module_cfg_update & ISP32_MODULE_RAWHIST0)
        ret = SplitRawHistLiteParamsVertical(&ori->meas.rawhist0, &left->meas.rawhist0, &right->meas.rawhist0);
    if (ori->module_cfg_update & ISP32_MODULE_RAWHIST1)
        ret = SplitRawHistBigParamsVertical(&ori->meas.rawhist1, &left->meas.rawhist1, &right->meas.rawhist1);
    if (ori->module_cfg_update & ISP32_MODULE_RAWHIST2)
        ret = SplitRawHistBigParamsVertical(&ori->meas.rawhist2, &left->meas.rawhist2, &right->meas.rawhist2);
    if (ori->module_cfg_update & ISP32_MODULE_RAWHIST3)
        ret = SplitRawHistBigParamsVertical(&ori->meas.rawhist3, &left->meas.rawhist3, &right->meas.rawhist3);


    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAwbParamsVertical<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori,
    struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    isp2x_window ori_win;
    isp2x_window left_win;
    isp2x_window right_win;
    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    u8 wnd_num = 15;

    ori_win.h_offs = ori->meas.rawawb.h_offs;
    ori_win.h_size = ori->meas.rawawb.h_size;
    ori_win.v_offs = ori->meas.rawawb.v_offs;
    ori_win.v_size = ori->meas.rawawb.v_size;

    memcpy(&left_win, &ori_win, sizeof(ori_win));
    memcpy(&right_win, &ori_win, sizeof(ori_win));

    // Awb measure window
    u8 awb_ds;
    if (ori->meas.rawawb.wind_size == 0) {
        awb_ds = 2;
    } else {
        awb_ds = 3;
    }
    u16 min_vsize = wnd_num << awb_ds;

    SplitAwbWinVertical(&ori_win, &left_win, &right_win, awb_ds, wnd_num, left_isp_rect_, bottom_left_isp_rect_, &mode);
    if (ori_win.v_size < min_vsize) {
        ori->meas.rawawb.blk_measure_enable = 0;
        left->meas.rawawb.blk_measure_enable = 0;
        right->meas.rawawb.blk_measure_enable = 0;
    }
    else {
        if (mode == LEFT_AND_RIGHT_MODE) {
            if (left_win.v_size < min_vsize)
                left->meas.rawawb.blk_measure_enable = 0;
            if (right_win.v_size < min_vsize)
                right->meas.rawawb.blk_measure_enable = 0;
        }
    }

    // Awb blk_wei_w
    //SplitAwbWeight(&ori_win, &left_win, &right_win, ori->meas.rawawb.wp_blk_wei_w, left->meas.rawawb.wp_blk_wei_w, right->meas.rawawb.wp_blk_wei_w, mode, wnd_num);
    SplitAecWeightVertical(ori->meas.rawawb.wp_blk_wei_w, left->meas.rawawb.wp_blk_wei_w, right->meas.rawawb.wp_blk_wei_w, mode, wnd_num);

    left->meas.rawawb.h_offs = left_win.h_offs;
    left->meas.rawawb.h_size = left_win.h_size;
    left->meas.rawawb.v_offs = left_win.v_offs;
    left->meas.rawawb.v_size = left_win.v_size;

    right->meas.rawawb.h_offs = right_win.h_offs;
    right->meas.rawawb.h_size = right_win.h_size;
    right->meas.rawawb.v_offs = right_win.v_offs;
    right->meas.rawawb.v_size = right_win.v_size;

    LOGD_AWB("Awb measure window  left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left_win.h_offs, left_win.v_offs,
             left_win.h_size, left_win.v_size, right_win.h_offs, right_win.v_offs,
             right_win.h_size, right_win.v_size);

    LOGV_AWB("Awb block weight: \n LEFT = { \n");

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            LOGV_AWB("%d ", left->meas.rawawb.wp_blk_wei_w[i * wnd_num + j]);
        LOGV_AWB("\n");
    }
    LOGV_AWB("} \n RIGHT = { \n");

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++)
            LOGV_AWB("%d ", right->meas.rawawb.wp_blk_wei_w[i * wnd_num + j]);
        LOGV_AWB("\n");
    }
    LOGV_AWB("}  \n");

    // Awb Multi Window
    isp2x_window sub_ori_win;
    isp2x_window sub_left_win;
    isp2x_window sub_right_win;
    u16 sub_win_st = 0;
    u16 sub_win_ed = 0;
    u16 main_win_st = 0;
    u16 main_win_ed = 0;

    if (ori->meas.rawawb.multiwindow_en) {
        // Awb Multi window 0
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow0_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.multiwindow0_h_size - ori->meas.rawawb.multiwindow0_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow0_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.multiwindow0_v_size - ori->meas.rawawb.multiwindow0_v_offs;

        sub_win_st = left_isp_rect_.y + sub_ori_win.v_offs;
        sub_win_ed = sub_win_st + sub_ori_win.v_size;
        main_win_st = left_isp_rect_.y + ori_win.v_offs;
        main_win_ed = main_win_st + ori_win.v_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.v_offs, sub_ori_win.v_size);
            sub_ori_win.v_offs = 0;
            sub_ori_win.v_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_0 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.v_offs, ori_win.v_offs);
            sub_ori_win.v_offs = left_isp_rect_.y + ori_win.v_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.v_offs, sub_ori_win.v_size, ori_win.v_offs, ori_win.v_size);
            sub_ori_win.v_offs = ori_win.v_offs;
            sub_ori_win.v_size = ori_win.v_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.v_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.v_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWinVertical(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, bottom_left_isp_rect_, &mode);
        left->meas.rawawb.multiwindow0_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow0_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow0_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow0_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow0_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow0_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow0_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow0_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 0 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 1
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow1_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.multiwindow1_h_size - ori->meas.rawawb.multiwindow1_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow1_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.multiwindow1_v_size - ori->meas.rawawb.multiwindow1_v_offs;

        sub_win_st = left_isp_rect_.y + sub_ori_win.v_offs;
        sub_win_ed = sub_win_st + sub_ori_win.v_size;
        main_win_st = left_isp_rect_.y + ori_win.v_offs;
        main_win_ed = main_win_st + ori_win.v_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.v_offs, sub_ori_win.v_size);
            sub_ori_win.v_offs = 0;
            sub_ori_win.v_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_1 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.v_offs, ori_win.v_offs);
            sub_ori_win.v_offs = left_isp_rect_.y + ori_win.v_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.v_offs, sub_ori_win.v_size, ori_win.v_offs, ori_win.h_size);
            sub_ori_win.v_offs = ori_win.v_offs;
            sub_ori_win.v_size = ori_win.v_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.v_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.v_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWinVertical(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, bottom_left_isp_rect_, &mode);
        left->meas.rawawb.multiwindow1_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow1_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow1_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow1_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow1_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow1_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow1_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow1_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 1 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 2
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow2_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.multiwindow2_h_size - ori->meas.rawawb.multiwindow2_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow2_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.multiwindow2_v_size - ori->meas.rawawb.multiwindow2_v_offs;

        sub_win_st = left_isp_rect_.y + sub_ori_win.v_offs;
        sub_win_ed = sub_win_st + sub_ori_win.v_size;
        main_win_st = left_isp_rect_.y + ori_win.v_offs;
        main_win_ed = main_win_st + ori_win.v_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.v_offs, sub_ori_win.v_size);
            sub_ori_win.v_offs = 0;
            sub_ori_win.v_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_2 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.v_offs, ori_win.v_offs);
            sub_ori_win.v_offs = left_isp_rect_.y + ori_win.v_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.v_offs, sub_ori_win.v_size, ori_win.v_offs, ori_win.v_size);
            sub_ori_win.h_offs = ori_win.v_offs;
            sub_ori_win.h_size = ori_win.v_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.v_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.v_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWinVertical(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, bottom_left_isp_rect_, &mode);
        left->meas.rawawb.multiwindow2_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow2_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow2_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow2_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow2_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow2_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow2_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow2_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 2 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);

        // Awb Multi window 3
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow3_h_offs;
        sub_ori_win.h_size = ori->meas.rawawb.multiwindow3_h_size - ori->meas.rawawb.multiwindow3_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow3_v_offs;
        sub_ori_win.v_size = ori->meas.rawawb.multiwindow3_v_size - ori->meas.rawawb.multiwindow3_v_offs ;

        sub_win_st = left_isp_rect_.y + sub_ori_win.v_offs;
        sub_win_ed = sub_win_st + sub_ori_win.v_size;
        main_win_st = left_isp_rect_.y + ori_win.v_offs;
        main_win_ed = main_win_st + ori_win.v_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.v_offs, sub_ori_win.v_size);
            sub_ori_win.v_offs = 0;
            sub_ori_win.v_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_3 hoffs(%d) reset as same as main window offs(%d) \n", sub_ori_win.v_offs, ori_win.v_offs);
            sub_ori_win.v_offs = left_isp_rect_.y + ori_win.v_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n", sub_ori_win.v_offs, sub_ori_win.v_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.v_offs = ori_win.v_offs;
            sub_ori_win.v_size = ori_win.v_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n", sub_ori_win.v_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.v_size = main_win_ed - sub_win_st;
        }


        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win, left_isp_rect_, bottom_left_isp_rect_, &mode);
        left->meas.rawawb.multiwindow3_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow3_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow3_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow3_v_size = sub_left_win.v_size + sub_left_win.v_offs ;

        right->meas.rawawb.multiwindow3_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow3_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow3_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow3_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 3 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs, sub_left_win.v_offs,
                 sub_left_win.h_size, sub_left_win.v_size, sub_right_win.h_offs, sub_right_win.v_offs,
                 sub_right_win.h_size, sub_right_win.v_size);
    }

    return ret;

}

template <>
XCamReturn IspParamsSplitter::SplitAfParamsVertical<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori,
    struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {
    struct isp32_rawaf_meas_cfg org_af = left->meas.rawaf;
    struct isp32_rawaf_meas_cfg* l_af = &left->meas.rawaf;
    struct isp32_rawaf_meas_cfg* r_af = &right->meas.rawaf;
    struct isp2x_rawaelite_meas_cfg org_ae0 = left->meas.rawae0;
    struct isp2x_rawaelite_meas_cfg* l_ae0 = &left->meas.rawae0;
    struct isp2x_rawaelite_meas_cfg* r_ae0 = &right->meas.rawae0;
    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t y_st, y_ed, l_blknum, r_blknum, ov_h, blk_h, r_skip_blknum;
    uint8_t wnd_num;

    wnd_num = sqrt(ISP32L_RAWAF_WND_DATA);

    ov_h = left_isp_rect_.h + left_isp_rect_.y - bottom_left_isp_rect_.y;
    y_st = org_af.win[0].v_offs;
    y_ed = y_st + org_af.win[0].v_size;
    l_isp_st = left_isp_rect_.y;
    l_isp_ed = left_isp_rect_.y + left_isp_rect_.h;
    r_isp_st = bottom_left_isp_rect_.y;
    r_isp_ed = bottom_left_isp_rect_.y + bottom_left_isp_rect_.h;
    LOGD_AF("wina.x_st %d, wina.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            y_st, y_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    //// winA ////
    // af win in both side
    if ((y_st < r_isp_st) && (y_ed > l_isp_ed)) {
        // af win < one isp width
        if (org_af.win[0].v_size < left_isp_rect_.h) {
            blk_h = org_af.win[0].v_size / wnd_num;
            l_blknum = (l_isp_ed - y_st + blk_h - 1) / blk_h;
            r_blknum = wnd_num - l_blknum;
            l_win_ed = l_isp_ed - 4;
            l_win_st = l_win_ed - blk_h * wnd_num;
            if (blk_h < ov_h) {
                r_skip_blknum = ov_h / blk_h;
                r_win_st = ov_h - r_skip_blknum * blk_h;
                r_win_ed = ov_h + (wnd_num - r_skip_blknum) * blk_h;
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + wnd_num * blk_h;
            }
        }
        // af win < one isp width * 1.5
        else if (org_af.win[0].v_size < left_isp_rect_.h * 3 / 2) {
            l_win_st = y_st;
            l_win_ed = l_isp_ed - 4;
            blk_h = (l_win_ed - l_win_st) / (wnd_num + 1);
            l_win_st = l_win_ed - blk_h * wnd_num;
            l_blknum = ((l_win_ed - l_win_st) * wnd_num + org_af.win[0].v_size - 1) / org_af.win[0].v_size;
            r_blknum = wnd_num - l_blknum;
            if (blk_h < ov_h) {
                r_skip_blknum = ov_h / blk_h;
                r_win_st = ov_h - r_skip_blknum * blk_h;
                r_win_ed = ov_h + (wnd_num - r_skip_blknum) * blk_h;
            }
            else {
                r_skip_blknum = 0;
                r_win_st = 2;
                r_win_ed = r_win_st + wnd_num * blk_h;
            }
        } else {
            l_win_st = y_st;
            l_win_ed = l_isp_ed - 4;
            blk_h = (l_win_ed - l_win_st) / wnd_num;
            l_win_st = l_win_ed - blk_h * wnd_num;
            r_win_st = 2;
            r_win_ed = r_win_st + blk_h * wnd_num;
            l_blknum = wnd_num;
            r_blknum = wnd_num;
            r_skip_blknum = 0;
        }
        LOGD_AF("wina: blk_w %d, ov_w %d, t_blknum %d, b_blknum %d, b_skip_blknum %d",
                blk_h, ov_h, l_blknum, r_blknum, r_skip_blknum);
    }
    // af win in right side
    else if ((y_st >= r_isp_st) && (y_ed > l_isp_ed)) {
        l_blknum = 0;
        r_blknum = wnd_num;
        r_win_st = y_st - bottom_left_isp_rect_.y;
        r_win_ed = y_ed - bottom_left_isp_rect_.y;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_blknum = wnd_num;
        r_blknum = 0;
        l_win_st = y_st;
        l_win_ed = y_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    l_af->win[0].v_offs = l_win_st;
    l_af->win[0].v_size = l_win_ed - l_win_st;
    r_af->win[0].v_offs = r_win_st;
    r_af->win[0].v_size = r_win_ed - r_win_st;

    //// winB ////
    y_st = org_af.win[1].v_offs;
    y_ed = y_st + org_af.win[1].v_size;
    LOGD_AF("winb.x_st %d, winb.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d",
            y_st, y_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    // af win in both side
    if ((y_st < r_isp_st) && (y_ed > l_isp_ed)) {
        l_win_st = y_st;
        l_win_ed = l_isp_ed - 4;
        r_win_st = ov_h - 2;
        r_win_ed = y_ed - bottom_left_isp_rect_.y;
    }
    // af win in right side
    else if ((y_st >= r_isp_st) && (y_ed > l_isp_ed)) {
        r_win_st = y_st - bottom_left_isp_rect_.y;
        r_win_ed = y_ed - bottom_left_isp_rect_.y;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_win_st = y_st;
        l_win_ed = y_ed;
        r_win_st = l_win_st;
        r_win_ed = l_win_ed;
    }

    l_af->win[1].v_offs = l_win_st;
    l_af->win[1].v_size = l_win_ed - l_win_st;
    r_af->win[1].v_offs = r_win_st;
    r_af->win[1].v_size = r_win_ed - r_win_st;

    // rawae0 is used by af now!!!
    if (org_af.ae_mode) {
        l_ae0->win.h_offs = l_af->win[0].h_offs;
        l_ae0->win.v_offs = l_af->win[0].v_offs;
        l_ae0->win.h_size = l_af->win[0].h_size;
        l_ae0->win.v_size = l_af->win[0].v_size;
        r_ae0->win.h_offs = r_af->win[0].h_offs;
        r_ae0->win.v_offs = r_af->win[0].v_offs;
        r_ae0->win.h_size = r_af->win[0].h_size;
        r_ae0->win.v_size = r_af->win[0].v_size;
    }

    LOGD_AF("AfWinA top=%d-%d-%d-%d, bottom=%d-%d-%d-%d",
            l_af->win[0].h_offs, l_af->win[0].v_offs,
            l_af->win[0].h_size, l_af->win[0].v_size,
            r_af->win[0].h_offs, r_af->win[0].v_offs,
            r_af->win[0].h_size, r_af->win[0].v_size);

    LOGD_AF("AfWinB top=%d-%d-%d-%d, bottom=%d-%d-%d-%d",
            l_af->win[1].h_offs, l_af->win[1].v_offs,
            l_af->win[1].h_size, l_af->win[1].v_size,
            r_af->win[1].h_offs, r_af->win[1].v_offs,
            r_af->win[1].h_size, r_af->win[1].v_size);

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAynrParamsVertical<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori, struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {
    struct isp32_ynr_cfg* ynr_cfg_ori = &ori->others.ynr_cfg;
    struct isp32_ynr_cfg* ynr_cfg_lef = &left->others.ynr_cfg;
    struct isp32_ynr_cfg* ynr_cfg_rht = &right->others.ynr_cfg;

    //left half pic center pixel calculate
    ynr_cfg_lef->rnr_center_coorh = pic_rect_.w / 2;
    ynr_cfg_lef->rnr_center_coorv = pic_rect_.h / 2;

    //right half pic center pixel calculate
    ynr_cfg_rht->rnr_center_coorh = (right_isp_rect_.w - pic_rect_.w / 2);
    ynr_cfg_rht->rnr_center_coorv = (bottom_right_isp_rect_.h - pic_rect_.h / 2);

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitAsharpParamsVertical<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori, struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {
    struct isp32_sharp_cfg* sharp_cfg_ori = &ori->others.sharp_cfg;
    struct isp32_sharp_cfg* sharp_cfg_lef = &left->others.sharp_cfg;
    struct isp32_sharp_cfg* sharp_cfg_rht = &right->others.sharp_cfg;

    //left half pic center pixel calculate
    sharp_cfg_lef->center_mode = 1;
    sharp_cfg_lef->center_wid = pic_rect_.w / 2;
    sharp_cfg_lef->center_het = left_isp_rect_.h / 2;

    //right half pic center pixel calculate
    sharp_cfg_rht->center_mode = 1;
    sharp_cfg_rht->center_wid = (right_isp_rect_.w - pic_rect_.w / 2);
    sharp_cfg_rht->center_het = (bottom_right_isp_rect_.h - pic_rect_.h / 2);

    return XCAM_RETURN_NO_ERROR;
}

int SplitAlscYtable(const unsigned short* in_array, int in_size, int ori_imgh,
                    unsigned short* dst_top, unsigned short* dst_bottom,
                    int top_h, int bottom_h) {
    int in_index = 0;
    int top_index = 0;
    int bottom_index = 0;
    for (in_index = 0; in_index < in_size; in_index++) {
        if (in_index < (in_size / 2)) {
            dst_top[top_index++] =
                ceil(in_array[in_index] * 1.0 / ori_imgh * top_h);
            dst_top[top_index++] =
                floor(in_array[in_index] * 1.0 / ori_imgh * top_h);
        } else {
            dst_bottom[bottom_index++] =
                ceil(in_array[in_index] * 1.0 / ori_imgh * bottom_h);
            dst_bottom[bottom_index++] =
                floor(in_array[in_index] * 1.0 / ori_imgh * bottom_h);
        }
    }

    return 0;
}

int AlscMatrixScaleVertical(unsigned short ori_matrix[], unsigned short left_matrix[],
                    unsigned short right_matrix[], int cols, int rows) {
    int ori_col_index = 0;
    int lef_dst_index = 0;
    int rht_dst_index = 0;
    int mid_col = cols / 2;
    int row_index = 0;

    for (row_index = 0; row_index < rows; row_index++) {
        for (ori_col_index = 0; ori_col_index < cols; ori_col_index++) {
            if (ori_col_index < mid_col) {
                left_matrix[row_index + ori_col_index * 2 * cols] =
                    ori_matrix[row_index + ori_col_index * cols];
                left_matrix[row_index + (ori_col_index * 2 + 1) * cols] =
                    (ori_matrix[row_index + ori_col_index * cols] +
                     ori_matrix[row_index + (ori_col_index + 1) * cols]) / 2;
            } else if (ori_col_index == mid_col) {
                left_matrix[row_index + (cols - 1) * cols] =
                    ori_matrix[row_index + ori_col_index * cols];
                right_matrix[row_index] =
                    ori_matrix[row_index + ori_col_index * cols];
            } else {
                right_matrix[row_index + (ori_col_index * 2 - cols) * cols] =
                    (ori_matrix[row_index + ori_col_index * cols] +
                     ori_matrix[row_index + (ori_col_index - 1) * cols]) / 2;
                right_matrix[row_index + (ori_col_index * 2 - cols + 1) * cols] =
                    ori_matrix[row_index + ori_col_index * cols];
            }
        }
    }

    return 0;
}

template <>
XCamReturn IspParamsSplitter::SplitAlscParamsVertical<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* ori, struct isp32_isp_params_cfg* left,
    struct isp32_isp_params_cfg* right) {

    struct isp3x_lsc_cfg* lsc_cfg_ori = &ori->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_top = &left->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_btm = &right->others.lsc_cfg;

    memcpy(lsc_cfg_top->x_size_tbl, lsc_cfg_ori->x_size_tbl,
           sizeof(lsc_cfg_ori->x_size_tbl));
    memcpy(lsc_cfg_btm->x_size_tbl, lsc_cfg_ori->x_size_tbl,
           sizeof(lsc_cfg_ori->x_size_tbl));
    
    SplitAlscXtable(lsc_cfg_ori->y_size_tbl, ISP3X_LSC_SIZE_TBL_SIZE,
                    pic_rect_.h,
                    lsc_cfg_top->y_size_tbl,
                    lsc_cfg_btm->y_size_tbl,
                    left_isp_rect_.h,
                    bottom_left_isp_rect_.h);

    AlscMatrixScaleVertical(lsc_cfg_ori->r_data_tbl,
                            lsc_cfg_top->r_data_tbl,
                            lsc_cfg_btm->r_data_tbl,
                            ISP3X_LSC_SIZE_TBL_SIZE + 1,
                            ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScaleVertical(lsc_cfg_ori->gr_data_tbl,
                            lsc_cfg_top->gr_data_tbl,
                            lsc_cfg_btm->gr_data_tbl,
                            ISP3X_LSC_SIZE_TBL_SIZE + 1,
                            ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScaleVertical(lsc_cfg_ori->gb_data_tbl,
                            lsc_cfg_top->gb_data_tbl,
                            lsc_cfg_btm->gb_data_tbl,
                            ISP3X_LSC_SIZE_TBL_SIZE + 1,
                            ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AlscMatrixScaleVertical(lsc_cfg_ori->b_data_tbl,
                            lsc_cfg_top->b_data_tbl,
                            lsc_cfg_btm->b_data_tbl,
                            ISP3X_LSC_SIZE_TBL_SIZE + 1,
                            ISP3X_LSC_SIZE_TBL_SIZE + 1);

    lscGradUpdate(lsc_cfg_top->x_grad_tbl, lsc_cfg_top->y_grad_tbl,
                  lsc_cfg_top->x_size_tbl, lsc_cfg_top->y_size_tbl,
                  ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    lscGradUpdate(lsc_cfg_btm->x_grad_tbl, lsc_cfg_btm->y_grad_tbl,
                  lsc_cfg_btm->x_size_tbl, lsc_cfg_btm->y_size_tbl,
                  ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    return XCAM_RETURN_NO_ERROR;
}

template <>
XCamReturn IspParamsSplitter::SplitIspParamsVertical<struct isp32_isp_params_cfg>(
    struct isp32_isp_params_cfg* orig_isp_params,
    struct isp32_isp_params_cfg* isp_params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct isp32_isp_params_cfg* top_left_isp_params     = isp_params;
    struct isp32_isp_params_cfg* top_right_isp_params    = isp_params + 1;
    struct isp32_isp_params_cfg* bottom_left_isp_params  = isp_params + 2;
    struct isp32_isp_params_cfg* bottom_right_isp_params = isp_params + 3;
    struct isp32_isp_params_cfg tmp_isp_param;

    // struct isp32_cac_cfg cac_right;
    // struct isp32_ldch_cfg ldch_right;
    // memcpy(&cac_right, &right_isp_params->others.cac_cfg, sizeof(struct isp32_cac_cfg));
    // memcpy(&ldch_right, &right_isp_params->others.ldch_cfg, sizeof(struct isp32_ldch_cfg));

    // Modules that use the same params for both left and right isp
    // will not need to implent split function
    
    SplitIspParams(orig_isp_params, isp_params);

    tmp_isp_param = *top_left_isp_params;
    memcpy(bottom_left_isp_params, top_left_isp_params, sizeof(struct isp32_isp_params_cfg));

    // memcpy(&right_isp_params->others.cac_cfg, &cac_right, sizeof(struct isp32_cac_cfg));
    // memcpy(&right_isp_params->others.ldch_cfg, &ldch_right, sizeof(struct isp32_ldch_cfg));

    ret = SplitAecParamsVertical(&tmp_isp_param, top_left_isp_params, bottom_left_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_RAWAWB)
        ret = SplitAwbParamsVertical(&tmp_isp_param, top_left_isp_params, bottom_left_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_RAWAF)
        ret = SplitAfParamsVertical(&tmp_isp_param, top_left_isp_params, bottom_left_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_SHARP)
        ret = SplitAsharpParamsVertical(&tmp_isp_param, top_left_isp_params, bottom_left_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_YNR)
        ret = SplitAynrParamsVertical(&tmp_isp_param, top_left_isp_params, bottom_left_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_LSC)
        ret = SplitAlscParamsVertical(&tmp_isp_param, top_left_isp_params, bottom_left_isp_params);

    tmp_isp_param = *top_right_isp_params;
    memcpy(bottom_right_isp_params, top_right_isp_params, sizeof(struct isp32_isp_params_cfg));

    // memcpy(&right_isp_params->others.cac_cfg, &cac_right, sizeof(struct isp32_cac_cfg));
    // memcpy(&right_isp_params->others.ldch_cfg, &ldch_right, sizeof(struct isp32_ldch_cfg));

    ret = SplitAecParamsVertical(&tmp_isp_param, top_right_isp_params, bottom_right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_RAWAWB)
        ret = SplitAwbParamsVertical(&tmp_isp_param, top_right_isp_params, bottom_right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_RAWAF)
        ret = SplitAfParamsVertical(&tmp_isp_param, top_right_isp_params, bottom_right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_SHARP)
        ret = SplitAsharpParamsVertical(&tmp_isp_param, top_right_isp_params, bottom_right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_YNR)
        ret = SplitAynrParamsVertical(&tmp_isp_param, top_right_isp_params, bottom_right_isp_params);
    if (orig_isp_params->module_cfg_update & ISP32_MODULE_LSC)
        ret = SplitAlscParamsVertical(&tmp_isp_param, top_right_isp_params, bottom_right_isp_params);

    // Should return failure ?
    // if (orig_isp_params->module_cfg_update & ISP32_MODULE_RAWAWB)
    //     ret = SplitAwbParams(orig_isp_params, left_isp_params, right_isp_params);
    // if (orig_isp_params->module_cfg_update & ISP32_MODULE_RAWAF)
    //     ret = SplitAfParams(orig_isp_params, left_isp_params, right_isp_params);
    // if (orig_isp_params->module_cfg_update & ISP32_MODULE_LSC)
    //     ret = SplitAlscParams(orig_isp_params, left_isp_params, right_isp_params);
    // if (orig_isp_params->module_cfg_update & ISP32_MODULE_YNR)
    //     ret = SplitAynrParams(orig_isp_params, left_isp_params, right_isp_params);
    // if (orig_isp_params->module_cfg_update & ISP32_MODULE_SHARP)
    //     ret = SplitAsharpParams(orig_isp_params, left_isp_params, right_isp_params);
    LOGD_CAMHW("Split ISP Params: left %p right %p size %d",
               top_left_isp_params,
               top_right_isp_params,
               sizeof(*top_left_isp_params));

    return ret;
}

template <>
XCamReturn IspParamsSplitter::SplitIspParams<struct isp39_isp_params_cfg>(
    struct isp39_isp_params_cfg* orig_isp_params,
    struct isp39_isp_params_cfg* isp_params) {
    // TODO
    XCamReturn ret = XCAM_RETURN_ERROR_FAILED;
    return ret;
}
