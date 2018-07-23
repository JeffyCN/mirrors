#ifndef __XCAM_UTILS_LOG_H
#define __XCAM_UTILS_LOG_H

#ifndef ANDROID_OS

#include <base/xcam_common.h>
#define ALOGD(...) XCAM_LOG_DEBUG(__VA_ARGS__)
#define ALOGE(...) XCAM_LOG_ERROR(__VA_ARGS__)
#define ALOGW(...) XCAM_LOG_WARNING(__VA_ARGS__)
#define ALOGV(...) XCAM_LOG_VERBOSE(__VA_ARGS__)
#define ALOGI(...) XCAM_LOG_INFO(__VA_ARGS__)

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGW ALOGW
#define LOGV ALOGV
#define LOGI ALOGI

#else

#include <utils/Log.h>
#include <base/xcam_common.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkisp"
#define LOGD(...) XCAM_LOG_DEBUG(__VA_ARGS__) 
#define LOGE(...) XCAM_LOG_ERROR(__VA_ARGS__) 
#define LOGW(...) XCAM_LOG_WARNING(__VA_ARGS__) 
#define LOGV(...) XCAM_LOG_VERBOSE(__VA_ARGS__) 
#define LOGI(...) XCAM_LOG_INFO(__VA_ARGS__) 

#define TRACE_D ALOGIF
#endif
#endif
