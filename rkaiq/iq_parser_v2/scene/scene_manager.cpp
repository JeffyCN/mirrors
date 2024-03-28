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
#include "RkAiqCalibDbV2.h"
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
    printf("No avaliable main scene for %s!\n", main_scene);
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
    printf("No avaliable sub scene!\n");
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

cJSON *RkAiqSceneManager::findMainScene(cJSON *base_json, const char *name) {
  cJSON *json_item = NULL;
  cJSON *found_main_json = NULL;
  cJSON *main_scene_json = NULL;

  if (!base_json) {
    XCAM_LOG_ERROR("input base json is invalid!\n");
    return NULL;
  }

  main_scene_json = cJSONUtils_GetPointer(base_json, "/main_scene");
  if (!main_scene_json) {
    XCAM_LOG_ERROR("invalid main scene!\n");
    return NULL;
  }

  int main_scene_sum = cJSON_GetArraySize(main_scene_json);
  if (main_scene_sum <= 0) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  json_item = main_scene_json->child;

  for (int i = 0; i <= (main_scene_sum - 1); ++i) {
    if (json_item) {
      char *name_str = cJSON_GetObjectItem(json_item, "name")->valuestring;
      if (name_str && strstr(name_str, name)) {
        found_main_json = json_item;
        break;
      }
          }
    json_item = json_item->next;
  }

  return found_main_json;
}

cJSON *RkAiqSceneManager::findSubScene(cJSON *main_json, const char *name) {
  cJSON *json_item = NULL;
  cJSON *found_sub_json = NULL;
  cJSON *sub_scene_json = NULL;

  if (!main_json) {
    XCAM_LOG_ERROR("input main scene json is invalid!\n");
    return NULL;
  }

  sub_scene_json = cJSONUtils_GetPointer(main_json, "/sub_scene");
  if (!sub_scene_json) {
    XCAM_LOG_ERROR("invalid sub scene!\n");
    return NULL;
  }

  int sub_scene_sum = cJSON_GetArraySize(sub_scene_json);
  if (sub_scene_sum <= 0) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  json_item = sub_scene_json->child;

  for (int i = 0; i < sub_scene_sum; ++i) {
    if (json_item) {
      char *name_str = cJSON_GetObjectItem(json_item, "name")->valuestring;
      if (name_str && strstr(name_str, name)) {
        found_sub_json = json_item;
        break;
      }
    }
    json_item = json_item->next;
  }

  return found_sub_json;
}

cJSON *RkAiqSceneManager::findSubScene(cJSON *json, const char *main_scene,
                                       const char *sub_scene) {
  return findSubScene(findMainScene(json, main_scene), sub_scene);
}

cJSON *RkAiqSceneManager::mergeSubMultiScene(cJSON *sub_scene_list,
                                             cJSON* full_param, bool skip) {
  cJSON *json_item = NULL;
  cJSON *new_item = NULL;
  int sub_scene_sum = 0;
  int i = 0;

  if (!sub_scene_list || !full_param) {
    XCAM_LOG_ERROR("input base json is invalid!\n");
    return NULL;
  }

  // need skip first full param scene
  sub_scene_sum = cJSON_GetArraySize(sub_scene_list);
  if ((skip && sub_scene_sum < 2) ||
      (!skip && sub_scene_sum < 1)) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  json_item = sub_scene_list->child;

  for (i = 0; i < sub_scene_sum; i++) {
    if (json_item) {
      cJSON* temp_item = json_item;
      json_item = json_item->next;
      // skip the full param scene
      if (i == 0 && skip) {
        continue;
      }
      new_item = cJSON_Duplicate(full_param, 1);
      new_item = cJSONUtils_MergePatch(new_item, temp_item);
      cJSON_ReplaceItemInArray(sub_scene_list, i, new_item);
    } else {
      break;
    }
  }
    return sub_scene_list;
}

cJSON *RkAiqSceneManager::mergeMainMultiScene(cJSON *main_scene_list) {
  cJSON *json_item = NULL;
  cJSON *first_sub_scene_list = NULL;
  cJSON *main_first = NULL;
  cJSON *full_param = NULL;

  if (!main_scene_list) {
    XCAM_LOG_ERROR("input main scene list json is invalid!\n");
    return NULL;
  }

  if (cJSON_GetArraySize(main_scene_list) <= 0) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  main_first = cJSON_GetArrayItem(main_scene_list, 0);
  first_sub_scene_list = cJSONUtils_GetPointer(main_first, "/sub_scene");

  if (cJSON_GetArraySize(first_sub_scene_list) <= 0) {
    XCAM_LOG_ERROR("invalid sub scene len!\n");
    return NULL;
  }

  full_param = cJSON_GetArrayItem(first_sub_scene_list, 0);

  if (!full_param) {
    XCAM_LOG_ERROR("invalid full param scene!\n");
    return NULL;
  }

  json_item = main_scene_list->child;

  int main_scene_sum = cJSON_GetArraySize(main_scene_list);
  for (int i = 0; i < main_scene_sum; i++) {
    // need skip first main scene's sub scene
    if (json_item == NULL)
      break;
    cJSON *sub_scene_list = cJSONUtils_GetPointer(json_item, "/sub_scene");
    if (sub_scene_list) {
      mergeSubMultiScene(sub_scene_list, full_param, i == 0);
    }
    json_item = json_item->next;
  }

  return main_scene_list;
}

cJSON *RkAiqSceneManager::mergeMultiSceneIQ(cJSON *base_json) {
  cJSON *main_scene_list_json = NULL;

  // 1. foreach every sub scene.
  // 2. merge every sub scene to base scene.
  // 3. replace sub scene with new json.
  if (!base_json) {
    XCAM_LOG_ERROR("input base json is invalid!\n");
    return NULL;
  }

  main_scene_list_json = cJSONUtils_GetPointer(base_json, "/main_scene");
  if (!main_scene_list_json) {
    XCAM_LOG_ERROR("invalid main scene!\n");
    return NULL;
  }

  if (cJSON_GetArraySize(main_scene_list_json) <= 0) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  if (!mergeMainMultiScene(main_scene_list_json)) {
    return NULL;
  }

  return base_json;
}

CamCalibDbV2Context_t* RkAiqSceneManager::createSceneCalib(
    CamCalibDbProj_t* calibproj,
    const char* main_scene,
    const char* sub_scene) {
  CamCalibDbV2Context_t *calib = NULL;
  CamCalibDbV2Context_t* new_calib = NULL;
  char* json_buff = NULL;
  cJSON* root_json = NULL;
  cJSON* base_json = NULL;
  cJSON* diff_json = NULL;
  cJSON* scene_json = NULL;
  cJSON* calib_json = NULL;
  j2s_ctx ctx;
  int ret = -1;

#if defined(ISP_HW_V20)
    CamCalibDbV2ContextIsp20_t *calib_scene = new CamCalibDbV2ContextIsp20_t;
#elif defined(ISP_HW_V21)
    CamCalibDbV2ContextIsp21_t *calib_scene = new CamCalibDbV2ContextIsp21_t;
#elif defined(ISP_HW_V30)
    CamCalibDbV2ContextIsp30_t *calib_scene = new CamCalibDbV2ContextIsp30_t;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    CamCalibDbV2ContextIsp32_t *calib_scene = new CamCalibDbV2ContextIsp32_t;
#elif defined(ISP_HW_V39)
    CamCalibDbV2ContextIsp39_t *calib_scene = new CamCalibDbV2ContextIsp39_t;
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20 AND V21 AND V30 NOW !"
#endif

  j2s_init(&ctx);
  json_buff = j2s_dump_root_struct(&ctx, calibproj);

  if (!json_buff) {
    XCAM_LOG_ERROR("create CamCalibDbProj json failed.");
    return NULL;
  }

  root_json = cJSON_Parse(json_buff);
  if (!root_json) {
    XCAM_LOG_ERROR("create root json failed.");
    goto error;
  }

  base_json = findSubScene(root_json, "normal", "day");
  diff_json = findSubScene(root_json, main_scene, sub_scene);

  if (!base_json || !diff_json) {
    XCAM_LOG_ERROR("find sub scene json failed.");
    goto error;
  }

  scene_json = cJSONUtils_MergePatch(base_json, diff_json);
  if (!scene_json) {
    XCAM_LOG_ERROR("merge sub scene json failed.");
    goto error;
  }

#if defined(ISP_HW_V20)
    calib_json = cJSONUtils_GetPointer(scene_json, "scene_isp20");
#elif defined(ISP_HW_V21)
    calib_json = cJSONUtils_GetPointer(scene_json, "scene_isp21");
#elif defined(ISP_HW_V30)
    calib_json = cJSONUtils_GetPointer(scene_json, "scene_isp30");
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    calib_json = cJSONUtils_GetPointer(scene_json, "scene_isp32");
#elif defined(ISP_HW_V39)
    calib_json = cJSONUtils_GetPointer(scene_json, "scene_isp39");
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20 AND V21 AND V30 NOW !"
#endif

    calib = RkAiqCalibDbV2::CalibV2Alloc();
    ret = j2s_json_to_struct(&ctx, calib_json,
                             calibdbv2_get_scene_ctx_struct_name(calib),
                             calib_scene);
    if (ret) {
      XCAM_LOG_ERROR("merge sub scene json failed.");
      goto error;
    }

    calib->module_info = &calibproj->module_calib;
    calib->module_info_len = 1;
    calib->sensor_info = &calibproj->sensor_calib;
    calib->sensor_info_len = 1;
    calib->sys_cfg = &calibproj->sys_static_cfg;
    calib->sys_cfg_len = 1;
    calib->calib_scene = (char*)calib_scene;

    new_calib = calib;

error:
  if (json_buff) {
    free(json_buff);
  }

  if (root_json) {
    cJSON_Delete(root_json);
  }

  j2s_deinit(&ctx);

  return new_calib;
}

} // namespace RkCam
