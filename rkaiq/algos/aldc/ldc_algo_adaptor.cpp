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
 */

#include "ldc_algo_adaptor.h"

#include <fcntl.h>

#include <fstream>
#include <iostream>

#define __STDC_FORMAT_MACROS
#include <cinttypes>

#include "algos/aldc/rk_aiq_types_aldc_algo.h"
#include "algos/aldc/rk_aiq_types_aldc_algo_int.h"
#include "common/rk_aiq_comm.h"
#include "common/rk_aiq_types_priv.h"
#include "xcore/base/xcam_log.h"

namespace RkCam {

LdcAlgoAdaptor::~LdcAlgoAdaptor() {
    if (ldc_gen_mesh_thread_->is_running()) {
        ldc_gen_mesh_thread_->triger_stop();
        ldc_gen_mesh_thread_->stop();
    }
};

void LdcAlgoAdaptor::GenMeshDoneCallback(const LdcGenMeshCfg_t& cfg) {
    ENTER_ALDC_FUNCTION();

    LOGD_ALDC("Generate the mesh of level %d successfully", cfg.level);

    ldch_algo_adaptee_->UpdateMesh(cfg.level, cfg.ldch.fd);
    ldcv_algo_adaptee_->UpdateMesh(cfg.level, cfg.ldcv.fd);

    EXIT_ALDC_FUNCTION();
}

void LdcAlgoAdaptor::GenMeshFailedCallback(const int32_t fd) {
    ENTER_ALDC_FUNCTION();

    LOGE_ALDC("Failed to  generate mesh, buf fd: %d", fd);

    EXIT_ALDC_FUNCTION();
}

XCamReturn LdcAlgoAdaptor::SetApiAttr(const rk_aiq_ldc_attrib_t* attr) {
    ENTER_ALDC_FUNCTION();

    if (!ldch_algo_adaptee_ || !ldcv_algo_adaptee_ || !attr) return XCAM_RETURN_ERROR_PARAM;

    LOGD_ALDC("attr: ldch/v correct_level: %d/%d", attr->ldch.correct_level,
              attr->ldcv.correct_level);

    if (!valid_) return XCAM_RETURN_BYPASS;

    bool isReGenMesh_ = false;
    // 1.update ldch attribute.
    rk_aiq_ldc_ldch_attrib_t ldch_attrib;
    ldch_algo_adaptee_->GetApiAttr(&ldch_attrib);
    if (0 != memcmp(&ldch_attrib, &attr->ldch, sizeof(attr))) {
        ldch_algo_adaptee_->SetApiAttr(&attr->ldch);

        if (ldch_attrib.correct_level != attr->ldch.correct_level) isReGenMesh_ = true;
    }

    // 3.update ldcv attribute.
    rk_aiq_ldc_ldcv_attrib_t ldcv_attrib;
    ldcv_algo_adaptee_->GetApiAttr(&ldcv_attrib);
    if (0 != memcmp(&ldcv_attrib, &attr, sizeof(attr))) {
        ldcv_algo_adaptee_->SetApiAttr(&attr->ldcv);

        if (ldcv_attrib.correct_level != attr->ldcv.correct_level) isReGenMesh_ = true;
    }

    // 3.triger generating new mesh.
    if (isReGenMesh_) {
        RKAIQLdcUpdateLutMode ldch_upd = ldch_algo_adaptee_->GetUpdLutBufMode();
        RKAIQLdcUpdateLutMode ldcv_upd = ldcv_algo_adaptee_->GetUpdLutBufMode();
        if (ldch_upd == kRKAIQLdcUpdateLutOnLine || ldcv_upd == kRKAIQLdcUpdateLutOnLine) {
            SmartPtr<LdcGenMeshCfg_t> cfg = new LdcGenMeshCfg_t;
            memset(cfg.ptr(), 0, sizeof(*cfg.ptr()));

            if (ldcv_algo_adaptee_->IsEnabled() && ldch_upd == kRKAIQLdcUpdateLutOnLine) {
                cfg->level     = ldch_algo_adaptee_->GetCurAttrib()->correct_level;
                cfg->ldch.addr = ldch_algo_adaptee_->GetFreeLutBuf(0, &cfg->ldch.fd);
            }
            if (ldcv_algo_adaptee_->IsEnabled() && ldcv_upd == kRKAIQLdcUpdateLutOnLine) {
                cfg->level     = ldcv_algo_adaptee_->GetCurAttrib()->correct_level;
                cfg->ldcv.addr = ldcv_algo_adaptee_->GetFreeLutBuf(0, &cfg->ldcv.fd);
            }

            if (ldc_gen_mesh_thread_ && (cfg->ldch.addr || cfg->ldcv.addr)) {
                if (!ldc_gen_mesh_thread_->is_running()) {
                    ldc_gen_mesh_thread_->triger_start();
                    ldc_gen_mesh_thread_->start();
                }
                ldc_gen_mesh_thread_->push_attr(cfg);

                ldch_algo_adaptee_->SetGenMeshState(LdcGenMeshState::kLdcGenMeshWorking);
                ldcv_algo_adaptee_->SetGenMeshState(LdcGenMeshState::kLdcGenMeshWorking);
            }
        }

        isReGenMesh_ = false;
    }

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcAlgoAdaptor::GetApiAttr(rk_aiq_ldc_attrib_t* attr) {
    ENTER_ALDC_FUNCTION();

    if (!ldch_algo_adaptee_ || !ldcv_algo_adaptee_ || !attr) return XCAM_RETURN_ERROR_PARAM;

    ldch_algo_adaptee_->GetApiAttr(&attr->ldch);
    ldcv_algo_adaptee_->GetApiAttr(&attr->ldcv);

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcAlgoAdaptor::Config(const AlgoCtxInstanceCfg* config, const int32_t conf_type,
                                  const CalibDbV2_LDC_t* calib) {
    ENTER_ALDC_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ctx_config_ = config;

    if (ldch_algo_adaptee_ == nullptr)
        ldch_algo_adaptee_ = std::unique_ptr<LdchAlgoAdaptee>(new LdchAlgoAdaptee);

    if (ldcv_algo_adaptee_ == nullptr)
        ldcv_algo_adaptee_ = std::unique_ptr<LdcvAlgoAdaptee>(new LdcvAlgoAdaptee);

    if (!ldc_algo_gen_mesh_.ptr()) ldc_algo_gen_mesh_ = new LdcAlgoGenMesh;

    if (ldc_gen_mesh_thread_ == nullptr) {
        ldc_gen_mesh_thread_ =
            std::unique_ptr<LdcGenMeshThread>(new LdcGenMeshThread(ldc_algo_gen_mesh_));
        ldc_gen_mesh_thread_->SetGenMeshCallback(this);
    }

    if (ldch_algo_adaptee_.get()) ldch_algo_adaptee_->Config(config, conf_type, calib);
    if (ldcv_algo_adaptee_.get()) ldcv_algo_adaptee_->Config(config, conf_type, calib);

    ldch_en_ = ldch_algo_adaptee_->IsEnabled();
    ldcv_en_ = ldcv_algo_adaptee_->IsEnabled();
    enable_  = ldch_en_ | ldcv_en_;
    if (!enable_) return XCAM_RETURN_BYPASS;

    valid_ = true;

    EXIT_ALDC_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcAlgoAdaptor::Prepare(const RkAiqAlgoConfigAldc* config) {
    ENTER_ALDC_FUNCTION();
    LdcLutBufferConfig lut_config{};
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!valid_) return XCAM_RETURN_BYPASS;

    ldch_algo_adaptee_->Prepare(config);
    ldcv_algo_adaptee_->Prepare(config);

    ldch_en_ = ldch_algo_adaptee_->IsEnabled();
    ldcv_en_ = ldcv_algo_adaptee_->IsEnabled();
    enable_  = ldch_en_ | ldcv_en_;

    if (!enable_) return XCAM_RETURN_BYPASS;

    auto* calibv2   = config->com.u.prepare.calibv2;
    auto* calib_ldc = (CalibDbV2_LDC_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, ldc));
    int32_t srcW    = config->com.u.prepare.sns_op_width;
    int32_t srcH    = config->com.u.prepare.sns_op_height;
    int32_t dstW    = config->com.u.prepare.sns_op_width;
    int32_t dstH    = config->com.u.prepare.sns_op_height;

    if (!srcW && !srcH) return XCAM_RETURN_BYPASS;

    RKAIQLdcUpdateLutMode ldch_upd = ldch_algo_adaptee_->GetUpdLutBufMode();
    RKAIQLdcUpdateLutMode ldcv_upd = ldcv_algo_adaptee_->GetUpdLutBufMode();
    if (ldch_upd == kRKAIQLdcUpdateLutOnLine || ldcv_upd == kRKAIQLdcUpdateLutOnLine) {
        uint32_t level          = calib_ldc->param.correct_level;
        uint32_t cur_ldch_level = ldch_algo_adaptee_->GetCorrectLevel();
        uint32_t cur_ldcv_level = ldcv_algo_adaptee_->GetCorrectLevel();
        if (started_ &&
            ((ldch_en_ && level == cur_ldch_level) || (ldch_en_ && level == cur_ldch_level))) {
            LOGD_ALDC("Has been started and the level of correction has not changed ");
            goto end;
        }

        if (!ldc_gen_mesh_thread_->is_empty()) ldc_gen_mesh_thread_->clear_attr();
        if (ldc_gen_mesh_thread_->is_running()) {
            ldc_gen_mesh_thread_->triger_stop();
            ldc_gen_mesh_thread_->stop();
        }

        uint16_t *pMeshX = nullptr, *pMeshY = nullptr;
        int32_t h_mesh_fd = 0, v_mesh_fd = 0;
        LdcEnState state = LdcEnState::kLdcEnLdchLdcv;
        if (ldch_en_ && ldcv_en_ &&
            (ldch_upd == kRKAIQLdcUpdateLutOnLine || ldcv_upd == kRKAIQLdcUpdateLutOnLine)) {
            state  = LdcEnState::kLdcEnLdchLdcv;
            pMeshX = reinterpret_cast<uint16_t*>(ldch_algo_adaptee_->GetFreeLutBuf(0, &h_mesh_fd));
            pMeshY = reinterpret_cast<uint16_t*>(ldcv_algo_adaptee_->GetFreeLutBuf(0, &v_mesh_fd));
        } else if (ldch_en_ && ldch_upd == kRKAIQLdcUpdateLutOnLine) {
            state  = LdcEnState::kLdcEnLdch;
            pMeshX = reinterpret_cast<uint16_t*>(ldch_algo_adaptee_->GetFreeLutBuf(0, &h_mesh_fd));
        } else if (ldcv_en_ && ldcv_upd == kRKAIQLdcUpdateLutOnLine) {
            state  = LdcEnState::kLdcEnLdcv;
            pMeshY = reinterpret_cast<uint16_t*>(ldcv_algo_adaptee_->GetFreeLutBuf(0, &v_mesh_fd));
        }

        if (ldc_algo_gen_mesh_->Init(state, srcW, srcH, dstW, dstH, calib_ldc) < 0) {
            LOGE_ALDC("Failed to init ldc GenMesh");
        } else {
            ret = ldc_algo_gen_mesh_->GenMeshNLevel(0, level, pMeshX, pMeshY);
            if (ret < 0) {
                LOGE_ALDC("Failed to GenMeshNLevel: %d", calib_ldc->param.correct_level);
            }
        }

        if (ldch_en_) ldch_algo_adaptee_->UpdateMesh(level, h_mesh_fd);
        if (ldcv_en_) ldcv_algo_adaptee_->UpdateMesh(level, v_mesh_fd);

        ldc_gen_mesh_thread_->triger_start();
        ldc_gen_mesh_thread_->start();
    }

end:
    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void LdcAlgoAdaptor::OnFrameEvent(const RkAiqAlgoProcAldc* input, RkAiqAlgoProcResAldc* output) {
    ENTER_ALDC_FUNCTION();

    if (!enable_) {
        output->res_com.cfg_update = false;
        LOGV_ALDC("skip ldc reCalc");
        if (input->com.u.proc.init) {
            output->res_com.cfg_update = true;
            output->ldch_en = false;
            output->ldcv_en = false;
        }
        return;
    }

    output->res_com.cfg_update = true;

    output->ldch_en = false;
    output->ldcv_en = false;

    if (ldch_en_) ldch_algo_adaptee_->OnFrameEvent(input, output);
    if (ldcv_en_) ldcv_algo_adaptee_->OnFrameEvent(input, output);

    if (ldch_en_ || ldcv_en_) started_ = true;

    EXIT_ALDC_FUNCTION();
}

}  // namespace RkCam
