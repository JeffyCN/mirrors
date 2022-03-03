/*
 * cac_algo_adaptor.h
 *
 *  Copyright (c) 2021 Rockchip Electronics Co.,Ltd
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
#ifndef ALGOS_ACAC_CAC_ALGO_H
#define ALGOS_ACAC_CAC_ALGO_H

#include <memory>

#include "RkAiqCalibDbTypesV2.h"
#include "rk_aiq_algo_des.h"
#include "rk_aiq_algo_types.h"
#include "rk_aiq_types_priv.h"
#include "xcam_common.h"

using namespace XCam;

namespace RkCam {

struct LutBuffer;
class LutBufferManager;

class CacAlgoAdaptor : public std::enable_shared_from_this<CacAlgoAdaptor> {
 public:
    CacAlgoAdaptor() = default;
    virtual ~CacAlgoAdaptor();
    CacAlgoAdaptor(const CacAlgoAdaptor&) = delete;
    const CacAlgoAdaptor& operator=(const CacAlgoAdaptor&) = delete;

    XCamReturn Config(const AlgoCtxInstanceCfg* config, const CalibDbV2_Cac_t* calib);
    XCamReturn Prepare(const RkAiqAlgoConfigAcac* config);
    const bool IsEnabled() const { return enable_; }
    const bool IsStarted() const { return started_; }

    void OnFrameEvent(const RkAiqAlgoProcAcac* input, RkAiqAlgoProcResAcac* output);

 private:
    const RkAiqAlgoConfigAcac* config_;
    const CalibDbV2_Cac_t* calib_;
    bool enable_;
    bool started_;
    bool valid_;
    LutBufferManager* lut_manger_;
    LutBuffer* current_lut_[2];
};

}  // namespace RkCam

#endif  // ALGOS_ACAC_CAC_ALGO_H
