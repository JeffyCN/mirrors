/*
 * scaler_service.h
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
#ifndef ALGOS_AEIS_SCALER_SERVICE_H
#define ALGOS_AEIS_SCALER_SERVICE_H

#include <memory>
#include <vector>

#include "task_service.h"

namespace XCam {

class DmaVideoBuffer;

}  // namespace XCam

using namespace XCam;

namespace RkCam {

class ImageProcessor;

struct scaler_param {
    uint32_t frame_id;
    std::shared_ptr<DmaVideoBuffer> input_image;
    std::vector<std::shared_ptr<DmaVideoBuffer>> scaled_images;
};

class ScalerTask final : public ServiceTask<scaler_param> {
 public:
    ScalerTask() = delete;
    explicit ScalerTask(std::unique_ptr<ImageProcessor> proc);
    ~ScalerTask()                 = default;
    ScalerTask(const ScalerTask&) = delete;
    const ScalerTask& operator=(const ScalerTask&) = delete;

    TaskResult operator()(ServiceParam<scaler_param>& p);

 private:
    std::unique_ptr<ImageProcessor> proc_;
};

using ScalerService = TaskService<scaler_param>;

}  // namespace RkCam

#endif  // ALGOS_AEIS_EIS_ALGO_H
