/*
 * imu_service.h - interfaces for EIS algorithm to access imu sensor
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
#ifndef ALGOS_AEIS_IMU_SERVICE_H
#define ALGOS_AEIS_IMU_SERVICE_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "rk_aiq_mems_sensor.h"
#include "task_service.h"
#include "xcam_common.h"

using namespace XCam;

namespace RkCam {

class EisImuAdaptor;

class EisImuData {
 public:
    EisImuData() = delete;
    explicit EisImuData(std::shared_ptr<EisImuAdaptor> imu, mems_sensor_type_t type,
                        mems_sensor_event_t* data, size_t count);
    EisImuData(const EisImuData&) = delete;
    const EisImuData& operator=(const EisImuData&) = delete;
    ~EisImuData();

    mems_sensor_type_t GetType() const;
    mems_sensor_event_t* GetData() const;
    size_t GetCount() const;

 private:
    const std::shared_ptr<EisImuAdaptor> imu_;
    const mems_sensor_type_t type_;
    mems_sensor_event_t* data_;
    const size_t count_;
};

class EisImuAdaptor : std::enable_shared_from_this<EisImuAdaptor> {
 public:
    EisImuAdaptor() = delete;
    explicit EisImuAdaptor(const rk_aiq_mems_sensor_intf_t intf,
                           const mems_sensor_type_t type = SENSOR_ALL_TYPE);
    EisImuAdaptor(const EisImuAdaptor&) = delete;
    const EisImuAdaptor& operator=(const EisImuAdaptor&) = delete;
    ~EisImuAdaptor();

    mems_sensor_type_t GetType() const { return type_; };
    std::shared_ptr<EisImuAdaptor> GetPtr() { return shared_from_this(); }

    mems_sensor_event_t* GetData(size_t* num_sample);
    void FreeData(mems_sensor_event_t* data);

    XCamReturn Init(float sample_rate);

 private:
    XCamReturn DeInit();

    const rk_aiq_mems_sensor_intf_t intf_;
    const mems_sensor_type_t type_;
    std::string key_;
    mems_sensor_ctx_t ctx_;
    mems_sensor_handle_t handle_;
    std::vector<std::pair<size_t, mems_sensor_event_t*>> datas_;
};

struct imu_param {
    uint32_t frame_id;
    TaskTimePoint time_point;
    std::unique_ptr<EisImuData> data;
};

class ImuTask final : public ServiceTask<imu_param> {
 public:
    ImuTask() = delete;
    explicit ImuTask(std::shared_ptr<EisImuAdaptor> imu) : imu_(imu) {}
    ImuTask(const ImuTask&) = delete;
    const ImuTask& operator=(const ImuTask&) = delete;
    ~ImuTask()                               = default;

    TaskResult operator()(ServiceParam<imu_param>& p);

 private:
    std::shared_ptr<EisImuAdaptor> imu_;
};

using ImuService = TaskService<imu_param>;

}  // namespace RkCam

#endif  // ALGOS_AEIS_IMU_SERVICE_H
