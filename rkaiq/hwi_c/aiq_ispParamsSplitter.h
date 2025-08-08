
/*
 * IspParamsSplitter.h - Split ISP params to Left/Right ISP params
 *
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */

#ifndef __AIQ_COMMON_HWI_ISP_PARAMS_SPLITTER_H__
#define __AIQ_COMMON_HWI_ISP_PARAMS_SPLITTER_H__

#include "xcam_common.h"
#include "xcam_log.h"

typedef struct Splitter_Rectangle_s {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
} Splitter_Rectangle_t;

typedef struct AiqIspParamsSplitter_s AiqIspParamsSplitter_t;

struct AiqIspParamsSplitter_s {
    Splitter_Rectangle_t pic_rect_;
    Splitter_Rectangle_t left_isp_rect_;
    Splitter_Rectangle_t right_isp_rect_;
    Splitter_Rectangle_t bottom_left_isp_rect_;
    Splitter_Rectangle_t bottom_right_isp_rect_;
    int                  isp_unite_mode_;

    XCamReturn (*SplitIspParams)(AiqIspParamsSplitter_t* pSplit, void* orig_isp_params,
                                 void* isp_params);
    XCamReturn (*SplitIspParamsVertical)(AiqIspParamsSplitter_t* pSplit, void* orig_isp_params,
                                         void* isp_params);
};

XCamReturn AiqIspParamsSplitter_init(AiqIspParamsSplitter_t* pSplit, int ispVer);
void AiqIspParamsSplitter_deinit(AiqIspParamsSplitter_t* pSplit);

AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetPicInfo(AiqIspParamsSplitter_t* pSplit,
                                                        Splitter_Rectangle_t* pic_rect);
AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetLeftIspRect(AiqIspParamsSplitter_t* pSplit,
                                                            Splitter_Rectangle_t* left_isp_rect);
AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetRightIspRect(AiqIspParamsSplitter_t* pSplit,
                                                             Splitter_Rectangle_t* right_isp_rect);
AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetBottomLeftIspRect(
    AiqIspParamsSplitter_t* pSplit, Splitter_Rectangle_t* bottom_left_isp_rect);
AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetBottomRightIspRect(
    AiqIspParamsSplitter_t* pSplit, Splitter_Rectangle_t* bottom_right_isp_rect);

const Splitter_Rectangle_t* AiqIspParamsSplitter_GetPicInfo(AiqIspParamsSplitter_t* pSplit);
const Splitter_Rectangle_t* AiqIspParamsSplitter_GetLeftIspRect(AiqIspParamsSplitter_t* pSplit);
const Splitter_Rectangle_t* AiqIspParamsSplitter_GetRightIspRect(AiqIspParamsSplitter_t* pSplit);

int AiqIspParamsSplitter_GetIspUniteMode(AiqIspParamsSplitter_t* pSplit);
XCamReturn AiqIspParamsSplitter_SetIspUniteMode(AiqIspParamsSplitter_t* pSplit, int mode);

#endif  // __COMMON_HWI_ISP_PARAMS_SPLITTER_H__
