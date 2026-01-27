/*
 * rk_aiq_algo_aldch_algo.cpp
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

#include "algos/newStruct/ldc/include/ldc_ldch_adaptee.h"

#include "algos/algo_types_priv.h"
#include "algos/newStruct/ldc/ldc_types_prvt.h"
#include "algos/rk_aiq_algo_des.h"
#include "common/rk_aiq_types_priv_c.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "xcore/base/xcam_common.h"

#define ALDC_LDCH_SUBM (0x1)

LdcLutBuffer* LdchAdaptee_getFreeLutBuf(LdchAdaptee* ldch, int8_t isp_id) {
    if (ldch->lut_manger_ == NULL) {
        LdcLutBufferConfig lut_config = {};

        if (ldch->config_) {
            lut_config.Width  = ldch->config_->com.u.prepare.sns_op_width;
            lut_config.Height = ldch->config_->com.u.prepare.sns_op_height;
        }

        ldch->lut_manger_ = (LdcLutBufferManager*)aiq_mallocz(sizeof(*ldch->lut_manger_));
        LdcLutBufMng_init(ldch->lut_manger_, lut_config, ldch->config_->mem_ops);
        LdcLutBufMng_importHwBuffers(ldch->lut_manger_, 0, MEM_TYPE_LDCH);
    }

    aiqMutex_lock(&ldch->_mutex);
    if (ldch->cached_lut_[0].Fd != LDC_BUF_FD_DEFAULT) {
        LOGW_ALDC_SUBM(ALDC_LDCH_SUBM, "the cached lut of LDCH is in use!!");
        aiqMutex_unlock(&ldch->_mutex);
        return NULL;
    }
    aiqMutex_unlock(&ldch->_mutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret            = LdcLutBufMng_getFreeHwBuffer(ldch->lut_manger_, 0, &ldch->cached_lut_[0]);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGW_ALDC_SUBM(ALDC_LDCH_SUBM, "LDCH hasn't buffer available, maybe only one buffer ?!");
        return NULL;
    }

    if (ldch->cached_lut_[0].State != kLdcLutBufWait2Chip) {
        LOGW_ALDC_SUBM(ALDC_LDCH_SUBM, "Buffer in use, will not update lut!");
        return NULL;
    }

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "LDCH get lut buf fd %d", ldch->cached_lut_[0].Fd);

    return &ldch->cached_lut_[0];
}

void LdchAdaptee_init(LdchAdaptee* ldch) {
    aiqMutex_init(&ldch->_mutex);

    ldch->enable_         = false;
    ldch->started_        = false;
    ldch->valid_          = false;
    ldch->isReCal_        = false;
    ldch->correct_level_  = 0;
    ldch->gen_mesh_state_ = kLdcGenMeshIdle;
    ldch->calib_          = NULL;
    ldch->config_         = NULL;

    ldch->lut_manger_ = NULL;
    for (int i = 0; i < 2; i++) {
        ldch->cached_lut_[i].Addr     = NULL;
        ldch->cached_lut_[i].Fd       = LDC_BUF_FD_DEFAULT;
        ldch->chip_in_use_lut[i].addr = NULL;
        ldch->chip_in_use_lut[i].fd   = LDC_BUF_FD_DEFAULT;
    }
};

void LdchAdaptee_deinit(LdchAdaptee* ldch) {
    aiqMutex_deInit(&ldch->_mutex);

    if (ldch->lut_manger_) {
        LdcLutBufMng_deinit(ldch->lut_manger_);
        aiq_free(ldch->lut_manger_);
        ldch->lut_manger_ = NULL;
    }
};

XCamReturn LdchAdaptee_updateMesh(LdchAdaptee* ldch, uint8_t level, LdcMeshBufInfo_t cfg) {
    ENTER_ALDC_FUNCTION();

    if (ldch->cached_lut_[0].Fd == LDC_BUF_FD_DEFAULT) XCAM_RETURN_BYPASS;

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "LDCH update mesh fd: correct level %d", level);

    aiqMutex_lock(&ldch->_mutex);
    ldch->chip_in_use_lut[0].fd   = ldch->cached_lut_[0].Fd;
    ldch->chip_in_use_lut[0].addr = ldch->cached_lut_[0].Addr;
    ldch->correct_level_          = level;
    ldch->enable_                 = ldch->calib_->en;
    ldch->gen_mesh_state_         = kLdcGenMeshFinish;

    ldch->cached_lut_[0].Fd = LDC_BUF_FD_DEFAULT;
    aiqMutex_unlock(&ldch->_mutex);

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "%s LDCH and update chip_in_use_lut_fd_ %d",
                   ldch->enable_ ? "on" : "off", ldch->chip_in_use_lut[0].fd);

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdchAdaptee_config(LdchAdaptee* ldch, const ldc_api_attrib_t* calib) {
    ENTER_ALDC_FUNCTION();

    ldch->calib_ = calib;
    ldch->valid_ = true;

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdchAdaptee_prepare(LdchAdaptee* ldch, const RkAiqAlgoConfigLdc* config) {
    ENTER_ALDC_FUNCTION();

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "%s : LDCH en %d valid: %d", __func__, ldch->enable_,
                   ldch->valid_);

    ldch->isReCal_ = true;
    ldch->config_  = config;

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void LdchAdaptee_onFrameEvent(LdchAdaptee* ldch, const RkAiqAlgoCom* inparams,
                              RkAiqAlgoResCom* outparams) {
    ENTER_ALDC_FUNCTION();

    aiqMutex_lock(&ldch->_mutex);
    if (ldch->gen_mesh_state_ == kLdcGenMeshFinish) {
        ldch->gen_mesh_state_ = kLdcGenMeshIdle;
        ldch->isReCal_        = true;
    }

    if (inparams->u.proc.is_attrib_update) {
        ldch->isReCal_ = true;
        /*
         * 1. off -> on:  enable LDCH only after the mesh table is generated asynchronously.
         * 2. on -> off:  diable the LDCH immediately.
         */
        if (!ldch->calib_->en) ldch->enable_ = ldch->calib_->en;
    }

    if (!ldch->isReCal_) {
        outparams->cfg_update = false;
        LOGV_ALDC_SUBM(ALDC_LDCH_SUBM, "Skip update");
        aiqMutex_unlock(&ldch->_mutex);
        return;
    }

    ldch->isReCal_ = false;

    ldc_param_t* ldcRes            = (ldc_param_t*)(outparams->algoRes);
    ldc_ldch_params_cfg_t* userCfg = &ldcRes->sta.ldchCfg;
    if (ldch->enable_) {
        if (ldch->chip_in_use_lut[0].fd >= 0) {
            userCfg->lutMapCfg.sw_ldcT_lutMapBuf_fd[0] = ldch->chip_in_use_lut[0].fd;
            ldch->chip_in_use_lut[0].fd                = LDC_BUF_FD_DEFAULT;
            userCfg->en = ldch->enable_ = true;
            outparams->cfg_update       = true;
        } else {
            outparams->cfg_update = false;
        }
    }
    userCfg->en = ldch->enable_;
    aiqMutex_unlock(&ldch->_mutex);

    LOGD_ALDC_SUBM(ALDC_LDCH_SUBM, "LDCH result: cfg upd %d, en %d, mesh level %d, buf_fd %d",
                   outparams->cfg_update, userCfg->en, ldch->correct_level_,
                   userCfg->lutMapCfg.sw_ldcT_lutMapBuf_fd[0]);

    EXIT_ALDC_FUNCTION();
}
