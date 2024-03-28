/*
 * eis_algo_service.h
 *
 *  Copyright (c) 2021 Rockchip Electronics Co., Ltd.
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
#include "eis_algo_service.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "Stream.h"
#include "dma_buffer.h"
#include "dvs_app.h"
#include "eis_loader.h"
#include "image_processor.h"
#include "imu_service.h"
#include "rk_aiq_mems_sensor.h"
#include "smart_buffer_priv.h"
#include "xcam_log.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define SUCCESS 0
#define ERROR   1

//#define DEBUG_FEC

using namespace XCam;

namespace RkCam {

namespace {

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) {
        return func(args...);
    }
    static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

}  // namespace

EisAlgoAdaptor::~EisAlgoAdaptor() {
    Stop();

    if (lib_ != nullptr && engine_ != nullptr) {
        lib_->GetOps()->DeInit(engine_.get());
    }
}

XCamReturn EisAlgoAdaptor::LoadLibrary() {
    lib_ = std::make_shared<DvsLibrary>();

    if (!lib_->Init()) {
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (!lib_->LoadSymbols()) {
        return XCAM_RETURN_ERROR_FAILED;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn EisAlgoAdaptor::CreateImuService(const rk_aiq_mems_sensor_intf_t* mems_sensor_intf) {
    if (mems_sensor_intf == nullptr) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    auto adaptor = std::make_shared<EisImuAdaptor>(*mems_sensor_intf);
    if (XCAM_RETURN_NO_ERROR == adaptor->Init(1000.0)) {
        imu_ = std::unique_ptr<ImuService>(
            new ImuService(std::unique_ptr<ImuTask>(new ImuTask(std::move(adaptor))), false, 1,
                           std::chrono::milliseconds(10)));
    } else {
        imu_ = nullptr;
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn EisAlgoAdaptor::CreateScalerService() {
    std::unique_ptr<ImageProcessor> proc(new ImageProcessor());
    proc->set_operator("rga");
    scl_ = std::unique_ptr<ScalerService>(
        new ScalerService(std::unique_ptr<ScalerTask>(new ScalerTask(std::move(proc))), true, 7));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn EisAlgoAdaptor::CreateFecRemapBackend(const FecMeshConfig& config,
                                                 const isp_drv_share_mem_ops_t* mem_ops) {
    XCAM_ASSERT(mem_ops != nullptr);

    remap_ = std::unique_ptr<FecRemapBackend>(new FecRemapBackend(config, mem_ops));

    return XCAM_RETURN_NO_ERROR;
}

int EisAlgoAdaptor::OnMeshCallback(struct dvsEngine* engine, struct meshxyFEC* mesh) {
    XCAM_ASSERT(mesh != nullptr);

    LOGD_AEIS("OnMeshCallback got img id %d , mesh idx %d, img idx %d", mesh->image_index,
              mesh->mesh_buffer_index, mesh->image_buffer_index);
    remap_->Remap(mesh);
    if (lib_ != nullptr) {
        auto* new_mesh = remap_->GetAvailUserBuffer();
        if (new_mesh != nullptr) {
            auto dvs_new_mesh = dvs_meshes_.find(new_mesh->Index);
            if (dvs_new_mesh != dvs_meshes_.end()) {
                LOGD_AEIS("OnMeshCallBack push back available mesh id %d",
                          dvs_new_mesh->second->mesh_buffer_index);
                lib_->GetOps()->PutMesh(engine_.get(), dvs_new_mesh->second.get());
            }
        }
    }

    auto img = dvs_images_.find(mesh->image_index);
    if (img != dvs_images_.end()) {
        dvs_images_.erase(img);
    } else {
        return 1;
    }
    return 0;
}

XCamReturn EisAlgoAdaptor::Config(const AlgoCtxInstanceCfg* config,
                                  const CalibDbV2_Eis_t* calib) {
    calib_  = calib;
    enable_ = calib_->enable;

    if (config->isp_hw_version == 1) {
        valid_ = false;
        LOGE_AEIS("EIS does not compatible with ISP21");
        return XCAM_RETURN_BYPASS;
    }

    if (XCAM_RETURN_NO_ERROR != LoadLibrary()) {
        LOGE_AEIS("EIS library does not exists");
        valid_ = false;
        return XCAM_RETURN_BYPASS;
    }

    valid_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn EisAlgoAdaptor::Prepare(const rk_aiq_mems_sensor_intf_t* mems_sensor_intf,
                                   const isp_drv_share_mem_ops_t* mem_ops) {
    if (!calib_->enable && !enable_) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!valid_) {
        LOGE_AEIS("EIS Invalid, bypassing!");
        return XCAM_RETURN_BYPASS;
    }

    XCamReturn ret;

    if (calib_->mode == EIS_MODE_IMU_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG) {
        ret = CreateImuService(mems_sensor_intf);
        if (ret != XCAM_RETURN_NO_ERROR) {
            valid_ = false;
            LOGE_AEIS("EIS IMU interface invalid, bypassing!");
            return XCAM_RETURN_BYPASS;
        }
    }

    if (calib_->mode == EIS_MODE_IMG_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG) {
        ret = CreateScalerService();
        if (ret != XCAM_RETURN_NO_ERROR) {
            valid_ = false;
            if (calib_->mode == EIS_MODE_IMU_AND_IMG) {
                imu_ = nullptr;
            }
            LOGE_AEIS("EIS scaler interface invalid, bypassing!");
            return XCAM_RETURN_BYPASS;
        }
    }

    int mesh_size;
    lib_->GetOps()->GetMeshSize(calib_->src_image_height, calib_->src_image_width, &mesh_size);
    FecMeshConfig FecCfg;
    FecCfg.Width  = calib_->src_image_width;
    FecCfg.Height = calib_->src_image_height;
    if (FecCfg.Width <= 1920)
        FecCfg.MeshDensity = 0;
    else
        FecCfg.MeshDensity = 1;
    FecCfg.MeshSize = mesh_size;

    // TODO(Cody): FEC may support different src and dst image width/height
    ret = CreateFecRemapBackend(FecCfg, mem_ops);
    if (ret != XCAM_RETURN_NO_ERROR) {
        valid_ = false;
        LOGE_AEIS("EIS remap backend invalid, bypassing!");
        return XCAM_RETURN_BYPASS;
    }

    // TODO(Cody): width/height may change during runtime
    engine_ = std::unique_ptr<dvsEngine>(new dvsEngine());

    lib_->GetOps()->Prepare(engine_.get());

    initialParams init_params;
    init_params.image_buffer_number      = 7;
    init_params.input_image_size.width   = calib_->src_image_width;
    init_params.input_image_size.height  = calib_->src_image_height;
    init_params.output_image_size.width  = calib_->src_image_width;
    init_params.output_image_size.height = calib_->src_image_width;
    init_params.clip_ratio_x             = calib_->clip_ratio_x;
    init_params.clip_ratio_y             = calib_->clip_ratio_y;
    lib_->GetOps()->InitParams(engine_.get(), &init_params);

    if (!lib_->GetOps()->InitFromXmlFile(engine_.get(), calib_->debug_xml_path)) {
        valid_ = false;
        LOGE_AEIS("EIS init algo from xml failed, bypassing!");
        return XCAM_RETURN_BYPASS;
    }

    for (int i = 0; i < 7; i++) {
        auto* mesh                   = remap_->AllocUserBuffer();
        struct meshxyFEC* dvs_mesh   = new meshxyFEC;
        dvs_mesh->is_skip            = false;
        dvs_mesh->image_buffer_index = mesh->ImageBufferIndex;
        dvs_mesh->image_index        = mesh->FrameId;
        dvs_mesh->mesh_buffer_index  = mesh->Index;
        dvs_mesh->mesh_size          = remap_->GetConfig().MeshSize;
        dvs_mesh->pMeshXF            = mesh->MeshXf;
        dvs_mesh->pMeshXI            = mesh->MeshXi;
        dvs_mesh->pMeshYF            = mesh->MeshYf;
        dvs_mesh->pMeshYI            = mesh->MeshYi;
        dvs_meshes_.emplace(dvs_mesh->mesh_buffer_index, dvs_mesh);
        remap_meshes_.emplace(mesh->Index, mesh);
        if (i == 6) {
            dvs_mesh->image_index = mesh->FrameId = -1;
            lib_->GetOps()->GetOriginalMeshXY(calib_->src_image_width, calib_->src_image_height,
                                              calib_->clip_ratio_x, calib_->clip_ratio_y, dvs_mesh);
            remap_->Remap(dvs_mesh);
            default_mesh_ = mesh;
        } else {
            lib_->GetOps()->PutMesh(engine_.get(), dvs_mesh);
        }
    }

    Callback<int(struct dvsEngine*, struct meshxyFEC*)>::func = std::bind(
        &EisAlgoAdaptor::OnMeshCallback, this, std::placeholders::_1, std::placeholders::_2);
    dvsFrameCallBackFEC mesh_cb = static_cast<dvsFrameCallBackFEC>(
        Callback<int(struct dvsEngine*, struct meshxyFEC*)>::callback);
    lib_->GetOps()->RegisterRemap(engine_.get(), mesh_cb);

    return XCAM_RETURN_NO_ERROR;
}

void EisAlgoAdaptor::Start() {
    if (started_ || !valid_) {
        return;
    }

    if (imu_ != nullptr &&
        (calib_->mode == EIS_MODE_IMU_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG)) {
        imu_->start();
    }

    if (scl_ != nullptr &&
        (calib_->mode == EIS_MODE_IMU_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG)) {
        scl_->start();
    }

    if (lib_->GetOps()->Start(engine_.get())) {
        lib_->GetOps()->DeInit(engine_.get());
        goto error_out;
    }

    started_ = true;
    return;

error_out:
    if (imu_ != nullptr &&
        (calib_->mode == EIS_MODE_IMU_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG)) {
        imu_->stop();
        imu_ = nullptr;
    }

    if (scl_ != nullptr &&
        (calib_->mode == EIS_MODE_IMU_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG)) {
        scl_->stop();
        scl_ = nullptr;
    }

    started_ = false;
    engine_  = nullptr;
    valid_   = false;
}

void EisAlgoAdaptor::OnFrameEvent(const RkAiqAlgoProcAeis* input) {
    if (input->orb_stats_buf == nullptr || input->nr_img_buf == nullptr) {
        // valid_ = false;
        LOGE_AEIS("EIS process gets no orb stats/nr image, bypassing!");
        return;
    }
    RkAiqOrbStats* orbStats =
        reinterpret_cast<RkAiqOrbStats*>(input->orb_stats_buf->map(input->orb_stats_buf));
    SmartPtr<SubVideoBuffer> nrImg = reinterpret_cast<SmartBufferPriv*>(input->nr_img_buf)
                                         ->get_video_buffer()
                                         .dynamic_cast_ptr<SubVideoBuffer>();
    auto id     = orbStats->orb_stats.frame_id;
    auto nr_idx = nrImg->get_index();
    auto nr_num = nrImg->get_buf_num();
    auto nr_fd  = nrImg->get_fd();

    LOGV_AEIS("OnFrameEvent id %d idx %d fd %d sof %" PRId64 " skew %lf igt %f ag %d fw %u fh %u mode %d",
              id, nr_idx, nr_fd, input->sof, input->rolling_shutter_skew,
              input->integration_time, input->analog_gain, input->frame_width,
              input->frame_height, calib_->mode);
    if (imu_ != nullptr &&
        (calib_->mode == EIS_MODE_IMU_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG)) {
        auto p = imu_->dequeue();
        if (p.state == ParamState::kAllocated) {
            auto& param     = p.payload;
            param->frame_id = id;
            p.unique_id     = id;
            imu_->enqueue(p);
        } else if (p.state == ParamState::kProcessedSuccess ||
                   p.state == ParamState::kProcessedError) {
            auto& param = p.payload;
            if (param->data != nullptr) {
                LOGD_AEIS("IMU-%d: get data state %d id %d count %d %" PRIu64 "", p.unique_id,
                          p.state, param->frame_id, param->data->GetCount(),
                          (param->data->GetData())[param->data->GetCount() - 1].timestamp_us);
                lib_->GetOps()->PutImuFrame(engine_.get(),
                                            (mems_sensor_event_t*)param->data->GetData(),
                                            param->data->GetCount());
                param->data.reset();
            }
            param->frame_id = id;
            p.unique_id     = id;
            imu_->enqueue(p);
        }
    }

    if (nr_idx < 0) {
        LOGW_AEIS("Process %d frame has invalid frame idx %d", orbStats->orb_stats.frame_id,
                  nr_idx);
        return;
    }

    image_indexes_[nr_idx] = nr_fd;

    if (scl_ != nullptr &&
        (calib_->mode == EIS_MODE_IMG_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG)) {
        // TODO(Cody): dequeue and enqueue scaler param
    }

    struct imageData* image = new imageData();
    if (image != nullptr) {
        auto& meta                = image->meta_data;
        meta.iso_speed            = input->analog_gain;
        meta.exp_time             = input->integration_time;
        meta.rolling_shutter_skew = input->rolling_shutter_skew / 1000000000;
        meta.zoom_ratio           = 1;
        meta.timestamp_sof_us     = input->sof / 1000;

        image->buffer_index = nr_idx;
        image->frame_index  = orbStats->orb_stats.frame_id;

        dvs_images_.emplace(orbStats->orb_stats.frame_id, image);
    }

#if DEBUG
    if (image != nullptr) {
        const char* dump_env = std::getenv("eis_dump_imu");
        int dump             = 0;
        if (dump_env) {
            dump = atoi(dump_env);
        }
        if (dump > 0) {
            std::ofstream ofs("/data/img.txt", std::ios::app);
            if (ofs.is_open()) {
                ofs << image->frame_index << "," << image->buffer_index << ","
                    << image->meta_data.timestamp_sof_us << std::endl;
            }
            ofs.close();
        }
        LOGD_AEIS("Put img frame id %d idx %d ts %" PRId64 "", image->frame_index,
                  image->buffer_index, image->meta_data.timestamp_sof_us);
#endif
        lib_->GetOps()->PutImageFrame(engine_.get(), image);
    }
}

void EisAlgoAdaptor::GetProcResult(RkAiqAlgoProcResAeis* output) {
    auto* mesh  = remap_->GetPendingHwResult();
    auto config = remap_->GetConfig();
    if (mesh != nullptr) {
        LOGD_AEIS("Got DVS result : id %u, idx %d, fd %d", mesh->FrameId, mesh->ImageBufferIndex,
                  mesh->Fd);
        output->update        = 1;
        output->frame_id      = mesh->FrameId > 0 ? mesh->FrameId : 0;
        output->img_buf_index = mesh->ImageBufferIndex;
        if (!image_indexes_.empty()) {
            output->img_size = image_indexes_[mesh->ImageBufferIndex];
        } else {
            output->img_size = 0;
        }
#ifdef DEBUG
        if (mesh->FrameId == 1) remap_->WriteMeshToFile(mesh);
#endif
        output->fd           = mesh->Fd;
        output->mesh_size    = config.MeshSize;
        output->mesh_density = config.MeshDensity;
        if (mesh->Fd >= 0)
            delete mesh;
        else
            remap_->FreeUserBuffer(mesh);
    } else {
        output->update = 0;
        output->fd     = -1;
    }
}

void EisAlgoAdaptor::Stop() {
    if (!started_ || !valid_) {
        return;
    }

    if (imu_ != nullptr &&
        (calib_->mode == EIS_MODE_IMU_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG)) {
        imu_.reset();
    }

    if (scl_ != nullptr &&
        (calib_->mode == EIS_MODE_IMU_ONLY || calib_->mode == EIS_MODE_IMU_AND_IMG)) {
        scl_.reset();
    }

    if (lib_ != nullptr && engine_ != nullptr) {
        lib_->GetOps()->RequestStop(engine_.get());
    }
    started_ = false;
}

}  // namespace RkCam
