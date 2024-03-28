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
#include "RkAiqCamProfiles.h"

#include <errno.h>
#include <unistd.h>

#include "RkAiqCoreConfig.h"
#include "rk_aiq_algo_des.h"
#include "rkaiq_ini.h"
#include "xcam_log.h"

namespace RkCam {

CamProfiles::CamProfiles() {
    for (int i = RK_AIQ_ALGO_TYPE_NONE + 1; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        algo_policies_.emplace(i, AlgoSchedPolicy::kBoth);
    }
}

CamProfiles::~CamProfiles() = default;

void CamProfiles::ParseFromIni(const std::string ini_path) {
    if (-1 == access(ini_path.c_str(), R_OK)) {
        XCAM_LOG_WARNING("Failed to access profile config file %s\n", ini_path.c_str(),
                       strerror(errno));
        return;
    }
    auto* ini = rkaiq_ini_load(ini_path.c_str());
    rkaiq_ini_sget(ini, "root", "group_count", "%d", &group_count_);

    for (int i = 0; i < group_count_; i++) {
        CamProfiles::GroupProfile group;
        std::string section = "group";
        section.append(std::to_string(i));
        int val = 0;
        if (rkaiq_ini_sget(ini, section.c_str(), "master_cam", "%d", &val)) {
            group.master_cam = val;
        }
        if (rkaiq_ini_sget(ini, section.c_str(), "sync", "%d", &val)) {
            group.sync = !!val;
        } else {
            group.sync = true;
        }
        for (int c = 0; c < 16; c++) {
            std::string cam = "use_cam";
            cam.append(std::to_string(c));
            if (rkaiq_ini_sget(ini, section.c_str(), cam.c_str(), "%d", &val)) {
                group.cams = 1 << c;
            }
        }
        if (group.cams != 0) {
            group.group_id = i;
            group_profiles_.push_back(std::move(group));
        }
    }

    for (auto& p : group_profiles_) {
        std::string policy = "sched_policy";
        policy.append(std::to_string(p.group_id));
        int val = 0;

        if (rkaiq_ini_sget(ini, policy.c_str(), "ae", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AE] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "awb", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AWB] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "af", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AF] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "bls", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ABLC] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "dpcc", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ADPCC] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "lsc", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ALSC] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "merge", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AMERGE] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "drc", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ADRC] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "bayertnr", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AMFNR] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "bayer2dnr", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ARAWNR] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "cac", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ACAC] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "gic", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AGIC] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "debayer ", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ADEBAYER] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "ccm", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ACCM] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "gamma", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AGAMMA] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "lut3d", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_A3DLUT] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "ldch", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ALDCH] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "csm", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ACSM] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "cp", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ACP] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "ie", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AIE] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "gain", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AGAIN] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "dehaze", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ADHAZ] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "ynr", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_AYNR] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "cnr", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ACNR] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "sharp", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ASHARP] = val;
        }
        if (rkaiq_ini_sget(ini, policy.c_str(), "rgbir", "%d", &val)) {
            algo_policies_[RK_AIQ_ALGO_TYPE_ARGBIR] = val;
        }
    }

    for (auto p : algo_policies_) {
        XCAM_LOG_INFO("Parsed profile algo: %s policy: %d\n",
                      AlgoTypeToString((RkAiqAlgoType_t)p.first).c_str(), p.second);
    }
}

}  // namespace RkCam
