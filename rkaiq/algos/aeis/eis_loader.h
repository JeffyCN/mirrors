/*
 * algo_loader.h - The dynamic loads algorithm library
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
#ifndef ALGOS_AEIS_EIS_LOADER_H
#define ALGOS_AEIS_EIS_LOADER_H

#include <string>

#include "dvs_app.h"
#include "rk_aiq_mems_sensor.h"

namespace RkCam {

using dvsRegisterRemap   = int (*)(struct dvsEngine* engine, dvsFrameCallBackFEC callback);
using dvsPrepare         = int (*)(struct dvsEngine* engine);
using getMeshSize        = void (*)(int image_height, int image_width, int* mesh_size);
using getOriginalMeshXY  = void (*)(int image_width, int image_height, double clip_ratio_x, double clip_ratio_y, meshxyFEC* pmesh_fec);
using dvsPutImageFrame   = int (*)(struct dvsEngine* engine, struct imageData* pimage_data);
using dvsPutMesh         = int (*)(struct dvsEngine* engine, struct meshxyFEC* pmesh_fec);
using dvsPutImuFrame     = int (*)(struct dvsEngine* engine, mems_sensor_event_s* pimu_data,
                               int buff_number);
using dvsInitFromXmlFile = int (*)(struct dvsEngine* engine, const char* path);
using dvsInitParams      = int (*)(struct dvsEngine* engine, struct initialParams* init_params);
using dvsStart           = int (*)(struct dvsEngine* engine);
using dvsRequestStop     = int (*)(struct dvsEngine* engine);
using dvsDeinit          = int (*)(struct dvsEngine* engine);

struct DvsOps {
    dvsRegisterRemap RegisterRemap;
    dvsPrepare Prepare;
    getMeshSize GetMeshSize;
    getOriginalMeshXY GetOriginalMeshXY;
    dvsPutImageFrame PutImageFrame;
    dvsPutMesh PutMesh;
    dvsPutImuFrame PutImuFrame;
    dvsInitFromXmlFile InitFromXmlFile;
    dvsInitParams InitParams;
    dvsStart Start;
    dvsRequestStop RequestStop;
    dvsDeinit DeInit;
};

class DvsLibrary {
 public:
    DvsLibrary() = default;
    virtual ~DvsLibrary();

    bool Init();
    bool LoadSymbols();

    DvsOps* GetOps();

 private:
    void* handle_;
    DvsOps ops_;
};

}  // namespace RkCam

#endif  // ALGOS_AEIS_EIS_LOADER_H
