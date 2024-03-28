/*
 * rk_aiq_algo_camgroup_adrc_itf.c
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
#include "algos/adrc/rk_aiq_algo_adrc_itf.h"
#include "xcam_log.h"
#if RKAIQ_HAVE_DRC_V10
#include "adrc/rk_aiq_adrc_algo_v10.h"
#endif
#if RKAIQ_HAVE_DRC_V11
#include "adrc/rk_aiq_adrc_algo_v11.h"
#endif
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE
#include "adrc/rk_aiq_adrc_algo_v12.h"
#endif
#if RKAIQ_HAVE_DRC_V20
#include "adrc/rk_aiq_adrc_algo_v20.h"
#endif

RKAIQ_BEGIN_DECLARE

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ATMO(" %s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    AlgoCtxInstanceCfgCamGroup* instanc_int = (AlgoCtxInstanceCfgCamGroup*)cfg;
    AdrcContext_t* pAdrcGrpCtx = NULL;

    result = AdrcInit(&pAdrcGrpCtx, (CamCalibDbV2Context_t*)(instanc_int->s_calibv2));

    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_ATMO("%s Adrc Init failed: %d", __FUNCTION__, result);
        return(XCAM_RETURN_ERROR_FAILED);
    }
    *context = (RkAiqAlgoContext *)(pAdrcGrpCtx);

    LOG1_ATMO(" %s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    if(context != NULL) {

        AdrcContext_t* pAdrcGrpCtx = (AdrcContext_t*)context;
        result = AdrcRelease(pAdrcGrpCtx);
        if (result != XCAM_RETURN_NO_ERROR) {
            LOGE_ATMO("%s Adrc Release failed: %d", __FUNCTION__, result);
            return(XCAM_RETURN_ERROR_FAILED);
        }
        context = NULL;
    }

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    AdrcContext_t* pAdrcGrpCtx = (AdrcContext_t*)params->ctx;
    RkAiqAlgoCamGroupPrepare* AdrcCfgParam = (RkAiqAlgoCamGroupPrepare*)params; //come from params in html
    const CamCalibDbV2Context_t* pCalibDb = AdrcCfgParam->s_calibv2;

    if (AdrcCfgParam->gcom.com.u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAdrcGrpCtx->FrameNumber = LINEAR_NUM;
    else if (AdrcCfgParam->gcom.com.u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AdrcCfgParam->gcom.com.u.prepare.working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAdrcGrpCtx->FrameNumber = HDR_2X_NUM;
    else
        pAdrcGrpCtx->FrameNumber = HDR_3X_NUM;
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || RKAIQ_HAVE_DRC_V20
    if (AdrcCfgParam->gcom.com.u.prepare.compr_bit) {
        pAdrcGrpCtx->FrameNumber = SENSOR_MGE;
        pAdrcGrpCtx->compr_bit   = AdrcCfgParam->gcom.com.u.prepare.compr_bit;
        if (pAdrcGrpCtx->compr_bit > ISP_HDR_BIT_NUM_MAX)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) > %d!!!\n", __FUNCTION__, pAdrcGrpCtx->compr_bit,
                      ISP_HDR_BIT_NUM_MAX);
        if (pAdrcGrpCtx->compr_bit < ISP_HDR_BIT_NUM_MIN)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) < %d!!!\n", __FUNCTION__, pAdrcGrpCtx->compr_bit,
                      ISP_HDR_BIT_NUM_MIN);
        pAdrcGrpCtx->compr_bit =
            LIMIT_VALUE(pAdrcGrpCtx->compr_bit, ISP_HDR_BIT_NUM_MAX, ISP_HDR_BIT_NUM_MIN);
    }
#endif

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGI_ATMO("%s: Adrc Reload Para!\n", __FUNCTION__);
#if RKAIQ_HAVE_DRC_V10
        CalibDbV2_drc_V10_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcGrpCtx->drcAttrV10.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_V10_t));  // reload stAuto
#endif
#if RKAIQ_HAVE_DRC_V11
        CalibDbV2_drc_V11_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcGrpCtx->drcAttrV11.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_V11_t));  // reload stAuto
#endif
#if RKAIQ_HAVE_DRC_V12
        CalibDbV2_drc_V12_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V12_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcGrpCtx->drcAttrV12.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_V12_t));  // reload stAuto
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
        CalibDbV2_drc_v12_lite_t* calibv2_adrc_calib =
            (CalibDbV2_drc_v12_lite_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcGrpCtx->drcAttrV12.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_v12_lite_t));  // reload stAuto
#endif
#if RKAIQ_HAVE_DRC_V20
        CalibDbV2_drc_V20_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V20_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcGrpCtx->drcAttrV20.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_V20_t));  // reload stAuto
#endif
        pAdrcGrpCtx->ifReCalcStAuto = true;
    } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
        pAdrcGrpCtx->isCapture = true;
    }

    if(/* !params->u.prepare.reconfig*/true) {
        AdrcStop(pAdrcGrpCtx); // stop firstly for re-preapre
        result = AdrcStart(pAdrcGrpCtx);
        if (result != XCAM_RETURN_NO_ERROR) {
            LOGE_ATMO("%s Adrc Start failed: %d", __FUNCTION__, result);
            return(XCAM_RETURN_ERROR_FAILED);
        }
    }

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    bool bypass_tuning_params                 = true;
    bool bypass_expo_params                   = true;
    AdrcContext_t* pAdrcGrpCtx = (AdrcContext_t*)inparams->ctx;
    pAdrcGrpCtx->FrameID                      = inparams->frame_id;
    RkAiqAlgoCamGroupProcIn* pAdrcGrpParams = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* pAdrcGrpProcRes = (RkAiqAlgoCamGroupProcOut*)outparams;

    LOGD_ATMO(
        "%s:////////////////////////////////////////////ADRC Group "
        "Start//////////////////////////////////////////// \n",
        __func__);

    if (pAdrcGrpCtx->isCapture) {
        LOGD_ATMO("%s: It's capturing, using pre frame params\n", __func__);
        pAdrcGrpCtx->isCapture = false;
    } else {
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || RKAIQ_HAVE_DRC_V20 && !USE_NEWSTRUCT
        pAdrcGrpCtx->ablcV32_proc_res.blc_ob_enable =
            pAdrcGrpParams->camgroupParmasArray[0]->ablc._blcConfig_v32->blc_ob_enable;
        pAdrcGrpCtx->ablcV32_proc_res.isp_ob_predgain =
            pAdrcGrpParams->camgroupParmasArray[0]->ablc._blcConfig_v32->isp_ob_predgain;
        if (pAdrcGrpCtx->ablcV32_proc_res.blc_ob_enable) {
            if (pAdrcGrpCtx->ablcV32_proc_res.isp_ob_predgain < ISP_PREDGAIN_DEFAULT) {
                LOGE_ATMO("%s: ob_enable ON, and ob_predgain[%f] < 1.0f, clip to 1.0!!!\n",
                          __FUNCTION__, pAdrcGrpCtx->ablcV32_proc_res.isp_ob_predgain);
                pAdrcGrpCtx->ablcV32_proc_res.isp_ob_predgain = ISP_PREDGAIN_DEFAULT;
            }
        }
#endif
        if (DrcEnableSetting(pAdrcGrpCtx, pAdrcGrpProcRes->camgroupParmasArray[0]->_adrcConfig)) {
            // get LongFrmMode
            RkAiqAlgoProcResAeShared_t* pAEProcRes = &pAdrcGrpParams->camgroupParmasArray[0]->aec._aeProcRes;
            pAdrcGrpCtx->NextData.AEData.LongFrmMode = pAEProcRes->LongFrmMode;

        // get eff expo
        if (pAdrcGrpCtx->FrameNumber == LINEAR_NUM) {
            pAdrcGrpCtx->NextData.AEData.SExpo =
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.integration_time;
            pAdrcGrpCtx->NextData.AEData.MExpo = pAdrcGrpCtx->NextData.AEData.SExpo;
            pAdrcGrpCtx->NextData.AEData.LExpo = pAdrcGrpCtx->NextData.AEData.SExpo;
            pAdrcGrpCtx->NextData.AEData.ISO =
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain *
                ISOMIN;
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || RKAIQ_HAVE_DRC_V20 && !USE_NEWSTRUCT
            if (pAdrcGrpCtx->ablcV32_proc_res.blc_ob_enable) {
                if (pAdrcGrpCtx->ablcV32_proc_res.isp_ob_predgain < ISP_PREDGAIN_DEFAULT) {
                    LOGE_ATMO("%s: ob_enable ON, and ob_predgain[%f] < 1.0f, clip to 1.0!!!\n",
                              __FUNCTION__, pAdrcGrpCtx->ablcV32_proc_res.isp_ob_predgain);
                    pAdrcGrpCtx->ablcV32_proc_res.isp_ob_predgain = ISP_PREDGAIN_DEFAULT;
                }
                pAdrcGrpCtx->NextData.AEData.ISO *= pAdrcGrpCtx->ablcV32_proc_res.isp_ob_predgain;
            }
#endif
        } else if (pAdrcGrpCtx->FrameNumber == HDR_2X_NUM) {
            pAdrcGrpCtx->NextData.AEData.SExpo = pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[0]
                                                     .exp_real_params.analog_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[0]
                                                     .exp_real_params.digital_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[0]
                                                     .exp_real_params.isp_dgain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[0]
                                                     .exp_real_params.integration_time;
            pAdrcGrpCtx->NextData.AEData.MExpo = pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[1]
                                                     .exp_real_params.analog_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[1]
                                                     .exp_real_params.digital_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[1]
                                                     .exp_real_params.isp_dgain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[1]
                                                     .exp_real_params.integration_time;
            pAdrcGrpCtx->NextData.AEData.LExpo = pAdrcGrpCtx->NextData.AEData.MExpo;
            pAdrcGrpCtx->NextData.AEData.ISO   = pAdrcGrpParams->camgroupParmasArray[0]
                                                   ->aec._effAecExpInfo.HdrExp[1]
                                                   .exp_real_params.analog_gain *
                                               pAdrcGrpParams->camgroupParmasArray[0]
                                                   ->aec._effAecExpInfo.HdrExp[1]
                                                   .exp_real_params.digital_gain *
                                               pAdrcGrpParams->camgroupParmasArray[0]
                                                   ->aec._effAecExpInfo.HdrExp[1]
                                                   .exp_real_params.isp_dgain *
                                               ISOMIN;
        } else if (pAdrcGrpCtx->FrameNumber == HDR_3X_NUM) {
            pAdrcGrpCtx->NextData.AEData.SExpo = pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[0]
                                                     .exp_real_params.analog_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[0]
                                                     .exp_real_params.digital_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[0]
                                                     .exp_real_params.isp_dgain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[0]
                                                     .exp_real_params.integration_time;
            pAdrcGrpCtx->NextData.AEData.MExpo = pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[1]
                                                     .exp_real_params.analog_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[1]
                                                     .exp_real_params.digital_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[1]
                                                     .exp_real_params.isp_dgain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[1]
                                                     .exp_real_params.integration_time;
            pAdrcGrpCtx->NextData.AEData.LExpo = pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[2]
                                                     .exp_real_params.analog_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[2]
                                                     .exp_real_params.digital_gain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[2]
                                                     .exp_real_params.isp_dgain *
                                                 pAdrcGrpParams->camgroupParmasArray[0]
                                                     ->aec._effAecExpInfo.HdrExp[2]
                                                     .exp_real_params.integration_time;
            pAdrcGrpCtx->NextData.AEData.ISO = pAdrcGrpParams->camgroupParmasArray[0]
                                                   ->aec._effAecExpInfo.HdrExp[1]
                                                   .exp_real_params.analog_gain *
                                               pAdrcGrpParams->camgroupParmasArray[0]
                                                   ->aec._effAecExpInfo.HdrExp[1]
                                                   .exp_real_params.digital_gain *
                                               pAdrcGrpParams->camgroupParmasArray[0]
                                                   ->aec._effAecExpInfo.HdrExp[1]
                                                   .exp_real_params.isp_dgain *
                                               ISOMIN;
        } else if (pAdrcGrpCtx->FrameNumber == SENSOR_MGE) {
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || RKAIQ_HAVE_DRC_V20
            pAdrcGrpCtx->NextData.AEData.MExpo =
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.integration_time;
            pAdrcGrpCtx->NextData.AEData.LExpo = pAdrcGrpCtx->NextData.AEData.MExpo;
            pAdrcGrpCtx->NextData.AEData.SExpo =
                pAdrcGrpCtx->NextData.AEData.MExpo /
                pow(2.0f, float(pAdrcGrpCtx->compr_bit - ISP_HDR_BIT_NUM_MIN));
            pAdrcGrpCtx->NextData.AEData.ISO =
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                pAdrcGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain *
                ISOMIN;
#endif
        }
        pAdrcGrpCtx->NextData.AEData.ISO =
            LIMIT_VALUE(pAdrcGrpCtx->NextData.AEData.ISO, ISOMAX, ISOMIN);
        if (pAdrcGrpCtx->FrameNumber == HDR_2X_NUM || pAdrcGrpCtx->FrameNumber == HDR_2X_NUM) {
            LOGV_ATMO("%s: nextFrame: sexp: %f-%f, mexp: %f-%f, lexp: %f-%f\n", __FUNCTION__,
                      pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[0]
                              .exp_real_params.analog_gain *
                          pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[0]
                              .exp_real_params.digital_gain *
                          pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[0]
                              .exp_real_params.isp_dgain,
                      pAdrcGrpParams->camgroupParmasArray[0]
                          ->aec._effAecExpInfo.HdrExp[0]
                          .exp_real_params.integration_time,
                      pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[1]
                              .exp_real_params.analog_gain *
                          pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[1]
                              .exp_real_params.digital_gain *
                          pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[1]
                              .exp_real_params.isp_dgain,
                      pAdrcGrpParams->camgroupParmasArray[0]
                          ->aec._effAecExpInfo.HdrExp[1]
                          .exp_real_params.integration_time,
                      pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[2]
                              .exp_real_params.analog_gain *
                          pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[2]
                              .exp_real_params.digital_gain *
                          pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.HdrExp[2]
                              .exp_real_params.isp_dgain,
                      pAdrcGrpParams->camgroupParmasArray[0]
                          ->aec._effAecExpInfo.HdrExp[2]
                          .exp_real_params.integration_time);
        } else if (pAdrcGrpCtx->FrameNumber == LINEAR_NUM ||
                   pAdrcGrpCtx->FrameNumber == SENSOR_MGE) {
            LOGV_ATMO("%s: nextFrame: exp: %f-%f\n", __FUNCTION__,
                      pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                          pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                          pAdrcGrpParams->camgroupParmasArray[0]
                              ->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain,
                      pAdrcGrpParams->camgroupParmasArray[0]
                          ->aec._effAecExpInfo.LinearExp.exp_real_params.integration_time);
        }
        if (pAdrcGrpCtx->NextData.AEData.SExpo > FLT_EPSILON) {
            pAdrcGrpCtx->NextData.AEData.L2S_Ratio =
                pAdrcGrpCtx->NextData.AEData.LExpo / pAdrcGrpCtx->NextData.AEData.SExpo;
            if (pAdrcGrpCtx->NextData.AEData.L2S_Ratio < RATIO_DEFAULT) {
                LOGE_ATMO("%s: Next L2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                          __FUNCTION__, pAdrcGrpCtx->NextData.AEData.L2S_Ratio);
                pAdrcGrpCtx->NextData.AEData.L2S_Ratio = RATIO_DEFAULT;
            }
            pAdrcGrpCtx->NextData.AEData.M2S_Ratio =
                pAdrcGrpCtx->NextData.AEData.MExpo / pAdrcGrpCtx->NextData.AEData.SExpo;
            if (pAdrcGrpCtx->NextData.AEData.M2S_Ratio < RATIO_DEFAULT) {
                LOGE_ATMO("%s: Next M2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                          __FUNCTION__, pAdrcGrpCtx->NextData.AEData.M2S_Ratio);
                pAdrcGrpCtx->NextData.AEData.M2S_Ratio = RATIO_DEFAULT;
            }
        } else
            LOGE_ATMO("%s: Next frame for drc expo sync is ERROR!!!\n", __FUNCTION__);
        if (pAdrcGrpCtx->NextData.AEData.MExpo > FLT_EPSILON) {
            pAdrcGrpCtx->NextData.AEData.L2M_Ratio =
                pAdrcGrpCtx->NextData.AEData.LExpo / pAdrcGrpCtx->NextData.AEData.MExpo;
            if (pAdrcGrpCtx->NextData.AEData.L2M_Ratio < RATIO_DEFAULT) {
                LOGE_ATMO("%s: Next L2M_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                          __FUNCTION__, pAdrcGrpCtx->NextData.AEData.L2M_Ratio);
                pAdrcGrpCtx->NextData.AEData.L2M_Ratio = RATIO_DEFAULT;
            }
        } else
            LOGE_ATMO("%s: Next Midlle frame for drc expo sync is ERROR!!!\n", __FUNCTION__);
        // clip for long frame mode
        if (pAdrcGrpCtx->NextData.AEData.LongFrmMode) {
            pAdrcGrpCtx->NextData.AEData.L2S_Ratio = LONG_FRAME_MODE_RATIO;
            pAdrcGrpCtx->NextData.AEData.M2S_Ratio = LONG_FRAME_MODE_RATIO;
            pAdrcGrpCtx->NextData.AEData.L2M_Ratio = LONG_FRAME_MODE_RATIO;
        }
        // clip L2M_ratio to 32x
        if (pAdrcGrpCtx->NextData.AEData.L2M_Ratio > AE_RATIO_L2M_MAX) {
            LOGE_ATMO("%s: Next L2M_ratio:%f out of range, clip to 32.0x!!!\n", __FUNCTION__,
                      pAdrcGrpCtx->NextData.AEData.L2M_Ratio);
            pAdrcGrpCtx->NextData.AEData.L2M_Ratio = AE_RATIO_L2M_MAX;
        }
        // clip L2S_ratio
        if (pAdrcGrpCtx->NextData.AEData.L2S_Ratio > AE_RATIO_MAX) {
            LOGE_ATMO("%s: Next L2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                      pAdrcGrpCtx->NextData.AEData.L2S_Ratio);
            pAdrcGrpCtx->NextData.AEData.L2S_Ratio = AE_RATIO_MAX;
        }
        // clip L2M_ratio and M2S_Ratio
        if (pAdrcGrpCtx->NextData.AEData.L2M_Ratio * pAdrcGrpCtx->NextData.AEData.M2S_Ratio >
            AE_RATIO_MAX) {
            LOGE_ATMO(
                "%s: Next L2M_Ratio*M2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                pAdrcGrpCtx->NextData.AEData.L2M_Ratio * pAdrcGrpCtx->NextData.AEData.M2S_Ratio);
            pAdrcGrpCtx->NextData.AEData.M2S_Ratio =
                AE_RATIO_MAX / pAdrcGrpCtx->NextData.AEData.L2M_Ratio;
        }

        // get bypass_expo_params
        if (pAdrcGrpCtx->FrameID <= INIT_CALC_PARAMS_NUM)
            bypass_expo_params = false;
        else if (pAdrcGrpCtx->ifReCalcStAuto || pAdrcGrpCtx->ifReCalcStManual)
            bypass_expo_params = false;
        else if (!pAdrcGrpCtx->CurrData.AEData.LongFrmMode !=
                 !pAdrcGrpCtx->NextData.AEData.LongFrmMode)
            bypass_expo_params = false;
        else if ((pAdrcGrpCtx->CurrData.AEData.L2M_Ratio - pAdrcGrpCtx->NextData.AEData.L2M_Ratio) >
                     FLT_EPSILON ||
                 (pAdrcGrpCtx->CurrData.AEData.L2M_Ratio - pAdrcGrpCtx->NextData.AEData.L2M_Ratio) <
                     -FLT_EPSILON ||
                 (pAdrcGrpCtx->CurrData.AEData.M2S_Ratio - pAdrcGrpCtx->NextData.AEData.M2S_Ratio) >
                     FLT_EPSILON ||
                 (pAdrcGrpCtx->CurrData.AEData.M2S_Ratio - pAdrcGrpCtx->NextData.AEData.M2S_Ratio) <
                     -FLT_EPSILON ||
                 (pAdrcGrpCtx->CurrData.AEData.L2S_Ratio - pAdrcGrpCtx->NextData.AEData.L2S_Ratio) >
                     FLT_EPSILON ||
                 (pAdrcGrpCtx->CurrData.AEData.L2S_Ratio - pAdrcGrpCtx->NextData.AEData.L2S_Ratio) <
                     -FLT_EPSILON)
            bypass_expo_params = false;
        else
            bypass_expo_params = true;

        // get ae pre res and bypass_tuning_params
        XCamVideoBuffer* xCamAePreRes = pAdrcGrpParams->camgroupParmasArray[0]->aec._aePreRes;
        RkAiqAlgoPreResAe* pAEPreRes  = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            if (pAdrcGrpCtx->FrameNumber == LINEAR_NUM)
                pAdrcGrpCtx->NextData.AEData.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
            else if (pAdrcGrpCtx->FrameNumber == HDR_2X_NUM ||
                     pAdrcGrpCtx->FrameNumber == HDR_3X_NUM)
                pAdrcGrpCtx->NextData.AEData.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
            else
                pAdrcGrpCtx->NextData.AEData.EnvLv = ENVLVMIN;
            // Normalize the current envLv for AEC
            pAdrcGrpCtx->NextData.AEData.EnvLv =
                (pAdrcGrpCtx->NextData.AEData.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
            pAdrcGrpCtx->NextData.AEData.EnvLv =
                LIMIT_VALUE(pAdrcGrpCtx->NextData.AEData.EnvLv, ENVLVMAX, ENVLVMIN);
        } else {
            pAdrcGrpCtx->NextData.AEData.EnvLv = ENVLVMIN;
            LOGW_ATMO("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        // get motion coef
        pAdrcGrpCtx->NextData.MotionCoef = MOVE_COEF_DEFAULT;

        // get bypass_tuning_params
        bypass_tuning_params = AdrcByPassTuningProcessing(pAdrcGrpCtx);

        // get tuning paras
        if (!bypass_expo_params || !bypass_tuning_params || !pAdrcGrpCtx->isDampStable)
            AdrcTuningParaProcessing(pAdrcGrpCtx,
                                     pAdrcGrpProcRes->camgroupParmasArray[0]->_adrcConfig);

        // expo para process
        if (!bypass_expo_params || !bypass_tuning_params || !pAdrcGrpCtx->isDampStable)
            AdrcExpoParaProcessing(pAdrcGrpCtx,
                                   pAdrcGrpProcRes->camgroupParmasArray[0]->_adrcConfig);
        } else {
            LOGD_ATMO("%s: Group Drc Enable is OFF, Bypass Drc !!! \n", __func__);
        }
    }
    LOGD_ATMO(
        "%s:////////////////////////////////////////////ADRC Group "
        "Over//////////////////////////////////////////// \n",
        __func__);

    // output ProcRes
    outparams->cfg_update = !bypass_tuning_params || !bypass_expo_params ||
                            pAdrcGrpCtx->ifReCalcStAuto || pAdrcGrpCtx->ifReCalcStManual ||
                            !pAdrcGrpCtx->isDampStable || inparams->u.proc.init;
    IS_UPDATE_MEM((pAdrcGrpProcRes->camgroupParmasArray[0]->_adrcConfig), pAdrcGrpParams->_offset_is_update) =
        outparams->cfg_update;
    for (int i = 1; i < pAdrcGrpProcRes->arraySize; i++) {
        pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->bDrcEn =
            pAdrcGrpProcRes->camgroupParmasArray[0]->_adrcConfig->bDrcEn;
        pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->DrcProcRes =
            pAdrcGrpProcRes->camgroupParmasArray[0]->_adrcConfig->DrcProcRes;
        IS_UPDATE_MEM((pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig), pAdrcGrpParams->_offset_is_update) =
            outparams->cfg_update;
    }
    if (pAdrcGrpCtx->ifReCalcStAuto) pAdrcGrpCtx->ifReCalcStAuto = false;
    if (pAdrcGrpCtx->ifReCalcStManual) pAdrcGrpCtx->ifReCalcStManual = false;
    // store expo data
    pAdrcGrpCtx->CurrData.AEData.LongFrmMode = pAdrcGrpCtx->NextData.AEData.LongFrmMode;
    pAdrcGrpCtx->CurrData.AEData.L2M_Ratio   = pAdrcGrpCtx->NextData.AEData.L2M_Ratio;
    pAdrcGrpCtx->CurrData.AEData.M2S_Ratio   = pAdrcGrpCtx->NextData.AEData.M2S_Ratio;
    pAdrcGrpCtx->CurrData.AEData.L2S_Ratio   = pAdrcGrpCtx->NextData.AEData.L2S_Ratio;

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAdrc = {
    .common = {
        .version = RKISP_ALGO_ADRC_VERSION,
        .vendor  = RKISP_ALGO_ADRC_VENDOR,
        .description = RKISP_ALGO_ADRC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADRC,
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
