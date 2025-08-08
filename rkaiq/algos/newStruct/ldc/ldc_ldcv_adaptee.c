/*
 * rk_aiq_algo_aldcv_algo.cpp
 *
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

#include "algos/newStruct/ldc/include/ldc_ldcv_adaptee.h"

#include "algos/algo_types_priv.h"
#include "algos/newStruct/ldc/ldc_types_prvt.h"
#include "algos/rk_aiq_algo_des.h"
#include "common/rk_aiq_types_priv_c.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_common.h"

#define ALDC_LDCV_SUBM (0x1)

void LdcvAdaptee_init(LdcvAdaptee* ldcv) {
    aiqMutex_init(&ldcv->_mutex);

    ldcv->enable_         = false;
    ldcv->started_        = false;
    ldcv->valid_          = false;
    ldcv->isReCal_        = false;
    ldcv->correct_level_  = 0;
    ldcv->gen_mesh_state_ = kLdcGenMeshIdle;
    ldcv->calib_          = NULL;
    ldcv->config_         = NULL;

    ldcv->lut_manger_ = NULL;
    for (int i = 0; i < 2; i++) {
        ldcv->cached_lut_[i].Addr     = NULL;
        ldcv->cached_lut_[i].Fd       = LDC_BUF_FD_DEFAULT;
        ldcv->chip_in_use_lut[i].addr = NULL;
        ldcv->chip_in_use_lut[i].fd   = LDC_BUF_FD_DEFAULT;
    }
};

void LdcvAdaptee_deinit(LdcvAdaptee* ldcv) {
    aiqMutex_deInit(&ldcv->_mutex);

    if (ldcv->lut_manger_) {
        LdcLutBufMng_deinit(ldcv->lut_manger_);
        aiq_free(ldcv->lut_manger_);
        ldcv->lut_manger_ = NULL;
    }
};

const LdcLutBuffer* LdcvAdaptee_getFreeLutBuf(LdcvAdaptee* ldcv, int8_t isp_id) {
    if (ldcv->lut_manger_ == NULL) {
        LdcLutBufferConfig lut_config = {};

        if (ldcv->config_) {
            lut_config.Width  = ldcv->config_->com.u.prepare.sns_op_width;
            lut_config.Height = ldcv->config_->com.u.prepare.sns_op_height;
        }

        ldcv->lut_manger_ = (LdcLutBufferManager*)aiq_mallocz(sizeof(*ldcv->lut_manger_));
        LdcLutBufMng_init(ldcv->lut_manger_, lut_config, ldcv->config_->mem_ops);
        LdcLutBufMng_importHwBuffers(ldcv->lut_manger_, 0, MEM_TYPE_LDCV);
    }

    aiqMutex_lock(&ldcv->_mutex);
    if (ldcv->cached_lut_[0].Fd != LDC_BUF_FD_DEFAULT) {
        LOGW_ALDC_SUBM(ALDC_LDCV_SUBM, "the cached lut of LDCV is in use!!");
        aiqMutex_unlock(&ldcv->_mutex);
        return NULL;
    }
    aiqMutex_unlock(&ldcv->_mutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret            = LdcLutBufMng_getFreeHwBuffer(ldcv->lut_manger_, 0, &ldcv->cached_lut_[0]);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGW_ALDC_SUBM(ALDC_LDCV_SUBM, "LDCV hasn't buffer available, maybe only one buffer ?!");
        return NULL;
    }

    if (ldcv->cached_lut_[0].State != kLdcLutBufWait2Chip) {
        LOGW_ALDC_SUBM(ALDC_LDCV_SUBM, "Buffer in use, will not update lut!");
        return NULL;
    }

    LOGD_ALDC_SUBM(ALDC_LDCV_SUBM, "LDCV get lut buf fd %d", ldcv->cached_lut_[0].Fd);

    return &ldcv->cached_lut_[0];
}

XCamReturn LdcvAdaptee_updateMesh(LdcvAdaptee* ldcv, uint8_t level, LdcMeshBufInfo_t cfg) {
    ENTER_ALDC_FUNCTION();

    if (ldcv->cached_lut_[0].Fd == LDC_BUF_FD_DEFAULT) return XCAM_RETURN_BYPASS;

    LOGD_ALDC_SUBM(ALDC_LDCV_SUBM, "LDCV update mesh fd: correct level %d", level);

    aiqMutex_lock(&ldcv->_mutex);
    ldcv->enable_                 = (ldcv->calib_->tunning.enMode == LDC_LDCH_LDCV_EN);
    ldcv->correct_level_          = level;
    ldcv->chip_in_use_lut[0].fd   = ldcv->cached_lut_[0].Fd;
    ldcv->chip_in_use_lut[0].addr = ldcv->cached_lut_[0].Addr;
    ldcv->gen_mesh_state_         = kLdcGenMeshFinish;

    ldcv->cached_lut_[0].Fd = LDC_BUF_FD_DEFAULT;
    aiqMutex_unlock(&ldcv->_mutex);

    LOGD_ALDC_SUBM(ALDC_LDCV_SUBM, "%s LDCV and update chip_in_use_lut_fd_ %d",
                   ldcv->enable_ ? "on" : "off", ldcv->chip_in_use_lut[0].fd);

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcvAdaptee_config(LdcvAdaptee* ldcv, const ldc_api_attrib_t* calib) {
    ENTER_ALDC_FUNCTION();

    ldcv->calib_ = calib;
    ldcv->valid_ = true;

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcvAdaptee_prepare(LdcvAdaptee* ldcv, const RkAiqAlgoConfigLdc* config) {
    ENTER_ALDC_FUNCTION();

    LOGD_ALDC_SUBM(ALDC_LDCV_SUBM, "%s : LDCV en %d valid: %d", __func__, ldcv->enable_,
                   ldcv->valid_);

    ldcv->isReCal_ = true;
    ldcv->config_  = config;

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void LdcvAdaptee_onFrameEvent(LdcvAdaptee* ldcv, const RkAiqAlgoCom* inparams,
                              RkAiqAlgoResCom* outparams) {
    ENTER_ALDC_FUNCTION();

    aiqMutex_lock(&ldcv->_mutex);
    if (ldcv->gen_mesh_state_ == kLdcGenMeshFinish) {
        ldcv->gen_mesh_state_ = kLdcGenMeshIdle;
        ldcv->isReCal_        = true;
    }

    if (inparams->u.proc.is_attrib_update) {
        ldcv->isReCal_ = true;
        /*
         * 1. off -> on:  enable LDCV only after the mesh table is generated asynchronously.
         * 2. on -> off:  diable the LDCV immediately.
         */
        if (ldcv->calib_->tunning.enMode != LDC_LDCH_LDCV_EN) ldcv->enable_ = false;
    }

    if (!ldcv->isReCal_) {
        LOGV_ALDC_SUBM(ALDC_LDCV_SUBM, "Skip update");
        aiqMutex_unlock(&ldcv->_mutex);
        return;
    }

    ldcv->isReCal_ = false;

    ldc_param_t* ldcRes            = (ldc_param_t*)(outparams->algoRes);
    ldc_ldcv_params_cfg_t* userCfg = &ldcRes->sta.ldcvCfg;

    if (ldcv->enable_) {
        if (ldcv->chip_in_use_lut[0].fd >= 0) {
            userCfg->lutMapCfg.sw_ldcT_lutMapBuf_fd[0] = ldcv->chip_in_use_lut[0].fd;
            ldcv->chip_in_use_lut[0].fd                = LDC_BUF_FD_DEFAULT;
            userCfg->en = ldcv->enable_ = true;
        }
    }
    userCfg->en = ldcv->enable_;
    aiqMutex_unlock(&ldcv->_mutex);

    LOGD_ALDC_SUBM(ALDC_LDCV_SUBM, "LDCV result: cfg upd %d, en %d, mesh level %d, buf_fd %d",
                   outparams->cfg_update, userCfg->en, ldcv->correct_level_,
                   userCfg->lutMapCfg.sw_ldcT_lutMapBuf_fd[0]);

    EXIT_ALDC_FUNCTION();
}
