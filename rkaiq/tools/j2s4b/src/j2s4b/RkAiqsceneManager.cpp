#include "RkAiqsceneManager.h"

#include <cstring>

cJSON *RkAiqsceneManager::mergeSubMultiScene(cJSON *sub_scene_list,
                                             cJSON *full_param, bool skip) {
  cJSON *json_item = NULL;
  cJSON *new_item = NULL;
  int sub_scene_sum = 0;
  int i = 0;

  if (!sub_scene_list || !full_param) {
    printf("input base json is invalid!\n");
    return NULL;
  }

  // need skip first full param scene
  if (cJSON_GetArraySize(sub_scene_list) <= skip) {
    printf("%s %d only one sub scene, ignore!\n", __func__, __LINE__);
    return NULL;
  }

  json_item = sub_scene_list->child;

  sub_scene_sum = cJSON_GetArraySize(sub_scene_list);
  for (i = 0; i < sub_scene_sum; i++) {
    if (json_item) {
      cJSON *temp_item = json_item;
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

cJSON *RkAiqsceneManager::mergeMainMultiScene(cJSON *main_scene_list) {
  cJSON *json_item = NULL;
  cJSON *first_sub_scene_list = NULL;
  cJSON *main_first = NULL;
  cJSON *full_param = NULL;

  if (!main_scene_list) {
    printf("input main scene list json is invalid!\n");
    return NULL;
  }

  if (cJSON_GetArraySize(main_scene_list) <= 0) {
    printf("invalid main scene len!\n");
    return NULL;
  }

  main_first = cJSON_GetArrayItem(main_scene_list, 0);
  first_sub_scene_list = cJSONUtils_GetPointer(main_first, "/sub_scene");

  if (cJSON_GetArraySize(first_sub_scene_list) <= 0) {
    printf("%s %d invalid sub scene len!\n", __func__, __LINE__);
    return NULL;
  }

  full_param = cJSON_GetArrayItem(first_sub_scene_list, 0);

  if (!full_param) {
    printf("invalid full param scene!\n");
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

cJSON *RkAiqsceneManager::mergeMultiSceneIQ(cJSON *base_json) {
  cJSON *main_scene_list_json = NULL;

  // 1. foreach every sub scene.
  // 2. merge every sub scene to base scene.
  // 3. replace sub scene with new json.
  if (!base_json) {
    printf("input base json is invalid!\n");
    return NULL;
  }

  main_scene_list_json = cJSONUtils_GetPointer(base_json, "/main_scene");
  if (!main_scene_list_json) {
    printf("invalid main scene!\n");
    return NULL;
  }

  if (cJSON_GetArraySize(main_scene_list_json) <= 0) {
    printf("invalid main scene len!\n");
    return NULL;
  }

  if (!mergeMainMultiScene(main_scene_list_json)) {
    return NULL;
  }

  return base_json;
}

cJSON *RkAiqsceneManager::findMainScene(cJSON *base_json, const char *name) {
  cJSON *json_item = NULL;
  cJSON *found_main_json = NULL;
  cJSON *main_scene_json = NULL;

  if (!base_json) {
    printf("input base json is invalid!\n");
    return NULL;
  }

  main_scene_json = cJSONUtils_GetPointer(base_json, "/main_scene");
  if (!main_scene_json) {
    printf("invalid main scene!\n");
    return NULL;
  }

  int main_scene_sum = cJSON_GetArraySize(main_scene_json);
  if (main_scene_sum <= 0) {
    printf("invalid main scene len!\n");
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

cJSON *RkAiqsceneManager::findSubScene(cJSON *main_json, const char *name) {
  cJSON *json_item = NULL;
  cJSON *found_sub_json = NULL;
  cJSON *sub_scene_json = NULL;

  if (!main_json) {
    printf("input main scene json is invalid!\n");
    return NULL;
  }

  sub_scene_json = cJSONUtils_GetPointer(main_json, "/sub_scene");
  if (!sub_scene_json) {
    printf("invalid sub scene!\n");
    return NULL;
  }

  int sub_scene_sum = cJSON_GetArraySize(sub_scene_json);
  if (sub_scene_sum <= 0) {
    printf("%s %d invalid sub scene len!\n", __func__, __LINE__);
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

cJSON *RkAiqsceneManager::findSubScene(cJSON *json, const char *main_scene,
                                       const char *sub_scene) {

  return findSubScene(findMainScene(json, main_scene), sub_scene);
}

std::map<std::string, cJSON *>
RkAiqsceneManager::getMainSceneList(cJSON *root_js) {
  cJSON *json_item = NULL;
  cJSON *main_scene_json = NULL;
  std::map<std::string, cJSON *> mMap;

  if (!root_js) {
    printf("input base json is invalid!\n");
    return {};
  }

  main_scene_json = cJSONUtils_GetPointer(root_js, "/main_scene");
  if (!main_scene_json) {
    printf("invalid main scene!\n");
    return {};
  }

  int main_scene_sum = cJSON_GetArraySize(main_scene_json);
  if (main_scene_sum <= 0) {
    printf("invalid main scene len!\n");
    return {};
  }

  json_item = main_scene_json->child;

  for (int i = 0; i <= (main_scene_sum - 1); ++i) {
    if (json_item) {
      char *name_str = cJSON_GetObjectItem(json_item, "name")->valuestring;
      if (name_str && strlen(name_str) > 0) {
        mMap[std::string(name_str)] = json_item;
      }
    }
    json_item = json_item->next;
  }

  return mMap;
}

std::map<std::string, cJSON *>
RkAiqsceneManager::getSubSceneList(cJSON *main_js, const std::string name) {
  cJSON *json_item = NULL;
  cJSON *sub_scene_json = NULL;
  std::map<std::string, cJSON *> mMap;

  if (!main_js) {
    printf("input main scene json is invalid!\n");
    return {};
  }

  sub_scene_json = cJSONUtils_GetPointer(main_js, "/sub_scene");
  if (!sub_scene_json) {
    printf("invalid sub scene!\n");
    return {};
  }

  int sub_scene_sum = cJSON_GetArraySize(sub_scene_json);
  if (sub_scene_sum <= 0) {
    printf("%s %d invalid sub scene len!\n", __func__, __LINE__);
    return {};
  }

  json_item = sub_scene_json->child;

  for (int i = 0; i < sub_scene_sum; ++i) {
    if (json_item) {
      char *name_str = cJSON_GetObjectItem(json_item, "name")->valuestring;
      if (name_str && strlen(name_str) > 0) {
        mMap[std::string(name + "/" + std::string(name_str))] = json_item;
      }
    }
    json_item = json_item->next;
  }

  return mMap;
}

std::map<std::string, cJSON *>
RkAiqsceneManager::expandAllScene(cJSON *root_js) {
  if (!root_js) {
    return {};
  }
  return {};
}

cJSON *RkAiqsceneManager::splitScene(cJSON *root_js, const char *main_scene,
                                     const char *sub_scene) {
  cJSON *new_json = NULL;
  std::string scene_index = "";

  if (!root_js || !main_scene || !sub_scene) {
    printf("input is invalid argument!\n");
    return NULL;
  }

  scene_index = std::string(main_scene) + "/" + std::string(sub_scene);

  new_json = cJSON_Duplicate(root_js, 1);

  if (!new_json) {
    printf("clone json failed!\n");
    return NULL;
  }

  std::map<std::string, cJSON *> main_scene_list = getMainSceneList(new_json);

  for (auto mScene : main_scene_list) {
    // delete main scene witch we don't want.
    if (0 != std::string(main_scene).compare(mScene.first) && mScene.second) {
      cJSON_DetachItemViaPointer(new_json, mScene.second);
      main_scene_list.erase(mScene.first);
    }
  }

  return new_json;
}

