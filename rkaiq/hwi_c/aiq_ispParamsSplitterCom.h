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
#ifndef __AIQ_ISP_PARAMS_SPLITTER_COM_H__
#define __AIQ_ISP_PARAMS_SPLITTER_COM_H__

#include "include/common/rk_isp20_hw.h"
#include "include/common/rkisp2-config.h"
#include "xcam_common.h"

#include "algos/ae/rk_aiq_types_ae_hw.h"

typedef struct AiqIspParamsSplitter_s AiqIspParamsSplitter_t;
typedef struct Splitter_Rectangle_s Splitter_Rectangle_t;

void SplitAecWin(struct isp2x_window* ori_win, struct isp2x_window* left_win,
                 struct isp2x_window* right_win, u8 wnd_num,
                 Splitter_Rectangle_t left_isp_rect_, Splitter_Rectangle_t right_isp_rect_,
                 WinSplitMode* mode);

void SplitAecWinVertical(struct isp2x_window* ori_win, struct isp2x_window* left_win,
                         struct isp2x_window* right_win, u8 wnd_num,
                         Splitter_Rectangle_t left_isp_rect_,
                         Splitter_Rectangle_t right_isp_rect_, WinSplitMode* mode);

void AiqIspParamsSplitter_SplitAecWeight(u8* ori_weight, u8* left_weight, u8* right_weight,
                                         WinSplitMode mode, u8 wnd_num);

void AiqIspParamsSplitter_SplitAecWeightVertical(u8* ori_weight, u8* left_weight, u8* right_weight,
                                                 WinSplitMode mode, u8 wnd_num);

void AiqIspParamsSplitter_SplitAwbWin(struct isp2x_window* ori_win, struct isp2x_window* left_win,
                                      struct isp2x_window* right_win, u8 ds_awb, u8 wnd_num,
                                      Splitter_Rectangle_t left_isp_rect_,
                                      Splitter_Rectangle_t right_isp_rect_, WinSplitMode* mode);

void AiqIspParamsSplitter_SplitAwbMultiWin(
    struct isp2x_window* ori_win, struct isp2x_window* left_win, struct isp2x_window* right_win,
    struct isp2x_window* main_left_win, struct isp2x_window* main_right_win,
    Splitter_Rectangle_t left_isp_rect_, Splitter_Rectangle_t right_isp_rect_, WinSplitMode* mode);

void AiqIspParamsSplitter_SplitAwbWinVertical(struct isp2x_window* ori_win,
                                              struct isp2x_window* left_win,
                                              struct isp2x_window* right_win, u8 ds_awb, u8 wnd_num,
                                              Splitter_Rectangle_t left_isp_rect_,
                                              Splitter_Rectangle_t right_isp_rect_,
                                              WinSplitMode* mode);

void AiqIspParamsSplitter_AiqIspParamsSplitter_SplitAwbMultiWinVertical(
    struct isp2x_window* ori_win, struct isp2x_window* left_win, struct isp2x_window* right_win,
    struct isp2x_window* main_left_win, struct isp2x_window* main_right_win,
    Splitter_Rectangle_t left_isp_rect_, Splitter_Rectangle_t right_isp_rect_, WinSplitMode* mode);

XCamReturn AiqIspParamsSplitter_SplitRawAeLiteParams(AiqIspParamsSplitter_t* pSplit,
                                                     struct isp2x_rawaelite_meas_cfg* ori,
                                                     struct isp2x_rawaelite_meas_cfg* left,
                                                     struct isp2x_rawaelite_meas_cfg* right);

XCamReturn AiqIspParamsSplitter_SplitRawAeBigParams(AiqIspParamsSplitter_t* pSplit,
                                                    struct isp2x_rawaebig_meas_cfg* ori,
                                                    struct isp2x_rawaebig_meas_cfg* left,
                                                    struct isp2x_rawaebig_meas_cfg* right);

XCamReturn AiqIspParamsSplitter_SplitRawHistLiteParams(AiqIspParamsSplitter_t* pSplit,
                                                       struct isp2x_rawhistlite_cfg* ori,
                                                       struct isp2x_rawhistlite_cfg* left,
                                                       struct isp2x_rawhistlite_cfg* right);

XCamReturn AiqIspParamsSplitter_SplitRawHistBigParams(AiqIspParamsSplitter_t* pSplit,
                                                      struct isp2x_rawhistbig_cfg* ori,
                                                      struct isp2x_rawhistbig_cfg* left,
                                                      struct isp2x_rawhistbig_cfg* right);

int AiqIspParamsSplitter_AlscMatrixScale(float lrate, int lmid_col_idx, float rrate, int rmid_col_idx,
                                         unsigned short ori_matrix[], unsigned short left_matrix[],
                                         unsigned short right_matrix[], int cols, int rows);

int AiqIspParamsSplitter_AlscMatrixScaleVertical(float trate, int tmid_row_idx,
                                                 float brate, int bmid_row_idx,
                                                 unsigned short ori_matrix[],
                                                 unsigned short top_matrix[],
                                                 unsigned short btm_matrix[],
                                                 int rows, int cols);

int AiqIspParamsSplitter_SplitAlscXtable(const unsigned short* in_array, int in_size,
                                         unsigned short* dst_left, unsigned short* dst_right);

int AiqIspParamsSplitter_LscGradUpdate(unsigned short xgrad_tbl[], unsigned short ygrad_tbl[],
                                       unsigned short x_sect_tbl[], unsigned short y_sect_tbl[],
                                       int x_sect_size, int y_sect_size);

XCamReturn AiqIspParamsSplitter_SplitRawAeLiteParamsVertical(
    AiqIspParamsSplitter_t* pSplit, struct isp2x_rawaelite_meas_cfg* ori,
    struct isp2x_rawaelite_meas_cfg* left, struct isp2x_rawaelite_meas_cfg* right);

XCamReturn AiqIspParamsSplitter_SplitRawAeBigParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                                            struct isp2x_rawaebig_meas_cfg* ori,
                                                            struct isp2x_rawaebig_meas_cfg* left,
                                                            struct isp2x_rawaebig_meas_cfg* right);

XCamReturn AiqIspParamsSplitter_SplitRawHistLiteParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                                               struct isp2x_rawhistlite_cfg* ori,
                                                               struct isp2x_rawhistlite_cfg* left,
                                                               struct isp2x_rawhistlite_cfg* right);

XCamReturn AiqIspParamsSplitter_SplitRawHistBigParamsVertical(AiqIspParamsSplitter_t* pSplit,
                                                              struct isp2x_rawhistbig_cfg* ori,
                                                              struct isp2x_rawhistbig_cfg* left,
                                                              struct isp2x_rawhistbig_cfg* right);

#endif  // __AIQ_ISP_PARAMS_SPLITTER_COM_H__
