/*
 * eis_algo_service.h
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
#ifndef ALGOS_AEIS_EIS_ALGO_H
#define ALGOS_AEIS_EIS_ALGO_H

#include <map>
#include <memory>
#include <vector>

#include "RkAiqCalibDbTypesV2.h"
#include "drm_device.h"
#include "dvs_app.h"
#include "eis_loader.h"
#include "imu_service.h"
#include "remap_backend.h"
#include "rk_aiq_algo_des.h"
#include "rk_aiq_types_priv.h"
#include "scaler_service.h"
#include "task_service.h"
#include "rk_aiq_algo_types.h"
#include "xcam_common.h"

namespace XCam {

class DmaVideoBuffer;

}

using namespace XCam;

namespace RkCam {

struct FecMeshConfig;
struct FecMeshBuffer;
class FecRemapBackend;

class EisAlgoAdaptor : public std::enable_shared_from_this<EisAlgoAdaptor> {
 public:
    EisAlgoAdaptor() = default;
    virtual ~EisAlgoAdaptor();
    EisAlgoAdaptor(const EisAlgoAdaptor&) = delete;
    const EisAlgoAdaptor& operator=(const EisAlgoAdaptor&) = delete;

    XCamReturn Config(const AlgoCtxInstanceCfg* config, const CalibDbV2_Eis_t* calib);
    XCamReturn Prepare(const rk_aiq_mems_sensor_intf_t* mems_sensor_intf,
                       const isp_drv_share_mem_ops_t* mem_ops);
    bool IsValid() { return valid_; }
    bool IsEnabled() { return enable_; }

    void Start();
    void OnFrameEvent(const RkAiqAlgoProcAeis* input);
    void GetProcResult(RkAiqAlgoProcResAeis* output);
    void Stop();

 private:
    XCamReturn LoadLibrary();
    XCamReturn CreateImuService(const rk_aiq_mems_sensor_intf_t* mems_sensor_intf);
    XCamReturn CreateScalerService();
    XCamReturn CreateFecRemapBackend(const FecMeshConfig& config,
                                     const isp_drv_share_mem_ops_t* mem_ops);

    int OnMeshCallback(struct dvsEngine* engine, struct meshxyFEC* mesh);

    // Import image buffers
    // Alloc Drm Buffers if image mode enabled
    void ImportImageBuffers(std::map<int, int> indexes);

    // Check if hardware compatible
    bool valid_;
    // Enable/Disable by user
    bool enable_;
    bool started_;
    // Configs and calib data
    const CalibDbV2_Eis_t* calib_;

    // DVS algo library
    std::shared_ptr<DvsLibrary> lib_;
    std::unique_ptr<struct dvsEngine> engine_;
    std::map<int, std::unique_ptr<meshxyFEC>> dvs_meshes_;
    std::map<int, std::unique_ptr<imageData>> dvs_images_;

    // The remap backend
    std::unique_ptr<FecRemapBackend> remap_;
    std::map<int, FecMeshBuffer*> remap_meshes_;
    FecMeshBuffer* default_mesh_;

    // IMU Service Handler
    std::unique_ptr<ImuService> imu_;

    // Scaler Service Handler
    std::unique_ptr<ScalerService> scl_;
    // Drm Buffer Manager
    // DrmDevice should implement singlton
    std::shared_ptr<DrmDevice> drm_dev_;
    using ImageBuffer     = std::shared_ptr<DmaVideoBuffer>;
    using ImageBufferList = std::vector<ImageBuffer>;
    // index <unique>, original fd
    std::map<int, int> image_indexes_;
    // index <unique>, nr image, scaled images
    std::tuple<int, ImageBuffer, ImageBufferList> image_groups_;
};

}  // namespace RkCam

#endif  // ALGOS_AEIS_EIS_ALGO_H
