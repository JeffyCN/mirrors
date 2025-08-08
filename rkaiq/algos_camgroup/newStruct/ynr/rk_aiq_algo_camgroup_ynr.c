/*
 * rk_aiq_algo_camgroup_ynr.cpp
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
#include "newStruct/ynr/include/ynr_algo_api.h"

extern void algo_camgroup_update_results(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, void** pGpRstArray,int itemSize);

static XCamReturn groupYnrCreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );

    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    *context = cfgInt->pSingleAlgoCtx;
    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;

}

static XCamReturn groupYnrDestroyCtx(RkAiqAlgoContext *context)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );

    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupYnrPrepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ANR("%s enter \n", __FUNCTION__ );

    LOGD_ANR("%s exit ret:%d\n", __FUNCTION__, ret);
    return ret;
}

static XCamReturn groupYnrProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
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

    outparams->algoRes = procResParaGroup->camgroupParmasArray[0]->ynr;
    Aynr_processing(inparams, outparams, iso);

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->ynr);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->ynr;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    LOGD_ANR("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_YNR_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_YNR_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_YNR_DESCRIPTION "Rockchip Ynr camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupYnr = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_YNR_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_YNR_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_YNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AYNR,
        .id      = 0,
        .create_context  = groupYnrCreateCtx,
        .destroy_context = groupYnrDestroyCtx,
    },
    .prepare = groupYnrPrepare,
    .pre_process = NULL,
    .processing = groupYnrProcessing,
    .post_process = NULL,
};
