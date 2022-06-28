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
#include "rk_aiq_user_api_sysctl.h"
#include "stdlib.h"
#include "string.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_imgproc.h"
#include "uAPI2/rk_aiq_user_api2_wrapper.h"
#include "uAPI2/rk_aiq_user_api2_agic.h"

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
__RKAIQUAPI_SET_WRAPPER(rk_aiq_user_api2_aie_SetAttrib, aie_attrib_t);
__RKAIQUAPI_SET_WRAPPER(rk_aiq_user_api2_acp_SetAttrib, acp_attrib_t);

/********************** Add Attr caller here ************************/
__RKAIQUAPI_CALLER(uapi_expsw_attr_t);
__RKAIQUAPI_CALLER(uapi_expinfo_t);
__RKAIQUAPI_CALLER(aiq_scene_t);
__RKAIQUAPI_CALLER(work_mode_t);
__RKAIQUAPI_CALLER(uapi_wb_gain_t);
__RKAIQUAPI_CALLER(uapi_wb_mode_t);
__RKAIQUAPI_CALLER(uapiMergeCurrCtlData_t);
__RKAIQUAPI_CALLER(mMergeAttrV30_t);
__RKAIQUAPI_CALLER(DrcInfo_t);
__RKAIQUAPI_CALLER(mdrcAttr_V30_t);
__RKAIQUAPI_CALLER(mDehazeAttr_t);
__RKAIQUAPI_CALLER(uapi_ae_hwstats_t);
__RKAIQUAPI_CALLER(rk_tool_awb_stat_res2_v30_t);
__RKAIQUAPI_CALLER(rk_tool_awb_stat_res_full_t);
__RKAIQUAPI_CALLER(rk_tool_awb_strategy_result_t);
__RKAIQUAPI_CALLER(Adpcc_Manual_Attr_t);
__RKAIQUAPI_CALLER(Agamma_api_manualV30_t);
__RKAIQUAPI_CALLER(RK_SHARP_Params_V4_Select_t);
__RKAIQUAPI_CALLER(RK_Bayer2dnr_Params_V2_Select_t);
__RKAIQUAPI_CALLER(RK_Bayertnr_Params_V2_Select_t);
__RKAIQUAPI_CALLER(RK_YNR_Params_V3_Select_t);
__RKAIQUAPI_CALLER(RK_CNR_Params_V2_Select_t);
__RKAIQUAPI_CALLER(RK_GAIN_Select_V2_t);
__RKAIQUAPI_CALLER(AblcSelect_t);
__RKAIQUAPI_CALLER(rk_aiq_ccm_mccm_attrib_t);
__RKAIQUAPI_CALLER(rk_aiq_lut3d_attrib_t);
__RKAIQUAPI_CALLER(Csm_Param_t);
__RKAIQUAPI_CALLER(rkaiq_gic_v2_api_attr_t);
__RKAIQUAPI_CALLER(aie_attrib_t);
__RKAIQUAPI_CALLER(acp_attrib_t);
__RKAIQUAPI_CALLER(rk_aiq_lsc_table_t);
__RKAIQUAPI_CALLER(camgroup_uapi_t);
__RKAIQUAPI_CALLER(adebayer_attrib_t);

RkAiqUapiDesc_t rkaiq_uapidesc_list[] = {
    __RKAIQUAPI_DESC_DEF("/uapi/0/ae_uapi/expsw_attr", uapi_expsw_attr_t,
                         __RKAIQUAPI_SET_WRAPPER_NAME(rk_aiq_tool_api_ae_setExpSwAttr),
                         rk_aiq_user_api2_ae_getExpSwAttr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ae_uapi/expinfo", uapi_expinfo_t, NULL,
                         rk_aiq_user_api2_ae_queryExpResInfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/awb_uapi/wbgain", uapi_wb_gain_t, rk_aiq_tool_api_setMWBGain,
                         rk_aiq_uapi2_getWBGain),
    __RKAIQUAPI_DESC_DEF("/uapi/0/awb_uapi/mode", uapi_wb_mode_t, rk_aiq_tool_api_setWBMode,
                         rk_aiq_uapi2_getWBMode2),
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/ctldata", uapiMergeCurrCtlData_t, NULL,
                         rk_aiq_user_api2_amerge_GetCtldata),
    __RKAIQUAPI_DESC_DEF("/uapi/0/amerge_uapi/stManual", mMergeAttrV30_t,
                         rk_aiq_set_amerge_manual_attr, rk_aiq_get_amerge_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/stManualV30", mdrcAttr_V30_t,
                         rk_aiq_set_adrc_manual_attr, rk_aiq_get_adrc_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adrc_uapi/info", DrcInfo_t, NULL,
                         rk_aiq_user_api2_adrc_queryinfo),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adehaze_uapi/stManual", mDehazeAttr_t,
                         rk_aiq_set_adehaze_manual_attr, rk_aiq_get_adehaze_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/system/work_mode", work_mode_t,
                         rk_aiq_tool_api_sysctl_swWorkingModeDyn,
                         rk_aiq_uapi_sysctl_getWorkingModeDyn),
    __RKAIQUAPI_DESC_DEF("/uapi/0/system/scene", aiq_scene_t, rk_aiq_tool_api_set_scene,
                         rk_aiq_user_api2_get_scene),
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/ae_hwstats", uapi_ae_hwstats_t, NULL,
                         rk_aiq_uapi_get_ae_hwstats),
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/wb_log/info/awb_stat", rk_tool_awb_stat_res2_v30_t,
                         NULL, rk_aiq_uapi_get_awb_stat),
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/wb_log/info/awb_stat_algo",
                         rk_tool_awb_stat_res_full_t, NULL, rk_aiq_user_api2_awbV30_getAlgoSta),
    __RKAIQUAPI_DESC_DEF("/uapi/0/measure_info/wb_log/info/awb_strategy_result",
                         rk_tool_awb_strategy_result_t, NULL,
                         rk_aiq_user_api2_awbV30_getStrategyResult),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adpcc_manual", Adpcc_Manual_Attr_t, rk_aiq_set_adpcc_manual_attr,
                         rk_aiq_get_adpcc_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/agamma_uapi/stManual", Agamma_api_manualV30_t,
                         rk_aiq_set_agamma_manual_attr, rk_aiq_get_agamma_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/asharp_v4_uapi/manual", RK_SHARP_Params_V4_Select_t,
                         rk_aiq_set_asharp_manual_attr, rk_aiq_get_asharp_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/abayer2dnr_v2_uapi/manual", RK_Bayer2dnr_Params_V2_Select_t,
                         rk_aiq_set_abayer2dnr_manual_attr, rk_aiq_get_abayer2dnr_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/abayertnr_v2_uapi/manual", RK_Bayertnr_Params_V2_Select_t,
                         rk_aiq_set_abayertnr_manual_attr, rk_aiq_get_abayertnr_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/aynr_v3_uapi/manual", RK_YNR_Params_V3_Select_t,
                         rk_aiq_set_aynr_manual_attr, rk_aiq_get_aynr_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/acnr_v2_uapi/manual", RK_CNR_Params_V2_Select_t,
                         rk_aiq_set_acnr_manual_attr, rk_aiq_get_acnr_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/again_v2_uapi/manual", RK_GAIN_Select_V2_t,
                         rk_aiq_set_again_manual_attr, rk_aiq_get_again_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ablc_v30_uapi/bls0", AblcSelect_t, rk_aiq_set_ablc0_manual_attr,
                         rk_aiq_get_ablc0_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/ablc_v30_uapi/bls1", AblcSelect_t, rk_aiq_set_ablc1_manual_attr,
                         rk_aiq_get_ablc1_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/accm_uapi/manual", rk_aiq_ccm_mccm_attrib_t,
                         rk_aiq_set_accm_manual_attr, rk_aiq_get_accm_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/a3dlut_uapi/manual", rk_aiq_lut3d_attrib_t,
                         rk_aiq_set_a3dlut_manual_attr, rk_aiq_get_a3dlut_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/csm_uapi/param", Csm_Param_t, rk_aiq_set_acsm_manual_attr,
                         rk_aiq_get_acsm_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/agicv2_uapi/attr",
                         rkaiq_gic_v2_api_attr_t,
                         rk_aiq_user_api2_agic_v2_SetAttrib,
                         rk_aiq_user_api2_agic_v2_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/aie_uapi/attr",
                         aie_attrib_t,
                         __RKAIQUAPI_SET_WRAPPER_NAME(
                             rk_aiq_user_api2_aie_SetAttrib),
                         rk_aiq_user_api2_aie_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/acp_uapi/attr",
                         acp_attrib_t,
                         __RKAIQUAPI_SET_WRAPPER_NAME(
                         rk_aiq_user_api2_acp_SetAttrib),
                         rk_aiq_user_api2_acp_GetAttrib),
    __RKAIQUAPI_DESC_DEF("/uapi/0/alsc_uapi/manual",
                         rk_aiq_lsc_table_t,
                         rk_aiq_set_alsc_manual_attr,
                         rk_aiq_get_alsc_manual_attr),
    __RKAIQUAPI_DESC_DEF("/uapi/0/camgroup_uapi",
                         camgroup_uapi_t,
                         rk_aiq_set_current_camindex,
                         rk_aiq_get_current_camindex),
    __RKAIQUAPI_DESC_DEF("/uapi/0/adebayer_uapi/adebayer_attr",
                         adebayer_attrib_t,
                         rk_aiq_set_adebayer_attr,
                         rk_aiq_get_adebayer_attr),
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

int rkaiq_uapi_best_match(const char *cmd_path_str) {
  int i = 0;
  int list_len = -1;
  int beset_match = -1;
  int max_length = -1;

  list_len = sizeof(rkaiq_uapidesc_list) / sizeof(RkAiqUapiDesc_t);
  if (list_len <= 0) {
    return -1;
  }

  // Find most match uapi
  for (i = 0; i < list_len; i++) {
    RkAiqUapiDesc_t *temp_uapi_desc = &rkaiq_uapidesc_list[i];
    if (strstr(cmd_path_str, temp_uapi_desc->arg_path)) {
      int path_length = std::string(temp_uapi_desc->arg_path).length();
      if (path_length < max_length) {
        continue;
      }
      max_length = path_length;
      beset_match = i;
    }
  }

  return beset_match;
}

int rkaiq_uapi_unified_ctl(rk_aiq_sys_ctx_t *sys_ctx, const char *js_str,
                           char **ret_str, int op_mode) {
  RkAiqUapiDesc_t *uapi_desc = NULL;
  std::string cmd_path_str;
  std::string final_path = "/";
  cJSON *cmd_js = NULL;
  cJSON *ret_js = NULL;
  cJSON *arr_item = NULL;
  int list_len = -1;
  int change_sum = -1;
  int max_length = -1;
  int i = 0;
  rk_aiq_sys_ctx_t* msys_ctx = NULL;
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
      if (cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring) {
        cmd_path_str = std::string(
            cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring);
        int desc_i = rkaiq_uapi_best_match(cmd_path_str.c_str());
        if (desc_i >= 0) {
          uapi_desc = &rkaiq_uapidesc_list[desc_i];
          if (0 == std::string(uapi_desc->arg_path).compare(cmd_path_str)) {
            final_path = "/";
          } else {
            final_path =
                cmd_path_str.substr(std::string(uapi_desc->arg_path).length());
          }
          cJSON_ReplaceItemInObject(arr_item, JSON_PATCH_PATH,
                                    cJSON_CreateString(final_path.c_str()));
        }
      }
    }
    arr_item = arr_item->next;
  }

  if (!uapi_desc) {
    XCAM_LOG_ERROR("can't find uapi for %s\n", cmd_path_str.c_str());
    return -1;
  }

  msys_ctx = rk_aiq_get_last_sysctx(sys_ctx);
  if (!msys_ctx) {
    XCAM_LOG_ERROR("can't find valid ctx\n");
    return -1;
  }

  uapi_desc->uapi_caller(uapi_desc, msys_ctx, cmd_js,
                         (void **)&ret_js, op_mode);

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
