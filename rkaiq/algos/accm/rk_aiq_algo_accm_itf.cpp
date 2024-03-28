/*
 * rk_aiq_algo_accm_itf.c
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

#include "accm/rk_aiq_algo_accm_itf.h"

#include "accm/rk_aiq_accm_algo.h"
#include "rk_aiq_algo_types.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ACCM( "%s: create ccm context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ret = AccmInit(&ctx->accm_para, cfg->calibv2);

    *context = ctx;

    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return ret;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);

    AccmRelease((accm_handle_t)context->accm_para);
    delete context;
    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);
    accm_handle_t hAccm = (accm_handle_t)(params->ctx->accm_para);
    //RkAiqAlgoConfigAccmInt *para = (RkAiqAlgoConfigAccmInt*)params;
    hAccm->accmSwInfo.prepare_type = params->u.prepare.conf_type;
    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)){
        RkAiqAlgoConfigAccm* confPara = (RkAiqAlgoConfigAccm*)params;
#if RKAIQ_HAVE_CCM_V1
        hAccm->ccm_v1 = (CalibDbV2_Ccm_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR(
            confPara->com.u.prepare.calibv2, ccm_calib));
#endif

#if RKAIQ_HAVE_CCM_V2
        hAccm->ccm_v2 = (CalibDbV2_Ccm_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR(
            confPara->com.u.prepare.calibv2, ccm_calib_v2));
#endif

#if RKAIQ_HAVE_CCM_V3
        hAccm->ccm_v3 = (CalibDbV2_Ccm_Para_V39_t*)(CALIBDBV2_GET_MODULE_PTR(
            confPara->com.u.prepare.calibv2, ccm_calib_v2));
#endif
    }
    // just update calib ptr
    if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
#if (RKAIQ_HAVE_CCM_V1 || RKAIQ_HAVE_CCM_V2 || RKAIQ_HAVE_CCM_V3)
#if RKAIQ_HAVE_CCM_V1
        CalibDbV2_Ccm_Para_V2_t* pCcm = hAccm->ccm_v1;
#elif RKAIQ_HAVE_CCM_V2
        CalibDbV2_Ccm_Para_V32_t* pCcm = hAccm->ccm_v2;
#elif RKAIQ_HAVE_CCM_V3
        CalibDbV2_Ccm_Para_V39_t* pCcm = hAccm->ccm_v3;
#endif
        pCcmMatrixAll_init(pCcm->TuningPara.aCcmCof,
                           pCcm->TuningPara.aCcmCof_len,
                           pCcm->TuningPara.matrixAll,
                           pCcm->TuningPara.matrixAll_len,
                           hAccm->pCcmMatrixAll);
#endif

        return XCAM_RETURN_NO_ERROR;
    }


    AccmPrepare((accm_handle_t)(params->ctx->accm_para));

    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);
    AccmPreProc((accm_handle_t)(inparams->ctx->accm_para));

    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);

    RkAiqAlgoProcAccm *procAccm = (RkAiqAlgoProcAccm*)inparams;
    RkAiqAlgoProcResAccm *proResAccm = (RkAiqAlgoProcResAccm*)outparams;
    accm_handle_t hAccm = (accm_handle_t)(inparams->ctx->accm_para);

    hAccm->isReCal_ = hAccm->isReCal_ ||
                    (procAccm->accm_sw_info.grayMode != procAccm->com.u.proc.gray_mode);
    procAccm->accm_sw_info.grayMode = procAccm->com.u.proc.gray_mode;
    procAccm->accm_sw_info.ccmConverged = hAccm->accmSwInfo.ccmConverged;
    hAccm->accmSwInfo = procAccm->accm_sw_info;
    //LOGI_ACCM( "%s accm_proc_com.u.init:%d \n", __FUNCTION__, inparams->u.proc.init);
    LOGD_ACCM( "%s: awbIIRDampCoef:%f\n", __FUNCTION__, hAccm->accmSwInfo.awbIIRDampCoef);

    AccmConfig(hAccm);
    proResAccm->res_com.cfg_update = hAccm->isReCal_;
#if defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    if (proResAccm->res_com.cfg_update)
        memcpy(proResAccm->accm_hw_conf_v2, &hAccm->ccmHwConf_v2, sizeof(rk_aiq_ccm_cfg_v2_t));
#else
    if (proResAccm->res_com.cfg_update)
        memcpy(proResAccm->accm_hw_conf, &hAccm->ccmHwConf, sizeof(rk_aiq_ccm_cfg_t));
#endif
    hAccm->isReCal_ = false;
    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);

    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;

}

RkAiqAlgoDescription g_RkIspAlgoDescAccm = {
    .common = {
        .version = RKISP_ALGO_ACCM_VERSION,
        .vendor  = RKISP_ALGO_ACCM_VENDOR,
        .description = RKISP_ALGO_ACCM_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACCM,
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
