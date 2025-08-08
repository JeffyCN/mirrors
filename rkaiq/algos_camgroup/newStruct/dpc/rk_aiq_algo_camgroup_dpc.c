/*
 * rk_aiq_algo_camgroup_dpcc.c
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
#include "newStruct/dpc/dpc_types_prvt.h"
#include "c_base/aiq_base.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)
typedef DpcContext_t DpcGroupContext_t;

static XCamReturn groupDpccProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ADPCC("%s enter", __FUNCTION__ );

    DpcGroupContext_t* pDpcGroupCtx = (DpcGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == NULL || procParaGroup->camgroupParmasArray == NULL) {
        LOGD_ADPCC("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pDpcGroupCtx->dpc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    int iso = pDpcGroupCtx->iso;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        iso = scam_3a_res->hdrIso;
    }

    if (procParaGroup->attribUpdated) {
        LOGI("%s attribUpdated", __func__);
        pDpcGroupCtx->isReCal_ = true;
    }
    int delta_iso = abs(iso - pDpcGroupCtx->iso);
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pDpcGroupCtx->isReCal_ = true;

    dpc_param_t* dpc_param = procResParaGroup->camgroupParmasArray[0]->dpc;

    if (pDpcGroupCtx->isReCal_) {
        DpcSelectParam(pDpcGroupCtx, dpc_param, iso);
		outparams->cfg_update = true;
        outparams->en = pDpcGroupCtx->dpc_attrib->en;
        outparams->bypass = pDpcGroupCtx->dpc_attrib->bypass;
	} else {
		outparams->cfg_update = false;
	}

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->dpc);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->dpc;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    pDpcGroupCtx->iso = iso;
    pDpcGroupCtx->isReCal_ = false;

    LOGD_ADPCC("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_DPCC_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_DPCC_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_DPCC_DESCRIPTION "Rockchip Dpcc camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupDpcc = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_DPCC_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_DPCC_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_DPCC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADPCC,
        .id      = 0,
        .create_context  = algo_camgroup_CreateCtx,
        .destroy_context = algo_camgroup_DestroyCtx,
    },
    .prepare = algo_camgroup_Prepare,
    .pre_process = NULL,
    .processing = groupDpccProcessing,
    .post_process = NULL,
};
