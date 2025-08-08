/*
 * rk_aiq_algo_aldch_algo.cpp
 *
 *  Copyright (c) 2023 Rockchip Corporation
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

#include "algos/aldc/rk_aiq_algo_aldch_adaptee.h"

#include <string.h>
#include <sys/stat.h>

#include "algos/rk_aiq_algo_des.h"
#include "algos/rk_aiq_algo_types.h"
#include "common/rk_aiq_types_priv.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_common.h"

namespace RkCam {

#define ALDC_LDCH_SUBM (0x1)

static const uint8_t default_bic_table[9][4] = {
    {0x00, 0x80, 0x00, 0x00},  // table0: 0, 0, 128, 0
    {0xfc, 0x7f, 0x05, 0x00},  // table1: 0, 5, 127, -4
    {0xfa, 0x7b, 0x0c, 0xff},  // table2: -1, 12, 123, -6
    {0xf8, 0x76, 0x14, 0xfe},  // table3: -2, 20, 118, -8
    {0xf7, 0x6f, 0x1d, 0xfd},  // table4: -3, 29, 111, -9
    {0xf7, 0x66, 0x27, 0xfc},  // table4: -4, 39, 102, -9
    {0xf7, 0x5d, 0x32, 0xfa},  // table4: -6, 50, 93, -9
    {0xf7, 0x53, 0x3d, 0xf9},  // table4: -7, 61, 83, -9
    {0xf8, 0x48, 0x48, 0xf8},  // table4: -8, 72, 72, -8
};

static inline void CalcCacLutConfig(uint32_t width, uint32_t height, LdcLutBufferConfig& config) {}

LdchAlgoAdaptee::~LdchAlgoAdaptee() {
    cached_lut_.clear();
    lut_manger_ = nullptr;
};

XCamReturn LdchAlgoAdaptee::UpdateMesh(const uint8_t level, const int32_t fd) {
    ENTER_ALDC_FUNCTION();

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "LDCH update mesh fd: correct level %d, cached lut size %d",
                   level, cached_lut_.size());

    if (cached_lut_.size() > 0) {
        chip_in_use_lut_fd_.clear();
        chip_in_use_lut_fd_.emplace_back(cached_lut_[0]->Fd);

        gen_mesh_state_.store(LdcGenMeshState::kLdcGenMeshFinish, std::memory_order_release);
        correct_level_ = level;

        LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "LDCH update chip_in_use_lut_fd_ %d",
                       chip_in_use_lut_fd_[0]);
    }

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdchAlgoAdaptee::SetApiAttr(const rk_aiq_ldc_ldch_attrib_t* attr) {
    ENTER_ALDC_FUNCTION();
    if (!attr_) attr_ = std::unique_ptr<rk_aiq_ldc_ldch_attrib_t>(new rk_aiq_ldc_ldch_attrib_t);

    if (0 != memcmp(attr_.get(), &attr, sizeof(attr))) {
        memcpy(attr_.get(), attr, sizeof(*attr));
        isReCal_ = true;
    }

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdchAlgoAdaptee::GetApiAttr(rk_aiq_ldc_ldch_attrib_t* attr) {
    ENTER_ALDC_FUNCTION();
    if (!attr_) return XCAM_RETURN_ERROR_FAILED;
    memcpy(attr, attr_.get(), sizeof(*attr));
    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdchAlgoAdaptee::Config(const AlgoCtxInstanceCfg* config, const int32_t conf_type,
                                   const CalibDbV2_LDC_t* calib) {
    ENTER_ALDC_FUNCTION();
    int32_t level = 0;

    (void)(config);

    if (conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
        calib_ = calib;
        return XCAM_RETURN_NO_ERROR;
    }

    ctx_config_ = config;
    calib_      = calib;
    enable_     = calib != nullptr ? calib->param.ldch_en : false;
    level       = calib != nullptr ? calib->param.correct_level : 0;

    if (!attr_) attr_ = std::unique_ptr<rk_aiq_ldc_ldch_attrib_t>(new rk_aiq_ldc_ldch_attrib_t);

    if (!enable_) {
        return XCAM_RETURN_BYPASS;
    }

    attr_->correct_level = level;
    memcpy(attr_->bic_weight, default_bic_table, sizeof(default_bic_table));

    valid_ = true;

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdchAlgoAdaptee::Prepare(const RkAiqAlgoConfigAldc* config) {
    ENTER_ALDC_FUNCTION();

    uint32_t width  = config->com.u.prepare.sns_op_width;
    uint32_t height = config->com.u.prepare.sns_op_height;

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "%s : LDCH en %d valid: %d", __func__, enable_, valid_);

    isReCal_ = true;

    if (!enable_ || !valid_) return XCAM_RETURN_BYPASS;

    config_ = config;

    char file_name[600] = {};
    sprintf(file_name, "%s/%s/ldch_mesh.bin", config->iqpath, calib_->param.ldch_meshfile);
    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "%s : try reading mesh from %s", __FUNCTION__, file_name);
    if (access(file_name, F_OK) == 0) {
        int32_t buf_fd = 0;
        void* lut_buf  = GetFreeLutBuf(0, &buf_fd);
        if (!lut_buf) {
            LOGE_ALDC_SUBM(ALDC_LDCH_SUBM, "%s : Failed to get lut buffer, disable LDCH",
                           __FUNCTION__);
            enable_ = false;
        }

        upd_lut_mode_ = kRKAIQLdcUpdateLutFromFile;
        if (ReadMeshFromFile(file_name, lut_buf, cached_lut_[0]->Config) < 0) {
            LOGW_ALDC_SUBM(ALDC_LDCH_SUBM, "%s : Failed to read mesh from %s, disable LDCH",
                           __FUNCTION__, file_name);
            enable_ = false;
        } else {
            UpdateMesh(0, buf_fd);
        }
    }

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void LdchAlgoAdaptee::OnFrameEvent(const RkAiqAlgoProcAldc* input, RkAiqAlgoProcResAldc* output) {
    ENTER_ALDC_FUNCTION();

    if (cached_lut_.empty()) {
        valid_ = false;
    } else {
        valid_ = true;
    }

    if (upd_lut_mode_ == kRKAIQLdcUpdateLutOnLine) {
        if (gen_mesh_state_.load(std::memory_order_acquire) == LdcGenMeshState::kLdcGenMeshFinish) {
            gen_mesh_state_.store(LdcGenMeshState::kLdcGenMeshIdle, std::memory_order_release);
            isReCal_ = true;
        }
    }

    if (!isReCal_) {
        output->res_com.cfg_update = false;
        LOGV_ALDC_SUBM(ALDC_LDCH_SUBM, "Skip update");
        return;
    }

    if (gen_mesh_state_.load(std::memory_order_acquire) == LdcGenMeshState::kLdcGenMeshFinish)
        gen_mesh_state_.store(LdcGenMeshState::kLdcGenMeshIdle, std::memory_order_release);

    isReCal_        = false;
    output->ldch_en = enable_;

    auto* result   = &output->config->ldch;
    int32_t width  = config_->com.u.prepare.sns_op_width;
    int32_t height = config_->com.u.prepare.sns_op_height;

    if (chip_in_use_lut_fd_.size() > 0 && chip_in_use_lut_fd_[0] >= 0) {
        result->buf_fd = chip_in_use_lut_fd_[0];
    } else {
        LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "mesh buf fd is invalid, disable LDCH");
        output->ldch_en = enable_  = 0;
        output->res_com.cfg_update = false;
        return;
    }

    result->frm_end_dis   = 0;
    result->sample_avr_en = 0;
    result->bic_mode_en   = 1;
    result->force_map_en  = 0;
    result->map13p3_en    = 0;
    memcpy(result->bicubic, default_bic_table, sizeof(default_bic_table));
    result->out_hsize = width;
    result->hsize     = (((width + 15) / 16 + 1) + 1) / 2;
    result->vsize     = (height + 7) / 8 + 1;

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM,
                   "LDCH result bic_mode_en %d, force_map_en %d, frm_end_dis %d, "
                   "map13p3_en %d, sample_avr_en %d",
                   result->bic_mode_en, result->force_map_en, result->frm_end_dis,
                   result->map13p3_en, result->sample_avr_en);
    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "LDCH result mesh level %d, buf_fd %d, h/vsize: %dx%d",
                   correct_level_, result->buf_fd, result->hsize, result->vsize);

    EXIT_ALDC_FUNCTION();
}

void* LdchAlgoAdaptee::GetFreeLutBuf(int8_t isp_id, int32_t* out_fd) {
    if (lut_manger_ == nullptr) {
        LdcLutBufferConfig lut_config{};

        lut_config.Width  = config_->com.u.prepare.sns_op_width;
        lut_config.Height = config_->com.u.prepare.sns_op_height;

        lut_manger_ = std::unique_ptr<LdcLutBufferManager>(
            new LdcLutBufferManager(lut_config, config_->mem_ops));
        lut_manger_->ImportHwBuffers(0, MEM_TYPE_LDCH);
    }

    auto* buf = lut_manger_->GetFreeHwBuffer(0);
    if (buf == nullptr) {
        LOGW_ALDC_SUBM(ALDC_LDCH_SUBM, "No buffer available, maybe only one buffer ?!");
        return nullptr;
    }

    if (buf->State != LdcLutBufferState::kWait2Chip) {
        LOGW_ALDC_SUBM(ALDC_LDCH_SUBM, "Buffer in use, will not update lut!");
        return nullptr;
    }

    cached_lut_.clear();
    cached_lut_.emplace_back(buf);

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "LDCH get lut buf fd %d, cached lut size %d", buf->Fd,
                   cached_lut_.size());

    if (out_fd) *out_fd = buf->Fd;
    return cached_lut_.size() > 0 ? cached_lut_[0]->Addr : nullptr;
}

XCamReturn LdchAlgoAdaptee::ReadMeshFromFile(const char* file_name, const void* addr,
                                             LdcLutBufferConfig& config) {
    FILE* ofp;
    ofp = fopen(file_name, "rb");
    if (ofp != NULL) {
        uint32_t lut_size = 0;
#ifdef HAVE_HEADINFO
        uint16_t hpic = 0, vpic = 0, hsize = 0, vsize = 0, hstep = 0, vstep = 0;

        fread(&hpic, sizeof(uint16_t), 1, ofp);
        fread(&vpic, sizeof(uint16_t), 1, ofp);
        fread(&hsize, sizeof(uint16_t), 1, ofp);
        fread(&vsize, sizeof(uint16_t), 1, ofp);
        fread(&hstep, sizeof(uint16_t), 1, ofp);
        fread(&vstep, sizeof(uint16_t), 1, ofp);

        LOGI_ALDC("head info: hpic %d, vpic %d, hsize %d, vsize %d, hstep %d, vstep %d", hpic, vpic,
                  hsize, vsize, hstep, vstep);

        if (config.Width != hpic || config.Height != vpic) {
            LOGE_ALDC("mismatched pic size in lut file : src: %dx%d, lut: %dx%d", config.Width,
                      config.Height, hpic, vpic);
            return XCAM_RETURN_ERROR_FAILED;
        }

        config.LutHCount = hsize;
        config.LutVCount = vsize;
        config.LutHStep  = hstep;
        config.LutVStep  = vstep;

        lut_size = hsize * vsize * sizeof(uint16_t);
#else
        struct stat file_stat;
        if (fstat(fileno(ofp), &file_stat) == -1) {
            LOGE_ALDC("Failed to get file size");
            fclose(ofp);
            return XCAM_RETURN_ERROR_FAILED;
        }

        LOGI_ALDC("%s size %d", file_name, file_stat.st_size);
        lut_size     = file_stat.st_size;
#endif
        uint16_t* buf = (uint16_t*)addr;
#if 1
        for (int i = 0; i < 137; i++) {
            size_t bytesRead = fread(buf + i * 122, 1, 121 * 2, ofp);
            if (bytesRead == 0) break;

            buf[i * 122 + 121] = 0;
        }
#else
        uint16_t num = fread((void*)addr, 1, lut_size, ofp);
#endif
        fclose(ofp);

        LOGD_ALDC("ldcv lut[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", buf[0], buf[1],
                  buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11],
                  buf[12], buf[13], buf[14], buf[15]);

#ifdef HAVE_HEADINFO
        if (num != lut_size) {
            LOGE_ALDCH("mismatched the size of mesh");
            return XCAM_RETURN_ERROR_FAILED;
        }
#endif

        LOGI_ALDC("the mesh size is %d in %s", lut_size, file_name);
    } else {
        LOGW_ALDC("lut file %s not exist", file_name);
        return XCAM_RETURN_ERROR_FAILED;
    }

    return XCAM_RETURN_NO_ERROR;
}

}  // namespace RkCam
