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

#include "android_utils/android_utils.h"
#include "drm_utils/drm_utils.h"

/*
 * When a pointer is converted to uint64_t, it must first be assigned to
 * an integer of the same size, and then converted to uint64_t. The opposite
 * is true.
 */
#define ptr_to_u64(ptr) ((uint64_t)(uintptr_t)(ptr))
#define u64_to_ptr(var) ((void *)(uintptr_t)(var))

#define is_rga_format(format) ((format) & 0xff00 || (format) == 0)

int convert_to_rga_format(int ex_format);
