/*
 * RkAiqConfigTranslator.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_RESOURCE_TRANSLATOR_H_
#define _RK_AIQ_RESOURCE_TRANSLATOR_H_

#include "IRkAiqResourceTranslator.h"
#include "rk_aiq_types.h"

namespace RkCam {

class RkAiqResourceTranslator
    : public IRkAiqResourceTranslator
{
public:
    RkAiqResourceTranslator ()          = default;
    virtual ~RkAiqResourceTranslator () = default;

    virtual XCamReturn translateIspStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqIspStatsIntProxy>& to,
                                         const SmartPtr<RkAiqAecStatsProxy>& aecStat,
                                         const SmartPtr<RkAiqAwbStatsProxy>& awbStat,
                                         const SmartPtr<RkAiqAfStatsProxy>& afStat,
                                         const SmartPtr<RkAiqAtmoStatsProxy>& tmoStat,
                                         const SmartPtr<RkAiqAdehazeStatsProxy>& dehazeStat);
    virtual XCamReturn translateAecStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAecStatsProxy>& to);
    virtual XCamReturn translateAwbStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqAwbStatsProxy>& to);
    virtual XCamReturn translateAfStats(const SmartPtr<VideoBuffer>& from,
                                        SmartPtr<RkAiqAfStatsProxy>& to);
    virtual XCamReturn translateOrbStats(const SmartPtr<VideoBuffer>& from,
                                         SmartPtr<RkAiqOrbStatsProxy>& to);
    virtual XCamReturn translateAtmoStats(const SmartPtr<VideoBuffer>& from,
                                          SmartPtr<RkAiqAtmoStatsProxy>& to);
    virtual XCamReturn translateAdehazeStats(const SmartPtr<VideoBuffer>& from,
                                             SmartPtr<RkAiqAdehazeStatsProxy>& to);
    virtual XCamReturn translatePdafStats(const SmartPtr<VideoBuffer>& from,
                                          SmartPtr<RkAiqPdafStatsProxy>& to, bool sns_mirror);

 private:
    XCAM_DEAD_COPY (RkAiqResourceTranslator);
};

}

#endif //_RK_AIQ_CONFIG_TRANSLATOR_H_
