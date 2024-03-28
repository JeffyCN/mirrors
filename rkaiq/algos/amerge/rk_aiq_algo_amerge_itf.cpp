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
#if RKAIQ_HAVE_MERGE_V10
#include "amerge/rk_aiq_amerge_algo_v10.h"
#endif
#if RKAIQ_HAVE_MERGE_V11
#include "amerge/rk_aiq_amerge_algo_v11.h"
#endif
#if RKAIQ_HAVE_MERGE_V12
#include "amerge/rk_aiq_amerge_algo_v12.h"
#endif
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
        LOGE_AMERGE("%s(%d) Amerge Init failed: %d", __FUNCTION__, __LINE__, ret);
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
            LOGE_AMERGE("%s(%d) Amerge Release failed: %d", __FUNCTION__, __LINE__, ret);
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
    RkAiqAlgoConfigAmerge* AmergeCfgParam = (RkAiqAlgoConfigAmerge*)params;
    const CamCalibDbV2Context_t* pCalibDb = AmergeCfgParam->com.u.prepare.calibv2;

    if (AmergeCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeCtx->FrameNumber = LINEAR_NUM;
    else if (AmergeCfgParam->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             AmergeCfgParam->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pAmergeCtx->FrameNumber = HDR_2X_NUM;
    else {
        pAmergeCtx->FrameNumber = HDR_3X_NUM;
    }

    if (pAmergeCtx->FrameNumber == HDR_2X_NUM || pAmergeCtx->FrameNumber == HDR_3X_NUM) {
        if (!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB)) {
            // just update calib ptr
            if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
                return XCAM_RETURN_NO_ERROR;
            }
            LOGI_AMERGE("%s: Amerge Reload Para!\n", __FUNCTION__);
#if RKAIQ_HAVE_MERGE_V10
            CalibDbV2_merge_v10_t* calibv2_amerge_calib =
                (CalibDbV2_merge_v10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeCtx->mergeAttrV10.stAuto, calibv2_amerge_calib,
                   sizeof(CalibDbV2_merge_v10_t));
#endif
#if RKAIQ_HAVE_MERGE_V11
            CalibDbV2_merge_v11_t* calibv2_amerge_calib =
                (CalibDbV2_merge_v11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeCtx->mergeAttrV11.stAuto, calibv2_amerge_calib,
                   sizeof(CalibDbV2_merge_v11_t));
#endif
#if RKAIQ_HAVE_MERGE_V12
            CalibDbV2_merge_v12_t* calibv2_amerge_calib =
                (CalibDbV2_merge_v12_t*)(CALIBDBV2_GET_MODULE_PTR((void*)pCalibDb, amerge_calib));
            memcpy(&pAmergeCtx->mergeAttrV12.stAuto, calibv2_amerge_calib,
                   sizeof(CalibDbV2_merge_v12_t));
#endif
            pAmergeCtx->ifReCalcStAuto = true;
        } else if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_CHANGERES) {
            pAmergeCtx->isCapture = true;
        }

        if (/* !params->u.prepare.reconfig*/ true) {
            AmergeStop(pAmergeCtx);  // stop firstly for re-preapre
            ret = AmergeStart(pAmergeCtx);
            if (ret != XCAM_RETURN_NO_ERROR) {
                LOGE_AMERGE("%s(%d) Amerge Start failed: %d\n", __FUNCTION__, __LINE__, ret);
                return (XCAM_RETURN_ERROR_FAILED);
            }
        }
    }

    LOG1_AMERGE("%s:Exit!\n", __FUNCTION__);
    return(XCAM_RETURN_NO_ERROR);
}

static XCamReturn AmergeProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_AMERGE("%s:Enter!\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    bool bypass_tuning_process = true;
    bool bypass_expo_process   = true;

    AmergeContext_t* pAmergeCtx = (AmergeContext_t*)inparams->ctx;
    pAmergeCtx->FrameID                    = inparams->frame_id;
    RkAiqAlgoProcAmerge* pAmergeParams = (RkAiqAlgoProcAmerge*)inparams;
    RkAiqAlgoProcResAmerge* pAmergeProcRes = (RkAiqAlgoProcResAmerge*)outparams;

    if(pAmergeCtx->FrameNumber == HDR_2X_NUM || pAmergeCtx->FrameNumber == HDR_3X_NUM) {
        LOGD_AMERGE("%s:/#####################################Amerge Start#####################################/ \n", __func__);

        if (pAmergeCtx->isCapture) {
            LOGD_AMERGE("%s: It's capturing, using pre frame params\n", __func__);
            pAmergeCtx->isCapture = false;
        } else {
            // get LongFrmMode
            XCamVideoBuffer* xCamAeProcRes = pAmergeParams->com.u.proc.res_comb->ae_proc_res;
            pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode =
                pAmergeParams->LongFrmMode &&
                (pAmergeCtx->FrameNumber != LINEAR_NUM);

            // get ae pre res
            XCamVideoBuffer* xCamAePreRes = pAmergeParams->com.u.proc.res_comb->ae_pre_res;
            RkAiqAlgoPreResAe* pAEPreRes  = NULL;
            if (xCamAePreRes) {
                pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
                if (pAmergeCtx->FrameNumber == LINEAR_NUM)
                    pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv =
                        pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
                else if (pAmergeCtx->FrameNumber == HDR_2X_NUM ||
                         pAmergeCtx->FrameNumber == HDR_3X_NUM)
                    pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv =
                        pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                else
                    pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv = ENVLVMIN;
                // Normalize the current envLv for AEC
                pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv =
                    (pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv - MIN_ENV_LV) /
                    (MAX_ENV_LV - MIN_ENV_LV);
                pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv =
                    LIMIT_VALUE(pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv, ENVLVMAX, ENVLVMIN);
            } else {
                pAmergeCtx->NextData.CtrlData.ExpoData.EnvLv = ENVLVMIN;
                LOGW_AMERGE("%s: ae Pre result is null!!!\n", __FUNCTION__);
            }

            // get motion coef
            pAmergeCtx->NextData.CtrlData.MoveCoef = MOVE_COEF_DEFAULT;

            // expo para process
            pAmergeCtx->NextData.CtrlData.ExpoData.SGain =
                pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain;
            pAmergeCtx->NextData.CtrlData.ExpoData.MGain =
                pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain;

            pAmergeCtx->NextData.CtrlData.ExpoData.SExpo =
                pAmergeCtx->NextData.CtrlData.ExpoData.SGain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
            pAmergeCtx->NextData.CtrlData.ExpoData.MExpo =
                pAmergeCtx->NextData.CtrlData.ExpoData.MGain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
            if (pAmergeCtx->NextData.CtrlData.ExpoData.SExpo < FLT_EPSILON) {
                pAmergeCtx->NextData.CtrlData.ExpoData.SGain =
                    pAmergeParams->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain;
                pAmergeCtx->NextData.CtrlData.ExpoData.MGain =
                    pAmergeParams->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain;

                pAmergeCtx->NextData.CtrlData.ExpoData.SExpo =
                    pAmergeCtx->NextData.CtrlData.ExpoData.SGain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
                pAmergeCtx->NextData.CtrlData.ExpoData.MExpo =
                    pAmergeCtx->NextData.CtrlData.ExpoData.MGain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
            }
        if (pAmergeCtx->FrameNumber == HDR_2X_NUM) {
            pAmergeCtx->NextData.CtrlData.ExpoData.LExpo =
                pAmergeCtx->NextData.CtrlData.ExpoData.MExpo;
        } else if (pAmergeCtx->FrameNumber == HDR_3X_NUM) {
            pAmergeCtx->NextData.CtrlData.ExpoData.LExpo =
                pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.isp_dgain *
                pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time;
            if (pAmergeCtx->NextData.CtrlData.ExpoData.SExpo < FLT_EPSILON) {
                pAmergeCtx->NextData.CtrlData.ExpoData.LExpo =
                    pAmergeParams->com.u.proc.curExp->HdrExp[2].exp_real_params.analog_gain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[2].exp_real_params.digital_gain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[2].exp_real_params.isp_dgain *
                    pAmergeParams->com.u.proc.curExp->HdrExp[2].exp_real_params.integration_time;
            }
        }
        pAmergeCtx->NextData.CtrlData.ExpoData.ISO =
            pAmergeCtx->NextData.CtrlData.ExpoData.MGain * ISOMIN;
        pAmergeCtx->NextData.CtrlData.ExpoData.ISO =
            LIMIT_VALUE(pAmergeCtx->NextData.CtrlData.ExpoData.ISO, ISOMAX, ISOMIN);
        LOGV_AMERGE("%s: nextFrame: sexp: %f-%f, mexp: %f-%f, lexp: %f-%f\n", __FUNCTION__,
                    pAmergeCtx->NextData.CtrlData.ExpoData.SGain,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time,
                    pAmergeCtx->NextData.CtrlData.ExpoData.MGain,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
                        pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
                        pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.isp_dgain,
                    pAmergeParams->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time);
        if (pAmergeCtx->NextData.CtrlData.ExpoData.SExpo > FLT_EPSILON) {
            pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS =
                pAmergeCtx->NextData.CtrlData.ExpoData.LExpo /
                pAmergeCtx->NextData.CtrlData.ExpoData.SExpo;
            if (pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS < RATIO_DEFAULT) {
                LOGE_AMERGE("%s: Next RatioLS:%f is less than 1.0x, clip to 1.0x!!!\n",
                            __FUNCTION__, pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS);
                pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS = RATIO_DEFAULT;
            }
        } else
            LOGE_AMERGE("%s(%d): Short frame for merge expo sync is ERROR!!!\n", __FUNCTION__,
                        __LINE__);
        if (pAmergeCtx->NextData.CtrlData.ExpoData.MExpo > FLT_EPSILON) {
            pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM =
                pAmergeCtx->NextData.CtrlData.ExpoData.LExpo /
                pAmergeCtx->NextData.CtrlData.ExpoData.MExpo;
            if (pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM < RATIO_DEFAULT) {
                LOGE_AMERGE("%s: Next RatioLM:%f is less than 1.0x, clip to 1.0x!!!\n",
                            __FUNCTION__, pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM);
                pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM = RATIO_DEFAULT;
            }
        } else
            LOGE_AMERGE("%s(%d): Middle frame for merge expo sync is ERROR!!!\n", __FUNCTION__,
                        __LINE__);
        //clip for long frame mode
        if (pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode) {
            pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS = LONG_FRAME_MODE_RATIO;
            pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM = LONG_FRAME_MODE_RATIO;
        }
        // clip L2M_ratio to 32x
        if (pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM > AE_RATIO_L2M_MAX) {
            LOGE_AMERGE("%s: Next L2M_ratio:%f out of range, clip to 32.0x!!!\n", __FUNCTION__,
                        pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM);
            pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM = AE_RATIO_L2M_MAX;
        }
        // clip L2S_ratio
        if (pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS > AE_RATIO_MAX) {
            LOGE_AMERGE("%s: Next RatioLS:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                        pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS);
            pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS = AE_RATIO_MAX;
        }

        // get bypass_tuning_process
        bypass_tuning_process = AmergeByPassProcessing(pAmergeCtx);

        // get bypass_expo_process
        if (pAmergeCtx->FrameID <= INIT_CALC_PARAMS_NUM)
            bypass_expo_process = false;
        else if (pAmergeCtx->ifReCalcStAuto || pAmergeCtx->ifReCalcStManual)
            bypass_expo_process = false;
        else if (!pAmergeCtx->CurrData.CtrlData.ExpoData.LongFrmMode !=
                 !pAmergeCtx->NextData.CtrlData.ExpoData.LongFrmMode)
            bypass_expo_process = false;
#if RKAIQ_HAVE_MERGE_V10
            else if ((pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLS -
                      pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS) > FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLS -
                      pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS) < -FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLM -
                      pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM) > FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLM -
                      pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM) < -FLT_EPSILON)
                bypass_expo_process = false;
#endif
#if RKAIQ_HAVE_MERGE_V11 || RKAIQ_HAVE_MERGE_V12
            else if ((pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLS -
                      pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS) > FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLS -
                      pAmergeCtx->NextData.CtrlData.ExpoData.RatioLS) < -FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLM -
                      pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM) > FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.RatioLM -
                      pAmergeCtx->NextData.CtrlData.ExpoData.RatioLM) < -FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.SGain -
                      pAmergeCtx->NextData.CtrlData.ExpoData.SGain) > FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.SGain -
                      pAmergeCtx->NextData.CtrlData.ExpoData.SGain) < -FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.MGain -
                      pAmergeCtx->NextData.CtrlData.ExpoData.MGain) > FLT_EPSILON ||
                     (pAmergeCtx->CurrData.CtrlData.ExpoData.MGain -
                      pAmergeCtx->NextData.CtrlData.ExpoData.MGain) < -FLT_EPSILON)
                bypass_expo_process = false;
#endif
            else
                bypass_expo_process = true;

        // get tuning para process
        if (!bypass_expo_process || !bypass_tuning_process)
            AmergeTuningProcessing(pAmergeCtx, pAmergeProcRes->AmergeProcRes);

        // get expo para process
        if (!bypass_expo_process || !bypass_tuning_process)
            AmergeExpoProcessing(pAmergeCtx, &pAmergeCtx->NextData.CtrlData.ExpoData,
                                 pAmergeProcRes->AmergeProcRes);
        }

        if (pAmergeCtx->ifReCalcStAuto) pAmergeCtx->ifReCalcStAuto = false;
        if (pAmergeCtx->ifReCalcStManual) pAmergeCtx->ifReCalcStManual = false;
        outparams->cfg_update = !bypass_tuning_process || !bypass_expo_process;
        LOGD_AMERGE(
            "%s:/#####################################Amerge "
            "Over#####################################/ \n",
            __func__);
    } else {
        // TODO: disable merge
        if (inparams->u.proc.init)
            outparams->cfg_update = true;
        else
            outparams->cfg_update = false;
        LOGD_AMERGE("%s FrameID:%d, It's in Linear Mode, Merge function bypass_tuning_process\n",
                    __FUNCTION__, pAmergeCtx->FrameID);
    }

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
    .pre_process = NULL,
    .processing = AmergeProcess,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
