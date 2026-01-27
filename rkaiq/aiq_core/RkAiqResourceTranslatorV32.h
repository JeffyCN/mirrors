/*
 * Copyright (c) 2022 Rockchip Eletronics Co., Ltd.
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
#ifndef _RK_AIQ_RESOURCE_TRANSLATOR_V32_H_
#define _RK_AIQ_RESOURCE_TRANSLATOR_V32_H_

#include "RkAiqResourceTranslatorV3x.h"

#define ISP32_DHAZ_HIST_IIR_MAX (ISP3X_DHAZ_HIST_IIR_MAX)
#define ISP32_DHAZ_PIC_SUM_MIN  (ISP3X_DHAZ_PIC_SUM_MIN)

namespace RkCam {

class RkAiqResourceTranslatorV32 : public RkAiqResourceTranslatorV3x {
 public:
    RkAiqResourceTranslatorV32();
    virtual ~RkAiqResourceTranslatorV32() = default;

    virtual XCamReturn translateAecStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAecStatsProxy>& to);
    virtual XCamReturn translateAwbStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAwbStatsProxy>& to);
    virtual XCamReturn translateAfStats(const SmartPtr<VideoBuffer>& from,
                                        SmartPtr<RkAiqAfStatsProxy>& to);

#if defined(RKAIQ_HAVE_MULTIISP) && defined(ISP_HW_V32)
    virtual XCamReturn translateMultiAwbStats(const SmartPtr<VideoBuffer>& from,
                                              SmartPtr<RkAiqAwbStatsProxy>& to);
    virtual XCamReturn translateMultiAecStats(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAecStatsProxy>& to);
    virtual XCamReturn translateMultiAfStats(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAfStatsProxy>& to);
#if RKAIQ_HAVE_DEHAZE_V12
    virtual XCamReturn translateMultiAdehazeStats(const SmartPtr<VideoBuffer>& from,
                                                  SmartPtr<RkAiqAdehazeStatsProxy>& to);
#endif
#endif

#if defined(ISP_HW_V32)
    virtual XCamReturn translateAgainStats(const SmartPtr<VideoBuffer>& from,
                                        SmartPtr<RkAiqAgainStatsProxy>& to);
#endif
#if RKAIQ_HAVE_DEHAZE_V12
    virtual XCamReturn translateAdehazeStats(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAdehazeStatsProxy>& to);
#endif

#if defined(RKAIQ_HAVE_MULTIISP) && defined(ISP_HW_V32_LITE)
    XCamReturn translateMultiAecStatsV32Lite(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAecStatsProxy>& to);
    XCamReturn translateMultiAwbStatsV32Lite(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAwbStatsProxy>& to);
    XCamReturn translateMultiAfStatsV32Lite(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAfStatsProxy>& to);
#endif

    XCamReturn translateMultiAecStatsVertialV32Lite(SmartPtr<RkAiqAecStatsProxy> top,
        SmartPtr<RkAiqAecStatsProxy>& bot, SmartPtr<RkAiqAecStatsProxy>& to);

 private:
    XCAM_DEAD_COPY(RkAiqResourceTranslatorV32);

    bool mIsMultiIsp;
    Rectangle pic_rect_;
    Rectangle left_isp_rect_;
    Rectangle right_isp_rect_;
};

}  // namespace RkCam

#endif  //_RK_AIQ_CONFIG_TRANSLATOR_H_
