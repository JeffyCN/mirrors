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
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/ioctl.h>

#include "im2d.h"
#include "im2d_impl.h"
#include "im2d_log.h"
#include "im2d_hardware.h"

#include "RockchipRga.h"
#include "core/NormalRga.h"
#include "RgaUtils.h"
#include "utils.h"

#ifdef ANDROID
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

extern struct rgaContext *rgaCtx;

struct im2d_job_manager g_im2d_job_manager;
__thread im_context_t g_im2d_context;

IM_API static IM_STATUS rga_get_context(void) {
    if (rgaCtx == NULL) {
        RockchipRga& rkRga(RockchipRga::get());
        if (rgaCtx == NULL) {
            IM_LOGE("The current RockchipRga singleton is destroyed. "
                    "Please check if RkRgaInit/RkRgaDeInit are called, if so, please disable them.");
            return IM_STATUS_FAILED;
        }
    }

    return IM_STATUS_SUCCESS;
}

static IM_STATUS rga_support_info_merge_table(rga_info_table_entry *dst_table, rga_info_table_entry *merge_table) {
    if (dst_table == NULL || merge_table == NULL) {
        IM_LOGE("%s[%d] dst or merge table is NULL!\n", __FUNCTION__, __LINE__);
        return IM_STATUS_FAILED;
    }

    dst_table->version              |= merge_table->version;
    dst_table->input_format         |= merge_table->input_format;
    dst_table->output_format        |= merge_table->output_format;
    dst_table->feature              |= merge_table->feature;

    dst_table->input_resolution     = MAX(dst_table->input_resolution, merge_table->input_resolution);
    dst_table->output_resolution    = MAX(dst_table->output_resolution, merge_table->output_resolution);
    dst_table->byte_stride          = MAX(dst_table->byte_stride, merge_table->byte_stride);
    dst_table->scale_limit          = MAX(dst_table->scale_limit, merge_table->scale_limit);
    dst_table->performance          = MAX(dst_table->performance, merge_table->performance);

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
int rga_version_table_get_current_index(rga_version_t &version, const rga_version_bind_table_entry_t *table, int table_size) {
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
int rga_version_table_get_minimum_index(rga_version_t &version, const rga_version_bind_table_entry_t *table, int table_size) {
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
int rga_version_table_check_minimum_range(rga_version_t &version,
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

static IM_STATUS rga_version_get_current_index_failed_default(rga_version_t &current, rga_version_t &minimum) {
    UNUSED(current);
    UNUSED(minimum);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_minimum_index_failed_default(rga_version_t &current, rga_version_t &minimum) {
    UNUSED(current);
    UNUSED(minimum);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_witnin_minimun_range_default(rga_version_t &current, rga_version_t &minimum) {
    UNUSED(current);
    UNUSED(minimum);

    return IM_STATUS_SUCCESS;
}

static IM_STATUS rga_version_above_minimun_range_default(rga_version_t &current, rga_version_t &minimum, const rga_version_bind_table_entry_t *least_version_table) {
    UNUSED(current);
    UNUSED(minimum);
    UNUSED(least_version_table);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_below_minimun_range_default(rga_version_t &current, rga_version_t &minimum, const rga_version_bind_table_entry_t *least_version_table) {
    UNUSED(current);
    UNUSED(minimum);
    UNUSED(least_version_table);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_current_index_failed_user_header(rga_version_t &user_version, rga_version_t &header_version) {
    IM_LOGE("Failed to get the version binding table of librga, "
            "current version: librga: %s, header: %s",
            user_version.str, header_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_minimum_index_failed_user_header(rga_version_t &user_version, rga_version_t &header_version) {
    IM_LOGE("Failed to get the version binding table of header file, "
            "current version: librga: %s, header: %s",
            user_version.str, header_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_above_minimun_range_user_header(rga_version_t &user_version, rga_version_t &header_version, const rga_version_bind_table_entry_t *least_version_table) {
    IM_LOGE("The current referenced header_version is %s, but the running librga's version(%s) is too old, "
            "The librga must to be updated to version %s at least. "
            "You can try to update the SDK or update librga.so and header files "
            "through github(https://github.com/airockchip/librga/). ",
            header_version.str, user_version.str,
            least_version_table->current.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_below_minimun_range_user_header(rga_version_t &user_version, rga_version_t &header_version, const rga_version_bind_table_entry_t *least_version_table) {
    IM_LOGE("The current librga.so's verison is %s, but the referenced header_version(%s) is too old, "
            "it is recommended to update the librga's header_version to %s and above."
            "You can try to update the SDK or update librga.so and header files "
            "through github(https://github.com/airockchip/librga/)",
            user_version.str, header_version.str,
            least_version_table->minimum.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_current_index_faile_user_driver(rga_version_t &user_version, rga_version_t &driver_version) {
    IM_LOGE("Failed to get the version binding table of librga, "
            "current version: librga: %s, driver: %s",
            user_version.str, driver_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_get_minimum_index_failed_user_driver(rga_version_t &user_version, rga_version_t &driver_version) {
    IM_LOGE("Failed to get the version binding table of rga_driver, "
            "current version: librga: %s, driver: %s",
            user_version.str, driver_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_above_minimun_range_user_driver(rga_version_t &user_version, rga_version_t &driver_version, const rga_version_bind_table_entry_t *least_version_table) {
    IM_LOGE("The librga must to be updated to version %s at least. "
            "You can try to update the SDK or update librga.so and header files "
            "through github(https://github.com/airockchip/librga/). "
            "current version: librga %s, driver %s.",
            least_version_table->current.str,
            user_version.str, driver_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static IM_STATUS rga_version_below_minimun_range_user_driver(rga_version_t &user_version, rga_version_t &driver_version, const rga_version_bind_table_entry_t *least_version_table) {
    IM_LOGE("The driver may be compatible, "
            "but it is best to update the driver to version %s. "
            "You can try to update the SDK or update the "
            "<SDK>/kernel/drivers/video/rockchip/rga3 directory individually. "
            "current version: librga %s, driver %s.",
            least_version_table->minimum.str,
            user_version.str, driver_version.str);

    return IM_STATUS_ERROR_VERSION;
}

static const rga_version_check_ops_t rga_version_check_user_header_ops {
    .get_current_index_failed = rga_version_get_current_index_failed_user_header,
    .get_minimum_index_failed = rga_version_get_minimum_index_failed_user_header,
    .witnin_minimun_range = rga_version_witnin_minimun_range_default,
    .above_minimun_range = rga_version_above_minimun_range_user_header,
    .below_minimun_range = rga_version_below_minimun_range_user_header,
};

static const rga_version_check_ops_t rga_version_check_user_driver_ops {
    .get_current_index_failed = rga_version_get_current_index_faile_user_driver,
    .get_minimum_index_failed = rga_version_get_minimum_index_failed_user_driver,
    .witnin_minimun_range = rga_version_witnin_minimun_range_default,
    .above_minimun_range = rga_version_above_minimun_range_user_driver,
    .below_minimun_range = rga_version_below_minimun_range_user_driver,
};

static int rga_version_check(rga_version_t &current_version, rga_version_t &minimum_version,
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

IM_STATUS rga_set_buffer_info(rga_buffer_t dst, rga_info_t* dstinfo) {
    if(NULL == dstinfo) {
        IM_LOGE("Invaild dstinfo, dst structure address is NULL!");
        return IM_STATUS_INVALID_PARAM;
    }

    if (dst.handle > 0) {
        dstinfo->handle = dst.handle;
    } else if(dst.phy_addr != NULL) {
        dstinfo->phyAddr= dst.phy_addr;
    } else if(dst.fd > 0) {
        dstinfo->fd = dst.fd;
        dstinfo->mmuFlag = 1;
    } else if(dst.vir_addr != NULL) {
        dstinfo->virAddr = dst.vir_addr;
        dstinfo->mmuFlag = 1;
    } else {
        IM_LOGE("Invaild dst buffer, no address available in dst buffer, phy_addr = %ld, fd = %d, vir_addr = %ld, handle = %d",
                (unsigned long)dst.phy_addr, dst.fd, (unsigned long)dst.vir_addr, dst.handle);
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_set_buffer_info(const rga_buffer_t src, rga_buffer_t dst, rga_info_t* srcinfo, rga_info_t* dstinfo) {
    if(NULL == srcinfo) {
        IM_LOGE("Invaild srcinfo, src structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }
    if(NULL == dstinfo) {
        IM_LOGE("Invaild dstinfo, dst structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }

    if (src.handle > 0) {
        srcinfo->handle = src.handle;
    } else if(src.phy_addr != NULL) {
        srcinfo->phyAddr = src.phy_addr;
    } else if(src.fd > 0) {
        srcinfo->fd = src.fd;
        srcinfo->mmuFlag = 1;
    } else if(src.vir_addr != NULL) {
        srcinfo->virAddr = src.vir_addr;
        srcinfo->mmuFlag = 1;
    } else {
        IM_LOGE("Invaild src buffer, no address available in src buffer, phy_addr = %ld, fd = %d, vir_addr = %ld, handle = %d",
                (unsigned long)src.phy_addr, src.fd, (unsigned long)src.vir_addr, src.handle);
        return IM_STATUS_INVALID_PARAM;
    }

    if (dst.handle > 0) {
        dstinfo->handle = dst.handle;
    } else if(dst.phy_addr != NULL) {
        dstinfo->phyAddr= dst.phy_addr;
    } else if(dst.fd > 0) {
        dstinfo->fd = dst.fd;
        dstinfo->mmuFlag = 1;
    } else if(dst.vir_addr != NULL) {
        dstinfo->virAddr = dst.vir_addr;
        dstinfo->mmuFlag = 1;
    } else {
        IM_LOGE("Invaild dst buffer, no address available in dst buffer, phy_addr = %ld, fd = %d, vir_addr = %ld, handle = %d",
                (unsigned long)dst.phy_addr, dst.fd, (unsigned long)dst.vir_addr, dst.handle);
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_get_info(rga_info_table_entry *return_table) {
    int ret;
    int  rga_version = 0;
    rga_info_table_entry merge_table;

    ret = rga_get_context();
    if (ret != IM_STATUS_SUCCESS)
        return (IM_STATUS)ret;

    memset(&merge_table, 0x0, sizeof(merge_table));

    for (uint32_t i = 0; i < rgaCtx->mHwVersions.size; i++) {
        if (rgaCtx->mHwVersions.version[i].major == 2 &&
            rgaCtx->mHwVersions.version[i].minor == 0) {
            if (rgaCtx->mHwVersions.version[i].revision == 0) {
                rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
                memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
            } else {
                goto TRY_TO_COMPATIBLE;
            }
        } else if (rgaCtx->mHwVersions.version[i].major == 3 &&
                   rgaCtx->mHwVersions.version[i].minor == 0) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x16445 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                case 0x22245 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                case 0x76831 :
                    rga_version = IM_RGA_HW_VERSION_RGA_3_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (rgaCtx->mHwVersions.version[i].major == 3 &&
                   rgaCtx->mHwVersions.version[i].minor == 2) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x18218 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_ROP;
                    break;
                case 0x56726 :
                case 0x63318 :
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
        } else if (rgaCtx->mHwVersions.version[i].major == 3 &&
                   rgaCtx->mHwVersions.version[i].minor == 3) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x87975:
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
        } else if (rgaCtx->mHwVersions.version[i].major == 3 &&
                   rgaCtx->mHwVersions.version[i].minor == 7) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x93215:
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
        } else if (rgaCtx->mHwVersions.version[i].major == 4 &&
                   rgaCtx->mHwVersions.version[i].minor == 0) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x18632 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_LITE0_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                case 0x23998 :
                case 0x28610 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));

                    merge_table.feature |= IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC;
                    break;
                default :
                    goto TRY_TO_COMPATIBLE;
            }
        } else if (rgaCtx->mHwVersions.version[i].major == 42 &&
                   rgaCtx->mHwVersions.version[i].minor == 0) {
            if (rgaCtx->mHwVersions.version[i].revision == 0x17760) {
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
    if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "1.3", 3) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_1_INDEX;
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "1.6", 3) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_1_PLUS_INDEX;
    /*3288 vesion is 2.00*/
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "2.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
    /*3288w version is 3.00*/
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "3.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_INDEX;
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "3.02", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_ENHANCE_INDEX;
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "4.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_LITE0_INDEX;
    /*The version number of lite1 cannot be obtained temporarily.*/
    else if (strncmp((char *)rgaCtx->mHwVersions.version[0].str, "4.00", 4) == 0)
        rga_version = IM_RGA_HW_VERSION_RGA_2_LITE1_INDEX;
    else
        rga_version = IM_RGA_HW_VERSION_RGA_V_ERR_INDEX;

    memcpy(return_table, &hw_info_table[rga_version], sizeof(rga_info_table_entry));

    if (rga_version == IM_RGA_HW_VERSION_RGA_V_ERR_INDEX) {
        IM_LOGE("Can not get the correct RGA version, please check the driver, version=%s\n",
                rgaCtx->mHwVersions.version[0].str);
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_check_header(rga_version_t header_version) {
    int ret;
    int table_size = sizeof(user_header_bind_table) / sizeof(rga_version_bind_table_entry_t);
    rga_version_t user_version = RGA_SET_CURRENT_API_VERSION;

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

IM_STATUS rga_check_driver(rga_version_t &driver_version) {
    int ret;
    int table_size = sizeof(user_driver_bind_table) / sizeof(rga_version_bind_table_entry_t);
    rga_version_t user_version = RGA_SET_CURRENT_API_VERSION;

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

IM_STATUS rga_check_info(const char *name, const rga_buffer_t info, const im_rect rect, int resolution_usage) {
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
    if (rect.width < 0 || rect.height < 0 || rect.x < 0 || rect.y < 0) {
        IM_LOGW("Illegal %s rect, the parameter cannot be negative, rect[x,y,w,h] = [%d, %d, %d, %d]",
                name, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if ((rect.width > 0  && rect.width < 2) || (rect.height > 0 && rect.height < 2) ||
        (rect.x > 0 && rect.x < 2)          || (rect.y > 0 && rect.y < 2)) {
        IM_LOGW("Hardware limitation %s rect, unsupported operation of images smaller than 2 pixels, "
                "rect[x,y,w,h] = [%d, %d, %d, %d]",
                name, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_INVALID_PARAM;
    }

    if ((rect.width + rect.x > info.wstride) || (rect.height + rect.y > info.hstride)) {
        IM_LOGW("Invaild %s rect, the sum of width and height of rect needs to be less than wstride or hstride, "
                "rect[x,y,w,h] = [%d, %d, %d, %d], wstride = %d, hstride = %d",
                name, rect.x, rect.y, rect.width, rect.height, info.wstride, info.hstride);
        return IM_STATUS_INVALID_PARAM;
    }

    /**************** resolution check ****************/
    if (info.width > resolution_usage ||
        info.height > resolution_usage) {
        IM_LOGW("Unsupported %s to input resolution more than %d, width = %d, height = %d",
                name, resolution_usage, info.width, info.height);
        return IM_STATUS_NOT_SUPPORTED;
    } else if ((rect.width > 0 && rect.width > resolution_usage) ||
               (rect.height > 0 && rect.height > resolution_usage)) {
        IM_LOGW("Unsupported %s rect to output resolution more than %d, rect[x,y,w,h] = [%d, %d, %d, %d]",
                name, resolution_usage, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_limit(rga_buffer_t src, rga_buffer_t dst, int scale_usage, int mode_usage) {
    int src_width = 0, src_height = 0;
    int dst_width = 0, dst_height = 0;

    src_width = src.width;
    src_height = src.height;

    if (mode_usage & IM_HAL_TRANSFORM_ROT_270 || mode_usage & IM_HAL_TRANSFORM_ROT_90) {
        dst_width = dst.height;
        dst_height = dst.width;
    } else {
        dst_width = dst.width;
        dst_height = dst.height;
    }
    if (((src_width >> (int)(log(scale_usage)/log(2))) > dst_width) ||
       ((src_height >> (int)(log(scale_usage)/log(2))) > dst_height)) {
        IM_LOGW("Unsupported to scaling less than 1/%d ~ %d times, src[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                scale_usage, scale_usage, src.width, src.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }
    if (((dst_width >> (int)(log(scale_usage)/log(2))) > src_width) ||
       ((dst_height >> (int)(log(scale_usage)/log(2))) > src_height)) {
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
    } else if (format == RK_FORMAT_RGBA_4444 || format == RK_FORMAT_BGRA_4444 ||
               format == RK_FORMAT_RGBA_5551 || format == RK_FORMAT_BGRA_5551 ||
               format == RK_FORMAT_ARGB_4444 || format == RK_FORMAT_ABGR_4444 ||
               format == RK_FORMAT_ARGB_5551 || format == RK_FORMAT_ABGR_5551) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGB_OTHER) {
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
    } else if (format == RK_FORMAT_YCrCb_422_10b_SP || format == RK_FORMAT_YCbCr_422_10b_SP) {
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
    } else {
        IM_LOGW("%s unsupported this format, format = 0x%x(%s)\n%s",
                name, info.format, translate_format_str(info.format),
                querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_align(const char *name, rga_buffer_t info, int byte_stride) {
    int bpp = 0;
    int bit_stride, pixel_stride, align, gcd;

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

    src_isRGB = NormalRgaIsRgbFormat(src_fmt);
    pat_isRGB = NormalRgaIsRgbFormat(pat_fmt);
    dst_isRGB = NormalRgaIsRgbFormat(dst_fmt);

    /**************** blend mode check ****************/
    switch (mode_usage & IM_ALPHA_BLEND_MASK) {
        case IM_ALPHA_BLEND_SRC :
        case IM_ALPHA_BLEND_DST :
            break;
        case IM_ALPHA_BLEND_SRC_OVER :
            if (!NormalRgaFormatHasAlpha(src_fmt)) {
                IM_LOGW("Blend mode 'src_over' unsupported src format without alpha, "
                        "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                        src_fmt, translate_format_str(src_fmt),
                        pat_fmt, translate_format_str(pat_fmt),
                        dst_fmt, translate_format_str(dst_fmt));
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;
        case IM_ALPHA_BLEND_DST_OVER :
            if (pat_enable) {
                if (!NormalRgaFormatHasAlpha(pat_fmt)) {
                    IM_LOGW("Blend mode 'dst_over' unsupported pat format without alpha, "
                            "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                            src_fmt, translate_format_str(src_fmt),
                            pat_fmt, translate_format_str(pat_fmt),
                            dst_fmt, translate_format_str(dst_fmt));
                    return IM_STATUS_NOT_SUPPORTED;
                }
            } else {
                if (!NormalRgaFormatHasAlpha(dst_fmt)) {
                    IM_LOGW("Blend mode 'dst_over' unsupported dst format without alpha, "
                            "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                            src_fmt, translate_format_str(src_fmt),
                            pat_fmt, translate_format_str(pat_fmt),
                            dst_fmt, translate_format_str(dst_fmt));
                    return IM_STATUS_NOT_SUPPORTED;
                }
            }
            break;
        default :
            if (!(NormalRgaFormatHasAlpha(src_fmt) || NormalRgaFormatHasAlpha(dst_fmt))) {
                IM_LOGW("Blend mode unsupported format without alpha, "
                        "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                        src_fmt, translate_format_str(src_fmt),
                        pat_fmt, translate_format_str(pat_fmt),
                        dst_fmt, translate_format_str(dst_fmt));
                return IM_STATUS_NOT_SUPPORTED;
            }
            break;
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

IM_STATUS rga_check_rotate(int mode_usage, rga_info_table_entry &table) {
    if (table.version & (IM_RGA_HW_VERSION_RGA_1 | IM_RGA_HW_VERSION_RGA_1_PLUS)) {
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

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check(const rga_buffer_t src, const rga_buffer_t dst, const rga_buffer_t pat,
                    const im_rect src_rect, const im_rect dst_rect, const im_rect pat_rect, int mode_usage) {
    bool pat_enable = 0;
    IM_STATUS ret = IM_STATUS_NOERROR;
    rga_info_table_entry rga_info;

    memset(&rga_info, 0x0, sizeof(rga_info));
    ret = rga_get_info(&rga_info);
    if (IM_STATUS_FAILED == ret) {
        IM_LOGE("rga im2d: rga2 get info failed!\n");
        return IM_STATUS_FAILED;
    }

    if (mode_usage & IM_ALPHA_BLEND_MASK) {
        if (rga_is_buffer_valid(pat))
            pat_enable = 1;
    }

    /**************** feature judgment ****************/
    ret = rga_check_feature(src, pat, dst, pat_enable, mode_usage, rga_info.feature);
    if (ret != IM_STATUS_NOERROR)
        return ret;

    /**************** info judgment ****************/
    if (~mode_usage & IM_COLOR_FILL) {
        ret = rga_check_info("src", src, src_rect, rga_info.input_resolution);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_format("src", src, src_rect, rga_info.input_format, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_align("src", src, rga_info.byte_stride);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }
    if (pat_enable) {
        /* RGA1 cannot support src1. */
        if (rga_info.version & (IM_RGA_HW_VERSION_RGA_1 | IM_RGA_HW_VERSION_RGA_1_PLUS)) {
            IM_LOGW("RGA1/RGA1_PLUS cannot support src1.");
            return IM_STATUS_NOT_SUPPORTED;
        }


        ret = rga_check_info("pat", pat, pat_rect, rga_info.input_resolution);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_format("pat", pat, pat_rect, rga_info.input_format, mode_usage);
        if (ret != IM_STATUS_NOERROR)
            return ret;
        ret = rga_check_align("pat", pat, rga_info.byte_stride);
        if (ret != IM_STATUS_NOERROR)
            return ret;
    }
    ret = rga_check_info("dst", dst, dst_rect, rga_info.output_resolution);
    if (ret != IM_STATUS_NOERROR)
        return ret;
    ret = rga_check_format("dst", dst, dst_rect, rga_info.output_format, mode_usage);
    if (ret != IM_STATUS_NOERROR)
        return ret;
    ret = rga_check_align("dst", dst, rga_info.byte_stride);
    if (ret != IM_STATUS_NOERROR)
        return ret;

    if ((~mode_usage & IM_COLOR_FILL)) {
        ret = rga_check_limit(src, dst, rga_info.scale_limit, mode_usage);
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

    ret = rga_get_context();
    if (ret != IM_STATUS_SUCCESS)
        return (IM_STATUS)ret;

    if (buffer_pool == NULL) {
        IM_LOGW("buffer pool is null!");
        return IM_STATUS_FAILED;
    }

    ret = ioctl(rgaCtx->rgaFd, RGA_IOC_IMPORT_BUFFER, buffer_pool);
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
        return -1;

    return buffers[0].handle;
}

IM_API rga_buffer_handle_t rga_import_buffer(uint64_t memory, int type, im_handle_param_t *param) {
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

    ret = rga_get_context();
    if (ret != IM_STATUS_SUCCESS)
        return (IM_STATUS)ret;

    if (buffer_pool == NULL) {
        IM_LOGW("buffer pool is null!");
        return IM_STATUS_FAILED;
    }

    ret = ioctl(rgaCtx->rgaFd, RGA_IOC_RELEASE_BUFFER, buffer_pool);
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

static void rga_dump_channel_info(int log_level, const char *name, im_rect &rect, rga_buffer_t &image) {
    log_level |= IM_LOG_FORCE;

    IM_LOG(log_level,
           "%s_channel: \n"
           "  rect[x,y,w,h] = [%d, %d, %d, %d]\n"
           "  image[w,h,ws,hs,f] = [%d, %d, %d, %d, %s]\n"
           "  buffer[handle,fd,va,pa] = [%d, %d, %lx, %lx]\n"
           "  color_space = 0x%x, global_alpha = 0x%x, rd_mode = 0x%x\n",
           name,
           rect.x, rect.y, rect.width, rect.height,
           image.width, image.height, image.wstride, image.hstride, translate_format_str(image.format),
           image.handle, image.fd, (unsigned long)image.vir_addr, (unsigned long)image.phy_addr,
           image.color_space_mode, image.global_alpha, image.rd_mode);
}

static void rga_dump_osd_info(int log_level, im_osd_t &osd_info) {
    IM_LOG(log_level, "osd_mode[0x%x]:\n", osd_info.osd_mode);

    IM_LOG(log_level, "  block: \n"
                      "    width_mode[0x%x], width/witdh_index[0x%x], block_count[%d]\n"
                      "    background_config[0x%x], direction[0x%x], color_mode[0x%x]\n"
                      "    normal_color[0x%x], invert_color[0x%x]\n",
           osd_info.block_parm.width_mode, osd_info.block_parm.width, osd_info.block_parm.block_count,
           osd_info.block_parm.background_config, osd_info.block_parm.direction, osd_info.block_parm.color_mode,
           osd_info.block_parm.normal_color.value, osd_info.block_parm.invert_color.value);

    IM_LOG(log_level, "  invert_config:\n"
                      "    channel[0x%x], flags_mode[0x%x], flages_index[%d] threash[0x%x]\n"
                      "    flages: invert[0x%llx], current[0x%llx]\n"
                      "    invert_mode[%x]",
           osd_info.invert_config.invert_channel, osd_info.invert_config.flags_mode,
           osd_info.invert_config.flags_index, osd_info.invert_config.threash,
           (unsigned long long)osd_info.invert_config.invert_flags,
           (unsigned long long)osd_info.invert_config.current_flags,
           osd_info.invert_config.invert_mode);
    if (osd_info.invert_config.invert_mode == IM_OSD_INVERT_USE_FACTOR)
        IM_LOG(log_level, "    factor[min,max] = alpha[0x%x, 0x%x], yg[0x%x, 0x%x], crb[0x%x, 0x%x]\n",
               osd_info.invert_config.factor.alpha_min, osd_info.invert_config.factor.alpha_max,
               osd_info.invert_config.factor.yg_min, osd_info.invert_config.factor.yg_max,
               osd_info.invert_config.factor.crb_min, osd_info.invert_config.factor.crb_max);
    else
        IM_LOG(log_level, "\n");

    IM_LOG(log_level, "  bpp2rgb info:\n"
                      "    ac_swap[0x%x], endian_swap[0x%x], color0[0x%x], color1[0x%x]\n",
           osd_info.bpp2_info.ac_swap, osd_info.bpp2_info.endian_swap,
           osd_info.bpp2_info.color0.value, osd_info.bpp2_info.color1.value);
}

static void rga_dump_opt(int log_level, im_opt_t &opt, int usage) {
    log_level |= IM_LOG_FORCE;

    IM_LOG(log_level, "opt version[0x%x]:\n", opt.version);
    IM_LOG(log_level, "set_core[0x%x], priority[%d]\n", opt.core, opt.priority);

    if (usage & IM_COLOR_FILL)
        IM_LOG(log_level, "color[0x%x] ", opt.color);
    if (usage & IM_MOSAIC)
        IM_LOG(log_level, "mosaic[%d] ", opt.mosaic_mode);
    if (usage & IM_ROP)
        IM_LOG(log_level, "rop[0x%x] ", opt.rop_code);
    if (usage & IM_ALPHA_COLORKEY_MASK)
        IM_LOG(log_level, "color_key[min,max] = [0x%x, 0x%x] ",
               opt.colorkey_range.min, opt.colorkey_range.max);
    if (usage & (IM_COLOR_FILL | IM_MOSAIC | IM_ROP | IM_ALPHA_COLORKEY_MASK))
        IM_LOG(log_level, "\n");

    if (usage & IM_NN_QUANTIZE)
        IM_LOG(log_level, "nn:\n"
                          "  scale[r,g,b] = [%d, %d, %d], offset[r,g,b] = [0x%x, 0x%x, 0x%x]\n",
               opt.nn.scale_r, opt.nn.scale_g, opt.nn.scale_b,
               opt.nn.offset_r, opt.nn.offset_g, opt.nn.offset_b);

    if (usage & IM_OSD)
        rga_dump_osd_info(log_level, opt.osd_config);

    if (usage & IM_PRE_INTR)
        IM_LOG(log_level, "pre_intr:\n"
                          "  flags[0x%x], read_threshold[0x%x], write_start[0x%x], write_step[0x%x]\n",
               opt.intr_config.flags, opt.intr_config.read_threshold,
               opt.intr_config.write_start, opt.intr_config.write_step);
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

static IM_STATUS rga_task_submit(im_job_handle_t job_handle, rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                                 im_rect srect, im_rect drect, im_rect prect,
                                 int acquire_fence_fd, int *release_fence_fd,
                                 im_opt_t *opt_ptr, int usage) {
    int ret;
    int format;
    rga_info_t srcinfo;
    rga_info_t dstinfo;
    rga_info_t patinfo;

    im_opt_t opt;

    if (rga_get_opt(&opt, opt_ptr) == IM_STATUS_FAILED)
        memset(&opt, 0x0, sizeof(opt));

    memset(&srcinfo, 0, sizeof(rga_info_t));
    memset(&dstinfo, 0, sizeof(rga_info_t));
    memset(&patinfo, 0, sizeof(rga_info_t));

    if (usage & IM_COLOR_FILL)
        ret = rga_set_buffer_info(dst, &dstinfo);
    else
        ret = rga_set_buffer_info(src, dst, &srcinfo, &dstinfo);

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

        ret = rga_set_buffer_info(pat, &patinfo);
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

    /* Blend */
    if (usage & IM_ALPHA_BLEND_MASK) {
        switch(usage & IM_ALPHA_BLEND_MASK) {
            case IM_ALPHA_BLEND_SRC:
                srcinfo.blend = 0x1;
                break;
            case IM_ALPHA_BLEND_DST:
                srcinfo.blend = 0x2;
                break;
            case IM_ALPHA_BLEND_SRC_OVER:
                srcinfo.blend = (usage & IM_ALPHA_BLEND_PRE_MUL) ? 0x405 : 0x105;
                break;
            case IM_ALPHA_BLEND_SRC_IN:
                break;
            case IM_ALPHA_BLEND_DST_IN:
                break;
            case IM_ALPHA_BLEND_SRC_OUT:
                break;
            case IM_ALPHA_BLEND_DST_OVER:
                srcinfo.blend = (usage & IM_ALPHA_BLEND_PRE_MUL) ? 0x504 : 0x501;
                break;
            case IM_ALPHA_BLEND_SRC_ATOP:
                break;
            case IM_ALPHA_BLEND_DST_OUT:
                break;
            case IM_ALPHA_BLEND_XOR:
                break;
        }

        if(srcinfo.blend == 0)
            IM_LOGE("rga_im2d: Could not find blend usage : 0x%x \n", usage);

        /* set global alpha */
        if (src.global_alpha > 0)
            srcinfo.blend ^= src.global_alpha << 16;
        else {
            srcinfo.blend ^= 0xFF << 16;
        }
    }

    /* color key */
    if (usage & IM_ALPHA_COLORKEY_MASK) {
        srcinfo.blend = 0xff0105;

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
            NormalRgaIsYuvFormat(src.format) &&
            NormalRgaIsRgbFormat(pat.format) &&
            NormalRgaIsYuvFormat(dst.format)) {
            dstinfo.color_space_mode = dst.color_space_mode;
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
            NormalRgaIsYuvFormat(src.format) &&
            NormalRgaIsRgbFormat(pat.format) &&
            NormalRgaIsRgbFormat(dst.format)) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else if (NormalRgaIsYuvFormat(src.format) &&
                   NormalRgaIsRgbFormat(dst.format)) {
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
            NormalRgaIsRgbFormat(src.format) &&
            NormalRgaIsRgbFormat(pat.format) &&
            NormalRgaIsYuvFormat(dst.format)) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else if (NormalRgaIsRgbFormat(src.format) &&
                   NormalRgaIsYuvFormat(dst.format)) {
            dstinfo.color_space_mode = dst.color_space_mode;
        } else {
            IM_LOGW("Not rgb to yuv does not need for color_sapce_mode, please fix, "
                    "src_fromat = 0x%x(%s), src1_format = 0x%x(%s), dst_format = 0x%x(%s)",
                    src.format, translate_format_str(src.format),
                    pat.format, rga_is_buffer_valid(pat) ? translate_format_str(pat.format) : "none",
                    dst.format, translate_format_str(dst.format));
            return IM_STATUS_ILLEGAL_PARAM;
        }
    } else if (src.color_space_mode & IM_FULL_CSC_MASK ||
               dst.color_space_mode & IM_FULL_CSC_MASK) {
        /* Get default color space */
        if (src.color_space_mode == IM_COLOR_SPACE_DEFAULT) {
            if  (NormalRgaIsRgbFormat(src.format)) {
                src.color_space_mode = IM_RGB_FULL;
            } else if (NormalRgaIsYuvFormat(src.format)) {
                src.color_space_mode = IM_YUV_BT601_LIMIT_RANGE;
            }
        }

        if (dst.color_space_mode == IM_COLOR_SPACE_DEFAULT) {
            if  (NormalRgaIsRgbFormat(dst.format)) {
                src.color_space_mode = IM_RGB_FULL;
            } else if (NormalRgaIsYuvFormat(dst.format)) {
                src.color_space_mode = IM_YUV_BT601_LIMIT_RANGE;
            }
        }

        if (src.color_space_mode == IM_RGB_FULL &&
            dst.color_space_mode == IM_YUV_BT709_FULL_RANGE) {
            dstinfo.color_space_mode = rgb2yuv_709_full;
        } else if (src.color_space_mode == IM_YUV_BT601_FULL_RANGE &&
                   dst.color_space_mode == IM_YUV_BT709_LIMIT_RANGE) {
            dstinfo.color_space_mode = yuv2yuv_601_full_2_709_limit;
        } else if (src.color_space_mode == IM_YUV_BT709_LIMIT_RANGE &&
                   dst.color_space_mode == IM_YUV_BT601_LIMIT_RANGE) {
            dstinfo.color_space_mode = yuv2yuv_709_limit_2_601_limit;
        } else if (src.color_space_mode == IM_YUV_BT709_FULL_RANGE &&
                   dst.color_space_mode == IM_YUV_BT601_LIMIT_RANGE) {
            dstinfo.color_space_mode = yuv2yuv_709_full_2_601_limit;
        } else if (src.color_space_mode == IM_YUV_BT709_FULL_RANGE &&
                   dst.color_space_mode == IM_YUV_BT601_FULL_RANGE) {
            dstinfo.color_space_mode = yuv2yuv_709_full_2_601_full;
        } else {
            IM_LOGW("Unsupported full csc mode! src_csm = 0x%x, dst_csm = 0x%x",
                    src.color_space_mode, dst.color_space_mode);
            return IM_STATUS_NOT_SUPPORTED;
        }
    }

    if (dst.format == RK_FORMAT_Y4) {
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

    srcinfo.rd_mode = src.rd_mode;
    dstinfo.rd_mode = dst.rd_mode;
    if (rga_is_buffer_valid(pat))
        patinfo.rd_mode = pat.rd_mode;

    RockchipRga& rkRga(RockchipRga::get());

    if (usage & IM_ASYNC) {
        if (release_fence_fd == NULL) {
            IM_LOGW("Async mode release_fence_fd cannot be NULL!");
            return IM_STATUS_ILLEGAL_PARAM;
        }

        dstinfo.sync_mode = RGA_BLIT_ASYNC;

    } else if (usage & IM_SYNC) {
        dstinfo.sync_mode = RGA_BLIT_SYNC;
    }

    dstinfo.in_fence_fd = acquire_fence_fd;
    dstinfo.core = opt.core ? opt.core : g_im2d_context.core;
    dstinfo.priority = opt.priority ? opt.priority : g_im2d_context.priority;

    dstinfo.job_handle = job_handle;

    if (usage & IM_COLOR_FILL) {
        dstinfo.color = opt.color;
        ret = rkRga.RkRgaCollorFill(&dstinfo);
    } else if (usage & IM_COLOR_PALETTE) {
        ret = rkRga.RkRgaCollorPalette(&srcinfo, &dstinfo, &patinfo);
    } else if ((usage & IM_ALPHA_BLEND_MASK) && rga_is_buffer_valid(pat)) {
        ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, &patinfo);
    }else {
        ret = rkRga.RkRgaBlit(&srcinfo, &dstinfo, NULL);
    }

    if (ret) {
        IM_LOGE("Failed to call RockChipRga interface, please use 'dmesg' command to view driver error log.");
        rga_dump_channel_info(IM_LOG_ERROR | IM_LOG_FORCE, "src", srect, src);
        if (rga_is_buffer_valid(pat))
            rga_dump_channel_info(IM_LOG_ERROR | IM_LOG_FORCE, "src1/pat", prect, pat);
        rga_dump_channel_info(IM_LOG_ERROR | IM_LOG_FORCE, "dst", drect, dst);

        if (opt_ptr != NULL)
            rga_dump_opt(IM_LOG_ERROR | IM_LOG_FORCE, *opt_ptr, usage);

        IM_LOGFE("acquir_fence[%d], release_fence_ptr[0x%lx], usage[0x%x]\n",
                 acquire_fence_fd, (unsigned long)release_fence_fd, usage);

        return IM_STATUS_FAILED;
    }

    if (usage & IM_ASYNC)
        *release_fence_fd = dstinfo.out_fence_fd;

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_single_task_submit(rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                                 im_rect srect, im_rect drect, im_rect prect,
                                 int acquire_fence_fd, int *release_fence_fd,
                                 im_opt_t *opt_ptr, int usage) {
    return rga_task_submit(0, src, dst, pat, srect, drect, prect, acquire_fence_fd, release_fence_fd, opt_ptr, usage);
}

IM_STATUS rga_task_submit(im_job_handle_t job_handle, rga_buffer_t src, rga_buffer_t dst, rga_buffer_t pat,
                          im_rect srect, im_rect drect, im_rect prect, im_opt_t *opt_ptr, int usage) {
    return rga_task_submit(job_handle, src, dst, pat, srect, drect, prect, 0, NULL, opt_ptr, usage);
}

im_job_handle_t rga_job_create(uint32_t flags) {
    int ret;
    im_job_handle_t job_handle;
    im_rga_job_t *job = NULL;

    if (rga_get_context() != IM_STATUS_SUCCESS)
        return IM_STATUS_FAILED;

    if (ioctl(rgaCtx->rgaFd, RGA_IOC_REQUEST_CREATE, &flags) < 0) {
        IM_LOGE(" %s(%d) start config fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        return IM_STATUS_FAILED;
    }

    job_handle = flags;

    g_im2d_job_manager.mutex.lock();

    if (g_im2d_job_manager.job_map.count(job_handle) != 0) {
        IM_LOGE("job_map error! handle[%d] already exists[%lu]!\n",
                job_handle, (unsigned long)g_im2d_job_manager.job_map.count(job_handle));
        ret = IM_STATUS_FAILED;
        goto error_cancel_job;
    }

    job = (im_rga_job_t *)malloc(sizeof(*job));
    if (job == NULL) {
        IM_LOGE("rga job alloc error!\n");
        ret = IM_STATUS_FAILED;
        goto error_cancel_job;
    }

    memset(job, 0x0, sizeof(*job));

    job->id = job_handle;
    g_im2d_job_manager.job_map[job_handle] = job;
    g_im2d_job_manager.job_count++;

    g_im2d_job_manager.mutex.unlock();

    return job_handle;

error_cancel_job:
    g_im2d_job_manager.mutex.unlock();
    rga_job_cancel(job_handle);

    return ret;
}

IM_STATUS rga_job_cancel(im_job_handle_t job_handle) {
    im_rga_job_t *job = NULL;

    if (rga_get_context() != IM_STATUS_SUCCESS)
        return IM_STATUS_FAILED;

    g_im2d_job_manager.mutex.lock();

    if (g_im2d_job_manager.job_map.count(job_handle) > 0) {
        job = g_im2d_job_manager.job_map[job_handle];
        if (job != NULL)
            free(job);

        g_im2d_job_manager.job_map.erase(job_handle);
    }

    g_im2d_job_manager.job_count--;

    g_im2d_job_manager.mutex.unlock();

    if (ioctl(rgaCtx->rgaFd, RGA_IOC_REQUEST_CANCEL, &job_handle) < 0) {
        IM_LOGE(" %s(%d) start config fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_job_submit(im_job_handle_t job_handle, int sync_mode, int acquire_fence_fd, int *release_fence_fd) {
    int ret;
    im_rga_job_t *job = NULL;
    struct rga_user_request submit_request;

    if (rga_get_context() != IM_STATUS_SUCCESS)
        return IM_STATUS_FAILED;

    g_im2d_job_manager.mutex.lock();

    if (g_im2d_job_manager.job_map.count(job_handle) == 0) {
        IM_LOGE("job_handle[%d] is illegal!\n", job_handle);

        g_im2d_job_manager.mutex.unlock();
        return IM_STATUS_ILLEGAL_PARAM;
    }

    job = g_im2d_job_manager.job_map[job_handle];
    if (job == NULL) {
        IM_LOGE("job is NULL!\n");

        g_im2d_job_manager.mutex.unlock();
        return IM_STATUS_FAILED;
    }

    memset(&submit_request, 0x0, sizeof(submit_request));

    submit_request.task_ptr = ptr_to_u64(&job->req);
    submit_request.task_num = job->task_count;
    submit_request.id = job->id;

    g_im2d_job_manager.job_map.erase(job_handle);
    g_im2d_job_manager.job_count--;

    g_im2d_job_manager.mutex.unlock();

    free(job);

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

    submit_request.acquire_fence_fd = acquire_fence_fd;

    ret = ioctl(rgaCtx->rgaFd, RGA_IOC_REQUEST_SUBMIT, &submit_request);
    if (ret < 0) {
        IM_LOGE(" %s(%d) start config fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        return IM_STATUS_FAILED;
    }

    if ((sync_mode == IM_ASYNC) && release_fence_fd)
        *release_fence_fd = submit_request.release_fence_fd;

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_job_config(im_job_handle_t job_handle, int sync_mode, int acquire_fence_fd, int *release_fence_fd) {
    int ret;
    im_rga_job_t *job = NULL;
    struct rga_user_request config_request;

    if (rga_get_context() != IM_STATUS_SUCCESS)
        return IM_STATUS_FAILED;

    g_im2d_job_manager.mutex.lock();

    if (g_im2d_job_manager.job_map.count(job_handle) == 0) {
        IM_LOGE("job_handle[%d] is illegal!\n", job_handle);

        g_im2d_job_manager.mutex.unlock();
        return IM_STATUS_ILLEGAL_PARAM;
    }

    job = g_im2d_job_manager.job_map[job_handle];
    if (job == NULL) {
        IM_LOGE("job is NULL!\n");

        g_im2d_job_manager.mutex.unlock();
        return IM_STATUS_FAILED;
    }

    memset(&config_request, 0x0, sizeof(config_request));

    config_request.task_ptr = ptr_to_u64(&job->req);
    config_request.task_num = job->task_count;
    config_request.id = job->id;

    g_im2d_job_manager.mutex.unlock();

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

    config_request.acquire_fence_fd = acquire_fence_fd;

    ret = ioctl(rgaCtx->rgaFd, RGA_IOC_REQUEST_CONFIG, &config_request);
    if (ret < 0) {
        IM_LOGE(" %s(%d) start config fail: %s",__FUNCTION__, __LINE__,strerror(errno));
        return IM_STATUS_FAILED;
    }

    if ((sync_mode == IM_ASYNC) && release_fence_fd)
        *release_fence_fd = config_request.release_fence_fd;

    return IM_STATUS_SUCCESS;
}
