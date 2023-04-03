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
#if RKAIQ_HAVE_DEHAZE_V12
    virtual XCamReturn translateAdehazeStats(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAdehazeStatsProxy>& to);
#endif

 private:
    XCAM_DEAD_COPY(RkAiqResourceTranslatorV32);
};

}  // namespace RkCam

#endif  //_RK_AIQ_CONFIG_TRANSLATOR_H_
