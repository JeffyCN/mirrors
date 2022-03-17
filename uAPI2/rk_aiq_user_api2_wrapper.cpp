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

#include "RkAiqCalibDbV2.h"
#include "RkAiqUapitypes.h"
#include "cJSON.h"
#include "cJSON_Utils.h"
#include "j2s.h"
#include "stdlib.h"
#include "string.h"
#include "uAPI2/rk_aiq_user_api2_wrapper.h"
#include "uAPI/rk_aiq_user_api_sysctl.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_imgproc.h"

int rk_aiq_uapi_sysctl_swWorkingModeDyn2(const rk_aiq_sys_ctx_t *ctx,
                                         work_mode_t *mode) {
  return rk_aiq_uapi_sysctl_swWorkingModeDyn(ctx, mode->mode);
}

int rk_aiq_uapi_sysctl_getWorkingModeDyn(const rk_aiq_sys_ctx_t *ctx,
                                         work_mode_t *mode) {
  (void)ctx;
  mode->mode = RK_AIQ_WORKING_MODE_NORMAL;
  return 0;
}

int rk_aiq_uapi2_setWBMode2(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode) {
  return rk_aiq_uapi2_setWBMode(ctx, mode->mode);
}

int rk_aiq_uapi2_getWBMode2(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode) {
  return rk_aiq_uapi2_getWBMode(ctx, &mode->mode);
}

int rk_aiq_user_api2_amerge_SetAttrib2(const rk_aiq_sys_ctx_t* sys_ctx, uapiMergeCurrCtlData_t* ctldata)
{
  amerge_attrib_t setdata;

  memset(&setdata, 0, sizeof(amerge_attrib_t));

  rk_aiq_user_api2_amerge_GetAttrib(sys_ctx, &setdata);

  memcpy(&setdata.CtlInfo, ctldata, sizeof(uapiMergeCurrCtlData_t));

  return rk_aiq_user_api2_amerge_SetAttrib(sys_ctx, setdata);
}

int rk_aiq_user_api2_amerge_GetAttrib2(const rk_aiq_sys_ctx_t* sys_ctx, uapiMergeCurrCtlData_t* ctldata)
{
  amerge_attrib_t setdata;

  memset(&setdata, 0, sizeof(amerge_attrib_t));

  rk_aiq_user_api2_amerge_GetAttrib(sys_ctx, &setdata);

  memcpy(ctldata, &setdata.CtlInfo, sizeof(uapiMergeCurrCtlData_t));

  return 0;
}

int rk_aiq_user_api2_set_scene(const rk_aiq_sys_ctx_t* sys_ctx, aiq_scene_t* scene)
{
  return rk_aiq_uapi_sysctl_switch_scene(sys_ctx, scene->main_scene, scene->sub_scene);
}

int rk_aiq_user_api2_get_scene(const rk_aiq_sys_ctx_t* sys_ctx, aiq_scene_t* scene)
{
  (void)sys_ctx;
  scene->main_scene = strdup("normal");
  scene->sub_scene = strdup("day");

  return 0;
}

