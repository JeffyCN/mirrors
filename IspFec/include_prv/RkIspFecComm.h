/*
 *  Copyright (c) 2025 Rockchip Corporation
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

#ifndef __RK_ISPFEC_COM_H__
#define __RK_ISPFEC_COM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/time.h>
#ifdef ANDROID_OS
#include <android/log.h>
#endif

#include "rkfec_config.h"

#define RKFEC_MAX(a, b)  ((a) > (b) ? (a) : (b))
#define RKFEC_MIN(a, b)  ((a) < (b) ? (a) : (b))
#define RKFEC_CLAMP(v, min, max)   \
    (((v) < (min)) ? (min) : (((v) > (max)) ? (max) : (v)))

#ifdef ANDROID_OS
#define LOG_TAG "RKFEC"
#define rkfec_err(fmt, arg...) \
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##arg)

#define rkfec_warn(fmt, arg...) \
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ##arg)

#define rkfec_info(fmt, arg...) \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##arg)

#define rkfec_dbg(fmt, arg...) \
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##arg)

#define rkfec_verb(fmt, arg...) \
    __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt, ##arg)
#else
#define rkfec_err(fmt, arg...)  fprintf(stderr, "[ERROR] " fmt "\n", ##arg)
#define rkfec_warn(fmt, arg...) printf("[WARN] " fmt "\n", ##arg)
#define rkfec_info(fmt, arg...) printf("[INFO] " fmt "\n", ##arg)

#define rkfec_print(fmt, arg...) printf("[DBG] " fmt "\n", ##arg)
#define rkfec_dbg(level, debug, fmt, arg...)           \
    do {                                               \
        if (debug >= (level)) rkfec_print(fmt, ##arg); \
    } while (0)
#endif

#ifdef RKFEC_HW_V20
#define RKFEC_DEBUG_LEVEL_PATH "/sys/module/video_rkfec/parameters/user_debug"
#else
#define RKFEC_DEBUG_LEVEL_PATH "/sys/module/video_rkispp/parameters/user_debug"
#endif

extern int rkfec_debug;

typedef struct {
    long long count;
    double sum_time;
    struct timeval start_time;
    struct timeval end_time;
} Profiler;

static inline void rkfec_profiling_start(Profiler* p) { gettimeofday(&p->start_time, NULL); }

static inline void rkfec_profiling_end(Profiler* p, const char* name, int print_interval) {
    gettimeofday(&p->end_time, NULL);
    double delta = (p->end_time.tv_sec - p->start_time.tv_sec) * 1000.0 +
                   (p->end_time.tv_usec - p->start_time.tv_usec) / 1000.0;
    p->sum_time += delta;
    ++p->count;

    if (p->count >= print_interval) {
        rkfec_info("profiling %s, fps: %.2f, avg duration: %.2f ms\n", name,
                   (p->count * 1000.0 / p->sum_time), p->sum_time / p->count);
        p->count    = 0;
        p->sum_time = 0.0;
    }
}

int rkfec_read_sysfs_int(const char* path);

#ifdef __cplusplus
}
#endif

#endif
