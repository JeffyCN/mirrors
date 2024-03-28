/*
 * rk_aiq_algo_camgroup_agamma_itf.cpp
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

#include "rk_aiq_algo_camgroup_types.h"
#include "algos/agamma/rk_aiq_algo_agamma_itf.h"
#if RKAIQ_HAVE_GAMMA_V10
#include "agamma/rk_aiq_agamma_algo_v10.h"
#endif
#if RKAIQ_HAVE_GAMMA_V11
#include "agamma/rk_aiq_agamma_algo_v11.h"
#endif


RKAIQ_BEGIN_DECLARE


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* pAgammaGrpCtx = NULL;
    AlgoCtxInstanceCfgCamGroup* instanc_int = (AlgoCtxInstanceCfgCamGroup*)cfg;

    ret = AgammaInit(&pAgammaGrpCtx, (CamCalibDbV2Context_t*)(instanc_int->s_calibv2));

    *context = (RkAiqAlgoContext *)(pAgammaGrpCtx);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    AgammaHandle_t* pAgammaGrpCtx = (AgammaHandle_t*)context;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AgammaRelease(pAgammaGrpCtx);

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t * pAgammaGrpCtx = (AgammaHandle_t *)params->ctx;
    RkAiqAlgoCamGroupPrepare* pCfgParam = (RkAiqAlgoCamGroupPrepare*)params;

    if (!!(pCfgParam->gcom.com.u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
#if RKAIQ_HAVE_GAMMA_V10
        CalibDbV2_gamma_v10_t* calibv2_agamma_calib =
            (CalibDbV2_gamma_v10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCfgParam->s_calibv2),
                                                              agamma_calib));
        memcpy(&pAgammaGrpCtx->agammaAttrV10.stAuto, calibv2_agamma_calib,
               sizeof(CalibDbV2_gamma_v10_t));  // reload iq
#endif
#if RKAIQ_HAVE_GAMMA_V11
        CalibDbV2_gamma_v11_t* calibv2_agamma_calib =
            (CalibDbV2_gamma_v11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)(pCfgParam->s_calibv2),
                                                              agamma_calib));
        memcpy(&pAgammaGrpCtx->agammaAttrV11.stAuto, calibv2_agamma_calib,
               sizeof(CalibDbV2_gamma_v11_t));  // reload iq
#endif
        LOGI_AGAMMA("%s: Agamma Reload Para!!!\n", __FUNCTION__);
        pAgammaGrpCtx->ifReCalcStAuto = true;
    }

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AGAMMA("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AgammaHandle_t* pAgammaGrpCtx = (AgammaHandle_t *)inparams->ctx;
    pAgammaGrpCtx->FrameID                      = inparams->frame_id;
    RkAiqAlgoCamGroupProcOut* pAgammaGrpProcRes = (RkAiqAlgoCamGroupProcOut*)outparams;
    bool bypass                                 = true;

#if RKAIQ_HAVE_GAMMA_V10
    if (pAgammaGrpCtx->FrameID <= INIT_CALC_PARAMS_NUM || inparams->u.proc.init)
        bypass = false;
    else if (pAgammaGrpCtx->agammaAttrV10.mode != pAgammaGrpCtx->CurrApiMode)
        bypass = false;
    else if (pAgammaGrpCtx->agammaAttrV10.mode == RK_AIQ_GAMMA_MODE_MANUAL)
        bypass = !pAgammaGrpCtx->ifReCalcStManual;
    else if (pAgammaGrpCtx->agammaAttrV10.mode == RK_AIQ_GAMMA_MODE_AUTO)
        bypass = !pAgammaGrpCtx->ifReCalcStAuto;
#endif
#if RKAIQ_HAVE_GAMMA_V11
    if (pAgammaGrpCtx->FrameID <= INIT_CALC_PARAMS_NUM || inparams->u.proc.init)
        bypass = false;
    else if (pAgammaGrpCtx->agammaAttrV11.mode != pAgammaGrpCtx->CurrApiMode)
        bypass = false;
    else if (pAgammaGrpCtx->agammaAttrV11.mode == RK_AIQ_GAMMA_MODE_MANUAL)
        bypass = !pAgammaGrpCtx->ifReCalcStManual;
    else if (pAgammaGrpCtx->agammaAttrV11.mode == RK_AIQ_GAMMA_MODE_AUTO)
        bypass = !pAgammaGrpCtx->ifReCalcStAuto;
#endif

    if (!bypass)
        AgammaProcessing(pAgammaGrpCtx, pAgammaGrpProcRes->camgroupParmasArray[0]->_agammaConfig);

    // set proc res
    outparams->cfg_update = !bypass;
    IS_UPDATE_MEM((pAgammaGrpProcRes->camgroupParmasArray[0]->_agammaConfig), ((RkAiqAlgoCamGroupProcIn*)inparams)->_offset_is_update) =
        outparams->cfg_update;
    for (int i = 1; i < pAgammaGrpProcRes->arraySize; i++) {
        if (outparams->cfg_update) {
            memcpy(pAgammaGrpProcRes->camgroupParmasArray[i]->_agammaConfig,
                   pAgammaGrpProcRes->camgroupParmasArray[0]->_agammaConfig,
                   sizeof(AgammaProcRes_t));
        }
        IS_UPDATE_MEM((pAgammaGrpProcRes->camgroupParmasArray[i]->_agammaConfig), ((RkAiqAlgoCamGroupProcIn*)inparams)->_offset_is_update) =
            outparams->cfg_update;
    }

    if (pAgammaGrpCtx->ifReCalcStAuto) pAgammaGrpCtx->ifReCalcStAuto = false;
    if (pAgammaGrpCtx->ifReCalcStManual) pAgammaGrpCtx->ifReCalcStManual = false;

    LOG1_AGAMMA("EXIT: %s \n", __func__);
    return ret;
}

RkAiqAlgoDescription g_RkIspAlgoDescamgroupAgamma = {
    .common = {
        .version = RKISP_ALGO_AGAMMA_VERSION,
        .vendor  = RKISP_ALGO_AGAMMA_VENDOR,
        .description = RKISP_ALGO_AGAMMA_DESCRIPTION,
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

RKAIQ_END_DECLARE
