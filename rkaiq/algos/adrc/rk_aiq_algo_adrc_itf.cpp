/*
 * rk_aiq_algo_adrc_itf.c
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

#include "adrc/rk_aiq_algo_adrc_itf.h"
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
#include "adrc/rk_aiq_types_adrc_algo_prvt.h"

RKAIQ_BEGIN_DECLARE


static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_ATMO(" %s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    AdrcContext_t* pAdrcCtx = NULL;

    result = AdrcInit(&pAdrcCtx, (CamCalibDbV2Context_t*)(cfg->calibv2));

    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_ATMO("%s Adrc Init failed: %d", __FUNCTION__, result);
        return(XCAM_RETURN_ERROR_FAILED);
    }
    *context = (RkAiqAlgoContext *)(pAdrcCtx);

    LOG1_ATMO(" %s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    if(context != NULL) {

        AdrcContext_t* pAdrcCtx = (AdrcContext_t*)context;
        result = AdrcRelease(pAdrcCtx);
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

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)params->ctx;
    RkAiqAlgoConfigAdrc* AdrcCfgParam = (RkAiqAlgoConfigAdrc*)params; //come from params in html
    const CamCalibDbV2Context_t* pCalibDb = AdrcCfgParam->com.u.prepare.calibv2;

    if (AdrcCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAdrcCtx->FrameNumber = LINEAR_NUM;
    else if (AdrcCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AdrcCfgParam->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAdrcCtx->FrameNumber = HDR_2X_NUM;
    else
        pAdrcCtx->FrameNumber = HDR_3X_NUM;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGI_ATMO("%s: Adrc Reload Para!\n", __FUNCTION__);
#if RKAIQ_HAVE_DRC_V10
        CalibDbV2_drc_V10_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV10.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_V10_t));  // reload stAuto
#endif
#if RKAIQ_HAVE_DRC_V11
        CalibDbV2_drc_V11_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV11.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_V11_t));  // reload stAuto
#endif
#if RKAIQ_HAVE_DRC_V12
        CalibDbV2_drc_V12_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V12_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV12.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_V12_t));  // reload stAuto
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
        CalibDbV2_drc_v12_lite_t* calibv2_adrc_calib =
            (CalibDbV2_drc_v12_lite_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV12.stAuto, calibv2_adrc_calib,
               sizeof(CalibDbV2_drc_v12_lite_t));  // reload stAuto
#endif
        pAdrcCtx->ifReCalcStAuto = true;
    } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
        pAdrcCtx->isCapture = true;
    }

    if(/* !params->u.prepare.reconfig*/true) {
        AdrcStop(pAdrcCtx); // stop firstly for re-preapre
        result = AdrcStart(pAdrcCtx);
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
    bool bypass_tuning_params          = true;
    bool bypass_expo_params            = true;
    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)inparams->ctx;
    RkAiqAlgoProcAdrc* pAdrcParams = (RkAiqAlgoProcAdrc*)inparams;
    RkAiqAlgoProcResAdrc* pAdrcProcRes = (RkAiqAlgoProcResAdrc*)outparams;
    pAdrcCtx->FrameID                  = inparams->frame_id;

    LOGD_ATMO(
        "%s:////////////////////////////////////////////ADRC "
        "Start//////////////////////////////////////////// \n",
        __func__);

    if (pAdrcCtx->isCapture) {
        LOGD_ATMO("%s: It's capturing, using pre frame params\n", __func__);
        pAdrcCtx->isCapture = false;
    } else {
        bool Enable = DrcEnableSetting(pAdrcCtx);

        if (Enable) {
            // get Sensor Info
            XCamVideoBuffer* xCamAeProcRes = pAdrcParams->com.u.proc.res_comb->ae_proc_res;
            RkAiqAlgoProcResAe* pAEProcRes = NULL;
            if (xCamAeProcRes) {
                pAEProcRes = (RkAiqAlgoProcResAe*)xCamAeProcRes->map(xCamAeProcRes);
                pAdrcCtx->NextData.AEData.LongFrmMode = pAEProcRes->ae_proc_res_rk.LongFrmMode;
            } else {
                pAdrcCtx->NextData.AEData.LongFrmMode = false;
                LOGW_ATMO("%s: Ae Proc result is null!!!\n", __FUNCTION__);
            }

        // get eff expo data
        if(pAdrcCtx->FrameNumber == LINEAR_NUM) {
            pAdrcCtx->NextData.AEData.SExpo =
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
            pAdrcCtx->NextData.AEData.ISO =
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
            if (pAdrcCtx->NextData.AEData.SExpo < FLT_EPSILON) {
                pAdrcCtx->NextData.AEData.SExpo =
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.integration_time;
                pAdrcCtx->NextData.AEData.ISO =
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
            }
            pAdrcCtx->NextData.AEData.MExpo = pAdrcCtx->NextData.AEData.SExpo;
            pAdrcCtx->NextData.AEData.LExpo = pAdrcCtx->NextData.AEData.SExpo;

#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE
            pAdrcCtx->ablcV32_proc_res.blc_ob_enable = pAdrcParams->ablcV32_proc_res.blc_ob_enable;
            pAdrcCtx->ablcV32_proc_res.isp_ob_predgain =
                pAdrcParams->ablcV32_proc_res.isp_ob_predgain;
            if (pAdrcCtx->ablcV32_proc_res.blc_ob_enable) {
                if (pAdrcCtx->ablcV32_proc_res.isp_ob_predgain < ISP_PREDGAIN_DEFAULT) {
                    LOGE_ATMO("%s: ob_enable ON, and ob_predgain[%f] < 1.0f, clip to 1.0!!!\n",
                              __FUNCTION__, pAdrcCtx->ablcV32_proc_res.isp_ob_predgain);
                    pAdrcCtx->ablcV32_proc_res.isp_ob_predgain = ISP_PREDGAIN_DEFAULT;
                }
                pAdrcCtx->NextData.AEData.ISO *= pAdrcCtx->ablcV32_proc_res.isp_ob_predgain;
            }
#endif
        }
        else if(pAdrcCtx->FrameNumber == HDR_2X_NUM) {
            pAdrcCtx->NextData.AEData.SExpo =
                pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
            pAdrcCtx->NextData.AEData.MExpo =
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
            pAdrcCtx->NextData.AEData.ISO =
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
            if (pAdrcCtx->NextData.AEData.SExpo < FLT_EPSILON) {
                pAdrcCtx->NextData.AEData.SExpo =
                    pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
                pAdrcCtx->NextData.AEData.MExpo =
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
                pAdrcCtx->NextData.AEData.ISO =
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
            }
            pAdrcCtx->NextData.AEData.LExpo = pAdrcCtx->NextData.AEData.MExpo;
        }
        else if(pAdrcCtx->FrameNumber == HDR_3X_NUM) {
            pAdrcCtx->NextData.AEData.SExpo =
                pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
            pAdrcCtx->NextData.AEData.MExpo =
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
            pAdrcCtx->NextData.AEData.LExpo =
                pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.isp_dgain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time;
            pAdrcCtx->NextData.AEData.ISO =
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
            if (pAdrcCtx->NextData.AEData.SExpo < FLT_EPSILON) {
                pAdrcCtx->NextData.AEData.SExpo =
                    pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
                pAdrcCtx->NextData.AEData.MExpo =
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
                pAdrcCtx->NextData.AEData.LExpo =
                    pAdrcParams->com.u.proc.curExp->HdrExp[2].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[2].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[2].exp_real_params.isp_dgain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[2].exp_real_params.integration_time;
                pAdrcCtx->NextData.AEData.ISO =
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
            }
        }
        pAdrcCtx->NextData.AEData.ISO = LIMIT_VALUE(pAdrcCtx->NextData.AEData.ISO, ISOMAX, ISOMIN);
        if (pAdrcCtx->FrameNumber == HDR_2X_NUM || pAdrcCtx->FrameNumber == HDR_2X_NUM) {
            LOGV_ATMO("%s: nextFrame: sexp: %f-%f, mexp: %f-%f, lexp: %f-%f\n", __FUNCTION__,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
                          pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
                          pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                          pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
                          pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
                          pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
                          pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.isp_dgain,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time);
            LOGV_ATMO("%s: CurrFrame: sexp: %f-%f, mexp: %f-%f, lexp: %f-%f\n", __FUNCTION__,
                      pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                          pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                          pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain,
                      pAdrcParams->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time,
                      pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                          pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                          pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain,
                      pAdrcParams->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time,
                      pAdrcParams->com.u.proc.curExp->HdrExp[2].exp_real_params.analog_gain *
                          pAdrcParams->com.u.proc.curExp->HdrExp[2].exp_real_params.digital_gain *
                          pAdrcParams->com.u.proc.curExp->HdrExp[2].exp_real_params.isp_dgain,
                      pAdrcParams->com.u.proc.curExp->HdrExp[2].exp_real_params.integration_time);
        } else if (pAdrcCtx->FrameNumber == LINEAR_NUM) {
            LOGV_ATMO("%s: nextFrame: exp: %f-%f CurrFrame: exp: %f-%f\n", __FUNCTION__,
                      pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
                          pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
                          pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain,
                      pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time,
                      pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                          pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                          pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain,
                      pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.integration_time);
        }
        if (pAdrcCtx->NextData.AEData.SExpo > FLT_EPSILON) {
            pAdrcCtx->NextData.AEData.L2S_Ratio =
                pAdrcCtx->NextData.AEData.LExpo / pAdrcCtx->NextData.AEData.SExpo;
            pAdrcCtx->NextData.AEData.M2S_Ratio =
                pAdrcCtx->NextData.AEData.MExpo / pAdrcCtx->NextData.AEData.SExpo;
        } else
            LOGE_ATMO("%s: Next Short frame for drc expo sync is %f!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.AEData.SExpo);
        if (pAdrcCtx->NextData.AEData.MExpo > FLT_EPSILON)
            pAdrcCtx->NextData.AEData.L2M_Ratio =
                pAdrcCtx->NextData.AEData.LExpo / pAdrcCtx->NextData.AEData.MExpo;
        else
            LOGE_ATMO("%s: Next Midlle frame for drc expo sync is %f!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.AEData.MExpo);
        //clip for long frame mode
        if (pAdrcCtx->NextData.AEData.LongFrmMode) {
            pAdrcCtx->NextData.AEData.L2S_Ratio = LONG_FRAME_MODE_RATIO;
            pAdrcCtx->NextData.AEData.M2S_Ratio = LONG_FRAME_MODE_RATIO;
            pAdrcCtx->NextData.AEData.L2M_Ratio = LONG_FRAME_MODE_RATIO;
        }

        // get ae pre res and bypass_tuning_params
        XCamVideoBuffer* xCamAePreRes = pAdrcParams->com.u.proc.res_comb->ae_pre_res;
        RkAiqAlgoPreResAe* pAEPreRes  = NULL;
        if (xCamAePreRes) {
            pAEPreRes            = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            bypass_tuning_params = AdrcByPassTuningProcessing(pAdrcCtx, pAEPreRes->ae_pre_res_rk);
        } else {
                AecPreResult_t AecHdrPreResult;
                memset(&AecHdrPreResult, 0x0, sizeof(AecPreResult_t));
                bypass_tuning_params = AdrcByPassTuningProcessing(pAdrcCtx, AecHdrPreResult);
                bypass_tuning_params = false;
                LOGW_ATMO("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        // get bypass_expo_params
        if (pAdrcCtx->NextData.AEData.L2S_Ratio >= RATIO_DEFAULT &&
            pAdrcCtx->NextData.AEData.L2M_Ratio >= RATIO_DEFAULT) {
            if (pAdrcCtx->FrameID <= 2)
                bypass_expo_params = false;
            else if (pAdrcCtx->ifReCalcStAuto || pAdrcCtx->ifReCalcStManual)
                bypass_expo_params = false;
            else if (!pAdrcCtx->CurrData.AEData.LongFrmMode !=
                     !pAdrcCtx->NextData.AEData.LongFrmMode)
                bypass_expo_params = false;
            else if (pAdrcCtx->CurrData.AEData.L2M_Ratio != pAdrcCtx->NextData.AEData.L2M_Ratio ||
                     pAdrcCtx->CurrData.AEData.M2S_Ratio != pAdrcCtx->NextData.AEData.M2S_Ratio ||
                     pAdrcCtx->CurrData.AEData.L2S_Ratio != pAdrcCtx->NextData.AEData.L2S_Ratio)
                bypass_expo_params = false;
            else
                bypass_expo_params = true;
        } else {
            LOGE_ATMO("%s: AE L2S_Ratio:%f L2M_Ratio:%f for drc expo sync is under one!!!\n",
                      __FUNCTION__, __LINE__, pAdrcCtx->NextData.AEData.L2S_Ratio,
                      pAdrcCtx->NextData.AEData.L2M_Ratio);
            bypass_expo_params = true;
        }

        // get tuning paras
        if (!bypass_tuning_params || !pAdrcCtx->isDampStable) AdrcTuningParaProcessing(pAdrcCtx);

        // get expo related paras
        if (!bypass_expo_params || !pAdrcCtx->isDampStable) AdrcExpoParaProcessing(pAdrcCtx);

        } else {
            LOGD_ATMO("%s: Drc Enable is OFF, Bypass Drc !!! \n", __func__);
        }
    }
    LOGD_ATMO(
        "%s:////////////////////////////////////////////ADRC "
        "Over//////////////////////////////////////////// \n",
        __func__);

    // output ProcRes
    pAdrcProcRes->AdrcProcRes.update = !bypass_tuning_params || !bypass_expo_params ||
                                       pAdrcCtx->ifReCalcStAuto || pAdrcCtx->ifReCalcStManual ||
                                       !pAdrcCtx->isDampStable;
    if (pAdrcProcRes->AdrcProcRes.update) {
        pAdrcProcRes->AdrcProcRes.bDrcEn = pAdrcCtx->NextData.Enable;
        memcpy(&pAdrcProcRes->AdrcProcRes.DrcProcRes, &pAdrcCtx->AdrcProcRes.DrcProcRes,
               sizeof(DrcProcRes_t));
    }

    pAdrcCtx->CurrData.Enable  = pAdrcCtx->NextData.Enable;
    pAdrcCtx->ifReCalcStAuto   = false;
    pAdrcCtx->ifReCalcStManual = false;

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAdrc = {
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
