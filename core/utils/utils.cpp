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

#include "rga.h"
#include "utils.h"

static int get_compatible_format(int format) {
#if LINUX
    if (format == 0)
        return format;

    if ((format >> 8) != 0) {
        return format;
    } else {
        return format << 8;
    }
#endif
    return format;
}

int convert_to_rga_format(int ex_format) {
    int fmt;

    if (is_drm_fourcc(ex_format))
        return get_format_from_drm_fourcc(ex_format);

    ex_format = get_compatible_format(ex_format);
    if (is_android_hal_format(ex_format))
        return get_format_from_android_hal(ex_format);
    else if (is_rga_format(ex_format))
        return ex_format;

    return RK_FORMAT_UNKNOWN;
}
