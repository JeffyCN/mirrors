/*
 * Copyright (C) 2022 Rockchip Electronics Co., Ltd.
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
#ifndef _im2d_log_hpp_
#define _im2d_log_hpp_

#define IM_ERR_MSG_LEN 512

typedef enum {
    IM_LOG_UNKNOWN      = 0x0,
    IM_LOG_DEFAULT      = 0x1,
    IM_LOG_DEBUG        = 0x3,
    IM_LOG_INFO         = 0x4,
    IM_LOG_WARN         = 0x5,
    IM_LOG_ERROR        = 0x6,
    IM_LOG_LEVEL_MASK   = 0xff,

    IM_LOG_FORCE        = 0x1 << 8,         /* This will force output to stdout, not to internal error messages. */
} IM_LOG_LEVEL;

#define GET_LOG_LEVEL(level) ((level) & IM_LOG_LEVEL_MASK)
#define LOG_LEVEL_CHECK(level) ((level) >= rga_log_level_get())

int rga_error_msg_set(const char* format, ...);
int rga_log_level_init(void);
void rga_log_level_update(void);
int rga_log_level_get(void);
int rga_log_enable_get(void);

size_t rga_get_current_time_ms(void);
size_t rga_get_start_time_ms(void);

#ifdef ANDROID
#define IM_LOG(level, ...) \
    do { \
        if (!((level) & IM_LOG_FORCE)) \
            rga_error_msg_set(__VA_ARGS__); \
        if ((rga_log_enable_get() > 0 && LOG_LEVEL_CHECK(level)) || \
            GET_LOG_LEVEL(level) == ANDROID_LOG_ERROR || \
            (level) & IM_LOG_FORCE) \
            ((void)__android_log_print(GET_LOG_LEVEL(level), LOG_TAG, __VA_ARGS__)); \
    } while(0)
#define IM_LOGD(_str, ...) IM_LOG(ANDROID_LOG_DEBUG, _str , ## __VA_ARGS__)
#define IM_LOGI(_str, ...) IM_LOG(ANDROID_LOG_INFO, _str , ## __VA_ARGS__)
#define IM_LOGW(_str, ...) IM_LOG(ANDROID_LOG_WARN, _str , ## __VA_ARGS__)
#define IM_LOGE(_str, ...) IM_LOG(ANDROID_LOG_ERROR, _str , ## __VA_ARGS__)
#define IM_LOGFD(_str, ...) IM_LOG(ANDROID_LOG_DEBUG | IM_LOG_FORCE, _str , ## __VA_ARGS__)
#define IM_LOGFI(_str, ...) IM_LOG(ANDROID_LOG_INFO | IM_LOG_FORCE, _str , ## __VA_ARGS__)
#define IM_LOGFW(_str, ...) IM_LOG(ANDROID_LOG_WARN | IM_LOG_FORCE, _str , ## __VA_ARGS__)
#define IM_LOGFE(_str, ...) IM_LOG(ANDROID_LOG_ERROR | IM_LOG_FORCE, _str , ## __VA_ARGS__)

#else
#define IM_LOG(level, _str, ...) \
    do { \
        if (!((level) & IM_LOG_FORCE)) \
            rga_error_msg_set(_str, ## __VA_ARGS__); \
        if ((rga_log_enable_get() > 0 && LOG_LEVEL_CHECK(level)) || \
            GET_LOG_LEVEL(level) == IM_LOG_ERROR || \
            (level) & IM_LOG_FORCE) \
            fprintf(stdout, "%ld " LOG_TAG " %s(%d): " _str "\n", \
                    (unsigned long)(rga_get_current_time_ms()-rga_get_start_time_ms()), \
                    __FUNCTION__, __LINE__, ## __VA_ARGS__); \
    } while(0)
#define IM_LOGD(_str, ...) IM_LOG(IM_LOG_DEBUG, _str , ## __VA_ARGS__)
#define IM_LOGI(_str, ...) IM_LOG(IM_LOG_INFO, _str , ## __VA_ARGS__)
#define IM_LOGW(_str, ...) IM_LOG(IM_LOG_WARN, _str , ## __VA_ARGS__)
#define IM_LOGE(_str, ...) IM_LOG(IM_LOG_ERROR, _str , ## __VA_ARGS__)
#define IM_LOGFD(_str, ...) IM_LOG(IM_LOG_DEBUG | IM_LOG_FORCE, _str , ## __VA_ARGS__)
#define IM_LOGFI(_str, ...) IM_LOG(IM_LOG_INFO | IM_LOG_FORCE, _str , ## __VA_ARGS__)
#define IM_LOGFW(_str, ...) IM_LOG(IM_LOG_WARN | IM_LOG_FORCE, _str , ## __VA_ARGS__)
#define IM_LOGFE(_str, ...) IM_LOG(IM_LOG_ERROR | IM_LOG_FORCE, _str , ## __VA_ARGS__)
#endif

#endif /* #define _im2d_log_hpp_ */
