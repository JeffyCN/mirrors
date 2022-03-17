/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _ISP20_MODULE_DBG_H_
#define _ISP20_MODULE_DBG_H_

#define RUNTIME_MODULE_DEBUG
#ifdef RUNTIME_MODULE_DEBUG
#include <stdint.h>

extern unsigned long long g_disable_isp_modules_en;
extern unsigned long long g_disable_isp_modules_cfg_update;
extern int g_disable_ispp_modules_en;
extern int g_disable_ispp_modules_cfg_update;
extern int g_bypass_exp_params;
extern int g_bypass_isp_params;
extern int g_bypass_ispp_params;
extern int g_apply_init_params_only;
extern uint64_t g_disable_algo_user_api_mask;
extern void get_dbg_force_disable_mods_env();
extern int get_rkaiq_runtime_dbg();
#define CHECK_USER_API_ENABLE(mask) \
    if (g_disable_algo_user_api_mask & (1ULL << mask)) { \
        ALOGE("algo module index %d user api disabled !", mask); \
        return XCAM_RETURN_NO_ERROR; \
    }

#endif

#endif
