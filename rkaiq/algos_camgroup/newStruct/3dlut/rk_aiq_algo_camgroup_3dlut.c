/*
 * rk_aiq_algo_camgroup_a3dlut_itf.c
 *
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
#include "newStruct/3dlut/3dlut_types_prvt.h"
#include "xcam_log.h"
#include "c_base/aiq_base.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    LOG1_A3DLUT( "%s: (enter)\n", __FUNCTION__);

    XCamReturn result = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    if (!procParaGroup->arraySize) {
        LOG1_A3DLUT( "%s: do nothing \n", __FUNCTION__);
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

    outparams->algoRes = procResParaGroup->camgroupParmasArray[0]->lut3d;
    A3dlut_processing(inparams, outparams, &swinfo);

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->lut3d);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->lut3d;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    LOG1_A3DLUT( "%s: (exit)\n", __FUNCTION__);
    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_3DLUT_VERSION     "v0.0.1"
#define RKISP_ALGO_3DLUT_VENDOR      "Rockchip"
#define RKISP_ALGO_3DLUT_DESCRIPTION "Rockchip 3dlut algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupLut3d = {
    .common = {
        .version = RKISP_ALGO_3DLUT_VERSION,
        .vendor  = RKISP_ALGO_3DLUT_VENDOR,
        .description = RKISP_ALGO_3DLUT_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_A3DLUT,
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
