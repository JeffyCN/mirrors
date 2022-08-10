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

#ifndef _RGA_UTILS_DRM_UTILS_H_
#define _RGA_UTILS_DRM_UTILS_H_

#include <stdint.h>

#define is_drm_fourcc(format) (((format >> 24) & 0xff) && \
                               ((format >> 16) & 0xff) && \
                               ((format >> 8) & 0xff) && \
                               ((format) & 0xff))

uint32_t get_format_from_drm_fourcc(uint32_t drm_fourcc);

#endif /* #ifndef _RGA_UTILS_DRM_UTILS_H_ */
