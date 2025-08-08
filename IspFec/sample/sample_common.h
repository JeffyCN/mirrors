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

#ifndef _SAMPLE_COMMON_H_
#define _SAMPLE_COMMON_H_

#include <sys/time.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define FEC_STATIC_FPS_CALCULATION(objname, count)                                     \
    do {                                                                               \
        static uint32_t num_frame = 0;                                                 \
        static struct timeval last_sys_time;                                           \
        static struct timeval first_sys_time;                                          \
        static bool b_last_sys_time_init = false;                                      \
        if (!b_last_sys_time_init) {                                                   \
            gettimeofday(&last_sys_time, NULL);                                        \
            gettimeofday(&first_sys_time, NULL);                                       \
            b_last_sys_time_init = true;                                               \
        } else {                                                                       \
            if ((num_frame % count) == 0) {                                            \
                double total, current;                                                 \
                struct timeval cur_sys_time;                                           \
                gettimeofday(&cur_sys_time, NULL);                                     \
                total = (cur_sys_time.tv_sec - first_sys_time.tv_sec) * 1.0f +         \
                        (cur_sys_time.tv_usec - first_sys_time.tv_usec) / 1000000.0f;  \
                current = (cur_sys_time.tv_sec - last_sys_time.tv_sec) * 1.0f +        \
                          (cur_sys_time.tv_usec - last_sys_time.tv_usec) / 1000000.0f; \
                info("%s Current fps: %.2f, Total avg fps: %.2f\n", #objname,          \
                     ((float)(count)) / current, (float)num_frame / total);            \
                last_sys_time = cur_sys_time;                                          \
            }                                                                          \
        }                                                                              \
        ++num_frame;                                                                   \
    } while (0)

#endif  // _SAMPLE_COMMON_H_