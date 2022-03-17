/*
 * IRkAiqConfigTranslator.h
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

#ifndef _I_RK_AIQ_RESOURCE_TRANSLATOR_H_
#define _I_RK_AIQ_RESOURCE_TRANSLATOR_H_

#include "smartptr.h"
#include "xcam_log.h"
#include "video_buffer.h"
#include "rk_aiq_pool.h"
#include "RkAiqSharedDataManager.h"

namespace RkCam {

class IRkAiqResourceTranslator {
public:
    IRkAiqResourceTranslator ()          {};
    virtual ~IRkAiqResourceTranslator () {};

    virtual XCamReturn translateIspStats (const SmartPtr<VideoBuffer> &from,
                                  SmartPtr<RkAiqIspStatsIntProxy> &to) = 0;
    virtual XCamReturn translateAecStats (const SmartPtr<VideoBuffer> &from,
                                  SmartPtr<RkAiqAecStatsProxy> &to) = 0;
    virtual XCamReturn translateAwbStats (const SmartPtr<VideoBuffer> &from,
                                  SmartPtr<RkAiqAwbStatsProxy> &to) = 0;
    virtual XCamReturn translateAfStats (const SmartPtr<VideoBuffer> &from,
                                 SmartPtr<RkAiqAfStatsProxy> &to) = 0;
    virtual XCamReturn translateOrbStats (const SmartPtr<VideoBuffer> &from,
                                  SmartPtr<RkAiqOrbStatsProxy> &to) = 0;
    virtual XCamReturn translateAtmoStats (const SmartPtr<VideoBuffer> &from,
                                   SmartPtr<RkAiqAtmoStatsProxy> &to) = 0;
    virtual XCamReturn translateAdehazeStats (const SmartPtr<VideoBuffer> &from,
                                      SmartPtr<RkAiqAdehazeStatsProxy> &to) = 0;

private:
    XCAM_DEAD_COPY (IRkAiqResourceTranslator);
};

}

#endif //_I_RK_AIQ_CONFIG_TRANSLATOR_H_
