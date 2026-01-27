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

#include "hwi_c/aiq_CamHwBase.h"

#include <stdio.h>
#if RKAIQ_HAVE_DUMPSYS
#include <time.h>
#endif

#include "common/linux/rk-video-format.h"
#ifdef ANDROID_OS
#include <cutils/properties.h>
#endif
#include "hwi/isp20/rkispp-config.h"
#include "include/common/rk-aiisp-config.h"
#include "include/common/rk-camera-module.h"
#include "include/common/rkcif-config.h"
#include "include/common/rk-isp33-config.h"
#include "include/common/rk-isp39-config.h"
#include "include/common/rkisp2-config.h"
#include "include/common/rkisp21-config.h"
#include "include/common/rkisp3-config.h"
#include "include/common/rkisp32-config.h"
#include "include/common/rkcif-config.h"
#include "iq_parser_v2/cis_head.h"

// #include "aiq_core/thumbnails.h"
#include "RkAiqManager_c.h"
#include "c_base/aiq_list.h"
#include "c_base/aiq_map.h"
#include "c_base/aiq_pool.h"
#include "common/code_to_pixel_format.h"
#include "common/mediactl/mediactl-priv.h"
#include "common/rk_aiq_types_priv_c.h"
#include "hwi_c/aiq_camHw.h"
#include "hwi_c/aiq_cifSclStream.h"
#include "hwi_c/aiq_ispParamsCvt.h"
#include "hwi_c/aiq_ispParamsSplitter.h"
#include "hwi_c/aiq_lensHw.h"
#include "hwi_c/aiq_pdafStreamProcUnit.h"
#include "hwi_c/aiq_rawStreamCapUnit.h"
#include "hwi_c/aiq_rawStreamProcUnit.h"
#include "hwi_c/aiq_AiRmsStreamProcUnit.h"
#include "hwi_c/aiq_stream.h"
#include "hwi_c/rawReprocess/aiq_btnrReproc.h"
#include "algos/aiisp/rk_aiisp.h"
#include "include/iq_parser_v2/RkAiqCalibDbV2Helper.h"
#include "xcore/base/xcam_defs.h"
#include "xcore_c/aiq_v4l2_device.h"
#include <dlfcn.h>
#include <linux/dma-buf.h>
#include <sys/ioctl.h>

#if RKAIQ_HAVE_DUMPSYS
#include "aiq_CamHwBaseInfo.h"
#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"
#include "dumpcam_server/third-party/argparse/argparse.h"
#include "info/aiq_hwiIspModsInfo.h"
#include "info/aiq_ispActiveParamsInfo.h"
#endif

XCAM_BEGIN_DECLARE

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#define container_of(ptr, type, member) ({              \
        void *__mptr = (void *)(ptr); \
        (type *)((char *)__mptr - offsetof(type,member));})

#endif

typedef struct CamHwInfoWraps_s {
    char key[32];
    rk_aiq_static_info_t data;
    struct CamHwInfoWraps_s* next;
} CamHwInfoWraps_t;

typedef struct FakeCamNmWraps_s {
    char key[32];
    char data[32];
    struct FakeCamNmWraps_s* next;
} FakeCamNmWraps_t;

static CamHwInfoWraps_t* g_mCamHwInfos = NULL;
rk_aiq_isp_hw_info_t g_mIspHwInfos;
static rk_aiq_cif_hw_info_t g_mCifHwInfos;
static rk_aiq_aiisp_hw_info_t g_mAiispHwInfos;
SnsFullInfoWraps_t* g_mSensorHwInfos        = NULL;
static FakeCamNmWraps_t* g_mFakeCameraName  = NULL;
bool g_mIsMultiIspMode               = false;
static uint16_t g_mMultiIspExtendedPixel    = 0;
// TODO: Sync 1608 sensor start streaming
static AiqMutex_t g_sync_1608_mutex;
static bool g_sync_1608_done = false;

sensor_info_share_t g_rk1608_share_inf;

static XCamReturn _setIspConfig(AiqCamHwBase_t* pCamHw, AiqList_t* result_list);
static XCamReturn _setFastAeExp(AiqCamHwBase_t* pCamHw, uint32_t frameId);
static XCamReturn _setIrcutParams(AiqCamHwBase_t* pCamHw, bool on);
static XCamReturn AiqCamHw_process_restriction(AiqCamHwBase_t* pCamHw,
        void* isp_params);
static XCamReturn AiqCamHw_setAiispMode(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_cfg* aiisp_cfg);
static XCamReturn AiqCamHw_read_aiisp_result(AiqCamHwBase_t* pCamHw, rk_aiq_aiisp_t* aiisp_evt);
static XCamReturn AiqCamHw_get_aiisp_bay3dbuf(AiqCamHwBase_t* pCamHw);
static XCamReturn AiqCamHw_aiisp_processing(AiqCamHwBase_t* pCamHw, rk_aiq_aiisp_t* aiisp_evt);
static XCamReturn SetLastAeExpToRttShared(AiqCamHwBase_t* pCamHw);
static XCamReturn _setCifSclStartFlag(AiqCamHwBase_t* pCamHw, int ratio, bool mode);
static XCamReturn AiqCamHw_set_sceneCisParams(AiqCamHwBase_t* pCamHw);

// dumpsys
static void AiqCamHwBase_initNotifier(AiqCamHwBase_t* pCamHw);
static int AiqCamHw_dump(void* pCamHw, st_string* result, int argc, void* argv[]);
static int __dump(void* dumper, st_string* result, int argc, void* argv[]);

#ifdef ISP_HW_V30
#define CAMHWISP_EFFECT_ISP_POOL_NUM 12
#elif defined(ISP_HW_V35)
#define CAMHWISP_EFFECT_ISP_POOL_NUM 8
#else
#define CAMHWISP_EFFECT_ISP_POOL_NUM 6
#endif

static XCamReturn _get_isp_ver(rk_aiq_isp_hw_info_t* hw_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct v4l2_capability cap;
    AiqV4l2Device_t vdev;
    int ret1 = AiqV4l2Device_init(&vdev, hw_info->isp_info[0].stats_path);

    if (ret1) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to open dev (%s)", hw_info->isp_info[0].stats_path);
        return XCAM_RETURN_ERROR_FAILED;
    }

    ret = AiqV4l2Device_open(&vdev, false);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to open dev (%s)", hw_info->isp_info[0].stats_path);
        AiqV4l2Device_deinit(&vdev);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (AiqV4l2Device_queryCap(&vdev, &cap) == XCAM_RETURN_NO_ERROR) {
        char* p;
        p = strrchr((char*)cap.driver, '_');
        if (p == NULL) {
            ret = XCAM_RETURN_ERROR_FAILED;
            goto out;
        }

        if (*(p + 1) != 'v') {
            ret = XCAM_RETURN_ERROR_FAILED;
            goto out;
        }

        hw_info->hw_ver_info.isp_ver = atoi(p + 2);
        // awb/aec version?
        ret = XCAM_RETURN_NO_ERROR;
        goto out;
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        goto out;
    }

out:
    AiqV4l2Device_close(&vdev);
    AiqV4l2Device_deinit(&vdev);
    if (ret) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get isp version failed !");
    return ret;
}

static XCamReturn _get_sensor_caps(rk_sensor_full_info_t* sensor_info) {
    struct v4l2_subdev_frame_size_enum fsize_enum;
    struct v4l2_subdev_mbus_code_enum code_enum;
    rk_frame_fmt_t frameSize;
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    struct rkmodule_inf* minfo = &(sensor_info->mod_info);

    AiqV4l2SubDevice_t vdev;
    int ret1 = AiqV4l2SubDevice_init(&vdev, sensor_info->device_name);
    if (ret1) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to open dev (%s)", sensor_info->device_name);
        return XCAM_RETURN_ERROR_FAILED;
    }

    ret = AiqV4l2SubDevice_open(&vdev, false);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to open dev (%s)", sensor_info->device_name);
        AiqV4l2SubDevice_deinit(&vdev);
        return XCAM_RETURN_ERROR_FAILED;
    }
    // get module info
    if (AiqV4l2SubDevice_ioctl(&vdev, RKMODULE_GET_MODULE_INFO, minfo) < 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "@%s %s: Get sensor module info failed", __FUNCTION__,
                        sensor_info->device_name);
        ret = XCAM_RETURN_ERROR_FAILED;
        goto out;
    }

    strncpy(sensor_info->len_name, minfo->base.lens, DEV_PATH_LEN);
    if (minfo->awb.flag && (minfo->awb.golden_r_value == 0 ||
        minfo->awb.golden_b_value == 0 ||
        minfo->awb.golden_gr_value == 0 ||
        minfo->awb.golden_gb_value == 0)) {
        minfo->awb.flag = 0;
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "@%s awb otp info is abnormal", __FUNCTION__);
    }

    {
        struct v4l2_subdev_frame_interval_enum fie;
        memset(&fie, 0, sizeof(fie));
        sensor_info->frame_size_cnt = 0;
        while (AiqV4l2SubDevice_ioctl(&vdev, VIDIOC_SUBDEV_ENUM_FRAME_INTERVAL, &fie) == 0) {
            frameSize.format   = (rk_aiq_format_t)fie.code;
            frameSize.width    = fie.width;
            frameSize.height   = fie.height;
            frameSize.fps      = fie.interval.denominator / fie.interval.numerator;
            frameSize.hdr_mode = fie.reserved[0];
            if (sensor_info->frame_size_cnt == SUPPORT_FMT_MAX) {
                LOGW_CAMHW ("fmts num over %d, ignore !", SUPPORT_FMT_MAX);
                break;
            }
            sensor_info->frame_size[sensor_info->frame_size_cnt++] = frameSize;
            fie.index++;
        }
        if (fie.index == 0) {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "@%s %s: Enum sensor frame interval failed", __FUNCTION__,
                            sensor_info->device_name);
        }
        struct rkmodule_capture_info cap_info;
        memset(&cap_info, 0, sizeof(cap_info));
        if (AiqV4l2SubDevice_ioctl(&vdev, RKMODULE_GET_CAPTURE_MODE, &cap_info) == 0) {
            if (cap_info.mode == RKMODULE_MULTI_CH_TO_MULTI_ISP) {
                sensor_info->linked_to_serdes = true;
                LOGK_CAMHW("%s is used as serdes", sensor_info->sensor_name);
            }
        }
    }
out:
    AiqV4l2SubDevice_close(&vdev);
    AiqV4l2SubDevice_deinit(&vdev);
    return ret;
}

static XCamReturn _parse_module_info(rk_sensor_full_info_t* sensor_info) {
    // sensor entity name format SHOULD be like this:
    // m00_b_ov13850 1-0010
    const char* entity_name = sensor_info->sensor_name;
    int parse_index         = 0;

    if (!entity_name) return XCAM_RETURN_ERROR_SENSOR;

    if (entity_name[parse_index] != 'm') {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name, parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    strncpy(sensor_info->module_index_str, entity_name + parse_index, 3);
    parse_index += 3;

    if (entity_name[parse_index] != '_') {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name, parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    parse_index++;

    if (entity_name[parse_index] != 'b' && entity_name[parse_index] != 'f') {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name, parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }
    sensor_info->phy_module_orient = entity_name[parse_index];

    parse_index++;

    if (entity_name[parse_index] != '_') {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name, parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    parse_index++;

    {
        char* real_name_end = strchr(entity_name, ' ');
        int size            = (int)(real_name_end - entity_name) - parse_index;

        if (real_name_end == NULL) {
            LOGE_CAMHW_SUBM(
                ISP20HW_SUBM,
                "%d:parse sensor entity name %s error at %d, please check sensor driver !",
                __LINE__, entity_name, parse_index);
            return XCAM_RETURN_ERROR_SENSOR;
        }

        strncpy(sensor_info->module_real_sensor_name, entity_name + parse_index, size);

        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s:%d, real sensor name %s, module ori %c, module id %s",
                        __FUNCTION__, __LINE__, sensor_info->module_real_sensor_name,
                        sensor_info->phy_module_orient, sensor_info->module_index_str);
    }
    return XCAM_RETURN_NO_ERROR;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

static rk_aiq_isp_t* _get_isp_subdevs(struct media_device* device, const char* devpath,
                                      rk_aiq_isp_t* isp_info) {
    struct media_entity* entity = NULL;
    const char* entity_name = NULL;
    int index               = 0;
    if (!device || !isp_info || !devpath) return NULL;

    for (index = 0; index < MAX_CAM_NUM; index++) {
        if (0 == strlen(isp_info[index].media_dev_path)) {
            isp_info[index].logic_id = index;
            break;
        }
        if (0 == strncmp(isp_info[index].media_dev_path, devpath,
                         sizeof(isp_info[index].media_dev_path))) {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp info of path %s exists!", devpath);
            return &isp_info[index];
        }
    }
    if (index >= MAX_CAM_NUM) return NULL;

#if defined(ISP_HW_V30)
    // parse driver pattern: soc:rkisp0-vir0
    int model_idx = -1;
    char* rkisp   = strstr(device->info.driver, "rkisp");
    if (rkisp) {
        char* str_unite = NULL;
        str_unite       = strstr(device->info.driver, "unite");
        if (str_unite) {
            model_idx = 0;
        } else {
            int isp_idx = atoi(rkisp + strlen("rkisp"));
            char* vir   = strstr(device->info.driver, "vir");
            if (vir) {
                int vir_idx            = atoi(vir + strlen("vir"));
                model_idx              = isp_idx * 4 + vir_idx;
                isp_info[index].phy_id = isp_idx;
            }
        }

        strcpy(isp_info[index].driver, device->info.driver);
    }

    if (model_idx == -1) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "wrong isp media driver info: %s", device->info.driver);
        return NULL;
    }

    isp_info[index].model_idx = model_idx;
#else
    if (strcmp(device->info.model, "rkisp0") == 0 || strcmp(device->info.model, "rkisp") == 0)
        isp_info[index].model_idx = 0;
    else if (strcmp(device->info.model, "rkisp1") == 0)
        isp_info[index].model_idx = 1;
    else if (strcmp(device->info.model, "rkisp2") == 0)
        isp_info[index].model_idx = 2;
    else if (strcmp(device->info.model, "rkisp3") == 0)
        isp_info[index].model_idx = 3;
    else if (strcmp(device->info.model, "rkisp4") == 0)
        isp_info[index].model_idx = 4;
    else
        isp_info[index].model_idx               = -1;

    strcpy(isp_info[index].driver, device->info.driver);
#endif

    strncpy(isp_info[index].media_dev_path, devpath, sizeof(isp_info[index].media_dev_path));

    entity = media_get_entity_by_name(device, "rkisp-isp-subdev", strlen("rkisp-isp-subdev"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].isp_dev_path, entity_name,
                    sizeof(isp_info[index].isp_dev_path));
        }
    }

    entity = media_get_entity_by_name(device, "rkisp-csi-subdev", strlen("rkisp-csi-subdev"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].csi_dev_path, entity_name,
                    sizeof(isp_info[index].csi_dev_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-mpfbc-subdev", strlen("rkisp-mpfbc-subdev"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].mpfbc_dev_path, entity_name,
                    sizeof(isp_info[index].mpfbc_dev_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_mainpath", strlen("rkisp_mainpath"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].main_path, entity_name, sizeof(isp_info[index].main_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_selfpath", strlen("rkisp_selfpath"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].self_path, entity_name, sizeof(isp_info[index].self_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_ldcpath", strlen("rkisp_ldcpath"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].ldc_path, entity_name, sizeof(isp_info[index].ldc_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawwr0", strlen("rkisp_rawwr0"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].rawwr0_path, entity_name, sizeof(isp_info[index].rawwr0_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawwr1", strlen("rkisp_rawwr1"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].rawwr1_path, entity_name, sizeof(isp_info[index].rawwr1_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawwr2", strlen("rkisp_rawwr2"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].rawwr2_path, entity_name, sizeof(isp_info[index].rawwr2_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawwr3", strlen("rkisp_rawwr3"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].rawwr3_path, entity_name, sizeof(isp_info[index].rawwr3_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_dmapath", strlen("rkisp_dmapath"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].dma_path, entity_name, sizeof(isp_info[index].dma_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawrd0_m", strlen("rkisp_rawrd0_m"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].rawrd0_m_path, entity_name,
                    sizeof(isp_info[index].rawrd0_m_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawrd1_l", strlen("rkisp_rawrd1_l"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].rawrd1_l_path, entity_name,
                    sizeof(isp_info[index].rawrd1_l_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawrd2_s", strlen("rkisp_rawrd2_s"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].rawrd2_s_path, entity_name,
                    sizeof(isp_info[index].rawrd2_s_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-statistics", strlen("rkisp-statistics"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].stats_path, entity_name, sizeof(isp_info[index].stats_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-input-params", strlen("rkisp-input-params"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].input_params_path, entity_name,
                    sizeof(isp_info[index].input_params_path));
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-mipi-luma", strlen("rkisp-mipi-luma"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].mipi_luma_path, entity_name,
                    sizeof(isp_info[index].mipi_luma_path));
        }
    }
    entity =
        media_get_entity_by_name(device, "rockchip-mipi-dphy-rx", strlen("rockchip-mipi-dphy-rx"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].mipi_dphy_rx_path, entity_name,
                    sizeof(isp_info[index].mipi_dphy_rx_path));
        }
    } else {
        entity =
            media_get_entity_by_name(device, "rockchip-csi2-dphy0", strlen("rockchip-csi2-dphy0"));
        if (entity) {
            entity_name = media_entity_get_devname(entity);
            if (entity_name) {
                strncpy(isp_info[index].mipi_dphy_rx_path, entity_name,
                        sizeof(isp_info[index].mipi_dphy_rx_path));
            }
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-pdaf", strlen("rkisp-pdaf"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(isp_info[index].pdaf_path, entity_name, sizeof(isp_info[index].pdaf_path));
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_dvp", strlen("rkcif_dvp"));
    if (entity)
        isp_info[index].linked_dvp = true;
    else
        isp_info[index].linked_dvp = false;

    entity = media_get_entity_by_name(device, "rkcif-dvp", strlen("rkcif-dvp"));
    if (entity)
        isp_info[index].linked_dvp = true;
    else
        isp_info[index].linked_dvp = false;

    {
        const char* linked_entity_name_strs[] = {
            "rkcif-dvp",        "rkcif_dvp",        "rkcif_lite_mipi_lvds", "rkcif_mipi_lvds",
            "rkcif_mipi_lvds1", "rkcif_mipi_lvds2", "rkcif_mipi_lvds3",     "rkcif_mipi_lvds4",
            "rkcif_mipi_lvds5", "rkcif-mipi-lvds",  "rkcif-mipi-lvds1",     "rkcif-mipi-lvds2",
            "rkcif-mipi-lvds3", "rkcif-mipi-lvds4", "rkcif-mipi-lvds5",     NULL
        };

        int vicap_idx = 0;
        for (int i = 0; linked_entity_name_strs[i] != NULL; i++) {
            entity = media_get_entity_by_name(device, linked_entity_name_strs[i],
                                              strlen(linked_entity_name_strs[i]));
            if (entity) {
                strncpy(isp_info[index].linked_vicap[vicap_idx], entity->info.name,
                        sizeof(isp_info[index].linked_vicap[vicap_idx]));

                entity_name = media_entity_get_devname(entity);
                if (entity_name) {
                    strncpy(isp_info[index].linked_vicap_sd_path, entity_name,
                            sizeof(isp_info[index].linked_vicap_sd_path));
                }

                if (vicap_idx++ >= MAX_ISP_LINKED_VICAP_CNT) {
                    break;
                }
            }
        }
    }

    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "model(%s): isp_info(%d): ispp-subdev entity name: %s\n",
                    device->info.model, index, isp_info[index].isp_dev_path);

    return &isp_info[index];
}

static rk_aiq_cif_info_t* _get_cif_subdevs(struct media_device* device, const char* devpath,
        rk_aiq_cif_info_t* cif_info) {
    struct media_entity* entity = NULL;
    const char* entity_name = NULL;
    int index               = 0;
    if (!device || !devpath || !cif_info) return NULL;

    for (index = 0; index < MAX_CAM_NUM; index++) {
        if (0 == strlen(cif_info[index].media_dev_path)) break;
        if (0 == strncmp(cif_info[index].media_dev_path, devpath,
                         sizeof(cif_info[index].media_dev_path))) {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp info of path %s exists!", devpath);
            return &cif_info[index];
        }
    }
    if (index >= MAX_CAM_NUM) return NULL;

    cif_info[index].model_idx = index;

    strncpy(cif_info[index].media_dev_path, devpath, sizeof(cif_info[index].media_dev_path) - 1);

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id0", strlen("stream_cif_mipi_id0"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_id0, entity_name, sizeof(cif_info[index].mipi_id0) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id1", strlen("stream_cif_mipi_id1"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_id1, entity_name, sizeof(cif_info[index].mipi_id1) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id2", strlen("stream_cif_mipi_id2"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_id2, entity_name, sizeof(cif_info[index].mipi_id2) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id3", strlen("stream_cif_mipi_id3"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_id3, entity_name, sizeof(cif_info[index].mipi_id3) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_scale_ch0", strlen("rkcif_scale_ch0"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_scl0, entity_name, sizeof(cif_info[index].mipi_scl0) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_scale_ch1", strlen("rkcif_scale_ch1"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_scl1, entity_name, sizeof(cif_info[index].mipi_scl1) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_scale_ch2", strlen("rkcif_scale_ch2"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_scl2, entity_name, sizeof(cif_info[index].mipi_scl2) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_scale_ch3", strlen("rkcif_scale_ch3"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_scl3, entity_name, sizeof(cif_info[index].mipi_scl3) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_dvp_id0", strlen("stream_cif_dvp_id0"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].dvp_id0, entity_name, sizeof(cif_info[index].dvp_id0) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_dvp_id1", strlen("stream_cif_dvp_id1"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].dvp_id1, entity_name, sizeof(cif_info[index].dvp_id1) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_dvp_id2", strlen("stream_cif_dvp_id2"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].dvp_id2, entity_name, sizeof(cif_info[index].dvp_id2) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_dvp_id3", strlen("stream_cif_dvp_id3"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].dvp_id3, entity_name, sizeof(cif_info[index].dvp_id3) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif-mipi-luma", strlen("rkisp-mipi-luma"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_luma_path, entity_name,
                    sizeof(cif_info[index].mipi_luma_path) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rockchip-mipi-csi2", strlen("rockchip-mipi-csi2"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_csi2_sd_path, entity_name,
                    sizeof(cif_info[index].mipi_csi2_sd_path) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif-lvds-subdev", strlen("rkcif-lvds-subdev"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].lvds_sd_path, entity_name,
                    sizeof(cif_info[index].lvds_sd_path) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif-lite-lvds-subdev",
                                      strlen("rkcif-lite-lvds-subdev"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].lvds_sd_path, entity_name,
                    sizeof(cif_info[index].lvds_sd_path) - 1);
        }
    }

    entity =
        media_get_entity_by_name(device, "rockchip-mipi-dphy-rx", strlen("rockchip-mipi-dphy-rx"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].mipi_dphy_rx_path, entity_name,
                    sizeof(cif_info[index].mipi_dphy_rx_path) - 1);
        }
    } else {
        entity =
            media_get_entity_by_name(device, "rockchip-csi2-dphy0", strlen("rockchip-csi2-dphy0"));
        if (entity) {
            entity_name = media_entity_get_devname(entity);
            if (entity_name) {
                strncpy(cif_info[index].mipi_dphy_rx_path, entity_name,
                        sizeof(cif_info[index].mipi_dphy_rx_path) - 1);
            }
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif", strlen("stream_cif"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].stream_cif_path, entity_name,
                    sizeof(cif_info[index].stream_cif_path) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif-dvp-sof", strlen("rkcif-dvp-sof"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(cif_info[index].dvp_sof_sd_path, entity_name,
                    sizeof(cif_info[index].dvp_sof_sd_path) - 1);
        }
    }

    return &cif_info[index];
}

static rk_aiq_aiisp_info_t* _get_aiisp_subdevs(struct media_device* device, const char* devpath,
        rk_aiq_aiisp_info_t* aiisp_info) {
    struct media_entity* entity = NULL;
    const char* entity_name = NULL;
    int index               = 0;
    if (!device || !devpath || !aiisp_info) return NULL;

    for (index = 0; index < MAX_CAM_NUM; index++) {
        if (0 == strlen(aiisp_info[index].media_dev_path)) break;
        if (0 == strncmp(aiisp_info[index].media_dev_path, devpath,
                         sizeof(aiisp_info[index].media_dev_path))) {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp info of path %s exists!", devpath);
            return &aiisp_info[index];
        }
    }
    if (index >= MAX_CAM_NUM) return NULL;

    if (strcmp(device->info.model, "rkaiisp0") == 0)
        aiisp_info[index].model_idx = 0;
    else if (strcmp(device->info.model, "rkaiisp1") == 0)
        aiisp_info[index].model_idx = 1;
    else if (strcmp(device->info.model, "rkaiisp2") == 0)
        aiisp_info[index].model_idx = 2;
    else if (strcmp(device->info.model, "rkaiisp3") == 0)
        aiisp_info[index].model_idx = 3;
    else if (strcmp(device->info.model, "rkaiisp4") == 0)
        aiisp_info[index].model_idx = 4;
    else if (strcmp(device->info.model, "rkaiisp5") == 0)
        aiisp_info[index].model_idx = 5;
    else if (strcmp(device->info.model, "rkaiisp6") == 0)
        aiisp_info[index].model_idx = 6;
    else if (strcmp(device->info.model, "rkaiisp7") == 0)
        aiisp_info[index].model_idx = 7;
    else
        aiisp_info[index].model_idx = -1;

    strncpy(aiisp_info[index].media_dev_path, devpath, sizeof(aiisp_info[index].media_dev_path) - 1);

    entity = media_get_entity_by_name(device, "rkaiisp", strlen("rkaiisp"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(aiisp_info[index].video_path, entity_name, sizeof(aiisp_info[index].video_path) - 1);
        }
    }
    entity = media_get_entity_by_name(device, "rkaiisp-subdev", strlen("rkaiisp-subdev"));
    if (entity) {
        entity_name = media_entity_get_devname(entity);
        if (entity_name) {
            strncpy(aiisp_info[index].subdev_path, entity_name, sizeof(aiisp_info[index].subdev_path) - 1);
        }
    }
    LOGD_AIBNR("%s: model_idx %d, media_dev_path %s, video_path %s, subdev_path %s", __func__,
               aiisp_info[index].model_idx, aiisp_info[index].media_dev_path,
               aiisp_info[index].video_path, aiisp_info[index].subdev_path);
    return &aiisp_info[index];
}


static XCamReturn _SensorInfoCopy(rk_sensor_full_info_t* finfo, rk_aiq_static_info_t* info) {
    int fs_num, i = 0;
    rk_aiq_sensor_info_t* sinfo = NULL;

    // info->media_node_index = finfo->media_node_index;
    strncpy(info->lens_info.len_name, finfo->len_name, DEV_PATH_LEN);
    sinfo = &info->sensor_info;
    strncpy(sinfo->sensor_name, finfo->sensor_name, DEV_PATH_LEN);
    fs_num     = finfo->frame_size_cnt;
    sinfo->num = fs_num;
    memcpy(sinfo->support_fmt, finfo->frame_size, fs_num * sizeof(rk_frame_fmt_t));

    if (strlen(finfo->module_index_str) > 0) {
        sinfo->phyId = atoi(finfo->module_index_str + 1);
    } else {
        sinfo->phyId = -1;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamHw_selectIqFile(const char* sns_ent_name, char* iqfile_name) {
    const struct rkmodule_base_inf* base_inf = NULL;
    const char *sensor_name, *module_name, *lens_name;
    char sensor_name_full[32];
    rk_sensor_full_info_t* pSnsHwInfo = NULL;
    SnsFullInfoWraps_t* pSnsInfoWrap  = NULL;

    if (!sns_ent_name || !iqfile_name) return XCAM_RETURN_ERROR_SENSOR;

    pSnsInfoWrap = g_mSensorHwInfos;
    while (pSnsInfoWrap) {
        if (strcmp(sns_ent_name, pSnsInfoWrap->key) == 0) break;
        pSnsInfoWrap = pSnsInfoWrap->next;
    }

    if (!pSnsInfoWrap) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", sns_ent_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    pSnsHwInfo = &pSnsInfoWrap->data;

    base_inf = &(pSnsHwInfo->mod_info.base);
    if (!strlen(base_inf->module) || !strlen(base_inf->sensor) || !strlen(base_inf->lens)) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "no camera module info, check the drv !");
        return XCAM_RETURN_ERROR_SENSOR;
    }
    sensor_name = base_inf->sensor;
    strncpy(sensor_name_full, sensor_name, 32);

    module_name = base_inf->module;
    lens_name   = base_inf->lens;
    if (strlen(module_name) && strlen(lens_name)) {
        sprintf(iqfile_name, "%s_%s_%s.json", sensor_name_full, module_name, lens_name);
    } else {
        sprintf(iqfile_name, "%s.json", sensor_name_full);
    }

    return XCAM_RETURN_NO_ERROR;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

rk_aiq_static_info_t* AiqCamHw_getStaticCamHwInfoByPhyId(const char* sns_ent_name, uint16_t index) {
    CamHwInfoWraps_t* pCamHwInfoWraps = g_mCamHwInfos;

    if (sns_ent_name) {
        while (pCamHwInfoWraps) {
            if (strcmp(sns_ent_name, pCamHwInfoWraps->key) == 0) break;
            pCamHwInfoWraps = pCamHwInfoWraps->next;
        }
    } else {
        char index_str[32] = {'\0'};

        if (index < 10)
            snprintf(index_str, sizeof(index_str), "%s%d", "m0", index);
        else
            snprintf(index_str, sizeof(index_str), "%s%d", "m", index);

        while (pCamHwInfoWraps) {
            if (strncmp(index_str, pCamHwInfoWraps->key, 3) == 0) break;
            pCamHwInfoWraps = pCamHwInfoWraps->next;
        }
    }

    if (pCamHwInfoWraps) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "find camerainfo of %s!", sns_ent_name);
        return &pCamHwInfoWraps->data;
    } else {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "camerainfo of %s not found!", sns_ent_name);
    }

    return NULL;
}

rk_aiq_static_info_t* AiqCamHw_getStaticCamHwInfo(const char* sns_ent_name, uint16_t index) {
    CamHwInfoWraps_t* pCamHwInfoWraps = g_mCamHwInfos;

    if (sns_ent_name) {
        while (pCamHwInfoWraps) {
            if (strcmp(sns_ent_name, pCamHwInfoWraps->key) == 0) break;
            pCamHwInfoWraps = pCamHwInfoWraps->next;
        }
    } else {
        int i = 0;
        while (pCamHwInfoWraps) {
            if (i++ == index) break;
            pCamHwInfoWraps = pCamHwInfoWraps->next;
        }
    }

    if (pCamHwInfoWraps) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "find camerainfo of %s!", pCamHwInfoWraps->key);
        return &pCamHwInfoWraps->data;
    } else {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "camerainfo of %s not found!", sns_ent_name);
    }

    return NULL;
}

XCamReturn AiqCamHw_clearStaticCamHwInfo() {
    /* std::map<std::string, SmartPtr<rk_aiq_static_info_t>>::iterator it1; */
    /* std::map<std::string, SmartPtr<rk_sensor_full_info_t>>::iterator it2; */

    /* for (it1 = mCamHwInfos.begin(); it1 != mCamHwInfos.end(); it1++) { */
    /*     rk_aiq_static_info_t *ptr = it1->second.ptr(); */
    /*     delete ptr; */
    /* } */
    /* for (it2 = mSensorHwInfos.begin(); it2 != mSensorHwInfos.end(); it2++) { */
    /*     rk_sensor_full_info_t *ptr = it2->second.ptr(); */
    /*     delete ptr; */
    /* } */
    {
        CamHwInfoWraps_t* pCamHwInfoWraps     = g_mCamHwInfos;
        CamHwInfoWraps_t* pCamHwInfoWrapsNext = NULL;

        while (pCamHwInfoWraps) {
            pCamHwInfoWrapsNext = pCamHwInfoWraps->next;
            aiq_free(pCamHwInfoWraps);
            pCamHwInfoWraps = pCamHwInfoWrapsNext;
        }
        g_mCamHwInfos = NULL;
    }

    {
        SnsFullInfoWraps_t* pSnsInfoWrap     = g_mSensorHwInfos;
        SnsFullInfoWraps_t* pSnsInfoWrapNext = NULL;

        while (pSnsInfoWrap) {
            pSnsInfoWrapNext = pSnsInfoWrap->next;
            aiq_free(pSnsInfoWrap);
            pSnsInfoWrap = pSnsInfoWrapNext;
        }
        g_mSensorHwInfos = NULL;
    }

    {
        FakeCamNmWraps_t* pFakeCamNmWrap     = g_mFakeCameraName;
        FakeCamNmWraps_t* pFakeCamNmWrapNext = NULL;

        while (pFakeCamNmWrap) {
            pFakeCamNmWrapNext = pFakeCamNmWrap->next;
            aiq_free(pFakeCamNmWrap);
            pFakeCamNmWrap = pFakeCamNmWrapNext;
        }
        g_mFakeCameraName = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}

static void _findAttachedSubdevs(struct media_device* device, uint32_t count,
                                 rk_sensor_full_info_t* s_info) {
    const struct media_entity_desc* entity_info = NULL;
    struct media_entity* entity                 = NULL;
    uint32_t k;

    for (k = 0; k < count; ++k) {
        entity      = media_get_entity(device, k);
        entity_info = media_entity_get_info(entity);
        if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_LENS)) {
            if ((entity_info->name[0] == 'm') &&
                    (strncmp(entity_info->name, s_info->module_index_str, 3) == 0)) {
                if (entity_info->flags == 1)
                    strcpy(s_info->module_ircut_dev_name, media_entity_get_devname(entity));
                else  // vcm
                    strcpy(s_info->module_lens_dev_name, media_entity_get_devname(entity));
            }
        } else if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_FLASH)) {
            if ((entity_info->name[0] == 'm') &&
                    (strncmp(entity_info->name, s_info->module_index_str, 3) == 0)) {
                /* check if entity name has the format string mxx_x_xxx-irxxx */
                if (strstr(entity_info->name, "-ir") != NULL) {
                    strcpy(s_info->module_flash_ir_dev_name[s_info->flash_ir_num++],
                           media_entity_get_devname(entity));
                } else
                    strcpy(s_info->module_flash_dev_name[s_info->flash_num++],
                           media_entity_get_devname(entity));
            }
        }
    }
}

XCamReturn AiqCamHw_initCamHwInfos() {
    char sys_path[64], devpath[32];
    FILE* fp                    = NULL;
    struct media_device* device = NULL;
    int nents, j = 0, i = 0, node_index = 0, max_phyId = -1;
    const struct media_entity_desc* entity_info = NULL;
    struct media_entity* entity                 = NULL;

    xcam_mem_clear(g_mIspHwInfos);
    xcam_mem_clear(g_mCifHwInfos);

    while (i < MAX_MEDIA_INDEX) {
        node_index = i;
        snprintf(sys_path, 64, "/dev/media%d", i++);
        fp = fopen(sys_path, "r");
        if (!fp) continue;
        fclose(fp);
        device = media_device_new(sys_path);
        if (!device) {
            continue;
        }

        /* Enumerate entities, pads and links. */
        media_device_enumerate(device);

        rk_aiq_isp_t* isp_info      = NULL;
        rk_aiq_cif_info_t* cif_info = NULL;
        rk_aiq_aiisp_info_t* aiisp_info = NULL;
        bool dvp_itf                = false;
        if (strcmp(device->info.model, "rkispp0") == 0 ||
                strcmp(device->info.model, "rkispp1") == 0 ||
                strcmp(device->info.model, "rkispp2") == 0 ||
                strcmp(device->info.model, "rkispp3") == 0 ||
                strcmp(device->info.model, "rkispp") == 0) {
#if defined(ISP_HW_V20)
            rk_aiq_ispp_t* ispp_info = get_ispp_subdevs(device, sys_path, g_mIspHwInfos.ispp_info);
            if (ispp_info) ispp_info->valid = true;
#endif
            goto media_unref;
        } else if (strcmp(device->info.model, "rkisp0") == 0 ||
                   strcmp(device->info.model, "rkisp1") == 0 ||
                   strcmp(device->info.model, "rkisp2") == 0 ||
                   strcmp(device->info.model, "rkisp3") == 0 ||
                   strcmp(device->info.model, "rkisp4") == 0 ||
                   strcmp(device->info.model, "rkisp") == 0) {
            isp_info = _get_isp_subdevs(device, sys_path, g_mIspHwInfos.isp_info);
            if (strstr(device->info.driver, "rkisp-unite")) {
                isp_info->is_multi_isp_mode = true;
                g_mIsMultiIspMode           = true;
                g_mMultiIspExtendedPixel    = RKMOUDLE_UNITE_EXTEND_PIXEL;
            } else {
                isp_info->is_multi_isp_mode = false;
                g_mIsMultiIspMode           = false;
                g_mMultiIspExtendedPixel    = 0;
            }
            isp_info->valid = true;
        } else if (strcmp(device->info.model, "rkcif") == 0 ||
                   strcmp(device->info.model, "rkcif-dvp") == 0 ||
                   strcmp(device->info.model, "rkcif_dvp") == 0 ||
                   strstr(device->info.model, "rkcif_mipi_lvds") ||
                   strstr(device->info.model, "rkcif-mipi-lvds") ||
                   strcmp(device->info.model, "rkcif_lite_mipi_lvds") == 0) {
            cif_info = _get_cif_subdevs(device, sys_path, g_mCifHwInfos.cif_info);
            strncpy(cif_info->model_str, device->info.model, sizeof(cif_info->model_str));

            if (strcmp(device->info.model, "rkcif_dvp") == 0 ||
                    strcmp(device->info.model, "rkcif-dvp") == 0)
                dvp_itf = true;
        } else if (strncmp(device->info.model, "rkaiisp", sizeof("rkaiisp") - 1) == 0) {
            aiisp_info = _get_aiisp_subdevs(device, sys_path, g_mAiispHwInfos.aiisp_info);
            if (aiisp_info) {
                strncpy(aiisp_info->model_str, device->info.model, sizeof(aiisp_info->model_str));
                aiisp_info->valid = true;
            }
        } else {
            goto media_unref;
        }

        nents = media_get_entities_count(device);
        for (j = 0; j < nents; ++j) {
            entity      = media_get_entity(device, j);
            entity_info = media_entity_get_info(entity);
            if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR)) {
                CamHwInfoWraps_t* info_wrap          = NULL;
                SnsFullInfoWraps_t* s_full_info_wrap = NULL;
                rk_aiq_static_info_t* info           = NULL;
                rk_sensor_full_info_t* s_full_info   = NULL;

                if (strstr(entity_info->name, "1608")) {
                    // [baron] skip psy-sensor(m_09_RK1608Dphy), save cif inf addr.
                    if (cif_info) {
                        g_rk1608_share_inf.reference_mipi_cif = cif_info;
                    }
                    continue;
                }

                info_wrap        = (CamHwInfoWraps_t*)aiq_mallocz(sizeof(CamHwInfoWraps_t));
                s_full_info_wrap = (SnsFullInfoWraps_t*)aiq_mallocz(sizeof(SnsFullInfoWraps_t));
                info                          = &info_wrap->data;
                s_full_info                   = &s_full_info_wrap->data;
                strcpy(s_full_info_wrap->key, entity_info->name);
                strcpy(info_wrap->key, entity_info->name);
                s_full_info->media_node_index = node_index;
                strncpy(devpath, media_entity_get_devname(entity), sizeof(devpath) - 1);
                devpath[sizeof(devpath) - 1] = '\0';
                strcpy(s_full_info->device_name, devpath);
                strcpy(s_full_info->sensor_name, entity_info->name);
                strcpy(s_full_info->parent_media_dev, sys_path);
                _parse_module_info(s_full_info);
                _get_sensor_caps(s_full_info);

                if (cif_info) {
                    s_full_info->linked_to_isp = false;
                    s_full_info->cif_info      = cif_info;
                    s_full_info->isp_info      = NULL;
                    s_full_info->dvp_itf       = dvp_itf;
                    // [baron] add flag for 1608 sensor
                    s_full_info->linked_to_1608 = false;
                    info->_is_1608_sensor       = false;
                } else if (isp_info) {
                    s_full_info->linked_to_isp = true;
                    isp_info->linked_sensor    = true;
                    isp_info->isMultiplex      = false;
                    s_full_info->isp_info      = isp_info;
#if defined(ISP_HW_V30)
                    // FIXME: Just support isp3x(rk3588-8/9camera).
                    for (int vi_idx = 0; vi_idx < MAX_ISP_LINKED_VICAP_CNT; vi_idx++) {
                        if (!g_rk1608_share_inf.reference_mipi_cif) break;

                        if (strcmp(g_rk1608_share_inf.reference_mipi_cif->model_str,
                                   isp_info->linked_vicap[vi_idx]) != 0)
                            continue;

                        if (strlen(isp_info->linked_vicap[vi_idx]) > 0) {
                            strcpy(g_rk1608_share_inf.reference_name,
                                   isp_info->linked_vicap[vi_idx]);
                            s_full_info->cif_info       = g_rk1608_share_inf.reference_mipi_cif;
                            info->_is_1608_sensor       = true;
                            s_full_info->linked_to_1608 = true;
                        }
                    }
#endif
                } else {
                    LOGE_CAMHW_SUBM(ISP20HW_SUBM, "sensor device mount error!\n");
                }

#if 0
                printf("  >>>>>>>>> sensor(%s): cif addr(%p), link_to_1608[%d], share_vi(%s)\n",
                       entity_info->name,
                       s_full_info->cif_info,
                       s_full_info->linked_to_1608,
                       g_rk1608_share_inf.reference_name
                      );
#endif

                _findAttachedSubdevs(device, nents, s_full_info);
                _SensorInfoCopy(s_full_info, info);
                info->has_lens_vcm     = strlen(s_full_info->module_lens_dev_name) ? false : true;
                info->has_fl           = s_full_info->flash_num > 0 ? true : false;
                info->has_irc          = strlen(s_full_info->module_ircut_dev_name) ? false : true;
                info->fl_strth_adj_sup = s_full_info->fl_ir_strth_adj_sup;
                info->fl_ir_strth_adj_sup = s_full_info->fl_ir_strth_adj_sup;
                if (s_full_info->isp_info)
                    info->is_multi_isp_mode = s_full_info->isp_info->is_multi_isp_mode;
                info->multi_isp_extended_pixel = g_mMultiIspExtendedPixel;
                LOGD_CAMHW_SUBM(ISP20HW_SUBM,
                                "Init sensor %s with Multi-ISP Mode:%d Extended Pixels:%d ",
                                s_full_info->sensor_name, info->is_multi_isp_mode,
                                info->multi_isp_extended_pixel);

                if (!g_mSensorHwInfos) {
                    g_mSensorHwInfos = s_full_info_wrap;
                } else {
                    SnsFullInfoWraps_t* pItem = g_mSensorHwInfos;
                    while (pItem->next) {
                        pItem = pItem->next;
                    }
                    pItem->next = s_full_info_wrap;
                }

                if (!g_mCamHwInfos) {
                    g_mCamHwInfos = info_wrap;
                } else {
                    CamHwInfoWraps_t* pItem = g_mCamHwInfos;
                    while (pItem->next) {
                        pItem = pItem->next;
                    }
                    pItem->next = info_wrap;
                }
                if (info->sensor_info.phyId > max_phyId) {
                    max_phyId = info->sensor_info.phyId;
                }
            }
        }

media_unref:
        media_device_unref(device);
    }

#if defined(ISP_HW_V30)
    // judge isp if multiplex by multiple cams
    {
        rk_aiq_isp_t* isp_info = NULL;
        for (i = 0; i < MAX_CAM_NUM; i++) {
            isp_info = g_mIspHwInfos.isp_info[i];
            if (isp_info->valid) {
                for (j = i - 1; j >= 0; j--) {
                    if (isp_info->phy_id == g_mIspHwInfos.isp_info[j].phy_id) {
                        isp_info->isMultiplex                 = true;
                        g_mIspHwInfos.isp_info[j].isMultiplex = true;
                    }
                }
            }
        }
    }
#endif

    {
        SnsFullInfoWraps_t* tempSensorHwInfos = NULL;
        SnsFullInfoWraps_t* iter = g_mSensorHwInfos;

        {
            // check no repeated sensor phyId, m00 -> phyId = 0, m01 -> phyId = 1
            char sns_idx_str[4];
            char sns_name_info[MAX_CAM_NUM][64];
            int  sns_offset = 0;
            int  sns_num    = 0;
            for (int sns_idx = 0; sns_idx < MAX_CAM_NUM; sns_idx++) {
                sprintf(sns_idx_str, "m0%d", sns_idx);
                sns_num    = 0;
                sns_offset = 0;
                for (iter = g_mSensorHwInfos; iter != NULL; iter = iter->next) {
                    rk_sensor_full_info_t* s_full_info = &iter->data;
                    CamHwInfoWraps_t* pItem = g_mCamHwInfos;
                    while (pItem) {
                        if (strcmp(s_full_info->sensor_name, pItem->key) == 0) break;
                        pItem = pItem->next;
                    }
                    if (strcmp(s_full_info->module_index_str, sns_idx_str) == 0) {
                        snprintf(sns_name_info[sns_num], 64, "%s(%s phyId: %d)", sns_idx_str, s_full_info->sensor_name, pItem->data.sensor_info.phyId);
                        sns_num++;
                    }
                }
                if (sns_num > 1) {
                    LOGE_CAMHW("sensor index %d is more than one, maybe cause some unkown error, modify dts to fix", sns_idx);
                    for (int sns_num_idx = 0; sns_num_idx < sns_num; sns_num_idx++)
                        LOGE_CAMHW("%s", sns_name_info[sns_num_idx]);
                }
            }
        }

        for (iter = g_mSensorHwInfos; iter != NULL; iter = iter->next) {
            rk_sensor_full_info_t* s_full_info = &iter->data;
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "match the sensor_name(%s) media link\n",
                            iter->key);

            /*
             * The ISP and ISPP match links through the media device model
             */
            if (s_full_info->linked_to_isp) {
                for (i = 0; i < MAX_CAM_NUM; i++) {
                    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "isp model_idx: %d, ispp(%d) model_idx: %d\n",
                                    s_full_info->isp_info->model_idx, i,
                                    g_mIspHwInfos.ispp_info[i].model_idx);

                    if (g_mIspHwInfos.ispp_info[i].valid &&
                            (s_full_info->isp_info->model_idx ==
                             g_mIspHwInfos.ispp_info[i].model_idx)) {
                        CamHwInfoWraps_t* pItem = g_mCamHwInfos;
                        s_full_info->ispp_info  = &g_mIspHwInfos.ispp_info[i];
                        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "isp(%d) link to ispp(%d)\n",
                                        s_full_info->isp_info->model_idx,
                                        g_mIspHwInfos.ispp_info[i].model_idx);

                        while (pItem) {
                            if (strcmp(s_full_info->sensor_name, pItem->key) == 0) break;
                            pItem = pItem->next;
                        }

                        if (pItem) {
                            pItem->data.sensor_info.binded_strm_media_idx =
                                atoi(s_full_info->ispp_info->media_dev_path + strlen("/dev/media"));
                            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "sensor %s adapted to pp media %d:%s\n",
                                            s_full_info->sensor_name,
                                            pItem->data.sensor_info.binded_strm_media_idx,
                                            s_full_info->ispp_info->media_dev_path);
                        }
                        break;
                    }
                }
            } else {
                /*
                 * Determine which isp that vipCap is linked
                 */
                int vicap_linked_to_isp_nums = 0;
                for (i = 0; i < MAX_CAM_NUM; i++) {
                    if (s_full_info->linked_to_serdes)
                        break;
                    rk_aiq_isp_t* isp_info = &g_mIspHwInfos.isp_info[i];

                    for (int vicap_idx = 0; vicap_idx < MAX_ISP_LINKED_VICAP_CNT; vicap_idx++) {
                        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "vicap %s, linked_vicap %s",
                                        s_full_info->cif_info->model_str,
                                        isp_info->linked_vicap[vicap_idx]);
                        if (strcmp(s_full_info->cif_info->model_str,
                                   isp_info->linked_vicap[vicap_idx]) == 0) {
                            if (vicap_linked_to_isp_nums == 0) {
                                s_full_info->isp_info   = &g_mIspHwInfos.isp_info[i];
                                CamHwInfoWraps_t* pItem = g_mCamHwInfos;

                                while (pItem) {
                                    if (strcmp(s_full_info->sensor_name, pItem->key) == 0) break;
                                    pItem = pItem->next;
                                }

                                if (pItem) {
                                    pItem->data.is_multi_isp_mode =
                                        s_full_info->isp_info->is_multi_isp_mode;
                                    pItem->data.multi_isp_extended_pixel = g_mMultiIspExtendedPixel;
                                    if (g_mIspHwInfos.ispp_info[i].valid)
                                        s_full_info->ispp_info = &g_mIspHwInfos.ispp_info[i];
                                    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "vicap link to isp(%d) to ispp(%d)\n",
                                                    s_full_info->isp_info->model_idx,
                                                    s_full_info->ispp_info
                                                        ? s_full_info->ispp_info->model_idx
                                                        : -1);
                                    pItem->data.sensor_info.binded_strm_media_idx =
                                        s_full_info->ispp_info
                                            ? atoi(s_full_info->ispp_info->media_dev_path +
                                                strlen("/dev/media"))
                                            : -1;
                                    LOGI_CAMHW_SUBM(
                                        ISP20HW_SUBM, "sensor %s adapted to pp media %d:%s\n",
                                        s_full_info->sensor_name,
                                        pItem->data.sensor_info.binded_strm_media_idx,
                                        s_full_info->ispp_info ? s_full_info->ispp_info->media_dev_path
                                                            : "null");
                                    g_mIspHwInfos.isp_info[i].linked_sensor = true;
                                }
                            } else {
                                int first_num = 0, second_num = 0;
                                CamHwInfoWraps_t* pItem = g_mCamHwInfos;
                                CamHwInfoWraps_t* pItem_new = (CamHwInfoWraps_t*)aiq_mallocz(sizeof(CamHwInfoWraps_t));
                                SnsFullInfoWraps_t* s_full_info_new = (SnsFullInfoWraps_t*)aiq_mallocz(sizeof(SnsFullInfoWraps_t));

                                while (pItem) {
                                    if (strcmp(s_full_info->sensor_name, pItem->key) == 0) break;
                                    pItem = pItem->next;
                                }

                                if (pItem && pItem_new && s_full_info_new) {
                                    max_phyId++;
                                    if (max_phyId > 9) {
                                        first_num  = max_phyId / 10;
                                    }
                                    second_num = max_phyId % 10;

                                    s_full_info->split = true;
                                    strcpy(s_full_info_new->key, s_full_info->sensor_name);
                                    memcpy(&s_full_info_new->data, s_full_info, sizeof(rk_sensor_full_info_t));
                                    strcpy(s_full_info_new->data.sensor_name_before_split, s_full_info->sensor_name);

                                    s_full_info_new->key[1]                   = '0' + first_num;
                                    s_full_info_new->key[2]                   = '0' + second_num;
                                    s_full_info_new->data.sensor_name[1]      = '0' + first_num;
                                    s_full_info_new->data.sensor_name[2]      = '0' + second_num;
                                    s_full_info_new->data.module_index_str[1] = '0' + first_num;
                                    s_full_info_new->data.module_index_str[2] = '0' + second_num;

                                    strcpy(pItem_new->key, s_full_info_new->key);
                                    memcpy(&pItem_new->data, &pItem->data, sizeof(rk_aiq_static_info_t));
                                    strcpy(pItem_new->data.sensor_info.sensor_name, s_full_info_new->key);
                                    pItem_new->data.sensor_info.phyId = max_phyId;

                                    s_full_info_new->data.isp_info    = &g_mIspHwInfos.isp_info[i];
                                    pItem_new->data.is_multi_isp_mode =
                                        s_full_info_new->data.isp_info->is_multi_isp_mode;
                                    pItem_new->data.multi_isp_extended_pixel = g_mMultiIspExtendedPixel;
                                    if (g_mIspHwInfos.ispp_info[i].valid)
                                        s_full_info_new->data.ispp_info = &g_mIspHwInfos.ispp_info[i];
                                    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "vicap link to isp(%d) to ispp(%d)\n",
                                                        s_full_info->isp_info->model_idx,
                                                        s_full_info->ispp_info
                                                            ? s_full_info->ispp_info->model_idx
                                                            : -1);
                                    pItem_new->data.sensor_info.binded_strm_media_idx =
                                        s_full_info->ispp_info
                                            ? atoi(s_full_info->ispp_info->media_dev_path +
                                                strlen("/dev/media"))
                                            : -1;
                                    LOGI_CAMHW_SUBM(
                                        ISP20HW_SUBM, "sensor %s adapted to pp media %d:%s\n",
                                        s_full_info->sensor_name,
                                        pItem_new->data.sensor_info.binded_strm_media_idx,
                                        s_full_info->ispp_info ? s_full_info->ispp_info->media_dev_path
                                                            : "null");
                                    g_mIspHwInfos.isp_info[i].linked_sensor = true;
                                    if (!tempSensorHwInfos) {
                                        tempSensorHwInfos = s_full_info_new;
                                    } else {
                                        SnsFullInfoWraps_t *sns_pItem = tempSensorHwInfos;
                                        while (sns_pItem->next) {
                                            sns_pItem = sns_pItem->next;
                                        }
                                        sns_pItem->next = s_full_info_new;
                                    }
                                    while (pItem->next) {
                                        pItem = pItem->next;
                                    }
                                    pItem->next = pItem_new;
                                }
                            }
                            vicap_linked_to_isp_nums++;
                        }
                    }
                }
                if (s_full_info->linked_to_serdes) {
                    SnsFullInfoWraps_t* iter = g_mSensorHwInfos;
                    for (; iter != NULL; iter = iter->next) {
                        rk_sensor_full_info_t* tmp_sinfo = &iter->data;
                        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "match the sensor_name(%s) media link\n",
                                        iter->key);
                        if (!tmp_sinfo->isp_info)
                            continue;

                        // serdes mode, just support one vicap link to isp
                        if (!tmp_sinfo->linked_to_1608 && tmp_sinfo->isp_info->linked_sensor &&
                                strcmp(tmp_sinfo->isp_info->linked_vicap[0], s_full_info->cif_info->model_str) == 0) {
                            tmp_sinfo->cif_info = s_full_info->cif_info;
                            tmp_sinfo->dvp_itf  = s_full_info->dvp_itf;
                            tmp_sinfo->linked_to_serdes = s_full_info->linked_to_serdes;

                            CamHwInfoWraps_t* pItem = g_mCamHwInfos;
                            while (pItem) {
                                if (strcmp(s_full_info->sensor_name, pItem->key) == 0) break;
                                pItem = pItem->next;
                            }

                            if (pItem) {
                                pItem->data.is_multi_isp_mode =
                                    tmp_sinfo->isp_info->is_multi_isp_mode;
                                pItem->data.multi_isp_extended_pixel = g_mMultiIspExtendedPixel;
                            }
                        }
                    }
                }
            }

            for (i = 0; i < MAX_CAM_NUM; i++) {
                if (!s_full_info->isp_info)
                    continue;

                LOGI_CAMHW_SUBM(ISP20HW_SUBM, "isp model_idx: %d, aiisp(%d) model_idx: %d\n",
                                s_full_info->isp_info->model_idx, i,
                                g_mAiispHwInfos.aiisp_info[i].model_idx);

                if (g_mAiispHwInfos.aiisp_info[i].valid &&
                        (s_full_info->isp_info->model_idx ==
                         g_mAiispHwInfos.aiisp_info[i].model_idx)) {
                    s_full_info->aiisp_info  = &g_mAiispHwInfos.aiisp_info[i];
                    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "isp(%d) link to aiisp(%d)\n",
                                    s_full_info->isp_info->model_idx,
                                    g_mAiispHwInfos.aiisp_info[i].model_idx);
                    break;
                }
            }

            if (!s_full_info->isp_info /* || !s_full_info->ispp_info*/) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get isp or ispp info fail, something gos wrong!");
            } else {
                // CamHwIsp20::mCamHwInfos[s_full_info->sensor_name]->linked_isp_info =
                // *s_full_info->isp_info;
                // CamHwIsp20::mCamHwInfos[s_full_info->sensor_name]->linked_ispp_info =
                // *s_full_info->ispp_info;
            }
        }

        // store new sensor into sensor map
        iter = g_mSensorHwInfos;
        while (iter->next) {
            iter = iter->next;
        }
        iter->next = tempSensorHwInfos;
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "dump sensor link info:");
        for (iter = g_mSensorHwInfos; iter != NULL; iter = iter->next) {
            for (i = 0; i < MAX_ISP_LINKED_VICAP_CNT; i++) {
                if (iter->data.isp_info && strlen(iter->data.isp_info->linked_vicap[i]) > 0) {
                    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "sns_name:%s %d %s", iter->key,
                                    iter->data.isp_info->model_idx,
                                    iter->data.isp_info->linked_vicap[i]);
                    if (iter->data.cif_info) {
                        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "  --> linked vicap info:");
                        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "      model_str: %s",
                                        iter->data.cif_info->model_str);
                        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "      media_dev: %s",
                                        iter->data.cif_info->media_dev_path);
                    }
                    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "  --> linked isp info:");
                    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "      media_dev: %s",
                                    iter->data.isp_info->media_dev_path);
                    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "      linked_vicap: %s",
                                    iter->data.isp_info->linked_vicap[i]);
                }
            }
        }

        int lvds_fd = -1;
        int connect_id;
        char lvdsPath[DEV_PATH_LEN];
        AiqV4l2SubDevice_t lvdsDev;

        for (iter = g_mSensorHwInfos; iter != NULL; iter = iter->next) {
            //set connect id
            if (!iter->data.isp_info)
                continue;
            strncpy(lvdsPath, iter->data.isp_info->linked_vicap_sd_path, sizeof(lvdsPath));
            if (strlen(lvdsPath) > 0) {
                AiqV4l2SubDevice_init(&lvdsDev, lvdsPath);
                AiqV4l2SubDevice_open(&lvdsDev, false);
                lvds_fd = lvdsDev._v4l_base._fd;
                if (lvds_fd < 0) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get lvds_fd, value is %d\n", lvds_fd);
                    continue;
                }

                if (xcam_device_ioctl(lvds_fd, RKCIF_CMD_GET_CONNECT_ID, &connect_id) < 0) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to get stream connect id! lvds_fd is %d\n",
                                    lvds_fd);
                    AiqV4l2SubDevice_deinit(&lvdsDev);
                    continue;
                }
                iter->data.connect_id = connect_id;
                AiqV4l2SubDevice_deinit(&lvdsDev);
            }
        }
    }

    /* Look for free isp&ispp link to fake camera */
    for (i = 0; i < MAX_CAM_NUM; i++) {
        if (g_mIspHwInfos.isp_info[i].valid && !g_mIspHwInfos.isp_info[i].linked_sensor) {
            CamHwInfoWraps_t* info_wrap = (CamHwInfoWraps_t*)aiq_mallocz(sizeof(CamHwInfoWraps_t));
            SnsFullInfoWraps_t* s_full_info_wrap =
                (SnsFullInfoWraps_t*)aiq_mallocz(sizeof(SnsFullInfoWraps_t));
            rk_aiq_static_info_t* info           = &info_wrap->data;
            rk_sensor_full_info_t* fullinfo      = &s_full_info_wrap->data;

            fullinfo->isp_info = &g_mIspHwInfos.isp_info[i];
            if (g_mIspHwInfos.ispp_info[i].valid) {
                fullinfo->ispp_info = &g_mIspHwInfos.ispp_info[i];
                info->sensor_info.binded_strm_media_idx =
                    atoi(fullinfo->ispp_info->media_dev_path + strlen("/dev/media"));
            }
            if (g_mAiispHwInfos.aiisp_info[i].valid) {
                fullinfo->aiisp_info = &g_mAiispHwInfos.aiisp_info[i];
                LOGI_CAMHW_SUBM(ISP20HW_SUBM, "fake camera %d link to isp(%d) to aiisp(%d)\n", i,
                                fullinfo->isp_info->model_idx,
                                fullinfo->aiisp_info->model_idx);
            }

            fullinfo->media_node_index = -1;
            strcpy(fullinfo->device_name, "/dev/null");
            strcpy(fullinfo->sensor_name, "FakeCamera");
            {
                char itoc[3] = {'\0'};
                sprintf(itoc, "%d", i);
                strcat(fullinfo->sensor_name, itoc);
            }
            strcpy(s_full_info_wrap->key, fullinfo->sensor_name);
            strcpy(info_wrap->key, fullinfo->sensor_name);
            strcpy(fullinfo->parent_media_dev, "/dev/null");
            fullinfo->linked_to_isp = true;

            info->sensor_info.support_fmt[0].hdr_mode   = NO_HDR;
            info->sensor_info.support_fmt[1].hdr_mode   = HDR_X2;
            info->sensor_info.support_fmt[2].hdr_mode   = HDR_X3;
            info->sensor_info.num                       = 3;
            g_mIspHwInfos.isp_info[i].linked_sensor     = true;

            _SensorInfoCopy(fullinfo, info);
            info->has_lens_vcm             = false;
            info->has_fl                   = false;
            info->has_irc                  = false;
            info->fl_strth_adj_sup         = 0;
            info->fl_ir_strth_adj_sup      = 0;
            info->is_multi_isp_mode        = fullinfo->isp_info->is_multi_isp_mode;
            info->multi_isp_extended_pixel = g_mMultiIspExtendedPixel;
            info->sensor_info.phyId = i;

            if (!g_mSensorHwInfos) {
                g_mSensorHwInfos = s_full_info_wrap;
            } else {
                SnsFullInfoWraps_t* pItem = g_mSensorHwInfos;
                while (pItem->next) {
                    pItem = pItem->next;
                }
                pItem->next = s_full_info_wrap;
            }

            if (!g_mCamHwInfos) {
                g_mCamHwInfos = info_wrap;
            } else {
                CamHwInfoWraps_t* pItem = g_mCamHwInfos;
                while (pItem->next) {
                    pItem = pItem->next;
                }
                pItem->next = info_wrap;
            }

            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "fake camera %d link to isp(%d) to ispp(%d)\n", i,
                            fullinfo->isp_info->model_idx,
                            fullinfo->ispp_info ? fullinfo->ispp_info->model_idx : -1);
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "sensor %s adapted to pp media %d:%s\n",
                            fullinfo->sensor_name, info->sensor_info.binded_strm_media_idx,
                            fullinfo->ispp_info ? fullinfo->ispp_info->media_dev_path : "null");
        }
    }

    _get_isp_ver(&g_mIspHwInfos);

    {
        CamHwInfoWraps_t* pItem = g_mCamHwInfos;
        while (pItem) {
            pItem->data.isp_hw_ver = g_mIspHwInfos.hw_ver_info.isp_ver;
            pItem                  = pItem->next;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

const char* AiqCamHw_getBindedSnsEntNmByVd(const char* vd) {
    SnsFullInfoWraps_t* pItem = g_mSensorHwInfos;

    if (!vd) return NULL;

    for (; pItem != NULL; pItem = pItem->next) {
        rk_sensor_full_info_t* s_full_info = &pItem->data;

        // skip 1608-dphy 'sensor'
        if (strstr(s_full_info->sensor_name, "1608")) continue;
        if (!s_full_info->isp_info) continue;

        bool stream_vd = false;
        if (s_full_info->ispp_info) {
            if (strstr(s_full_info->ispp_info->pp_m_bypass_path, vd) ||
                    strstr(s_full_info->ispp_info->pp_scale0_path, vd) ||
                    strstr(s_full_info->ispp_info->pp_scale1_path, vd) ||
                    strstr(s_full_info->ispp_info->pp_scale2_path, vd))
                stream_vd = true;
        } else {
            if (strstr(s_full_info->isp_info->main_path, vd) ||
                    strstr(s_full_info->isp_info->self_path, vd) ||
                    strstr(s_full_info->isp_info->ldc_path, vd))
                stream_vd = true;
        }

        if (stream_vd) {
            // check linked
            if ((strstr(s_full_info->sensor_name, "FakeCamera") == NULL) &&
                (strstr(s_full_info->sensor_name, "_s_") == NULL) &&
                !s_full_info->split) {
                FILE* fp                    = NULL;
                struct media_device* device = NULL;
                uint32_t j = 0, i = 0;
                const struct media_entity_desc* entity_info = NULL;
                struct media_entity* entity                 = NULL;
                struct media_pad* src_pad_s                 = NULL;
                char sys_path[64];

                snprintf(sys_path, 64, "/dev/media%d", s_full_info->media_node_index);
                if (0 != access(sys_path, F_OK)) continue;

                device = media_device_new(sys_path);
                if (!device) return NULL;

                /* Enumerate entities, pads and links. */
                media_device_enumerate(device);
                entity      = media_get_entity_by_name(device, s_full_info->sensor_name,
                                                       strlen(s_full_info->sensor_name));
                entity_info = media_entity_get_info(entity);
                if (entity && entity->num_links > 0) {
                    if (entity->links[0].flags == MEDIA_LNK_FL_ENABLED) {
                        media_device_unref(device);
                        return s_full_info->sensor_name;
                    }
                }
                media_device_unref(device);
            } else
                return s_full_info->sensor_name;
        }
    }

    return NULL;
}

static XCamReturn _poll_buffer_ready(void* ctx, AiqHwEvt_t* evt, int dev_index) {
    if (evt->type == ISP_POLL_3A_STATS) {
        AiqCamHwBase_t* pCamHw = (AiqCamHwBase_t*)ctx;
        void* stats = (void*)AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)evt->vb);

#ifdef ISP_HW_V35
        if (pCamHw->mBtnrDsReproc) {
            AiqBtnrDsReproc_pushMsg(pCamHw->mBtnrDsReproc, stats);
        }
#endif

#if RKAIQ_HAVE_DUMPSYS
        // dump stats info
        {
            struct timespec time;
            clock_gettime(CLOCK_MONOTONIC, &time);

            if (evt->frame_id > pCamHw->stats.id)
                pCamHw->stats.frameloss += evt->frame_id ? evt->frame_id - pCamHw->stats.id - 1 : 0;
            pCamHw->stats.id        = evt->frame_id;
            pCamHw->stats.interval  = evt->mTimestamp - pCamHw->stats.timestamp;
            pCamHw->stats.timestamp = evt->mTimestamp;
            pCamHw->stats.delay     = XCAM_TIMESPEC_2_USEC(time) - evt->mTimestamp;
        }
#endif

        btnr_cvt_info_t *btnr_info = &pCamHw->_mIspParamsCvt->mBtnrInfo;
        bayertnr_save_stats(stats, btnr_info);

#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
        blc_cvt_info_t* blc_info = &pCamHw->_mIspParamsCvt->mBlcInfo;
        bool btnr_en = pCamHw->_mIspParamsCvt->mCommonCvtInfo.btnr_en;
#if ISP_HW_V33
        if (!blc_info->ds_address && btnr_en) {
            struct rkisp_bay3dbuf_info bay3dbuf;
            memset(&bay3dbuf, 0, sizeof(bay3dbuf));
            int res =
                AiqV4l2SubDevice_ioctl((AiqV4l2Device_t*)pCamHw->mIspCoreDev, RKISP_CMD_GET_BAY3D_BUFFD, &bay3dbuf);
            if (res) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get bay3dbuf failed! %d", res);
                return XCAM_RETURN_ERROR_IOCTL;
            }

            void* ds_address =
                (char*)mmap(NULL, bay3dbuf.u.v33.ds_size, PROT_READ | PROT_WRITE, MAP_SHARED, bay3dbuf.u.v33.ds_fd, 0);
            if (MAP_FAILED == ds_address) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "ds_fd mmap failed ds_fd is %d ds_size is %d", bay3dbuf.u.v33.ds_fd, bay3dbuf.u.v33.ds_size);
                return XCAM_RETURN_ERROR_FAILED;
            }
            LOGK_CAMHW_SUBM(ISP20HW_SUBM, "blc get bay3dbuf: ds_fd is %d ds_size is %d ds_address is 0x%x",
                            bay3dbuf.u.v33.ds_fd, bay3dbuf.u.v33.ds_size, ds_address);
            blc_info->ds_fd = bay3dbuf.u.v33.ds_fd;
            blc_info->gain_fd = bay3dbuf.u.v33.gain_fd;
            blc_info->iir_fd = bay3dbuf.iir_fd;
            blc_info->ds_size = bay3dbuf.u.v33.ds_size;
            blc_info->ds_address = ds_address;
        }
#elif ISP_HW_V35
        if (!blc_info->ds_address && btnr_en) {
            struct rkisp_bnr_buf_info bay3dbuf = pCamHw->_bnrDrvBuf._bay3dbuf;
            void* ds_address =
                (char*)mmap(NULL, bay3dbuf.u.v35.ds.buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, bay3dbuf.u.v35.ds.buf_fd[0], 0);
            if (MAP_FAILED == ds_address) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "ds_fd mmap failed ds_fd is %d ds_size is %d", bay3dbuf.u.v35.ds.buf_fd[0], bay3dbuf.u.v35.ds.buf_size);
                return XCAM_RETURN_ERROR_FAILED;
            }
            LOGK_CAMHW_SUBM(ISP20HW_SUBM, "blc get bay3dbuf: ds_fd is %d ds_size is %d ds_address is 0x%x",
                            bay3dbuf.u.v35.ds.buf_fd[0], bay3dbuf.u.v35.ds.buf_size, ds_address);
            blc_info->ds_fd = bay3dbuf.u.v35.ds.buf_fd[0];
            blc_info->gain_fd = bay3dbuf.u.v35.gain.buf_fd[0];
            blc_info->iir_fd = bay3dbuf.iir.buf_fd[0];
            blc_info->ds_size = bay3dbuf.u.v35.ds.buf_size;
            blc_info->ds_address = ds_address;
        }
#endif
        if (blc_info->autoblc_count > 0 && (blc_info->autoblc_count - 1) % 8 == 0) {
            if (!blc_info->tnr_ds_buf)
                blc_info->tnr_ds_buf = aiq_mallocz(blc_info->ds_size);
            struct dma_buf_sync sync = { 0 };
            sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_START;
            ioctl(blc_info->ds_fd, DMA_BUF_IOCTL_SYNC, &sync);

            memcpy(blc_info->tnr_ds_buf, blc_info->ds_address, blc_info->ds_size);

            sync.flags = DMA_BUF_SYNC_READ | DMA_BUF_SYNC_END;
            ioctl(blc_info->ds_fd, DMA_BUF_IOCTL_SYNC, &sync);
        }
#endif
    } else if (evt->type == ISP_POLL_PARAMS) {
        return XCAM_RETURN_NO_ERROR;
    } else if (evt->type == ISP_POLL_SOF) {
#if RKAIQ_HAVE_DUMPSYS
        // dump fs info
        {
            AiqCamHwBase_t* pCamHw = (AiqCamHwBase_t*)ctx;

            struct timespec time;
            clock_gettime(CLOCK_MONOTONIC, &time);

            pCamHw->prev_fs = pCamHw->fs;
            pCamHw->fs.frameloss += evt->frame_id ? evt->frame_id - pCamHw->fs.id - 1 : 0;
            pCamHw->fs.id        = evt->frame_id;
            pCamHw->fs.timestamp = evt->mTimestamp / 1000;
            pCamHw->fs.delay     = XCAM_TIMESPEC_2_USEC(time) - evt->mTimestamp / 1000;
        }
#endif
    }
    else if (evt->type == ISP_POLL_MEMC) {
#if RKAIQ_HAVE_MEMC
        AiqCamHwBase_t* pCamHw = (AiqCamHwBase_t*)ctx;
        AiqHwMemcEvt_t *memc_evt = (AiqHwMemcEvt_t *)evt;
        AiqMemcManager_pushIirMsg(pCamHw->mMemcStream, memc_evt->isp_idxbuf.iir_index, evt->frame_id);
#endif
    }

    AiqCamHwBase_t* pCamHw = (AiqCamHwBase_t*)ctx;
    if (pCamHw->_hwResListener.hwResCb) {
        return pCamHw->_hwResListener.hwResCb(pCamHw->_hwResListener._pCtx, evt);
    }

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_AIBNR
static XCamReturn AiqCamHw_aibnr_init(AiqCamHwBase_t* pCamHw, AibnrManager_t* pAibnrManager, struct rkisp_aiisp_cfg *aibnr_cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pCamHw->mAiIspDev)
        return XCAM_RETURN_ERROR_FAILED;

    pCamHw->mAibnr_cfg = *aibnr_cfg;
    pCamHw->mAibnrManager = pAibnrManager;

    if (!pCamHw->mAibnrIspStream) {
        pCamHw->mAibnrIspStream = (AiqAibnrIspStream_t*)aiq_mallocz(sizeof(AiqAibnrIspStream_t));
        AiqAibnr_IspStream_init(pCamHw->mAibnrIspStream, (AiqV4l2Device_t*)pCamHw->mAiIspCoreDev,
                                ISP_POLL_AIISP);
        pCamHw->mAibnrIspStream->_base.setPollCallback(&pCamHw->mAibnrIspStream->_base,
                &pCamHw->mPollCb);
        ret = pCamHw->mAibnrIspStream->set_aiisp_linecnt(pCamHw->mAibnrIspStream, pCamHw->mAibnr_cfg);
    } else {
        // restart aiisp for restart aiq without deinit
        ret = pCamHw->mAibnrIspStream->set_aiisp_linecnt(pCamHw->mAibnrIspStream, pCamHw->mAibnr_cfg);
    }

    if (!pCamHw->mAibnrAiispStream) {
        pCamHw->mAibnrAiispStream = (AiqAibnrAiispStream_t*)aiq_mallocz(sizeof(AiqAibnrAiispStream_t));
        AiqAibnr_AiispStream_init(pCamHw->mAibnrAiispStream, (AiqV4l2Device_t*)pCamHw->mAiIspSubDev,
                                  ISP_POLL_AIBNR_DONE);
        pCamHw->mAibnrAiispStream->_base.setPollCallback(&pCamHw->mAibnrAiispStream->_base,
                &pCamHw->mPollCb);
    }
    return ret;
}

static XCamReturn AiqCamHw_aibnr_prepare(AiqCamHwBase_t* pCamHw)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCamHw->mAiIspDev) {
        AiqV4l2Device_setBufCnt(pCamHw->mAiIspDev, 6);
        AiqV4l2Device_prepare(pCamHw->mAiIspDev);
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

static XCamReturn AiqCamHw_aibnr_start(AiqCamHwBase_t* pCamHw)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCamHw->mAibnrIspStream) {
        AiqStream_t* stream = (AiqStream_t*)(pCamHw->mAibnrIspStream);
        stream->setCamPhyId(stream, pCamHw->mCamPhyId);
        stream->start(stream);
    }
    if (pCamHw->mAibnrAiispStream) {
        AiqStream_t* stream = (AiqStream_t*)(pCamHw->mAibnrAiispStream);
        stream->setCamPhyId(stream, pCamHw->mCamPhyId);
        stream->start(stream);
    }

    if (pCamHw->mAiIspDev)
        AiqV4l2Device_start(pCamHw->mAiIspDev, true);
    else
        ret = XCAM_RETURN_ERROR_FAILED;

    return ret;
}

static XCamReturn AiqCamHw_aibnr_stop(AiqCamHwBase_t* pCamHw)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCamHw->mAibnrIspStream) {
        pCamHw->mAibnrIspStream->_base.stop(&pCamHw->mAibnrIspStream->_base);
    }

    if (pCamHw->mAibnrAiispStream) {
        pCamHw->mAibnrAiispStream->_base.stop(&pCamHw->mAibnrAiispStream->_base);
    }

    if (pCamHw->mAiIspDev)
        AiqV4l2Device_stop(pCamHw->mAiIspDev);
    else
        ret = XCAM_RETURN_ERROR_FAILED;

    return ret;
}

static XCamReturn AiqCamHw_aibnr_deinit(AiqCamHwBase_t* pCamHw)
{
    if (pCamHw->mAiIspDev && pCamHw->mAibnrBufPoolInit) {
        int dummy = 0;

        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_FREE_BUFPOOL, &dummy) != 0) {
            LOGE_AIBNR("%s:%d can't free bufpool", __FUNCTION__, __LINE__);
        }
        pCamHw->mAibnrBufPoolInit = false;
    }

    if (pCamHw->mAibnrIspStream) {
        AiqAibnr_IspStream_deinit(pCamHw->mAibnrIspStream);
        aiq_free(pCamHw->mAibnrIspStream);
        pCamHw->mAibnrIspStream = NULL;
    }
    if (pCamHw->mAibnrAiispStream) {
        AiqAibnr_AiispStream_deinit(pCamHw->mAibnrAiispStream);
        aiq_free(pCamHw->mAibnrAiispStream);
        pCamHw->mAibnrAiispStream = NULL;
    }

    if (pCamHw->mAiIspCoreDev) {
        AiqV4l2SubDevice_deinit(pCamHw->mAiIspCoreDev);
        aiq_free(pCamHw->mAiIspCoreDev);
        pCamHw->mAiIspCoreDev = NULL;
    }

    if (pCamHw->mAiIspSubDev) {
        AiqV4l2SubDevice_deinit(pCamHw->mAiIspSubDev);
        aiq_free(pCamHw->mAiIspSubDev);
        pCamHw->mAiIspSubDev = NULL;
    }

    if (pCamHw->mAiIspDev) {
        AiqV4l2Device_deinit(pCamHw->mAiIspDev);
        aiq_free(pCamHw->mAiIspDev);
        pCamHw->mAiIspDev = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aibnr_getParamsBuf(AiqCamHwBase_t* pCamHw, AiqV4l2Buffer_t** pV4l2Buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t frameId          = -1;

    *pV4l2Buf = NULL;
    if (pCamHw->mAiIspDev) {
        *pV4l2Buf = AiqV4l2Device_getBuf(pCamHw->mAiIspDev, -1);
        if (!*pV4l2Buf) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Can not get aiisp params buffer, queued cnts:%d \n",
                            AiqV4l2Device_getQueuedBufCnt(pCamHw->mAiIspDev));
            ret = XCAM_RETURN_ERROR_PARAM;
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

static XCamReturn AiqCamHw_aibnr_updateParams(AiqCamHwBase_t* pCamHw, AiqV4l2Buffer_t* pV4l2Buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pCamHw->mAiIspDev || !pV4l2Buf) {
        return XCAM_RETURN_BYPASS;
    }

    // AiqV4l2Buffer_setSequence(pV4l2Buf, frameId);
    if (AiqV4l2Device_qbuf(pCamHw->mAiIspDev, pV4l2Buf, true) != 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "RKISP1: failed to ioctl VIDIOC_QBUF %d %s.\n",
                        errno, strerror(errno));
        AiqV4l2Device_returnBufToPool(pCamHw->mAiIspDev, pV4l2Buf);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    int timeout = 1;
    uint32_t buf_counts = AiqV4l2Device_getBufCnt(pCamHw->mAiIspDev);
    uint32_t try_time   = 3;
    while (AiqV4l2Device_getQueuedBufCnt(pCamHw->mAiIspDev) > 2) {
        if (pCamHw->mAiIspDev->poll_event(pCamHw->mAiIspDev, timeout, -1) <= 0) {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "poll params error, queue cnts: %d !",
                            AiqV4l2Device_getQueuedBufCnt(pCamHw->mAiIspDev));
            if (AiqV4l2Device_getQueuedBufCnt(pCamHw->mAiIspDev) == buf_counts &&
                    try_time > 0) {
                timeout = 30;
                try_time--;
                continue;
            } else
                break;
        }
        AiqV4l2Buffer_t* dqbuf = pCamHw->mAiIspDev->_dequeue_buffer(pCamHw->mAiIspDev);
        if (!dqbuf) {
            XCAM_LOG_WARNING("dequeue buffer failed");
            // return ret;
        } else {
            AiqV4l2Device_returnBufToPool(pCamHw->mAiIspDev, dqbuf);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aibnr_doNrnn(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_st *isp_out)
{
    if (pCamHw->mAiIspDev) {
        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_QUEUE_BUF, isp_out) != 0) {
            LOGE_AIBNR("%s:%d can't do nrnn", __FUNCTION__, __LINE__);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aibnr_doIspBe(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_st *isp_in)
{
    if (pCamHw->mAibnrIspStream && pCamHw->_isp_stream_status != ISP_STREAM_STATUS_STREAM_OFF) {
        return pCamHw->mAibnrIspStream->start_ispbe_hdl(pCamHw->mAibnrIspStream, isp_in);
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aibnr_setMemMode(AiqCamHwBase_t* pCamHw, enum rkaiisp_mem_mode *memMode)
{
    if (pCamHw->mAiIspDev) {
        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_SET_MEMORY_MODE, memMode) != 0) {
            LOGE_AIBNR("%s:%d can't set memory mode", __FUNCTION__, __LINE__);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aibnr_setIspBufInf(AiqCamHwBase_t* pCamHw, struct rkaiisp_ispbuf_info *ispbuf_info)
{
    if (pCamHw->mAiIspDev) {
        if (pCamHw->mAibnrBufPoolInit) {
            int dummy = 0;

            if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                              RKAIISP_CMD_FREE_BUFPOOL, &dummy) != 0) {
                LOGE_AIBNR("%s:%d can't free bufpool", __FUNCTION__, __LINE__);
            }
            pCamHw->mAibnrBufPoolInit = false;
        }

        ispbuf_info->bnr_buf = pCamHw->_bnrDrvBuf._bay3dbuf;
        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_INIT_BUFPOOL, ispbuf_info) != 0) {
            LOGE_AIBNR("%s:%d can't int bufpool", __FUNCTION__, __LINE__);
        } else {
            pCamHw->mAibnrBufPoolInit = true;
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aibnr_setParamInf(AiqCamHwBase_t* pCamHw, struct rkaiisp_param_info *param_info)
{
    if (pCamHw->mAiIspDev) {
        pCamHw->mAibnrParamInfo = *param_info;
        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_SET_PARAM_INFO, param_info) != 0) {
            LOGE_AIBNR("%s:%d can't set param info", __FUNCTION__, __LINE__);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aibnr_setModelInf(AiqCamHwBase_t* pCamHw, RkAiqAibnrModelInfo_t *pAibnrModelInfo)
{
    memcpy(&pCamHw->_mIspParamsCvt->mCommonCvtInfo.aibnrModelInfo, pAibnrModelInfo, sizeof(RkAiqAibnrModelInfo_t));
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aibnr_setLinecnt(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_cfg *aibnr_cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pCamHw->mAiIspDev)
        return XCAM_RETURN_ERROR_FAILED;

    pCamHw->mAibnr_cfg = *aibnr_cfg;

    if (pCamHw->mAibnrIspStream)
        ret = pCamHw->mAibnrIspStream->set_aiisp_linecnt(pCamHw->mAibnrIspStream, pCamHw->mAibnr_cfg);

    return ret;
}

static XCamReturn AiqCamHw_aibnr_clrIqParam(AiqCamHwBase_t* pCamHw)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int dummy = 0;

    if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                      RKAIISP_CMD_CLEAR_IQPARAMS, &dummy) != 0) {
        LOGE_AIBNR("%s:%d can't clear iq params", __FUNCTION__, __LINE__);
    }

    return ret;
}

static XCamReturn AiqCamHw_btnrWgtReproc_updateEn(AiqCamHwBase_t* pCamHw, bool en)
{
    if (pCamHw->mBtnrDsReproc) {
        AiqBtnrDsReproc_updateEn(pCamHw->mBtnrDsReproc, en);
    }
    return XCAM_RETURN_NO_ERROR;
}

#endif

#if RKAIQ_HAVE_AIYNR
static XCamReturn AiqCamHw_aiynr_init(AiqCamHwBase_t* pCamHw, AiynrManager_t* pAiynrManager, struct rkisp_aiisp_cfg *aiynr_cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pCamHw->mAiIspDev)
        return XCAM_RETURN_ERROR_FAILED;

    pCamHw->mAiynr_cfg = *aiynr_cfg;
    pCamHw->mAiynrManager = pAiynrManager;

    if (!pCamHw->mAiynrIspStream) {
        pCamHw->mAiynrIspStream = (AiqAiynrIspStream_t*)aiq_mallocz(sizeof(AiqAiynrIspStream_t));
        AiqAiynr_IspStream_init(pCamHw->mAiynrIspStream, (AiqV4l2Device_t*)pCamHw->mAiIspCoreDev,
                                ISP_POLL_AIISP);
        pCamHw->mAiynrIspStream->_base.setPollCallback(&pCamHw->mAiynrIspStream->_base,
                &pCamHw->mPollCb);
        ret = pCamHw->mAiynrIspStream->set_aiisp_linecnt(pCamHw->mAiynrIspStream, pCamHw->mAiynr_cfg);
    } else {
        // restart aiisp for restart aiq without deinit
        ret = pCamHw->mAiynrIspStream->set_aiisp_linecnt(pCamHw->mAiynrIspStream, pCamHw->mAiynr_cfg);
    }

    if (!pCamHw->mAiynrAiispStream) {
        pCamHw->mAiynrAiispStream = (AiqAiynrAiispStream_t*)aiq_mallocz(sizeof(AiqAiynrAiispStream_t));
        AiqAiynr_AiispStream_init(pCamHw->mAiynrAiispStream, (AiqV4l2Device_t*)pCamHw->mAiIspSubDev,
                                  ISP_POLL_AIYNR_DONE);
        pCamHw->mAiynrAiispStream->_base.setPollCallback(&pCamHw->mAiynrAiispStream->_base,
                &pCamHw->mPollCb);
    }
    return ret;
}

static XCamReturn AiqCamHw_aiynr_prepare(AiqCamHwBase_t* pCamHw)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCamHw->mAiIspDev) {
        AiqV4l2Device_setBufCnt(pCamHw->mAiIspDev, 6);
        AiqV4l2Device_prepare(pCamHw->mAiIspDev);
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

static XCamReturn AiqCamHw_aiynr_start(AiqCamHwBase_t* pCamHw)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCamHw->mAiynrIspStream) {
        AiqStream_t* stream = (AiqStream_t*)(pCamHw->mAiynrIspStream);
        stream->setCamPhyId(stream, pCamHw->mCamPhyId);
        stream->start(stream);
    }
    if (pCamHw->mAiynrAiispStream) {
        AiqStream_t* stream = (AiqStream_t*)(pCamHw->mAiynrAiispStream);
        stream->setCamPhyId(stream, pCamHw->mCamPhyId);
        stream->start(stream);
    }

    if (pCamHw->mAiIspDev)
        AiqV4l2Device_start(pCamHw->mAiIspDev, true);
    else
        ret = XCAM_RETURN_ERROR_FAILED;

    return ret;
}

static XCamReturn AiqCamHw_aiynr_stop(AiqCamHwBase_t* pCamHw)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCamHw->mAiynrIspStream) {
        pCamHw->mAiynrIspStream->_base.stop(&pCamHw->mAiynrIspStream->_base);
    }

    if (pCamHw->mAiynrAiispStream) {
        pCamHw->mAiynrAiispStream->_base.stop(&pCamHw->mAiynrAiispStream->_base);
    }

    if (pCamHw->mAiIspDev)
        AiqV4l2Device_stop(pCamHw->mAiIspDev);
    else
        ret = XCAM_RETURN_ERROR_FAILED;

    return ret;
}

static XCamReturn AiqCamHw_aiynr_deinit(AiqCamHwBase_t* pCamHw)
{
    if (pCamHw->mAiIspDev && pCamHw->mAiynrBufPoolInit) {
        int dummy = 0;

        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_FREE_BUFPOOL, &dummy) != 0) {
            LOGE_AIYNR("%s:%d can't free bufpool", __FUNCTION__, __LINE__);
        }
        pCamHw->mAiynrBufPoolInit = false;
    }

    if (pCamHw->mAiynrIspStream) {
        AiqAiynr_IspStream_deinit(pCamHw->mAiynrIspStream);
        aiq_free(pCamHw->mAiynrIspStream);
        pCamHw->mAiynrIspStream = NULL;
    }
    if (pCamHw->mAiynrAiispStream) {
        AiqAiynr_AiispStream_deinit(pCamHw->mAiynrAiispStream);
        aiq_free(pCamHw->mAiynrAiispStream);
        pCamHw->mAiynrAiispStream = NULL;
    }

    if (pCamHw->mAiIspCoreDev) {
        AiqV4l2SubDevice_deinit(pCamHw->mAiIspCoreDev);
        aiq_free(pCamHw->mAiIspCoreDev);
        pCamHw->mAiIspCoreDev = NULL;
    }

    if (pCamHw->mAiIspSubDev) {
        AiqV4l2SubDevice_deinit(pCamHw->mAiIspSubDev);
        aiq_free(pCamHw->mAiIspSubDev);
        pCamHw->mAiIspSubDev = NULL;
    }

    if (pCamHw->mAiIspDev) {
        AiqV4l2Device_deinit(pCamHw->mAiIspDev);
        aiq_free(pCamHw->mAiIspDev);
        pCamHw->mAiIspDev = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aiynr_getParamsBuf(AiqCamHwBase_t* pCamHw, AiqV4l2Buffer_t** pV4l2Buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t frameId          = -1;

    *pV4l2Buf = NULL;
    if (pCamHw->mAiIspDev) {
        *pV4l2Buf = AiqV4l2Device_getBuf(pCamHw->mAiIspDev, -1);
        if (!*pV4l2Buf) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Can not get aiisp params buffer, queued cnts:%d \n",
                            AiqV4l2Device_getQueuedBufCnt(pCamHw->mAiIspDev));
            ret = XCAM_RETURN_ERROR_PARAM;
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
    }

    return ret;
}

static XCamReturn AiqCamHw_aiynr_updateParams(AiqCamHwBase_t* pCamHw, AiqV4l2Buffer_t* pV4l2Buf)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pCamHw->mAiIspDev || !pV4l2Buf) {
        return XCAM_RETURN_BYPASS;
    }

    // AiqV4l2Buffer_setSequence(pV4l2Buf, frameId);
    if (AiqV4l2Device_qbuf(pCamHw->mAiIspDev, pV4l2Buf, true) != 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "RKISP1: failed to ioctl VIDIOC_QBUF %d %s.\n",
                        errno, strerror(errno));
        AiqV4l2Device_returnBufToPool(pCamHw->mAiIspDev, pV4l2Buf);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    int timeout = 1;
    uint32_t buf_counts = AiqV4l2Device_getBufCnt(pCamHw->mAiIspDev);
    uint32_t try_time   = 3;
    while (AiqV4l2Device_getQueuedBufCnt(pCamHw->mAiIspDev) > 2) {
        if (pCamHw->mAiIspDev->poll_event(pCamHw->mAiIspDev, timeout, -1) <= 0) {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "poll params error, queue cnts: %d !",
                            AiqV4l2Device_getQueuedBufCnt(pCamHw->mAiIspDev));
            if (AiqV4l2Device_getQueuedBufCnt(pCamHw->mAiIspDev) == buf_counts &&
                    try_time > 0) {
                timeout = 30;
                try_time--;
                continue;
            } else
                break;
        }
        AiqV4l2Buffer_t* dqbuf = pCamHw->mAiIspDev->_dequeue_buffer(pCamHw->mAiIspDev);
        if (!dqbuf) {
            XCAM_LOG_WARNING("dequeue buffer failed");
            // return ret;
        } else {
            AiqV4l2Device_returnBufToPool(pCamHw->mAiIspDev, dqbuf);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aiynr_doNrnn(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_st *isp_out)
{
    if (pCamHw->mAiIspDev) {
        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_QUEUE_BUF, isp_out) != 0) {
            LOGE_AIYNR("%s:%d can't do nrnn", __FUNCTION__, __LINE__);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aiynr_doIspBe(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_st *isp_in)
{
    if (pCamHw->mAiynrIspStream) {
        return pCamHw->mAiynrIspStream->start_ispbe_hdl(pCamHw->mAiynrIspStream, isp_in);
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aiynr_setIspBufInf(AiqCamHwBase_t* pCamHw, struct rkaiisp_ispbuf_info *ispbuf_info)
{
    if (pCamHw->mAiIspDev) {
        if (pCamHw->mAiynrBufPoolInit) {
            int dummy = 0;

            if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                              RKAIISP_CMD_FREE_BUFPOOL, &dummy) != 0) {
                LOGE_AIYNR("%s:%d can't free bufpool", __FUNCTION__, __LINE__);
            }
            pCamHw->mAiynrBufPoolInit = false;
        }

        ispbuf_info->bnr_buf = pCamHw->_bnrDrvBuf._bay3dbuf;
        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_INIT_BUFPOOL, ispbuf_info) != 0) {
            LOGE_AIYNR("%s:%d can't int bufpool", __FUNCTION__, __LINE__);
        } else {
            pCamHw->mAiynrBufPoolInit = true;
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aiynr_setParamInf(AiqCamHwBase_t* pCamHw, struct rkaiisp_param_info *param_info)
{
    if (pCamHw->mAiIspDev) {
        pCamHw->mAiynrParamInfo = *param_info;
        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_SET_PARAM_INFO, param_info) != 0) {
            LOGE_AIYNR("%s:%d can't set param info", __FUNCTION__, __LINE__);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aiynr_setModelInf(AiqCamHwBase_t* pCamHw, RkAiqAiynrModelInfo_t *pAiynrModelInfo)
{
    memcpy(&pCamHw->_mIspParamsCvt->mCommonCvtInfo.aiynrModelInfo, pAiynrModelInfo, sizeof(RkAiqAiynrModelInfo_t));
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aiynr_setLinecnt(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_cfg *aiynr_cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pCamHw->mAiIspDev)
        return XCAM_RETURN_ERROR_FAILED;

    pCamHw->mAiynr_cfg = *aiynr_cfg;

    if (pCamHw->mAiynrIspStream)
        ret = pCamHw->mAiynrIspStream->set_aiisp_linecnt(pCamHw->mAiynrIspStream, pCamHw->mAiynr_cfg);

    return ret;
}
static XCamReturn AiqCamHw_aiynr_getYnrbufInf(AiqCamHwBase_t* pCamHw, struct rkaiisp_ynrbuf_info *ynrbuf_info)
{
    if (pCamHw->mAiIspDev) {
        if (pCamHw->mAiIspDev->io_control(pCamHw->mAiIspDev,
                                          RKAIISP_CMD_GET_YNRBUF_INFO, ynrbuf_info) != 0) {
            LOGE_AIBNR("%s:%d can't get ynrbuf info", __FUNCTION__, __LINE__);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn AiqCamHwBase_init(AiqCamHwBase_t* pCamHw, const char* sns_ent_name) {
    ENTER_CAMHW_FUNCTION();

    SnsFullInfoWraps_t* pItem     = g_mSensorHwInfos;
    rk_sensor_full_info_t* s_info = NULL;

    while (pItem) {
        if (strcmp(pItem->key, sns_ent_name) == 0) break;
        pItem = pItem->next;
    }

    if (!pItem) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", sns_ent_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    pCamHw->mAweekId               = -1;
    pCamHw->use_aiisp              = false;
    pCamHw->aiisp_param            = NULL;
    pCamHw->_skipped_params        = NULL;
    pCamHw->_first_awb_param       = NULL;
    pCamHw->_first_awb_cfg         = NULL;
    pCamHw->mVicapItfDev           = NULL;
    pCamHw->mIsOnlineByWorkingMode = false;
    pCamHw->mIsListenStrmEvt       = true;
    pCamHw->_linked_to_serdes      = false;
    pCamHw->_not_skip_first        = true;
    aiqMutex_init(&pCamHw->_isp_params_cfg_mutex);
    aiqMutex_init(&pCamHw->_mem_mutex);
    aiqMutex_init(&pCamHw->_stop_cond_mutex);
    aiqCond_init(&pCamHw->_sync_done_cond);
    pCamHw->prepare = AiqCamHw_prepare;

    pCamHw->_state      = CAM_HW_STATE_INVALID;
    pCamHw->mNoReadBack = true;
    {
#ifndef ANDROID_OS
        char* valueStr = getenv("normal_no_read_back");
        if (valueStr) {
            pCamHw->mNoReadBack = atoi(valueStr) > 0 ? true : false;
        }
#else
        char property_value[PROPERTY_VALUE_MAX] = {'\0'};

        property_get("persist.vendor.rkisp_no_read_back", property_value, "-1");
        int val = atoi(property_value);
        if (val != -1) pCamHw->mNoReadBack = atoi(property_value) > 0 ? true : false;
#endif
    }

    pCamHw->_ldch_drv_mem_ctx.type     = MEM_TYPE_LDCH;
    pCamHw->_ldch_drv_mem_ctx.ops_ctx  = pCamHw;
    pCamHw->_ldch_drv_mem_ctx.mem_info = (void*)(pCamHw->ldch_mem_info_array);

    pCamHw->_ldcv_drv_mem_ctx.type     = MEM_TYPE_LDCV;
    pCamHw->_ldcv_drv_mem_ctx.ops_ctx  = pCamHw;
    pCamHw->_ldcv_drv_mem_ctx.mem_info = (void*)(pCamHw->ldcv_mem_info_array);

    pCamHw->_cac_drv_mem_ctx.type     = MEM_TYPE_CAC;
    pCamHw->_cac_drv_mem_ctx.ops_ctx  = pCamHw;
    pCamHw->_cac_drv_mem_ctx.mem_info = (void*)(pCamHw->cac_mem_info_array);

    pCamHw->_dbg_drv_mem_ctx.type     = MEM_TYPE_DBG_INFO;
    pCamHw->_dbg_drv_mem_ctx.ops_ctx  = pCamHw;
    pCamHw->_dbg_drv_mem_ctx.mem_info = (void*)(pCamHw->dbg_mem_info_array);
    pCamHw->_isp_stream_status        = ISP_STREAM_STATUS_INVALID;

    pCamHw->_btnr_drv_mem_ctx.type     = MEM_TYPE_BTNR;
    pCamHw->_btnr_drv_mem_ctx.ops_ctx  = pCamHw;
    pCamHw->_btnr_drv_mem_ctx.mem_info = (void*)(pCamHw->btnr_mem_info_array);

    {
        // init pool
        AiqPoolConfig_t effIspPoolCfg;
        AiqPoolItem_t* pItem         = NULL;
        int i                        = 0;
        effIspPoolCfg._name          = "EffIspParamsPool";
        effIspPoolCfg._item_nums     = CAMHWISP_EFFECT_ISP_POOL_NUM;
        effIspPoolCfg._item_size     = sizeof(aiq_isp_effect_params_t);
        pCamHw->mEffectIspParamsPool = aiqPool_init(&effIspPoolCfg);
        if (!pCamHw->mEffectIspParamsPool)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "cId:%d init %s error", pCamHw->mCamPhyId,
                            effIspPoolCfg._name);
        for (i = 0; i < pCamHw->mEffectIspParamsPool->_item_nums; i++) {
            pItem = &pCamHw->mEffectIspParamsPool->_item_array[i];
            AIQ_REF_BASE_INIT(&((aiq_isp_effect_params_t*)(pItem->_pData))->_ref_base, pItem,
                              aiqPoolItem_ref, aiqPoolItem_unref);
        }
    }

    {
        // init map
        AiqMapConfig_t effIspMapCfg;
        effIspMapCfg._name              = "ispEffMap";
        effIspMapCfg._key_type          = AIQ_MAP_KEY_TYPE_UINT32;
        effIspMapCfg._item_nums         = CAMHWISP_EFFECT_ISP_POOL_NUM;
        effIspMapCfg._item_size         = sizeof(aiq_isp_effect_params_t*);
        pCamHw->_effecting_ispparam_map = aiqMap_init(&effIspMapCfg);
        if (!pCamHw->_effecting_ispparam_map)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "cId:%d init %s error", pCamHw->mCamPhyId,
                            effIspMapCfg._name);
    }

    s_info = &pItem->data;

    pCamHw->_mSensorDev = (AiqSensorHw_t*)aiq_mallocz(sizeof(AiqSensorHw_t));
    if (!pCamHw->_mSensorDev) {
        goto fail;
    }
    AiqSensorHw_init(pCamHw->_mSensorDev, s_info->device_name, pCamHw->mCamPhyId);
    AiqSensorHw_setCamHw(pCamHw->_mSensorDev, pCamHw);
    pCamHw->_mSensorDev->open(pCamHw->_mSensorDev);
    // TODO: should be set before init
    AiqCamHw_setTbInfo(pCamHw, &pCamHw->mTbInfo);
    pCamHw->_mIspParamsCvt = (AiqIspParamsCvt_t*)aiq_mallocz(sizeof(AiqIspParamsCvt_t));
    if (!pCamHw->_mIspParamsCvt) {
        goto fail;
    }

    AiqIspParamsCvt_init(pCamHw->_mIspParamsCvt);
    AiqIspParamsCvt_setCamPhyId(pCamHw->_mIspParamsCvt, pCamHw->mCamPhyId);

    strncpy(pCamHw->sns_name, sns_ent_name, sizeof(pCamHw->sns_name) - 1);
    //
    // normal env.
    if (s_info->linked_to_isp) {
        pCamHw->_linked_to_isp  = true;
        pCamHw->_linked_to_1608 = false;
    } else {
        pCamHw->_linked_to_isp  = false;
        pCamHw->_linked_to_1608 = false;
    }

    // 1608 sensor env.
    if (s_info->linked_to_1608) {
        pCamHw->_linked_to_isp  = false;
        pCamHw->_linked_to_1608 = true;
        // [baron] Record the number of use sensors(valid 1608 sensor)
        g_rk1608_share_inf.en_sns_num++;
    }

    // serdes sensor
    if (s_info->linked_to_serdes) {
        pCamHw->_linked_to_isp    = false;
        pCamHw->_linked_to_1608   = false;
        pCamHw->_linked_to_serdes = true;
#if 0
        if (!pCamHw->use_rkrawstream)
            LOGK_CAMHW("linked to serdes, force to use_rkrawstream");
        pCamHw->use_rkrawstream = true;
#endif
        if (pCamHw->mIsListenStrmEvt)
            LOGK_CAMHW("linked to serdes, set mIsListenStrmEvt to false");
        pCamHw->mIsListenStrmEvt = false;
    } else {
        pCamHw->_linked_to_serdes = false;
    }

    // one sensor linked to multi isp
    if (s_info->split) {
        pCamHw->_linked_to_isp       = false;
        pCamHw->_linked_to_1608      = false;
        pCamHw->_linked_to_serdes    = false;
        pCamHw->_linked_to_multi_isp = true;
    } else {
        pCamHw->_linked_to_multi_isp = false;
    }
    // airms
    pCamHw->_airms_en = false;
    // aiynr
    pCamHw->_aiynr_en = false;

    pCamHw->mIspCoreDev = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
    if (!pCamHw->mIspCoreDev) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
        goto fail;
    }

    AiqV4l2SubDevice_init(pCamHw->mIspCoreDev, s_info->isp_info->isp_dev_path);
    pCamHw->mIspCoreDev->_v4l_base.open(&pCamHw->mIspCoreDev->_v4l_base, false);

    pCamHw->mIspStatsDev = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
    if (!pCamHw->mIspStatsDev) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
        goto fail;
    }
    AiqV4l2Device_init(pCamHw->mIspStatsDev, s_info->isp_info->stats_path);
    pCamHw->mIspStatsDev->open(pCamHw->mIspStatsDev, false);

    pCamHw->mIspParamsDev = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
    if (!pCamHw->mIspParamsDev) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
        goto fail;
    }
    AiqV4l2Device_init(pCamHw->mIspParamsDev, s_info->isp_info->input_params_path);
    pCamHw->mIspParamsDev->open(pCamHw->mIspParamsDev, false);

    if (strlen(s_info->module_lens_dev_name) > 0) {
        pCamHw->mLensDev = (AiqLensHw_t*)aiq_mallocz(sizeof(AiqLensHw_t));
        if (!pCamHw->mLensDev) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }
        AiqLensHw_init(pCamHw->mLensDev, s_info->module_lens_dev_name);
        pCamHw->mLensDev->open(pCamHw->mLensDev);
    }
#if defined(ISP_HW_V20) || defined(ISP_HW_V21)
    else {
        pCamHw->mLensDev = (AiqLensHw_t*)aiq_mallocz(sizeof(AiqLensHw_t));
        if (!pCamHw->mLensDev) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }
        // af soft stats need this device on 356x 1126
        AiqLensHw_init(pCamHw->mLensDev, NULL);
    }
#endif

    if (strlen(s_info->module_ircut_dev_name) > 0) {
        pCamHw->mIrcutDev = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
        if (!pCamHw->mIrcutDev) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }
        AiqV4l2SubDevice_init((AiqV4l2SubDevice_t*)(pCamHw->mIrcutDev),
                              s_info->module_ircut_dev_name);
        pCamHw->mIrcutDev->_v4l_base.open(&pCamHw->mIrcutDev->_v4l_base, false);
    }

    if (pCamHw->_linked_to_serdes || pCamHw->_linked_to_multi_isp) {
        if (strlen(s_info->isp_info->linked_vicap_sd_path) > 0) {
            pCamHw->mVicapItfDev = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
            AiqV4l2SubDevice_init((AiqV4l2SubDevice_t*)(pCamHw->mVicapItfDev),
                                  s_info->isp_info->linked_vicap_sd_path);
        } else {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "mVicapItfDev is null! \n");
        }
        pCamHw->mVicapItfDev->_v4l_base.open(&pCamHw->mVicapItfDev->_v4l_base, false);
        if (pCamHw->_linked_to_multi_isp) {
            pCamHw->_mSensorDev->setVicapExpFd(pCamHw->_mSensorDev, AiqV4l2Device_getFd(&pCamHw->mVicapItfDev->_v4l_base));
        }
    } else if (!pCamHw->_linked_to_isp) {
        if (strlen(s_info->cif_info->mipi_csi2_sd_path) > 0) {
            pCamHw->_cif_csi2_sd = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
            AiqV4l2SubDevice_init((AiqV4l2SubDevice_t*)(pCamHw->_cif_csi2_sd),
                                  s_info->cif_info->mipi_csi2_sd_path);
        } else if (strlen(s_info->cif_info->lvds_sd_path) > 0) {
            pCamHw->_cif_csi2_sd = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
            AiqV4l2SubDevice_init((AiqV4l2SubDevice_t*)(pCamHw->_cif_csi2_sd),
                                  s_info->cif_info->lvds_sd_path);
        } else if (strlen(s_info->cif_info->dvp_sof_sd_path) > 0) {
            pCamHw->_cif_csi2_sd = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
            AiqV4l2SubDevice_init((AiqV4l2SubDevice_t*)(pCamHw->_cif_csi2_sd),
                                  s_info->cif_info->dvp_sof_sd_path);
        } else {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "_cif_csi2_sd is null! \n");
        }
        pCamHw->_cif_csi2_sd->_v4l_base.open(&pCamHw->_cif_csi2_sd->_v4l_base, false);
    }

#if defined(RKAIQ_ENABLE_SPSTREAM)
    pCamHw->mIspSpDev = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
    AiqV4l2Device_init(pCamHw->mIspSpDev, s_info->isp_info->self_path);
    pCamHw->mIspSpDev->open(pCamHw->mIspSpDev, false);
    pCamHw->mSpStreamUnit = (AiqSPStreamProcUnit_t*)aiq_mallocz(sizeof(AiqSPStreamProcUnit_t));
    AiqSPStreamProcUnit_init(pCamHw->mSpStreamUnit, pCamHw->mIspSpDev, ISP_POLL_SP,
                             g_mIspHwInfos.hw_ver_info.isp_ver);
    AiqSPStreamProcUnit_set_devices(pCamHw->mSpStreamUnit, pCamHw, pCamHw->mIspCoreDev,
                                    pCamHw->mLensDev);
#endif
#if RKAIQ_HAVE_PDAF
    pCamHw->mPdafStreamUnit =
        (AiqPdafStreamProcUnit_t*)aiq_mallocz(sizeof(AiqPdafStreamProcUnit_t));
    AiqPdafStreamProcUnit_init(pCamHw->mPdafStreamUnit, ISP_POLL_PDAF_STATS);
    AiqPdafStreamProcUnit_set_devices(pCamHw->mPdafStreamUnit, pCamHw);
#endif
    if (pCamHw->mAiqPreCtrlInfo.mode == RK_AIQ_CONTROL_ISP_PARAM_ONLY ||
            pCamHw->mAiqPreCtrlInfo.mode == RK_AIQ_CONTROL_CIS_ISP_PARAM) {
        pCamHw->use_rkrawstream  = true;
        pCamHw->mIsListenStrmEvt = false;
        pCamHw->mNoReadBack      = true;
    } else if (pCamHw->mAiqPreCtrlInfo.mode == RK_AIQ_CONTROL_CIS_ISP_PARAM_AND_RAW) {
        pCamHw->mNoReadBack = 0;
    }

    if (!pCamHw->use_rkrawstream) {
        int i       = 0;
        int buf_cnt = 0;
        for (; i < pCamHw->mDevBufCntMap_size; i++) {
            if (strstr(pCamHw->mDevBufCntMap->_devName, "rkraw_tx"))
                break;
            else if (strstr(pCamHw->mDevBufCntMap->_devName, "rkraw_rx"))
                break;
            else if (strstr(pCamHw->mDevBufCntMap->_devName, "stream_cif_mipi_id"))
                break;
            else if (strstr(pCamHw->mDevBufCntMap->_devName, "rkisp_rawwr"))
                break;
        }

        if (i < pCamHw->mDevBufCntMap_size) {
            buf_cnt = pCamHw->mDevBufCntMap[i]._bufCnts;
        }

        if (!pCamHw->_linked_to_1608) {
            pCamHw->mRawProcUnit =
                (AiqRawStreamProcUnit_t*)aiq_mallocz(sizeof(AiqRawStreamProcUnit_t));
            AiqRawStreamProcUnit_init(pCamHw->mRawProcUnit, s_info, pCamHw->_linked_to_isp,
                                      buf_cnt);

            pCamHw->mRawCapUnit =
                (AiqRawStreamCapUnit_t*)aiq_mallocz(sizeof(AiqRawStreamCapUnit_t));
            AiqRawStreamCapUnit_init(pCamHw->mRawCapUnit, s_info, pCamHw->_linked_to_isp, buf_cnt);

            // set sensor stream flag.
            AiqRawStreamCapUnit_setSensorCategory(pCamHw->mRawCapUnit, false);
            AiqRawStreamProcUnit_setSensorCategory(pCamHw->mRawProcUnit, false);
            AiqRawStreamCapUnit_set_devices(pCamHw->mRawCapUnit, pCamHw->mIspCoreDev, pCamHw,
                                            pCamHw->mRawProcUnit, NULL);
        } else {
            // 1608 sensor
            if (NULL == g_rk1608_share_inf.raw_cap_unit) {
                // [baron] just new buffer in 1st.
                pCamHw->mRawCapUnit =
                    (AiqRawStreamCapUnit_t*)aiq_mallocz(sizeof(AiqRawStreamCapUnit_t));
                AiqRawStreamCapUnit_init(pCamHw->mRawCapUnit, s_info, pCamHw->_linked_to_isp,
                                         buf_cnt);
                g_rk1608_share_inf.raw_cap_unit = (aiq_autoptr_t*)(pCamHw->mRawCapUnit);
            }

            pCamHw->mRawProcUnit =
                (AiqRawStreamProcUnit_t*)aiq_mallocz(sizeof(AiqRawStreamProcUnit_t));
            AiqRawStreamProcUnit_init(pCamHw->mRawProcUnit, s_info, pCamHw->_linked_to_isp,
                                      buf_cnt);
            // [baron] save multi rx addr for 1 tx.
            g_rk1608_share_inf.raw_proc_unit[pCamHw->mCamPhyId] = pCamHw->mRawProcUnit;

            // update tx by bakeup tx.
            pCamHw->mRawCapUnit = (AiqRawStreamCapUnit_t*)g_rk1608_share_inf.raw_cap_unit;
            AiqRawStreamCapUnit_setSensorCategory(pCamHw->mRawCapUnit, true);
            AiqRawStreamProcUnit_setSensorCategory(pCamHw->mRawProcUnit, true);
            AiqRawStreamCapUnit_set_devices(pCamHw->mRawCapUnit, pCamHw->mIspCoreDev, pCamHw,
                                            pCamHw->mRawProcUnit, NULL);
        }

#if RKAIQ_HAVE_AIRMS
        if (s_info->aiisp_info) {
            pCamHw->mAiRmsProcUnit =
                (AiqAiRmsStreamProcUnit_t*)aiq_mallocz(sizeof(AiqAiRmsStreamProcUnit_t));
            AiqAiRmsStreamProcUnit_init(pCamHw, pCamHw->mAiRmsProcUnit, s_info->aiisp_info);
        }
#endif

        AiqRawStreamProcUnit_set_devices(pCamHw->mRawProcUnit, pCamHw->mIspCoreDev, pCamHw);
        AiqRawStreamCapUnit_setCamPhyId(pCamHw->mRawCapUnit, pCamHw->mCamPhyId);
        AiqRawStreamProcUnit_setCamPhyId(pCamHw->mRawProcUnit, pCamHw->mCamPhyId);
    }
    // cif scale
    if (!pCamHw->_linked_to_isp && !pCamHw->_linked_to_1608 && !pCamHw->_linked_to_serdes) {
        if (strlen(s_info->cif_info->mipi_scl0))
            pCamHw->mCifScaleStream = (AiqCifSclStream_t*)aiq_mallocz(sizeof(AiqCifSclStream_t));
    }
    //
    // isp stats
    pCamHw->mIspStatsStream = (AiqStatsStream_t*)aiq_mallocz(sizeof(AiqStatsStream_t));
    AiqStatsStream_init(pCamHw->mIspStatsStream, pCamHw->mIspStatsDev, ISP_POLL_3A_STATS);
    pCamHw->mIspStatsStream->_base.setPollCallback(&pCamHw->mIspStatsStream->_base,
            &pCamHw->mPollCb);
    pCamHw->mIspStatsStream->set_event_handle_dev(pCamHw->mIspStatsStream, pCamHw->_mSensorDev);
    if (pCamHw->mLensDev) {
        pCamHw->mIspStatsStream->set_focus_handle_dev(pCamHw->mIspStatsStream, pCamHw->mLensDev);
    }
    pCamHw->mIspStatsStream->set_rx_handle_dev(pCamHw->mIspStatsStream, pCamHw);
    pCamHw->mIspStatsStream->_base.setCamPhyId(&pCamHw->mIspStatsStream->_base, pCamHw->mCamPhyId);

    // AIISP
    pCamHw->setAiispMode       = AiqCamHw_setAiispMode;
    pCamHw->aiisp_processing   = AiqCamHw_aiisp_processing;
    pCamHw->read_aiisp_result  = AiqCamHw_read_aiisp_result;
    AiqAiIsp_Init(&pCamHw->lib_aiisp_);

    //AIBNR
#if RKAIQ_HAVE_AIBNR
    pCamHw->aibnr_init         = AiqCamHw_aibnr_init;
    pCamHw->aibnr_prepare      = AiqCamHw_aibnr_prepare;
    pCamHw->aibnr_start        = AiqCamHw_aibnr_start;
    pCamHw->aibnr_stop         = AiqCamHw_aibnr_stop;
    pCamHw->aibnr_deinit       = AiqCamHw_aibnr_deinit;
    pCamHw->aibnr_getParamsBuf = AiqCamHw_aibnr_getParamsBuf;
    pCamHw->aibnr_updateParams = AiqCamHw_aibnr_updateParams;
    pCamHw->aibnr_doNrnn       = AiqCamHw_aibnr_doNrnn;
    pCamHw->aibnr_doIspBe      = AiqCamHw_aibnr_doIspBe;
    pCamHw->aibnr_setMemMode   = AiqCamHw_aibnr_setMemMode;
    pCamHw->aibnr_setIspBufInf = AiqCamHw_aibnr_setIspBufInf;
    pCamHw->aibnr_setParamInf  = AiqCamHw_aibnr_setParamInf;
    pCamHw->aibnr_setModelInf  = AiqCamHw_aibnr_setModelInf;
    pCamHw->aibnr_setLinecnt   = AiqCamHw_aibnr_setLinecnt;
    pCamHw->aibnr_clrIqParam   = AiqCamHw_aibnr_clrIqParam;

    pCamHw->btnrWgtReproc_updateEn = AiqCamHw_btnrWgtReproc_updateEn;
    pCamHw->mAibnrBufCnt.isp_fe    = AIBNR_IIR_BUF_CNT;
    pCamHw->mAibnrBufCnt.isp_be    = AIBNR_AIISP_BUF_CNT;
#endif

    //AIYNR
#if RKAIQ_HAVE_AIYNR
    pCamHw->aiynr_init         = AiqCamHw_aiynr_init;
    pCamHw->aiynr_prepare      = AiqCamHw_aiynr_prepare;
    pCamHw->aiynr_start        = AiqCamHw_aiynr_start;
    pCamHw->aiynr_stop         = AiqCamHw_aiynr_stop;
    pCamHw->aiynr_deinit       = AiqCamHw_aiynr_deinit;
    pCamHw->aiynr_getParamsBuf = AiqCamHw_aiynr_getParamsBuf;
    pCamHw->aiynr_updateParams = AiqCamHw_aiynr_updateParams;
    pCamHw->aiynr_doNrnn       = AiqCamHw_aiynr_doNrnn;
    pCamHw->aiynr_doIspBe      = AiqCamHw_aiynr_doIspBe;
    pCamHw->aiynr_setIspBufInf = AiqCamHw_aiynr_setIspBufInf;
    pCamHw->aiynr_setParamInf  = AiqCamHw_aiynr_setParamInf;
    pCamHw->aiynr_setModelInf  = AiqCamHw_aiynr_setModelInf;
    pCamHw->aiynr_setLinecnt   = AiqCamHw_aiynr_setLinecnt;
    pCamHw->aiynr_getYnrbufInf = AiqCamHw_aiynr_getYnrbufInf;
#endif

#if defined(RKAIQ_HAVE_AIBNR) || defined(RKAIQ_HAVE_AIYNR)
    if (s_info->aiisp_info) {
        pCamHw->mAiIspCoreDev = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
        if (!pCamHw->mAiIspCoreDev) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }

        AiqV4l2SubDevice_init(pCamHw->mAiIspCoreDev, s_info->isp_info->isp_dev_path);
        pCamHw->mAiIspCoreDev->_v4l_base.open(&pCamHw->mAiIspCoreDev->_v4l_base, false);

        pCamHw->mAiIspDev = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
        if (!pCamHw->mAiIspDev) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }
        AiqV4l2Device_init(pCamHw->mAiIspDev, s_info->aiisp_info->video_path);
        pCamHw->mAiIspDev->open(pCamHw->mAiIspDev, false);

        pCamHw->mAiIspSubDev = (AiqV4l2SubDevice_t*)aiq_mallocz(sizeof(AiqV4l2SubDevice_t));
        if (!pCamHw->mAiIspSubDev) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: alloc fail !", __LINE__);
            goto fail;
        }
        AiqV4l2SubDevice_init(pCamHw->mAiIspSubDev, s_info->aiisp_info->subdev_path);
        pCamHw->mAiIspSubDev->_v4l_base.open(&pCamHw->mAiIspSubDev->_v4l_base, false);
    } else {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "%d: can not find aiisp device !", __LINE__);
    }
#endif

    pCamHw->mPollCb._pCtx             = pCamHw;
    pCamHw->mPollCb.poll_buffer_ready = _poll_buffer_ready;
    pCamHw->mPollCb.poll_event_ready  = NULL;

    pCamHw->mFpnStreamUnit = NULL;

#if RKAIQ_HAVE_FPNSW
    // fpn
    pCamHw->mFpnStreamUnit = (AiqFpnSwStreamProcUnit_t*)aiq_mallocz(sizeof(AiqFpnSwStreamProcUnit_t));
    if (pCamHw->mFpnStreamUnit) {
        AiqFpnSwStreamProcUnit_init(pCamHw->mFpnStreamUnit, pCamHw->_mSensorDev);
        AiqFpnSwStreamProcUnit_setCamPhyId(pCamHw->mFpnStreamUnit, pCamHw->mCamPhyId);
    }
#endif

    pCamHw->mMemcStream    = NULL;
#if RKAIQ_HAVE_MEMC
    // memc
    pCamHw->mMemcStream = (AiqMemcManager_t*)aiq_mallocz(sizeof(AiqMemcManager_t));
    if (pCamHw->mMemcStream) {
        isp_drv_share_mem_ops_t *mem_ops = NULL;
        AiqCamHw_getShareMemOps(pCamHw, &mem_ops);
        AiqMemcManager_init(pCamHw->mMemcStream, &pCamHw->_drv_share_mem_ops, pCamHw);
    }
#endif

#ifdef ISP_HW_V35
    pCamHw->mBtnrDsReproc = (AiqBtnrDsReproc_t*)aiq_mallocz(sizeof(AiqBtnrDsReproc_t));
    if (pCamHw->mBtnrDsReproc) {
        AiqBtnrDsReproc_init(pCamHw->mBtnrDsReproc, pCamHw->mIspCoreDev);
    }
#endif

    // dumpsys
#if RKAIQ_HAVE_DUMPSYS
    pCamHw->dump = AiqCamHw_dump;
    AiqCamHwBase_initNotifier(pCamHw);
#endif

    pCamHw->_state = CAM_HW_STATE_INITED;
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
fail:
    return XCAM_RETURN_ERROR_FAILED;
}

void AiqCamHwBase_deinit(AiqCamHwBase_t* pCamHw) {
    ENTER_CAMHW_FUNCTION();

    if (pCamHw->mIspSofStream) {
        AiqSofEvtStream_deinit(pCamHw->mIspSofStream);
        aiq_free(pCamHw->mIspSofStream);
        pCamHw->mIspSofStream = NULL;
    }

#ifdef RKAIQ_HAVE_AIBNR
    AiqCamHw_aibnr_deinit(pCamHw);
#endif
#ifdef RKAIQ_HAVE_AIYNR
    AiqCamHw_aiynr_deinit(pCamHw);
#endif

#if !defined(RKAIQ_HAVE_AIBNR) && !defined(RKAIQ_HAVE_AIYNR)
    if (pCamHw->use_aiisp) {
        if (pCamHw->mIspAiispStream) {
            AiqAiIspStream_deinit(pCamHw->mIspAiispStream);
            aiq_free(pCamHw->mIspAiispStream);
            pCamHw->mIspAiispStream = NULL;
        }
        pCamHw->lib_aiisp_.Deinit(&pCamHw->lib_aiisp_);
        struct AiispOps* ops = AiqAiIsp_GetOps(&pCamHw->lib_aiisp_);
        ops->aiisp_deinit(pCamHw->aiisp_param);
        if (pCamHw->aiisp_param) {
            aiq_free(pCamHw->aiisp_param);
            pCamHw->aiisp_param = NULL;
        }
    }
#endif

    if (pCamHw->mIspStremEvtTh) {
        AiqStreamEventPollThread_deinit(pCamHw->mIspStremEvtTh);
        aiq_free(pCamHw->mIspStremEvtTh);
        pCamHw->mIspStremEvtTh = NULL;
    }

    if (pCamHw->mIspStatsStream) {
        AiqStatsStream_deinit(pCamHw->mIspStatsStream);
        aiq_free(pCamHw->mIspStatsStream);
        pCamHw->mIspStatsStream = NULL;
    }

    if (pCamHw->mCifScaleStream) {
        AiqCifSclStream_deinit(pCamHw->mCifScaleStream);
        aiq_free(pCamHw->mCifScaleStream);
        pCamHw->mCifScaleStream = NULL;
    }

    if (pCamHw->mRawProcUnit) {
        AiqRawStreamProcUnit_deinit(pCamHw->mRawProcUnit);
        aiq_free(pCamHw->mRawProcUnit);
        pCamHw->mRawProcUnit = NULL;
    }

    if (pCamHw->mRawCapUnit) {
        AiqRawStreamCapUnit_deinit(pCamHw->mRawCapUnit);
        aiq_free(pCamHw->mRawCapUnit);
        pCamHw->mRawCapUnit = NULL;
    }
#if RKAIQ_HAVE_AIRMS
    if (pCamHw->mAiRmsProcUnit) {
        AiqAiRmsStreamProcUnit_deinit(pCamHw->mAiRmsProcUnit);
        aiq_free(pCamHw->mAiRmsProcUnit);
        pCamHw->mAiRmsProcUnit = NULL;
    }
#endif

#if RKAIQ_HAVE_FPNSW
    if (pCamHw->mFpnStreamUnit) {
        AiqFpnSwStreamProcUnit_deinit(pCamHw->mFpnStreamUnit);
        aiq_free(pCamHw->mFpnStreamUnit);
        pCamHw->mFpnStreamUnit = NULL;
    }
#endif

#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcStream) {
        AiqMemcManager_deinit(pCamHw->mMemcStream);
        aiq_free(pCamHw->mMemcStream);
        pCamHw->mMemcStream = NULL;
    }

    if (pCamHw->mIspMemcIirStream) {
        AiqMemcIirStream_deinit(pCamHw->mIspMemcIirStream);
        aiq_free(pCamHw->mIspMemcIirStream);
        pCamHw->mIspMemcIirStream = NULL;
    }
#endif

#ifdef ISP_HW_V35
    if (pCamHw->mBtnrDsReproc) {
        AiqBtnrDsReproc_deinit(pCamHw->mBtnrDsReproc);
        aiq_free(pCamHw->mBtnrDsReproc);
        pCamHw->mBtnrDsReproc = NULL;
    }
#endif

#if RKAIQ_HAVE_PDAF
    if (pCamHw->mPdafStreamUnit) {
        AiqPdafStreamProcUnit_deinit(pCamHw->mPdafStreamUnit);
        aiq_free(pCamHw->mPdafStreamUnit);
        pCamHw->mPdafStreamUnit = NULL;
    }
#endif
#if defined(RKAIQ_ENABLE_SPSTREAM)
    if (pCamHw->mIspSpDev) {
        AiqSPStreamProcUnit_deinit(pCamHw->mSpStreamUnit);
        pCamHw->mIspSpDev->close(pCamHw->mIspSpDev);
        if (pCamHw->mIspSpDev) {
            aiq_free(pCamHw->mIspSpDev);
            pCamHw->mIspSpDev = NULL;
        }
        if (pCamHw->mSpStreamUnit) {
            aiq_free(pCamHw->mSpStreamUnit);
            pCamHw->mSpStreamUnit = NULL;
        }
    }
#endif
    if (pCamHw->_cif_csi2_sd) {
        AiqV4l2SubDevice_deinit(pCamHw->_cif_csi2_sd);
        aiq_free(pCamHw->_cif_csi2_sd);
        pCamHw->_cif_csi2_sd = NULL;
    }
    if (pCamHw->mIrcutDev) {
        AiqV4l2SubDevice_deinit(pCamHw->mIrcutDev);
        aiq_free(pCamHw->mIrcutDev);
        pCamHw->mIrcutDev = NULL;
    }
    if (pCamHw->mLensDev) {
        AiqLensHw_deinit(pCamHw->mLensDev);
        aiq_free(pCamHw->mLensDev);
        pCamHw->mLensDev = NULL;
    }
    if (pCamHw->mIspParamsDev) {
        AiqV4l2Device_deinit(pCamHw->mIspParamsDev);
        aiq_free(pCamHw->mIspParamsDev);
        pCamHw->mIspParamsDev = NULL;
    }
    if (pCamHw->mIspStatsDev) {
        AiqV4l2Device_deinit(pCamHw->mIspStatsDev);
        aiq_free(pCamHw->mIspStatsDev);
        pCamHw->mIspStatsDev = NULL;
    }
    if (pCamHw->_mIspParamsCvt) {
        AiqIspParamsCvt_deinit(pCamHw->_mIspParamsCvt);
        aiq_free(pCamHw->_mIspParamsCvt);
        pCamHw->_mIspParamsCvt = NULL;
    }
    if (pCamHw->mIspCoreDev) {
        AiqV4l2SubDevice_deinit(pCamHw->mIspCoreDev);
        aiq_free(pCamHw->mIspCoreDev);
        pCamHw->mIspCoreDev = NULL;
    }
    if (pCamHw->mVicapItfDev) {
        AiqV4l2SubDevice_deinit(pCamHw->mVicapItfDev);
        aiq_free(pCamHw->mVicapItfDev);
        pCamHw->mVicapItfDev = NULL;
    }
    if (pCamHw->_mSensorDev) {
        AiqSensorHw_deinit(pCamHw->_mSensorDev);
        aiq_free(pCamHw->_mSensorDev);
        pCamHw->_mSensorDev = NULL;
    }
    if (pCamHw->_effecting_ispparam_map) {
        aiqMap_deinit(pCamHw->_effecting_ispparam_map);
        pCamHw->_effecting_ispparam_map = NULL;
    }
    if (pCamHw->mEffectIspParamsPool) {
        aiqPool_deinit(pCamHw->mEffectIspParamsPool);
        pCamHw->mEffectIspParamsPool = NULL;
    }

    aiqMutex_deInit(&pCamHw->_stop_cond_mutex);
    aiqMutex_deInit(&pCamHw->_mem_mutex);
    aiqMutex_deInit(&pCamHw->_isp_params_cfg_mutex);
    aiqCond_deInit(&pCamHw->_sync_done_cond);

    EXIT_CAMHW_FUNCTION();
}

static XCamReturn _pixFmt2Bpp(uint32_t pixFmt, int8_t *bpp) {
    switch (pixFmt) {
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SRGGB8:
        *bpp = 8;
        break;
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_Y10:
        *bpp = 10;
        break;
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SRGGB12:
        *bpp = 12;
        break;
    case V4L2_PIX_FMT_SBGGR14:
    case V4L2_PIX_FMT_SGBRG14:
    case V4L2_PIX_FMT_SGRBG14:
    case V4L2_PIX_FMT_SRGGB14:
        *bpp = 14;
        break;
    case V4L2_PIX_FMT_SBGGR16:
    case V4L2_PIX_FMT_SGBRG16:
    case V4L2_PIX_FMT_SGRBG16:
    case V4L2_PIX_FMT_SRGGB16:
        *bpp = 16;
        break;
    default:
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "unknown format");
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _setupPipelineFmtCif(AiqCamHwBase_t* pCamHw,
                                       struct v4l2_subdev_selection sns_sd_sel,
                                       struct v4l2_subdev_format* sns_sd_fmt,
                                       __u32 sns_v4l_pix_fmt) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int8_t bpp     = 0;
    struct v4l2_subdev_format isp_sink_fmt;
    struct v4l2_subdev_selection aSelection;
    struct v4l2_subdev_format isp_src_fmt;

    _pixFmt2Bpp(sns_v4l_pix_fmt, &bpp);

    bool multi_isp_align = false;
    int tx_word_align_mode = CSI_LVDS_MEM_COMPACT;
    int rx_word_align_mode = CSI_MEM_COMPACT;

    multi_isp_align = g_mIsMultiIspMode && !!(((sns_sd_sel.r.width / 2 - RKMOUDLE_UNITE_EXTEND_PIXEL) * bpp / 8) & 0xf);
    if (multi_isp_align) {
        // for multi isp, need to align to 16 bytes
        tx_word_align_mode = CSI_LVDS_MEM_WORD_HIGH_ALIGN;
        rx_word_align_mode = CSI_MEM_WORD_BIG_ALIGN;
    }

    bool algo_need_align = false;
#ifdef RKAIQ_HAVE_FPNSW
    if (pCamHw->mFpnStreamUnit) {
        bool fpn_uncompact_mode = false;
        AiqFpnSwStreamProcUnit_getRawAlignMode(pCamHw->mFpnStreamUnit, &algo_need_align, &fpn_uncompact_mode);
        if (algo_need_align) {
            if (fpn_uncompact_mode) {
                tx_word_align_mode = CSI_LVDS_MEM_WORD_LOW_ALIGN;
                rx_word_align_mode = CSI_MEM_WORD_LITTLE_ALIGN;
            } else {
                if (multi_isp_align) {
                    tx_word_align_mode = CSI_LVDS_MEM_WORD_LOW_ALIGN;
                    rx_word_align_mode = CSI_MEM_WORD_LITTLE_ALIGN;
                    AiqFpnSwStreamProcUnit_setWorkingMode(pCamHw->mFpnStreamUnit, true);
                    LOGK_CAMHW("force fpnSw working on high bw mode, set raw align to uncompact");
                } else {
                    tx_word_align_mode = CSI_LVDS_MEM_COMPACT;
                    rx_word_align_mode = CSI_MEM_COMPACT;
                }
            }
        }

        AiqFpnSwStreamProcUnit_setFormat(pCamHw->mFpnStreamUnit, sns_sd_sel.r.width, sns_sd_sel.r.height, bpp);
    }
#endif

    if ((algo_need_align || multi_isp_align) && !pCamHw->mNoReadBack) {
        ret = AiqRawStreamCapUnit_set_csi_mem_word_align_mode(pCamHw->mRawCapUnit, tx_word_align_mode);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "tx set csi_mem_word_align_mode %d failed!\n", tx_word_align_mode);
            return ret;
        }

        ret = AiqRawStreamProcUnit_set_csi_mem_word_align_mode(pCamHw->mRawProcUnit, rx_word_align_mode);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "rx set csi_mem_word_align_mode %d failed!\n", rx_word_align_mode);
            return ret;
        }

        LOGK_CAMHW_SUBM(ISP20HW_SUBM, "set tx/rx to csi mem word align mode %d/%d", tx_word_align_mode, rx_word_align_mode);
    }

    // TODO: set cif crop according to sensor crop bounds
    if (!pCamHw->_linked_to_1608) {
        AiqRawStreamCapUnit_set_tx_format2(pCamHw->mRawCapUnit, &sns_sd_sel, sns_v4l_pix_fmt);
    } else {
        // [baron] Only processed the first time
        if (g_rk1608_share_inf.first_en[pCamHw->mCamPhyId]) {
            // [baron] Timing issue, for 1608.
            AiqRawStreamCapUnit_set_tx_format2(pCamHw->mRawCapUnit, &sns_sd_sel, sns_v4l_pix_fmt);
        }
    }

    AiqRawStreamProcUnit_set_rx_format2(pCamHw->mRawProcUnit, &sns_sd_sel, sns_v4l_pix_fmt);

    // set cif scale fmt
    if (pCamHw->mCifScaleStream) {
        AiqCifSclStream_set_format2(pCamHw->mCifScaleStream, &sns_sd_sel, sns_v4l_pix_fmt, bpp);
    }

    // set isp sink fmt, same as sensor bounds - crop
    memset(&isp_sink_fmt, 0, sizeof(isp_sink_fmt));
    isp_sink_fmt.pad   = 0;
    isp_sink_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    ret                = pCamHw->mIspCoreDev->getFormat(pCamHw->mIspCoreDev, &isp_sink_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev fmt failed !\n");
        return ret;
    }
    isp_sink_fmt.format.width  = sns_sd_sel.r.width;
    isp_sink_fmt.format.height = sns_sd_sel.r.height;
    isp_sink_fmt.format.code   = sns_sd_fmt->format.code;

    ret = pCamHw->mIspCoreDev->setFormat(pCamHw->mIspCoreDev, &isp_sink_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev fmt failed !\n");
        return ret;
    }

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp sink fmt info: fmt 0x%x, %dx%d !", isp_sink_fmt.format.code,
                    isp_sink_fmt.format.width, isp_sink_fmt.format.height);

    // set selection, isp needn't do the crop
    memset(&aSelection, 0, sizeof(aSelection));

    aSelection.which    = V4L2_SUBDEV_FORMAT_ACTIVE;
    aSelection.pad      = 0;
    aSelection.flags    = 0;
    aSelection.target   = V4L2_SEL_TGT_CROP;
    aSelection.r.width  = sns_sd_sel.r.width;
    aSelection.r.height = sns_sd_sel.r.height;
    aSelection.r.left   = 0;
    aSelection.r.top    = 0;
    ret                 = pCamHw->mIspCoreDev->set_selection(pCamHw->mIspCoreDev, &aSelection);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev crop failed !\n");
        return ret;
    }

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp sink crop info: %dx%d@%d,%d !", aSelection.r.width,
                    aSelection.r.height, aSelection.r.left, aSelection.r.top);

    // set isp rkisp-isp-subdev src crop
    aSelection.pad = 2;
#if 1  // isp src has no crop
    ret = pCamHw->mIspCoreDev->set_selection(pCamHw->mIspCoreDev, &aSelection);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev source crop failed !\n");
        return ret;
    }
#endif
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp src crop info: %dx%d@%d,%d !", aSelection.r.width,
                    aSelection.r.height, aSelection.r.left, aSelection.r.top);

    // set isp rkisp-isp-subdev src pad fmt
    memset(&isp_src_fmt, 0, sizeof(isp_src_fmt));
    isp_src_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    isp_src_fmt.pad   = 2;
    ret               = pCamHw->mIspCoreDev->getFormat(pCamHw->mIspCoreDev, &isp_src_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get mIspCoreDev src fmt failed !\n");
        return ret;
    }

    isp_src_fmt.format.width  = aSelection.r.width;
    isp_src_fmt.format.height = aSelection.r.height;
    ret                       = pCamHw->mIspCoreDev->setFormat(pCamHw->mIspCoreDev, &isp_src_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev src fmt failed !\n");
        return ret;
    }

    // for send bytes_perline to driver
    if (pCamHw->_airms_en)
        AiqRawStreamProcUnit_set_rx_format2(pCamHw->mRawProcUnit, &sns_sd_sel, sns_v4l_pix_fmt);

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp src fmt info: fmt 0x%x, %dx%d !", isp_src_fmt.format.code,
                    isp_src_fmt.format.width, isp_src_fmt.format.height);

    return ret;
}

static XCamReturn _setupPipelineFmtIsp(AiqCamHwBase_t* pCamHw,
                                       struct v4l2_subdev_selection* sns_sd_sel,
                                       struct v4l2_subdev_format* sns_sd_fmt,
                                       __u32 sns_v4l_pix_fmt) {
    struct v4l2_subdev_format isp_sink_fmt;
    struct v4l2_subdev_selection aSelection;
    struct v4l2_subdev_format isp_src_fmt;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pCamHw->_linked_to_1608 || g_rk1608_share_inf.first_en[pCamHw->mCamPhyId]) {
        AiqRawStreamCapUnit_set_tx_format(pCamHw->mRawCapUnit, sns_sd_fmt, sns_v4l_pix_fmt);
    }

    AiqRawStreamProcUnit_set_rx_format(pCamHw->mRawProcUnit, sns_sd_fmt, sns_v4l_pix_fmt);

    // set scale fmt
    if (pCamHw->mCifScaleStream) {
        int8_t bpp = 0;
        _pixFmt2Bpp(sns_v4l_pix_fmt, &bpp);
        AiqCifSclStream_set_format2(pCamHw->mCifScaleStream, sns_sd_sel, sns_v4l_pix_fmt, bpp);
    }

#ifndef ANDROID_OS  // Android camera hal will set pipeline itself
    // set isp sink fmt, same as sensor fmt

    memset(&isp_sink_fmt, 0, sizeof(isp_sink_fmt));
    isp_sink_fmt.pad   = 0;
    isp_sink_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    ret                = pCamHw->mIspCoreDev->getFormat(pCamHw->mIspCoreDev, &isp_sink_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev fmt failed !\n");
        return ret;
    }

    isp_sink_fmt.format.width  = sns_sd_fmt->format.width;
    isp_sink_fmt.format.height = sns_sd_fmt->format.height;
    isp_sink_fmt.format.code   = sns_sd_fmt->format.code;

    ret = pCamHw->mIspCoreDev->setFormat(pCamHw->mIspCoreDev, &isp_sink_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev fmt failed !\n");
        return ret;
    }

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp sink fmt info: fmt 0x%x, %dx%d !", isp_sink_fmt.format.code,
                    isp_sink_fmt.format.width, isp_sink_fmt.format.height);

    // set selection, isp do the crop
    memset(&aSelection, 0, sizeof(aSelection));

    aSelection.which    = V4L2_SUBDEV_FORMAT_ACTIVE;
    aSelection.pad      = 0;
    aSelection.flags    = 0;
    aSelection.target   = V4L2_SEL_TGT_CROP;
    aSelection.r.width  = sns_sd_sel->r.width;
    aSelection.r.height = sns_sd_sel->r.height;
    aSelection.r.left   = sns_sd_sel->r.left;
    aSelection.r.top    = sns_sd_sel->r.top;
    ret                 = pCamHw->mIspCoreDev->set_selection(pCamHw->mIspCoreDev, &aSelection);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev crop failed !\n");
        return ret;
    }

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp sink crop info: %dx%d@%d,%d !", aSelection.r.width,
                    aSelection.r.height, aSelection.r.left, aSelection.r.top);

    // set isp rkisp-isp-subdev src crop
    aSelection.pad      = 2;
    aSelection.target   = V4L2_SEL_TGT_CROP;
    aSelection.r.left   = 0;
    aSelection.r.top    = 0;
    aSelection.r.width  = sns_sd_sel->r.width;
    aSelection.r.height = sns_sd_sel->r.height;
#if 1  // isp src has no crop
    ret = pCamHw->mIspCoreDev->set_selection(pCamHw->mIspCoreDev, &aSelection);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev source crop failed !\n");
        return ret;
    }
#endif
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp src crop info: %dx%d@%d,%d !", aSelection.r.width,
                    aSelection.r.height, aSelection.r.left, aSelection.r.top);

    // set isp rkisp-isp-subdev src pad fmt

    isp_src_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    isp_src_fmt.pad   = 2;
    ret               = pCamHw->mIspCoreDev->getFormat(pCamHw->mIspCoreDev, &isp_src_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get mIspCoreDev src fmt failed !\n");
        return ret;
    }

    isp_src_fmt.format.width  = aSelection.r.width;
    isp_src_fmt.format.height = aSelection.r.height;
    ret                       = pCamHw->mIspCoreDev->setFormat(pCamHw->mIspCoreDev, &isp_src_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set mIspCoreDev src fmt failed !\n");
        return ret;
    }

    // for send bytes_perline to driver
    if (pCamHw->_airms_en)
        AiqRawStreamProcUnit_set_rx_format(pCamHw->mRawProcUnit, sns_sd_fmt, sns_v4l_pix_fmt);

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "isp src fmt info: fmt 0x%x, %dx%d !", isp_src_fmt.format.code,
                    isp_src_fmt.format.width, isp_src_fmt.format.height);
#endif
    return ret;
}

static XCamReturn _setupPipelineFmt(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    __u32 sns_v4l_pix_fmt;
    struct v4l2_subdev_format sns_sd_fmt;
    struct v4l2_subdev_selection sns_sd_sel;

    // get sensor v4l2 pixfmt
    AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
    rk_aiq_exposure_sensor_descriptor sns_des;
    if (mSensorSubdev->get_sensor_descriptor(mSensorSubdev, &sns_des)) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "getSensorModeData failed \n");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    sns_v4l_pix_fmt = sns_des.sensor_pixelformat;

    // get sensor real outupt size
    memset(&sns_sd_fmt, 0, sizeof(sns_sd_fmt));
    sns_sd_fmt.pad   = 0;
    sns_sd_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    ret              = mSensorSubdev->mSd->getFormat(mSensorSubdev->mSd, &sns_sd_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get sensor fmt failed !\n");
        return ret;
    }

    // get sensor crop bounds
    memset(&sns_sd_sel, 0, sizeof(sns_sd_sel));

    ret = mSensorSubdev->mSd->get_selection(mSensorSubdev->mSd, 0, V4L2_SEL_TGT_CROP_BOUNDS,
                                            &sns_sd_sel);
    if (ret) {
        LOGW_CAMHW_SUBM(ISP20HW_SUBM, "get_selection failed !\n");
        // TODO, some sensor driver has not implemented this
        // ioctl now
        sns_sd_sel.r.width  = sns_sd_fmt.format.width;
        sns_sd_sel.r.height = sns_sd_fmt.format.height;
        ret                 = XCAM_RETURN_NO_ERROR;
    }

    if (!pCamHw->_linked_to_isp && pCamHw->_crop_rect.width && pCamHw->_crop_rect.height) {
        struct v4l2_format mipi_tx_fmt;
        memset(&mipi_tx_fmt, 0, sizeof(mipi_tx_fmt));
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "vicap get_crop %dx%d@%d,%d\n", pCamHw->_crop_rect.width,
                        pCamHw->_crop_rect.height, pCamHw->_crop_rect.left, pCamHw->_crop_rect.top);

        if (!pCamHw->_linked_to_1608) {
            AiqV4l2Device_t* v4l2_dev = AiqRawStreamCapUnit_get_tx_device(pCamHw->mRawCapUnit, 0);
            v4l2_dev->get_format(v4l2_dev, &mipi_tx_fmt);
        } else {
            if (g_rk1608_share_inf.first_en[pCamHw->mCamPhyId]) {
                AiqV4l2Device_t* v4l2_dev =
                    AiqRawStreamCapUnit_get_tx_device(pCamHw->mRawCapUnit, 0);
                v4l2_dev->get_format(v4l2_dev, &mipi_tx_fmt);
            }
        }
        mipi_tx_fmt.fmt.pix.width  = pCamHw->_crop_rect.width;
        mipi_tx_fmt.fmt.pix.height = pCamHw->_crop_rect.height;

        if (!pCamHw->_linked_to_1608) {
            AiqV4l2Device_t* v4l2_dev = AiqRawStreamCapUnit_get_tx_device(pCamHw->mRawCapUnit, 0);
            AiqV4l2Device_setV4lFmt(v4l2_dev, &mipi_tx_fmt);
        } else {
            if (g_rk1608_share_inf.first_en[pCamHw->mCamPhyId]) {
                AiqV4l2Device_t* v4l2_dev =
                    AiqRawStreamCapUnit_get_tx_device(pCamHw->mRawCapUnit, 0);
                AiqV4l2Device_setV4lFmt(v4l2_dev, &mipi_tx_fmt);
            }
        }
        sns_sd_sel.r.width       = pCamHw->_crop_rect.width;
        sns_sd_sel.r.height      = pCamHw->_crop_rect.height;
        sns_sd_fmt.format.width  = pCamHw->_crop_rect.width;
        sns_sd_fmt.format.height = pCamHw->_crop_rect.height;
        ret                      = XCAM_RETURN_NO_ERROR;
    }

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "sensor fmt info: bounds %dx%d, crop %dx%d@%d,%d !",
                    sns_sd_sel.r.width, sns_sd_sel.r.height, sns_sd_fmt.format.width,
                    sns_sd_fmt.format.height, sns_sd_sel.r.left, sns_sd_sel.r.top);

    if (pCamHw->_linked_to_isp)
        ret = _setupPipelineFmtIsp(pCamHw, &sns_sd_sel, &sns_sd_fmt, sns_v4l_pix_fmt);
    else
        ret = _setupPipelineFmtCif(pCamHw, sns_sd_sel, &sns_sd_fmt, sns_v4l_pix_fmt);

    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set ispcore fmt failed !\n");
        return ret;
    }

    struct v4l2_subdev_format isp_src_fmt;
    // set isp rkisp-isp-subdev src pad fmt
    memset(&isp_src_fmt, 0, sizeof(isp_src_fmt));
    isp_src_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    isp_src_fmt.pad   = 2;
    ret               = pCamHw->mIspCoreDev->getFormat(pCamHw->mIspCoreDev, &isp_src_fmt);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get mIspCoreDev src fmt failed !\n");
        return ret;
    }

    // set sp format to NV12 as default
    if (pCamHw->mIspSpDev) {
        struct v4l2_selection selection;
        struct v4l2_format fmt;

        memset(&selection, 0, sizeof(selection));

        selection.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        selection.target   = V4L2_SEL_TGT_CROP;
        selection.flags    = 0;
        selection.r.left   = 0;
        selection.r.top    = 0;
        selection.r.width  = isp_src_fmt.format.width;
        selection.r.height = isp_src_fmt.format.height;

        ret = AiqV4l2Device_setSelection(pCamHw->mIspSpDev, &selection);

        ret = AiqV4l2Device_getV4lFmt(pCamHw->mIspSpDev, &fmt);
        if (ret) {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM, "get mIspSpDev fmt failed !\n");
            // return;
        }
        if (V4L2_PIX_FMT_FBCG == fmt.fmt.pix.pixelformat) {
            AiqV4l2Device_setFmt(pCamHw->mIspSpDev, /*isp_src_fmt.format.width*/ 1920,
                                 /*isp_src_fmt.format.height*/ 1080, V4L2_PIX_FMT_NV12,
                                 V4L2_FIELD_NONE, 0);
        }
    }
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "ispp sd fmt info: %dx%d", isp_src_fmt.format.width,
                    isp_src_fmt.format.height);
    return ret;
}

static XCamReturn _setupHdrLink(AiqCamHwBase_t* pCamHw, int hdr_mode, int isp_index, bool enable) {
    struct media_device* device = NULL;
    struct media_entity* entity = NULL;
    struct media_pad *src_pad_s = NULL, *src_pad_m = NULL, *src_pad_l = NULL, *sink_pad = NULL;

    device = media_device_new(g_mIspHwInfos.isp_info[isp_index].media_dev_path);
    if (!device) return XCAM_RETURN_ERROR_FAILED;

    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);
    entity = media_get_entity_by_name(device, "rkisp-isp-subdev", strlen("rkisp-isp-subdev"));
    if (entity) {
        sink_pad = (struct media_pad*)media_entity_get_pad(entity, 0);
        if (!sink_pad) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get HDR sink pad failed!\n");
            goto FAIL;
        }
    }

    entity = media_get_entity_by_name(device, "rkisp_rawrd2_s", strlen("rkisp_rawrd2_s"));
    if (entity) {
        src_pad_s = (struct media_pad*)media_entity_get_pad(entity, 0);
        if (!src_pad_s) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get HDR source pad s failed!\n");
            goto FAIL;
        }
    }
    if (src_pad_s && sink_pad) {
        if (enable) {
            media_setup_link(device, src_pad_s, sink_pad, MEDIA_LNK_FL_ENABLED);
        } else
            media_setup_link(device, src_pad_s, sink_pad, 0);
    }

    entity = media_get_entity_by_name(device, "rkisp_rawrd0_m", strlen("rkisp_rawrd0_m"));
    if (entity) {
        src_pad_m = (struct media_pad*)media_entity_get_pad(entity, 0);
        if (!src_pad_m) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get HDR source pad m failed!\n");
            goto FAIL;
        }
    }

    if (src_pad_m && sink_pad) {
        if (RK_AIQ_HDR_GET_WORKING_MODE(hdr_mode) >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                !RK_AIQ_HDR_IS_SENSOR_BUILTIN(hdr_mode) && enable) {
            media_setup_link(device, src_pad_m, sink_pad, MEDIA_LNK_FL_ENABLED);
        } else
            media_setup_link(device, src_pad_m, sink_pad, 0);
    }

    entity = media_get_entity_by_name(device, "rkisp_rawrd1_l", strlen("rkisp_rawrd1_l"));
    if (entity) {
        src_pad_l = (struct media_pad*)media_entity_get_pad(entity, 0);
        if (!src_pad_l) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get HDR source pad l failed!\n");
            goto FAIL;
        }
    }

    if (src_pad_l && sink_pad) {
        if (RK_AIQ_HDR_GET_WORKING_MODE(hdr_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3 &&
                !RK_AIQ_HDR_IS_SENSOR_BUILTIN(hdr_mode) && enable) {
            media_setup_link(device, src_pad_l, sink_pad, MEDIA_LNK_FL_ENABLED);
        } else
            media_setup_link(device, src_pad_l, sink_pad, 0);
    }
    media_device_unref(device);
    return XCAM_RETURN_NO_ERROR;
FAIL:
    media_device_unref(device);
    return XCAM_RETURN_ERROR_FAILED;
}

static XCamReturn _setExpDelayInfo(AiqCamHwBase_t* pCamHw, int mode) {
    ENTER_CAMHW_FUNCTION();
    AiqSensorHw_t* sensorHw = pCamHw->_mSensorDev;

    if (mode != RK_AIQ_WORKING_MODE_NORMAL) {
        sint32_t timeDelay = pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Hdr.time_update;
        sint32_t gainDelay = pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Hdr.gain_update;

        sensorHw->set_exp_delay_info(
            sensorHw, pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Hdr.time_update,
            pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Hdr.gain_update,
            pCamHw->_cur_calib_infos.sensor.CISDcgSet.Hdr.support_en
            ? pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Hdr.dcg_update
            : -1);

        pCamHw->_exp_delay = timeDelay > gainDelay ? timeDelay : gainDelay;
    } else {
        sint32_t timeDelay = pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Linear.time_update;
        sint32_t gainDelay = pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Linear.gain_update;

        sensorHw->set_exp_delay_info(
            sensorHw, pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Linear.time_update,
            pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Linear.gain_update,
            pCamHw->_cur_calib_infos.sensor.CISDcgSet.Linear.support_en
            ? pCamHw->_cur_calib_infos.sensor.CISExpUpdate.Linear.dcg_update
            : -1);
        pCamHw->_exp_delay = timeDelay > gainDelay ? timeDelay : gainDelay;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _setLensVcmModInfo(AiqCamHwBase_t* pCamHw, struct rkmodule_inf* mod_info) {
    ENTER_CAMHW_FUNCTION();
    AiqLensHw_t* lensHw = pCamHw->mLensDev;
    rk_aiq_lens_vcmcfg old_cfg, new_cfg;
    int old_maxpos, new_maxpos;
    struct rk_cam_otp_info otp_info;
    struct rkmodule_af_otp *af_otp[RKMODULE_AF_OTP_MAX_LEN] = { NULL };
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (lensHw) {
        CalibDbV2_Af_VcmCfg_t* vcmcfg = &pCamHw->_cur_calib_infos.af.vcmcfg;
        float posture_diff            = vcmcfg->posture_diff;
        float extend_range_ratio      = vcmcfg->extend_range_ratio;

        ret = AiqLensHw_getLensVcmCfg(lensHw, &old_cfg);
        if (ret != XCAM_RETURN_NO_ERROR) return ret;
        ret = AiqLensHw_getLensVcmMaxlogpos(lensHw, &old_maxpos);
        if (ret != XCAM_RETURN_NO_ERROR) return ret;

        new_cfg = old_cfg;
        if (vcmcfg->start_current != -1) {
            new_cfg.start_ma = vcmcfg->start_current;
        }
        if (vcmcfg->rated_current != -1) {
            new_cfg.rated_ma = vcmcfg->rated_current;
        }
        if (vcmcfg->step_mode != -1) {
            new_cfg.step_mode = vcmcfg->step_mode;
        }

        if (vcmcfg->start_current == -1 && vcmcfg->rated_current == -1 && vcmcfg->step_mode == -1) {
            if (mod_info->af.flag) {
                int range    = new_cfg.rated_ma - new_cfg.start_ma;
                int start_ma = new_cfg.start_ma;
                int rated_ma = new_cfg.rated_ma;

                for (uint32_t i = 0; i < mod_info->af.dir_cnt; i++) {
                    if (mod_info->af.af_otp[i].vcm_dir == AF_OTP_DIR_HORIZONTAL)
                        af_otp[0] = &mod_info->af.af_otp[i];
                    else if (mod_info->af.af_otp[i].vcm_dir == AF_OTP_DIR_UP)
                        af_otp[1] = &mod_info->af.af_otp[i];
                    else if (mod_info->af.af_otp[i].vcm_dir == AF_OTP_DIR_DOWN)
                        af_otp[2] = &mod_info->af.af_otp[i];
                }

                start_ma = af_otp[0]->vcm_start;
                rated_ma = af_otp[0]->vcm_end;
                range    = rated_ma - start_ma;
                if (extend_range_ratio != 0) {
                    start_ma = start_ma - (int)(range * extend_range_ratio);
                    rated_ma = rated_ma + (int)(range * extend_range_ratio);

                    if (start_ma < 0)
                        start_ma = 0;
                    if (rated_ma > 1023)
                        rated_ma = 1023;

                    LOGD_AF("extend_range_ratio %f, start_ma %d -> %d, rated_ma %d -> %d", extend_range_ratio,
                            af_otp[0]->vcm_start, start_ma, af_otp[0]->vcm_end, rated_ma);
                }

                new_cfg.start_ma = start_ma;
                new_cfg.rated_ma = rated_ma;
                if (vcmcfg->use_gsensor) {
                    otp_info.otp_valid    = false;
                    otp_info.hysteresis   = -1;
                    otp_info.startCurrent = start_ma;
                    otp_info.endCurrent   = rated_ma;
                    if (af_otp[1] && af_otp[0]) {
                        otp_info.posture = af_otp[1]->vcm_start - af_otp[0]->vcm_start;
                        otp_info.otp_valid = true;
                    } else if (af_otp[2] && af_otp[0]) {
                        otp_info.posture = af_otp[0]->vcm_start - af_otp[2]->vcm_start;
                        otp_info.otp_valid = true;
                    } else if (posture_diff != 0) {
                        otp_info.posture = (int)(range * posture_diff);
                        otp_info.otp_valid = true;
                    }
                    if (otp_info.otp_valid)
                        AiqLensHw_setOTPData(lensHw, &otp_info);
                }
            }
        }

        if ((new_cfg.start_ma != old_cfg.start_ma) || (new_cfg.rated_ma != old_cfg.rated_ma) ||
                (new_cfg.step_mode != old_cfg.step_mode)) {
            ret = AiqLensHw_setLensVcmCfg(lensHw, &new_cfg);
        }

        new_maxpos = old_maxpos;
        if (vcmcfg->max_logical_pos > 0) {
            new_maxpos = vcmcfg->max_logical_pos;
        }
        if (old_maxpos != new_maxpos) {
            ret = AiqLensHw_setLensVcmMaxlogpos(lensHw, &new_maxpos);
        }
    }
    EXIT_CAMHW_FUNCTION();
    return ret;
}

#if RKAIQ_HAVE_PDAF
static XCamReturn _get_sensor_pdafinfo(AiqCamHwBase_t* pCamHw, rk_sensor_full_info_t* sensor_info,
                                       rk_sensor_pdaf_info_t* pdaf_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rkmodule_channel_info channel;
    memset(&channel, 0, sizeof(struct rkmodule_channel_info));
    AiqV4l2SubDevice_t subDev;

    AiqV4l2SubDevice_init(&subDev, sensor_info->device_name);
    ret = AiqV4l2SubDevice_open(&subDev, false);
    if (ret != XCAM_RETURN_NO_ERROR) {
        AiqV4l2SubDevice_deinit(&subDev);
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to open dev (%s)", sensor_info->device_name);
        return XCAM_RETURN_ERROR_FAILED;
    }

    AiqV4l2Device_t* vdev   = &subDev._v4l_base;
    pdaf_info->pdaf_support = false;
    for (int i = 0; i < 4; i++) {
        channel.index = i;
        if (subDev._v4l_base.io_control(vdev, RKMODULE_GET_CHANNEL_INFO, &channel) == 0) {
            if (channel.bus_fmt == MEDIA_BUS_FMT_SPD_2X8) {
                pdaf_info->pdaf_support = true;
                pdaf_info->pdaf_vc      = i;
                pdaf_info->pdaf_code    = channel.bus_fmt;
                pdaf_info->pdaf_width   = channel.width;
                pdaf_info->pdaf_height  = channel.height;
                if (channel.data_bit == 10)
                    pdaf_info->pdaf_pixelformat = V4L2_PIX_FMT_SRGGB10;
                else if (channel.data_bit == 12)
                    pdaf_info->pdaf_pixelformat = V4L2_PIX_FMT_SRGGB12;
                else if (channel.data_bit == 8)
                    pdaf_info->pdaf_pixelformat = V4L2_PIX_FMT_SRGGB8;
                else
                    pdaf_info->pdaf_pixelformat = V4L2_PIX_FMT_SRGGB16;
                LOGI_CAMHW_SUBM(ISP20HW_SUBM, "channel.bus_fmt 0x%x, pdaf_width %d, pdaf_height %d",
                                channel.bus_fmt, pdaf_info->pdaf_width, pdaf_info->pdaf_height);
                break;
            }
        }
    }

    if (pdaf_info->pdaf_support) {
        if (sensor_info->linked_to_isp) {
            switch (pdaf_info->pdaf_vc) {
            case 0:
                strcpy(pdaf_info->pdaf_vdev, sensor_info->isp_info->rawwr0_path);
                break;
            case 1:
                strcpy(pdaf_info->pdaf_vdev, sensor_info->isp_info->rawwr1_path);
                break;
            case 2:
                strcpy(pdaf_info->pdaf_vdev, sensor_info->isp_info->rawwr2_path);
                break;
            case 3:
            default:
                strcpy(pdaf_info->pdaf_vdev, sensor_info->isp_info->rawwr3_path);
                break;
            }
        } else {
            switch (pdaf_info->pdaf_vc) {
            case 0:
                strcpy(pdaf_info->pdaf_vdev, sensor_info->cif_info->mipi_id0);
                break;
            case 1:
                strcpy(pdaf_info->pdaf_vdev, sensor_info->cif_info->mipi_id1);
                break;
            case 2:
                strcpy(pdaf_info->pdaf_vdev, sensor_info->cif_info->mipi_id2);
                break;
            case 3:
            default:
                strcpy(pdaf_info->pdaf_vdev, sensor_info->cif_info->mipi_id3);
                break;
            }
        }
    }
    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "%s: pdaf_vdev %s", __func__, pdaf_info->pdaf_vdev);

    AiqV4l2Device_close(vdev);
    AiqV4l2SubDevice_deinit(&subDev);
    return ret;
}
#endif
void AiqCamHw_setCalib(AiqCamHwBase_t* pCamHw, const CamCalibDbV2Context_t* calibv2) {
    pCamHw->mCalibDbV2                    = calibv2;
    CalibDb_Aec_ParaV2_t* aec             = NULL;
    CalibDb_Sensor_ParaV2_t* sensor_calib = NULL;

    CalibDbV2_MFNR_t* mfnr =
        (CalibDbV2_MFNR_t*)CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), mfnr_v1);
    if (mfnr) {
        pCamHw->_cur_calib_infos.mfnr.enable           = mfnr->TuningPara.enable;
        pCamHw->_cur_calib_infos.mfnr.motion_detect_en = mfnr->TuningPara.motion_detect_en;
    } else {
        pCamHw->_cur_calib_infos.mfnr.enable           = false;
        pCamHw->_cur_calib_infos.mfnr.motion_detect_en = false;
    }

    aec = (CalibDb_Aec_ParaV2_t*)CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), ae_calib);
    if (aec) {
        pCamHw->_cur_calib_infos.aec.IrisType = aec->IrisCtrl.IrisType;
    } else {
        pCamHw->_cur_calib_infos.aec.IrisType = IRISV2_DC_TYPE;
    }

#if RKAIQ_HAVE_AF_V31
    CalibDbV2_AFV31_t* af_v31 =
        (CalibDbV2_AFV31_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af_v31));
    if (af_v31) {
        pCamHw->_cur_calib_infos.af.vcmcfg = af_v31->TuningPara.vcmcfg;
    } else {
        memset(&pCamHw->_cur_calib_infos.af.vcmcfg, 0, sizeof(CalibDbV2_Af_VcmCfg_t));
    }
    memset(&pCamHw->_cur_calib_infos.af.ldg_param, 0, sizeof(CalibDbV2_Af_LdgParam_t));
#endif
#if RKAIQ_HAVE_AF_V30
    CalibDbV2_AFV30_t* af_v30 =
        (CalibDbV2_AFV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af_v30));
    if (af_v30) {
        pCamHw->_cur_calib_infos.af.vcmcfg = af_v30->TuningPara.vcmcfg;
    } else {
        memset(&pCamHw->_cur_calib_infos.af.vcmcfg, 0, sizeof(CalibDbV2_Af_VcmCfg_t));
    }
    memset(&pCamHw->_cur_calib_infos.af.ldg_param, 0, sizeof(CalibDbV2_Af_LdgParam_t));
#endif
#if RKAIQ_HAVE_AF_V32_LITE
    CalibDbV2_AFV32_t* af_v32 =
        (CalibDbV2_AFV32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af_v32));
    if (af_v32) {
        pCamHw->_cur_calib_infos.af.vcmcfg = af_v32->TuningPara.vcmcfg;
    } else {
        memset(&pCamHw->_cur_calib_infos.af.vcmcfg, 0, sizeof(CalibDbV2_Af_VcmCfg_t));
    }
    memset(&pCamHw->_cur_calib_infos.af.ldg_param, 0, sizeof(CalibDbV2_Af_LdgParam_t));
#endif
#if RKAIQ_HAVE_AF_V33
#if ISP_HW_V39
    CalibDbV2_AFV33_t* af_v33 =
        (CalibDbV2_AFV33_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af_v33));
    if (af_v33) {
        CalibDbV2_Af_VcmCfg_t *vcmcfg = &pCamHw->_cur_calib_infos.af.vcmcfg;

        vcmcfg->max_logical_pos = af_v33->VcmCfg.MaxLogicalPos;
        vcmcfg->start_current = af_v33->VcmCfg.StartCurrent;
        vcmcfg->rated_current = af_v33->VcmCfg.RatedCurrent;
        vcmcfg->step_mode = af_v33->VcmCfg.StepMode;
        vcmcfg->extra_delay = af_v33->VcmCfg.ExtraDelay;
        vcmcfg->extend_range_ratio = af_v33->VcmCfg.ExtendRangeRatio;
        vcmcfg->posture_diff = af_v33->VcmCfg.PostureDiff;
        vcmcfg->use_gsensor = af_v33->VcmCfg.UseGsensor;
    } else {
        memset(&pCamHw->_cur_calib_infos.af.vcmcfg, 0, sizeof(CalibDbV2_Af_VcmCfg_t));
    }
    memset(&pCamHw->_cur_calib_infos.af.ldg_param, 0, sizeof(CalibDbV2_Af_LdgParam_t));
#else
    af_param_t* af_calib =
        (af_param_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af_calib));
    if (af_calib) {
        CalibDbV2_Af_VcmCfg_t *vcmcfg = &pCamHw->_cur_calib_infos.af.vcmcfg;

        vcmcfg->max_logical_pos = af_calib->vcmCfg.sw_afT_maxLogicalPos_val;
        vcmcfg->start_current = af_calib->vcmCfg.sw_afT_startCurrent_val;
        vcmcfg->rated_current = af_calib->vcmCfg.sw_afT_ratedCurrent_val;
        vcmcfg->step_mode = af_calib->vcmCfg.sw_afT_stepMode_val;
        vcmcfg->extra_delay = af_calib->vcmCfg.sw_afT_extraDelay_val;
        vcmcfg->extend_range_ratio = af_calib->vcmCfg.sw_afT_extendRange_ratio;
        vcmcfg->posture_diff = af_calib->vcmCfg.sw_afT_postureDiff_ratio;
        vcmcfg->use_gsensor = af_calib->vcmCfg.sw_afT_useGsensor_en;
    } else {
        memset(&pCamHw->_cur_calib_infos.af.vcmcfg, 0, sizeof(CalibDbV2_Af_VcmCfg_t));
    }
    memset(&pCamHw->_cur_calib_infos.af.ldg_param, 0, sizeof(CalibDbV2_Af_LdgParam_t));
#endif
#endif
#if RKAIQ_HAVE_AF_V20
    CalibDbV2_AF_t* af =
        (CalibDbV2_AF_t*)CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af);
    if (af) {
        pCamHw->_cur_calib_infos.af.vcmcfg    = af->TuningPara.vcmcfg;
        pCamHw->_cur_calib_infos.af.ldg_param = af->TuningPara.ldg_param;
        pCamHw->_cur_calib_infos.af.highlight = af->TuningPara.highlight;
    } else {
        memset(&pCamHw->_cur_calib_infos.af.vcmcfg, 0, sizeof(CalibDbV2_Af_VcmCfg_t));
        memset(&pCamHw->_cur_calib_infos.af.ldg_param, 0, sizeof(CalibDbV2_Af_LdgParam_t));
    }
#endif

    sensor_calib = (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2),
                   sensor_calib));
    if (sensor_calib) {
        pCamHw->_cur_calib_infos.sensor.CISDcgSet    = sensor_calib->CISDcgSet;
        pCamHw->_cur_calib_infos.sensor.CISExpUpdate = sensor_calib->CISExpUpdate;
    } else {
        memset(&pCamHw->_cur_calib_infos.sensor, 0, sizeof(pCamHw->_cur_calib_infos.sensor));
    }

#if RKAIQ_HAVE_FPNSW
    // fpn
    if (pCamHw->mFpnStreamUnit) {
        AiqFpnSwStreamProcUnit_set_csi_gain_max(pCamHw->mFpnStreamUnit, sensor_calib->CISGainSet.CISAgainRange.Max, sensor_calib->CISGainSet.CISDgainRange.Max);
        AiqFpnSwStreamProcUnit_setCalib(pCamHw->mFpnStreamUnit, pCamHw->mCalibDbV2);
    }
#endif

#if defined(RKAIQ_HAVE_MEMC)
    if (pCamHw->mMemcStream)
        AiqMemcManager_setCalib(pCamHw->mMemcStream, pCamHw->mCalibDbV2);
#endif

#ifdef ISP_HW_V35
    if (pCamHw->mBtnrDsReproc) {
        AiqBtnrDsReproc_setCalib(pCamHw->mBtnrDsReproc, pCamHw->mCalibDbV2);
    }
#endif

    // update infos to sensor hw
    _setExpDelayInfo(pCamHw, pCamHw->_hdr_mode);

    AiqIspParamsCvt_setCalib(pCamHw->_mIspParamsCvt, pCamHw->mCalibDbV2);

    AiqCamHw_set_sceneCisParams(pCamHw);
}

static XCamReturn _setupHdrLink_vidcap(AiqCamHwBase_t* pCamHw, int hdr_mode, int cif_index,
                                       bool enable) {
    // TODO: have some bugs now
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _init_bay3dbuf(AiqCamHwBase_t* pCamHw, void* isp_params) {
#if defined(ISP_HW_V39) || defined(ISP_HW_V35)
    if (pCamHw->_bnrDrvBuf._init)
        return XCAM_RETURN_NO_ERROR;
    int res = -1;
    int i = 0;
#if defined(ISP_HW_V39)
    if (!pCamHw->use_aiisp) {
        pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt = 1;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.aiisp.buf_cnt = 0;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.gain.buf_cnt = 0;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.iirsparse_en = 0;
    } else {
        pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt = 3;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.aiisp.buf_cnt = 3;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.gain.buf_cnt = 4;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.iirsparse_en = 1;
    }
#elif defined(ISP_HW_V35)
    pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt = 1;
    pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.ds.buf_cnt = 1;
    pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_cnt = 1;
    //pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.iir_rw_fmt = 1;

#if defined(RKAIQ_HAVE_AIBNR) || defined(RKAIQ_HAVE_AIYNR)
    bool aibnr_en = false;
    bool aiynr_en = false;

#ifdef RKAIQ_HAVE_AIBNR
    aibnr_api_attrib_t* aibnr_attrib =
        (aibnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), aibnr));
    btnr_api_attrib_t *btnr2_attr = (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), bayertnr));
    aibnr_en = aibnr_attrib->en;
#endif

#ifdef RKAIQ_HAVE_AIYNR
    aiynr_api_attrib_t* aiynr_attrib =
        (aiynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), aiynr));
    aiynr_en = aiynr_attrib->en;
#endif

    if (aibnr_en && aiynr_en) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't enable both aibnr and aiynr, only enable aibnr");
    }

    if (aibnr_en) {
        pCamHw->use_aiisp = true;
        //pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.iir_rw_fmt = 2;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.gain_mode  = 1;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.yraw_sel   = 1;
        if (btnr2_attr && btnr2_attr->stAuto.sta.dLoMd_mode == btnr_dLoMdCurOrPre_mode)
            pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aibnr_l2 = 1;
        else
            pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aibnr_l2 = 0;

        uint16_t feBuf_cnt = pCamHw->mAibnrBufCnt.isp_fe;
        uint16_t beBuf_cnt = pCamHw->mAibnrBufCnt.isp_be;
        if (!feBuf_cnt) {
            feBuf_cnt                   = AIBNR_IIR_BUF_CNT;
            pCamHw->mAibnrBufCnt.isp_fe = feBuf_cnt;
        }

        if (!beBuf_cnt) {
            beBuf_cnt                   = AIBNR_AIISP_BUF_CNT;
            pCamHw->mAibnrBufCnt.isp_be = beBuf_cnt;
        }

        pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt              = feBuf_cnt;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.gain.buf_cnt       = AIBNR_GAIN_BUF_CNT;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aipre_gain.buf_cnt = feBuf_cnt;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.vpsl.buf_cnt       = feBuf_cnt;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aiisp.buf_cnt      = beBuf_cnt;

        LOGK_AIBNR("request feBuf_cnt %d, beBuf_cnt %d", pCamHw->mAibnrBufCnt.isp_fe,
                   pCamHw->mAibnrBufCnt.isp_be);
        //pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.y_src.buf_cnt      = AIBNR_AIISP_BUF_CNT;
    } else if (aiynr_en) {
        pCamHw->use_aiisp = true;
        //pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.iir_rw_fmt = 2;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.gain_mode  = 0;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.yraw_sel   = 0;

        //pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt              = AIYNR_IIR_BUF_CNT;
        //pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.gain.buf_cnt       = AIYNR_GAIN_BUF_CNT;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aipre_gain.buf_cnt = AIYNR_AIPRE_BUF_CNT;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.vpsl.buf_cnt       = AIYNR_VPSL_BUF_CNT;
        //pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aiisp.buf_cnt      = AIYNR_AIISP_BUF_CNT;
        pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.y_src.buf_cnt      = AIYNR_YSRC_BUF_CNT;
    } else {
        pCamHw->use_aiisp = false;
    }
#endif

    pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.iir_rw_fmt =
        ((struct isp35_isp_params_cfg*)isp_params)->others.bay3d_cfg.iir_rw_fmt;
#endif
#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcStream && pCamHw->mMemcEn)
        pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt = 4;
#endif
    res =
        AiqV4l2SubDevice_ioctl(pCamHw->mIspCoreDev, RKISP_CMD_INIT_BNR_BUF, &pCamHw->_bnrDrvBuf._bay3dbuf);
    if (res) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get aiisp bay3dbuf failed! %d", res);
        return XCAM_RETURN_ERROR_IOCTL;
    }
#if defined(ISP_HW_V39) || RKAIQ_HAVE_MEMC
    for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt; i++) {
        int iir_fd   = pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_fd[i];
        int iir_size = pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_size;
        pCamHw->_bnrDrvBuf.iir_address[i] =
            (char*)mmap(NULL, iir_size, PROT_READ | PROT_WRITE, MAP_SHARED, iir_fd, 0);
        if (MAP_FAILED == pCamHw->_bnrDrvBuf.iir_address[i]) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "iir_fd mmap failed");
            return XCAM_RETURN_ERROR_FAILED;
        }
        LOGK_CAMHW_SUBM(ISP20HW_SUBM, "buf[%d]: iir_fd %d, iir_size %d", i, iir_fd, iir_size);
    }
#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcStream && pCamHw->mMemcEn) {
        AiqMemcManager_setIirAddress(pCamHw->mMemcStream, pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_fd, pCamHw->_bnrDrvBuf.iir_address, pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_size);
    }
#endif
#endif
#if defined(ISP_HW_V35) && RKAIQ_HAVE_AIBNR
    if (aibnr_en) {
        for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_cnt; i++) {
            int wgt_fd   = pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_fd[i];
            int wgt_size = pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_size;
            pCamHw->_bnrDrvBuf.wgt_address[i] =
                (char*)mmap(NULL, wgt_size, PROT_READ | PROT_WRITE, MAP_SHARED, wgt_fd, 0);
            if (MAP_FAILED == pCamHw->_bnrDrvBuf.wgt_address[i]) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "iir_fd mmap failed");
                return XCAM_RETURN_ERROR_FAILED;
            }
            LOGK_CAMHW_SUBM(ISP20HW_SUBM, "buf[%d]: wgt_fd %d, wgt_size %d", i, wgt_fd, wgt_size);
        }

        if (pCamHw->mBtnrDsReproc) {
            AiqBtnrDsReproc_setBtnrWgtBuf(pCamHw->mBtnrDsReproc, pCamHw->_bnrDrvBuf.wgt_address, pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_size, pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_fd);
        }
    }
#endif
#if defined(ISP_HW_V39)
    for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.gain.buf_cnt; i++) {
        int gain_fd   = pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.gain.buf_fd[i];
        int gain_size = pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.gain.buf_size;
        pCamHw->_bnrDrvBuf.gain_address[i] =
            (char*)mmap(NULL, gain_size, PROT_READ | PROT_WRITE, MAP_SHARED, gain_fd, 0);
        if (MAP_FAILED == pCamHw->_bnrDrvBuf.gain_address[i]) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "gain_fd mmap failed");
            return XCAM_RETURN_ERROR_FAILED;
        }
        LOGK_CAMHW_SUBM(ISP20HW_SUBM, "buf[%d]: gain_fd %d, gain_size %d", i, gain_fd, gain_size);
    }
    for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.aiisp.buf_cnt; i++) {
        int aiisp_fd   = pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.aiisp.buf_fd[i];
        int aiisp_size = pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.aiisp.buf_size;
        pCamHw->_bnrDrvBuf.aiisp_address[i] =
            (char*)mmap(NULL, aiisp_size, PROT_READ | PROT_WRITE, MAP_SHARED, aiisp_fd, 0);
        if (MAP_FAILED == pCamHw->_bnrDrvBuf.aiisp_address[i]) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "aiisp_fd mmap failed");
            return XCAM_RETURN_ERROR_FAILED;
        }
        LOGK_CAMHW_SUBM(ISP20HW_SUBM, "buf[%d]: aiisp_fd %d, aiisp_size %d", i, aiisp_fd, aiisp_size);
    }
#endif
    pCamHw->_bnrDrvBuf._init = true;
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "get bay3dbuf: iir buf_cnt is %d iir_size is %d",
                    pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt, pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_size);
#endif
    return XCAM_RETURN_NO_ERROR;
}

static void _deinit_bay3dbuf(AiqCamHwBase_t* pCamHw) {
#if defined(ISP_HW_V39) || defined(ISP_HW_V35)
    int i = 0;
    if (!pCamHw->_bnrDrvBuf._init)
        return ;
    for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_fd[i]);
        if (pCamHw->_bnrDrvBuf.iir_address[i])
            munmap(pCamHw->_bnrDrvBuf.iir_address[i], pCamHw->_bnrDrvBuf._bay3dbuf.iir.buf_size);
    }
#if defined(ISP_HW_V39)
    for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.aiisp.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.aiisp.buf_fd[i]);
        munmap(pCamHw->_bnrDrvBuf.aiisp_address[i], pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.aiisp.buf_size);
    }
    for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.gain.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.gain.buf_fd[i]);
        munmap(pCamHw->_bnrDrvBuf.gain_address[i], pCamHw->_bnrDrvBuf._bay3dbuf.u.v39.gain.buf_size);
    }
#elif defined(ISP_HW_V35)
    for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.ds.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.ds.buf_fd[i]);
    }
    for (i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_fd[i]);
        if (pCamHw->_bnrDrvBuf.wgt_address[i])
            munmap(pCamHw->_bnrDrvBuf.wgt_address[i], pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.wgt.buf_size);
    }
#if defined(RKAIQ_HAVE_AIBNR) || defined(RKAIQ_HAVE_AIYNR)
    for(i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.gain.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.gain.buf_fd[i]);
    }
    for(i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aipre_gain.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aipre_gain.buf_fd[i]);
    }
    for(i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.vpsl.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.vpsl.buf_fd[i]);
    }
    for(i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aiisp.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.aiisp.buf_fd[i]);
    }
    for(i = 0; i < pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.y_src.buf_cnt; i++) {
        close(pCamHw->_bnrDrvBuf._bay3dbuf.u.v35.y_src.buf_fd[i]);
    }
#endif
#endif
    pCamHw->_bnrDrvBuf._init = false;
#endif
}

#define ASSIGN_POINT(p, _x, _y, _w, _h) \
    do {                                \
        (p).x = (_x);                   \
        (p).y = (_y);                   \
        (p).w = (_w);                   \
        (p).h = (_h);                   \
    } while (0)

XCamReturn AiqCamHw_prepare(AiqCamHwBase_t* pCamHw, uint32_t width, uint32_t height, int mode,
                            int t_delay, int g_delay) {
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    AiqSensorHw_t* sensorHw       = NULL;
    AiqLensHw_t* lensHw           = pCamHw->mLensDev;
    SnsFullInfoWraps_t* pItem     = g_mSensorHwInfos;
    rk_sensor_full_info_t* s_info = NULL;
    int isp_index                 = -1;
    struct v4l2_subdev_format isp_src_fmt;
    Splitter_Rectangle_t split_rectangle;

    ENTER_CAMHW_FUNCTION();

    XCAM_ASSERT(pCamHw->mCalibDbV2);

    pCamHw->_hdr_mode = mode;
    AiqIspParamsCvt_set_working_mode(pCamHw->_mIspParamsCvt, pCamHw->_hdr_mode);

    while (pItem) {
        if (strcmp(pCamHw->sns_name, pItem->key) == 0) break;
        pItem = pItem->next;
    }

    if (!pItem) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", pCamHw->sns_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    s_info    = &pItem->data;
    isp_index = s_info->isp_info->logic_id;
    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "sensor_name(%s) is linked to isp_index(%d)", pCamHw->sns_name,
                    isp_index);

    if ((pCamHw->_hdr_mode > 0 && pCamHw->mIsOnlineByWorkingMode) ||
            (!pCamHw->_linked_to_isp && !pCamHw->mVicapIspPhyLinkSupported) ||
            (s_info->linked_to_serdes || s_info->split)) {
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "use read back mode!");
        pCamHw->mNoReadBack = false;
    }

#if RKAIQ_HAVE_MEMC || RKAIQ_HAVE_FPNSW
    if (pCamHw->mRawCapUnit && pCamHw->mRawProcUnit) {
#if RKAIQ_HAVE_MEMC && RKAIQ_HAVE_FPNSW
        if (pCamHw->mMemcStream && pCamHw->mFpnStreamUnit) {
            // rawcap -> fpn -> memc -> rawproc
            AiqRawStreamCapUnit_setSwStreamInfo(pCamHw->mRawCapUnit, pCamHw->mFpnStreamUnit, AiqFpnSwStreamProcUnit_send_sync_buf);
            AiqFpnSwStreamProcUnit_setSwStreamInfo(pCamHw->mFpnStreamUnit, pCamHw->mMemcStream, AiqMemcManager_send_sync_buf);
            AiqMemcManager_setProcStreamCtx(pCamHw->mMemcStream, pCamHw->mRawProcUnit);
        } else if (pCamHw->mMemcStream) {
            // rawcap -> memc -> rawproc
            AiqRawStreamCapUnit_setSwStreamInfo(pCamHw->mRawCapUnit, pCamHw->mMemcStream, AiqMemcManager_send_sync_buf);
            AiqMemcManager_setProcStreamCtx(pCamHw->mMemcStream, pCamHw->mRawProcUnit);
        } else if (pCamHw->mFpnStreamUnit) {
            // rawcap -> fpn -> rawproc
            AiqRawStreamCapUnit_setSwStreamInfo(pCamHw->mRawCapUnit, pCamHw->mFpnStreamUnit, AiqFpnSwStreamProcUnit_send_sync_buf);
            AiqFpnSwStreamProcUnit_set_proc_stream(pCamHw->mFpnStreamUnit, pCamHw->mRawProcUnit);
        }
#elif RKAIQ_HAVE_MEMC
        if (pCamHw->mMemcStream) {
            // rawcap -> memc -> rawproc
            AiqRawStreamCapUnit_setSwStreamInfo(pCamHw->mRawCapUnit, pCamHw->mMemcStream, AiqMemcManager_send_sync_buf);
            AiqMemcManager_setProcStreamCtx(pCamHw->mMemcStream, pCamHw->mRawProcUnit);
        }
#elif RKAIQ_HAVE_FPNSW
        if (pCamHw->mFpnStreamUnit) {
            // rawcap -> fpn -> rawproc
            AiqRawStreamCapUnit_setSwStreamInfo(pCamHw->mRawCapUnit, pCamHw->mFpnStreamUnit, AiqFpnSwStreamProcUnit_send_sync_buf);
            AiqFpnSwStreamProcUnit_set_proc_stream(pCamHw->mFpnStreamUnit, pCamHw->mRawProcUnit);
        }
#endif

#if RKAIQ_HAVE_MEMC
        if (pCamHw->mMemcStream && pCamHw->mMemcEn && AiqMemcManager_getMemcIsSyncBaseImuMode(pCamHw->mMemcStream)) {
            AiqCamHw_setRawBufNum(pCamHw, 5);
            pCamHw->mNoReadBack = false;
            LOGK_CAMHW_SUBM(ISP20HW_SUBM, "btnr memc force use read back mode, force raw buf num to 5!");
        }
#endif

#if RKAIQ_HAVE_FPNSW
        fpnSw_api_attrib_t* fpnSw_attrib =
            (fpnSw_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), fpnSw));
        if (pCamHw->mFpnStreamUnit && fpnSw_attrib && fpnSw_attrib->en) {
            if (pCamHw->mNoReadBack) {
                pCamHw->mNoReadBack = false;
                LOGK_CAMHW_SUBM(ISP20HW_SUBM, "fpnSw force use read back mode!");
            }
        }
#endif
    }
#endif

#if 0 // have no this restriction from isp39
    // multimplex mode should be using readback mode
    if (s_info->isp_info->isMultiplex) pCamHw->mNoReadBack = false;
#endif
    if (pCamHw->mTbInfo.is_fastboot) {
        pCamHw->mNoReadBack = true;
    }

#if 0 // have no this restriction for new aiisp drv
    if (pCamHw->use_aiisp) pCamHw->mNoReadBack = false;
#endif

    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "isp hw working mode: %s !",
                    pCamHw->mNoReadBack ? "online" : "readback");

    // sof event
    if (!pCamHw->mIspSofStream) {
        if (pCamHw->_linked_to_isp) {
            pCamHw->mIspSofStream = (AiqSofEvtStream_t*)aiq_mallocz(sizeof(AiqSofEvtStream_t));
            AiqSofEvtStream_init(pCamHw->mIspSofStream, &pCamHw->mIspCoreDev->_v4l_base,
                                 ISP_POLL_SOF, pCamHw->_linked_to_1608);
        } else if (pCamHw->_linked_to_serdes || s_info->split) {
            pCamHw->mIspSofStream = (AiqSofEvtStream_t*)aiq_mallocz(sizeof(AiqSofEvtStream_t));
            AiqSofEvtStream_init(pCamHw->mIspSofStream, &pCamHw->mVicapItfDev->_v4l_base,
                                 ISP_POLL_SOF, pCamHw->_linked_to_1608);
        } else {
            pCamHw->mIspSofStream = (AiqSofEvtStream_t*)aiq_mallocz(sizeof(AiqSofEvtStream_t));
            AiqSofEvtStream_init(pCamHw->mIspSofStream, &pCamHw->_cif_csi2_sd->_v4l_base,
                                 ISP_POLL_SOF, pCamHw->_linked_to_1608);
        }
        ((AiqStream_t*)(pCamHw->mIspSofStream))
        ->setPollCallback((AiqStream_t*)(pCamHw->mIspSofStream), &pCamHw->mPollCb);
    }

    pCamHw->_isp_stream_status = ISP_STREAM_STATUS_INVALID;
    if (pCamHw->mIsListenStrmEvt && !pCamHw->mIspStremEvtTh) {
        pCamHw->mIspStremEvtTh =
            (AiqStreamEventPollThread_t*)aiq_mallocz(sizeof(AiqStreamEventPollThread_t));
        AiqStreamEventPollThread_init(pCamHw->mIspStremEvtTh, "StreamEvt",
                                      s_info->isp_info->input_params_path, pCamHw);
    }

#if defined(RKAIQ_HAVE_AIBNR) || defined(RKAIQ_HAVE_AIYNR)
    bool aibnr_en = false;
    bool aiynr_en = false;

#ifdef RKAIQ_HAVE_AIBNR
    aibnr_api_attrib_t* aibnr_attrib =
        (aibnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), aibnr));
    aibnr_en = aibnr_attrib->en;
#endif

#ifdef RKAIQ_HAVE_AIYNR
    aiynr_api_attrib_t* aiynr_attrib =
        (aiynr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), aiynr));
    aiynr_en = aiynr_attrib->en;
    pCamHw->_aiynr_en = aiynr_attrib->en;
#endif

    if (aibnr_en && aiynr_en) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't enable both aibnr and aiynr, only enable aibnr");
        pCamHw->use_aiisp = true; // use aibnr only
        pCamHw->_aiynr_en = false;
    } else {
        pCamHw->use_aiisp = aibnr_en || aiynr_en;
    }

#else
    // aiisp event
    if (!pCamHw->mIspAiispStream) {
        if (pCamHw->use_aiisp) {
            pCamHw->mIspAiispStream = (AiqAiIspStream_t*)aiq_mallocz(sizeof(AiqAiIspStream_t));
            AiqAiIspStream_init(pCamHw->mIspAiispStream, (AiqV4l2Device_t*)pCamHw->mIspCoreDev,
                                ISP_POLL_AIISP);
            pCamHw->mIspAiispStream->_base.setPollCallback(&pCamHw->mIspAiispStream->_base,
                    &pCamHw->mPollCb);
            pCamHw->mIspAiispStream->set_aiisp_linecnt(pCamHw->mIspAiispStream, &pCamHw->mAiisp_cfg);
        }
    } else {
        // restart aiisp for restart aiq without deinit
        pCamHw->mIspAiispStream->set_aiisp_linecnt(pCamHw->mIspAiispStream, &pCamHw->mAiisp_cfg);
    }
#endif
    //_init_bay3dbuf(pCamHw);

#ifdef RKAIQ_HAVE_AIRMS
    airms_api_attrib_t *airms_attrib = (airms_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), airms));

    pCamHw->_airms_en = airms_attrib->en;
    strcpy(pCamHw->airms_model_file, airms_attrib->stAuto.sta.model_path.sw_aiRmsCfg_model_file);
    if (pCamHw->use_aiisp && pCamHw->_airms_en) {
        airms_attrib->en = false;
        pCamHw->_airms_en = false;
        LOGE_AIRMS("%s: airms and aibnr/aiynr can not enable together, disable algo airms!!", __func__);
    }

    if (pCamHw->_airms_en) {
        AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
        rk_aiq_exposure_sensor_descriptor sns_des;

        if (mSensorSubdev->get_sensor_descriptor(mSensorSubdev, &sns_des)) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get_sensor_descriptor failed \n");
            return XCAM_RETURN_ERROR_UNKNOWN;
        }

        if (sns_des.bayer_mode != RKMODULE_QUARD_BAYER) {
            pCamHw->_airms_en = false;
            LOGI_AIRMS("%s: sensor bayer_mode is not RKMODULE_QUARD_BAYER, disable algo airms!!", __func__);
        }
    }
    if (pCamHw->_airms_en) {
        pCamHw->mNoReadBack = false;
        // set sensor stream flag.
        AiqRawStreamCapUnit_setSensorCategory(pCamHw->mRawCapUnit, false);
        AiqRawStreamProcUnit_setSensorCategory(pCamHw->mRawProcUnit, false);

        AiqRawStreamCapUnit_setTxBufferCnt(pCamHw->mRawCapUnit, AIRMS_VIPCAP_TX_BUF_NUM);
        AiqRawStreamCapUnit_set_devices(pCamHw->mRawCapUnit, pCamHw->mIspCoreDev, pCamHw,
                                        NULL, pCamHw->mAiRmsProcUnit);
        AiqAiRmsStreamProcUnit_set_devices(pCamHw->mAiRmsProcUnit, pCamHw->mRawProcUnit);
    }
#endif

    if (!pCamHw->use_rkrawstream) {
        if (!pCamHw->mNoReadBack) {
            _setupHdrLink(pCamHw, pCamHw->_hdr_mode, isp_index, true);
            if (!pCamHw->_linked_to_isp) {
                int cif_index = s_info->cif_info->model_idx;
                _setupHdrLink_vidcap(pCamHw, pCamHw->_hdr_mode, cif_index, true);
            }
        } else
            _setupHdrLink(pCamHw, RK_AIQ_WORKING_MODE_ISP_HDR3, isp_index, false);
    }
    sensorHw = pCamHw->_mSensorDev;
    ret      = sensorHw->set_working_mode(sensorHw, mode);
    if (ret) {
        LOGW_CAMHW_SUBM(ISP20HW_SUBM, "set sensor mode error !");
        return ret;
    }

    {
        uint32_t snsExpMode;
        calibdb_cis_hdr_t* cis_hdr =
            (calibdb_cis_hdr_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCamHw->mCalibDbV2, CisHdr));
        if (cis_hdr) {
            snsExpMode = cis_hdr->sta.hw_cisCfg_cisHdr_mode;

            if (sensorHw->set_exposure_mode(sensorHw, snsExpMode))
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set sensor exposure mode %d error !", snsExpMode);

            struct rkmodule_mge_oeWgt mge_oe_wgt;
            if (!AiqSensorHw_getMergeWgtCurve(sensorHw, &mge_oe_wgt)) {
                AiqSensorHwsendMergeWgtCurve2NR(&mge_oe_wgt,
                                                &pCamHw->_mIspParamsCvt->mergeLuma2Wgt);
            }
        }
    }

    if (!pCamHw->mTbInfo.is_fastboot) {
        if (pCamHw->mIsGroupMode) {
            ret = sensorHw->set_sync_mode(
                      sensorHw, pCamHw->mIsMain ? INTERNAL_MASTER_MODE : EXTERNAL_MASTER_MODE);
            if (ret) {
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "set sensor group mode error !\n");
            }
        } else {
            sensorHw->set_sync_mode(sensorHw, NO_SYNC_MODE);
        }
    }

    if (!pCamHw->use_rkrawstream) {
        AiqRawStreamCapUnit_set_working_mode(pCamHw->mRawCapUnit, mode);
        AiqRawStreamProcUnit_set_working_mode(pCamHw->mRawProcUnit, mode);
    }

    _setExpDelayInfo(pCamHw, mode);
    _setLensVcmModInfo(pCamHw, &s_info->mod_info);
    xcam_mem_clear(pCamHw->_lens_des);
    if (pCamHw->mLensDev) AiqLensHw_getLensModeData(pCamHw->mLensDev, &pCamHw->_lens_des);

    if (!pCamHw->use_rkrawstream) {
        ret = _setupPipelineFmt(pCamHw);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "setupPipelineFmt err: %d\n", ret);
        }
    }

    memset(&isp_src_fmt, 0, sizeof(isp_src_fmt));
    isp_src_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
    isp_src_fmt.pad   = 2;
    ret               = pCamHw->mIspCoreDev->getFormat(pCamHw->mIspCoreDev, &isp_src_fmt);
#if defined(RKAIQ_HAVE_MULTIISP)
    if (ret == XCAM_RETURN_NO_ERROR && s_info->isp_info->is_multi_isp_mode) {
        uint16_t extended_pixel = g_mMultiIspExtendedPixel;
        uint32_t width          = isp_src_fmt.format.width;
        uint32_t height         = isp_src_fmt.format.height;
        if (!pCamHw->mParamsSplitter) {
            pCamHw->mParamsSplitter =
                (AiqIspParamsSplitter_t*)aiq_mallocz(sizeof(AiqIspParamsSplitter_t));
            AiqIspParamsSplitter_init(pCamHw->mParamsSplitter, 0);
        }

        struct rkisp_isp_info isp_act_info;
        memset(&isp_act_info, 0, sizeof(isp_act_info));
        if (pCamHw->mIspCoreDev->_v4l_base.io_control(&pCamHw->mIspCoreDev->_v4l_base,
                RKISP_CMD_GET_ISP_INFO, &isp_act_info) != 0) {
            LOGE_CAMHW("%s:%d can't get act isp info", __FUNCTION__, __LINE__);
        }

        if (isp_act_info.act_width  == width / 2 + extended_pixel &&
                isp_act_info.act_height == height / 2 + extended_pixel) {
            AiqIspParamsSplitter_SetIspUniteMode(pCamHw->mParamsSplitter, RK_AIQ_ISP_UNITE_MODE_FOUR_GRID);
            ASSIGN_POINT(split_rectangle, 0, 0, width, height);
            AiqIspParamsSplitter_SetPicInfo(pCamHw->mParamsSplitter, &split_rectangle);
            ASSIGN_POINT(split_rectangle, 0, 0, width / 2 + extended_pixel,
                         height / 2 + extended_pixel);
            AiqIspParamsSplitter_SetLeftIspRect(pCamHw->mParamsSplitter, &split_rectangle);
            ASSIGN_POINT(split_rectangle, width / 2 - extended_pixel, 0, width / 2 + extended_pixel,
                         height / 2 + extended_pixel);
            AiqIspParamsSplitter_SetLeftIspRect(pCamHw->mParamsSplitter, &split_rectangle);
            AiqIspParamsSplitter_SetRightIspRect(pCamHw->mParamsSplitter, &split_rectangle);
            ASSIGN_POINT(split_rectangle, 0, height / 2 - extended_pixel, width / 2 + extended_pixel,
                         height / 2 + extended_pixel);
            AiqIspParamsSplitter_SetBottomLeftIspRect(pCamHw->mParamsSplitter, &split_rectangle);
            ASSIGN_POINT(split_rectangle, width / 2 - extended_pixel, height / 2 - extended_pixel,
                         width / 2 + extended_pixel, height / 2 + extended_pixel);
            AiqIspParamsSplitter_SetBottomRightIspRect(pCamHw->mParamsSplitter, &split_rectangle);
        } else if (isp_act_info.act_width  == width / 2 + extended_pixel &&
                   isp_act_info.act_height == height) {
            AiqIspParamsSplitter_SetIspUniteMode(pCamHw->mParamsSplitter, RK_AIQ_ISP_UNITE_MODE_TWO_GRID);
            ASSIGN_POINT(split_rectangle, 0, 0, width, height);
            AiqIspParamsSplitter_SetPicInfo(pCamHw->mParamsSplitter, &split_rectangle);
            ASSIGN_POINT(split_rectangle, 0, 0, width / 2 + extended_pixel, height);
            AiqIspParamsSplitter_SetLeftIspRect(pCamHw->mParamsSplitter, &split_rectangle);
            ASSIGN_POINT(split_rectangle, width / 2 - extended_pixel, 0, width / 2 + extended_pixel,
                         height);
            AiqIspParamsSplitter_SetRightIspRect(pCamHw->mParamsSplitter, &split_rectangle);
        } else {
            AiqIspParamsSplitter_SetIspUniteMode(pCamHw->mParamsSplitter, RK_AIQ_ISP_UNITE_MODE_NORMAL);
            ASSIGN_POINT(split_rectangle, 0, 0, width, height);
            AiqIspParamsSplitter_SetPicInfo(pCamHw->mParamsSplitter, &split_rectangle);
        }

        {
            const Splitter_Rectangle_t* f =
                AiqIspParamsSplitter_GetPicInfo(pCamHw->mParamsSplitter);
            const Splitter_Rectangle_t* l =
                AiqIspParamsSplitter_GetLeftIspRect(pCamHw->mParamsSplitter);
            const Splitter_Rectangle_t* r =
                AiqIspParamsSplitter_GetRightIspRect(pCamHw->mParamsSplitter);
            LOGK_ANALYZER(
                "Set Multi-ISP Mode ParamSplitter:\n"
                " Unite mode: %d\n"
                " Extended Pixel%d\n"
                " F : { %u, %u, %u, %u }\n"
                " L : { %u, %u, %u, %u }\n"
                " R : { %u, %u, %u, %u }\n",
                AiqIspParamsSplitter_GetIspUniteMode(pCamHw->mParamsSplitter),
                extended_pixel, f->x, f->y, f->w, f->h, l->x, l->y, l->w, l->h, r->x, r->y, r->w,
                r->h);
        }
    }
#endif

#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcStream && pCamHw->mMemcEn) {
        // memc event
        if (!pCamHw->mIspMemcIirStream) {
            if (pCamHw->mMemcEn) {
                pCamHw->mAiisp_cfg.mode = 0;
                pCamHw->mAiisp_cfg.rd_linecnt = 0;
                pCamHw->mAiisp_cfg.wr_linecnt =  isp_src_fmt.format.height / 4;
                pCamHw->mAiisp_cfg.wr_mode = 1;

                pCamHw->mIspMemcIirStream = (AiqMemcIirStream_t*)aiq_mallocz(sizeof(AiqMemcIirStream_t));
                AiqMemcIirStream_init(pCamHw->mIspMemcIirStream, (AiqV4l2Device_t*)pCamHw->mAiIspCoreDev,
                                      ISP_POLL_MEMC);
                pCamHw->mIspMemcIirStream->_base.setPollCallback(&pCamHw->mIspMemcIirStream->_base,
                        &pCamHw->mPollCb);
                pCamHw->mIspMemcIirStream->set_Memc_linecnt(pCamHw->mIspMemcIirStream, pCamHw->mAiisp_cfg);
            }
        } else {
            // restart memc for restart aiq without deinit
            pCamHw->mIspMemcIirStream->set_Memc_linecnt(pCamHw->mIspMemcIirStream, pCamHw->mAiisp_cfg);
        }

        AiqMemcManager_setAiispCfg(pCamHw->mMemcStream, pCamHw->mAiisp_cfg);
    }
#endif

    if (!pCamHw->_linked_to_isp && !pCamHw->mNoReadBack && !pCamHw->use_rkrawstream) {
        if (!pCamHw->_linked_to_1608) {
            AiqRawStreamCapUnit_prepare_cif_mipi(pCamHw->mRawCapUnit);
        } else {
            if (g_rk1608_share_inf.first_en[pCamHw->mCamPhyId]) {
                AiqRawStreamCapUnit_prepare_cif_mipi(pCamHw->mRawCapUnit);
                g_rk1608_share_inf.first_en[pCamHw->mCamPhyId] = 0;
            }
        }
    }

#if defined(RKAIQ_ENABLE_SPSTREAM)
    if ((pCamHw->_cur_calib_infos.mfnr.enable && pCamHw->_cur_calib_infos.mfnr.motion_detect_en) ||
            pCamHw->_cur_calib_infos.af.ldg_param.enable) {
        AiqSPStreamProcUnit_prepare(pCamHw->mSpStreamUnit, &pCamHw->_cur_calib_infos.af.ldg_param,
                                    &pCamHw->_cur_calib_infos.af.highlight, 0, 0, 0);
    }
#endif

#if RKAIQ_HAVE_AF_V30
    CalibDbV2_AFV30_t* af_v30 =
        (CalibDbV2_AFV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af_v30));
    CalibDbV2_Af_Pdaf_t* pdaf = &af_v30->TuningPara.pdaf;

    if (pdaf && pdaf->enable) {
        bool find_flg = false;

        for (int i = 0; i < pdaf->pdResoInf_len; i++) {
            if ((pdaf->pdResoInf[i].imageWidth == isp_src_fmt.format.width) &&
                    (pdaf->pdResoInf[i].imageHeight == isp_src_fmt.format.height)) {
                pCamHw->mPdafInfo.pdaf_type   = (PdafSensorType_t)pdaf->pdResoInf[i].pdType;
                pCamHw->mPdafInfo.pdaf_width  = pdaf->pdResoInf[i].pdOutWidth;
                pCamHw->mPdafInfo.pdaf_height = pdaf->pdResoInf[i].pdOutHeight;
                find_flg                      = true;
                break;
            }
        }

        if (find_flg) {
            pCamHw->mPdafInfo.pdaf_lrdiffline = pdaf->pdLRInDiffLine;
            if (pCamHw->mPdafInfo.pdaf_type != PDAF_SENSOR_TYPE3) {
                _get_sensor_pdafinfo(pCamHw, s_info, &pCamHw->mPdafInfo);
            } else {
                pCamHw->mPdafInfo.pdaf_support = true;
                if (!pdaf->pdLRInDiffLine)
                    pCamHw->mPdafInfo.pdaf_width *= 2;
                else
                    pCamHw->mPdafInfo.pdaf_height *= 2;
                pCamHw->mPdafInfo.pdaf_pixelformat = V4l2_PIX_FMT_SPD16;
                strcpy(pCamHw->mPdafInfo.pdaf_vdev, s_info->isp_info->pdaf_path);
            }

            if (pCamHw->mPdafInfo.pdaf_support) {
                strcpy(pCamHw->mPdafInfo.sns_name, s_info->mod_info.base.sensor);
                AiqPdafStreamProcUnit_preapre(pCamHw->mPdafStreamUnit, &pCamHw->mPdafInfo);
            }
        } else {
            pCamHw->mPdafInfo.pdaf_support = false;
            LOGI_AF("can not find pd inf for %d * %d", isp_src_fmt.format.width, isp_src_fmt.format.height);
        }
    } else {
        pCamHw->mPdafInfo.pdaf_support = false;
    }
#endif

#if RKAIQ_HAVE_AF_V33
#if ISP_HW_V39
    CalibDbV2_AFV33_t* af_v33 =
        (CalibDbV2_AFV33_t*)CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af_v33);
    Af_Pdaf_t *pdaf = &af_v33->Pdaf;

    if (pdaf && pdaf->Enable) {
        bool find_flg = false;

        for (int i = 0; i < pdaf->PdResoInf_len; i++) {
            if ((pdaf->PdResoInf[i].ImageWidth == isp_src_fmt.format.width) &&
                    (pdaf->PdResoInf[i].ImageHeight == isp_src_fmt.format.height)) {
                pCamHw->mPdafInfo.pdaf_type   = (PdafSensorType_t)pdaf->PdResoInf[i].PdType;
                pCamHw->mPdafInfo.pdaf_width  = pdaf->PdResoInf[i].PdOutWidth;
                pCamHw->mPdafInfo.pdaf_height = pdaf->PdResoInf[i].PdOutHeight;
                find_flg                      = true;
                break;
            }
        }

        if (find_flg) {
            pCamHw->mPdafInfo.pdaf_lrdiffline = pdaf->PdLRInDiffLine;
            if (pCamHw->mPdafInfo.pdaf_type != PDAF_SENSOR_TYPE3) {
                _get_sensor_pdafinfo(pCamHw, s_info, &pCamHw->mPdafInfo);
            } else {
                pCamHw->mPdafInfo.pdaf_support = true;
                if (!pdaf->PdLRInDiffLine)
                    pCamHw->mPdafInfo.pdaf_width *= 2;
                else
                    pCamHw->mPdafInfo.pdaf_height *= 2;
                pCamHw->mPdafInfo.pdaf_pixelformat = V4l2_PIX_FMT_SPD16;
                strcpy(pCamHw->mPdafInfo.pdaf_vdev, s_info->isp_info->pdaf_path);
            }

            if (pCamHw->mPdafInfo.pdaf_support) {
                strcpy(pCamHw->mPdafInfo.sns_name, s_info->mod_info.base.sensor);
                AiqPdafStreamProcUnit_preapre(pCamHw->mPdafStreamUnit, &pCamHw->mPdafInfo);
            }
        } else {
            pCamHw->mPdafInfo.pdaf_support = false;
            LOGI_AF("can not find pd inf for %d * %d", width, height);
        }
    } else {
        pCamHw->mPdafInfo.pdaf_support = false;
    }
#else
    af_param_t* af_calib =
        (af_param_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), af_calib));
    af_pdaf_t *pdaf = &af_calib->pdaf;

    if (pdaf && pdaf->sw_afT_pdAf_en) {
        bool find_flg = false;

        for (int i = 0; i < pdaf->sw_afT_pdResoInfTbl_len; i++) {
            if ((pdaf->pdResoInfTbl[i].sw_afT_image_width == isp_src_fmt.format.width) &&
                    (pdaf->pdResoInfTbl[i].sw_afT_image_height == isp_src_fmt.format.height)) {
                pCamHw->mPdafInfo.pdaf_type   = (PdafSensorType_t)pdaf->pdResoInfTbl[i].sw_afT_pdSensorType_mode;
                pCamHw->mPdafInfo.pdaf_width  = pdaf->pdResoInfTbl[i].sw_afT_pdOut_width;
                pCamHw->mPdafInfo.pdaf_height = pdaf->pdResoInfTbl[i].sw_afT_pdOut_height;
                find_flg                      = true;
                break;
            }
        }

        if (find_flg) {
            pCamHw->mPdafInfo.pdaf_lrdiffline = pdaf->sw_afT_pdLRInDiffLine_en;
            if (pCamHw->mPdafInfo.pdaf_type != PDAF_SENSOR_TYPE3) {
                _get_sensor_pdafinfo(pCamHw, s_info, &pCamHw->mPdafInfo);
            } else {
                pCamHw->mPdafInfo.pdaf_support = true;
                if (!pdaf->sw_afT_pdLRInDiffLine_en)
                    pCamHw->mPdafInfo.pdaf_width *= 2;
                else
                    pCamHw->mPdafInfo.pdaf_height *= 2;
                pCamHw->mPdafInfo.pdaf_pixelformat = V4l2_PIX_FMT_SPD16;
                strcpy(pCamHw->mPdafInfo.pdaf_vdev, s_info->isp_info->pdaf_path);
            }

            if (pCamHw->mPdafInfo.pdaf_support) {
                strcpy(pCamHw->mPdafInfo.sns_name, s_info->mod_info.base.sensor);
                AiqPdafStreamProcUnit_preapre(pCamHw->mPdafStreamUnit, &pCamHw->mPdafInfo);
            }
        } else {
            pCamHw->mPdafInfo.pdaf_support = false;
            LOGI_AF("can not find pd inf for %d * %d", width, height);
        }
    } else {
        pCamHw->mPdafInfo.pdaf_support = false;
    }
#endif
#endif

#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcStream) {
        rk_aiq_exposure_sensor_descriptor sensor_des;
        AiqCamHw_getSensorModeData(pCamHw, pCamHw->sns_name, &sensor_des);
        AiqMemcManager_prepare(pCamHw->mMemcStream, &sensor_des);
    }
#endif

    if (pCamHw->mCifScaleStream) {
        AiqCifSclStream_set_working_mode(pCamHw->mCifScaleStream, mode);
        AiqCifSclStream_prepare(pCamHw->mCifScaleStream);
    }

#ifdef ISP_HW_V35
    if (pCamHw->mBtnrDsReproc) {
        AiqBtnrDsReproc_prepare(pCamHw->mBtnrDsReproc, isp_src_fmt.format.width, isp_src_fmt.format.height);
    }
#endif

#if RKAIQ_HAVE_DUMPSYS
    if (pCamHw->mNoReadBack) {
        aiq_notifier_remove_subscriber(&pCamHw->notifier, AIQ_NOTIFIER_MATCH_HWI_STREAM_CAP);
        aiq_notifier_remove_subscriber(&pCamHw->notifier, AIQ_NOTIFIER_MATCH_HWI_STREAM_PROC);
    }
#endif

    pCamHw->_state = CAM_HW_STATE_PREPARED;
    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _hdr_mipi_prepare_mode(AiqCamHwBase_t* pCamHw, int mode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int new_mode   = RK_AIQ_HDR_GET_WORKING_MODE(mode);

    if (!pCamHw->mNoReadBack && !pCamHw->use_rkrawstream) {
        bool init_state = true;
        if (pCamHw->_linked_to_1608) {
            if (g_rk1608_share_inf.us_prepare_cnt > 0) {
                init_state = false;
            }
            g_rk1608_share_inf.us_prepare_cnt++;
            if (g_rk1608_share_inf.us_prepare_cnt > 10) g_rk1608_share_inf.us_prepare_cnt = 10;
        }

        if (new_mode == RK_AIQ_WORKING_MODE_NORMAL ||
                RK_AIQ_HDR_IS_SENSOR_BUILTIN(mode)) {
            if (!pCamHw->_linked_to_1608) {
                ret = AiqRawStreamCapUnit_prepare(pCamHw->mRawCapUnit, MIPI_STREAM_IDX_0);
            } else {
                if (init_state) {
                    ret = AiqRawStreamCapUnit_prepare(pCamHw->mRawCapUnit, MIPI_STREAM_IDX_0);
                }
            }
            ret = AiqRawStreamProcUnit_prepare(pCamHw->mRawProcUnit, MIPI_STREAM_IDX_0);
#if RKAIQ_HAVE_AIRMS
            if (pCamHw->_airms_en)
                ret = AiqAiRmsStreamProcUnit_prepare(pCamHw->mAiRmsProcUnit,
                                                     pCamHw->_mIspParamsCvt->sensor_output_width,
                                                     pCamHw->_mIspParamsCvt->sensor_output_height,
                                                     pCamHw->airms_model_file);
#endif
        } else if (new_mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            if (!pCamHw->_linked_to_1608) {
                ret = AiqRawStreamCapUnit_prepare(pCamHw->mRawCapUnit,
                                                  MIPI_STREAM_IDX_0 | MIPI_STREAM_IDX_1);
            } else {
                if (init_state) {
                    ret = AiqRawStreamCapUnit_prepare(pCamHw->mRawCapUnit,
                                                      MIPI_STREAM_IDX_0 | MIPI_STREAM_IDX_1);
                }
            }
            ret = AiqRawStreamProcUnit_prepare(pCamHw->mRawProcUnit,
                                               MIPI_STREAM_IDX_0 | MIPI_STREAM_IDX_1);
        } else {
            if (!pCamHw->_linked_to_1608) {
                ret = AiqRawStreamCapUnit_prepare(pCamHw->mRawCapUnit, MIPI_STREAM_IDX_ALL);
            } else {
                if (init_state) {
                    ret = AiqRawStreamCapUnit_prepare(pCamHw->mRawCapUnit, MIPI_STREAM_IDX_ALL);
                }
            }
            ret = AiqRawStreamProcUnit_prepare(pCamHw->mRawProcUnit, MIPI_STREAM_IDX_ALL);
        }
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "hdr mipi start err: %d\n", ret);
        }
    }
    return ret;
}

static XCamReturn _hdr_mipi_start_mode(AiqCamHwBase_t* pCamHw, int mode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s enter", __FUNCTION__);

#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcStream) {
        AiqMemcManager_start(pCamHw->mMemcStream);
    }
#endif

    if (!pCamHw->mNoReadBack && !pCamHw->use_rkrawstream) {

#if RKAIQ_HAVE_FPNSW
        if (pCamHw->mFpnStreamUnit) {
            AiqFpnSwStreamProcUnit_start(pCamHw->mFpnStreamUnit);
        }
#endif

        if (!pCamHw->_linked_to_1608) {
            AiqRawStreamProcUnit_start(pCamHw->mRawProcUnit, mode);
            if (!AiqRawStreamProcUnit_getStreamOnStatus(pCamHw->mRawProcUnit)) {
                LOGE_CAMHW("rx stream on failed, now stop rx stream!\n");
                AiqRawStreamProcUnit_stop(pCamHw->mRawProcUnit);
            } else {
                AiqRawStreamCapUnit_start(pCamHw->mRawCapUnit, mode);
                if (!AiqRawStreamCapUnit_getStreamOnStatus(pCamHw->mRawCapUnit)) {
                    LOGE_CAMHW("tx stream on failed, now stop both tx and rx stream!\n");
                    AiqRawStreamCapUnit_stop(pCamHw->mRawCapUnit);
                    AiqRawStreamProcUnit_stop(pCamHw->mRawProcUnit);
                }
            }
#if RKAIQ_HAVE_AIRMS
            if (pCamHw->_airms_en)
                AiqAiRmsStreamProcUnit_start(pCamHw->mAiRmsProcUnit);
#endif
        } else {
            bool stream_on = false;
            aiqMutex_lock(&g_sync_1608_mutex);
            g_rk1608_share_inf.us_stream_cnt++;
            if (g_rk1608_share_inf.us_stream_cnt > 10) g_rk1608_share_inf.us_stream_cnt = 10;
            if (g_rk1608_share_inf.us_stream_cnt >= g_rk1608_share_inf.en_sns_num) {
                // only processed the last streaming
                stream_on = true;
            }
            if (stream_on) {
                AiqRawStreamCapUnit_start(pCamHw->mRawCapUnit, mode);
            }
            AiqRawStreamProcUnit_start(pCamHw->mRawProcUnit, mode);
            if (stream_on) {
                g_sync_1608_done = true;
                aiqCond_broadcast(&pCamHw->_sync_done_cond);
            } else {
                g_sync_1608_done = false;
            }
            aiqMutex_unlock(&g_sync_1608_mutex);
        }
    }
    if (pCamHw->mCifScaleStream) AiqCifSclStream_start(pCamHw->mCifScaleStream);
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "%s exit", __FUNCTION__);
    return ret;
}

static XCamReturn _hdr_mipi_stop(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcStream) {
        AiqMemcManager_stop(pCamHw->mMemcStream);
    }
#endif

    if (!pCamHw->mNoReadBack && !pCamHw->use_rkrawstream) {

#if RKAIQ_HAVE_FPNSW
        if (pCamHw->mFpnStreamUnit) {
            AiqFpnSwStreamProcUnit_stop(pCamHw->mFpnStreamUnit);
        }
#endif

        AiqRawStreamProcUnit_stop(pCamHw->mRawProcUnit);
#if RKAIQ_HAVE_AIRMS
        if (pCamHw->_airms_en)
            AiqAiRmsStreamProcUnit_stop(pCamHw->mAiRmsProcUnit);
#endif
        if (!pCamHw->_linked_to_1608) {
            AiqRawStreamCapUnit_stop(pCamHw->mRawCapUnit);
        } else {
            bool stop_en = false;
            g_rk1608_share_inf.us_stop_cnt++;
            if (g_rk1608_share_inf.us_stop_cnt > 10) g_rk1608_share_inf.us_stop_cnt = 10;
            if (g_rk1608_share_inf.us_stop_cnt == g_rk1608_share_inf.en_sns_num) {
                stop_en = true;
            }
            if (stop_en) {
                AiqRawStreamCapUnit_stop(pCamHw->mRawCapUnit);
            }
        }
    }
    if (pCamHw->mCifScaleStream) AiqCifSclStream_stopThreadOnly(pCamHw->mCifScaleStream);
    return ret;
}

XCamReturn AiqCamHw_start(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    AiqSensorHw_t* sensorHw = NULL;
    AiqLensHw_t* lensHw     = NULL;

    ENTER_CAMHW_FUNCTION();
    sensorHw = pCamHw->_mSensorDev;
    lensHw   = pCamHw->mLensDev;

    if (pCamHw->_state != CAM_HW_STATE_PREPARED && pCamHw->_state != CAM_HW_STATE_STOPPED) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "camhw state err: %d\n", ret);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (pCamHw->mIspSofStream) {
        AiqStream_t* stream = (AiqStream_t*)(pCamHw->mIspSofStream);
        stream->setCamPhyId(stream, pCamHw->mCamPhyId);
        stream->start(stream);
    }
#if !defined(RKAIQ_HAVE_AIBNR) && !defined(RKAIQ_HAVE_AIYNR)
    if (pCamHw->mIspAiispStream) {
        AiqStream_t* stream = (AiqStream_t*)(pCamHw->mIspAiispStream);
        stream->setCamPhyId(stream, pCamHw->mCamPhyId);
        stream->start(stream);
    }
#endif
#if RKAIQ_HAVE_MEMC
    if (pCamHw->mIspMemcIirStream) {
        AiqStream_t* stream = (AiqStream_t*)(pCamHw->mIspMemcIirStream);
        stream->setCamPhyId(stream, pCamHw->mCamPhyId);
        stream->start(stream);
    }
#endif
    if (pCamHw->_linked_to_isp)
        AiqV4l2Device_subscribeEvt(&pCamHw->mIspCoreDev->_v4l_base, V4L2_EVENT_FRAME_SYNC);

    if (pCamHw->mIspStremEvtTh) {
        AiqPollThread_t* streamTh = (AiqPollThread_t*)(pCamHw->mIspStremEvtTh);
        ret                       = streamTh->start(streamTh);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "start isp stream event failed: %d\n", ret);
        }
    } else {
        ret = _hdr_mipi_start_mode(pCamHw, pCamHw->_hdr_mode);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "hdr mipi start err: %d\n", ret);
        }
    }

    ret = pCamHw->mIspCoreDev->_v4l_base.start(&pCamHw->mIspCoreDev->_v4l_base, true);
    if (ret < 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "start isp core dev err: %d\n", ret);
    }

#ifdef ISP_HW_V35
    if (pCamHw->mBtnrDsReproc) {
        AiqBtnrDsReproc_start(pCamHw->mBtnrDsReproc);
    }
#endif

    if (pCamHw->mIspStatsStream) {
        AiqStream_t* stream = (AiqStream_t*)(pCamHw->mIspStatsStream);
        stream->start(stream);
    }

#if defined(RKAIQ_ENABLE_SPSTREAM)
    if ((pCamHw->_cur_calib_infos.mfnr.enable && pCamHw->_cur_calib_infos.mfnr.motion_detect_en) ||
            pCamHw->_cur_calib_infos.af.ldg_param.enable) {
        AiqSPStreamProcUnit_start(pCamHw->mSpStreamUnit);
    }
#endif
#if RKAIQ_HAVE_PDAF
    if (pCamHw->mPdafInfo.pdaf_support) {
        AiqPdafStreamProcUnit_start(pCamHw->mPdafStreamUnit);
    }
#endif
    sensorHw->start(sensorHw, true);
    if (lensHw) lensHw->start(lensHw, false);
    pCamHw->_is_exit = false;
    pCamHw->_state   = CAM_HW_STATE_STARTED;

    LOGK_CAMHW("cid[%d] %s success. isGroup:%d, isOnline:%d, isMultiIsp:%d, init_ens:0x%llx",
               pCamHw->mCamPhyId, __func__, pCamHw->mIsGroupMode, pCamHw->mNoReadBack,
               g_mIsMultiIspMode, pCamHw->_isp_module_ens);
    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqCamHw_stop(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCamHw->_state == CAM_HW_STATE_STOPPED) return ret;

    if (pCamHw->mIspStatsStream)
        pCamHw->mIspStatsStream->_base.stop(&pCamHw->mIspStatsStream->_base);
    if (pCamHw->mIspSofStream) pCamHw->mIspSofStream->_base.stop(&pCamHw->mIspSofStream->_base);
#if !defined(RKAIQ_HAVE_AIBNR) && !defined(RKAIQ_HAVE_AIYNR)
    if (pCamHw->mIspAiispStream) pCamHw->mIspAiispStream->_base.stop(&pCamHw->mIspAiispStream->_base);
#endif
#if defined(RKAIQ_HAVE_MEMC)
    if (pCamHw->mIspMemcIirStream) pCamHw->mIspMemcIirStream->_base.stop(&pCamHw->mIspMemcIirStream->_base);
#endif
#if defined(RKAIQ_ENABLE_SPSTREAM)
    if ((pCamHw->_cur_calib_infos.mfnr.enable && pCamHw->_cur_calib_infos.mfnr.motion_detect_en) ||
            pCamHw->_cur_calib_infos.af.ldg_param.enable) {
        if (pCamHw->mSpStreamUnit) AiqSPStreamProcUnit_stop(pCamHw->mSpStreamUnit);
    }
#endif
#if RKAIQ_HAVE_PDAF
    if (pCamHw->mPdafInfo.pdaf_support) {
        AiqPdafStreamProcUnit_stop(pCamHw->mPdafStreamUnit);
    }
#endif
    if (pCamHw->mTbInfo.is_fastboot) {
        if (SetLastAeExpToRttShared(pCamHw) != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "<TB>: fastboot rewrite ae info err: %d\n", ret);
    }

#ifdef ISP_HW_V35
    if (pCamHw->mBtnrDsReproc) {
        AiqBtnrDsReproc_stop(pCamHw->mBtnrDsReproc);
    }
#endif

    // stop after pollthread, ensure that no new events
    // come into snesorHw
    pCamHw->_mSensorDev->stop(pCamHw->_mSensorDev);

    if (pCamHw->mLensDev) pCamHw->mLensDev->stop(pCamHw->mLensDev);

    if (pCamHw->_linked_to_isp)
        AiqV4l2Device_unsubscribeEvt(&pCamHw->mIspCoreDev->_v4l_base, V4L2_EVENT_FRAME_SYNC);
    pCamHw->mIspCoreDev->_v4l_base.stop(&pCamHw->mIspCoreDev->_v4l_base);
    if (ret < 0) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "stop isp core dev err: %d\n", ret);
    }

    if (pCamHw->mIspStremEvtTh) {
        pCamHw->mIspStremEvtTh->_base._base.stop(&pCamHw->mIspStremEvtTh->_base._base);
        if (true) {
            aiqMutex_lock(&pCamHw->_stop_cond_mutex);
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "stop isp stream");
            if (pCamHw->mIspParamsDev) AiqV4l2Device_stop(pCamHw->mIspParamsDev);
            _hdr_mipi_stop(pCamHw);
            pCamHw->_isp_stream_status = ISP_STREAM_STATUS_INVALID;
            aiqMutex_unlock(&pCamHw->_stop_cond_mutex);
        }
    } else {
        if (pCamHw->mIspParamsDev) AiqV4l2Device_stop(pCamHw->mIspParamsDev);

        ret = _hdr_mipi_stop(pCamHw);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "hdr mipi stop err: %d\n", ret);
        }
    }

    if (!pCamHw->mKpHwSt) _setIrcutParams(pCamHw, false);

    {
        // TODO: [baron] PAL/NTSC mode convert[Start / Stop]. recovery cnt.
        // [Stage 02] {start} <-> {stop}
        g_rk1608_share_inf.us_stream_cnt = 0;
        if (g_rk1608_share_inf.us_stop_cnt >= g_rk1608_share_inf.en_sns_num) {
            // ensure all valid 1608-sensor stoped.
            g_rk1608_share_inf.us_stop_cnt = 0;
        }
    }

    if (pCamHw->_mIspParamsCvt) {
        AiqAutoblc_deinit(pCamHw->_mIspParamsCvt);
    }
    _deinit_bay3dbuf(pCamHw);

    pCamHw->_state = CAM_HW_STATE_STOPPED;
    return ret;
}

void AiqCamHw_clean(AiqCamHwBase_t* pCamHw)
{
    AiqMapItem_t* pItem = NULL;
    bool rm = false;
    AIQ_MAP_FOREACH(pCamHw->_effecting_ispparam_map, pItem, rm) {
        aiq_isp_effect_params_t* params = *(aiq_isp_effect_params_t**)(pItem->_pData);
        pItem = aiqMap_erase_locked(pCamHw->_effecting_ispparam_map, pItem->_key);
        rm = true;
        AIQ_REF_BASE_UNREF(&params->_ref_base);
    }
    aiqMap_reset(pCamHw->_effecting_ispparam_map);
    aiqPool_reset(pCamHw->mEffectIspParamsPool);
    AiqSensorHw_clean(pCamHw->_mSensorDev);
}

#if RKAIQ_HAVE_PDAF
static XCamReturn _showOtpPdafData(struct rkmodule_pdaf_inf* otp_pdaf) {
    unsigned int gainmap_w, gainmap_h;
    unsigned int dccmap_w, dccmap_h;
    char print_buf[256];
    unsigned int i, j;

    if (otp_pdaf->flag) {
        gainmap_w = otp_pdaf->gainmap_width;
        gainmap_h = otp_pdaf->gainmap_height;
        dccmap_w  = otp_pdaf->dccmap_width;
        dccmap_h  = otp_pdaf->dccmap_height;
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "[RKPDAFOTPParam]");
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "flag=%d;", otp_pdaf->flag);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "gainmap_width=%d;", gainmap_w);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "gainmap_height=%d;", gainmap_h);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "gainmap_table=");
        for (i = 0; i < gainmap_h; i++) {
            memset(print_buf, 0, sizeof(print_buf));
            for (j = 0; j < gainmap_w; j++) {
                sprintf(print_buf + strlen(print_buf), "%d ", otp_pdaf->gainmap[i * gainmap_w + j]);
            }
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "%s", print_buf);
        }

        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "dcc_mode=%d;", otp_pdaf->dcc_mode);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "dcc_dir=%d;", otp_pdaf->dcc_dir);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "dccmap_width=%d;", otp_pdaf->dccmap_width);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "dccmap_height=%d;", otp_pdaf->dccmap_height);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "dccmap_table=");
        for (i = 0; i < dccmap_h; i++) {
            memset(print_buf, 0, sizeof(print_buf));
            for (j = 0; j < dccmap_w; j++) {
                sprintf(print_buf + strlen(print_buf), "%d ", otp_pdaf->dccmap[i * dccmap_w + j]);
            }
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "%s", print_buf);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif
static XCamReturn _showOtpAfData(struct rkmodule_af_inf* af_inf) {
    unsigned int i;

    if (af_inf->flag) {
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "[RKAFOTPParam]");
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "flag=%d;", af_inf->flag);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "dir_cnt=%d;", af_inf->dir_cnt);

        for (i = 0; i < af_inf->dir_cnt; i++) {
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "af_inf=%d;", af_inf->af_otp[i].vcm_dir);
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "af_macro=%d;", af_inf->af_otp[i].vcm_start);
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "af_macro=%d;", af_inf->af_otp[i].vcm_end);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamHw_getSensorModeData(AiqCamHwBase_t* pCamHw, const char* sns_ent_name,
                                      rk_aiq_exposure_sensor_descriptor* sns_des) {
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
    AiqLensHw_t* mLensSubdev     = pCamHw->mLensDev;
    struct v4l2_subdev_selection select;
    struct rkisp_isp_info isp_info;
    struct rkmodule_hdr_compr_single_frame_info compr_single_info;

    ret = pCamHw->_mSensorDev->getSensorModeData(pCamHw->_mSensorDev, sns_ent_name, sns_des);
    if (ret) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "getSensorModeData failed \n");
        return ret;
    }

    if (pCamHw->mTbInfo.is_fastboot) {
        if (pCamHw->mTbInfo.is_start_again) {
            sns_des->pixel_clock_freq_mhz = pCamHw->mTbInfo.pixel_clock_freq_mhz;
        } else {
            pCamHw->mTbInfo.pixel_clock_freq_mhz = sns_des->pixel_clock_freq_mhz;
        }
    }

    xcam_mem_clear(select);
    ret = pCamHw->mIspCoreDev->get_selection(pCamHw->mIspCoreDev, 0, V4L2_SEL_TGT_CROP, &select);
    if (ret == XCAM_RETURN_NO_ERROR) {
        sns_des->isp_acq_width  = select.r.width;
        sns_des->isp_acq_height = select.r.height;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "get isp acq,w: %d, h: %d\n", sns_des->isp_acq_width,
                        sns_des->isp_acq_height);
    } else {
        LOGW_CAMHW_SUBM(ISP20HW_SUBM, "get selecttion error \n");
        sns_des->isp_acq_width  = sns_des->sensor_output_width;
        sns_des->isp_acq_height = sns_des->sensor_output_height;
        ret                     = XCAM_RETURN_NO_ERROR;
    }

    pCamHw->_mIspParamsCvt->sensor_output_width  = sns_des->isp_acq_width;
    pCamHw->_mIspParamsCvt->sensor_output_height = sns_des->isp_acq_height;

    pCamHw->_mIspParamsCvt->mCommonCvtInfo.rawWidth  = sns_des->isp_acq_width;
    pCamHw->_mIspParamsCvt->mCommonCvtInfo.rawHeight = sns_des->isp_acq_height;
    switch (sns_des->sensor_pixelformat) {
    case V4L2_PIX_FMT_SBGGR14:
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SBGGR10:
        pCamHw->_mIspParamsCvt->mCommonCvtInfo.bayer_fmt = 0;
        break;
    case V4L2_PIX_FMT_SGBRG14:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGBRG10:
        pCamHw->_mIspParamsCvt->mCommonCvtInfo.bayer_fmt = 1;
        break;
    case V4L2_PIX_FMT_SGRBG14:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SGRBG10:
        pCamHw->_mIspParamsCvt->mCommonCvtInfo.bayer_fmt = 2;
        break;
    case V4L2_PIX_FMT_SRGGB14:
    case V4L2_PIX_FMT_SRGGB12:
    case V4L2_PIX_FMT_SRGGB10:
        pCamHw->_mIspParamsCvt->mCommonCvtInfo.bayer_fmt = 3;
        break;
    default:
        pCamHw->_mIspParamsCvt->mCommonCvtInfo.bayer_fmt = 0;
    }

#if ISP_HW_V39
    pCamHw->_mIspParamsCvt->mCacInfo.mem_ops =
        pCamHw->rkAiqManager->mRkAiqAnalyzer->mShareMemOps;
    pCamHw->_mIspParamsCvt->mCacInfo.is_multi_sensor =
        pCamHw->rkAiqManager->mRkAiqAnalyzer->mAlogsComSharedParams.is_multi_sensor;
    pCamHw->_mIspParamsCvt->mCacInfo.is_multi_isp = g_mIsMultiIspMode;
    pCamHw->_mIspParamsCvt->mCacInfo.multi_isp_extended_pixel = g_mMultiIspExtendedPixel;
    if (g_mIsMultiIspMode && pCamHw->mParamsSplitter) {
        pCamHw->_mIspParamsCvt->mCacInfo.isp_unite_mode =
            AiqIspParamsSplitter_GetIspUniteMode(pCamHw->mParamsSplitter);
    }
    if (pCamHw->rkAiqManager->mRkAiqAnalyzer->mAlogsComSharedParams.resourcePath) {
        strcpy(pCamHw->_mIspParamsCvt->mCacInfo.iqpath, pCamHw->rkAiqManager->mRkAiqAnalyzer->mAlogsComSharedParams.resourcePath);
    } else {
        strcpy(pCamHw->_mIspParamsCvt->mCacInfo.iqpath, "/etc/iqfiles");
    }
#endif
    pCamHw->_mIspParamsCvt->mCommonCvtInfo.mge_oe_wgt = &pCamHw->mSnsDes.mge_oe_wgt;

    xcam_mem_clear(isp_info);

    if (pCamHw->mIspCoreDev->_v4l_base.io_control(&pCamHw->mIspCoreDev->_v4l_base,
            RKISP_CMD_GET_ISP_INFO, &isp_info) == 0) {
        sns_des->compr_bit = isp_info.compr_bit;
    }

    if (AiqSensorHw_getBuiltInHdrSingleBit(pCamHw->_mSensorDev, &compr_single_info) == 0) {
        sns_des->compr_single_bit = compr_single_info.single_bitwidth;
    }

    xcam_mem_clear(sns_des->lens_des);
    if (pCamHw->mLensDev) AiqLensHw_getLensModeData(pCamHw->mLensDev, &sns_des->lens_des);

    SnsFullInfoWraps_t* pSnsInfoWrap = NULL;

    pSnsInfoWrap = g_mSensorHwInfos;
    while (pSnsInfoWrap) {
        if (strcmp(pCamHw->sns_name, pSnsInfoWrap->key) == 0) break;
        pSnsInfoWrap = pSnsInfoWrap->next;
    }

    if (!pSnsInfoWrap) {
        LOGW_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", pCamHw->sns_name);
    } else {
        struct rkmodule_inf* minfo = &(pSnsInfoWrap->data.mod_info);
        if (minfo->awb.flag) {
            memcpy(&sns_des->otp_awb, &minfo->awb, sizeof(minfo->awb));
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "sensor config otp_awb");
        } else {
            sns_des->otp_awb.flag = 0;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "sensor otp_awb is null");
        }

        if (minfo->lsc.flag) {
            sns_des->otp_lsc = &minfo->lsc;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "sensor config otp_lsc");
        } else {
            sns_des->otp_lsc = NULL;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "sensor otp_lsc is null");
        }

        if (minfo->af.flag) {
            sns_des->otp_af = &minfo->af;
            _showOtpAfData(sns_des->otp_af);
        } else {
            sns_des->otp_af = NULL;
        }

        if (minfo->pdaf.flag) {
#if RKAIQ_HAVE_PDAF
            sns_des->otp_pdaf = &minfo->pdaf;
            _showOtpPdafData(sns_des->otp_pdaf);
        } else {
#endif
            sns_des->otp_pdaf = NULL;
        }
    }

    pCamHw->mSnsDes = *sns_des;

    return ret;
}

XCamReturn AiqCamHw_setExposureParams(AiqCamHwBase_t* pCamHw, AiqAecExpInfoWrapper_t* expPar) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    // exp
    ret = pCamHw->_mSensorDev->setExposureParams(pCamHw->_mSensorDev, expPar);
    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _setIrisParams(AiqCamHwBase_t* pCamHw, AiqIrisInfoWrapper_t* irisPar,
                                 CalibDb_IrisTypeV2_t irisType) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();
    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    if (irisType == IRISV2_P_TYPE) {
        // P-iris
        int step    = irisPar->PIris.step;
        bool update = irisPar->PIris.update;

        if (mLensSubdev && update) {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||set P-Iris step: %d", step);
            if (AiqLensHw_setPIrisParams(mLensSubdev, step) < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set P-Iris step failed to device");
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    } else if (irisType == IRISV2_DC_TYPE) {
        // DC-iris
        int PwmDuty = irisPar->DCIris.pwmDuty;
        bool update = irisPar->DCIris.update;

        if (mLensSubdev && update) {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||set DC-Iris PwmDuty: %d", PwmDuty);
            if (AiqLensHw_setDCIrisParams(mLensSubdev, PwmDuty) < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set DC-Iris PwmDuty failed to device");
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    } else if (irisType == IRISV2_HDC_TYPE) {
        // HDC-iris
        int target  = irisPar->HDCIris.target;
        bool update = irisPar->HDCIris.update;
        if (mLensSubdev && update) {
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||set HDC-Iris Target: %d", target);
            if (AiqLensHw_setHDCIrisParams(mLensSubdev, target) < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set HDC-Iris target failed to device");
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }
    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _getIrisParams(AiqCamHwBase_t* pCamHw, AiqIrisInfoWrapper_t* irisPar,
                                 CalibDb_IrisTypeV2_t irisType) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();
    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    if (irisType == IRISV2_HDC_TYPE) {
        // HDC-iris
        int adc      = 0;
        int position = 0;
        if (mLensSubdev) {
            if (AiqLensHw_getHDCIrisParams(mLensSubdev, &adc) < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get HDC-Iris adc failed to device");
                return XCAM_RETURN_ERROR_IOCTL;
            }
            if (AiqLensHw_getZoomParams(mLensSubdev, &position) < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get zoom result failed to device");
                return XCAM_RETURN_ERROR_IOCTL;
            }
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||get HDC-Iris ADC: %d  get zoomPos: %d\n", adc,
                            position);
            irisPar->HDCIris.adc     = adc;
            irisPar->HDCIris.zoomPos = position;
        }
    }
    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _setFocusParams(AiqCamHwBase_t* pCamHw, rk_aiq_focus_params_t* focus_params) {
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    AiqLensHw_t* mLensSubdev       = pCamHw->mLensDev;
    rk_aiq_focus_params_t* p_focus = focus_params;
    bool focus_valid               = p_focus->lens_pos_valid;
    bool zoom_valid                = p_focus->zoom_pos_valid;
    bool focus_correction          = p_focus->focus_correction;
    bool zoom_correction           = p_focus->zoom_correction;
    bool zoomfocus_modifypos       = p_focus->zoomfocus_modifypos;
    bool end_zoom_chg              = p_focus->end_zoom_chg;
    bool IsNeedCkRebackAtStart     = p_focus->IsNeedCkRebackAtStart;
    bool vcm_config_valid          = p_focus->vcm_config_valid;
    rk_aiq_lens_vcmcfg lens_cfg;

    ENTER_CAMHW_FUNCTION();

    if (!mLensSubdev) goto OUT;

    if (zoomfocus_modifypos) AiqLensHw_ZoomFocusModifyPosition(mLensSubdev, focus_params);
    if (focus_correction) AiqLensHw_FocusCorrection(mLensSubdev);
    if (zoom_correction) AiqLensHw_ZoomCorrection(mLensSubdev);

    if (focus_valid && !zoom_valid) {
        if (AiqLensHw_setFocusParams(mLensSubdev, focus_params) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set focus result failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    } else if ((focus_valid && zoom_valid) || end_zoom_chg || IsNeedCkRebackAtStart) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||setZoomFocusParams");
        if (AiqLensHw_setZoomFocusParams(mLensSubdev, focus_params) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set setZoomFocusParams failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    if (mLensSubdev && vcm_config_valid) {
        AiqLensHw_getLensVcmCfg(mLensSubdev, &lens_cfg);
        lens_cfg.start_ma = p_focus->vcm_start_ma;
        lens_cfg.rated_ma = p_focus->vcm_end_ma;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||set vcm config: %d, %d", lens_cfg.start_ma,
                        lens_cfg.rated_ma);
        if (AiqLensHw_setLensVcmCfg(mLensSubdev, &lens_cfg) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set vcm config failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

OUT:
    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn _getZoomPosition(AiqCamHwBase_t* pCamHw, int* position) {
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    if (mLensSubdev) {
        if (AiqLensHw_getZoomParams(mLensSubdev, position) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get zoom result failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||get zoom result: %d", *position);
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _setLensVcmCfg(AiqCamHwBase_t* pCamHw, rk_aiq_lens_vcmcfg* lens_cfg) {
    ENTER_CAMHW_FUNCTION();

    XCamReturn ret           = XCAM_RETURN_NO_ERROR;
    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    if (mLensSubdev) {
        if (AiqLensHw_setLensVcmCfg(mLensSubdev, lens_cfg) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set vcm config failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _FocusCorrection(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret           = XCAM_RETURN_NO_ERROR;
    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    ENTER_CAMHW_FUNCTION();

    if (mLensSubdev) {
        if (AiqLensHw_ZoomCorrection(mLensSubdev) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "focus correction failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _ZoomCorrection(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret           = XCAM_RETURN_NO_ERROR;
    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    ENTER_CAMHW_FUNCTION();

    if (mLensSubdev) {
        if (AiqLensHw_ZoomCorrection(mLensSubdev) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "zoom correction failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _getLensVcmCfg(AiqCamHwBase_t* pCamHw, rk_aiq_lens_vcmcfg* lens_cfg) {
    XCamReturn ret           = XCAM_RETURN_NO_ERROR;
    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    ENTER_CAMHW_FUNCTION();

    if (mLensSubdev) {
        if (AiqLensHw_getLensVcmCfg(pCamHw->mLensDev, lens_cfg) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get vcm config failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _setAngleZ(AiqCamHwBase_t* pCamHw, float angleZ) {
    XCamReturn ret           = XCAM_RETURN_NO_ERROR;
    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    ENTER_CAMHW_FUNCTION();

    if (mLensSubdev) {
        if (AiqLensHw_setAngleZ(mLensSubdev, angleZ) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "setAngleZ failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _getFocusPosition(AiqCamHwBase_t* pCamHw, int* position) {
    XCamReturn ret           = XCAM_RETURN_NO_ERROR;
    AiqLensHw_t* mLensSubdev = pCamHw->mLensDev;

    ENTER_CAMHW_FUNCTION();

    if (mLensSubdev) {
        if (AiqLensHw_getFocusParams(mLensSubdev, position) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get focus position failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||get focus position: %d", *position);
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

aiq_isp_effect_params_t* AiqCamHw_getParamsForEffMap(AiqCamHwBase_t* pCamHw, uint32_t frame_id) {
    AiqPoolItem_t* pPoolItem         = NULL;
    AiqMapItem_t* pMapItem =
        aiqMap_get(pCamHw->_effecting_ispparam_map, (void*)(uintptr_t)frame_id);
    aiq_isp_effect_params_t* pDstEff = NULL;
    if (!pMapItem) {
        pPoolItem = aiqPool_getFree(pCamHw->mEffectIspParamsPool);
        if (!pPoolItem) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "no free isp pool params for %d !", frame_id);
            return NULL;
        }

        pDstEff = (aiq_isp_effect_params_t*)(pPoolItem->_pData);
        aiqMap_insert(pCamHw->_effecting_ispparam_map, (void*)(uintptr_t)frame_id,
                      &(pPoolItem->_pData));
    } else {
        pDstEff = *(aiq_isp_effect_params_t**)(pMapItem->_pData);
    }

    return pDstEff;
}

XCamReturn AiqCamHw_getEffectiveIspParams(AiqCamHwBase_t* pCamHw,
        aiq_isp_effect_params_t** ispParams, uint32_t frame_id) {
    ENTER_CAMHW_FUNCTION();

    uint32_t search_id  = frame_id == (uint32_t)(-1) ? 0 : frame_id;
    AiqMapItem_t* pItem = NULL;

    aiqMutex_lock(&pCamHw->_isp_params_cfg_mutex);

    if (aiqMap_size(pCamHw->_effecting_ispparam_map) == 0) {
        if (frame_id != 0 && pCamHw->_state == CAM_HW_STATE_STARTED)
            LOGE_CAMHW_SUBM(
                ISP20HW_SUBM, "camId: %d, can't search id %d,  _effecting_ispparam_map is %d\n",
                pCamHw->mCamPhyId, frame_id, aiqMap_size(pCamHw->_effecting_ispparam_map));
        aiqMutex_unlock(&pCamHw->_isp_params_cfg_mutex);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pItem = aiqMap_get(pCamHw->_effecting_ispparam_map, (void*)(uintptr_t)search_id);

    // havn't found
    if (!pItem) {
        AiqMapItem_t* pLastItem         = NULL;
        bool rm = false;
        AIQ_MAP_FOREACH(pCamHw->_effecting_ispparam_map, pItem, rm) {
            if ((uint32_t)(long)pItem->_key >= search_id)
                break;
            pLastItem = pItem;
        }

        pItem = pLastItem;
        if (pItem) {
            LOGD_CAMHW_SUBM(
                ISP20HW_SUBM,
                "camId: %d, can't find id %d, get latest id %d in _effecting_ispparam_map\n",
                pCamHw->mCamPhyId, search_id, (uint32_t)(long)(pItem->_key));
            *ispParams = *(aiq_isp_effect_params_t**)(pItem->_pData);
        } else {
            LOGE_CAMHW_SUBM(
                ISP20HW_SUBM,
                "camId: %d, can't find the latest effecting ispparams for id %d, impossible case !",
                pCamHw->mCamPhyId, frame_id);
            aiqMutex_unlock(&pCamHw->_isp_params_cfg_mutex);
            return XCAM_RETURN_ERROR_PARAM;
        }
    } else {
        *ispParams = *(aiq_isp_effect_params_t**)(pItem->_pData);
    }

    if (*ispParams) AIQ_REF_BASE_REF(&(*ispParams)->_ref_base);

    aiqMutex_unlock(&pCamHw->_isp_params_cfg_mutex);

    EXIT_CAMHW_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamHw_setModuleCtl(AiqCamHwBase_t* pCamHw, rk_aiq_module_id_t moduleId, bool en) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pCamHw->_cur_calib_infos.mfnr.enable && pCamHw->_cur_calib_infos.mfnr.motion_detect_en) {
        if ((moduleId == RK_MODULE_TNR) && (en == false)) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "motion detect is running, operate not permit!");
            return XCAM_RETURN_ERROR_FAILED;
        }
    }
    // Todo
    // setModuleStatus(moduleId, en);
    return ret;
}

XCamReturn AiqCamHw_getModuleCtl(AiqCamHwBase_t* pCamHw, rk_aiq_module_id_t moduleId, bool* en) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    // Todo
    // getModuleStatus(moduleId, en);
    return ret;
}

XCamReturn AiqCamHw_notify_capture_raw(AiqCamHwBase_t* pCamHw) {
    if (pCamHw->mRawProcUnit && !pCamHw->use_rkrawstream)
        return AiqRawStreamProcUnit_notify_capture_raw(pCamHw->mRawProcUnit);
    else
        return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn AiqCamHw_capture_raw_ctl(AiqCamHwBase_t* pCamHw, capture_raw_t type, int count,
                                    const char* capture_dir, char* output_dir) {
    if (pCamHw->use_rkrawstream) return XCAM_RETURN_ERROR_FAILED;
    if (!pCamHw->mRawProcUnit) return XCAM_RETURN_ERROR_FAILED;

    if (type == CAPTURE_RAW_AND_YUV_SYNC)
        return AiqRawStreamProcUnit_capture_raw_ctl(pCamHw->mRawProcUnit, type, 0, NULL, NULL);
    else if (type == CAPTURE_RAW_SYNC)
        return AiqRawStreamProcUnit_capture_raw_ctl(pCamHw->mRawProcUnit, type, count, capture_dir,
                output_dir);

    return XCAM_RETURN_ERROR_FAILED;
}

static XCamReturn _setIrcutParams(AiqCamHwBase_t* pCamHw, bool on) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct v4l2_control control;

    ENTER_CAMHW_FUNCTION();

    xcam_mem_clear(control);
    control.id = V4L2_CID_BAND_STOP_FILTER;
    if (on)
        control.value = IRCUT_STATE_CLOSED;
    else
        control.value = IRCUT_STATE_OPENED;
    if (pCamHw->mIrcutDev) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "set ircut value: %d", control.value);
        if (pCamHw->mIrcutDev->_v4l_base.io_control(&pCamHw->mIrcutDev->_v4l_base, VIDIOC_S_CTRL,
                &control) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set ircut value failed to device!");
            ret = XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

uint64_t AiqCamHw_getIspModuleEnState(AiqCamHwBase_t* pCamHw) {
    return pCamHw->_isp_module_ens;
}

XCamReturn AiqCamHw_setSensorFlip(AiqCamHwBase_t* pCamHw, bool mirror, bool flip,
                                  int skip_frm_cnt) {
    AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;

    int32_t skip_frame_sequence = 0;
    if (!pCamHw->use_rkrawstream) {
        if (pCamHw->_state == CAM_HW_STATE_STARTED) {
            AiqRawStreamCapUnit_stop_vicap_stream_only(pCamHw->mRawCapUnit);
        }
    }
    ret = mSensorSubdev->set_mirror_flip(mSensorSubdev, mirror, flip, &skip_frame_sequence);
    if (!pCamHw->use_rkrawstream) {
        if (pCamHw->_state == CAM_HW_STATE_STARTED) {
            AiqRawStreamCapUnit_skip_frame_and_restart_vicap_stream(pCamHw->mRawCapUnit, skip_frm_cnt);
        }
    }
    /* struct timespec tp; */
    /* clock_gettime(CLOCK_MONOTONIC, &tp); */
    /* int64_t skip_ts = (int64_t)(tp.tv_sec) * 1000 * 1000 * 1000 + (int64_t)(tp.tv_nsec); */

    return ret;
}

XCamReturn AiqCamHw_getSensorFlip(AiqCamHwBase_t* pCamHw, bool* mirror, bool* flip) {
    AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;

    return mSensorSubdev->get_mirror_flip(mSensorSubdev, mirror, flip);
}

XCamReturn AiqCamHw_setSensorCrop(AiqCamHwBase_t* pCamHw, rk_aiq_rect_t* rect) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct v4l2_crop crop;

    if (pCamHw->use_rkrawstream) return ret;

    for (int8_t i = 0; i < 3; i++) {
        AiqV4l2Device_t* mipi_tx = AiqRawStreamCapUnit_get_tx_device(pCamHw->mRawCapUnit, i);
        memset(&crop, 0, sizeof(crop));
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (mipi_tx) {
            ret           = AiqV4l2Device_getCrop(mipi_tx, &crop);
            crop.c.left   = rect->left;
            crop.c.top    = rect->top;
            crop.c.width  = rect->width;
            crop.c.height = rect->height;
            ret           = AiqV4l2Device_setCrop(mipi_tx, &crop);
        }
    }
    pCamHw->_crop_rect = *rect;
    return ret;
}

XCamReturn AiqCamHw_getSensorCrop(AiqCamHwBase_t* pCamHw, rk_aiq_rect_t* rect) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct v4l2_crop crop;
    AiqV4l2Device_t* mipi_tx = NULL;

    if (pCamHw->use_rkrawstream) return ret;

    mipi_tx = AiqRawStreamCapUnit_get_tx_device(pCamHw->mRawCapUnit, 0);
    memset(&crop, 0, sizeof(crop));
    ret          = AiqV4l2Device_getCrop(mipi_tx, &crop);
    rect->left   = crop.c.left;
    rect->top    = crop.c.top;
    rect->width  = crop.c.width;
    rect->height = crop.c.height;
    return ret;
}

void AiqCamHw_setMulCamConc(AiqCamHwBase_t* pCamHw, bool cc) {
    if (pCamHw->use_rkrawstream) return;
    AiqRawStreamProcUnit_setMulCamConc(pCamHw->mRawProcUnit, cc);
    if (cc) pCamHw->mNoReadBack = false;
}

static void _allocMemResource(uint8_t id, void* ops_ctx, void* config, void** mem_ctx) {
    int ret = -1;
    struct rkispp_fecbuf_info fecbuf_info;
    struct rkisp_meshbuf_info cacbuf_info;
    struct rkispp_fecbuf_size fecbuf_size;
    struct rkisp_meshbuf_size cacbuf_size;
    struct rkisp_info2ddr dbgbuf_info;

    uint8_t offset = id * ISP3X_MESH_BUF_NUM;

    AiqCamHwBase_t* isp20                    = container_of(ops_ctx, AiqCamHwBase_t, _drv_share_mem_ops);
    rk_aiq_share_mem_config_t* share_mem_cfg = (rk_aiq_share_mem_config_t*)config;

    aiqMutex_lock(&isp20->_mem_mutex);
    if (share_mem_cfg->mem_type == MEM_TYPE_LDCH || share_mem_cfg->mem_type == MEM_TYPE_LDCV ||
            share_mem_cfg->mem_type == MEM_TYPE_BTNR) {
        rk_aiq_ldch_share_mem_info_t* mem_info_array = NULL;
        int i                                        = 0;
        uint64_t module_id                           = 0;
#if defined(ISP_HW_V20) || defined(ISP_HW_V21)
        struct rkisp_meshbuf_size meshbuf_size;
        struct rkisp_meshbuf_info meshbuf_info;
        unsigned long cmd = RKISP_CMD_SET_meshbuf_size;

        meshbuf_size.meas_width  = share_mem_cfg->alloc_param.width;
        meshbuf_size.meas_height = share_mem_cfg->alloc_param.height;
#else
        struct rkisp_meshbuf_info meshbuf_info;
        struct rkisp_meshbuf_size meshbuf_size;
        unsigned long cmd = RKISP_CMD_SET_MESHBUF_SIZE;

        switch (share_mem_cfg->mem_type)
        {
        case MEM_TYPE_LDCH:
            module_id = ISP3X_MODULE_LDCH;
            mem_info_array = (rk_aiq_lut_share_mem_info_t*)(isp20->_ldch_drv_mem_ctx.mem_info);
            *mem_ctx = (void*)(&isp20->_ldch_drv_mem_ctx);
            break;
        case MEM_TYPE_LDCV:
            module_id = ISP39_MODULE_LDCV;
            mem_info_array = (rk_aiq_lut_share_mem_info_t*)(isp20->_ldcv_drv_mem_ctx.mem_info);
            *mem_ctx = (void*)(&isp20->_ldcv_drv_mem_ctx);
            break;
        case MEM_TYPE_BTNR:
            module_id = ISP35_MODULE_BAY3D;
            mem_info_array = (rk_aiq_lut_share_mem_info_t*)(isp20->_btnr_drv_mem_ctx.mem_info);
            *mem_ctx = (void*)(&isp20->_btnr_drv_mem_ctx);
            break;
        default:
            break;
        }

        meshbuf_size.unite_isp_id = id;
        meshbuf_size.module_id    = module_id;
        meshbuf_size.meas_width   = share_mem_cfg->alloc_param.width;
        meshbuf_size.meas_height  = share_mem_cfg->alloc_param.height;
        meshbuf_size.buf_cnt      = ISP2X_MESH_BUF_NUM;
#endif
        ret = isp20->mIspCoreDev->_v4l_base.io_control(&isp20->mIspCoreDev->_v4l_base, cmd,
                &meshbuf_size);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "alloc ldch buf failed!");
            *mem_ctx = NULL;
            aiqMutex_unlock(&isp20->_mem_mutex);
            return;
        }
        xcam_mem_clear(meshbuf_info);
#if defined(ISP_HW_V20) || defined(ISP_HW_V21)
        cmd = RKISP_CMD_GET_meshbuf_info;
#else
        meshbuf_info.unite_isp_id = id;
        meshbuf_info.module_id    = module_id;
        cmd                       = RKISP_CMD_GET_MESHBUF_INFO;
#endif
        ret = isp20->mIspCoreDev->_v4l_base.io_control(&isp20->mIspCoreDev->_v4l_base, cmd,
                &meshbuf_info);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to get ldch buf info!!");
            *mem_ctx = NULL;
            aiqMutex_unlock(&isp20->_mem_mutex);
            return;
        }

        struct isp2x_mesh_head* head = NULL;
        for (i = 0; i < ISP2X_MESH_BUF_NUM; i++) {
            mem_info_array[offset + i].map_addr =
                mmap(NULL, meshbuf_info.buf_size[i], PROT_READ | PROT_WRITE, MAP_SHARED,
                     meshbuf_info.buf_fd[i], 0);
            if (MAP_FAILED == mem_info_array[offset + i].map_addr)
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to map ldch buf!!");

            mem_info_array[offset + i].fd   = meshbuf_info.buf_fd[i];
            mem_info_array[offset + i].size = meshbuf_info.buf_size[i];
            head = (struct isp2x_mesh_head*)mem_info_array[offset + i].map_addr;
            mem_info_array[offset + i].addr =
                (void*)((char*)mem_info_array[offset + i].map_addr + head->data_oft);
            mem_info_array[offset + i].addr1 =
                (void*)((char*)mem_info_array[offset + i].map_addr + head->data1_oft);
            mem_info_array[offset + i].state = (char*)&head->stat;

            LOGD_ALDC("Get %s buf : fd %d, size %d from drv",
                      share_mem_cfg->mem_type == MEM_TYPE_LDCH ? "LDCH" : "LDCV",
                      meshbuf_info.buf_fd[i], meshbuf_info.buf_size[i]);

            LOGK_ANR("B3DLDC buf : fd %d, size %d, addr %p, addr1 %p, state %p data_oft %d data1_oft %d\n",
                     meshbuf_info.buf_fd[i], meshbuf_info.buf_size[i],
                     mem_info_array[offset + i].addr, mem_info_array[offset + i].addr1,
                     mem_info_array[offset + i].state, head->data_oft, head->data1_oft);
        }

    } else if (share_mem_cfg->mem_type == MEM_TYPE_CAC) {
        rk_aiq_cac_share_mem_info_t* mem_info_array = NULL;
        int i                                       = 0;
        cacbuf_size.unite_isp_id                    = id;
        cacbuf_size.module_id                       = ISP3X_MODULE_CAC;
        cacbuf_size.meas_width                      = share_mem_cfg->alloc_param.width;
        cacbuf_size.meas_height                     = share_mem_cfg->alloc_param.height;
        cacbuf_size.buf_cnt                         = share_mem_cfg->alloc_param.reserved[0];
        ret = isp20->mIspCoreDev->_v4l_base.io_control(&isp20->mIspCoreDev->_v4l_base,
                RKISP_CMD_SET_MESHBUF_SIZE, &cacbuf_size);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "alloc cac buf failed!");
            *mem_ctx = NULL;
            aiqMutex_unlock(&isp20->_mem_mutex);
            return;
        }
        xcam_mem_clear(cacbuf_info);
        cacbuf_info.unite_isp_id = id;
        cacbuf_info.module_id    = ISP3X_MODULE_CAC;
        ret = isp20->mIspCoreDev->_v4l_base.io_control(&isp20->mIspCoreDev->_v4l_base,
                RKISP_CMD_GET_MESHBUF_INFO, &cacbuf_info);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to get cac buf info!!");
            *mem_ctx = NULL;
            aiqMutex_unlock(&isp20->_mem_mutex);
            return;
        }
        mem_info_array = (rk_aiq_cac_share_mem_info_t*)(isp20->_cac_drv_mem_ctx.mem_info);
        struct isp2x_mesh_head* head = NULL;
        for (i = 0; i < cacbuf_size.buf_cnt; i++) {
            mem_info_array[offset + i].map_addr =
                mmap(NULL, cacbuf_info.buf_size[i], PROT_READ | PROT_WRITE, MAP_SHARED,
                     cacbuf_info.buf_fd[i], 0);
            if (MAP_FAILED == mem_info_array[offset + i].map_addr) {
                mem_info_array[offset + i].map_addr = NULL;
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to map cac buf!!");
                *mem_ctx = NULL;
                aiqMutex_unlock(&isp20->_mem_mutex);
                return;
            }

            mem_info_array[offset + i].fd   = cacbuf_info.buf_fd[i];
            mem_info_array[offset + i].size = cacbuf_info.buf_size[i];
            head = (struct isp2x_mesh_head*)mem_info_array[offset + i].map_addr;
            mem_info_array[offset + i].addr =
                (void*)((char*)mem_info_array[offset + i].map_addr + head->data_oft);
            mem_info_array[offset + i].state = (char*)&head->stat;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "Got CAC LUT fd %d for ISP %d",
                            mem_info_array[offset + i].fd, id);
        }
        *mem_ctx = (void*)(&isp20->_cac_drv_mem_ctx);
    } else if (share_mem_cfg->mem_type == MEM_TYPE_DBG_INFO) {
        rk_aiq_dbg_share_mem_info_t* mem_info_array = NULL;
        int i                                       = 0;
        id                                          = 0;
        offset                                      = id * RKISP_INFO2DDR_BUF_MAX;
        dbgbuf_info.wsize                           = share_mem_cfg->alloc_param.width;
        dbgbuf_info.vsize                           = share_mem_cfg->alloc_param.height;
        dbgbuf_info.owner = (enum rkisp_info2ddr_owner)(share_mem_cfg->alloc_param.reserved[0]);
        if (dbgbuf_info.owner == RKISP_INFO2DRR_OWNER_AWB) {
            dbgbuf_info.u.awb.awb2ddr_sel = share_mem_cfg->alloc_param.reserved[1];
        } else if (dbgbuf_info.owner == RKISP_INFO2DRR_OWNER_GAIN) {
            dbgbuf_info.u.gain.gain2ddr_mode = share_mem_cfg->alloc_param.reserved[1];
        } else {
            *mem_ctx = NULL;
            aiqMutex_unlock(&isp20->_mem_mutex);
            return;
        }
        dbgbuf_info.buf_cnt = share_mem_cfg->alloc_param.reserved[2];
        ret = isp20->mIspCoreDev->_v4l_base.io_control(&isp20->mIspCoreDev->_v4l_base,
                RKISP_CMD_INFO2DDR, &dbgbuf_info);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "alloc dbg buf failed!");
            *mem_ctx = NULL;
            aiqMutex_unlock(&isp20->_mem_mutex);
            return;
        }
        mem_info_array = (rk_aiq_dbg_share_mem_info_t*)(isp20->_dbg_drv_mem_ctx.mem_info);
        for (i = 0; i < dbgbuf_info.buf_cnt; i++) {
            mem_info_array[offset + i].map_addr =
                mmap(NULL, dbgbuf_info.wsize * dbgbuf_info.vsize, PROT_READ | PROT_WRITE,
                     MAP_SHARED, dbgbuf_info.buf_fd[i], 0);
            if (MAP_FAILED == mem_info_array[offset + i].map_addr) {
                mem_info_array[offset + i].map_addr = NULL;
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "failed to map dbg buf!!");
                *mem_ctx = NULL;
                aiqMutex_unlock(&isp20->_mem_mutex);
                return;
            }
            mem_info_array[offset + i].size = dbgbuf_info.wsize * dbgbuf_info.vsize;
            mem_info_array[offset + i].fd   = dbgbuf_info.buf_fd[i];
        }
        *mem_ctx = (void*)(&isp20->_dbg_drv_mem_ctx);
    }
    aiqMutex_unlock(&isp20->_mem_mutex);
}

static void _releaseMemResource(uint8_t id, void* mem_ctx) {
    int ret                          = -1;
    drv_share_mem_ctx_t* drv_mem_ctx = (drv_share_mem_ctx_t*)mem_ctx;
    AiqCamHwBase_t* isp20            = NULL;
    uint8_t offset                   = id * ISP3X_MESH_BUF_NUM;
    uint64_t module_id               = 0;

    if (mem_ctx == NULL) return;
    isp20 = (AiqCamHwBase_t*)(drv_mem_ctx->ops_ctx);

    aiqMutex_lock(&isp20->_mem_mutex);
    if (drv_mem_ctx->type == MEM_TYPE_LDCH || drv_mem_ctx->type == MEM_TYPE_LDCV ||
            drv_mem_ctx->type == MEM_TYPE_BTNR) {
        int i = 0;
        rk_aiq_ldch_share_mem_info_t* mem_info_array =
            (rk_aiq_ldch_share_mem_info_t*)(drv_mem_ctx->mem_info);
        for (i = 0; i < ISP2X_MESH_BUF_NUM; i++) {
            if (mem_info_array[offset + i].map_addr) {
                if (mem_info_array[offset + i].state &&
                        (MESH_BUF_CHIPINUSE != mem_info_array[offset + i].state[0])) {
                    mem_info_array[offset + i].state[0] = MESH_BUF_INIT;
                }
                ret = munmap(mem_info_array[offset + i].map_addr, mem_info_array[offset + i].size);
                if (ret < 0) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "munmap ldch buf info!!");
                mem_info_array[offset + i].map_addr = NULL;
            }
            if (mem_info_array[offset + i].fd > 0) {
                close(mem_info_array[offset + i].fd);
                mem_info_array[offset + i].fd = -1;
            }
        }

        switch (drv_mem_ctx->type)
        {
        case MEM_TYPE_LDCH:
            module_id = ISP3X_MODULE_LDCH;
            break;
        case MEM_TYPE_LDCV:
            module_id = ISP39_MODULE_LDCV;
            break;
        case MEM_TYPE_BTNR:
            module_id = ISP35_MODULE_BAY3D;
            break;
        default:
            module_id = 0;
            break;
        }
    } else if (drv_mem_ctx->type == MEM_TYPE_FEC) {
        int i = 0;
        rk_aiq_fec_share_mem_info_t* mem_info_array =
            (rk_aiq_fec_share_mem_info_t*)(drv_mem_ctx->mem_info);
        for (i = 0; i < FEC_MESH_BUF_NUM; i++) {
            if (mem_info_array[i].map_addr) {
                if (mem_info_array[i].state && (MESH_BUF_CHIPINUSE != mem_info_array[i].state[0])) {
                    mem_info_array[i].state[0] = MESH_BUF_INIT;
                }
                ret = munmap(mem_info_array[i].map_addr, mem_info_array[i].size);
                if (ret < 0) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "munmap fec buf info!!");
                mem_info_array[i].map_addr = NULL;
            }
            if (mem_info_array[i].fd > 0) {
                close(mem_info_array[i].fd);
                mem_info_array[i].fd = -1;
            }
        }
    } else if (drv_mem_ctx->type == MEM_TYPE_CAC) {
        int i = 0;
        rk_aiq_cac_share_mem_info_t* mem_info_array =
            (rk_aiq_cac_share_mem_info_t*)(drv_mem_ctx->mem_info);
        for (i = 0; i < ISP3X_MESH_BUF_NUM; i++) {
            if (mem_info_array[offset + i].map_addr) {
                if (mem_info_array[offset + i].state &&
                        (MESH_BUF_CHIPINUSE != mem_info_array[offset + i].state[0])) {
                    mem_info_array[offset + i].state[0] = MESH_BUF_INIT;
                }
                ret = munmap(mem_info_array[offset + i].map_addr, mem_info_array[offset + i].size);
                if (ret < 0) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "munmap cac buf info!!");
                mem_info_array[offset + i].map_addr = NULL;
            }
            if (mem_info_array[offset + i].fd > 0) {
                close(mem_info_array[offset + i].fd);
                mem_info_array[offset + i].fd = -1;
            }
        }
        module_id = ISP3X_MODULE_CAC;
    } else if (drv_mem_ctx->type == MEM_TYPE_DBG_INFO) {
        int i = 0;
        rk_aiq_dbg_share_mem_info_t* mem_info_array =
            (rk_aiq_dbg_share_mem_info_t*)(drv_mem_ctx->mem_info);
        struct rkisp_info2ddr info;
        if (mem_info_array == NULL) {
            aiqMutex_unlock(&isp20->_mem_mutex);
            return;
        }
        info.owner = RKISP_INFO2DRR_OWNER_NULL;
        ret        = isp20->mIspCoreDev->_v4l_base.io_control(&isp20->mIspCoreDev->_v4l_base,
                     RKISP_CMD_INFO2DDR, &info);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "alloc dbg buf failed!");
            aiqMutex_unlock(&isp20->_mem_mutex);
            return;
        }
        for (i = 0; i < RKISP_INFO2DDR_BUF_MAX; i++) {
            if (mem_info_array[offset + i].map_addr) {
                ret = munmap(mem_info_array[offset + i].map_addr, mem_info_array[offset + i].size);
                if (ret < 0)
                    LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                                    "%dth,munmap dbg buf info!! error:%s, map_addr:%p, size:%d", i,
                                    strerror(errno), mem_info_array[offset + i].map_addr,
                                    mem_info_array[offset + i].size);
                mem_info_array[offset + i].map_addr = NULL;
            }
            if (mem_info_array[offset + i].fd > 0) close(mem_info_array[offset + i].fd);
        }
    }
    if (module_id != 0) {
        ret = isp20->mIspCoreDev->_v4l_base.io_control(&isp20->mIspCoreDev->_v4l_base,
                RKISP_CMD_MESHBUF_FREE, &module_id);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "free dbg buf failed!");
        }
    }
    aiqMutex_unlock(&isp20->_mem_mutex);
}

static void* _getFreeItem(uint8_t id, void* mem_ctx) {
    unsigned int idx;
    int retry_cnt                    = 3;
    drv_share_mem_ctx_t* drv_mem_ctx = (drv_share_mem_ctx_t*)mem_ctx;
    AiqCamHwBase_t* isp20            = NULL;
    uint8_t offset                   = id * ISP3X_MESH_BUF_NUM;

    if (!mem_ctx || !drv_mem_ctx->ops_ctx) return NULL;

    isp20 = (AiqCamHwBase_t*)(drv_mem_ctx->ops_ctx);

    aiqMutex_lock(&isp20->_mem_mutex);
    if (drv_mem_ctx->type == MEM_TYPE_LDCH || drv_mem_ctx->type == MEM_TYPE_LDCV ||
            drv_mem_ctx->type == MEM_TYPE_BTNR) {
        rk_aiq_lut_share_mem_info_t* mem_info_array =
            (rk_aiq_lut_share_mem_info_t*)(drv_mem_ctx->mem_info);
        do {
            for (idx = 0; idx < ISP2X_MESH_BUF_NUM; idx++) {
                if (mem_info_array[offset + idx].map_addr) {
                    if (mem_info_array[offset + idx].state &&
                            (0 == mem_info_array[offset + idx].state[0])) {
                        aiqMutex_unlock(&isp20->_mem_mutex);
                        return (void*)&mem_info_array[offset + idx];
                    }
                }
            }
        } while (retry_cnt--);
    } else if (drv_mem_ctx->type == MEM_TYPE_FEC) {
        rk_aiq_fec_share_mem_info_t* mem_info_array =
            (rk_aiq_fec_share_mem_info_t*)(drv_mem_ctx->mem_info);
        if (mem_info_array == NULL) return NULL;
        do {
            for (idx = 0; idx < FEC_MESH_BUF_NUM; idx++) {
                if (mem_info_array[idx].map_addr) {
                    if (mem_info_array[idx].state && (0 == mem_info_array[idx].state[0])) {
                        aiqMutex_unlock(&isp20->_mem_mutex);
                        return (void*)&mem_info_array[idx];
                    }
                }
            }
        } while (retry_cnt--);
    } else if (drv_mem_ctx->type == MEM_TYPE_CAC) {
        rk_aiq_cac_share_mem_info_t* mem_info_array =
            (rk_aiq_cac_share_mem_info_t*)(drv_mem_ctx->mem_info);
        if (mem_info_array == NULL) return NULL;
        do {
            for (idx = 0; idx < ISP3X_MESH_BUF_NUM; idx++) {
                if (mem_info_array[offset + idx].map_addr != NULL) {
                    if (-1 != mem_info_array[offset + idx].fd) {
                        if (mem_info_array[offset + idx].state &&
                                (0 == mem_info_array[offset + idx].state[0])) {
                            aiqMutex_unlock(&isp20->_mem_mutex);
                            return (void*)&mem_info_array[offset + idx];
                        }
                    }
                }
            }
        } while (retry_cnt--);
    } else if (drv_mem_ctx->type == MEM_TYPE_DBG_INFO) {
        rk_aiq_dbg_share_mem_info_t* mem_info_array =
            (rk_aiq_dbg_share_mem_info_t*)(drv_mem_ctx->mem_info);
        if (mem_info_array == NULL) return NULL;
        do {
            for (idx = 0; idx < RKISP_INFO2DDR_BUF_MAX; idx++) {
                if (mem_info_array[offset + idx].map_addr) {
                    uint32_t state = *(uint32_t*)(mem_info_array[offset + idx].map_addr);
                    if (state == RKISP_INFO2DDR_BUF_INIT) {
                        aiqMutex_unlock(&isp20->_mem_mutex);
                        return (void*)&mem_info_array[offset + idx];
                    }
                }
            }
        } while (retry_cnt--);
    }
    aiqMutex_unlock(&isp20->_mem_mutex);
    return NULL;
}

void AiqCamHw_getShareMemOps(AiqCamHwBase_t* pCamHw, isp_drv_share_mem_ops_t** mem_ops) {
    pCamHw->_drv_share_mem_ops.alloc_mem     = (alloc_mem_t)&_allocMemResource;
    pCamHw->_drv_share_mem_ops.release_mem   = (release_mem_t)&_releaseMemResource;
    pCamHw->_drv_share_mem_ops.get_free_item = (get_free_item_t)&_getFreeItem;
    *mem_ops                                 = &pCamHw->_drv_share_mem_ops;
}

void AiqCamHw_make_ispHwEvt(AiqCamHwBase_t* pCamHw, Aiqisp20Evt_t* pEvt, uint32_t sequence,
                            int type, int64_t timestamp) {
    if (type == V4L2_EVENT_FRAME_SYNC) {
        pEvt->_base.type       = type;
        pEvt->_base.frame_id   = sequence;
        pEvt->_expDelay        = pCamHw->_exp_delay;
        pEvt->_base.mTimestamp = timestamp;
        pEvt->mCamHw           = pCamHw;
    }
}

XCamReturn AiqCamHw_handleIspRstList(AiqCamHwBase_t* pCamHw, AiqList_t* pList) {
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pCamHw->_is_exit) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "set 3a config bypass since ia engine has stop");
        return XCAM_RETURN_BYPASS;
    }

    bool restart = false;
    if (pCamHw->_state == CAM_HW_STATE_PREPARED || pCamHw->_state == CAM_HW_STATE_STOPPED ||
            pCamHw->_state == CAM_HW_STATE_PAUSED) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "hdr-debug: %s: first set ispparams\n", __func__);
        if (!AiqV4l2Device_isActivated(pCamHw->mIspParamsDev)) {

            if (!AiqV4l2Device_isActivated(pCamHw->mIspStatsDev)) {
                ret = pCamHw->mIspStatsDev->start(pCamHw->mIspStatsDev, false);
                if (ret < 0) {
                    LOGE_CAMHW_SUBM(ISP20HW_SUBM, "prepare isp stats dev err: %d\n", ret);
                }
            }

            ret = pCamHw->mIspParamsDev->start(pCamHw->mIspParamsDev, false);
            if (ret < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "prepare isp params dev err: %d\n", ret);
            }

//#if defined(RKAIQ_HAVE_MULTIISP)
            if (/*g_mIsMultiIspMode && pCamHw->mParamsSplitter*/true) {
                // dequeue param buffer to judge whether unite isp is splitted into two sides or not.
                AiqV4l2Buffer_t* pV4l2Buf    = NULL;
                int              buf_length  = 0;
                int              struct_size = 0;
                int              isp_unite   = 0;
                int              buf_cnt     = 0;
                pV4l2Buf = AiqV4l2Device_getBuf(pCamHw->mIspParamsDev, -1);
                if (!pV4l2Buf) {
                    LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Can not get isp params buffer, queued cnts:%d \n",
                                    AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev));
                } else {
                    buf_length = pV4l2Buf->_buf.length;
                }
                if (pCamHw->mParamsSplitter)
                    isp_unite = AiqIspParamsSplitter_GetIspUniteMode(pCamHw->mParamsSplitter);
                if (isp_unite == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
                    buf_cnt = 2;
                } else if (isp_unite == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                    buf_cnt = 4;
                } else {
                    buf_cnt = 1;
                }
#if defined(ISP_HW_V33)
                struct_size = sizeof(struct isp33_isp_params_cfg);
#elif defined(ISP_HW_V35)
                struct_size = sizeof(struct isp35_isp_params_cfg);
#elif defined(ISP_HW_V39)
                struct_size = sizeof(struct isp39_isp_params_cfg);
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
                struct_size = sizeof(struct isp32_isp_params_cfg);
#elif defined(ISP_HW_V30)
                struct_size = sizeof(struct isp3x_isp_params_cfg);
#elif defined(ISP_HW_V21)
                struct_size = sizeof(struct isp21_isp_params_cfg);
#endif
                if (struct_size && buf_length && (buf_length / struct_size) != buf_cnt) {
                    LOGE_CAMHW("isp param buf length %d, isp param struct size %d", buf_length, struct_size);
                    LOGE_CAMHW("isp size error!!!!! raw size is %s than expect size",
                               (buf_length / struct_size) > buf_cnt ? "largger" : "smaller");
                }

                if (pV4l2Buf) {
                    AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
                }

            }
//#endif
            ret = _hdr_mipi_prepare_mode(pCamHw, pCamHw->_hdr_mode);
            if (ret < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "hdr mipi start err: %d\n", ret);
            }
        }

        AiqListItem_t* pItem = NULL;
        bool rm              = false;
        AIQ_LIST_FOREACH(pList, pItem, rm) {
            aiq_params_base_t* params = *(aiq_params_base_t**)(pItem->_pData);
            params->frame_id          = 0;
        }
        restart = true;
    }

    aiqMutex_lock(&pCamHw->_stop_cond_mutex);
    if (pCamHw->_isp_stream_status != ISP_STREAM_STATUS_STREAM_OFF || restart) {
        ret = _setIspConfig(pCamHw, pList);
    }
    aiqMutex_unlock(&pCamHw->_stop_cond_mutex);
    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqCamHw_handleIspRst(AiqCamHwBase_t* pCamHw, aiq_params_base_t* result) {
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pCamHw->_is_exit) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "set 3a config bypass since ia engine has stop");
        return XCAM_RETURN_BYPASS;
    }

    if (pCamHw->_state == CAM_HW_STATE_PREPARED || pCamHw->_state == CAM_HW_STATE_STOPPED ||
            pCamHw->_state == CAM_HW_STATE_PAUSED) {
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "hdr-debug: %s: first set ispparams id[%d]\n", __func__,
                        result->frame_id);
        if (!AiqV4l2Device_isActivated(pCamHw->mIspParamsDev)) {
            ret = pCamHw->mIspParamsDev->start(pCamHw->mIspParamsDev, false);
            if (ret < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "prepare isp params dev err: %d\n", ret);
            }

            ret = _hdr_mipi_prepare_mode(pCamHw, pCamHw->_hdr_mode);
            if (ret < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "hdr mipi start err: %d\n", ret);
            }
        }
        // for aiq restart without re-prepare
        //_init_bay3dbuf(pCamHw);
    }

    // FIXME: to process the relevant logic code
#if 0
    aiqMutex_lock(&pCamHw->_stop_cond_mutex);
    if (pCamHw->_isp_stream_status != ISP_STREAM_STATUS_STREAM_OFF) {
        ret = _setIspConfig(pCamHw, result);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "setIspConfig failed !");
        }
    }
    aiqMutex_unlock(&pCamHw->_stop_cond_mutex);
#endif

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _setCifSclStartFlag(AiqCamHwBase_t* pCamHw, int ratio, bool mode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mode == AiqCifSclStream_getIsActive(pCamHw->mCifScaleStream)) {
        return ret;
    }

    SnsFullInfoWraps_t* pSnsInfoWrap = g_mSensorHwInfos;
    while (pSnsInfoWrap) {
        if (strcmp(pCamHw->sns_name, pSnsInfoWrap->key) == 0) break;
        pSnsInfoWrap = pSnsInfoWrap->next;
    }

    if (!pSnsInfoWrap) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", pCamHw->sns_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    rk_sensor_full_info_t *s_info = &pSnsInfoWrap->data;
    ret = AiqCifSclStream_restart(pCamHw->mCifScaleStream, s_info, ratio, &pCamHw->mPollCb, mode);
    return ret;
}

static XCamReturn _dispatchResult(AiqCamHwBase_t* pCamHw, aiq_params_base_t* params) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    LOG1("%s enter, msg type(0x%x)", __FUNCTION__, params->type);
    switch (params->type) {
    case RESULT_TYPE_EXPOSURE_PARAM: {
        AiqAecExpInfoWrapper_t* exp = (AiqAecExpInfoWrapper_t*)(params->_data);
        ret                         = AiqCamHw_setExposureParams(pCamHw, exp);
        if (ret)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "setExposureParams error %d id %d", ret,
                            params->frame_id);
        break;
    }
    case RESULT_TYPE_FOCUS_PARAM: {
        rk_aiq_focus_params_t* focus = (rk_aiq_focus_params_t*)(params->_data);
        ret                          = _setFocusParams(pCamHw, focus);
        if (ret) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "setFocusParams error %d", ret);
        break;
    }
    case RESULT_TYPE_IRIS_PARAM: {
        AiqIrisInfoWrapper_t* iris = (AiqIrisInfoWrapper_t*)(params->_data);
        ret = _setIrisParams(pCamHw, iris, pCamHw->_cur_calib_infos.aec.IrisType);
        if (ret) LOGE_CAMHW_SUBM(ISP20HW_SUBM, "setIrisParams error %d", ret);
        ret = _getIrisParams(pCamHw, iris, pCamHw->_cur_calib_infos.aec.IrisType);
        if (ret) LOGE_ANALYZER("getIrisParams error %d", ret);
        break;
    }
    case RESULT_TYPE_AFD_PARAM: {
        rk_aiq_isp_afd_t* afd = (rk_aiq_isp_afd_t*)(params->_data);
        if (pCamHw->mCifScaleStream) {
            bool enable = afd->enable;
            int ratio   = afd->ratio;
            _setCifSclStartFlag(pCamHw, ratio, enable);
        }
        break;
    }
    default:
        LOGE("unknown param type(0x%x)!", params->type);
        break;
    }
    return ret;
    LOGD("%s exit", __FUNCTION__);
}

static XCamReturn _dispatchResultList(AiqCamHwBase_t* pCamHw, AiqList_t* pList) {
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(pList, pItem, rm) {
        aiq_params_base_t* params = *(aiq_params_base_t**)(pItem->_pData);
        switch (params->type) {
        case RESULT_TYPE_EXPOSURE_PARAM:
        case RESULT_TYPE_FOCUS_PARAM:
        case RESULT_TYPE_IRIS_PARAM:
        case RESULT_TYPE_AFD_PARAM:
            _dispatchResult(pCamHw, params);
            pItem = aiqList_erase_item_locked(pList, pItem);
            rm    = true;
            break;
        default:
            break;
        }
    }
    if (aiqList_size(pList) > 0) {
        AiqCamHw_handleIspRstList(pCamHw, pList);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamHw_applyAnalyzerResult(AiqCamHwBase_t* pCamHw, aiq_params_base_t* base,
                                        bool sync) {
    return _dispatchResult(pCamHw, base);
}

XCamReturn AiqCamHw_applyAnalyzerResultList(AiqCamHwBase_t* pCamHw, AiqList_t* pList) {
    return _dispatchResultList(pCamHw, pList);
}

XCamReturn AiqCamHw_notify_sof(AiqCamHwBase_t* pCamHw, AiqHwEvt_t* sof_evt) {
    AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
    AiqLensHw_t* mLensSubdev     = pCamHw->mLensDev;

    mSensorSubdev->handle_sof(mSensorSubdev, sof_evt->mTimestamp, sof_evt->frame_id);
    if (mLensSubdev) AiqLensHw_handle_sof(mLensSubdev, sof_evt->mTimestamp, sof_evt->frame_id);
#if RKAIQ_HAVE_PDAF
    if (pCamHw->mPdafStreamUnit) AiqPdafStreamProcUnit_notify_sof(pCamHw->mPdafStreamUnit);
#endif
#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcEn && pCamHw->mMemcStream) AiqMemcManager_pushSofTime(pCamHw->mMemcStream, sof_evt->frame_id, sof_evt->mTimestamp);
#endif
    return XCAM_RETURN_NO_ERROR;
}

#if defined(ISP_HW_V39) || defined(ISP_HW_V35) || defined(ISP_HW_V33)
static void _setIspExp(AiqCamHwBase_t* pCamHw, struct sensor_exposure_cfg* pExpCfg, AiqSensorExpInfo_t* ae_exp)
{
    rk_aiq_exposure_sensor_descriptor* pSnsDes = &pCamHw->_mSensorDev->_sensor_desc;
    if (pSnsDes->vt_pix_clk_freq_hz == 0 || pSnsDes->sensor_output_height == 0) {
        LOGV_CAMHW_SUBM(ISP20HW_SUBM, "sensor info is not ready, skip set exposure");
        return;
    }

    uint64_t nano_seconds_per_second = 1000000000ULL;
    uint32_t rolling_shutter_skew =
        pSnsDes->line_length_pck * nano_seconds_per_second /
        pSnsDes->vt_pix_clk_freq_hz * pSnsDes->sensor_output_height / 1000;
    if (RK_AIQ_HDR_GET_WORKING_MODE(pCamHw->_hdr_mode) == RK_AIQ_WORKING_MODE_NORMAL) {
        pExpCfg->linear_exp.fine_integration_time = 0;
        pExpCfg->linear_exp.coarse_integration_time = ae_exp->aecExpInfo.LinearExp.exp_real_params.integration_time * 1000 * 1000; // us
        pExpCfg->linear_exp.analog_gain_code_global = ae_exp->aecExpInfo.LinearExp.exp_real_params.analog_gain * 1000;
        pExpCfg->linear_exp.digital_gain_global = ae_exp->aecExpInfo.LinearExp.exp_real_params.digital_gain * 1000;;
        pExpCfg->linear_exp.isp_digital_gain = ae_exp->aecExpInfo.LinearExp.exp_real_params.isp_dgain * 1000;
        pExpCfg->linear_exp.rolling_shutter_skew = rolling_shutter_skew;
    } else if (RK_AIQ_HDR_GET_WORKING_MODE(pCamHw->_hdr_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
        pExpCfg->hdr_exp[0].fine_integration_time = 0;
        pExpCfg->hdr_exp[0].coarse_integration_time = ae_exp->aecExpInfo.HdrExp[0].exp_real_params.integration_time * 1000 * 1000; // us
        pExpCfg->hdr_exp[0].analog_gain_code_global = ae_exp->aecExpInfo.HdrExp[0].exp_real_params.analog_gain * 1000;
        pExpCfg->hdr_exp[0].digital_gain_global = ae_exp->aecExpInfo.HdrExp[0].exp_real_params.digital_gain * 1000;;
        pExpCfg->hdr_exp[0].isp_digital_gain = ae_exp->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain * 1000;
        pExpCfg->hdr_exp[0].rolling_shutter_skew = rolling_shutter_skew;
        pExpCfg->hdr_exp[1].fine_integration_time = 0;
        pExpCfg->hdr_exp[1].coarse_integration_time = ae_exp->aecExpInfo.HdrExp[1].exp_real_params.integration_time * 1000 * 1000; // us
        pExpCfg->hdr_exp[1].analog_gain_code_global = ae_exp->aecExpInfo.HdrExp[1].exp_real_params.analog_gain * 1000;
        pExpCfg->hdr_exp[1].digital_gain_global = ae_exp->aecExpInfo.HdrExp[1].exp_real_params.digital_gain * 1000;;
        pExpCfg->hdr_exp[1].isp_digital_gain = ae_exp->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain * 1000;
        pExpCfg->hdr_exp[1].rolling_shutter_skew = rolling_shutter_skew;
    } else if (RK_AIQ_HDR_GET_WORKING_MODE(pCamHw->_hdr_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
        pExpCfg->hdr_exp[0].fine_integration_time = 0;
        pExpCfg->hdr_exp[0].coarse_integration_time = ae_exp->aecExpInfo.HdrExp[0].exp_real_params.integration_time * 1000 * 1000; // us
        pExpCfg->hdr_exp[0].analog_gain_code_global = ae_exp->aecExpInfo.HdrExp[0].exp_real_params.analog_gain * 1000;
        pExpCfg->hdr_exp[0].digital_gain_global = ae_exp->aecExpInfo.HdrExp[0].exp_real_params.digital_gain * 1000;;
        pExpCfg->hdr_exp[0].isp_digital_gain = ae_exp->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain * 1000;
        pExpCfg->hdr_exp[0].rolling_shutter_skew = rolling_shutter_skew;
        pExpCfg->hdr_exp[1].fine_integration_time = 0;
        pExpCfg->hdr_exp[1].coarse_integration_time = ae_exp->aecExpInfo.HdrExp[1].exp_real_params.integration_time * 1000 * 1000; // us
        pExpCfg->hdr_exp[1].analog_gain_code_global = ae_exp->aecExpInfo.HdrExp[1].exp_real_params.analog_gain * 1000;
        pExpCfg->hdr_exp[1].digital_gain_global = ae_exp->aecExpInfo.HdrExp[1].exp_real_params.digital_gain * 1000;;
        pExpCfg->hdr_exp[1].isp_digital_gain = ae_exp->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain * 1000;
        pExpCfg->hdr_exp[1].rolling_shutter_skew = rolling_shutter_skew;
        pExpCfg->hdr_exp[2].fine_integration_time = 0;
        pExpCfg->hdr_exp[2].coarse_integration_time = ae_exp->aecExpInfo.HdrExp[2].exp_real_params.integration_time * 1000 * 1000; // us
        pExpCfg->hdr_exp[2].analog_gain_code_global = ae_exp->aecExpInfo.HdrExp[2].exp_real_params.analog_gain * 1000;
        pExpCfg->hdr_exp[2].digital_gain_global = ae_exp->aecExpInfo.HdrExp[2].exp_real_params.digital_gain * 1000;;
        pExpCfg->hdr_exp[2].isp_digital_gain = ae_exp->aecExpInfo.HdrExp[2].exp_real_params.isp_dgain * 1000;
        pExpCfg->hdr_exp[2].rolling_shutter_skew = rolling_shutter_skew;
    }

}
#endif

static XCamReturn _setIspConfig(AiqCamHwBase_t* pCamHw, AiqList_t* result_list) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();

    AiqV4l2Buffer_t* pV4l2Buf = NULL;
    uint32_t frameId          = -1;
    int ispUniteMode          = 0;

    aiqMutex_lock(&pCamHw->_isp_params_cfg_mutex);
    while (aiqMap_size(pCamHw->_effecting_ispparam_map) > (CAMHWISP_EFFECT_ISP_POOL_NUM - 2)) {
        AiqMapItem_t* pItem             = aiqMap_begin(pCamHw->_effecting_ispparam_map);
        aiq_isp_effect_params_t* params = *(aiq_isp_effect_params_t**)(pItem->_pData);
        aiqMap_erase(pCamHw->_effecting_ispparam_map, pItem->_key);
        AIQ_REF_BASE_UNREF(&params->_ref_base);
    }
    aiqMutex_unlock(&pCamHw->_isp_params_cfg_mutex);

    if (pCamHw->mIspParamsDev) {
retry:
        pV4l2Buf = AiqV4l2Device_getBuf(pCamHw->mIspParamsDev, -1);
        if (!pV4l2Buf) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Can not get isp params buffer, queued cnts:%d \n",
                            AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev));
            if (pCamHw->mIspParamsDev->poll_event(pCamHw->mIspParamsDev, 100, -1) <= 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "poll params error, queue cnts: %d !",
                                AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev));
            } else {
                AiqV4l2Buffer_t* dqbuf = pCamHw->mIspParamsDev->_dequeue_buffer(pCamHw->mIspParamsDev);
                if (!dqbuf) {
                    XCAM_LOG_ERROR("dequeue buffer failed");
                    return ret;
                }
                AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, dqbuf);
                LOGK_CAMHW_SUBM(ISP20HW_SUBM, "retry to get isp params buf !");
                goto retry;
            }
            return XCAM_RETURN_ERROR_PARAM;
        }
    } else
        return XCAM_RETURN_BYPASS;

    AiqList_t* ready_results   = result_list;
    AiqListItem_t* pItem       = aiqList_get_item(ready_results, NULL);
    aiq_params_base_t* pParams = *(aiq_params_base_t**)(pItem->_pData);
    frameId                    = pParams->frame_id;
    LOGD_CAMHW("----------%s, cam%d start config id(%d)'s isp params", __FUNCTION__,
               pCamHw->mCamPhyId, frameId);

    // add isp dgain results to ready results
    AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
    AiqSensorExpInfo_t* expParam = NULL;
    if (mSensorSubdev) {
        expParam = mSensorSubdev->getEffectiveExpParams(mSensorSubdev, frameId);

        if (!expParam) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "frame_id(%d), get exposure failed!!!\n", frameId);
        } else {
            expParam->_base.type = RESULT_TYPE_EXPOSURE_PARAM;
            aiqList_push(ready_results, &expParam);
        }
    }

    if (pV4l2Buf) {
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        struct isp32_isp_params_cfg* isp_params =
            (struct isp32_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
#elif defined(ISP_HW_V30)
        struct isp3x_isp_params_cfg* isp_params =
            (struct isp3x_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
#elif defined(ISP_HW_V21)
        struct isp21_isp_params_cfg* isp_params =
            (struct isp21_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
#elif defined(ISP_HW_V39)
        struct isp39_isp_params_cfg* isp_params =
            (struct isp39_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
#elif defined(ISP_HW_V33)
        struct isp33_isp_params_cfg* isp_params =
            (struct isp33_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
#elif defined(ISP_HW_V35)
        struct isp35_isp_params_cfg* isp_params =
            (struct isp35_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
#else
        struct isp2x_isp_params_cfg* isp_params =
            (struct isp2x_isp_params_cfg*)(AiqV4l2Buffer_getBuf(pV4l2Buf)->m.userptr);
#endif
        int buf_index      = AiqV4l2Buffer_getBuf(pV4l2Buf)->index;
        bool isMultiIsp    = g_mIsMultiIspMode;
        bool extened_pixel = g_mMultiIspExtendedPixel;
        ispUniteMode       = AiqCamHw_getIspUniteMode(pCamHw);

#if defined(ISP_HW_V39) || defined(ISP_HW_V35) || defined(ISP_HW_V33)
        if (expParam)
            _setIspExp(pCamHw, &isp_params->exposure, expParam);
#endif
        isp_params->module_en_update  = 0;
        isp_params->module_cfg_update = 0;
        isp_params->module_ens = 0;
        XCamReturn ret_b =
            AiqIspParamsCvt_merge_isp_results(pCamHw->_mIspParamsCvt, ready_results, isp_params,
                                              !!(g_mIsMultiIspMode && ispUniteMode), pCamHw->use_aiisp,
                                              pCamHw->_airms_en, pCamHw->_aiynr_en);
        if (ret_b != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "ISP parameter translation error\n");

        if (isp_params->module_cfg_update == 0 && isp_params->module_en_update == 0) {
            AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "fid:%d no new ISP parameters to drv", frameId);
            return ret;
        }

        isp_params->module_cfg_update |= ISP32_MODULE_FORCE;

        // TODO: isp driver has bug now, lsc cfg_up should be set along with
        // en_up
        if (isp_params->module_cfg_update & ISP2X_MODULE_LSC)
            isp_params->module_en_update |= ISP2X_MODULE_LSC;

        isp_params->frame_id = frameId;
        AiqV4l2Buffer_setSequence(pV4l2Buf, frameId);

        // should be before SplitIspParams, restrictions must be applied
        // to all ISPs
        AiqCamHw_process_restriction(pCamHw, isp_params);

#if defined(ISP_HW_V30)
        if(pCamHw->IsMultiIspMode == false) {
            pCamHw->_mIspParamsCvt->fixedAwbOveflowToIsp3xParams(pCamHw->_mIspParamsCvt, isp_params,
                    pCamHw->IsMultiIspMode);
        }
#endif

#if defined(RKAIQ_HAVE_MULTIISP) && defined(ISP_HW_V30)
        struct isp3x_isp_params_cfg ori_params;
        if (g_mIsMultiIspMode && ispUniteMode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
            ori_params = *isp_params;
            pCamHw->mParamsSplitter->SplitIspParams(pCamHw->mParamsSplitter, &ori_params,
                                                    isp_params);
            pCamHw->_mIspParamsCvt->fixedAwbOveflowToIsp3xParams(pCamHw->_mIspParamsCvt, isp_params,
                    pCamHw->IsMultiIspMode);
        }
#endif

#if defined(RKAIQ_HAVE_MULTIISP) && (defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE))
        struct isp32_isp_params_cfg ori_params;
        if (g_mIsMultiIspMode) {
            ori_params = *isp_params;
            if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_FOUR_GRID) {
                pCamHw->mParamsSplitter->SplitIspParamsVertical(pCamHw->mParamsSplitter, &ori_params,
                        isp_params);
            } else if (ispUniteMode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
                pCamHw->mParamsSplitter->SplitIspParams(pCamHw->mParamsSplitter, &ori_params,
                                                        isp_params);
            }
        }
#endif

#if defined(RKAIQ_HAVE_MULTIISP) && defined(ISP_HW_V39)
        struct isp39_isp_params_cfg ori_params;
        if (g_mIsMultiIspMode && ispUniteMode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
            ori_params = *isp_params;
            pCamHw->mParamsSplitter->SplitIspParams(pCamHw->mParamsSplitter, &ori_params,
                                                    isp_params);
        }
#endif

#if defined(RKAIQ_HAVE_MULTIISP) && defined(ISP_HW_V33)
        struct isp33_isp_params_cfg ori_params;
        if (g_mIsMultiIspMode && ispUniteMode == RK_AIQ_ISP_UNITE_MODE_TWO_GRID) {
            ori_params = *isp_params;
            pCamHw->mParamsSplitter->SplitIspParams(pCamHw->mParamsSplitter, &ori_params,
                                                    isp_params);
        }
#endif

#if defined(RKAIQ_HAVE_MULTIISP) && defined(ISP_HW_V35)
        struct isp35_isp_params_cfg ori_params;
        if (g_mIsMultiIspMode && ispUniteMode != RK_AIQ_ISP_UNITE_MODE_NORMAL) {
            ori_params = *isp_params;
            pCamHw->mParamsSplitter->SplitIspParams(pCamHw->mParamsSplitter, &ori_params,
                                                    isp_params);
        }
#endif

#if defined(RKAIQ_ENABLE_SPSTREAM)
        if (pCamHw->_mIspParamsCvt->mAfParams) {
            rk_aiq_af_algo_meas_t* afParams =
                (rk_aiq_af_algo_meas_t*)(pCamHw->_mIspParamsCvt->mAfParams->_data);
            if (pCamHw->mSpStreamUnit) {
                AiqSPStreamProcUnit_upd_af_meas(pCamHw->mSpStreamUnit, afParams);
            }
        }
#endif

#if RKAIQ_HAVE_MEMC
        if (pCamHw->mMemcEn) {
            bool btnr_en = isp_params->module_ens & ISP3X_MODULE_BAY3D;
            if(btnr_en)
                _init_bay3dbuf(pCamHw, isp_params);

            AiqMemcManager_send_ispparam(pCamHw->mMemcStream, pV4l2Buf);
            return XCAM_RETURN_NO_ERROR;
        }
#endif

        if (pCamHw->updateEffParams) {
#if defined(RKAIQ_HAVE_MULTIISP) && (defined(ISP_HW_V30) || defined(ISP_HW_V32) || \
                defined(ISP_HW_V32_LITE) || defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35))
            if (g_mIsMultiIspMode)
                pCamHw->updateEffParams(pCamHw, isp_params, &ori_params);
            else
                pCamHw->updateEffParams(pCamHw, isp_params, NULL);
#else
            pCamHw->updateEffParams(pCamHw, isp_params, NULL);
#endif
        }
        bool is_wait_params_done = false;
        if (pCamHw->mTbInfo.is_fastboot) {
            // skip the params
            if (pCamHw->processTb && pCamHw->processTb(pCamHw, isp_params)) {
                AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
                return XCAM_RETURN_NO_ERROR;
            }
            if (pCamHw->mAweekId == frameId) is_wait_params_done = true;
        }
        if (pCamHw->mAweekId == frameId) {
            isp_params->module_cfg_update |= ISP32_MODULE_RTT_FST;
        }

        uint64_t oldEns = pCamHw->_isp_module_ens;
        // assume the max valid bit is 60
        for (int i = 0; i < 60; i++) {
            if (isp_params->module_en_update & (1ULL << i)) {
                if (isp_params->module_ens & (1ULL << i))
                    pCamHw->_isp_module_ens |= (1ULL << i);
                else
                    pCamHw->_isp_module_ens &= ~(1ULL << i);
            }
        }
        if (oldEns == pCamHw->_isp_module_ens && pCamHw->_state == CAM_HW_STATE_STARTED)
            isp_params->module_en_update = 0;

#if defined(RKAIQ_HAVE_MULTIISP)
        // sync ispparam 0 module_en_update to ispparam 1 module_en_update
        if (g_mIsMultiIspMode) {
            if (isp_params->module_en_update != (isp_params + 1)->module_en_update) {
                (isp_params + 1)->module_en_update = isp_params->module_en_update;
            }
        }
#endif
        // should be called after params generated.
        // because iir_raw_fmt is decided by aiisp_en, isp_mode,
        // btnr_ldc_en,btnr_pixDomain
        bool btnr_en = isp_params->module_ens & ISP3X_MODULE_BAY3D;
        if(btnr_en)
            _init_bay3dbuf(pCamHw, isp_params);

#if RKAIQ_HAVE_AIBNR
        // should be called after isp buffer is init
        if (pCamHw->_mIspParamsCvt->mAibnrParams) {
            rk_aiq_isp_aibnr_params_t* isp_aibnr_param =
                (rk_aiq_isp_aibnr_params_t*)(pCamHw->_mIspParamsCvt->mAibnrParams->_data);
            if (pCamHw->mAibnrManager && (isp_params->module_cfg_update & ISP35_MODULE_AI)) {
                AibnrManager_updateParams(pCamHw->mAibnrManager, isp_aibnr_param, frameId);
            }
        }
#endif

#if RKAIQ_HAVE_AIYNR
        if (pCamHw->_aiynr_en) {
            // should be called after isp buffer is init
            if (pCamHw->_mIspParamsCvt->mAiynrParams) {
                rk_aiq_isp_aiynr_params_t* isp_aiynr_param =
                    (rk_aiq_isp_aiynr_params_t*)(pCamHw->_mIspParamsCvt->mAiynrParams->_data);
                if (pCamHw->mAiynrManager && (isp_params->module_cfg_update & ISP35_MODULE_AI)) {
                    AiynrManager_updateParams(pCamHw->mAiynrManager, isp_aiynr_param, frameId);
                }
            }
        }
#endif

        if (AiqV4l2Device_qbuf(pCamHw->mIspParamsDev, pV4l2Buf, true) != 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                            "RKISP1: failed to ioctl VIDIOC_QBUF for index %d, %d %s.\n", buf_index,
                            errno, strerror(errno));
            AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, pV4l2Buf);
            return XCAM_RETURN_ERROR_IOCTL;
        }

        int timeout = is_wait_params_done ? 100 : 1;
        uint32_t buf_counts = AiqV4l2Device_getBufCnt(pCamHw->mIspParamsDev);
        uint32_t try_time   = 3;
        while (AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev) > (buf_counts - 2) || is_wait_params_done) {
            if (pCamHw->mIspParamsDev->poll_event(pCamHw->mIspParamsDev, timeout, -1) <= 0) {
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "poll params error, queue cnts: %d !",
                                AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev));
                if (AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev) == buf_counts &&
                        try_time > 0) {
                    timeout = 30;
                    try_time--;
                    continue;
                } else
                    break;
            }
            AiqV4l2Buffer_t* dqbuf = pCamHw->mIspParamsDev->_dequeue_buffer(pCamHw->mIspParamsDev);
            if (!dqbuf) {
                XCAM_LOG_WARNING("dequeue buffer failed");
                // return ret;
            } else {
                if (is_wait_params_done) {
                    is_wait_params_done = false;
                    AiqHwEvt_t evt;
                    evt.type = ISP_POLL_PARAMS;
                    LOGK_CAMHW("<TB> poll param id:%d, call err_cb", frameId);
                    if (pCamHw->_hwResListener.hwResCb)
                        pCamHw->_hwResListener.hwResCb(pCamHw->_hwResListener._pCtx, &evt);
                }
                AiqV4l2Device_returnBufToPool(pCamHw->mIspParamsDev, dqbuf);
            }
        }

        pCamHw->_curIspParamsSeq = frameId;
        if (frameId == 0) {
            AiqSensorHw_t* sensorHw = pCamHw->_mSensorDev;
            // set initial blc/wb gain after isp params applied
            sensorHw->set_exposure_mode(sensorHw, -1);
        }
        LOGD_CAMHW_SUBM(
            ISP20HW_SUBM,
            "Config id(%u)'s isp params, full_en 0x%llx ens 0x%llx ens_up 0x%llx, cfg_up 0x%llx", isp_params->frame_id,
            pCamHw->_isp_module_ens,
            isp_params->module_ens,
            isp_params->module_en_update,
            isp_params->module_cfg_update);
        LOGD_CAMHW_SUBM(
            ISP20HW_SUBM,
            "device(%s) queue buffer index %d, queue cnt %d, check exit status again[exit: %d]",
            XCAM_STR(AiqV4l2Device_getDevName(pCamHw->mIspParamsDev)), buf_index,
            AiqV4l2Device_getQueuedBufCnt(pCamHw->mIspParamsDev), pCamHw->_is_exit);
        if (pCamHw->_is_exit) return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static aiq_params_base_t* _get_3a_module_result(AiqList_t* results, int32_t type) {
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(results, pItem, rm) {
        aiq_params_base_t* base = *(aiq_params_base_t**)(pItem->_pData);
        if (base->type == type) return base;
    }

    return NULL;
}

static XCamReturn _get_stream_format(AiqCamHwBase_t* pCamHw, rkaiq_stream_type_t type,
                                     struct v4l2_format* format) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    switch (type) {
    case RKISP20_STREAM_MIPITX_S:
    case RKISP20_STREAM_MIPITX_M:
    case RKISP20_STREAM_MIPITX_L: {
        memset(format, 0, sizeof(*format));
        if (!pCamHw->use_rkrawstream) {
            AiqV4l2Device_t* dev = AiqRawStreamCapUnit_get_tx_device(pCamHw->mRawCapUnit, 0);
            ret                  = dev->get_format(dev, format);
        }
        break;
    }
    case RKISP20_STREAM_SP:
    case RKISP20_STREAM_NR: {
        struct v4l2_subdev_format isp_fmt;
        isp_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        isp_fmt.pad   = 2;
        ret           = pCamHw->mIspCoreDev->getFormat(pCamHw->mIspCoreDev, &isp_fmt);
        if (ret == XCAM_RETURN_NO_ERROR) {
            format->fmt.pix.width       = isp_fmt.format.width;
            format->fmt.pix.height      = isp_fmt.format.height;
            format->fmt.pix.pixelformat = get_v4l2_pixelformat(isp_fmt.format.code);
        }
        break;
    }
    default:
        ret = XCAM_RETURN_ERROR_PARAM;
        break;
    }
    return ret;
}

static XCamReturn _get_sp_resolution(AiqCamHwBase_t* pCamHw, int* width, int* height,
                                     int* aligned_w, int* aligned_h) {
#if defined(RKAIQ_ENABLE_SPSTREAM)
    return AiqSPStreamProcUnit_get_sp_resolution(pCamHw->mSpStreamUnit, width, height, aligned_w,
            aligned_h);
#else
    return XCAM_RETURN_NO_ERROR;
#endif
}

#if RKAIQ_HAVE_PDAF
bool _get_pdaf_support(AiqCamHwBase_t* pCamHw) {
    bool pdaf_support = false;

    if (pCamHw->mPdafStreamUnit) pdaf_support = pCamHw->mPdafInfo.pdaf_support;

    return pdaf_support;
}
#endif

static XCamReturn _waitLastSensorDone(AiqCamHwBase_t* pCamHw) {
    aiqMutex_lock(&g_sync_1608_mutex);

    while (!g_sync_1608_done) {
        aiqCond_timedWait(&pCamHw->_sync_done_cond, &g_sync_1608_mutex, 100000ULL);
    }
    aiqMutex_unlock(&g_sync_1608_mutex);

    return XCAM_RETURN_NO_ERROR;
}

void _notify_isp_stream_status(AiqCamHwBase_t* pCamHw, bool on) {
    if (on) {
        LOGK_CAMHW_SUBM(ISP20HW_SUBM, "camId:%d, %s on", pCamHw->mCamPhyId, __func__);
        XCamReturn ret = _hdr_mipi_start_mode(pCamHw, pCamHw->_hdr_mode);
        if (ret < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "hdr mipi start err: %d\n", ret);
        }
        pCamHw->_isp_stream_status = ISP_STREAM_STATUS_STREAM_ON;

        if (pCamHw->_hwResListener.hwResCb) {
            // FIXME: [Baron] 2022-06-29, all rx started, tx start
            AiqHwEvt_t hwEvt;
            hwEvt.type = VICAP_STREAM_ON_EVT;
            if (!pCamHw->_linked_to_1608) {
                pCamHw->_hwResListener.hwResCb(pCamHw->_hwResListener._pCtx, &hwEvt);
            } else {
                _waitLastSensorDone(pCamHw);
                pCamHw->_hwResListener.hwResCb(pCamHw->_hwResListener._pCtx, &hwEvt);
            }
        }
    } else {
        LOGK_CAMHW_SUBM(ISP20HW_SUBM, "camId:%d, %s off", pCamHw->mCamPhyId, __func__);
        pCamHw->_isp_stream_status = ISP_STREAM_STATUS_STREAM_OFF;
        // if CIFISP_V4L2_EVENT_STREAM_STOP event is listened, isp driver
        // will wait isp params streaming off
        {
            aiqMutex_lock(&pCamHw->_stop_cond_mutex);
            if (pCamHw->mIspParamsDev) pCamHw->mIspParamsDev->stop(pCamHw->mIspParamsDev);
            aiqMutex_unlock(&pCamHw->_stop_cond_mutex);
        }
        _hdr_mipi_stop(pCamHw);
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "camId:%d, %s off done", pCamHw->mCamPhyId, __func__);
    }
}

XCamReturn AiqCamHw_reset_hardware(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pCamHw->mRawCapUnit && !pCamHw->use_rkrawstream) {
        ret = AiqRawStreamCapUnit_reset_hardware(pCamHw->mRawCapUnit);
    }
    return ret;
}

XCamReturn AiqCamHw_rawReproc_genIspParams(AiqCamHwBase_t* pCamHw, uint32_t sequence,
        rk_aiq_frame_info_t* offline_finfo, int mode) {
    XCamReturn ret;
    if (mode) {
        if (offline_finfo) {
            AiqSensorHw_t* mSensor = pCamHw->_mSensorDev;
            ret = mSensor->set_effecting_exp_map(mSensor, sequence + 1, offline_finfo, 1);
        }

        AiqStream_t* sofStream = (AiqStream_t*)(pCamHw->mIspSofStream);
        sofStream->stop(sofStream);

        if (pCamHw->_hwResListener.hwResCb) {
            AiqHwEvt_t sofEvt;
            sofEvt.frame_id = sequence;
            sofEvt.type     = ISP_POLL_SOF;
            sofEvt.vb       = NULL;
            pCamHw->_hwResListener.hwResCb(pCamHw->_hwResListener._pCtx, &sofEvt);
        }
    } else {
        // wait until params of frame index 'sequence' have been done
        int8_t wait_times = 100;
        while ((sequence > pCamHw->_curIspParamsSeq) && (wait_times-- > 0)) {
            usleep(10 * 1000);
        }
        if (wait_times == 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "wait params %d(cur:%d) done over 1 seconds !", sequence,
                            pCamHw->_curIspParamsSeq);
        } else {
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "wait params %d(cur:%d) success !", sequence,
                            pCamHw->_curIspParamsSeq);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

const char* AiqCamHw_rawReproc_preInit(const char* isp_driver) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_XCORE_FUNCTION();
    int isp_driver_index = -1;
    if (!isp_driver) {
        return NULL;
    } else {
        if (strstr(isp_driver, "unite")) {
            isp_driver_index = 0;
        } else if (strstr(isp_driver, "-vir")) {
            char* isp_driver_name = (char*)isp_driver;
            char *rkisp = strstr(isp_driver_name, "rkisp");
            if (rkisp) {
                int isp_mode = atoi(rkisp + strlen("rkisp"));
                char* vir = strstr(isp_driver_name, "vir");
                if (vir) {
                    int vir_idx = atoi(vir + strlen("vir"));
                    isp_driver_index = isp_mode * 4 + vir_idx;
                }
            }
        } else {
            if (CHECK_ISP_HW_V30()) {
                LOGE_CAMHW("RK3588 isp_driver just support use media driver name, no support model name");
                isp_driver_index = -1;
            } else if (strcmp(isp_driver, "rkisp0") == 0 ||
                       strcmp(isp_driver, "rkisp") == 0)
                isp_driver_index = 0;
            else if (strcmp(isp_driver, "rkisp1") == 0)
                isp_driver_index = 1;
            else if (strcmp(isp_driver, "rkisp2") == 0)
                isp_driver_index = 2;
            else if (strcmp(isp_driver, "rkisp3") == 0)
                isp_driver_index = 3;
            else if (strcmp(isp_driver, "rkisp4") == 0)
                isp_driver_index = 4;
            else
                isp_driver_index = -1;
        }
    }

    if (isp_driver_index == -1) {
        LOGE_CAMHW("isp_driver name %s is error, please use media driver name or model name(RK3588 no support) as input isp_driver name", isp_driver);
        return NULL;
    }

    SnsFullInfoWraps_t* pSnsInfoWrap = NULL;
    for (pSnsInfoWrap = g_mSensorHwInfos; pSnsInfoWrap != NULL; pSnsInfoWrap = pSnsInfoWrap->next) {
        rk_sensor_full_info_t* s_full_info_f = &pSnsInfoWrap->data;
        if (s_full_info_f->isp_info) {
            if (s_full_info_f->isp_info->model_idx != isp_driver_index) {
                continue;
            }

            return s_full_info_f->sensor_name;
        }
    }
    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "offline preInit faile\n");
    EXIT_XCORE_FUNCTION();
    return NULL;
}

XCamReturn _rawReproc_deInit(const char* fakeSensor) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    FakeCamNmWraps_t* pFakeCamNmWrap = g_mFakeCameraName;
    while (pFakeCamNmWrap) {
        if (strcmp(fakeSensor, pFakeCamNmWrap->key) == 0) break;
        pFakeCamNmWrap = pFakeCamNmWrap->next;
    }

    if (!pFakeCamNmWrap) {
        return XCAM_RETURN_BYPASS;
    }

    char* real_sns_name = pFakeCamNmWrap->data;

    SnsFullInfoWraps_t* pSnsInfoWrap = NULL;
    pSnsInfoWrap                     = g_mSensorHwInfos;
    while (pSnsInfoWrap) {
        if (strcmp(fakeSensor, pSnsInfoWrap->key) == 0) break;
        pSnsInfoWrap = pSnsInfoWrap->next;
    }

    if (!pSnsInfoWrap) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", fakeSensor);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    strcpy(pSnsInfoWrap->key, real_sns_name);
    rk_sensor_full_info_t* s_full_info_f = &pSnsInfoWrap->data;
    strcpy(s_full_info_f->sensor_name, real_sns_name);

    CamHwInfoWraps_t* pCamHwInfoWraps = g_mCamHwInfos;
    while (pCamHwInfoWraps) {
        if (strcmp(fakeSensor, pCamHwInfoWraps->key) == 0) break;
        pCamHwInfoWraps = pCamHwInfoWraps->next;
    }

    if (!pCamHwInfoWraps) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", fakeSensor);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    strcpy(pCamHwInfoWraps->key, real_sns_name);

    pFakeCamNmWrap                       = g_mFakeCameraName;
    FakeCamNmWraps_t* pFakeCamNmWrapPrev = NULL;
    while (pFakeCamNmWrap) {
        if (strcmp(fakeSensor, pFakeCamNmWrap->key)) {
            if (pFakeCamNmWrapPrev)
                pFakeCamNmWrapPrev->next = pFakeCamNmWrap->next;
            else
                g_mFakeCameraName = pFakeCamNmWrap->next;
            aiq_free(pFakeCamNmWrap);
            break;
        }
        pFakeCamNmWrap = pFakeCamNmWrap->next;
        pFakeCamNmWrapPrev = pFakeCamNmWrap;
    }

    return ret;
}

static XCamReturn _setFastAeExp(AiqCamHwBase_t* pCamHw, uint32_t frameId) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    if (pCamHw->mTbInfo.rtt_share_addr) {
        struct rkisp32_thunderboot_resmem_head fastAeAwbInfo;
        rk_aiq_exposure_params_t fastae;
        AiqSensorHw_t* mSensor = pCamHw->_mSensorDev;
        if (pCamHw->mIspCoreDev->_v4l_base.io_control(
                    &pCamHw->mIspCoreDev->_v4l_base, RKISP_CMD_GET_TB_HEAD_V32, &fastAeAwbInfo) < 0)
            ret = XCAM_RETURN_ERROR_FAILED;

        if (ret == XCAM_RETURN_NO_ERROR) {
            if (pCamHw->mWorkingMode == RK_AIQ_WORKING_MODE_NORMAL) {
                fastae.LinearExp.exp_real_params.analog_gain =
                    (float)fastAeAwbInfo.head.exp_gain[0] / (1 << 16);
                fastae.LinearExp.exp_real_params.integration_time =
                    (float)fastAeAwbInfo.head.exp_time[0] / (1 << 16);
                fastae.LinearExp.exp_real_params.digital_gain = 1.0f;
                fastae.LinearExp.exp_real_params.isp_dgain =
                    (float)fastAeAwbInfo.head.exp_isp_dgain[0] / (1 << 16);
                fastae.LinearExp.exp_sensor_params.analog_gain_code_global =
                    fastAeAwbInfo.head.exp_gain_reg[0];
                fastae.LinearExp.exp_sensor_params.coarse_integration_time =
                    fastAeAwbInfo.head.exp_time_reg[0];
                LOGD_CAMHW("fast LinearExp ae set frame %u effect exp %f %f %f", frameId,
                           fastae.LinearExp.exp_real_params.analog_gain,
                           fastae.LinearExp.exp_real_params.integration_time,
                           fastae.LinearExp.exp_real_params.isp_dgain);
            } else {
                fastae.HdrExp[0].exp_real_params.analog_gain =
                    (float)fastAeAwbInfo.head.exp_gain[0] / (1 << 16);
                fastae.HdrExp[0].exp_real_params.integration_time =
                    (float)fastAeAwbInfo.head.exp_time[0] / (1 << 16);
                fastae.HdrExp[0].exp_real_params.digital_gain = 1.0f;
                fastae.HdrExp[0].exp_real_params.isp_dgain =
                    (float)fastAeAwbInfo.head.exp_isp_dgain[0] / (1 << 16);
                fastae.HdrExp[0].exp_sensor_params.analog_gain_code_global =
                    fastAeAwbInfo.head.exp_gain_reg[0];
                fastae.HdrExp[0].exp_sensor_params.coarse_integration_time =
                    fastAeAwbInfo.head.exp_time_reg[0];

                fastae.HdrExp[1].exp_real_params.analog_gain =
                    (float)fastAeAwbInfo.head.exp_gain[1] / (1 << 16);
                fastae.HdrExp[1].exp_real_params.integration_time =
                    (float)fastAeAwbInfo.head.exp_time[1] / (1 << 16);
                fastae.HdrExp[1].exp_real_params.digital_gain = 1.0f;
                fastae.HdrExp[1].exp_real_params.isp_dgain =
                    (float)fastAeAwbInfo.head.exp_isp_dgain[1] / (1 << 16);
                fastae.HdrExp[1].exp_sensor_params.analog_gain_code_global =
                    fastAeAwbInfo.head.exp_gain_reg[1];
                fastae.HdrExp[1].exp_sensor_params.coarse_integration_time =
                    fastAeAwbInfo.head.exp_time_reg[1];

                fastae.HdrExp[2].exp_real_params.analog_gain =
                    (float)fastAeAwbInfo.head.exp_gain[2] / (1 << 16);
                fastae.HdrExp[2].exp_real_params.integration_time =
                    (float)fastAeAwbInfo.head.exp_time[2] / (1 << 16);
                fastae.HdrExp[2].exp_real_params.digital_gain = 1.0f;
                fastae.HdrExp[2].exp_real_params.isp_dgain =
                    (float)fastAeAwbInfo.head.exp_isp_dgain[2] / (1 << 16);
                fastae.HdrExp[2].exp_sensor_params.analog_gain_code_global =
                    fastAeAwbInfo.head.exp_gain_reg[2];
                fastae.HdrExp[2].exp_sensor_params.coarse_integration_time =
                    fastAeAwbInfo.head.exp_time_reg[2];
                LOGD_CAMHW("fast HdrExp[0] ae set frame %u effect exp %f %f %f", frameId,
                           fastae.HdrExp[0].exp_real_params.analog_gain,
                           fastae.HdrExp[0].exp_real_params.integration_time,
                           fastae.HdrExp[0].exp_real_params.isp_dgain);
            }
            mSensor->set_effecting_exp_map(mSensor, frameId, &fastae, 0);
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
    }
#endif
    pCamHw->mAweekId = frameId;

    return ret;
}

XCamReturn AiqCamHw_setVicapStreamMode(AiqCamHwBase_t* pCamHw, int mode, bool is_single_mode) {
    AiqSensorHw_t* mSensor = pCamHw->_mSensorDev;
    uint32_t frameId       = 0;
    bool isSglMd           = false;
    // mode: 0: pause, 1: resume
    if (mode) {
        isSglMd = mSensor->get_is_single_mode(mSensor);
        if (!isSglMd) return XCAM_RETURN_NO_ERROR;

        mSensor->set_pause_flag(mSensor, false, 0, isSglMd);
        AiqRawStreamCapUnit_setVicapStreamMode(pCamHw->mRawCapUnit, mode, &frameId, isSglMd);
        LOGD_CAMHW("raw stream is resume");
    } else {
        AiqRawStreamCapUnit_setVicapStreamMode(pCamHw->mRawCapUnit, mode, &frameId, is_single_mode);
        mSensor->set_pause_flag(mSensor, true, frameId, is_single_mode);
        LOGD_CAMHW("raw stream is stop, id %u, switch to %s frame mode", frameId,
                   is_single_mode ? "single" : "multi");
    }

    return XCAM_RETURN_NO_ERROR;
}

rk_sensor_full_info_t* AiqCamHw_getFullSnsInfo(const char* sensor_name)
{
    SnsFullInfoWraps_t* pSnsInfoWrap  = NULL;
    pSnsInfoWrap = g_mSensorHwInfos;
    while (pSnsInfoWrap) {
        if (strcmp(sensor_name, pSnsInfoWrap->key) == 0) break;
        pSnsInfoWrap = pSnsInfoWrap->next;
    }

    if (!pSnsInfoWrap ) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", sensor_name);
        return NULL;
    }

    return &pSnsInfoWrap->data;
}

void AiqCamHw_setCamPhyId(AiqCamHwBase_t* pCamHw, int phyId) {
    pCamHw->mCamPhyId = phyId;
}

int AiqCamHw_getCamPhyId(AiqCamHwBase_t* pCamHw) {
    return pCamHw->mCamPhyId;
}

XCamReturn AiqCamHw_setHwResListener(AiqCamHwBase_t* pCamHw, AiqHwResListener_t* resListener) {
    pCamHw->_hwResListener._pCtx   = resListener->_pCtx;
    pCamHw->_hwResListener.hwResCb = resListener->hwResCb;

    return XCAM_RETURN_NO_ERROR;
}

void AiqCamHw_setTbInfo(AiqCamHwBase_t* pCamHw, rk_aiq_tb_info_t* info) {
    pCamHw->mTbInfo = *info;
    if (pCamHw->mTbInfo.is_fastboot && pCamHw->getLastEffectParamTb) {
        pCamHw->getLastEffectParamTb(pCamHw);
    }
}

XCamReturn AiqCamHw_rawReProc_prepare(AiqCamHwBase_t* pCamHw, uint32_t sequence) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = pCamHw->_mSensorDev->set_effecting_exp_map(pCamHw->_mSensorDev, sequence,
            &pCamHw->mAiqPreCtrlInfo.isp_param_only_info.first_exp_info,
            1);
    ret = pCamHw->_mSensorDev->set_effecting_exp_map(pCamHw->_mSensorDev, sequence + 1,
            &pCamHw->mAiqPreCtrlInfo.isp_param_only_info.second_exp_info,
            1);
    return ret;
}

void AiqCamHw_setDevBufCnt(AiqCamHwBase_t* pCamHw, AiqDevBufCntsInfo_t* devBufCntsInfo, int cnt) {
    if (!pCamHw) return;

    if (!pCamHw->mDevBufCntMap) {
        pCamHw->mDevBufCntMap =
            (AiqDevBufCntsInfo_t*)aiq_mallocz(sizeof(AiqDevBufCntsInfo_t) * cnt);
    }

    if (pCamHw->mDevBufCntMap) {
        memcpy(pCamHw->mDevBufCntMap, devBufCntsInfo, sizeof(*pCamHw->mDevBufCntMap) * cnt);
        pCamHw->mDevBufCntMap_size = cnt;
    } else
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed malloc mDevBufCntMap");
}

void AiqCamHw_setAiqPreCtrlInfo(AiqCamHwBase_t* pCamHw, rk_aiq_control_preinit_t* info) {
    if (!pCamHw) return;

    pCamHw->mAiqPreCtrlInfo = *info;
}

XCamReturn AiqCamHw_get_sp_resolution(AiqCamHwBase_t* pCamHw, int* width, int* height,
                                      int* aligned_w, int* aligned_h) {
#if defined(RKAIQ_ENABLE_SPSTREAM)
    AiqSPStreamProcUnit_get_sp_resolution(pCamHw->mSpStreamUnit, width, height, aligned_w,
                                          aligned_h);
    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
}

rk_isp_stream_mode_t AiqCamHw_getIspStreamMode(AiqCamHwBase_t* pCamHw) {
    if (true == pCamHw->mNoReadBack)
        return RK_ISP_STREAM_MODE_ONLNIE;
    else
        return RK_ISP_STREAM_MODE_OFFLNIE;
}

#if RKAIQ_HAVE_PDAF
bool AiqCamHw_get_pdaf_support(AiqCamHwBase_t* pCamHw) {
    return _get_pdaf_support(pCamHw);
}

PdafSensorType_t AiqCamHw_get_pdaf_type(AiqCamHwBase_t* pCamHw) {
    PdafSensorType_t pdaf_type = PDAF_SENSOR_TYPE2;

    if (pCamHw->mPdafStreamUnit) pdaf_type = pCamHw->mPdafInfo.pdaf_type;

    return pdaf_type;
}
#endif

void AiqCamHw_keepHwStAtStop(AiqCamHwBase_t* pCamHw, bool ks) {
    pCamHw->mKpHwSt = ks;
}

XCamReturn AiqCamHw_getZoomPosition(AiqCamHwBase_t* pCamHw, int* position) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    if (pCamHw->mLensDev) {
        if (AiqLensHw_getZoomParams(pCamHw->mLensDev, position) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get zoom result failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||get zoom result: %d", position);
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqCamHw_getLensVcmCfg(AiqCamHwBase_t* pCamHw, rk_aiq_lens_vcmcfg* lens_cfg) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    if (pCamHw->mLensDev) {
        if (AiqLensHw_getLensVcmCfg(pCamHw->mLensDev, lens_cfg) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get vcm config failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqCamHw_setLensVcmCfg(AiqCamHwBase_t* pCamHw, rk_aiq_lens_vcmcfg* lens_cfg) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    if (pCamHw->mLensDev) {
        if (AiqLensHw_setLensVcmCfg(pCamHw->mLensDev, lens_cfg) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "set vcm config failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqCamHw_FocusCorrection(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    if (pCamHw->mLensDev) {
        if (AiqLensHw_FocusCorrectionSync(pCamHw->mLensDev) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "focus correction failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqCamHw_ZoomCorrection(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    if (pCamHw->mLensDev) {
        if (AiqLensHw_ZoomCorrection(pCamHw->mLensDev) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "zoom correction failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqCamHw_setAngleZ(AiqCamHwBase_t* pCamHw, float angleZ) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    if (pCamHw->mLensDev) {
        if (AiqLensHw_setAngleZ(pCamHw->mLensDev, angleZ) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "setAngleZ failed");
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn AiqCamHw_getFocusPosition(AiqCamHwBase_t* pCamHw, int* position) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_CAMHW_FUNCTION();

    if (pCamHw->mLensDev) {
        if (AiqLensHw_getFocusParams(pCamHw->mLensDev, position) < 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get focus position failed to device");
            return XCAM_RETURN_ERROR_IOCTL;
        }
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "|||get focus position: %d", position);
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn AiqCamHw_setAiispMode(AiqCamHwBase_t* pCamHw, struct rkisp_aiisp_cfg* aiisp_cfg) {
    pCamHw->use_aiisp  = true;
    pCamHw->mAiisp_cfg = *aiisp_cfg;
    if (!pCamHw->aiisp_param) {
        pCamHw->aiisp_param = (rk_aiisp_param*)aiq_mallocz(sizeof(rk_aiisp_param));
        if (!pCamHw->aiisp_param) goto fail;
    }
    rk_aiq_exposure_sensor_descriptor sensor_des;
    AiqCamHw_getSensorModeData(pCamHw, pCamHw->sns_name, &sensor_des);
    pCamHw->aiisp_param->rawHgt    = sensor_des.isp_acq_height;
    pCamHw->aiisp_param->rawWid    = sensor_des.isp_acq_width;
    pCamHw->aiisp_param->rawHgtStd = pCamHw->aiisp_param->rawHgt;
    pCamHw->aiisp_param->rawWidStd = pCamHw->aiisp_param->rawWid;
    pCamHw->aiisp_param->rawBit    = 12;
    pCamHw->aiisp_param->gainHgt   = sensor_des.isp_acq_height / 2;
    pCamHw->aiisp_param->gainWid   = sensor_des.isp_acq_width / 8;

    if (strstr(pCamHw->sns_name, "os04a10")) {
        pCamHw->aiisp_param->sensorType = 0;
    } else if (strstr(pCamHw->sns_name, "imx415")) {
        pCamHw->aiisp_param->sensorType = 1;
    } else if (strstr(pCamHw->sns_name, "imx464")) {
        pCamHw->aiisp_param->sensorType = 2;
    } else if (strstr(pCamHw->sns_name, "sc200ai")) {
        pCamHw->aiisp_param->sensorType = 3;
    } else {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Invalid sns_name\n");
        pCamHw->aiisp_param->sensorType = 0;
    }
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "aiisp_param->sensorType %d", pCamHw->aiisp_param->sensorType);

    switch (sensor_des.sensor_pixelformat) {
    case V4L2_PIX_FMT_SBGGR14:
    case V4L2_PIX_FMT_SRGGB14:
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SRGGB12:
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SRGGB10:
        pCamHw->aiisp_param->bayerPattern = 0;
        break;
    case V4L2_PIX_FMT_SGBRG14:
    case V4L2_PIX_FMT_SGRBG14:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGRBG10:
        pCamHw->aiisp_param->bayerPattern = 1;
        break;
    default:
        pCamHw->aiisp_param->bayerPattern = 0;
    }
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "aiisp_param->rawHgt %d, aiisp_param->rawWid %d",
                    pCamHw->aiisp_param->rawHgt, pCamHw->aiisp_param->rawWid);
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "aiisp_param->gainHgt %d, aiisp_param->gainWid %d",
                    pCamHw->aiisp_param->gainHgt, pCamHw->aiisp_param->gainWid);

    if (!pCamHw->lib_aiisp_.handle_) {
        if (!pCamHw->lib_aiisp_.Init(&pCamHw->lib_aiisp_)) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "AiispLibrary init failed");
            return XCAM_RETURN_ERROR_FAILED;
        }

        if (!pCamHw->lib_aiisp_.LoadSymbols(&pCamHw->lib_aiisp_)) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "AiispLibrary LoadSymbols failed");
            return XCAM_RETURN_ERROR_FAILED;
        }
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "AiispLibrary init success");

        struct AiispOps* ops = AiqAiIsp_GetOps(&pCamHw->lib_aiisp_);
        ops->aiisp_init(pCamHw->aiisp_param);
    }
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "aiisp_init success");
    return XCAM_RETURN_NO_ERROR;
fail:
    return XCAM_RETURN_ERROR_FAILED;
}

static XCamReturn AiqCamHw_process_restriction(AiqCamHwBase_t* pCamHw, void* isp_cfg)
{
#if ISP_HW_V39
    struct isp39_isp_params_cfg* isp_params = (struct isp39_isp_params_cfg*)isp_cfg;
#elif ISP_HW_V33
    struct isp33_isp_params_cfg* isp_params = (struct isp33_isp_params_cfg*)isp_cfg;
#elif ISP_HW_V35
    struct isp35_isp_params_cfg* isp_params = (struct isp35_isp_params_cfg*)isp_cfg;
#endif
#if defined(ISP_HW_V39)
    if (pCamHw->use_aiisp) {
        if (isp_params->others.bls_cfg.isp_ob_predgain != 0 ||
                isp_params->others.bay3d_cfg.iirsparse_en != 1 ||
                isp_params->others.bay3d_cfg.transf_bypass_en != 1) {
            LOGW_CAMHW_SUBM(ISP20HW_SUBM,
                            "When aiisp on, isp_ob_predain should be 0"
                            "sw_bay3d_iirsparse_en and hw_btnr_transf_bypass_en should be 1");
            isp_params->others.bls_cfg.isp_ob_predgain    = 0;
            isp_params->others.bay3d_cfg.iirsparse_en     = 1;
            isp_params->others.bay3d_cfg.transf_bypass_en = 1;
        }
    }
#endif

#if defined(ISP_HW_V35)
    // isp35 ie use cnr module's uv_dis and exgain_bypass
    if ((isp_params->module_cfg_update & ISP2X_MODULE_CNR) ||
            (isp_params->module_en_update & ISP2X_MODULE_IE)) {
        bool old_en_ie = !!(pCamHw->_isp_module_ens & ISP2X_MODULE_IE);
        bool new_en_ie =
            !!(isp_params->module_en_update & ISP2X_MODULE_IE) ? !!(isp_params->module_ens & ISP2X_MODULE_IE) : old_en_ie;

        if (new_en_ie) {
            isp_params->others.cnr_cfg.uv_dis        = 1;
            isp_params->others.cnr_cfg.exgain_bypass = 1;
            isp_params->module_cfg_update &= ~ISP2X_MODULE_AWB_GAIN;
        } else {
            isp_params->others.cnr_cfg.uv_dis        = 0;
            isp_params->others.cnr_cfg.exgain_bypass = 0;
        }
        LOGD_CAMHW("%s set ie by cnr uv_dis %d exgain_bypass %d cnr_update %d",
                   __func__, isp_params->others.cnr_cfg.uv_dis,
                   isp_params->others.cnr_cfg.exgain_bypass,
                   !!(isp_params->module_cfg_update & ISP2X_MODULE_CNR));
    }
#endif

//global gain constraint
    bool old_en_drc = !!(pCamHw->_isp_module_ens & ISP2X_MODULE_DRC);
    bool old_en_mge = !!(pCamHw->_isp_module_ens & ISP2X_MODULE_HDRMGE);
    bool old_en_btnr = !!(pCamHw->_isp_module_ens & ISP3X_MODULE_BAY3D);
    bool old_en_lsc = !!(pCamHw->_isp_module_ens & ISP2X_MODULE_LSC);
#ifdef ISP_HW_V39
    bool old_en_dehaze = !!(pCamHw->_isp_module_ens & ISP2X_MODULE_DHAZ);
    bool old_en_yuvme = !!(pCamHw->_isp_module_ens & ISP39_MODULE_YUVME);
#endif

    bool new_en_drc =
        !!(isp_params->module_en_update & ISP2X_MODULE_DRC) ? !!(isp_params->module_ens & ISP2X_MODULE_DRC) : old_en_drc;
    bool new_en_mge =
        !!(isp_params->module_en_update & ISP2X_MODULE_HDRMGE) ? !!(isp_params->module_ens & ISP2X_MODULE_HDRMGE) : old_en_mge;
    bool new_en_btnr =
        !!(isp_params->module_en_update & ISP3X_MODULE_BAY3D) ? !!(isp_params->module_ens & ISP3X_MODULE_BAY3D) : old_en_btnr;
    bool new_en_lsc =
        !!(isp_params->module_en_update & ISP2X_MODULE_LSC) ? !!(isp_params->module_ens & ISP2X_MODULE_LSC) : old_en_lsc;
#ifdef ISP_HW_V39
    bool new_en_dehaze =
        !!(isp_params->module_en_update & ISP2X_MODULE_DHAZ) ? !!(isp_params->module_ens & ISP2X_MODULE_DHAZ) : old_en_dehaze;
    bool new_en_yuvme =
        !!(isp_params->module_en_update & ISP39_MODULE_YUVME) ? !!(isp_params->module_ens & ISP39_MODULE_YUVME) : old_en_yuvme;
#endif
    bool gain_en = true;
    if(isp_params->module_cfg_update & ISP2X_MODULE_CNR)
        pCamHw->exgain_status.exgain_bypass = isp_params->others.cnr_cfg.exgain_bypass;
    if(isp_params->module_cfg_update & ISP2X_MODULE_SHARP)
        pCamHw->exgain_status.local_gain_bypass = isp_params->others.sharp_cfg.local_gain_bypass;
#if ISP_HW_V33
    if(isp_params->module_cfg_update & ISP2X_MODULE_GIC)
        pCamHw->exgain_status.gain_bypass_en = isp_params->others.gic_cfg.gain_bypass_en;
    if (!new_en_drc && !new_en_mge && !new_en_btnr && !new_en_lsc) {
#elif ISP_HW_V35
    if(isp_params->module_cfg_update & ISP2X_MODULE_GIC)
        pCamHw->exgain_status.gain_bypass_en = isp_params->others.gic_cfg.gain_bypass_en;
    if (!new_en_drc && !new_en_mge && !new_en_btnr && !new_en_lsc) {
#elif ISP_HW_V39
    if (!new_en_drc && !new_en_mge && !new_en_btnr && !new_en_lsc && !new_en_dehaze && !new_en_yuvme) {
#endif
        gain_en = false;
        isp_params->module_ens &= ~ISP3X_MODULE_GAIN;
        isp_params->module_en_update |= ISP3X_MODULE_GAIN;
        isp_params->others.cnr_cfg.exgain_bypass = 1;
        isp_params->others.sharp_cfg.local_gain_bypass = 1;
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
        isp_params->others.gic_cfg.gain_bypass_en = 1;
#endif
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "When global gain related module turn off, autoly turn off gain and turn on exgain_bypass");
    }
    else {
        if (pCamHw->exgain_status.gain_module_en) {
            isp_params->module_ens |= ISP3X_MODULE_GAIN;
            isp_params->module_en_update |= ISP3X_MODULE_GAIN;
        }
        isp_params->others.cnr_cfg.exgain_bypass = pCamHw->exgain_status.exgain_bypass;
        isp_params->others.sharp_cfg.local_gain_bypass = pCamHw->exgain_status.local_gain_bypass;
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
        isp_params->others.gic_cfg.gain_bypass_en = pCamHw->exgain_status.gain_bypass_en;
#endif
    }

//ynr cnr sharp enh gain constraint
#if ISP_HW_V39
    int state = AiqManager_getAiqState(pCamHw->rkAiqManager);
    if (state != AIQ_STATE_INITED && state != AIQ_STATE_STOPED) {
        bool update_ynr = isp_params->module_en_update & ISP2X_MODULE_YNR;
        bool update_cnr = isp_params->module_en_update & ISP2X_MODULE_CNR;
        bool update_sharp = isp_params->module_en_update & ISP2X_MODULE_SHARP;
        if(!(update_ynr && update_cnr && update_sharp)) {
            isp_params->module_ens &= ~ISP3X_MODULE_CNR;
            isp_params->module_ens &= ~ISP3X_MODULE_SHARP;
            isp_params->module_ens &= ~ISP3X_MODULE_YNR;
            isp_params->module_en_update &= ~ISP3X_MODULE_CNR;
            isp_params->module_en_update &= ~ISP3X_MODULE_SHARP;
            isp_params->module_en_update &= ~ISP3X_MODULE_YNR;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "ynr, cnr and sharp'en should be update together!");
        } else {
            bool new_en_ynr = isp_params->module_ens & ISP2X_MODULE_YNR;
            bool new_en_cnr = isp_params->module_ens & ISP2X_MODULE_CNR;
            bool new_en_sharp = isp_params->module_ens & ISP2X_MODULE_SHARP;

            // check if all true or all false
            if (new_en_ynr && new_en_cnr && new_en_sharp && gain_en) {
                isp_params->module_ens |= ISP3X_MODULE_GAIN;
                isp_params->module_en_update |= ISP3X_MODULE_GAIN;
                pCamHw->exgain_status.gain_module_en = true;
                return XCAM_RETURN_NO_ERROR;
            }
            if (!new_en_ynr && !new_en_cnr && !new_en_sharp) {
                isp_params->module_ens &= ~ISP3X_MODULE_GAIN;
                isp_params->module_en_update |= ISP3X_MODULE_GAIN;
                pCamHw->exgain_status.gain_module_en = false;
                return XCAM_RETURN_NO_ERROR;
            }
        }
    }
#endif
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    int state = AiqManager_getAiqState(pCamHw->rkAiqManager);
    if (state != AIQ_STATE_INITED && state != AIQ_STATE_STOPED) {
        bool update_ynr = isp_params->module_en_update & ISP2X_MODULE_YNR;
        bool update_cnr = isp_params->module_en_update & ISP2X_MODULE_CNR;
        bool update_sharp = isp_params->module_en_update & ISP2X_MODULE_SHARP;
        bool update_enh = isp_params->module_en_update & ISP33_MODULE_ENH;

        if(!(update_ynr && update_cnr && update_sharp && update_enh)) {
            isp_params->module_ens &= ~ISP3X_MODULE_CNR;
            isp_params->module_ens &= ~ISP3X_MODULE_SHARP;
            isp_params->module_ens &= ~ISP3X_MODULE_YNR;
            isp_params->module_ens &= ~ISP33_MODULE_ENH;
            isp_params->module_en_update &= ~ISP3X_MODULE_CNR;
            isp_params->module_en_update &= ~ISP3X_MODULE_SHARP;
            isp_params->module_en_update &= ~ISP3X_MODULE_YNR;
            isp_params->module_en_update &= ~ISP33_MODULE_ENH;
            LOGD_CAMHW_SUBM(ISP20HW_SUBM, "ynr, cnr and sharp'en should be update together!");
        }
        else {
            bool new_en_ynr = isp_params->module_ens & ISP2X_MODULE_YNR;
            bool new_en_cnr = isp_params->module_ens & ISP2X_MODULE_CNR;
            bool new_en_sharp = isp_params->module_ens & ISP2X_MODULE_SHARP;
            bool new_en_enh = isp_params->module_ens & ISP33_MODULE_ENH;

            // check if all true or all false
            if (new_en_ynr && new_en_cnr && new_en_sharp && new_en_enh && gain_en) {
                isp_params->module_ens |= ISP3X_MODULE_GAIN;
                isp_params->module_en_update |= ISP3X_MODULE_GAIN;
                pCamHw->exgain_status.gain_module_en = true;
                return XCAM_RETURN_NO_ERROR;
            }
            if (!new_en_ynr && !new_en_cnr && !new_en_sharp && !new_en_enh) {
                isp_params->module_ens &= ~ISP3X_MODULE_GAIN;
                isp_params->module_en_update |= ISP3X_MODULE_GAIN;
                pCamHw->exgain_status.gain_module_en = false;
                return XCAM_RETURN_NO_ERROR;
            }
        }
    }

#endif
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_read_aiisp_result(AiqCamHwBase_t* pCamHw, rk_aiq_aiisp_t* aiisp_evt) {
    if (pCamHw->mIspAiispStream) {
        struct rkisp_aiisp_st aIispDrvParams;
        memset(&aIispDrvParams, 0, sizeof(aIispDrvParams));
        aIispDrvParams.timestamp = aiisp_evt->timestamp;
        aIispDrvParams.sequence = aiisp_evt->sequence;
        aIispDrvParams.iir_index = aiisp_evt->iir_index;
        aIispDrvParams.gain_index = aiisp_evt->gain_index;
        aIispDrvParams.aiisp_index = aiisp_evt->aiisp_index;
        return pCamHw->mIspAiispStream->call_aiisp_rd_start(pCamHw->mIspAiispStream, &aIispDrvParams);
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn AiqCamHw_aiisp_processing(AiqCamHwBase_t* pCamHw, rk_aiq_aiisp_t* aiisp_evt) {
#if defined(ISP_HW_V39)
    struct rkisp_bnr_buf_info* bay3dbuf   = aiisp_evt->bay3dbuf;
    pCamHw->aiisp_param->bufInType   = 0;
    pCamHw->aiisp_param->pBufIn      = aiisp_evt->iir_address;
    pCamHw->aiisp_param->bufInFd     = bay3dbuf->iir.buf_fd[aiisp_evt->iir_index];
    pCamHw->aiisp_param->bufOutType  = 0;
    pCamHw->aiisp_param->pBufOut     = aiisp_evt->aiisp_address;
    pCamHw->aiisp_param->bufOutFd    = bay3dbuf->u.v39.aiisp.buf_fd[aiisp_evt->aiisp_index];
    pCamHw->aiisp_param->pGainMapBuf = aiisp_evt->gain_address;
    pCamHw->aiisp_param->gainMapFd   = bay3dbuf->u.v39.gain.buf_fd[aiisp_evt->gain_index];

    AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
    if (mSensorSubdev) {
        AiqSensorExpInfo_t* expParam =
            mSensorSubdev->getEffectiveExpParams(mSensorSubdev, aiisp_evt->sequence);

        if (!expParam) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "frame_id(%d), get exposure failed!!!\n",
                            aiisp_evt->sequence);
        } else {
            float iso = expParam->aecExpInfo.LinearExp.exp_real_params.analog_gain *
                        expParam->aecExpInfo.LinearExp.exp_real_params.digital_gain *
                        expParam->aecExpInfo.LinearExp.exp_real_params.isp_dgain * 50;
            pCamHw->aiisp_param->ISO = iso;
            AIQ_REF_BASE_UNREF(&expParam->_base._ref_base);
        }
    }

    aiq_isp_effect_params_t* ispParams;
    AiqCamHw_getEffectiveIspParams(pCamHw, &ispParams, aiisp_evt->sequence);
    if (ispParams) {
        pCamHw->aiisp_param->rGain = ispParams->awb_gain_cfg.awb1_gain_r == 0
                                     ? 0
                                     : ((float)ispParams->awb_gain_cfg.awb1_gain_r) /
                                     ((float)ispParams->awb_gain_cfg.awb1_gain_gr);
        pCamHw->aiisp_param->bGain = ispParams->awb_gain_cfg.awb1_gain_b == 0
                                     ? 0
                                     : ((float)ispParams->awb_gain_cfg.awb1_gain_b) /
                                     ((float)ispParams->awb_gain_cfg.awb1_gain_gb);
        AIQ_REF_BASE_UNREF(&ispParams->_ref_base);
    } else {
        LOGE("aiisp: can't get wbgain !");
    }
    LOGD_CAMHW_SUBM(ISP20HW_SUBM,
                    "aiisp_param->ISO %f, aiisp_param->rGain %f, aiisp_param->bGain %f",
                    pCamHw->aiisp_param->ISO, pCamHw->aiisp_param->rGain,
                    pCamHw->aiisp_param->bGain);

    struct AiispOps* ops = AiqAiIsp_GetOps(&pCamHw->lib_aiisp_);
    ops->aiisp_proc(pCamHw->aiisp_param);
    // memcpy(pCamHw->aiisp_param->pBufOut, pCamHw->aiisp_param->pBufIn, bay3dbuf.iir_size);

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
}

XCamReturn AiqCamHw_setUserOtpInfo(AiqCamHwBase_t* pCamHw, rk_aiq_user_otp_info_t* otp_info)
{
    LOGD_CAMHW("user awb otp: flag: %d, r:%d,b:%d,gr:%d,gb:%d, golden r:%d,b:%d,gr:%d,gb:%d;\
                user lsc otp: flag: %d, r[0]: %d, gr[0]: %d, gb[0]: %d, b[0]: %d\n",
               otp_info->otp_awb.flag,
               otp_info->otp_awb.r_value, otp_info->otp_awb.b_value,
               otp_info->otp_awb.gr_value, otp_info->otp_awb.gb_value,
               otp_info->otp_awb.golden_r_value, otp_info->otp_awb.golden_b_value,
               otp_info->otp_awb.golden_gr_value, otp_info->otp_awb.golden_gb_value,
               otp_info->otp_lsc.flag,
               otp_info->otp_lsc.lsc_r[0], otp_info->otp_lsc.lsc_gr[0],
               otp_info->otp_lsc.lsc_gb[0], otp_info->otp_lsc.lsc_b[0]);

    pCamHw->_mIspParamsCvt->mCommonCvtInfo.otp_awb = otp_info->otp_awb;
    pCamHw->_mIspParamsCvt->mCommonCvtInfo.otp_lsc = otp_info->otp_lsc;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn SetLastAeExpToRttShared(AiqCamHwBase_t* pCamHw) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    unsigned long cmd;
    struct rkisp_thunderboot_resmem_head *head = NULL;
    AiqV4l2SubDevice_t* mIspCoreDev = pCamHw->mIspCoreDev;
#if defined(ISP_HW_V33)
    struct rkisp33_thunderboot_resmem_head fastAeAwbInfo;
    head = &fastAeAwbInfo.head;
#elif defined(ISP_HW_V35)
    struct rkisp_thunderboot_resmem_head fastAeAwbInfo;
    head = &fastAeAwbInfo;
#else
    struct rkisp32_thunderboot_resmem_head fastAeAwbInfo;
    head = &fastAeAwbInfo.head;
#endif

#if defined(ISP_HW_V33)
    cmd = RKISP_CMD_GET_TB_HEAD_V33;
#elif defined(ISP_HW_V35)
    cmd = RKISP_CMD_GET_TB_HEAD;
#else
    cmd = RKISP_CMD_GET_TB_HEAD_V32;
#endif
    if (mIspCoreDev->_v4l_base.io_control(&pCamHw->mIspCoreDev->_v4l_base, cmd, &fastAeAwbInfo) < 0)
        ret = XCAM_RETURN_ERROR_FAILED;

    if (ret == XCAM_RETURN_NO_ERROR) {
        AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;
        if (!mSensorSubdev) {
            LOGE_CAMHW("<TB>: sensor is NULL\n");
            return XCAM_RETURN_ERROR_SENSOR;
        }

        AiqSensorExpInfo_t* last_ae =
            mSensorSubdev->getEffectiveExpParams(mSensorSubdev, (uint32_t)(-1) - 1);
        if (!last_ae) {
            LOGE_CAMHW("<TB>: get last exp error\n");
            return ret;
        }

        if (pCamHw->mWorkingMode == RK_AIQ_WORKING_MODE_NORMAL) {
            head->exp_gain[0] =
                (uint32_t)(last_ae->aecExpInfo.LinearExp.exp_real_params.analog_gain * (1 << 16));
            head->exp_time[0] = (uint32_t)(
                                    last_ae->aecExpInfo.LinearExp.exp_real_params.integration_time * (1 << 16));
            head->exp_isp_dgain[0] =
                (uint32_t)(last_ae->aecExpInfo.LinearExp.exp_real_params.isp_dgain * (1 << 16));
        } else {
            head->exp_gain[0] =
                (uint32_t)(last_ae->aecExpInfo.HdrExp[0].exp_real_params.analog_gain * (1 << 16));
            head->exp_time[0] = (uint32_t)(
                                    last_ae->aecExpInfo.HdrExp[0].exp_real_params.integration_time * (1 << 16));
            head->exp_isp_dgain[0] =
                (uint32_t)(last_ae->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain * (1 << 16));

            head->exp_gain[0] =
                (uint32_t)(last_ae->aecExpInfo.HdrExp[1].exp_real_params.analog_gain * (1 << 16));
            head->exp_time[0] = (uint32_t)(
                                    last_ae->aecExpInfo.HdrExp[1].exp_real_params.integration_time * (1 << 16));
            head->exp_isp_dgain[0] =
                (uint32_t)(last_ae->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain * (1 << 16));

            head->exp_gain[0] =
                (uint32_t)(last_ae->aecExpInfo.HdrExp[2].exp_real_params.analog_gain * (1 << 16));
            head->exp_time[0] = (uint32_t)(
                                    last_ae->aecExpInfo.HdrExp[2].exp_real_params.integration_time * (1 << 16));
            head->exp_isp_dgain[0] =
                (uint32_t)(last_ae->aecExpInfo.HdrExp[2].exp_real_params.isp_dgain * (1 << 16));
        }

        AIQ_REF_BASE_UNREF(&last_ae->_base._ref_base);

#if defined(ISP_HW_V33)
        cmd = RKISP_CMD_SET_TB_HEAD_V33;
#elif defined(ISP_HW_V35)
        cmd = RKISP_CMD_GET_TB_HEAD;
#else
        cmd = RKISP_CMD_SET_TB_HEAD_V32;
#endif
        if (mIspCoreDev->_v4l_base.io_control(&pCamHw->mIspCoreDev->_v4l_base, cmd,
                                              &fastAeAwbInfo) < 0) {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_CAMHW("<TB>: fastboot set last ae exp to rtt share faile");
            return ret;
        }

        LOGK_CAMHW("save last exp to rtt share: gain: 0x%x time 0x%x isp_dgain %0x",
                   head->exp_gain[0], head->exp_time[0],
                   head->exp_isp_dgain[0]);
    }

    if (pCamHw->mTbInfo.is_start_again) {
        return ret;
    }

    if (pCamHw->saveInfotoFileTb) {
        pCamHw->saveInfotoFileTb(pCamHw);
    }

    return ret;
}

RkAiqIspUniteMode
AiqCamHw_getIspUniteMode(AiqCamHwBase_t* pCamHw) {
    if (pCamHw->mParamsSplitter)
        return AiqIspParamsSplitter_GetIspUniteMode(pCamHw->mParamsSplitter);

    return RK_AIQ_ISP_UNITE_MODE_NORMAL;
}

XCamReturn
AiqCamHw_setNoReadbackMode(AiqCamHwBase_t* pCamHw, bool on)
{
    pCamHw->mNoReadBack = !on;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqCamHw_setRawBufNum(AiqCamHwBase_t* pCamHw, uint16_t buf_num)
{
    if (pCamHw->_state == CAM_HW_STATE_PREPARED || pCamHw->_state == CAM_HW_STATE_STARTED)
        return XCAM_RETURN_ERROR_FAILED;

    AiqRawStreamCapUnit_setTxBufferCnt(pCamHw->mRawCapUnit, buf_num);
    AiqRawStreamProcUnit_setRxBufferCnt(pCamHw->mRawProcUnit, buf_num);

    return XCAM_RETURN_NO_ERROR;
}

const char*
AiqCamHw_getBindedIspDrvNmBySns(const char* sns_ent_name)
{

    if (strstr(sns_ent_name, "FakeCamera")) {
        LOGK_CAMHW("%s: get isp driver name by virtual sensor(%s)", __func__, sns_ent_name);
    }

    SnsFullInfoWraps_t* pItem = g_mSensorHwInfos;
    rk_sensor_full_info_t* s_info = NULL;
    while (pItem) {
        if (strcmp(sns_ent_name, pItem->key) == 0) break;
        pItem = pItem->next;
    }

    if (!pItem) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", sns_ent_name);
        return NULL;
    }

    s_info = &pItem->data;

    if (!s_info->isp_info) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,
                        "can't find isp driver name by sensor %s, maybe %s no binded to isp driver",
                        sns_ent_name, sns_ent_name);
        return NULL;
    }

    return s_info->isp_info->driver;
}

XCamReturn
AiqCamHw_setAibnrDelayCnt(AiqCamHwBase_t* pCamHw, int delayCnt)
{
    if (pCamHw->_state == CAM_HW_STATE_PREPARED || pCamHw->_state == CAM_HW_STATE_STARTED)
        return XCAM_RETURN_ERROR_FAILED;

    pCamHw->mDefaultDelayCnt = delayCnt;
    if (pCamHw->mIspStatsDev)
        AiqV4l2Device_setBufCnt(pCamHw->mIspStatsDev, delayCnt + 2);

    if (pCamHw->mIspParamsDev)
        AiqV4l2Device_setBufCnt(pCamHw->mIspParamsDev, delayCnt + 3);

    if (pCamHw->_mIspParamsCvt)
        AiqIspParamsCvt_setStastDelayCnt(pCamHw->_mIspParamsCvt, delayCnt);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqCamHw_setAiynrDelayCnt(AiqCamHwBase_t* pCamHw, int delayCnt)
{
    if (pCamHw->_state == CAM_HW_STATE_PREPARED || pCamHw->_state == CAM_HW_STATE_STARTED)
        return XCAM_RETURN_ERROR_FAILED;

    pCamHw->mDefaultDelayCnt = delayCnt;
    if (pCamHw->mIspStatsDev)
        AiqV4l2Device_setBufCnt(pCamHw->mIspStatsDev, delayCnt + 2);

    if (pCamHw->mIspParamsDev)
        AiqV4l2Device_setBufCnt(pCamHw->mIspParamsDev, delayCnt + 2);

    if (pCamHw->_mIspParamsCvt)
        AiqIspParamsCvt_setStastDelayCnt(pCamHw->_mIspParamsCvt, delayCnt);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqCamHw_setSnsOtpInfo(AiqCamHwBase_t* pCamHw,
                       struct rkmodule_awb_inf * otp_awb,
                       struct rkmodule_lsc_inf * otp_lsc,
                       rk_aiq_user_otp_info_t* user_otp)
{
    if (user_otp->otp_awb.flag == 0) {
        if (otp_awb == NULL) {
            pCamHw->_mIspParamsCvt->mCommonCvtInfo.otp_awb.flag = 0;
        } else {
            pCamHw->_mIspParamsCvt->mCommonCvtInfo.otp_awb = *otp_awb;
        }
    }
    if (user_otp->otp_lsc.flag == 0) {
        if (otp_lsc == NULL) {
            pCamHw->_mIspParamsCvt->mCommonCvtInfo.otp_lsc.flag = 0;
        } else {
            pCamHw->_mIspParamsCvt->mCommonCvtInfo.otp_lsc = *otp_lsc;
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqCamHw_pushImuData(AiqCamHwBase_t* pCamHw, AiqImuData_t *data)
{
#if RKAIQ_HAVE_MEMC
    AiqMemcManager_pushImuData(pCamHw->mMemcStream, data);
#endif
    return XCAM_RETURN_NO_ERROR;
}

bool AiqCamHw_getMemcNeededDelay(AiqCamHwBase_t* pCamHw) {

    if (!pCamHw && !pCamHw->mMemcStream) {
        return false;
    }

#if defined(RKAIQ_HAVE_MEMC)
    AiqMemcManager_setMemcAlgoStatus(pCamHw->mMemcStream);
    pCamHw->mMemcEn = AiqMemcManager_getMemcEn(pCamHw->mMemcStream);
    if (pCamHw->mMemcEn) {
        return AiqMemcManager_getMemcIsSyncBaseImuMode(pCamHw->mMemcStream);
    }
#endif

    return false;

}

XCamReturn
AiqCamHw_queryFpnSwStatus(AiqCamHwBase_t* pCamHw, void *status)
{

#ifdef RKAIQ_HAVE_FPNSW
    if (pCamHw->mFpnStreamUnit) {
        return AiqFpnSwStreamProcUnit_queryStatus(pCamHw->mFpnStreamUnit, status);
    }
#endif

    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn AiqCamHw_setSingleRawBufAsyncMode(AiqCamHwBase_t* pCamHw, bool mode)
{
    if (!pCamHw && !pCamHw->mRawCapUnit) {
        return XCAM_RETURN_ERROR_FAILED;
    }

    return AiqRawStreamCapUnit_setSingleBufAsyncMode(pCamHw->mRawCapUnit, mode);
}

bool
AiqCamHw_getAiqReadbackOn(AiqCamHwBase_t* pCamHw)
{
    rk_sensor_full_info_t* s_info = NULL;
    SnsFullInfoWraps_t* pItem     = g_mSensorHwInfos;
    while (pItem) {
        if (strcmp(pCamHw->sns_name, pItem->key) == 0) break;
        pItem = pItem->next;
    }

    if (!pItem) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", pCamHw->sns_name);
        return false;
    }

    s_info = &pItem->data;

    bool aiq_read_back_on = false;
    if ((pCamHw->_hdr_mode > 0 && pCamHw->mIsOnlineByWorkingMode) ||
        (!pCamHw->_linked_to_isp && !pCamHw->mVicapIspPhyLinkSupported) ||
        (s_info->linked_to_serdes || s_info->split)) {
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "use read back mode!");
        aiq_read_back_on = true;
    }

#if RKAIQ_HAVE_MEMC
    if (pCamHw->mMemcStream && pCamHw->mMemcEn && AiqMemcManager_getMemcIsSyncBaseImuMode(pCamHw->mMemcStream)) {
        AiqCamHw_setRawBufNum(pCamHw, 5);
        aiq_read_back_on = true;
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "btnr memc force use read back mode, force raw buf num to 5!");
    }
#endif

#if RKAIQ_HAVE_FPNSW
    if (pCamHw->mCalibDbV2) {
        fpnSw_api_attrib_t* fpnSw_attrib =
            (fpnSw_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCamHw->mCalibDbV2), fpnSw));
        if (pCamHw->mFpnStreamUnit && fpnSw_attrib && fpnSw_attrib->en) {
            aiq_read_back_on = true;
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "fpnSw force use read back mode!");
        }
    }
#endif

    return aiq_read_back_on;

}

static void AiqCamHwBase_initNotifier(AiqCamHwBase_t* pCamHw) {
#if RKAIQ_HAVE_DUMPSYS
    aiq_notifier_init(&pCamHw->notifier);

    {
        pCamHw->sub_base.match_type     = AIQ_NOTIFIER_MATCH_HWI_BASE;
        pCamHw->sub_base.name           = "HWI -> base";
        pCamHw->sub_base.dump.dump_fn_t = __dump;
        pCamHw->sub_base.dump.dumper    = pCamHw;

        aiq_notifier_add_subscriber(&pCamHw->notifier, &pCamHw->sub_base);
    }

    {
        pCamHw->sub_stream_cap.match_type     = AIQ_NOTIFIER_MATCH_HWI_STREAM_CAP;
        pCamHw->sub_stream_cap.name           = "HWI -> stream_cap";
        pCamHw->sub_stream_cap.dump.dump_fn_t = AiqRawStreamCapUnit_dump;
        pCamHw->sub_stream_cap.dump.dumper    = pCamHw->mRawCapUnit;

        aiq_notifier_add_subscriber(&pCamHw->notifier, &pCamHw->sub_stream_cap);
    }

    {
        pCamHw->sub_stream_proc.match_type     = AIQ_NOTIFIER_MATCH_HWI_STREAM_PROC;
        pCamHw->sub_stream_proc.name           = "HWI -> stream_proc";
        pCamHw->sub_stream_proc.dump.dump_fn_t = AiqRawStreamProcUnit_dump;
        pCamHw->sub_stream_proc.dump.dumper    = pCamHw->mRawProcUnit;

        aiq_notifier_add_subscriber(&pCamHw->notifier, &pCamHw->sub_stream_proc);
    }

    {
        pCamHw->sub_sensor.match_type     = AIQ_NOTIFIER_MATCH_HWI_SENSOR;
        pCamHw->sub_sensor.name           = "HWI -> sensor";
        pCamHw->sub_sensor.dump.dump_fn_t = pCamHw->_mSensorDev->dump;
        pCamHw->sub_sensor.dump.dumper    = pCamHw->_mSensorDev;

        aiq_notifier_add_subscriber(&pCamHw->notifier, &pCamHw->sub_sensor);
    }

    hwi_isp_mods_add_subscriber(pCamHw);
#endif
}

static int __dump(void* dumper, st_string * result, int argc, void* argv[]) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if RKAIQ_HAVE_DUMPSYS
    hwi_base_dump_mod_param((AiqCamHwBase_t*)dumper, result);
    hwi_base_dump_chn_status((AiqCamHwBase_t*)dumper, result);
    hwi_base_dump_stats_videobuf_status((AiqCamHwBase_t*)dumper, result);
    hwi_base_dump_params_videobuf_status((AiqCamHwBase_t*)dumper, result);
#endif

    return ret;
}

#if RKAIQ_HAVE_DUMPSYS
static const char* const usages[] = {
    "./dumpcam hwi cmd [args]",
    NULL,
};

static int _gHelp = 0;
static int dbg_help_cb(struct argparse * self, const struct argparse_option * option) {
    _gHelp = 1;

    return 0;
}
#endif

static int AiqCamHw_dump(void* pCamHw, st_string * result, int argc, void* argv[]) {
#if RKAIQ_HAVE_DUMPSYS
    int dump_args[AIQ_NOTIFIER_MATCH_MAX] = {0};

    if (argc < 2) {
        dump_args[AIQ_NOTIFIER_MATCH_HWI_ALL] = 1;
    } else {
        char buffer[MAX_LINE_LENGTH * 4] = {0};
        struct argparse_option options[] = {
            OPT_GROUP("basic options:"),
            OPT_BOOLEAN('A', "all", &dump_args[AIQ_NOTIFIER_MATCH_HWI_ALL], "dump all info", NULL,
                        0, 0),
            OPT_BOOLEAN('b', "base", &dump_args[AIQ_NOTIFIER_MATCH_HWI_BASE], "dump base info",
                        NULL, 0, 0),
            OPT_BOOLEAN('c', "cap", &dump_args[AIQ_NOTIFIER_MATCH_HWI_STREAM_CAP],
                        "dump stream capture info", NULL, 0, 0),
            OPT_BOOLEAN('d', "proc", &dump_args[AIQ_NOTIFIER_MATCH_HWI_STREAM_PROC],
                        "dump stream proc info", NULL, 0, 0),
            OPT_BOOLEAN('s', "sensor", &dump_args[AIQ_NOTIFIER_MATCH_HWI_SENSOR],
                        "dump sensor info ", NULL, 0, 0),

            OPT_GROUP("isp modules options:"),
            OPT_BOOLEAN('e', "ae", &dump_args[AIQ_NOTIFIER_MATCH_AEC], "dump ae module info", NULL,
                        0, 0),
            OPT_BOOLEAN('f', "hist", &dump_args[AIQ_NOTIFIER_MATCH_HIST], "dump hist module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('i', "awb", &dump_args[AIQ_NOTIFIER_MATCH_AWB], "dump awb module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('j', "wbgain", &dump_args[AIQ_NOTIFIER_MATCH_AWB],
                        "dump awb gain module info ", NULL, 0, 0),
            OPT_BOOLEAN('k', "af", &dump_args[AIQ_NOTIFIER_MATCH_AF], "dump af module info", NULL,
                        0, 0),
            OPT_BOOLEAN('m', "dpc", &dump_args[AIQ_NOTIFIER_MATCH_DPCC], "dump dpcc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('n', "merge", &dump_args[AIQ_NOTIFIER_MATCH_MERGE],
                        "dump merge module info ", NULL, 0, 0),
            OPT_BOOLEAN('o', "ccm", &dump_args[AIQ_NOTIFIER_MATCH_CCM], "dump ccm module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('p', "lsc", &dump_args[AIQ_NOTIFIER_MATCH_LSC], "dump lsc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('r', "blc", &dump_args[AIQ_NOTIFIER_MATCH_BLC], "dump blc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('V', "rawnr", &dump_args[AIQ_NOTIFIER_MATCH_RAWNR],
                        "dump rawnr module info ", NULL, 0, 0),
            OPT_BOOLEAN('t', "gic", &dump_args[AIQ_NOTIFIER_MATCH_GIC], "dump gic module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('u', "debayer", &dump_args[AIQ_NOTIFIER_MATCH_DEBAYER],
                        "dump debayer module info ", NULL, 0, 0),
            OPT_BOOLEAN('v', "lut3d", &dump_args[AIQ_NOTIFIER_MATCH_LUT3D],
                        "dump lut3d module info ", NULL, 0, 0),
            OPT_BOOLEAN('w', "dehaz", &dump_args[AIQ_NOTIFIER_MATCH_DEHAZE],
                        "dump dehaze module info ", NULL, 0, 0),
            OPT_BOOLEAN('x', "gamma", &dump_args[AIQ_NOTIFIER_MATCH_AGAMMA],
                        "dump gamma module info ", NULL, 0, 0),
            OPT_BOOLEAN('y', "degamma", &dump_args[AIQ_NOTIFIER_MATCH_ADEGAMMA],
                        "dump degamma module info ", NULL, 0, 0),
            OPT_BOOLEAN('z', "csm", &dump_args[AIQ_NOTIFIER_MATCH_CSM], "dump csm module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('B', "cgc", &dump_args[AIQ_NOTIFIER_MATCH_CGC], "dump cgc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('C', "gain", &dump_args[AIQ_NOTIFIER_MATCH_GAIN], "dump gain module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('D', "cp", &dump_args[AIQ_NOTIFIER_MATCH_CP], "dump cproc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('E', "ie", &dump_args[AIQ_NOTIFIER_MATCH_IE], "dump ie module info", NULL,
                        0, 0),
            OPT_BOOLEAN('F', "tnr", &dump_args[AIQ_NOTIFIER_MATCH_TNR], "dump tnr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('G', "ynr", &dump_args[AIQ_NOTIFIER_MATCH_YNR], "dump ynr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('H', "cnr", &dump_args[AIQ_NOTIFIER_MATCH_CNR], "dump uvnr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('I', "sharp", &dump_args[AIQ_NOTIFIER_MATCH_SHARPEN],
                        "dump sharp module info ", NULL, 0, 0),
            OPT_BOOLEAN('J', "drc", &dump_args[AIQ_NOTIFIER_MATCH_DRC], "dump drc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('K', "cac", &dump_args[AIQ_NOTIFIER_MATCH_CAC], "dump cac module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('M', "afd", &dump_args[AIQ_NOTIFIER_MATCH_AFD], "dump afd module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('N', "rgbir", &dump_args[AIQ_NOTIFIER_MATCH_RGBIR],
                        "dump rgbir module info ", NULL, 0, 0),
            OPT_BOOLEAN('O', "ldc", &dump_args[AIQ_NOTIFIER_MATCH_LDC], "dump ldc module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('P', "aestats", &dump_args[AIQ_NOTIFIER_MATCH_AESTATS],
                        "dump ae stats module info ", NULL, 0, 0),
            OPT_BOOLEAN('R', "histeq", &dump_args[AIQ_NOTIFIER_MATCH_HISTEQ],
                        "dump histeq module info ", NULL, 0, 0),
            OPT_BOOLEAN('S', "enhaz", &dump_args[AIQ_NOTIFIER_MATCH_ENH],
                        "dump enhance module info", NULL, 0, 0),
            OPT_BOOLEAN('T', "texest", &dump_args[AIQ_NOTIFIER_MATCH_TEXEST],
                        "dump texest module info ", NULL, 0, 0),
            OPT_BOOLEAN('U', "hsv", &dump_args[AIQ_NOTIFIER_MATCH_HSV], "dump hsv module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('V', "aibnr", &dump_args[AIQ_NOTIFIER_MATCH_AIBNR], "dump aibnr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('W', "aiynr", &dump_args[AIQ_NOTIFIER_MATCH_AIYNR], "dump aiynr module info",
                        NULL, 0, 0),
            OPT_BOOLEAN('\0', "help", NULL, "show this help message and exit", dbg_help_cb, 0,
                        OPT_NONEG),
            OPT_END(),
        };

        char additional_desc[1024 * 3] = {0};
        snprintf(additional_desc, sizeof(additional_desc),
                 "\n\nAdditional description of 'dumpcam hwi'.\n\n"
                 "eg., 'dumpcam hwi -A' or 'dumpcam hwi all'\n"
                 "  Dump all information.\n\n"
                 "eg., 'dumpcam hwi -b' or 'dumpcam hwi --base'\n"
                 "  Dump base information.\n\n"
                 "eg., 'dumpcam hwi -bs' or 'dumpcam hwi --base --sensor'\n"
                 "  Dump base and sensor information.\n\n");

        struct argparse argparse;
        argparse_init(&argparse, options, usages, 0);
        argparse_describe(&argparse, "\nA brief description of how to dump hwi.", additional_desc);

        argc = argparse_parse(&argparse, argc, (const char**)argv);
        if (_gHelp || argc < 0) {
            _gHelp = 0;
            argparse_usage_string(&argparse, buffer);
            aiq_string_printf(result, buffer);
        }
    }

    if (dump_args[AIQ_NOTIFIER_MATCH_HWI_ALL]) {
        for (int32_t i = AIQ_NOTIFIER_MATCH_HWI_BASE; i < AIQ_NOTIFIER_MATCH_HWI_ALL; i++)
            aiq_notifier_notify_dumpinfo(&((AiqCamHwBase_t*)pCamHw)->notifier, i, result, argc,
                                         argv);

        return true;
    }

    for (int32_t i = 0; i < AIQ_NOTIFIER_MATCH_ALL; i++) {
        if (dump_args[i])
            aiq_notifier_notify_dumpinfo(&((AiqCamHwBase_t*)pCamHw)->notifier, i, result, argc,
                                         argv);
    }

    return true;
#else
    return true;
#endif
}

static XCamReturn AiqCamHw_set_sceneCisParams(AiqCamHwBase_t* pCamHw) {
    AiqSensorHw_t* mSensor = pCamHw->_mSensorDev;
    if (!mSensor) {
        LOGE_CAMHW("set scene cis params failed, sensor is NULL");
        return XCAM_RETURN_ERROR_SENSOR;
    }

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    calibdb_cis_reg_setting_t* cis_reg = (calibdb_cis_reg_setting_t*)(CALIBDBV2_GET_MODULE_PTR(
            (void*)pCamHw->mCalibDbV2, CisRegSetting));
    if (cis_reg) {
        ret = mSensor->set_regSetting(mSensor, cis_reg);
        if (ret != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to set CIS register settings, ret: %d", ret);
    }

    calibdb_cis_blc_t* cis_blc =
        (calibdb_cis_blc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCamHw->mCalibDbV2, CisBlc));
    if (cis_blc) {
        ret = mSensor->set_blc(mSensor, cis_blc);
        if (ret != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to set CIS blc, ret: %d", ret);
    }

    calibdb_cis_hdr_t* cis_hdr =
        (calibdb_cis_hdr_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCamHw->mCalibDbV2, CisHdr));
    if (cis_hdr) {
        ret = mSensor->set_hdr(mSensor, cis_hdr);
        if (ret != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to set CIS hdr, ret: %d", ret);
    }

    calibdb_cis_qbc_rmsc_t* cis_qbcrmsc =
        (calibdb_cis_qbc_rmsc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCamHw->mCalibDbV2, CisQbcRmsc));
    if (cis_qbcrmsc) {
        ret = mSensor->set_qbcRmsc(mSensor, cis_qbcrmsc);
        if (ret != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to set CIS qbcrmsc, ret: %d", ret);
    }

    calibdb_cis_cmps_out_t* cis_cmpsout =
        (calibdb_cis_cmps_out_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCamHw->mCalibDbV2, CisCmpsOut));
    if (cis_cmpsout) {
        // ret = mSensor->set_cmpsOut(mSensor, cis_cmpsout);
        if (ret != XCAM_RETURN_NO_ERROR)
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to set CIS cmpsout, ret: %d", ret);
    }

    return ret;
}

XCamReturn AiqCamHw_getHdrComprCurve(AiqCamHwBase_t* pCamHw, RkAiqHdrCompr_t* compr) {
    AiqSensorHw_t* mSensorSubdev = pCamHw->_mSensorDev;

    return AiqSensorHw_getHdrComprCurve(mSensorSubdev, compr);
}

XCamReturn AiqCamHw_setAiBnrBufCnt(AiqCamHwBase_t* pCamHw,
                                   rk_aiq_aibnr_buffer_count_t aibnr_buf_cnt) {
#if RKAIQ_HAVE_AIBNR
    if (pCamHw->_state == CAM_HW_STATE_PREPARED || pCamHw->_state == CAM_HW_STATE_STARTED) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Can't set aibnr buf cnt in state(%d)", pCamHw->_state);
        return XCAM_RETURN_ERROR_FAILED;
    }

    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "set aibnr buf cnt to (isp_fe: %d, isp_be: %d)",
                    aibnr_buf_cnt.isp_fe, aibnr_buf_cnt.isp_be);

    pCamHw->mAibnrBufCnt = aibnr_buf_cnt;

    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
}

XCamReturn AiqCamHw_getAiBnrBufCnt(AiqCamHwBase_t* pCamHw,
                                   rk_aiq_aibnr_buffer_count_t* aibnr_buf_cnt) {
#if RKAIQ_HAVE_AIBNR
    if (!pCamHw) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "invalid params");
        return XCAM_RETURN_ERROR_PARAM;
    }

    *aibnr_buf_cnt = pCamHw->mAibnrBufCnt;
    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_FAILED;
#endif
}

XCAM_END_DECLARE
