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

#include "ie_types_prvt.h"
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

    IeContext_t* ctx = aiq_mallocz(sizeof(IeContext_t));

    if (ctx == NULL) {
        LOGE_AIE("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->ie_attrib =
        (ie_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, ie));

    CalibDbV2_ColorAsGrey_t *colorAsGrey =
        (CalibDbV2_ColorAsGrey_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, colorAsGrey));

    if (colorAsGrey && colorAsGrey->param.enable) {
        ctx->pre_params.skip_frame = colorAsGrey->param.skip_frame;
    } else {
        ctx->pre_params.skip_frame = 10;
    }

    *context = (RkAiqAlgoContext* )ctx;

    LOGV_AIE("%s: Ie (exit)\n", __FUNCTION__ );

    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    IeContext_t* pIeCtx = (IeContext_t*)context;
    aiq_free(pIeCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    IeContext_t* pIeCtx = (IeContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pIeCtx->ie_attrib =
                (ie_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ie));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    if (pIeCtx->skip_frame <= 0) {
        CalibDbV2_ColorAsGrey_t *colorAsGrey =
            (CalibDbV2_ColorAsGrey_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, colorAsGrey));

        if (colorAsGrey && colorAsGrey->param.enable) {
            pIeCtx->pre_params.skip_frame = colorAsGrey->param.skip_frame;
        }
    }

    pIeCtx->ie_attrib =
        (ie_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ie));
    
    pIeCtx->isReCal_ = true;

    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    IeContext_t* pIeCtx = (IeContext_t *)inparams->ctx;
    ie_api_attrib_t* ie_attrib = pIeCtx->ie_attrib;
    ie_param_t* pIeProcResParams = outparams->algoRes;
    RkAiqAlgoProcIe* ie_proc_param = (RkAiqAlgoProcIe*)inparams;

    LOGV_AIE("%s: Ie (enter)\n", __FUNCTION__ );

    outparams->cfg_update = false;
    bool init = inparams->u.proc.init;
    if (ie_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AIE("mode is %d, not auto mode, ignore", ie_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    int gray_mode = inparams->u.proc.gray_mode;

    if (inparams->u.proc.is_attrib_update || init || pIeCtx->isReCal_) {
        if (pIeCtx->mode && !pIeCtx->ie_attrib->en) {
            pIeCtx->skip_frame = pIeCtx->pre_params.skip_frame;
        }
        
        if (!gray_mode && !pIeCtx->mode) {
            pIeCtx->mode = pIeCtx->ie_attrib->en;
        }
        pIeCtx->isReCal_ = true;
    }

    if (gray_mode && !pIeCtx->mode) {
        pIeCtx->pre_params.mode = pIeCtx->mode;
        pIeCtx->mode = gray_mode;
        pIeCtx->skip_frame = pIeCtx->pre_params.skip_frame;
        pIeCtx->isReCal_ = true;
    } else if (!gray_mode && pIeCtx->mode) {
        if ((pIeCtx->skip_frame) &&
            (--pIeCtx->skip_frame == 0)) {
            pIeCtx->mode = pIeCtx->pre_params.mode;
            pIeCtx->isReCal_ = true;
        }
        if (pIeCtx->skip_frame) {
            LOGD_AIE("still need skip %d frames !!! \n", pIeCtx->skip_frame);
        }
    }

    if (pIeCtx->isReCal_) {
        outparams->cfg_update = true;
        outparams->en = pIeCtx->mode;
        outparams->bypass = ie_attrib->bypass;
        LOGD_AIE("Ie Process: ie en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pIeCtx->isReCal_ = false;

    LOGV_AIE("%s: Ie (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_ie_SetAttrib(RkAiqAlgoContext *ctx,
                                    const ie_api_attrib_t *attr,
                                    bool need_sync)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_AIE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    IeContext_t* pIeCtx = (IeContext_t*)ctx;
    ie_api_attrib_t* ie_attrib = pIeCtx->ie_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AIE("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    ie_attrib->opMode = attr->opMode;
    ie_attrib->en = attr->en;
    ie_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        ie_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        ie_attrib->stMan = attr->stMan;
    else {
        LOGW_AIE("wrong mode: %d\n", attr->opMode);
    }

    pIeCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ie_GetAttrib(const RkAiqAlgoContext *ctx,
                                    ie_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_AIE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    IeContext_t* pIeCtx = (IeContext_t*)ctx;
    ie_api_attrib_t* ie_attrib = pIeCtx->ie_attrib;

    attr->opMode = ie_attrib->opMode;
    attr->en = ie_attrib->en;
    attr->bypass = ie_attrib->bypass;
    memcpy(&attr->stAuto, &ie_attrib->stAuto, sizeof(ie_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_AIE_V10
XCamReturn IeSelectParam
(
    ie_param_auto_t *pAuto,
    ie_param_t* out)
{
    if(pAuto == NULL || out == NULL) {
        LOGE_AIE("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    ie_param_auto_t *paut = pAuto;

    return XCAM_RETURN_NO_ERROR;
}

#endif

#define RKISP_ALGO_IE_VERSION     "v0.0.9"
#define RKISP_ALGO_IE_VENDOR      "Rockchip"
#define RKISP_ALGO_IE_DESCRIPTION "Rockchip ie algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescIe = {
    .common = {
        .version = RKISP_ALGO_IE_VERSION,
        .vendor  = RKISP_ALGO_IE_VENDOR,
        .description = RKISP_ALGO_IE_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AIE,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

