/*
 * rk_aiq_algo_afec_itf.c
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

#include "afec/rk_aiq_algo_afec_itf.h"
#include "afec/rk_aiq_types_afec_algo_prvt.h"
#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define EPSINON 0.0000001

RKAIQ_BEGIN_DECLARE

static XCamReturn release_fec_buf(FECContext_t* fecCtx);
static XCamReturn alloc_fec_buf(FECContext_t* fecCtx)
{
    // release-first
    release_fec_buf(fecCtx);
    rk_aiq_share_mem_config_t share_mem_config;
    share_mem_config.alloc_param.width =  fecCtx->dst_width;
    share_mem_config.alloc_param.height = fecCtx->dst_height;
    share_mem_config.alloc_param.reserved[0] = fecCtx->mesh_density;
    share_mem_config.mem_type = MEM_TYPE_FEC;
    fecCtx->share_mem_ops->alloc_mem(0, fecCtx->share_mem_ops,
                                     &share_mem_config,
                                     &fecCtx->share_mem_ctx);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn release_fec_buf(FECContext_t* fecCtx)
{
    if (fecCtx->share_mem_ctx)
        fecCtx->share_mem_ops->release_mem(0, fecCtx->share_mem_ctx);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn get_fec_buf(FECContext_t* fecCtx)
{
    fecCtx->fec_mem_info = (rk_aiq_fec_share_mem_info_t *)
            fecCtx->share_mem_ops->get_free_item(0, fecCtx->share_mem_ctx);
    if (fecCtx->fec_mem_info == NULL) {
        LOGE_AFEC( "%s(%d): no free fec buf", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_MEM;
    } else {
        LOGD_AFEC("get buf fd=%d\n",fecCtx->fec_mem_info->fd);
        fecCtx->meshxi = fecCtx->fec_mem_info->meshxi;
        fecCtx->meshxf = fecCtx->fec_mem_info->meshxf;
        fecCtx->meshyi = fecCtx->fec_mem_info->meshyi;
        fecCtx->meshyf = fecCtx->fec_mem_info->meshyf;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
gen_mesh_table(
    float k1, float k2, float p1, float p2, float k3,
    int src_width, int src_height,
    int mesh_h_size, int mesh_v_size,
    int* meshxi, int* meshxf, int* meshyi, int* meshyf) {
    //TODO::implement mesh table generation function
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_AFEC( "%s: create afec context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->hFEC = new FECContext_t();
    if (ctx->hFEC == NULL) {
        LOGE_AFEC( "%s: create afec handle fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset((void *)(ctx->hFEC), 0, sizeof(FECContext_t));
    *context = ctx;

    FECHandle_t fecCtx = ctx->hFEC;

    CalibDbV2_FEC_t* calib_fec_db =
            (CalibDbV2_FEC_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, afec));
    if (!calib_fec_db) {
        LOGE_AFEC("%s: Could not get fec iq calib!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    CalibDbV2_Fec_Param_t* calib_fec = &calib_fec_db->param;

    memset(&fecCtx->user_config, 0, sizeof(fecCtx->user_config));
#if RKAIQ_HAVE_EIS_V1
    CalibDbV2_Eis_t* calib_eis =
                (CalibDbV2_Eis_t*)(CALIBDBV2_GET_MODULE_PTR(cfg->calibv2, eis_calib));
    fecCtx->fec_en = fecCtx->user_config.en = calib_eis->enable ? 0 : calib_fec->fec_en;
    if (!fecCtx->fec_en) {
        if (calib_eis->enable) {
            LOGE_AFEC("FEC diabled because of EIS");
        }
        return XCAM_RETURN_NO_ERROR;
    }
#else
    fecCtx->fec_en = fecCtx->user_config.en = calib_fec->fec_en;
    if (!fecCtx->fec_en) {
        return XCAM_RETURN_NO_ERROR;
    }
#endif

#if GENMESH_ONLINE
    ctx->hFEC->isAttribUpdated = false;
    ctx->hFEC->afecReadMeshThread = new RKAiqAfecThread(ctx->hFEC);
#endif

    memcpy(fecCtx->meshfile, calib_fec->meshfile, sizeof(fecCtx->meshfile));
#if GENMESH_ONLINE
    fecCtx->camCoeff.cx = calib_fec->light_center[0];
    fecCtx->camCoeff.cy = calib_fec->light_center[1];
    fecCtx->camCoeff.a0 = calib_fec->coefficient[0];
    fecCtx->camCoeff.a2 = calib_fec->coefficient[1];
    fecCtx->camCoeff.a3 = calib_fec->coefficient[2];
    fecCtx->camCoeff.a4 = calib_fec->coefficient[3];
    LOGI_AFEC("(%s) len light center(%.16f, %.16f)\n",
            __FUNCTION__,
            fecCtx->camCoeff.cx, fecCtx->camCoeff.cy);
    LOGI_AFEC("(%s) len coefficient(%.16f, %.16f, %.16f, %.16f)\n",
            __FUNCTION__,
            fecCtx->camCoeff.a0, fecCtx->camCoeff.a2,
            fecCtx->camCoeff.a3, fecCtx->camCoeff.a4);
#endif
    fecCtx->correct_level = calib_fec->correct_level;
    fecCtx->correct_level = fecCtx->user_config.correct_level = calib_fec->correct_level;
    fecCtx->correct_direction = fecCtx->user_config.direction = FEC_CORRECT_DIRECTION_XY;
    fecCtx->fecParams.correctX = 1;
    fecCtx->fecParams.correctY = 1;
    fecCtx->fecParams.saveMesh4bin = 0;

    ctx->hFEC->eState = FEC_STATE_INVALID;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    FECHandle_t hFEC = (FECHandle_t)context->hFEC;
    FECContext_t* fecCtx = (FECContext_t*)hFEC;
#if GENMESH_ONLINE
    if (fecCtx->fec_en) {
        fecCtx->afecReadMeshThread->triger_stop();
        fecCtx->afecReadMeshThread->stop();
        //    if (fecCtx->meshxi != NULL || fecCtx->meshyi != NULL ||
        //        fecCtx->meshxf != NULL || fecCtx->meshyf != NULL)
        //        freeFecMesh(fecCtx->meshxi, fecCtx->meshxf,
        //                fecCtx->meshyi, fecCtx->meshyf);
        genFecMeshDeInit(fecCtx->fecParams);
    }
#else
//    if (fecCtx->meshxi != NULL) {
//        free(fecCtx->meshxi);
//        fecCtx->meshxi = NULL;
//    }
 
//    if (fecCtx->meshyi != NULL) {
//        free(fecCtx->meshyi);
//        fecCtx->meshyi = NULL;
//    }

//    if (fecCtx->meshxf != NULL) {
//        free(fecCtx->meshxf);
//        fecCtx->meshxf = NULL;
//    }
//
//    if (fecCtx->meshyf != NULL) {
//        free(fecCtx->meshyf);
//        fecCtx->meshyf = NULL;
//    }
#endif
    release_fec_buf(fecCtx);
    delete context->hFEC;
    context->hFEC = NULL;
    delete context;
    context = NULL;
    return XCAM_RETURN_NO_ERROR;
}

#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN(x,a)      __ALIGN_MASK(x, a-1)

uint32_t cal_fec_mesh(uint32_t width, uint32_t height, uint32_t mode, uint32_t &meshw, uint32_t &meshh)
{
    uint32_t mesh_size, mesh_left_height;
    uint32_t w = ALIGN(width, 32);
    uint32_t h = ALIGN(height, 32);
    uint32_t spb_num = (h + 127) >> 7;
    uint32_t left_height = h & 127;
    uint32_t mesh_width = mode ? (w / 32 + 1) : (w / 16 + 1);
    uint32_t mesh_height = mode ? 9 : 17;

    if (!left_height)
        left_height = 128;
    mesh_left_height = mode ? (left_height / 16 + 1) :
                       (left_height / 8 + 1);
    mesh_size = (spb_num - 1) * mesh_width * mesh_height +
                mesh_width * mesh_left_height;

    meshw = mesh_width;
    meshh = (spb_num - 1) * mesh_height + (spb_num - 1);
    return mesh_size;
}

static XCamReturn
read_mesh_table(FECContext_t* fecCtx, unsigned int correct_level)
{
#if OPENCV_SUPPORT
    gen_default_mesh_table(fecCtx->src_width, fecCtx->src_height, fecCtx->mesh_density,
                           fecCtx->meshxi, fecCtx->meshyi, fecCtx->meshxf, fecCtx->meshyf);
#elif GENMESH_ONLINE
    bool ret;
    ret = genFECMeshNLevel(fecCtx->fecParams, fecCtx->camCoeff, correct_level, fecCtx->meshxi,
                           fecCtx->meshxf, fecCtx->meshyi, fecCtx->meshyf);
    if (!ret) {
        LOGE_AFEC("afec gen mesh false!");
        return XCAM_RETURN_ERROR_FAILED;
    }
#else
    FILE* ofp;
    char filename[512];
    sprintf(filename, "%s/%s/meshxi_level%u.bin",
            fecCtx->resource_path,
            fecCtx->meshfile,
            correct_level);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshxi, 1, fecCtx->fec_mesh_size * sizeof(unsigned short), ofp);
        fclose(ofp);

        if (num != fecCtx->fec_mesh_size * sizeof(unsigned short)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh XI file");
        }
    } else {
        LOGE_AFEC("mesh XI file %s not exist", filename);
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "%s/%s/meshxf_level%u.bin",
            fecCtx->resource_path,
            fecCtx->meshfile,
            correct_level);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshxf, 1, fecCtx->fec_mesh_size * sizeof(unsigned char), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned char)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh XF file");
        }
    } else {
        LOGE_AFEC("mesh XF file %s not exist", filename);
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "%s/%s/meshyi_level%u.bin",
            fecCtx->resource_path,
            fecCtx->meshfile,
            correct_level);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshyi, 1, fecCtx->fec_mesh_size * sizeof(unsigned short), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned short)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh YI file");
        }
    } else {
        LOGE_AFEC("mesh YI file %s not exist", filename);
        fecCtx->fec_en = 0;
    }

    sprintf(filename, "%s/%s/meshyf_level%u.bin",
            fecCtx->resource_path,
            fecCtx->meshfile,
            correct_level);
    ofp = fopen(filename, "rb");
    if (ofp != NULL) {
        unsigned int num = fread(fecCtx->meshyf, 1, fecCtx->fec_mesh_size * sizeof(unsigned char), ofp);
        fclose(ofp);
        if (num != fecCtx->fec_mesh_size * sizeof(unsigned char)) {
            fecCtx->fec_en = 0;
            LOGE_AFEC("mismatched mesh YF file");
        }
    } else {
        LOGE_AFEC("mesh YF file %s not exist", filename);
        fecCtx->fec_en = 0;
    }
#endif

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    FECHandle_t hFEC = (FECHandle_t)params->ctx->hFEC;
    FECContext_t* fecCtx = (FECContext_t*)hFEC;
    RkAiqAlgoConfigAfec* rkaiqAfecConfig = (RkAiqAlgoConfigAfec*)params;

    if (!fecCtx->fec_en)
        return XCAM_RETURN_NO_ERROR;

#if 0 //moved to create_ctx
    fecCtx->fec_en = rkaiqAfecConfig->afec_calib_cfg.fec_en;
    memcpy(fecCtx->meshfile, rkaiqAfecConfig->afec_calib_cfg.meshfile, sizeof(fecCtx->meshfile));
    fecCtx->camCoeff.cx = rkaiqAfecConfig->afec_calib_cfg.light_center[0];
    fecCtx->camCoeff.cy = rkaiqAfecConfig->afec_calib_cfg.light_center[1];
    fecCtx->camCoeff.a0 = rkaiqAfecConfig->afec_calib_cfg.coefficient[0];
    fecCtx->camCoeff.a2 = rkaiqAfecConfig->afec_calib_cfg.coefficient[1];
    fecCtx->camCoeff.a3 = rkaiqAfecConfig->afec_calib_cfg.coefficient[2];
    fecCtx->camCoeff.a4 = rkaiqAfecConfig->afec_calib_cfg.coefficient[3];
    LOGI_AFEC("(%s) len light center(%.16f, %.16f)\n",
            __FUNCTION__,
            fecCtx->camCoeff.cx, fecCtx->camCoeff.cy);
    LOGI_AFEC("(%s) len coefficient(%.16f, %.16f, %.16f, %.16f)\n",
            __FUNCTION__,
            fecCtx->camCoeff.a0, fecCtx->camCoeff.a2,
            fecCtx->camCoeff.a3, fecCtx->camCoeff.a4);
#endif
    fecCtx->share_mem_ops = rkaiqAfecConfig->mem_ops_ptr;
    fecCtx->src_width = params->u.prepare.sns_op_width;
    fecCtx->src_height = params->u.prepare.sns_op_height;
    fecCtx->resource_path = rkaiqAfecConfig->resource_path;
    fecCtx->dst_width = params->u.prepare.sns_op_width;
    fecCtx->dst_height = params->u.prepare.sns_op_height;

    if (fecCtx->src_width <= 1920) {
        fecCtx->mesh_density = 0;
    } else {
        fecCtx->mesh_density = 1;
    }

#if GENMESH_ONLINE
    // process the new attrib set before prepare
    fecCtx->afecReadMeshThread->triger_stop();
    fecCtx->afecReadMeshThread->stop();
    if (!fecCtx->afecReadMeshThread->is_empty()) {
        fecCtx->afecReadMeshThread->clear_attr();
        fecCtx->isAttribUpdated = true;
    }
#endif

    double correct_level = fecCtx->correct_level;
    if (fecCtx->isAttribUpdated) {
        fecCtx->fec_en = fecCtx->user_config.en;
        if (fecCtx->user_config.bypass)
            correct_level = 0;
        correct_level = fecCtx->user_config.correct_level;
        switch (fecCtx->user_config.direction) {
        case FEC_CORRECT_DIRECTION_X:
            fecCtx->fecParams.correctY = 0;
            fecCtx->correct_direction = fecCtx->user_config.direction;
            break;
        case FEC_CORRECT_DIRECTION_Y:
            fecCtx->fecParams.correctX = 0;
            fecCtx->correct_direction = fecCtx->user_config.direction;
            break;
        default:
            break;
        }

        fecCtx->mode = fecCtx->user_config.mode;
        switch (fecCtx->mode) {
        case FEC_COMPRES_IMAGE_KEEP_FOV:
            fecCtx->fecParams.saveMaxFovX = 1;
            break;
        case FEC_KEEP_ASPECT_RATIO_REDUCE_FOV:
            fecCtx->fecParams.saveMaxFovX = 0;
            break;
        case FEC_ALTER_ASPECT_RATIO_KEEP_FOV:
            break;
        default:
            break;
        }

        if (fecCtx->fecParams.saveMesh4bin)
            sprintf(fecCtx->fecParams.mesh4binPath, "/tmp/");

        fecCtx->isAttribUpdated = false;
    } else {
        fecCtx->user_config.en = fecCtx->fec_en;
    }
    fecCtx->user_config.correct_level = correct_level;

#if GENMESH_ONLINE
    // deinit firtly
    if (fecCtx->fecParams.pMeshXY)
        genFecMeshDeInit(fecCtx->fecParams);

    //if (fecCtx->meshxi != NULL || fecCtx->meshyi != NULL ||
    //    fecCtx->meshxf != NULL || fecCtx->meshyf != NULL)
    //    freeFecMesh(fecCtx->meshxi, fecCtx->meshxf,
    //            fecCtx->meshyi, fecCtx->meshyf);

    fecCtx->fecParams.isFecOld = 1;
    genFecMeshInit(fecCtx->src_width, fecCtx->src_height, fecCtx->dst_width,
            fecCtx->dst_height, fecCtx->fecParams, fecCtx->camCoeff);
    //mallocFecMesh(fecCtx->fecParams.meshSize4bin, &fecCtx->meshxi,
    //        &fecCtx->meshxf, &fecCtx->meshyi, &fecCtx->meshyf);
    fecCtx->fec_mesh_h_size = fecCtx->fecParams.meshSizeW;
    fecCtx->fec_mesh_v_size = fecCtx->fecParams.meshSizeH;
    alloc_fec_buf(fecCtx);
    get_fec_buf(fecCtx);
    fecCtx->fec_mesh_size = fecCtx->fecParams.meshSize4bin;
    LOGI_AFEC("en: %d, mode(%d), bypass(%d), correct_level(%d), direction(%d), dimen(%d-%d), mesh dimen(%d-%d), size(%d)",
              fecCtx->fec_en,
              fecCtx->mode,
              fecCtx->user_config.bypass,
              fecCtx->user_config.correct_level,
              fecCtx->correct_direction,
              fecCtx->src_width, fecCtx->src_height,
              fecCtx->fec_mesh_h_size, fecCtx->fec_mesh_v_size,
              fecCtx->fec_mesh_size);

    fecCtx->afecReadMeshThread->triger_start();
    fecCtx->afecReadMeshThread->start();
    if (!fecCtx->fec_en)
        return XCAM_RETURN_NO_ERROR;
#else
    fecCtx->fec_mesh_size =
        cal_fec_mesh(fecCtx->src_width, fecCtx->src_height, fecCtx->mesh_density,
                     fecCtx->fec_mesh_h_size, fecCtx->fec_mesh_v_size);

    LOGI_AFEC("(%s) en: %d, user_en: %d, correct_level: %d, dimen: %d-%d, mesh dimen: %d-%d, size: %d",
              fecCtx->meshfile, fecCtx->fec_en,
              fecCtx->user_config.en, fecCtx->user_config.correct_level,
              fecCtx->src_width, fecCtx->src_height,
              fecCtx->fec_mesh_h_size, fecCtx->fec_mesh_v_size,
              fecCtx->fec_mesh_size);
    if (!fecCtx->fec_en)
        return XCAM_RETURN_NO_ERROR;

    // need realloc ?
    if (fecCtx->meshxi) {
        free(fecCtx->meshxi);
        fecCtx->meshxi = NULL;
    }
    if (fecCtx->meshxf) {
        free(fecCtx->meshxf);
        fecCtx->meshxf = NULL;
    }
    if (fecCtx->meshyi) {
        free(fecCtx->meshyi);
        fecCtx->meshyi = NULL;
    }
    if (fecCtx->meshyf) {
        free(fecCtx->meshyf);
        fecCtx->meshyf = NULL;
    }
    fecCtx->meshxi = (unsigned short*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned short));
    fecCtx->meshxf = (unsigned char*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned char));
    fecCtx->meshyi = (unsigned short*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned short));
    fecCtx->meshyf = (unsigned char*)malloc(fecCtx->fec_mesh_size * sizeof(unsigned char));

#endif
    read_mesh_table(fecCtx, fecCtx->user_config.correct_level);
    fecCtx->eState = FEC_STATE_INITIALIZED;

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
    FECHandle_t hFEC = (FECHandle_t)inparams->ctx->hFEC;
    FECContext_t* fecCtx = (FECContext_t*)hFEC;
    RkAiqAlgoProcResAfec* fecPreOut = (RkAiqAlgoProcResAfec*)outparams;

    if (!fecCtx->fec_en)
        return XCAM_RETURN_NO_ERROR;

    fecPreOut->afec_result->sw_fec_en = fecCtx->fec_en;
    fecPreOut->afec_result->crop_en = 0;
    fecPreOut->afec_result->crop_width = 0;
    fecPreOut->afec_result->crop_height = 0;
    fecPreOut->afec_result->mesh_density = fecCtx->mesh_density;
    fecPreOut->afec_result->mesh_size = fecCtx->fec_mesh_size;
    // TODO: should check the fec mode,
    // if mode == RK_AIQ_ISPP_STATIC_FEC_WORKING_MODE_STABLIZATION
    // params may be changed
    fecCtx->eState = FEC_STATE_RUNNING;

    if (inparams->u.proc.init) {
        outparams->cfg_update = true;
    } else {

        if (fecCtx->isAttribUpdated) {
            fecCtx->isAttribUpdated = false;
            outparams->cfg_update = true;
        } else {
            outparams->cfg_update = false;
        }

        LOGV_AFEC("en(%d), bypass(%d), level(%d), direction(%d), result update(%d)\n",
                fecCtx->fec_en,
                fecCtx->user_config.bypass,
                fecCtx->correct_level,
                fecCtx->correct_direction,
                outparams->cfg_update);
    }

    if (outparams->cfg_update) {
        //memcpy(fecPreOut->afec_result.meshxi, fecCtx->meshxi,
        //       fecCtx->fec_mesh_size * sizeof(unsigned short));
        //memcpy(fecPreOut->afec_result.meshxf, fecCtx->meshxf,
        //       fecCtx->fec_mesh_size * sizeof(unsigned char));
        //memcpy(fecPreOut->afec_result.meshyi, fecCtx->meshyi,
        //       fecCtx->fec_mesh_size * sizeof(unsigned short));
        //memcpy(fecPreOut->afec_result.meshyf, fecCtx->meshyf,
        //       fecCtx->fec_mesh_size * sizeof(unsigned char));

        if (fecCtx->fec_mem_info == NULL) {
            LOGE_AFEC("%s: no available fec buf!", __FUNCTION__);
            outparams->cfg_update = false;
            return XCAM_RETURN_NO_ERROR;
        }
        fecPreOut->afec_result->mesh_buf_fd = fecCtx->fec_mem_info->fd;
        fecCtx->fec_mem_info->state[0] = 1; //mark that this buf is using.
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAfec = {
    .common = {
        .version = RKISP_ALGO_AFEC_VERSION,
        .vendor  = RKISP_ALGO_AFEC_VENDOR,
        .description = RKISP_ALGO_AFEC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AFEC,
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

bool RKAiqAfecThread::loop()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;
    SmartPtr<rk_aiq_fec_cfg_t> attrib = mAttrQueue.pop (timeout);

    if (!attrib.ptr()) {
        LOGW_ANALYZER("RKAiqAfecThread got empty attrib, stop thread");
        return false;
    }

    if (hFEC->eState != FEC_STATE_RUNNING) {
        hFEC->isAttribUpdated = true;
        return true;
    }
    get_fec_buf(hFEC);
    if (hFEC->user_config.bypass) {
        ret = read_mesh_table(hFEC, 0);
    } else {
        ret = read_mesh_table(hFEC, attrib->correct_level);
        hFEC->correct_level = attrib->correct_level;
    }

    if (ret == XCAM_RETURN_NO_ERROR) {
        hFEC->isAttribUpdated = true;
        return true;
    }

    LOGE_ANALYZER("RKAiqAfecThread failed to read mesh table!");

    EXIT_ANALYZER_FUNCTION();

    return false;
}
