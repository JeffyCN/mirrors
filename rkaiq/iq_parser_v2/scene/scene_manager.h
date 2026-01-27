/*
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

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "cJSON.h"
#include "cJSON_Utils.h"

cJSON *RkAiqSceneManagerLoadIQFile(const char *name);
CamCalibDbV2Context_t RkAiqSceneManagerRefToScene(CamCalibDbProj_t *calibproj,
                                                  const char *main_scene,
                                                  const char *sub_scene);
cJSON *RkAiqSceneManagerFindMainScene(cJSON *base_json, const char *name);
cJSON *RkAiqSceneManagerFindSubScene(cJSON *json, const char *main_scene,
                                     const char *sub_scene);
CamCalibDbV2Context_t* RkAiqSceneManagerCreateSceneCalib(CamCalibDbProj_t *calibproj,
                                                         const char *main_scene,
                                                         const char *sub_scene);
cJSON *RkAiqSceneManagerMergeMultiSceneIQ(cJSON *base_json);

#ifdef __cplusplus
}
#endif

#endif /*__SCENE_MANAGER_H__*/
