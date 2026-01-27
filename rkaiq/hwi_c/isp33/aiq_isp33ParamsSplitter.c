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

#include "hwi_c/isp33/aiq_isp33ParamsSplitter.h"

#include "algos/ae/rk_aiq_types_ae_hw.h"
#include "hwi_c/aiq_ispParamsSplitter.h"
#include "hwi_c/aiq_ispParamsSplitterCom.h"
#include "include/common/rk_isp20_hw.h"
#include "include/common/rk-isp33-config.h"

//#define DEBUG

XCamReturn isp33SplitAecParams(AiqIspParamsSplitter_t* pSplit, struct isp33_isp_params_cfg* ori,
                               struct isp33_isp_params_cfg* left,
                               struct isp33_isp_params_cfg* right) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // RAWAE
    if (ori->module_cfg_update & ISP33_MODULE_RAWAE0)
        ret = AiqIspParamsSplitter_SplitRawAeBigParams(pSplit, &ori->meas.rawae0,
                                                       &left->meas.rawae0, &right->meas.rawae0);
    if (ori->module_cfg_update & ISP33_MODULE_RAWAE3)
        ret = AiqIspParamsSplitter_SplitRawAeBigParams(pSplit, &ori->meas.rawae3,
                                                       &left->meas.rawae3, &right->meas.rawae3);

    // RAWHIST
    if (ori->module_cfg_update & ISP33_MODULE_RAWHIST0)
        ret = AiqIspParamsSplitter_SplitRawHistBigParams(
            pSplit, &ori->meas.rawhist0, &left->meas.rawhist0, &right->meas.rawhist0);
    if (ori->module_cfg_update & ISP33_MODULE_RAWHIST3)
        ret = AiqIspParamsSplitter_SplitRawHistBigParams(
            pSplit, &ori->meas.rawhist3, &left->meas.rawhist3, &right->meas.rawhist3);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp33SplitAwbParams(AiqIspParamsSplitter_t* pSplit, struct isp33_isp_params_cfg* ori,
                               struct isp33_isp_params_cfg* left,
                               struct isp33_isp_params_cfg* right) {
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

XCamReturn isp33SplitAlscParams(AiqIspParamsSplitter_t* pSplit, struct isp33_isp_params_cfg* ori,
                                struct isp33_isp_params_cfg* left,
                                struct isp33_isp_params_cfg* right) {
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

XCamReturn isp33SplitAynrParams(AiqIspParamsSplitter_t* pSplit, struct isp33_isp_params_cfg* ori,
                                struct isp33_isp_params_cfg* left,
                                struct isp33_isp_params_cfg* right) {
    struct isp33_ynr_cfg* ynr_cfg_ori = &ori->others.ynr_cfg;
    struct isp33_ynr_cfg* ynr_cfg_lef = &left->others.ynr_cfg;
    struct isp33_ynr_cfg* ynr_cfg_rht = &right->others.ynr_cfg;

    // left half pic center pixel calculate
    ynr_cfg_lef->rnr_center_h = pSplit->pic_rect_.w / 2;
    ynr_cfg_lef->rnr_center_v = pSplit->left_isp_rect_.h / 2;

    // right half pic center pixel calculate
    ynr_cfg_rht->rnr_center_h = (pSplit->right_isp_rect_.w - pSplit->pic_rect_.w / 2);
    ynr_cfg_rht->rnr_center_v = pSplit->right_isp_rect_.h / 2;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn isp33SplitAsharpParams(AiqIspParamsSplitter_t* pSplit, struct isp33_isp_params_cfg* ori,
                                  struct isp33_isp_params_cfg* left,
                                  struct isp33_isp_params_cfg* right) {
    struct isp33_sharp_cfg* sharp_cfg_ori = &ori->others.sharp_cfg;
    struct isp33_sharp_cfg* sharp_cfg_lef = &left->others.sharp_cfg;
    struct isp33_sharp_cfg* sharp_cfg_rht = &right->others.sharp_cfg;

    // left half pic center pixel calculate
    sharp_cfg_lef->center_x = pSplit->pic_rect_.w / 2;
    sharp_cfg_lef->center_y = pSplit->left_isp_rect_.h / 2;

    // right half pic center pixel calculate
    sharp_cfg_rht->center_x = (pSplit->right_isp_rect_.w - pSplit->pic_rect_.w / 2);
    sharp_cfg_rht->center_y = pSplit->right_isp_rect_.h / 2;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn Isp33SplitIspParams(AiqIspParamsSplitter_t* pSplit, void* orig_isp_params,
                               void* isp_params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct isp33_isp_params_cfg* left_isp_params  = (struct isp33_isp_params_cfg*)isp_params;
    struct isp33_isp_params_cfg* right_isp_params = (struct isp33_isp_params_cfg*)isp_params + 1;

    struct isp33_cac_cfg cac_right;
    struct isp32_ldch_cfg ldch_right;
    memcpy(&cac_right, &right_isp_params->others.cac_cfg, sizeof(struct isp33_cac_cfg));
    memcpy(&ldch_right, &right_isp_params->others.ldch_cfg, sizeof(struct isp32_ldch_cfg));

    // Modules that use the same params for both left and right isp
    // will not need to implent split function
    memcpy(right_isp_params, left_isp_params, sizeof(struct isp33_isp_params_cfg));

    memcpy(&right_isp_params->others.cac_cfg, &cac_right, sizeof(struct isp33_cac_cfg));
    memcpy(&right_isp_params->others.ldch_cfg, &ldch_right, sizeof(struct isp32_ldch_cfg));

    ret = isp33SplitAecParams(pSplit, (struct isp33_isp_params_cfg*)orig_isp_params,
                              left_isp_params, right_isp_params);
    // Should return failure ?
    if (((struct isp33_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP33_MODULE_RAWAWB)
        ret = isp33SplitAwbParams(pSplit, (struct isp33_isp_params_cfg*)orig_isp_params,
                                  left_isp_params, right_isp_params);
    if (((struct isp33_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP33_MODULE_LSC)
        ret = isp33SplitAlscParams(pSplit, (struct isp33_isp_params_cfg*)orig_isp_params,
                                   left_isp_params, right_isp_params);
    if (((struct isp33_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP33_MODULE_YNR)
        ret = isp33SplitAynrParams(pSplit, (struct isp33_isp_params_cfg*)orig_isp_params,
                                   left_isp_params, right_isp_params);
    if (((struct isp33_isp_params_cfg*)orig_isp_params)->module_cfg_update & ISP33_MODULE_SHARP)
        ret = isp33SplitAsharpParams(pSplit, (struct isp33_isp_params_cfg*)orig_isp_params,
                                     left_isp_params, right_isp_params);
    LOGD_CAMHW("Split ISP Params: left %p right %p size %d", left_isp_params, right_isp_params,
               sizeof(*left_isp_params));

    return ret;
}
