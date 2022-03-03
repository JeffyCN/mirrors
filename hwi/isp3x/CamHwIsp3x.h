/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef _CAM_HW_ISP3x_H_
#define _CAM_HW_ISP3x_H_

#include "isp21/CamHwIsp21.h"
#include "Isp3xParams.h"

namespace RkCam {

class FakeCamHwIsp21;

class CamHwIsp3x
    : virtual public CamHwIsp21
    , virtual public Isp3xParams {
    friend class FakeCamHwIsp3x;
public:
    explicit CamHwIsp3x();
    virtual ~CamHwIsp3x();
    virtual XCamReturn init(const char* sns_ent_name);
    virtual XCamReturn stop();
    XCamReturn setIspParamsSync(int frameId);
protected:
    virtual XCamReturn setIspConfig();
private:
    void gen_full_isp_params(const struct isp3x_isp_params_cfg* update_params,
                             struct isp3x_isp_params_cfg* full_params,
                             uint64_t* module_en_update_partial,
                             uint64_t* module_cfg_update_partial);
    struct isp3x_isp_params_cfg _full_active_isp3x_params;
};

};

#endif
