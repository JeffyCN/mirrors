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
#include "adrc/rk_aiq_adrc_algo.h"
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

        if(CHECK_ISP_HW_V21()) {
            CalibDbV2_drc_t* calibv2_adrc_calib =
                (CalibDbV2_drc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));

            memcpy(&pAdrcCtx->pCalibDB.Drc_v21, calibv2_adrc_calib, sizeof(CalibDbV2_drc_t)); //reload iq paras
        }
        else if(CHECK_ISP_HW_V30()) {
            CalibDbV2_drc_V2_t* calibv2_adrc_calib =
                (CalibDbV2_drc_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));

            memcpy(&pAdrcCtx->pCalibDB.Drc_v30, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V2_t)); //reload iq paras
        }
    }

    if(/* !params->u.prepare.reconfig*/true) {
        AdrcStop(pAdrcCtx); // stop firstly for re-preapre
        result = AdrcStart(pAdrcCtx);
        if (result != XCAM_RETURN_NO_ERROR) {
            LOGE_ATMO("%s Adrc Start failed: %d", __FUNCTION__, result);
            return(XCAM_RETURN_ERROR_FAILED);
        }
    }

    //update
    DrcPrepareJsonMalloc(&pAdrcCtx->Config, &pAdrcCtx->pCalibDB);
    AdrcPrePareJsonUpdateConfig(pAdrcCtx, &pAdrcCtx->pCalibDB);

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn
pre_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    RESULT ret = XCAM_RETURN_NO_ERROR;


    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    bool bypass = false;

    AdrcContext_t* pAdrcCtx = (AdrcContext_t*)inparams->ctx;
    pAdrcCtx->frameCnt = inparams->frame_id > 2 ? (inparams->frame_id - 2) : 0;
    RkAiqAlgoProcAdrc* pAdrcParams = (RkAiqAlgoProcAdrc*)inparams;
    RkAiqAlgoProcResAdrc* pAdrcProcRes = (RkAiqAlgoProcResAdrc*)outparams;

    //update config
    if(pAdrcCtx->drcAttr.opMode > DRC_OPMODE_API_OFF) {
        DrcProcApiMalloc(&pAdrcCtx->Config, &pAdrcCtx->drcAttr, &pAdrcCtx->pCalibDB);
        AdrcProcUpdateConfig(pAdrcCtx, &pAdrcCtx->pCalibDB, &pAdrcCtx->drcAttr);
    }
    DrcEnableSetting(pAdrcCtx);

    // get Sensor Info
    XCamVideoBuffer* xCamAeProcRes = pAdrcParams->com.u.proc.res_comb->ae_proc_res;
    RkAiqAlgoProcResAe* pAEProcRes = NULL;
    if (xCamAeProcRes) {
        pAEProcRes = (RkAiqAlgoProcResAe*)xCamAeProcRes->map(xCamAeProcRes);
        AdrcGetSensorInfo(pAdrcCtx, pAEProcRes->ae_proc_res_rk);
    } else {
        AecProcResult_t AeProcResult;
        memset(&AeProcResult, 0x0, sizeof(AecProcResult_t));
        LOGW_ATMO("%s: Ae Proc result is null!!!\n", __FUNCTION__);
        AdrcGetSensorInfo(pAdrcCtx, AeProcResult);
    }

        //get ae pre res and proc
        XCamVideoBuffer* xCamAePreRes = pAdrcParams->com.u.proc.res_comb->ae_pre_res;
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            bypass = AdrcByPassProcessing(pAdrcCtx, pAEPreRes->ae_pre_res_rk);
        }
        else {
            AecPreResult_t AecHdrPreResult;
            memset(&AecHdrPreResult, 0x0, sizeof(AecPreResult_t));
            bypass = AdrcByPassProcessing(pAdrcCtx, AecHdrPreResult);
            bypass = false;
            LOGW_ATMO("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        bool Enable = false;
        if (CHECK_ISP_HW_V21())
            Enable = pAdrcCtx->Config.Drc_v21.Enable;
        else if (CHECK_ISP_HW_V30())
            Enable = pAdrcCtx->Config.Drc_v30.Enable;

        if (Enable) {
            LOGD_ATMO(
                "%s://////////////////////////////////////ADRC "
                "Start////////////////////////////////////// \n",
                __func__);

            if (!bypass) AdrcTuningParaProcessing(pAdrcCtx);

            // expo para process
            DrcExpoData_t ExpoData;
            memset(&ExpoData, 0, sizeof(DrcExpoData_t));
            if (pAdrcCtx->FrameNumber == LINEAR_NUM) {
                ExpoData.nextSExpo =
                    pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
                ExpoData.nextMExpo = ExpoData.nextSExpo;
                ExpoData.nextLExpo = ExpoData.nextSExpo;
            } else if (pAdrcCtx->FrameNumber == HDR_2X_NUM) {
                ExpoData.nextSExpo =
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
                ExpoData.nextMExpo =
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
                ExpoData.nextLExpo = ExpoData.nextMExpo;
            } else if (pAdrcCtx->FrameNumber == HDR_3X_NUM) {
                ExpoData.nextSExpo =
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
                ExpoData.nextMExpo =
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
                ExpoData.nextLExpo =
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
                    pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time;
            }
            LOGV_ATMO("%s: nextFrame: sexp: %f-%f, mexp: %f-%f, lexp: %f-%f\n", __FUNCTION__,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain,
                      pAdrcParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time);
            if (ExpoData.nextSExpo > 0)
                ExpoData.nextRatioLS = ExpoData.nextLExpo / ExpoData.nextSExpo;
            else
                LOGE_ATMO("%s: Short frame for drc expo sync is ERROR!!!\n", __FUNCTION__);
            if (ExpoData.nextMExpo > 0)
                ExpoData.nextRatioLM = ExpoData.nextLExpo / ExpoData.nextMExpo;
            else
                LOGE_ATMO("%s: Midlle frame for drc expo sync is ERROR!!!\n", __FUNCTION__);
            // clip for long frame mode
            if (pAdrcCtx->SensorInfo.LongFrmMode) {
                ExpoData.nextRatioLS = 1.0;
                ExpoData.nextRatioLM = 1.0;
            }

            if (ExpoData.nextRatioLS >= 1 && ExpoData.nextRatioLM >= 1)
                AdrcExpoParaProcessing(pAdrcCtx, &ExpoData);
            else
                LOGE_ATMO("%s: AE ratio for drc expo sync is under one!!!\n", __FUNCTION__);

            pAdrcCtx->PrevData.ApiMode = pAdrcCtx->drcAttr.opMode;

            LOGD_ATMO(
                "%s://////////////////////////////////////ADRC "
                "Over////////////////////////////////////// \n",
                __func__);
        } else
            LOGD_ATMO("%s: Drc Enable if OFF, Bypass Drc !!! \n", __func__);

        // output ProcRes
        pAdrcProcRes->AdrcProcRes.update         = !bypass;  // not use in isp3xparams for now
        pAdrcProcRes->AdrcProcRes.CompressMode   = pAdrcCtx->AdrcProcRes.CompressMode;
        pAdrcProcRes->AdrcProcRes.LongFrameMode  = pAdrcCtx->AdrcProcRes.LongFrameMode;
        pAdrcProcRes->AdrcProcRes.isHdrGlobalTmo = pAdrcCtx->AdrcProcRes.isHdrGlobalTmo;
        pAdrcProcRes->AdrcProcRes.bTmoEn         = Enable;
        pAdrcProcRes->AdrcProcRes.isLinearTmo    = pAdrcCtx->AdrcProcRes.isLinearTmo;
        memcpy(&pAdrcProcRes->AdrcProcRes.DrcProcRes, &pAdrcCtx->AdrcProcRes.DrcProcRes,
               sizeof(DrcProcRes_t));

        LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
        return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
post_process(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
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
    .pre_process = pre_process,
    .processing = processing,
    .post_process = post_process,
};

RKAIQ_END_DECLARE
