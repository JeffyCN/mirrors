/*
 * Copyright (C) 2021 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Cerf Yu <cerf.yu@rock-chips.com>
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>

#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
using namespace std;
#endif

#ifdef ANDROID
#include <android/log.h>
#include <sys/system_properties.h>
#endif

#include "im2d_log.h"

static int rga_log_property_get(void);
static int rga_log_level_property_get(void);

__thread char g_rga_err_str[IM_ERR_MSG_LEN] = "The current error message is empty!";
static atomic_int g_log_en = ATOMIC_VAR_INIT(rga_log_property_get());
static atomic_int g_log_level = ATOMIC_VAR_INIT(rga_log_level_property_get());
static size_t g_start_time = rga_get_current_time_ms();

int rga_error_msg_set(const char* format, ...) {
    int ret = 0;
    va_list ap;

    va_start(ap, format);
    ret = vsnprintf(g_rga_err_str, IM_ERR_MSG_LEN, format, ap);
    va_end(ap);

    return ret;
}

static int inline rga_log_property_get(void) {
#ifdef ANDROID
    char level[PROP_VALUE_MAX];
    __system_property_get("vendor.rga.log" ,level);
#else
    char *level = getenv("ROCKCHIP_RGA_LOG");
    if (level == nullptr)
        level = (char *)"0";
#endif

    return atoi(level);
}

static int inline rga_log_level_property_get(void) {
#ifdef ANDROID
    char level[PROP_VALUE_MAX];
    __system_property_get("vendor.rga.log_level" ,level);
#else
    char *level = getenv("ROCKCHIP_RGA_LOG_LEVEL");
    if (level == nullptr)
        level = (char *)"0";
#endif

    return atoi(level);
}

int rga_log_level_init(void) {
    return rga_log_level_get();
}

void rga_log_level_update(void) {
    g_log_level = rga_log_level_get();
}

int rga_log_level_get(void) {
    return g_log_level;
}

int rga_log_enable_get(void) {
    return g_log_en;
}

size_t rga_get_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

size_t rga_get_start_time_ms(void) {
    return g_start_time;
}
