
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

#ifndef _FAKE_CAM_HW_ISP20_H_
#define _FAKE_CAM_HW_ISP20_H_

#include "CamHwIsp20.h"

namespace RkCam {

#define START_TAG           0xFF00
#define FORMAT_TAG          0xFF01
#define NORMAL_RAW_TAG      0xFF02
#define HDR_S_RAW_TAG       0xFF03
#define HDR_M_RAW_TAG       0xFF04
#define HDR_L_RAW_TAG       0xFF05
#define STATS_TAG           0xFF06
#define ISP_REG_FMT_TAG     0xFF07
#define ISP_REG_TAG         0xFF08
#define ISPP_REG_FMT_TAG    0xFF09
#define ISPP_REG_TAG        0xFF0A
#define PLATFORM_TAG        0xFF0B
#define END_TAG             0x00FF

#define TAG_BYTE_LEN     (2)
#define BLOCK_BYTE_LEN   (4)

struct _block_header {
    unsigned short block_id;
    unsigned int block_length;
}__attribute__ ((packed));

struct _st_addrinfo
{
    uint32_t fd;
    uint32_t haddr;
    uint32_t laddr;
    uint32_t size;
}__attribute__ ((packed));

struct _raw_format
{
    uint16_t tag;
    uint32_t size;
    uint16_t vesrion;
    char sensor[32];
    char scene[32];
    uint32_t frame_id;
    uint16_t width;
    uint16_t height;
    uint8_t bit_width;
    uint8_t bayer_fmt;
    uint8_t hdr_mode;
    uint8_t buf_type;
    uint16_t line_length;
    uint16_t acive_line_length;
    uint8_t byte_order;
}__attribute__ ((packed));

struct _frame_info
{
    uint16_t tag;
    uint32_t size;
    uint16_t vesrion;
    uint32_t frame_id;

    float normal_exp;
    float normal_gain;
    uint32_t normal_exp_reg;
    uint32_t normal_gain_reg;

    float hdr_exp_l;
    float hdr_gain_l;
    uint32_t hdr_exp_l_reg;
    uint32_t hdr_gain_l_reg;

    float hdr_exp_m;
    float hdr_gain_m;
    uint32_t hdr_gain_m_reg;
    uint32_t hdr_exp_m_reg;

    float hdr_exp_s;
    float hdr_gain_s;
    uint32_t hdr_exp_s_reg;
    uint32_t hdr_gain_s_reg;

    float awg_rgain;
    float awg_bgain;
}__attribute__ ((packed));

class FakeCamHwIsp20 : public CamHwIsp20 {
public:
    explicit FakeCamHwIsp20();
    virtual ~FakeCamHwIsp20();
    virtual XCamReturn init(const char* sns_ent_name);
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay);
    virtual XCamReturn enqueueRawBuffer(void *rawdata, bool sync);
    virtual XCamReturn enqueueRawFile(const char *path);
    virtual XCamReturn registRawdataCb(void (*callback)(void *));
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop);
private:
    XCamReturn init_mipi_devices(rk_sensor_full_info_t *s_info);
    void parse_rk_rawdata(void *rawdata, struct rk_aiq_vbuf *vbuf);
    void parse_rk_rawfile(FILE *fp, struct rk_aiq_vbuf *vbuf);
    struct _st_addrinfo _st_addr[3];
    struct _raw_format _rawfmt;
    struct _frame_info _finfo;
    enum v4l2_memory    _rx_memory_type;
    enum v4l2_memory    _tx_memory_type;
    rk_aiq_rawbuf_type_t _rawbuf_type;
};

};
#endif
