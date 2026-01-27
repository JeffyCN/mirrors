
/*
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

#ifndef _CAM_HW_ISP21_H_
#define _CAM_HW_ISP21_H_

#include "isp20/CamHwIsp20.h"
#include "Isp21Params.h"

namespace RkCam {

class FakeCamHwIsp21;

class CamHwIsp21
    : virtual public CamHwIsp20
    , virtual public Isp21Params {
    friend class FakeCamHwIsp21;
public:
    explicit CamHwIsp21();
    virtual ~CamHwIsp21();
    virtual XCamReturn init(const char* sns_ent_name);
    virtual XCamReturn stop();
    XCamReturn setIspParamsSync(uint32_t frameId);
    XCamReturn setIsppParamsSync(uint32_t frameId);
    virtual void updateEffParams(void* params, void* ori_params);
    virtual bool processTb(void* params);
protected:
    virtual XCamReturn dispatchResult(SmartPtr<cam3aResult> cam3a_result);
    virtual XCamReturn dispatchResult(cam3aResultList& list);
    virtual bool isOnlineByWorkingMode();
};

}

#endif
