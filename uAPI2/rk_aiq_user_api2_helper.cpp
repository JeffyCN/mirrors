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

#include "uAPI2/rk_aiq_user_api2_helper.h"
#include "RkAiqCalibDbV2.h"
#include "RkAiqUapitypes.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "j2s.h"
#include "stdlib.h"
#include "string.h"
#include "uAPI/rk_aiq_user_api_sysctl.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_imgproc.h"
#include "uAPI2/rk_aiq_user_api2_wrapper.h"

#include "rk_aiq_tool_api.h"

// json cmd example {
//    "op":"replace",
//    "path":"/uapi/0/system/crop"
//    "value":"{
//      "x":0,
//      "y":0,
//      "w":1280,
//      "h":720
//    }"
// }
//
// json cmd example {
//    "op":"replace",
//    "path":"/uapi/0/ae_uapi/opmode"
//    "value":"{
//      1
//    }"
// }
//

/*****************Add UAPI wrapper here if necessary*****************/
__RKAIQUAPI_SET_WRAPPER(rk_aiq_tool_api_ae_setExpSwAttr, Uapi_ExpSwAttrV2_t);
__RKAIQUAPI_SET_WRAPPER(rk_aiq_user_api2_adrc_SetAttrib, drcAttr_t);

/********************** Add Attr caller here ************************/
__RKAIQUAPI_CALLER(uapi_expsw_attr_t);
__RKAIQUAPI_CALLER(uapi_expinfo_t);
__RKAIQUAPI_CALLER(aiq_scene_t);
__RKAIQUAPI_CALLER(work_mode_t);
__RKAIQUAPI_CALLER(uapi_wb_gain_t);
__RKAIQUAPI_CALLER(uapi_wb_mode_t);
__RKAIQUAPI_CALLER(uapiMergeCurrCtlData_t);
__RKAIQUAPI_CALLER(drcAttr_t);

RkAiqUapiDesc_t rkaiq_uapidesc_list[] = {
    __RKAIQUAPI_DESC_DEF(
        "/uapi/0/ae_uapi/expsw_attr", uapi_expsw_attr_t,
        __RKAIQUAPI_SET_WRAPPER_NAME(rk_aiq_tool_api_ae_setExpSwAttr),
        rk_aiq_user_api2_ae_getExpSwAttr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ae_uapi/expinfo", uapi_expinfo_t, NULL,
                         rk_aiq_user_api_ae_queryExpResInfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/awb_uapi/wbgain", uapi_wb_gain_t,
                         rk_aiq_tool_api_setMWBGain, rk_aiq_uapi2_getWBGain),
    __RKAIQUAPI_DESC_DEF("/uapi/0/awb_uapi/mode", uapi_wb_mode_t,
                         rk_aiq_tool_api_setWBMode, rk_aiq_uapi2_getWBMode2),
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/ctldata", uapiMergeCurrCtlData_t,
                         rk_aiq_tool_api_amerge_SetAttrib2,
                         rk_aiq_user_api2_amerge_GetAttrib2),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/drc", drcAttr_t,
                         rk_aiq_tool_api_adrc_SetAttrib,
                         rk_aiq_user_api2_adrc_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/system/work_mode", work_mode_t,
                         rk_aiq_tool_api_sysctl_swWorkingModeDyn,
                         rk_aiq_uapi_sysctl_getWorkingModeDyn),
    __RKAIQUAPI_DESC_DEF("/uapi/0/system/scene", aiq_scene_t,
                         rk_aiq_tool_api_set_scene,
                         rk_aiq_user_api2_get_scene),
};
/***********************END OF CUSTOM AREA**************************/

char *rkaiq_uapi_rpc_response(const char *cmd_path, cJSON *root_js,
                              const char *sub_node) {
  char *ret_str = NULL;
  cJSON *ret_json = NULL;
  cJSON *node_json = NULL;
  ret_json = cJSON_CreateArray();

  if (!root_js || !sub_node) {
    XCAM_LOG_ERROR("invalid json argument for sysctl!");
    return NULL;
  }

  if (0 == strcmp(sub_node, "/")) {
    node_json = root_js;
  } else {
    node_json = cJSONUtils_GetPointer(root_js, sub_node);
    if (node_json)
      cJSON_DetachItemViaPointer(root_js, node_json);
  }

  if (root_js) {
    cJSON *ret_item = cJSON_CreateObject();
    cJSON_AddStringToObject(ret_item, JSON_PATCH_PATH, cmd_path);
    cJSON_AddItemToObject(ret_item, JSON_PATCH_VALUE, node_json);
    cJSON_AddItemToArray(ret_json, ret_item);
  }

  ret_str = cJSON_PrintUnformatted(ret_json);

  if (ret_json)
    cJSON_free(ret_json);

  if (node_json && node_json != root_js)
    cJSON_free(node_json);

  return ret_str;
}

int rkaiq_uapi_unified_ctl(rk_aiq_sys_ctx_t *sys_ctx, const char *js_str,
                           char **ret_str, int op_mode) {
  RkAiqUapiDesc_t *uapi_desc = NULL;
  std::string cmd_path_str;
  std::string final_path = "/";
  char *cmd_path = NULL;
  cJSON *cmd_js = NULL;
  cJSON *ret_js = NULL;
  cJSON *arr_item = NULL;
  int list_len = -1;
  int change_sum = -1;
  int i = 0;
  *ret_str = NULL;

  list_len = sizeof(rkaiq_uapidesc_list) / sizeof(RkAiqUapiDesc_t);
  if (list_len <= 0) {
    return -1;
  }

  cmd_js = cJSON_Parse(js_str);
  change_sum = cJSON_GetArraySize(cmd_js);

  if (change_sum <= 0) {
    XCAM_LOG_ERROR("can't find json patch operation\n");
    return -1;
  }

  arr_item = cmd_js->child;

  for (int i = 0; i <= (change_sum - 1); ++i) {
    if (arr_item) {
      if (strdup(cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring)) {
        cmd_path_str = std::string(
            cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring);
        auto pos = cmd_path_str.find_first_not_of("/");
        pos = cmd_path_str.find_first_of("/", pos);
        pos = cmd_path_str.find_first_not_of("/", pos);
        pos = cmd_path_str.find_first_of("/", pos);
        pos = cmd_path_str.find_first_not_of("/", pos);
        pos = cmd_path_str.find_first_of("/", pos);
        pos = cmd_path_str.find_first_not_of("/", pos);
        pos = cmd_path_str.find_first_of("/", pos);
        auto final_cmd_path_str = cmd_path_str.substr(0, pos);

        printf("[UAPI RPC]final cmd path str:\n%s\n",
               final_cmd_path_str.c_str());

        cmd_path = strdup(final_cmd_path_str.c_str());

        auto path_str = std::string(
            cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring);
        final_path = "/";
        // remove /uapi/xxx/
        if (pos < cmd_path_str.size()) {
          final_path = cmd_path_str.substr(pos);
        }
        cJSON_ReplaceItemInObject(arr_item, JSON_PATCH_PATH,
                                  cJSON_CreateString(final_path.c_str()));
      }
    }
    arr_item = arr_item->next;
  }

  for (i = 0; i < list_len; i++) {
    RkAiqUapiDesc_t *temp_uapi_desc = &rkaiq_uapidesc_list[i];
    if (strstr(temp_uapi_desc->arg_path, cmd_path)) {
      uapi_desc = temp_uapi_desc;
      break;
    }
  }

  if (!uapi_desc) {
    XCAM_LOG_ERROR("can't find uapi for %s\n", cmd_path);
    return -1;
  }

  printf("[UAPI RPC] final json cmd:\n%s\n", cJSON_Print(cmd_js));

  uapi_desc->uapi_caller(uapi_desc, sys_ctx, cmd_js, (void **)&ret_js, op_mode);

  if (op_mode == RKAIQUAPI_OPMODE_SET) {
    *ret_str = NULL;
  } else if (op_mode == RKAIQUAPI_OPMODE_GET) {
    if (ret_js) {
      *ret_str = rkaiq_uapi_rpc_response(cmd_path_str.c_str(), ret_js,
                                         final_path.c_str());
      cJSON_free(ret_js);
    }
  }

  return 0;
}
