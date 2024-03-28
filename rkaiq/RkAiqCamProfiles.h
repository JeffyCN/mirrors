/*
 *  Copyright (c) 2022 Rockchip Eletronics Co., Ltd
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

#ifndef _RK_AIQ_CAM_PROFILES_H_
#define _RK_AIQ_CAM_PROFILES_H_

#include <unordered_map>
#include <vector>
#include <stdint.h>
#include <string>

namespace RkCam {

class CamProfiles {
 public:
    enum AlgoSchedPolicy {
        kDisabled   = 0,
        kSingleOnly = (1 << 0),
        kGroupOnly  = (1 << 1),
        kBoth       = (kSingleOnly | kGroupOnly),
    };

    struct GroupProfile {
        uint8_t group_id;
        union {
            uint16_t cam0 : 1;
            uint16_t cam1 : 1;
            uint16_t cam2 : 1;
            uint16_t cam3 : 1;
            uint16_t cam4 : 1;
            uint16_t cam5 : 1;
            uint16_t cam6 : 1;
            uint16_t cam7 : 1;
            uint16_t reserved : 8;
            uint16_t cams;
        };
        uint8_t master_cam;
        bool sync;
    };

    CamProfiles();
    ~CamProfiles();

    std::unordered_map<int, int>& getAlgoPolicies() { return algo_policies_; }

    void ParseFromIni(const std::string ini_path);

 private:
    uint8_t group_count_{0};
    std::vector<GroupProfile> group_profiles_{};
    std::unordered_map<int, int> algo_policies_{};
};

}  // namespace RkCam

#endif  //_RK_AIQ_CAM_PROFILES_H_
