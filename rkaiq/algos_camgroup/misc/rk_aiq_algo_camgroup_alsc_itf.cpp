/*
 * rk_aiq_algo_camgroup_alsc_itf.c
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
#include "alsc/rk_aiq_algo_alsc_itf.h"
#include "alsc/rk_aiq_alsc_algo.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ALSC( "%s: create camgroup alsc context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    AlscInit(&ctx->alsc_para, cfgInt->s_calibv2);
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

    RkAiqAlgoCamGroupPrepare* para = (RkAiqAlgoCamGroupPrepare*)params;

    sprintf(hAlsc->cur_res.name, "%dx%d", para->gcom.com.u.prepare.sns_op_width,
            para->gcom.com.u.prepare.sns_op_height );
    hAlsc->alscSwInfo.prepare_type = params->u.prepare.conf_type;
   if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
       hAlsc->calibLscV2 =
            (CalibDbV2_LSC_t*)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)(para->s_calibv2), lsc_v2));
   }
    AlscPrepare((alsc_handle_t)(params->ctx->alsc_para));

    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ALSC( "%s: (enter)\n", __FUNCTION__);

    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    alsc_handle_t hAlsc = (alsc_handle_t)(inparams->ctx->alsc_para);

    hAlsc->alscSwInfo.grayMode = procParaGroup->_gray_mode;

    if (!procParaGroup->arraySize) {
        LOG1_ALSC( "%s: do nothing \n", __FUNCTION__);
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    RkAiqAlgoProcResAwbShared_t* awb_proc_res = &scam_3a_res->awb._awbProcRes;
    if (awb_proc_res) {
        RkAiqAlgoProcResAwbShared_t* awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res;
        if(awb_res) {
            if(awb_res->awb_gain_algo.grgain < DIVMIN ||
                    awb_res->awb_gain_algo.gbgain < DIVMIN ) {
                LOGW("get wrong awb gain from AWB module ,use default value ");
            } else {
                hAlsc->alscSwInfo.awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                hAlsc->alscSwInfo.awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            hAlsc->alscSwInfo.awbIIRDampCoef = awb_res->awb_smooth_factor;
            hAlsc->alscSwInfo.varianceLuma = awb_res->varianceLuma;
            hAlsc->alscSwInfo.awbConverged = awb_res->awbConverged;
        } else {
            LOGW("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGW("fail to get awb gain form AWB module,use default value ");
    }

    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            hAlsc->alscSwInfo.sensorGain = pCurExp->LinearExp.exp_real_params.analog_gain
                    * pCurExp->LinearExp.exp_real_params.digital_gain
                    * pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)procParaGroup->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from second hdr frame for alsc");
            hAlsc->alscSwInfo.sensorGain = pCurExp->HdrExp[1].exp_real_params.analog_gain
                    * pCurExp->HdrExp[1].exp_real_params.digital_gain
                    * pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD("sensor gain choose from third hdr frame for alsc");
            hAlsc->alscSwInfo.sensorGain = pCurExp->HdrExp[2].exp_real_params.analog_gain
                    * pCurExp->HdrExp[2].exp_real_params.digital_gain
                    * pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGW("working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default value ",
                 procParaGroup->working_mode);
        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }

    //LOGI_ALSC( "%s alsc_proc_com.u.init:%d \n", __FUNCTION__, inparams->u.proc.init);
    LOGD_ALSC( "%s: sensorGain:%f, awbGain:%f,%f, resName:%s, awbIIRDampCoef:%f\n", __FUNCTION__,
               hAlsc->alscSwInfo.sensorGain,
               hAlsc->alscSwInfo.awbGain[0], hAlsc->alscSwInfo.awbGain[1],
               hAlsc->cur_res.name, hAlsc->alscSwInfo.awbIIRDampCoef);

    AlscConfig(hAlsc);

    for (int i = 0; i < procResParaGroup->arraySize; i++) {
        if (hAlsc->isReCal_) {
            *(procResParaGroup->camgroupParmasArray[i]->_lscConfig) =
                hAlsc->lscHwConf;
            outparams->cfg_update = true;
        } else {
            outparams->cfg_update = false;
        }
        // TODO
        IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->_lscConfig), procParaGroup->_offset_is_update) =
            outparams->cfg_update;
    }
    hAlsc->isReCal_ = false;

    LOG1_ALSC( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAlsc = {
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
