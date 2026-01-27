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

#include "Isp20_module_dbg.h"
#include "base/xcam_log.h"
#include "rk_isp20_hw.h"

#ifdef RUNTIME_MODULE_DEBUG
/* bit 36 */
#define ALL_ISP_MODULES    RK_ISP2X_MAX_ID
/* bit 41 */
#define ALL_ISPP_MODULES   RK_ISP2X_PP_MAX_ID
/* bit 42 */
#define SENSOR_EXPOSURE_ID (ALL_ISPP_MODULES + 1)
/* bit 43 */
#define ONLY_INIT_PARAMS   (SENSOR_EXPOSURE_ID + 1)

static const char* rkaiq_runtime_dbg_en = "rkaiq_runtime_dbg_en";
static const char* force_disable_modules_en = "force_disable_modules_en";
static const char* force_disable_modules_cfg_update = "force_disable_modules_cfg_update";
static const char* force_bypass_modules_params = "force_bypass_modules_params";
static const char* disable_algo_user_api_mask = "disable_algo_user_api_mask";
static unsigned long long g_disable_modules_en = 0x0;
static unsigned long long g_disable_modules_cfg_update = 0x0;
static unsigned long long g_bypass_module_params = 0;

/* disable specific isp modules ennable or cfg_update */
unsigned long long g_disable_isp_modules_en = 0x0;
unsigned long long g_disable_isp_modules_cfg_update = 0x0;
/* disable specific ispp modules ennable or cfg_update */
int g_disable_ispp_modules_en = 0x0;
int g_disable_ispp_modules_cfg_update = 0x0;
/* not apply exposure params, including the first params */
int g_bypass_exp_params = 0;
/* not apply isp params, including the first params */
int g_bypass_isp_params = 0;
/* not apply ispp params, including the first params */
int g_bypass_ispp_params = 0;
/* just apply the init params, and bypass the latter params */
int g_apply_init_params_only = 0;
/* mask bit refer to RkAiqAlgoType_t in rk_aiq_algo_des.h */
uint64_t g_disable_algo_user_api_mask = 0x0ULL;

int get_rkaiq_runtime_dbg()
{
    unsigned long long rkaiq_runtime_dbg_en_tmp = 0x0;

    xcam_get_enviroment_value(rkaiq_runtime_dbg_en, &rkaiq_runtime_dbg_en_tmp);

    return (int)rkaiq_runtime_dbg_en_tmp;
}

void get_dbg_force_disable_mods_env()
{
    unsigned long long tmp = 0;
    xcam_get_enviroment_value(disable_algo_user_api_mask, &tmp);
    g_disable_algo_user_api_mask = (int)tmp;

    xcam_get_enviroment_value(force_bypass_modules_params, &g_bypass_module_params);

    if (g_bypass_module_params & (1ULL << SENSOR_EXPOSURE_ID))
        g_bypass_exp_params = 1;
    else
        g_bypass_exp_params = 0;

    if (g_bypass_module_params & (1ULL << ALL_ISP_MODULES))
        g_bypass_isp_params = 1;
    else
        g_bypass_isp_params = 0;

    if (g_bypass_module_params & (1ULL << ALL_ISPP_MODULES))
        g_bypass_ispp_params = 1;
    else
        g_bypass_ispp_params = 0;
    LOGI("ALL_ISP_MODULES %d, ALL_ISPP_MODULES %d,ONLY_INIT_PARAMS %d",
         ALL_ISP_MODULES, ALL_ISPP_MODULES, ONLY_INIT_PARAMS);
    LOGI("g_bypass_module_params 0x%llx", g_bypass_module_params);
    if (g_bypass_module_params & (1ULL << ONLY_INIT_PARAMS))
        g_apply_init_params_only = 1;
    else
        g_apply_init_params_only = 0;

    xcam_get_enviroment_value(force_disable_modules_en, &g_disable_modules_en);

    if (g_disable_modules_en & (1ULL << ALL_ISP_MODULES)) {
        for (int i = 0; i < ALL_ISP_MODULES; i++)
            g_disable_isp_modules_en |= 1ULL << i;
    } else {
        for (int i = 0; i < ALL_ISP_MODULES; i++) {
            if (g_disable_modules_en & (1ULL << i))
                g_disable_isp_modules_en |= 1ULL << i;
            else
                g_disable_isp_modules_en &= ~(1ULL << i);
        }
    }

    if (g_disable_modules_en & (1ULL << ALL_ISPP_MODULES)) {
        for (int i = 0; i < ALL_ISPP_MODULES - ALL_ISP_MODULES; i++)
            g_disable_ispp_modules_en |= 1ULL << i;

    } else {
        for (int i = 0; i < ALL_ISPP_MODULES - ALL_ISP_MODULES; i++) {
            if (g_disable_modules_en & (1ULL << (ALL_ISP_MODULES + 1 + i)))
                g_disable_ispp_modules_en |= 1ULL << i;
            else
                g_disable_ispp_modules_en &= ~(1ULL << i);
        }
    }

    xcam_get_enviroment_value(force_disable_modules_cfg_update, &g_disable_modules_cfg_update);

    if (g_disable_modules_cfg_update & (1ULL << ALL_ISP_MODULES)) {
        for (int i = 0; i < ALL_ISP_MODULES; i++)
            g_disable_isp_modules_cfg_update |= 1ULL << i;
    } else {
        for (int i = 0; i < ALL_ISP_MODULES; i++) {
            if (g_disable_modules_cfg_update & (1ULL << i))
                g_disable_isp_modules_cfg_update |= 1ULL << i;
            else
                g_disable_isp_modules_cfg_update &= ~(1ULL << i);
        }
    }

    if (g_disable_modules_cfg_update & (1ULL << ALL_ISPP_MODULES)) {
        for (int i = 0; i < ALL_ISPP_MODULES - ALL_ISP_MODULES; i++)
            g_disable_ispp_modules_cfg_update |= 1 << i;

    } else {
        for (int i = 0; i < ALL_ISPP_MODULES - ALL_ISP_MODULES; i++) {
            if (g_disable_modules_cfg_update & (1ULL << (ALL_ISP_MODULES + 1 + i)))
                g_disable_ispp_modules_cfg_update |= 1 << i;
            else
                g_disable_ispp_modules_cfg_update &= ~(1 << i);
        }
    }

    LOGI("isp(en:0x%llx, cfg_up:0x%llx, bypass:%d),\n"
         "ispp(en:0x%x, cfg_up:0x%x, bypass:%d),\n"
         "exp_byapss:%d, init_params_only:%d",
         g_disable_isp_modules_en, g_disable_isp_modules_cfg_update, g_bypass_isp_params,
         g_disable_ispp_modules_en, g_disable_ispp_modules_cfg_update, g_bypass_ispp_params,
         g_bypass_exp_params, g_apply_init_params_only);
}

#endif
