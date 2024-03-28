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
#include "rk_aiq_alsc_convert_otp.h"

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
    alsc_handle_t hAlsc = (alsc_handle_t)ctx->alsc_para;
    hAlsc->eState = ALSC_STATE_INITIALIZED;
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
    hAlsc->cur_res.width = para->com.u.prepare.sns_op_width;
    hAlsc->cur_res.height = para->com.u.prepare.sns_op_height;

    hAlsc->alscSwInfo.prepare_type = params->u.prepare.conf_type;
    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        hAlsc->calibLscV2 =
            (CalibDbV2_LSC_t*)(CALIBDBV2_GET_MODULE_PTR(para->com.u.prepare.calibv2, lsc_v2));
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return UpdateLscCalibPtr(hAlsc);
        }
    }

    if((para->alsc_sw_info.otpInfo.flag && !hAlsc->otpGrad.flag) || \
       !!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) || \
       !!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGECAMS)) {
        alscGetOtpInfo(params);
        adjustVignettingForLscOTP(hAlsc->otpGrad.lsc_b, hAlsc->otpGrad.lsc_gb, \
                hAlsc->otpGrad.lsc_gr, hAlsc->otpGrad.lsc_r, \
                20, hAlsc->cur_res.width, hAlsc->cur_res.height);

        LOGD_ALSC( "adjustVignettingForLscOTP r[0:4]:%d,%d,%d,%d,%d, gr[0:4]:%d,%d,%d,%d,%d, gb[0:4]:%d,%d,%d,%d,%d, b[0:4]:%d,%d,%d,%d,%d\n",
                hAlsc->otpGrad.lsc_r[0],
                hAlsc->otpGrad.lsc_r[1],
                hAlsc->otpGrad.lsc_r[2],
                hAlsc->otpGrad.lsc_r[3],
                hAlsc->otpGrad.lsc_r[4],
                hAlsc->otpGrad.lsc_gr[0],
                hAlsc->otpGrad.lsc_gr[1],
                hAlsc->otpGrad.lsc_gr[2],
                hAlsc->otpGrad.lsc_gr[3],
                hAlsc->otpGrad.lsc_gr[4],
                hAlsc->otpGrad.lsc_gb[0],
                hAlsc->otpGrad.lsc_gb[1],
                hAlsc->otpGrad.lsc_gb[2],
                hAlsc->otpGrad.lsc_gb[3],
                hAlsc->otpGrad.lsc_gb[4],
                hAlsc->otpGrad.lsc_b[0],
                hAlsc->otpGrad.lsc_b[1],
                hAlsc->otpGrad.lsc_b[2],
                hAlsc->otpGrad.lsc_b[3],
                hAlsc->otpGrad.lsc_b[4]);
        convertSensorLscOTP(&hAlsc->cur_res, &hAlsc->otpGrad, para->alsc_sw_info.bayerPattern);
    }

    AlscPrepare((alsc_handle_t)(params->ctx->alsc_para));
    params->ctx->alsc_para->isReCal_ = true;

    hAlsc->eState = ALSC_STATE_STOPPED;
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
    LOGD_ALSC( "=============== lsc count:%d =============\n", hAlsc->count);
    LOGD_ALSC( "%s: sensorGain:%f, awbGain:%f,%f, resName:%s, awbIIRDampCoef:%f\n", __FUNCTION__,
               hAlsc->alscSwInfo.sensorGain,
               hAlsc->alscSwInfo.awbGain[0], hAlsc->alscSwInfo.awbGain[1],
               hAlsc->cur_res.name, hAlsc->alscSwInfo.awbIIRDampCoef);

    AlscConfig(hAlsc);
    if (hAlsc->isReCal_) {
        memcpy(proResAlsc->alsc_hw_conf, &hAlsc->lscHwConf, sizeof(rk_aiq_lsc_cfg_t));
        outparams->cfg_update = true;
        hAlsc->isReCal_ = false;
    } else {
        outparams->cfg_update = false;
    }
#if 0
    if (procAlsc->tx != nullptr) {
        XCamVideoBuffer* txBuf = procAlsc->tx;
        /*
         * If the subsequent flow also needs the data from the previous frame,
         * it will need to add a reference
         */
        txBuf->ref(txBuf);
        LOGD_ALSC("tx buf fd is: %d", txBuf->get_fd(txBuf));
        // uint8_t *virTxBuf = txBuf->map(txBuf);
        txBuf->unref(txBuf);
    }
#endif

    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    hAlsc->eState = ALSC_STATE_RUNNING;
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
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
