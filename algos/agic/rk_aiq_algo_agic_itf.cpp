/*
 * rk_aiq_algo_agic_itf.c
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

#include "rk_aiq_algo_types_int.h"
#include "agic/rk_aiq_algo_agic_itf.h"
#include "agic/rk_aiq_types_algo_agic_prvt.h"

RKAIQ_BEGIN_DECLARE


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_AGIC( "%s: create agic context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    LOGI_AGIC("%s: (enter)\n", __FUNCTION__ );
    AlgoCtxInstanceCfgInt* instanc_int = (AlgoCtxInstanceCfgInt*)cfg;
    CamCalibDbV2Context_t* calibv2 = instanc_int->calibv2;
    AgicInit(&ctx->agicCtx, calibv2);

    *context = ctx;
    LOGI_AGIC("%s(%d): Exit!\n", __FUNCTION__, __LINE__);
    return result;

}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AgicContext_t* pAgicCtx = (AgicContext_t*)&context->agicCtx;
    AgicRelease(pAgicCtx);
    delete context;

    LOGI_AGIC("%s(%d): Exit!\n", __FUNCTION__, __LINE__);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AgicContext_t* pAgicCtx = (AgicContext_t *)&params->ctx->agicCtx;
    RkAiqAlgoConfigAgicInt* pCfgParam = (RkAiqAlgoConfigAgicInt*)params;
    CamCalibDbV2Context_t* calibv2 = pCfgParam->rk_com.u.prepare.calibv2;



    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGD_AGIC("%s: Agic Reload Para!\n", __FUNCTION__);

        if(CHECK_ISP_HW_V20()) {
            CalibDbV2_Gic_V20_t* calibv2_agic_calib_V20 =
                (CalibDbV2_Gic_V20_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, agic_calib_v20));
            memcpy(pAgicCtx->full_param.gic_v20, calibv2_agic_calib_V20, sizeof(CalibDbV2_Gic_V20_t));
        } else if(CHECK_ISP_HW_V21()) {
            CalibDbV2_Gic_V21_t* calibv2_agic_calib_V21 =
                (CalibDbV2_Gic_V21_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, agic_calib_v21));
            memcpy(pAgicCtx->full_param.gic_v21, calibv2_agic_calib_V21, sizeof(CalibDbV2_Gic_V21_t));
        }

        pAgicCtx->calib_changed = true;

    }

    pAgicCtx->working_mode = pCfgParam->agic_config_com.com.u.prepare.working_mode;

    LOGI_AGIC("%s(%d): Exit!\n", __FUNCTION__, __LINE__);
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    RkAiqAlgoConfigAgicInt* config = (RkAiqAlgoConfigAgicInt*)inparams;
    RkAiqAlgoPreResAgicInt* pAgicPreResParams = (RkAiqAlgoPreResAgicInt*)outparams;
    AgicContext_t* pAgicCtx = (AgicContext_t *)&inparams->ctx->agicCtx;

    if (config->rk_com.u.proc.gray_mode)
        pAgicCtx->Gic_Scene_mode = GIC_NIGHT;
    else if (GIC_NORMAL == pAgicCtx->working_mode)
        pAgicCtx->Gic_Scene_mode = GIC_NORMAL;
    else
        pAgicCtx->Gic_Scene_mode = GIC_HDR;

    LOGI_AGIC("%s(%d): Exit!\n", __FUNCTION__, __LINE__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOGI_AGIC("%s(%d): enter!\n", __FUNCTION__, __LINE__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int iso = 50;
    RkAiqAlgoProcAgicInt* pAgicProcParams = (RkAiqAlgoProcAgicInt*)inparams;
    RkAiqAlgoProcResAgicInt* pAgicProcResParams = (RkAiqAlgoProcResAgicInt*)outparams;
    AgicContext_t* pAgicCtx = (AgicContext_t *)&inparams->ctx->agicCtx;
    RKAiqAecExpInfo_t* aeCurExp = pAgicProcParams->rk_com.u.proc.curExp;

    if(aeCurExp != NULL) {
        if(pAgicProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            iso = aeCurExp->LinearExp.exp_real_params.analog_gain * 50;
            LOGD_AGIC("%s:NORMAL:iso=%d,again=%f\n", __FUNCTION__, iso,
                      aeCurExp->LinearExp.exp_real_params.analog_gain);
        } else if(RK_AIQ_HDR_GET_WORKING_MODE(pAgicProcParams->hdr_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            iso = aeCurExp->HdrExp[1].exp_real_params.analog_gain * 50;
            LOGD_AGIC("%s:HDR2:iso=%d,again=%f\n", __FUNCTION__, iso,
                      aeCurExp->HdrExp[1].exp_real_params.analog_gain);
        } else if(RK_AIQ_HDR_GET_WORKING_MODE(pAgicProcParams->hdr_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
            iso = aeCurExp->HdrExp[2].exp_real_params.analog_gain * 50;
            LOGD_AGIC("%s:HDR3:iso=%d,again=%f\n", __FUNCTION__, iso,
                      aeCurExp->HdrExp[2].exp_real_params.analog_gain);
        }
    } else {
        LOGE_AGIC("%s: pAEPreRes is NULL, so use default instead \n", __FUNCTION__);
    }

    pAgicCtx->Gic_Scene_mode = 0;
    if (pAgicCtx->last_iso != iso || pAgicCtx->calib_changed) {
        AgicProcess(pAgicCtx, iso, pAgicCtx->Gic_Scene_mode);
        AgicGetProcResult(pAgicCtx);
        pAgicCtx->calib_changed = false;
        pAgicCtx->ProcRes.gic_cfg_update = true;
    } else {
        pAgicCtx->ProcRes.gic_cfg_update = false;
    }

    memcpy(&pAgicProcResParams->gicRes, &pAgicCtx->ProcRes, sizeof(AgicProcResult_t));

    LOGI_AGIC("%s(%d): Exit!\n", __FUNCTION__, __LINE__);
    return result;

}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAgic = {
    .common = {
        .version = RKISP_ALGO_AGIC_VERSION,
        .vendor  = RKISP_ALGO_AGIC_VENDOR,
        .description = RKISP_ALGO_AGIC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGIC,
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
