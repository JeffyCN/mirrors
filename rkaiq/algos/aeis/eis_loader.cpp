/*
 * algo_loader.cpp - The dynamic loads algorithm library
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
#include "eis_loader.h"

#include <dlfcn.h>

#include <string>

#include "dvs_app.h"
#include "rk_aiq_mems_sensor.h"
#include "xcam_log.h"

namespace RkCam {

const static std::string DvsLibraryName = "libdvs_gyro.so";

DvsLibrary::~DvsLibrary() {
    if (handle_ != nullptr) {
        ::dlclose(handle_);
    }
}

bool DvsLibrary::Init() {
    handle_ = ::dlopen(DvsLibraryName.c_str(), RTLD_LAZY);
    if (handle_ == nullptr) {
        return false;
    }
    return true;
}

bool DvsLibrary::LoadSymbols() {
    char* error;

    ops_.RegisterRemap = (dvsRegisterRemap)dlsym(handle_, "dvsRegisterRemap");
    error              = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsRegisterRemap: %s", error);
        goto error_out;
    }

    ops_.Prepare = (dvsPrepare)dlsym(handle_, "dvsPrepare");
    error        = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsPrepare: %s", error);
        goto error_out;
    }

    ops_.GetMeshSize = (getMeshSize)dlsym(handle_, "getMeshSize");
    error            = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol getMeshSize: %s", error);
        goto error_out;
    }

    ops_.GetOriginalMeshXY = (getOriginalMeshXY)dlsym(handle_, "getOriginalMeshXY");
    error            = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol getOriginalMeshXY: %s", error);
        goto error_out;
    }

    ops_.PutImageFrame = (dvsPutImageFrame)dlsym(handle_, "dvsPutImageFrame");
    error              = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsPutImageFrame: %s", error);
        goto error_out;
    }

    ops_.PutMesh = (dvsPutMesh)dlsym(handle_, "dvsPutMesh");
    error        = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsPutMesh: %s", error);
        goto error_out;
    }

    ops_.PutImuFrame = (dvsPutImuFrame)dlsym(handle_, "dvsPutImuFrame");
    error            = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsPutImuFrame: %s", error);
        goto error_out;
    }

    ops_.InitFromXmlFile = (dvsInitFromXmlFile)dlsym(handle_, "dvsInitFromXmlFile");
    error                = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsInitFromXmlFile: %s", error);
        goto error_out;
    }

    ops_.InitParams = (dvsInitParams)dlsym(handle_, "dvsInitParams");
    error           = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsInitParams: %s", error);
        goto error_out;
    }

    ops_.Start = (dvsStart)dlsym(handle_, "dvsStart");
    error      = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsStart: %s", error);
        goto error_out;
    }

    ops_.RequestStop = (dvsRequestStop)dlsym(handle_, "dvsRequestStop");
    error            = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsRequestStop: %s", error);
        goto error_out;
    }

    ops_.DeInit = (dvsDeinit)dlsym(handle_, "dvsDeinit");
    error       = dlerror();
    if (error != NULL) {
        LOGE("Failed to resolve symbol dvsDeinit: %s", error);
        goto error_out;
    }

    return true;
error_out:
    dlclose(handle_);
    handle_ = nullptr;
    return false;
}

DvsOps* DvsLibrary::GetOps() { return &ops_; }

}  // namespace RkCam

