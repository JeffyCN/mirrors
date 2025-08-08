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

#ifndef _AIQ_CAMHW_C_H_
#define _AIQ_CAMHW_C_H_

#include "rk_aiq_types.h"
#include "common/rk-aiisp-config.h"

#define ISP20HW_SUBM (0x1)

typedef struct AiqCamHwBase_s AiqCamHwBase_t;
typedef struct AiqLensHw_s AiqLensHw_t;
typedef struct AiqSensorHw_s AiqSensorHw_t;
typedef struct aiq_VideoBuffer_s aiq_VideoBuffer_t;

struct rk_aiq_vbuf_info {
    uint32_t frame_id;
    uint32_t timestamp;
    float exp_time;
    float exp_gain;
    uint32_t exp_time_reg;
    uint32_t exp_gain_reg;
    uint32_t data_fd;
    uint8_t* data_addr;
    uint32_t data_length;
    rk_aiq_rawbuf_type_t buf_type;
    bool valid;
};

struct rk_aiq_vbuf {
    void* base_addr;
    uint32_t frame_width;
    uint32_t frame_height;
    struct rk_aiq_vbuf_info buf_info[3]; /*index: 0-short,1-medium,2-long*/
};

typedef struct rk_aiq_tx_info_s {
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t bayer_fmt;
    uint32_t stridePerLine;
    uint32_t bytesPerLine;
    bool storage_type;
    uint32_t id;
    // get from AE
    bool IsAeConverged;
    bool envChange;
    void* data_addr;
    RKAiqAecExpInfo_t* aecExpInfo;
} rk_aiq_tx_info_t;

#define MAX_MEDIA_INDEX               16
#define DEV_PATH_LEN                  32
#define SENSOR_ATTACHED_FLASH_MAX_NUM 2
#define MAX_CAM_NUM                   8

#define MAX_ISP_LINKED_VICAP_CNT 5

#define ISP_TX_BUF_NUM 4
#define VIPCAP_TX_BUF_NUM      4
#define AIRMS_VIPCAP_TX_BUF_NUM      3
#define VIPCAP_TX_BUF_NUM_1608 6  // For mount 3 sensor, is mount 4 sensor, is 7

typedef struct {
    char driver[16];
    int model_idx;
    int logic_id;
    int phy_id;  // physical isp id
    int linked_sensor;
    bool is_multi_isp_mode;  // isp-unit mode, 2 isp to 1
    bool isMultiplex;        // muliplex mode, virtually sed by more than one sensor
    bool linked_dvp;
    bool valid;
    char media_dev_path[DEV_PATH_LEN];
    char isp_dev_path[DEV_PATH_LEN];
    char csi_dev_path[DEV_PATH_LEN];
    char mpfbc_dev_path[DEV_PATH_LEN];
    char main_path[DEV_PATH_LEN];
    char self_path[DEV_PATH_LEN];
    char ldc_path[DEV_PATH_LEN];
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
    char linked_vicap_sd_path[DEV_PATH_LEN];
    char pdaf_path[DEV_PATH_LEN];
} rk_aiq_isp_t;

typedef struct {
    int model_idx;
    bool valid;
    char media_dev_path[DEV_PATH_LEN];
    char pp_input_image_path[DEV_PATH_LEN];
    char pp_m_bypass_path[DEV_PATH_LEN];
    char pp_scale0_path[DEV_PATH_LEN];
    char pp_scale1_path[DEV_PATH_LEN];
    char pp_scale2_path[DEV_PATH_LEN];
#if defined(ISP_HW_V20)
    char pp_input_params_path[DEV_PATH_LEN];
    char pp_stats_path[DEV_PATH_LEN];
    char pp_tnr_params_path[DEV_PATH_LEN];
    char pp_tnr_stats_path[DEV_PATH_LEN];
    char pp_nr_params_path[DEV_PATH_LEN];
    char pp_nr_stats_path[DEV_PATH_LEN];
#endif
    char pp_fec_params_path[DEV_PATH_LEN];
    char pp_dev_path[DEV_PATH_LEN];
} rk_aiq_ispp_t;

typedef struct {
    char driver[16];
    int model_idx;
    int logic_id;
    int phy_id;
    bool valid;
    char media_dev_path[DEV_PATH_LEN];
    char video_path[DEV_PATH_LEN];
    char model_str[DEV_PATH_LEN];
    char subdev_path[DEV_PATH_LEN];
} rk_aiq_aiisp_info_t;

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
    int model_idx;
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
    rk_aiq_aiisp_info_t aiisp_info[MAX_CAM_NUM];
    rk_aiq_hw_ver_t hw_ver_info;
} rk_aiq_aiisp_hw_info_t;

typedef struct {
    char sns_name[32];
    PdafSensorType_t pdaf_type;
    bool pdaf_support;
    uint32_t pdaf_vc;
    uint32_t pdaf_width;
    uint32_t pdaf_height;
    uint32_t pdaf_pixelformat;
    uint32_t pdaf_code;
    uint8_t pdaf_lrdiffline;
    char pdaf_vdev[DEV_PATH_LEN];
} rk_sensor_pdaf_info_t;

typedef struct rk_sensor_full_info_s {
    /* sensor entity name format:
     * m01_b_ov13850 1-0010, where 'm01' means module index number
     * 'b' meansback or front, 'ov13850' is real sensor name
     * '1-0010' means the i2c bus and sensor i2c slave address
     */
    char sensor_name[DEV_PATH_LEN];
    char device_name[DEV_PATH_LEN];
    char len_name[DEV_PATH_LEN];
    char parent_media_dev[DEV_PATH_LEN];
    int media_node_index;
    int csi_port;
    char module_lens_dev_name[DEV_PATH_LEN];  // matched using mPhyModuleIndex
    char module_ircut_dev_name[DEV_PATH_LEN];
    int flash_num;
    char module_flash_dev_name[SENSOR_ATTACHED_FLASH_MAX_NUM]
                              [DEV_PATH_LEN];  // matched using mPhyModuleIndex
    bool fl_strth_adj_sup;
    int flash_ir_num;
    char module_flash_ir_dev_name[SENSOR_ATTACHED_FLASH_MAX_NUM][DEV_PATH_LEN];
    bool fl_ir_strth_adj_sup;
    char module_real_sensor_name[32];     // parsed frome sensor entity name
    char module_index_str[DEV_PATH_LEN];  // parsed from sensor entity name
    char phy_module_orient;               // parsed from sensor entity name
    rk_frame_fmt_t frame_size[SUPPORT_FMT_MAX];
    int frame_size_cnt;
    rk_aiq_isp_t* isp_info;
    rk_aiq_cif_info_t* cif_info;
    rk_aiq_ispp_t* ispp_info;
    rk_aiq_aiisp_info_t* aiisp_info;
    bool linked_to_isp;
    bool linked_to_1608;
    bool linked_to_serdes;
    bool dvp_itf;
    struct rkmodule_inf mod_info;
} rk_sensor_full_info_t;

typedef struct AiqHwEvt_s {
    int type;
    uint32_t frame_id;
    int64_t mTimestamp;
    aiq_VideoBuffer_t* vb;
} AiqHwEvt_t;

typedef struct Aiqisp20Evt_s {
    AiqHwEvt_t _base;
    AiqCamHwBase_t* mCamHw;
    uint32_t _expDelay;
} Aiqisp20Evt_t;

typedef struct AiqHwPdafEvt_s {
    AiqHwEvt_t _base;
    rk_aiq_isp_pdaf_meas_t pdaf_meas;
} AiqHwPdafEvt_t;

typedef struct AiqHwStatsEvt_s {
    AiqHwEvt_t _base;
    AiqCamHwBase_t* mCamHw;
    AiqLensHw_t* mFocusLensHw;
    AiqLensHw_t* mIrishw;
    AiqSensorHw_t* mSensorHw;
} AiqHwStatsEvt_t;

typedef struct AiqHwAiispEvt_s {
    AiqHwEvt_t _base;
    int32_t _height;
	int iir_index;
	int gain_index;
	int aiisp_index;
} AiqHwAiispEvt_t;

typedef struct AiqHwMemcEvt_s {
    AiqHwEvt_t _base;
    int32_t _height;
    uint32_t _event_id;
    struct rkisp_aiisp_st isp_idxbuf;
} AiqHwMemcEvt_t;

typedef struct AiqHwAinnEvt_s {
    AiqHwEvt_t _base;
    int32_t _height;
    uint32_t _event_id;
    union rkaiisp_queue_buf queue_buf;
} AiqHwAinnEvt_t;

typedef struct AiqHwResListener_s {
    void* _pCtx;
    XCamReturn (*hwResCb)(void* ctx, AiqHwEvt_t* evt);
} AiqHwResListener_t;

#endif
