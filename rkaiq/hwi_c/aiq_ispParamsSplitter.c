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

#include "aiq_ispParamsSplitter.h"

#if defined(ISP_HW_V30)
#include "hwi_c/isp3x/aiq_isp3xParamsSplitter.h"
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
#include "hwi_c/isp32/aiq_isp32ParamsSplitter.h"
#elif defined(ISP_HW_V39)
#include "hwi_c/isp39/aiq_isp39ParamsSplitter.h"
#elif defined(ISP_HW_V33)
#include "hwi_c/isp33/aiq_isp33ParamsSplitter.h"
#elif defined(ISP_HW_V35)
#include "hwi_c/isp35/aiq_isp35ParamsSplitter.h"
#endif

//#define DEBUG

XCamReturn AiqIspParamsSplitter_init(AiqIspParamsSplitter_t* pSplit, int ispVer) {
    if (!pSplit) return XCAM_RETURN_ERROR_PARAM;

    pSplit->SplitIspParams         = NULL;
    pSplit->SplitIspParamsVertical = NULL;
    pSplit->isp_unite_mode_        = 0;

    memset(&pSplit->left_isp_rect_, 0, sizeof(pSplit->bottom_left_isp_rect_));
    memset(&pSplit->right_isp_rect_, 0, sizeof(pSplit->right_isp_rect_));
    memset(&pSplit->bottom_left_isp_rect_, 0, sizeof(pSplit->bottom_left_isp_rect_));
    memset(&pSplit->bottom_right_isp_rect_, 0, sizeof(pSplit->bottom_right_isp_rect_));
    memset(&pSplit->pic_rect_, 0, sizeof(pSplit->pic_rect_));

#if defined(ISP_HW_V30)
    pSplit->SplitIspParams = Isp3xSplitIspParams;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    pSplit->SplitIspParams         = Isp32SplitIspParams;
    pSplit->SplitIspParamsVertical = Isp32SplitIspParamsVertical;
#elif defined(ISP_HW_V39)
    pSplit->SplitIspParams         = Isp39SplitIspParams;
#elif defined(ISP_HW_V33)
    pSplit->SplitIspParams         = Isp33SplitIspParams;
#elif defined(ISP_HW_V35)
    pSplit->SplitIspParams         = Isp35SplitIspParams;
#endif

    return XCAM_RETURN_NO_ERROR;
}

void AiqIspParamsSplitter_deinit(AiqIspParamsSplitter_t* pSplit) {}

AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetPicInfo(AiqIspParamsSplitter_t* pSplit,
                                                        Splitter_Rectangle_t* pic_rect) {
    pSplit->pic_rect_ = *pic_rect;
    return pSplit;
}

AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetLeftIspRect(AiqIspParamsSplitter_t* pSplit,
                                                            Splitter_Rectangle_t* left_isp_rect) {
    pSplit->left_isp_rect_ = *left_isp_rect;
    return pSplit;
}

AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetRightIspRect(AiqIspParamsSplitter_t* pSplit,
                                                             Splitter_Rectangle_t* right_isp_rect) {
    pSplit->right_isp_rect_ = *right_isp_rect;
    return pSplit;
}

AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetBottomLeftIspRect(
    AiqIspParamsSplitter_t* pSplit, Splitter_Rectangle_t* bottom_left_isp_rect) {
    pSplit->bottom_left_isp_rect_ = *bottom_left_isp_rect;
    return pSplit;
}

AiqIspParamsSplitter_t* AiqIspParamsSplitter_SetBottomRightIspRect(
    AiqIspParamsSplitter_t* pSplit, Splitter_Rectangle_t* bottom_right_isp_rect) {
    pSplit->bottom_right_isp_rect_ = *bottom_right_isp_rect;
    return pSplit;
}

const Splitter_Rectangle_t* AiqIspParamsSplitter_GetPicInfo(AiqIspParamsSplitter_t* pSplit) {
    return &pSplit->pic_rect_;
}

const Splitter_Rectangle_t* AiqIspParamsSplitter_GetLeftIspRect(AiqIspParamsSplitter_t* pSplit) {
    return &pSplit->left_isp_rect_;
}

const Splitter_Rectangle_t* AiqIspParamsSplitter_GetRightIspRect(AiqIspParamsSplitter_t* pSplit) {
    return &pSplit->right_isp_rect_;
}

int AiqIspParamsSplitter_GetIspUniteMode(AiqIspParamsSplitter_t* pSplit) {
    return pSplit->isp_unite_mode_;
}

XCamReturn AiqIspParamsSplitter_SetIspUniteMode(AiqIspParamsSplitter_t* pSplit, int mode) {
    pSplit->isp_unite_mode_ = mode;
    return XCAM_RETURN_NO_ERROR;
}
