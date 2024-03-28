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
#ifndef __COMMON_HWI_ISP_PARAMS_SPLITTER_H__
#define __COMMON_HWI_ISP_PARAMS_SPLITTER_H__

#include "xcam_common.h"
#include "xcam_log.h"


namespace RkCam {

class IspParamsSplitter {
public:
    struct Rectangle {
        uint32_t x;
        uint32_t y;
        uint32_t w;
        uint32_t h;
    };

    IspParamsSplitter()                         = default;
    ~IspParamsSplitter()                        = default;
    IspParamsSplitter(const IspParamsSplitter&) = delete;
    IspParamsSplitter& operator=(const IspParamsSplitter&) = delete;

    IspParamsSplitter& SetPicInfo(IspParamsSplitter::Rectangle&& pic_rect);
    IspParamsSplitter& SetLeftIspRect(IspParamsSplitter::Rectangle&& left_isp_rect);
    IspParamsSplitter& SetRightIspRect(IspParamsSplitter::Rectangle&& right_isp_rect);
    IspParamsSplitter& SetBottomLeftIspRect(IspParamsSplitter::Rectangle&& bottom_left_isp_rect);
    IspParamsSplitter& SetBottomRightIspRect(IspParamsSplitter::Rectangle&& bottom_right_isp_rect);
    IspParamsSplitter& SetPicInfo(IspParamsSplitter::Rectangle& pic_rect);
    IspParamsSplitter& SetLeftIspRect(IspParamsSplitter::Rectangle& left_isp_rect);
    IspParamsSplitter& SetRightIspRect(IspParamsSplitter::Rectangle& right_isp_rect);
    IspParamsSplitter& SetBottomLeftIspRect(IspParamsSplitter::Rectangle& bottom_left_isp_rect);
    IspParamsSplitter& SetBottomRightIspRect(IspParamsSplitter::Rectangle& bottom_right_isp_rect);
    const IspParamsSplitter::Rectangle& GetPicInfo() const;
    const IspParamsSplitter::Rectangle& GetLeftIspRect() const;
    const IspParamsSplitter::Rectangle& GetRightIspRect() const;

    template <typename U>
    XCamReturn SplitIspParams(U* orig_isp_params, U* isp_params);

    template <typename U>
    XCamReturn SplitIspParamsVertical(U* orig_isp_params, U* isp_params);

private:
    //ae
    template <typename U>
    XCamReturn SplitAecParams(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitRawHistLiteParams(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitRawHistBigParams(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitRawAeLiteParams(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitRawAeBigParams(U* ori, U* left, U* right);

    template <typename U>
    XCamReturn SplitAecParamsVertical(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitRawHistLiteParamsVertical(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitRawHistBigParamsVertical(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitRawAeLiteParamsVertical(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitRawAeBigParamsVertical(U* ori, U* left, U* right);

    //awb
    template <typename U>
    XCamReturn SplitAwbParamsVertical(U* ori, U* left, U* right);

    template <typename U>
    XCamReturn SplitAwbParams(U* ori, U* left, U* right);

    template <typename U>
    XCamReturn SplitAfParams(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitAfParamsVertical(U* ori, U* left, U* right);

    // LSC
    template <typename U>
    XCamReturn SplitAlscParams(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitAlscParamsVertical(U* ori, U* left, U* right);
    // ynr
    template <typename U>
    XCamReturn SplitAynrParams(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitAynrParamsVertical(U* ori, U* left, U* right);
    // sharp
    template <typename U>
    XCamReturn SplitAsharpParams(U* ori, U* left, U* right);
    template <typename U>
    XCamReturn SplitAsharpParamsVertical(U* ori, U* left, U* right);

    Rectangle pic_rect_;
    Rectangle left_isp_rect_;
    Rectangle right_isp_rect_;
    Rectangle bottom_left_isp_rect_;
    Rectangle bottom_right_isp_rect_;
};

}  // namespace RkCam

#endif  // __COMMON_HWI_ISP_PARAMS_SPLITTER_H__
