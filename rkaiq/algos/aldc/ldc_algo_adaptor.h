/*
 * ldc_algo_adaptor.h
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
 */
#ifndef ALGOS_ALDC_LDC_ALGO_H
#define ALGOS_ALDC_LDC_ALGO_H

#include <memory>
#include <vector>

#include "algos/aldc/ldc_lut_buffer.h"
#include "algos/aldc/rk_aiq_algo_aldc_gen_mesh.h"
#include "algos/aldc/rk_aiq_algo_aldch_adaptee.h"
#include "algos/aldc/rk_aiq_algo_aldcv_adaptee.h"
#include "algos/aldc/rk_aiq_types_aldc_algo_int.h"
#include "algos/rk_aiq_algo_des.h"
#include "algos/rk_aiq_algo_types.h"
#include "common/rk_aiq_types_priv.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_common.h"

namespace RkCam {

class LdcAlgoAdaptor : public GenMeshCallback {
 public:
    LdcAlgoAdaptor() = default;
    virtual ~LdcAlgoAdaptor();
    LdcAlgoAdaptor(const LdcAlgoAdaptor&) = delete;
    const LdcAlgoAdaptor& operator=(const LdcAlgoAdaptor&) = delete;

    XCamReturn Config(const AlgoCtxInstanceCfg* config, const int32_t conf_type,
                      const CalibDbV2_LDC_t* calib);
    XCamReturn SetApiAttr(const rk_aiq_ldc_attrib_t* attr);
    XCamReturn GetApiAttr(rk_aiq_ldc_attrib_t* attr);

    XCamReturn Prepare(const RkAiqAlgoConfigAldc* config);
    bool IsEnabled() const { return enable_; }
    bool IsStarted() const { return started_; }

    void OnFrameEvent(const RkAiqAlgoProcAldc* input, RkAiqAlgoProcResAldc* output);

    const AlgoCtxInstanceCfg* GetConfig() { return ctx_config_; }

 private:
    void GenMeshDoneCallback(const LdcGenMeshCfg_t& cfg);
    void GenMeshFailedCallback(const int32_t fd);

    bool enable_  = false;
    bool ldch_en_ = false;
    bool ldcv_en_ = false;
    bool started_ = false;
    bool valid_   = true;

    const AlgoCtxInstanceCfg* ctx_config_               = nullptr;
    std::unique_ptr<LdchAlgoAdaptee> ldch_algo_adaptee_ = nullptr;
    std::unique_ptr<LdcvAlgoAdaptee> ldcv_algo_adaptee_ = nullptr;

    uint8_t correct_level_                                 = 0;
    SmartPtr<LdcAlgoGenMesh> ldc_algo_gen_mesh_            = nullptr;
    std::unique_ptr<LdcGenMeshThread> ldc_gen_mesh_thread_ = nullptr;
};

}  // namespace RkCam

#endif  // ALGOS_ALDC_LDC_ALGO_H
