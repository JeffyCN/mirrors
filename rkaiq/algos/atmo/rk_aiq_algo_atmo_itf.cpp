/*
 * rk_aiq_algo_ae_itf.c
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

#include "atmo/rk_aiq_algo_atmo_itf.h"
#include "xcam_log.h"
#include "atmo/rk_aiq_atmo_algo.h"
#include "atmo/rk_aiq_types_atmo_algo_prvt.h"
#include "rk_aiq_algo_types.h"


RKAIQ_BEGIN_DECLARE

static XCamReturn AtmoCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    RESULT ret = ATMO_RET_SUCCESS;
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_ATMO( "%s: create atmo context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset(&ctx->AtmoInstConfig, 0x00, sizeof(AtmoInstanceConfig_t));
    ret = AtmoInit(&ctx->AtmoInstConfig, cfg->calibv2);

    if (ret != ATMO_RET_SUCCESS) {
        LOGE_ATMO("%s ATMO Init failed: %d", __FUNCTION__, ret);
        return(XCAM_RETURN_ERROR_FAILED);
    }

    *context = ctx;

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AtmoDestroyCtx(RkAiqAlgoContext *context)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    RESULT ret = ATMO_RET_SUCCESS;

    if(context != NULL) {
        AtmoHandle_t pAtmoCtx = (AtmoHandle_t)context->AtmoInstConfig.hAtmo;
        ret = AtmoRelease(pAtmoCtx);
        if (ret != ATMO_RET_SUCCESS) {
            LOGE_ATMO("%s Atmo Release failed: %d", __FUNCTION__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
        delete context;
        context = NULL;
    }

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AtmoPrepare(RkAiqAlgoCom* params)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    RESULT ret = ATMO_RET_SUCCESS;

    AtmoHandle_t pAtmoCtx = params->ctx->AtmoInstConfig.hAtmo;
    RkAiqAlgoConfigAtmo* AtmoCfgParam = (RkAiqAlgoConfigAtmo*)params; //come from params in html
    const CamCalibDbV2Context_t* pCalibDbv2 = AtmoCfgParam->com.u.prepare.calibv2;
    pAtmoCtx->width = AtmoCfgParam->rawWidth;
    pAtmoCtx->height = AtmoCfgParam->rawHeight;

    if (AtmoCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAtmoCtx->FrameNumber = LINEAR_NUM;
    else if (AtmoCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AtmoCfgParam->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAtmoCtx->FrameNumber = HDR_2X_NUM;
    else
        pAtmoCtx->FrameNumber = HDR_3X_NUM;

    //get aec delay frame
    CalibDb_Aec_ParaV2_t* aec =
        (CalibDb_Aec_ParaV2_t*)CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbv2, ae_calib);
    pAtmoCtx->CurrAeResult.AecDelayframe = MAX(aec->CommCtrl.AecDelayFrmNum.BlackDelay,
                                           aec->CommCtrl.AecDelayFrmNum.WhiteDelay);

    LOG1_ATMO("%s:AecDelayframe:%d\n", __FUNCTION__, pAtmoCtx->CurrAeResult.AecDelayframe);

    CalibDbV2_tmo_t* calibv2_atmo_calib =
        (CalibDbV2_tmo_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDbv2, atmo_calib));

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            return XCAM_RETURN_NO_ERROR;
        }
        LOGI_ATMO("%s: Ahdr Reload Para!\n", __FUNCTION__);
        memcpy(&pAtmoCtx->pCalibDB, calibv2_atmo_calib, sizeof(CalibDbV2_tmo_t));//load iq paras
    }

    if(/* !params->u.prepare.reconfig*/true) {
        AtmoStop(pAtmoCtx); // stop firstly for re-preapre
        ret = AtmoStart(pAtmoCtx);
        if (ret != ATMO_RET_SUCCESS) {
            LOGE_ATMO("%s Atmo Start failed: %d", __FUNCTION__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
    }

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AtmoPreProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    RESULT ret = ATMO_RET_SUCCESS;

    AtmoHandle_t pAtmoCtx = inparams->ctx->AtmoInstConfig.hAtmo;

    if(pAtmoCtx->tmoAttr.opMode == TMO_OPMODE_TOOL) {
        newMalloc(&pAtmoCtx->AtmoConfig, &pAtmoCtx->tmoAttr.stTool);
        AtmoUpdateConfig(pAtmoCtx, &pAtmoCtx->tmoAttr.stTool);
    }
    else {
        newMalloc(&pAtmoCtx->AtmoConfig, &pAtmoCtx->pCalibDB);
        AtmoUpdateConfig(pAtmoCtx, &pAtmoCtx->pCalibDB);
    }

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AtmoProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    RESULT ret = ATMO_RET_SUCCESS;

    AtmoHandle_t pAtmoCtx = (AtmoHandle_t)inparams->ctx->AtmoInstConfig.hAtmo;
    RkAiqAlgoProcAtmo* AtmoParams = (RkAiqAlgoProcAtmo*)inparams;
    RkAiqAlgoProcResAtmo* AtmoProcResParams = (RkAiqAlgoProcResAtmo*)outparams;
    AtmoGetStats(pAtmoCtx, &AtmoParams->ispAtmoStats);

    //get Sensor Info
    XCamVideoBuffer* xCamAeProcRes = AtmoParams->com.u.proc.res_comb->ae_proc_res;
    RkAiqAlgoProcResAe* pAEProcRes = NULL;
    if (xCamAeProcRes) {
        pAEProcRes = (RkAiqAlgoProcResAe*)xCamAeProcRes->map(xCamAeProcRes);
        AtmoGetSensorInfo(pAtmoCtx, pAEProcRes->ae_proc_res_rk);
    } else {
        AecProcResult_t AeProcResult;
        memset(&AeProcResult, 0x0, sizeof(AecProcResult_t));
        LOGW_ATMO("%s: Ae Proc result is null!!!\n", __FUNCTION__);
        AtmoGetSensorInfo(pAtmoCtx, AeProcResult);
    }

    //get ae pre res and proc
    XCamVideoBuffer* xCamAePreRes = AtmoParams->com.u.proc.res_comb->ae_pre_res;
    RkAiqAlgoPreResAe* pAEPreRes = NULL;
    if (xCamAePreRes) {
        pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
    }

    // TODO: get af pre result
    af_preprocess_result_t* af_pre_res_int = NULL;

    if (pAEPreRes && af_pre_res_int)
        AtmoProcessing(pAtmoCtx,
                       pAEPreRes->ae_pre_res_rk,
                       *af_pre_res_int);
    else if (pAEPreRes) {
        af_preprocess_result_t AfPreResult;
        memset(&AfPreResult, 0x0, sizeof(af_preprocess_result_t));
        LOGW_ATMO("%s: af Pre result is null!!!\n", __FUNCTION__);
        AtmoProcessing(pAtmoCtx,
                       pAEPreRes->ae_pre_res_rk,
                       AfPreResult);
    }
    else {
        AecPreResult_t AecHdrPreResult;
        af_preprocess_result_t AfPreResult;
        memset(&AfPreResult, 0x0, sizeof(af_preprocess_result_t));
        memset(&AecHdrPreResult, 0x0, sizeof(AecPreResult_t));
        LOGW_ATMO("%s: ae/af Pre result is null!!!\n", __FUNCTION__);
        AtmoProcessing(pAtmoCtx,
                       AecHdrPreResult,
                       AfPreResult);
    }

    pAtmoCtx->ProcRes.LongFrameMode = pAtmoCtx->SensorInfo.LongFrmMode;
    AtmoProcResParams->AtmoProcRes.LongFrameMode = pAtmoCtx->ProcRes.LongFrameMode;
    AtmoProcResParams->AtmoProcRes.isHdrGlobalTmo = pAtmoCtx->ProcRes.isHdrGlobalTmo;
    AtmoProcResParams->AtmoProcRes.bTmoEn = pAtmoCtx->ProcRes.bTmoEn;
    AtmoProcResParams->AtmoProcRes.isLinearTmo = pAtmoCtx->ProcRes.isLinearTmo;
    memcpy(&AtmoProcResParams->AtmoProcRes.Res, &pAtmoCtx->ProcRes.Res, sizeof(TmoProcRes_t));
    memcpy(&AtmoProcResParams->AtmoProcRes.TmoFlicker, &pAtmoCtx->ProcRes.TmoFlicker, sizeof(TmoFlickerPara_t));

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AtmoPostProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ATMO("%s:Enter!\n", __FUNCTION__);
    RESULT ret = ATMO_RET_SUCCESS;

    //TODO

    LOG1_ATMO("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

RkAiqAlgoDescription g_RkIspAlgoDescAtmo = {
    .common = {
        .version = RKISP_ALGO_ATMO_VERSION,
        .vendor  = RKISP_ALGO_ATMO_VENDOR,
        .description = RKISP_ALGO_ATMO_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ATMO,
        .id      = 0,
        .create_context  = AtmoCreateCtx,
        .destroy_context = AtmoDestroyCtx,
    },
    .prepare = AtmoPrepare,
    .pre_process = AtmoPreProcess,
    .processing = AtmoProcess,
    .post_process = AtmoPostProcess,
};

RKAIQ_END_DECLARE
