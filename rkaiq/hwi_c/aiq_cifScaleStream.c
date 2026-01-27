/*
 *  Copyright (c) 2024 Rockchip Electronics Co., Ltd
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

#include <stdint.h>

#include "hwi_c/aiq_camHw.h"
#include "hwi_c/aiq_cifSclStream.h"
#include "hwi_c/aiq_stream.h"
#include "xcam_log.h"
#include "xcore_c/aiq_v4l2_device.h"

static XCamReturn set_ratio_fmt(AiqCifSclStream_t* pCifSclStrm, int32_t ratio) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (ratio > 0 && ratio <= 8) {
        pCifSclStrm->_ratio = 8;
    } else if (ratio > 8 && ratio <= 16) {
        pCifSclStrm->_ratio = 16;
    } else {
        pCifSclStrm->_ratio = 32;
    }

    uint32_t dst_width = pCifSclStrm->_width / pCifSclStrm->_ratio;
    if ((dst_width % 2) != 0) dst_width -= 1;

    uint32_t dst_height = pCifSclStrm->_height / pCifSclStrm->_ratio;
    if ((dst_height % 2) != 0) dst_height -= 1;

    for (int i = 0; i < pCifSclStrm->_index; i++) {
        ret = AiqV4l2Device_setFmt(pCifSclStrm->_dev[i], dst_width, dst_height,
                                   pCifSclStrm->_sns_v4l_pix_fmt, V4L2_FIELD_NONE, 0);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set format failed",
                            AiqV4l2Device_getDevName(pCifSclStrm->_dev[i]));
        } else {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) set fmt info: fmt 0x%x, %dx%d !",
                            AiqV4l2Device_getDevName(pCifSclStrm->_dev[i]),
                            pCifSclStrm->_sns_v4l_pix_fmt, dst_width, dst_height);
        }
    }

    return ret;
}

XCamReturn AiqCifSclStream_init(AiqCifSclStream_t* pCifSclStrm, const rk_sensor_full_info_t* s_info,
                                AiqPollCallback_t* cb) {
    if (!pCifSclStrm || !s_info) return XCAM_RETURN_ERROR_PARAM;

    pCifSclStrm->_first_start = true;

    if (!s_info->cif_info) {
        LOGD_CAMHW("no link to vicap\n");
        return XCAM_RETURN_NO_ERROR;
    }

    switch (pCifSclStrm->_working_mode) {
        case 0x0:
            pCifSclStrm->_index = 1;
            break;
        case 0x11:
        case 0x12:
            pCifSclStrm->_index = 2;
            break;
        case 0x21:
        case 0x22:
            pCifSclStrm->_index = 3;
            break;
        default:
            if (RK_AIQ_HDR_IS_SENSOR_BUILTIN(pCifSclStrm->_working_mode))
                pCifSclStrm->_index = 1;
            else
                pCifSclStrm->_index = 0;
            break;
    }

    if (pCifSclStrm->_index >= 1) {
        if (strlen(s_info->cif_info->mipi_scl0)) {
            pCifSclStrm->_dev[0] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
            if (!pCifSclStrm->_dev[0]) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
                goto fail;
            }
            AiqV4l2Device_init(pCifSclStrm->_dev[0], s_info->cif_info->mipi_scl0);
            pCifSclStrm->_dev[0]->open(pCifSclStrm->_dev[0], false);
        }
    }
    if (pCifSclStrm->_index >= 2) {
        if (strlen(s_info->cif_info->mipi_scl1)) {
            pCifSclStrm->_dev[1] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
            if (!pCifSclStrm->_dev[1]) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
                goto fail;
            }
            AiqV4l2Device_init(pCifSclStrm->_dev[1], s_info->cif_info->mipi_scl0);
            pCifSclStrm->_dev[1]->open(pCifSclStrm->_dev[1], false);
        }
    }

    if (pCifSclStrm->_index == 3) {
        if (strlen(s_info->cif_info->mipi_scl2)) {
            pCifSclStrm->_dev[2] = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
            if (!pCifSclStrm->_dev[2]) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
                goto fail;
            }
            AiqV4l2Device_init(pCifSclStrm->_dev[2], s_info->cif_info->mipi_scl0);
        }
    }

    for (int i = 0; i < pCifSclStrm->_index; i++) {
        if (pCifSclStrm->_dev[i]) {
            pCifSclStrm->_stream[i] = (AiqRawStream_t*)aiq_mallocz(sizeof(AiqRawStream_t));
            AiqRawStream_init(pCifSclStrm->_stream[i], pCifSclStrm->_dev[i], VICAP_POLL_SCL, i);
            if (cb) AiqPollThread_setPollCallback(pCifSclStrm->_stream[i]->_base._poll_thread, cb);
        }
    }

    pCifSclStrm->_init = true;
    return XCAM_RETURN_NO_ERROR;
fail:
    AiqCifSclStream_deinit(pCifSclStrm);
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqCifSclStream_deinit(AiqCifSclStream_t* pCifSclStrm) {
    if (!pCifSclStrm) return;

    for (int32_t i = 0; i < 3; i++) {
        if (pCifSclStrm->_dev[i]) {
            pCifSclStrm->_dev[i]->close(pCifSclStrm->_dev[i]);
            if (pCifSclStrm->_stream[i]) {
                AiqRawStream_deinit(pCifSclStrm->_stream[i]);
                aiq_free(pCifSclStrm->_stream[i]);
                pCifSclStrm->_stream[i] = NULL;
            }
            aiq_free(pCifSclStrm->_dev[i]);
            pCifSclStrm->_dev[i] = NULL;
        }
    }
}

XCamReturn AiqCifSclStream_start(AiqCifSclStream_t* pCifSclStrm) {
    if (!pCifSclStrm) return XCAM_RETURN_ERROR_PARAM;

    if (pCifSclStrm->_init && !pCifSclStrm->_active) {
        for (int i = 0; i < pCifSclStrm->_index; i++) {
            if (pCifSclStrm->_stream[i])
                pCifSclStrm->_stream[i]->_base.start(&pCifSclStrm->_stream[i]->_base);
        }
        pCifSclStrm->_active = true;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCifSclStream_stopThreadOnly(AiqCifSclStream_t* pCifSclStrm) {
    if (!pCifSclStrm) return XCAM_RETURN_ERROR_PARAM;

    for (int32_t i = 0; i < pCifSclStrm->_index; i++) {
        if (pCifSclStrm->_stream[i])
            pCifSclStrm->_stream[i]->_base.stopThreadOnly(&pCifSclStrm->_stream[i]->_base);
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCifSclStream_stop(AiqCifSclStream_t* pCifSclStrm) {
    if (!pCifSclStrm) return XCAM_RETURN_ERROR_PARAM;

    for (int32_t i = 0; i < pCifSclStrm->_index; i++) {
        if (pCifSclStrm->_stream[i])
            pCifSclStrm->_stream[i]->_base.stop(&pCifSclStrm->_stream[i]->_base);
    }
    pCifSclStrm->_active = false;
    return XCAM_RETURN_NO_ERROR;
}

void AiqCifSclStream_set_working_mode(AiqCifSclStream_t* pCifSclStrm, int mode) {
    if (!pCifSclStrm) return;

    pCifSclStrm->_working_mode = mode;
}

XCamReturn AiqCifSclStream_set_format(AiqCifSclStream_t* pCifSclStrm,
                                      const struct v4l2_subdev_format* sns_sd_fmt,
                                      uint32_t sns_v4l_pix_fmt, int bpp) {
    if (!pCifSclStrm || !sns_sd_fmt) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pCifSclStrm->_width  = sns_sd_fmt->format.width;
    pCifSclStrm->_height = sns_sd_fmt->format.height;
    pCifSclStrm->_bpp    = bpp;

    switch (sns_v4l_pix_fmt) {
        case V4L2_PIX_FMT_SBGGR8:
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_SBGGR12:
        case V4L2_PIX_FMT_SBGGR14:
        case V4L2_PIX_FMT_SBGGR16:
            pCifSclStrm->_sns_v4l_pix_fmt = V4L2_PIX_FMT_SBGGR16;
            break;
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGBRG14:
        case V4L2_PIX_FMT_SGBRG16:
            pCifSclStrm->_sns_v4l_pix_fmt = V4L2_PIX_FMT_SGBRG16;
            break;
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SGRBG14:
        case V4L2_PIX_FMT_SGRBG16:
            pCifSclStrm->_sns_v4l_pix_fmt = V4L2_PIX_FMT_SGRBG16;
            break;
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SRGGB10:
        case V4L2_PIX_FMT_SRGGB12:
        case V4L2_PIX_FMT_SRGGB14:
        case V4L2_PIX_FMT_SRGGB16:
            pCifSclStrm->_sns_v4l_pix_fmt = V4L2_PIX_FMT_SRGGB16;
            break;
        default:
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "unknown format");
            return XCAM_RETURN_ERROR_PARAM;
    }

    return ret;
}

XCamReturn AiqCifSclStream_set_format2(AiqCifSclStream_t* pCifSclStrm,
                                       const struct v4l2_subdev_selection* sns_sd_sel,
                                       uint32_t sns_v4l_pix_fmt, int bpp) {
    if (!pCifSclStrm || !sns_sd_sel) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret       = XCAM_RETURN_NO_ERROR;
    pCifSclStrm->_width  = sns_sd_sel->r.width;
    pCifSclStrm->_height = sns_sd_sel->r.height;
    pCifSclStrm->_bpp    = bpp;

    switch (sns_v4l_pix_fmt) {
        case V4L2_PIX_FMT_SBGGR8:
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_SBGGR12:
        case V4L2_PIX_FMT_SBGGR14:
        case V4L2_PIX_FMT_SBGGR16:
            pCifSclStrm->_sns_v4l_pix_fmt = V4L2_PIX_FMT_SBGGR16;
            break;
        case V4L2_PIX_FMT_SGBRG8:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGBRG14:
        case V4L2_PIX_FMT_SGBRG16:
            pCifSclStrm->_sns_v4l_pix_fmt = V4L2_PIX_FMT_SGBRG16;
            break;
        case V4L2_PIX_FMT_SGRBG8:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SGRBG14:
        case V4L2_PIX_FMT_SGRBG16:
            pCifSclStrm->_sns_v4l_pix_fmt = V4L2_PIX_FMT_SGRBG16;
            break;
        case V4L2_PIX_FMT_SRGGB8:
        case V4L2_PIX_FMT_SRGGB10:
        case V4L2_PIX_FMT_SRGGB12:
        case V4L2_PIX_FMT_SRGGB14:
        case V4L2_PIX_FMT_SRGGB16:
            pCifSclStrm->_sns_v4l_pix_fmt = V4L2_PIX_FMT_SRGGB16;
            break;
        default:
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "unknown format");
            return XCAM_RETURN_ERROR_PARAM;
    }
    return ret;
}

XCamReturn AiqCifSclStream_prepare(AiqCifSclStream_t* pCifSclStrm) {
    if (!pCifSclStrm) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (!pCifSclStrm->_init || pCifSclStrm->_active) return XCAM_RETURN_NO_ERROR;

    for (int i = 0; i < pCifSclStrm->_index; i++) {
        ret = pCifSclStrm->_dev[i]->prepare(pCifSclStrm->_dev[i]);
        if (ret < 0) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "mipi tx:%d prepare err: %d\n", i, ret);

        pCifSclStrm->_stream[i]->_base.set_device_prepared(&pCifSclStrm->_stream[i]->_base, true);
    }

    return ret;
}

XCamReturn AiqCifSclStream_restart(AiqCifSclStream_t* pCifSclStrm,
                                   const rk_sensor_full_info_t* s_info, int ratio,
                                   AiqPollCallback_t* callback, int mode) {
    if (!pCifSclStrm) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (mode) {
        if (!pCifSclStrm->_init) ret = AiqCifSclStream_init(pCifSclStrm, s_info, callback);

        if (ratio != pCifSclStrm->_ratio) {
            if (pCifSclStrm->_active) {
                AiqCifSclStream_stop(pCifSclStrm);
            }
            ret = set_ratio_fmt(pCifSclStrm, ratio);
        }

        if (!pCifSclStrm->_active && pCifSclStrm->_init) {
            ret = AiqCifSclStream_prepare(pCifSclStrm);
            ret = AiqCifSclStream_start(pCifSclStrm);
        }
    } else {
        if (pCifSclStrm->_active) ret = AiqCifSclStream_stop(pCifSclStrm);
    }
    return ret;
}

bool AiqCifSclStream_setPollCallback(AiqCifSclStream_t* pCifSclStrm, AiqPollCallback_t* cb) {
    if (!pCifSclStrm || !cb) return false;

    for (int i = 0; i < pCifSclStrm->_index; i++) {
        AiqPollThread_setPollCallback(pCifSclStrm->_stream[i]->_base._poll_thread, cb);
    }

    return true;
}

bool AiqCifSclStream_getIsActive(AiqCifSclStream_t* pCifSclStrm) { return pCifSclStrm->_active; }
