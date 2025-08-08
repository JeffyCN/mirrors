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

#include "cp_types_prvt.h"
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

    CpContext_t* ctx = aiq_mallocz(sizeof(CpContext_t));

    if (ctx == NULL) {
        LOGE_ACP("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->cp_attrib =
        (cp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, cp));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ACP("%s: Cp (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CpContext_t* pCpCtx = (CpContext_t*)context;
    aiq_free(pCpCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CpContext_t* pCpCtx = (CpContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pCpCtx->cp_attrib =
                (cp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cp));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pCpCtx->working_mode = params->u.prepare.working_mode;
    pCpCtx->cp_attrib =
        (cp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cp));
    pCpCtx->prepare_params = &params->u.prepare;
    pCpCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    CpContext_t* pCpCtx = (CpContext_t *)inparams->ctx;
    cp_api_attrib_t* cp_attrib = pCpCtx->cp_attrib;
    cp_param_t* pCpProcResParams = outparams->algoRes;
    RkAiqAlgoProcCp* cp_proc_param = (RkAiqAlgoProcCp*)inparams;

    LOGV_ACP("%s: Cp (enter)\n", __FUNCTION__ );

    if (!cp_attrib) {
        LOGE_ACP("cp_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    float blc_ob_predgain = 1.0;
    int iso = inparams->u.proc.iso;

    if (pCpCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL)
        iso = iso * blc_ob_predgain;

    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pCpCtx->iso);

    outparams->cfg_update = false;

    if (cp_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACP("mode is %d, not auto mode, ignore", cp_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (init) {
        pCpCtx->isReCal_ = true;
    }

    if (inparams->u.proc.is_attrib_update)
        pCpCtx->isReCal_ = true;

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pCpCtx->isReCal_ = true;

    if (pCpCtx->isReCal_) {
        CpSelectParam(&pCpCtx->cp_attrib->stAuto, pCpProcResParams, iso);
        outparams->cfg_update = true;
        outparams->en = cp_attrib->en;
        outparams->bypass = cp_attrib->bypass;
        LOGD_ACP("CP Process: iso %d, cp en:%d, bypass:%d", iso, outparams->en, outparams->bypass);
    }

    pCpCtx->iso = iso;
    pCpCtx->isReCal_ = false;

    LOGV_ACP("%s: Cp (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_cp_SetAttrib(RkAiqAlgoContext *ctx,
                                    const cp_api_attrib_t *attr,
                                    bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ACP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CpContext_t* pCpCtx = (CpContext_t*)ctx;
    cp_api_attrib_t* cp_attrib = pCpCtx->cp_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACP("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    cp_attrib->opMode = attr->opMode;
    cp_attrib->en = attr->en;
    cp_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        cp_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        cp_attrib->stMan = attr->stMan;
    else {
        LOGW_ACP("wrong mode: %d\n", attr->opMode);
    }

    pCpCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_cp_GetAttrib(const RkAiqAlgoContext *ctx,
                                    cp_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ACP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CpContext_t* pCpCtx = (CpContext_t*)ctx;
    cp_api_attrib_t* cp_attrib = pCpCtx->cp_attrib;

    attr->opMode = cp_attrib->opMode;
    attr->en = cp_attrib->en;
    attr->bypass = cp_attrib->bypass;
    memcpy(&attr->stAuto, &cp_attrib->stAuto, sizeof(cp_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_ACP_V10
XCamReturn CpSelectParam
(
    cp_param_auto_t *pAuto,
    cp_param_t* out,
    int iso)
{
    if(pAuto == NULL || out == NULL) {
        LOGE_ACP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    cp_param_auto_t *paut = pAuto;
    out->sta.brightness = paut->sta.brightness;
    out->sta.hue = paut->sta.hue;
    out->sta.saturation = paut->sta.saturation;
    out->sta.contrast = paut->sta.contrast;

    return XCAM_RETURN_NO_ERROR;
}

#endif

#define RKISP_ALGO_CP_VERSION     "v0.0.9"
#define RKISP_ALGO_CP_VENDOR      "Rockchip"
#define RKISP_ALGO_CP_DESCRIPTION "Rockchip cp algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescCp = {
    .common = {
        .version = RKISP_ALGO_CP_VERSION,
        .vendor  = RKISP_ALGO_CP_VENDOR,
        .description = RKISP_ALGO_CP_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACP,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

