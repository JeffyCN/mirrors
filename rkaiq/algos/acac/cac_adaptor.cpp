/*
 *  Copyright (c) 2022 Rockchip Electronics Co., Ltd.
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */
#include "cac_adaptor.h"

#include <fcntl.h>

#include <fstream>
#include <iostream>

#define __STDC_FORMAT_MACROS
#include <cinttypes>

#include "algos/acac/lut_buffer.h"
#include "algos/acac/rk_aiq_types_acac_algo.h"
#include "common/rk_aiq_comm.h"
#include "common/rk_aiq_types_priv.h"
#include "xcore/base/xcam_log.h"

namespace RkCam {

#define RKCAC_STRENGTH_FIX_BITS    7
#define RKCAC_EDGE_DETECT_FIX_BITS 4

#define CAC_PSF_BUF_NUM 2
#define BITS_PER_BYTE   8
#define BYTES_PER_WORD  4
#define BITS_PER_WORD   (BITS_PER_BYTE * BYTES_PER_WORD)

#define INTERP_CAC(x0, x1, ratio) ((ratio) * ((x1) - (x0)) + (x0))

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

#if RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10
static const uint32_t IspBigModeWidthLimit = 2688;
static const uint32_t IspBigModeSizeLimit  = IspBigModeWidthLimit * 1536;
static const uint32_t CacPsfCountLimit     = 1632;
static const uint8_t CacChannelCount       = 2;
#else  // RKAIQ_HAVE_CAC_V11
static const uint32_t IspBigModeWidthLimit = 1536;
static const uint32_t IspBigModeSizeLimit  = IspBigModeWidthLimit * 864;
static const uint32_t CacPsfCountLimit     = 336;
static const uint8_t CacChannelCount       = 2;
#endif
static const uint8_t CacScaleFactorDefault = 64;
static const uint8_t CacScaleFactorBigMode = 128;
static const uint16_t CacScaleFactor256Mode = 256;
static const uint8_t CacStrengthDistance   = 128;
static const uint8_t CacPsfKernelSize      = 7 * 5;
static const uint8_t CacPsfKernelWordSizeInMemory =
    DIV_ROUND_UP((CacPsfKernelSize - 1) * BITS_PER_BYTE, BITS_PER_WORD);
static const uint8_t CacPsfBufferCount = CAC_PSF_BUF_NUM;

template<class T>
static constexpr const T& CacClamp( const T& v, const T& lo, const T& hi ) {
    return v < lo ? lo : hi < v ? hi : v;
}

static inline bool IsIspBigMode(uint32_t width, uint32_t height, bool is_multi_sensor) {
    if (is_multi_sensor || width > IspBigModeWidthLimit || width * height > IspBigModeSizeLimit) {
        return true;
    }

    return false;
}

/**
 * cac_wsize=bigmode_en ? (pic_width+126)>>7 : (pic_width+62)>>6;
 * cac_hsize=bigmode_en ? (pic_height+126)>>7 : (pic_height+62)>>6;
 * lut_h_wsize = cac_wsize*9
 * lut_v_size = cac_hsize*2
 */

#if RKAIQ_HAVE_CAC_V12
static inline void CalcCacLutConfig(uint32_t width, uint32_t height, bool is_big_mode,
                                    LutBufferConfig& config) {
    //is_big_mode is useless;
    config.Width     = width;
    config.Height    = height;
    config.IsBigMode = true;
    config.ScaleFactor = CacScaleFactor256Mode;
    /**
     * CAC only processes R & B channels, that means for R or R channels,
     * which have only half size of full picture, only need to div round up by 32(scale==64) or
     * 64(scale==128). For calculate convinient, use full picture size to calculate
     */
    config.LutHCount   =(width + 254) >> 8 ;
    config.LutVCount   = (height  + 254) >> 8;
    config.PsfCfgCount = config.LutHCount * config.LutVCount;
    XCAM_ASSERT(config.PsfCfgCount <= CacPsfCountLimit);
    /**
     * CAC stores one PSF point's kernel in 9 words, one kernel size is 8 bytes.
     * (8bytes*8bits/byte + 32 - 1) / 32bits/word = 9 words.
     */
}
#else
static inline void CalcCacLutConfig(uint32_t width, uint32_t height, bool is_big_mode,
                                    LutBufferConfig& config) {
    config.Width     = width;
    config.Height    = height;
    config.IsBigMode = is_big_mode;
    if (config.IsBigMode) {
        config.ScaleFactor = CacScaleFactorBigMode;
    } else {
        config.ScaleFactor = CacScaleFactorDefault;
    }
    /**
     * CAC only processes R & B channels, that means for R or R channels,
     * which have only half size of full picture, only need to div round up by 32(scale==64) or
     * 64(scale==128). For calculate convinient, use full picture size to calculate
     */
    config.LutHCount   = is_big_mode ? (width + 126) >> 7 : (width + 62) >> 6;
    config.LutVCount   = is_big_mode ? (height + 126) >> 7 : (height + 62) >> 6;
    config.PsfCfgCount = config.LutHCount * config.LutVCount;
    XCAM_ASSERT(config.PsfCfgCount <= CacPsfCountLimit);
    /**
     * CAC stores one PSF point's kernel in 9 words, one kernel size is 8 bytes.
     * (8bytes*8bits/byte + 32 - 1) / 32bits/word = 9 words.
     */
}

#endif
CacAlgoAdaptor::~CacAlgoAdaptor() {
    current_lut_.clear();
    lut_manger_ = nullptr;
};

#if RKAIQ_HAVE_CAC_V03
XCamReturn CacAlgoAdaptor::SetApiAttr(const rkaiq_cac_v03_api_attr_t* attr) {
    if (!attr_) {
        attr_ = std::unique_ptr<rkaiq_cac_v03_api_attr_t>(new rkaiq_cac_v03_api_attr_t);
    }
    if (attr->op_mode == RKAIQ_CAC_API_OPMODE_INVALID) {
        return XCAM_RETURN_ERROR_PARAM;
    } else if (attr->op_mode == RKAIQ_CAC_API_OPMODE_AUTO) {
        attr_->iso_cnt = attr->iso_cnt;
        attr_->op_mode = attr->op_mode;
        memcpy(attr_->auto_params, attr->auto_params, sizeof(attr->auto_params));
    } else if (attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        attr_->op_mode = attr->op_mode;
        memcpy(&attr_->manual_param, &attr->manual_param, sizeof(attr->manual_param));
    }
    enable_ = attr_->enable = attr->enable;
    isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}
#elif RKAIQ_HAVE_CAC_V10
XCamReturn CacAlgoAdaptor::SetApiAttr(const rkaiq_cac_v10_api_attr_t* attr) {
    if (!attr_) {
        attr_ = std::unique_ptr<rkaiq_cac_v10_api_attr_t>(new rkaiq_cac_v10_api_attr_t);
    }
    if (attr->op_mode == RKAIQ_CAC_API_OPMODE_INVALID) {
        return XCAM_RETURN_ERROR_PARAM;
    } else if (attr->op_mode == RKAIQ_CAC_API_OPMODE_AUTO) {
        attr_->iso_cnt = attr->iso_cnt;
        memcpy(attr_->auto_params, attr->auto_params, sizeof(attr->auto_params));
    } else if (attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        memcpy(&attr_->manual_param, &attr->manual_param, sizeof(attr->manual_param));
    }
    attr_->op_mode = attr->op_mode;
    enable_ = attr_->enable = attr->enable;
    isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}
#elif RKAIQ_HAVE_CAC_V11
XCamReturn CacAlgoAdaptor::SetApiAttr(const rkaiq_cac_v11_api_attr_t* attr) {
    if (attr->op_mode == RKAIQ_CAC_API_OPMODE_INVALID) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (!attr_) {
        attr_ = std::unique_ptr<rkaiq_cac_v11_api_attr_t>(new rkaiq_cac_v11_api_attr_t);
    }

    attr_->op_mode = attr->op_mode;
    if (attr->op_mode == RKAIQ_CAC_API_OPMODE_AUTO) {
        attr_->iso_cnt = attr->iso_cnt;
        memcpy(attr_->auto_params, attr->auto_params, sizeof(attr->auto_params));
    } else if (attr->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        memcpy(&attr_->manual_param, &attr->manual_param, sizeof(attr->manual_param));
    }
    enable_ = attr_->enable = attr->enable;
    isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_CAC_V03
XCamReturn CacAlgoAdaptor::GetApiAttr(rkaiq_cac_v03_api_attr_t* attr) {
    if (attr_ == nullptr) {
        return XCAM_RETURN_ERROR_FAILED;
    }
    memcpy(attr, attr_.get(), sizeof(*attr));

    return XCAM_RETURN_NO_ERROR;
}
#elif RKAIQ_HAVE_CAC_V10
XCamReturn CacAlgoAdaptor::GetApiAttr(rkaiq_cac_v10_api_attr_t* attr) {
    if (attr_ == nullptr) {
        return XCAM_RETURN_ERROR_FAILED;
    }
    memcpy(attr, attr_.get(), sizeof(*attr));

    return XCAM_RETURN_NO_ERROR;
}
#elif RKAIQ_HAVE_CAC_V11
XCamReturn CacAlgoAdaptor::GetApiAttr(rkaiq_cac_v11_api_attr_t* attr) {
    if (attr_ == nullptr) {
        return XCAM_RETURN_ERROR_FAILED;
    }
    memcpy(attr, attr_.get(), sizeof(*attr));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10 || RKAIQ_HAVE_CAC_V11
#if RKAIQ_HAVE_CAC_V03
XCamReturn CacAlgoAdaptor::Config(const AlgoCtxInstanceCfg* config,
                                  const CalibDbV2_Cac_V03_t* calib) {
#elif RKAIQ_HAVE_CAC_V10
XCamReturn CacAlgoAdaptor::Config(const AlgoCtxInstanceCfg* config,
                                  const CalibDbV2_Cac_V10_t* calib) {
#elif RKAIQ_HAVE_CAC_V11
XCamReturn CacAlgoAdaptor::Config(const AlgoCtxInstanceCfg* config,
                                  const CalibDbV2_Cac_V11_t* calib) {
#endif
    (void)(config);

    LOGD_ACAC("%s : Enter", __func__);
    ctx_config_ = config;
    enable_ = calib != nullptr ? calib->SettingPara.enable : false;
    calib_  = calib;

    if (attr_ == nullptr) {
#if RKAIQ_HAVE_CAC_V03
        attr_ = std::unique_ptr<rkaiq_cac_v03_api_attr_t>(new rkaiq_cac_v03_api_attr_t);
#elif RKAIQ_HAVE_CAC_V10
        attr_ = std::unique_ptr<rkaiq_cac_v10_api_attr_t>(new rkaiq_cac_v10_api_attr_t);
#elif RKAIQ_HAVE_CAC_V11
        attr_ = std::unique_ptr<rkaiq_cac_v11_api_attr_t>(new rkaiq_cac_v11_api_attr_t);
#endif
    }

    attr_->op_mode = RKAIQ_CAC_API_OPMODE_AUTO;
    attr_->iso_cnt = calib_->TuningPara.SettingByIso_len;
    XCAM_ASSERT(attr_->iso_cnt <= RKAIQ_CAC_MAX_ISO_CNT);
    memcpy(attr_->auto_params, calib_->TuningPara.SettingByIso,
           sizeof(calib_->TuningPara.SettingByIso[0]) * attr_->iso_cnt);
    memcpy(&attr_->manual_param, calib_->TuningPara.SettingByIso,
           sizeof(calib_->TuningPara.SettingByIso[0]));
#if !RKAIQ_HAVE_CAC_V03
    memcpy(&attr_->persist_params, &calib->SettingPara, sizeof(calib->SettingPara));
    attr_->enable = attr_->persist_params.enable;

#else
    attr_->enable = enable_;
#endif

    if (!enable_) {
        attr_->enable = false;
        return XCAM_RETURN_BYPASS;
    }

    valid_ = true;
    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn CacAlgoAdaptor::Prepare(const RkAiqAlgoConfigAcac* config) {
    LutBufferConfig lut_config{};
    LutBufferConfig full_lut_config{};
    uint32_t width   = config->width;
    uint32_t height  = config->height;
    bool is_big_mode = IsIspBigMode(width, height, config->is_multi_sensor);
    char cac_map_path[RKCAC_MAX_PATH_LEN] = {0};

    LOGD_ACAC("%s : en %d valid: %d Enter", __func__, enable_, valid_);

    isReCal_ = true;

    if (!enable_ || !valid_) {
        return XCAM_RETURN_BYPASS;
    }

    config_ = config;
    if (config->is_multi_isp) {
#if (RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10) && defined(ISP_HW_V30) || RKAIQ_HAVE_CAC_V11
        CalcCacLutConfig(width, height, is_big_mode, full_lut_config);
        width = width / 2 + config->multi_isp_extended_pixel;
        CalcCacLutConfig(width, height, is_big_mode, lut_config);
#endif
    } else {
        CalcCacLutConfig(width, height, is_big_mode, lut_config);
    }
    if (lut_manger_ == nullptr) {
        lut_manger_ =
            std::unique_ptr<LutBufferManager>(new LutBufferManager(lut_config, config->mem_ops));
        lut_manger_->ImportHwBuffers(0);
        if (config->is_multi_isp) {
            lut_manger_->ImportHwBuffers(1);
        }
    }
    auto* buf = lut_manger_->GetFreeHwBuffer(0);
    if (buf == nullptr) {
        LOGW_ACAC("No buffer available, maybe only one buffer ?!");
        return XCAM_RETURN_NO_ERROR;
    }
    current_lut_.clear();
    current_lut_.emplace_back(buf);
    if (buf->State != LutBufferState::kInitial) {
        LOGW_ACAC("Buffer in use, will not update lut!");
        return XCAM_RETURN_NO_ERROR;
    }
#if (RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10) && defined(ISP_HW_V30) || RKAIQ_HAVE_CAC_V11
    if (config->is_multi_isp) {
        auto* buf = lut_manger_->GetFreeHwBuffer(1);
        if (buf == nullptr) {
            LOGW_ACAC("No buffer available, maybe only one buffer ?!");
            return XCAM_RETURN_NO_ERROR;
        }
        current_lut_.emplace_back(buf);
    }
#endif
    XCAM_ASSERT(current_lut_.size() == (uint32_t)(config->is_multi_isp + 1));

#if !RKAIQ_HAVE_CAC_V03
    if (attr_->persist_params.psf_path[0] != '/') {
        strcpy(cac_map_path, config->iqpath);
        strcat(cac_map_path, "/");
    }
    strcat(cac_map_path, attr_->persist_params.psf_path);

    std::ifstream ifs(cac_map_path, std::ios::binary);
    if (!ifs.is_open()) {
        LOGE_ACAC("Failed to open PSF file %s", cac_map_path);
        valid_ = false;
        return XCAM_RETURN_ERROR_FILE;
    }

    if (!config->is_multi_isp) {
        uint32_t line_offset = lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        uint32_t size = lut_config.LutHCount * lut_config.LutVCount * CacPsfKernelWordSizeInMemory *
                        BYTES_PER_WORD;
        for (int ch = 0; ch < CacChannelCount; ch++) {
            char* addr0 = reinterpret_cast<char*>(current_lut_[0]->Addr) + ch * size;
            ifs.read(addr0, size);
        }
    } else {
#if RKAIQ_HAVE_CAC_V10 && defined(ISP_HW_V30) || RKAIQ_HAVE_CAC_V11
        XCAM_ASSERT(current_lut_.size() > 1);
        // Read and Split Memory
        //   a == line_size - line_offset
        //   b == line_offset
        //   c == line_offset - a = 2 * line_offset - line_size
        // For each line:
        //   read b size to left
        //   copy c from left to right
        //   read a' to right
        // - +---------------------------+
        // | |<---a---->|  |  |<---a'--->|
        // | |                 |<-c->|          |
        // v |<---b---------->|          |
        // | |          |  |  |          |
        // - +---------------------------+
        //   |<---------line_size------->|
        //
        uint32_t line_offset = lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        uint32_t line_size =
            full_lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        for (int ch = 0; ch < CacChannelCount; ch++) {
            char* addr0 = reinterpret_cast<char*>(current_lut_[0]->Addr) +
                          ch * line_offset * lut_config.LutVCount;
            char* addr1 = reinterpret_cast<char*>(current_lut_[1]->Addr) +
                          ch * line_offset * lut_config.LutVCount;
            for (uint32_t i = 0; i < full_lut_config.LutVCount; i++) {
                ifs.read(addr0 + (i * line_offset), line_offset);
                memcpy(addr1 + (i * line_offset),
                       addr0 + (i * line_offset) + line_size - line_offset,
                       2 * line_offset - line_size);
                ifs.read(addr1 + (i * line_size) + line_offset, line_size - line_offset);
            }
        }
#endif
    }
#endif

    return XCAM_RETURN_NO_ERROR;
}

void CacAlgoAdaptor::OnFrameEvent(const RkAiqAlgoProcAcac* input, RkAiqAlgoProcResAcac* output) {
    int i;
    int iso_low  = 50;
    int iso_high = 50;
    int gain_high, gain_low;
    float ratio      = 1.0;
    int iso_div      = 50;
    int max_iso_step = attr_->iso_cnt;
    int iso          = input->iso;
    LOGD_ACAC("%s : en %d valid: %d Enter", __func__, enable_, valid_);

    if (current_lut_.empty()) {
        valid_ = false;
    } else {
        valid_ = true;
    }

    if (attr_->op_mode == RKAIQ_CAC_API_OPMODE_AUTO &&
        lastIso_ != input->iso)
        isReCal_ = true;

    lastIso_ = input->iso;
    if (!isReCal_) {
        output->res_com.cfg_update = false;
        LOGD_ACAC("skip cac reCalc");
        return ;
    }

    output->res_com.cfg_update = true;
    isReCal_ = false;

    if (!enable_ || !valid_) {
        output->config[0].bypass_en = 1;
#if (RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10) && defined(ISP_HW_V30)|| RKAIQ_HAVE_CAC_V11
        output->config[1].bypass_en = 1;
#endif
        output->enable = false;
        return;
    }

    for (i = max_iso_step - 1; i >= 0; i--) {
        if (iso < iso_div * (2 << i)) {
            iso_low  = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = static_cast<float>(iso - iso_low) / (iso_high - iso_low);
    if (iso_low == iso) {
        iso_high = iso;
        ratio    = 0;
    }
    if (iso_high == iso) {
        iso_low = iso;
        ratio   = 1;
    }
    gain_high = (int)(log(static_cast<float>(iso_high) / 50) / log(2.0));
    gain_low  = (int)(log(static_cast<float>(iso_low) / 50) / log(2.0));

    gain_low  = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high = MIN(MAX(gain_high, 0), max_iso_step - 1);

    XCAM_ASSERT(gain_low >= 0 && gain_low < max_iso_step);
    XCAM_ASSERT(gain_high >= 0 && gain_high < max_iso_step);

#if RKAIQ_HAVE_CAC_V03
    output->enable = attr_->enable;
    output->config[0].center_en     = 0;
    output->config[0].center_width  = 0;
    output->config[0].center_height = 0;
    output->config[0].psf_sft_bit   = 2;
    output->config[0].bypass_en = attr_->enable;
    output->config[0].cfg_num       = current_lut_[0]->Config.PsfCfgCount;
    output->config[0].buf_fd        = current_lut_[0]->Fd;
    output->config[0].hsize = current_lut_[0]->Config.LutHCount * CacPsfKernelWordSizeInMemory;
    output->config[0].vsize = current_lut_[0]->Config.LutVCount * CacChannelCount;
    if (config_->is_multi_isp) {
        memcpy(&output->config[1], &output->config[0], sizeof(output->config[0]));
        if (current_lut_.size() > 1) {
            output->config[1].buf_fd = current_lut_[1]->Fd;
        }
    }
#else //not  RKAIQ_HAVE_CAC_V03

#if 0
    output->config[0].strength[0] = 128;
    output->config[0].strength[1] = 256;
    output->config[0].strength[2] = 384;
    output->config[0].strength[3] = 512;
    output->config[0].strength[4] = 640;
    output->config[0].strength[5] = 768;
    output->config[0].strength[6] = 896;
    output->config[0].strength[7] = 1024;
    output->config[0].strength[8] = 1152;
    output->config[0].strength[9] = 1280;
    output->config[0].strength[10] = 1408;
    output->config[0].strength[11] = 1536;
    output->config[0].strength[12] = 1568;
    output->config[0].strength[13] = 1600;
    output->config[0].strength[14] = 1632;
    output->config[0].strength[15] = 1664;
    output->config[0].strength[16] = 1696;
    output->config[0].strength[17] = 1728;
    output->config[0].strength[18] = 1760;
    output->config[0].strength[19] = 1792;
    output->config[0].strength[20] = 1824;
    output->config[0].strength[21] = 2047;
#endif
    output->config[0].center_en     = attr_->persist_params.center_en;
    output->config[0].center_width  = attr_->persist_params.center_x;
    output->config[0].center_height = attr_->persist_params.center_y;
    output->config[0].psf_sft_bit   = attr_->persist_params.psf_shift_bits;
    output->config[0].cfg_num       = current_lut_[0]->Config.PsfCfgCount;
    output->config[0].buf_fd        = current_lut_[0]->Fd;
    output->config[0].hsize = current_lut_[0]->Config.LutHCount * CacPsfKernelWordSizeInMemory;
    output->config[0].vsize = current_lut_[0]->Config.LutVCount * CacChannelCount;

    if (attr_->op_mode == RKAIQ_CAC_API_OPMODE_MANUAL) {
        for (i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
            float strength = attr_->manual_param.global_strength > 0
                                 ? attr_->manual_param.global_strength
                                 : attr_->manual_param.strength_table[i];
            output->config[0].strength[i] = ROUND_F(attr_->manual_param.strength_table[i] * (1 << RKCAC_STRENGTH_FIX_BITS));
            output->config[0].strength[i] =
                output->config[0].strength[i] > 2047 ? 2047 : output->config[0].strength[i];
        }
        output->config[0].bypass_en = attr_->manual_param.bypass;
#if RKAIQ_HAVE_CAC_V11
        output->config[0].clip_g_mode    = attr_->manual_param.clip_g_mode;
        output->config[0].neg_clip0_en   = attr_->manual_param.neg_clip0_enable;
        output->config[0].edge_detect_en = attr_->manual_param.edge_detect_en;
        output->config[0].flat_thed_b = ROUND_F(attr_->manual_param.flat_thed_b * (1 << RKCAC_EDGE_DETECT_FIX_BITS));
        output->config[0].flat_thed_r = ROUND_F(attr_->manual_param.flat_thed_r * (1 << RKCAC_EDGE_DETECT_FIX_BITS));
        output->config[0].offset_r = CacClamp<uint32_t>(attr_->manual_param.offset_r, 0, 1 << 16);
            //ROUND_F(attr_->manual_param.offset_r);
        output->config[0].offset_b = CacClamp<uint32_t>(attr_->manual_param.offset_b, 0, 1 << 16);
            //ROUND_F(attr_->manual_param.offset_b);
        output->config[0].expo_thed_b =
            (!attr_->manual_param.expo_det_b_en << 20) | CacClamp<uint32_t>(attr_->manual_param.expo_thed_b, 0, 0xfffff);
        output->config[0].expo_thed_r =
            (!attr_->manual_param.expo_det_r_en << 20) | CacClamp<uint32_t>(attr_->manual_param.expo_thed_r, 0, 0xfffff);
        output->config[0].expo_adj_b = CacClamp<uint32_t>(attr_->manual_param.expo_adj_b, 0, 0xfffff);
        output->config[0].expo_adj_r = CacClamp<uint32_t>(attr_->manual_param.expo_adj_r, 0, 0xfffff);
        output->enable = attr_->enable;
#endif
    } else if (attr_->op_mode == RKAIQ_CAC_API_OPMODE_AUTO) {
        float strength[RKCAC_STRENGTH_TABLE_LEN] = {1.0f};
        float strenth_low = 0.0;
        float strenth_high = 0.0;
        for (i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
            strenth_low = attr_->auto_params[gain_low].global_strength > 0
                ? attr_->auto_params[gain_low].global_strength
                : attr_->auto_params[gain_low].strength_table[i];
            strenth_high = attr_->auto_params[gain_high].global_strength > 0
                ? attr_->auto_params[gain_high].global_strength
                : attr_->auto_params[gain_high].strength_table[i];
            strength[i] = INTERP_CAC(strenth_low,
                                     strenth_high, ratio);
            output->config[0].strength[i] = ROUND_F(strength[i] * (1 << RKCAC_STRENGTH_FIX_BITS));
            output->config[0].strength[i] =
                output->config[0].strength[i] > 2047 ? 2047 : output->config[0].strength[i];
        }
        output->config[0].bypass_en = INTERP_CAC(attr_->auto_params[gain_low].bypass,
                                                 attr_->auto_params[gain_high].bypass, ratio);

#if RKAIQ_HAVE_CAC_V11
        output->config[0].clip_g_mode =
            INTERP_CAC((int)attr_->auto_params[gain_low].clip_g_mode,
                       (int)attr_->auto_params[gain_high].clip_g_mode, ratio);
        output->config[0].neg_clip0_en =
            INTERP_CAC((int)attr_->auto_params[gain_low].neg_clip0_enable,
                       (int)attr_->auto_params[gain_high].neg_clip0_enable, ratio);
        output->config[0].edge_detect_en =
            INTERP_CAC((int)attr_->auto_params[gain_low].edge_detect_en,
                       (int)attr_->auto_params[gain_high].edge_detect_en, ratio);

        float flat_thed_b             = INTERP_CAC(attr_->auto_params[gain_low].flat_thed_b,
                                       attr_->auto_params[gain_high].flat_thed_b, ratio);
        output->config[0].flat_thed_b = ROUND_F(flat_thed_b * (1 << RKCAC_EDGE_DETECT_FIX_BITS));

        float flat_thed_r             = INTERP_CAC(attr_->auto_params[gain_low].flat_thed_r,
                                       attr_->auto_params[gain_high].flat_thed_r, ratio);
        output->config[0].flat_thed_r = ROUND_F(flat_thed_r * (1 << RKCAC_EDGE_DETECT_FIX_BITS));

        float offset_b             = INTERP_CAC(attr_->auto_params[gain_low].offset_b,
                                    attr_->auto_params[gain_high].offset_b, ratio);
        output->config[0].offset_b = CacClamp<uint32_t>(ROUND_F(input->hdr_ratio * offset_b), 0, 1 << 16);

        float offset_r             = INTERP_CAC(attr_->auto_params[gain_low].offset_r,
                                    attr_->auto_params[gain_high].offset_r, ratio);
        output->config[0].offset_r = CacClamp<uint32_t>(ROUND_F(input->hdr_ratio * offset_r), 0, 1 << 16);

        int exp_det_r_en = INTERP_CAC(attr_->auto_params[gain_low].expo_det_r_en,
                                       attr_->auto_params[gain_high].expo_det_r_en, ratio);
        int exp_det_b_en = INTERP_CAC(attr_->auto_params[gain_low].expo_det_b_en,
                                       attr_->auto_params[gain_high].expo_det_b_en, ratio);

        uint32_t expo_thed_b          = INTERP_CAC(attr_->auto_params[gain_low].expo_thed_b,
                                          attr_->auto_params[gain_high].expo_thed_b, ratio);
        uint32_t expo_thed_r          = INTERP_CAC(attr_->auto_params[gain_low].expo_thed_r,
                                          attr_->auto_params[gain_high].expo_thed_r, ratio);
        uint32_t expo_adj_b          = INTERP_CAC(attr_->auto_params[gain_low].expo_adj_b,
                                         attr_->auto_params[gain_high].expo_adj_b, ratio);
        uint32_t expo_adj_r          = INTERP_CAC(attr_->auto_params[gain_low].expo_adj_r,
                                         attr_->auto_params[gain_high].expo_adj_r, ratio);
        expo_thed_b = input->hdr_ratio * expo_thed_b;
        expo_thed_r = input->hdr_ratio * expo_thed_r;
        expo_adj_b = input->hdr_ratio * expo_adj_b;
        expo_adj_r = input->hdr_ratio * expo_adj_r;
        output->config[0].expo_thed_b =
            (static_cast<int>(!exp_det_b_en) << 20) | CacClamp<uint32_t>(expo_thed_b, 0, 0xfffff);
        output->config[0].expo_thed_r =
            (static_cast<int>(!exp_det_r_en) << 20) | CacClamp<uint32_t>(expo_thed_r, 0, 0xfffff);
        output->config[0].expo_adj_b = CacClamp<uint32_t>(expo_adj_b, 0, 0xfffff);
        output->config[0].expo_adj_r = CacClamp<uint32_t>(expo_adj_r, 0, 0xfffff);
#endif

#if (RKAIQ_HAVE_CAC_V10) && defined(ISP_HW_V30) || RKAIQ_HAVE_CAC_V11
        memcpy(&output->config[1], &output->config[0], sizeof(output->config[0]));
        if (current_lut_.size() > 1) {
            output->config[1].buf_fd = current_lut_[1]->Fd;
            if (output->config[0].center_en) {
                uint16_t w                     = config_->width / 4;
                uint16_t e                     = config_->multi_isp_extended_pixel / 4;
                uint16_t x                     = attr_->persist_params.center_x;
                output->config[1].center_width = x - (w / 2 - e);
            }
        }
#endif



    }
#endif
    output->enable = attr_->enable;

#if 0
    LOGD_ACAC("global en : %d", output->enable);
    LOGD_ACAC("center en: %d", output->config[0].center_en);
    LOGD_ACAC("center x: %u", output->config[0].center_width);
    LOGD_ACAC("center y: %u", output->config[0].center_height);
    LOGD_ACAC("psf shift bits: %u", output->config[0].psf_sft_bit);
    LOGD_ACAC("psf cfg num: %u", output->config[0].cfg_num);
    LOGD_ACAC("psf buf fd: %d", output->config[0].buf_fd);
#if (RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10) && defined(ISP_HW_V30)
    if (current_lut_.size() > 1) {
        LOGD_ACAC("psf buf fd right: %d", output->config[1].buf_fd);
        LOGD_ACAC("center x right: %d", output->config[1].center_width);
        LOGD_ACAC("center y right: %d", output->config[1].center_height);
    }
#endif
    LOGD_ACAC("psf hwsize: %u", output->config[0].hsize);
    LOGD_ACAC("psf vsize: %u", output->config[0].vsize);
    for (i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        LOGD_ACAC("strength %d: %u", i, output->config[0].strength[i]);
    }
#if RKAIQ_HAVE_CAC_V11
    LOGD_ACAC("clip_g_mode: %u", output->config[0].clip_g_mode);
    LOGD_ACAC("edge_detect_en: %u", output->config[0].edge_detect_en);
    LOGD_ACAC("neg_clip0_en: %u", output->config[0].neg_clip0_en);
    LOGD_ACAC("expo_thed_b: %u", output->config[0].expo_thed_b);
    LOGD_ACAC("expo_thed_r: %u", output->config[0].expo_thed_r);
    LOGD_ACAC("expo_adj_b: %u", output->config[0].expo_adj_b);
    LOGD_ACAC("expo_adj_r: %u", output->config[0].expo_adj_r);
    LOGD_ACAC("flat_thed_b: %u", output->config[0].flat_thed_b);
    LOGD_ACAC("flat_thed_r: %u", output->config[0].flat_thed_r);
#endif
#endif
}

}  // namespace RkCam
