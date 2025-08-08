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
#include "c_base/aiq_base.h"
#include "j2s.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

cJSON *RkAiqSceneManagerLoadIQFile(const char *name) {
  char *json_data = NULL;
  size_t json_data_size = 0;
  cJSON *out_json = NULL;

  json_data = (char *)j2s_read_file(name, &json_data_size);
  out_json = RkCam_cJSON_Parse(json_data);
  if (json_data) {
    aiq_free(json_data);
  }

  return out_json;
}

CamCalibDbV2Context_t
RkAiqSceneManagerRefToScene(CamCalibDbProj_t *calibproj,
                              const char *main_scene, const char *sub_scene) {
  CamCalibDbV2Context_t ctx;
  CamCalibMainSceneList_t *main_list = NULL;
  CamCalibSubSceneList_t *sub_list = NULL;
  void* dst_calib = NULL;
  int main_list_len = -1;
  int sub_list_len = -1;
  int curr_main_scene = 0;
  int curr_sub_scene = 0;

  memset(&ctx, 0, sizeof(ctx));
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
    ctx.scene_cis   = calibdbv2_get_scene_cis_ptr(&sub_list[curr_sub_scene]);

    return ctx;
  }

  printf("Can't find scene:[%s]/[%s]!\n", main_scene, sub_scene);

  return ctx;
}

cJSON *RkAiqSceneManagerFindMainScene(cJSON *base_json, const char *name) {
  cJSON *json_item = NULL;
  cJSON *found_main_json = NULL;
  cJSON *main_scene_json = NULL;

  if (!base_json) {
    XCAM_LOG_ERROR("input base json is invalid!\n");
    return NULL;
  }

  main_scene_json = RkCam_cJSONUtils_GetPointer(base_json, "/main_scene");
  if (!main_scene_json) {
    XCAM_LOG_ERROR("invalid main scene!\n");
    return NULL;
  }

  int main_scene_sum = RkCam_cJSON_GetArraySize(main_scene_json);
  if (main_scene_sum <= 0) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  json_item = main_scene_json->child;

  for (int i = 0; i <= (main_scene_sum - 1); ++i) {
    if (json_item) {
      char *name_str = RkCam_cJSON_GetObjectItem(json_item, "name")->valuestring;
      if (name_str && strstr(name_str, name)) {
        found_main_json = json_item;
        break;
      }
          }
    json_item = json_item->next;
  }

  return found_main_json;
}

static cJSON *findSubScene(cJSON *main_json, const char *name) {
  cJSON *json_item = NULL;
  cJSON *found_sub_json = NULL;
  cJSON *sub_scene_json = NULL;

  if (!main_json) {
    XCAM_LOG_ERROR("input main scene json is invalid!\n");
    return NULL;
  }

  sub_scene_json = RkCam_cJSONUtils_GetPointer(main_json, "/sub_scene");
  if (!sub_scene_json) {
    XCAM_LOG_ERROR("invalid sub scene!\n");
    return NULL;
  }

  int sub_scene_sum = RkCam_cJSON_GetArraySize(sub_scene_json);
  if (sub_scene_sum <= 0) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  json_item = sub_scene_json->child;

  for (int i = 0; i < sub_scene_sum; ++i) {
    if (json_item) {
      char *name_str = RkCam_cJSON_GetObjectItem(json_item, "name")->valuestring;
      if (name_str && strstr(name_str, name)) {
        found_sub_json = json_item;
        break;
      }
    }
    json_item = json_item->next;
  }

  return found_sub_json;
}

cJSON *RkAiqSceneManagerFindSubScene(cJSON *json, const char *main_scene,
                                       const char *sub_scene) {
  return findSubScene(RkAiqSceneManagerFindMainScene(json, main_scene), sub_scene);
}

static cJSON *mergeSubMultiScene(cJSON *sub_scene_list,
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
  sub_scene_sum = RkCam_cJSON_GetArraySize(sub_scene_list);
  if ((skip && sub_scene_sum < 2) ||
      (!skip && sub_scene_sum < 1)) {
    if (!skip)
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

      // JSON diff parameters based on main_scene[X].sub_scene[0]
      if (!skip && i > 0) {
          cJSON* first_sub_scene_item = RkCam_cJSON_GetArrayItem(sub_scene_list, 0);
          if (!first_sub_scene_item)
              XCAM_LOG_ERROR("invalid first sub scene item!\n");
          else
              full_param = first_sub_scene_item;
      }

      new_item = RkCam_cJSON_Duplicate(full_param, 1);
      new_item = RkCam_cJSONUtils_MergePatch(new_item, temp_item);
      RkCam_cJSON_ReplaceItemInArray(sub_scene_list, i, new_item);
    } else {
      break;
    }
  }
    return sub_scene_list;
}

static cJSON *mergeMainMultiScene(cJSON *main_scene_list) {
  cJSON *json_item = NULL;
  cJSON *first_sub_scene_list = NULL;
  cJSON *main_first = NULL;
  cJSON *full_param = NULL;

  if (!main_scene_list) {
    XCAM_LOG_ERROR("input main scene list json is invalid!\n");
    return NULL;
  }

  if (RkCam_cJSON_GetArraySize(main_scene_list) <= 0) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  main_first = RkCam_cJSON_GetArrayItem(main_scene_list, 0);
  first_sub_scene_list = RkCam_cJSONUtils_GetPointer(main_first, "/sub_scene");

  if (RkCam_cJSON_GetArraySize(first_sub_scene_list) <= 0) {
    XCAM_LOG_ERROR("invalid sub scene len!\n");
    return NULL;
  }

  full_param = RkCam_cJSON_GetArrayItem(first_sub_scene_list, 0);

  if (!full_param) {
    XCAM_LOG_ERROR("invalid full param scene!\n");
    return NULL;
  }

  json_item = main_scene_list->child;

  int main_scene_sum = RkCam_cJSON_GetArraySize(main_scene_list);
  for (int i = 0; i < main_scene_sum; i++) {
    // need skip first main scene's sub scene
    if (json_item == NULL)
      break;
    cJSON *sub_scene_list = RkCam_cJSONUtils_GetPointer(json_item, "/sub_scene");
    if (sub_scene_list) {
      mergeSubMultiScene(sub_scene_list, full_param, i == 0);
    }
    json_item = json_item->next;
  }

  return main_scene_list;
}

cJSON *RkAiqSceneManagerMergeMultiSceneIQ(cJSON *base_json) {
  cJSON *main_scene_list_json = NULL;

  // 1. foreach every sub scene.
  // 2. merge every sub scene to base scene.
  // 3. replace sub scene with new json.
  if (!base_json) {
    XCAM_LOG_ERROR("input base json is invalid!\n");
    return NULL;
  }

  main_scene_list_json = RkCam_cJSONUtils_GetPointer(base_json, "/main_scene");
  if (!main_scene_list_json) {
    XCAM_LOG_ERROR("invalid main scene!\n");
    return NULL;
  }

  if (RkCam_cJSON_GetArraySize(main_scene_list_json) <= 0) {
    XCAM_LOG_ERROR("invalid main scene len!\n");
    return NULL;
  }

  if (!mergeMainMultiScene(main_scene_list_json)) {
    return NULL;
  }

  return base_json;
}

CamCalibDbV2Context_t* RkAiqSceneManagerCreateSceneCalib(
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
    CamCalibDbV2ContextIsp20_t *calib_scene = aiq_mallocz(sizeof(CamCalibDbV2ContextIsp20_t));
#elif defined(ISP_HW_V21)
    CamCalibDbV2ContextIsp21_t *calib_scene = aiq_mallocz(sizeof(CamCalibDbV2ContextIsp21_t));
#elif defined(ISP_HW_V30)
    CamCalibDbV2ContextIsp30_t *calib_scene = aiq_mallocz(sizeof(CamCalibDbV2ContextIsp30_t));
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    CamCalibDbV2ContextIsp32_t *calib_scene = aiq_mallocz(sizeof(CamCalibDbV2ContextIsp32_t));
#elif defined(ISP_HW_V39)
    CamCalibDbV2ContextIsp39_t *calib_scene = aiq_mallocz(sizeof(CamCalibDbV2ContextIsp39_t));
#elif defined(ISP_HW_V33)
    CamCalibDbV2ContextIsp33_t *calib_scene = aiq_mallocz(sizeof(CamCalibDbV2ContextIsp33_t));
#elif defined(ISP_HW_V35)
    CamCalibDbV2ContextIsp35_t *calib_scene = aiq_mallocz(sizeof(CamCalibDbV2ContextIsp35_t));
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20 AND V21 AND V30 NOW !"
#endif

  j2s_init(&ctx);
  json_buff = j2s_dump_root_struct(&ctx, calibproj);

  if (!json_buff) {
    XCAM_LOG_ERROR("create CamCalibDbProj json failed.");
    return NULL;
  }

  root_json = RkCam_cJSON_Parse(json_buff);
  if (!root_json) {
    XCAM_LOG_ERROR("create root json failed.");
    goto error;
  }

  base_json = RkAiqSceneManagerFindSubScene(root_json, "normal", "day");
  diff_json = RkAiqSceneManagerFindSubScene(root_json, main_scene, sub_scene);

  if (!base_json || !diff_json) {
    XCAM_LOG_ERROR("find sub scene json failed.");
    goto error;
  }

  scene_json = RkCam_cJSONUtils_MergePatch(base_json, diff_json);
  if (!scene_json) {
    XCAM_LOG_ERROR("merge sub scene json failed.");
    goto error;
  }

#if defined(ISP_HW_V20)
    calib_json = RkCam_cJSONUtils_GetPointer(scene_json, "scene_isp20");
#elif defined(ISP_HW_V21)
    calib_json = RkCam_cJSONUtils_GetPointer(scene_json, "scene_isp21");
#elif defined(ISP_HW_V30)
    calib_json = RkCam_cJSONUtils_GetPointer(scene_json, "scene_isp30");
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    calib_json = RkCam_cJSONUtils_GetPointer(scene_json, "scene_isp32");
#elif defined(ISP_HW_V39)
    calib_json = RkCam_cJSONUtils_GetPointer(scene_json, "scene_isp39");
#elif defined(ISP_HW_V33)
    calib_json = RkCam_cJSONUtils_GetPointer(scene_json, "scene_isp33");
#elif defined(ISP_HW_V35)
    calib_json = RkCam_cJSONUtils_GetPointer(scene_json, "scene_isp35");
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20 AND V21 AND V30 NOW !"
#endif

    calib = CamCalibDbCalibV2Alloc();
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
    aiq_free(json_buff);
  }

  if (root_json) {
    RkCam_cJSON_Delete(root_json);
  }

  j2s_deinit(&ctx);

  return new_calib;
}

