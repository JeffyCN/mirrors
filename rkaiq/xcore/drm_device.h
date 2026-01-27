/*
 * drm_device.h - DRM Device Implementation
 *
 *  Copyright (c) 2021 Rockchip Corporation
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
#ifndef _DRM_DEVICE_H_
#define _DRM_DEVICE_H_

#include <stdint.h>

#include <memory>

#include "unique_fd.h"
#include "xcam_common.h"

extern "C" {

#define RKCAM_MAX_DRM_PLANES 4

struct DrmDumbObject {
    uint32_t drm_format;
    uint32_t width;
    uint32_t height;
    int num_planes;
    int strides[RKCAM_MAX_DRM_PLANES];
    uint32_t offsets[RKCAM_MAX_DRM_PLANES];
    int fds[RKCAM_MAX_DRM_PLANES];
    uint32_t handles[RKCAM_MAX_DRM_PLANES];
    size_t sizes[RKCAM_MAX_DRM_PLANES];
};
}

namespace XCam {

class DrmDevice {
 public:
    DrmDevice();
    ~DrmDevice();
    DrmDevice(const DrmDevice&) = delete;
    DrmDevice& operator=(const DrmDevice&) = delete;

    static bool Available();

    std::unique_ptr<DrmDumbObject> CreateDumbObject(unsigned int width, unsigned int height,
                                                    unsigned int bpp        = 8,
                                                    unsigned int num_planes = 1);
    XCamReturn DestroyDumbObject(const std::unique_ptr<DrmDumbObject>& bo);
    XCamReturn RequestMapDumbObject(const std::unique_ptr<DrmDumbObject>& bo,
                                    unsigned int plane = 0);

 private:
    int Open();
    void Close();
    UniqueFd fd_;
};

}  // namespace XCam

#endif  // _DRM_DEVICE_H_
