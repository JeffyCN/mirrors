/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _SMART_IR_DEMO_H_
#define _SMART_IR_DEMO_H_

#include <linux/videodev2.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdatomic.h>

#include <rk_aiq_user_api2_camgroup.h>
#include <rk_aiq_user_api2_imgproc.h>
#include <rk_aiq_user_api2_sysctl.h>

#include "rk_smart_ir_api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct buffer {
    void* start;
    size_t length;
    int export_fd;
    int sequence;
};

typedef struct _smartIr_demo_context {
    char dev_name[256];
    char sns_name[32];
    char iqpath[256];
    int width;
    int height;
    int format;
    int fd;
    enum v4l2_buf_type buf_type;
    struct buffer* buffers;
    unsigned int n_buffers;

    rk_aiq_sys_ctx_t* aiq_ctx;
    rk_smart_ir_ctx_t* smartIr_ctx;

    int hdr_mode;
    int limit_range;
    int run_time;
    bool camGroup;
    bool en_calib;
} smartIr_demo_context_t;

#ifdef __cplusplus
}
#endif

#endif
