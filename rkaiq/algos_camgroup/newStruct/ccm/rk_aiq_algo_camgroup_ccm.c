/*
 *  Copyright (c) 2021 Rockchip Corporation
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
#include "newStruct/ccm/ccm_types_prvt.h"
#include "xcam_log.h"
#include "c_base/aiq_base.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_ACCM( "%s: (enter)\n", __FUNCTION__);

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    if (!procParaGroup->arraySize) {
        LOG1_ACCM( "%s: do nothing \n", __FUNCTION__);
        return XCAM_RETURN_NO_ERROR;
    }

    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    RkAiqAlgoProcResAwbShared_t* awb_proc_res = &scam_3a_res->awb._awbProcRes;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    RKAiqAecExpInfo_t* pCurExp = NULL;

    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res;
    }
    if(scam_3a_res->aec._bEffAecExpValid) {
        pCurExp = &scam_3a_res->aec._effAecExpInfo;
    }

    illu_estm_info_t swinfo;
    get_illu_estm_info(&swinfo, awb_res, pCurExp, procParaGroup->working_mode);

    //inparams->u.proc.iso = scam_3a_res->hdrIso;
    outparams->algoRes = procResParaGroup->camgroupParmasArray[0]->ccm;
    Accm_processing(inparams, outparams, &swinfo);

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->ccm);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->ccm;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    LOG1_ACCM( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_CCM_VERSION     "v0.0.1"
#define RKISP_ALGO_CCM_VENDOR      "Rockchip"
#define RKISP_ALGO_CCM_DESCRIPTION "Rockchip ccm algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupCcm = {
    .common = {
        .version = RKISP_ALGO_CCM_VERSION,
        .vendor  = RKISP_ALGO_CCM_VENDOR,
        .description = RKISP_ALGO_CCM_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACCM,
        .id      = 0,
        .create_context  = algo_camgroup_CreateCtx,
        .destroy_context = algo_camgroup_DestroyCtx,
    },
    .prepare = algo_camgroup_Prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

RKAIQ_END_DECLARE
