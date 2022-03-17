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

#include "rk_aiq_algo_types_int.h"
#include "amerge/rk_aiq_algo_amerge_itf.h"
#include "xcam_log.h"
#include "amerge/rk_aiq_amerge_algo.h"
#include "amerge/rk_aiq_types_amerge_algo_prvt.h"



RKAIQ_BEGIN_DECLARE



static XCamReturn AmergeCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    RESULT ret = AMERGE_RET_SUCCESS;
    RkAiqAlgoContext *ctx = new RkAiqAlgoContext();
    AlgoCtxInstanceCfgInt* instanc_int = (AlgoCtxInstanceCfgInt*)cfg;
    if (ctx == NULL) {
        LOGE_AMERGE( "%s: create amerge context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset(&ctx->AmergeInstConfig, 0x00, sizeof(AmergeInstanceConfig_t));
    ret = AmergeInit(&ctx->AmergeInstConfig, instanc_int->calibv2);

    if (ret != AMERGE_RET_SUCCESS) {
        LOGE_AMERGE("%s Amerge Init failed: %d", __FUNCTION__, ret);
        return(XCAM_RETURN_ERROR_FAILED);
    }

    *context = ctx;

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergeDestroyCtx(RkAiqAlgoContext *context)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    RESULT ret = AMERGE_RET_SUCCESS;

    if(context != NULL) {
        AmergeHandle_t pAmergeCtx = (AmergeHandle_t)context->AmergeInstConfig.hAmerge;
        ret = AmergeRelease(pAmergeCtx);
        if (ret != AMERGE_RET_SUCCESS) {
            LOGE_AMERGE("%s Amerge Release failed: %d", __FUNCTION__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
        delete context;
        context = NULL;
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergePrepare(RkAiqAlgoCom* params)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    RESULT ret = AMERGE_RET_SUCCESS;

    AmergeHandle_t pAmergeCtx = params->ctx->AmergeInstConfig.hAmerge;
    RkAiqAlgoConfigAmergeInt* AhdrCfgParam = (RkAiqAlgoConfigAmergeInt*)params; //come from params in html
    const CamCalibDbV2Context_t* pCalibDb = AhdrCfgParam->rk_com.u.prepare.calibv2;
    pAmergeCtx->width = AhdrCfgParam->rawWidth;
    pAmergeCtx->height = AhdrCfgParam->rawHeight;

    if (AhdrCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeCtx->FrameNumber = 1;
    else if (AhdrCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AhdrCfgParam->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeCtx->FrameNumber = 2;
    else
        pAmergeCtx->FrameNumber = 3;


    CalibDbV2_merge_t* calibv2_amerge_calib =
        (CalibDbV2_merge_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGD_AMERGE("%s: Amerge Reload Para!\n", __FUNCTION__);
        memcpy(&pAmergeCtx->pCalibDB, calibv2_amerge_calib, sizeof(CalibDbV2_merge_t));//load iq paras
    }

    if(/* !params->u.prepare.reconfig*/true) {
        AmergeStop(pAmergeCtx); // stop firstly for re-preapre
        ret = AmergeStart(pAmergeCtx);
        if (ret != AMERGE_RET_SUCCESS) {
            LOGE_AMERGE("%s Amerge Start failed: %d\n", __FUNCTION__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergePreProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    RESULT ret = AMERGE_RET_SUCCESS;

    AmergeHandle_t pAmergeCtx = inparams->ctx->AmergeInstConfig.hAmerge;
    RkAiqAlgoConfigAmergeInt* AhdrCfgParam = (RkAiqAlgoConfigAmergeInt*)inparams;

    if(pAmergeCtx->mergeAttr.opMode == MERGE_OPMODE_TOOL) {
        MergeNewMalloc(&pAmergeCtx->Config, &pAmergeCtx->mergeAttr.stTool);
        AmergeUpdateConfig(pAmergeCtx, &pAmergeCtx->mergeAttr.stTool);
    }
    else {
        MergeNewMalloc(&pAmergeCtx->Config, &pAmergeCtx->pCalibDB);
        AmergeUpdateConfig(pAmergeCtx, &pAmergeCtx->pCalibDB);
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergeProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    RESULT ret = AMERGE_RET_SUCCESS;
    bool bypass = false;

    AmergeHandle_t pAmergeCtx = (AmergeHandle_t)inparams->ctx->AmergeInstConfig.hAmerge;
    RkAiqAlgoProcAmergeInt* AmergeParams = (RkAiqAlgoProcAmergeInt*)inparams;
    RkAiqAlgoProcResAmergeInt* AmergeProcResParams = (RkAiqAlgoProcResAmergeInt*)outparams;
    // pAmergeCtx->frameCnt = inparams->frame_id;

    //get Sensor Info
    XCamVideoBuffer* xCamAeProcRes = AmergeParams->rk_com.u.proc.res_comb->ae_proc_res;
    RkAiqAlgoProcResAeInt* pAEProcRes = NULL;
    if (xCamAeProcRes) {
        pAEProcRes = (RkAiqAlgoProcResAeInt*)xCamAeProcRes->map(xCamAeProcRes);
        AmergeGetSensorInfo(pAmergeCtx, pAEProcRes->ae_proc_res_rk);
    }
    else {
        AecProcResult_t AeProcResult;
        memset(&AeProcResult, 0x0, sizeof(AecProcResult_t));
        LOGE_AMERGE("%s: Ae Proc result is null!!!\n", __FUNCTION__);
        AmergeGetSensorInfo(pAmergeCtx, AeProcResult);
    }

    //get ae pre res and proc
    XCamVideoBuffer* xCamAePreRes = AmergeParams->rk_com.u.proc.res_comb->ae_pre_res;
    RkAiqAlgoPreResAeInt* pAEPreRes = NULL;
    if (xCamAePreRes) {
        pAEPreRes = (RkAiqAlgoPreResAeInt*)xCamAePreRes->map(xCamAePreRes);
        bypass = AmergeByPassProcessing(pAmergeCtx, pAEPreRes->ae_pre_res_rk);
    }
    else {
        AecPreResult_t AecHdrPreResult;
        memset(&AecHdrPreResult, 0x0, sizeof(AecPreResult_t));
        bypass = AmergeByPassProcessing(pAmergeCtx, AecHdrPreResult);
        bypass = false;
        LOGE_AMERGE("%s: ae Pre result is null!!!\n", __FUNCTION__);
    }

    if(!bypass)
        AmergeProcessing(pAmergeCtx);

    pAmergeCtx->frameCnt++;
    pAmergeCtx->PrevData.CtrlData.ApiMode = pAmergeCtx->mergeAttr.opMode;
    pAmergeCtx->ProcRes.Res.update = !bypass;
    pAmergeCtx->ProcRes.LongFrameMode = pAmergeCtx->SensorInfo.LongFrmMode;
    AmergeProcResParams->AmergeProcRes.LongFrameMode = pAmergeCtx->ProcRes.LongFrameMode;
    memcpy(&AmergeProcResParams->AmergeProcRes.Res, &pAmergeCtx->ProcRes.Res, sizeof(MgeProcRes_t));

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergePostProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    RESULT ret = AMERGE_RET_SUCCESS;

    //TODO

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

RkAiqAlgoDescription g_RkIspAlgoDescAmerge = {
    .common = {
        .version = RKISP_ALGO_AMERGE_VERSION,
        .vendor  = RKISP_ALGO_AMERGE_VENDOR,
        .description = RKISP_ALGO_AMERGE_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMERGE,
        .id      = 0,
        .create_context  = AmergeCreateCtx,
        .destroy_context = AmergeDestroyCtx,
    },
    .prepare = AmergePrepare,
    .pre_process = AmergePreProcess,
    .processing = AmergeProcess,
    .post_process = AmergePostProcess,
};

RKAIQ_END_DECLARE
