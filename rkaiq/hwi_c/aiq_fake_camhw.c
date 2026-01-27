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

#include "aiq_fake_camhw.h"

#if RKAIQ_HAVE_DUMPSYS
#include "common/aiq_notifier.h"
#endif
#include "hwi_c/aiq_CamHwBase.h"
#include "hwi_c/aiq_camHw.h"
#include "hwi_c/aiq_fake_camhw.h"
#include "hwi_c/aiq_fake_sensor.h"
#include "hwi_c/aiq_rawStreamCapUnit.h"
#include "hwi_c/aiq_rawStreamProcUnit.h"
#include "hwi_c/isp39/aiq_CamHwIsp39.h"
#include "hwi_c/isp33/aiq_CamHwIsp33.h"
#include "hwi_c/isp35/aiq_CamHwIsp35.h"
#include "include/common/mediactl/mediactl.h"

#ifndef FAKECAM_SUBM
#define FAKECAM_SUBM (0x40)
#endif

extern rk_aiq_isp_hw_info_t g_mIspHwInfos;
extern SnsFullInfoWraps_t* g_mSensorHwInfos;

static void parse_rk_rawdata(AiqCamHwFake_t* pFakeCamHw, void* rawdata, struct rk_aiq_vbuf* vbuf);
static XCamReturn parse_rk_rawfile(AiqCamHwFake_t* pFakeCamHw, FILE* fp, struct rk_aiq_vbuf* vbuf);

static XCamReturn init_mipi_devices(AiqCamHwFake_t* pFakeCamHw, rk_sensor_full_info_t* s_info) {
    /*
     * for _mipi_tx_devs, index 0 refer to short frame always, inedex 1 refer
     * to middle frame always, index 2 refert to long frame always.
     * for CIF usecase, because mipi_id0 refert to long frame always, so we
     * should know the HDR mode firstly befor building the relationship between
     * _mipi_tx_devs array and mipi_idx. here we just set the mipi_idx to
     * _mipi_tx_devs, we will build the real relation in start.
     * for CIF usecase, rawwr2_path is always connected to _mipi_tx_devs[0],
     * rawwr0_path is always connected to _mipi_tx_devs[1], and rawwr1_path is always
     * connected to _mipi_tx_devs[0]
     */

    AiqCamHwBase_t* pCamBase      = &pFakeCamHw->_base;
    AiqFakeV4l2Device_t* pFakeDev = NULL;
    AiqV4l2Device_t* pDev         = NULL;
    int i                         = 0;
    for (i = 0; i < 3; i++) {
        pFakeDev = (AiqFakeV4l2Device_t*)aiq_mallocz(sizeof(AiqFakeV4l2Device_t));
        if (!pFakeDev) {
            return XCAM_RETURN_ERROR_MEM;
        }
        pDev                         = &pFakeDev->_base._v4l_base;
        pFakeCamHw->_mipi_tx_devs[i] = pDev;
        AiqFakeV4l2Device_init(pFakeDev, "/dev/zero");
        pDev->open(pDev, false);
        AiqV4l2Device_setBufType(pDev, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

        pDev = (AiqV4l2Device_t*)aiq_mallocz(sizeof(AiqV4l2Device_t));
        if (!pDev) {
            return XCAM_RETURN_ERROR_MEM;
        }
        if (i == 0)
            AiqV4l2Device_init(pDev, s_info->isp_info->rawrd2_s_path);  // short
        else if (i == 1)
            AiqV4l2Device_init(pDev, s_info->isp_info->rawrd0_m_path);  // middle
        else
            AiqV4l2Device_init(pDev, s_info->isp_info->rawrd1_l_path);  // long
        pFakeCamHw->_mipi_rx_devs[i] = pDev;
        pDev->open(pDev, false);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn prepare_mipi_devices(AiqCamHwFake_t* pFakeCamHw) {

    AiqCamHwBase_t* pCamBase      = &pFakeCamHw->_base;
    AiqFakeV4l2Device_t* pFakeDev = NULL;
    int i                         = 0;

    for (i = 0; i < 3; i++) {
        AiqV4l2Device_setMemType(pFakeCamHw->_mipi_tx_devs[i], pFakeCamHw->_tx_memory_type);
        AiqV4l2Device_setMemType(pFakeCamHw->_mipi_rx_devs[i], pFakeCamHw->_rx_memory_type);
        if (pCamBase->_linked_to_isp) {
            if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FILE) {
                pFakeDev = (AiqFakeV4l2Device_t*)pFakeCamHw->_mipi_tx_devs[i];
                pFakeDev->_base._v4l_base._use_type = 2;
                AiqV4l2Device_setBufCnt(pFakeCamHw->_mipi_tx_devs[i], 1);
                AiqV4l2Device_setBufCnt(pFakeCamHw->_mipi_rx_devs[i], 1);
            } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_ADDR) {
                pFakeDev = (AiqFakeV4l2Device_t*)pFakeCamHw->_mipi_tx_devs[i];
                pFakeDev->_base._v4l_base._use_type = 1;
                AiqV4l2Device_setBufCnt(pFakeCamHw->_mipi_tx_devs[i], ISP_TX_BUF_NUM);
                AiqV4l2Device_setBufCnt(pFakeCamHw->_mipi_rx_devs[i], ISP_TX_BUF_NUM);
            } else {
                if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_DATA) {
                    pFakeDev = (AiqFakeV4l2Device_t*)pFakeCamHw->_mipi_rx_devs[i];
                    pFakeDev->_base._v4l_base._use_type = 1;
                }

                AiqV4l2Device_setBufCnt(pFakeCamHw->_mipi_tx_devs[i], ISP_TX_BUF_NUM);
                AiqV4l2Device_setBufCnt(pFakeCamHw->_mipi_rx_devs[i], ISP_TX_BUF_NUM);
            }
        } else {
            if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_DATA) {
                pFakeDev = (AiqFakeV4l2Device_t*)pFakeCamHw->_mipi_rx_devs[i];
                pFakeDev->_base._v4l_base._use_type = 1;
            }
            AiqV4l2Device_setBufCnt(pFakeCamHw->_mipi_tx_devs[i], VIPCAP_TX_BUF_NUM);
            AiqV4l2Device_setBufCnt(pFakeCamHw->_mipi_rx_devs[i], VIPCAP_TX_BUF_NUM);
        }
        AiqFakeSensorHw_t* fakeSensor = (AiqFakeSensorHw_t*)pCamBase->_mSensorDev;
        rk_aiq_exposure_sensor_descriptor sns_des;
        fakeSensor->_base.get_sensor_descriptor(&fakeSensor->_base, &sns_des);
        AiqV4l2Device_setFmt(pFakeCamHw->_mipi_tx_devs[i], sns_des.sensor_output_width,
                             sns_des.sensor_output_height, sns_des.sensor_pixelformat,
                             V4L2_FIELD_NONE, 0);
        AiqV4l2Device_setFmt(pFakeCamHw->_mipi_rx_devs[i], sns_des.sensor_output_width,
                             sns_des.sensor_output_height, sns_des.sensor_pixelformat,
                             V4L2_FIELD_NONE, 0);
    }

    return XCAM_RETURN_NO_ERROR;
}

static void parse_rk_rawdata(AiqCamHwFake_t* pFakeCamHw, void* rawdata, struct rk_aiq_vbuf* vbuf) {
    unsigned short tag = 0;
    struct _block_header header;
    uint8_t* p             = (uint8_t*)rawdata;
    uint8_t* actual_raw[3] = {NULL, NULL, NULL};
    int actual_raw_len[3]  = {0, 0, 0};
    bool is_actual_rawdata = false;
    bool bExit             = false;
    while (!bExit) {
        tag = *((unsigned short*)p);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "tag=0x%04x\n", tag);
        switch (tag) {
            case START_TAG:
                p = p + TAG_BYTE_LEN;
                memset(pFakeCamHw->_st_addr, 0, sizeof(pFakeCamHw->_st_addr));
                memset(&pFakeCamHw->_rawfmt, 0, sizeof(pFakeCamHw->_rawfmt));
                memset(&pFakeCamHw->_finfo, 0, sizeof(pFakeCamHw->_finfo));
                break;
            case NORMAL_RAW_TAG: {
                header = *((struct _block_header*)p);
                p      = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo)) {
                    pFakeCamHw->_st_addr[0] = *((struct _st_addrinfo*)p);
                } else {
                    // actual raw data
                    is_actual_rawdata = true;
                    actual_raw[0]     = p;
                    actual_raw_len[0] = header.block_length;
                }
                p = p + header.block_length;

                break;
            }
            case HDR_S_RAW_TAG: {
                header = *((struct _block_header*)p);
                p      = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo)) {
                    pFakeCamHw->_st_addr[0] = *((struct _st_addrinfo*)p);
                } else {
                    // actual raw data
                    is_actual_rawdata = true;
                    actual_raw[0]     = p;
                    actual_raw_len[0] = header.block_length;
                }
                p = p + header.block_length;
                break;
            }
            case HDR_M_RAW_TAG: {
                header = *((struct _block_header*)p);
                p      = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo)) {
                    pFakeCamHw->_st_addr[1] = *((struct _st_addrinfo*)p);
                } else {
                    // actual raw data
                    is_actual_rawdata = true;
                    actual_raw[1]     = p;
                    actual_raw_len[1] = header.block_length;
                }
                p = p + header.block_length;
                break;
            }
            case HDR_L_RAW_TAG: {
                header = *((struct _block_header*)p);
                p      = p + sizeof(struct _block_header);
                if (header.block_length == sizeof(struct _st_addrinfo)) {
                    pFakeCamHw->_st_addr[2] = *((struct _st_addrinfo*)p);
                } else {
                    // actual raw data
                    is_actual_rawdata = true;
                    actual_raw[2]     = p;
                    actual_raw_len[2] = header.block_length;
                }
                p = p + header.block_length;
                break;
            }
            case FORMAT_TAG: {
                pFakeCamHw->_rawfmt = *((struct _raw_format*)p);
                LOGD_CAMHW_SUBM(FAKECAM_SUBM, "hdr_mode=%d,bayer_fmt=%d\n",
                                pFakeCamHw->_rawfmt.hdr_mode, pFakeCamHw->_rawfmt.bayer_fmt);
                p = p + sizeof(struct _block_header) + pFakeCamHw->_rawfmt.size;
                break;
            }
            case STATS_TAG: {
                header = *((struct _block_header *)p);
                p      += sizeof(struct _block_header);
                if (header.block_length <= sizeof(pFakeCamHw->_finfo)) {
                    pFakeCamHw->_finfo = *((rk_aiq_frame_info_t *)p);
                    if (START_TAG_VERSION != pFakeCamHw->_finfo.vesrion)
                        LOGE_CAMHW("%s tag version error, STATS_TAG version need %x, raw file version is %x",
                                    __func__, START_TAG_VERSION, pFakeCamHw->_finfo.vesrion);
                } else {
                    LOGE_CAMHW("%s tag size error, block size %d, struct size %u, please check rkraw and aiq version",
                                __func__, header.block_length, sizeof(pFakeCamHw->_finfo));
                }
                p = p + header.block_length;
                break;
            }
            case ISP_REG_FMT_TAG: {
                header = *((struct _block_header*)p);
                p += sizeof(struct _block_header);
                p = p + header.block_length;
                break;
            }
            case ISP_REG_TAG: {
                header = *((struct _block_header*)p);
                p += sizeof(struct _block_header);
                p = p + header.block_length;
                break;
            }
            case ISPP_REG_FMT_TAG: {
                header = *((struct _block_header*)p);
                p += sizeof(struct _block_header);
                p = p + header.block_length;
                break;
            }
            case ISPP_REG_TAG: {
                header = *((struct _block_header*)p);
                p += sizeof(struct _block_header);
                p = p + header.block_length;
                break;
            }
            case PLATFORM_TAG: {
                header = *((struct _block_header*)p);
                p += sizeof(struct _block_header);
                p = p + header.block_length;
                break;
            }
            case END_TAG: {
                bExit = true;
                break;
            }
            default: {
                LOGE_CAMHW_SUBM(FAKECAM_SUBM, "Not support TAG(0x%04x)\n", tag);
                bExit = true;
                break;
            }
        }
    }

    vbuf->frame_width  = pFakeCamHw->_rawfmt.width;
    vbuf->frame_height = pFakeCamHw->_rawfmt.height;
    vbuf->base_addr    = rawdata;
    for (int i = 0; i < 3; i++) {
        if (pFakeCamHw->_finfo.isp_dgain[i] < 1.0) {
            pFakeCamHw->_finfo.isp_dgain[i] = 1.0;
        }
    }
    if (pFakeCamHw->_rawfmt.hdr_mode == 1) {
        if (is_actual_rawdata) {
            vbuf->buf_info[0].data_addr   = actual_raw[0];
            vbuf->buf_info[0].data_fd     = 0;
            vbuf->buf_info[0].data_length = actual_raw_len[0];
        } else {
            if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_ADDR) {
                if (sizeof(uint8_t*) == 4) {
                    vbuf->buf_info[0].data_addr = (uint8_t*)(long)(pFakeCamHw->_st_addr[0].laddr);
                } else if (sizeof(uint8_t*) == 8) {
                    vbuf->buf_info[0].data_addr =
                        (uint8_t*)(intptr_t)(((uint64_t)pFakeCamHw->_st_addr[0].haddr << 32) +
                                   pFakeCamHw->_st_addr[0].laddr);
                }

                vbuf->buf_info[0].data_fd = 0;
            } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[0].data_fd   = pFakeCamHw->_st_addr[0].fd;
                vbuf->buf_info[0].data_addr = NULL;
            }
            vbuf->buf_info[0].data_length = pFakeCamHw->_st_addr[0].size;
        }
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "data_addr=%p,fd=%d,length=%d\n", vbuf->buf_info[0].data_addr,
                        vbuf->buf_info[0].data_fd, vbuf->buf_info[0].data_length);

        vbuf->buf_info[0].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[0].exp_gain     = (float)pFakeCamHw->_finfo.normal_gain;
        vbuf->buf_info[0].exp_time     = (float)pFakeCamHw->_finfo.normal_exp;
        vbuf->buf_info[0].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[0];
        vbuf->buf_info[0].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.normal_gain_reg;
        vbuf->buf_info[0].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.normal_exp_reg;
        vbuf->buf_info[0].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[0].exp_gain, vbuf->buf_info[0].exp_time,
                        vbuf->buf_info[0].exp_gain_reg, vbuf->buf_info[0].exp_time_reg);
    } else if (pFakeCamHw->_rawfmt.hdr_mode == 2) {
        if (is_actual_rawdata) {
            vbuf->buf_info[0].data_addr   = actual_raw[0];
            vbuf->buf_info[0].data_fd     = 0;
            vbuf->buf_info[0].data_length = actual_raw_len[0];
        } else {
            if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_ADDR) {
                if (sizeof(uint8_t*) == 4) {
                    vbuf->buf_info[0].data_addr = (uint8_t*)(long)(pFakeCamHw->_st_addr[0].laddr);
                } else if (sizeof(uint8_t*) == 8) {
                    vbuf->buf_info[0].data_addr =
                        (uint8_t*)(intptr_t)(((uint64_t)pFakeCamHw->_st_addr[0].haddr << 32) +
                                   pFakeCamHw->_st_addr[0].laddr);
                }
                vbuf->buf_info[0].data_fd = 0;
            } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[0].data_addr = NULL;
                vbuf->buf_info[0].data_fd   = pFakeCamHw->_st_addr[0].fd;
            }
            vbuf->buf_info[0].data_length = pFakeCamHw->_st_addr[0].size;
        }
        vbuf->buf_info[0].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[0].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_s;
        vbuf->buf_info[0].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_s;
        vbuf->buf_info[0].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[0];
        vbuf->buf_info[0].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_s_reg;
        vbuf->buf_info[0].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_s_reg;
        vbuf->buf_info[0].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[0]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[0].data_addr, vbuf->buf_info[0].data_fd,
                        vbuf->buf_info[0].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[0]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[0].exp_gain, vbuf->buf_info[0].exp_time,
                        vbuf->buf_info[0].exp_gain_reg, vbuf->buf_info[0].exp_time_reg);
        if (is_actual_rawdata) {
            vbuf->buf_info[1].data_addr   = actual_raw[1];  // actual_raw[1]
            vbuf->buf_info[1].data_fd     = 0;
            vbuf->buf_info[1].data_length = actual_raw_len[1];  // actual_raw_len[1]
        } else {
            if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_ADDR) {
                if (sizeof(uint8_t*) == 4) {
                    vbuf->buf_info[1].data_addr = (uint8_t*)(long)(pFakeCamHw->_st_addr[1].laddr);
                } else if (sizeof(uint8_t*) == 8) {
                    vbuf->buf_info[1].data_addr =
                        (uint8_t*)(intptr_t)(((uint64_t)pFakeCamHw->_st_addr[1].haddr << 32) +
                                   pFakeCamHw->_st_addr[1].laddr);
                }
                vbuf->buf_info[1].data_fd = 0;
            } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[1].data_addr = NULL;
                vbuf->buf_info[1].data_fd   = pFakeCamHw->_st_addr[1].fd;
            }
            vbuf->buf_info[1].data_length = pFakeCamHw->_st_addr[1].size;
        }
        vbuf->buf_info[1].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[1].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_m;
        vbuf->buf_info[1].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_m;
        vbuf->buf_info[1].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[1];
        vbuf->buf_info[1].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_m_reg;
        vbuf->buf_info[1].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_m_reg;
        vbuf->buf_info[1].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[1]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[1].data_addr, vbuf->buf_info[1].data_fd,
                        vbuf->buf_info[1].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[1]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[1].exp_gain, vbuf->buf_info[1].exp_time,
                        vbuf->buf_info[1].exp_gain_reg, vbuf->buf_info[1].exp_time_reg);
    } else if (pFakeCamHw->_rawfmt.hdr_mode == 3) {
        if (is_actual_rawdata) {
            vbuf->buf_info[0].data_addr   = actual_raw[0];
            vbuf->buf_info[0].data_fd     = 0;
            vbuf->buf_info[0].data_length = actual_raw_len[0];
        } else {
            if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_ADDR) {
                if (sizeof(uint8_t*) == 4) {
                    vbuf->buf_info[0].data_addr = (uint8_t*)(long)(pFakeCamHw->_st_addr[0].laddr);
                } else if (sizeof(uint8_t*) == 8) {
                    vbuf->buf_info[0].data_addr =
                        (uint8_t*)(intptr_t)(((uint64_t)pFakeCamHw->_st_addr[0].haddr << 32) +
                                   pFakeCamHw->_st_addr[0].laddr);
                }
                vbuf->buf_info[0].data_fd = 0;
            } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[0].data_addr = NULL;
                vbuf->buf_info[0].data_fd   = pFakeCamHw->_st_addr[0].fd;
            }
            vbuf->buf_info[0].data_length = pFakeCamHw->_st_addr[0].size;
        }
        vbuf->buf_info[0].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[0].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_s;
        vbuf->buf_info[0].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_s;
        vbuf->buf_info[0].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[0];
        vbuf->buf_info[0].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_s_reg;
        vbuf->buf_info[0].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_s_reg;
        vbuf->buf_info[0].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[0]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[0].data_addr, vbuf->buf_info[0].data_fd,
                        vbuf->buf_info[0].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[0]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[0].exp_gain, vbuf->buf_info[0].exp_time,
                        vbuf->buf_info[0].exp_gain_reg, vbuf->buf_info[0].exp_time_reg);

        if (is_actual_rawdata) {
            vbuf->buf_info[1].data_addr   = actual_raw[1];
            vbuf->buf_info[1].data_fd     = 0;
            vbuf->buf_info[1].data_length = actual_raw_len[1];
        } else {
            if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_ADDR) {
                if (sizeof(uint8_t*) == 4) {
                    vbuf->buf_info[1].data_addr = (uint8_t*)(long)(pFakeCamHw->_st_addr[1].laddr);
                } else if (sizeof(uint8_t*) == 8) {
                    vbuf->buf_info[1].data_addr =
                        (uint8_t*)(intptr_t)(((uint64_t)pFakeCamHw->_st_addr[1].haddr << 32) +
                                   pFakeCamHw->_st_addr[1].laddr);
                }
                vbuf->buf_info[1].data_fd = 0;
            } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[1].data_addr = NULL;
                vbuf->buf_info[1].data_fd   = pFakeCamHw->_st_addr[1].fd;
            }
            vbuf->buf_info[1].data_length = pFakeCamHw->_st_addr[1].size;
        }
        vbuf->buf_info[1].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[1].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_m;
        vbuf->buf_info[1].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_m;
        vbuf->buf_info[1].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[1];
        vbuf->buf_info[1].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_m_reg;
        vbuf->buf_info[1].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_m_reg;
        vbuf->buf_info[1].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[1]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[1].data_addr, vbuf->buf_info[1].data_fd,
                        vbuf->buf_info[1].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[1]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[1].exp_gain, vbuf->buf_info[1].exp_time,
                        vbuf->buf_info[1].exp_gain_reg, vbuf->buf_info[1].exp_time_reg);

        if (is_actual_rawdata) {
            vbuf->buf_info[2].data_addr   = actual_raw[2];
            vbuf->buf_info[2].data_fd     = 0;
            vbuf->buf_info[2].data_length = actual_raw_len[2];
        } else {
            if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_ADDR) {
                if (sizeof(uint8_t*) == 4) {
                    vbuf->buf_info[2].data_addr = (uint8_t*)(long)(pFakeCamHw->_st_addr[2].laddr);
                } else if (sizeof(uint8_t*) == 8) {
                    vbuf->buf_info[2].data_addr =
                        (uint8_t*)(intptr_t)(((uint64_t)pFakeCamHw->_st_addr[2].haddr << 32) +
                                   pFakeCamHw->_st_addr[2].laddr);
                }
                vbuf->buf_info[2].data_fd = 0;
            } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FD) {
                vbuf->buf_info[2].data_addr = NULL;
                vbuf->buf_info[2].data_fd   = pFakeCamHw->_st_addr[2].fd;
            }
            vbuf->buf_info[2].data_length = pFakeCamHw->_st_addr[2].size;
        }
        vbuf->buf_info[2].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[2].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_l;
        vbuf->buf_info[2].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_l;
        vbuf->buf_info[2].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[2];
        vbuf->buf_info[2].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_l_reg;
        vbuf->buf_info[2].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_l_reg;
        vbuf->buf_info[2].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[2]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[2].data_addr, vbuf->buf_info[2].data_fd,
                        vbuf->buf_info[2].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[2]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[2].exp_gain, vbuf->buf_info[2].exp_time,
                        vbuf->buf_info[2].exp_gain_reg, vbuf->buf_info[2].exp_time_reg);
    }
}

static XCamReturn parse_rk_rawfile(AiqCamHwFake_t* pFakeCamHw, FILE* fp, struct rk_aiq_vbuf* vbuf) {
    unsigned short tag = 0;
    struct _block_header header;
    bool bExit = false;

    if (!AiqV4l2Device_isActivated(pFakeCamHw->_mipi_rx_devs[0])) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "device(%s) hasn't activated(%d) yet!!!\n",
                        AiqV4l2Device_getDevName(pFakeCamHw->_mipi_rx_devs[0]),
                        AiqV4l2Device_isActivated(pFakeCamHw->_mipi_rx_devs[0]));
        return XCAM_RETURN_ERROR_FAILED;
    }

    while (!bExit) {
        int ret = fread(&tag, sizeof(tag), 1, fp);
        if (ret == 0) break;
        fseek(fp, TAG_BYTE_LEN * (-1), SEEK_CUR);  // backforwad to tag start
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "tag=0x%04x\n", tag);
        switch (tag) {
            case START_TAG:
                fseek(fp, TAG_BYTE_LEN, SEEK_CUR);
                memset(pFakeCamHw->_st_addr, 0, sizeof(pFakeCamHw->_st_addr));
                memset(&pFakeCamHw->_rawfmt, 0, sizeof(pFakeCamHw->_rawfmt));
                memset(&pFakeCamHw->_finfo, 0, sizeof(pFakeCamHw->_finfo));
                break;
            case NORMAL_RAW_TAG: {
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length > 0) {
                    vbuf->buf_info[0].data_addr =
                        (uint8_t*)(AiqV4l2Device_getBufByIndex(pFakeCamHw->_mipi_rx_devs[0], 0)
                                       ->_expbuf_usrptr);
                    fread(vbuf->buf_info[0].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[0].data_length = header.block_length;
                }
                break;
            }
            case HDR_S_RAW_TAG: {
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length > 0) {
                    vbuf->buf_info[0].data_addr =
                        (uint8_t*)(AiqV4l2Device_getBufByIndex(pFakeCamHw->_mipi_rx_devs[0], 0)
                                       ->_expbuf_usrptr);
                    fread(vbuf->buf_info[0].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[0].data_length = header.block_length;
                }
                break;
            }
            case HDR_M_RAW_TAG: {
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length > 0) {
                    vbuf->buf_info[1].data_addr =
                        (uint8_t*)(AiqV4l2Device_getBufByIndex(pFakeCamHw->_mipi_rx_devs[1], 0)
                                       ->_expbuf_usrptr);
                    fread(vbuf->buf_info[1].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[1].data_length = header.block_length;
                }
                break;
            }
            case HDR_L_RAW_TAG: {
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length > 0) {
                    vbuf->buf_info[2].data_addr =
                        (uint8_t*)(AiqV4l2Device_getBufByIndex(pFakeCamHw->_mipi_rx_devs[2], 0)
                                       ->_expbuf_usrptr);
                    fread(vbuf->buf_info[2].data_addr, header.block_length, 1, fp);
                    vbuf->buf_info[2].data_length = header.block_length;
                }
                break;
            }
            case FORMAT_TAG: {
                fread(&pFakeCamHw->_rawfmt, sizeof(pFakeCamHw->_rawfmt), 1, fp);
                LOGD_CAMHW_SUBM(FAKECAM_SUBM, "hdr_mode=%d,bayer_fmt=%d\n", pFakeCamHw->_rawfmt.hdr_mode,
                                pFakeCamHw->_rawfmt.bayer_fmt);
                break;
            }
            case STATS_TAG: {
                fread(&header, sizeof(header), 1, fp);
                if (header.block_length <= sizeof(pFakeCamHw->_finfo)) {
                    fread(&pFakeCamHw->_finfo, header.block_length, 1, fp);
                    if (START_TAG_VERSION != pFakeCamHw->_finfo.vesrion)
                        LOGE_CAMHW("%s tag version error, STATS_TAG version need %x, raw file version is %x",
                                    __func__, START_TAG_VERSION, pFakeCamHw->_finfo.vesrion);
                } else {
                    LOGE_CAMHW("%s tag size error, block size %d, struct size %u, please check rkraw and aiq version",
                                __func__, header.block_length, sizeof(pFakeCamHw->_finfo));
                    fseek(fp, header.block_length, SEEK_CUR);
                }
                break;
            }
            case ISP_REG_FMT_TAG: {
                fread(&header, sizeof(header), 1, fp);
                fseek(fp, header.block_length, SEEK_CUR);
                break;
            }
            case ISP_REG_TAG: {
                fread(&header, sizeof(header), 1, fp);
                fseek(fp, header.block_length, SEEK_CUR);
                break;
            }
            case ISPP_REG_FMT_TAG: {
                fread(&header, sizeof(header), 1, fp);
                fseek(fp, header.block_length, SEEK_CUR);
                break;
            }
            case ISPP_REG_TAG: {
                fread(&header, sizeof(header), 1, fp);
                fseek(fp, header.block_length, SEEK_CUR);
                break;
            }
            case PLATFORM_TAG: {
                fread(&header, sizeof(header), 1, fp);
                fseek(fp, header.block_length, SEEK_CUR);
                break;
            }
            case END_TAG: {
                bExit = true;
                break;
            }
            default: {
                LOGE_CAMHW_SUBM(FAKECAM_SUBM, "Not support TAG(0x%04x)\n", tag);
                bExit = true;
                break;
            }
        }
    }

    vbuf->frame_width  = pFakeCamHw->_rawfmt.width;
    vbuf->frame_height = pFakeCamHw->_rawfmt.height;
    for (int i = 0; i < 3; i++) {
        if (pFakeCamHw->_finfo.isp_dgain[i] < 1.0) {
            pFakeCamHw->_finfo.isp_dgain[i] = 1.0;
        }
    }
    if (pFakeCamHw->_rawfmt.hdr_mode == 1) {
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "data_addr=%p,fd=%d,length=%d\n", vbuf->buf_info[0].data_addr,
                        vbuf->buf_info[0].data_fd, vbuf->buf_info[0].data_length);

        vbuf->buf_info[0].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[0].exp_gain     = (float)pFakeCamHw->_finfo.normal_gain;
        vbuf->buf_info[0].exp_time     = (float)pFakeCamHw->_finfo.normal_exp;
        vbuf->buf_info[0].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[0];
        vbuf->buf_info[0].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.normal_gain_reg;
        vbuf->buf_info[0].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.normal_exp_reg;
        vbuf->buf_info[0].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[0].exp_gain, vbuf->buf_info[0].exp_time,
                        vbuf->buf_info[0].exp_gain_reg, vbuf->buf_info[0].exp_time_reg);
    } else if (pFakeCamHw->_rawfmt.hdr_mode == 2) {
        vbuf->buf_info[0].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[0].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_s;
        vbuf->buf_info[0].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_s;
        vbuf->buf_info[0].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[0];
        vbuf->buf_info[0].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_s_reg;
        vbuf->buf_info[0].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_s_reg;
        vbuf->buf_info[0].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[0]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[0].data_addr, vbuf->buf_info[0].data_fd,
                        vbuf->buf_info[0].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[0]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[0].exp_gain, vbuf->buf_info[0].exp_time,
                        vbuf->buf_info[0].exp_gain_reg, vbuf->buf_info[0].exp_time_reg);

        vbuf->buf_info[1].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[1].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_m;
        vbuf->buf_info[1].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_m;
        vbuf->buf_info[1].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[1];
        vbuf->buf_info[1].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_m_reg;
        vbuf->buf_info[1].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_m_reg;
        vbuf->buf_info[1].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[1]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[1].data_addr, vbuf->buf_info[1].data_fd,
                        vbuf->buf_info[1].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[1]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[1].exp_gain, vbuf->buf_info[1].exp_time,
                        vbuf->buf_info[1].exp_gain_reg, vbuf->buf_info[1].exp_time_reg);
    } else if (pFakeCamHw->_rawfmt.hdr_mode == 3) {
        vbuf->buf_info[0].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[0].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_s;
        vbuf->buf_info[0].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_s;
        vbuf->buf_info[0].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[0];
        vbuf->buf_info[0].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_s_reg;
        vbuf->buf_info[0].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_s_reg;
        vbuf->buf_info[0].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[0]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[0].data_addr, vbuf->buf_info[0].data_fd,
                        vbuf->buf_info[0].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[0]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[0].exp_gain, vbuf->buf_info[0].exp_time,
                        vbuf->buf_info[0].exp_gain_reg, vbuf->buf_info[0].exp_time_reg);

        vbuf->buf_info[1].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[1].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_m;
        vbuf->buf_info[1].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_m;
        vbuf->buf_info[1].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[1];
        vbuf->buf_info[1].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_m_reg;
        vbuf->buf_info[1].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_m_reg;
        vbuf->buf_info[1].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[1]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[1].data_addr, vbuf->buf_info[1].data_fd,
                        vbuf->buf_info[1].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[1]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[1].exp_gain, vbuf->buf_info[1].exp_time,
                        vbuf->buf_info[1].exp_gain_reg, vbuf->buf_info[1].exp_time_reg);

        vbuf->buf_info[2].frame_id     = pFakeCamHw->_rawfmt.frame_id;
        vbuf->buf_info[2].exp_gain     = (float)pFakeCamHw->_finfo.hdr_gain_l;
        vbuf->buf_info[2].exp_time     = (float)pFakeCamHw->_finfo.hdr_exp_l;
        vbuf->buf_info[2].exp_ispdgain = (float)pFakeCamHw->_finfo.isp_dgain[2];
        vbuf->buf_info[2].exp_gain_reg = (uint32_t)pFakeCamHw->_finfo.hdr_gain_l_reg;
        vbuf->buf_info[2].exp_time_reg = (uint32_t)pFakeCamHw->_finfo.hdr_exp_l_reg;
        vbuf->buf_info[2].valid        = true;
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[2]: data_addr=%p,fd=%d,,length=%d\n",
                        vbuf->buf_info[2].data_addr, vbuf->buf_info[2].data_fd,
                        vbuf->buf_info[2].data_length);
        LOGD_CAMHW_SUBM(FAKECAM_SUBM, "buf_info[2]: gain:%f,time:%f,gain_reg:0x%x,time_reg:0x%x\n",
                        vbuf->buf_info[2].exp_gain, vbuf->buf_info[2].exp_time,
                        vbuf->buf_info[2].exp_gain_reg, vbuf->buf_info[2].exp_time_reg);
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn setupOffLineLink(AiqCamHwFake_t* pFakeCamHw, int isp_index, bool enable) {
    struct media_device* device = NULL;
    struct media_entity* entity = NULL;
    struct media_pad* src_pad   = NULL;
    struct media_pad* sink_pad  = NULL;
    int lvds_max_entities = 6;
    int lvds_entity       = 0;

    device = media_device_new(g_mIspHwInfos.isp_info[isp_index].media_dev_path);
    if (!device) return XCAM_RETURN_ERROR_FAILED;

    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);
    entity = media_get_entity_by_name(device, "rkisp-isp-subdev", strlen("rkisp-isp-subdev"));
    if (!entity) {
        goto FAIL;
    }

    sink_pad = (struct media_pad*)media_entity_get_pad(entity, 0);
    if (!sink_pad) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get rkisp-isp-subdev sink pad failed!\n");
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
            src_pad = (struct media_pad*)media_entity_get_pad(entity, 0);
            if (!src_pad) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get rkcif-mipi-lvds%d source pad s failed!\n",
                                lvds_entity);
                goto FAIL;
            }
        }

        if (src_pad && sink_pad) {
            if (enable) {
                media_setup_link(device, src_pad, sink_pad, 0);
            } else
                media_setup_link(device, src_pad, sink_pad, MEDIA_LNK_FL_ENABLED);
        }
    }

    media_device_unref(device);
    return XCAM_RETURN_NO_ERROR;
FAIL:
    media_device_unref(device);
    return XCAM_RETURN_ERROR_FAILED;
}

static XCamReturn FakeCamHwIsp20_prepare(AiqCamHwBase_t* pCamHw, uint32_t width, uint32_t height,
                                         int mode, int t_delay, int g_delay) {
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    AiqCamHwFake_t* pFakeCamHw = (AiqCamHwFake_t*)pCamHw;

    ENTER_CAMHW_FUNCTION();

    SnsFullInfoWraps_t* pSnsInfoWrap = NULL;

    pSnsInfoWrap = g_mSensorHwInfos;
    while (pSnsInfoWrap) {
        if (strcmp(pCamHw->sns_name, pSnsInfoWrap->key) == 0) break;
        pSnsInfoWrap = pSnsInfoWrap->next;
    }

    if (!pSnsInfoWrap) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", pCamHw->sns_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    rk_sensor_full_info_t* s_info = &pSnsInfoWrap->data;
    uint32_t isp_index            = s_info->isp_info->logic_id;

    if (!pCamHw->use_rkrawstream) {
        setupOffLineLink(pFakeCamHw, isp_index, true);
        prepare_mipi_devices(pFakeCamHw);
    } else {
        struct v4l2_subdev_format isp_sink_fmt;

        memset(&isp_sink_fmt, 0, sizeof(isp_sink_fmt));
        isp_sink_fmt.pad = 0;
        isp_sink_fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
        ret = pCamHw->mIspCoreDev->getFormat(pCamHw->mIspCoreDev, &isp_sink_fmt);
        if (ret) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get mIspCoreDev fmt failed to set fake sensor!\n");
        }
        AiqFakeSensorHw_t* fakeSensorHw = (AiqFakeSensorHw_t*)pCamHw->_mSensorDev;
        fakeSensorHw->set_fake_sensor_format(fakeSensorHw, isp_sink_fmt.format.width,
                                             isp_sink_fmt.format.height, isp_sink_fmt.format.code);
    }

    ret = AiqCamHw_prepare(pCamHw, width, height, mode, t_delay, g_delay);
    if (ret) return ret;

    if (pCamHw->mIspSofStream)
        ((AiqStream_t*)(pCamHw->mIspSofStream))
            ->setPollCallback((AiqStream_t*)(pCamHw->mIspSofStream), NULL);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn FakeCamHwIsp20_poll_buffer_ready(void* ctx, AiqHwEvt_t* evt, int dev_index) {
    AiqCamHwBase_t* pCamBase        = (AiqCamHwBase_t*)ctx;
    AiqFakeSensorHw_t* fakeSensorHw = (AiqFakeSensorHw_t*)pCamBase->_mSensorDev;
    // notify fakesensor one frame has been processed by ISP
    return fakeSensorHw->on_dqueue(fakeSensorHw, dev_index, (AiqV4l2Buffer_t*)evt->vb);
}

static XCamReturn FakeCamHwIsp20_poll_event_ready(void* ctx, uint32_t sequence, int type) {
    AiqCamHwBase_t* pCamBase = (AiqCamHwBase_t*)ctx;

    ((AiqStream_t*)(pCamBase->mIspSofStream))->stop((AiqStream_t*)(pCamBase->mIspSofStream));

    AiqHwEvt_t evt;
    evt.type       = type;
    evt.frame_id   = sequence;
    evt.mTimestamp = 0;
    if (pCamBase->_hwResListener.hwResCb) {
        return pCamBase->_hwResListener.hwResCb(pCamBase->_hwResListener._pCtx, &evt);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqCamHwFake_init(AiqCamHwFake_t* pFakeCamHw, const char* sns_ent_name) {
    AiqCamHwBase_t* pCamBase = &pFakeCamHw->_base;
    XCamReturn ret           = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39)
    ret = AiqCamHwIsp39_init(pCamBase, sns_ent_name);
#elif defined(ISP_HW_V33)
    ret = AiqCamHwIsp33_init(pCamBase, sns_ent_name);
#elif defined(ISP_HW_V35)
    ret = AiqCamHwIsp35_init(pCamBase, sns_ent_name);
#else
    XCAM_ASSERT(0);
#endif
    if (pCamBase->_mSensorDev) {
        pCamBase->_mSensorDev->close(pCamBase->_mSensorDev);
        AiqSensorHw_deinit(pCamBase->_mSensorDev);
        aiq_free(pCamBase->_mSensorDev);
        pCamBase->_mSensorDev = NULL;
        pCamBase->_mSensorDev = (AiqSensorHw_t*)aiq_mallocz(sizeof(AiqFakeSensorHw_t));
        if (!pCamBase->_mSensorDev) {
            return XCAM_RETURN_ERROR_MEM;
        }
        AiqFakeSensorHw_init((AiqFakeSensorHw_t*)pCamBase->_mSensorDev, "/dev/null",
                             pCamBase->mCamPhyId);
        pCamBase->_mSensorDev->open(pCamBase->_mSensorDev);
    }
    pFakeCamHw->_rx_memory_type         = V4L2_MEMORY_DMABUF;
    pFakeCamHw->_tx_memory_type         = V4L2_MEMORY_DMABUF;
    pCamBase->mNoReadBack               = false;
    pCamBase->prepare                   = FakeCamHwIsp20_prepare;
    pFakeCamHw->mPollCb._pCtx             = pFakeCamHw;
    pFakeCamHw->mPollCb.poll_buffer_ready = FakeCamHwIsp20_poll_buffer_ready;
    pFakeCamHw->mPollCb.poll_event_ready  = FakeCamHwIsp20_poll_event_ready;
    if (pCamBase->mIspSofStream)
        ((AiqStream_t*)(pCamBase->mIspSofStream))
            ->setPollCallback((AiqStream_t*)(pCamBase->mIspSofStream), NULL);
    if (pCamBase->mIspStatsStream)
        pCamBase->mIspStatsStream->set_event_handle_dev(pCamBase->mIspStatsStream,
                                                        pCamBase->_mSensorDev);
#if RKAIQ_HAVE_DUMPSYS
    aiq_notifier_remove_subscriber(&pCamBase->notifier, AIQ_NOTIFIER_MATCH_HWI_SENSOR);

    {
        pCamBase->sub_sensor.match_type     = AIQ_NOTIFIER_MATCH_HWI_SENSOR;
        pCamBase->sub_sensor.name           = "HWI -> sensor";
        pCamBase->sub_sensor.dump.dump_fn_t = pCamBase->_mSensorDev->dump;
        pCamBase->sub_sensor.dump.dumper    = pCamBase->_mSensorDev;

        aiq_notifier_add_subscriber(&pCamBase->notifier, &pCamBase->sub_sensor);
    }
#endif

    SnsFullInfoWraps_t* pSnsInfoWrap = NULL;

    pSnsInfoWrap = g_mSensorHwInfos;
    while (pSnsInfoWrap) {
        if (strcmp(pCamBase->sns_name, pSnsInfoWrap->key) == 0) break;
        pSnsInfoWrap = pSnsInfoWrap->next;
    }

    if (!pSnsInfoWrap) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "can't find sensor %s", pCamBase->sns_name);
        return XCAM_RETURN_ERROR_SENSOR;
    }

    AiqFakeSensorHw_t* fakeSensorHw = (AiqFakeSensorHw_t*)pCamBase->_mSensorDev;
    fakeSensorHw->use_rkrawstream   = pCamBase->use_rkrawstream;

    rk_sensor_full_info_t* s_info   = &pSnsInfoWrap->data;

    if (!pCamBase->use_rkrawstream) {
        init_mipi_devices(pFakeCamHw, s_info);
        fakeSensorHw->set_mipi_tx_devs(fakeSensorHw, pFakeCamHw->_mipi_tx_devs);
        AiqRawStreamCapUnit_set_tx_devices(pCamBase->mRawCapUnit, pFakeCamHw->_mipi_tx_devs);
        AiqRawStreamProcUnit_set_rx_devices(pCamBase->mRawProcUnit, pFakeCamHw->_mipi_rx_devs);
        AiqRawStreamProcUnit_setPollCallback(pCamBase->mRawProcUnit, &pFakeCamHw->mPollCb);
    }

    return ret;
}

void AiqCamHwFake_deinit(AiqCamHwFake_t* pFakeCamHw) {
    AiqCamHwBase_deinit((AiqCamHwBase_t*)pFakeCamHw);
}

XCamReturn AiqCamHwFake_enqueueRawBuffer(AiqCamHwFake_t* pFakeCamHw, void* rawdata, bool sync) {
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rk_aiq_vbuf vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    parse_rk_rawdata(pFakeCamHw, rawdata, &vbuf);
    AiqFakeSensorHw_t* fakeSensorHw = (AiqFakeSensorHw_t*)pFakeCamHw->_base._mSensorDev;
    fakeSensorHw->enqueue_rawbuffer(fakeSensorHw, &vbuf, sync);
    FakeCamHwIsp20_poll_event_ready(pFakeCamHw, vbuf.buf_info[0].frame_id, ISP_POLL_SOF);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqCamHwFake_enqueueRawFile(AiqCamHwFake_t* pFakeCamHw, const char* path) {
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct rk_aiq_vbuf vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    if (0 != access(path, F_OK)) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "file: %s is not exist!", path);
        return XCAM_RETURN_ERROR_PARAM;
    }

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "open file: %s failed", path);
        return XCAM_RETURN_ERROR_FAILED;
    }

    ret = parse_rk_rawfile(pFakeCamHw, fp, &vbuf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "Failed to parse rk rawfile!");
        fclose(fp);
        return ret;
    }
    fclose(fp);
    AiqFakeSensorHw_t* fakeSensorHw = (AiqFakeSensorHw_t*)pFakeCamHw->_base._mSensorDev;
    fakeSensorHw->enqueue_rawbuffer(fakeSensorHw, &vbuf, true);
    FakeCamHwIsp20_poll_event_ready(pFakeCamHw, vbuf.buf_info[0].frame_id, ISP_POLL_SOF);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqCamHwFake_registRawdataCb(AiqCamHwFake_t* pFakeCamHw, void (*callback)(void*)) {
    ENTER_XCORE_FUNCTION();
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    AiqFakeSensorHw_t* fakeSensorHw = (AiqFakeSensorHw_t*)pFakeCamHw->_base._mSensorDev;
    ret = fakeSensorHw->register_rawdata_callback(fakeSensorHw, callback);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqCamHwFake_rawdataPrepare(AiqCamHwFake_t* pFakeCamHw, rk_aiq_raw_prop_t prop) {
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pFakeCamHw->_rawbuf_type = prop.rawbuf_type;
    if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_ADDR) {
        pFakeCamHw->_rx_memory_type = V4L2_MEMORY_USERPTR;
        pFakeCamHw->_tx_memory_type = V4L2_MEMORY_USERPTR;
    } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FD) {
        pFakeCamHw->_rx_memory_type = V4L2_MEMORY_DMABUF;
        pFakeCamHw->_tx_memory_type = V4L2_MEMORY_DMABUF;
    } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_DATA) {
        pFakeCamHw->_rx_memory_type = V4L2_MEMORY_MMAP;
        pFakeCamHw->_tx_memory_type = V4L2_MEMORY_USERPTR;
    } else if (pFakeCamHw->_rawbuf_type == RK_AIQ_RAW_FILE) {
        pFakeCamHw->_rx_memory_type = V4L2_MEMORY_MMAP;
        pFakeCamHw->_tx_memory_type = V4L2_MEMORY_USERPTR;
    } else {
        LOGE_CAMHW_SUBM(FAKECAM_SUBM, "Not support raw data type:%d", pFakeCamHw->_rawbuf_type);
        return XCAM_RETURN_ERROR_PARAM;
    }
    AiqFakeSensorHw_t* fakeSensorHw = (AiqFakeSensorHw_t*)pFakeCamHw->_base._mSensorDev;
    ret                             = fakeSensorHw->prepare((AiqSensorHw_t*)fakeSensorHw, &prop);
    EXIT_XCORE_FUNCTION();
    return ret;
}
