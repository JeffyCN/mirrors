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

#include "MediaInfo.h"

namespace RkRawStream {

int8_t pixFmt2Bpp(uint32_t pixFmt)
{
    int8_t bpp;
    switch (pixFmt) {
    case V4L2_PIX_FMT_SBGGR8:
    case V4L2_PIX_FMT_SGBRG8:
    case V4L2_PIX_FMT_SGRBG8:
    case V4L2_PIX_FMT_SRGGB8:
        bpp = 8;
        break;
    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SRGGB10:
        bpp = 10;
        break;
    case V4L2_PIX_FMT_SBGGR12:
    case V4L2_PIX_FMT_SGBRG12:
    case V4L2_PIX_FMT_SGRBG12:
    case V4L2_PIX_FMT_SRGGB12:
        bpp = 12;
        break;
    case V4L2_PIX_FMT_SBGGR14:
    case V4L2_PIX_FMT_SGBRG14:
    case V4L2_PIX_FMT_SGRBG14:
    case V4L2_PIX_FMT_SRGGB14:
        bpp = 14;
        break;
    case V4L2_PIX_FMT_SBGGR16:
    case V4L2_PIX_FMT_SGBRG16:
    case V4L2_PIX_FMT_SGRBG16:
    case V4L2_PIX_FMT_SRGGB16:
        bpp = 16;
        break;
    default:
        return -1;
    }

    return bpp;
}

MediaInfo::MediaInfo (){
    LOGD_RKSTREAM ("MediaInfo constructed.");
}

MediaInfo::~MediaInfo(){
    LOGD_RKSTREAM ("~MediaInfo destructed");
}

static XCamReturn get_isp_ver(rk_aiq_isp_hw_info_t *hw_info) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct v4l2_capability cap;
    V4l2Device vdev(hw_info->isp_info[0].stats_path);


    ret = vdev.open();
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_RKSTREAM("failed to open dev (%s)", hw_info->isp_info[0].stats_path);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (vdev.query_cap(cap) == XCAM_RETURN_NO_ERROR) {
        char *p;
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
        //awb/aec version?
        vdev.close();
        return XCAM_RETURN_NO_ERROR;
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        goto out;
    }

out:
    vdev.close();
    LOGE_RKSTREAM("get isp version failed !");
    return ret;
}

static XCamReturn get_sensor_caps(rk_sensor_full_info_t *sensor_info) {
    struct v4l2_subdev_frame_size_enum fsize_enum;
    struct v4l2_subdev_mbus_code_enum  code_enum;
    std::vector<uint32_t> formats;
    rk_frame_fmt_t frameSize;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    V4l2SubDevice vdev(sensor_info->device_name.c_str());
    ret = vdev.open();
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_RKSTREAM("failed to open dev (%s)", sensor_info->device_name.c_str());
        return XCAM_RETURN_ERROR_FAILED;
    }
    //get module info
    struct rkmodule_inf *minfo = &(sensor_info->mod_info);
    if(vdev.io_control(RKMODULE_GET_MODULE_INFO, minfo) < 0) {
        LOGE_RKSTREAM("@%s %s: Get sensor module info failed", __FUNCTION__, sensor_info->device_name.c_str());
        return XCAM_RETURN_ERROR_FAILED;
    }
    sensor_info->len_name = std::string(minfo->base.lens);

    struct v4l2_subdev_frame_interval_enum fie;
    memset(&fie, 0, sizeof(fie));
    while(vdev.io_control(VIDIOC_SUBDEV_ENUM_FRAME_INTERVAL, &fie) == 0) {
        frameSize.format = (rk_aiq_format_t)fie.code;
        frameSize.width = fie.width;
        frameSize.height = fie.height;
        frameSize.fps = fie.interval.denominator / fie.interval.numerator;
        frameSize.hdr_mode = fie.reserved[0];
        sensor_info->frame_size.push_back(frameSize);
        fie.index++;
    }
    if (fie.index == 0)
        LOGE_RKSTREAM("@%s %s: Enum sensor frame interval failed", __FUNCTION__, sensor_info->device_name.c_str());
    vdev.close();

    return ret;
}

static XCamReturn
parse_module_info(rk_sensor_full_info_t *sensor_info)
{
    // sensor entity name format SHOULD be like this:
    // m00_b_ov13850 1-0010
    std::string entity_name(sensor_info->sensor_name);

    if (entity_name.empty())
        return XCAM_RETURN_ERROR_SENSOR;

    int parse_index = 0;

    if (entity_name.at(parse_index) != 'm') {
        LOGE_RKSTREAM("%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name.c_str(), parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    std::string index_str = entity_name.substr (parse_index, 3);
    sensor_info->module_index_str = index_str;

    parse_index += 3;

    if (entity_name.at(parse_index) != '_') {
        LOGE_RKSTREAM("%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name.c_str(), parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    parse_index++;

    if (entity_name.at(parse_index) != 'b' &&
            entity_name.at(parse_index) != 'f') {
        LOGE_RKSTREAM("%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name.c_str(), parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }
    sensor_info->phy_module_orient = entity_name.at(parse_index);

    parse_index++;

    if (entity_name.at(parse_index) != '_') {
        LOGE_RKSTREAM("%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name.c_str(), parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    parse_index++;

    std::size_t real_name_end = std::string::npos;
    if ((real_name_end = entity_name.find(' ')) == std::string::npos) {
        LOGE_RKSTREAM("%d:parse sensor entity name %s error at %d, please check sensor driver !",
                        __LINE__, entity_name.c_str(), parse_index);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    std::string real_name_str = entity_name.substr(parse_index, real_name_end - parse_index);
    sensor_info->module_real_sensor_name = real_name_str;

    LOGI_RKSTREAM("%s:%d, real sensor name %s, module ori %c, module id %s",
                    __FUNCTION__, __LINE__, sensor_info->module_real_sensor_name.c_str(),
                    sensor_info->phy_module_orient, sensor_info->module_index_str.c_str());

    return XCAM_RETURN_NO_ERROR;
}

static rk_aiq_ispp_t*
get_ispp_subdevs(struct media_device *device, const char *devpath, rk_aiq_ispp_t* ispp_info)
{
    media_entity *entity = NULL;
    const char *entity_name = NULL;
    int index = 0;

    if(!device || !ispp_info || !devpath)
        return NULL;

    for(index = 0; index < MAX_CAM_NUM; index++) {
        if (0 == strlen(ispp_info[index].media_dev_path))
            break;
        if (0 == strncmp(ispp_info[index].media_dev_path, devpath, sizeof(ispp_info[index].media_dev_path)-1)) {
            LOGE_RKSTREAM("isp info of path %s exists!", devpath);
            return &ispp_info[index];
        }
    }

    if (index >= MAX_CAM_NUM)
        return NULL;
#if defined(ISP_HW_V30)
    // parse driver pattern: soc:rkisp0-vir0
    int model_idx = -1;
    char* rkispp = strstr(device->info.driver, "rkispp");
    if (rkispp) {
        int ispp_idx = atoi(rkispp + strlen("rkispp"));
        char* vir = strstr(device->info.driver, "vir");
        if (vir) {
            int vir_idx = atoi(vir + strlen("vir"));
            model_idx = ispp_idx * 4 + vir_idx;
        }
    }

    if (model_idx == -1) {
        LOGE_RKSTREAM("wrong ispp media driver info: %s", device->info.driver);
        return NULL;
    }

    ispp_info[index].model_idx = model_idx;
#else

    if (strcmp(device->info.model, "rkispp0") == 0 ||
            strcmp(device->info.model, "rkispp") == 0)
        ispp_info[index].model_idx = 0;
    else if (strcmp(device->info.model, "rkispp1") == 0)
        ispp_info[index].model_idx = 1;
    else if (strcmp(device->info.model, "rkispp2") == 0)
        ispp_info[index].model_idx = 2;
    else if (strcmp(device->info.model, "rkispp3") == 0)
        ispp_info[index].model_idx = 3;
    else
        ispp_info[index].model_idx = -1;
#endif
    strncpy(ispp_info[index].media_dev_path, devpath, sizeof(ispp_info[index].media_dev_path)-1);

    entity = media_get_entity_by_name(device, "rkispp_input_image", strlen("rkispp_input_image"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_input_image_path, entity_name, sizeof(ispp_info[index].pp_input_image_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkispp_m_bypass", strlen("rkispp_m_bypass"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_m_bypass_path, entity_name, sizeof(ispp_info[index].pp_m_bypass_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkispp_scale0", strlen("rkispp_scale0"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_scale0_path, entity_name, sizeof(ispp_info[index].pp_scale0_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkispp_scale1", strlen("rkispp_scale1"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_scale1_path, entity_name, sizeof(ispp_info[index].pp_scale1_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkispp_scale2", strlen("rkispp_scale2"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_scale2_path, entity_name, sizeof(ispp_info[index].pp_scale2_path)-1);
        }
    }
    /*
    entity = media_get_entity_by_name(device, "rkispp_tnr_params", strlen("rkispp_tnr_params"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_tnr_params_path, entity_name, sizeof(ispp_info[index].pp_tnr_params_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkispp_tnr_stats", strlen("rkispp_tnr_stats"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_tnr_stats_path, entity_name, sizeof(ispp_info[index].pp_tnr_stats_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkispp_nr_params", strlen("rkispp_nr_params"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_nr_params_path, entity_name, sizeof(ispp_info[index].pp_nr_params_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkispp_nr_stats", strlen("rkispp_nr_stats"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_nr_stats_path, entity_name, sizeof(ispp_info[index].pp_nr_stats_path)-1);
        }
    }
    */
    entity = media_get_entity_by_name(device, "rkispp_fec_params", strlen("rkispp_fec_params"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_fec_params_path, entity_name, sizeof(ispp_info[index].pp_fec_params_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkispp-subdev", strlen("rkispp-subdev"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(ispp_info[index].pp_dev_path, entity_name, sizeof(ispp_info[index].pp_dev_path)-1);
        }
    }

    LOGI_RKSTREAM( "model(%s): ispp_info(%d): ispp-subdev entity name: %s\n",
                    device->info.model, index,
                    ispp_info[index].pp_dev_path);

    return &ispp_info[index];
}

static rk_aiq_isp_t*
get_isp_subdevs(struct media_device *device, const char *devpath, rk_aiq_isp_t* isp_info)
{
    media_entity *entity = NULL;
    const char *entity_name = NULL;
    int index = 0;
    if(!device || !isp_info || !devpath)
        return NULL;

    for(index = 0; index < MAX_CAM_NUM; index++) {
        if (0 == strlen(isp_info[index].media_dev_path)) {
            isp_info[index].logic_id = index;
            break;
        }
        if (0 == strncmp(isp_info[index].media_dev_path, devpath, sizeof(isp_info[index].media_dev_path)-1)) {
            LOGE_RKSTREAM("isp info of path %s exists!", devpath);
            return &isp_info[index];
        }
    }
    if (index >= MAX_CAM_NUM)
        return NULL;

#if defined(ISP_HW_V30)
    // parse driver pattern: soc:rkisp0-vir0
    int model_idx = -1;
    char* rkisp = strstr(device->info.driver, "rkisp");
    if (rkisp) {
        char* str_unite = NULL;
        str_unite = strstr(device->info.driver, "unite");
        if (str_unite) {
            model_idx = 0;
        } else {
            int isp_idx = atoi(rkisp + strlen("rkisp"));
            char* vir = strstr(device->info.driver, "vir");
            if (vir) {
                int vir_idx = atoi(vir + strlen("vir"));
                model_idx = isp_idx * 4 + vir_idx;
                isp_info[index].phy_id = isp_idx;
            }
        }
    }

    if (model_idx == -1) {
        LOGE_RKSTREAM("wrong isp media driver info: %s", device->info.driver);
        return NULL;
    }

    isp_info[index].model_idx = model_idx;
#else
    if (strcmp(device->info.model, "rkisp0") == 0 ||
            strcmp(device->info.model, "rkisp") == 0)
        isp_info[index].model_idx = 0;
    else if (strcmp(device->info.model, "rkisp1") == 0)
        isp_info[index].model_idx = 1;
    else if (strcmp(device->info.model, "rkisp2") == 0)
        isp_info[index].model_idx = 2;
    else if (strcmp(device->info.model, "rkisp3") == 0)
        isp_info[index].model_idx = 3;
    else
        isp_info[index].model_idx = -1;
#endif

    strncpy(isp_info[index].media_dev_path, devpath, sizeof(isp_info[index].media_dev_path)-1);

    entity = media_get_entity_by_name(device, "rkisp-isp-subdev", strlen("rkisp-isp-subdev"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].isp_dev_path, entity_name, sizeof(isp_info[index].isp_dev_path)-1);
        }
    }

    entity = media_get_entity_by_name(device, "rkisp-csi-subdev", strlen("rkisp-csi-subdev"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].csi_dev_path, entity_name, sizeof(isp_info[index].csi_dev_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-mpfbc-subdev", strlen("rkisp-mpfbc-subdev"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].mpfbc_dev_path, entity_name, sizeof(isp_info[index].mpfbc_dev_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_mainpath", strlen("rkisp_mainpath"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].main_path, entity_name, sizeof(isp_info[index].main_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_selfpath", strlen("rkisp_selfpath"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].self_path, entity_name, sizeof(isp_info[index].self_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawwr0", strlen("rkisp_rawwr0"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].rawwr0_path, entity_name, sizeof(isp_info[index].rawwr0_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawwr1", strlen("rkisp_rawwr1"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].rawwr1_path, entity_name, sizeof(isp_info[index].rawwr1_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawwr2", strlen("rkisp_rawwr2"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].rawwr2_path, entity_name, sizeof(isp_info[index].rawwr2_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawwr3", strlen("rkisp_rawwr3"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].rawwr3_path, entity_name, sizeof(isp_info[index].rawwr3_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_dmapath", strlen("rkisp_dmapath"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].dma_path, entity_name, sizeof(isp_info[index].dma_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawrd0_m", strlen("rkisp_rawrd0_m"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].rawrd0_m_path, entity_name, sizeof(isp_info[index].rawrd0_m_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawrd1_l", strlen("rkisp_rawrd1_l"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].rawrd1_l_path, entity_name, sizeof(isp_info[index].rawrd1_l_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp_rawrd2_s", strlen("rkisp_rawrd2_s"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].rawrd2_s_path, entity_name, sizeof(isp_info[index].rawrd2_s_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-statistics", strlen("rkisp-statistics"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].stats_path, entity_name, sizeof(isp_info[index].stats_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-input-params", strlen("rkisp-input-params"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].input_params_path, entity_name, sizeof(isp_info[index].input_params_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rkisp-mipi-luma", strlen("rkisp-mipi-luma"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].mipi_luma_path, entity_name, sizeof(isp_info[index].mipi_luma_path)-1);
        }
    }
    entity = media_get_entity_by_name(device, "rockchip-mipi-dphy-rx", strlen("rockchip-mipi-dphy-rx"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(isp_info[index].mipi_dphy_rx_path, entity_name, sizeof(isp_info[index].mipi_dphy_rx_path)-1);
        }
    } else {
        entity = media_get_entity_by_name(device, "rockchip-csi2-dphy0", strlen("rockchip-csi2-dphy0"));
        if(entity) {
            entity_name = media_entity_get_devname (entity);
            if(entity_name) {
                strncpy(isp_info[index].mipi_dphy_rx_path, entity_name, sizeof(isp_info[index].mipi_dphy_rx_path)-1);
            }
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_dvp", strlen("rkcif_dvp"));
    if(entity)
        isp_info[index].linked_dvp = true;
    else
        isp_info[index].linked_dvp = false;

    const char* linked_entity_name_strs[] = {
        "rkcif_dvp",
        "rkcif_lite_mipi_lvds",
        "rkcif_mipi_lvds",
        "rkcif_mipi_lvds1",
        "rkcif_mipi_lvds2",
        "rkcif_mipi_lvds3",
        "rkcif_mipi_lvds4",
        "rkcif_mipi_lvds5",
        "rkcif-mipi-lvds",
        "rkcif-mipi-lvds1",
        "rkcif-mipi-lvds2",
        "rkcif-mipi-lvds3",
        "rkcif-mipi-lvds4",
        "rkcif-mipi-lvds5",
        NULL
    };

    int vicap_idx = 0;
    for (int i = 0; linked_entity_name_strs[i] != NULL; i++) {
        entity = media_get_entity_by_name(device, linked_entity_name_strs[i], strlen(linked_entity_name_strs[i]));
        if (entity) {
            strncpy(isp_info[index].linked_vicap[vicap_idx], entity->info.name, sizeof(isp_info[index].linked_vicap[vicap_idx]));
            isp_info[index].linked_sensor = true;
            if (vicap_idx++ >= MAX_ISP_LINKED_VICAP_CNT) {
                break;
            }
        }
    }

    LOGI_RKSTREAM("model(%s): isp_info(%d): ispp-subdev entity name: %s\n",
                    device->info.model, index,
                    isp_info[index].isp_dev_path);

    return &isp_info[index];
}

static rk_aiq_cif_info_t*
get_cif_subdevs(struct media_device *device, const char *devpath, rk_aiq_cif_info_t* cif_info)
{
    media_entity *entity = NULL;
    const char *entity_name = NULL;
    int index = 0;
    if(!device || !devpath || !cif_info)
        return NULL;

    for(index = 0; index < MAX_CAM_NUM; index++) {
        if (0 == strlen(cif_info[index].media_dev_path))
            break;
        if (0 == strncmp(cif_info[index].media_dev_path, devpath, sizeof(cif_info[index].media_dev_path)-1)) {
            LOGE_RKSTREAM("isp info of path %s exists!", devpath);
            return &cif_info[index];
        }
    }
    if (index >= MAX_CAM_NUM)
        return NULL;

    cif_info[index].model_idx = index;

    strncpy(cif_info[index].media_dev_path, devpath, sizeof(cif_info[index].media_dev_path)-1 - 1);

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id0", strlen("stream_cif_mipi_id0"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_id0, entity_name, sizeof(cif_info[index].mipi_id0) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id1", strlen("stream_cif_mipi_id1"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_id1, entity_name, sizeof(cif_info[index].mipi_id1) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id2", strlen("stream_cif_mipi_id2"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_id2, entity_name, sizeof(cif_info[index].mipi_id2) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_mipi_id3", strlen("stream_cif_mipi_id3"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_id3, entity_name, sizeof(cif_info[index].mipi_id3) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_scale_ch0", strlen("rkcif_scale_ch0"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_scl0, entity_name, sizeof(cif_info[index].mipi_scl0) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_scale_ch1", strlen("rkcif_scale_ch1"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_scl1, entity_name, sizeof(cif_info[index].mipi_scl1) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_scale_ch2", strlen("rkcif_scale_ch2"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_scl2, entity_name, sizeof(cif_info[index].mipi_scl2) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif_scale_ch3", strlen("rkcif_scale_ch3"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_scl3, entity_name, sizeof(cif_info[index].mipi_scl3) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_dvp_id0", strlen("stream_cif_dvp_id0"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].dvp_id0, entity_name, sizeof(cif_info[index].dvp_id0) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_dvp_id1", strlen("stream_cif_dvp_id1"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].dvp_id1, entity_name, sizeof(cif_info[index].dvp_id1) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_dvp_id2", strlen("stream_cif_dvp_id2"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].dvp_id2, entity_name, sizeof(cif_info[index].dvp_id2) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif_dvp_id3", strlen("stream_cif_dvp_id3"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].dvp_id3, entity_name, sizeof(cif_info[index].dvp_id3) - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif-mipi-luma", strlen("rkisp-mipi-luma"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_luma_path, entity_name, sizeof(cif_info[index].mipi_luma_path)-1 - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rockchip-mipi-csi2", strlen("rockchip-mipi-csi2"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_csi2_sd_path, entity_name, sizeof(cif_info[index].mipi_csi2_sd_path)-1 - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif-lvds-subdev", strlen("rkcif-lvds-subdev"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].lvds_sd_path, entity_name, sizeof(cif_info[index].lvds_sd_path)-1 - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif-lite-lvds-subdev", strlen("rkcif-lite-lvds-subdev"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].lvds_sd_path, entity_name, sizeof(cif_info[index].lvds_sd_path)-1 - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rockchip-mipi-dphy-rx", strlen("rockchip-mipi-dphy-rx"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].mipi_dphy_rx_path, entity_name, sizeof(cif_info[index].mipi_dphy_rx_path)-1 - 1);
        }
    } else {
        entity = media_get_entity_by_name(device, "rockchip-csi2-dphy0", strlen("rockchip-csi2-dphy0"));
        if(entity) {
            entity_name = media_entity_get_devname (entity);
            if(entity_name) {
                strncpy(cif_info[index].mipi_dphy_rx_path, entity_name, sizeof(cif_info[index].mipi_dphy_rx_path)-1 - 1);
            }
        }
    }

    entity = media_get_entity_by_name(device, "stream_cif", strlen("stream_cif"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].stream_cif_path, entity_name, sizeof(cif_info[index].stream_cif_path)-1 - 1);
        }
    }

    entity = media_get_entity_by_name(device, "rkcif-dvp-sof", strlen("rkcif-dvp-sof"));
    if(entity) {
        entity_name = media_entity_get_devname (entity);
        if(entity_name) {
            strncpy(cif_info[index].dvp_sof_sd_path, entity_name, sizeof(cif_info[index].dvp_sof_sd_path)-1 - 1);
        }
    }

    return &cif_info[index];
}

static
XCamReturn
SensorInfoCopy(rk_sensor_full_info_t *finfo, rk_aiq_static_info_t *info) {
    int fs_num, i = 0;
    rk_aiq_sensor_info_t *sinfo = NULL;

    //info->media_node_index = finfo->media_node_index;
    strncpy(info->lens_info.len_name, finfo->len_name.c_str(), sizeof(info->lens_info.len_name)-1);
    sinfo = &info->sensor_info;
    strncpy(sinfo->sensor_name, finfo->sensor_name.c_str(), sizeof(sinfo->sensor_name)-1);
    fs_num = finfo->frame_size.size();
    if (fs_num) {
        for (auto iter = finfo->frame_size.begin(); iter != finfo->frame_size.end() && i < 10; ++iter, i++) {
            sinfo->support_fmt[i].width = (*iter).width;
            sinfo->support_fmt[i].height = (*iter).height;
            sinfo->support_fmt[i].format = (*iter).format;
            sinfo->support_fmt[i].fps = (*iter).fps;
            sinfo->support_fmt[i].hdr_mode = (*iter).hdr_mode;
        }
        sinfo->num = i;
    }

    if (finfo->module_index_str.size()) {
        sinfo->phyId = atoi(finfo->module_index_str.c_str() + 1);
    } else {
        sinfo->phyId = -1;
    }

    return XCAM_RETURN_NO_ERROR;
}

rk_aiq_static_info_t*
MediaInfo::getStaticCamHwInfo(const char* sns_ent_name, uint16_t index)
{
    std::map<std::string, SmartPtr<rk_aiq_static_info_t>>::iterator it;

    if (sns_ent_name) {
        std::string str(sns_ent_name);

        it = mCamHwInfos.find(str);
        if (it != mCamHwInfos.end()) {
            LOGD_RKSTREAM("find camerainfo of %s!", sns_ent_name);
            return it->second.ptr();
        } else {
            LOGE_RKSTREAM("camerainfo of %s not fount!", sns_ent_name);
        }
    } else {
        if (index >= 0 && index < mCamHwInfos.size()) {
            int i = 0;
            for (it = mCamHwInfos.begin(); it != mCamHwInfos.end(); it++, i++) {
                if (i == index)
                    return it->second.ptr();
            }
        }
    }

    return NULL;
}

rk_sensor_full_info_t*
MediaInfo::getSensorFullInfo(char* sns_ent_name, uint16_t index)
{
    std::map<std::string, SmartPtr<rk_sensor_full_info_t>>::iterator it;
    if (sns_ent_name) {
        std::string str(sns_ent_name);

        it = mSensorHwInfos.find(str);
        if (it != mSensorHwInfos.end()) {
            LOGD_RKSTREAM("find camerainfo of %s!", sns_ent_name);
            return it->second.ptr();
        } else {
            LOGE_RKSTREAM("camerainfo of %s not fount!", sns_ent_name);
        }
    } else {
        if (index >= 0 && index < mSensorHwInfos.size()) {
            int i = 0;
            for (it = mSensorHwInfos.begin(); it != mSensorHwInfos.end(); it++, i++) {
                if (i == index)
                    return it->second.ptr();
            }
        }
    }

    return NULL;
}

XCamReturn
MediaInfo::clearStaticCamHwInfo()
{
    mCamHwInfos.clear();
    mSensorHwInfos.clear();

    return XCAM_RETURN_NO_ERROR;
}

void
MediaInfo::findAttachedSubdevs(struct media_device *device, uint32_t count, rk_sensor_full_info_t *s_info)
{
    const struct media_entity_desc *entity_info = NULL;
    struct media_entity *entity = NULL;
    uint32_t k;

    for (k = 0; k < count; ++k) {
        entity = media_get_entity (device, k);
        entity_info = media_entity_get_info(entity);
        if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_LENS)) {
            if ((entity_info->name[0] == 'm') &&
                    (strncmp(entity_info->name, s_info->module_index_str.c_str(), 3) == 0)) {
                if (entity_info->flags == 1)
                    s_info->module_ircut_dev_name = std::string(media_entity_get_devname(entity));
                else//vcm
                    s_info->module_lens_dev_name = std::string(media_entity_get_devname(entity));
            }
        } else if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_FLASH)) {
            if ((entity_info->name[0] == 'm') &&
                    (strncmp(entity_info->name, s_info->module_index_str.c_str(), 3) == 0)) {

                /* check if entity name has the format string mxx_x_xxx-irxxx */
                if (strstr(entity_info->name, "-ir") != NULL) {
                    s_info->module_flash_ir_dev_name[s_info->flash_ir_num++] =
                        std::string(media_entity_get_devname(entity));
                } else
                    s_info->module_flash_dev_name[s_info->flash_num++] =
                        std::string(media_entity_get_devname(entity));
            }
        }
    }
    // query flash infos
    /*
    if (s_info->flash_num) {
        SmartPtr<FlashLightHw> fl = new FlashLightHw(s_info->module_flash_dev_name, s_info->flash_num);
        fl->init(1);
        s_info->fl_strth_adj_sup = fl->isStrengthAdj();
        fl->deinit();
    }
    if (s_info->flash_ir_num) {
        SmartPtr<FlashLightHw> fl_ir = new FlashLightHw(s_info->module_flash_ir_dev_name, s_info->flash_ir_num);
        fl_ir->init(1);
        s_info->fl_ir_strth_adj_sup = fl_ir->isStrengthAdj();
        fl_ir->deinit();
    }
    */
}

XCamReturn
MediaInfo::initCamHwInfos()
{
    char sys_path[64], devpath[32];
    FILE *fp = NULL;
    struct media_device *device = NULL;
    int nents, j = 0, i = 0, node_index = 0;
    const struct media_entity_desc *entity_info = NULL;
    struct media_entity *entity = NULL;

    xcam_mem_clear (mIspHwInfos);
    xcam_mem_clear (mCifHwInfos);

    while (i < MAX_MEDIA_INDEX) {
        node_index = i;
        snprintf (sys_path, 64, "/dev/media%d", i++);

        fp = fopen (sys_path, "r");
        if (!fp)
            continue;
        fclose (fp);
        device = media_device_new (sys_path);
        if (!device) {
            continue;
        }

        /* Enumerate entities, pads and links. */
        media_device_enumerate (device);

        rk_aiq_isp_t* isp_info = NULL;
        rk_aiq_cif_info_t* cif_info = NULL;
        bool dvp_itf = false;
        if (strcmp(device->info.model, "rkispp0") == 0 ||
                strcmp(device->info.model, "rkispp1") == 0 ||
                strcmp(device->info.model, "rkispp2") == 0 ||
                strcmp(device->info.model, "rkispp3") == 0 ||
                strcmp(device->info.model, "rkispp") == 0) {
            rk_aiq_ispp_t* ispp_info = get_ispp_subdevs(device, sys_path, mIspHwInfos.ispp_info);
            if (ispp_info)
                ispp_info->valid = true;
            goto media_unref;
        } else if (strcmp(device->info.model, "rkisp0") == 0 ||
                   strcmp(device->info.model, "rkisp1") == 0 ||
                   strcmp(device->info.model, "rkisp2") == 0 ||
                   strcmp(device->info.model, "rkisp3") == 0 ||
                   strcmp(device->info.model, "rkisp") == 0) {
            isp_info = get_isp_subdevs(device, sys_path, mIspHwInfos.isp_info);
            if (strstr(device->info.driver, "rkisp-unite")) {
                isp_info->is_multi_isp_mode = true;
                mIsMultiIspMode = true;
                mMultiIspExtendedPixel = RKMOUDLE_UNITE_EXTEND_PIXEL;
            } else {
                isp_info->is_multi_isp_mode = false;
                mIsMultiIspMode = false;
                mMultiIspExtendedPixel = 0;
            }
            isp_info->valid = true;
        } else if (strcmp(device->info.model, "rkcif") == 0 ||
                   strcmp(device->info.model, "rkcif_dvp") == 0 ||
                   strstr(device->info.model, "rkcif_mipi_lvds") ||
                   strstr(device->info.model, "rkcif-mipi-lvds") ||
                   strcmp(device->info.model, "rkcif_lite_mipi_lvds") == 0) {
            cif_info = get_cif_subdevs(device, sys_path, mCifHwInfos.cif_info);
            strncpy(cif_info->model_str, device->info.model, sizeof(cif_info->model_str));

            if (strcmp(device->info.model, "rkcif_dvp") == 0)
                dvp_itf = true;
        } else {
            goto media_unref;
        }

        nents = media_get_entities_count (device);
        for (j = 0; j < nents; ++j) {
            entity = media_get_entity (device, j);
            entity_info = media_entity_get_info(entity);
            if ((NULL != entity_info) && (entity_info->type == MEDIA_ENT_T_V4L2_SUBDEV_SENSOR)) {
                rk_aiq_static_info_t *info = new rk_aiq_static_info_t();
                rk_sensor_full_info_t *s_full_info = new rk_sensor_full_info_t();
                s_full_info->media_node_index = node_index;
                strncpy(devpath, media_entity_get_devname(entity), sizeof(devpath)-1);
                s_full_info->device_name = std::string(devpath);
                s_full_info->sensor_name = std::string(entity_info->name);
                s_full_info->parent_media_dev = std::string(sys_path);
                parse_module_info(s_full_info);
                get_sensor_caps(s_full_info);

                if (cif_info) {
                    s_full_info->linked_to_isp = false;
                    s_full_info->cif_info = cif_info;
                    s_full_info->isp_info = NULL;
                    s_full_info->dvp_itf = dvp_itf;
                } else if (isp_info) {
                    s_full_info->linked_to_isp = true;
                    isp_info->linked_sensor = true;
                    isp_info->isMultiplex = false;
                    s_full_info->isp_info = isp_info;
                } else {
                    LOGE_RKSTREAM("sensor device mount error!\n");
                }

                findAttachedSubdevs(device, nents, s_full_info);
                SensorInfoCopy(s_full_info, info);
                info->has_lens_vcm = s_full_info->module_lens_dev_name.empty() ? false : true;
                info->has_fl = s_full_info->flash_num > 0 ? true : false;
                info->has_irc = s_full_info->module_ircut_dev_name.empty() ? false : true;
                info->fl_strth_adj_sup = s_full_info->fl_ir_strth_adj_sup;
                info->fl_ir_strth_adj_sup = s_full_info->fl_ir_strth_adj_sup;
                if (s_full_info->isp_info)
                    info->is_multi_isp_mode = s_full_info->isp_info->is_multi_isp_mode;
                info->multi_isp_extended_pixel = mMultiIspExtendedPixel;
                LOGD_RKSTREAM("Init sensor %s with Multi-ISP Mode:%d Extended Pixels:%d ",
                                s_full_info->sensor_name.c_str(), info->is_multi_isp_mode,
                                info->multi_isp_extended_pixel);
                mSensorHwInfos[s_full_info->sensor_name] = s_full_info;
                mCamHwInfos[s_full_info->sensor_name] = info;
            }
        }

media_unref:
        media_device_unref (device);
    }

    // judge isp if multiplex by multiple cams
    rk_aiq_isp_t* isp_info = NULL;
    for (i = 0; i < MAX_CAM_NUM; i++) {
        isp_info = &mIspHwInfos.isp_info[i];
        if (isp_info->valid) {
            for (j = i - 1; j >= 0; j--) {
                if (isp_info->phy_id == mIspHwInfos.isp_info[j].phy_id) {
                    isp_info->isMultiplex = true;
                    mIspHwInfos.isp_info[j].isMultiplex = true;
                }
            }
        }
    }

    std::map<std::string, SmartPtr<rk_sensor_full_info_t>>::iterator iter;
    for(iter = mSensorHwInfos.begin(); \
            iter != mSensorHwInfos.end(); iter++) {
        LOGI_RKSTREAM("match the sensor_name(%s) media link\n", (iter->first).c_str());
        SmartPtr<rk_sensor_full_info_t> s_full_info = iter->second;

        /*
         * The ISP and ISPP match links through the media device model
         */
        if (s_full_info->linked_to_isp) {
            for (i = 0; i < MAX_CAM_NUM; i++) {
                LOGI_RKSTREAM("isp model_idx: %d, ispp(%d) model_idx: %d\n",
                                s_full_info->isp_info->model_idx,
                                i,
                                mIspHwInfos.ispp_info[i].model_idx);

                if (mIspHwInfos.ispp_info[i].valid &&
                        (s_full_info->isp_info->model_idx == mIspHwInfos.ispp_info[i].model_idx)) {
                    s_full_info->ispp_info = &mIspHwInfos.ispp_info[i];
                    LOGI_RKSTREAM("isp(%d) link to ispp(%d)\n",
                                    s_full_info->isp_info->model_idx,
                                    mIspHwInfos.ispp_info[i].model_idx);
                    mCamHwInfos[s_full_info->sensor_name]->sensor_info.binded_strm_media_idx =
                        atoi(s_full_info->ispp_info->media_dev_path + strlen("/dev/media"));
                    LOGI_RKSTREAM("sensor %s adapted to pp media %d:%s\n",
                                    s_full_info->sensor_name.c_str(),
                                    mCamHwInfos[s_full_info->sensor_name]->sensor_info.binded_strm_media_idx,
                                    s_full_info->ispp_info->media_dev_path);
                    break;
                }
            }
        } else {
            /*
             * Determine which isp that vipCap is linked
             */
            for (i = 0; i < MAX_CAM_NUM; i++) {
                rk_aiq_isp_t* isp_info = &mIspHwInfos.isp_info[i];

                for (int vicap_idx = 0; vicap_idx < MAX_ISP_LINKED_VICAP_CNT; vicap_idx++) {
                    //LOGI_RKSTREAM("vicap %s, linked_vicap %s",
                    //                s_full_info->cif_info->model_str, isp_info->linked_vicap[vicap_idx]);
                    if (strcmp(s_full_info->cif_info->model_str, isp_info->linked_vicap[vicap_idx]) == 0) {
                        s_full_info->isp_info = &mIspHwInfos.isp_info[i];
                        mCamHwInfos[s_full_info->sensor_name]->is_multi_isp_mode =
                            s_full_info->isp_info->is_multi_isp_mode;
                        mCamHwInfos[s_full_info->sensor_name]
                        ->multi_isp_extended_pixel = mMultiIspExtendedPixel;
                        if (mIspHwInfos.ispp_info[i].valid)
                            s_full_info->ispp_info = &mIspHwInfos.ispp_info[i];
                        LOGI_RKSTREAM("vicap link to isp(%d) to ispp(%d)\n",
                                        s_full_info->isp_info->model_idx,
                                        s_full_info->ispp_info ? s_full_info->ispp_info->model_idx : -1);
                        mCamHwInfos[s_full_info->sensor_name]->sensor_info.binded_strm_media_idx =
                            s_full_info->ispp_info ? atoi(s_full_info->ispp_info->media_dev_path + strlen("/dev/media")) :
                            -1;
                        LOGI_RKSTREAM("sensor %s adapted to pp media %d:%s\n",
                                        s_full_info->sensor_name.c_str(),
                                        mCamHwInfos[s_full_info->sensor_name]->sensor_info.binded_strm_media_idx,
                                        s_full_info->ispp_info ? s_full_info->ispp_info->media_dev_path : "null");
                        mIspHwInfos.isp_info[i].linked_sensor = true;
                    }
                }
            }
        }
        if (!s_full_info->isp_info/* || !s_full_info->ispp_info*/) {
            LOGE_RKSTREAM("get isp or ispp info fail, something gos wrong!");
        } else {
            //mCamHwInfos[s_full_info->sensor_name]->linked_isp_info = *s_full_info->isp_info;
            //mCamHwInfos[s_full_info->sensor_name]->linked_ispp_info = *s_full_info->ispp_info;
        }
    }

    /* Look for free isp&ispp link to fake camera */
    for (i = 0; i < MAX_CAM_NUM; i++) {
        if (mIspHwInfos.isp_info[i].valid &&
                !mIspHwInfos.isp_info[i].linked_sensor) {
            rk_aiq_static_info_t *hwinfo = new rk_aiq_static_info_t();
            rk_sensor_full_info_t *fullinfo = new rk_sensor_full_info_t();

            fullinfo->isp_info = &mIspHwInfos.isp_info[i];
            if (mIspHwInfos.ispp_info[i].valid) {
                fullinfo->ispp_info = &mIspHwInfos.ispp_info[i];
                hwinfo->sensor_info.binded_strm_media_idx =
                    atoi(fullinfo->ispp_info->media_dev_path + strlen("/dev/media"));
            }
            fullinfo->media_node_index = -1;
            fullinfo->device_name = std::string("/dev/null");
            fullinfo->sensor_name = std::string("FakeCamera");
            fullinfo->sensor_name += std::to_string(i);
            fullinfo->parent_media_dev = std::string("/dev/null");
            fullinfo->linked_to_isp = true;

            hwinfo->sensor_info.support_fmt[0].hdr_mode = NO_HDR;
            hwinfo->sensor_info.support_fmt[1].hdr_mode = HDR_X2;
            hwinfo->sensor_info.support_fmt[2].hdr_mode = HDR_X3;
            hwinfo->sensor_info.num = 3;
            mIspHwInfos.isp_info[i].linked_sensor = true;

            SensorInfoCopy(fullinfo, hwinfo);
            hwinfo->has_lens_vcm = false;
            hwinfo->has_fl = false;
            hwinfo->has_irc = false;
            hwinfo->fl_strth_adj_sup = 0;
            hwinfo->fl_ir_strth_adj_sup = 0;
            hwinfo->is_multi_isp_mode        = fullinfo->isp_info->is_multi_isp_mode;
            hwinfo->multi_isp_extended_pixel = mMultiIspExtendedPixel;
            mSensorHwInfos[fullinfo->sensor_name] = fullinfo;
            mCamHwInfos[fullinfo->sensor_name] = hwinfo;
            LOGI_RKSTREAM("fake camera %d link to isp(%d) to ispp(%d)\n",
                            i,
                            fullinfo->isp_info->model_idx,
                            fullinfo->ispp_info ? fullinfo->ispp_info->model_idx : -1);
            LOGI_RKSTREAM("sensor %s adapted to pp media %d:%s\n",
                            fullinfo->sensor_name.c_str(),
                            mCamHwInfos[fullinfo->sensor_name]->sensor_info.binded_strm_media_idx,
                            fullinfo->ispp_info ? fullinfo->ispp_info->media_dev_path : "null");
        }
    }
    get_isp_ver(&mIspHwInfos);
    for (auto &item : mCamHwInfos)
        item.second->isp_hw_ver = mIspHwInfos.hw_ver_info.isp_ver;
    return XCAM_RETURN_NO_ERROR;

}

void MediaInfo::getCamHwEntNames(char buf[12][32])
{
    int i = 0;
    for(auto it=mSensorHwInfos.begin(); it!=mSensorHwInfos.end(); ++it){
        strcpy(buf[i], it->first.c_str());
        i++;
        //printf("printStaticCamHwInfo %s\n",it->first.c_str());
    }
}

const char *
MediaInfo::offline(int isp_index, const char* offline_sns_ent_name)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    std::map<std::string, SmartPtr<rk_sensor_full_info_t> >::iterator iter;
    std::map<std::string, SmartPtr<rk_aiq_static_info_t> >::iterator iter_info;
    for (iter = mSensorHwInfos.begin(); \
            iter != mSensorHwInfos.end(); iter++) {
        rk_sensor_full_info_t *s_full_info_f = iter->second.ptr();
        if (s_full_info_f->isp_info) {
            if (s_full_info_f->isp_info->model_idx != isp_index) {
                continue;
            }
            rk_sensor_full_info_t *s_full_info = new rk_sensor_full_info_t;
            *s_full_info = *s_full_info_f;
            iter_info = mCamHwInfos.find(iter->first);
            if (iter_info == mCamHwInfos.end()) {
                continue;
            }
            rk_aiq_static_info_t *cam_hw_info_f = iter_info->second.ptr();
            rk_aiq_static_info_t *cam_hw_info = new rk_aiq_static_info_t;
            *cam_hw_info = *cam_hw_info_f;
            char sensor_name_real[64];
            std::string sns_fake_name;
            if (!strstr(const_cast<char*>(s_full_info->sensor_name.c_str()), offline_sns_ent_name)) {
                int module_index = 0;
                std::map<std::string, SmartPtr<rk_sensor_full_info_t> >::iterator sns_it;
                for (sns_it = mSensorHwInfos.begin(); \
                        sns_it != mSensorHwInfos.end(); sns_it++) {
                    rk_sensor_full_info_t *sns_full= sns_it->second.ptr();
                    if (strstr(sns_full->sensor_name.c_str(), "_s_")) {
                        int sns_index = atoi(sns_full->sensor_name.c_str() + 2);
                        if (module_index <= sns_index) {
                            module_index = sns_index + 1;
                        }
                    }
                }
                std::string tmp_sensor_name = s_full_info_f->sensor_name;
                s_full_info->module_real_sensor_name = (std::string)offline_sns_ent_name;
                s_full_info->phy_module_orient = 's';
                memset(sensor_name_real, 0, sizeof(sensor_name_real));
                sprintf(sensor_name_real, "%s%d%s%s%s", "m0", module_index, "_s_",
                                                    s_full_info->module_real_sensor_name.c_str(),
                                                    " 1-111a");
                sns_fake_name = sensor_name_real;
                s_full_info->sensor_name = sns_fake_name;
                LOGD_RKSTREAM("sns_fake_name:%s origin:%s\n", s_full_info->sensor_name.c_str(), s_full_info_f->sensor_name.c_str());
                mSensorHwInfos[s_full_info->sensor_name] = s_full_info;
                mCamHwInfos[s_full_info->sensor_name] = cam_hw_info;
                mSensorHwInfos.erase(tmp_sensor_name);
                mCamHwInfos.erase(tmp_sensor_name);
                iter_info++;
            } else {
                std::string tmp_sensor_name = s_full_info_f->sensor_name;
                s_full_info->phy_module_orient = 's';
                memset(sensor_name_real, 0, sizeof(sensor_name_real));
                sprintf(sensor_name_real, "%s%s%s%s", s_full_info->module_index_str.c_str(), "_s_",
                                                    s_full_info->module_real_sensor_name.c_str(),
                                                    " 1-111a");
                sns_fake_name = sensor_name_real;
                s_full_info->sensor_name = sns_fake_name;
                LOGD_RKSTREAM("sns_fake_name:%s %s\n", s_full_info->sensor_name.c_str(), s_full_info_f->sensor_name.c_str());
                mSensorHwInfos[s_full_info->sensor_name] = s_full_info;
                mCamHwInfos[s_full_info->sensor_name] = cam_hw_info;
                mSensorHwInfos.erase(tmp_sensor_name);
                mCamHwInfos.erase(tmp_sensor_name);
                iter_info++;
            }
            return mSensorHwInfos[s_full_info->sensor_name]->sensor_name.c_str();
        }
    }
    printf("offline preInit failed\n");
    return NULL;
}

XCamReturn
MediaInfo::setupOffLineLink(int isp_index, bool enable)
{
    media_device* device  = NULL;
    media_entity* entity  = NULL;
    media_pad* src_pad    = NULL;
    media_pad* sink_pad   = NULL;
    int lvds_max_entities = 6;
    int lvds_entity       = 0;

    device = media_device_new(mIspHwInfos.isp_info[isp_index].media_dev_path);
    if (!device) return XCAM_RETURN_ERROR_FAILED;

    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);
    entity = media_get_entity_by_name(device, "rkisp-isp-subdev", strlen("rkisp-isp-subdev"));
    if (!entity) {
        goto FAIL;
    }

    sink_pad = (media_pad*)media_entity_get_pad(entity, 0);
    if (!sink_pad) {
        LOGE_RKSTREAM("get rkisp-isp-subdev sink pad failed!\n");
        goto FAIL;
    }

    for (lvds_entity = 0; lvds_entity < lvds_max_entities; lvds_entity++) {
        char entity_name[128] = {0};
        src_pad               = NULL;
        if (!lvds_entity) {
          snprintf(entity_name, 128, "rkcif-mipi-lvds");
        } else {
          snprintf(entity_name, 128, "rkcif-mipi-lvds%d", lvds_entity);
        }
        entity = media_get_entity_by_name(device, entity_name, strlen(entity_name));
        if (entity) {
            src_pad = (media_pad*)media_entity_get_pad(entity, 0);
            if (!src_pad) {
                LOGE_RKSTREAM("get rkcif-mipi-lvds%d source pad s failed!\n",
                                lvds_entity);
                goto FAIL;
            }
        }

        if (src_pad && sink_pad) {
            if (enable) {
                media_setup_link(device, src_pad, sink_pad, 0);
            } else {
                media_setup_link(device, src_pad, sink_pad, MEDIA_LNK_FL_ENABLED);
            }
        }
    }

    entity = media_get_entity_by_name(device, "rkisp_rawrd2_s", strlen("rkisp_rawrd2_s"));
    if (entity) {
        src_pad = (media_pad*)media_entity_get_pad(entity, 0);
        if (src_pad) {
            if (enable) {
                media_setup_link(device, src_pad, sink_pad, MEDIA_LNK_FL_ENABLED);
            } else {
                media_setup_link(device, src_pad, sink_pad, 0);
            }
        }
    }

    media_device_unref(device);
    return XCAM_RETURN_NO_ERROR;
FAIL:
    media_device_unref(device);
    return XCAM_RETURN_ERROR_FAILED;
}

}
