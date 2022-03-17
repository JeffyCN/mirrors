/*
 * rk_aiq_algo_alsc_itf.c
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

#include "alsc/rk_aiq_algo_alsc_itf.h"
#include "alsc/rk_aiq_alsc_algo.h"
#include "rk_aiq_algo_types.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE



static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ALSC( "%s: create alsc context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    AlscInit(&ctx->alsc_para, cfg->calibv2);
    *context = ctx;
    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);

    AlscRelease((alsc_handle_t)context->alsc_para);
    delete context;
    context = NULL;
    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);
    alsc_handle_t hAlsc = (alsc_handle_t)(params->ctx->alsc_para);

    RkAiqAlgoConfigAlsc *para = (RkAiqAlgoConfigAlsc *)params;

    sprintf(hAlsc->cur_res.name, "%dx%d", para->com.u.prepare.sns_op_width,
            para->com.u.prepare.sns_op_height );
    hAlsc->alscSwInfo.prepare_type = params->u.prepare.conf_type;
   if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
       hAlsc->calibLscV2 =
            (CalibDbV2_LSC_t*)(CALIBDBV2_GET_MODULE_PTR(para->com.u.prepare.calibv2, lsc_v2));
   }
    AlscPrepare((alsc_handle_t)(params->ctx->alsc_para));

    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);

    AlscPreProc((alsc_handle_t)(inparams->ctx->alsc_para));

    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);

    RkAiqAlgoProcAlsc *procAlsc = (RkAiqAlgoProcAlsc*)inparams;
    RkAiqAlgoProcResAlsc *proResAlsc = (RkAiqAlgoProcResAlsc*)outparams;
    alsc_handle_t hAlsc = (alsc_handle_t)(inparams->ctx->alsc_para);
    RkAiqAlgoProcAlsc* procPara = (RkAiqAlgoProcAlsc*)inparams;
    procAlsc->alsc_sw_info.grayMode = procPara->com.u.proc.gray_mode;
    hAlsc->alscSwInfo = procAlsc->alsc_sw_info;
    //LOGI_ALSC( "%s alsc_proc_com.u.init:%d \n", __FUNCTION__, inparams->u.proc.init);
    LOGD_ALSC( "%s: sensorGain:%f, awbGain:%f,%f, resName:%s, awbIIRDampCoef:%f\n", __FUNCTION__,
               hAlsc->alscSwInfo.sensorGain,
               hAlsc->alscSwInfo.awbGain[0], hAlsc->alscSwInfo.awbGain[1],
               hAlsc->cur_res.name, hAlsc->alscSwInfo.awbIIRDampCoef);

    AlscConfig(hAlsc);
    memcpy(&proResAlsc->alsc_hw_conf, &hAlsc->lscHwConf, sizeof(rk_aiq_lsc_cfg_t));

#if 1
    if (procAlsc->tx != nullptr) {
        XCamVideoBuffer* txBuf = procAlsc->tx;
        /*
         * If the subsequent flow also needs the data from the previous frame,
         * it will need to add a reference
         */
        txBuf->ref(txBuf);
        LOGD_ALSC("tx buf fd is: %d", txBuf->get_fd(txBuf));
        uint8_t *virTxBuf = txBuf->map(txBuf);
        txBuf->unref(txBuf);
    }
#endif

    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);

    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;

}

RkAiqAlgoDescription g_RkIspAlgoDescAlsc = {
    .common = {
        .version = RKISP_ALGO_ALSC_VERSION,
        .vendor  = RKISP_ALGO_ALSC_VENDOR,
        .description = RKISP_ALGO_ALSC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ALSC,
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
