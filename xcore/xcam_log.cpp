/*
 * xcam_log.cpp - xcam log
 *
 *  Copyright (c) 2014-2015 Intel Corporation
 *  Copyright (c) 2019, Fuzhou Rockchip Electronics Co., Ltd
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

#include <base/xcam_log.h>
#include <base/xcam_defs.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef ANDROID_OS
#include <cutils/properties.h>
#ifdef ALOGV
#undef ALOGV
#define ALOGV(...) ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#endif
#endif

static char log_file_name[XCAM_MAX_STR_SIZE] = {0};
/* use a 32 bit value to represent all modules bug level, and
 * each module represents by 4 bits, and the module bit maps is
 * as follow:
 * bit:    31-28 27-24  23-20   19-16 15-12  11-8  7-4   3-0
 * module: [u]    [u]  [XCORE]  [ISP] [AF]   [AWB] [AEC] [NO]
 * [u] means unused now.
 * each module log has following ascending levels:
 *      0: error
 *      1: warning
 *      2: info
 *      3: verbose
 *      4: debug
 *      5: low1
 *      6-7: unused, now the same as debug
 *  set debug level example:
 *  eg. set module afc log level to debug, and others to error:
 *    Android:
 *      setprop persist.vendor.rkisp.log 0x4000
 *    Linux:
 *      export persist_camera_engine_log=0x4000
 */
static unsigned int g_cam_engine_log_level = 0;

typedef struct xcore_cam_log_module_info_s {
    const char* module_name;
    int log_level;
} xcore_cam_log_module_info_t;

static xcore_cam_log_module_info_t g_xcore_log_infos[XCORE_LOG_MODULE_MAX] = {
    { "NO", XCORE_LOG_LEVEL_ERR}, // XCORE_LOG_MODULE_NO
    { "AEC", XCORE_LOG_LEVEL_ERR}, // XCORE_LOG_MODULE_AEC
    { "AWB", XCORE_LOG_LEVEL_ERR}, // XCORE_LOG_MODULE_AWB
    { "AF", XCORE_LOG_LEVEL_ERR}, // XCORE_LOG_MODULE_AF
    { "ISP", XCORE_LOG_LEVEL_ERR}, // XCORE_LOG_MODULE_ISP
    { "XCORE", XCORE_LOG_LEVEL_ERR}, // XCORE_LOG_MODULE_XCORE
};

bool xcam_get_enviroment_value(const char* variable, int* value)
{
    if (!variable || !value) {
         return false;

    }

    char* valueStr = getenv(variable);
    if (valueStr) {
        *value = strtoul(valueStr, nullptr, 16);
        return true;
    }
    return false;
}

int xcam_get_log_level() {
#ifdef ANDROID_OS
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.rkisp.log", property_value, "0");
    g_cam_engine_log_level = strtoul(property_value, nullptr, 16);
#else
    xcam_get_enviroment_value("persist_camera_engine_log",
                              (int*)(&g_cam_engine_log_level));
#endif

    for (int i = 0; i < XCORE_LOG_MODULE_MAX; i++) {
        g_xcore_log_infos[i].log_level =
            (g_cam_engine_log_level >> (i*4)) & 0xf;
    }

    return g_cam_engine_log_level;
}

void xcam_print_log (int module, int level, const char* format, ...) {
    char buffer[XCAM_MAX_STR_SIZE] = {0};

    va_list va_list;
    va_start (va_list, format);
    vsnprintf (buffer, XCAM_MAX_STR_SIZE, format, va_list);
    va_end (va_list);

    if (strlen (log_file_name) > 0) {
        FILE* p_file = fopen (log_file_name, "ab+");
        if (NULL != p_file) {
            fwrite (buffer, sizeof (buffer[0]), strlen (buffer), p_file);
            fclose (p_file);
        } else {
            printf("error! can't open log file !\n");
        }
        return ;
    }
#ifdef ANDROID_OS
    if (level <= g_xcore_log_infos[module].log_level) {
        switch(level) {
        case XCORE_LOG_LEVEL_ERR:
            ALOGE("[%s]:%s", g_xcore_log_infos[module].module_name, buffer);
            break;
        case XCORE_LOG_LEVEL_WARNING:
            ALOGW("[%s]:%s", g_xcore_log_infos[module].module_name, buffer);
            break;
        case XCORE_LOG_LEVEL_INFO:
            ALOGI("[%s]:%s", g_xcore_log_infos[module].module_name, buffer);
            break;
        case XCORE_LOG_LEVEL_VERBOSE:
            ALOGV("[%s]:%s", g_xcore_log_infos[module].module_name, buffer);
            break;
        case XCORE_LOG_LEVEL_DEBUG:
        default:
            ALOGD("[%s]:%s", g_xcore_log_infos[module].module_name, buffer);
            break;
        }
    }
#else
    if (level <= g_cam_engine_log_level)
        printf ("[%s]:%s", g_xcore_log_infos[module].module_name, buffer);
#endif
}

void xcam_set_log (const char* file_name) {
    if (NULL != file_name) {
        memset (log_file_name, 0, XCAM_MAX_STR_SIZE);
        strncpy (log_file_name, file_name, XCAM_MAX_STR_SIZE);
    }
}
