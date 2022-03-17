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
#include "algos/adrc/rk_aiq_adrc_algo.h"

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

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGI_ATMO("%s: Adrc Reload Para!\n", __FUNCTION__);

        if(CHECK_ISP_HW_V21()) {
            CalibDbV2_drc_t* calibv2_adrc_calib =
                (CalibDbV2_drc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));

            memcpy(&pAdrcGrpCtx->pCalibDB.Drc_v21, calibv2_adrc_calib, sizeof(CalibDbV2_drc_t)); //reload iq paras
        }
        else if(CHECK_ISP_HW_V30()) {
            CalibDbV2_drc_V2_t* calibv2_adrc_calib =
                (CalibDbV2_drc_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, adrc_calib));

            memcpy(&pAdrcGrpCtx->pCalibDB.Drc_v30, calibv2_adrc_calib, sizeof(CalibDbV2_drc_V2_t)); //reload iq paras
        }
    }

    if(/* !params->u.prepare.reconfig*/true) {
        AdrcStop(pAdrcGrpCtx); // stop firstly for re-preapre
        result = AdrcStart(pAdrcGrpCtx);
        if (result != XCAM_RETURN_NO_ERROR) {
            LOGE_ATMO("%s Adrc Start failed: %d", __FUNCTION__, result);
            return(XCAM_RETURN_ERROR_FAILED);
        }
    }

    //update
    DrcPrepareJsonMalloc(&pAdrcGrpCtx->Config, &pAdrcGrpCtx->pCalibDB);
    AdrcPrePareJsonUpdateConfig(pAdrcGrpCtx, &pAdrcGrpCtx->pCalibDB);

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return result;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    bool bypass = false;

    AdrcContext_t* pAdrcGrpCtx = (AdrcContext_t*)inparams->ctx;
    pAdrcGrpCtx->frameCnt = inparams->frame_id > 2 ? (inparams->frame_id - 2) : 0;
    RkAiqAlgoCamGroupProcIn* pAdrcGrpParams = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* pAdrcGrpProcRes = (RkAiqAlgoCamGroupProcOut*)outparams;

    //update config
    if(pAdrcGrpCtx->drcAttr.opMode > DRC_OPMODE_API_OFF) {
        DrcProcApiMalloc(&pAdrcGrpCtx->Config, &pAdrcGrpCtx->drcAttr, &pAdrcGrpCtx->pCalibDB);
        AdrcProcUpdateConfig(pAdrcGrpCtx, &pAdrcGrpCtx->pCalibDB, &pAdrcGrpCtx->drcAttr);
    }
    DrcEnableSetting(pAdrcGrpCtx);

    bool Enable = false;
    if(CHECK_ISP_HW_V21())
        Enable = pAdrcGrpCtx->Config.Drc_v21.Enable;
    else if(CHECK_ISP_HW_V30())
        Enable = pAdrcGrpCtx->Config.Drc_v30.Enable;

    if(Enable) {
        LOGD_ATMO("%s://////////////////////////////////////ADRC Group Start////////////////////////////////////// \n", __func__);

        //get Sensor Info
        XCamVideoBuffer* xCamAeProcRes = pAdrcGrpParams->camgroupParmasArray[0]->aec._aeProcRes;
        RkAiqAlgoProcResAe* pAEProcRes = NULL;
        if (xCamAeProcRes) {
            pAEProcRes = (RkAiqAlgoProcResAe*)xCamAeProcRes->map(xCamAeProcRes);
            AdrcGetSensorInfo(pAdrcGrpCtx, pAEProcRes->ae_proc_res_rk);
        }
        else {
            AecProcResult_t AeProcResult;
            memset(&AeProcResult, 0x0, sizeof(AecProcResult_t));
            LOGW_ATMO("%s: Ae Proc result is null!!!\n", __FUNCTION__);
            AdrcGetSensorInfo(pAdrcGrpCtx, AeProcResult);
        }

        //get ae pre res and proc
        XCamVideoBuffer* xCamAePreRes = pAdrcGrpParams->camgroupParmasArray[0]->aec._aePreRes;
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            bypass = AdrcByPassProcessing(pAdrcGrpCtx, pAEPreRes->ae_pre_res_rk);
        }
        else {
            AecPreResult_t AecHdrPreResult;
            memset(&AecHdrPreResult, 0x0, sizeof(AecPreResult_t));
            bypass = AdrcByPassProcessing(pAdrcGrpCtx, AecHdrPreResult);
            bypass = false;
            LOGW_ATMO("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        if(!bypass)
            AdrcTuningParaProcessing(pAdrcGrpCtx);

        //expo para process
        DrcExpoData_t ExpoData;
        memset(&ExpoData, 0, sizeof(DrcExpoData_t));
        if(pAdrcGrpCtx->FrameNumber == LINEAR_NUM) {
            ExpoData.nextSExpo = pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.integration_time;
            ExpoData.nextMExpo = ExpoData.nextSExpo;
            ExpoData.nextLExpo = ExpoData.nextSExpo;
        }
        else if(pAdrcGrpCtx->FrameNumber == HDR_2X_NUM) {
            ExpoData.nextSExpo = pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.analog_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.digital_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.integration_time;
            ExpoData.nextMExpo = pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.analog_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.digital_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.integration_time;
            ExpoData.nextLExpo = ExpoData.nextMExpo;
        }
        else if(pAdrcGrpCtx->FrameNumber == HDR_3X_NUM) {
            ExpoData.nextSExpo = pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.analog_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.digital_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.integration_time;
            ExpoData.nextMExpo = pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.analog_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.digital_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.integration_time;
            ExpoData.nextLExpo = pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.analog_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.digital_gain *
                                 pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.integration_time;
        }
        LOGV_ATMO("%s: nextFrame: sexp: %f-%f, mexp: %f-%f, lexp: %f-%f\n", __FUNCTION__,
                  pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                  pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.integration_time,
                  pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                  pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.integration_time,
                  pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                  pAdrcGrpParams->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.integration_time);
        if(ExpoData.nextSExpo > 0 )
            ExpoData.nextRatioLS = ExpoData.nextLExpo / ExpoData.nextSExpo;
        else
            LOGE_ATMO("%s: Short frame for drc expo sync is ERROR!!!\n", __FUNCTION__);
        if(ExpoData.nextMExpo > 0 )
            ExpoData.nextRatioLM = ExpoData.nextLExpo / ExpoData.nextMExpo;
        else
            LOGE_ATMO("%s: Midlle frame for drc expo sync is ERROR!!!\n", __FUNCTION__);
        //clip for long frame mode
        if (pAdrcGrpCtx->SensorInfo.LongFrmMode) {
            ExpoData.nextRatioLS = 1.0;
            ExpoData.nextRatioLM = 1.0;
        }

        if(ExpoData.nextRatioLS >= 1 && ExpoData.nextRatioLM >= 1)
            AdrcExpoParaProcessing(pAdrcGrpCtx, &ExpoData);
        else
            LOGE_ATMO("%s: AE ratio for drc expo sync is under one!!!\n", __FUNCTION__);

        pAdrcGrpCtx->PrevData.ApiMode = pAdrcGrpCtx->drcAttr.opMode;
        //output ProcRes
        for(int i = 0; i < pAdrcGrpProcRes->arraySize; i++) {
            pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->update = !bypass ;//not use in isp3xparams for now
            pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->CompressMode = pAdrcGrpCtx->AdrcProcRes.CompressMode;
            pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->LongFrameMode = pAdrcGrpCtx->AdrcProcRes.LongFrameMode;
            pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->isHdrGlobalTmo = pAdrcGrpCtx->AdrcProcRes.isHdrGlobalTmo;
            pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->bTmoEn = pAdrcGrpCtx->AdrcProcRes.bTmoEn;
            pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->isLinearTmo = pAdrcGrpCtx->AdrcProcRes.isLinearTmo;
            memcpy(&pAdrcGrpProcRes->camgroupParmasArray[i]->_adrcConfig->DrcProcRes, &pAdrcGrpCtx->AdrcProcRes.DrcProcRes, sizeof(DrcProcRes_t));
        }

        LOGD_ATMO("%s://////////////////////////////////////ADRC Group Over////////////////////////////////////// \n", __func__);
    }
    else
        LOGD_ATMO("%s: Group Drc Enable if OFF, Bypass Drc !!! \n", __func__);

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
