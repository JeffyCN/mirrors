/*
 *  Copyright (c) 2021 Rockchip Corporation
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
#ifndef _MEDIA_INFO_H_
#define _MEDIA_INFO_H_

#include <v4l2_device.h>
#include <map>
#include "mediactl/mediactl-priv.h"
#include <linux/v4l2-subdev.h>
#include "xcam_thread.h"
#include "xcam_log.h"
#include "Stream.h"
#include <time.h>
#include <unistd.h>

using namespace XCam;

namespace RkRawStream {

//#define MAX_MEDIA_INDEX               16
//#define DEV_PATH_LEN                  64
//#define SENSOR_ATTACHED_FLASH_MAX_NUM 2
//#define MAX_CAM_NUM                   8

//#define MAX_ISP_LINKED_VICAP_CNT      4

/*
typedef struct {
    int  model_idx;
    int  logic_id;
    int  phy_id; // physical isp id
    int  linked_sensor;
    bool is_multi_isp_mode; // isp-unit mode, 2 isp to 1
    bool isMultiplex;      // muliplex mode, virtually sed by more than one sensor
    bool linked_dvp;
    bool valid;
    char media_dev_path[DEV_PATH_LEN];
    char isp_dev_path[DEV_PATH_LEN];
    char csi_dev_path[DEV_PATH_LEN];
    char mpfbc_dev_path[DEV_PATH_LEN];
    char main_path[DEV_PATH_LEN];
    char self_path[DEV_PATH_LEN];
    char rawwr0_path[DEV_PATH_LEN];
    char rawwr1_path[DEV_PATH_LEN];
    char rawwr2_path[DEV_PATH_LEN];
    char rawwr3_path[DEV_PATH_LEN];
    char dma_path[DEV_PATH_LEN];
    char rawrd0_m_path[DEV_PATH_LEN];
    char rawrd1_l_path[DEV_PATH_LEN];
    char rawrd2_s_path[DEV_PATH_LEN];
    char stats_path[DEV_PATH_LEN];
    char input_params_path[DEV_PATH_LEN];
    char mipi_luma_path[DEV_PATH_LEN];
    char mipi_dphy_rx_path[DEV_PATH_LEN];
    char linked_vicap[MAX_ISP_LINKED_VICAP_CNT][DEV_PATH_LEN];
} rk_aiq_isp_t;

typedef struct {
    int  model_idx;
    bool valid;
    char media_dev_path[DEV_PATH_LEN];
    char pp_input_image_path[DEV_PATH_LEN];
    char pp_m_bypass_path[DEV_PATH_LEN];
    char pp_scale0_path[DEV_PATH_LEN];
    char pp_scale1_path[DEV_PATH_LEN];
    char pp_scale2_path[DEV_PATH_LEN];
    char pp_input_params_path[DEV_PATH_LEN];
    char pp_stats_path[DEV_PATH_LEN];
    char pp_tnr_params_path[DEV_PATH_LEN];
    char pp_tnr_stats_path[DEV_PATH_LEN];
    char pp_nr_params_path[DEV_PATH_LEN];
    char pp_nr_stats_path[DEV_PATH_LEN];
    char pp_fec_params_path[DEV_PATH_LEN];
    char pp_dev_path[DEV_PATH_LEN];
} rk_aiq_ispp_t;

typedef struct {
    int isp_ver;
    int awb_ver;
    int aec_ver;
    int afc_ver;
    int ahdr_ver;
    int blc_ver;
    int dpcc_ver;
    int anr_ver;
    int debayer_ver;
    int lsc_ver;
    int ccm_ver;
    int gamma_ver;
    int gic_ver;
    int sharp_ver;
    int dehaze_ver;
} rk_aiq_hw_ver_t;

typedef struct {
    rk_aiq_isp_t isp_info[MAX_CAM_NUM];
    rk_aiq_ispp_t ispp_info[MAX_CAM_NUM];
    rk_aiq_hw_ver_t hw_ver_info;
} rk_aiq_isp_hw_info_t;

typedef struct {
    int  model_idx;
    char media_dev_path[DEV_PATH_LEN];
    char mipi_id0[DEV_PATH_LEN];
    char mipi_id1[DEV_PATH_LEN];
    char mipi_id2[DEV_PATH_LEN];
    char mipi_id3[DEV_PATH_LEN];
    char mipi_scl0[DEV_PATH_LEN];
    char mipi_scl1[DEV_PATH_LEN];
    char mipi_scl2[DEV_PATH_LEN];
    char mipi_scl3[DEV_PATH_LEN];
    char dvp_id0[DEV_PATH_LEN];
    char dvp_id1[DEV_PATH_LEN];
    char dvp_id2[DEV_PATH_LEN];
    char dvp_id3[DEV_PATH_LEN];
    char mipi_dphy_rx_path[DEV_PATH_LEN];
    char mipi_csi2_sd_path[DEV_PATH_LEN];
    char lvds_sd_path[DEV_PATH_LEN];
    char mipi_luma_path[DEV_PATH_LEN];
    char stream_cif_path[DEV_PATH_LEN];
    char dvp_sof_sd_path[DEV_PATH_LEN];
    char model_str[DEV_PATH_LEN];
} rk_aiq_cif_info_t;

typedef struct {
    rk_aiq_cif_info_t cif_info[MAX_CAM_NUM];
    rk_aiq_hw_ver_t hw_ver_info;
} rk_aiq_cif_hw_info_t;

typedef struct {
    bool pdaf_support;
    uint32_t pdaf_vc;
    uint32_t pdaf_width;
    uint32_t pdaf_height;
    uint32_t pdaf_pixelformat;
    uint32_t pdaf_code;
    char pdaf_vdev[DEV_PATH_LEN];
} rk_sensor_pdaf_info_t;

typedef struct {
    std::string sensor_name;
    std::string device_name;
    std::string len_name;
    std::string parent_media_dev;
    int media_node_index;
    int csi_port;
    std::string module_lens_dev_name; // matched using mPhyModuleIndex
    std::string module_ircut_dev_name;
    int flash_num;
    std::string module_flash_dev_name[SENSOR_ATTACHED_FLASH_MAX_NUM]; // matched using mPhyModuleIndex
    bool fl_strth_adj_sup;
    int flash_ir_num;
    std::string module_flash_ir_dev_name[SENSOR_ATTACHED_FLASH_MAX_NUM];
    bool fl_ir_strth_adj_sup;
    std::string module_real_sensor_name; //parsed frome sensor entity name
    std::string module_index_str; // parsed from sensor entity name
    char phy_module_orient; // parsed from sensor entity name
    std::vector<rk_frame_fmt_t>  frame_size;
    rk_aiq_isp_t *isp_info;
    rk_aiq_cif_info_t *cif_info;
    rk_aiq_ispp_t *ispp_info;
    bool linked_to_isp;
    bool dvp_itf;
    struct rkmodule_inf mod_info;
} rk_sensor_full_info_t;
*/

int8_t pixFmt2Bpp(uint32_t pixFmt);


class MediaInfo
{
public:
    MediaInfo();
    virtual ~MediaInfo();
    rk_aiq_isp_hw_info_t mIspHwInfos;
    rk_aiq_cif_hw_info_t mCifHwInfos;
    std::map<std::string, SmartPtr<rk_aiq_static_info_t>> mCamHwInfos;
    std::map<std::string, SmartPtr<rk_sensor_full_info_t>> mSensorHwInfos;
    bool mIsMultiIspMode;
    uint16_t mMultiIspExtendedPixel;
    void findAttachedSubdevs(struct media_device *device, uint32_t count, rk_sensor_full_info_t *s_info);
    XCamReturn initCamHwInfos();
    rk_aiq_static_info_t* getStaticCamHwInfo(const char* sns_ent_name, uint16_t index = 0);
    XCamReturn clearStaticCamHwInfo();
    void getCamHwEntNames(char buf[12][32]);
    rk_sensor_full_info_t* getSensorFullInfo(char* sns_ent_name, uint16_t index);
    const char* offline(int isp_index, const char* offline_sns_ent_name);
    XCamReturn setupOffLineLink(int isp_index, bool enable);
};

}
#endif
