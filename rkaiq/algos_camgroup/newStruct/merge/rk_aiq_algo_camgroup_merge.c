/*
 * rk_aiq_algo_camgroup_merge.c
 *
 *  Copyright (c) 2023 Rockchip Corporation
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
#include "newStruct/merge/merge_types_prvt.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef MergeContext_t MergeGroupContext_t;

static XCamReturn groupMergeProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_AMERGE("%s enter", __FUNCTION__ );

    MergeGroupContext_t* pMergeGroupCtx = (MergeGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    if (pMergeGroupCtx->FrameNumber == HDR_2X_NUM || pMergeGroupCtx->FrameNumber == HDR_3X_NUM) {
        //group empty
        if (procParaGroup == NULL || procParaGroup->camgroupParmasArray == NULL) {
            LOGE_AMERGE("procParaGroup or camgroupParmasArray is null");
            return(XCAM_RETURN_ERROR_FAILED);
        }

        // skip group algo if in MANUAL mode
        if (pMergeGroupCtx->merge_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
            return XCAM_RETURN_NO_ERROR;
        }

        if (pMergeGroupCtx->isCapture) {
            LOGE_AMERGE("%s: It's capturing, using pre frame params\n", __func__);
            pMergeGroupCtx->isCapture = false;
            return XCAM_RETURN_NO_ERROR;
        }

        // get LongFrmMode
        RkAiqAlgoProcResAeShared_t* pAEProcRes = &procParaGroup->camgroupParmasArray[0]->aec._aeProcRes;
        pMergeGroupCtx->NextCtrlData.ExpoData.LongFrmMode =
            pAEProcRes->LongFrmMode && (pMergeGroupCtx->FrameNumber != LINEAR_NUM);
        // get motion coef
        pMergeGroupCtx->NextCtrlData.MoveCoef = 0;
        // expo para process
        pMergeGroupCtx->NextCtrlData.ExpoData.SGain =
            procParaGroup->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[0]
                .exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[0]
                .exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[0]
                .exp_real_params.isp_dgain;
        pMergeGroupCtx->NextCtrlData.ExpoData.MGain =
            procParaGroup->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[1]
                .exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[1]
                .exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[1]
                .exp_real_params.isp_dgain;

        pMergeGroupCtx->NextCtrlData.ExpoData.SExpo =
            pMergeGroupCtx->NextCtrlData.ExpoData.SGain *
            procParaGroup->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[0]
                .exp_real_params.integration_time;
        pMergeGroupCtx->NextCtrlData.ExpoData.MExpo =
            pMergeGroupCtx->NextCtrlData.ExpoData.MGain *
            procParaGroup->camgroupParmasArray[0]
                ->aec._effAecExpInfo.HdrExp[1]
                .exp_real_params.integration_time;
        if (pMergeGroupCtx->FrameNumber == HDR_2X_NUM) {
            pMergeGroupCtx->NextCtrlData.ExpoData.LExpo =
                pMergeGroupCtx->NextCtrlData.ExpoData.MExpo;
        } else if (pMergeGroupCtx->FrameNumber == HDR_3X_NUM) {
            pMergeGroupCtx->NextCtrlData.ExpoData.LExpo =
                procParaGroup->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.HdrExp[2]
                    .exp_real_params.analog_gain *
                procParaGroup->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.HdrExp[2]
                    .exp_real_params.digital_gain *
                procParaGroup->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.HdrExp[2]
                    .exp_real_params.isp_dgain *
                procParaGroup->camgroupParmasArray[0]
                    ->aec._effAecExpInfo.HdrExp[2]
                    .exp_real_params.integration_time;
        }
    
        if (pMergeGroupCtx->NextCtrlData.ExpoData.SExpo > FLT_EPSILON)
            pMergeGroupCtx->NextCtrlData.ExpoData.RatioLS =
                pMergeGroupCtx->NextCtrlData.ExpoData.LExpo /
                pMergeGroupCtx->NextCtrlData.ExpoData.SExpo;
        else
            LOGE_AMERGE("%s(%d): Short frame for merge expo sync is ERROR!!!\n", __FUNCTION__,
                        __LINE__);
        if (pMergeGroupCtx->NextCtrlData.ExpoData.MExpo > FLT_EPSILON)
            pMergeGroupCtx->NextCtrlData.ExpoData.RatioLM =
                pMergeGroupCtx->NextCtrlData.ExpoData.LExpo /
                pMergeGroupCtx->NextCtrlData.ExpoData.MExpo;
        else
            LOGE_AMERGE("%s(%d): Middle frame for merge expo sync is ERROR!!!\n", __FUNCTION__,
                        __LINE__);
        //clip for long frame mode
        if (pMergeGroupCtx->NextCtrlData.ExpoData.LongFrmMode) {
            pMergeGroupCtx->NextCtrlData.ExpoData.RatioLS = LONG_FRAME_MODE_RATIO;
            pMergeGroupCtx->NextCtrlData.ExpoData.RatioLM = LONG_FRAME_MODE_RATIO;
        }

        int iso = pMergeGroupCtx->iso;
        rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
        if (scam_3a_res->aec._bEffAecExpValid) {
            iso = scam_3a_res->hdrIso;
        }

        if (procParaGroup->attribUpdated) {
            LOGI("%s attribUpdated", __func__);
            pMergeGroupCtx->isReCal_ = true;
        }

        int delta_iso = abs(iso - pMergeGroupCtx->iso);
        if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
            pMergeGroupCtx->isReCal_ = true;

        // get bypass_expo_process
        bool bypass_expo_process   = true;
        if (pMergeGroupCtx->NextCtrlData.ExpoData.RatioLS >= RATIO_DEFAULT &&
            pMergeGroupCtx->NextCtrlData.ExpoData.RatioLM >= RATIO_DEFAULT) {
            if (pMergeGroupCtx->FrameID <= INIT_CALC_PARAMS_NUM)
                bypass_expo_process = false;
            else if (!pMergeGroupCtx->CurrData.CtrlData.ExpoData.LongFrmMode !=
                     !pMergeGroupCtx->NextCtrlData.ExpoData.LongFrmMode)
                bypass_expo_process = false;
            else if ((pMergeGroupCtx->CurrData.CtrlData.ExpoData.RatioLS -
                      pMergeGroupCtx->NextCtrlData.ExpoData.RatioLS) > FLT_EPSILON ||
                     (pMergeGroupCtx->CurrData.CtrlData.ExpoData.RatioLS -
                      pMergeGroupCtx->NextCtrlData.ExpoData.RatioLS) < -FLT_EPSILON ||
                     (pMergeGroupCtx->CurrData.CtrlData.ExpoData.RatioLM -
                      pMergeGroupCtx->NextCtrlData.ExpoData.RatioLM) > FLT_EPSILON ||
                     (pMergeGroupCtx->CurrData.CtrlData.ExpoData.RatioLM -
                      pMergeGroupCtx->NextCtrlData.ExpoData.RatioLM) < -FLT_EPSILON ||
                     (pMergeGroupCtx->CurrData.CtrlData.ExpoData.SGain -
                      pMergeGroupCtx->NextCtrlData.ExpoData.SGain) > FLT_EPSILON ||
                     (pMergeGroupCtx->CurrData.CtrlData.ExpoData.SGain -
                      pMergeGroupCtx->NextCtrlData.ExpoData.SGain) < -FLT_EPSILON ||
                     (pMergeGroupCtx->CurrData.CtrlData.ExpoData.MGain -
                      pMergeGroupCtx->NextCtrlData.ExpoData.MGain) > FLT_EPSILON ||
                     (pMergeGroupCtx->CurrData.CtrlData.ExpoData.MGain -
                      pMergeGroupCtx->NextCtrlData.ExpoData.MGain) < -FLT_EPSILON)
                bypass_expo_process = false;
            else
                bypass_expo_process = true;
        } else {
            LOGE_AMERGE("%s(%d): AE RatioLS:%f RatioLM:%f for drc expo sync is under one!!!\n",
                        __FUNCTION__, __LINE__, pMergeGroupCtx->NextCtrlData.ExpoData.RatioLS,
                        pMergeGroupCtx->NextCtrlData.ExpoData.RatioLM);
            bypass_expo_process = true;
        }
        
        mge_param_t* merge_param = procResParaGroup->camgroupParmasArray[0]->merge;

        if (pMergeGroupCtx->isReCal_ || !bypass_expo_process) {
            MergeSelectParam(pMergeGroupCtx, merge_param, iso);
			outparams->cfg_update = true;
            outparams->en         = pMergeGroupCtx->merge_attrib->en;
            outparams->bypass     = pMergeGroupCtx->merge_attrib->bypass;
		} else {
			outparams->cfg_update = false;
		}

		void* gp_ptrs[procResParaGroup->arraySize];
		int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->merge);
		for (int i = 0; i < procResParaGroup->arraySize; i++)
			gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->merge;

		algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

        pMergeGroupCtx->iso = iso;
        pMergeGroupCtx->isReCal_ = false;
    } else {
        LOGD_AMERGE("%s FrameID:%d, It's in Linear Mode, Merge function bypass_tuning_process\n",
                    __FUNCTION__, pMergeGroupCtx->FrameID);
    }
    LOGD_AMERGE("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_MERGE_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_MERGE_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_MERGE_DESCRIPTION "Rockchip Merge camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupMerge = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_MERGE_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_MERGE_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_MERGE_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMERGE,
        .id      = 0,
        .create_context  = algo_camgroup_CreateCtx,
        .destroy_context = algo_camgroup_DestroyCtx,
    },
    .prepare = algo_camgroup_Prepare,
    .pre_process = NULL,
    .processing = groupMergeProcessing,
    .post_process = NULL,
};
