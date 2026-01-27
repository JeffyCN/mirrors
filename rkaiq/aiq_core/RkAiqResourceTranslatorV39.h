/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
#ifndef _RK_AIQ_RESOURCE_TRANSLATOR_V39_H_
#define _RK_AIQ_RESOURCE_TRANSLATOR_V39_H_

#include "RkAiqResourceTranslatorV32.h"

namespace RkCam {

class RkAiqResourceTranslatorV39 : public RkAiqResourceTranslatorV32 {
 public:
    RkAiqResourceTranslatorV39();
    virtual ~RkAiqResourceTranslatorV39() = default;

    virtual XCamReturn translateAecStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAecStatsProxy>& to);
    virtual XCamReturn translateAfStats(const SmartPtr<VideoBuffer>& from,
                                        SmartPtr<RkAiqAfStatsProxy>& to);
#if RKAIQ_HAVE_DEHAZE_V14
    virtual XCamReturn translateAdehazeStats(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAdehazeStatsProxy>& to);
#endif
    virtual XCamReturn translateAwbStats(const SmartPtr<VideoBuffer>& from,
            SmartPtr<RkAiqAwbStatsProxy>& to);
    XCamReturn translateBay3dStats(const SmartPtr<VideoBuffer>& from,
                                   std::list<SmartPtr<RkAiqBay3dStat>>& to, Mutex& mutex);

 private:
    XCAM_DEAD_COPY(RkAiqResourceTranslatorV39);
};

}  // namespace RkCam

#endif  //_RK_AIQ_CONFIG_TRANSLATOR_H_
