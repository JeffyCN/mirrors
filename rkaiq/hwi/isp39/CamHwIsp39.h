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
#ifndef _CAM_HW_ISP39_H_
#define _CAM_HW_ISP39_H_

#include "isp39/Isp39Params.h"
#include "isp3x/CamHwIsp3x.h"
#include "xcam_common.h"

namespace RkCam {

class FakeCamHwIsp39;

class CamHwIsp39 : virtual public CamHwIsp3x, virtual public Isp39Params {
    friend class FakeCamHwIsp39;

 public:
    explicit CamHwIsp39();
    virtual ~CamHwIsp39();
    virtual XCamReturn init(const char* sns_ent_name);
    virtual XCamReturn stop();
    XCamReturn setIspParamsSync(uint32_t frameId);
    virtual void updateEffParams(void* params, void* ori_params);
    virtual bool processTb(void* params);

 protected:
};

}  // namespace RkCam

#endif
