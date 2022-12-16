/*
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd.
 * Authors:
 *  Zhiqin Wei <wzq@rock-chips.com>
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

#ifndef _rockchip_normal_rga_context_h_
#define _rockchip_normal_rga_context_h_

#include "rga_ioctl.h"

#ifndef ANDROID
#define ALOGD(...) { printf(__VA_ARGS__); printf("\n"); }
#define ALOGE(...) { printf(__VA_ARGS__); printf("\n"); }
#endif

typedef enum {
    RGA_DRIVER_IOC_UNKONW = 0,
    RGA_DRIVER_IOC_RGA2,
    RGA_DRIVER_IOC_MULTI_RGA,

    RGA_DRIVER_IOC_DEFAULT = RGA_DRIVER_IOC_MULTI_RGA,
} RGA_DRIVER_IOC_TYPE;

struct rgaContext {
    int rgaFd;
    int mLogAlways;
    int mLogOnce;
    float mVersion;
    int Is_debug;
    struct rga_hw_versions_t mHwVersions;
    struct rga_version_t mDriverVersion;
    RGA_DRIVER_IOC_TYPE driver = RGA_DRIVER_IOC_DEFAULT;
};
#endif
