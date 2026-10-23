/*
 * Copyright (C) 2021 Rockchip Electronics Co., Ltd.
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

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "im2d_rga_impl"
#else
#define LOG_TAG "im2d_rga_impl"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "im2d.h"
#include "im2d_job.h"
#include "im2d_log.h"
#include "im2d_hardware.h"
#include "im2d_debugger.h"
#include "im2d_context.h"

#include "im2d_impl.h"

#include "core/NormalRga.h"
#include "RgaUtils.h"
#include "utils.h"

#define NORMAL_API_LOG_EN 0

#ifdef ANDROID
#include "GrallocOps.h"

using namespace android;
#endif

#define MAX(n1, n2) ((n1) > (n2) ? (n1) : (n2))
#define GET_GCD(n1, n2) \
    ({ \
        int i; \
        for(i = 1; i <= (n1) && i <= (n2); i++) { \
            if((n1) % i==0 && (n2) % i==0) \
                gcd = i; \
        } \
        gcd; \
    })
#define GET_LCM(n1, n2, gcd) (((n1) * (n2)) / gcd)

#ifdef RT_THREAD
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#endif

RGA_THREAD_LOCAL im_context_t g_im2d_context;

static IM_STATUS rga_support_info_merge_table(rga_info_table_entry *dst_table, rga_info_table_entry *merge_table) {
    if (dst_table == NULL || merge_table == NULL) {
        IM_LOGE("%s[%d] dst or merge table is NULL!\n", __FUNCTION__, __LINE__);
        return IM_STATUS_FAILED;
    }

    dst_table->version              |= merge_table->version;
    dst_table->input_format         |= merge_table->input_format;
    dst_table->output_format        |= merge_table->output_format;
    dst_table->feature              |= merge_table->feature;

    if (merge_table->input_resolution.width > dst_table->input_resolution.width &&
        merge_table->input_resolution.height > dst_table->input_resolution.height) {
        dst_table->input_resolution.width = merge_table->input_resolution.width;
        dst_table->input_resolution.height = merge_table->input_resolution.height;
    }

    if (merge_table->output_resolution.width > dst_table->output_resolution.width &&
        merge_table->output_resolution.height > dst_table->output_resolution.height) {
        dst_table->output_resolution.width = merge_table->output_resolution.width;
        dst_table->output_resolution.height = merge_table->output_resolution.height;
    }

    dst_table->byte_stride          = MAX(dst_table->byte_stride, merge_table->byte_stride);
    dst_table->scale_limit          = MAX(dst_table->scale_limit, merge_table->scale_limit);
    dst_table->performance          = MAX(dst_table->performance, merge_table->performance);
    dst_table->scale_ver_bicubic_limit = MAX(dst_table->scale_ver_bicubic_limit,
                                             merge_table->scale_ver_bicubic_limit);

    return IM_STATUS_SUCCESS;
}

/**
 * rga_version_compare() - Used to compare two struct rga_version_t.
 * @param version1
 * @param version2
 *
 * @returns
 *   if version1 > version2, return >0;
 *   if version1 = version2, return 0;
 *   if version1 < version2, retunr <0.
 */
int rga_version_compare(struct rga_version_t version1, struct rga_version_t version2) {
    if (version1.major > version2.major)
        return 1;
    else if (version1.major == version2.major && version1.minor > version2.minor)
        return 1;
    else if (version1.major == version2.major && version1.minor == version2.minor && version1.revision > version2.revision)
        return 1;
    else if (version1.major == version2.major && version1.minor == version2.minor && version1.revision == version2.revision)
        return 0;

    return -1;
}

/**
 * rga_version_table_get_current_index() - Find the current version index in bind_table.
 *
 * @param version
 * @param table
 * @param table_size
 *
 * @returns if return value >= 0, then index is found, otherwise, the query fails.
 */
int rga_version_table_get_current_index(struct rga_version_t version, const rga_version_bind_table_entry_t *table, int table_size) {
    int index = -1;

    for (int i = (table_size - 1); i >= 0; i--) {
        if (rga_version_compare(version, table[i].current) >= 0) {
            if (i == (table_size - 1)) {
                index = i;
                break;
            } else if (rga_version_compare(table[i + 1].current, version) > 0) {
                index = i;
                break;
            }
        }
    }

    return index;
}

/**
 * rga_version_table_get_minimum_index() - Find the current version index in bind_table.
 *
 * @param version
 * @param table
 * @param table_size
 *
 * @returns if return value >= 0, then index is found, otherwise, the query fails.
 */
int rga_version_table_get_minimum_index(struct rga_version_t version, const rga_version_bind_table_entry_t *table, int table_size) {
    int index = -1;

    for (int i = (table_size - 1); i >= 0; i--) {
        if (rga_version_compare(version, table[i].minimum) >= 0) {
            if (i == (table_size - 1)) {
                index = i;
                break;
            } else if (rga_version_compare(table[i + 1].minimum, version) > 0) {
                index = i;
                break;
            }
        }
    }

    return index;
}

/**
 * rga_version_table_check_minimum_range() - Check if the minimum version is within the required range.
 *
 * @param version
 * @param table
 * @param index
 *
 * @returns
 *   return value > 0, above range.
 *   return value = 0, within range.
 *   return value < 0, below range.
 */
int rga_version_table_check_minimum_range(struct rga_version_t version,
                                          const rga_version_bind_table_entry_t *table,
                                          int table_size, int index) {
    if (rga_version_compare(version, table[index].minimum) >= 0) {
        if (index == (table_size - 1))
            return 0;

        if (rga_version_compare(version, table[index + 1].minimum) < 0)
            return 0;
        else
            return 1;
    } else {
        return -1;
    }
}

static IM_STATUS rga_version_get_current_index_failed_default(struct rga_version_t current, struct rga_version_t minimum) {
    UNUSED(current);
    UNUSED(minimum);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_minimum_index_failed_default(struct rga_version_t current, struct rga_version_t minimum) {
    UNUSED(current);
    UNUSED(minimum);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_witnin_minimun_range_default(struct rga_version_t current, struct rga_version_t minimum) {
    UNUSED(current);
    UNUSED(minimum);

    return IM_STATUS_SUCCESS;
}

static IM_STATUS rga_version_above_minimun_range_default(struct rga_version_t current, struct rga_version_t minimum, const rga_version_bind_table_entry_t *least_version_table) {
    UNUSED(current);
    UNUSED(minimum);
    UNUSED(least_version_table);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_below_minimun_range_default(struct rga_version_t current, struct rga_version_t minimum, const rga_version_bind_table_entry_t *least_version_table) {
    UNUSED(current);
    UNUSED(minimum);
    UNUSED(least_version_table);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_current_index_failed_user_header(struct rga_version_t user_version, struct rga_version_t header_version) {
    IM_LOGE("Failed to get the version binding table of librga, "
            "current version: librga: %s, header: %s",
            user_version.str, header_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_minimum_index_failed_user_header(struct rga_version_t user_version, struct rga_version_t header_version) {
    IM_LOGE("Failed to get the version binding table of header file, "
            "current version: librga: %s, header: %s",
            user_version.str, header_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_above_minimun_range_user_header(struct rga_version_t user_version, struct rga_version_t header_version, const rga_version_bind_table_entry_t *least_version_table) {
    IM_LOGE("The current referenced header_version is %s, but the running librga's version(%s) is too old, "
            "The librga must to be updated to version %s at least. "
            "You can try to update the SDK or update librga.so and header files "
            "through github(https://github.com/airockchip/librga/). ",
            header_version.str, user_version.str,
            least_version_table->current.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_below_minimun_range_user_header(struct rga_version_t user_version, struct rga_version_t header_version, const rga_version_bind_table_entry_t *least_version_table) {
    IM_LOGE("The current librga.so's verison is %s, but the referenced header_version(%s) is too old, "
            "it is recommended to update the librga's header_version to %s and above."
            "You can try to update the SDK or update librga.so and header files "
            "through github(https://github.com/airockchip/librga/)",
            user_version.str, header_version.str,
            least_version_table->minimum.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_current_index_faile_user_driver(struct rga_version_t user_version, struct rga_version_t driver_version) {
    IM_LOGE("Failed to get the version binding table of librga, "
            "current version: librga: %s, driver: %s",
            user_version.str, driver_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_minimum_index_failed_user_driver(struct rga_version_t user_version, struct rga_version_t driver_version) {
    IM_LOGE("Failed to get the version binding table of rga_driver, "
            "current version: librga: %s, driver: %s",
            user_version.str, driver_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_above_minimun_range_user_driver(struct rga_version_t user_version, struct rga_version_t driver_version, const rga_version_bind_table_entry_t *least_version_table) {
    IM_LOGE("The librga must to be updated to version %s at least. "
            "You can try to update the SDK or update librga.so and header files "
            "through github(https://github.com/airockchip/librga/). "
            "current version: librga %s, driver %s.",
            least_version_table->current.str,
            user_version.str, driver_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_below_minimun_range_user_driver(struct rga_version_t user_version, struct rga_version_t driver_version, const rga_version_bind_table_entry_t *least_version_table) {
    IM_LOGE("The driver may be compatible, "
            "but it is best to update the driver to version %s. "
            "You can try to update the SDK or update the "
            "<SDK>/kernel/drivers/video/rockchip/rga3 directory individually. "
            "current version: librga %s, driver %s.",
            least_version_table->minimum.str,
            user_version.str, driver_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static const rga_version_check_ops_t rga_version_check_user_header_ops = {
    rga_version_get_current_index_failed_user_header,
    rga_version_get_minimum_index_failed_user_header,
    rga_version_witnin_minimun_range_default,
    rga_version_above_minimun_range_user_header,
    rga_version_below_minimun_range_user_header,
};

static const rga_version_check_ops_t rga_version_check_user_driver_ops = {
    rga_version_get_current_index_faile_user_driver,
    rga_version_get_minimum_index_failed_user_driver,
    rga_version_witnin_minimun_range_default,
    rga_version_above_minimun_range_user_driver,
    rga_version_below_minimun_range_user_driver,
};

static int rga_version_check(struct rga_version_t current_version, struct rga_version_t minimum_version,
                             const rga_version_bind_table_entry_t *table, int table_size,
                             const rga_version_check_ops_t *ops) {
    int ret;
    int current_bind_index, least_index;

    current_bind_index = rga_version_table_get_current_index(current_version, table, table_size);
    if (current_bind_index < 0)
        return ops->get_current_index_failed ?
               ops->get_current_index_failed(current_version, minimum_version) :
               rga_version_get_current_index_failed_default(current_version, minimum_version);

    switch (rga_version_table_check_minimum_range(minimum_version, table, table_size, current_bind_index)) {
        case 0:
            ops->witnin_minimun_range ?
                ops->witnin_minimun_range(current_version, minimum_version) :
                rga_version_witnin_minimun_range_default(current_version, minimum_version);
            return 0;

        case -1:
            ops->below_minimun_range ?
                ops->below_minimun_range(current_version, minimum_version, &(table[current_bind_index])) :
                rga_version_below_minimun_range_default(current_version, minimum_version, &(table[current_bind_index]));
            return -1;

        case 1:
            least_index = rga_version_table_get_minimum_index(minimum_version, table, table_size);
            if (least_index < 0) {
                ops->get_minimum_index_failed ?
                    ops->get_minimum_index_failed(current_version, minimum_version) :
                    rga_version_get_minimum_index_failed_default(current_version, minimum_version);
                return 1;
            }

            ops->above_minimun_range ?
                ops->above_minimun_range(current_version, minimum_version, &(table[least_index])) :
                rga_version_above_minimun_range_default(current_version, minimum_version, &(table[least_index]));
            return 1;

        default:
            IM_LOGE("This shouldn't happen!");
            return IM_STATUS_FAILED;
    }
}

static IM_STATUS rga_yuv_legality_check(const char *name, rga_buffer_t info, im_rect rect) {
    if ((info.wstride % 2) || (info.hstride % 2) ||
        (info.width % 2)  || (info.height % 2) ||
        (rect.x % 2) || (rect.y % 2) ||
        (rect.width % 2) || (rect.height % 2)) {
        IM_LOGW("%s, Error yuv not align to 2, rect[x,y,w,h] = [%d, %d, %d, %d], "
                "wstride = %d, hstride = %d, format = 0x%x(%s)",
                name, rect.x, rect.y, info.width, info.height, info.wstride, info.hstride,
                info.format, translate_format_str(info.format));
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

bool rga_is_buffer_valid(rga_buffer_t buf) {
    return (buf.phy_addr != NULL || buf.vir_addr != NULL || buf.fd > 0 || buf.handle > 0);
}

bool rga_is_rect_valid(im_rect rect) {
    return (rect.x > 0 || rect.y > 0 || (rect.width > 0 && rect.height > 0));
}

void empty_structure(rga_buffer_t *src, rga_buffer_t *dst, rga_buffer_t *pat,
                     im_rect *srect, im_rect *drect, im_rect *prect, im_opt_t *opt) {
    if (src != NULL)
        memset(src, 0, sizeof(*src));
    if (dst != NULL)
        memset(dst, 0, sizeof(*dst));
    if (pat != NULL)
        memset(pat, 0, sizeof(*pat));
    if (srect != NULL)
        memset(srect, 0, sizeof(*srect));
    if (drect != NULL)
        memset(drect, 0, sizeof(*drect));
    if (prect != NULL)
        memset(prect, 0, sizeof(*prect));
    if (opt != NULL)
        memset(opt, 0, sizeof(*opt));
}

IM_STATUS static rga_set_buffer_info(const char *name, rga_buffer_t image, rga_info_t* info) {
    if(!info) {
        IM_LOGE("Invaild rga_info_t, %s structure address is NULL!", name);
        return IM_STATUS_INVALID_PARAM;
    }

    if (image.handle > 0) {
        info->handle = image.handle;
    } else if(image.phy_addr != NULL) {
        info->phyAddr= image.phy_addr;
    } else if(image.fd > 0) {
        info->fd = image.fd;
        info->mmuFlag = 1;
    } else if(image.vir_addr != NULL) {
        info->virAddr = image.vir_addr;
        info->mmuFlag = 1;
    } else {
        IM_LOGE("Invaild %s image buffer, no address available in buffer buffer, phy_addr = %ld, fd = %d, vir_addr = %ld, handle = %d",
                name, (unsigned long)image.phy_addr, image.fd, (unsigned long)image.vir_addr, image.handle);
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_get_info(struct rga_hw_versions_t *version, rga_info_table_entry *return_table) {
    int ret;
    int  rga_version = 0;
    rga_info_table_entry merge_table;

    memset(&merge_table, 0x0, sizeof(merge_table));

    for (uint32_t i = 0; i < version->size; i++) {
        if (version->version[i].major == 2 &&
            version->version[i].minor == 0) {
            if (version->version[i].revision == 0) {
                rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
                memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
            } else {
                goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 3 &&
                   version->version[i].minor == 0) {
            switch (version->version[i].revision) {
                case 0x16445 :
                    // RK3288
                    rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                case 0x22245 :
                    // RK1108
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                case 0x76831 :
                    // RK3588
                    rga_version = IM_RGA_HW_VERSION_RGA_3_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 3 &&
                   version->version[i].minor == 2) {
            switch (version->version[i].revision) {
                case 0x18218 :
                    // RK3399
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_ROP;
                    break;
                case 0x56726 :
                    // RV1109
                case 0x63318 :
                    // RK3566/RK3568/RK3588
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.input_format |= IM_RGA_SUPPORT_FORMAT_YUYV_422 |
                                                 IM_RGA_SUPPORT_FORMAT_YUV_400;
                    merge_table.output_format |= IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                  IM_RGA_SUPPORT_FORMAT_Y4;
                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_QUANTIZE |
                                            IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC |
                                            IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 3 &&
                   version->version[i].minor == 3) {
            switch (version->version[i].revision) {
                case 0x87975:
                    // RV1106
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.input_format |= IM_RGA_SUPPORT_FORMAT_YUYV_422 |
                                                IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                IM_RGA_SUPPORT_FORMAT_RGBA2BPP;
                    merge_table.output_format |= IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                 IM_RGA_SUPPORT_FORMAT_Y4;
                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_QUANTIZE |
                                           IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC |
                                           IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC |
                                           IM_RGA_SUPPORT_FEATURE_MOSAIC |
                                           IM_RGA_SUPPORT_FEATURE_OSD |
                                           IM_RGA_SUPPORT_FEATURE_PRE_INTR;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 3 &&
                   version->version[i].minor == 6) {
            switch (version->version[i].revision) {
                case 0x92812:
                    // RK3562
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.input_format |= IM_RGA_SUPPORT_FORMAT_YUYV_422 |
                                                IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                IM_RGA_SUPPORT_FORMAT_RGBA2BPP;
                    merge_table.output_format |= IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                 IM_RGA_SUPPORT_FORMAT_Y4;
                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_QUANTIZE |
                                           IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC |
                                           IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC |
                                           IM_RGA_SUPPORT_FEATURE_MOSAIC |
                                           IM_RGA_SUPPORT_FEATURE_OSD |
                                           IM_RGA_SUPPORT_FEATURE_PRE_INTR;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
                }
        } else if (version->version[i].major == 3 &&
                   version->version[i].minor == 7) {
            switch (version->version[i].revision) {
                case 0x93215:
                    // RK3528
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.input_format |= IM_RGA_SUPPORT_FORMAT_YUYV_422 |
                                                IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                IM_RGA_SUPPORT_FORMAT_RGBA2BPP;
                    merge_table.output_format |= IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                 IM_RGA_SUPPORT_FORMAT_Y4;
                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_QUANTIZE |
                                           IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC |
                                           IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC |
                                           IM_RGA_SUPPORT_FEATURE_MOSAIC |
                                           IM_RGA_SUPPORT_FEATURE_OSD |
                                           IM_RGA_SUPPORT_FEATURE_PRE_INTR;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 3 &&
                   version->version[i].minor == 0xa) {
            switch (version->version[i].revision) {
                case 0x07135:
                    // RK3506
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    merge_table.input_resolution.width = 1280;
                    merge_table.input_resolution.height = 8192;
                    merge_table.output_resolution.width = 1280;
                    merge_table.output_resolution.height = 4096;

                    merge_table.input_format |= IM_RGA_SUPPORT_FORMAT_YUYV_422 |
                                                IM_RGA_SUPPORT_FORMAT_YUV_400 |
                                                IM_RGA_SUPPORT_FORMAT_ALPHA_8_BIT;
                    merge_table.output_format |= IM_RGA_SUPPORT_FORMAT_YUV_400;
                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC |
                                           IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC |
                                           IM_RGA_SUPPORT_FEATURE_GAUSS;
                    merge_table.feature &= ~IM_RGA_SUPPORT_FEATURE_ROP;
                    merge_table.scale_ver_bicubic_limit = 600;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 3 &&
                   version->version[i].minor == 0xe) {
            switch (version->version[i].revision) {
                case 0x19357:
                    // RK3576
                    rga_version = IM_RGA_HW_VERSION_RGA_2_PRO_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 4 &&
                   version->version[i].minor == 0x1) {
            switch (version->version[i].revision) {
                case 0x34669:
                    // RV1126B
                    rga_version = IM_RGA_HW_VERSION_RGA_2_PRO_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.output_format &= ~(IM_RGA_SUPPORT_FORMAT_Y4 | IM_RGA_SUPPORT_FORMAT_Y8);
                    merge_table.feature &= ~(IM_RGA_SUPPORT_FEATURE_MOSAIC | IM_RGA_SUPPORT_FEATURE_ROP |
                                             IM_RGA_SUPPORT_FEATURE_QUANTIZE | IM_RGA_SUPPORT_FEATURE_MOSAIC);

                    break;
                case 0x33689:
                    // RK1820
                    rga_version = IM_RGA_HW_VERSION_RGA_2_PRO_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.output_format &= ~(IM_RGA_SUPPORT_FORMAT_Y4 | IM_RGA_SUPPORT_FORMAT_Y8);
                    merge_table.feature &= ~IM_RGA_SUPPORT_FEATURE_MOSAIC;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 3 &&
                   version->version[i].minor == 0xf) {
            switch (version->version[i].revision) {
                case 0x23690:
                    // RV1103B
                    rga_version = IM_RGA_HW_VERSION_RGA_2_LITE2_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }

        } else if (version->version[i].major == 4 &&
                   version->version[i].minor == 0) {
            switch (version->version[i].revision) {
                case 0x18632 :
                    // RK3366/RK3368
                    rga_version = IM_RGA_HW_VERSION_RGA_2_LITE0_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                case 0x23998 :
                    // RK3228H
                case 0x27615 :
                    // RK1808
                case 0x28610 :
                    // RK3326
                    rga_version = IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 5 &&
                   version->version[i].minor == 2) {
            switch (version->version[i].revision) {
                case 0x48482 :
                    // RK3538
                    rga_version = IM_RGA_HW_VERSION_RGA_2_PRO_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.input_format |= IM_RGA_SUPPORT_FORMAT_RGBA_16BIT;
                    merge_table.output_format |= IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT |
                                                 IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT |
                                                 IM_RGA_SUPPORT_FORMAT_RGBA_1010102 |
                                                 IM_RGA_SUPPORT_FORMAT_YUV_444_PACED_10_BIT;
                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_SECURE;
                    merge_table.feature &= ~(IM_RGA_SUPPORT_FEATURE_QUANTIZE | IM_RGA_SUPPORT_FEATURE_ROP);
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (version->version[i].major == 42 &&
                   version->version[i].minor == 0) {
            if (version->version[i].revision == 0x17760) {
                // RK3228
                rga_version = IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX;
                memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
            } else {
                goto TRY_TO_COMPATIBLE;
            }
        } else {
            goto TRY_TO_COMPATIBLE;
        }

        rga_support_info_merge_table(return_table, &merge_table);
    }

    return IM_STATUS_SUCCESS;

TRY_TO_COMPATIBLE:
    if (strncmp((char *)version->version[0].str, "1.3", 3) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_1_INDEX;
    else if (strncmp((char *)version->version[0].str, "1.6", 3) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_1_PLUS_INDEX;
    /*3288 vesion is 2.00*/
    else if (strncmp((char *)version->version[0].str, "2.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
    /*3288w version is 3.00*/
    else if (strncmp((char *)version->version[0].str, "3.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
    else if (strncmp((char *)version->version[0].str, "3.02", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
    else if (strncmp((char *)version->version[0].str, "4.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_LITE0_INDEX;
    /*The version number of lite1 cannot be obtained temporarily.*/
    else if (strncmp((char *)version->version[0].str, "4.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX;
    else
        rga_version = IM_RGA_HW_VERSION_RGA_V_ERR_INDEX;

    memcpy(return_table, &hw_info_table[rga_version], sizeof(rga_info_table_entry));

    if (rga_version == IM_RGA_HW_VERSION_RGA_V_ERR_INDEX) {
        IM_LOGE("Can not get the correct RGA version, please check the driver, version=%s\n",
                version->version[0].str);
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_check_header(struct rga_version_t header_version) {
    int ret;
    int table_size = sizeof(user_header_bind_table) / sizeof(rga_version_bind_table_entry_t);
    struct rga_version_t user_version = RGA_SET_CURRENT_API_VERSION;

    ret = rga_version_check(user_version, header_version,
                            user_header_bind_table, table_size,
                            &rga_version_check_user_header_ops);
    switch (ret) {
        case 0:
            return IM_STATUS_SUCCESS;
        case 1:
        case -1:
        default:
            return IM_STATUS_ERROR_VERSION;
    }
}

IM_STATUS rga_check_driver(struct rga_version_t driver_version) {
    int ret;
    int table_size = sizeof(user_driver_bind_table) / sizeof(rga_version_bind_table_entry_t);
    struct rga_version_t user_version = RGA_SET_CURRENT_API_VERSION;

    ret =  rga_version_check(user_version, driver_version,
                             user_driver_bind_table, table_size,
                             &rga_version_check_user_driver_ops);
    switch (ret) {
        case 0:
        case -1:
            return IM_STATUS_SUCCESS;
        case 1:
        default:
            return IM_STATUS_ERROR_VERSION;
    }
}

IM_STATUS rga_check_info(const char *name, const rga_buffer_t info, const im_rect rect, rga_info_resolution_t resolution_usage) {
    /**************** src/dst judgment ****************/
    if (info.width <= 0 || info.height <= 0 || info.format < 0) {
        IM_LOGW("Illegal %s, the parameter cannot be negative or 0, width = %d, height = %d, format = 0x%x(%s)",
                name, info.width, info.height, info.format, translate_format_str(info.format));
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if (info.width < 2 || info.height < 2) {
        IM_LOGW("Hardware limitation %s, unsupported operation of images smaller than 2 pixels, "
                "width = %d, height = %d",
                name, info.width, info.height);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if (info.wstride < info.width || info.hstride < info.height) {
        IM_LOGW("Invaild %s, Virtual width or height is less than actual width and height, "
                "wstride = %d, width = %d, hstride = %d, height = %d",
                name, info.wstride, info.width, info.hstride, info.height);
        return IM_STATUS_INVALID_PARAM;
    }

    /**************** rect judgment ****************/
    if ((rect.width == 0 && rect.height > 0) ||
        (rect.width > 0 && rect.height == 0)) {
        IM_LOGW("Illegal %s rect, width or height cannot be 0, rect[x,y,w,h] = [%d, %d, %d, %d]",
                name, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if (rect.width < 0 || rect.height < 0 || rect.x < 0 || rect.y < 0) {
        IM_LOGW("Illegal %s rect, the parameter cannot be negative, rect[x,y,w,h] = [%d, %d, %d, %d]",
                name, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if ((rect.width + rect.x > info.wstride) || (rect.height + rect.y > info.hstride)) {
        IM_LOGW("Invaild %s rect, the sum of width and height of rect needs to be less than wstride or hstride, "
                "rect[x,y,w,h] = [%d, %d, %d, %d], wstride = %d, hstride = %d",
                name, rect.x, rect.y, rect.width, rect.height, info.wstride, info.hstride);
        return IM_STATUS_INVALID_PARAM;
    }

    /**************** resolution check ****************/
    if (info.width > resolution_usage.width ||
        info.height > resolution_usage.height) {
        IM_LOGW("Unsupported %s resolution more than %dx%d, width = %d, height = %d",
                name, resolution_usage.width, resolution_usage.height, info.width, info.height);
        return IM_STATUS_NOT_SUPPORTED;
    } else if ((rect.width > 0 && rect.width > resolution_usage.width) ||
               (rect.height > 0 && rect.height > resolution_usage.height)) {
        IM_LOGW("Unsupported %s rect resolution more than %dx%d, rect[x,y,w,h] = [%d, %d, %d, %d]",
                name, resolution_usage.width, resolution_usage.height, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_limit(rga_buffer_t src, rga_buffer_t dst, int scale_usage, int mode_usage) {
    float src_width = 0, src_height = 0;
    float dst_width = 0, dst_height = 0;

    src_width = src.width;
    src_height = src.height;

    if (mode_usage & IM_HAL_TRANSFORM_ROT_270 || mode_usage & IM_HAL_TRANSFORM_ROT_90) {
        dst_width = dst.height;
        dst_height = dst.width;
    } else {
        dst_width = dst.width;
        dst_height = dst.height;
    }

    if (src_width / dst_width > (float)scale_usage ||
        src_height / dst_height > (float)scale_usage ||
        dst_width / src_width > (float)scale_usage ||
        dst_height / src_height > (float)scale_usage) {
        IM_LOGW("Unsupported to scaling more than 1/%d ~ %d times, src[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                scale_usage, scale_usage, src.width, src.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_format(const char *name, rga_buffer_t info, im_rect rect, int format_usage, int mode_usgae) {
    IM_STATUS ret;
    int format = info.format;

    if (format == RK_FORMAT_RGBA_8888 || format == RK_FORMAT_BGRA_8888 ||
        format == RK_FORMAT_RGBX_8888 || format == RK_FORMAT_BGRX_8888 ||
        format == RK_FORMAT_ARGB_8888 || format == RK_FORMAT_ABGR_8888 ||
        format == RK_FORMAT_XRGB_8888 || format == RK_FORMAT_XBGR_8888 ||
        format == RK_FORMAT_RGB_888   || format == RK_FORMAT_BGR_888   ||
        format == RK_FORMAT_RGB_565   || format == RK_FORMAT_BGR_565) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGB) {
            IM_LOGW("%s unsupported RGB format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_ARGB_4444 || format == RK_FORMAT_ABGR_4444 ||
               format == RK_FORMAT_ARGB_5551 || format == RK_FORMAT_ABGR_5551) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_ARGB_16BIT) {
            IM_LOGW("%s unsupported ARGB 4444/5551 format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_RGBA_4444 || format == RK_FORMAT_BGRA_4444 ||
               format == RK_FORMAT_RGBA_5551 || format == RK_FORMAT_BGRA_5551) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGBA_16BIT) {
            IM_LOGW("%s unsupported RGBA 4444/5551 format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_BPP1 || format == RK_FORMAT_BPP2 ||
               format == RK_FORMAT_BPP4 || format == RK_FORMAT_BPP8) {
        if ((~format_usage & IM_RGA_SUPPORT_FORMAT_BPP) && !(mode_usgae & IM_COLOR_PALETTE)) {
            IM_LOGW("%s unsupported BPP format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_YCrCb_420_SP || format == RK_FORMAT_YCbCr_420_SP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT) {
            IM_LOGW("%s unsupported YUV420 semi-planner 8bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_420_P  || format == RK_FORMAT_YCbCr_420_P) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT) {
            IM_LOGW("%s unsupported YUV420 planner 8bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_422_SP || format == RK_FORMAT_YCbCr_422_SP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT) {
            IM_LOGW("%s unsupported YUV422 semi-planner 8bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_422_P  || format == RK_FORMAT_YCbCr_422_P) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT) {
            IM_LOGW("%s unsupported YUV422 planner 8bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_420_SP_10B || format == RK_FORMAT_YCbCr_420_SP_10B) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT) {
            IM_LOGW("%s unsupported YUV420 semi-planner 10bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
        IM_LOGW("If it is an RK encoder output, it needs to be aligned with an odd multiple of 256.\n");
    } else if (format == RK_FORMAT_YCrCb_422_SP_10B || format == RK_FORMAT_YCbCr_422_SP_10B) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT) {
            IM_LOGW("%s unsupported YUV422 semi-planner 10bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
        IM_LOGW("If it is an RK encoder output, it needs to be aligned with an odd multiple of 256.\n");
    } else if (format == RK_FORMAT_YUYV_420 || format == RK_FORMAT_YVYU_420 ||
               format == RK_FORMAT_UYVY_420 || format == RK_FORMAT_VYUY_420) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUYV_420) {
            IM_LOGW("%s unsupported YUYV format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YUYV_422 || format == RK_FORMAT_YVYU_422 ||
               format == RK_FORMAT_UYVY_422 || format == RK_FORMAT_VYUY_422) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUYV_422) {
            IM_LOGW("%s unsupported YUYV format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCbCr_400) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_400) {
            IM_LOGW("%s unsupported YUV400 format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_Y4) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_Y4) {
            IM_LOGW("%s unsupported Y4/Y1 format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_RGBA2BPP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGBA2BPP) {
            IM_LOGW("%s unsupported rgba2bpp format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_A8) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_ALPHA_8_BIT) {
            IM_LOGW("%s unsupported Alpha-8bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_YCrCb_444_SP || format == RK_FORMAT_YCbCr_444_SP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_444_SEMI_PLANNER_8_BIT) {
            IM_LOGW("%s unsupported YUV444 semi-planner 8bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_Y8) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_Y8) {
            IM_LOGW("%s unsupported Y8 format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_RGBA_1010102 || format == RK_FORMAT_BGRA_1010102 ||
               format == RK_FORMAT_ARGB_2101010 || format == RK_FORMAT_ABGR_2101010 ||
               format == RK_FORMAT_RGBX_1010102 || format == RK_FORMAT_BGRX_1010102 ||
               format == RK_FORMAT_XRGB_2101010 || format == RK_FORMAT_XBGR_2101010) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGBA_1010102) {
            IM_LOGW("%s unsupported RGB 10bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_YUV_444_10B) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_444_PACED_10_BIT) {
            IM_LOGW("%s unsupported YUV 10bit format, format = 0x%x(%s)\n%s",
                    name, info.format, translate_format_str(info.format),
                    querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_yuv_legality_check(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else {
        IM_LOGW("%s unsupported this format, format = 0x%x(%s)\n%s",
                name, info.format, translate_format_str(info.format),
                querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_align(const char *name, rga_buffer_t info, int byte_stride, bool is_read) {
    int bpp = 0;
    int bit_stride, pixel_stride, align, gcd;

    /* data mode align */
    switch (info.rd_mode) {
        case IM_FBC_MODE:
            if (info.wstride % 16) {
                IM_LOGE("%s FBC mode does not support width_stride[%d] is non-16 aligned\n",
                        name, info.width);
                return IM_STATUS_NOT_SUPPORTED;
            }

            if (info.hstride % 16) {
                IM_LOGE("%s FBC mode does not support height_stride[%d] is non-16 aligned\n",
                        name, info.height);
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;
        case IM_TILE_MODE:
            if (info.width % 8) {
                IM_LOGE("%s TILE8*8 mode does not support width[%d] is non-8 aligned\n",
                        name, info.width);
                return IM_STATUS_NOT_SUPPORTED;
            }

            if (info.height % 8) {
                IM_LOGE("%s TILE8*8 mode does not support height[%d] is non-8 aligned\n",
                        name, info.height);
                return IM_STATUS_NOT_SUPPORTED;
            }

            if (is_read) {
                if (info.wstride % 16) {
                    IM_LOGE("%s TILE8*8 mode does not support input width_stride[%d] is non-16 aligned\n",
                            name, info.wstride);
                    return IM_STATUS_NOT_SUPPORTED;
                }

                if (info.hstride % 16) {
                    IM_LOGE("%s TILE8*8 mode does not support input height_stride[%d] is non-16 aligned\n",
                            name, info.hstride);
                    return IM_STATUS_NOT_SUPPORTED;
                }
            }
            break;
        default:
            break;
    }

    pixel_stride = get_perPixel_stride_from_format(info.format);

    bit_stride = pixel_stride * info.wstride;
    if (bit_stride % (byte_stride * 8) == 0) {
        return IM_STATUS_NOERROR;
    } else {
        gcd = GET_GCD(pixel_stride, byte_stride * 8);
        align = GET_LCM(pixel_stride, byte_stride * 8, gcd) / pixel_stride;
        IM_LOGW("%s unsupport width stride %d, %s width stride should be %d aligned!",
                name, info.wstride, translate_format_str(info.format), align);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_blend(rga_buffer_t src, rga_buffer_t pat, rga_buffer_t dst, int pat_enable, int mode_usage) {
    int src_fmt, pat_fmt, dst_fmt;
    bool src_isRGB, pat_isRGB, dst_isRGB;

    src_fmt = src.format;
    pat_fmt = pat.format;
    dst_fmt = dst.format;

    src_isRGB = is_rga_format(src_fmt);
    pat_isRGB = is_rga_format(pat_fmt);
    dst_isRGB = is_rga_format(dst_fmt);

    /* bg format check */
    if (rga_is_buffer_valid(pat)) {
        if (!pat_isRGB) {
            IM_LOGW("Blend mode background layer unsupport non-RGB format, pat format = %#x(%s)",
                pat_fmt, translate_format_str(pat_fmt));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else {
        if (!dst_isRGB) {
            IM_LOGW("Blend mode background layer unsupport non-RGB format, dst format = %#x(%s)",
                dst_fmt, translate_format_str(dst_fmt));
            return IM_STATUS_NOT_SUPPORTED;
        }
    }

    /* src1 don't support scale, and src1's size must aqual to dst.' */
    if (pat_enable && (pat.width != dst.width || pat.height != dst.height)) {
        IM_LOGW("In the three-channel mode Alapha blend, the width and height of the src1 channel "
                "must be equal to the dst channel, src1[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                pat.width, pat.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_rotate(int mode_usage, rga_info_table_entry *table) {
    if (table->version & (IM_RGA_HW_VERSION_RGA_1 | IM_RGA_HW_VERSION_RGA_1_PLUS)) {
        if (mode_usage & IM_HAL_TRANSFORM_FLIP_H_V) {
            IM_LOGW("RGA1/RGA1_PLUS cannot support H_V mirror.");
            return IM_STATUS_NOT_SUPPORTED;
        }

        if ((mode_usage & (IM_HAL_TRANSFORM_ROT_90 + IM_HAL_TRANSFORM_ROT_180 + IM_HAL_TRANSFORM_ROT_270)) &&
            (mode_usage & (IM_HAL_TRANSFORM_FLIP_H + IM_HAL_TRANSFORM_FLIP_V + IM_HAL_TRANSFORM_FLIP_H_V))) {
            IM_LOGW("RGA1/RGA1_PLUS cannot support rotate with mirror.");
            return IM_STATUS_NOT_SUPPORTED;
        }
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_feature(rga_buffer_t src, rga_buffer_t pat, rga_buffer_t dst,
                                   int pat_enable, int mode_usage, int feature_usage) {
    if ((mode_usage & IM_COLOR_FILL) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_COLOR_FILL)) {
        IM_LOGW("The platform does not support color fill featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((mode_usage & IM_COLOR_PALETTE) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE)) {
        IM_LOGW("The platform does not support color palette featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((mode_usage & IM_ROP) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_ROP)) {
        IM_LOGW("The platform does not support ROP featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((mode_usage & IM_NN_QUANTIZE) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_QUANTIZE)) {
        IM_LOGW("The platform does not support quantize featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((pat_enable ? (pat.color_space_mode & IM_RGB_TO_YUV_MASK) : 0) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC)) {
        IM_LOGW("The platform does not support src1 channel RGB2YUV color space convert featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((src.color_space_mode & IM_FULL_CSC_MASK ||
        dst.color_space_mode & IM_FULL_CSC_MASK ||
        (pat_enable ? (pat.color_space_mode & IM_FULL_CSC_MASK) : 0)) &&
        (~feature_usage & IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC)) {
        IM_LOGW("The platform does not support dst channel full color space convert(Y2Y/Y2R) featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((mode_usage & IM_MOSAIC) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_MOSAIC)) {
        IM_LOGW("The platform does not support mosaic featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((mode_usage & IM_OSD) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_OSD)) {
        IM_LOGW("The platform does not support osd featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((mode_usage & IM_PRE_INTR) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_PRE_INTR)) {
        IM_LOGW("The platform does not support pre_intr featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((mode_usage & IM_ALPHA_BIT_MAP) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_ALPHA_BIT_MAP)) {
        IM_LOGW("The platform does not support alpha-bit map featrue. \n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((mode_usage & IM_GAUSS) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_GAUSS)) {
        IM_LOGW("The platform does not support gauss featrue.\n%s",
                querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check(const rga_buffer_t src, const rga_buffer_t dst, const rga_buffer_t pat,
                    const im_rect src_rect, const im_rect dst_rect, const im_rect pat_rect, int mode_usage) {
    bool pat_enable = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    rga_session_t *session;
    rga_info_table_entry *rga_info;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    rga_info = &session->hardware_info;

    if (mode_usage & IM_ALPHA_BLEND_MASK) {
        if (rga_is_buffer_valid(pat))
            pat_enable = 1;
    }

    /**************** feature judgment ****************/
    ret = rga_check_feature(src, pat, dst, pat_enable, mode_usage, rga_info->feature);
    if (ret != IM_STATUS_NOERROR)
        return ret;

    /**************** info judgment ****************/
    if (~mode_usage & IM_COLOR_FILL) {
        ret = rga_check_info("src", src, src_rect, rga_info->input_resolution);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_format("src", src, src_rect, rga_info->input_format, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_align("src", src, rga_info->byte_stride, true);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }
    if (pat_enable) {
        /* RGA1 cannot support src1. */
        if (rga_info->version & (IM_RGA_HW_VERSION_RGA_1 | IM_RGA_HW_VERSION_RGA_1_PLUS)) {
            IM_LOGW("RGA1/RGA1_PLUS cannot support src1.");
            return IM_STATUS_NOT_SUPPORTED;
        }


        ret = rga_check_info("pat", pat, pat_rect, rga_info->input_resolution);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_format("pat", pat, pat_rect, rga_info->input_format, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_align("pat", pat, rga_info->byte_stride, true);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }
    ret = rga_check_info("dst", dst, dst_rect, rga_info->output_resolution);
    if (ret != IM_STATUS_NOERROR)
        return ret;
    ret = rga_check_format("dst", dst, dst_rect, rga_info->output_format, mode_usage);
    if (ret != IM_STATUS_NOERROR)
        return ret;
    ret = rga_check_align("dst", dst, rga_info->byte_stride, false);
    if (ret != IM_STATUS_NOERROR)
        return ret;

    if ((~mode_usage & IM_COLOR_FILL)) {
        ret = rga_check_limit(src, dst, rga_info->scale_limit, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }

    if (mode_usage & IM_ALPHA_BLEND_MASK) {
        ret = rga_check_blend(src, pat, dst, pat_enable, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }

    ret = rga_check_rotate(mode_usage, rga_info);
    if (ret != IM_STATUS_NOERROR)
        return ret;

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_external(rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                             im_rect src_rect, im_rect dst_rect, im_rect pat_rect,
                             int mode_usage) {
    int ret;
    int format;

    if (mode_usage & IM_CROP) {
        dst_rect.width = src_rect.width;
        dst_rect.height = src_rect.height;
    }

    rga_apply_rect(&src, &src_rect);
    format = convert_to_rga_format(src.format);
    if (format == RK_FORMAT_UNKNOWN) {
        IM_LOGW("Invaild src format [0x%x]!\n", src.format);
        return IM_STATUS_NOT_SUPPORTED;
    }
    src.format = format;

    rga_apply_rect(&dst, &dst_rect);
    format = convert_to_rga_format(dst.format);
    if (format == RK_FORMAT_UNKNOWN) {
        IM_LOGW("Invaild dst format [0x%x]!\n", dst.format);
        return IM_STATUS_NOT_SUPPORTED;
    }
    dst.format = format;

    if (rga_is_buffer_valid(pat)) {
        rga_apply_rect(&pat, &pat_rect);
        format = convert_to_rga_format(pat.format);
        if (format == RK_FORMAT_UNKNOWN) {
            IM_LOGW("Invaild pat format [0x%x]!\n", pat.format);
            return IM_STATUS_NOT_SUPPORTED;
        }
        pat.format = format;
    }

    return rga_check(src, dst, pat, src_rect, dst_rect, pat_rect, mode_usage);
}

IM_API IM_STATUS rga_import_buffers(struct rga_buffer_pool *buffer_pool) {
    int ret = 0;
    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    if (buffer_pool == NULL) {
        IM_LOGW("buffer pool is null!");
        return IM_STATUS_FAILED;
    }

    ret = ioctl(session->rga_dev_fd, RGA_IOC_IMPORT_BUFFER, buffer_pool);
    if (ret < 0) {
        IM_LOGW("RGA_IOC_IMPORT_BUFFER fail! %s", strerror(errno));
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_API rga_buffer_handle_t rga_import_buffer(uint64_t memory, int type, uint32_t size) {
    struct rga_buffer_pool buffer_pool;
    struct rga_external_buffer buffers[1];

    memset(&buffer_pool, 0x0, sizeof(buffer_pool));
    memset(buffers, 0x0, sizeof(buffers));

    buffers[0].type = type;
    buffers[0].memory = memory;
    buffers[0].memory_info.size = size;

    buffer_pool.buffers = ptr_to_u64(buffers);
    buffer_pool.size = 1;

    if (rga_import_buffers(&buffer_pool) != IM_STATUS_SUCCESS)
        return 0;

    return buffers[0].handle;
}

IM_API rga_buffer_handle_t rga_import_buffer_param(uint64_t memory, int type, im_handle_param_t *param) {
    int format;
    struct rga_buffer_pool buffer_pool;
    struct rga_external_buffer buffers[1];

    memset(&buffer_pool, 0x0, sizeof(buffer_pool));
    memset(buffers, 0x0, sizeof(buffers));

    buffers[0].type = type;
    buffers[0].memory = memory;
    memcpy(&buffers[0].memory_info, param, sizeof(*param));
    format = convert_to_rga_format(buffers[0].memory_info.format);
    if (format == RK_FORMAT_UNKNOWN) {
        IM_LOGW("Invaild format [0x%x]!\n", buffers[0].memory_info.format);
        return IM_STATUS_NOT_SUPPORTED;
    }
    buffers[0].memory_info.format = format >> 8;

    buffer_pool.buffers = ptr_to_u64(buffers);
    buffer_pool.size = 1;

    if (rga_import_buffers(&buffer_pool) != IM_STATUS_SUCCESS)
        return 0;

    return buffers[0].handle;
}

IM_API IM_STATUS rga_release_buffers(struct rga_buffer_pool *buffer_pool) {
    int ret = 0;
    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    if (buffer_pool == NULL) {
        IM_LOGW("buffer pool is null!");
        return IM_STATUS_FAILED;
    }

    ret = ioctl(session->rga_dev_fd, RGA_IOC_RELEASE_BUFFER, buffer_pool);
    if (ret < 0) {
        IM_LOGW("RGA_IOC_RELEASE_BUFFER fail! %s", strerror(errno));
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_API IM_STATUS rga_release_buffer(int handle) {
    struct rga_buffer_pool buffer_pool;
    struct rga_external_buffer buffers[1];

    memset(&buffer_pool, 0x0, sizeof(buffer_pool));
    memset(buffers, 0x0, sizeof(buffers));

    buffers[0].handle = handle;

    buffer_pool.buffers = ptr_to_u64(buffers);
    buffer_pool.size = 1;

    return rga_release_buffers(&buffer_pool);
}

IM_STATUS rga_get_opt(im_opt_t *opt, void *ptr) {
    if (opt == NULL || ptr == NULL)
        return IM_STATUS_FAILED;

    /*
     * Prevent the value of 'color' from being mistakenly used as
     * version information.
     */
    if (rga_version_compare(RGA_GET_API_VERSION(*(im_api_version_t *)ptr),
                            (struct rga_version_t){ 2, 0, 0, {0}}) > 0)
        return IM_STATUS_FAILED;

    if (rga_version_compare(RGA_GET_API_VERSION(*(im_api_version_t *)ptr),
                            (struct rga_version_t){ 1, 7, 2, {0}}) <= 0) {
        opt->color = ((im_opt_t *)ptr)->color;
        memcpy(&opt->colorkey_range, &((im_opt_t *)ptr)->colorkey_range, sizeof(im_colorkey_range));
        memcpy(&opt->nn, &((im_opt_t *)ptr)->nn, sizeof(im_nn_t));
        opt->rop_code = ((im_opt_t *)ptr)->rop_code;
        opt->priority = ((im_opt_t *)ptr)->priority;
        opt->core = ((im_opt_t *)ptr)->core;
    } else {
        memcpy(opt, ptr, sizeof(im_opt_t));
    }

    return IM_STATUS_SUCCESS;
}

int generate_blit_req(struct rga_req *ioc_req, rga_info_t *src, rga_info_t *dst, rga_info_t *src1);
int generate_fill_req(struct rga_req *ioc_req, rga_info_t *dst);
int generate_color_palette_req(struct rga_req *ioc_req, rga_info_t *src, rga_info_t *dst, rga_info_t *lut);

void generate_gaussian_kernel(double sigma_x, double sigma_y, im_size_t ksize, double *kernel) {
    int i, j;
    double sum = 0.0;
    double sX = 2.0 * sigma_x * sigma_x;
    double sY = 2.0 * sigma_y * sigma_y;

    /* Calculate the weight of the Gaussian kernel */
    for (i = -ksize.height / 2; i <= ksize.height / 2; i++) {
        for (j = -ksize.width / 2; j <= ksize.width / 2; j++) {
            int index = (i + ksize.height / 2) * ksize.width + (j + ksize.width / 2);
            double weightX = exp(-(j * j) / sX);
            double weightY = exp(-(i * i) / sY);
            kernel[index] = weightX * weightY / (M_PI * sigma_x * sigma_y);
            sum += kernel[index];
        }
    }

    /* normalized */
    for (i = 0; i < ksize.width * ksize.height; i++) {
        kernel[i] /= sum;
    }
}

static int rga_get_gaussian_special_points(int rows, int cols, double *gauss_kernel, uint32_t *special_points, int factor, int center_factor) {
    int i;
    int index = 0;
    int center_rows = rows / 2;
    int center_cols = cols / 2;

    /* get (0,x) */
    for (i = 0; i <= center_rows; i++) {
        special_points[index] = (gauss_kernel[0 + i] * factor) + 0.5;
        index++;
    }

    /* get (x,center_rows) */
    for (i = 1; i <= center_cols; i++) {
        special_points[index] =
            (gauss_kernel[i * rows + center_rows] * (i == center_cols ? center_factor : factor)) + 0.5;
        index++;
    }

    return index;
}

static IM_STATUS rga_generate_gauss_coe(im_gauss_t *gauss, struct rga_gauss_config *config) {
    double *kernel;
    uint32_t *coe;
    int factor, center_factor;

    if (gauss->ksize.width != 3 ||
        gauss->ksize.height != 3) {
        IM_LOGW("Only supports 3x3 Gaussian blur, please modify ksize[%d, %d]\n",
                gauss->ksize.width, gauss->ksize.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    /* Calculate sigma */
    if (gauss->sigma_x <= 0 && gauss->sigma_y > 0)
        gauss->sigma_x = 0.3 * ((gauss->ksize.width - 1) * 0.5 - 1) + 0.8;

    if (gauss->sigma_x <= 0 && gauss->sigma_y <= 0) {
        gauss->sigma_x = 0.3 * ((gauss->ksize.width - 1) * 0.5 - 1) + 0.8;
        gauss->sigma_y = 0.3 * ((gauss->ksize.height - 1) * 0.5 - 1) + 0.8;
    }

    if (gauss->sigma_y <= 0)
        gauss->sigma_y = gauss->sigma_x;

    /* generate guassian kernel */
    if (gauss->matrix == NULL) {
        kernel = (double *)malloc(gauss->ksize.width * gauss->ksize.height * sizeof(double));

        generate_gaussian_kernel(gauss->sigma_x, gauss->sigma_y, gauss->ksize, kernel);
    } else {
        kernel = gauss->matrix;
    }

    factor = 0xff;
    center_factor = 0xff;

    config->size = (gauss->ksize.width + gauss->ksize.height) / 2;
    coe = (uint32_t *)malloc(config->size * sizeof(uint32_t));
    rga_get_gaussian_special_points(gauss->ksize.width, gauss->ksize.height,
                                    kernel, coe, factor, center_factor);
    config->coe_ptr = ptr_to_u64(coe);

    if (gauss->matrix == NULL)
        free(kernel);

    return IM_STATUS_SUCCESS;
}

static int rga_get_default_csc_mode(int format)
{
    if  (is_rgb_format(format)) {
        return IM_RGB_FULL;
    } else if (is_yuv_format(format)) {
        return IM_YUV_BT601_LIMIT_RANGE;
    }

    return 0;
}

static int rga_get_csc_mode(rga_info_table_entry *entry, int in, int out)
{
    int mode = 0;

    switch (in) {
        case IM_RGB_FULL:
            switch (out) {
                case IM_YUV_BT601_LIMIT_RANGE:
                    mode = IM_RGB_TO_YUV_BT601_LIMIT;
                    break;
                case IM_YUV_BT601_FULL_RANGE:
                    mode = IM_RGB_TO_YUV_BT601_FULL;
                    break;
                case IM_YUV_BT709_LIMIT_RANGE:
                    if (entry->feature & IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC)
                        mode = rgb2yuv_709_limit;
                    else
                        mode = IM_RGB_TO_YUV_BT709_LIMIT;

                    break;
                case IM_YUV_BT709_FULL_RANGE:
                    mode = rgb2yuv_709_full;
                    break;
                case IM_RGB_FULL:
                    break;
                case IM_RGB_CLIP:
                default:
                    IM_LOGW("Unsupported full CSC mode! src %s(0x%x), dst %s(0x%x)",
                            string_color_space(in), in,
                            string_color_space(out), out);
                    return IM_STATUS_NOT_SUPPORTED;
            }

            break;

        case IM_YUV_BT601_LIMIT_RANGE:
            switch (out) {
                case IM_RGB_FULL:
                    mode = IM_YUV_TO_RGB_BT601_LIMIT;
                    break;
                case IM_YUV_BT601_FULL_RANGE:
                    mode = yuv2yuv_601_limit_2_601_full;
                    break;
                case IM_YUV_BT709_LIMIT_RANGE:
                    mode = yuv2yuv_601_limit_2_709_limit;
                    break;
                case IM_YUV_BT709_FULL_RANGE:
                    mode = yuv2yuv_601_limit_2_709_full;
                    break;
                case IM_YUV_BT601_LIMIT_RANGE:
                    break;
                case IM_RGB_CLIP:
                default:
                    IM_LOGW("Unsupported full CSC mode! src %s(0x%x), dst %s(0x%x)",
                            string_color_space(in), in,
                            string_color_space(out), out);
                    return IM_STATUS_NOT_SUPPORTED;
            }
            break;

        case IM_YUV_BT601_FULL_RANGE:
            switch (out) {
                case IM_RGB_FULL:
                    mode = IM_YUV_TO_RGB_BT601_FULL;
                    break;
                case IM_YUV_BT601_LIMIT_RANGE:
                    mode = yuv2yuv_601_full_2_601_limit;
                    break;
                case IM_YUV_BT709_LIMIT_RANGE:
                    mode = yuv2yuv_601_full_2_709_limit;
                    break;
                case IM_YUV_BT709_FULL_RANGE:
                    mode = yuv2yuv_601_full_2_709_full;
                    break;
                case IM_YUV_BT601_FULL_RANGE:
                    break;
                case IM_RGB_CLIP:
                default:
                    IM_LOGW("Unsupported full CSC mode! src %s(0x%x), dst %s(0x%x)",
                            string_color_space(in), in,
                            string_color_space(out), out);
                    return IM_STATUS_NOT_SUPPORTED;
            }
            break;

        case IM_YUV_BT709_LIMIT_RANGE:
            switch (out) {
                case IM_RGB_FULL:
                    mode = IM_YUV_TO_RGB_BT709_LIMIT;
                    break;
                case IM_YUV_BT601_LIMIT_RANGE:
                    mode = yuv2yuv_709_limit_2_601_limit;
                    break;
                case IM_YUV_BT601_FULL_RANGE:
                    mode = yuv2yuv_709_limit_2_601_full;
                    break;
                case IM_YUV_BT709_FULL_RANGE:
                    mode = yuv2yuv_709_limit_2_709_full;
                    break;
                case IM_YUV_BT709_LIMIT_RANGE:
                    break;
                case IM_RGB_CLIP:
                default:
                    IM_LOGW("Unsupported full CSC mode! src %s(0x%x), dst %s(0x%x)",
                            string_color_space(in), in,
                            string_color_space(out), out);
                    return IM_STATUS_NOT_SUPPORTED;
            }
            break;

        case IM_YUV_BT709_FULL_RANGE:
            switch (out) {
                case IM_RGB_FULL:
                    mode = yuv2rgb_709_full;
                    break;
                case IM_YUV_BT601_LIMIT_RANGE:
                    mode = yuv2yuv_709_full_2_601_limit;
                    break;
                case IM_YUV_BT601_FULL_RANGE:
                    mode = yuv2yuv_709_full_2_601_full;
                    break;
                case IM_YUV_BT709_LIMIT_RANGE:
                    mode = yuv2yuv_709_full_2_709_limit;
                    break;
                case IM_YUV_BT709_FULL_RANGE:
                    break;
                case IM_RGB_CLIP:
                default:
                    IM_LOGW("Unsupported full CSC mode! src %s(0x%x), dst %s(0x%x)",
                            string_color_space(in), in,
                            string_color_space(out), out);
                    return IM_STATUS_NOT_SUPPORTED;
            }
            break;

        case IM_RGB_CLIP:
        default:
            IM_LOGW("Unsupported full CSC mode! src %s(0x%x), dst %s(0x%x)",
                    string_color_space(in), in,
                    string_color_space(out), out);
            return IM_STATUS_NOT_SUPPORTED;
    }

    return mode;
}

IM_STATUS rga_task_submit(im_job_handle_t job_handle, rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                          im_rect srect, im_rect drect, im_rect prect,
                          int acquire_fence_fd, int *release_fence_fd,
                          im_opt_t *opt_ptr, int usage) {
    int ret;
    int format;
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    rga_info_t patinfo;

    im_opt_t opt;

    struct rga_req req;
    struct rga2_req compat_req;
    void *ioc_req = NULL;

    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    get_debug_state();
    if (is_debug_en())
        rga_dump_info(IM_LOG_DEBUG | IM_LOG_FORCE,
                      job_handle, &src, &dst, &pat, &srect, &drect, &prect,
                      acquire_fence_fd, release_fence_fd, opt_ptr, usage);

    memset(&opt, 0x0, sizeof(opt));
    rga_get_opt(&opt, opt_ptr);

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));
    memset(&patinfo, 0, sizeof(rga_info_t));
    memset(&req, 0, sizeof(req));

    if (usage & IM_COLOR_FILL) {
        ret = rga_set_buffer_info("dst", dst, &dstinfo);
    } else {
        ret = rga_set_buffer_info("src", src, &srcinfo);
        ret = rga_set_buffer_info("dst", dst, &dstinfo);
    }

    if (ret <= 0)
        return (IM_STATUS)ret;

    rga_apply_rect(&src, &srect);
    format = convert_to_rga_format(src.format);
    if (format == RK_FORMAT_UNKNOWN) {
        IM_LOGW("Invaild src format [0x%x]!\n", src.format);
        return IM_STATUS_NOT_SUPPORTED;
    }
    src.format = format;

    rga_set_rect(&srcinfo.rect, srect.x, srect.y, src.width, src.height, src.wstride, src.hstride, src.format);

    rga_apply_rect(&dst, &drect);
    format = convert_to_rga_format(dst.format);
    if (format == RK_FORMAT_UNKNOWN) {
        IM_LOGW("Invaild dst format [0x%x]!\n", dst.format);
        return IM_STATUS_NOT_SUPPORTED;
    }
    dst.format = format;

    rga_set_rect(&dstinfo.rect, drect.x, drect.y, dst.width, dst.height, dst.wstride, dst.hstride, dst.format);

    if (((usage & IM_COLOR_PALETTE) || (usage & IM_ALPHA_BLEND_MASK)) &&
        rga_is_buffer_valid(pat)) {

        ret = rga_set_buffer_info("src1/pat", pat, &patinfo);
        if (ret <= 0)
            return (IM_STATUS)ret;

        rga_apply_rect(&pat, &prect);
        format = convert_to_rga_format(pat.format);
        if (format == RK_FORMAT_UNKNOWN) {
            IM_LOGW("Invaild pat format [0x%x]!\n", pat.format);
            return IM_STATUS_NOT_SUPPORTED;
        }
        pat.format = format;

        rga_set_rect(&patinfo.rect, prect.x, prect.y, pat.width, pat.height, pat.wstride, pat.hstride, pat.format);
    }

    ret = rga_check(src, dst, pat, srect, drect, prect, usage);
    if(ret != IM_STATUS_NOERROR)
        return (IM_STATUS)ret;

    /* scaling interpolation */
    if (opt.interp & IM_INTERP_HORIZ_FLAG ||
        opt.interp & IM_INTERP_VERTI_FLAG) {
        if (opt.interp & IM_INTERP_HORIZ_FLAG) {
            srcinfo.scale_mode |= opt.interp & (IM_INTERP_MASK << IM_INTERP_HORIZ_SHIFT);
        }
        if (opt.interp & IM_INTERP_VERTI_FLAG) {
            srcinfo.scale_mode |= opt.interp & (IM_INTERP_MASK << IM_INTERP_VERTI_SHIFT);
        }
    } else {
        srcinfo.scale_mode |= (opt.interp & IM_INTERP_MASK) << IM_INTERP_HORIZ_SHIFT;
        srcinfo.scale_mode |= (opt.interp & IM_INTERP_MASK) << IM_INTERP_VERTI_SHIFT;
    }

    /* Transform */
    if (usage & IM_HAL_TRANSFORM_MASK) {
        switch (usage & (IM_HAL_TRANSFORM_ROT_90 + IM_HAL_TRANSFORM_ROT_180 + IM_HAL_TRANSFORM_ROT_270)) {
            case IM_HAL_TRANSFORM_ROT_90:
                srcinfo.rotation = HAL_TRANSFORM_ROT_90;
                break;
            case IM_HAL_TRANSFORM_ROT_180:
                srcinfo.rotation = HAL_TRANSFORM_ROT_180;
                break;
            case IM_HAL_TRANSFORM_ROT_270:
                srcinfo.rotation = HAL_TRANSFORM_ROT_270;
                break;
        }

        switch (usage & (IM_HAL_TRANSFORM_FLIP_V + IM_HAL_TRANSFORM_FLIP_H + IM_HAL_TRANSFORM_FLIP_H_V)) {
            case IM_HAL_TRANSFORM_FLIP_V:
                srcinfo.rotation |= srcinfo.rotation ?
                                    HAL_TRANSFORM_FLIP_V << 4 :
                                    HAL_TRANSFORM_FLIP_V;
                break;
            case IM_HAL_TRANSFORM_FLIP_H:
                srcinfo.rotation |= srcinfo.rotation ?
                                    HAL_TRANSFORM_FLIP_H << 4 :
                                    HAL_TRANSFORM_FLIP_H;
                break;
            case IM_HAL_TRANSFORM_FLIP_H_V:
                srcinfo.rotation |= srcinfo.rotation ?
                                    HAL_TRANSFORM_FLIP_H_V << 4 :
                                    HAL_TRANSFORM_FLIP_H_V;
                break;
        }

        if(srcinfo.rotation ==0)
            IM_LOGE("rga_im2d: Could not find rotate/flip usage : 0x%x \n", usage);
    }

    /* set 5551 Alpha bit */
    if ((usage & IM_ALPHA_BIT_MAP) &&
        (pat.format == RK_FORMAT_RGBA_5551 || pat.format == RK_FORMAT_BGRA_5551 ||
         pat.format == RK_FORMAT_ARGB_5551 || pat.format == RK_FORMAT_ABGR_5551)) {
        srcinfo.rgba5551_flags = 1;
        srcinfo.rgba5551_alpha0 = pat.alpha_bit.alpha0;
        srcinfo.rgba5551_alpha1 = pat.alpha_bit.alpha1;
    }

    /* Blend */
    if (usage & IM_ALPHA_BLEND_MASK) {
        switch(usage & IM_ALPHA_BLEND_MASK) {
            case IM_ALPHA_BLEND_SRC:
                srcinfo.blend = RGA_ALPHA_BLEND_SRC;
                break;
            case IM_ALPHA_BLEND_DST:
                srcinfo.blend = RGA_ALPHA_BLEND_DST;
                break;
            case IM_ALPHA_BLEND_SRC_OVER:
                srcinfo.blend = RGA_ALPHA_BLEND_SRC_OVER;
                break;
            case IM_ALPHA_BLEND_DST_OVER:
                srcinfo.blend = RGA_ALPHA_BLEND_DST_OVER;
                break;
            case IM_ALPHA_BLEND_SRC_IN:
                srcinfo.blend = RGA_ALPHA_BLEND_SRC_IN;
                break;
            case IM_ALPHA_BLEND_DST_IN:
                srcinfo.blend = RGA_ALPHA_BLEND_DST_IN;
                break;
            case IM_ALPHA_BLEND_SRC_OUT:
                srcinfo.blend = RGA_ALPHA_BLEND_SRC_OUT;
                break;
            case IM_ALPHA_BLEND_DST_OUT:
                srcinfo.blend = RGA_ALPHA_BLEND_DST_OUT;
                break;
            case IM_ALPHA_BLEND_SRC_ATOP:
                srcinfo.blend = RGA_ALPHA_BLEND_SRC_ATOP;
                break;
            case IM_ALPHA_BLEND_DST_ATOP:
                srcinfo.blend = RGA_ALPHA_BLEND_DST_ATOP;
                break;
            case IM_ALPHA_BLEND_XOR:
                srcinfo.blend = RGA_ALPHA_BLEND_XOR;
                break;
        }

        if (usage & IM_ALPHA_BLEND_PRE_MUL)
            srcinfo.blend |= (1 << 12);

        if(srcinfo.blend == 0)
            IM_LOGE("rga_im2d: Could not find blend usage : 0x%x \n", usage);

        /* set global alpha */
        srcinfo.blend |= (src.global_alpha & 0xff) << 16;
        srcinfo.blend |= (dst.global_alpha & 0xff) << 24;
    }

    /* color key */
    if (usage & IM_ALPHA_COLORKEY_MASK) {
        if (!(srcinfo.blend & 0xfff))
            srcinfo.blend |= 0xffff1001;

        srcinfo.colorkey_en = 1;
        srcinfo.colorkey_min = opt.colorkey_range.min;
        srcinfo.colorkey_max = opt.colorkey_range.max;
        switch (usage & IM_ALPHA_COLORKEY_MASK) {
            case IM_ALPHA_COLORKEY_NORMAL:
                srcinfo.colorkey_mode = 0;
                break;
            case IM_ALPHA_COLORKEY_INVERTED:
                srcinfo.colorkey_mode = 1;
                break;
        }
    }

    /* OSD */
    if (usage & IM_OSD) {
        srcinfo.osd_info.enable = true;

        srcinfo.osd_info.mode_ctrl.mode = opt.osd_config.osd_mode;

        srcinfo.osd_info.mode_ctrl.width_mode = opt.osd_config.block_parm.width_mode;
        if (opt.osd_config.block_parm.width_mode == IM_OSD_BLOCK_MODE_NORMAL)
            srcinfo.osd_info.mode_ctrl.block_fix_width = opt.osd_config.block_parm.width;
        else if (opt.osd_config.block_parm.width_mode == IM_OSD_BLOCK_MODE_DIFFERENT)
            srcinfo.osd_info.mode_ctrl.unfix_index = opt.osd_config.block_parm.width_index;
        srcinfo.osd_info.mode_ctrl.block_num = opt.osd_config.block_parm.block_count;
        srcinfo.osd_info.mode_ctrl.default_color_sel = opt.osd_config.block_parm.background_config;
        srcinfo.osd_info.mode_ctrl.direction_mode = opt.osd_config.block_parm.direction;
        srcinfo.osd_info.mode_ctrl.color_mode = opt.osd_config.block_parm.color_mode;

        if (pat.format == RK_FORMAT_RGBA2BPP) {
            srcinfo.osd_info.bpp2_info.ac_swap = opt.osd_config.bpp2_info.ac_swap;
            srcinfo.osd_info.bpp2_info.endian_swap = opt.osd_config.bpp2_info.endian_swap;
            srcinfo.osd_info.bpp2_info.color0.value = opt.osd_config.bpp2_info.color0.value;
            srcinfo.osd_info.bpp2_info.color1.value = opt.osd_config.bpp2_info.color1.value;
        } else {
            srcinfo.osd_info.bpp2_info.color0.value = opt.osd_config.block_parm.normal_color.value;
            srcinfo.osd_info.bpp2_info.color1.value = opt.osd_config.block_parm.invert_color.value;
        }

        switch (opt.osd_config.invert_config.invert_channel) {
            case IM_OSD_INVERT_CHANNEL_NONE:
                srcinfo.osd_info.mode_ctrl.invert_enable = (0x1 << 1) | (0x1 << 2);
                break;
            case IM_OSD_INVERT_CHANNEL_Y_G:
                srcinfo.osd_info.mode_ctrl.invert_enable = 0x1 << 2;
                break;
            case IM_OSD_INVERT_CHANNEL_C_RB:
                srcinfo.osd_info.mode_ctrl.invert_enable = 0x1 << 1;
                break;
            case IM_OSD_INVERT_CHANNEL_ALPHA:
                srcinfo.osd_info.mode_ctrl.invert_enable = (0x1 << 0) | (0x1 << 1) | (0x1 << 2);
                break;
            case IM_OSD_INVERT_CHANNEL_COLOR:
                srcinfo.osd_info.mode_ctrl.invert_enable = 0;
                break;
            case IM_OSD_INVERT_CHANNEL_BOTH:
                srcinfo.osd_info.mode_ctrl.invert_enable = 0x1 << 0;
        }
        srcinfo.osd_info.mode_ctrl.invert_flags_mode = opt.osd_config.invert_config.flags_mode;
        srcinfo.osd_info.mode_ctrl.flags_index = opt.osd_config.invert_config.flags_index;

        srcinfo.osd_info.last_flags = opt.osd_config.invert_config.invert_flags;
        srcinfo.osd_info.cur_flags = opt.osd_config.invert_config.current_flags;

        srcinfo.osd_info.mode_ctrl.invert_mode = opt.osd_config.invert_config.invert_mode;
        if (opt.osd_config.invert_config.invert_mode == IM_OSD_INVERT_USE_FACTOR) {
            srcinfo.osd_info.cal_factor.alpha_max = opt.osd_config.invert_config.factor.alpha_max;
            srcinfo.osd_info.cal_factor.alpha_min = opt.osd_config.invert_config.factor.alpha_min;
            srcinfo.osd_info.cal_factor.crb_max = opt.osd_config.invert_config.factor.crb_max;
            srcinfo.osd_info.cal_factor.crb_min = opt.osd_config.invert_config.factor.crb_min;
            srcinfo.osd_info.cal_factor.yg_max = opt.osd_config.invert_config.factor.yg_max;
            srcinfo.osd_info.cal_factor.yg_min = opt.osd_config.invert_config.factor.yg_min;
        }
        srcinfo.osd_info.mode_ctrl.invert_thresh = opt.osd_config.invert_config.threash;
    }

    /* set NN quantize */
    if (usage & IM_NN_QUANTIZE) {
        dstinfo.nn.nn_flag = 1;
        dstinfo.nn.scale_r  = opt.nn.scale_r;
        dstinfo.nn.scale_g  = opt.nn.scale_g;
        dstinfo.nn.scale_b  = opt.nn.scale_b;
        dstinfo.nn.offset_r = opt.nn.offset_r;
        dstinfo.nn.offset_g = opt.nn.offset_g;
        dstinfo.nn.offset_b = opt.nn.offset_b;
    }

    /* set ROP */
    if (usage & IM_ROP) {
        srcinfo.rop_code = opt.rop_code;
    }

    /* set mosaic */
    if (usage & IM_MOSAIC) {
        srcinfo.mosaic_info.enable = true;
        srcinfo.mosaic_info.mode = opt.mosaic_mode;
    }

    /* set intr config */
    if (usage & IM_PRE_INTR) {
        srcinfo.pre_intr.enable = true;

        srcinfo.pre_intr.read_intr_en = opt.intr_config.flags & IM_INTR_READ_INTR ? true : false;
        if (srcinfo.pre_intr.read_intr_en) {
            srcinfo.pre_intr.read_intr_en = true;
            srcinfo.pre_intr.read_hold_en = opt.intr_config.flags & IM_INTR_READ_HOLD ? true : false;
            srcinfo.pre_intr.read_threshold = opt.intr_config.read_threshold;
        }

        srcinfo.pre_intr.write_intr_en = opt.intr_config.flags & IM_INTR_WRITE_INTR ? true : false;
        if (srcinfo.pre_intr.write_intr_en > 0) {
                srcinfo.pre_intr.write_start = opt.intr_config.write_start;
                srcinfo.pre_intr.write_step = opt.intr_config.write_step;
        }
    }

    /* special config for color space convert */
    if ((dst.color_space_mode & IM_YUV_TO_RGB_MASK) && (dst.color_space_mode & IM_RGB_TO_YUV_MASK)) {
        if (rga_is_buffer_valid(pat) &&
            is_yuv_format(src.format) &&
            is_rgb_format(pat.format) &&
            is_yuv_format(dst.format)) {
            /* When configured as 709_limit, full_csc of RGA2 is preferred. */
            if (((dst.color_space_mode & IM_RGB_TO_YUV_MASK) == IM_RGB_TO_YUV_BT709_LIMIT) &&
                (session->hardware_info.feature & IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC)) {
                dstinfo.color_space_mode = (dst.color_space_mode & IM_YUV_TO_RGB_MASK) | rgb2yuv_709_limit;
            } else {
                dstinfo.color_space_mode = dst.color_space_mode;
            }
        } else {
            IM_LOGW("Not yuv + rgb -> yuv does not need for color_sapce_mode R2Y & Y2R, please fix, "
                    "src_fromat = 0x%x(%s), src1_format = 0x%x(%s), dst_format = 0x%x(%s)",
                    src.format, translate_format_str(src.format),
                    pat.format, translate_format_str(pat.format),
                    dst.format, translate_format_str(dst.format));
            return IM_STATUS_ILLEGAL_PARAM;
        }
    } else if (dst.color_space_mode & (IM_YUV_TO_RGB_MASK)) {
        if (rga_is_buffer_valid(pat) &&
            is_yuv_format(src.format) &&
            is_rgb_format(pat.format) &&
            is_rgb_format(dst.format)) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else if (is_yuv_format(src.format) &&
                   is_rgb_format(dst.format)) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else {
            IM_LOGW("Not yuv to rgb does not need for color_sapce_mode, please fix, "
                    "src_fromat = 0x%x(%s), src1_format = 0x%x(%s), dst_format = 0x%x(%s)",
                    src.format, translate_format_str(src.format),
                    pat.format, rga_is_buffer_valid(pat) ? translate_format_str(pat.format) : "none",
                    dst.format, translate_format_str(dst.format));
            return IM_STATUS_ILLEGAL_PARAM;
        }
    } else if (dst.color_space_mode & (IM_RGB_TO_YUV_MASK)) {
        if (rga_is_buffer_valid(pat) &&
            is_rgb_format(src.format) &&
            is_rgb_format(pat.format) &&
            is_yuv_format(dst.format)) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else if (is_rgb_format(src.format) &&
                   is_yuv_format(dst.format)) {
            /* When configured as 709_limit, full_csc of RGA2 is preferred. */
            if (((dst.color_space_mode & IM_RGB_TO_YUV_MASK) == IM_RGB_TO_YUV_BT709_LIMIT) &&
                (session->hardware_info.feature & IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC)) {
                dstinfo.color_space_mode = rgb2yuv_709_limit;
            } else {
                dstinfo.color_space_mode = dst.color_space_mode;
            }
        } else {
            IM_LOGW("Not rgb to yuv does not need for color_sapce_mode, please fix, "
                    "src_fromat = 0x%x(%s), src1_format = 0x%x(%s), dst_format = 0x%x(%s)",
                    src.format, translate_format_str(src.format),
                    pat.format, rga_is_buffer_valid(pat) ? translate_format_str(pat.format) : "none",
                    dst.format, translate_format_str(dst.format));
            return IM_STATUS_ILLEGAL_PARAM;
        }
    } else if (src.color_space_mode & IM_FULL_CSC_MASK ||
               pat.color_space_mode & IM_FULL_CSC_MASK ||
               dst.color_space_mode & IM_FULL_CSC_MASK) {
        if (src.color_space_mode == IM_COLOR_SPACE_DEFAULT)
            src.color_space_mode = rga_get_default_csc_mode(src.format);
        if (pat.color_space_mode == IM_COLOR_SPACE_DEFAULT)
            pat.color_space_mode = rga_get_default_csc_mode(pat.format);
        if (dst.color_space_mode == IM_COLOR_SPACE_DEFAULT)
            dst.color_space_mode = rga_get_default_csc_mode(dst.format);

        if (rga_is_buffer_valid(pat)) {
            if (!is_rgb_format(pat.format)) {
                IM_LOGW("src1/pat channel only support RGBA/RGB format, please fix, "
                        "src_fromat = 0x%x(%s), src1_format = 0x%x(%s), dst_format = 0x%x(%s)",
                        src.format, translate_format_str(src.format),
                        pat.format, rga_is_buffer_valid(pat) ? translate_format_str(pat.format) : "none",
                        dst.format, translate_format_str(dst.format));
                return IM_STATUS_ILLEGAL_PARAM;
            }

            int src_csc_mode = rga_get_csc_mode(&session->hardware_info,
                                                src.color_space_mode, pat.color_space_mode);
            int pat_csc_mode = rga_get_csc_mode(&session->hardware_info,
                                                pat.color_space_mode, dst.color_space_mode);
            if (src_csc_mode < 0 || pat_csc_mode < 0)
                return IM_STATUS_NOT_SUPPORTED;

            dstinfo.color_space_mode = src_csc_mode | pat_csc_mode;
        } else {
            int csc_mode = rga_get_csc_mode(&session->hardware_info,
                                            src.color_space_mode, dst.color_space_mode);
            if (csc_mode < 0)
                return IM_STATUS_NOT_SUPPORTED;

            dstinfo.color_space_mode = csc_mode;
        }
    }

    if (dst.format == RK_FORMAT_Y4 || dst.format == RK_FORMAT_Y8) {
        switch (dst.color_space_mode) {
            case IM_RGB_TO_Y4 :
                dstinfo.dither.enable = 0;
                dstinfo.dither.mode = 0;
                break;
            case IM_RGB_TO_Y4_DITHER :
                dstinfo.dither.enable = 1;
                dstinfo.dither.mode = 0;
                break;
            case IM_RGB_TO_Y1_DITHER :
                dstinfo.dither.enable = 1;
                dstinfo.dither.mode = 1;
                break;
            default :
                dstinfo.dither.enable = 1;
                dstinfo.dither.mode = 0;
                break;
        }
        dstinfo.dither.lut0_l = 0x3210;
        dstinfo.dither.lut0_h = 0x7654;
        dstinfo.dither.lut1_l = 0xba98;
        dstinfo.dither.lut1_h = 0xfedc;
    }

    /* set gauss */
    if (usage & IM_GAUSS) {
        if (usage & IM_HAL_TRANSFORM_MASK) {
            IM_LOGW("Gaussian blur does not support rotation/mirror\n");
            return IM_STATUS_NOT_SUPPORTED;
        }

        if ((src.width != dst.width) || (src.height != dst.height)) {
            IM_LOGW("Gaussian blur does not support scaling, src[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                    src.width, src.height, dst.width, dst.height);
            return IM_STATUS_INVALID_PARAM;
        }

        ret = rga_generate_gauss_coe(&opt.gauss_config, &srcinfo.gauss_config);
        if (ret != IM_STATUS_SUCCESS)
            return (IM_STATUS)ret;

        srcinfo.blend |= (src.global_alpha & 0xff) << 16;
    }

    srcinfo.rd_mode = src.rd_mode;
    dstinfo.rd_mode = dst.rd_mode;
    if (rga_is_buffer_valid(pat))
        patinfo.rd_mode = pat.rd_mode;

    if (usage & IM_ASYNC) {
        if (release_fence_fd == NULL) {
            IM_LOGW("Async mode release_fence_fd cannot be NULL!");
            ret = IM_STATUS_ILLEGAL_PARAM;
            goto release_resource;
        }

        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    } else {
        dstinfo.sync_mode = RGA_BLIT_SYNC;
    }

    dstinfo.in_fence_fd = acquire_fence_fd;
    dstinfo.core = opt.core ? opt.core : g_im2d_context.core;
    dstinfo.priority = opt.priority ? opt.priority : g_im2d_context.priority;

    dstinfo.job_handle = job_handle;

    if (usage & IM_COLOR_FILL) {
        dstinfo.color = opt.color;

        ret = generate_fill_req(&req, &dstinfo);
    } else if (usage & IM_COLOR_PALETTE) {
        ret = generate_color_palette_req(&req, &srcinfo, &dstinfo, &patinfo);
    } else if ((usage & IM_ALPHA_BLEND_MASK) && rga_is_buffer_valid(pat)) {
        ret = generate_blit_req(&req, &srcinfo, &dstinfo, &patinfo);
    } else {
        ret = generate_blit_req(&req, &srcinfo, &dstinfo, NULL);
    }
    if (ret < 0) {
        IM_LOGE("failed to generate task req!\n");

        rga_dump_info(IM_LOG_ERROR | IM_LOG_FORCE,
                      job_handle, &src, &dst, &pat, &srect, &drect, &prect,
                      acquire_fence_fd, release_fence_fd, opt_ptr, usage);

        ret = IM_STATUS_FAILED;
        goto release_resource;
    }

    if (job_handle > 0) {
        im_rga_job_t *job = NULL;

        pthread_mutex_lock(&g_im2d_job_manager.mutex);

        job = rga_map_find_job(&g_im2d_job_manager.job_map, job_handle);
        if (job == NULL) {
            IM_LOGE("cannot find job_handle[%d]\n", job_handle);
            pthread_mutex_unlock(&g_im2d_job_manager.mutex);
            ret = IM_STATUS_ILLEGAL_PARAM;
            goto release_resource;
        } else if (job->task_count >= RGA_TASK_NUM_MAX) {
            IM_LOGE("job[%d] add task failed! too many tasks, count = %d\n", job_handle, job->task_count);

            pthread_mutex_unlock(&g_im2d_job_manager.mutex);
            ret = IM_STATUS_ILLEGAL_PARAM;
            goto release_resource;
        }

        job->req[job->task_count] = req;
        job->task_count++;

        pthread_mutex_unlock(&g_im2d_job_manager.mutex);
    } else {
        switch (session->driver_type) {
            case RGA_DRIVER_IOC_RGA1:
            case RGA_DRIVER_IOC_RGA2:
                memset(&compat_req, 0x0, sizeof(compat_req));
                NormalRgaCompatModeConvertRga2(&compat_req, &req);

                ioc_req = &compat_req;
                break;
            case RGA_DRIVER_IOC_MULTI_RGA:
                ioc_req = &req;
                break;

            default:
                IM_LOGW("unknow driver[0x%x]\n", session->driver_type);
                ret = IM_STATUS_FAILED;
                goto release_resource;
        }

        do {
            ret = ioctl(session->rga_dev_fd, dstinfo.sync_mode, ioc_req);
        } while (ret == -1 && (errno == EINTR || errno == 512));   /* ERESTARTSYS is 512. */
        if (ret) {
            IM_LOGE("Failed to call RockChipRga interface, please use 'dmesg' command to view driver error log.");

            rga_dump_info(IM_LOG_ERROR | IM_LOG_FORCE,
                        job_handle, &src, &dst, &pat, &srect, &drect, &prect,
                        acquire_fence_fd, release_fence_fd, opt_ptr, usage);

            ret = IM_STATUS_FAILED;
            goto release_resource;
        }

        if (usage & IM_ASYNC) {
            *release_fence_fd = req.out_fence_fd;

            if (session->driver_feature & RGA_DRIVER_FEATURE_USER_CLOSE_FENCE &&
                acquire_fence_fd > 0)
                close(acquire_fence_fd);
        }
    }

    ret = IM_STATUS_SUCCESS;

release_resource:
    if (usage & IM_GAUSS && req.gauss_config.coe_ptr != 0)
        free(u64_to_ptr(req.gauss_config.coe_ptr));

    return (IM_STATUS)ret;
}

IM_STATUS rga_single_task_submit(rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                                 im_rect srect, im_rect drect, im_rect prect,
                                 int acquire_fence_fd, int *release_fence_fd,
                                 im_opt_t *opt_ptr, int usage) {
    return rga_task_submit(0, src, dst, pat, srect, drect, prect, acquire_fence_fd, release_fence_fd, opt_ptr, usage);
}

im_job_handle_t rga_job_create(uint32_t flags) {
    int ret;
    im_job_handle_t job_handle;
    im_rga_job_t *job = NULL;
    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    if (ioctl(session->rga_dev_fd, RGA_IOC_REQUEST_CREATE, &flags) < 0) {
        IM_LOGE(" %s(%d) request create fail: %s\n",__FUNCTION__, __LINE__,strerror(errno));
        return 0;
    }

    job_handle = flags;

    pthread_mutex_lock(&g_im2d_job_manager.mutex);

    job = rga_map_find_job(&g_im2d_job_manager.job_map, job_handle);
    if (job != NULL) {
        IM_LOGE("job_map error! handle[%d] already exists[%d]!\n",
                job_handle, job->task_count);
        ret = 0;
        goto error_cancel_job;
    }

    job = (im_rga_job_t *)malloc(sizeof(*job));
    if (job == NULL) {
        IM_LOGE("rga job alloc error!\n");
        ret = 0;
        goto error_cancel_job;
    }

    memset(job, 0x0, sizeof(*job));

    job->id = job_handle;
    rga_map_insert_job(&g_im2d_job_manager.job_map, job_handle, job);
    g_im2d_job_manager.job_count++;

    pthread_mutex_unlock(&g_im2d_job_manager.mutex);

    return job_handle;

error_cancel_job:
    pthread_mutex_unlock(&g_im2d_job_manager.mutex);
    rga_job_cancel(job_handle);

    return ret;
}

IM_STATUS rga_job_cancel(im_job_handle_t job_handle) {
    im_rga_job_t *job = NULL;
    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    pthread_mutex_lock(&g_im2d_job_manager.mutex);

    job = rga_map_find_job(&g_im2d_job_manager.job_map, job_handle);
    if (job != NULL) {
        rga_map_delete_job(&g_im2d_job_manager.job_map, job_handle);
        free(job);
    }

    g_im2d_job_manager.job_count--;

    pthread_mutex_unlock(&g_im2d_job_manager.mutex);

    if (ioctl(session->rga_dev_fd, RGA_IOC_REQUEST_CANCEL, &job_handle) < 0) {
        IM_LOGE(" %s(%d) request cancel fail: %s\n",__FUNCTION__, __LINE__,strerror(errno));
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_job_submit(im_job_handle_t job_handle, int sync_mode, int acquire_fence_fd, int *release_fence_fd) {
    int ret;
    im_rga_job_t *job = NULL;
    struct rga_user_request submit_request;
    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    memset(&submit_request, 0x0, sizeof(submit_request));

    switch (sync_mode) {
        case IM_SYNC:
            submit_request.sync_mode = RGA_BLIT_SYNC;
            break;
        case IM_ASYNC:
            submit_request.sync_mode = RGA_BLIT_ASYNC;
            break;
        default:
            IM_LOGE("illegal sync mode!\n");
            return IM_STATUS_ILLEGAL_PARAM;
    }

    pthread_mutex_lock(&g_im2d_job_manager.mutex);

    job = rga_map_find_job(&g_im2d_job_manager.job_map, job_handle);
    if (job == NULL) {
        IM_LOGE("%s job_handle[%d] is illegal!\n", __func__, job_handle);

        pthread_mutex_unlock(&g_im2d_job_manager.mutex);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    rga_map_delete_job(&g_im2d_job_manager.job_map, job_handle);
    g_im2d_job_manager.job_count--;

    pthread_mutex_unlock(&g_im2d_job_manager.mutex);

    submit_request.task_ptr = ptr_to_u64(job->req);
    submit_request.task_num = job->task_count;
    submit_request.id = job->id;
    submit_request.acquire_fence_fd = acquire_fence_fd;

    ret = ioctl(session->rga_dev_fd, RGA_IOC_REQUEST_SUBMIT, &submit_request);
    if (ret < 0) {
        IM_LOGE(" %s(%d) request submit fail: %s\n",__FUNCTION__, __LINE__,strerror(errno));
        ret = IM_STATUS_FAILED;
        goto free_job;
    } else {
        ret = IM_STATUS_SUCCESS;
    }

    if ((sync_mode == IM_ASYNC) && release_fence_fd)
        *release_fence_fd = submit_request.release_fence_fd;

free_job:
    free(job);

    return (IM_STATUS)ret;
}

IM_STATUS rga_job_config(im_job_handle_t job_handle, int sync_mode, int acquire_fence_fd, int *release_fence_fd) {
    int ret;
    im_rga_job_t *job = NULL;
    struct rga_user_request config_request;
    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    memset(&config_request, 0x0, sizeof(config_request));

    switch (sync_mode) {
        case IM_SYNC:
            config_request.sync_mode = RGA_BLIT_SYNC;
            break;
        case IM_ASYNC:
            config_request.sync_mode = RGA_BLIT_ASYNC;
            break;
        default:
            IM_LOGE("illegal sync mode!\n");
            return IM_STATUS_ILLEGAL_PARAM;
    }

    pthread_mutex_lock(&g_im2d_job_manager.mutex);

    job = rga_map_find_job(&g_im2d_job_manager.job_map, job_handle);
    if (job == NULL) {
        IM_LOGE("%s job_handle[%d] is illegal!\n", __func__, job_handle);

        pthread_mutex_unlock(&g_im2d_job_manager.mutex);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    config_request.task_ptr = ptr_to_u64(job->req);
    config_request.task_num = job->task_count;
    config_request.id = job->id;
    config_request.acquire_fence_fd = acquire_fence_fd;

    pthread_mutex_unlock(&g_im2d_job_manager.mutex);

    ret = ioctl(session->rga_dev_fd, RGA_IOC_REQUEST_CONFIG, &config_request);
    if (ret < 0) {
        IM_LOGE(" %s(%d) request config fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        return IM_STATUS_FAILED;
    } else {
        ret = IM_STATUS_SUCCESS;
    }

    if ((sync_mode == IM_ASYNC) && release_fence_fd)
        *release_fence_fd = config_request.release_fence_fd;

    return IM_STATUS_SUCCESS;
}

int generate_blit_req(struct rga_req *ioc_req, rga_info_t *src, rga_info_t *dst, rga_info_t *src1) {
    int srcVirW,srcVirH,srcActW,srcActH,srcXPos,srcYPos;
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int src1VirW,src1VirH,src1ActW,src1ActH,src1XPos,src1YPos;
    int rotateMode,orientation,ditherEn;
    int srcType,dstType,src1Type,srcMmuFlag,dstMmuFlag,src1MmuFlag;
    int fg_global_alpha, bg_global_alpha;
    int dstFd = -1;
    int srcFd = -1;
    int src1Fd = -1;
    int rotation;
    int stretch = 0;
    float hScale = 1;
    float vScale = 1;
    struct rga_interp interp;
    int ret = 0;
    rga_rect_t relSrcRect,tmpSrcRect,relDstRect,tmpDstRect;
    rga_rect_t relSrc1Rect,tmpSrc1Rect;
    unsigned int blend;
    unsigned int yuvToRgbMode;
    unsigned int r2y_mode = 0, y2r_mode = 0;
    bool perpixelAlpha = 0;
    void *srcBuf = NULL;
    void *dstBuf = NULL;
    void *src1Buf = NULL;
    RECT clip;
    int sync_mode = RGA_BLIT_SYNC;
    struct rga_req rgaReg;

    rga_session_t *session;
    rga_info_table_entry *rga_info;
    int ver_bicubic_limit;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    rga_info = &session->hardware_info;
    ver_bicubic_limit = rga_info->scale_ver_bicubic_limit;

    //init
    memset(&rgaReg, 0, sizeof(struct rga_req));
    if (session->driver_feature & RGA_DRIVER_FEATURE_USER_CLOSE_FENCE)
        rgaReg.feature.user_close_fence = true;

    srcType = dstType = srcMmuFlag = dstMmuFlag = 0;
    src1Type = src1MmuFlag = 0;
    rotation = 0;
    blend = 0;
    yuvToRgbMode = 0;

#if NORMAL_API_LOG_EN
    /* print debug log by setting property vendor.rga.log as 1 */
    is_debug_log();
    if(is_out_log())
        ALOGD("<<<<-------- print rgaLog -------->>>>");
#endif

    if (!src && !dst && !src1) {
        ALOGE("src = %p, dst = %p, src1 = %p", src, dst, src1);
        return -EINVAL;
    }

    if (!src && !dst) {
        ALOGE("src = %p, dst = %p", src, dst);
        return -EINVAL;
    }

    /*
     * 1.if src exist, get some parameter from src, such as rotatiom.
     * 2.if need to blend, need blend variable from src to decide how to blend.
     * 3.get effective area from src, if the area is empty, choose to get parameter from handle.
     * */
    if (src) {
        rotation = src->rotation;
        blend = src->blend;
        interp.horiz = src->scale_mode & 0xf;
        interp.verti = (src->scale_mode >> 4) & 0xf;
        memcpy(&relSrcRect, &src->rect, sizeof(rga_rect_t));
    }

    /* get effective area from dst and src1, if the area is empty, choose to get parameter from handle. */
    if (dst)
        memcpy(&relDstRect, &dst->rect, sizeof(rga_rect_t));
    if (src1)
        memcpy(&relSrc1Rect, &src1->rect, sizeof(rga_rect_t));

    srcFd = dstFd = src1Fd = -1;

#if NORMAL_API_LOG_EN
    if (is_out_log()) {
        ALOGD("src->hnd = 0x%lx , dst->hnd = 0x%lx , src1->hnd = 0x%lx\n",
            (unsigned long)src->hnd, (unsigned long)dst->hnd, (unsigned long)(src1 ? src1->hnd : 0));
        ALOGD("src: handle = %d, Fd = %.2d ,phyAddr = %p ,virAddr = %p\n", src->handle, src->fd, src->phyAddr, src->virAddr);
        if (src1)
            ALOGD("src1: handle = %d, Fd = %.2d , phyAddr = %p , virAddr = %p\n", src1->handle, src1->fd, src1->phyAddr, src1->virAddr);
        ALOGD("dst: handle = %d, Fd = %.2d ,phyAddr = %p ,virAddr = %p\n", dst->handle, dst->fd, dst->phyAddr, dst->virAddr);
    }
#endif

    if (src1) {
        if (src->handle > 0 && dst->handle > 0 && src1->handle > 0) {
            /* This will mark the use of handle */
            rgaReg.handle_flag |= 1;
        } else if ((src->handle > 0 || dst->handle > 0 || src1->handle > 0) &&
                   (src->handle <= 0 || dst->handle <= 0 || src1->handle <= 0)) {
            ALOGE("librga only supports the use of handles only or no handles, [src,src1,dst] = [%d, %d, %d]\n",
                  src->handle, src1->handle, dst->handle);
            return -EINVAL;
        }
    } else {
        if (src->handle > 0 && dst->handle > 0) {
            /* This will mark the use of handle */
            rgaReg.handle_flag |= 1;
        } else if ((src->handle > 0 || dst->handle > 0) &&
                   (src->handle <= 0 || dst->handle <= 0)) {
            ALOGE("librga only supports the use of handles only or no handles, [src,dst] = [%d, %d]\n",
                  src->handle, dst->handle);
            return -EINVAL;
        }
    }

    /*********** get src addr *************/
    if (src && src->handle) {
        /* In order to minimize changes, the handle here will reuse the variable of Fd. */
        srcFd = src->handle;
    } else if (src && src->phyAddr) {
        srcBuf = src->phyAddr;
    } else if (src && src->fd > 0) {
        srcFd = src->fd;
        src->mmuFlag = 1;
    } else if (src && src->virAddr) {
        srcBuf = src->virAddr;
        src->mmuFlag = 1;
    }
    /*
     * After getting the fd or virtual address through the handle,
     * set 'srcType' to 1, and at the end, and then judge
     * the 'srcType' at the end whether to enable mmu.
     */
#ifdef ANDROID
    else if (src && src->hnd) {
#ifndef RK3188
        /* RK3188 is special, cannot configure rga through fd. */
        RkRgaGetHandleFd(src->hnd, &srcFd);
#endif
#ifndef ANDROID_8
        if (srcFd < 0 || srcFd == 0) {
            RkRgaGetHandleMapAddress(src->hnd, &srcBuf);
        }
#endif
        if ((srcFd < 0 || srcFd == 0) && srcBuf == NULL) {
            ALOGE("src handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &src->hnd);
            printf("src handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &src->hnd);
            return ret;
        }
        else {
            srcType = 1;
        }
    }

    if (!isRectValid(relSrcRect)) {
        ret = NormalRgaGetRect(src->hnd, &tmpSrcRect);
        if (ret) {
            ALOGE("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &src->hnd);
            printf("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &src->hnd);
            return ret;
        }
        memcpy(&relSrcRect, &tmpSrcRect, sizeof(rga_rect_t));
    }
#endif
    if (srcFd == -1 && !srcBuf) {
        ALOGE("%d:src has not fd and address for render", __LINE__);
        return ret;
    }
    if (srcFd == 0 && !srcBuf) {
        ALOGE("srcFd is zero, now driver not support");
        return -EINVAL;
    }
    /* Old rga driver cannot support fd as zero. */
    if (srcFd == 0)
        srcFd = -1;

    /*********** get src1 addr *************/
    if (src1) {
        if (src1->handle) {
            /* In order to minimize changes, the handle here will reuse the variable of Fd. */
            src1Fd = src1->handle;
        } else if (src1->phyAddr) {
            src1Buf = src1->phyAddr;
        } else if (src1->fd > 0) {
            src1Fd = src1->fd;
            src1->mmuFlag = 1;
        } else if (src1->virAddr) {
            src1Buf = src1->virAddr;
            src1->mmuFlag = 1;
        }
        /*
         * After getting the fd or virtual address through the handle,
         * set 'src1Type' to 1, and at the end, and then judge
         * the 'src1Type' at the end whether to enable mmu.
         */
#ifdef ANDROID
        else if (src1->hnd) {
#ifndef RK3188
            /* RK3188 is special, cannot configure rga through fd. */
        RkRgaGetHandleFd(src1->hnd, &src1Fd);
#endif
#ifndef ANDROID_8
            if (src1Fd < 0 || src1Fd == 0) {
                RkRgaGetHandleMapAddress(src1->hnd, &src1Buf);
            }
#endif
            if ((src1Fd < 0 || src1Fd == 0) && src1Buf == NULL) {
                ALOGE("src1 handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &src1->hnd);
                printf("src1 handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &src1->hnd);
                return ret;
            }
            else {
                src1Type = 1;
            }
        }

        if (!isRectValid(relSrc1Rect)) {
            ret = NormalRgaGetRect(src1->hnd, &tmpSrc1Rect);
            if (ret) {
                ALOGE("src1 handleGetRect fail ,ret = %d,hnd=%p", ret, &src1->hnd);
                printf("src1 handleGetRect fail ,ret = %d,hnd=%p", ret, &src1->hnd);
                return ret;
            }
            memcpy(&relSrc1Rect, &tmpSrc1Rect, sizeof(rga_rect_t));
        }
#endif
        if (src1Fd == -1 && !src1Buf) {
            ALOGE("%d:src1 has not fd and address for render", __LINE__);
            return ret;
        }
        if (src1Fd == 0 && !src1Buf) {
            ALOGE("src1Fd is zero, now driver not support");
            return -EINVAL;
        }
        /* Old rga driver cannot support fd as zero. */
        if (src1Fd == 0)
            src1Fd = -1;
    }

    /*********** get dst addr *************/
    if (dst && dst->handle) {
        /* In order to minimize changes, the handle here will reuse the variable of Fd. */
        dstFd = dst->handle;
    } else if (dst && dst->phyAddr) {
        dstBuf = dst->phyAddr;
    } else if (dst && dst->fd > 0) {
        dstFd = dst->fd;
        dst->mmuFlag = 1;
    } else if (dst && dst->virAddr) {
        dstBuf = dst->virAddr;
        dst->mmuFlag = 1;
    }
    /*
     * After getting the fd or virtual address through the handle,
     * set 'dstType' to 1, and at the end, and then judge
     * the 'dstType' at the end whether to enable mmu.
     */
#ifdef ANDROID
    else if (dst && dst->hnd) {
#ifndef RK3188
        /* RK3188 is special, cannot configure rga through fd. */
        RkRgaGetHandleFd(dst->hnd, &dstFd);
#endif
#ifndef ANDROID_8
        if (dstFd < 0 || dstFd == 0) {
            RkRgaGetHandleMapAddress(dst->hnd, &dstBuf);
        }
#endif
        if ((dstFd < 0 || dstFd == 0) && dstBuf == NULL) {
            ALOGE("dst handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &dst->hnd);
            printf("dst handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &dst->hnd);
            return ret;
        }
        else {
            dstType = 1;
        }
    }

    if (!isRectValid(relDstRect)) {
        ret = NormalRgaGetRect(dst->hnd, &tmpDstRect);
        if (ret) {
            ALOGE("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &dst->hnd);
            printf("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &dst->hnd);
            return ret;
        }
        memcpy(&relDstRect, &tmpDstRect, sizeof(rga_rect_t));
    }
#endif

    if (dstFd == -1 && !dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }
    if (dstFd == 0 && !dstBuf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    }
    /* Old rga driver cannot support fd as zero. */
    if (dstFd == 0)
        dstFd = -1;

#if NORMAL_API_LOG_EN
    if(is_out_log()) {
        ALOGD("handle_flag: 0x%x\n", rgaReg.handle_flag);
        ALOGD("src: Fd/handle = %.2d , buf = %p, mmuFlag = %d, mmuType = %d\n", srcFd, srcBuf, src->mmuFlag, srcType);
        if (src1)
            ALOGD("src1: Fd/handle = %.2d , buf = %p, mmuFlag = %d, mmuType = %d\n", src1Fd, src1Buf, src1->mmuFlag, src1Type);
        ALOGD("dst: Fd/handle = %.2d , buf = %p, mmuFlag = %d, mmuType = %d\n", dstFd, dstBuf, dst->mmuFlag, dstType);
    }
#endif

    relSrcRect.format = RkRgaCompatibleFormat(relSrcRect.format);
    relDstRect.format = RkRgaCompatibleFormat(relDstRect.format);
    if (isRectValid(relSrc1Rect))
        relSrc1Rect.format = RkRgaCompatibleFormat(relSrc1Rect.format);

#ifdef RK3126C
    if ( (relSrcRect.width == relDstRect.width) && (relSrcRect.height == relDstRect.height ) &&
         (relSrcRect.width + 2*relSrcRect.xoffset == relSrcRect.wstride) &&
         (relSrcRect.height + 2*relSrcRect.yoffset == relSrcRect.hstride) &&
         (relSrcRect.format == HAL_PIXEL_FORMAT_YCrCb_NV12) && (relSrcRect.xoffset > 0 && relSrcRect.yoffset > 0)
       ) {
        relSrcRect.width += 4;
        //relSrcRect.height += 4;
        relSrcRect.xoffset = (relSrcRect.wstride - relSrcRect.width) / 2;
    }
#endif

    /* determined by format, need pixel alpha or not. */
    perpixelAlpha = NormalRgaFormatHasAlpha(RkRgaGetRgaFormat(relSrcRect.format));

#if NORMAL_API_LOG_EN
    if(is_out_log())
        ALOGE("blend = %x , perpixelAlpha = %d",blend,perpixelAlpha);
#endif

    if (blend & 0xfff) {
        /* blend bit[16:23] is to set global alpha. */
        fg_global_alpha = (blend >> 16) & 0xff;
        bg_global_alpha = (blend >> 24) & 0xff;

        /*
         * In the legacy interface, the src-over mode supports globalAlpha
         * configuration for the src channel, while the other modes do not
         * support globalAlpha configuration.
         */
        switch (blend & 0xfff) {
            case 0x405:
                fg_global_alpha = (blend >> 16) & 0xff;
                bg_global_alpha = 0xff;

                blend = RGA_ALPHA_BLEND_SRC_OVER;
                blend |= 0x1 << 12;
                break;
            case 0x504:
                fg_global_alpha = 0xff;
                bg_global_alpha = 0xff;

                blend = RGA_ALPHA_BLEND_DST_OVER;
                blend |= 0x1 << 12;
                break;
            case 0x105:
                fg_global_alpha = (blend >> 16) & 0xff;
                bg_global_alpha = 0xff;

                blend = RGA_ALPHA_BLEND_SRC_OVER;
                break;
            case 0x501:
                fg_global_alpha = 0xff;
                bg_global_alpha = 0xff;

                blend = RGA_ALPHA_BLEND_DST_OVER;
                break;
            case 0x100:
                fg_global_alpha = 0xff;
                bg_global_alpha = 0xff;

                blend = RGA_ALPHA_BLEND_SRC;
                break;
        }

        rgaReg.feature.global_alpha_en = true;
        NormalRgaSetAlphaEnInfo(&rgaReg, 1, 1, fg_global_alpha, bg_global_alpha , 1, blend & 0xfff, 0);

        /* need to pre-multiply. */
        if ((blend >> 12) & 0x1)
            rgaReg.alpha_rop_flag |= (1 << 9);
    }

    /* discripe a picture need high stride.If high stride not to be set, need use height as high stride. */
    if (relSrcRect.hstride == 0)
        relSrcRect.hstride = relSrcRect.height;

    if (isRectValid(relSrc1Rect))
        if (relSrc1Rect.hstride == 0)
            relSrc1Rect.hstride = relSrc1Rect.height;

    if (relDstRect.hstride == 0)
        relDstRect.hstride = relDstRect.height;

    /* do some check, check the area of src and dst whether is effective. */
    if (src) {
        ret = checkRectForRga(relSrcRect);
        if (ret) {
            printf("Error srcRect\n");
            ALOGE("[%s,%d]Error srcRect \n", __func__, __LINE__);
            return ret;
        }
    }

    if (src1) {
        ret = checkRectForRga(relSrc1Rect);
        if (ret) {
            printf("Error src1Rect\n");
            ALOGE("[%s,%d]Error src1Rect \n", __func__, __LINE__);
            return ret;
        }
    }

    if (dst) {
        ret = checkRectForRga(relDstRect);
        if (ret) {
            printf("Error dstRect\n");
            ALOGE("[%s,%d]Error dstRect \n", __func__, __LINE__);
            return ret;
        }
    }

    /* check the scale magnification. */
    if (src1 && src) {
        hScale = (float)relSrcRect.width / relSrc1Rect.width;
        vScale = (float)relSrcRect.height / relSrc1Rect.height;
        if (rotation == HAL_TRANSFORM_ROT_90 || rotation == HAL_TRANSFORM_ROT_270) {
            hScale = (float)relSrcRect.width / relSrc1Rect.height;
            vScale = (float)relSrcRect.height / relSrc1Rect.width;
        }
    } else if (src && dst) {
        hScale = (float)relSrcRect.width / relDstRect.width;
        vScale = (float)relSrcRect.height / relDstRect.height;
        if (rotation == HAL_TRANSFORM_ROT_90 || rotation == HAL_TRANSFORM_ROT_270) {
            hScale = (float)relSrcRect.width / relDstRect.height;
            vScale = (float)relSrcRect.height / relDstRect.width;
        }
    }
    // check scale limit form low to high version, gradually strict, avoid invalid jugdement
    if (session->driver_type == RGA_DRIVER_IOC_RGA1) {
        if (session->core_version.version[0].minor <= 0 && (hScale < 1/2 || vScale < 1/2)) {
            ALOGE("e scale[%f,%f] ver[%s]", hScale, vScale, session->core_version.version[0].str);
            return -EINVAL;
        }
        if (session->core_version.version[0].major <= 2 && (hScale < 1/8 ||
                                    hScale > 8 || vScale < 1/8 || vScale > 8)) {
            ALOGE("Error scale[%f,%f] line %d", hScale, vScale, __LINE__);
            return -EINVAL;
        }
    }
    if (hScale < 1/16 || hScale > 16 || vScale < 1/16 || vScale > 16) {
        ALOGE("Error scale[%f,%f] line %d", hScale, vScale, __LINE__);
        return -EINVAL;
    }


    /* reselect the scale mode. */
    stretch = (hScale != 1.0f) || (vScale != 1.0f);

    if (interp.horiz == RGA_INTERP_DEFAULT) {
        if (hScale > 1.0f)
            interp.horiz = RGA_INTERP_AVERAGE;
        else if (hScale < 1.0f)
            interp.horiz = RGA_INTERP_BICUBIC;
    }

    if (interp.verti == RGA_INTERP_DEFAULT) {
        if (vScale > 1.0f) {
            interp.verti = RGA_INTERP_AVERAGE;
        } else if (vScale < 1.0f) {
            if (relSrcRect.width > ver_bicubic_limit ||
                (relDstRect.width > ver_bicubic_limit && hScale > 1.0f))
                interp.verti = RGA_INTERP_LINEAR;
            else
                interp.verti = RGA_INTERP_BICUBIC;
        }
    }

    /* check interpoletion limit */
    if (interp.verti == RGA_INTERP_BICUBIC && vScale < 1.0f) {
        if (relSrcRect.width > ver_bicubic_limit ||
            (relDstRect.width > ver_bicubic_limit && hScale > 1.0f)) {
            ALOGE("when using bicubic scaling in the vertical direction, it does not support input width larger than %d.",
                  ver_bicubic_limit);
            return -EINVAL;
        }
    }

    if (((vScale > 1.0f && interp.verti == RGA_INTERP_LINEAR) ||
         (hScale > 1.0f && interp.horiz == RGA_INTERP_LINEAR)) &&
        (hScale < 1.0f || vScale < 1.0f)) {
            ALOGE("when using bilinear scaling for downsizing, it does not support scaling up in other directions.");
            return -EINVAL;
    }

    if ((vScale > 1.0f && interp.verti == RGA_INTERP_LINEAR) &&
        relDstRect.width > 4096) {
        ALOGE("bi-linear scale-down only supports vertical direction smaller than 4096.");
        return -EINVAL;
    }

#if NORMAL_API_LOG_EN
    if(is_out_log())
        ALOGD("interp[horiz,verti] = [0x%x, 0x%x] , stretch = 0x%x",
              interp.horiz, interp.verti, stretch);
#endif

    /*
     * according to the rotation to set corresponding parameter.It's diffrient from the opengl.
     * Following's config which use frequently
     * */
    switch (rotation & 0x0f) {
        case HAL_TRANSFORM_FLIP_H:
            orientation = 0;
            rotateMode = 2;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            src1VirW = relSrc1Rect.wstride;
            src1VirH = relSrc1Rect.hstride;
            src1XPos = relSrc1Rect.xoffset;
            src1YPos = relSrc1Rect.yoffset;
            src1ActW = relSrc1Rect.width;
            src1ActH = relSrc1Rect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.hstride;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
        case HAL_TRANSFORM_FLIP_V:
            orientation = 0;
            rotateMode = 3;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            src1VirW = relSrc1Rect.wstride;
            src1VirH = relSrc1Rect.hstride;
            src1XPos = relSrc1Rect.xoffset;
            src1YPos = relSrc1Rect.yoffset;
            src1ActW = relSrc1Rect.width;
            src1ActH = relSrc1Rect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.hstride;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
        case HAL_TRANSFORM_FLIP_H_V:
            orientation = 0;
            rotateMode = 4;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            src1VirW = relSrc1Rect.wstride;
            src1VirH = relSrc1Rect.hstride;
            src1XPos = relSrc1Rect.xoffset;
            src1YPos = relSrc1Rect.yoffset;
            src1ActW = relSrc1Rect.width;
            src1ActH = relSrc1Rect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.hstride;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
        case HAL_TRANSFORM_ROT_90:
            orientation = 90;
            rotateMode = 1;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            src1VirW = relSrc1Rect.wstride;
            src1VirH = relSrc1Rect.hstride;
            src1XPos = relSrc1Rect.xoffset;
            src1YPos = relSrc1Rect.yoffset;
            src1ActW = relSrc1Rect.height;
            src1ActH = relSrc1Rect.width;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.hstride;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.height;
            dstActH = relDstRect.width;
            break;
        case HAL_TRANSFORM_ROT_180:
            orientation = 180;
            rotateMode = 1;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            src1VirW = relSrc1Rect.wstride;
            src1VirH = relSrc1Rect.hstride;
            src1XPos = relSrc1Rect.xoffset;
            src1YPos = relSrc1Rect.yoffset;
            src1ActW = relSrc1Rect.width;
            src1ActH = relSrc1Rect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.hstride;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
        case HAL_TRANSFORM_ROT_270:
            orientation = 270;
            rotateMode = 1;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            src1VirW = relSrc1Rect.wstride;
            src1VirH = relSrc1Rect.hstride;
            src1XPos = relSrc1Rect.xoffset;
            src1YPos = relSrc1Rect.yoffset;
            src1ActW = relSrc1Rect.height;
            src1ActH = relSrc1Rect.width;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.hstride;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.height;
            dstActH = relDstRect.width;
            break;
        default:
            orientation = 0;
            rotateMode = stretch;
            srcVirW = relSrcRect.wstride;
            srcVirH = relSrcRect.hstride;
            srcXPos = relSrcRect.xoffset;
            srcYPos = relSrcRect.yoffset;
            srcActW = relSrcRect.width;
            srcActH = relSrcRect.height;

            src1VirW = relSrc1Rect.wstride;
            src1VirH = relSrc1Rect.hstride;
            src1XPos = relSrc1Rect.xoffset;
            src1YPos = relSrc1Rect.yoffset;
            src1ActW = relSrc1Rect.width;
            src1ActH = relSrc1Rect.height;

            dstVirW = relDstRect.wstride;
            dstVirH = relDstRect.hstride;
            dstXPos = relDstRect.xoffset;
            dstYPos = relDstRect.yoffset;
            dstActW = relDstRect.width;
            dstActH = relDstRect.height;
            break;
    }

    switch ((rotation & 0xF0) >> 4) {
        case HAL_TRANSFORM_FLIP_H :
            rotateMode |= (2 << 4);
            break;
        case HAL_TRANSFORM_FLIP_V :
            rotateMode |= (3 << 4);
            break;
        case HAL_TRANSFORM_FLIP_H_V:
            rotateMode |= (4 << 4);
            break;
    }

    /* if pictual out of range should be cliped. */
    clip.xmin = 0;
    clip.xmax = dstVirW - 1;
    clip.ymin = 0;
    clip.ymax = dstVirH - 1;

    if  (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
         (RkRgaGetRgaFormat(relSrcRect.format) != RK_FORMAT_RGB_565 ||
         RkRgaGetRgaFormat(relSrcRect.format) != RK_FORMAT_BGR_565) &&
         (RkRgaGetRgaFormat(relDstRect.format) == RK_FORMAT_RGB_565 ||
         RkRgaGetRgaFormat(relDstRect.format) == RK_FORMAT_BGR_565))
        ditherEn = 1;
    else
        ditherEn = 0;

#if 0
    /* YUV HDS or VDS enable */
    if (NormalRgaIsYuvFormat(relDstRect.format)) {
        rgaReg.uvhds_mode = 1;
        if ((relDstRect.format == RK_FORMAT_YCbCr_420_SP ||
             relDstRect.format == RK_FORMAT_YCrCb_420_SP) &&
            rotation == 0 && hScale == 1.0f && vScale == 1.0f) {
            /* YUV420SP only support vds when without rotation and scale. */
            rgaReg.uvvds_mode = 1;
        }
    }
#endif

#if NORMAL_API_LOG_EN
    if(is_out_log())
        ALOGE("ditherEn =%d ", ditherEn);
#endif

    /* only to configure the parameter by driver version, because rga driver has too many version. */
    switch (session->driver_type) {
        /* the version 1.005 is different to assign fd from version 2.0 and above */
        case RGA_DRIVER_IOC_RGA1:
            if (session->core_version.version[0].minor < 6) {
                srcMmuFlag = dstMmuFlag = src1MmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
                NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH,
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
                /* src1 */
                if (src1)
                    NormalRgaSetPatVirtualInfo(&rgaReg, (unsigned long)src1Buf,
                                            (unsigned long)src1Buf + src1VirW * src1VirH,
                                            (unsigned long)src1Buf + src1VirW * src1VirH * 5/4,
                                            src1VirW, src1VirH, &clip,
                                            RkRgaGetRgaFormat(relSrc1Rect.format),0);
                /*dst*/
                NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#else
                NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH,
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
                /* src1 */
                if (src1)
                    NormalRgaSetPatVirtualInfo(&rgaReg, (unsigned long)src1Buf,
                                            (unsigned int)src1Buf + src1VirW * src1VirH,
                                            (unsigned int)src1Buf + src1VirW * src1VirH * 5/4,
                                            src1VirW, src1VirH, &clip,
                                            RkRgaGetRgaFormat(relSrc1Rect.format),0);
                /*dst*/
                NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);

#endif
            } else {
                /*Src*/
                if (srcFd != -1) {
                    srcMmuFlag = srcType ? 1 : 0;
                    if (src && srcFd == src->fd)
                        srcMmuFlag = src->mmuFlag ? 1 : 0;
                    NormalRgaSetSrcVirtualInfo(&rgaReg, 0, 0, 0, srcVirW, srcVirH,
                                            RkRgaGetRgaFormat(relSrcRect.format),0);
                    NormalRgaSetFdsOffsets(&rgaReg, srcFd, 0, 0, 0);
                } else {
                    if (src && src->hnd)
                        srcMmuFlag = srcType ? 1 : 0;
                    if (src && srcBuf == src->virAddr)
                        srcMmuFlag = 1;
                    if (src && srcBuf == src->phyAddr)
                        srcMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
                    NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                            (unsigned long)srcBuf + srcVirW * srcVirH,
                                            (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                            srcVirW, srcVirH,
                                            RkRgaGetRgaFormat(relSrcRect.format),0);
#else
                    NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                            (unsigned int)srcBuf + srcVirW * srcVirH,
                                            (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                            srcVirW, srcVirH,
                                            RkRgaGetRgaFormat(relSrcRect.format),0);
#endif
                }
                /* src1 */
                if (src1) {
                    if (src1Fd != -1) {
                        src1MmuFlag = src1Type ? 1 : 0;
                        if (src1Fd == src1->fd)
                            src1MmuFlag = src1->mmuFlag ? 1 : 0;
                        NormalRgaSetPatVirtualInfo(&rgaReg, 0, 0, 0, src1VirW, src1VirH, &clip,
                                                RkRgaGetRgaFormat(relSrc1Rect.format),0);
                        /*src dst fd*/
                        NormalRgaSetFdsOffsets(&rgaReg, 0, src1Fd, 0, 0);
                    } else {
                        if (src1->hnd)
                            src1MmuFlag = src1Type ? 1 : 0;
                        if (src1Buf == src1->virAddr)
                            src1MmuFlag = 1;
                        if (src1Buf == src1->phyAddr)
                            src1MmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
                        NormalRgaSetPatVirtualInfo(&rgaReg, (unsigned long)src1Buf,
                                                (unsigned long)src1Buf + src1VirW * src1VirH,
                                                (unsigned long)src1Buf + src1VirW * src1VirH * 5/4,
                                                src1VirW, src1VirH, &clip,
                                                RkRgaGetRgaFormat(relSrc1Rect.format),0);
#else
                        NormalRgaSetPatVirtualInfo(&rgaReg, (unsigned int)src1Buf,
                                                (unsigned int)src1Buf + src1VirW * src1VirH,
                                                (unsigned int)src1Buf + src1VirW * src1VirH * 5/4,
                                                src1VirW, src1VirH, &clip,
                                                RkRgaGetRgaFormat(relSrc1Rect.format),0);
#endif
                    }
                }
                /*dst*/
                if (dstFd != -1) {
                    dstMmuFlag = dstType ? 1 : 0;
                    if (dst && dstFd == dst->fd)
                        dstMmuFlag = dst->mmuFlag ? 1 : 0;
                    NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
                    /*src dst fd*/
                    NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
                } else {
                    if (dst && dst->hnd)
                        dstMmuFlag = dstType ? 1 : 0;
                    if (dst && dstBuf == dst->virAddr)
                        dstMmuFlag = 1;
                    if (dst && dstBuf == dst->phyAddr)
                        dstMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
                    NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                            (unsigned long)dstBuf + dstVirW * dstVirH,
                                            (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                            dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
#else
                    NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                            (unsigned int)dstBuf + dstVirW * dstVirH,
                                            (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                            dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
#endif
                }
            }

            break;
        case RGA_DRIVER_IOC_RGA2:
        case RGA_DRIVER_IOC_MULTI_RGA:
        default:
            if (src && src->hnd)
                srcMmuFlag = srcType ? 1 : 0;
            if (src && srcBuf == src->virAddr)
                srcMmuFlag = 1;
            if (src && srcBuf == src->phyAddr)
                srcMmuFlag = 0;
            if (srcFd != -1)
                srcMmuFlag = srcType ? 1 : 0;
            if (src && srcFd == src->fd)
                srcMmuFlag = src->mmuFlag ? 1 : 0;

            if (src1) {
                if (src1->hnd)
                    src1MmuFlag = src1Type ? 1 : 0;
                if (src1Buf == src1->virAddr)
                    src1MmuFlag = 1;
                if (src1Buf == src1->phyAddr)
                    src1MmuFlag = 0;
                if (src1Fd != -1)
                    src1MmuFlag = src1Type ? 1 : 0;
                if (src1Fd == src1->fd)
                    src1MmuFlag = src1->mmuFlag ? 1 : 0;
            }

            if (dst && dst->hnd)
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstBuf == dst->virAddr)
                dstMmuFlag = 1;
            if (dst && dstBuf == dst->phyAddr)
                dstMmuFlag = 0;
            if (dstFd != -1)
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstFd == dst->fd)
                dstMmuFlag = dst->mmuFlag ? 1 : 0;

            NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                    (uintptr_t)srcBuf,
                                    (uintptr_t)srcBuf + srcVirW * srcVirH,
                                    srcVirW, srcVirH,
                                    RkRgaGetRgaFormat(relSrcRect.format),0);
            /* src1 */
            if (src1)
                NormalRgaSetPatVirtualInfo(&rgaReg, src1Fd != -1 ? src1Fd : 0,
                                        (uintptr_t)src1Buf,
                                        (uintptr_t)src1Buf + src1VirW * src1VirH,
                                        src1VirW, src1VirH, &clip,
                                        RkRgaGetRgaFormat(relSrc1Rect.format),0);
            /*dst*/
            NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                    (uintptr_t)dstBuf,
                                    (uintptr_t)dstBuf + dstVirW * dstVirH,
                                    dstVirW, dstVirH, &clip,
                                    RkRgaGetRgaFormat(relDstRect.format),0);

            break;
    }

    /* set effective area of src and dst. */
    NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);
    if (src1)
        NormalRgaSetPatActiveInfo(&rgaReg, src1ActW, src1ActH, src1XPos, src1YPos);

    /* generate default csc mode */
    if (src1) {
        /* special config for yuv + rgb => rgb */
        /* src0 y2r, src1 bupass, dst bupass */
        if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relSrc1Rect.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relDstRect.format)))
            y2r_mode = IM_YUV_TO_RGB_BT601_LIMIT;

        /* special config for yuv + rgba => yuv on src1 */
        /* src0 y2r, src1 bupass, dst y2r */
        if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relSrc1Rect.format)) &&
            NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relDstRect.format))) {
            y2r_mode = IM_YUV_TO_RGB_BT601_LIMIT;
            r2y_mode = IM_RGB_TO_YUV_BT601_LIMIT;
        }

        /* special config for rgb + rgb => yuv on dst */
        /* src0 bupass, src1 bupass, dst y2r */
        if (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relSrc1Rect.format)) &&
            NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relDstRect.format)))
            r2y_mode = IM_RGB_TO_YUV_BT601_LIMIT;
    } else {
        /* special config for yuv to rgb */
        if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
            NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relDstRect.format)))
            y2r_mode = IM_YUV_TO_RGB_BT601_LIMIT;

        /* special config for rgb to yuv */
        if (NormalRgaIsRgbFormat(RkRgaGetRgaFormat(relSrcRect.format)) &&
            NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relDstRect.format)))
            r2y_mode = IM_RGB_TO_YUV_BT601_LIMIT;
    }

    if (dst->color_space_mode & IM_YUV_TO_RGB_MASK)
        y2r_mode = dst->color_space_mode & IM_YUV_TO_RGB_MASK;
    if (dst->color_space_mode & IM_RGB_TO_YUV_MASK)
        r2y_mode = dst->color_space_mode & IM_RGB_TO_YUV_MASK;

    if (dst->color_space_mode & full_csc_mask) {
        ret = NormalRgaFullColorSpaceConvert(&rgaReg, dst->color_space_mode & full_csc_mask);
        if (ret < 0) {
            ALOGE("Not support full csc mode [%x]\n", dst->color_space_mode & full_csc_mask);
            return -EINVAL;
        }

        if ((dst->color_space_mode & full_csc_mask) == rgb2yuv_709_limit)
            r2y_mode = IM_RGB_TO_YUV_BT709_LIMIT;
    }

    yuvToRgbMode = r2y_mode | y2r_mode;

    /* mode
     * interp:set different algorithm to scale.
     * rotateMode:rotation mode
     * Orientation:rotation orientation
     * ditherEn:enable or not.
     * yuvToRgbMode:yuv to rgb, rgb to yuv , or others
     * */
    NormalRgaSetBitbltMode(&rgaReg, &interp, rotateMode, orientation,
                           ditherEn, 0, yuvToRgbMode);

    NormalRgaNNQuantizeMode(&rgaReg, dst);

    NormalRgaDitherMode(&rgaReg, dst, relDstRect.format);

    if (srcMmuFlag || dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
    }
    if (src1) {
        if (src1MmuFlag) {
            rgaReg.mmu_info.mmu_flag |= (0x1 << 11);
            rgaReg.mmu_info.mmu_flag |= (0x1 << 9);
        }
        /*enable src0 + src1 => dst*/
        rgaReg.bsfilter_flag = 1;
    }

    /* ROP */
    /* This special Interface can do some basic logical operations */
    if(src->rop_code > 0)
    {
        rgaReg.rop_code = src->rop_code;
        rgaReg.alpha_rop_flag = 0x3;
        rgaReg.alpha_rop_mode = 0x1;
    }

    /*color key*/
    /* if need this funtion, maybe should patch the rga driver. */
    if(src->colorkey_en == 1) {
        rgaReg.alpha_rop_flag |= (1 << 9);  //real color mode
        switch (src->colorkey_mode) {
            case 0 :
                NormalRgaSetSrcTransModeInfo(&rgaReg, 0, 1, 1, 1, 1, src->colorkey_min, src->colorkey_max, 1);
                break;
            case 1 :
                NormalRgaSetSrcTransModeInfo(&rgaReg, 1, 1, 1, 1, 1, src->colorkey_min, src->colorkey_max, 1);
                break;
        }
    }

    /* mosaic */
    memcpy(&rgaReg.mosaic_info, &src->mosaic_info, sizeof(struct rga_mosaic_info));

    /* gauss */
    memcpy(&rgaReg.gauss_config, &src->gauss_config, sizeof(rgaReg.gauss_config));
    if (rgaReg.gauss_config.size > 0) {
        rgaReg.feature.global_alpha_en = true;
        rgaReg.fg_global_alpha = (blend >> 16) & 0xff;
    }

    /* OSD */
    memcpy(&rgaReg.osd_info, &src->osd_info, sizeof(struct rga_osd_info));

    /* pre_intr */
    memcpy(&rgaReg.pre_intr_info, &src->pre_intr, sizeof(src->pre_intr));

#if NORMAL_API_LOG_EN
    if(is_out_log()) {
        ALOGD("srcMmuFlag = %d , dstMmuFlag = %d , rotateMode = %d \n", srcMmuFlag, dstMmuFlag,rotateMode);
        ALOGD("<<<<-------- rgaReg -------->>>>\n");
        NormalRgaLogOutRgaReq(rgaReg);
    }
#endif

    /* RGBA5551 alpha control */
    if (src->rgba5551_flags == 1) {
        rgaReg.rgba5551_alpha.flags = src->rgba5551_flags;
        rgaReg.rgba5551_alpha.alpha0 = src->rgba5551_alpha0;
        rgaReg.rgba5551_alpha.alpha1 = src->rgba5551_alpha1;
    }

    if(src->sync_mode == RGA_BLIT_ASYNC || dst->sync_mode == RGA_BLIT_ASYNC) {
        sync_mode = RGA_BLIT_ASYNC;
    }

    /* rga3 rd_mode */
    /* If rd_mode is not configured, raster mode is executed by default. */
    rgaReg.src.rd_mode = src->rd_mode ? src->rd_mode : raster_mode;
    rgaReg.dst.rd_mode = dst->rd_mode ? dst->rd_mode : raster_mode;
    if (src1)
        rgaReg.pat.rd_mode = src1->rd_mode ? src1->rd_mode : raster_mode;

    rgaReg.in_fence_fd = dst->in_fence_fd;
    rgaReg.core = dst->core;
    rgaReg.priority = dst->priority;

    memcpy(ioc_req, &rgaReg, sizeof(rgaReg));

    return 0;
}

int generate_fill_req(struct rga_req *ioc_req, rga_info_t *dst) {
    int dstVirW,dstVirH,dstActW,dstActH,dstXPos,dstYPos;
    int dstType,dstMmuFlag;
    int dstFd = -1;
    int ret = 0;
    unsigned int color = 0x00000000;
    rga_rect_t relDstRect,tmpDstRect;
    COLOR_FILL fillColor ;
    void *dstBuf = NULL;
    RECT clip;
    struct rga_req rgaReg;

    int sync_mode = RGA_BLIT_SYNC;

    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

#if NORMAL_API_LOG_EN
    /* print debug log by setting property vendor.rga.log as 1 */
    is_debug_log();
    if(is_out_log()) {
        ALOGD("<<<<-------- print rgaLog -------->>>>");
        ALOGD("dst->hnd = 0x%lx\n", (unsigned long)dst->hnd);
        ALOGD("dst: handle = %d, Fd = %.2d ,phyAddr = %p ,virAddr = %p\n", dst->handle, dst->fd, dst->phyAddr, dst->virAddr);
    }
#endif

    memset(&rgaReg, 0, sizeof(struct rga_req));
    rgaReg.feature.user_close_fence = true;

    dstType = dstMmuFlag = 0;

    if (!dst) {
        ALOGE("dst = %p", dst);
        return -EINVAL;
    }

    color = dst->color;
    memcpy(&relDstRect, &dst->rect, sizeof(rga_rect_t));

    if (relDstRect.hstride == 0)
        relDstRect.hstride = relDstRect.height;
#ifdef ANDROID
    if (dst->hnd) {
        ret = RkRgaGetHandleFd(dst->hnd, &dstFd);
        if (ret) {
            ALOGE("dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
            printf("-dst handle get fd fail ret = %d,hnd=%p", ret, &dst->hnd);
            return ret;
        }
        if (!isRectValid(relDstRect)) {
            ret = NormalRgaGetRect(dst->hnd, &tmpDstRect);
            if (ret)
                return ret;
            memcpy(&relDstRect, &tmpDstRect, sizeof(rga_rect_t));
        }
        NormalRgaGetMmuType(dst->hnd, &dstType);
    }
#endif

    if (dst->handle > 0) {
        dstFd = dst->handle;
        /* This will mark the use of handle */
        rgaReg.handle_flag |= 1;
    } else {
        dstFd = dst->fd;
    }

    if (dst->phyAddr)
        dstBuf = dst->phyAddr;
    else if (dst->virAddr)
        dstBuf = dst->virAddr;
#ifdef ANDROID
    else if (dst->hnd)
        ret = RkRgaGetHandleMapAddress(dst->hnd, &dstBuf);
#endif

    if (dstFd == -1 && !dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }

    if (dstFd == 0 && !dstBuf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    }

#if NORMAL_API_LOG_EN
    if(is_out_log()) {
        ALOGD("handle_flag: 0x%x\n", rgaReg.handle_flag);
        ALOGD("dst: Fd/handle = %.2d , buf = %p, mmuFlag = %d, mmuType = %d\n", dstFd, dstBuf, dst->mmuFlag, dstType);
    }
#endif

    relDstRect.format = RkRgaCompatibleFormat(relDstRect.format);

    if (dstFd == 0)
        dstFd = -1;

    if (relDstRect.hstride == 0)
        relDstRect.hstride = relDstRect.height;

    dstVirW = relDstRect.wstride;
    dstVirH = relDstRect.hstride;
    dstXPos = relDstRect.xoffset;
    dstYPos = relDstRect.yoffset;
    dstActW = relDstRect.width;
    dstActH = relDstRect.height;

    clip.xmin = 0;
    clip.xmax = dstActW - 1;
    clip.ymin = 0;
    clip.ymax = dstActH - 1;

    switch (session->driver_type) {
        case RGA_DRIVER_IOC_RGA1:
            if (session->core_version.version[0].minor < 6) {
#if defined(__arm64__) || defined(__aarch64__)
                /*dst*/
                NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#else
                /*dst*/
                NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
#endif
            } else {
                /*dst*/
                if (dstFd != -1) {
                    dstMmuFlag = dstType ? 1 : 0;
                    if (dst && dstFd == dst->fd)
                        dstMmuFlag = dst->mmuFlag ? 1 : 0;
                    NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
                    /*src dst fd*/
                    NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
                } else {
                    if (dst && dst->hnd)
                        dstMmuFlag = dstType ? 1 : 0;
                    if (dst && dstBuf == dst->virAddr)
                        dstMmuFlag = 1;
                    if (dst && dstBuf == dst->phyAddr)
                        dstMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
                    NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                            (unsigned long)dstBuf + dstVirW * dstVirH,
                                            (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                            dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
#else
                    NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                            (unsigned int)dstBuf + dstVirW * dstVirH,
                                            (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                            dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
        #endif
                }
            }

            break;
        case RGA_DRIVER_IOC_RGA2:
        case RGA_DRIVER_IOC_MULTI_RGA:
        default:
            if (dst && dst->hnd)
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstBuf == dst->virAddr)
                dstMmuFlag = 1;
            if (dst && dstBuf == dst->phyAddr)
                dstMmuFlag = 0;
            if (dstFd != -1)
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstFd == dst->fd)
                dstMmuFlag = dst->mmuFlag ? 1 : 0;
            /*dst*/
            NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                    (uintptr_t)dstBuf,
                                    (uintptr_t)dstBuf + dstVirW * dstVirH,
                                    dstVirW, dstVirH, &clip,
                                    RkRgaGetRgaFormat(relDstRect.format),0);

            break;
    }

    if (NormalRgaIsYuvFormat(RkRgaGetRgaFormat(relDstRect.format))) {
        rgaReg.yuv2rgb_mode |= 0x2 << 2;
    }

    if(dst->color_space_mode > 0)
        rgaReg.yuv2rgb_mode = dst->color_space_mode;

    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);

    memset(&fillColor, 0x0, sizeof(COLOR_FILL));

    /*mode*/
    NormalRgaSetColorFillMode(&rgaReg, &fillColor, 0, 0, color, 0, 0, 0, 0, 0);

    if (dstMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, dstMmuFlag, dstMmuFlag);
    }

#if NORMAL_API_LOG_EN
    if(is_out_log()) {
        ALOGD("dstMmuFlag = %d\n", dstMmuFlag);
        ALOGD("<<<<-------- rgaReg -------->>>>\n");
        NormalRgaLogOutRgaReq(rgaReg);
    }
#endif

    if(dst->sync_mode == RGA_BLIT_ASYNC) {
        sync_mode = dst->sync_mode;
    }

    /* rga3 rd_mode */
    /* If rd_mode is not configured, raster mode is executed by default. */
    rgaReg.dst.rd_mode = dst->rd_mode ? dst->rd_mode : raster_mode;

    rgaReg.in_fence_fd = dst->in_fence_fd;
    rgaReg.core = dst->core;
    rgaReg.priority = dst->priority;

    memcpy(ioc_req, &rgaReg, sizeof(rgaReg));

    return 0;
}

int generate_color_palette_req(struct rga_req *ioc_req, rga_info_t *src, rga_info_t *dst, rga_info_t *lut) {
    struct rga_req  Rga_Request;
    struct rga_req  Rga_Request2;
    int srcVirW ,srcVirH ,srcActW ,srcActH ,srcXPos ,srcYPos;
    int dstVirW ,dstVirH ,dstActW ,dstActH ,dstXPos ,dstYPos;
    int lutVirW ,lutVirH ,lutActW ,lutActH ,lutXPos ,lutYPos;
    int srcType ,dstType ,lutType ,srcMmuFlag ,dstMmuFlag, lutMmuFlag;
    int dstFd = -1;
    int srcFd = -1;
    int lutFd = -1;
    int ret = 0;
    rga_rect_t relSrcRect,tmpSrcRect,relDstRect,tmpDstRect, relLutRect, tmpLutRect;
    struct rga_req rgaReg;
    void *srcBuf = NULL;
    void *dstBuf = NULL;
    void *lutBuf = NULL;
    RECT clip;

    rga_session_t *session;

    session = get_rga_session();
    if (IS_ERR(session))
        return (IM_STATUS)PTR_ERR(session);

    //init
    memset(&rgaReg, 0, sizeof(struct rga_req));
    rgaReg.feature.user_close_fence = true;

    srcType = dstType = lutType = srcMmuFlag = dstMmuFlag = lutMmuFlag = 0;

#if NORMAL_API_LOG_EN
    /* print debug log by setting property vendor.rga.log as 1 */
    is_debug_log();
    if(is_out_log())
    ALOGD("<<<<-------- print rgaLog -------->>>>");
#endif

    if (!src && !dst) {
        ALOGE("src = %p, dst = %p, lut = %p", src, dst, lut);
        return -EINVAL;
    }

     /* get effective area from src、dst and lut, if the area is empty, choose to get parameter from handle. */
    if (src)
        memcpy(&relSrcRect, &src->rect, sizeof(rga_rect_t));
    if (dst)
        memcpy(&relDstRect, &dst->rect, sizeof(rga_rect_t));
    if (lut)
        memcpy(&relLutRect, &lut->rect, sizeof(rga_rect_t));

    srcFd = dstFd = lutFd = -1;

#if NORMAL_API_LOG_EN
    if(is_out_log()) {
        ALOGD("src->hnd = 0x%lx , dst->hnd = 0x%lx, lut->hnd = 0x%lx \n",
            (unsigned long)src->hnd, (unsigned long)dst->hnd, (unsigned long)lut->hnd);
        ALOGD("src: Fd = %.2d , phyAddr = %p , virAddr = %p\n",src->fd,src->phyAddr,src->virAddr);
        ALOGD("dst: Fd = %.2d , phyAddr = %p , virAddr = %p\n",dst->fd,dst->phyAddr,dst->virAddr);
        ALOGD("lut: Fd = %.2d , phyAddr = %p , virAddr = %p\n",lut->fd,lut->phyAddr,lut->virAddr);
    }
#endif

    if (lut) {
        if (src->handle <= 0 || dst->handle <= 0 || lut->handle <= 0) {
            ALOGE("librga only supports the use of handles only or no handles, [src,lut,dst] = [%d, %d, %d]\n",
                    src->handle, lut->handle, dst->handle);
            return -EINVAL;
        }

        /* This will mark the use of handle */
        rgaReg.handle_flag |= 1;
    } else if (src->handle > 0 && dst->handle > 0) {
        /* This will mark the use of handle */
        rgaReg.handle_flag |= 1;
    } else {
        ALOGE("librga only supports the use of handles only or no handles, [src,dst] = [%d, %d]\n",
                  src->handle, dst->handle);
        return -EINVAL;
    }

    /*********** get src addr *************/
    if (src && src->handle) {
        /* In order to minimize changes, the handle here will reuse the variable of Fd. */
        srcFd = src->handle;
    } else if (src && src->phyAddr) {
        srcBuf = src->phyAddr;
    } else if (src && src->fd > 0) {
        srcFd = src->fd;
        src->mmuFlag = 1;
    } else if (src && src->virAddr) {
        srcBuf = src->virAddr;
        src->mmuFlag = 1;
    }
#ifdef ANDROID
    else if (src && src->hnd) {
#ifndef RK3188
        /* RK3188 is special, cannot configure rga through fd. */
        RkRgaGetHandleFd(src->hnd, &srcFd);
#endif
#ifndef ANDROID_8
        if (srcFd < 0 || srcFd == 0) {
            RkRgaGetHandleMapAddress(src->hnd, &srcBuf);
        }
#endif
        if ((srcFd < 0 || srcFd == 0) && srcBuf == NULL) {
            ALOGE("src handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &src->hnd);
            printf("src handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &src->hnd);
            return ret;
        }
        else {
            srcType = 1;
        }
    }

    if (!isRectValid(relSrcRect)) {
        ret = NormalRgaGetRect(src->hnd, &tmpSrcRect);
        if (ret) {
            ALOGE("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &src->hnd);
            printf("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &src->hnd);
            return ret;
        }
        memcpy(&relSrcRect, &tmpSrcRect, sizeof(rga_rect_t));
    }
#endif

    if (srcFd == -1 && !srcBuf) {
        ALOGE("%d:src has not fd and address for render", __LINE__);
        return ret;
    }
    if (srcFd == 0 && !srcBuf) {
        ALOGE("srcFd is zero, now driver not support");
        return -EINVAL;
    }
    /* Old rga driver cannot support fd as zero. */
    if (srcFd == 0)
        srcFd = -1;

    /*********** get dst addr *************/
    if (dst && dst->handle) {
        /* In order to minimize changes, the handle here will reuse the variable of Fd. */
        dstFd = dst->handle;
    } else if (dst && dst->phyAddr) {
        dstBuf = dst->phyAddr;
    } else if (dst && dst->fd > 0) {
        dstFd = dst->fd;
        dst->mmuFlag = 1;
    } else if (dst && dst->virAddr) {
        dstBuf = dst->virAddr;
        dst->mmuFlag = 1;
    }
#ifdef ANDROID
    else if (dst && dst->hnd) {
#ifndef RK3188
        /* RK3188 is special, cannot configure rga through fd. */
        RkRgaGetHandleFd(dst->hnd, &dstFd);
#endif
#ifndef ANDROID_8
        if (dstFd < 0 || dstFd == 0) {
            RkRgaGetHandleMapAddress(dst->hnd, &dstBuf);
        }
#endif
        if ((dstFd < 0 || dstFd == 0) && dstBuf == NULL) {
            ALOGE("dst handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &dst->hnd);
            printf("dst handle get fd and vir_addr fail ret = %d,hnd=%p", ret, &dst->hnd);
            return ret;
        }
        else {
            dstType = 1;
        }
    }

    if (!isRectValid(relDstRect)) {
        ret = NormalRgaGetRect(dst->hnd, &tmpDstRect);
        if (ret) {
            ALOGE("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &dst->hnd);
            printf("dst handleGetRect fail ,ret = %d,hnd=%p", ret, &dst->hnd);
            return ret;
        }
        memcpy(&relDstRect, &tmpDstRect, sizeof(rga_rect_t));
    }
#endif

    if (dstFd == -1 && !dstBuf) {
        ALOGE("%d:dst has not fd and address for render", __LINE__);
        return ret;
    }
    if (dstFd == 0 && !dstBuf) {
        ALOGE("dstFd is zero, now driver not support");
        return -EINVAL;
    }
    /* Old rga driver cannot support fd as zero. */
    if (dstFd == 0)
        dstFd = -1;

    /*********** get lut addr *************/
    if (lut && lut->handle) {
        /* In order to minimize changes, the handle here will reuse the variable of Fd. */
        lutFd = lut->handle;
    } else if (lut && lut->phyAddr) {
        lutBuf = lut->phyAddr;
    } else if (lut && lut->fd > 0) {
        lutFd = lut->fd;
        lut->mmuFlag = 1;
    } else if (lut && lut->virAddr) {
        lutBuf = lut->virAddr;
        lut->mmuFlag = 1;
    }
#ifdef ANDROID
    else if (lut && lut->hnd) {
#ifndef RK3188
        /* RK3188 is special, cannot configure rga through fd. */
        RkRgaGetHandleFd(lut->hnd, &lutFd);
#endif
#ifndef ANDROID_8
        if (lutFd < 0 || lutFd == 0) {
            RkRgaGetHandleMapAddress(lut->hnd, &lutBuf);
        }
#endif
        if ((lutFd < 0 || lutFd == 0) && lutBuf == NULL) {
            ALOGE("No lut address,not using update palette table mode.\n");
            printf("No lut address,not using update palette table mode.\n");
        }
        else {
            lutType = 1;
        }

        ALOGD("lut->mmuFlag = %d", lut->mmuFlag);
    }

    if (!isRectValid(relLutRect)) {
        ret = NormalRgaGetRect(lut->hnd, &tmpLutRect);
        if (ret) {
            ALOGE("lut handleGetRect fail ,ret = %d,hnd=%p", ret, &lut->hnd);
            printf("lut handleGetRect fail ,ret = %d,hnd=%p", ret, &lut->hnd);
        }
        memcpy(&relLutRect, &tmpLutRect, sizeof(rga_rect_t));
    }
#endif

    /* Old rga driver cannot support fd as zero. */
    if (lutFd == 0)
        lutFd = -1;

#if NORMAL_API_LOG_EN
    if(is_out_log()) {
        ALOGD("src: Fd = %.2d , buf = %p, mmuFlag = %d, mmuType = %d\n", srcFd, srcBuf, src->mmuFlag, srcType);
        ALOGD("dst: Fd = %.2d , buf = %p, mmuFlag = %d, mmuType = %d\n", dstFd, dstBuf, dst->mmuFlag, dstType);
        ALOGD("lut: Fd = %.2d , buf = %p, mmuFlag = %d, mmuType = %d\n", lutFd, lutBuf, lut->mmuFlag, lutType);
    }
#endif

    relSrcRect.format = RkRgaCompatibleFormat(relSrcRect.format);
    relDstRect.format = RkRgaCompatibleFormat(relDstRect.format);
    relLutRect.format = RkRgaCompatibleFormat(relLutRect.format);

#ifdef RK3126C
    if ( (relSrcRect.width == relDstRect.width) && (relSrcRect.height == relDstRect.height ) &&
         (relSrcRect.width + 2*relSrcRect.xoffset == relSrcRect.wstride) &&
         (relSrcRect.height + 2*relSrcRect.yoffset == relSrcRect.hstride) &&
         (relSrcRect.format == HAL_PIXEL_FORMAT_YCrCb_NV12) && (relSrcRect.xoffset > 0 && relSrcRect.yoffset > 0)
       ) {
        relSrcRect.width += 4;
        //relSrcRect.height += 4;
        relSrcRect.xoffset = (relSrcRect.wstride - relSrcRect.width) / 2;
    }
#endif
    /* discripe a picture need high stride.If high stride not to be set, need use height as high stride. */
    if (relSrcRect.hstride == 0)
        relSrcRect.hstride = relSrcRect.height;

    if (relDstRect.hstride == 0)
        relDstRect.hstride = relDstRect.height;

    /* do some check, check the area of src and dst whether is effective. */
    if (src) {
        ret = checkRectForRga(relSrcRect);
        if (ret) {
            printf("Error srcRect\n");
            ALOGE("[%s,%d]Error srcRect \n", __func__, __LINE__);
            return ret;
        }
    }

    if (dst) {
        ret = checkRectForRga(relDstRect);
        if (ret) {
            printf("Error dstRect\n");
            ALOGE("[%s,%d]Error dstRect \n", __func__, __LINE__);
            return ret;
        }
    }

    srcVirW = relSrcRect.wstride;
    srcVirH = relSrcRect.hstride;
    srcXPos = relSrcRect.xoffset;
    srcYPos = relSrcRect.yoffset;
    srcActW = relSrcRect.width;
    srcActH = relSrcRect.height;

    dstVirW = relDstRect.wstride;
    dstVirH = relDstRect.hstride;
    dstXPos = relDstRect.xoffset;
    dstYPos = relDstRect.yoffset;
    dstActW = relDstRect.width;
    dstActH = relDstRect.height;

    lutVirW = relLutRect.wstride;
    lutVirH = relLutRect.hstride;
    lutXPos = relLutRect.xoffset;
    lutYPos = relLutRect.yoffset;
    lutActW = relLutRect.width;
    lutActH = relLutRect.height;

    /* if pictual out of range should be cliped. */
    clip.xmin = 0;
    clip.xmax = dstVirW - 1;
    clip.ymin = 0;
    clip.ymax = dstVirH - 1;

    switch (session->driver_type) {
        case RGA_DRIVER_IOC_RGA1:
            if (session->core_version.version[0].minor < 6) {
                srcMmuFlag = dstMmuFlag = lutMmuFlag = 1;

#if defined(__arm64__) || defined(__aarch64__)
                NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned long)srcBuf + srcVirW * srcVirH,
                                        (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
                /*dst*/
                NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned long)dstBuf + dstVirW * dstVirH,
                                        (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
                /*lut*/
                NormalRgaSetPatVirtualInfo(&rgaReg, (unsigned long)lutBuf,
                                        (unsigned long)lutBuf + lutVirW * lutVirH,
                                        (unsigned long)lutBuf + lutVirW * lutVirH * 5/4,
                                        lutVirW, lutVirH, &clip,
                                        RkRgaGetRgaFormat(relLutRect.format),0);
#else
                NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                        (unsigned int)srcBuf + srcVirW * srcVirH,
                                        (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                        srcVirW, srcVirH,
                                        RkRgaGetRgaFormat(relSrcRect.format),0);
                /*dst*/
                NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                        (unsigned int)dstBuf + dstVirW * dstVirH,
                                        (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                        dstVirW, dstVirH, &clip,
                                        RkRgaGetRgaFormat(relDstRect.format),0);
                /*lut*/
                NormalRgaSetPatVirtualInfo(&rgaReg, (unsigned long)lutBuf,
                                        (unsigned int)lutBuf + lutVirW * lutVirH,
                                        (unsigned int)lutBuf + lutVirW * lutVirH * 5/4,
                                        lutVirW, lutVirH, &clip,
                                        RkRgaGetRgaFormat(relLutRect.format),0);

#endif
            } else {
                /*Src*/
                if (srcFd != -1) {
                    srcMmuFlag = srcType ? 1 : 0;
                    if (src && srcFd == src->fd)
                        srcMmuFlag = src->mmuFlag ? 1 : 0;
                    NormalRgaSetSrcVirtualInfo(&rgaReg, 0, 0, 0, srcVirW, srcVirH,
                                            RkRgaGetRgaFormat(relSrcRect.format),0);
                    NormalRgaSetFdsOffsets(&rgaReg, srcFd, 0, 0, 0);
                } else {
                    if (src && src->hnd)
                        srcMmuFlag = srcType ? 1 : 0;
                    if (src && srcBuf == src->virAddr)
                        srcMmuFlag = 1;
                    if (src && srcBuf == src->phyAddr)
                        srcMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
                    NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned long)srcBuf,
                                            (unsigned long)srcBuf + srcVirW * srcVirH,
                                            (unsigned long)srcBuf + srcVirW * srcVirH * 5/4,
                                            srcVirW, srcVirH,
                                            RkRgaGetRgaFormat(relSrcRect.format),0);
#else
                    NormalRgaSetSrcVirtualInfo(&rgaReg, (unsigned int)srcBuf,
                                            (unsigned int)srcBuf + srcVirW * srcVirH,
                                            (unsigned int)srcBuf + srcVirW * srcVirH * 5/4,
                                            srcVirW, srcVirH,
                                            RkRgaGetRgaFormat(relSrcRect.format),0);
#endif
                }
                /*dst*/
                if (dstFd != -1) {
                    dstMmuFlag = dstType ? 1 : 0;
                    if (dst && dstFd == dst->fd)
                        dstMmuFlag = dst->mmuFlag ? 1 : 0;
                    NormalRgaSetDstVirtualInfo(&rgaReg, 0, 0, 0, dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
                    /*src dst fd*/
                    NormalRgaSetFdsOffsets(&rgaReg, 0, dstFd, 0, 0);
                } else {
                    if (dst && dst->hnd)
                        dstMmuFlag = dstType ? 1 : 0;
                    if (dst && dstBuf == dst->virAddr)
                        dstMmuFlag = 1;
                    if (dst && dstBuf == dst->phyAddr)
                        dstMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
                    NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned long)dstBuf,
                                            (unsigned long)dstBuf + dstVirW * dstVirH,
                                            (unsigned long)dstBuf + dstVirW * dstVirH * 5/4,
                                            dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
#else
                    NormalRgaSetDstVirtualInfo(&rgaReg, (unsigned int)dstBuf,
                                            (unsigned int)dstBuf + dstVirW * dstVirH,
                                            (unsigned int)dstBuf + dstVirW * dstVirH * 5/4,
                                            dstVirW, dstVirH, &clip,
                                            RkRgaGetRgaFormat(relDstRect.format),0);
#endif
                }
                /*lut*/
                if (lutFd != -1) {
                    lutMmuFlag = lutType ? 1 : 0;
                    if (lut && lutFd == lut->fd)
                        lutMmuFlag = lut->mmuFlag ? 1 : 0;
                    NormalRgaSetPatVirtualInfo(&rgaReg, 0, 0, 0, lutVirW, lutVirH, &clip,
                                            RkRgaGetRgaFormat(relLutRect.format),0);
                    /*lut fd*/
                    NormalRgaSetFdsOffsets(&rgaReg, 0, lutFd, 0, 0);
                } else {
                    if (lut && lut->hnd)
                        lutMmuFlag = lutType ? 1 : 0;
                    if (lut && lutBuf == lut->virAddr)
                        lutMmuFlag = 1;
                    if (lut && lutBuf == lut->phyAddr)
                        lutMmuFlag = 0;
#if defined(__arm64__) || defined(__aarch64__)
                    NormalRgaSetPatVirtualInfo(&rgaReg, (unsigned long)lutBuf,
                                            (unsigned long)lutBuf + lutVirW * lutVirH,
                                            (unsigned long)lutBuf + lutVirW * lutVirH * 5/4,
                                            lutVirW, lutVirH, &clip,
                                            RkRgaGetRgaFormat(relLutRect.format),0);
#else
                    NormalRgaSetPatVirtualInfo(&rgaReg, (unsigned int)lutBuf,
                                            (unsigned int)lutBuf + lutVirW * lutVirH,
                                            (unsigned int)lutBuf + lutVirW * lutVirH * 5/4,
                                            lutVirW, lutVirH, &clip,
                                            RkRgaGetRgaFormat(relLutRect.format),0);
#endif
                }
            }

            break;
        case RGA_DRIVER_IOC_RGA2:
        case RGA_DRIVER_IOC_MULTI_RGA:
        default:
            if (src && src->hnd)
                srcMmuFlag = srcType ? 1 : 0;
            if (src && srcBuf == src->virAddr)
                srcMmuFlag = 1;
            if (src && srcBuf == src->phyAddr)
                srcMmuFlag = 0;
            if (srcFd != -1)
                srcMmuFlag = srcType ? 1 : 0;
            if (src && srcFd == src->fd)
                srcMmuFlag = src->mmuFlag ? 1 : 0;

            if (dst && dst->hnd)
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstBuf == dst->virAddr)
                dstMmuFlag = 1;
            if (dst && dstBuf == dst->phyAddr)
                dstMmuFlag = 0;
            if (dstFd != -1)
                dstMmuFlag = dstType ? 1 : 0;
            if (dst && dstFd == dst->fd)
                dstMmuFlag = dst->mmuFlag ? 1 : 0;

            if (lut && lut->hnd)
                lutMmuFlag = lutType ? 1 : 0;
            if (lut && lutBuf == lut->virAddr)
                lutMmuFlag = 1;
            if (lut && lutBuf == lut->phyAddr)
                lutMmuFlag = 0;
            if (lutFd != -1)
                lutMmuFlag = lutType ? 1 : 0;
            if (lut && lutFd == lut->fd)
                lutMmuFlag = lut->mmuFlag ? 1 : 0;

    #if defined(__arm64__) || defined(__aarch64__)
            NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                    (unsigned long)srcBuf,
                                    (unsigned long)srcBuf + srcVirW * srcVirH,
                                    srcVirW, srcVirH,
                                    RkRgaGetRgaFormat(relSrcRect.format),0);
            /*dst*/
            NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                    (unsigned long)dstBuf,
                                    (unsigned long)dstBuf + dstVirW * dstVirH,
                                    dstVirW, dstVirH, &clip,
                                    RkRgaGetRgaFormat(relDstRect.format),0);

            /*lut*/
            NormalRgaSetPatVirtualInfo(&rgaReg, lutFd != -1 ? lutFd : 0,
                                    (unsigned long)lutBuf,
                                    (unsigned long)lutBuf + lutVirW * lutVirH,
                                    lutVirW, lutVirH, &clip,
                                    RkRgaGetRgaFormat(relLutRect.format),0);
    #else
            NormalRgaSetSrcVirtualInfo(&rgaReg, srcFd != -1 ? srcFd : 0,
                                    (unsigned int)srcBuf,
                                    (unsigned int)srcBuf + srcVirW * srcVirH,
                                    srcVirW, srcVirH,
                                    RkRgaGetRgaFormat(relSrcRect.format),0);
            /*dst*/
            NormalRgaSetDstVirtualInfo(&rgaReg, dstFd != -1 ? dstFd : 0,
                                    (unsigned int)dstBuf,
                                    (unsigned int)dstBuf + dstVirW * dstVirH,
                                    dstVirW, dstVirH, &clip,
                                    RkRgaGetRgaFormat(relDstRect.format),0);
            /*lut*/
            NormalRgaSetPatVirtualInfo(&rgaReg, lutFd != -1 ? lutFd : 0,
                                    (unsigned int)lutBuf,
                                    (unsigned int)lutBuf + lutVirW * lutVirH,
                                    lutVirW, lutVirH, &clip,
                                    RkRgaGetRgaFormat(relLutRect.format),0);

    #endif
            break;
    }

    /* set effective area of src and dst. */
    NormalRgaSetSrcActiveInfo(&rgaReg, srcActW, srcActH, srcXPos, srcYPos);
    NormalRgaSetDstActiveInfo(&rgaReg, dstActW, dstActH, dstXPos, dstYPos);
    NormalRgaSetPatActiveInfo(&rgaReg, lutActW, lutActH, lutXPos, lutYPos);

    if (srcMmuFlag || dstMmuFlag || lutMmuFlag) {
        NormalRgaMmuInfo(&rgaReg, 1, 0, 0, 0, 0, 2);
        NormalRgaMmuFlag(&rgaReg, srcMmuFlag, dstMmuFlag);
        /*set lut mmu_flag*/
        if (lutMmuFlag) {
            rgaReg.mmu_info.mmu_flag |= (0x1 << 11);
            rgaReg.mmu_info.mmu_flag |= (0x1 << 9);
        }

    }

#if NORMAL_API_LOG_EN
    if(is_out_log()) {
        ALOGD("srcMmuFlag = %d , dstMmuFlag = %d , lutMmuFlag = %d\n", srcMmuFlag, dstMmuFlag, lutMmuFlag);
        ALOGD("<<<<-------- rgaReg -------->>>>\n");
        NormalRgaLogOutRgaReq(rgaReg);
    }
#endif

    switch (RkRgaGetRgaFormat(relSrcRect.format)) {
        case RK_FORMAT_BPP1 :
            rgaReg.palette_mode = 0;
            break;
        case RK_FORMAT_BPP2 :
            rgaReg.palette_mode = 1;
            break;
        case RK_FORMAT_BPP4 :
            rgaReg.palette_mode = 2;
            break;
        case RK_FORMAT_BPP8 :
        case RK_FORMAT_YCbCr_400 :
            rgaReg.palette_mode = 3;
            break;
    }

    /* rga3 rd_mode */
    /* If rd_mode is not configured, raster mode is executed by default. */
    rgaReg.src.rd_mode = src->rd_mode ? src->rd_mode : raster_mode;
    rgaReg.dst.rd_mode = dst->rd_mode ? dst->rd_mode : raster_mode;
    if (lut)
        rgaReg.pat.rd_mode = lut->rd_mode ? lut->rd_mode : raster_mode;

    rgaReg.in_fence_fd = dst->in_fence_fd;
    rgaReg.core = dst->core;
    rgaReg.priority = dst->priority;

    if (!(lutFd == -1 && lutBuf == NULL)) {
        rgaReg.fading.g = 0xff;
        rgaReg.render_mode = update_palette_table_mode;

        if(ioctl(session->rga_dev_fd, RGA_BLIT_SYNC, &rgaReg) != 0) {
            printf("update palette table mode ioctl err\n");
            return -1;
        }
    }

    rgaReg.render_mode = color_palette_mode;
    rgaReg.endian_mode = 1;

    memcpy(ioc_req, &rgaReg, sizeof(rgaReg));

    return 0;
}

