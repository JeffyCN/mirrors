/*
 *  Copyright (c) 2024 Rockchip Electronics Co., Ltd.
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

#include "algos/newStruct/ldc/include/ldc_algo_adaptor.h"

#include "algos/newStruct/ldc/ldc_types_prvt.h"
#include "common/rk_aiq_comm.h"
#include "common/rk_aiq_types_priv_c.h"
#include "xcore/base/xcam_log.h"

#define LEVEL_MAP(__level, __level_max) ((__level) * (__level_max) / 255);

void LdcAlgoAdaptor_genMeshDoneCb(void* ctx, LdcGenMeshCfg_t cfg) {
    ENTER_ALDC_FUNCTION();
    LdcAlgoAdaptor* ldc = (LdcAlgoAdaptor*)ctx;

    LOGD_ALDC("Generate the mesh of level %d successfully", cfg.level);

    LdchAdaptee_updateMesh(ldc->ldch_adaptee_, cfg.level, cfg.ldch);
#if RKAIQ_HAVE_LDCV
    LdcvAdaptee_updateMesh(ldc->ldcv_adaptee_, cfg.level, cfg.ldcv);
#endif

    EXIT_ALDC_FUNCTION();
}

void LdcAlgoAdaptor_genMeshFailedCb(void* ctx, LdcGenMeshCfg_t cfg) {
    ENTER_ALDC_FUNCTION();

    LOGE_ALDC("Failed to Generate the mesh of level %d", cfg.level);

    EXIT_ALDC_FUNCTION();
}

static XCamReturn LdcAlgoAdaptor_updateMesh(bool init, LdcAlgoAdaptor* adp) {
    ENTER_ALDC_FUNCTION();

    if (!adp->ldch_adaptee_ && !adp->ldcv_adaptee_) return XCAM_RETURN_ERROR_PARAM;

    if (!adp->valid_) return XCAM_RETURN_BYPASS;

    const ldc_api_attrib_t* attr = adp->calib;
    if (!attr) return XCAM_RETURN_BYPASS;

    uint8_t new_level = LEVEL_MAP(attr->tunning.autoGenMesh.sw_ldcT_correctStrg_val,
                                  attr->tunning.autoGenMesh.sw_ldcC_correctStrg_maxLimit);

    bool isLdchReGenMesh_ = false, isLdcvReGenMesh_ = false;

    // 1.check new attribute.
    uint8_t cur_level = LdchAdaptee_getCorrectLevel(adp->ldch_adaptee_);
    uint8_t en        = LdchAdaptee_isEnabled(adp->ldch_adaptee_);

    LOGD_ALDC("LDCH : cur en %d, level %d, new attr en %d, level %d, level maxLimit %d", en,
              cur_level, attr->en, new_level,
              attr->tunning.autoGenMesh.sw_ldcC_correctStrg_maxLimit);

    if (cur_level != new_level) {
        isLdchReGenMesh_ = true;
        LOGD_ALDC("LDCH change correct level: %d -> %d", cur_level, new_level);
    }
    if (attr->en != en) {
        isLdchReGenMesh_ = true;
        LOGD_ALDC("LDCH en: %d -> %d", en, attr->en);
    }

#if RKAIQ_HAVE_LDCV
    bool attr_ldcv_en = (attr->tunning.enMode == LDC_LDCH_LDCV_EN);

    cur_level = LdcvAdaptee_getCorrectLevel(adp->ldcv_adaptee_);
    en        = LdcvAdaptee_isEnabled(adp->ldcv_adaptee_);

    LOGD_ALDC("LDCV : cur en %d, level %d, new attr en %d, level %d", en, cur_level, attr_ldcv_en,
              new_level);

    if (attr_ldcv_en != en) {
        isLdcvReGenMesh_ = true;
        LOGD_ALDC("LDCV en: %d -> %d", en, attr->en);
    }
    if (attr_ldcv_en && cur_level != new_level) {
        isLdcvReGenMesh_ = true;
        LOGD_ALDC("LDCV change correct level: %d -> %d", cur_level, new_level);
    }
#endif

    // 2.triger generating new mesh.
    if (isLdchReGenMesh_ || isLdcvReGenMesh_) {
        LdcGenMeshCfg_t cfg;
        memset(&cfg, 0, sizeof(cfg));

        if (isLdchReGenMesh_) {
            const LdcLutBuffer* lut_buf = LdchAdaptee_getFreeLutBuf(adp->ldch_adaptee_, 0);
            if (lut_buf) {
                cfg.level     = new_level;
                cfg.ldch.addr = lut_buf->Addr;
                cfg.ldch.fd   = lut_buf->Fd;
            }
        }

#if RKAIQ_HAVE_LDCV
        if (isLdcvReGenMesh_) {
            const LdcLutBuffer* lut_buf = LdcvAdaptee_getFreeLutBuf(adp->ldcv_adaptee_, 0);
            if (lut_buf) {
                cfg.level     = new_level;
                cfg.ldcv.addr = lut_buf->Addr;
                cfg.ldcv.fd   = lut_buf->Fd;
            }
        }
#endif

        if (adp->gen_mesh_helper_ && (cfg.ldch.addr || cfg.ldcv.addr)) {
            if (!LdcAlgoGenMesh_IsEnabled(adp->gen_mesh_)) {
                LdcEnState state = kLdcEnLdchLdcv;
                if (isLdchReGenMesh_ && isLdcvReGenMesh_)
                    state = kLdcEnLdchLdcv;
                else if (isLdchReGenMesh_)
                    state = kLdcEnLdch;
                else if (isLdcvReGenMesh_)
                    state = kLdcEnLdcv;

                if (LdcAlgoGenMesh_init(adp->gen_mesh_, state, adp->width, adp->height, adp->width,
                                        adp->height, adp->calib) < 0) {
                    LOGE_ALDC("Failed to init ldc GenMesh");
                }

                if (!adp->gen_mesh_helper_->_base->_started)
                    LdcGenMeshHelperThd_start(adp->gen_mesh_helper_);
            }

            if (adp->gen_mesh_helper_->bQuit) {
                aiqThread_start(adp->gen_mesh_helper_->_base);
            }

            if (init) {
                XCamReturn ret = LdcAlgoGenMesh_genMeshNLevel(adp->gen_mesh_, 0, cfg.level,
                                                              (uint16_t*)(cfg.ldch.addr),
                                                              (uint16_t*)(cfg.ldcv.addr));
                if (ret < 0) {
                    LOGD_ALDC("Failed to GenMeshNLevel: %d", cfg.level);
                } else {
                    LOGD_ALDC("Successfully GenMeshNLevel: %d", cfg.level);
                    LdcAlgoAdaptor_genMeshDoneCb(adp, cfg);
                }
            } else {
                LOGD_ALDC("push reGenMesh %s%s mesh attr to LdcGenMeshHelperThd",
                          isLdchReGenMesh_ ? "LDCH" : "", isLdcvReGenMesh_ ? " & LDCV" : "");
                LdcGenMeshHelperThd_pushAttr(adp->gen_mesh_helper_, &cfg);
            }

            if (isLdchReGenMesh_)
                LdchAdaptee_setGenMeshState(adp->ldch_adaptee_, kLdcGenMeshWorking);
#if RKAIQ_HAVE_LDCV
            if (isLdcvReGenMesh_)
                LdcvAdaptee_setGenMeshState(adp->ldcv_adaptee_, kLdcGenMeshWorking);
#endif
        }

        isLdchReGenMesh_ = false;
        isLdcvReGenMesh_ = false;
    }

    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void LdcAlgoAdaptor_init(LdcAlgoAdaptor* adaptor) {
    adaptor->enable_  = false;
    adaptor->ldch_en_ = false;
    adaptor->ldcv_en_ = false;
    adaptor->started_ = false;
    adaptor->valid_   = false;

    adaptor->gen_mesh_        = NULL;
    adaptor->gen_mesh_helper_ = NULL;
    adaptor->ldch_adaptee_    = NULL;
    adaptor->ldcv_adaptee_    = NULL;
    adaptor->calib            = NULL;
}

void LdcAlgoAdaptor_deinit(LdcAlgoAdaptor* adaptor) {
    if (adaptor->gen_mesh_helper_) {
        LdcGenMeshHelperThd_stop(adaptor->gen_mesh_helper_);
        LdcGenMeshHelperThd_deinit(adaptor->gen_mesh_helper_);
        aiq_free(adaptor->gen_mesh_helper_);
        adaptor->gen_mesh_helper_ = NULL;
    }

    if (adaptor->gen_mesh_) {
        LdcAlgoGenMesh_deinit(adaptor->gen_mesh_);
        aiq_free(adaptor->gen_mesh_);
        adaptor->gen_mesh_ = NULL;
    }

    if (adaptor->ldch_adaptee_) {
        LdchAdaptee_deinit(adaptor->ldch_adaptee_);
        aiq_free(adaptor->ldch_adaptee_);
        adaptor->ldch_adaptee_ = NULL;
    }

#if RKAIQ_HAVE_LDCV
    if (adaptor->ldcv_adaptee_) {
        LdcvAdaptee_deinit(adaptor->ldcv_adaptee_);
        aiq_free(adaptor->ldcv_adaptee_);
        adaptor->ldcv_adaptee_ = NULL;
    }
#endif
}

XCamReturn LdcAlgoAdaptor_config(LdcAlgoAdaptor* adaptor, const ldc_api_attrib_t* calib) {
    ENTER_ALDC_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!adaptor->gen_mesh_)
        adaptor->gen_mesh_ = (LdcAlgoGenMesh*)aiq_mallocz(sizeof(*adaptor->gen_mesh_));

    if (!adaptor->gen_mesh_helper_) {
        adaptor->gen_mesh_helper_ =
            (LdcGenMeshHelperThd_t*)aiq_mallocz(sizeof(*adaptor->gen_mesh_helper_));
        LdcGenMeshHelperThd_init(adaptor->gen_mesh_helper_, adaptor->gen_mesh_);

        adaptor->gen_mesh_cb_.ctx_            = adaptor;
        adaptor->gen_mesh_cb_.gen_mesh_done   = LdcAlgoAdaptor_genMeshDoneCb;
        adaptor->gen_mesh_cb_.gen_mesh_failed = LdcAlgoAdaptor_genMeshFailedCb;
        LdcGenMeshHelperThd_setCb(adaptor->gen_mesh_helper_, &adaptor->gen_mesh_cb_);
    }

    if (!adaptor->ldch_adaptee_) {
        adaptor->ldch_adaptee_ = (LdchAdaptee*)aiq_mallocz(sizeof(*adaptor->ldch_adaptee_));
        LdchAdaptee_init(adaptor->ldch_adaptee_);
        LdchAdaptee_config(adaptor->ldch_adaptee_, calib);
        adaptor->ldch_en_ = LdchAdaptee_isEnabled(adaptor->ldch_adaptee_);
    }

#if RKAIQ_HAVE_LDCV
    if (!adaptor->ldcv_adaptee_) {
        adaptor->ldcv_adaptee_ = (LdcvAdaptee*)aiq_mallocz(sizeof(*adaptor->ldcv_adaptee_));
        LdcvAdaptee_init(adaptor->ldcv_adaptee_);
        LdcvAdaptee_config(adaptor->ldcv_adaptee_, calib);
        adaptor->ldcv_en_ = LdcvAdaptee_isEnabled(adaptor->ldcv_adaptee_);
    }
#endif

    adaptor->enable_ = adaptor->ldch_en_ | adaptor->ldcv_en_;

    adaptor->valid_ = true;

    EXIT_ALDC_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn LdcAlgoAdaptor_prepare(LdcAlgoAdaptor* adaptor, const RkAiqAlgoConfigLdc* config) {
    ENTER_ALDC_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!adaptor->valid_) return XCAM_RETURN_NO_ERROR;

    LdchAdaptee_prepare(adaptor->ldch_adaptee_, config);
#if RKAIQ_HAVE_LDCV
    LdcvAdaptee_prepare(adaptor->ldcv_adaptee_, config);
#endif

    CamCalibDbV2Context_t* calibv2 = config->com.u.prepare.calibv2;
    ldc_api_attrib_t* calib_ldc    = (ldc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, ldc));
    adaptor->calib                 = calib_ldc;

    adaptor->width  = config->com.u.prepare.sns_op_width;
    adaptor->height = config->com.u.prepare.sns_op_height;

    adaptor->ldch_en_ = LdchAdaptee_isEnabled(adaptor->ldch_adaptee_);
#if RKAIQ_HAVE_LDCV
    adaptor->ldcv_en_ = LdcvAdaptee_isEnabled(adaptor->ldcv_adaptee_);
#endif
    adaptor->enable_ = adaptor->ldch_en_ | adaptor->ldcv_en_;

    if (!adaptor->width && !adaptor->height) return XCAM_RETURN_BYPASS;

end:
    EXIT_ALDC_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void LdcAlgoAdaptor_onFrameEvent(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    ENTER_ALDC_FUNCTION();

    LdcContext_t* ldc_ctx = (LdcContext_t*)(inparams->ctx);
    LdcAlgoAdaptor* adp   = (LdcAlgoAdaptor*)(ldc_ctx->handler);

    outparams->cfg_update = true;

#if RKAIQ_HAVE_LDCV
    if (adp->calib->tunning.enMode == LDC_LDCH_LDCV_EN && !adp->calib->en) {
        outparams->cfg_update = false;
        return;
    }
#endif

    // 1. Check whether the mesh to be updated
    if (inparams->u.proc.is_attrib_update) {
        if (adp->calib->en) LdcAlgoAdaptor_updateMesh(inparams->u.proc.init, adp);
    }

    // 2. Update params for ldch
    LdchAdaptee_onFrameEvent(adp->ldch_adaptee_, inparams, outparams);
    adp->ldch_en_ = LdchAdaptee_isEnabled(adp->ldch_adaptee_);

#if RKAIQ_HAVE_LDCV
    // 3. Update params for ldcv
    LdcvAdaptee_onFrameEvent(adp->ldcv_adaptee_, inparams, outparams);
    adp->ldcv_en_ = LdcvAdaptee_isEnabled(adp->ldcv_adaptee_);
#endif

    adp->enable_ = adp->ldch_en_ | adp->ldcv_en_;

    if (adp->enable_) adp->started_ = true;

    if (outparams->cfg_update) {
        adp->correct_level_ = LdchAdaptee_getCorrectLevel(adp->ldch_adaptee_);
        LOGD_ALDC("LDC result: cfg upd %d, ldch en %d, ldcv en%d", outparams->cfg_update,
                  adp->ldch_en_, adp->ldcv_en_);
    }

    EXIT_ALDC_FUNCTION();
}

LdcLutBuffer* LdcAlgoAdaptor_getFreeLutBuf(LdcAlgoAdaptor* adp, LdcModType type, int8_t isp_id) {
    if (type == kLdch)
        return LdchAdaptee_getFreeLutBuf(adp->ldch_adaptee_, 0);
    else if (type == kLdcv)
#if RKAIQ_HAVE_LDCV
        return LdchAdaptee_getFreeLutBuf(adp->ldch_adaptee_, 0);
#else
        return NULL;
#endif
    else
        return NULL;
}
