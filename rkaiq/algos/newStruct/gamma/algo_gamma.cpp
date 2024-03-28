/*
 * rk_aiq_algo_debayer_itf.c
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

#include "rk_aiq_algo_types.h"
#include "gamma_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqHandle.h"

//RKAIQ_BEGIN_DECLARE

XCamReturn GammaSelectParam(GammaContext_t *pGammaCtx, gamma_param_t* out);

static XCamReturn
create_context
(
    RkAiqAlgoContext** context,
    const AlgoCtxInstanceCfg* cfg
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    GammaContext_t *ctx = new GammaContext_t();
    if (ctx == NULL) {
        LOGE_AGAMMA( "%s: create Gamma context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->gamma_attrib =
        (gamma_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, gamma));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_AGAMMA("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context
(
    RkAiqAlgoContext* context
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    GammaContext_t* pGammaCtx = (GammaContext_t*)context;
    delete pGammaCtx;
    return result;
}

static XCamReturn
prepare
(
    RkAiqAlgoCom* params
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    GammaContext_t* pGammaCtx = (GammaContext_t*)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pGammaCtx->gamma_attrib =
                (gamma_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, gamma));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pGammaCtx->gamma_attrib =
        (gamma_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, gamma));
    pGammaCtx->prepare_params = &params->u.prepare;
    pGammaCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    GammaContext_t* pGammaCtx = (GammaContext_t*)inparams->ctx;
    gamma_api_attrib_t* gamma_attrib = pGammaCtx->gamma_attrib;
    RkAiqAlgoProcResGamma* pGammaProcResParams = (RkAiqAlgoProcResGamma*)outparams;

    LOGV_AGAMMA("%s: (enter)\n", __FUNCTION__ );

    if (!gamma_attrib) {
        LOGE_AGAMMA("gamma_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    if (gamma_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AGAMMA("mode is %d, not auto mode, ignore", gamma_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pGammaCtx->isReCal_ = true;
    }

    if (pGammaCtx->isReCal_) {
        GammaSelectParam(&pGammaCtx->gamma_attrib->stAuto, pGammaProcResParams->gammaRes);
        outparams->cfg_update = true;
        outparams->en = gamma_attrib->en;
        outparams->bypass = gamma_attrib->bypass;
        LOGI_AGAMMA("gamma en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pGammaCtx->isReCal_ = false;

    LOGV_AGAMMA("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn GammaSelectParam
(
    gamma_param_auto_t* paut,
    gamma_param_t* out)
{
    LOGI_AGAMMA("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(paut == NULL || out ==NULL) {
        LOGE_AGAMMA("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    int gamma_out_segnum = ISP3X_SEGNUM_LOG_49;

    out->sta.Gamma_out_offset = paut->sta.Gamma_out_offset;
    out->sta.equ_segm = gamma_out_segnum > 1 ? ISP3X_SEGNUM_LOG_45: gamma_out_segnum;
    out->sta.EnableDot49 = gamma_out_segnum == ISP3X_SEGNUM_LOG_49 ? true : false;

    for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V11; i++) {
        out->sta.Gamma_curve[i] = paut->sta.Gamma_curve[i];
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_gamma_SetAttrib
(
    RkAiqAlgoContext* ctx,
    gamma_api_attrib_t *attr
) {
    if(ctx == NULL || attr == NULL) {
        LOGE_AGAMMA("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    GammaContext_t* pGammaCtx = (GammaContext_t*)ctx;
    gamma_api_attrib_t* gamma_attrib = pGammaCtx->gamma_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AGAMMA("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    gamma_attrib->opMode = attr->opMode;
    gamma_attrib->en = attr->en;
    gamma_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        gamma_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        gamma_attrib->stMan = attr->stMan;
    else {
        LOGW_AGAMMA("wrong mode: %d\n", attr->opMode);
    }

    pGammaCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_gamma_GetAttrib
(
    RkAiqAlgoContext* ctx,
    gamma_api_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_AGAMMA("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    GammaContext_t* pGammaCtx = (GammaContext_t*)ctx;
    gamma_api_attrib_t* gamma_attrib = pGammaCtx->gamma_attrib;

#if 0
    if (gamma_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AGAMMA("not auto mode: %d", gamma_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = gamma_attrib->opMode;
    attr->en = gamma_attrib->en;
    attr->bypass = gamma_attrib->bypass;
    memcpy(&attr->stAuto, &gamma_attrib->stAuto, sizeof(gamma_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_GAMMA_VERSION     "v0.1.0"
#define RKISP_ALGO_GAMMA_VENDOR      "Rockchip"
#define RKISP_ALGO_GAMMA_DESCRIPTION "Rockchip gamma algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescGamma = {
    .common = {
        .version = RKISP_ALGO_GAMMA_VERSION,
        .vendor  = RKISP_ALGO_GAMMA_VENDOR,
        .description = RKISP_ALGO_GAMMA_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGAMMA,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

//RKAIQ_END_DECLARE
