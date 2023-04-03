/*
 * rk_aiq_algo_camgroup_amerge_itf.c
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
#include "xcam_log.h"
#if RKAIQ_HAVE_MERGE_V10
#include "amerge/rk_aiq_amerge_algo_v10.h"
#endif
#if RKAIQ_HAVE_MERGE_V11
#include "amerge/rk_aiq_amerge_algo_v11.h"
#endif
#if RKAIQ_HAVE_MERGE_V12
#include "amerge/rk_aiq_amerge_algo_v12.h"
#endif
#include "algos/amerge/rk_aiq_types_amerge_algo_prvt.h"
#include "algos/amerge/rk_aiq_algo_amerge_itf.h"



RKAIQ_BEGIN_DECLARE


static XCamReturn AmergeCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AlgoCtxInstanceCfgCamGroup* instanc_int = (AlgoCtxInstanceCfgCamGroup*)cfg;
    AmergeContext_t* pAmergeGrpCtx = NULL;

    ret = AmergeInit(&pAmergeGrpCtx, (CamCalibDbV2Context_t*)(instanc_int->s_calibv2));

    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_AMERGE("%s(%d) Amerge Init failed: %d", __FUNCTION__, __LINE__, ret);
        return(XCAM_RETURN_ERROR_FAILED);
    }
    *context = (RkAiqAlgoContext *)(pAmergeGrpCtx);

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

static XCamReturn AmergeDestroyCtx(RkAiqAlgoContext *context)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(context != NULL) {
        AmergeContext_t* pAmergeGrpCtx = (AmergeContext_t*)context;
        ret = AmergeRelease(pAmergeGrpCtx);
        if (ret != XCAM_RETURN_NO_ERROR) {
            LOGE_AMERGE("%s(%d) Amerge Release failed: %d", __FUNCTION__, __LINE__, ret);
            return(XCAM_RETURN_ERROR_FAILED);
        }
        context = NULL;
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

static XCamReturn AmergePrepare(RkAiqAlgoCom* params)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AmergeContext_t* pAmergeGrpCtx = (AmergeContext_t*)params->ctx;
    RkAiqAlgoCamGroupPrepare* pAmergePrepParams = (RkAiqAlgoCamGroupPrepare*)params;
    const CamCalibDbV2Context_t* pCalibDb = pAmergePrepParams->s_calibv2;

    if (pAmergePrepParams->gcom.com.u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeGrpCtx->FrameNumber = LINEAR_NUM;
    else if (pAmergePrepParams->gcom.com.u.prepare.working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             pAmergePrepParams->gcom.com.u.prepare.working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeGrpCtx->FrameNumber = HDR_2X_NUM;
    else
        pAmergeGrpCtx->FrameNumber = HDR_3X_NUM;

    if (pAmergeGrpCtx->FrameNumber == HDR_2X_NUM || pAmergeGrpCtx->FrameNumber == HDR_3X_NUM) {
        if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
            LOGI_AMERGE("%s: Amerge Reload Para!\n", __FUNCTION__);
#if RKAIQ_HAVE_MERGE_V10
            CalibDbV2_merge_v10_t* calibv2_amerge_calib =
                (CalibDbV2_merge_v10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeGrpCtx->mergeAttrV10.stAuto, calibv2_amerge_calib,
                   sizeof(CalibDbV2_merge_v10_t));
#endif
#if RKAIQ_HAVE_MERGE_V11
            CalibDbV2_merge_v11_t* calibv2_amerge_calib =
                (CalibDbV2_merge_v11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeGrpCtx->mergeAttrV11.stAuto, calibv2_amerge_calib,
                   sizeof(CalibDbV2_merge_v11_t));
#endif
#if RKAIQ_HAVE_MERGE_V12
            CalibDbV2_merge_v12_t* calibv2_amerge_calib =
                (CalibDbV2_merge_v12_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeGrpCtx->mergeAttrV12.stAuto, calibv2_amerge_calib,
                   sizeof(CalibDbV2_merge_v12_t));
#endif
            pAmergeGrpCtx->ifReCalcStAuto = true;
        } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
            pAmergeGrpCtx->isCapture = true;
        }

        if (/* !params->u.prepare.reconfig*/ true) {
            AmergeStop(pAmergeGrpCtx);  // stop firstly for re-preapre
            ret = AmergeStart(pAmergeGrpCtx);
            if (ret != XCAM_RETURN_NO_ERROR) {
                LOGE_AMERGE("%s(%d) Amerge Start failed: %d\n", __FUNCTION__, __LINE__, ret);
                return (XCAM_RETURN_ERROR_FAILED);
            }
        }
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

static XCamReturn AmergeProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    bool bypass_tuning_process = true;
    bool bypass_expo_process   = true;

    AmergeContext_t* pAmergeGrpCtx = (AmergeContext_t*)inparams->ctx;
    pAmergeGrpCtx->FrameID                      = inparams->frame_id;
    RkAiqAlgoCamGroupProcIn* pAmergeGrpParams = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* pAmergeGrpProcRes = (RkAiqAlgoCamGroupProcOut*)outparams;

    if (pAmergeGrpCtx->FrameNumber == HDR_2X_NUM || pAmergeGrpCtx->FrameNumber == HDR_3X_NUM) {
        LOGD_AMERGE("/#####################################Amerge Group Start#####################################/ \n");

        if (pAmergeGrpCtx->isCapture) {
            LOGD_AMERGE("%s: It's capturing, using pre frame params\n", __func__);
            pAmergeGrpCtx->isCapture = false;
        } else {
            // get LongFrmMode
            XCamVideoBuffer* xCamAeProcRes =
                pAmergeGrpParams->camgroupParmasArray[0]->aec._aeProcRes;
            RkAiqAlgoProcResAe* pAEProcRes = NULL;
            if (xCamAeProcRes) {
                pAEProcRes = (RkAiqAlgoProcResAe*)xCamAeProcRes->map(xCamAeProcRes);
                pAmergeGrpCtx->NextData.CtrlData.ExpoData.LongFrmMode =
                    pAEProcRes->ae_proc_res_rk.LongFrmMode &&
                    (pAmergeGrpCtx->FrameNumber != LINEAR_NUM);
            } else {
                AecProcResult_t AeProcResult;
                memset(&AeProcResult, 0x0, sizeof(AecProcResult_t));
                LOGW_AMERGE("%s: Ae Proc result is null!!!\n", __FUNCTION__);
                pAmergeGrpCtx->NextData.CtrlData.ExpoData.LongFrmMode = false;
            }

        //get ae pre res and proc
        XCamVideoBuffer* xCamAePreRes = pAmergeGrpParams->camgroupParmasArray[0]->aec._aePreRes;
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            bypass_tuning_process = AmergeByPassProcessing(pAmergeGrpCtx, pAEPreRes->ae_pre_res_rk);
        }
        else {
                AecPreResult_t AecHdrPreResult;
                memset(&AecHdrPreResult, 0x0, sizeof(AecPreResult_t));
                bypass_tuning_process = AmergeByPassProcessing(pAmergeGrpCtx, AecHdrPreResult);
                bypass_tuning_process = false;
                LOGW_AMERGE("%s: ae Pre result is null!!!\n", __FUNCTION__);
        }

        //expo para process
        pAmergeGrpCtx->NextData.CtrlData.ExpoData.SGain = pAmergeGrpParams->camgroupParmasArray[0]
                                                              ->aec._effAecExpInfo.HdrExp[0]
                                                              .exp_real_params.analog_gain *
                                                          pAmergeGrpParams->camgroupParmasArray[0]
                                                              ->aec._effAecExpInfo.HdrExp[0]
                                                              .exp_real_params.digital_gain *
                                                          pAmergeGrpParams->camgroupParmasArray[0]
                                                              ->aec._effAecExpInfo.HdrExp[0]
                                                              .exp_real_params.isp_dgain;
        pAmergeGrpCtx->NextData.CtrlData.ExpoData.MGain = pAmergeGrpParams->camgroupParmasArray[0]
                                                              ->aec._effAecExpInfo.HdrExp[1]
                                                              .exp_real_params.analog_gain *
                                                          pAmergeGrpParams->camgroupParmasArray[0]
                                                              ->aec._effAecExpInfo.HdrExp[1]
                                                              .exp_real_params.digital_gain *
                                                          pAmergeGrpParams->camgroupParmasArray[0]
                                                              ->aec._effAecExpInfo.HdrExp[1]
                                                              .exp_real_params.isp_dgain;

        pAmergeGrpCtx->NextData.CtrlData.ExpoData.SExpo =
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.SGain *
            pAmergeGrpParams->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[0]
                .exp_real_params.integration_time;
        pAmergeGrpCtx->NextData.CtrlData.ExpoData.MExpo =
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.MGain *
            pAmergeGrpParams->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[1]
                .exp_real_params.integration_time;
        if (pAmergeGrpCtx->FrameNumber == HDR_2X_NUM) {
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.LExpo =
                pAmergeGrpCtx->NextData.CtrlData.ExpoData.MExpo;
        } else if (pAmergeGrpCtx->FrameNumber == HDR_3X_NUM) {
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.LExpo =
                pAmergeGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.HdrExp[2]
                    .exp_real_params.analog_gain *
                pAmergeGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.HdrExp[2]
                    .exp_real_params.digital_gain *
                pAmergeGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.HdrExp[2]
                    .exp_real_params.isp_dgain *
                pAmergeGrpParams->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.HdrExp[2]
                    .exp_real_params.integration_time;
        }
        pAmergeGrpCtx->NextData.CtrlData.ExpoData.ISO =
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.MGain * ISOMIN;
        pAmergeGrpCtx->NextData.CtrlData.ExpoData.ISO =
            LIMIT_VALUE(pAmergeGrpCtx->NextData.CtrlData.ExpoData.ISO, ISOMAX, ISOMIN);
        LOGV_AMERGE("%s: nextFrame: sexp: %f-%f, mexp: %f-%f, lexp: %f-%f\n", __FUNCTION__,
                    pAmergeGrpCtx->NextData.CtrlData.ExpoData.SGain,
                    pAmergeGrpParams->camgroupParmasArray[0]
                        ->aec._effAecExpInfo.HdrExp[0]
                        .exp_real_params.integration_time,
                    pAmergeGrpCtx->NextData.CtrlData.ExpoData.MGain,
                    pAmergeGrpParams->camgroupParmasArray[0]
                        ->aec._effAecExpInfo.HdrExp[1]
                        .exp_real_params.integration_time,
                    pAmergeGrpParams->camgroupParmasArray[0]
                            ->aec._effAecExpInfo.HdrExp[2]
                            .exp_real_params.analog_gain *
                        pAmergeGrpParams->camgroupParmasArray[0]
                            ->aec._effAecExpInfo.HdrExp[2]
                            .exp_real_params.digital_gain *
                        pAmergeGrpParams->camgroupParmasArray[0]
                            ->aec._effAecExpInfo.HdrExp[2]
                            .exp_real_params.isp_dgain,
                    pAmergeGrpParams->camgroupParmasArray[0]
                        ->aec._effAecExpInfo.HdrExp[2]
                        .exp_real_params.integration_time);
        if (pAmergeGrpCtx->NextData.CtrlData.ExpoData.SExpo > FLT_EPSILON)
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLS =
                pAmergeGrpCtx->NextData.CtrlData.ExpoData.LExpo /
                pAmergeGrpCtx->NextData.CtrlData.ExpoData.SExpo;
        else
            LOGE_AMERGE("%s(%d): Short frame for merge expo sync is ERROR!!!\n", __FUNCTION__,
                        __LINE__);
        if (pAmergeGrpCtx->NextData.CtrlData.ExpoData.MExpo > FLT_EPSILON)
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLM =
                pAmergeGrpCtx->NextData.CtrlData.ExpoData.LExpo /
                pAmergeGrpCtx->NextData.CtrlData.ExpoData.MExpo;
        else
            LOGE_AMERGE("%s(%d): Middle frame for merge expo sync is ERROR!!!\n", __FUNCTION__,
                        __LINE__);
        //clip for long frame mode
        if (pAmergeGrpCtx->NextData.CtrlData.ExpoData.LongFrmMode) {
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLS = LONG_FRAME_MODE_RATIO;
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLM = LONG_FRAME_MODE_RATIO;
        }

        if (pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLS >= RATIO_DEFAULT &&
            pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLM >= RATIO_DEFAULT) {
            if (pAmergeGrpCtx->FrameID <= 2)
                bypass_expo_process = false;
            else if (pAmergeGrpCtx->ifReCalcStAuto || pAmergeGrpCtx->ifReCalcStManual)
                bypass_expo_process = false;
            else if (!pAmergeGrpCtx->CurrData.CtrlData.ExpoData.LongFrmMode !=
                     !pAmergeGrpCtx->NextData.CtrlData.ExpoData.LongFrmMode)
                bypass_expo_process = false;
#if RKAIQ_HAVE_MERGE_V10
            else if (pAmergeGrpCtx->CurrData.CtrlData.ExpoData.RatioLS !=
                         pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLS ||
                     pAmergeGrpCtx->CurrData.CtrlData.ExpoData.RatioLM !=
                         pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLM)
                bypass_expo_process = false;
#endif
#if RKAIQ_HAVE_MERGE_V11 || RKAIQ_HAVE_MERGE_V12
            else if (pAmergeGrpCtx->CurrData.CtrlData.ExpoData.RatioLS !=
                         pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLS ||
                     pAmergeGrpCtx->CurrData.CtrlData.ExpoData.RatioLM !=
                         pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLM ||
                     pAmergeGrpCtx->CurrData.CtrlData.ExpoData.SGain !=
                         pAmergeGrpCtx->NextData.CtrlData.ExpoData.SGain ||
                     pAmergeGrpCtx->CurrData.CtrlData.ExpoData.MGain !=
                         pAmergeGrpCtx->NextData.CtrlData.ExpoData.MGain)
                bypass_expo_process = false;
#endif
            else
                bypass_expo_process = true;
        } else {
            LOGE_AMERGE("%s(%d): AE RatioLS:%f RatioLM:%f for drc expo sync is under one!!!\n",
                        __FUNCTION__, __LINE__, pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLS,
                        pAmergeGrpCtx->NextData.CtrlData.ExpoData.RatioLM);
            bypass_expo_process = true;
        }

        // get tuning para process
        if (!bypass_tuning_process) AmergeTuningProcessing(pAmergeGrpCtx);
        // get expo para process
        if (!bypass_expo_process)
            AmergeExpoProcessing(pAmergeGrpCtx, &pAmergeGrpCtx->NextData.CtrlData.ExpoData);

        pAmergeGrpCtx->ifReCalcStAuto   = false;
        pAmergeGrpCtx->ifReCalcStManual = false;
        pAmergeGrpCtx->ProcRes.update   = !bypass_tuning_process || !bypass_expo_process;
        }

        LOGD_AMERGE(
            "/#####################################Amerge Group "
            "Over#####################################/ \n");
    } else {
        LOGD_AMERGE("%s FrameID:%d, It's in Linear Mode, Merge function bypass_tuning_process\n",
                    __FUNCTION__, pAmergeGrpCtx->FrameID);
    }

    for (int i = 0; i < pAmergeGrpProcRes->arraySize; i++) {
        pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->update =
            pAmergeGrpCtx->ProcRes.update;
        if (pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->update) {
#if RKAIQ_HAVE_MERGE_V10
            memcpy(&pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->Merge_v10,
                   &pAmergeGrpCtx->ProcRes.Merge_v10, sizeof(MgeProcResV10_t));
#endif
#if RKAIQ_HAVE_MERGE_V11
            memcpy(&pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->Merge_v11,
                   &pAmergeGrpCtx->ProcRes.Merge_v11, sizeof(MgeProcResV11_t));
#endif
#if RKAIQ_HAVE_MERGE_V12
            memcpy(&pAmergeGrpProcRes->camgroupParmasArray[i]->_amergeConfig->Merge_v12,
                   &pAmergeGrpCtx->ProcRes.Merge_v12, sizeof(MgeProcResV12_t));
#endif
        }
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(ret);
}

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAmerge = {
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
    .pre_process = NULL,
    .processing = AmergeProcess,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
