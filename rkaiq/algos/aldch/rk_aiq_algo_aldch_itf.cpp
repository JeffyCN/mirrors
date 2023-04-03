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

RKAIQ_BEGIN_DECLARE


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

    if (calib_ldch->ldch_en) {
        if (!ldchCtx->ldch_en || calib_ldch->correct_level != hLDCH->correct_level) {
            aiqGenLdchMeshInit(hLDCH);
            bool success = aiqGenMesh(hLDCH, calib_ldch->correct_level);
            if (!success) {
                LOGE_ALDCH("lut is not exist");
                return XCAM_RETURN_ERROR_PARAM;
            }
        }
    }

    ldchCtx->ldch_en = calib_ldch->ldch_en;
    memcpy(ldchCtx->meshfile, calib_ldch->meshfile, sizeof(ldchCtx->meshfile));
    ldchCtx->correct_level = calib_ldch->correct_level;
    ldchCtx->correct_level_max = calib_ldch->correct_level_max;

    ldchCtx->isAttribUpdated = true;

    LOGI_ALDCH("en(%d), level(%d-%d), coeff(%f, %f, %f, %f, %f, %f)",
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

#if GENMESH_ONLINE
    ctx->hLDCH->isAttribUpdated = false;
    ctx->hLDCH->aldchReadMeshThread = new RKAiqAldchThread(ctx->hLDCH);
    /* ctx->hLDCH->aldchReadMeshThread->triger_start(); */
    /* ctx->hLDCH->aldchReadMeshThread->start(); */
#endif

    LDCHHandle_t ldchCtx = ctx->hLDCH;
    CalibDbV2_LDCH_t* calib_ldch_db =
            (CalibDbV2_LDCH_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, aldch));
    CalibDbV2_Ldch_Param_t* calib_ldch = &calib_ldch_db->param;

    ldchCtx->ldch_en = calib_ldch->ldch_en;
    memcpy(ldchCtx->meshfile, calib_ldch->meshfile, sizeof(ldchCtx->meshfile));
#if GENMESH_ONLINE
    ldchCtx->camCoeff.cx = calib_ldch->light_center[0];
    ldchCtx->camCoeff.cy = calib_ldch->light_center[1];
    ldchCtx->camCoeff.a0 = calib_ldch->coefficient[0];
    ldchCtx->camCoeff.a2 = calib_ldch->coefficient[1];
    ldchCtx->camCoeff.a3 = calib_ldch->coefficient[2];
    ldchCtx->camCoeff.a4 = calib_ldch->coefficient[3];
    LOGI_ALDCH("(%s) len light center(%.16f, %.16f)\n",
            __FUNCTION__,
            ldchCtx->camCoeff.cx, ldchCtx->camCoeff.cy);
    LOGI_ALDCH("(%s) len coefficient(%.16f, %.16f, %.16f, %.16f)\n",
            __FUNCTION__,
            ldchCtx->camCoeff.a0, ldchCtx->camCoeff.a2,
            ldchCtx->camCoeff.a3, ldchCtx->camCoeff.a4);
#endif
    ldchCtx->correct_level = calib_ldch->correct_level;
    ldchCtx->correct_level_max = calib_ldch->correct_level_max;

    LOGI_ALDCH("ldch en %d, meshfile: %s, correct_level-max: %d-%d from xml file",
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

#if GENMESH_ONLINE
    if (!hLDCH->aldchReadMeshThread->is_empty())
        hLDCH->aldchReadMeshThread->clear_attr();
    ldchCtx->aldchReadMeshThread->triger_stop();
    ldchCtx->aldchReadMeshThread->stop();
#endif

#if GENMESH_ONLINE
	genLdchMeshDeInit(ldchCtx->ldchParams);
#endif
    release_ldch_buf(ldchCtx);
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

    bool config_calib = !!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB);
    if (config_calib) {
        updateCalibConfig(params);
        return XCAM_RETURN_NO_ERROR;
    }

#if GENMESH_ONLINE
    // process the new attrib set before prepare
    hLDCH->aldchReadMeshThread->triger_stop();
    hLDCH->aldchReadMeshThread->stop();
    if (!hLDCH->aldchReadMeshThread->is_empty()) {
        hLDCH->aldchReadMeshThread->clear_attr();
        ldchCtx->isAttribUpdated = true;
    }

    if (ldchCtx->isAttribUpdated) {
        ldchCtx->ldch_en = ldchCtx->user_config.en;
        ldchCtx->correct_level = ldchCtx->user_config.correct_level;
        ldchCtx->isAttribUpdated = false;
    } else {
        ldchCtx->user_config.en = ldchCtx->ldch_en;
        ldchCtx->user_config.correct_level = ldchCtx->correct_level;
    }

    hLDCH->aldchReadMeshThread->triger_start();
    hLDCH->aldchReadMeshThread->start();
    if (!ldchCtx->ldch_en)
        return XCAM_RETURN_NO_ERROR;

    if (aiqGenLdchMeshInit(ldchCtx) == XCAM_RETURN_NO_ERROR) {
        bool success = aiqGenMesh(hLDCH, ldchCtx->correct_level);
        if (!success) {
            LOGW_ALDCH("lut is not exist");
            ldchCtx->ldch_en = 0;
        }
    }
#else
    if (!ldchCtx->ldch_en)
        return XCAM_RETURN_NO_ERROR;

    char filename[512];
    sprintf(filename, "%s/%s/mesh_level%d.bin",
            ldchCtx->resource_path,
            ldchCtx->meshfile,
            ldchCtx->correct_level);
    bool ret = read_mesh_from_file(ldchCtx, filename);
    if (!ret)
        ldchCtx->ldch_en = 0;
#endif

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
    RkAiqAlgoProcResAldch* ldchPreOut = (RkAiqAlgoProcResAldch*)outparams;

    if (inparams->u.proc.init) {
        ldchPreOut->ldch_result.update = 1;
    } else {
        if (ldchCtx->isAttribUpdated) {
            ldchCtx->isAttribUpdated = false;
            ldchPreOut->ldch_result.update = 1;
        } else {
            ldchPreOut->ldch_result.update = 0;
        }

        LOGV_ALDCH("(%s) en(%d), level(%d), user en(%d), level(%d), result update(%d)\n",
                __FUNCTION__,
                ldchCtx->ldch_en,
                ldchCtx->correct_level,
                ldchCtx->user_config.en,
                ldchCtx->user_config.correct_level,
                ldchPreOut->ldch_result.update);
    }

    if (ldchPreOut->ldch_result.update) {
        ldchPreOut->ldch_result.sw_ldch_en = ldchCtx->ldch_en;
        ldchPreOut->ldch_result.lut_h_size = ldchCtx->lut_h_size;
        ldchPreOut->ldch_result.lut_v_size = ldchCtx->lut_v_size;
        ldchPreOut->ldch_result.lut_map_size = ldchCtx->lut_mapxy_size;
        if (ldchCtx->ldch_en) {
            if (!ldchCtx->lut_mapxy || !ldchCtx->ldch_mem_info) {
                LOGE_ALDCH("no available ldch buf, lut_mapxy: %p, ldch_mem_info: %p",
                       ldchCtx->lut_mapxy, ldchCtx->ldch_mem_info);
                ldchPreOut->ldch_result.update = 0;
                return XCAM_RETURN_NO_ERROR;
            }
            ldchPreOut->ldch_result.lut_mapxy_buf_fd = ldchCtx->ldch_mem_info->fd;
            ldchCtx->ldch_mem_info->state[0] = 1; //mark that this buf is using.
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
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE


bool RKAiqAldchThread::loop()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;
    SmartPtr<rk_aiq_ldch_cfg_t> attrib = mAttrQueue.pop (timeout);

    if (!attrib.ptr()) {
        LOGW_ALDCH("RKAiqAldchThread got empty attrib, stop thread");
        return false;
    }
#if GENMESH_ONLINE
    if (attrib->en) {
        if (!hLDCH->ldch_en || attrib->correct_level != hLDCH->correct_level) {
            aiqGenLdchMeshInit(hLDCH);
            bool success = aiqGenMesh(hLDCH, attrib->correct_level);
            if (!success) {
                LOGW_ALDCH("lut is not exist");
            }
        }
    }

    hLDCH->ldch_en = hLDCH->user_config.en;
    hLDCH->correct_level = hLDCH->user_config.correct_level;
#endif
    if (ret == XCAM_RETURN_NO_ERROR) {
        hLDCH->isAttribUpdated = true;
        LOGV_ALDCH("ldch en(%d), level(%d)\n", hLDCH->ldch_en, hLDCH->correct_level);
        return true;
    }

    LOGE_ALDCH("RKAiqAldchThread failed to read mesh table!");

    EXIT_ANALYZER_FUNCTION();

    return false;
}
