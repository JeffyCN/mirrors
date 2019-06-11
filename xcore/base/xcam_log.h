/*
 * xcam_log.h - xcam log
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

#ifndef __XCAM_UTILS_LOG_H
#define __XCAM_UTILS_LOG_H

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>

typedef enum {
    XCORE_LOG_LEVEL_ERR,
    XCORE_LOG_LEVEL_WARNING,
    XCORE_LOG_LEVEL_INFO,
    XCORE_LOG_LEVEL_VERBOSE,
    XCORE_LOG_LEVEL_DEBUG,
    XCORE_LOG_LEVEL_LOW1,
} xcore_log_level_t;

typedef enum {
    XCORE_LOG_MODULE_NO,
    XCORE_LOG_MODULE_AEC,
    XCORE_LOG_MODULE_AWB,
    XCORE_LOG_MODULE_AF,
    XCORE_LOG_MODULE_ISP,
    XCORE_LOG_MODULE_XCORE,
    XCORE_LOG_MODULE_MAX,
} xcore_log_modules_t;

#ifdef  __cplusplus
extern "C" {
#endif
void xcam_set_log (const char* file_name);
void xcam_print_log (int module, int level, const char* format, ...);
int xcam_get_log_level();
#ifdef  __cplusplus
}
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkisp"

// generic debug
#define XCAM_LOG_ERROR(format, ...)    \
    xcam_print_log (XCORE_LOG_MODULE_XCORE, XCORE_LOG_LEVEL_ERR, "XCAM ERROR %s:%d: " format "\n", basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_LOG_WARNING(format, ...)   \
    xcam_print_log (XCORE_LOG_MODULE_XCORE, XCORE_LOG_LEVEL_WARNING, "XCAM WARNING %s:%d: " format "\n", basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_LOG_INFO(format, ...)   \
    xcam_print_log (XCORE_LOG_MODULE_XCORE, XCORE_LOG_LEVEL_INFO, "XCAM INFO (%d) %s:%d: " format "\n", getpid(), basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_LOG_VERBOSE(format, ...)   \
    xcam_print_log (XCORE_LOG_MODULE_XCORE, XCORE_LOG_LEVEL_VERBOSE, "XCAM VERBOSE (%d) %s:%d: " format "\n", getpid(), basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_LOG_DEBUG(format, ...)   \
      xcam_print_log (XCORE_LOG_MODULE_XCORE, XCORE_LOG_LEVEL_DEBUG, "XCAM DEBUG %s:%d: " format "\n", basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_LOG_LOW1(format, ...)   \
      xcam_print_log (XCORE_LOG_MODULE_XCORE, XCORE_LOG_LEVEL_LOW1, "XCAM LOW1 %s:%d: " format "\n", basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

// module debug
#define XCAM_MODULE_LOG_ERROR(module, format, ...)    \
    xcam_print_log (module, XCORE_LOG_LEVEL_ERR, "XCAM ERROR %s:%d: " format "\n", basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_MODULE_LOG_WARNING(module, format, ...)   \
    xcam_print_log (module, XCORE_LOG_LEVEL_WARNING, "XCAM WARNING %s:%d: " format "\n", basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_MODULE_LOG_INFO(module, format, ...)   \
    xcam_print_log (module, XCORE_LOG_LEVEL_INFO, "XCAM INFO (%d) %s:%d: " format "\n", getpid(), basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_MODULE_LOG_VERBOSE(module, format, ...)   \
    xcam_print_log (module, XCORE_LOG_LEVEL_VERBOSE, "XCAM VERBOSE (%d) %s:%d: " format "\n", getpid(), basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_MODULE_LOG_DEBUG(module, format, ...)   \
      xcam_print_log (module, XCORE_LOG_LEVEL_DEBUG, "XCAM DEBUG %s:%d: " format "\n", basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

#define XCAM_MODULE_LOG_LOW1(module, format, ...)   \
      xcam_print_log (module, XCORE_LOG_LEVEL_LOW1, "XCAM LOW1 %s:%d: " format "\n", basename((char*)__FILE__), __LINE__, ## __VA_ARGS__)

// define generic module logs
#define LOGD(...) XCAM_MODULE_LOG_DEBUG(XCORE_LOG_MODULE_NO, __VA_ARGS__)
#define LOGE(...) XCAM_MODULE_LOG_ERROR(XCORE_LOG_MODULE_NO, __VA_ARGS__)
#define LOGW(...) XCAM_MODULE_LOG_WARNING(XCORE_LOG_MODULE_NO, __VA_ARGS__)
#define LOGV(...) XCAM_MODULE_LOG_VERBOSE(XCORE_LOG_MODULE_NO, __VA_ARGS__)
#define LOGI(...) XCAM_MODULE_LOG_INFO(XCORE_LOG_MODULE_NO, __VA_ARGS__)
#define LOG1(...) XCAM_MODULE_LOG_LOW1(XCORE_LOG_MODULE_NO, __VA_ARGS__)

// define aec module logs
#define LOGD_AEC(...) XCAM_MODULE_LOG_DEBUG(XCORE_LOG_MODULE_AEC, __VA_ARGS__)
#define LOGE_AEC(...) XCAM_MODULE_LOG_ERROR(XCORE_LOG_MODULE_AEC, __VA_ARGS__)
#define LOGW_AEC(...) XCAM_MODULE_LOG_WARNING(XCORE_LOG_MODULE_AEC, __VA_ARGS__)
#define LOGV_AEC(...) XCAM_MODULE_LOG_VERBOSE(XCORE_LOG_MODULE_AEC, __VA_ARGS__)
#define LOGI_AEC(...) XCAM_MODULE_LOG_INFO(XCORE_LOG_MODULE_AEC, __VA_ARGS__)
#define LOG1_AEC(...) XCAM_MODULE_LOG_LOW1(XCORE_LOG_MODULE_AEC, __VA_ARGS__)

// define awb module logs
#define LOGD_AWB(...) XCAM_MODULE_LOG_DEBUG(XCORE_LOG_MODULE_AWB, __VA_ARGS__)
#define LOGE_AWB(...) XCAM_MODULE_LOG_ERROR(XCORE_LOG_MODULE_AWB, __VA_ARGS__)
#define LOGW_AWB(...) XCAM_MODULE_LOG_WARNING(XCORE_LOG_MODULE_AWB, __VA_ARGS__)
#define LOGV_AWB(...) XCAM_MODULE_LOG_VERBOSE(XCORE_LOG_MODULE_AWB, __VA_ARGS__)
#define LOGI_AWB(...) XCAM_MODULE_LOG_INFO(XCORE_LOG_MODULE_AWB, __VA_ARGS__)
#define LOG1_AWB(...) XCAM_MODULE_LOG_LOW1(XCORE_LOG_MODULE_AWB, __VA_ARGS__)

// define af module logs
#define LOGD_AF(...) XCAM_MODULE_LOG_DEBUG(XCORE_LOG_MODULE_AF, __VA_ARGS__)
#define LOGE_AF(...) XCAM_MODULE_LOG_ERROR(XCORE_LOG_MODULE_AF, __VA_ARGS__)
#define LOGW_AF(...) XCAM_MODULE_LOG_WARNING(XCORE_LOG_MODULE_AF, __VA_ARGS__)
#define LOGV_AF(...) XCAM_MODULE_LOG_VERBOSE(XCORE_LOG_MODULE_AF, __VA_ARGS__)
#define LOGI_AF(...) XCAM_MODULE_LOG_INFO(XCORE_LOG_MODULE_AF, __VA_ARGS__)
#define LOG1_AF(...) XCAM_MODULE_LOG_LOW1(XCORE_LOG_MODULE_AF, __VA_ARGS__)

// define isp module logs
#define LOGD_ISP(...) XCAM_MODULE_LOG_DEBUG(XCORE_LOG_MODULE_ISP, __VA_ARGS__)
#define LOGE_ISP(...) XCAM_MODULE_LOG_ERROR(XCORE_LOG_MODULE_ISP, __VA_ARGS__)
#define LOGW_ISP(...) XCAM_MODULE_LOG_WARNING(XCORE_LOG_MODULE_ISP, __VA_ARGS__)
#define LOGV_ISP(...) XCAM_MODULE_LOG_VERBOSE(XCORE_LOG_MODULE_ISP, __VA_ARGS__)
#define LOGI_ISP(...) XCAM_MODULE_LOG_INFO(XCORE_LOG_MODULE_ISP, __VA_ARGS__)
#define LOG1_ISP(...) XCAM_MODULE_LOG_LOW1(XCORE_LOG_MODULE_ISP, __VA_ARGS__)

// define xcore module logs
#define LOGD_XCORE(...) XCAM_MODULE_LOG_DEBUG(XCORE_LOG_MODULE_XCORE, __VA_ARGS__)
#define LOGE_XCORE(...) XCAM_MODULE_LOG_ERROR(XCORE_LOG_MODULE_XCORE, __VA_ARGS__)
#define LOGW_XCORE(...) XCAM_MODULE_LOG_WARNING(XCORE_LOG_MODULE_XCORE, __VA_ARGS__)
#define LOGV_XCORE(...) XCAM_MODULE_LOG_VERBOSE(XCORE_LOG_MODULE_XCORE, __VA_ARGS__)
#define LOGI_XCORE(...) XCAM_MODULE_LOG_INFO(XCORE_LOG_MODULE_XCORE, __VA_ARGS__)
#define LOG1_XCORE(...) XCAM_MODULE_LOG_LOW1(XCORE_LOG_MODULE_XCORE, __VA_ARGS__)

#ifndef ANDROID_OS
#define ALOGD LOGD
#define ALOGE LOGE
#define ALOGW LOGW
#define ALOGV LOGV
#define ALOGI LOGI
#define ALOG1 LOG1
#define ALOG2 LOG2
#else
#include <utils/Log.h>
#define TRACE_D ALOGIF
#endif // ANDROID_OS
#endif
