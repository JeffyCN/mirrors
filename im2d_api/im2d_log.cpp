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

#ifdef ANDROID
#include <android/log.h>
#include <sys/system_properties.h>
#endif

#include "im2d_log.h"

__thread char g_rga_err_str[IM_ERR_MSG_LEN] = "The current error message is empty!";
int g_log_level = rga_log_level_init();
size_t g_start_time = rga_get_current_time_ms();

int imSetErrorMsg(const char* format, ...) {
    int ret = 0;
    va_list ap;

    va_start(ap, format);
    ret = vsnprintf(g_rga_err_str, IM_ERR_MSG_LEN, format, ap);
    va_end(ap);

    return ret;
}

int rga_log_level_init(void) {
#ifdef ANDROID
    char level[PROP_VALUE_MAX];
    __system_property_get("vendor.rga.log" ,level);
#else
    char *level = getenv("RK_RGA_LOG_LEVEL");
    if (level == nullptr)
        level = (char *)"0";
#endif

    return atoi(level);
}

void rga_log_level_update(void) {
#ifdef ANDROID
    char level[PROP_VALUE_MAX];
    __system_property_get("vendor.rga.log" ,level);
#else
    char *level = getenv("RK_RGA_LOG_LEVEL");
    if (level == nullptr)
        level = (char *)"0";
#endif

    g_log_level = atoi(level);
}

int rga_log_level_get(void) {
    return g_log_level;
}

size_t rga_get_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

size_t rga_get_start_time_ms(void) {
    return g_start_time;
}
