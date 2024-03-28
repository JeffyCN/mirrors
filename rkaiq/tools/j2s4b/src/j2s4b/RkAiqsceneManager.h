#ifndef ___RK_AIQSCENE_MANAGER_H__
#define ___RK_AIQSCENE_MANAGER_H__

#include "cJSON_Utils.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <map>
#include <string>

using namespace RkCam;

class RkAiqsceneManager {
public:
  explicit RkAiqsceneManager() = default;
  ~RkAiqsceneManager() = default;

public:
  static cJSON *mergeSubMultiScene(cJSON *sub_scene_list, cJSON *full_param,
                                   bool skip);

  static cJSON *mergeMainMultiScene(cJSON *main_scene_list);

  static cJSON *mergeMultiSceneIQ(cJSON *base_json);

  static cJSON *findMainScene(cJSON *base_json, const char *name);

  static cJSON *findSubScene(cJSON *main_json, const char *name);

  static cJSON *findSubScene(cJSON *json, const char *main_scene,
                             const char *sub_scene);

  static cJSON *splitScene(cJSON *root_js, const char *main_scene,
                           const char *sub_scene);

  static std::map<std::string, cJSON *> expandAllScene(cJSON *root_js);
  static std::map<std::string, cJSON *> getMainSceneList(cJSON *root_js);
  static std::map<std::string, cJSON *> getSubSceneList(cJSON *root_js,
                                                        const std::string name);
};

#endif /*___RK_AIQSCENE_MANAGER_H__*/
