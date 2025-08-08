/*
 * rk_aiq_algo_aldc_gen_mesh.h
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

#ifndef _RK_AIQ_ALGO_ALDC_GEN_MESH_H_
#define _RK_AIQ_ALGO_ALDC_GEN_MESH_H_

#include <memory>

#include "algos/aldc/gen_mesh/include/rkAlgoGenFloatMeshFromRK.h"
#include "algos/aldc/gen_mesh/include/rkAlgoGenLdchLdcvMesh.h"
#include "algos/aldc/gen_mesh/include/rkAlgoMapSeparate.h"
#include "algos/aldc/ldc_lut_buffer.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_common.h"
#include "xcore/base/xcam_log.h"
#include "xcore/safe_list.h"
#include "xcore/smartptr.h"
#include "xcore/xcam_mutex.h"
#include "xcore/xcam_thread.h"

namespace RkCam {

using namespace XCam;

enum class LdcEnState {
    kLdcEnLdch     = 0,
    kLdcEnLdcv     = 1,
    kLdcEnLdchLdcv = 2,
};

enum class LdcGenMeshState {
    kLdcGenMeshIdle = 0,
    kLdcGenMeshWorking,
    kLdcGenMeshFinish,
};

typedef struct LdcMeshBufInfo_s {
    int32_t fd;
    void* addr;
} LdcMeshBufInfo_t;

typedef struct LdcGenMeshCfg_s {
    uint8_t level;

    LdcMeshBufInfo_t ldch;
    LdcMeshBufInfo_t ldcv;
} LdcGenMeshCfg_t;

class GenMeshCallback {
 public:
    explicit GenMeshCallback() {}
    virtual ~GenMeshCallback() {}
    virtual void GenMeshDoneCallback(const LdcGenMeshCfg_t& cfg) = 0;
    virtual void GenMeshFailedCallback(const int32_t fd)         = 0;

 private:
    XCAM_DEAD_COPY(GenMeshCallback);
};

class LdcAlgoGenMesh {
 public:
    LdcAlgoGenMesh() = default;
    virtual ~LdcAlgoGenMesh(){};
    LdcAlgoGenMesh(const LdcAlgoGenMesh&) = delete;
    const LdcAlgoGenMesh& operator=(const LdcAlgoGenMesh&) = delete;

    XCamReturn Init(LdcEnState state, int32_t srcW, int32_t srcH, int32_t dstW, int32_t dstH,
                    const CalibDbV2_LDC_t* calib);
    XCamReturn Deinit();
    XCamReturn GenMeshNLevel(uint8_t isp_id, uint32_t level, uint16_t* pMeshX, uint16_t* pMeshY);
    bool IsEnabled() const { return enable_; }

 private:
    bool enable_      = false;
    LdcEnState state_ = LdcEnState::kLdcEnLdchLdcv;

    /* 原始全分辨率浮点表 */
    std::unique_ptr<float> pf32MapxOri = nullptr;
    std::unique_ptr<float> pf32MapyOri = nullptr;
    /* 拆分转换后的下采样浮点表 */
    std::unique_ptr<float> pf32MeshX = nullptr;
    std::unique_ptr<float> pf32MeshY = nullptr;
    /* LDCH/LDCV定点化表 */
    /*
     * std::unique_ptr<uint16_t> pu16MeshX = nullptr;
     * std::unique_ptr<uint16_t> pu16MeshY = nullptr;
     */

    /* RK模型参数 */
    RKALGO_CAM_COEFF_INFO_S cam_coeff_;
    /* 原始全分辨率浮点表的相关参数 */
    RKALGO_GEN_FLOAT_MESH_INFO_S ori_float_mesh_Info_;
    /* 分步校正下采样浮点表相关参数 */
    RKALGO_SEPARATE_MESH_INFO_S st_separate_mesh_info_;
    /* LDCH和LDCV定点化表相关参数 */
    RKALGO_LDCH_LDCV_INFO_S st_ldch_ldcv_Info_;
};

class LdcGenMeshThread : public Thread {
 public:
    LdcGenMeshThread() = default;
    LdcGenMeshThread(const SmartPtr<LdcAlgoGenMesh>& gen_mesh)
        : Thread("LdcGenMeshThread"), gen_mesh_(gen_mesh){};
    LdcGenMeshThread(const LdcGenMeshThread&) = delete;
    ~LdcGenMeshThread() { attr_queue_.clear(); };

    void triger_stop() { attr_queue_.pause_pop(); };

    void triger_start() { attr_queue_.resume_pop(); };

    bool push_attr(const SmartPtr<LdcGenMeshCfg_t> attr) {
        attr_queue_.push(attr);
        return true;
    };

    bool is_empty() { return attr_queue_.is_empty(); };

    void clear_attr() { attr_queue_.clear(); };

    void SetGenMeshCallback(GenMeshCallback* cb) { callback_ = cb; }

 protected:
    // virtual bool started ();
    virtual void stopped() { attr_queue_.clear(); };
    virtual bool loop();

 private:
    SafeList<LdcGenMeshCfg_t> attr_queue_;

    SmartPtr<LdcAlgoGenMesh> gen_mesh_ = nullptr;
    GenMeshCallback* callback_         = nullptr;
};

}  // namespace RkCam

#endif  //_RK_AIQ_ALGO_ALDC_GEN_MESH_H_
