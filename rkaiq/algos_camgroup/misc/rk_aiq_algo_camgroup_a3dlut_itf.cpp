/*
 * rk_aiq_algo_camgroup_a3dlut_itf.c
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
#include "a3dlut/rk_aiq_algo_a3dlut_itf.h"
#include "a3dlut/rk_aiq_a3dlut_algo.h"
#include "xcam_log.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_A3DLUT( "%s: create camgroup a3dlut context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    Alut3dInit(&ctx->a3dlut_para, cfgInt->s_calibv2);
    *context = ctx;
    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);

    Alut3dRelease((alut3d_handle_t)context->a3dlut_para);
    delete context;
    context = NULL;
    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);
    alut3d_handle_t hAlut3d = (alut3d_handle_t)(params->ctx->a3dlut_para);

    RkAiqAlgoCamGroupPrepare *para = (RkAiqAlgoCamGroupPrepare *)params;
    hAlut3d->prepare_type = params->u.prepare.conf_type;
   if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
       hAlut3d->calibV2_lut3d =
            (CalibDbV2_Lut3D_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR((CamCalibDbV2Context_t*)(para->s_calibv2), lut3d_calib));
   }
    Alut3dPrepare((alut3d_handle_t)(params->ctx->a3dlut_para));

    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);

    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    alut3d_handle_t hAlut3d = (alut3d_handle_t)(inparams->ctx->a3dlut_para);

    if (!procParaGroup->arraySize) {
        LOG1_A3DLUT( "%s: do nothing \n", __FUNCTION__);
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    RkAiqAlgoProcResAwbShared_t* awb_proc_res = &scam_3a_res->awb._awbProcRes;
    if (awb_proc_res) {
        RkAiqAlgoProcResAwbShared_t* awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res;
        if(awb_res) {
            if(awb_res->awb_gain_algo.grgain < DIVMIN ||
                    awb_res->awb_gain_algo.gbgain < DIVMIN ) {
                LOGW_A3DLUT("get wrong awb gain from AWB module ,use default value ");
            } else {
                hAlut3d->swinfo.awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                hAlut3d->swinfo.awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            hAlut3d->swinfo.awbIIRDampCoef = awb_res->awb_smooth_factor;
            hAlut3d->swinfo.awbConverged = awb_res->awbConverged;
        } else {
            LOGW_A3DLUT("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGW_A3DLUT("fail to get awb gain form AWB module,use default value ");
    }

    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            hAlut3d->swinfo.sensorGain = pCurExp->LinearExp.exp_real_params.analog_gain
                    * pCurExp->LinearExp.exp_real_params.digital_gain
                    * pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)procParaGroup->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD_A3DLUT("sensor gain choose from second hdr frame for a3dlut");
            hAlut3d->swinfo.sensorGain = pCurExp->HdrExp[1].exp_real_params.analog_gain
                    * pCurExp->HdrExp[1].exp_real_params.digital_gain
                    * pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if((rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2
                  && (rk_aiq_working_mode_t)procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3)  {
            LOGD_A3DLUT("sensor gain choose from third hdr frame for a3dlut");
            hAlut3d->swinfo.sensorGain = pCurExp->HdrExp[2].exp_real_params.analog_gain
                    * pCurExp->HdrExp[2].exp_real_params.digital_gain
                    * pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGW_A3DLUT("working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default value ",
                 procParaGroup->working_mode);
        }
    } else {
        LOGW_A3DLUT("fail to get sensor gain form AE module,use default value ");
    }

    //LOGI_A3DLUT( "%s a3dlut_proc_com.u.init:%d \n", __FUNCTION__, inparams->u.proc.init);
    LOGD_A3DLUT( "%s: sensorGain:%f, awbGain:%f,%f, awbIIRDampCoef:%f\n", __FUNCTION__,
               hAlut3d->swinfo.sensorGain,
               hAlut3d->swinfo.awbGain[0], hAlut3d->swinfo.awbGain[1],
               hAlut3d->swinfo.awbIIRDampCoef);

    Alut3dConfig(hAlut3d);
    if (hAlut3d->lut3d_hw_conf.enable || !hAlut3d->lut3d_hw_conf.bypass_en) {
        LOGD_A3DLUT("update:%d, updateAtt: %d, converge: %d\n",
                hAlut3d->update, hAlut3d->updateAtt, hAlut3d->swinfo.lut3dConverged);
        outparams->cfg_update = hAlut3d->update || hAlut3d->updateAtt || (!hAlut3d->swinfo.lut3dConverged);
    } else {
        outparams->cfg_update = hAlut3d->updateAtt || hAlut3d->calib_update;
    }

    hAlut3d->calib_update = false;
    hAlut3d->updateAtt = false;
    for (int i = 0; i < procResParaGroup->arraySize; i++) {
        if (outparams->cfg_update) {
            *(procResParaGroup->camgroupParmasArray[i]->_lut3dCfg) =
                hAlut3d->lut3d_hw_conf;
        }
        //TODO
        IS_UPDATE_MEM((procResParaGroup->camgroupParmasArray[i]->_lut3dCfg), procParaGroup->_offset_is_update) =
            outparams->cfg_update;
    }

    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupA3dlut = {
    .common = {
        .version = RKISP_ALGO_A3DLUT_VERSION,
        .vendor  = RKISP_ALGO_A3DLUT_VENDOR,
        .description = RKISP_ALGO_A3DLUT_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_A3DLUT,
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
