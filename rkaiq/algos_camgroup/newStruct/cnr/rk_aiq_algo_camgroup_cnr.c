/*
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
#include "newStruct/cnr/include/cnr_algo_api.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

static XCamReturn groupCnrProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter", __FUNCTION__ );

    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == NULL || procParaGroup->camgroupParmasArray == NULL) {
        LOGE_ANR("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    int iso = 50;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];

    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        iso = scam_3a_res->hdrIso;
    }

    outparams->algoRes = procResParaGroup->camgroupParmasArray[0]->cnr;
    Acnr_processing(inparams, outparams, iso);

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->cnr);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->cnr;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);
    LOGD_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_CNR_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_CNR_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_CNR_DESCRIPTION "Rockchip Cnr camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupCnr = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_CNR_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_CNR_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_CNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACNR,
        .id      = 0,
        .create_context  = algo_camgroup_CreateCtx,
        .destroy_context = algo_camgroup_DestroyCtx,
    },
    .prepare = algo_camgroup_Prepare,
    .pre_process = NULL,
    .processing = groupCnrProcessing,
    .post_process = NULL,
};
