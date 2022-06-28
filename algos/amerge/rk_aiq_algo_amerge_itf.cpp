/*
 * rk_aiq_algo_amerge_itf.c
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

#include "amerge/rk_aiq_algo_amerge_itf.h"
#include "xcam_log.h"
#include "amerge/rk_aiq_amerge_algo.h"
#include "amerge/rk_aiq_types_amerge_algo_prvt.h"
#include "rk_aiq_algo_types.h"


RKAIQ_BEGIN_DECLARE


static XCamReturn AmergeCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{

    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AmergeContext_t* pAmergeCtx = NULL;

    ret = AmergeInit(&pAmergeCtx, (CamCalibDbV2Context_t*)(cfg->calibv2));

    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_AMERGE("%s Amerge Init failed: %d", __FUNCTION__, ret);
        return(XCAM_RETURN_ERROR_FAILED);
    }
    *context = (RkAiqAlgoContext *)(pAmergeCtx);

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

static XCamReturn AmergeDestroyCtx(RkAiqAlgoContext *context)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(context != NULL) {
        AmergeContext_t* pAmergeCtx = (AmergeContext_t*)context;
        ret = AmergeRelease(pAmergeCtx);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE_AMERGE("%s Amerge Release failed: %d", __FUNCTION__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
        context = NULL;
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergePrepare(RkAiqAlgoCom* params)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)params->ctx;
    RkAiqAlgoConfigAmerge* AhdrCfgParam = (RkAiqAlgoConfigAmerge*)params; //come from params in html
    const CamCalibDbV2Context_t* pCalibDb = AhdrCfgParam->com.u.prepare.calibv2;

    if (AhdrCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeCtx->FrameNumber = LINEAR_NUM;
    else if (AhdrCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AhdrCfgParam->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeCtx->FrameNumber = HDR_2X_NUM;
    else
        pAmergeCtx->FrameNumber = HDR_3X_NUM;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        LOGD_AMERGE("%s: Amerge Reload Para!\n", __FUNCTION__);
        if(CHECK_ISP_HW_V21()) {
            CalibDbV2_merge_t* calibv2_amerge_calib =
                (CalibDbV2_merge_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeCtx->pCalibDB.Merge_v20, calibv2_amerge_calib, sizeof(CalibDbV2_merge_t));//load iq paras
        }
        else if(CHECK_ISP_HW_V30()) {
            CalibDbV2_merge_V2_t* calibv2_amerge_calib =
                (CalibDbV2_merge_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeCtx->pCalibDB.Merge_v30, calibv2_amerge_calib, sizeof(CalibDbV2_merge_V2_t));//load iq paras

        }
    }

    if(/* !params->u.prepare.reconfig*/true) {
        AmergeStop(pAmergeCtx); // stop firstly for re-preapre
        ret = AmergeStart(pAmergeCtx);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE_AMERGE("%s Amerge Start failed: %d\n", __FUNCTION__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
    }

    if(pAmergeCtx->FrameNumber == HDR_2X_NUM || pAmergeCtx->FrameNumber == HDR_3X_NUM) {
        MergePrepareJsonMalloc(&pAmergeCtx->Config, &pAmergeCtx->pCalibDB);
        AmergePrepareJsonUpdateConfig(pAmergeCtx, &pAmergeCtx->pCalibDB);
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergePreProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)inparams->ctx;
    RkAiqAlgoConfigAmerge* AhdrCfgParam = (RkAiqAlgoConfigAmerge*)inparams;


    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergeProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    bool bypass = false;

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)inparams->ctx;
    pAmergeCtx->frameCnt = inparams->frame_id > 2 ? (inparams->frame_id - 2) : 0;
    RkAiqAlgoProcAmerge* pAmergeParams = (RkAiqAlgoProcAmerge*)inparams;
    RkAiqAlgoProcResAmerge* pAmergeProcRes = (RkAiqAlgoProcResAmerge*)outparams;

    if(pAmergeCtx->FrameNumber == HDR_2X_NUM || pAmergeCtx->FrameNumber == HDR_3X_NUM) {
        LOGD_AMERGE("%s:/#####################################Amerge Start#####################################/ \n", __func__);

        //update config
        merge_OpModeV21_t mode = MERGE_OPMODE_API_OFF;
        if(CHECK_ISP_HW_V21())
            mode = pAmergeCtx->mergeAttr.attrV21.opMode;
        else if(CHECK_ISP_HW_V30())
            mode = pAmergeCtx->mergeAttr.attrV30.opMode;
        if(mode == MERGE_OPMODE_MANU) {
            MergeProcApiMalloc(&pAmergeCtx->Config, &pAmergeCtx->mergeAttr);
            AmergeProcApiUpdateConfig(pAmergeCtx);
        }

        //get Sensor Info
        XCamVideoBuffer* xCamAeProcRes = pAmergeParams->com.u.proc.res_comb->ae_proc_res;
        RkAiqAlgoProcResAe* pAEProcRes = NULL;
        if (xCamAeProcRes) {
            pAEProcRes = (RkAiqAlgoProcResAe*)xCamAeProcRes->map(xCamAeProcRes);
            AmergeGetSensorInfo(pAmergeCtx, pAEProcRes->ae_proc_res_rk);
        }
        else {
            AecProcResult_t AeProcResult;
            memset(&AeProcResult, 0x0, sizeof(AecProcResult_t));
            LOGE_AMERGE("%s: Ae Proc result is null!!!\n", __FUNCTION__);
            AmergeGetSensorInfo(pAmergeCtx, AeProcResult);
        }

        //get ae pre res and proc
        XCamVideoBuffer* xCamAePreRes = pAmergeParams->com.u.proc.res_comb->ae_pre_res;
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            bypass = AmergeByPassProcessing(pAmergeCtx, pAEPreRes->ae_pre_res_rk);
        }
        else {
            AecPreResult_t AecHdrPreResult;
            memset(&AecHdrPreResult, 0x0, sizeof(AecPreResult_t));
            bypass = AmergeByPassProcessing(pAmergeCtx, AecHdrPreResult);
            bypass = false;
            LOGE_AMERGE("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        //merge tuning para process
        if(!bypass)
            AmergeTuningProcessing(pAmergeCtx);

        //expo para process
        MergeExpoData_t ExpoData;
        memset(&ExpoData, 0, sizeof(MergeExpoData_t));
        if(pAmergeCtx->FrameNumber == HDR_2X_NUM) {
            ExpoData.nextSExpo = pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
                                 pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
            ExpoData.nextMExpo = pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                                 pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
            ExpoData.nextLExpo = ExpoData.nextMExpo;

            ExpoData.nextSGain = pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain;
            ExpoData.nextMGain = pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain;

        }
        else if(pAmergeCtx->FrameNumber == HDR_3X_NUM) {
            ExpoData.nextSExpo = pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
                                 pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
            ExpoData.nextMExpo = pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                                 pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
            ExpoData.nextLExpo = pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
                                 pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time;

            ExpoData.nextSGain = pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain;
            ExpoData.nextMGain = pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain * pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain;
        }
        LOGV_AMERGE("%s: nextFrame: sexp: %f-%f, mexp: %f-%f, lexp: %f-%f\n", __FUNCTION__,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time);
        if(ExpoData.nextSExpo > 0 )
            ExpoData.nextRatioLS = ExpoData.nextLExpo / ExpoData.nextSExpo;
        else
            LOGE_AMERGE("%s: Short frame for merge expo sync is ERROR!!!\n", __FUNCTION__);
        if(ExpoData.nextMExpo > 0 )
            ExpoData.nextRatioLM = ExpoData.nextLExpo / ExpoData.nextMExpo;
        else
            LOGE_AMERGE("%s: Middle frame for merge expo sync is ERROR!!!\n", __FUNCTION__);
        //clip for long frame mode
        if (pAmergeCtx->SensorInfo.LongFrmMode) {
            ExpoData.nextRatioLS = 1.0;
            ExpoData.nextRatioLM = 1.0;
        }

        if(ExpoData.nextRatioLS >= 1 && ExpoData.nextRatioLM >= 1)
            AmergeExpoProcessing(pAmergeCtx, &ExpoData);
        else
            LOGE_AMERGE("%s: AE ratio for merge expo sync is under one!!!\n", __FUNCTION__);

        if(CHECK_ISP_HW_V21())
            pAmergeCtx->PrevData.CtrlData.ApiMode = pAmergeCtx->mergeAttr.attrV21.opMode;
        else if(CHECK_ISP_HW_V30())
            pAmergeCtx->PrevData.CtrlData.ApiMode = pAmergeCtx->mergeAttr.attrV30.opMode;
        pAmergeCtx->ProcRes.update = !bypass;
        pAmergeCtx->ProcRes.LongFrameMode = pAmergeCtx->SensorInfo.LongFrmMode;
        pAmergeProcRes->AmergeProcRes.update = pAmergeCtx->ProcRes.update;
        pAmergeProcRes->AmergeProcRes.LongFrameMode = pAmergeCtx->ProcRes.LongFrameMode;
        if(CHECK_ISP_HW_V21())
            memcpy(&pAmergeProcRes->AmergeProcRes.Merge_v20, &pAmergeCtx->ProcRes.Merge_v20, sizeof(MgeProcRes_t));
        else if(CHECK_ISP_HW_V30())
            memcpy(&pAmergeProcRes->AmergeProcRes.Merge_v30, &pAmergeCtx->ProcRes.Merge_v30, sizeof(MgeProcResV2_t));

        LOGD_AMERGE("%s:/#####################################Amerge Over#####################################/ \n", __func__);
    }
    else
        LOGD_AMERGE("%s FrameID:%d, It's in Linear Mode, Merge function bypass\n", __FUNCTION__, pAmergeCtx->frameCnt);

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergePostProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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
