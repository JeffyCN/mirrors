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
#if RKAIQ_HAVE_DRC_V20
#include "adrc/rk_aiq_adrc_algo_v20.h"
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
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || RKAIQ_HAVE_DRC_V20
    if (AdrcCfgParam->compr_bit) {
        pAdrcCtx->FrameNumber = SENSOR_MGE;
        pAdrcCtx->compr_bit   = AdrcCfgParam->compr_bit;
        if (pAdrcCtx->compr_bit > ISP_HDR_BIT_NUM_MAX)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) > %d!!!\n", __FUNCTION__, pAdrcCtx->compr_bit,
                      ISP_HDR_BIT_NUM_MAX);
        if (pAdrcCtx->compr_bit < ISP_HDR_BIT_NUM_MIN)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) < %d!!!\n", __FUNCTION__, pAdrcCtx->compr_bit,
                      ISP_HDR_BIT_NUM_MIN);
        pAdrcCtx->compr_bit =
            LIMIT_VALUE(pAdrcCtx->compr_bit, ISP_HDR_BIT_NUM_MAX, ISP_HDR_BIT_NUM_MIN);
    }
#endif

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGI_ATMO("%s: Adrc Reload Para!\n", __FUNCTION__);
#if RKAIQ_HAVE_DRC_V10
        CalibDbV2_drc_V10_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV10.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V10_t));
#endif
#if RKAIQ_HAVE_DRC_V11
        CalibDbV2_drc_V11_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV11.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V11_t));
#endif
#if RKAIQ_HAVE_DRC_V12
        CalibDbV2_drc_V12_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V12_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV12.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V12_t));
        AdrcV12ClipStAutoParams(pAdrcCtx);
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
        CalibDbV2_drc_v12_lite_t* calibv2_adrc_calib =
            (CalibDbV2_drc_v12_lite_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV12.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_v12_lite_t));
        AdrcV12ClipStAutoParams(pAdrcCtx);
#endif

#if RKAIQ_HAVE_DRC_V20
        CalibDbV2_drc_V20_t* calibv2_adrc_calib =
            (CalibDbV2_drc_V20_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));
        memcpy(&pAdrcCtx->drcAttrV20.stAuto, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V20_t));
#endif
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR)
            return XCAM_RETURN_NO_ERROR;
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
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || RKAIQ_HAVE_DRC_V20
        pAdrcCtx->ablcV32_proc_res = pAdrcParams->ablcV32_proc_res;
#endif
        if (DrcEnableSetting(pAdrcCtx, pAdrcProcRes->AdrcProcRes)) {
            // get Sensor Info
            pAdrcCtx->NextData.AEData.LongFrmMode = pAdrcParams->LongFrmMode;

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
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || (RKAIQ_HAVE_DRC_V20 && !USE_NEWSTRUCT)
            if (pAdrcCtx->ablcV32_proc_res.blc_ob_enable)
                pAdrcCtx->NextData.AEData.ISO *= pAdrcCtx->ablcV32_proc_res.isp_ob_predgain;
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
        } else if (pAdrcCtx->FrameNumber == SENSOR_MGE) {
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE || RKAIQ_HAVE_DRC_V20
            pAdrcCtx->NextData.AEData.MExpo =
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
            pAdrcCtx->NextData.AEData.ISO =
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
                pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
            if (pAdrcCtx->NextData.AEData.MExpo < FLT_EPSILON) {
                pAdrcCtx->NextData.AEData.MExpo =
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.integration_time;
                pAdrcCtx->NextData.AEData.ISO =
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
            }
            pAdrcCtx->NextData.AEData.LExpo = pAdrcCtx->NextData.AEData.MExpo;
            pAdrcCtx->NextData.AEData.SExpo =
                pAdrcCtx->NextData.AEData.MExpo /
                pow(2.0f, float(pAdrcCtx->compr_bit - ISP_HDR_BIT_NUM_MIN));
#endif
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
        } else if (pAdrcCtx->FrameNumber == LINEAR_NUM || pAdrcCtx->FrameNumber == SENSOR_MGE) {
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
            if (pAdrcCtx->NextData.AEData.L2S_Ratio < RATIO_DEFAULT) {
                LOGE_ATMO("%s: Next L2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                          __FUNCTION__, pAdrcCtx->NextData.AEData.L2S_Ratio);
                pAdrcCtx->NextData.AEData.L2S_Ratio = RATIO_DEFAULT;
            }
            pAdrcCtx->NextData.AEData.M2S_Ratio =
                pAdrcCtx->NextData.AEData.MExpo / pAdrcCtx->NextData.AEData.SExpo;
            if (pAdrcCtx->NextData.AEData.M2S_Ratio < RATIO_DEFAULT) {
                LOGE_ATMO("%s: Next M2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                          __FUNCTION__, pAdrcCtx->NextData.AEData.M2S_Ratio);
                pAdrcCtx->NextData.AEData.M2S_Ratio = RATIO_DEFAULT;
            }
        } else
            LOGE_ATMO("%s: Next Short frame for drc expo sync is %f!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.AEData.SExpo);
        if (pAdrcCtx->NextData.AEData.MExpo > FLT_EPSILON) {
            pAdrcCtx->NextData.AEData.L2M_Ratio =
                pAdrcCtx->NextData.AEData.LExpo / pAdrcCtx->NextData.AEData.MExpo;
            if (pAdrcCtx->NextData.AEData.L2M_Ratio < RATIO_DEFAULT) {
                LOGE_ATMO("%s: Next L2M_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                          __FUNCTION__, pAdrcCtx->NextData.AEData.L2M_Ratio);
                pAdrcCtx->NextData.AEData.L2M_Ratio = RATIO_DEFAULT;
            }
        } else
            LOGE_ATMO("%s: Next Midlle frame for drc expo sync is %f!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.AEData.MExpo);
        //clip for long frame mode
        if (pAdrcCtx->NextData.AEData.LongFrmMode) {
            pAdrcCtx->NextData.AEData.L2S_Ratio = LONG_FRAME_MODE_RATIO;
            pAdrcCtx->NextData.AEData.M2S_Ratio = LONG_FRAME_MODE_RATIO;
            pAdrcCtx->NextData.AEData.L2M_Ratio = LONG_FRAME_MODE_RATIO;
        }
        // clip L2M_ratio to 32x
        if (pAdrcCtx->NextData.AEData.L2M_Ratio > AE_RATIO_L2M_MAX) {
            LOGE_ATMO("%s: Next L2M_ratio:%f out of range, clip to 32.0x!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.AEData.L2M_Ratio);
            pAdrcCtx->NextData.AEData.L2M_Ratio = AE_RATIO_L2M_MAX;
        }
        // clip L2S_ratio
        if (pAdrcCtx->NextData.AEData.L2S_Ratio > AE_RATIO_MAX) {
            LOGE_ATMO("%s: Next L2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                      pAdrcCtx->NextData.AEData.L2S_Ratio);
            pAdrcCtx->NextData.AEData.L2S_Ratio = AE_RATIO_MAX;
        }
        // clip L2M_ratio and M2S_Ratio
        if (pAdrcCtx->NextData.AEData.L2M_Ratio * pAdrcCtx->NextData.AEData.M2S_Ratio >
            AE_RATIO_MAX) {
            LOGE_ATMO("%s: Next L2M_Ratio*M2S_Ratio:%f out of range, clip to 256.0x!!!\n",
                      __FUNCTION__,
                      pAdrcCtx->NextData.AEData.L2M_Ratio * pAdrcCtx->NextData.AEData.M2S_Ratio);
            pAdrcCtx->NextData.AEData.M2S_Ratio =
                AE_RATIO_MAX / pAdrcCtx->NextData.AEData.L2M_Ratio;
        }

        // get ae pre res
        XCamVideoBuffer* xCamAePreRes = pAdrcParams->com.u.proc.res_comb->ae_pre_res;
        RkAiqAlgoPreResAe* pAEPreRes  = NULL;
        if (xCamAePreRes) {
            pAEPreRes            = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            if (pAdrcCtx->FrameNumber == LINEAR_NUM)
                pAdrcCtx->NextData.AEData.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
            else if (pAdrcCtx->FrameNumber == HDR_2X_NUM || pAdrcCtx->FrameNumber == HDR_3X_NUM)
                pAdrcCtx->NextData.AEData.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
            else
                pAdrcCtx->NextData.AEData.EnvLv = ENVLVMIN;
            // Normalize the current envLv for AEC
            pAdrcCtx->NextData.AEData.EnvLv =
                (pAdrcCtx->NextData.AEData.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
            pAdrcCtx->NextData.AEData.EnvLv =
                LIMIT_VALUE(pAdrcCtx->NextData.AEData.EnvLv, ENVLVMAX, ENVLVMIN);
        } else {
            pAdrcCtx->NextData.AEData.EnvLv = ENVLVMIN;
            LOGW_ATMO("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        // get motion coef
        pAdrcCtx->NextData.MotionCoef = MOVE_COEF_DEFAULT;

        // get bypass_tuning_params
        bypass_tuning_params = AdrcByPassTuningProcessing(pAdrcCtx);

        // get bypass_expo_params
        if (pAdrcCtx->FrameID <= INIT_CALC_PARAMS_NUM)
            bypass_expo_params = false;
        else if (pAdrcCtx->ifReCalcStAuto || pAdrcCtx->ifReCalcStManual)
            bypass_expo_params = false;
        else if (!pAdrcCtx->CurrData.AEData.LongFrmMode != !pAdrcCtx->NextData.AEData.LongFrmMode)
            bypass_expo_params = false;
        else if ((pAdrcCtx->CurrData.AEData.L2M_Ratio - pAdrcCtx->NextData.AEData.L2M_Ratio) >
                     FLT_EPSILON ||
                 (pAdrcCtx->CurrData.AEData.L2M_Ratio - pAdrcCtx->NextData.AEData.L2M_Ratio) <
                     -FLT_EPSILON ||
                 (pAdrcCtx->CurrData.AEData.M2S_Ratio - pAdrcCtx->NextData.AEData.M2S_Ratio) >
                     FLT_EPSILON ||
                 (pAdrcCtx->CurrData.AEData.M2S_Ratio - pAdrcCtx->NextData.AEData.M2S_Ratio) <
                     -FLT_EPSILON ||
                 (pAdrcCtx->CurrData.AEData.L2S_Ratio - pAdrcCtx->NextData.AEData.L2S_Ratio) >
                     FLT_EPSILON ||
                 (pAdrcCtx->CurrData.AEData.L2S_Ratio - pAdrcCtx->NextData.AEData.L2S_Ratio) <
                     -FLT_EPSILON)
            bypass_expo_params = false;
        else
            bypass_expo_params = true;

        // get tuning paras
        if (!bypass_expo_params || !bypass_tuning_params || !pAdrcCtx->isDampStable)
            AdrcTuningParaProcessing(pAdrcCtx, pAdrcProcRes->AdrcProcRes);

        // get expo related paras
        if (!bypass_expo_params || !bypass_tuning_params || !pAdrcCtx->isDampStable)
            AdrcExpoParaProcessing(pAdrcCtx, pAdrcProcRes->AdrcProcRes);
        } else {
            LOGD_ATMO("%s: Drc Enable is OFF, Bypass Drc !!! \n", __func__);
        }
    }
    LOGD_ATMO(
        "%s:////////////////////////////////////////////ADRC "
        "Over//////////////////////////////////////////// \n",
        __func__);

    // output ProcRes
    outparams->cfg_update = !bypass_tuning_params || !bypass_expo_params ||
                            pAdrcCtx->ifReCalcStAuto || pAdrcCtx->ifReCalcStManual ||
                            !pAdrcCtx->isDampStable || inparams->u.proc.init;
    if (pAdrcCtx->ifReCalcStAuto) pAdrcCtx->ifReCalcStAuto = false;
    if (pAdrcCtx->ifReCalcStManual) pAdrcCtx->ifReCalcStManual = false;
    // store expo data
    pAdrcCtx->CurrData.AEData.LongFrmMode = pAdrcCtx->NextData.AEData.LongFrmMode;
    pAdrcCtx->CurrData.AEData.L2M_Ratio   = pAdrcCtx->NextData.AEData.L2M_Ratio;
    pAdrcCtx->CurrData.AEData.M2S_Ratio   = pAdrcCtx->NextData.AEData.M2S_Ratio;
    pAdrcCtx->CurrData.AEData.L2S_Ratio   = pAdrcCtx->NextData.AEData.L2S_Ratio;

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
