/*
 * algo_csm.cpp
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

#include "algo_types_priv.h"
#include "csm_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
//#include "RkAiqHandle.h"

#include "interpolation.h"
#include "c_base/aiq_base.h"

// RKAIQ_BEGIN_DECLARE

static csm_param_t g_csm_def = {
    .sta = {
        .hw_csmT_full_range = true,
        .hw_csmT_y_offset = 0, // 0:Y = f(coe_x) + 0/16 else Y = f(coe_x) + y_offset
        .hw_csmT_c_offset = 0, // 0:Cb/r = f(coe_x) + 128 else Cb/r = f(coe_x) + c_offset
        .sw_csmT_coeff = {
            0.299, 0.587, 0.114,
            -0.169, -0.331, 0.5,
            0.5, -0.419, -0.081
        }
    }
};

static XCamReturn CsmSelectParam(CsmContext_t *pCsmCtx, csm_param_t* out);

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;
    LOG1_ACSM("%s: (enter)\n", __FUNCTION__ );

    CsmContext_t* ctx = aiq_mallocz(sizeof(CsmContext_t));

    ctx->isReCal_ = true;
    ctx->csm_attrib =
        (csm_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, csm));

    *context = (RkAiqAlgoContext *)(ctx);

    LOG1_ACSM("%s: (exit)\n", __FUNCTION__ );
    return result;

}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ACSM("%s: (enter)\n", __FUNCTION__ );

#if 1
    CsmContext_t* pCsmCtx = (CsmContext_t*)context;
    aiq_free(pCsmCtx);
#endif

    LOG1_ACSM("%s: (exit)\n", __FUNCTION__ );
    return result;

}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ACSM("%s: (enter)\n", __FUNCTION__ );
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    CsmContext_t* pCsmCtx = (CsmContext_t *)params->ctx;
    RkAiqAlgoConfigAcsm* pCfgParam = (RkAiqAlgoConfigAcsm*)params;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pCsmCtx->csm_attrib =
                (csm_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, csm));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pCsmCtx->csm_attrib =
        (csm_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, csm));
    pCsmCtx->isReCal_ = true;

    LOG1_ACSM("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ACSM("%s: (enter)\n", __FUNCTION__ );
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ACSM("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOG1_ACSM("%s: (enter)\n", __FUNCTION__ );


    RkAiqAlgoProcAcsm* pAcsmProcParams = (RkAiqAlgoProcAcsm*)inparams;
    csm_param_t* pAcsmProcResParams = outparams->algoRes;
    CsmContext_t* pCsmCtx = (CsmContext_t *)inparams->ctx;
    csm_api_attrib_t* csm_attrib = pCsmCtx->csm_attrib;

    if (pAcsmProcParams->com.u.proc.is_attrib_update)
        pCsmCtx->isReCal_ = true;

    if (pCsmCtx->isReCal_) {
        CsmSelectParam(pCsmCtx, pAcsmProcResParams);
        outparams->cfg_update = true;
        outparams->en = csm_attrib->en;
        outparams->bypass = csm_attrib->bypass;
        LOGD_ACSM("%s:%d processing ACSM recalculate\n", __FUNCTION__, __LINE__);
    } else {
        outparams->cfg_update = false;
    }

    LOG1_ACSM("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
CsmSelectParam(CsmContext_t *pCsmCtx, csm_param_t* out)
{
    LOG1_ACSM("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pCsmCtx == NULL) {
        LOGE_ACSM("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (out == NULL) {
        LOGE_ACSM("%s(%d): NULL pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    // TODO: selecct param
    if (pCsmCtx->csm_attrib->opMode == RK_AIQ_OP_MODE_AUTO)
        memcpy(out, &g_csm_def, sizeof(csm_param_t));

    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_csm_SetAttrib
(
    RkAiqAlgoContext* ctx,
    csm_api_attrib_t *attr
) {
    if(ctx == NULL || attr == NULL) {
        LOGE_ACSM("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CsmContext_t* pCsmCtx = (CsmContext_t*)ctx;
    csm_api_attrib_t* csm_attrib = pCsmCtx->csm_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACSM("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    csm_attrib->opMode = attr->opMode;
    csm_attrib->en = attr->en;
    csm_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        csm_attrib->stMan = attr->stMan;
    else {
        LOGW_ACSM("wrong mode: %d\n", attr->opMode);
    }

    pCsmCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_csm_GetAttrib
(
    RkAiqAlgoContext* ctx,
    csm_api_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ACSM("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CsmContext_t* pCsmCtx = (CsmContext_t*)ctx;
    csm_api_attrib_t* csm_attrib = pCsmCtx->csm_attrib;

#if 0
    if (csm_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACSM("not auto mode: %d", csm_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = csm_attrib->opMode;
    attr->en = csm_attrib->en;
    attr->bypass = csm_attrib->bypass;

    return XCAM_RETURN_NO_ERROR;
}
#endif
#define RKISP_ALGO_CSM_VERSION     "v0.0.1"
#define RKISP_ALGO_CSM_VENDOR      "Rockchip"
#define RKISP_ALGO_CSM_DESCRIPTION "Rockchip csm algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCsm = {
    .common = {
        .version = RKISP_ALGO_CSM_VERSION,
        .vendor  = RKISP_ALGO_CSM_VENDOR,
        .description = RKISP_ALGO_CSM_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACSM,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

// RKAIQ_END_DECLARE
