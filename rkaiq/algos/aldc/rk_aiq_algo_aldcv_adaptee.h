/*
 * rk_aiq_algo_aldcv_algo.h
 *
 *  Copyright (c) 2023 Rockchip Corporation
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

#ifndef _RK_AIQ_ALGO_ALDCV_ALGO_H_
#define _RK_AIQ_ALGO_ALDCV_ALGO_H_

#include "algos/aldc/ldc_lut_buffer.h"
#include "algos/aldc/rk_aiq_algo_aldc_gen_mesh.h"
#include "algos/aldc/rk_aiq_types_aldc_algo_int.h"
#include "algos/rk_aiq_algo_types.h"

namespace RkCam {

class LdcvAlgoAdaptee {
 public:
    LdcvAlgoAdaptee() = default;
    virtual ~LdcvAlgoAdaptee();
    LdcvAlgoAdaptee(const LdcvAlgoAdaptee&) = delete;
    const LdcvAlgoAdaptee& operator=(const LdcvAlgoAdaptee&) = delete;

    XCamReturn Config(const AlgoCtxInstanceCfg* config, const int32_t conf_type,
                      const CalibDbV2_LDC_t* calib);
    XCamReturn SetApiAttr(const rk_aiq_ldc_ldcv_attrib_t* attr);
    XCamReturn GetApiAttr(rk_aiq_ldc_ldcv_attrib_t* attr);

    XCamReturn Prepare(const RkAiqAlgoConfigAldc* config);
    bool IsEnabled() const { return enable_; }
    bool IsStarted() const { return started_; }
    void* GetFreeLutBuf(int8_t isp_id, int32_t* out_fd);
    RKAIQLdcUpdateLutMode GetUpdLutBufMode() { return upd_lut_mode_; }
    rk_aiq_ldc_ldcv_attrib_t* GetCurAttrib() const { return attr_ ? attr_.get() : nullptr; }
    uint8_t GetCorrectLevel() const { return attr_ ? attr_->correct_level : 0; }
    XCamReturn UpdateMesh(const uint8_t level, const int32_t fd);
    void SetGenMeshState(const LdcGenMeshState state) {
        gen_mesh_state_.store(state, std::memory_order_release);
    }

    void OnFrameEvent(const RkAiqAlgoProcAldc* input, RkAiqAlgoProcResAldc* output);
    XCamReturn ReadMeshFromFile(const char* file_name, const void* addr,
                                LdcLutBufferConfig& config);

 private:
    bool enable_           = false;
    bool started_          = false;
    bool valid_            = true;
    bool isReCal_          = true;
    uint8_t correct_level_ = 0;
    std::atomic<LdcGenMeshState> gen_mesh_state_{LdcGenMeshState::kLdcGenMeshIdle};

    const AlgoCtxInstanceCfg* ctx_config_           = nullptr;
    const RkAiqAlgoConfigAldc* config_              = nullptr;
    const CalibDbV2_LDC_t* calib_                   = nullptr;
    std::unique_ptr<rk_aiq_ldc_ldcv_attrib_t> attr_ = nullptr;

    RKAIQLdcUpdateLutMode upd_lut_mode_{kRKAIQLdcUpdateLutOnLine};
    std::unique_ptr<LdcLutBufferManager> lut_manger_;
    std::vector<std::unique_ptr<LdcLutBuffer>> cached_lut_ = {};
    std::vector<int32_t> chip_in_use_lut_fd_               = {};
};

}  // namespace RkCam

#endif  //_RK_AIQ_ALGO_ALDCV_ALGO_H_
