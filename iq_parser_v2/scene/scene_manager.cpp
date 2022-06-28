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

#include "scene_manager.h"
#include "j2s.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace RkCam {

std::map<std::string, AiqScene> RkAiqSceneManager::sceneMap;

cJSON *RkAiqSceneManager::loadIQFile(const char *name) {
  char *json_data = NULL;
  size_t json_data_size = 0;
  cJSON *out_json = NULL;

  json_data = (char *)j2s_read_file(name, &json_data_size);
  out_json = cJSON_Parse(json_data);
  if (json_data) {
    free(json_data);
  }

  return out_json;
}

int RkAiqSceneManager::addScene(const char *name, const char *base,
                                const char *fragment) {
  sceneMap[name] = std::make_pair(std::string(base), std::string(fragment));

  return 0;
}

int RkAiqSceneManager::addScene(const char *name, AiqScene scene) {
  sceneMap[name] = scene;
  return 0;
}

AiqScene RkAiqSceneManager::matchScene(const char *name) {
  auto search = sceneMap.find(std::string(name));

  if (search != sceneMap.end()) {
    return search->second;
  } else {
    std::cout << "Not found\n";
  }

  return AiqScene();
}

const char *RkAiqSceneManager::getSceneIQ(const char *base,
                                          const char *fragment) {
  cJSON *base_json = NULL;
  cJSON *fragment_json = NULL;
  cJSON *out_json = NULL;

  base_json = loadIQFile(base);
  if (!base_json) {
    printf("Error before: [%s]\n", cJSON_GetErrorPtr());
  } else {
    char *out = cJSON_Print(base_json);
    // printf("%s\n", out);
    free(out);
  }

  fragment_json = loadIQFile(fragment);
  if (!fragment_json) {
    printf("Error before: [%s]\n", cJSON_GetErrorPtr());
  } else {
    char *out = cJSON_Print(fragment_json);
    // printf("%s\n", out);
    free(out);
  }

  // foreach fragment json obj, if fragment first level son obj is not empty
  // then override base json's same object
  if (cJSON_Compare(base_json, fragment_json, 1)) {
    printf("same iq files, do nothing!\n");
    char *out = cJSON_Print(base_json);
    return out;
  }

  for (int i = 0; i < cJSON_GetArraySize(base_json); i++) {
    printf("[%d]:", i);
    cJSON *item = cJSON_GetArrayItem(base_json, i);
    if (cJSON_Invalid != item->type) {
      printf("[%s]\n", item->string);
      // printJson(item);
    }
  }

  out_json = cJSON_Duplicate(base_json, 1);

  // FIXME: memery leack issue
  for (int i = 0; i < cJSON_GetArraySize(fragment_json); i++) {
    printf("[%d]:", i);
    cJSON *item = cJSON_GetArrayItem(fragment_json, i);
    if (cJSON_Invalid != item->type) {
      printf("[%s]\n", item->string);
      // printJson(item);
      cJSON_ReplaceItemInObjectCaseSensitive(out_json, item->string,
                                             cJSON_Duplicate(item, 1));
    }
  }

  printf("new json:>\n%s\n", cJSON_Print(out_json));

  cJSON_Delete(base_json);
  cJSON_Delete(fragment_json);

  return cJSON_Print(out_json);
}

const char *RkAiqSceneManager::getSceneIQ(AiqScene scene) {
  if (access(scene.first.c_str(), F_OK)) {
    printf("[RkAiqSceneManager]: access %s failed\n", scene.first.c_str());
    return NULL;
  }

  if (access(scene.second.c_str(), F_OK)) {
    printf("[RkAiqSceneManager]: access %s failed\n", scene.first.c_str());
    return NULL;
  }

  return getSceneIQ(scene.first.c_str(), scene.second.c_str());
}

const char *RkAiqSceneManager::getSceneIQ(const char *scene) {
  return getSceneIQ(matchScene(scene));
}

int RkAiqSceneManager::addScene(CamCalibDbProj_t *calibproj,
                                const char *main_scene, const char *sub_scene,
                                CamCalibDbV2Context_t *calib) {
  return 0;
}

CamCalibDbV2Context_t
RkAiqSceneManager::refToScene(CamCalibDbProj_t *calibproj,
                              const char *main_scene, const char *sub_scene) {
  CamCalibDbV2Context_t ctx;
  CamCalibMainSceneList_t *main_list = nullptr;
  CamCalibSubSceneList_t *sub_list = nullptr;
  void* dst_calib = nullptr;
  int main_list_len = -1;
  int sub_list_len = -1;
  int curr_main_scene = 0;
  int curr_sub_scene = 0;

  if (!calibproj) {
    printf("No avaliable CamCalibDbProj loadded!\n");
    return ctx;
  }

  main_list = calibproj->main_scene;
  main_list_len = calibproj->main_scene_len;
  memset(&ctx, 0, sizeof(CamCalibDbV2Context_t));

  if (!main_list || main_list_len < 1) {
    printf("No avaliable main scene!\n");
    return ctx;
  }

  // Find main scene
  for (curr_main_scene = 0; curr_main_scene < main_list_len;
       curr_main_scene++) {
    if (0 == strcmp(main_list[curr_main_scene].name, main_scene)) {
      // Found what we want, and this must be the only one.
      sub_list = main_list[curr_main_scene].sub_scene;
      sub_list_len = main_list[curr_main_scene].sub_scene_len;
      break;
    }
  }

  if (!sub_list || sub_list_len < 1) {
    printf("No avaliable main scene!\n");
    return ctx;
  }

  // Find sub scene
  for (curr_sub_scene = 0; curr_sub_scene < sub_list_len; curr_sub_scene++) {
    if (0 != strcmp(sub_list[curr_sub_scene].name, sub_scene)) {
      // Not what we want
      continue;
    }
    dst_calib = calibdbv2_get_scene_ptr(&sub_list[curr_sub_scene]);
    ctx.calib_scene = (char*)(dst_calib);
    ctx.sensor_info = &calibproj->sensor_calib;
    ctx.module_info = &calibproj->module_calib;
    ctx.sys_cfg = &calibproj->sys_static_cfg;

    return ctx;
  }

  printf("Can't find scene:[%s]/[%s]!\n", main_scene, sub_scene);

  return ctx;
}

} // namespace RkCam
