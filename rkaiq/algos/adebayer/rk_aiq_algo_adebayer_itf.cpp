/*
 * rk_aiq_algo_debayer_itf.c
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

#include "adebayer/rk_aiq_algo_adebayer_itf.h"
#if RKAIQ_HAVE_DEBAYER_V1
#include "adebayer/rk_aiq_adebayer_algo_v1.h"
#endif
#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE
#include "adebayer/rk_aiq_adebayer_algo_v2.h"
#endif
#if RKAIQ_HAVE_DEBAYER_V3
#include "adebayer/rk_aiq_adebayer_algo_v3.h"
#endif
#include "rk_aiq_algo_types.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context
(
    RkAiqAlgoContext **context,
    const AlgoCtxInstanceCfg* cfg
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ADEBAYER( "%s: create adebayer context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    LOGV_ADEBAYER("%s: (enter)\n", __FUNCTION__ );

    //hw version check

#if RKAIQ_HAVE_DEBAYER_V1
    if(cfg->module_hw_version != ADEBAYER_HARDWARE_V1)
        LOGE_ADEBAYER("%s: wrong HW version(%d)", __FUNCTION__, cfg->module_hw_version);
#endif

#if RKAIQ_HAVE_DEBAYER_V2
    if(cfg->module_hw_version != ADEBAYER_HARDWARE_V2)
        LOGE_ADEBAYER("%s: wrong HW version(%d)", __FUNCTION__, cfg->module_hw_version);
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
    if(cfg->module_hw_version != ADEBAYER_HARDWARE_V2_LITE)
        LOGE_ADEBAYER("%s: wrong HW version(%d)", __FUNCTION__, cfg->module_hw_version);
#endif

#if RKAIQ_HAVE_DEBAYER_V3
    if(cfg->module_hw_version != ADEBAYER_HARDWARE_V3)
        LOGE_ADEBAYER("%s: wrong HW version(%d)", __FUNCTION__, cfg->module_hw_version);
#endif

    //copy params from calib
    AdebayerInit(&ctx->adebayerCtx, cfg->calib, cfg->calibv2);

    *context = ctx;
    LOGV_ADEBAYER("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context
(
    RkAiqAlgoContext *context
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGV_ADEBAYER("%s: (enter)\n", __FUNCTION__ );

    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t*)&context->adebayerCtx;
    AdebayerRelease(pAdebayerCtx);

    delete context;
    LOGV_ADEBAYER("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
prepare
(
    RkAiqAlgoCom* params
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGV_ADEBAYER("%s: (enter)\n", __FUNCTION__ );
    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t *)&params->ctx->adebayerCtx;
    RkAiqAlgoConfigAdebayer* pCfgParam = (RkAiqAlgoConfigAdebayer*)params;

    //reconfig
    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;

#if RKAIQ_HAVE_DEBAYER_V1
        result = AdebayerFullParamsInit(pAdebayerCtx, pCfgParam->com.u.prepare.calib, pCfgParam->com.u.prepare.calibv2);
#endif

#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE || RKAIQ_HAVE_DEBAYER_V3
        result = AdebayerCalibConfig(pAdebayerCtx, pCfgParam->com.u.prepare.calibv2);

#if RKAIQ_HAVE_DEBAYER_V3
        pAdebayerCtx->compr_bit = pCfgParam->compr_bit;
#endif

#endif

        pAdebayerCtx->config.updatecfg = true;
    }

    AdebayerStart(pAdebayerCtx);
    LOGV_ADEBAYER("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
pre_process
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    LOGV_ADEBAYER("%s: (enter)\n", __FUNCTION__ );

    LOGV_ADEBAYER("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int iso = 50;

    RkAiqAlgoProcAdebayer* pAdebayerProcParams = (RkAiqAlgoProcAdebayer*)inparams;
    RkAiqAlgoProcResAdebayer* pAdebayerProcResParams = (RkAiqAlgoProcResAdebayer*)outparams;
    AdebayerContext_t* pAdebayerCtx = (AdebayerContext_t *)&inparams->ctx->adebayerCtx;

    LOGV_ADEBAYER("%s: (enter)\n", __FUNCTION__ );

    if (pAdebayerProcParams->com.u.proc.is_bw_sensor) {
        pAdebayerCtx->config.enable = 0;
        if (pAdebayerProcParams->com.u.proc.init)
            pAdebayerCtx->config.updatecfg = true;
    } else {
        //get ISO from curExp
        RKAiqAecExpInfo_t *curExp = pAdebayerProcParams->com.u.proc.curExp;
        if(curExp != NULL) {
            if(pAdebayerProcParams->hdr_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                iso = curExp->LinearExp.exp_real_params.analog_gain *
                      curExp->LinearExp.exp_real_params.digital_gain *
                      curExp->LinearExp.exp_real_params.isp_dgain * 50;
                LOGD_ADEBAYER("%s:Frmid:%d,NORMAL:iso=%d,again=%f\n", __FUNCTION__, inparams->frame_id, iso,
                              curExp->LinearExp.exp_real_params.analog_gain);
            } else if(RK_AIQ_HDR_GET_WORKING_MODE(pAdebayerProcParams->hdr_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
                iso = curExp->HdrExp[1].exp_real_params.analog_gain *
                      curExp->HdrExp[1].exp_real_params.digital_gain *
                      curExp->HdrExp[1].exp_real_params.isp_dgain * 50;
                LOGD_ADEBAYER("%s:Frmid:%d,HDR2:iso=%d,again=%f\n", __FUNCTION__, inparams->frame_id,  iso,
                              curExp->HdrExp[1].exp_real_params.analog_gain);
            } else if(RK_AIQ_HDR_GET_WORKING_MODE(pAdebayerProcParams->hdr_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
                iso = curExp->HdrExp[2].exp_real_params.analog_gain *
                      curExp->HdrExp[2].exp_real_params.digital_gain *
                      curExp->HdrExp[2].exp_real_params.isp_dgain * 50;
                LOGD_ADEBAYER("%s:Frmid:%d,HDR3:iso=%d,again=%f\n", __FUNCTION__, inparams->frame_id,  iso,
                              curExp->HdrExp[2].exp_real_params.analog_gain);
            }
        } else {
            LOGE_ADEBAYER("%s: curExp is NULL, so use default instead \n", __FUNCTION__);
        }

        if (iso != pAdebayerCtx->iso) {
            pAdebayerCtx->iso = iso;
            pAdebayerCtx->config.updatecfg = true;
        }

        if (pAdebayerCtx->is_reconfig) {
            LOGD_ADEBAYER("%s: is_reconfig = true \n", __FUNCTION__);
            pAdebayerCtx->config.updatecfg = true;
            pAdebayerCtx->is_reconfig = false;
        }

        if (pAdebayerCtx->config.updatecfg) {
            AdebayerProcess(pAdebayerCtx, iso);
        }
    }

    if (pAdebayerCtx->config.updatecfg) {
#if RKAIQ_HAVE_DEBAYER_V1
        AdebayerGetProcResult(pAdebayerCtx, &pAdebayerProcResParams->debayerResV1);
#endif

#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE
        AdebayerGetProcResult(pAdebayerCtx, &pAdebayerProcResParams->debayerResV2);
#endif

#if RKAIQ_HAVE_DEBAYER_V3
        AdebayerGetProcResult(pAdebayerCtx, &pAdebayerProcResParams->debayerResV3);
#endif
        outparams->cfg_update = true;
    } else {
        outparams->cfg_update = false;
    }

    LOGV_ADEBAYER("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    LOGV_ADEBAYER("%s: (enter)\n", __FUNCTION__ );

    LOGV_ADEBAYER("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAdebayer = {
    .common = {
        .version = RKISP_ALGO_ADEBAYER_VERSION,
        .vendor  = RKISP_ALGO_ADEBAYER_VENDOR,
        .description = RKISP_ALGO_ADEBAYER_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADEBAYER,
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
