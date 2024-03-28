// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_LOGGER_H_
#define _RK_LOGGER_H_

#include <errno.h>

#include <chrono>

#ifdef __ANDROID__
    #include <log/log.h>
#endif

extern int log_level;

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

#ifndef LOG_TAG
    #define LOG_TAG "aiqtool"
#endif // LOG_TAG

#define __BI_FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef __ANDROID__
    #define LOG_INFO(format, ...)                                                                                                                                                                                                                                                                          \
        do                                                                                                                                                                                                                                                                                                 \
        {                                                                                                                                                                                                                                                                                                  \
            if (log_level < LOG_LEVEL_INFO)                                                                                                                                                                                                                                                                \
                break;                                                                                                                                                                                                                                                                                     \
            ALOGI("%s:%d - " format "", __BI_FILENAME__, __LINE__, ##__VA_ARGS__);                                                                                                                                                                                                                         \
        } while (0)

    #define LOG_WARN(format, ...)                                                                                                                                                                                                                                                                          \
        do                                                                                                                                                                                                                                                                                                 \
        {                                                                                                                                                                                                                                                                                                  \
            if (log_level < LOG_LEVEL_WARN)                                                                                                                                                                                                                                                                \
                break;                                                                                                                                                                                                                                                                                     \
            ALOGW("%s:%d - " format "", __BI_FILENAME__, __LINE__, ##__VA_ARGS__);                                                                                                                                                                                                                         \
        } while (0)

    #define LOG_ERROR(format, ...)                                                                                                                                                                                                                                                                         \
        do                                                                                                                                                                                                                                                                                                 \
        {                                                                                                                                                                                                                                                                                                  \
            if (log_level < LOG_LEVEL_ERROR)                                                                                                                                                                                                                                                               \
                break;                                                                                                                                                                                                                                                                                     \
            ALOGE("%s:%d - " format "", __BI_FILENAME__, __LINE__, ##__VA_ARGS__);                                                                                                                                                                                                                         \
        } while (0)

    #define LOG_DEBUG(format, ...)                                                                                                                                                                                                                                                                         \
        do                                                                                                                                                                                                                                                                                                 \
        {                                                                                                                                                                                                                                                                                                  \
            if (log_level < LOG_LEVEL_DEBUG)                                                                                                                                                                                                                                                               \
                break;                                                                                                                                                                                                                                                                                     \
            ALOGD("%s:%d - " format "", __BI_FILENAME__, __LINE__, ##__VA_ARGS__);                                                                                                                                                                                                                         \
        } while (0)
#else

    #define LOG_INFO(format, ...)                                                                                                                                                                                                                                                                          \
        do                                                                                                                                                                                                                                                                                                 \
        {                                                                                                                                                                                                                                                                                                  \
            if (log_level < LOG_LEVEL_INFO)                                                                                                                                                                                                                                                                \
                break;                                                                                                                                                                                                                                                                                     \
            fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);                                                                                                                                                                                                                     \
        } while (0)

    #define LOG_WARN(format, ...)                                                                                                                                                                                                                                                                          \
        do                                                                                                                                                                                                                                                                                                 \
        {                                                                                                                                                                                                                                                                                                  \
            if (log_level < LOG_LEVEL_WARN)                                                                                                                                                                                                                                                                \
                break;                                                                                                                                                                                                                                                                                     \
            fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);                                                                                                                                                                                                                     \
        } while (0)

    #define LOG_ERROR(format, ...)                                                                                                                                                                                                                                                                         \
        do                                                                                                                                                                                                                                                                                                 \
        {                                                                                                                                                                                                                                                                                                  \
            if (log_level < LOG_LEVEL_ERROR)                                                                                                                                                                                                                                                               \
                break;                                                                                                                                                                                                                                                                                     \
            fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);                                                                                                                                                                                                                     \
        } while (0)

    #define LOG_DEBUG(format, ...)                                                                                                                                                                                                                                                                         \
        do                                                                                                                                                                                                                                                                                                 \
        {                                                                                                                                                                                                                                                                                                  \
            if (log_level < LOG_LEVEL_DEBUG)                                                                                                                                                                                                                                                               \
                break;                                                                                                                                                                                                                                                                                     \
            fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);                                                                                                                                                                                                                     \
        } while (0)

#endif

#define errno_debug(fmt) LOG_ERROR("%s error %d, %s\n", (fmt), errno, strerror(errno))

inline int64_t gettimeofday()
{
    std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
    return us.count();
}

class AutoDuration
{
  public:
    AutoDuration()
    {
        Reset();
    }
    int64_t Get()
    {
        return gettimeofday() - start;
    }
    void Reset()
    {
        start = gettimeofday();
    }
    int64_t GetAndReset()
    {
        int64_t now = gettimeofday();
        int64_t pretime = start;
        start = now;
        return now - pretime;
    }

  private:
    int64_t start;
};

#endif
