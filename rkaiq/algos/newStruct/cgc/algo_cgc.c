/*
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

#include "cgc_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "c_base/aiq_base.h"
#include "interpolation.h"

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    CgcContext_t* ctx = aiq_mallocz(sizeof(CgcContext_t));

    if (ctx == NULL) {
        LOGE_ACGC("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->cgc_attrib =
        (cgc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, cgc));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ACGC("%s: Cgc (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CgcContext_t* pCgcCtx = (CgcContext_t*)context;
    aiq_free(pCgcCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CgcContext_t* pCgcCtx = (CgcContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pCgcCtx->cgc_attrib =
                (cgc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cgc));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pCgcCtx->working_mode = params->u.prepare.working_mode;
    pCgcCtx->cgc_attrib =
        (cgc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cgc));
    pCgcCtx->prepare_params = &params->u.prepare;
    pCgcCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    CgcContext_t* pCgcCtx = (CgcContext_t *)inparams->ctx;
    cgc_api_attrib_t* cgc_attrib = pCgcCtx->cgc_attrib;
    cgc_param_t* pCgcProcResParams = outparams->algoRes;
    RkAiqAlgoProcCgc* cgc_proc_param = (RkAiqAlgoProcCgc*)inparams;

    LOGV_ACGC("%s: Cgc (enter)\n", __FUNCTION__ );

    if (!cgc_attrib) {
        LOGE_ACGC("cgc_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    float blc_ob_predgain = 1.0;
    int iso = inparams->u.proc.iso;

    if (pCgcCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        iso = iso * blc_ob_predgain;

    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pCgcCtx->iso);

    outparams->cfg_update = false;

    if (cgc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACGC("mode is %d, not auto mode, ignore", cgc_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (init) {
        pCgcCtx->isReCal_ = true;
    }

    if (inparams->u.proc.is_attrib_update)
        pCgcCtx->isReCal_ = true;

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pCgcCtx->isReCal_ = true;

    if (pCgcCtx->isReCal_) {
        pCgcProcResParams->sta.cgc_ratio_en = false;
        pCgcProcResParams->sta.cgc_yuv_limit = false;
        outparams->cfg_update = true;
        outparams->en = cgc_attrib->en;
        outparams->bypass = cgc_attrib->bypass;
        LOGD_ACGC("CGC Process: iso %d, cgc en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    pCgcCtx->iso = iso;
    pCgcCtx->isReCal_ = false;

    LOGV_ACGC("%s: Cgc (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}
#if 0
XCamReturn
algo_cgc_SetAttrib(RkAiqAlgoContext *ctx,
                                    const cgc_api_attrib_t *attr,
                                    bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ACGC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CgcContext_t* pCgcCtx = (CgcContext_t*)ctx;
    cgc_api_attrib_t* cgc_attrib = pCgcCtx->cgc_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACGC("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    cgc_attrib->opMode = attr->opMode;
    cgc_attrib->en = attr->en;
    cgc_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        cgc_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        cgc_attrib->stMan = attr->stMan;
    else {
        LOGW_ACGC("wrong mode: %d\n", attr->opMode);
    }

    pCgcCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_cgc_GetAttrib(const RkAiqAlgoContext *ctx,
                                    cgc_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ACGC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CgcContext_t* pCgcCtx = (CgcContext_t*)ctx;
    cgc_api_attrib_t* cgc_attrib = pCgcCtx->cgc_attrib;

    attr->opMode = cgc_attrib->opMode;
    attr->en = cgc_attrib->en;
    attr->bypass = cgc_attrib->bypass;
    memcpy(&attr->stAuto, &cgc_attrib->stAuto, sizeof(cgc_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#define RKISP_ALGO_CGC_VERSION     "v0.0.9"
#define RKISP_ALGO_CGC_VENDOR      "Rockchip"
#define RKISP_ALGO_CGC_DESCRIPTION "Rockchip cgc algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescCgc = {
    .common = {
        .version = RKISP_ALGO_CGC_VERSION,
        .vendor  = RKISP_ALGO_CGC_VENDOR,
        .description = RKISP_ALGO_CGC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACGC,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

