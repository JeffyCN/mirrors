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
#include <time.h>
#include <sys/time.h>
#ifdef ANDROID_OS
#include <cutils/properties.h>
#ifdef ALOGV
#undef ALOGV
#define ALOGV(...) ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#endif
#endif

#define LOG_FILE_NAME_MAX_LENGTH 256
static char log_file_name[LOG_FILE_NAME_MAX_LENGTH] = {0};
/* use a 64 bits value to represent all modules bug level, and the
 * module bit maps is as follow:
 *
 * bit:      7-4                                       3-0
 * meaning:  [sub modules]                             [level]
 *
 * bit:      15          14       13          12       11-8
 * meaning:  [ABLC]      [AF]     [AWB]       [AEC]    [sub modules]
 *
 * bit:      23          22       21          20       19       18        17          16
 * meaning:  [AGAMMA]    [ACCM]   [ADEBAYER]  [AGIC]   [ALSC]   [ANR]     [ATMO]      [ADPCC]
 *
 * bit:      31          30       29          28       27       26        25          24
 * meaning:  [ASHARP]    [AIE]    [ACP]       [ACSM]   [ALDCH]  [A3DLUT]  [ADEHAZE]   [AWDR]
 *
 * bit:      39          38       37          36       35       34        33          32
 * meaning:  [ADEGAMMA]  [CAMHW]  [ANALYZER]  [XCORE]  [ASD]    [AFEC]    [ACGC]      [AORB]
 *
 * bit:      48          46       45          44       43       42        41          40
 * meaning:  [IPC]   [RKSTREAM] [GROUPAEC] [AWBGROUP]  [CAMGROUP]   [ACAC]    [AMD]   [AMERGE]
 *
 * bit:                                                                       49        48
 * meaning:                                                                 [ARGBIR]   [AFD]
 *
 * bit:     [63-50]
 * meaning:  [U]
 *
 * [U] means unused now.
 * [level]: use 4 bits to define log levels.
 *     each module log has following ascending levels:
 *          0: error
 *          1: warning
 *          2: info
 *          3: debug
 *          4: verbose
 *          5: low1
 *          6-7: unused, now the same as debug
 * [sub modules]: use bits 4-11 to define the sub modules of each module, the
 *     specific meaning of each bit is decided by the module itself. These bits
 *     is designed to implement the sub module's log switch.
 * [modules]: AEC, AWB, AF ...
 *
 * set debug level example:
 * eg. set module af log level to debug, and enable all sub modules of af:
 *    Android:
 *      setprop persist.vendor.rkisp.log 0x4ff4
 *    Linux:
 *      export persist_camera_engine_log=0x4ff4
 *    And if only want enable the sub module 1 log of af:
 *    Android:
 *      setprop persist.vendor.rkisp.log 0x4014
 *    Linux:
 *      export persist_camera_engine_log=0x4014
 */
static unsigned long long g_cam_engine_log_level = 0xff1;

#if 0
typedef struct xcore_cam_log_module_info_s {
    const char* module_name;
    int log_level;
    int sub_modules;
} xcore_cam_log_module_info_t;
#endif

xcore_cam_log_module_info_t g_xcore_log_infos[XCORE_LOG_MODULE_MAX] = {
    {"AEC", XCORE_LOG_LEVEL_ERR, 0xff},        // XCORE_LOG_MODULE_AEC
    {"AWB", XCORE_LOG_LEVEL_ERR, 0xff},        // XCORE_LOG_MODULE_AWB
    {"AF", XCORE_LOG_LEVEL_ERR, 0xff},         // XCORE_LOG_MODULE_AF
    {"ABLC", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_ABLC
    {"ADPCC", XCORE_LOG_LEVEL_ERR, 0xff},      // XCORE_LOG_MODULE_ADPCC
    {"ATMO", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_ATMO
    {"ANR", XCORE_LOG_LEVEL_ERR, 0xff},        // XCORE_LOG_MODULE_ANR
    {"ALSC", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_ALSC
    {"AGIC", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_AGIC
    {"ADEBAYER", XCORE_LOG_LEVEL_ERR, 0xff},   // XCORE_LOG_MODULE_ADEBAYER
    {"ACCM", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_ACCM
    {"AGAMMA", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_AGAMMA
    {"AWDR", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_AWDR
    {"ADEHAZE", XCORE_LOG_LEVEL_ERR, 0xff},    // XCORE_LOG_MODULE_ADEHAZE
    {"A3DLUT", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_A3DLUT
    {"ALDCH", XCORE_LOG_LEVEL_ERR, 0xff},      // XCORE_LOG_MODULE_ALDCH
    {"ACSM", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_ACSM
    {"ACP", XCORE_LOG_LEVEL_ERR, 0xff},        // XCORE_LOG_MODULE_ACP
    {"AIE", XCORE_LOG_LEVEL_ERR, 0xff},        // XCORE_LOG_MODULE_AIE
    {"ASHARP", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_ASHARP
    {"AORB", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_AORB
    {"AFEC", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_AFEC
    {"ACGC", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_ACGC
    {"ASD", XCORE_LOG_LEVEL_ERR, 0xff},        // XCORE_LOG_MODULE_ASD
    {"XCORE", XCORE_LOG_LEVEL_ERR, 0xff},      // XCORE_LOG_MODULE_XCORE
    {"ANALYZER", XCORE_LOG_LEVEL_ERR, 0xff},   // XCORE_LOG_MODULE_ANALYZER
    {"CAMHW", XCORE_LOG_LEVEL_ERR, 0xff},      // XCORE_LOG_MODULE_CAMHW
    {"ADEGAMMA", XCORE_LOG_LEVEL_ERR, 0xff},   // XCORE_LOG_MODULE_ADEGAMMA
    {"AMERGE", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_AMERGE
    {"AMD", XCORE_LOG_LEVEL_ERR, 0xff},        // XCORE_LOG_MODULE_AMMD
    {"ACAC", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_AMMD
    {"CAMGROUP", XCORE_LOG_LEVEL_ERR, 0xff},   // XCORE_LOG_MODULE_CAMGROUP
    {"AWBGROUP", XCORE_LOG_LEVEL_ERR, 0xff},   // XCORE_LOG_MODULE_CAMGROUP
    {"GROUPAEC", XCORE_LOG_LEVEL_ERR, 0xff},   // XCORE_LOG_MODULE_GROUPAEC
    {"RKSTREAM", XCORE_LOG_LEVEL_ERR, 0xff},   // XCORE_LOG_MODULE_RKSTREAM
    {"IPCSERVER", XCORE_LOG_LEVEL_ERR, 0xff},  // XCORE_LOG_MODULE_IPC
    {"AFD", XCORE_LOG_LEVEL_ERR, 0xff},        // XCORE_LOG_MODULE_AFD
    {"ARGBIR", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_RGBIR
};

bool xcam_get_enviroment_value(const char* variable, unsigned long long* value)
{
    if (!variable || !value) {
        return false;

    }

    char* valueStr = getenv(variable);
    if (valueStr) {
        *value = strtoull(valueStr, nullptr, 16);

        return true;
    }
    return false;
}

void xcam_get_runtime_log_level() {
#ifdef ANDROID_OS
    const char* file_name = "/data/.rkaiq_log";
#else
    const char* file_name = "/tmp/.rkaiq_log";
#endif
    if (!access(file_name, F_OK)) {
        FILE *fp = fopen(file_name, "r");
        char level[64] = {'\0'};

        if (!fp)
            return;

        fseek(fp, 0, SEEK_SET);
        if (fp && fread(level, 1, sizeof (level), fp) > 0) {
            for (int i = 0; i < XCORE_LOG_MODULE_MAX; i++) {
                g_xcore_log_infos[i].log_level = 0;
                g_xcore_log_infos[i].sub_modules = 0;
            }
            g_cam_engine_log_level = strtoull(level, nullptr, 16);
            unsigned long long module_mask = g_cam_engine_log_level >> 12;
            for (int i = 0; i < XCORE_LOG_MODULE_MAX; i++) {
                if (module_mask & (1ULL << i)) {
                    g_xcore_log_infos[i].log_level = g_cam_engine_log_level & 0xf;
                    g_xcore_log_infos[i].sub_modules = (g_cam_engine_log_level >> 4) & 0xff;
                }
            }
        }

        fclose (fp);
    }
}

int xcam_get_log_level() {
#ifdef ANDROID_OS
    char property_value[PROPERTY_VALUE_MAX] = {0};
    char property_value_default[PROPERTY_VALUE_MAX] = {0};
    sprintf(property_value_default, "%llx", g_cam_engine_log_level);
    property_get("persist.vendor.rkisp.log", property_value, property_value_default);
    g_cam_engine_log_level = strtoull(property_value, nullptr, 16);
    ALOGI("rkaiq log level %llx\n", g_cam_engine_log_level);
#else
    xcam_get_enviroment_value("persist_camera_engine_log",
                              &g_cam_engine_log_level);
#endif
    printf("rkaiq log level %llx\n", g_cam_engine_log_level);
    unsigned long long module_mask = g_cam_engine_log_level >> 12;

    for (int i = 0; i < XCORE_LOG_MODULE_MAX; i++) {
        if (module_mask & (1ULL << i)) {
            g_xcore_log_infos[i].log_level = g_cam_engine_log_level & 0xf;
            g_xcore_log_infos[i].sub_modules = (g_cam_engine_log_level >> 4) & 0xff;
        } else if ( g_cam_engine_log_level == 0) {
            g_xcore_log_infos[i].log_level = 0;
        }
    }

    return 0;
}

char* timeString() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm * timeinfo = localtime(&tv.tv_sec);
    static char timeStr[64];
    sprintf(timeStr, "%.2d:%.2d:%.2d.%.6ld", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tv.tv_usec);
    return timeStr;
}

void xcam_print_log (int module, int sub_modules, int level, const char* format, ...) {
    if ((g_cam_engine_log_level & 0xf) == 0) return;
    char buffer[XCAM_MAX_STR_SIZE] = {0};
    va_list va_list;
    va_start (va_list, format);
    vsnprintf (buffer, XCAM_MAX_STR_SIZE, format, va_list);
    va_end (va_list);

#ifdef NDEBUG
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
#endif
#ifdef ANDROID_OS
    switch(level) {
    case XCORE_LOG_LEVEL_NONE:
        ALOGI("[%s]:%s", g_xcore_log_infos[module].module_name, buffer);
        break;
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
#else
    printf ("%s:%s", g_xcore_log_infos[module].module_name, buffer);
#endif
}

void xcam_set_log (const char* file_name) {
    if (NULL != file_name) {
        memset (log_file_name, 0, LOG_FILE_NAME_MAX_LENGTH);
        strncpy (log_file_name, file_name, LOG_FILE_NAME_MAX_LENGTH - 1);
        log_file_name[LOG_FILE_NAME_MAX_LENGTH - 1] = '\0';
    }
}
void xcam_get_awb_log_level(unsigned char *log_level, unsigned char *sub_modules)
{
    //xcam_get_log_level();
    *log_level = g_xcore_log_infos[XCORE_LOG_MODULE_AWB].log_level;
    *sub_modules = g_xcore_log_infos[XCORE_LOG_MODULE_AWB].sub_modules;
}
