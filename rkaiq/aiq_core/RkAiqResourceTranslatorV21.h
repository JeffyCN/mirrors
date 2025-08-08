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

#ifndef _RK_AIQ_RESOURCE_TRANSLATOR_V21_H_
#define _RK_AIQ_RESOURCE_TRANSLATOR_V21_H_

#include "RkAiqResourceTranslator.h"

namespace RkCam {

class RkAiqResourceTranslatorV21
    : public RkAiqResourceTranslator
{
public:
    RkAiqResourceTranslatorV21 ()          = default;
    virtual ~RkAiqResourceTranslatorV21 () = default;

    virtual XCamReturn translateAwbStats (const SmartPtr<VideoBuffer> &from,
                                  SmartPtr<RkAiqAwbStatsProxy> &to);
#if RKAIQ_HAVE_DEHAZE_V11
    virtual XCamReturn translateAdehazeStats (const SmartPtr<VideoBuffer> &from,
                                      SmartPtr<RkAiqAdehazeStatsProxy> &to);
#endif

    XCamReturn translateOrbStats (const SmartPtr<VideoBuffer> &from,
                                  SmartPtr<RkAiqOrbStatsProxy> &to) {
        return  XCAM_RETURN_BYPASS;
    };
    XCamReturn translateAtmoStats (const SmartPtr<VideoBuffer> &from,
                                   SmartPtr<RkAiqAtmoStatsProxy> &to) {
        return  XCAM_RETURN_BYPASS;
    };

private:
    XCAM_DEAD_COPY (RkAiqResourceTranslatorV21);
};

}

#endif //_RK_AIQ_CONFIG_TRANSLATOR_H_
