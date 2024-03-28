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
#include <vector>

#include "algos/acac/lut_buffer.h"
#include "algos/acac/rk_aiq_types_acac_algo_int.h"
#include "algos/rk_aiq_algo_des.h"
#include "algos/rk_aiq_algo_types.h"
#include "common/rk_aiq_types_priv.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_common.h"

namespace RkCam {

class CacAlgoAdaptor {
 public:
    CacAlgoAdaptor() = default;
    virtual ~CacAlgoAdaptor();
    CacAlgoAdaptor(const CacAlgoAdaptor&) = delete;
    const CacAlgoAdaptor& operator=(const CacAlgoAdaptor&) = delete;

#if RKAIQ_HAVE_CAC_V03
    XCamReturn Config(const AlgoCtxInstanceCfg* config, const CalibDbV2_Cac_V03_t* calib);
#elif RKAIQ_HAVE_CAC_V10
    XCamReturn Config(const AlgoCtxInstanceCfg* config, const CalibDbV2_Cac_V10_t* calib);
#elif RKAIQ_HAVE_CAC_V11
    XCamReturn Config(const AlgoCtxInstanceCfg* config, const CalibDbV2_Cac_V11_t* calib);
#endif
#if RKAIQ_HAVE_CAC_V03
    XCamReturn SetApiAttr(const rkaiq_cac_v03_api_attr_t* attr);
#elif RKAIQ_HAVE_CAC_V10
    XCamReturn SetApiAttr(const rkaiq_cac_v10_api_attr_t* attr);
#elif RKAIQ_HAVE_CAC_V11
    XCamReturn SetApiAttr(const rkaiq_cac_v11_api_attr_t* attr);
#endif
#if RKAIQ_HAVE_CAC_V03
    XCamReturn GetApiAttr(rkaiq_cac_v03_api_attr_t* attr);
#elif RKAIQ_HAVE_CAC_V10
    XCamReturn GetApiAttr(rkaiq_cac_v10_api_attr_t* attr);
#elif RKAIQ_HAVE_CAC_V11
    XCamReturn GetApiAttr(rkaiq_cac_v11_api_attr_t* attr);
#endif


    XCamReturn Prepare(const RkAiqAlgoConfigAcac* config);
    bool IsEnabled() const { return enable_; }
    bool IsStarted() const { return started_; }

    void OnFrameEvent(const RkAiqAlgoProcAcac* input, RkAiqAlgoProcResAcac* output);

    const AlgoCtxInstanceCfg* GetConfig() { return ctx_config_; }

 private:
    bool enable_  = false;
    bool started_ = false;
    bool valid_   = true;
    bool isReCal_ = true;
    int  lastIso_ = 0;
    std::unique_ptr<LutBufferManager> lut_manger_;
    std::vector<std::unique_ptr<LutBuffer>> current_lut_ = {};

    const AlgoCtxInstanceCfg* ctx_config_ = nullptr;
    const RkAiqAlgoConfigAcac* config_ = nullptr;
#if RKAIQ_HAVE_CAC_V03
    const CalibDbV2_Cac_V03_t* calib_ = nullptr;
#elif RKAIQ_HAVE_CAC_V10
    const CalibDbV2_Cac_V10_t* calib_ = nullptr;
#elif RKAIQ_HAVE_CAC_V11
    const CalibDbV2_Cac_V11_t* calib_ = nullptr;
#endif
#if RKAIQ_HAVE_CAC_V03
    std::unique_ptr<rkaiq_cac_v03_api_attr_t> attr_ = nullptr;
#elif RKAIQ_HAVE_CAC_V10
    std::unique_ptr<rkaiq_cac_v10_api_attr_t> attr_ = nullptr;
#elif RKAIQ_HAVE_CAC_V11
    std::unique_ptr<rkaiq_cac_v11_api_attr_t> attr_ = nullptr;
#endif
};

}  // namespace RkCam

#endif  // ALGOS_ACAC_CAC_ALGO_H
