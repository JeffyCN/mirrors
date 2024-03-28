/*
 * rk_aiq_algo_camgroup_blc_itf.cpp
 *
 *  Copyright (c) 2021 Rockchip Corporation
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

#include "rk_aiq_algo_camgroup_types.h"
#include "misc/rk_aiq_algo_camgroup_misc_itf.h"
#include "newStruct/blc/blc_types_prvt.h"


RKAIQ_BEGIN_DECLARE


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );

    BlcContext_t* ctx = new BlcContext_t();

    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->blc_attrib =
        (blc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)(cfgInt->s_calibv2), blc));

    *context = (RkAiqAlgoContext *)(ctx);

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );

#if 1
    BlcContext_t* pBlcCtx = (BlcContext_t*)context;
    delete pBlcCtx;
#endif

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ABLC("%s: (enter)\n", __FUNCTION__ );
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    BlcContext_t* pBlcCtx = (BlcContext_t *)params->ctx;
    RkAiqAlgoConfigAblc* pCfgParam = (RkAiqAlgoConfigAblc*)params;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pBlcCtx->blc_attrib =
                (blc_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, blc));
            pBlcCtx->isReCal_ = true;
        }
    }

    LOG1_ABLC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int delta_iso = 0;
    LOGI_ABLC("%s: (enter)\n", __FUNCTION__ );

    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    BlcContext_t* pBlcCtx = (BlcContext_t *)inparams->ctx;
    blc_api_attrib_t* blc_attrib = pBlcCtx->blc_attrib;
    rk_aiq_isp_blc_params_t *blc_param = procResParaGroup->camgroupParmasArray[0]->ablc.blc;

    if (procParaGroup->gcom.com.u.proc.is_attrib_update) {
        pBlcCtx->isReCal_ = true;
    }

    int iso = inparams->u.proc.iso;
    bool init = inparams->u.proc.init;
    delta_iso = abs(iso - pBlcCtx->iso);

    if(delta_iso > ABLC_RECALCULATE_DELTE_ISO) {
        pBlcCtx->isReCal_ = true;
    }

    if (pBlcCtx->isReCal_) {
        BlcSelectParam(pBlcCtx, &blc_param->result, iso);
        blc_param->is_update = true;
        blc_param->en = blc_attrib->en;
        blc_param->bypass = blc_attrib->bypass;
        LOGD_ABLC("%s:%d processing ABLC recalculate delta_iso:%d \n", __FUNCTION__, __LINE__, delta_iso);

        for (int i = 1; i < procResParaGroup->arraySize; i++) {
            if (pBlcCtx->isReCal_) {
                procResParaGroup->camgroupParmasArray[i]->ablc.blc->result = blc_param->result;
                procResParaGroup->camgroupParmasArray[i]->ablc.blc->en = blc_param->en;
                procResParaGroup->camgroupParmasArray[i]->ablc.blc->is_update = true;
                procResParaGroup->camgroupParmasArray[i]->ablc.blc->bypass = blc_param->bypass;
            }
        }

    } else {
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            procResParaGroup->camgroupParmasArray[i]->ablc.blc->is_update = false;
        }
    }

    pBlcCtx->isReCal_ = 0;

    LOGI_ABLC("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_CAMGROUP_ALGO_BLC_VERSION     "v0.0.1"
#define RKISP_CAMGROUP_ALGO_BLC_VENDOR      "Rockchip"
#define RKISP_CAMGROUP_ALGO_BLC_DESCRIPTION "Rockchip blc algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupBlc = {
    .common = {
        .version = RKISP_CAMGROUP_ALGO_BLC_VERSION,
        .vendor  = RKISP_CAMGROUP_ALGO_BLC_VENDOR,
        .description = RKISP_CAMGROUP_ALGO_BLC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ABLC,
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
