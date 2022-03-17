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

#include "rk_aiq_algo_types_int.h"
#include "accm/rk_aiq_algo_accm_itf.h"
#include "accm/rk_aiq_accm_algo.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);
    AlgoCtxInstanceCfgInt *cfgInt = (AlgoCtxInstanceCfgInt*)cfg;
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ACCM( "%s: create ccm context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
        AccmInit(&ctx->accm_para, cfgInt->calibv2);

    *context = ctx;

    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
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
        RkAiqAlgoConfigAccmInt* confPara = (RkAiqAlgoConfigAccmInt*)params;

        hAccm->calibV2Ccm =
                (CalibDbV2_Ccm_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR(confPara->rk_com.u.prepare.calibv2, ccm_calib));
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

    RkAiqAlgoProcAccmInt *procAccm = (RkAiqAlgoProcAccmInt*)inparams;
    RkAiqAlgoProcResAccmInt *proResAccm = (RkAiqAlgoProcResAccmInt*)outparams;
    accm_handle_t hAccm = (accm_handle_t)(inparams->ctx->accm_para);
    RkAiqAlgoProcAccmInt* procPara = (RkAiqAlgoProcAccmInt*)inparams;
    // cal gain diff, whether converge?
    float d_gain = 0;
    d_gain = fabs(procAccm->accm_sw_info.sensorGain - hAccm->accmSwInfo.sensorGain);
    float d_wbgain = 0;
    d_wbgain =sqrt( (procAccm->accm_sw_info.awbGain[0]-hAccm->accmSwInfo.awbGain[0])*(procAccm->accm_sw_info.awbGain[0]-hAccm->accmSwInfo.awbGain[0])
                                     + (procAccm->accm_sw_info.awbGain[1]-hAccm->accmSwInfo.awbGain[1])*(procAccm->accm_sw_info.awbGain[1]-hAccm->accmSwInfo.awbGain[1]));
    if ((d_wbgain < hAccm->calibV2Ccm->control.wbgain_tolerance)&&(d_gain < hAccm->calibV2Ccm->control.gain_tolerance)
                   && (!hAccm->calib_update))
                   hAccm->update = false;
    else
        hAccm->update = true;
    hAccm->calib_update = false;

    procAccm->accm_sw_info.grayMode = procPara->rk_com.u.proc.gray_mode;
    procAccm->accm_sw_info.ccmConverged = hAccm->accmSwInfo.ccmConverged;
    hAccm->accmSwInfo = procAccm->accm_sw_info;
    //LOGI_ACCM( "%s accm_proc_com.u.init:%d \n", __FUNCTION__, inparams->u.proc.init);
    LOGD_ACCM( "%s: sensorGain:%f, awbGain:%f,%f,  awbIIRDampCoef:%f\n", __FUNCTION__,
               hAccm->accmSwInfo.sensorGain,
               hAccm->accmSwInfo.awbGain[0], hAccm->accmSwInfo.awbGain[1],
               hAccm->accmSwInfo.awbIIRDampCoef);

    AccmConfig(hAccm);
    memcpy(&proResAccm->accm_proc_res_com.accm_hw_conf, &hAccm->ccmHwConf, sizeof(rk_aiq_ccm_cfg_t));
    proResAccm->accm_proc_res_com.ccm_update = hAccm->update ||hAccm->updateAtt || hAccm->accmSwInfo.ccmConverged;
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
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE
