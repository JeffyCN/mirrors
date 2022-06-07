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

#define IM_ERR_MSG_LEN 300

enum LOG_LEVEL {
    IM_LOG_ERROR    = 0,
    IM_LOG_WARN     = 1,
    IM_LOG_INFO     = 1,
    IM_LOG_DEBUG    = 2,
};

int imSetErrorMsg(const char* format, ...);
int rga_log_level_init(void);
void rga_log_level_update(void);
int rga_log_level_get(void);
size_t rga_get_current_time_ms(void);
size_t rga_get_start_time_ms(void);

#ifdef ANDROID
#define IM_LOG(level, ...) \
    do { \
        if(rga_log_level_get() > 0 || level == ANDROID_LOG_ERROR) \
            ((void)__android_log_print(level, LOG_TAG, __VA_ARGS__)); \
    } while(0)
#define IM_LOGD(_str, ...) IM_LOG(ANDROID_LOG_DEBUG, _str , ## __VA_ARGS__)
#define IM_LOGI(_str, ...) IM_LOG(ANDROID_LOG_INFO, _str , ## __VA_ARGS__)
#define IM_LOGW(_str, ...) IM_LOG(ANDROID_LOG_WARN, _str , ## __VA_ARGS__)
#define IM_LOGE(_str, ...) IM_LOG(ANDROID_LOG_ERROR, _str , ## __VA_ARGS__)
#else
#define IM_LOG(level, _str, ...) \
    do {        \
        if (level <= rga_log_level_get()) \
            fprintf(stdout, "%ld " LOG_TAG " %s(%d): " _str, \
                    (unsigned long)(rga_get_current_time_ms()-rga_get_start_time_ms()), \
                    __FUNCTION__, __LINE__, ## __VA_ARGS__); \
    } while(0)
#define IM_LOGD(_str, ...) IM_LOG(IM_LOG_DEBUG, _str , ## __VA_ARGS__)
#define IM_LOGI(_str, ...) IM_LOG(IM_LOG_INFO, _str , ## __VA_ARGS__)
#define IM_LOGW(_str, ...) IM_LOG(IM_LOG_WARN, _str , ## __VA_ARGS__)
#define IM_LOGE(_str, ...) IM_LOG(IM_LOG_ERROR, _str , ## __VA_ARGS__)
#endif

#endif /* #define _im2d_log_hpp_ */
