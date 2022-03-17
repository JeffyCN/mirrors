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

#ifndef __RK_AIQ_USER_API2_WRAPPER_H__
#define __RK_AIQ_USER_API2_WRAPPER_H__

#include "rk_aiq_user_api_sysctl.h"

int rk_aiq_uapi_sysctl_swWorkingModeDyn2(const rk_aiq_sys_ctx_t *ctx,
                                         work_mode_t *mode);

int rk_aiq_uapi_sysctl_getWorkingModeDyn(const rk_aiq_sys_ctx_t *ctx,
                                         work_mode_t *mode);

int rk_aiq_uapi2_setWBMode2(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode);

int rk_aiq_uapi2_getWBMode2(rk_aiq_sys_ctx_t *ctx, uapi_wb_mode_t *mode);

int rk_aiq_user_api2_amerge_SetAttrib2(const rk_aiq_sys_ctx_t *sys_ctx,
                                       uapiMergeCurrCtlData_t *ctldata);

int rk_aiq_user_api2_amerge_GetAttrib2(const rk_aiq_sys_ctx_t *sys_ctx,
                                       uapiMergeCurrCtlData_t *ctldata);

int rk_aiq_user_api2_set_scene(const rk_aiq_sys_ctx_t *sys_ctx,
                               aiq_scene_t *scene);

int rk_aiq_user_api2_get_scene(const rk_aiq_sys_ctx_t *sys_ctx,
                               aiq_scene_t *scene);

#endif /*__RK_AIQ_USER_API2_WRAPPER_H__*/
