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
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#include "rk_aiq_user_api_sysctl.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_api2_imgproc.h"
#include "uAPI/include/rk_aiq_user_api_sysctl.h"

int rk_aiq_uapi_sysctl_swWorkingModeDyn2(const rk_aiq_sys_ctx_t *ctx,
        work_mode_t *mode) {
    return rk_aiq_uapi_sysctl_swWorkingModeDyn(ctx, mode->mode);
}

int rk_aiq_uapi_sysctl_getWorkingModeDyn(const rk_aiq_sys_ctx_t *ctx,
        work_mode_t *mode) {
    return rk_aiq_uapi2_sysctl_getWorkingMode(ctx, &mode->mode);
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

    if(CHECK_ISP_HW_V21())
        memcpy(&setdata.attrV21.CtlInfo, ctldata, sizeof(uapiMergeCurrCtlData_t));
    else if(CHECK_ISP_HW_V30())
        memcpy(&setdata.attrV30.CtlInfo, ctldata, sizeof(uapiMergeCurrCtlData_t));

    return rk_aiq_user_api2_amerge_SetAttrib(sys_ctx, setdata);
}

int rk_aiq_user_api2_amerge_GetAttrib2(const rk_aiq_sys_ctx_t* sys_ctx, uapiMergeCurrCtlData_t* ctldata)
{
    amerge_attrib_t setdata;

    memset(&setdata, 0, sizeof(amerge_attrib_t));

    rk_aiq_user_api2_amerge_GetAttrib(sys_ctx, &setdata);

    if(CHECK_ISP_HW_V21())
        memcpy(ctldata, &setdata.attrV21.CtlInfo, sizeof(uapiMergeCurrCtlData_t));
    else if(CHECK_ISP_HW_V30())
        memcpy(ctldata, &setdata.attrV30.CtlInfo, sizeof(uapiMergeCurrCtlData_t));

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

int rk_aiq_uapi_get_ae_hwstats(const rk_aiq_sys_ctx_t* sys_ctx, uapi_ae_hwstats_t* ae_hwstats)
{
  rk_aiq_isp_stats_t isp_stats;
  rk_aiq_uapi_sysctl_get3AStats(sys_ctx, &isp_stats);

  memcpy(ae_hwstats, &isp_stats.aec_stats.ae_data, sizeof(uapi_ae_hwstats_t));

  return 0;
}

int rk_aiq_uapi_get_awb_stat(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_stat_res2_v30_t* awb_stat)
{
  rk_aiq_isp_stats_t isp_stats;
  rk_aiq_uapi_sysctl_get3AStats(sys_ctx, &isp_stats);

  memcpy(awb_stat, &isp_stats.awb_stats_v3x, sizeof(rk_tool_awb_stat_res2_v30_t));

  return 0;
}
