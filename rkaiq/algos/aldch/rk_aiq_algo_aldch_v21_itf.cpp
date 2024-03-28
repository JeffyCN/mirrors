/*
 * rk_aiq_algo_aldch_itf.c
 *
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

#include "aldch/rk_aiq_algo_aldch_itf.h"
#include "aldch/rk_aiq_types_aldch_algo_prvt.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"
#include "rk_aiq_ldch_generate_mesh.h"
#include "aldch/rk_aiq_uapi_aldch_v21_int.h"


RKAIQ_BEGIN_DECLARE

static const uint8_t default_bic_table[9][4] = {
    {0x00, 0x80, 0x00, 0x00}, // table0: 0, 0, 128, 0
    {0xfc, 0x7f, 0x05, 0x00}, // table1: 0, 5, 127, -4
    {0xfa, 0x7b, 0x0c, 0xff}, // table2: -1, 12, 123, -6
    {0xf8, 0x76, 0x14, 0xfe}, // table3: -2, 20, 118, -8
    {0xf7, 0x6f, 0x1d, 0xfd}, // table4: -3, 29, 111, -9
    {0xf7, 0x66, 0x27, 0xfc}, // table4: -4, 39, 102, -9
    {0xf7, 0x5d, 0x32, 0xfa}, // table4: -6, 50, 93, -9
    {0xf7, 0x53, 0x3d, 0xf9}, // table4: -7, 61, 83, -9
    {0xf8, 0x48, 0x48, 0xf8}, // table4: -8, 72, 72, -8
};

static XCamReturn
updateCalibConfig(RkAiqAlgoCom* params)
{
    LDCHHandle_t hLDCH = (LDCHHandle_t)params->ctx->hLDCH;
    LDCHContext_t* ldchCtx = (LDCHContext_t*)hLDCH;
    RkAiqAlgoConfigAldch* rkaiqAldchConfig = (RkAiqAlgoConfigAldch*)params;

    CalibDbV2_LDCH_t* calib_ldch_db =
        (CalibDbV2_LDCH_t*)(CALIBDBV2_GET_MODULE_PTR(rkaiqAldchConfig->com.u.prepare.calibv2, aldch));
    CalibDbV2_Ldch_Param_t* calib_ldch = &calib_ldch_db->param;

    ldchCtx->camCoeff.cx = calib_ldch->light_center[0];
    ldchCtx->camCoeff.cy = calib_ldch->light_center[1];
    ldchCtx->camCoeff.a0 = calib_ldch->coefficient[0];
    ldchCtx->camCoeff.a2 = calib_ldch->coefficient[1];
    ldchCtx->camCoeff.a3 = calib_ldch->coefficient[2];
    ldchCtx->camCoeff.a4 = calib_ldch->coefficient[3];
    memcpy(ldchCtx->meshfile, calib_ldch->meshfile, sizeof(ldchCtx->meshfile));

    if (calib_ldch->ldch_en) {
        if ((!ldchCtx->ldch_en || calib_ldch->correct_level != hLDCH->correct_level) && \
            ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_ON_LINE) {
            if (aiqGenLdchMeshInit(hLDCH) < 0) {
                LOGE_ALDCH("Failed to init gen mesh");
                return XCAM_RETURN_ERROR_FAILED;
            }

            for (uint8_t i = 0; i < ldchCtx->multi_isp_number; i++) {
                if (get_ldch_buf(hLDCH, i) != XCAM_RETURN_NO_ERROR) {
                    LOGE_ALDCH("Failed to get ldch buf\n");
                    return XCAM_RETURN_ERROR_FAILED;
                }

                bool success = aiqGenMesh(hLDCH, calib_ldch->correct_level, i);
                if (!success) {
                    LOGW_ALDCH("lut is not exist");
                    put_ldch_buf(hLDCH, i);
                    return XCAM_RETURN_ERROR_FAILED;
                }

                if (hLDCH->ldch_mem_info[i])
                    hLDCH->ready_lut_mem_fd[i] = hLDCH->ldch_mem_info[i]->fd;
            }

            ldchCtx->isLutUpdated.store(true, std::memory_order_release);
        }
    }

    ldchCtx->user_config.en =  calib_ldch->ldch_en;
    ldchCtx->user_config.correct_level = calib_ldch->correct_level;


    LOGI_ALDCH("update calib en(%d), level(%d-%d), coeff(%f, %f, %f, %f, %f, %f)",
            calib_ldch->ldch_en,
            calib_ldch->correct_level,
            calib_ldch->correct_level_max,
            calib_ldch->light_center[0],
            calib_ldch->light_center[1],
            calib_ldch->coefficient[0],
            calib_ldch->coefficient[1],
            calib_ldch->coefficient[2],
            calib_ldch->coefficient[3]);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn update_custom_lut_from_file(LDCHContext_t* ldchCtx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (get_ldch_buf(ldchCtx, 0) != XCAM_RETURN_NO_ERROR) {
        LOGE_ALDCH("Failed to get ldch buf\n");
        ret = XCAM_RETURN_ERROR_MEM;
    } else {
        rk_aiq_ldch_lut_external_file_t *lut = &ldchCtx->user_config.lut.u.file;

        char filename[200] = {0};
        sprintf(filename, "%s/%s",
                lut->config_file_dir,
                lut->mesh_file_name);

        LOGD_ALDCH("read lut file name: %s/%s\n",
                lut->config_file_dir,
                lut->mesh_file_name);

        bool ret1 = read_mesh_from_file(ldchCtx, filename);
        if (!ret1) {
            LOGE_ALDCH("Failed to read mesh, disable ldch!");
            ldchCtx->ldch_en = ldchCtx->user_config.en = false;
            put_ldch_buf(ldchCtx, 0);
            ret = XCAM_RETURN_ERROR_FILE;
        } else {
            uint16_t *addr = (uint16_t *)ldchCtx->ldch_mem_info[0]->addr;
            LOGD_ALDCH("lut[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                    addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7],
                    addr[8], addr[9], addr[10], addr[11],
                    addr[12], addr[13], addr[14], addr[15]);
        }

        if (ldchCtx->ldch_mem_info[0]) {
            ldchCtx->ready_lut_mem_fd[0] = ldchCtx->ldch_mem_info[0]->fd;
            ldchCtx->update_lut_mem_fd[0] = ldchCtx->ready_lut_mem_fd[0];
        }

        LOGD_ALDCH("update custom lut from external file, lut_mem_fd %d\n", ldchCtx->update_lut_mem_fd[0]);
    }

    return ret;
}

static XCamReturn update_custom_lut_from_external_buffer(LDCHContext_t* ldchCtx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!ldchCtx->_lutCache->GetBuffer()) {
        LOGE_ALDCH("Failed to get ldch lut cache\n");
        ret = XCAM_RETURN_ERROR_MEM;
    }

    if (get_ldch_buf(ldchCtx, 0) != XCAM_RETURN_NO_ERROR) {
        LOGE_ALDCH("Failed to get ldch buf\n");
        ret = XCAM_RETURN_ERROR_MEM;
    } else {
        uint16_t hpic, vpic, hsize, vsize, hstep, vstep;
        uint32_t lut_size = 0;
        uint16_t *addr = (uint16_t *)ldchCtx->_lutCache->GetBuffer();

        hpic    = *addr++;
        vpic    = *addr++;
        hsize   = *addr++;
        vsize   = *addr++;
        hstep   = *addr++;
        vstep   = *addr++;

        lut_size = hsize * vsize *  sizeof(uint16_t);
        LOGD_ALDCH("lut info: [%d-%d-%d-%d-%d-%d]", hpic, vpic, hsize, vsize, hstep, vstep);

        if (ldchCtx->src_width != hpic || ldchCtx->src_height != vpic || \
            lut_size > (uint32_t)ldchCtx->ldch_mem_info[0]->size) {
            LOGE_ALDCH("mismatched lut pic resolution: src %dx%d, lut %dx%d, disable ldch",
                    ldchCtx->src_width, ldchCtx->src_height, hpic, vpic);
            LOGE_ALDCH("Invalid lut buffer size %zu, ldch drv bufer size is %u, disable ldch",
                       lut_size, ldchCtx->ldch_mem_info[0]->size);
            ldchCtx->ldch_en = ldchCtx->user_config.en = false;
            put_ldch_buf(ldchCtx, 0);
            ret = XCAM_RETURN_ERROR_PARAM;
        } else {
            ldchCtx->lut_h_size = hsize;
            ldchCtx->lut_v_size = vsize;
            ldchCtx->lut_mapxy_size = lut_size;
            ldchCtx->lut_h_size = hsize / 2; //word unit

            memcpy(ldchCtx->ldch_mem_info[0]->addr, addr, ldchCtx->lut_mapxy_size);
            LOGD_ALDCH("lut[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                    addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7],
                    addr[8], addr[9], addr[10], addr[11],
                    addr[12], addr[13], addr[14], addr[15]);
            ldchCtx->_lutCache.release();
            ldchCtx->_lutCache = nullptr;
        }

        if (ldchCtx->ldch_mem_info[0]) {
            ldchCtx->ready_lut_mem_fd[0] = ldchCtx->ldch_mem_info[0]->fd;
            ldchCtx->update_lut_mem_fd[0] = ldchCtx->ready_lut_mem_fd[0];
        }

        LOGD_ALDCH("update custom lut from external buffer, lut_mem_fd %d\n", ldchCtx->update_lut_mem_fd[0]);
    }

    return ret;
}

static XCamReturn update_uapi_attribute(LDCHContext_t* ldchCtx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_FILE) {
        if (ldchCtx->user_config.en) {
            if (ldchCtx->user_config.lut.update_flag) {
                if (update_custom_lut_from_file(ldchCtx) < 0) {
                    LOGE_ALDCH("Failed update custom lut\n");
                    return XCAM_RETURN_ERROR_FAILED;
                } else {
                    ldchCtx->user_config.lut.update_flag = false;
                }
            }
        } else {
            ldchCtx->ldch_en = false;
            LOGD_ALDCH("disable ldch by user api\n");
        }

        ldchCtx->isAttribUpdated = false;
    } else if (ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_ON_LINE) {
        if (ldchCtx->user_config.en) {
            SmartPtr<rk_aiq_ldch_v21_attrib_t> attrPtr = new rk_aiq_ldch_v21_attrib_t;
            if (attrPtr.ptr()) {
                memcpy(attrPtr.ptr(), &ldchCtx->user_config, sizeof(ldchCtx->user_config));

                ldchCtx->aldchReadMeshThread->push_attr(attrPtr);
                ldchCtx->isAttribUpdated = false;
            } else {
                LOGE_ALDCH("Failed to new ldch attr, don't update attrib\n");
            }
        } else {
            ldchCtx->ldch_en = false;
            ldchCtx->isAttribUpdated = false;
        }
    } else if (ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_BUFFER) {
        if (ldchCtx->user_config.en) {
            if (ldchCtx->user_config.lut.update_flag) {
                if (update_custom_lut_from_external_buffer(ldchCtx) < 0) {
                    LOGE_ALDCH("Failed update custom lut from external buffer\n");
                } else {
                    ldchCtx->user_config.lut.update_flag = false;
                }
            }
        } else {
            ldchCtx->ldch_en = false;
            LOGD_ALDCH("disable ldch by user api\n");
        }

        ldchCtx->isAttribUpdated = false;
    } else {
        LOGE_ALDCH("unknow updating lut mode %d\n", ldchCtx->user_config.update_lut_mode);
        ldchCtx->isAttribUpdated = false;
        ret = XCAM_RETURN_ERROR_PARAM;
    }

    // update user params after lut is generated by RKAiqAldchThread in online mode
    if (ret ==  XCAM_RETURN_NO_ERROR && \
        ldchCtx->user_config.update_lut_mode != RK_AIQ_LDCH_UPDATE_LUT_ON_LINE) {
        ldchCtx->ldch_en          = ldchCtx->user_config.en;
        ldchCtx->correct_level    = ldchCtx->user_config.correct_level;
        ldchCtx->zero_interp_en   = ldchCtx->user_config.zero_interp_en;
        ldchCtx->sample_avr_en    = ldchCtx->user_config.sample_avr_en;
        ldchCtx->bic_mode_en      = ldchCtx->user_config.bic_mode_en;
        memcpy(ldchCtx->bicubic, ldchCtx->user_config.bic_weight, sizeof(ldchCtx->bicubic));
    }

    return ret;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ALDCH("%s: (enter)\n", __FUNCTION__ );
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ALDCH( "%s: create aldch context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->hLDCH = new LDCHContext_t;
    if (ctx->hLDCH == NULL) {
        LOGE_ALDCH( "%s: create aldch handle fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    /* setup config */
    memset((void *)(ctx->hLDCH), 0, sizeof(LDCHContext_t) );

    /* return handle */
    *context = ctx;

    ctx->hLDCH->isAttribUpdated = false;
    ctx->hLDCH->aldchReadMeshThread = new RKAiqAldchThread(ctx->hLDCH);
    ctx->hLDCH->isLutUpdated.store(false, std::memory_order_release);

    LDCHHandle_t ldchCtx = ctx->hLDCH;
    CalibDbV2_LDCH_t* calib_ldch_db =
            (CalibDbV2_LDCH_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, aldch));
    CalibDbV2_Ldch_Param_t* calib_ldch = &calib_ldch_db->param;

    ldchCtx->ldch_en = calib_ldch->ldch_en;
    memcpy(ldchCtx->meshfile, calib_ldch->meshfile, sizeof(ldchCtx->meshfile));

    ldchCtx->camCoeff.cx = calib_ldch->light_center[0];
    ldchCtx->camCoeff.cy = calib_ldch->light_center[1];
    ldchCtx->camCoeff.a0 = calib_ldch->coefficient[0];
    ldchCtx->camCoeff.a2 = calib_ldch->coefficient[1];
    ldchCtx->camCoeff.a3 = calib_ldch->coefficient[2];
    ldchCtx->camCoeff.a4 = calib_ldch->coefficient[3];
    LOGD_ALDCH("(%s) len light center(%.16f, %.16f)\n",
            __FUNCTION__,
            ldchCtx->camCoeff.cx, ldchCtx->camCoeff.cy);
    LOGD_ALDCH("(%s) len coefficient(%.16f, %.16f, %.16f, %.16f)\n",
            __FUNCTION__,
            ldchCtx->camCoeff.a0, ldchCtx->camCoeff.a2,
            ldchCtx->camCoeff.a3, ldchCtx->camCoeff.a4);

    ldchCtx->correct_level = calib_ldch->correct_level;
    ldchCtx->correct_level_max = calib_ldch->correct_level_max;

    memset(&ldchCtx->user_config, 0, sizeof(ldchCtx->user_config));
    ldchCtx->user_config.en             = ldchCtx->ldch_en;
    ldchCtx->user_config.correct_level  = ldchCtx->correct_level;
    ldchCtx->user_config.bic_mode_en = 1;
    memcpy(ldchCtx->user_config.bic_weight, default_bic_table, sizeof(default_bic_table));

    ldchCtx->frm_end_dis    = 0;
    ldchCtx->zero_interp_en = 0;
    ldchCtx->sample_avr_en  = 0;
    ldchCtx->bic_mode_en    = 1;
    ldchCtx->force_map_en   = 0;
    ldchCtx->map13p3_en     = 0;
    memcpy(ldchCtx->bicubic, default_bic_table, sizeof(default_bic_table));

    for (int i = 0; i < 2; i++) {
        ldchCtx->update_lut_mem_fd[i] = -1;
        ldchCtx->ready_lut_mem_fd[i] = -1;
    }

    ldchCtx->hasAllocShareMem.store(false, std::memory_order_release);
    ldchCtx->_lutCache = nullptr;
    ldchCtx->is_multi_isp = false;
    ldchCtx->multi_isp_extended_pixel = 0;
    ldchCtx->multi_isp_number = 1;

    LOGD_ALDCH("bic table0: 0x%x, table1: 0x%x, table2: 0x%x, table3: 0x%x,",
               *(uint32_t *)ldchCtx->bicubic,
               *(uint32_t *)(ldchCtx->bicubic + 4),
               *(uint32_t *)(ldchCtx->bicubic + 8),
               *(uint32_t *)(ldchCtx->bicubic + 12));
    LOGD_ALDCH("bic table4: 0x%x, table5: 0x%x, table6: 0x%x, table7: 0x%x,",
               *(uint32_t *)(ldchCtx->bicubic + 16),
               *(uint32_t *)(ldchCtx->bicubic + 20),
               *(uint32_t *)(ldchCtx->bicubic + 24),
               *(uint32_t *)(ldchCtx->bicubic + 28));
    LOGD_ALDCH("bic table8: 0x%x, bic_mode_en: %d", *(uint32_t *)(ldchCtx->bicubic + 32), ldchCtx->bic_mode_en);

    LOGD_ALDCH("ldch en %d, meshfile: %s, correct_level-max: %d-%d from xml file",
               calib_ldch->ldch_en,
               ldchCtx->meshfile,
               ldchCtx->correct_level,
               ldchCtx->correct_level_max);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LDCHHandle_t hLDCH = (LDCHHandle_t)context->hLDCH;
    LDCHContext_t* ldchCtx = (LDCHContext_t*)hLDCH;

    if (ldchCtx->aldchReadMeshThread->is_running()) {
        ldchCtx->aldchReadMeshThread->triger_stop();
        ldchCtx->aldchReadMeshThread->stop();
    }

    if (!hLDCH->aldchReadMeshThread->is_empty()) {
        hLDCH->aldchReadMeshThread->clear_attr();
    }

    if (ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_ON_LINE) {
        genLdchMeshDeInit(ldchCtx->ldchParams);
    }

    release_ldch_buf(ldchCtx, 0);
    if (ldchCtx->is_multi_isp)
        release_ldch_buf(ldchCtx, 1);

    delete context->hLDCH;
    context->hLDCH = NULL;
    delete context;
    context = NULL;
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LDCHHandle_t hLDCH = (LDCHHandle_t)params->ctx->hLDCH;
    LDCHContext_t* ldchCtx = (LDCHContext_t*)hLDCH;
    RkAiqAlgoConfigAldch* rkaiqAldchConfig = (RkAiqAlgoConfigAldch*)params;

    ldchCtx->src_width = params->u.prepare.sns_op_width;
    ldchCtx->src_height = params->u.prepare.sns_op_height;
    ldchCtx->dst_width = params->u.prepare.sns_op_width;
    ldchCtx->dst_height = params->u.prepare.sns_op_height;
    ldchCtx->resource_path = rkaiqAldchConfig->resource_path;
    ldchCtx->share_mem_ops = rkaiqAldchConfig->mem_ops_ptr;
    ldchCtx->is_multi_isp = rkaiqAldchConfig->is_multi_isp;
    ldchCtx->multi_isp_extended_pixel = rkaiqAldchConfig->multi_isp_extended_pixel;
    if (ldchCtx->is_multi_isp)
        ldchCtx->multi_isp_number = 2;
    else
        ldchCtx->multi_isp_number = 1;


    LOGD_ALDCH("update_lut_mode %d\n", ldchCtx->user_config.update_lut_mode);
    LOGD_ALDCH("is_multi_isp %d, multi_isp_extended_pixel %d\n", ldchCtx->is_multi_isp,
               ldchCtx->multi_isp_extended_pixel);

    // 1.update cailb
    bool config_calib = !!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);
    if (config_calib) {
        if(updateCalibConfig(params) != XCAM_RETURN_NO_ERROR) {
            LOGW_ALDCH("Failed to update calib config");
        }

        return XCAM_RETURN_NO_ERROR;
    }

    // 2.process the new attrib set before prepare
    if (ldchCtx->aldchReadMeshThread->is_running()) {
        hLDCH->aldchReadMeshThread->triger_stop();
        hLDCH->aldchReadMeshThread->stop();
    }

    if (!hLDCH->aldchReadMeshThread->is_empty()) {
        hLDCH->aldchReadMeshThread->clear_attr();
    }

    // discard the lut generated ReadMeshThread in before aiq prepare
    if (ldchCtx->isLutUpdated.load(std::memory_order_acquire)) {
        put_ldch_buf(hLDCH, 0);
        ldchCtx->isLutUpdated.store(false, std::memory_order_release);
    }

    // 3.update uapi attribute
    if (ldchCtx->isAttribUpdated) {
        ldchCtx->ldch_en            = ldchCtx->user_config.en;
        ldchCtx->correct_level      = ldchCtx->user_config.correct_level;
        ldchCtx->zero_interp_en     = ldchCtx->user_config.zero_interp_en;
        ldchCtx->sample_avr_en      = ldchCtx->user_config.sample_avr_en;
        ldchCtx->bic_mode_en        = ldchCtx->user_config.bic_mode_en;
        memcpy(ldchCtx->bicubic, ldchCtx->user_config.bic_weight, sizeof(ldchCtx->bicubic));

        ldchCtx->isAttribUpdated = false;
    }

    // 4.update ldch result
    if (ldchCtx->ldch_en) {
        if (ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_FILE) {
            if (ldchCtx->user_config.lut.update_flag) {
                if (update_custom_lut_from_file(ldchCtx) < 0) {
                    LOGE_ALDCH("Failed update custom lut from file\n");
                    ldchCtx->ldch_en = false;
                } else {
                    ldchCtx->user_config.lut.update_flag = false;
                }
            }
        } else if (ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_ON_LINE) {
            if (aiqGenLdchMeshInit(ldchCtx) >= 0) {
                for (uint8_t i = 0; i < ldchCtx->multi_isp_number; i++) {
                    if (get_ldch_buf(ldchCtx, i) != XCAM_RETURN_NO_ERROR) {
                        LOGE_ALDCH("Failed to get mesh buf, disable LDCH\n");
                        ldchCtx->ldch_en = ldchCtx->user_config.en = false;
                    }

                    bool success = aiqGenMesh(ldchCtx, ldchCtx->correct_level, i);
                    if (!success) {
                        LOGE_ALDCH("Failed to gen mesh, disable LDCH\n");
                        put_ldch_buf(ldchCtx, i);
                        ldchCtx->ldch_en = ldchCtx->user_config.en = false;
                    }

                    if (ldchCtx->ldch_mem_info[i])
                        ldchCtx->ready_lut_mem_fd[i] = ldchCtx->ldch_mem_info[i]->fd;
                }
            }
        } else if (ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_BUFFER) {
            if (ldchCtx->user_config.lut.update_flag) {
                if (update_custom_lut_from_external_buffer(ldchCtx) < 0) {
                    LOGE_ALDCH("Failed update custom lut from external buffer\n");
                    ldchCtx->ldch_en = false;
                } else {
                    ldchCtx->user_config.lut.update_flag = false;
                }
            }
        } else {
            LOGE_ALDCH("unknow updating lut mode %d\n", ldchCtx->user_config.update_lut_mode);
        }
    }

    if (ldchCtx->user_config.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_ON_LINE) {
        hLDCH->aldchReadMeshThread->triger_start();
        hLDCH->aldchReadMeshThread->start();
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LDCHHandle_t hLDCH = (LDCHHandle_t)inparams->ctx->hLDCH;
    LDCHContext_t* ldchCtx = (LDCHContext_t*)hLDCH;
    RkAiqAlgoProcResAldchV21* ldchPreOut = (RkAiqAlgoProcResAldchV21*)outparams;
    bool update_params = false;

    // 1.initial state processing or updating uapi attrib after lut is generated by RKAiqAldchThread
    if (inparams->u.proc.init) {
        for (uint8_t i = 0; i < ldchCtx->multi_isp_number; i++) {
            if (ldchCtx->ready_lut_mem_fd[i] >= 0)
                ldchCtx->update_lut_mem_fd[i] = ldchCtx->ready_lut_mem_fd[i];
            LOGD_ALDCH("isp_id: %d, update update_lut_mem_fd %d\n",
                        i, ldchCtx->update_lut_mem_fd[i]);
        }

        update_params = true;
    } else if (ldchCtx->isLutUpdated.load(std::memory_order_acquire)) {
        // update user params after lut is generated by RKAiqAldchThread
        ldchCtx->ldch_en          = ldchCtx->user_config.en;
        ldchCtx->correct_level    = ldchCtx->user_config.correct_level;
        ldchCtx->zero_interp_en   = ldchCtx->user_config.zero_interp_en;
        ldchCtx->sample_avr_en    = ldchCtx->user_config.sample_avr_en;
        ldchCtx->bic_mode_en      = ldchCtx->user_config.bic_mode_en;
        memcpy(ldchCtx->bicubic, ldchCtx->user_config.bic_weight, sizeof(ldchCtx->bicubic));

        for (uint8_t i = 0; i < ldchCtx->multi_isp_number; i++) {
            if (ldchCtx->ready_lut_mem_fd[i] >= 0)
                ldchCtx->update_lut_mem_fd[i] = ldchCtx->ready_lut_mem_fd[i];
            LOGD_ALDCH("isp_id: %d, update update_lut_mem_fd %d\n", i, ldchCtx->update_lut_mem_fd[i]);
        }

        update_params = true;
        ldchCtx->isLutUpdated.store(false, std::memory_order_release);
    }

    // 2.update uapi attribute
    if (ldchCtx->isAttribUpdated) {
        if (update_uapi_attribute(ldchCtx) < 0) {
            LOGE_ALDCH("Failed to update uapi attribute %d", ldchCtx->update_lut_mem_fd[0]);
        }

        // update params asynchronously after lut is generated by RKAiqAldchThread
        // in online mode
        auto update_lut_mode = ldchCtx->user_config.update_lut_mode;
        if (update_lut_mode != RK_AIQ_LDCH_UPDATE_LUT_ON_LINE ||
            (update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_ON_LINE && !ldchCtx->ldch_en)) {
            update_params = true;
        }
    }

    // 3.update ldch result
    if (ldchCtx->ldch_en) {
        for (uint8_t i = 0; i < ldchCtx->multi_isp_number; i++) {
            if (ldchCtx->update_lut_mem_fd[i] < 0) {
                LOGE_ALDCH("isp_id: %d, invalid mesh buf!", i);
                if (inparams->u.proc.init) {
                    LOGE_ALDCH("mesh buf is invalid, disable LDCH!");
                    ldchCtx->ldch_en = ldchCtx->user_config.en = false;
                    ldchPreOut->ldch_result->base.sw_ldch_en = false;
                    outparams->cfg_update = true;
                } else {
                    outparams->cfg_update = false;
                }
                return XCAM_RETURN_NO_ERROR;
            }

            ldchPreOut->ldch_result->base.lut_mapxy_buf_fd[i] = ldchCtx->update_lut_mem_fd[i];
        }

        ldchPreOut->ldch_result->base.lut_h_size = ldchCtx->lut_h_size;
        ldchPreOut->ldch_result->base.lut_v_size = ldchCtx->lut_v_size;
        ldchPreOut->ldch_result->base.lut_map_size = ldchCtx->lut_mapxy_size;

        ldchPreOut->ldch_result->frm_end_dis = ldchCtx->frm_end_dis;
        ldchPreOut->ldch_result->zero_interp_en = ldchCtx->zero_interp_en;
        ldchPreOut->ldch_result->sample_avr_en = ldchCtx->sample_avr_en;
        ldchPreOut->ldch_result->bic_mode_en = ldchCtx->bic_mode_en;
        ldchPreOut->ldch_result->force_map_en = ldchCtx->force_map_en;
        ldchPreOut->ldch_result->map13p3_en = ldchCtx->map13p3_en;
        memcpy(ldchPreOut->ldch_result->bicubic, ldchCtx->bicubic, sizeof(ldchCtx->bicubic));
    }

    ldchPreOut->ldch_result->base.sw_ldch_en = ldchCtx->ldch_en;
    outparams->cfg_update = update_params;

    if (update_params) {
        LOGD_ALDCH("en:%d, level:%d, h/v: %dx%d, interp_en:%d, avr_en:%d, bic_en:%d, "
                   "force_map_en:%d, map13p3_en:%d\n",
                   ldchCtx->ldch_en, ldchCtx->correct_level,
                   ldchCtx->lut_h_size, ldchCtx->lut_v_size,
                   ldchCtx->zero_interp_en, ldchCtx->sample_avr_en,
                   ldchCtx->bic_mode_en, ldchCtx->force_map_en,
                   ldchCtx->map13p3_en);
        LOGD_ALDCH("cfg_update %d, update_lut_mode %d, lut fd %d\n",
                   outparams->cfg_update,
                   ldchCtx->user_config.update_lut_mode,
                   ldchPreOut->ldch_result->base.lut_mapxy_buf_fd[0]);

        if (ldchCtx->is_multi_isp) {
            LOGD_ALDCH("multi isp: ldch mesh fd of right isp: %d",
                    ldchPreOut->ldch_result->base.lut_mapxy_buf_fd[1]);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAldch = {
    .common = {
        .version = RKISP_ALGO_ALDCH_VERSION,
        .vendor  = RKISP_ALGO_ALDCH_VENDOR,
        .description = RKISP_ALGO_ALDCH_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ALDCH,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE


bool RKAiqAldchThread::loop()
{
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;
    SmartPtr<rk_aiq_ldch_v21_cfg_t> attrib = mAttrQueue.pop (timeout);

    if (!attrib.ptr()) {
        LOGW_ALDCH("RKAiqAldchThread got empty attrib, stop thread");
        return false;
    }

    if (!hLDCH->ldch_en || attrib->correct_level != hLDCH->correct_level) {
        if (aiqGenLdchMeshInit(hLDCH) < 0) {
            LOGE_ALDCH("Failed to init gen mesh");
            return true;
        }

        for (uint8_t i = 0; i < hLDCH->multi_isp_number; i++) {
            if (get_ldch_buf(hLDCH, i) != XCAM_RETURN_NO_ERROR) {
                LOGE_ALDCH("Failed to get ldch buf\n");
                return true;
            }

            bool success = aiqGenMesh(hLDCH, attrib->correct_level, i);
            if (!success) {
                LOGE_ALDCH("Failed to gen mesh\n");
                put_ldch_buf(hLDCH, i);
                return true;
            }

            if (hLDCH->ldch_mem_info[i])
                hLDCH->ready_lut_mem_fd[i] = hLDCH->ldch_mem_info[i]->fd;

            LOGD_ALDCH("ldch gen mesh: level %d, ready lut fd %d : %d\n",
                       attrib->correct_level, hLDCH->ready_lut_mem_fd[i]);
        }

        hLDCH->isLutUpdated.store(true, std::memory_order_release);
    }

    EXIT_ANALYZER_FUNCTION();

    return true;
}
