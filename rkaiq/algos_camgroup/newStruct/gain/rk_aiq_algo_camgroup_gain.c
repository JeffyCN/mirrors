/*
 * rk_aiq_algo_camgroup_gain.c
 *
 *  Copyright (c) 2024 Rockchip Corporation
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
#include "newStruct/gain/gain_types_prvt.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef GainContext_t GainGroupContext_t;

static XCamReturn groupGainProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter", __FUNCTION__ );

    GainGroupContext_t* pGainGroupCtx = (GainGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == NULL || procParaGroup->camgroupParmasArray == NULL) {
        LOGE_ANR("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pGainGroupCtx->gain_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    int iso = 50;
    float blc_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            iso =  blc_ob_predgain * scam_3a_res->hdrIso;
        } else {
            iso = scam_3a_res->hdrIso;
        }
    }

    outparams->algoRes = procResParaGroup->camgroupParmasArray[0]->gain;
    Again_processing(inparams, outparams, iso);

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->gain);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->gain;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    LOGD_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_GAIN_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_GAIN_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_GAIN_DESCRIPTION "Rockchip Gain camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupGain = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_GAIN_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_GAIN_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_GAIN_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AGAIN,
        .id      = 0,
        .create_context  = algo_camgroup_CreateCtx,
        .destroy_context = algo_camgroup_DestroyCtx,
    },
    .prepare = algo_camgroup_Prepare,
    .pre_process = NULL,
    .processing = groupGainProcessing,
    .post_process = NULL,
};
