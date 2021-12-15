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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "im2d.h"
#include "im2d_common.h"
#include "im2d_hardware.h"

#include "RockchipRga.h"
#include "core/NormalRga.h"
#include "RgaUtils.h"

#ifdef ANDROID
using namespace android;
#endif

extern struct rgaContext *rgaCtx;
extern __thread char rga_err_str[ERR_MSG_LEN];

static IM_STATUS rga_support_info_merge_table(rga_info_table_entry *dst_table, rga_info_table_entry *merge_table) {
    if (dst_table == NULL || merge_table == NULL) {
        ALOGE("%s[%d] dst or merge table is NULL!\n", __FUNCTION__, __LINE__);
        return IM_STATUS_FAILED;
    }

    dst_table->version |= merge_table->version;
    dst_table->input_format |= merge_table->input_format;
    dst_table->output_format |= merge_table->output_format;
    dst_table->feature |= merge_table->feature;

    dst_table->input_resolution = dst_table->input_resolution > merge_table->input_resolution ?
                                  dst_table->input_resolution : merge_table->input_resolution;
    dst_table->output_resolution = dst_table->output_resolution > merge_table->output_resolution ?
                                   dst_table->output_resolution : merge_table->output_resolution;
    dst_table->scale_limit = dst_table->scale_limit > merge_table->scale_limit ?
                             dst_table->scale_limit : merge_table->scale_limit;
    dst_table->performance = dst_table->performance > merge_table->performance ?
                             dst_table->performance : merge_table->performance;

    return IM_STATUS_SUCCESS;
}

/*
 * rga_version_compare() - Used to compare two struct rga_version_t.
 * @version1
 * @version2
 *
 * if version1 > version2, return >0;
 * if version1 = version2, return 0;
 * if version1 < version2, retunr <0.
 */
static inline int rga_version_compare(struct rga_version_t version1, struct rga_version_t version2) {
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

int imSetErrorMsg(const char* format, ...) {
    int ret = 0;
    va_list ap;

    va_start(ap, format);
    ret = vsnprintf(rga_err_str, ERR_MSG_LEN, format, ap);
    va_end(ap);

    return ret;
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

bool rga_is_buffer_valid(rga_buffer_t buf) {
    return (buf.phy_addr != NULL || buf.fd > 0 || buf.vir_addr != NULL);
}

bool rga_is_rect_valid(im_rect rect) {
    return (rect.x > 0 || rect.y > 0 || (rect.width > 0 && rect.height > 0));
}

IM_STATUS rga_align_check_yuv_8(const char *name, rga_buffer_t info, im_rect rect) {
    if ((info.wstride % 4) || (info.hstride % 2) ||
        (info.width % 2)  || (info.height % 2) ||
        (rect.x % 2) || (rect.y % 2) ||
        (rect.width % 2) || (rect.height % 2)) {
        imSetErrorMsg("%s, Error yuv not align to 2 or width stride not align to 4, "
                        "rect[x,y,w,h] = [%d, %d, %d, %d], "
                        "wstride = %d, hstride = %d, format = 0x%x(%s)\n%s",
                        name, rect.x, rect.y, info.width, info.height, info.wstride, info.hstride,
                        info.format, translate_format_str(info.format),
                        querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_align_check_yuv_10(const char *name, rga_buffer_t info, im_rect rect) {
    if ((info.wstride % 16) || (info.hstride % 2) ||
        (info.width % 2)  || (info.height % 2) ||
        (rect.x % 2) || (rect.y % 2) ||
        (rect.width % 2) || (rect.height % 2)) {
        imSetErrorMsg("%s, Err src wstride is not align to 16 or yuv not align to 2, "
                        "rect[x,y,w,h] = [%d, %d, %d, %d], "
                        "wstride = %d, hstride = %d, format = 0x%x(%s)\n%s",
                        name, rect.x, rect.y, info.width, info.height, info.wstride, info.hstride,
                        info.format, translate_format_str(info.format),
                        querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_set_buffer_info(rga_buffer_t dst, rga_info_t* dstinfo) {
    if(NULL == dstinfo) {
        ALOGE("rga_im2d: invaild dstinfo");
        imSetErrorMsg("Dst structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }

    if(dst.phy_addr != NULL)
        dstinfo->phyAddr= dst.phy_addr;
    else if(dst.fd > 0) {
        dstinfo->fd = dst.fd;
        dstinfo->mmuFlag = 1;
    } else if(dst.vir_addr != NULL) {
        dstinfo->virAddr = dst.vir_addr;
        dstinfo->mmuFlag = 1;
    } else {
        ALOGE("rga_im2d: invaild dst buffer");
        imSetErrorMsg("No address available in dst buffer, phy_addr = %ld, fd = %d, vir_addr = %ld",
                      (unsigned long)dst.phy_addr, dst.fd, (unsigned long)dst.vir_addr);
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_set_buffer_info(const rga_buffer_t src, rga_buffer_t dst, rga_info_t* srcinfo, rga_info_t* dstinfo) {
    if(NULL == srcinfo) {
        ALOGE("rga_im2d: invaild srcinfo");
        imSetErrorMsg("Src structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }
    if(NULL == dstinfo) {
        ALOGE("rga_im2d: invaild dstinfo");
        imSetErrorMsg("Dst structure address is NULL.");
        return IM_STATUS_INVALID_PARAM;
    }

    if(src.phy_addr != NULL)
        srcinfo->phyAddr = src.phy_addr;
    else if(src.fd > 0) {
        srcinfo->fd = src.fd;
        srcinfo->mmuFlag = 1;
    } else if(src.vir_addr != NULL) {
        srcinfo->virAddr = src.vir_addr;
        srcinfo->mmuFlag = 1;
    } else {
        ALOGE("rga_im2d: invaild src buffer");
        imSetErrorMsg("No address available in src buffer, phy_addr = %ld, fd = %d, vir_addr = %ld",
                      (unsigned long)src.phy_addr, src.fd, (unsigned long)src.vir_addr);
        return IM_STATUS_INVALID_PARAM;
    }

    if(dst.phy_addr != NULL)
        dstinfo->phyAddr= dst.phy_addr;
    else if(dst.fd > 0) {
        dstinfo->fd = dst.fd;
        dstinfo->mmuFlag = 1;
    } else if(dst.vir_addr != NULL) {
        dstinfo->virAddr = dst.vir_addr;
        dstinfo->mmuFlag = 1;
    } else {
        ALOGE("rga_im2d: invaild dst buffer");
        imSetErrorMsg("No address available in dst buffer, phy_addr = %ld, fd = %d, vir_addr = %ld",
                      (unsigned long)dst.phy_addr, dst.fd, (unsigned long)dst.vir_addr);
        return IM_STATUS_INVALID_PARAM;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_get_info(rga_info_table_entry *return_table) {
    rga_info_table_entry merge_table;
    int  rga_version = 0;

    /* Get RGA context */
    if (rgaCtx == NULL) {
        RockchipRga& rkRga(RockchipRga::get());
        if (rgaCtx == NULL) {
            memcpy(return_table, &hw_info_table[IM_RGA_HW_VERSION_RGA_V_ERR_INDEX], sizeof(return_table));

            ALOGE("rga_im2d: The current RockchipRga singleton is destroyed. "
                  "Please check if RkRgaInit/RkRgaDeInit are called, if so, please disable them.");
            imSetErrorMsg("The current RockchipRga singleton is destroyed."
                          "Please check if RkRgaInit/RkRgaDeInit are called, if so, please disable them.");
            return IM_STATUS_FAILED;
        }
    }

    memset(&merge_table, 0x0, sizeof(merge_table));

    for (int i = 0; i < rgaCtx->mHwVersions.size; i++) {
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
        } else if (rgaCtx->mHwVersions.version[i].major == 4 &&
                   rgaCtx->mHwVersions.version[i].minor == 0) {
            switch (rgaCtx->mHwVersions.version[i].revision) {
                case 0x18632 :
                    rga_version = IM_RGA_HW_VERSION_RGA_2_LITE0_INDEX;
                    memcpy(&merge_table, &hw_info_table[rga_version], sizeof(merge_table));
                    break;
                case 0x23998 :
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
        ALOGE("rga_im2d: Can not get the correct RGA version, please check the driver, version=%s\n",
              rgaCtx->mHwVersions.version[0].str);
        imSetErrorMsg("Can not get the correct RGA version, please check the driver, version=%s",
                      rgaCtx->mHwVersions.version[0].str);
        return IM_STATUS_FAILED;
    }

    return IM_STATUS_SUCCESS;
}

IM_STATUS rga_check_driver(void) {
    int table_size, bind_index, least_index;
    bool user_bind = false;

    if (rgaCtx == NULL) {
        ALOGE("rga context is NULL!");
        imSetErrorMsg("rga context is NULL!");
        return IM_STATUS_FAILED;
    }

    table_size = sizeof(driver_bind_table) / sizeof(rga_dirver_bind_table_entry);

    /* First, find the driver version corresponding to librga. */
    for (int i = (table_size - 1); i >= 0; i--) {
        if (rga_version_compare((struct rga_version_t)
                                { RGA_API_MAJOR_VERSION,
                                  RGA_API_MINOR_VERSION,
                                  RGA_API_REVISION_VERSION,
                                  RGA_API_VERSION },
                                driver_bind_table[i].user) >= 0) {
            if (i == (table_size - 1)) {
                user_bind = true;
                bind_index = i;

                break;
            } else if (rga_version_compare(driver_bind_table[i + 1].user,
                                           (struct rga_version_t)
                                           { RGA_API_MAJOR_VERSION,
                                             RGA_API_MINOR_VERSION,
                                             RGA_API_REVISION_VERSION,
                                             RGA_API_VERSION }) > 0) {
                user_bind = true;
                bind_index = i;

                break;
            }
        }
    }

    if (user_bind) {
        /* Second, check whether the current driver version matches. */
        if (rga_version_compare(rgaCtx->mDriverVersion, driver_bind_table[bind_index].driver) >= 0) {
            if (bind_index == table_size - 1) {
                return IM_STATUS_SUCCESS;
            } else if (rga_version_compare(driver_bind_table[bind_index + 1].driver, rgaCtx->mDriverVersion) > 0) {
                return IM_STATUS_SUCCESS;
            } else {
                /* find needs to be update version at least. */
                least_index = table_size - 1;
                for (int i = (table_size - 1); i >= 0; i--) {
                    if (rga_version_compare(rgaCtx->mDriverVersion, driver_bind_table[i].driver) >= 0) {
                        if (i == (table_size - 1)) {
                            least_index = i;
                            break;
                        } else if (rga_version_compare(driver_bind_table[i + 1].driver, rgaCtx->mDriverVersion) > 0) {
                            least_index = i;
                            break;
                        }
                    }
                }

                ALOGE("The librga needs to be updated to version %s at least. "
                      "current version: librga %s, driver %s.",
                      driver_bind_table[least_index].user.str,
                      RGA_API_VERSION, rgaCtx->mDriverVersion.str);
                imSetErrorMsg("The librga needs to be updated to version %s at least. "
                              "current version: librga %s, driver %s.",
                              driver_bind_table[least_index].user.str,
                              RGA_API_VERSION, rgaCtx->mDriverVersion.str);

                return IM_STATUS_ERROR_VERSION;
            }
        } else {
            ALOGE("The driver may be compatible, "
                  "but it is best to update the driver to version %s. "
                  "current version: librga %s, driver %s.",
                  driver_bind_table[bind_index].driver.str,
                  RGA_API_VERSION, rgaCtx->mDriverVersion.str);
            imSetErrorMsg("The driver may be compatible, "
                          "but it is best to update the driver to version %s. "
                          "current version: librga %s, driver %s.",
                          driver_bind_table[bind_index].driver.str,
                          RGA_API_VERSION, rgaCtx->mDriverVersion.str);

            /* Sometimes it is possible to enter compatibility mode. */
            return IM_STATUS_NOERROR;
        }
    } else {
        ALOGE("Failed to get the version binding table of librga, "
            "current version: librga: %s, driver: %s",
            RGA_API_VERSION, rgaCtx->mDriverVersion.str);
        imSetErrorMsg("Failed to get the version binding table of librga, "
                    "current version: librga: %s, driver: %s",
                    RGA_API_VERSION, rgaCtx->mDriverVersion.str);

        return IM_STATUS_ERROR_VERSION;
    }
}

IM_STATUS rga_check_info(const char *name, const rga_buffer_t info, const im_rect rect, int resolution_usage) {
    /**************** src/dst judgment ****************/
    if (info.width <= 0 || info.height <= 0 || info.format < 0) {
        imSetErrorMsg("Illegal %s, the parameter cannot be negative or 0, width = %d, height = %d, format = 0x%x(%s)",
                      name, info.width, info.height, info.format, translate_format_str(info.format));
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if (info.width < 2 || info.height < 2) {
        imSetErrorMsg("Hardware limitation %s, unsupported operation of images smaller than 2 pixels, "
                      "width = %d, height = %d",
                      name, info.width, info.height);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if (info.wstride < info.width || info.hstride < info.height) {
        imSetErrorMsg("Invaild %s, Virtual width or height is less than actual width and height, "
                      "wstride = %d, width = %d, hstride = %d, height = %d",
                      name, info.wstride, info.width, info.hstride, info.height);
        return IM_STATUS_INVALID_PARAM;
    }

    /**************** rect judgment ****************/
    if (rect.width < 0 || rect.height < 0 || rect.x < 0 || rect.y < 0) {
        imSetErrorMsg("Illegal %s rect, the parameter cannot be negative, rect[x,y,w,h] = [%d, %d, %d, %d]",
                      name, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_ILLEGAL_PARAM;
    }

    if ((rect.width > 0  && rect.width < 2) || (rect.height > 0 && rect.height < 2) ||
        (rect.x > 0 && rect.x < 2)          || (rect.y > 0 && rect.y < 2)) {
        imSetErrorMsg("Hardware limitation %s rect, unsupported operation of images smaller than 2 pixels, "
                      "rect[x,y,w,h] = [%d, %d, %d, %d]",
                      name, rect.x, rect.y, rect.width, rect.height);
        return IM_STATUS_INVALID_PARAM;
    }

    if ((rect.width + rect.x > info.wstride) || (rect.height + rect.y > info.hstride)) {
        imSetErrorMsg("Invaild %s rect, the sum of width and height of rect needs to be less than wstride or hstride, "
                      "rect[x,y,w,h] = [%d, %d, %d, %d], wstride = %d, hstride = %d",
                      name, rect.x, rect.y, rect.width, rect.height, info.wstride, info.hstride);
        return IM_STATUS_INVALID_PARAM;
    }

    /**************** resolution check ****************/
    if (info.width > resolution_usage ||
        info.height > resolution_usage) {
        imSetErrorMsg("Unsupported %s to input resolution more than %d, width = %d, height = %d",
                      name, resolution_usage, info.width, info.height);
        return IM_STATUS_NOT_SUPPORTED;
    } else if ((rect.width > 0 && rect.width > resolution_usage) ||
               (rect.height > 0 && rect.height > resolution_usage)) {
        imSetErrorMsg("Unsupported %s rect to output resolution more than %d, rect[x,y,w,h] = [%d, %d, %d, %d]",
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
        imSetErrorMsg("Unsupported to scaling less than 1/%d ~ %d times, src[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                      scale_usage, scale_usage, src.width, src.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }
    if (((dst_width >> (int)(log(scale_usage)/log(2))) > src_width) ||
       ((dst_height >> (int)(log(scale_usage)/log(2))) > src_height)) {
        imSetErrorMsg("Unsupported to scaling more than 1/%d ~ %d times, src[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                      scale_usage, scale_usage, src.width, src.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_format(const char *name, rga_buffer_t info, im_rect rect, int format_usage, int mode_usgae) {
    IM_STATUS ret;
    int format = -1;

    format = RkRgaGetRgaFormat(RkRgaCompatibleFormat(info.format));
    if (-1 == format) {
        imSetErrorMsg("illegal %s format, please query and fix, format = 0x%x(%s)\n%s",
                      name, info.format, translate_format_str(info.format),
                      querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (format == RK_FORMAT_RGBA_8888 || format == RK_FORMAT_BGRA_8888 ||
        format == RK_FORMAT_RGBX_8888 || format == RK_FORMAT_BGRX_8888 ||
        format == RK_FORMAT_ARGB_8888 || format == RK_FORMAT_ABGR_8888 ||
        format == RK_FORMAT_XRGB_8888 || format == RK_FORMAT_XBGR_8888 ||
        format == RK_FORMAT_RGB_888   || format == RK_FORMAT_BGR_888   ||
        format == RK_FORMAT_RGB_565   || format == RK_FORMAT_BGR_565) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGB) {
            imSetErrorMsg("%s unsupported RGB format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_RGBA_4444 || format == RK_FORMAT_BGRA_4444 ||
               format == RK_FORMAT_RGBA_5551 || format == RK_FORMAT_BGRA_5551 ||
               format == RK_FORMAT_ARGB_4444 || format == RK_FORMAT_ABGR_4444 ||
               format == RK_FORMAT_ARGB_5551 || format == RK_FORMAT_ABGR_5551) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_RGB_OTHER) {
            imSetErrorMsg("%s unsupported RGBA 4444/5551 format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_BPP1 || format == RK_FORMAT_BPP2 ||
               format == RK_FORMAT_BPP4 || format == RK_FORMAT_BPP8) {
        if ((~format_usage & IM_RGA_SUPPORT_FORMAT_BPP) && !(mode_usgae & IM_COLOR_PALETTE)) {
            imSetErrorMsg("%s unsupported BPP format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }
    } else if (format == RK_FORMAT_YCrCb_420_SP || format == RK_FORMAT_YCbCr_420_SP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_8_BIT) {
            imSetErrorMsg("%s unsupported YUV420 semi-planner 8bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_420_P  || format == RK_FORMAT_YCbCr_420_P) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_PLANNER_8_BIT) {
            imSetErrorMsg("%s unsupported YUV420 planner 8bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_422_SP || format == RK_FORMAT_YCbCr_422_SP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_8_BIT) {
            imSetErrorMsg("%s unsupported YUV422 semi-planner 8bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_422_P  || format == RK_FORMAT_YCbCr_422_P) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_422_PLANNER_8_BIT) {
            imSetErrorMsg("%s unsupported YUV422 planner 8bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCrCb_420_SP_10B || format == RK_FORMAT_YCbCr_420_SP_10B) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_420_SEMI_PLANNER_10_BIT) {
            imSetErrorMsg("%s unsupported YUV420 semi-planner 10bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_10(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
        ALOGE("If it is an RK encoder output, it needs to be aligned with an odd multiple of 256.\n");
    } else if (format == RK_FORMAT_YCrCb_422_10b_SP || format == RK_FORMAT_YCbCr_422_10b_SP) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_422_SEMI_PLANNER_10_BIT) {
            imSetErrorMsg("%s unsupported YUV422 semi-planner 10bit format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_10(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
        ALOGE("If it is an RK encoder output, it needs to be aligned with an odd multiple of 256.\n");
    } else if (format == RK_FORMAT_YUYV_420 || format == RK_FORMAT_YVYU_420 ||
               format == RK_FORMAT_UYVY_420 || format == RK_FORMAT_VYUY_420) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUYV_420) {
            imSetErrorMsg("%s unsupported YUYV format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YUYV_422 || format == RK_FORMAT_YVYU_422 ||
               format == RK_FORMAT_UYVY_422 || format == RK_FORMAT_VYUY_422) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUYV_422) {
            imSetErrorMsg("%s unsupported YUYV format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_YCbCr_400) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_YUV_400) {
            imSetErrorMsg("%s unsupported YUV400 format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else if (format == RK_FORMAT_Y4) {
        if (~format_usage & IM_RGA_SUPPORT_FORMAT_Y4) {
            imSetErrorMsg("%s unsupported Y4/Y1 format, format = 0x%x(%s)\n%s",
                          name, info.format, translate_format_str(info.format),
                          querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
            return IM_STATUS_NOT_SUPPORTED;
        }

        ret = rga_align_check_yuv_8(name, info, rect);
        if (ret != IM_STATUS_SUCCESS)
            return ret;
    } else {
        imSetErrorMsg("%s unsupported this format, format = 0x%x(%s)\n%s",
                      name, info.format, translate_format_str(info.format),
                      querystring((strcmp("dst", name) == 0) ? RGA_OUTPUT_FORMAT : RGA_INPUT_FORMAT));
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_blend(rga_buffer_t src, rga_buffer_t pat, rga_buffer_t dst, int pat_enable, int mode_usage) {
    int src_fmt, pat_fmt, dst_fmt;
    bool src_isRGB, pat_isRGB, dst_isRGB;

    src_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(src.format));
    pat_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(pat.format));
    dst_fmt = RkRgaGetRgaFormat(RkRgaCompatibleFormat(dst.format));

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
                imSetErrorMsg("Blend mode 'src_over' unsupported src format without alpha, "
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
                    imSetErrorMsg("Blend mode 'dst_over' unsupported pat format without alpha, "
                                  "format[src,src1,dst] = [0x%x(%s), 0x%x(%s), 0x%x(%s)]",
                                  src_fmt, translate_format_str(src_fmt),
                                  pat_fmt, translate_format_str(pat_fmt),
                                  dst_fmt, translate_format_str(dst_fmt));
                    return IM_STATUS_NOT_SUPPORTED;
                }
            } else {
                if (!NormalRgaFormatHasAlpha(dst_fmt)) {
                    imSetErrorMsg("Blend mode 'dst_over' unsupported dst format without alpha, "
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
                imSetErrorMsg("Blend mode unsupported format without alpha, "
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
        imSetErrorMsg("In the three-channel mode Alapha blend, the width and height of the src1 channel "
                      "must be equal to the dst channel, src1[w,h] = [%d, %d], dst[w,h] = [%d, %d]",
                      pat.width, pat.height, dst.width, dst.height);
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_rotate(int mode_usage, rga_info_table_entry &table) {
    if (table.version & (IM_RGA_HW_VERSION_RGA_1 | IM_RGA_HW_VERSION_RGA_1_PLUS)) {
        if (mode_usage & IM_HAL_TRANSFORM_FLIP_H_V) {
            imSetErrorMsg("RGA1/RGA1_PLUS cannot support H_V mirror.");
            return IM_STATUS_NOT_SUPPORTED;
        }

        if ((mode_usage & (IM_HAL_TRANSFORM_ROT_90 + IM_HAL_TRANSFORM_ROT_180 + IM_HAL_TRANSFORM_ROT_270)) &&
            (mode_usage & (IM_HAL_TRANSFORM_FLIP_H + IM_HAL_TRANSFORM_FLIP_V + IM_HAL_TRANSFORM_FLIP_H_V))) {
            imSetErrorMsg("RGA1/RGA1_PLUS cannot support rotate with mirror.");
            return IM_STATUS_NOT_SUPPORTED;
        }
    }

    return IM_STATUS_NOERROR;
}

IM_STATUS rga_check_feature(rga_buffer_t src, rga_buffer_t pat, rga_buffer_t dst,
                                   int pat_enable, int mode_usage, int feature_usage) {
    if (mode_usage == IM_COLOR_FILL && (~feature_usage & IM_RGA_SUPPORT_FEATURE_COLOR_FILL)) {
        imSetErrorMsg("The platform does not support color fill featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (mode_usage == IM_COLOR_PALETTE && (~feature_usage & IM_RGA_SUPPORT_FEATURE_COLOR_PALETTE)) {
        imSetErrorMsg("The platform does not support color palette featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (mode_usage == IM_ROP && (~feature_usage & IM_RGA_SUPPORT_FEATURE_ROP)) {
        imSetErrorMsg("The platform does not support ROP featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if (mode_usage == IM_NN_QUANTIZE && (~feature_usage & IM_RGA_SUPPORT_FEATURE_QUANTIZE)) {
        imSetErrorMsg("The platform does not support quantize featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((pat_enable ? (pat.color_space_mode & IM_RGB_TO_YUV_MASK) : 0) && (~feature_usage & IM_RGA_SUPPORT_FEATURE_SRC1_R2Y_CSC)) {
        imSetErrorMsg("The platform does not support src1 channel RGB2YUV color space convert featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    if ((src.color_space_mode & IM_FULL_CSC_MASK ||
        dst.color_space_mode & IM_FULL_CSC_MASK ||
        (pat_enable ? (pat.color_space_mode & IM_FULL_CSC_MASK) : 0)) &&
        (~feature_usage & IM_RGA_SUPPORT_FEATURE_DST_FULL_CSC)) {
        imSetErrorMsg("The platform does not support dst channel full color space convert(Y2Y/Y2R) featrue. \n%s",
                      querystring(RGA_FEATURE));
        return IM_STATUS_NOT_SUPPORTED;
    }

    return IM_STATUS_NOERROR;
}
