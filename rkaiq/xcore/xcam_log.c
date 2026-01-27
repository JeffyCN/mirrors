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

#include "base/xcam_log.h"
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

#if RKAIQ_HAVE_DUMPSYS
#include "dumpcam_server/third-party/argparse/argparse.h"
#include "rk_info_utils.h"
#include "st_string.h"
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
 * bit:                                                                     49        48
 * meaning:                                                                [ARGBIR]   [AFD]
 *
 * bit:     [63-51]
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
    {"ALDC", XCORE_LOG_LEVEL_ERR, 0xff},       // XCORE_LOG_MODULE_ALDC
    {"AHISTEQ", XCORE_LOG_LEVEL_ERR, 0xff},    // XCORE_LOG_MODULE_HISTEQ
    {"AHSV", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_AHSV
    {"AIBNR", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_AIBNR
    {"AMTD", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_AMTD
    {"AIRMS", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_AIRMS
    {"AIYNR", XCORE_LOG_LEVEL_ERR, 0xff},     // XCORE_LOG_MODULE_AIYNR
};

typedef struct xcamLogCb_s {
    rkaiq_log_callback_t logcb;
    void *user_data;
} xcamLogCb_t;

static xcamLogCb_t g_logcb = {
    .logcb = NULL,
    .user_data = NULL,
};


bool xcam_get_enviroment_value(const char* variable, unsigned long long* value)
{
    if (!variable || !value) {
        return false;

    }

    char* valueStr = getenv(variable);
    if (valueStr) {
        *value = strtoull(valueStr, NULL, 16);

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
            g_cam_engine_log_level = strtoull(level, NULL, 16);
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
    g_cam_engine_log_level = strtoull(property_value, NULL, 16);
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

#if RKAIQ_HAVE_DUMPSYS
static void xcam_set_log_level(uint64_t log_level) {
    g_cam_engine_log_level = log_level;

    uint64_t module_mask = g_cam_engine_log_level >> 12;
    for (int32_t i = 0; i < XCORE_LOG_MODULE_MAX; i++) {
        if (module_mask & (1ULL << i)) {
            g_xcore_log_infos[i].log_level   = g_cam_engine_log_level & 0xf;
            g_xcore_log_infos[i].sub_modules = (g_cam_engine_log_level >> 4) & 0xff;
        } else {
            g_xcore_log_infos[i].log_level   = 0;
            g_xcore_log_infos[i].sub_modules = 0;
        }
    }
}
#endif

char* timeString() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm * timeinfo = localtime(&tv.tv_sec);
    static char timeStr[64];
    sprintf(timeStr, "%.2d:%.2d:%.2d."
#ifdef __UCLIBC_USE_TIME64__
            "%.6lld",
#else
            "%.6ld",
#endif
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tv.tv_usec);
    return timeStr;
}

static const char g_level_strs[] = {
    'K', 'E', 'W', 'I', 'D', 'V', 'L'
};

void xcam_print_log (int module, int level, const char* format, ...) {
    if ((g_cam_engine_log_level & 0xf) == 0) return;
    char buffer[XCAM_MAX_STR_SIZE] = {g_level_strs[level], ':'};
    va_list va_list;
    va_start (va_list, format);
    vsnprintf (buffer + 2, XCAM_MAX_STR_SIZE - 2, format, va_list);
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
    if (g_logcb.logcb) {
        g_logcb.logcb((xcore_log_level_t)level, NULL, g_xcore_log_infos[module].module_name,
                buffer + 2, strlen(buffer) - 2, g_logcb.user_data);
    } else {
        printf ("%s:%s\n", g_xcore_log_infos[module].module_name, buffer);
    }
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

#if RKAIQ_HAVE_DUMPSYS

#ifndef BIT_ULL
#define BIT_ULL(nr) (1ULL << (nr))
#endif

static const char* const usages[] = {
    "dumpcam log [options] [[--] args]",
    "dumpcam log [options]",
    NULL,
};

static int _gHelp = 0;
static int dbg_help_cb(struct argparse* self, const struct argparse_option* option) {
    _gHelp = 1;

    return 0;
}

int xcam_dump_log(st_string* result, int argc, void* argv[]) {
    char buffer[MAX_LINE_LENGTH * 4] = {0};

#ifdef NDEBUG
    sprintf(buffer, "\nNot supported, AIQ logs has been removed at compile time\n");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    return true;
#endif

    if (!argc) {
        sprintf(buffer, "\nCurrent AIQ log level: 0x%llx\n", g_cam_engine_log_level);
        aiq_string_printf(result, buffer);
        aiq_string_printf(result, "\n");
        return true;
    }

    char argvArray[256][256];
    char* extended_argv[256];
    int extended_argc = 0;

    snprintf(argvArray[0], sizeof(argvArray[extended_argc]), "%s", "log");
    extended_argv[0] = argvArray[0];
    extended_argc++;
    for (int i = 0; i < argc; i++) {
        LOG1("argv[%d]: %s", i, *((const char**)argv + i));
        snprintf(argvArray[extended_argc], sizeof(argvArray[extended_argc]), "%s",
                 *((const char**)argv + i));
        extended_argv[extended_argc] = argvArray[extended_argc];
        extended_argc++;
    }

    uint64_t log_cfg = 0;
    uint64_t log_mod = 0;
    char* endptr;
    const char *log_sub_mod_str = NULL, *log_level_str = NULL;
    uint32_t log_sub_mod = 0xff, log_level = 4;
    struct argparse_option options[] = {
        OPT_GROUP("Log level:"),
        OPT_STRING('l', "level", &log_level_str, "log level", NULL, 0, 0),
        OPT_GROUP("Log sub modules:"),
        OPT_STRING('s', "sub", &log_sub_mod_str, "the sub modules of each module", NULL, 0, 0),
        OPT_GROUP("Log modules:"),
        OPT_BIT('A', "all", &log_mod, "all", NULL, BIT_ULL(XCORE_LOG_MODULE_ALL), OPT_NONEG),
        OPT_BIT('b', "ae", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AEC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AEC), OPT_NONEG),
        OPT_BIT('c', "awb", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AWB].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AWB), 0),
        OPT_BIT('d', "af", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AF].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AF), 0),
        OPT_BIT('c', "blc", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ABLC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ABLC), 0),
        OPT_BIT('d', "dpc", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ADPCC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ADPCC), 0),
        OPT_BIT('e', "tmo", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ATMO].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ATMO), 0),
        OPT_BIT('f', "anr", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ANR].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ANR), 0),
        OPT_BIT('g', "lsc", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ALSC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ALSC), 0),
        OPT_BIT('i', "gic", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AGIC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AGIC), 0),
        OPT_BIT('j', "dm", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ADEBAYER].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ADEBAYER), 0),
        OPT_BIT('m', "ccm", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ACCM].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ACCM), 0),
        OPT_BIT('n', "goc", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AGAMMA].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AGAMMA), 0),
        OPT_BIT('k', "wdr", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AWDR].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AWDR), 0),
        OPT_BIT('o', "dhz", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ADEHAZE].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ADEHAZE), 0),
        OPT_BIT('p', "lut3d", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_A3DLUT].module_name,
                NULL, BIT_ULL(XCORE_LOG_MODULE_A3DLUT), 0),
        OPT_BIT('r', "ldch", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ALDCH].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ALDCH), 0),
        OPT_BIT('t', "cp", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ACP].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ACP), 0),
        OPT_BIT('u', "ie", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AIE].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AIE), 0),
        OPT_BIT('v', "sharp", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ASHARP].module_name,
                NULL, BIT_ULL(XCORE_LOG_MODULE_ASHARP), 0),
        OPT_BIT('w', "cgc", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ACGC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ACGC), 0),
        OPT_BIT('x', "asd", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ASD].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ASD), 0),
        OPT_BIT('y', "xcore", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_XCORE].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_XCORE), 0),
        OPT_BIT('z', "analyzer", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ANALYZER].module_name,
                NULL, BIT_ULL(XCORE_LOG_MODULE_ANALYZER), 0),
        OPT_BIT('0', "hw", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_CAMHW].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_CAMHW), 0),
        OPT_BIT('1', "dgm", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ADEGAMMA].module_name,
                NULL, BIT_ULL(XCORE_LOG_MODULE_ADEGAMMA), 0),
        OPT_BIT('2', "mge", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AMERGE].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AMERGE), 0),
        OPT_BIT('3', "md", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AMD].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AMD), 0),
        OPT_BIT('4', "cac", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ACAC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ACAC), 0),
        OPT_BIT('5', "camgrp", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_CAMGROUP].module_name,
                NULL, BIT_ULL(XCORE_LOG_MODULE_CAMGROUP), 0),
        OPT_BIT('6', "grpwb", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AWBGROUP].module_name,
                NULL, BIT_ULL(XCORE_LOG_MODULE_AWBGROUP), 0),
        OPT_BIT('7', "grpae", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_GROUPAEC].module_name,
                NULL, BIT_ULL(XCORE_LOG_MODULE_GROUPAEC), 0),
        OPT_BIT('8', "rawstrm", &log_mod,
                g_xcore_log_infos[XCORE_LOG_MODULE_RKRAWSTREAM].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_RKRAWSTREAM), 0),
        OPT_BIT('9', "ipc", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_IPC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_IPC), 0),
        OPT_BIT('q', "afd", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AFD].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AFD), 0),
        OPT_BIT('B', "rgbir", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ARGBIR].module_name,
                NULL, BIT_ULL(XCORE_LOG_MODULE_ARGBIR), 0),
        OPT_BIT('C', "ldc", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ALDC].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ALDC), 0),
        OPT_BIT('D', "hist", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AHIST].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AHIST), 0),
        OPT_BIT('E', "hsv", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AHSV].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AHSV), 0),
        OPT_BIT('F', "csm", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_ACSM].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_ACSM), 0),
        OPT_BIT('G', "aibnr", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AIBNR].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AIBNR), 0),
        OPT_BIT('H', "airms", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AIRMS].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AIRMS), 0),
        OPT_BIT('i', "aiynr", &log_mod, g_xcore_log_infos[XCORE_LOG_MODULE_AIYNR].module_name, NULL,
                BIT_ULL(XCORE_LOG_MODULE_AIYNR), 0),
        OPT_BOOLEAN('\0', "help", NULL, "show this help message and exit", dbg_help_cb, 0,
                    OPT_NONEG),
        OPT_END(),
    };

    char additional_desc[1024] = {0};
    snprintf(additional_desc, sizeof(additional_desc),
             "\n\nAdditional description of dumpcam log.\n\n"
             "[Level] : [-l] [--level] use 4 bits to set log levels.\n"
             "  each module log has following ascending levels:\n"
             "       0: none\n"
             "       1: error\n"
             "       2: warning\n"
             "       3: info\n"
             "       4: debug\n"
             "       5: verbose\n"
             "       6: low1\n"
             "       7: unused, now the same as debug\n"
             "[Sub modules] : [-s] [--sub] use bits 4-11 to switch the sub module's log of each "
             "module.\n"
             "[Modules] : [-bc ...] [--ae --awb] switch the module log. eg., ae, awb, af, ...\n"
             "\n\n"
             "eg., dumpcam log\n"
             "  Query the current log level\n\n"
             "eg., 'dumpcam log -b' or 'dumpcam log --ae'\n"
             "  [Modules]: ae, [Sub modules]: default, [Level]: default\n\n"
             "eg., dumpcam log -bcd\n"
             "  [Modules]: ae && awb && af, [Sub modules]: default, [Level]: default\n\n"
             "eg., dumpcam log -b -l 0x4\n"
             "  [Modules]: ae, [Sub modules]: default, [Level]: debug\n\n"
             "eg., dumpcam log -b -l 0x4 -s 0xff\n"
             "  [Modules]: ae, [Sub modules]: 0xff, [Level]: debug\n");

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nA brief description of how to set aiq log level.",
                      additional_desc);

    extended_argc = argparse_parse(&argparse, extended_argc, (const char**)extended_argv);
    if (_gHelp || extended_argc < 0) {
        _gHelp = 0;
        goto __FAILED;
    }

    if (log_sub_mod_str) {
        log_sub_mod = strtoul(log_sub_mod_str, &endptr, 0);
        if (*endptr) {
            goto __FAILED;
        }
    } else {
        log_sub_mod = (g_cam_engine_log_level >> 4) & 0xff;
    }

    if (log_level_str) {
        log_level = strtoul(log_level_str, &endptr, 0);
        if (*endptr) {
            goto __FAILED;
        }
    } else {
        log_level = g_cam_engine_log_level & 0xf;
    }

    if (!log_mod)
        log_mod = g_cam_engine_log_level >> 12;
    else if (log_mod & BIT_ULL(XCORE_LOG_MODULE_ALL))
        log_mod = 0xFFFFFFFFFFF;

    log_cfg = log_mod << 12 | log_sub_mod << 4 | log_level;

    xcam_set_log_level(log_cfg);

    sprintf(buffer, "\nSet AIQ log level: 0x%llx\n", g_cam_engine_log_level);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    return true;

__FAILED:
    argparse_usage_string(&argparse, buffer);
    aiq_string_printf(result, buffer);

    return true;
}
#endif

void rkaiq_set_log_callback(rkaiq_log_callback_t cb, void *user_data)
{
    g_logcb.logcb = cb;
    g_logcb.user_data = user_data;
}
