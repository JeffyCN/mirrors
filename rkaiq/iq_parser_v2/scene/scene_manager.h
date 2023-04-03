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

#include <iostream>
#include <map>

#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "cJSON.h"
#include "cJSON_Utils.h"

namespace RkCam {

typedef std::pair<std::string, std::string> AiqScene;

class RkAiqSceneManager {
public:
  explicit RkAiqSceneManager() = default;
  ~RkAiqSceneManager() = default;

public:
  static cJSON *loadIQFile(const char *name);
  static int addScene(const char *name, const char *base, const char *fragment);
  static int addScene(const char *name, AiqScene scene);
  static AiqScene matchScene(const char *name);
  static const char *getSceneIQ(AiqScene scene);
  static const char *getSceneIQ(const char *scene);
  static const char *getSceneIQ(const char *base, const char *fragment);

  static int addScene(CamCalibDbProj_t *calibproj, const char *main_scene,
                      const char *sub_scene, CamCalibDbV2Context_t *calib);
  static CamCalibDbV2Context_t refToScene(CamCalibDbProj_t *calibproj,
                                           const char *main_scene,
                                           const char *sub_scene);

  static cJSON *findMainScene(cJSON *base_json, const char *name);
  static cJSON *findSubScene(cJSON *main_json, const char *name);
  static cJSON *findSubScene(cJSON *json, const char *main_scene,
                             const char *sub_scene);

  static CamCalibDbV2Context_t* createSceneCalib(CamCalibDbProj_t *calibproj,
                                                 const char *main_scene,
                                                 const char *sub_scene);

  static cJSON *mergeSubMultiScene(cJSON *sub_scene_list,
                                   cJSON* full_param, bool skip);
  static cJSON *mergeMainMultiScene(cJSON *main_scene_list);
  static cJSON *mergeMultiSceneIQ(cJSON *base_json);

private:
  static std::map<std::string, AiqScene> sceneMap;
};

} // namespace RkCam

#endif /*__SCENE_MANAGER_H__*/
