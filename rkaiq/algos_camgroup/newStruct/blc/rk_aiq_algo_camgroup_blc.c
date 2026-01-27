/*
 * rk_aiq_algo_camgroup_blc_itf.cpp
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
#include "misc/rk_aiq_algo_camgroup_misc_itf.h"
#include "newStruct/blc/blc_types_prvt.h"
#include "c_base/aiq_base.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

RKAIQ_BEGIN_DECLARE

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    int delta_iso = 0;
    LOGI_ABLC("%s: (enter)\n", __FUNCTION__ );

    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    BlcContext_t* pBlcCtx = (BlcContext_t *)inparams->ctx;
    blc_api_attrib_t* blc_attrib = pBlcCtx->blc_attrib;
    blc_param_t* blc_param = (blc_param_t*)procResParaGroup->camgroupParmasArray[0]->ablc.blc;

    if (procParaGroup->gcom.com.u.proc.is_attrib_update) {
        pBlcCtx->isReCal_ = true;
    }

    int iso = 50;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];

    if(scam_3a_res->aec._bEffAecExpValid)
        iso = scam_3a_res->hdrIso;

    bool init = inparams->u.proc.init;
    delta_iso = abs(iso - pBlcCtx->iso);

    if(delta_iso > ABLC_RECALCULATE_DELTE_ISO) {
        pBlcCtx->isReCal_ = true;
    }
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    if (!pBlcCtx->aeIsConverged && procParaGroup->camgroupParmasArray[0]->aec._aeProcRes.IsConverged)
        pBlcCtx->isReCal_ = true;
    pBlcCtx->aeIsConverged = procParaGroup->camgroupParmasArray[0]->aec._aeProcRes.IsConverged;
#endif

    if (pBlcCtx->isReCal_) {
        BlcSelectParam(pBlcCtx, blc_param, iso);
        outparams->en = blc_attrib->en;
        outparams->bypass = blc_attrib->bypass;
		outparams->cfg_update = true;
        LOGD_ABLC("%s:%d processing ABLC recalculate delta_iso:%d \n", __FUNCTION__, __LINE__, delta_iso);
	} else {
		outparams->cfg_update = false;
	}

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->ablc.blc);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->ablc.blc;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    pBlcCtx->iso = iso;
    pBlcCtx->isReCal_ = 0;

    LOGI_ABLC("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_CAMGROUP_ALGO_BLC_VERSION     "v0.0.1"
#define RKISP_CAMGROUP_ALGO_BLC_VENDOR      "Rockchip"
#define RKISP_CAMGROUP_ALGO_BLC_DESCRIPTION "Rockchip blc algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupBlc = {
    .common = {
        .version = RKISP_CAMGROUP_ALGO_BLC_VERSION,
        .vendor  = RKISP_CAMGROUP_ALGO_BLC_VENDOR,
        .description = RKISP_CAMGROUP_ALGO_BLC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ABLC,
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
