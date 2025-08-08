/*
 *  Copyright (c) 2019 Rockchip Corporation
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


#ifndef _RKISP_DEMO_H_
#define _RKISP_DEMO_H_

#include <linux/videodev2.h>
#include "uAPI2/rk_aiq_user_api2_imgproc.h"
#include "uAPI2/rk_aiq_user_api2_camgroup.h"
#include "common/list.h"


#define DBG(...) do { if(!silent) printf(__VA_ARGS__); } while(0)
#define ERR(...) do { printf(__VA_ARGS__); } while (0)

#define RAWBUF_MAX_FRAME 2

typedef struct fakecam_rawbuffer_s {
    void        *vaddr;
    uint32_t    index;

    struct list_head list;
} fakecam_rawbuffer;

typedef struct _demo_context {
    char                    out_file[255];
    char                    dev_name[255];
    char                    dev_name2[255];
    char                    dev_name3[255];
    char                    dev_name4[255];
    char                    dev_name5[255];
    char                    sns_name[32];
    int                     dev_using;
    int                     width;
    int                     height;
    int                     format;
    int                     fd;
    enum v4l2_buf_type      buf_type;
    struct buffer           *buffers;
    unsigned int            n_buffers;
    int                     frame_count;
    FILE                    *fp;
    rk_aiq_sys_ctx_t*       aiq_ctx;
    rk_aiq_camgroup_ctx_t*  camgroup_ctx;
    int                     vop;
    int                     rkaiq;
    int                     writeFile;
    int                     writeFileSync;
    int                     pponeframe;
    int                     hdrmode;
    int                     limit_range;
    int                     fd_pp_input;
    int                     fd_isp_mp;
    struct buffer           *buffers_mp;
    int                     outputCnt;
    int                     skipCnt;
    bool                    use_poll;

    char                    yuv_dir_path[64];
    bool                    _is_yuv_dir_exist;
    int                     capture_yuv_num;
    bool                    is_capture_yuv;
    int                     ctl_type;
    char                    iqpath[256];
    char                    orppath[256];
    int                     orpRawW;
    int                     orpRawH;
    char                    orpRawFmt[10];
    rk_aiq_rawbuf_type_t    orpRawBufType;
    bool                    isOrp;
    bool                    orpStop;
    bool                    orpStopped;
    bool                    camGroup;

    fakecam_rawbuffer       rawBufs[RAWBUF_MAX_FRAME];
    pthread_mutex_t         mutex;
    struct list_head        queue;

    bool                    aovEnterSleep;
    bool                    aovPauseAiq;
    bool                    isAovMode;
    int                     aovLoopCnt;
    int                     aovContinueCnt;
    int                     aovLoopRunCnt;
    int                     aovContinueRunCnt;
    char                    main_scene[2][32];
    char                    sub_scene[2][32];
} demo_context_t;

#ifndef CAM_STATIC_FPS_CALCULATION
#define XCAM_STATIC_FPS_CALCULATION(objname, count)                                    \
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
                DBG("%s Current fps: %.2f, Total avg fps: %.2f\n", #objname,           \
                    ((float)(count)) / current, (float)num_frame / total);             \
                last_sys_time = cur_sys_time;                                          \
            }                                                                          \
        }                                                                              \
        ++num_frame;                                                                   \
    } while (0)
#endif

#endif
