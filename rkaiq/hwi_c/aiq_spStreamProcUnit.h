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
#ifndef _AIQ_SP_STREAM_PROC_UNIT_H_
#define _AIQ_SP_STREAM_PROC_UNIT_H_

#include "hwi_c/aiq_stream.h"

enum rkisp_isp_ver {
    ISP_V10 = 0x0,
    ISP_V11 = 0x1,
    ISP_V12 = 0x2,
    ISP_V13 = 0x3,
    ISP_V20 = 0x4,
    ISP_V21 = 0x5,
    ISP_V30 = 0x6,
    ISP_V32 = 0x7,
};

typedef struct AiqSPStreamProcUnit_s {
    AiqStream_t* _spStream;
    AiqCamHwBase_t* _camHw;
    AiqV4l2SubDevice_t* _isp_core_dev;
    struct AiqLensHw_s* _focus_dev;
    int _ds_width;
    int _ds_height;
    int _ds_width_align;
    int _ds_height_align;
    int _src_width;
    int _src_height;
    int img_ds_size_x;
    int img_ds_size_y;

    int _isp_ver;
    AiqMutex_t _afmeas_param_mutex;
    unsigned char ldg_enable;
    uint8_t* pAfTmp;
    uint32_t sub_shp4_4[RKAIQ_RAWAF_ROI_SUBWINS_NUM];
    uint32_t sub_shp8_8[RKAIQ_RAWAF_ROI_SUBWINS_NUM];
    uint32_t high_light[RKAIQ_RAWAF_ROI_SUBWINS_NUM];
    uint32_t high_light2[RKAIQ_RAWAF_ROI_SUBWINS_NUM];
    rk_aiq_af_algo_meas_t _af_meas_params;
    rk_aiq_lens_descriptor _lens_des;
    int af_img_width;
    int af_img_height;
    int af_img_width_align;
    int af_img_height_align;

    AiqPollCallback_t* _pcb;
} AiqSPStreamProcUnit_t;

XCamReturn AiqSPStreamProcUnit_init(AiqSPStreamProcUnit_t* pSpSrmU, AiqV4l2Device_t* isp_sp_dev,
                                    int type, int isp_ver);
void AiqSPStreamProcUnit_deinit(AiqSPStreamProcUnit_t* pSpSrmU);
XCamReturn AiqSPStreamProcUnit_start(AiqSPStreamProcUnit_t* pSpSrmU);
void AiqSPStreamProcUnit_stop(AiqSPStreamProcUnit_t* pSpSrmU);
XCamReturn AiqSPStreamProcUnit_prepare(AiqSPStreamProcUnit_t* pSpSrmU,
                                       CalibDbV2_Af_LdgParam_t* ldg_param,
                                       CalibDbV2_Af_HighLightParam_t* highlight, int width,
                                       int height, int stride);
void AiqSPStreamProcUnit_set_devices(AiqSPStreamProcUnit_t* pSpSrmU, AiqCamHwBase_t* camHw,
                                     AiqV4l2SubDevice_t* isp_core_dev, AiqLensHw_t* lensdev);
XCamReturn AiqSPStreamProcUnit_get_sp_resolution(AiqSPStreamProcUnit_t* pSpSrmU, int* width,
                                                 int* height, int* aligned_w, int* aligned_h);

void AiqSPStreamProcUnit_upd_af_meas(AiqSPStreamProcUnit_t* pSpSrmU, rk_aiq_isp_af_meas_t* sp_meas);

#endif /* _AIQ_SP_STREAM_PROC_UNIT_H_ */
