/*
 *  Copyright (c) 2024 Rockchip Electronics Co., Ltd
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

#include "hwi_c/isp35/aiq_isp35ParamsSplitter.h"

#include "algos/ae/rk_aiq_types_ae_hw.h"
#include "hwi_c/aiq_ispParamsSplitter.h"
#include "hwi_c/aiq_ispParamsSplitterCom.h"
#include "include/common/rk_isp20_hw.h"
#include "include/common/rk-isp35-config.h"
#include "rk_aiq_types_priv_c.h"

static XCamReturn
isp35SplitRawAeParams(AiqIspParamsSplitter_t* pSplit,
                      struct isp35_rawae_meas_cfg* ori,
                      struct isp35_rawae_meas_cfg* left,
                      struct isp35_rawae_meas_cfg* right) {

    u8 wnd_num = 0;

    if (ori->wnd_num == 0)
        wnd_num = 1;
    else if (ori->wnd_num == 1)
        wnd_num = 5;
    else
        wnd_num = 15;

    Splitter_Rectangle_t left_isp_rect_ = pSplit->left_isp_rect_;
    Splitter_Rectangle_t right_isp_rect_ = pSplit->right_isp_rect_;
    WinSplitMode mode        = LEFT_AND_RIGHT_MODE;
    struct isp2x_window ori_win, left_win, right_win;
    ori_win.h_offs = ori->win0_h_offset;
    ori_win.h_size = ori->win0_h_size;
    ori_win.v_offs = ori->win0_v_offset;
    ori_win.v_size = ori->win0_v_size;

    // win0
    SplitAecWin(&ori_win, &left_win, &right_win, wnd_num,
                left_isp_rect_, right_isp_rect_, &mode);

    left->win0_h_offset = left_win.h_offs;
    left->win0_h_size   = left_win.h_size;
    left->win0_v_offset = left_win.v_offs;
    left->win0_v_size   = left_win.v_size;

    right->win0_h_offset = right_win.h_offs;
    right->win0_h_size   = right_win.h_size;
    right->win0_v_offset = right_win.v_offs;
    right->win0_v_size   = right_win.v_size;

    // win1
    if (ori->wnd1_en) {
#ifdef DEBUG
        printf(" win locate in left isp\n");
#endif
        ori_win.h_offs = ori->win1_h_offset;
        ori_win.h_size = ori->win1_h_size;
        ori_win.v_offs = ori->win1_v_offset;
        ori_win.v_size = ori->win1_v_size;
        if (ori_win.h_offs + ori_win.h_size <= left_isp_rect_.w) {
            mode = LEFT_MODE;
            left_win.h_offs = ori_win.h_offs;
            left_win.h_size = ori_win.h_size;
            left_win.v_offs = ori_win.v_offs;
            left_win.v_size = ori_win.v_size;

            right_win.h_offs = 0;
            right_win.h_size = ori_win.h_size;
            right_win.v_offs = ori_win.v_offs;
            right_win.v_size = ori_win.v_size;
        } else if (ori_win.h_offs >= right_isp_rect_.x) {
#ifdef DEBUG
        printf(" win locate at right isp\n");
#endif
            mode = RIGHT_MODE;

            left_win.h_offs = 0;
            left_win.h_size = ori_win.h_size;
            left_win.v_offs = ori_win.v_offs;
            left_win.v_size = ori_win.v_size;

            right_win.h_offs = ori_win.h_offs - right_isp_rect_.x;
            right_win.h_size = ori_win.h_size;
            right_win.v_offs = ori_win.v_offs;
            right_win.v_size = ori_win.v_size;
        } else {
#ifdef DEBUG
        printf(" win locate at left&right isp\n");
#endif
            mode = LEFT_AND_RIGHT_MODE;

            left_win.h_offs = ori_win.h_offs;
            left_win.h_size = MAX(0, ((long)left_isp_rect_.w - (long)left_win.h_offs));
            left_win.v_offs = ori_win.v_offs;
            left_win.v_size = ori_win.v_size;

            right_win.h_offs = left_win.h_offs + left_win.h_size - right_isp_rect_.x;
            right_win.h_size = MAX(0, ((long)ori_win.h_size - (long)left_win.h_size));
            right_win.v_offs = ori_win.v_offs;
            right_win.v_size = ori_win.v_size;
        }

        left->win1_h_offset = left_win.h_offs;
        left->win1_h_size   = left_win.h_size;
        left->win1_v_offset = left_win.v_offs;
        left->win1_v_size   = left_win.v_size;

        right->win1_h_offset = right_win.h_offs;
        right->win1_h_size   = right_win.h_size;
        right->win1_v_offset = right_win.v_offs;
        right->win1_v_size   = right_win.v_size;

        switch (mode)
        {
        case LEFT_AND_RIGHT_MODE:
            left->wnd1_en = true;
            right->wnd1_en = true;
            break;
        case LEFT_MODE:
            left->wnd1_en = true;
            right->wnd1_en = false;
            break;
        case RIGHT_MODE:
            left->wnd1_en = false;
            right->wnd1_en = true;
            break;
        default:
            break;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
isp35SplitRawHistParams(AiqIspParamsSplitter_t* pSplit,
                        struct isp35_rawhist_meas_cfg* ori,
                        struct isp35_rawhist_meas_cfg* left,
                        struct isp35_rawhist_meas_cfg* right) {

    u8 wnd_num = 0;

    if (ori->wnd_num <= 1)
        wnd_num = 5;
    else
        wnd_num = 15;

    Splitter_Rectangle_t left_isp_rect_ = pSplit->left_isp_rect_;
    Splitter_Rectangle_t right_isp_rect_ = pSplit->right_isp_rect_;

    WinSplitMode mode        = LEFT_AND_RIGHT_MODE;
    struct isp2x_window ori_win, left_win, right_win;

    ori_win.h_offs = ori->h_offset;
    ori_win.h_size = ori->h_size;
    ori_win.v_offs = ori->v_offset;
    ori_win.v_size = ori->v_size;

    // win0
    SplitAecWin(&ori_win, &left_win, &right_win, wnd_num,
                left_isp_rect_, right_isp_rect_, &mode);

    left->h_offset = left_win.h_offs;
    left->h_size   = left_win.h_size;
    left->v_offset = left_win.v_offs;
    left->v_size   = left_win.v_size;

    right->h_offset = right_win.h_offs;
    right->h_size   = right_win.h_size;
    right->v_offset = right_win.v_offs;
    right->v_size   = right_win.v_size;

    AiqIspParamsSplitter_SplitAecWeight(ori->weight, left->weight, right->weight, mode, wnd_num);

#ifdef DEBUG
    printf("HistBig left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left->h_offs, left->v_offs,
           left->h_size, left->v_size, right->h_offs, right->v_offs,
           right->h_size, right->v_size);

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++) printf("%d ", left->weight[i * wnd_num + j]);
        printf("\n");
    }
    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++) printf("%d ", right->weight[i * wnd_num + j]);
        printf("\n");
    }

#endif

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn isp35SplitAecParams(AiqIspParamsSplitter_t* pSplit, struct isp35_isp_params_cfg* ori,
                               struct isp35_isp_params_cfg* left,
                               struct isp35_isp_params_cfg* right) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // RAWAE
    if (ori->module_cfg_update & ISP35_MODULE_RAWAE0)
        ret = isp35SplitRawAeParams(pSplit, &ori->meas.rawae0,
                                    &left->meas.rawae0, &right->meas.rawae0);
    if (ori->module_cfg_update & ISP35_MODULE_RAWAE3)
        ret = isp35SplitRawAeParams(pSplit, &ori->meas.rawae3,
                                    &left->meas.rawae3, &right->meas.rawae3);

    // RAWHIST
    if (ori->module_cfg_update & ISP35_MODULE_RAWHIST0)
        ret = isp35SplitRawHistParams(
            pSplit, &ori->meas.rawhist0, &left->meas.rawhist0, &right->meas.rawhist0);
    if (ori->module_cfg_update & ISP35_MODULE_RAWHIST3)
        ret = isp35SplitRawHistParams(
            pSplit, &ori->meas.rawhist3, &left->meas.rawhist3, &right->meas.rawhist3);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
isp35SplitRawAeParamsVertical(AiqIspParamsSplitter_t* pSplit,
                              struct isp35_rawae_meas_cfg* ori,
                              struct isp35_rawae_meas_cfg* left,
                              struct isp35_rawae_meas_cfg* right) {

    u8 wnd_num = 0;

    if (ori->wnd_num == 0)
        wnd_num = 1;
    else if (ori->wnd_num == 1)
        wnd_num = 5;
    else
        wnd_num = 15;

    Splitter_Rectangle_t left_isp_rect_ = pSplit->left_isp_rect_;
    Splitter_Rectangle_t right_isp_rect_ = pSplit->bottom_left_isp_rect_;
    WinSplitMode mode        = LEFT_AND_RIGHT_MODE;
    struct isp2x_window ori_win, left_win, right_win;
    ori_win.h_offs = ori->win0_h_offset;
    ori_win.h_size = ori->win0_h_size;
    ori_win.v_offs = ori->win0_v_offset;
    ori_win.v_size = ori->win0_v_size;

    // win0
    SplitAecWinVertical(&ori_win, &left_win, &right_win, wnd_num,
                        left_isp_rect_, right_isp_rect_, &mode);

    left->win0_h_offset = left_win.h_offs;
    left->win0_h_size   = left_win.h_size;
    left->win0_v_offset = left_win.v_offs;
    left->win0_v_size   = left_win.v_size;

    right->win0_h_offset = right_win.h_offs;
    right->win0_h_size   = right_win.h_size;
    right->win0_v_offset = right_win.v_offs;
    right->win0_v_size   = right_win.v_size;

    // win1
    if (ori->wnd1_en) {
#ifdef DEBUG
        printf(" win locate in left isp\n");
#endif
        ori_win.h_offs = ori->win1_h_offset;
        ori_win.h_size = ori->win1_h_size;
        ori_win.v_offs = ori->win1_v_offset;
        ori_win.v_size = ori->win1_v_size;
        if (ori_win.v_offs + ori_win.v_size <= left_isp_rect_.h) {
            mode = LEFT_MODE;
            left_win.h_offs = ori_win.h_offs;
            left_win.h_size = ori_win.h_size;
            left_win.v_offs = ori_win.v_offs;
            left_win.v_size = ori_win.v_size;

            right_win.h_offs = ori_win.h_offs;
            right_win.h_size = ori_win.h_size;
            right_win.v_offs = 0;
            right_win.v_size = ori_win.v_size;
        } else if (ori_win.v_offs >= right_isp_rect_.y) {
#ifdef DEBUG
        printf(" win locate at right isp\n");
#endif
            mode = RIGHT_MODE;

            left_win.h_offs = ori_win.h_offs;
            left_win.h_size = ori_win.h_size;
            left_win.v_offs = 0;
            left_win.v_size = ori_win.v_size;

            right_win.h_offs = ori_win.h_offs;
            right_win.h_size = ori_win.h_size;
            right_win.v_offs = ori_win.v_offs - right_isp_rect_.y;
            right_win.v_size = ori_win.v_size;
        } else {
#ifdef DEBUG
        printf(" win locate at left&right isp\n");
#endif
            mode = LEFT_AND_RIGHT_MODE;

            left_win.h_offs = ori_win.h_offs;
            left_win.h_size = ori_win.h_size;
            left_win.v_offs = ori_win.v_offs;
            left_win.v_size = MAX(0, ((long)left_isp_rect_.h - (long)left_win.v_offs));

            right_win.h_offs = ori_win.h_offs;
            right_win.h_size = ori_win.h_size;
            right_win.v_offs = left_win.v_offs + left_win.v_size - right_isp_rect_.y;
            right_win.v_size = MAX(0, ((long)ori_win.v_size - (long)left_win.v_size));
        }

        left->win1_h_offset = left_win.h_offs;
        left->win1_h_size   = left_win.h_size;
        left->win1_v_offset = left_win.v_offs;
        left->win1_v_size   = left_win.v_size;

        right->win1_h_offset = right_win.h_offs;
        right->win1_h_size   = right_win.h_size;
        right->win1_v_offset = right_win.v_offs;
        right->win1_v_size   = right_win.v_size;

        switch (mode)
        {
        case LEFT_AND_RIGHT_MODE:
            left->wnd1_en = true;
            right->wnd1_en = true;
            break;
        case LEFT_MODE:
            left->wnd1_en = true;
            right->wnd1_en = false;
            break;
        case RIGHT_MODE:
            left->wnd1_en = false;
            right->wnd1_en = true;
            break;
        default:
            break;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
isp35SplitRawHistParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                struct isp35_rawhist_meas_cfg* ori,
                                struct isp35_rawhist_meas_cfg* left,
                                struct isp35_rawhist_meas_cfg* right) {

    u8 wnd_num = 0;

    if (ori->wnd_num <= 1)
        wnd_num = 5;
    else
        wnd_num = 15;

    Splitter_Rectangle_t left_isp_rect_ = pSplit->left_isp_rect_;
    Splitter_Rectangle_t right_isp_rect_ = pSplit->bottom_left_isp_rect_;

    WinSplitMode mode        = LEFT_AND_RIGHT_MODE;
    struct isp2x_window ori_win, left_win, right_win;

    ori_win.h_offs = ori->h_offset;
    ori_win.h_size = ori->h_size;
    ori_win.v_offs = ori->v_offset;
    ori_win.v_size = ori->v_size;

    // win0
    SplitAecWinVertical(&ori_win, &left_win, &right_win, wnd_num,
                        left_isp_rect_, right_isp_rect_, &mode);

    left->h_offset = left_win.h_offs;
    left->h_size   = left_win.h_size;
    left->v_offset = left_win.v_offs;
    left->v_size   = left_win.v_size;

    right->h_offset = right_win.h_offs;
    right->h_size   = right_win.h_size;
    right->v_offset = right_win.v_offs;
    right->v_size   = right_win.v_size;

    AiqIspParamsSplitter_SplitAecWeightVertical(ori->weight, left->weight, right->weight, mode, wnd_num);

#ifdef DEBUG
    printf("HistBig top=%d-%d-%d-%d, bottom=%d-%d-%d-%d\n", left->h_offs, left->v_offs,
           left->h_size, left->v_size, right->h_offs, right->v_offs,
           right->h_size, right->v_size);

    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++) printf("%d ", left->weight[i * wnd_num + j]);
        printf("\n");
    }
    for (int i = 0; i < wnd_num; i++) {
        for (int j = 0; j < wnd_num; j++) printf("%d ", right->weight[i * wnd_num + j]);
        printf("\n");
    }

#endif

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn isp35SplitAecParamsVertical(AiqIspParamsSplitter_t* pSplit, struct isp35_isp_params_cfg* ori,
                               struct isp35_isp_params_cfg* left,
                               struct isp35_isp_params_cfg* right) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // RAWAE
    if (ori->module_cfg_update & ISP35_MODULE_RAWAE0)
        ret = isp35SplitRawAeParamsVertical(pSplit, &ori->meas.rawae0,
                                    &left->meas.rawae0, &right->meas.rawae0);
    if (ori->module_cfg_update & ISP35_MODULE_RAWAE3)
        ret = isp35SplitRawAeParamsVertical(pSplit, &ori->meas.rawae3,
                                    &left->meas.rawae3, &right->meas.rawae3);

    // RAWHIST
    if (ori->module_cfg_update & ISP35_MODULE_RAWHIST0)
        ret = isp35SplitRawHistParamsVertical(
            pSplit, &ori->meas.rawhist0, &left->meas.rawhist0, &right->meas.rawhist0);
    if (ori->module_cfg_update & ISP35_MODULE_RAWHIST3)
        ret = isp35SplitRawHistParamsVertical(
            pSplit, &ori->meas.rawhist3, &left->meas.rawhist3, &right->meas.rawhist3);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp35SplitAwbParams(AiqIspParamsSplitter_t* pSplit, struct isp35_isp_params_cfg* ori,
                               struct isp35_isp_params_cfg* left,
                               struct isp35_isp_params_cfg* right) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct isp2x_window ori_win;
    struct isp2x_window left_win;
    struct isp2x_window right_win;
    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    u8 wnd_num        = 15;

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

    AiqIspParamsSplitter_SplitAwbWin(&ori_win, &left_win, &right_win, awb_ds, wnd_num,
                                     pSplit->left_isp_rect_, pSplit->right_isp_rect_, &mode);
    if (ori_win.h_size < min_hsize) {
        ori->meas.rawawb.blk_measure_enable   = 0;
        left->meas.rawawb.blk_measure_enable  = 0;
        right->meas.rawawb.blk_measure_enable = 0;
    } else {
        if (mode == LEFT_AND_RIGHT_MODE) {
            if (left_win.h_size < min_hsize) left->meas.rawawb.blk_measure_enable = 0;
            if (right_win.h_size < min_hsize) right->meas.rawawb.blk_measure_enable = 0;
        }
    }

    // Awb blk_wei_w
    // SplitAwbWeight(&ori_win, &left_win, &right_win, ori->meas.rawawb.wp_blk_wei_w,
    // left->meas.rawawb.wp_blk_wei_w, right->meas.rawawb.wp_blk_wei_w, mode, wnd_num);
    AiqIspParamsSplitter_SplitAecWeight(ori->meas.rawawb.wp_blk_wei_w,
                                        left->meas.rawawb.wp_blk_wei_w,
                                        right->meas.rawawb.wp_blk_wei_w, mode, wnd_num);

    left->meas.rawawb.h_offs = left_win.h_offs;
    left->meas.rawawb.h_size = left_win.h_size;
    left->meas.rawawb.v_offs = left_win.v_offs;
    left->meas.rawawb.v_size = left_win.v_size;

    right->meas.rawawb.h_offs = right_win.h_offs;
    right->meas.rawawb.h_size = right_win.h_size;
    right->meas.rawawb.v_offs = right_win.v_offs;
    right->meas.rawawb.v_size = right_win.v_size;

    LOGD_AWB("Awb measure window  left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left_win.h_offs,
             left_win.v_offs, left_win.h_size, left_win.v_size, right_win.h_offs, right_win.v_offs,
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
    struct isp2x_window sub_ori_win;
    struct isp2x_window sub_left_win;
    struct isp2x_window sub_right_win;
    u16 sub_win_st  = 0;
    u16 sub_win_ed  = 0;
    u16 main_win_st = 0;
    u16 main_win_ed = 0;

    if (ori->meas.rawawb.multiwindow_en) {
        // Awb Multi window 0
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow0_h_offs;
        sub_ori_win.h_size =
            ori->meas.rawawb.multiwindow0_h_size - ori->meas.rawawb.multiwindow0_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow0_v_offs;
        sub_ori_win.v_size =
            ori->meas.rawawb.multiwindow0_v_size - ori->meas.rawawb.multiwindow0_v_offs;

        sub_win_st  = pSplit->left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed  = sub_win_st + sub_ori_win.h_size;
        main_win_st = pSplit->left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs,
                     sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_0 hoffs(%d) reset as same as main window offs(%d) \n",
                     sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = pSplit->left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n",
                     sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n",
                     sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        AiqIspParamsSplitter_SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win,
                                              &left_win, &right_win, pSplit->left_isp_rect_,
                                              pSplit->right_isp_rect_, &mode);
        left->meas.rawawb.multiwindow0_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow0_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow0_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow0_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow0_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow0_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow0_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow0_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 0 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs,
                 sub_left_win.v_offs, sub_left_win.h_size, sub_left_win.v_size,
                 sub_right_win.h_offs, sub_right_win.v_offs, sub_right_win.h_size,
                 sub_right_win.v_size);

        // Awb Multi window 1
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow1_h_offs;
        sub_ori_win.h_size =
            ori->meas.rawawb.multiwindow1_h_size - ori->meas.rawawb.multiwindow1_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow1_v_offs;
        sub_ori_win.v_size =
            ori->meas.rawawb.multiwindow1_v_size - ori->meas.rawawb.multiwindow1_v_offs;

        sub_win_st  = pSplit->left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed  = sub_win_st + sub_ori_win.h_size;
        main_win_st = pSplit->left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs,
                     sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_1 hoffs(%d) reset as same as main window offs(%d) \n",
                     sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = pSplit->left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n",
                     sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n",
                     sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        AiqIspParamsSplitter_SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win,
                                              &left_win, &right_win, pSplit->left_isp_rect_,
                                              pSplit->right_isp_rect_, &mode);
        left->meas.rawawb.multiwindow1_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow1_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow1_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow1_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow1_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow1_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow1_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow1_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 1 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs,
                 sub_left_win.v_offs, sub_left_win.h_size, sub_left_win.v_size,
                 sub_right_win.h_offs, sub_right_win.v_offs, sub_right_win.h_size,
                 sub_right_win.v_size);

        // Awb Multi window 2
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow2_h_offs;
        sub_ori_win.h_size =
            ori->meas.rawawb.multiwindow2_h_size - ori->meas.rawawb.multiwindow2_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow2_v_offs;
        sub_ori_win.v_size =
            ori->meas.rawawb.multiwindow2_v_size - ori->meas.rawawb.multiwindow2_v_offs;

        sub_win_st  = pSplit->left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed  = sub_win_st + sub_ori_win.h_size;
        main_win_st = pSplit->left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs,
                     sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_2 hoffs(%d) reset as same as main window offs(%d) \n",
                     sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = pSplit->left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n",
                     sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n",
                     sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        AiqIspParamsSplitter_SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win,
                                              &left_win, &right_win, pSplit->left_isp_rect_,
                                              pSplit->right_isp_rect_, &mode);
        left->meas.rawawb.multiwindow2_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow2_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow2_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow2_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow2_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow2_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow2_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow2_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 2 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs,
                 sub_left_win.v_offs, sub_left_win.h_size, sub_left_win.v_size,
                 sub_right_win.h_offs, sub_right_win.v_offs, sub_right_win.h_size,
                 sub_right_win.v_size);

        // Awb Multi window 3
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow3_h_offs;
        sub_ori_win.h_size =
            ori->meas.rawawb.multiwindow3_h_size - ori->meas.rawawb.multiwindow3_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow3_v_offs;
        sub_ori_win.v_size =
            ori->meas.rawawb.multiwindow3_v_size - ori->meas.rawawb.multiwindow3_v_offs;

        sub_win_st  = pSplit->left_isp_rect_.x + sub_ori_win.h_offs;
        sub_win_ed  = sub_win_st + sub_ori_win.h_size;
        main_win_st = pSplit->left_isp_rect_.x + ori_win.h_offs;
        main_win_ed = main_win_st + ori_win.h_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.h_offs,
                     sub_ori_win.h_size);
            sub_ori_win.h_offs = 0;
            sub_ori_win.h_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_3 hoffs(%d) reset as same as main window offs(%d) \n",
                     sub_ori_win.h_offs, ori_win.h_offs);
            sub_ori_win.h_offs = pSplit->left_isp_rect_.x + ori_win.h_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n",
                     sub_ori_win.h_offs, sub_ori_win.h_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.h_offs = ori_win.h_offs;
            sub_ori_win.h_size = ori_win.h_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n",
                     sub_ori_win.h_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.h_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        AiqIspParamsSplitter_SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win,
                                              &left_win, &right_win, pSplit->left_isp_rect_,
                                              pSplit->right_isp_rect_, &mode);
        left->meas.rawawb.multiwindow3_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow3_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow3_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow3_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow3_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow3_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow3_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow3_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 3 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs,
                 sub_left_win.v_offs, sub_left_win.h_size, sub_left_win.v_size,
                 sub_right_win.h_offs, sub_right_win.v_offs, sub_right_win.h_size,
                 sub_right_win.v_size);
    }

    return ret;
}

XCamReturn isp35SplitAwbParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                       struct isp35_isp_params_cfg* ori,
                                       struct isp35_isp_params_cfg* left,
                                       struct isp35_isp_params_cfg* right) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct isp2x_window ori_win;
    struct isp2x_window left_win;
    struct isp2x_window right_win;
    WinSplitMode mode = LEFT_AND_RIGHT_MODE;
    u8 wnd_num        = 15;

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

    AiqIspParamsSplitter_SplitAwbWinVertical(&ori_win, &left_win, &right_win, awb_ds, wnd_num,
                                             pSplit->left_isp_rect_, pSplit->bottom_left_isp_rect_,
                                             &mode);
    if (ori_win.v_size < min_vsize) {
        ori->meas.rawawb.blk_measure_enable   = 0;
        left->meas.rawawb.blk_measure_enable  = 0;
        right->meas.rawawb.blk_measure_enable = 0;
    } else {
        if (mode == LEFT_AND_RIGHT_MODE) {
            if (left_win.v_size < min_vsize) left->meas.rawawb.blk_measure_enable = 0;
            if (right_win.v_size < min_vsize) right->meas.rawawb.blk_measure_enable = 0;
        }
    }

    // Awb blk_wei_w
    // SplitAwbWeight(&ori_win, &left_win, &right_win, ori->meas.rawawb.wp_blk_wei_w,
    // left->meas.rawawb.wp_blk_wei_w, right->meas.rawawb.wp_blk_wei_w, mode, wnd_num);
    AiqIspParamsSplitter_SplitAecWeightVertical(ori->meas.rawawb.wp_blk_wei_w,
                                                left->meas.rawawb.wp_blk_wei_w,
                                                right->meas.rawawb.wp_blk_wei_w, mode, wnd_num);

    left->meas.rawawb.h_offs = left_win.h_offs;
    left->meas.rawawb.h_size = left_win.h_size;
    left->meas.rawawb.v_offs = left_win.v_offs;
    left->meas.rawawb.v_size = left_win.v_size;

    right->meas.rawawb.h_offs = right_win.h_offs;
    right->meas.rawawb.h_size = right_win.h_size;
    right->meas.rawawb.v_offs = right_win.v_offs;
    right->meas.rawawb.v_size = right_win.v_size;

    LOGD_AWB("Awb measure window  left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", left_win.h_offs,
             left_win.v_offs, left_win.h_size, left_win.v_size, right_win.h_offs, right_win.v_offs,
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
    struct isp2x_window sub_ori_win;
    struct isp2x_window sub_left_win;
    struct isp2x_window sub_right_win;
    u16 sub_win_st  = 0;
    u16 sub_win_ed  = 0;
    u16 main_win_st = 0;
    u16 main_win_ed = 0;

    if (ori->meas.rawawb.multiwindow_en) {
        // Awb Multi window 0
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow0_h_offs;
        sub_ori_win.h_size =
            ori->meas.rawawb.multiwindow0_h_size - ori->meas.rawawb.multiwindow0_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow0_v_offs;
        sub_ori_win.v_size =
            ori->meas.rawawb.multiwindow0_v_size - ori->meas.rawawb.multiwindow0_v_offs;

        sub_win_st  = pSplit->left_isp_rect_.y + sub_ori_win.v_offs;
        sub_win_ed  = sub_win_st + sub_ori_win.v_size;
        main_win_st = pSplit->left_isp_rect_.y + ori_win.v_offs;
        main_win_ed = main_win_st + ori_win.v_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.v_offs,
                     sub_ori_win.v_size);
            sub_ori_win.v_offs = 0;
            sub_ori_win.v_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_0 hoffs(%d) reset as same as main window offs(%d) \n",
                     sub_ori_win.v_offs, ori_win.v_offs);
            sub_ori_win.v_offs = pSplit->left_isp_rect_.y + ori_win.v_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n",
                     sub_ori_win.v_offs, sub_ori_win.v_size, ori_win.v_offs, ori_win.v_size);
            sub_ori_win.v_offs = ori_win.v_offs;
            sub_ori_win.v_size = ori_win.v_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_0 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n",
                     sub_ori_win.v_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.v_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        AiqIspParamsSplitter_AiqIspParamsSplitter_SplitAwbMultiWinVertical(
            &sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win,
            pSplit->left_isp_rect_, pSplit->bottom_left_isp_rect_, &mode);
        left->meas.rawawb.multiwindow0_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow0_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow0_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow0_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow0_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow0_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow0_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow0_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 0 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs,
                 sub_left_win.v_offs, sub_left_win.h_size, sub_left_win.v_size,
                 sub_right_win.h_offs, sub_right_win.v_offs, sub_right_win.h_size,
                 sub_right_win.v_size);

        // Awb Multi window 1
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow1_h_offs;
        sub_ori_win.h_size =
            ori->meas.rawawb.multiwindow1_h_size - ori->meas.rawawb.multiwindow1_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow1_v_offs;
        sub_ori_win.v_size =
            ori->meas.rawawb.multiwindow1_v_size - ori->meas.rawawb.multiwindow1_v_offs;

        sub_win_st  = pSplit->left_isp_rect_.y + sub_ori_win.v_offs;
        sub_win_ed  = sub_win_st + sub_ori_win.v_size;
        main_win_st = pSplit->left_isp_rect_.y + ori_win.v_offs;
        main_win_ed = main_win_st + ori_win.v_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.v_offs,
                     sub_ori_win.v_size);
            sub_ori_win.v_offs = 0;
            sub_ori_win.v_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_1 hoffs(%d) reset as same as main window offs(%d) \n",
                     sub_ori_win.v_offs, ori_win.v_offs);
            sub_ori_win.v_offs = pSplit->left_isp_rect_.y + ori_win.v_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n",
                     sub_ori_win.v_offs, sub_ori_win.v_size, ori_win.v_offs, ori_win.h_size);
            sub_ori_win.v_offs = ori_win.v_offs;
            sub_ori_win.v_size = ori_win.v_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_1 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n",
                     sub_ori_win.v_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.v_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        AiqIspParamsSplitter_AiqIspParamsSplitter_SplitAwbMultiWinVertical(
            &sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win,
            pSplit->left_isp_rect_, pSplit->bottom_left_isp_rect_, &mode);
        left->meas.rawawb.multiwindow1_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow1_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow1_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow1_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow1_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow1_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow1_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow1_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 1 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs,
                 sub_left_win.v_offs, sub_left_win.h_size, sub_left_win.v_size,
                 sub_right_win.h_offs, sub_right_win.v_offs, sub_right_win.h_size,
                 sub_right_win.v_size);

        // Awb Multi window 2
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow2_h_offs;
        sub_ori_win.h_size =
            ori->meas.rawawb.multiwindow2_h_size - ori->meas.rawawb.multiwindow2_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow2_v_offs;
        sub_ori_win.v_size =
            ori->meas.rawawb.multiwindow2_v_size - ori->meas.rawawb.multiwindow2_v_offs;

        sub_win_st  = pSplit->left_isp_rect_.y + sub_ori_win.v_offs;
        sub_win_ed  = sub_win_st + sub_ori_win.v_size;
        main_win_st = pSplit->left_isp_rect_.y + ori_win.v_offs;
        main_win_ed = main_win_st + ori_win.v_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.v_offs,
                     sub_ori_win.v_size);
            sub_ori_win.v_offs = 0;
            sub_ori_win.v_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_2 hoffs(%d) reset as same as main window offs(%d) \n",
                     sub_ori_win.v_offs, ori_win.v_offs);
            sub_ori_win.v_offs = pSplit->left_isp_rect_.y + ori_win.v_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n",
                     sub_ori_win.v_offs, sub_ori_win.v_size, ori_win.v_offs, ori_win.v_size);
            sub_ori_win.h_offs = ori_win.v_offs;
            sub_ori_win.h_size = ori_win.v_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_2 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n",
                     sub_ori_win.v_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.v_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        AiqIspParamsSplitter_AiqIspParamsSplitter_SplitAwbMultiWinVertical(
            &sub_ori_win, &sub_left_win, &sub_right_win, &left_win, &right_win,
            pSplit->left_isp_rect_, pSplit->bottom_left_isp_rect_, &mode);
        left->meas.rawawb.multiwindow2_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow2_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow2_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow2_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow2_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow2_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow2_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow2_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 2 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs,
                 sub_left_win.v_offs, sub_left_win.h_size, sub_left_win.v_size,
                 sub_right_win.h_offs, sub_right_win.v_offs, sub_right_win.h_size,
                 sub_right_win.v_size);

        // Awb Multi window 3
        sub_ori_win.h_offs = ori->meas.rawawb.multiwindow3_h_offs;
        sub_ori_win.h_size =
            ori->meas.rawawb.multiwindow3_h_size - ori->meas.rawawb.multiwindow3_h_offs;
        sub_ori_win.v_offs = ori->meas.rawawb.multiwindow3_v_offs;
        sub_ori_win.v_size =
            ori->meas.rawawb.multiwindow3_v_size - ori->meas.rawawb.multiwindow3_v_offs;

        sub_win_st  = pSplit->left_isp_rect_.y + sub_ori_win.v_offs;
        sub_win_ed  = sub_win_st + sub_ori_win.v_size;
        main_win_st = pSplit->left_isp_rect_.y + ori_win.v_offs;
        main_win_ed = main_win_st + ori_win.v_size;

        if ((sub_win_ed <= main_win_st) || (sub_win_st >= main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset to [0 0] \n", sub_ori_win.v_offs,
                     sub_ori_win.v_size);
            sub_ori_win.v_offs = 0;
            sub_ori_win.v_size = 0;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed <= main_win_ed)) {
            LOGW_AWB("multiwindow_3 hoffs(%d) reset as same as main window offs(%d) \n",
                     sub_ori_win.v_offs, ori_win.v_offs);
            sub_ori_win.v_offs = pSplit->left_isp_rect_.y + ori_win.v_offs;
        } else if ((sub_win_st < main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 [hoffs(%d) hsize(%d)] reset as same as main window [%d %d] \n",
                     sub_ori_win.v_offs, sub_ori_win.v_size, ori_win.h_offs, ori_win.h_size);
            sub_ori_win.v_offs = ori_win.v_offs;
            sub_ori_win.v_size = ori_win.v_size;
        } else if ((sub_win_st >= main_win_st) && (sub_win_ed > main_win_ed)) {
            LOGW_AWB("multiwindow_3 hsize(%d) reset to %d (main_win_ed %d - sub_win_st %d) \n",
                     sub_ori_win.v_size, main_win_ed - sub_win_st, main_win_ed, sub_win_st);
            sub_ori_win.v_size = main_win_ed - sub_win_st;
        }

        memcpy(&sub_left_win, &sub_ori_win, sizeof(sub_ori_win));
        memcpy(&sub_right_win, &sub_ori_win, sizeof(sub_ori_win));

        AiqIspParamsSplitter_SplitAwbMultiWin(&sub_ori_win, &sub_left_win, &sub_right_win,
                                              &left_win, &right_win, pSplit->left_isp_rect_,
                                              pSplit->bottom_left_isp_rect_, &mode);
        left->meas.rawawb.multiwindow3_h_offs = sub_left_win.h_offs;
        left->meas.rawawb.multiwindow3_h_size = sub_left_win.h_size + sub_left_win.h_offs;
        left->meas.rawawb.multiwindow3_v_offs = sub_left_win.v_offs;
        left->meas.rawawb.multiwindow3_v_size = sub_left_win.v_size + sub_left_win.v_offs;

        right->meas.rawawb.multiwindow3_h_offs = sub_right_win.h_offs;
        right->meas.rawawb.multiwindow3_h_size = sub_right_win.h_size + sub_right_win.h_offs;
        right->meas.rawawb.multiwindow3_v_offs = sub_right_win.v_offs;
        right->meas.rawawb.multiwindow3_v_size = sub_right_win.v_size + sub_right_win.v_offs;

        LOGD_AWB("Awb Multi window 3 left=%d-%d-%d-%d, right=%d-%d-%d-%d\n", sub_left_win.h_offs,
                 sub_left_win.v_offs, sub_left_win.h_size, sub_left_win.v_size,
                 sub_right_win.h_offs, sub_right_win.v_offs, sub_right_win.h_size,
                 sub_right_win.v_size);
    }

    return ret;
}

XCamReturn isp35SplitAlscParams(AiqIspParamsSplitter_t* pSplit, struct isp35_isp_params_cfg* ori,
                                struct isp35_isp_params_cfg* left,
                                struct isp35_isp_params_cfg* right) {
    struct isp3x_lsc_cfg* lsc_cfg_ori = &ori->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_lef = &left->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_rht = &right->others.lsc_cfg;

    memcpy(lsc_cfg_lef->y_size_tbl, lsc_cfg_ori->y_size_tbl, sizeof(lsc_cfg_ori->y_size_tbl));
    memcpy(lsc_cfg_rht->y_size_tbl, lsc_cfg_ori->y_size_tbl, sizeof(lsc_cfg_ori->y_size_tbl));

    int lsc_tbl_idx_lef = ISP3X_LSC_SIZE_TBL_SIZE / 2;
    int lsc_tbl_idx_rht = ISP3X_LSC_SIZE_TBL_SIZE / 2 - 1;
    unsigned short lsc_x_size0_lef = 0;
    unsigned short lsc_x_size0_rht = 0;

    for (int i = ISP3X_LSC_SIZE_TBL_SIZE/2 - 1; i > 0; i--) {
        lsc_x_size0_rht += lsc_cfg_ori->x_size_tbl[i];
        lsc_tbl_idx_rht = i;
        if (RKMOUDLE_UNITE_EXTEND_PIXEL < lsc_x_size0_rht) {
            break;
        }
    }
    for (int i = ISP3X_LSC_SIZE_TBL_SIZE/2; i < ISP3X_LSC_SIZE_TBL_SIZE; i++) {
        lsc_x_size0_lef += lsc_cfg_ori->x_size_tbl[i];
        lsc_tbl_idx_lef = i;
        if (RKMOUDLE_UNITE_EXTEND_PIXEL < lsc_x_size0_lef) {
            break;
        }
    }
    float rate_rht = RKMOUDLE_UNITE_EXTEND_PIXEL * 1.0 / lsc_x_size0_rht;
    float rate_lef = RKMOUDLE_UNITE_EXTEND_PIXEL * 1.0 / lsc_x_size0_lef;

    AiqIspParamsSplitter_SplitAlscXtable(lsc_cfg_ori->x_size_tbl, ISP3X_LSC_SIZE_TBL_SIZE,
                                         lsc_cfg_lef->x_size_tbl, lsc_cfg_rht->x_size_tbl);

    AiqIspParamsSplitter_AlscMatrixScale(rate_lef, lsc_tbl_idx_lef,
                                         rate_rht, lsc_tbl_idx_rht,
                                         lsc_cfg_ori->r_data_tbl, lsc_cfg_lef->r_data_tbl,
                                         lsc_cfg_rht->r_data_tbl, ISP3X_LSC_SIZE_TBL_SIZE + 1,
                                         ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AiqIspParamsSplitter_AlscMatrixScale(rate_lef, lsc_tbl_idx_lef,
                                         rate_rht, lsc_tbl_idx_rht,
                                         lsc_cfg_ori->gr_data_tbl, lsc_cfg_lef->gr_data_tbl,
                                         lsc_cfg_rht->gr_data_tbl, ISP3X_LSC_SIZE_TBL_SIZE + 1,
                                         ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AiqIspParamsSplitter_AlscMatrixScale(rate_lef, lsc_tbl_idx_lef,
                                         rate_rht, lsc_tbl_idx_rht,
                                         lsc_cfg_ori->gb_data_tbl, lsc_cfg_lef->gb_data_tbl,
                                         lsc_cfg_rht->gb_data_tbl, ISP3X_LSC_SIZE_TBL_SIZE + 1,
                                         ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AiqIspParamsSplitter_AlscMatrixScale(rate_lef, lsc_tbl_idx_lef,
                                         rate_rht, lsc_tbl_idx_rht,
                                         lsc_cfg_ori->b_data_tbl, lsc_cfg_lef->b_data_tbl,
                                         lsc_cfg_rht->b_data_tbl, ISP3X_LSC_SIZE_TBL_SIZE + 1,
                                         ISP3X_LSC_SIZE_TBL_SIZE + 1);

    AiqIspParamsSplitter_LscGradUpdate(lsc_cfg_lef->x_grad_tbl, lsc_cfg_lef->y_grad_tbl,
                                       lsc_cfg_lef->x_size_tbl, lsc_cfg_lef->y_size_tbl,
                                       ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    AiqIspParamsSplitter_LscGradUpdate(lsc_cfg_rht->x_grad_tbl, lsc_cfg_rht->y_grad_tbl,
                                       lsc_cfg_rht->x_size_tbl, lsc_cfg_rht->y_size_tbl,
                                       ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp35SplitAlscParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                        struct isp35_isp_params_cfg* ori,
                                        struct isp35_isp_params_cfg* left,
                                        struct isp35_isp_params_cfg* right) {
    struct isp3x_lsc_cfg* lsc_cfg_ori = &ori->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_top = &left->others.lsc_cfg;
    struct isp3x_lsc_cfg* lsc_cfg_btm = &right->others.lsc_cfg;

    memcpy(lsc_cfg_top->x_size_tbl, lsc_cfg_ori->x_size_tbl, sizeof(lsc_cfg_ori->x_size_tbl));
    memcpy(lsc_cfg_btm->x_size_tbl, lsc_cfg_ori->x_size_tbl, sizeof(lsc_cfg_ori->x_size_tbl));

    int lsc_tbl_idx_top = ISP3X_LSC_SIZE_TBL_SIZE / 2;
    int lsc_tbl_idx_btm = ISP3X_LSC_SIZE_TBL_SIZE / 2 - 1;
    unsigned short lsc_x_size0_top = 0;
    unsigned short lsc_x_size0_btm = 0;

    for (int i = ISP3X_LSC_SIZE_TBL_SIZE/2 - 1; i > 0; i--) {
        lsc_x_size0_btm += lsc_cfg_ori->y_size_tbl[i];
        lsc_tbl_idx_btm = i;
        if (RKMOUDLE_UNITE_EXTEND_PIXEL < lsc_x_size0_btm) {
            break;
        }
    }
    for (int i = ISP3X_LSC_SIZE_TBL_SIZE/2; i < ISP3X_LSC_SIZE_TBL_SIZE; i++) {
        lsc_x_size0_top += lsc_cfg_ori->y_size_tbl[i];
        lsc_tbl_idx_top = i;
        if (RKMOUDLE_UNITE_EXTEND_PIXEL < lsc_x_size0_top) {
            break;
        }
    }
    float rate_btm = RKMOUDLE_UNITE_EXTEND_PIXEL * 1.0 / lsc_x_size0_btm;
    float rate_top = RKMOUDLE_UNITE_EXTEND_PIXEL * 1.0 / lsc_x_size0_top;

    AiqIspParamsSplitter_SplitAlscXtable(lsc_cfg_ori->y_size_tbl, ISP3X_LSC_SIZE_TBL_SIZE,
                                         lsc_cfg_top->y_size_tbl, lsc_cfg_btm->y_size_tbl);

    AiqIspParamsSplitter_AlscMatrixScaleVertical(rate_top, lsc_tbl_idx_top, rate_btm, lsc_tbl_idx_btm,
        lsc_cfg_ori->r_data_tbl, lsc_cfg_top->r_data_tbl, lsc_cfg_btm->r_data_tbl,
        ISP3X_LSC_SIZE_TBL_SIZE + 1, ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AiqIspParamsSplitter_AlscMatrixScaleVertical(rate_top, lsc_tbl_idx_top, rate_btm, lsc_tbl_idx_btm,
        lsc_cfg_ori->gr_data_tbl, lsc_cfg_top->gr_data_tbl, lsc_cfg_btm->gr_data_tbl,
        ISP3X_LSC_SIZE_TBL_SIZE + 1, ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AiqIspParamsSplitter_AlscMatrixScaleVertical(rate_top, lsc_tbl_idx_top, rate_btm, lsc_tbl_idx_btm,
        lsc_cfg_ori->gb_data_tbl, lsc_cfg_top->gb_data_tbl, lsc_cfg_btm->gb_data_tbl,
        ISP3X_LSC_SIZE_TBL_SIZE + 1, ISP3X_LSC_SIZE_TBL_SIZE + 1);
    AiqIspParamsSplitter_AlscMatrixScaleVertical(rate_top, lsc_tbl_idx_top, rate_btm, lsc_tbl_idx_btm,
        lsc_cfg_ori->b_data_tbl, lsc_cfg_top->b_data_tbl, lsc_cfg_btm->b_data_tbl,
        ISP3X_LSC_SIZE_TBL_SIZE + 1, ISP3X_LSC_SIZE_TBL_SIZE + 1);

    AiqIspParamsSplitter_LscGradUpdate(lsc_cfg_top->x_grad_tbl, lsc_cfg_top->y_grad_tbl,
                                       lsc_cfg_top->x_size_tbl, lsc_cfg_top->y_size_tbl,
                                       ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    AiqIspParamsSplitter_LscGradUpdate(lsc_cfg_btm->x_grad_tbl, lsc_cfg_btm->y_grad_tbl,
                                       lsc_cfg_btm->x_size_tbl, lsc_cfg_btm->y_size_tbl,
                                       ISP3X_LSC_GRAD_TBL_SIZE, ISP3X_LSC_GRAD_TBL_SIZE);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp35SplitAynrParams(AiqIspParamsSplitter_t* pSplit, struct isp35_isp_params_cfg* ori,
                                struct isp35_isp_params_cfg* left,
                                struct isp35_isp_params_cfg* right) {
    struct isp35_ynr_cfg* ynr_cfg_ori = &ori->others.ynr_cfg;
    struct isp35_ynr_cfg* ynr_cfg_lef = &left->others.ynr_cfg;
    struct isp35_ynr_cfg* ynr_cfg_rht = &right->others.ynr_cfg;

    // left half pic center pixel calculate
    ynr_cfg_lef->rnr_center_h = pSplit->pic_rect_.w / 2;
    ynr_cfg_lef->rnr_center_v = pSplit->left_isp_rect_.h / 2;

    // right half pic center pixel calculate
    ynr_cfg_rht->rnr_center_h = (pSplit->right_isp_rect_.w - pSplit->pic_rect_.w / 2);
    ynr_cfg_rht->rnr_center_v = pSplit->right_isp_rect_.h / 2;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp35SplitAynrParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                        struct isp35_isp_params_cfg* ori,
                                        struct isp35_isp_params_cfg* left,
                                        struct isp35_isp_params_cfg* right) {
    struct isp35_ynr_cfg* ynr_cfg_ori = &ori->others.ynr_cfg;
    struct isp35_ynr_cfg* ynr_cfg_lef = &left->others.ynr_cfg;
    struct isp35_ynr_cfg* ynr_cfg_rht = &right->others.ynr_cfg;

    // left half pic center pixel calculate
    ynr_cfg_lef->rnr_center_h = pSplit->pic_rect_.w / 2;
    ynr_cfg_lef->rnr_center_v = pSplit->pic_rect_.h / 2;

    // right half pic center pixel calculate
    ynr_cfg_rht->rnr_center_h = (pSplit->right_isp_rect_.w - pSplit->pic_rect_.w / 2);
    ynr_cfg_rht->rnr_center_v = (pSplit->bottom_right_isp_rect_.h - pSplit->pic_rect_.h / 2);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp35SplitAsharpParams(AiqIspParamsSplitter_t* pSplit, struct isp35_isp_params_cfg* ori,
                                  struct isp35_isp_params_cfg* left,
                                  struct isp35_isp_params_cfg* right) {
    struct isp35_sharp_cfg* sharp_cfg_ori = &ori->others.sharp_cfg;
    struct isp35_sharp_cfg* sharp_cfg_lef = &left->others.sharp_cfg;
    struct isp35_sharp_cfg* sharp_cfg_rht = &right->others.sharp_cfg;

    // left half pic center pixel calculate
    sharp_cfg_lef->center_x = pSplit->pic_rect_.w / 2;
    sharp_cfg_lef->center_y = pSplit->left_isp_rect_.h / 2;

    // right half pic center pixel calculate
    sharp_cfg_rht->center_x = (pSplit->right_isp_rect_.w - pSplit->pic_rect_.w / 2);
    sharp_cfg_rht->center_y = pSplit->right_isp_rect_.h / 2;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp35SplitAsharpParamsVertical(AiqIspParamsSplitter_t* pSplit, struct isp35_isp_params_cfg* ori,
                                          struct isp35_isp_params_cfg* left,
                                          struct isp35_isp_params_cfg* right) {
    struct isp35_sharp_cfg* sharp_cfg_ori = &ori->others.sharp_cfg;
    struct isp35_sharp_cfg* sharp_cfg_lef = &left->others.sharp_cfg;
    struct isp35_sharp_cfg* sharp_cfg_rht = &right->others.sharp_cfg;

    // left half pic center pixel calculate
    sharp_cfg_lef->center_x = pSplit->pic_rect_.w / 2;
    sharp_cfg_lef->center_y = pSplit->left_isp_rect_.h / 2;

    // right half pic center pixel calculate
    sharp_cfg_rht->center_x = (pSplit->right_isp_rect_.w - pSplit->pic_rect_.w / 2);
    sharp_cfg_rht->center_y = (pSplit->bottom_right_isp_rect_.h - pSplit->pic_rect_.h / 2);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp35SplitAfParams(AiqIspParamsSplitter_t* pSplit, struct isp35_isp_params_cfg* ori,
                              struct isp35_isp_params_cfg* left,
                              struct isp35_isp_params_cfg* right) {
    struct isp35_rawaf_meas_cfg org_af = left->meas.rawaf;
    struct isp35_rawaf_meas_cfg* l_af  = &left->meas.rawaf;
    struct isp35_rawaf_meas_cfg* r_af  = &right->meas.rawaf;

    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t x_st, x_ed, l_blknum, r_blknum, ov_w, blk_w, r_skip_blknum;

    ov_w     = pSplit->left_isp_rect_.w + pSplit->left_isp_rect_.x - pSplit->right_isp_rect_.x;
    x_st     = org_af.win[0].h_offs;
    x_ed     = x_st + org_af.win[0].h_size;
    l_isp_st = pSplit->left_isp_rect_.x;
    l_isp_ed = pSplit->left_isp_rect_.x + pSplit->left_isp_rect_.w;
    r_isp_st = pSplit->right_isp_rect_.x;
    r_isp_ed = pSplit->right_isp_rect_.x + pSplit->right_isp_rect_.w;
    LOGD_AF("wina.x_st %d, wina.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d", x_st,
            x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    //// winA ////
    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        // af win < one isp width
        if (org_af.win[0].h_size < pSplit->left_isp_rect_.w) {
            blk_w    = org_af.win[0].h_size / ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum = (l_isp_ed - x_st + blk_w - 1) / blk_w;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW - l_blknum;
            l_win_ed = l_isp_ed - 2;
            l_win_st = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            if (blk_w < ov_w) {
                r_skip_blknum = ov_w / blk_w;
                r_win_st      = ov_w - r_skip_blknum * blk_w;
                r_win_ed      = ov_w + (ISP2X_RAWAF_SUMDATA_ROW - r_skip_blknum) * blk_w;
            } else {
                r_skip_blknum = 0;
                r_win_st      = 2;
                r_win_ed      = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;
            }
        }
        // af win < one isp width * 1.5
        else if (org_af.win[0].h_size < pSplit->left_isp_rect_.w * 3 / 2) {
            l_win_st = x_st;
            l_win_ed = l_isp_ed - 2;
            blk_w    = (l_win_ed - l_win_st) / (ISP2X_RAWAF_SUMDATA_ROW + 1);
            l_win_st = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum =
                ((l_win_ed - l_win_st) * ISP2X_RAWAF_SUMDATA_ROW + org_af.win[0].h_size - 1) /
                org_af.win[0].h_size;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW - l_blknum;
            if (blk_w < ov_w) {
                r_skip_blknum = ov_w / blk_w;
                r_win_st      = ov_w - r_skip_blknum * blk_w;
                r_win_ed      = ov_w + (ISP2X_RAWAF_SUMDATA_ROW - r_skip_blknum) * blk_w;
            } else {
                r_skip_blknum = 0;
                r_win_st      = 2;
                r_win_ed      = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_w;
            }
        } else {
            l_win_st      = x_st;
            l_win_ed      = l_isp_ed - 2;
            blk_w         = (l_win_ed - l_win_st) / ISP2X_RAWAF_SUMDATA_ROW;
            l_win_st      = l_win_ed - blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            r_win_st      = 2;
            r_win_ed      = r_win_st + blk_w * ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum      = ISP2X_RAWAF_SUMDATA_ROW;
            r_blknum      = ISP2X_RAWAF_SUMDATA_ROW;
            r_skip_blknum = 0;
        }
        LOGD_AF("wina: blk_w %d, ov_w %d, l_blknum %d, r_blknum %d, r_skip_blknum %d", blk_w, ov_w,
                l_blknum, r_blknum, r_skip_blknum);
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        l_blknum = 0;
        r_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        r_win_st = x_st - pSplit->right_isp_rect_.x;
        r_win_ed = x_ed - pSplit->right_isp_rect_.x;
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
    LOGD_AF("winb.x_st %d, winb.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d", x_st,
            x_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    // af win in both side
    if ((x_st < r_isp_st) && (x_ed > l_isp_ed)) {
        l_win_st = x_st;
        l_win_ed = l_isp_ed - 2;
        r_win_st = ov_w - 2;
        r_win_ed = x_ed - pSplit->right_isp_rect_.x;
    }
    // af win in right side
    else if ((x_st >= r_isp_st) && (x_ed > l_isp_ed)) {
        r_win_st = x_st - pSplit->right_isp_rect_.x;
        r_win_ed = x_ed - pSplit->right_isp_rect_.x;
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

    LOGD_AF("AfWinA left=%d-%d-%d-%d, right=%d-%d-%d-%d", l_af->win[0].h_offs, l_af->win[0].v_offs,
            l_af->win[0].h_size, l_af->win[0].v_size, r_af->win[0].h_offs, r_af->win[0].v_offs,
            r_af->win[0].h_size, r_af->win[0].v_size);

    LOGD_AF("AfWinB left=%d-%d-%d-%d, right=%d-%d-%d-%d", l_af->win[1].h_offs, l_af->win[1].v_offs,
            l_af->win[1].h_size, l_af->win[1].v_size, r_af->win[1].h_offs, r_af->win[1].v_offs,
            r_af->win[1].h_size, r_af->win[1].v_size);

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn isp35SplitAfParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                      struct isp35_isp_params_cfg* ori,
                                      struct isp35_isp_params_cfg* left,
                                      struct isp35_isp_params_cfg* right) {
    struct isp35_rawaf_meas_cfg org_af      = left->meas.rawaf;
    struct isp35_rawaf_meas_cfg* l_af       = &left->meas.rawaf;
    struct isp35_rawaf_meas_cfg* r_af       = &right->meas.rawaf;
    int32_t l_isp_st, l_isp_ed, r_isp_st, r_isp_ed;
    int32_t l_win_st, l_win_ed, r_win_st, r_win_ed;
    int32_t y_st, y_ed, l_blknum, r_blknum, ov_h, blk_h, r_skip_blknum;

    ov_h = pSplit->left_isp_rect_.h + pSplit->left_isp_rect_.y - pSplit->bottom_left_isp_rect_.y;
    y_st = org_af.win[0].v_offs;
    y_ed = y_st + org_af.win[0].v_size;
    l_isp_st = pSplit->left_isp_rect_.y;
    l_isp_ed = pSplit->left_isp_rect_.y + pSplit->left_isp_rect_.h;
    r_isp_st = pSplit->bottom_left_isp_rect_.y;
    r_isp_ed = pSplit->bottom_left_isp_rect_.y + pSplit->bottom_left_isp_rect_.h;
    LOGD_AF("wina.x_st %d, wina.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d", y_st,
            y_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    //// winA ////
    // af win in both side
    if ((y_st < r_isp_st) && (y_ed > l_isp_ed)) {
        // af win < one isp width
        if (org_af.win[0].v_size < pSplit->left_isp_rect_.h) {
            blk_h    = org_af.win[0].v_size / ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum = (l_isp_ed - y_st + blk_h - 1) / blk_h;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW - l_blknum;
            l_win_ed = l_isp_ed - 4;
            l_win_st = l_win_ed - blk_h * ISP2X_RAWAF_SUMDATA_ROW;
            if (blk_h < ov_h) {
                r_skip_blknum = ov_h / blk_h;
                r_win_st      = ov_h - r_skip_blknum * blk_h;
                r_win_ed      = ov_h + (ISP2X_RAWAF_SUMDATA_ROW - r_skip_blknum) * blk_h;
            } else {
                r_skip_blknum = 0;
                r_win_st      = 2;
                r_win_ed      = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_h;
            }
        }
        // af win < one isp width * 1.5
        else if (org_af.win[0].v_size < pSplit->left_isp_rect_.h * 3 / 2) {
            l_win_st = y_st;
            l_win_ed = l_isp_ed - 4;
            blk_h    = (l_win_ed - l_win_st) / (ISP2X_RAWAF_SUMDATA_ROW + 1);
            l_win_st = l_win_ed - blk_h * ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum =
                ((l_win_ed - l_win_st) * ISP2X_RAWAF_SUMDATA_ROW + org_af.win[0].v_size - 1) / org_af.win[0].v_size;
            r_blknum = ISP2X_RAWAF_SUMDATA_ROW - l_blknum;
            if (blk_h < ov_h) {
                r_skip_blknum = ov_h / blk_h;
                r_win_st      = ov_h - r_skip_blknum * blk_h;
                r_win_ed      = ov_h + (ISP2X_RAWAF_SUMDATA_ROW - r_skip_blknum) * blk_h;
            } else {
                r_skip_blknum = 0;
                r_win_st      = 2;
                r_win_ed      = r_win_st + ISP2X_RAWAF_SUMDATA_ROW * blk_h;
            }
        } else {
            l_win_st      = y_st;
            l_win_ed      = l_isp_ed - 4;
            blk_h         = (l_win_ed - l_win_st) / ISP2X_RAWAF_SUMDATA_ROW;
            l_win_st      = l_win_ed - blk_h * ISP2X_RAWAF_SUMDATA_ROW;
            r_win_st      = 2;
            r_win_ed      = r_win_st + blk_h * ISP2X_RAWAF_SUMDATA_ROW;
            l_blknum      = ISP2X_RAWAF_SUMDATA_ROW;
            r_blknum      = ISP2X_RAWAF_SUMDATA_ROW;
            r_skip_blknum = 0;
        }
        LOGD_AF("wina: blk_w %d, ov_w %d, t_blknum %d, b_blknum %d, b_skip_blknum %d", blk_h, ov_h,
                l_blknum, r_blknum, r_skip_blknum);
    }
    // af win in right side
    else if ((y_st >= r_isp_st) && (y_ed > l_isp_ed)) {
        l_blknum = 0;
        r_blknum = ISP2X_RAWAF_SUMDATA_ROW;
        r_win_st = y_st - pSplit->bottom_left_isp_rect_.y;
        r_win_ed = y_ed - pSplit->bottom_left_isp_rect_.y;
        l_win_st = r_win_st;
        l_win_ed = r_win_ed;
    }
    // af win in left side
    else {
        l_blknum = ISP2X_RAWAF_SUMDATA_ROW;
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
    LOGD_AF("winb.x_st %d, winb.x_ed %d, l_isp_st %d, l_isp_ed %d, r_isp_st %d, r_isp_ed %d", y_st,
            y_ed, l_isp_st, l_isp_ed, r_isp_st, r_isp_ed);

    // af win in both side
    if ((y_st < r_isp_st) && (y_ed > l_isp_ed)) {
        l_win_st = y_st;
        l_win_ed = l_isp_ed - 4;
        r_win_st = ov_h - 2;
        r_win_ed = y_ed - pSplit->bottom_left_isp_rect_.y;
    }
    // af win in right side
    else if ((y_st >= r_isp_st) && (y_ed > l_isp_ed)) {
        r_win_st = y_st - pSplit->bottom_left_isp_rect_.y;
        r_win_ed = y_ed - pSplit->bottom_left_isp_rect_.y;
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

    LOGD_AF("AfWinA top=%d-%d-%d-%d, bottom=%d-%d-%d-%d", l_af->win[0].h_offs, l_af->win[0].v_offs,
            l_af->win[0].h_size, l_af->win[0].v_size, r_af->win[0].h_offs, r_af->win[0].v_offs,
            r_af->win[0].h_size, r_af->win[0].v_size);

    LOGD_AF("AfWinB top=%d-%d-%d-%d, bottom=%d-%d-%d-%d", l_af->win[1].h_offs, l_af->win[1].v_offs,
            l_af->win[1].h_size, l_af->win[1].v_size, r_af->win[1].h_offs, r_af->win[1].v_offs,
            r_af->win[1].h_size, r_af->win[1].v_size);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn Isp35SplitIspParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                       struct isp35_isp_params_cfg* ori,
                                       struct isp35_isp_params_cfg* top,
                                       struct isp35_isp_params_cfg* bottom) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // struct isp33_cac_cfg cac_right;
    // struct isp32_ldch_cfg ldch_right;
    // memcpy(&cac_right, &right_isp_params->others.cac_cfg, sizeof(struct isp33_cac_cfg));
    // memcpy(&ldch_right, &right_isp_params->others.ldch_cfg, sizeof(struct isp32_ldch_cfg));

    // Modules that use the same params for both left and right isp
    // will not need to implent split function
    memcpy(bottom, top, sizeof(struct isp35_isp_params_cfg));

    // memcpy(&right_isp_params->others.cac_cfg, &cac_right, sizeof(struct isp33_cac_cfg));
    // memcpy(&right_isp_params->others.ldch_cfg, &ldch_right, sizeof(struct isp32_ldch_cfg));

    ret = isp35SplitAecParamsVertical(pSplit, ori, top, bottom);
    // Should return failure ?
    if (ori->module_cfg_update & ISP35_MODULE_RAWAWB)
        ret = isp35SplitAwbParamsVertical(pSplit, ori, top, bottom);
    if (ori->module_cfg_update & ISP35_MODULE_LSC)
        ret = isp35SplitAlscParamsVertical(pSplit, ori, top, bottom);
    if (ori->module_cfg_update & ISP35_MODULE_YNR)
        ret = isp35SplitAynrParamsVertical(pSplit, ori, top, bottom);
    if (ori->module_cfg_update & ISP35_MODULE_SHARP)
        ret = isp35SplitAsharpParamsVertical(pSplit, ori, top, bottom);
    if (ori->module_cfg_update & ISP35_MODULE_RAWAF)
        ret = isp35SplitAfParamsVertical(pSplit, ori, top, bottom);
    LOGD_CAMHW("Split ISP Params Vertical: top %p bottom %p size %d", top, bottom,
               sizeof(*top));

    return ret;
}

XCamReturn Isp35SplitIspParams(AiqIspParamsSplitter_t* pSplit, void* orig_isp_params,
                               void* isp_params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct isp35_isp_params_cfg* left_isp_params  = (struct isp35_isp_params_cfg*)isp_params;
    struct isp35_isp_params_cfg* right_isp_params = (struct isp35_isp_params_cfg*)isp_params + 1;

    struct isp33_cac_cfg cac_right;
    struct isp32_ldch_cfg ldch_right;
    memcpy(&cac_right, &right_isp_params->others.cac_cfg, sizeof(struct isp33_cac_cfg));
    memcpy(&ldch_right, &right_isp_params->others.ldch_cfg, sizeof(struct isp32_ldch_cfg));

    // Modules that use the same params for both left and right isp
    // will not need to implent split function
    memcpy(right_isp_params, left_isp_params, sizeof(struct isp35_isp_params_cfg));

    memcpy(&right_isp_params->others.cac_cfg, &cac_right, sizeof(struct isp33_cac_cfg));
    memcpy(&right_isp_params->others.ldch_cfg, &ldch_right, sizeof(struct isp32_ldch_cfg));

    ret = isp35SplitAecParams(pSplit, (struct isp35_isp_params_cfg*)orig_isp_params,
                              left_isp_params, right_isp_params);
    // Should return failure ?
    if (((struct isp35_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP35_MODULE_RAWAWB)
        ret = isp35SplitAwbParams(pSplit, (struct isp35_isp_params_cfg*)orig_isp_params,
                                  left_isp_params, right_isp_params);
    if (((struct isp35_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP35_MODULE_LSC)
        ret = isp35SplitAlscParams(pSplit, (struct isp35_isp_params_cfg*)orig_isp_params,
                                   left_isp_params, right_isp_params);
    if (((struct isp35_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP35_MODULE_YNR)
        ret = isp35SplitAynrParams(pSplit, (struct isp35_isp_params_cfg*)orig_isp_params,
                                   left_isp_params, right_isp_params);
    if (((struct isp35_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP35_MODULE_SHARP)
        ret = isp35SplitAsharpParams(pSplit, (struct isp35_isp_params_cfg*)orig_isp_params,
                                     left_isp_params, right_isp_params);
    if (((struct isp35_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP35_MODULE_RAWAF)
        ret = isp35SplitAfParams(pSplit, (struct isp35_isp_params_cfg*)orig_isp_params,
                                 left_isp_params, right_isp_params);

    if (pSplit->isp_unite_mode_ == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
        struct isp35_isp_params_cfg* bottom_left_isp_params  = (struct isp35_isp_params_cfg*)isp_params + 2;
        struct isp35_isp_params_cfg* bottom_right_isp_params = (struct isp35_isp_params_cfg*)isp_params + 3;

        struct isp35_isp_params_cfg tmp_orig_params;
        tmp_orig_params = *left_isp_params;
        ret = Isp35SplitIspParamsVertical(pSplit, &tmp_orig_params, left_isp_params, bottom_left_isp_params);
        tmp_orig_params = *right_isp_params;
        ret = Isp35SplitIspParamsVertical(pSplit, &tmp_orig_params, right_isp_params, bottom_right_isp_params);
    }
    LOGD_CAMHW("Split ISP Params: left %p right %p size %d", left_isp_params, right_isp_params,
               sizeof(*left_isp_params));

    return ret;
}
