/*
 * xcam_common.cpp - xcam common
 *
 *  Copyright (c) 2014-2015 Intel Corporation
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#include "config.h"

#include <base/xcam_common.h>
#include <base/log.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#ifdef ANDROID_OS
#include <cutils/properties.h>
#ifdef ALOGV
#undef ALOGV
#define ALOGV(...) ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#endif
#endif

static char log_file_name[XCAM_MAX_STR_SIZE] = {0};
static int g_cam_engine_log_level = 0;

uint32_t xcam_version ()
{
    return XCAM_VERSION;
}

void * xcam_malloc(size_t size)
{
    return malloc (size);
}

void * xcam_malloc0(size_t size)
{
    void * ptr = malloc (size);
    memset (ptr, 0, size);
    return ptr;
}

void xcam_free(void *ptr)
{
    if (ptr)
        free (ptr);
}

int xcam_device_ioctl (int fd, int cmd, void *arg)
{
    int ret = 0;
    int tried_time = 0;

    if (fd < 0)
        return -1;

    while (1) {
        ret = ioctl (fd, cmd, arg);
        if (ret >= 0)
            break;
        if (errno != EINTR && errno != EAGAIN)
            break;
        if (++tried_time > 5)
            break;
    }

    if (ret >= 0) {
        XCAM_LOG_DEBUG ("ioctl return ok on fd(%d), cmd:0x%.8x", fd, cmd);
    } else {
        XCAM_LOG_DEBUG ("ioctl failed on fd(%d), cmd:0x%.8x, error:%s",
                        fd, cmd, strerror(errno));
    }
    return ret;
}

const char *
xcam_fourcc_to_string (uint32_t fourcc)
{
    static char str[5];

    xcam_mem_clear (str);
    memcpy (str, &fourcc, 4);
    return str;
}

bool xcam_get_enviroment_value(const char* variable, int* value)
{
    if (!variable || !value) {
         return false;

    }

    char* valueStr = getenv(variable);
    if (valueStr) {
        *value = strtoul(valueStr, nullptr, 0);
        return true;
    }
    return false;
}

int xcam_get_log_level() {
#ifdef ANDROID_OS
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.vendor.rkisp.log", property_value, "0");
    g_cam_engine_log_level = atoi(property_value);
#else
    xcam_get_enviroment_value("persist_camera_engine_log", &g_cam_engine_log_level);
#endif

    return g_cam_engine_log_level;
}

void xcam_print_log (int level, const char* format, ...) {
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
            fprintf(stderr, "error! can't open log file !\n");
        }
        return ;
    }
#ifdef ANDROID_OS
    if (level <= g_cam_engine_log_level) {
        switch(level) {
        case ERROR_LEVEL:
            ALOGE("%s", buffer);
            break;
        case WARNING_LEVEL:
            ALOGW("%s", buffer);
            break;
        case INFO_LEVEL:
            ALOGI("%s", buffer);
            break;
        case VERBOSE_LEVEL:
            ALOGV("%s", buffer);
            break;
        case DEBUG_LEVEL:
            ALOGD("%s", buffer);
            break;
        default:
            ALOGE("debug level not support");
            break;
        }
    }
#else
    if (level <= g_cam_engine_log_level) {
        if (level <= WARNING_LEVEL)
            fprintf(stderr, "%s", buffer);
        else
            fprintf(stdout, "%s", buffer);
    }
#endif
}

void xcam_set_log (const char* file_name) {
    if (NULL != file_name) {
        memset (log_file_name, 0, XCAM_MAX_STR_SIZE);
        strncpy (log_file_name, file_name, XCAM_MAX_STR_SIZE);
    }
}

