/*
 * imu_service.cpp - interfaces for EIS algorithm to access imu sensor
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
#include "imu_service.h"

#include <iostream>
#include <fstream>

#include "xcam_common.h"
#include "xcam_log.h"

using namespace XCam;

namespace RkCam {

EisImuData::EisImuData(std::shared_ptr<EisImuAdaptor> imu, mems_sensor_type_t type,
                       mems_sensor_event_t* data, size_t count)
    : imu_(imu), type_(type), data_(data), count_(count) {
    XCAM_ASSERT(imu_.get() != nullptr && data_ != nullptr);
}

EisImuData::~EisImuData() { imu_->FreeData(data_); }

mems_sensor_type_t EisImuData::GetType() const { return type_; }

mems_sensor_event_t* EisImuData::GetData() const { return data_; }

size_t EisImuData::GetCount() const { return count_; }

EisImuAdaptor::EisImuAdaptor(const rk_aiq_mems_sensor_intf_t intf, const mems_sensor_type_t type)
    : intf_(intf), type_(type), ctx_(nullptr), handle_(nullptr) {
    XCAM_ASSERT(intf.createContext != nullptr && intf.getSensorList != nullptr);
}

EisImuAdaptor::~EisImuAdaptor() { DeInit(); }

XCamReturn EisImuAdaptor::Init(float sample_rate) {
    mems_sensor_list_t sensors_list;
    mems_sensor_capabilities_t caps;

    if (ctx_ == nullptr) {
        ctx_ = intf_.createContext();
        if (ctx_ == nullptr) {
            LOGE_AEIS("Create IMU context failed");
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    memset(&sensors_list, 0, sizeof(mems_sensor_list_t));
    auto result = intf_.getSensorList(ctx_, type_, &sensors_list);
    if (result == SENSOR_NO_ERR && sensors_list.count > 0) {
        LOGV_AEIS("Get supported sensor list: ");
        for (int32_t i = 0; i < sensors_list.count; i++) {
            LOGV_AEIS("%d: %s", i, sensors_list.key_list[i]);
        }
        key_ = std::string(sensors_list.key_list[0]);
        intf_.releaseSensorList(&sensors_list);
    } else {
        LOGE_AEIS("Init IMU get no dev");
        intf_.destroyContext(ctx_);
        ctx_ = nullptr;
        return XCAM_RETURN_ERROR_FAILED;
    }

    memset(&caps, 0, sizeof(mems_sensor_capabilities_t));
    const char* c = key_.c_str();
    result = intf_.getSensorCapabilities(ctx_, type_, c, &caps);
    if (result) {
        LOGE_AEIS("Get IMU capabilities failed");
        intf_.destroyContext(ctx_);
        ctx_ = nullptr;
        return XCAM_RETURN_ERROR_FAILED;
    }

    for (int32_t i = 0; i < caps.num_sample_rates; i++) {
        LOGE_AEIS("Get IMU capabilities: %d:sample_cap: %f", i, caps.sample_rates[i]);
    }

    float matched_sample_rate = 0.0f;
    float max_sample_rate     = 0.0f;
    for (int i = 0; i < caps.num_sample_rates; i++) {
        if (max_sample_rate < caps.sample_rates[i]) {
            max_sample_rate = caps.sample_rates[i];
        }
        if (sample_rate == caps.sample_rates[i]) {
            matched_sample_rate = sample_rate;
            break;
        }
    }
    intf_.releaseSensorCapabilities(&caps);
    if (0.0 == matched_sample_rate) {
        matched_sample_rate = max_sample_rate;
    }

    XCAM_ASSERT(matched_sample_rate != 0.0f);
    if (matched_sample_rate != 0.0) {
        mems_sensor_config_t sensor_cfg;
        memset(&sensor_cfg, 0, sizeof(mems_sensor_config_t));
        result = intf_.getConfig(ctx_, type_, c, &sensor_cfg);
        if (result != SENSOR_NO_ERR) {
            LOGE_AEIS("Get current IMU config failed");
            intf_.destroyContext(ctx_);
            ctx_ = nullptr;
            return XCAM_RETURN_ERROR_FAILED;
        }
        LOGD_AEIS("Get current IMU config: %f", sensor_cfg.sample_rate);
        sensor_cfg.sample_rate = matched_sample_rate;

        result = intf_.setConfig(ctx_, type_, c, sensor_cfg);
        if (result != SENSOR_NO_ERR) {
            LOGE_AEIS("Set IMU config failed\n");
            intf_.destroyContext(ctx_);
            ctx_ = nullptr;
            return XCAM_RETURN_ERROR_FAILED;
        }
        LOGD_AEIS("Set IMU config succeed %f", matched_sample_rate);
    }

    handle_ = intf_.createHandle(ctx_, type_, key_.c_str(), 200, 7);
    if (handle_ == nullptr) {
        LOGE_AEIS("create IMU handle failed");
        intf_.destroyContext(ctx_);
        ctx_ = nullptr;
        return XCAM_RETURN_ERROR_FAILED;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn EisImuAdaptor::DeInit() {
    XCAM_ASSERT(intf_.createContext != nullptr);
    if (ctx_ != nullptr) {
        if (handle_ != nullptr) {
            intf_.destroyHandle(handle_);
        }
        intf_.destroyContext(ctx_);
    }

    return XCAM_RETURN_NO_ERROR;
}

mems_sensor_event_t* EisImuAdaptor::GetData(size_t* num_sample) {
    XCAM_ASSERT(ctx_ != nullptr && handle_ != nullptr);
    size_t count = 0;

    auto data   = reinterpret_cast<mems_sensor_event_t*>(intf_.getData(handle_, &count));
    *num_sample = count;

#ifdef DEBUG
    const char *dump_env = std::getenv("eis_dump_imu");
    int dump = 0;
    if (dump_env) {
        dump = atoi(dump_env);
    }

    if (dump > 0 && data != nullptr && count > 0) {
        std::ofstream ofs("/data/imu.txt", std::ios::app);
        if (ofs.is_open()) {
            for (size_t i = 0; i < count; i++) {
                ofs << data[i].id << "," << data[i].timestamp_us << "," << data[i].all.gyro.x << ","
                    << data[i].all.gyro.y << "," << data[i].all.gyro.z << std::endl;
            }
        }
        ofs.close();
    }
#endif

    return data;
}

void EisImuAdaptor::FreeData(mems_sensor_event_t* data) {
    XCAM_ASSERT(ctx_ != nullptr && handle_ != nullptr);
    intf_.releaseSamplesData(handle_, data);
}

TaskResult ImuTask::operator()(ServiceParam<imu_param>& p) {
    auto payload = p.payload;
    size_t count = 0;

    auto data = imu_->GetData(&count);
    if (count > 0 && data != nullptr) {
        payload->data =
            std::unique_ptr<EisImuData>(new EisImuData(imu_, imu_->GetType(), data, count));
        payload->time_point = std::chrono::system_clock::now();
    } else {
        payload->data       = nullptr;
        payload->time_point = std::chrono::system_clock::now();
        return TaskResult::kFailed;
    }

    return TaskResult::kSuccess;
}

}  // namespace RkCam
