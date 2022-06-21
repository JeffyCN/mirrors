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
#ifndef _im2d_task_hpp_
#define _im2d_task_hpp_

#include "im2d_type.h"

#ifdef __cplusplus

/* Start task api */
IM_API im_job_id_t im_BeginJob(uint64_t flags);

IM_API IM_STATUS imCancelJob(im_job_id_t id);

IM_API IM_STATUS imEndJob(im_job_id_t job_id, int sync_mode, int acquire_fence_fd, int *release_fence_fd);

IM_API IM_STATUS imfill(im_job_id_t job_id, rga_buffer_t dst, im_rect rect, int color);

IM_API IM_STATUS improcess(im_job_id_t job_id,
                           rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                           im_rect srect, im_rect drect, im_rect prect,
                           im_opt_t *opt_ptr, int usage);
#endif

#endif /* #ifndef _im2d_task_hpp_ */
