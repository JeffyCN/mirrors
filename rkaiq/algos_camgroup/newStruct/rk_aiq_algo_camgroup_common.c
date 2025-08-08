/*
 * rk_aiq_algo_camgroup_common.c
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

#include "rk_aiq_algo_camgroup_common.h"

XCamReturn algo_camgroup_CreateCtx(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AlgoCtxInstanceCfgCamGroup *cfgInt = (AlgoCtxInstanceCfgCamGroup*)cfg;
    *context = cfgInt->pSingleAlgoCtx;
    return ret;
}

XCamReturn algo_camgroup_DestroyCtx(RkAiqAlgoContext *context)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    return ret;
}

XCamReturn algo_camgroup_Prepare(RkAiqAlgoCom* params)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    return ret;
}

void algo_camgroup_update_results(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, void** pGpRstArray,int itemSize)
{
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    if (outparams->cfg_update) {
        bool *pUpdate= GET_UPDATE_PTR(pGpRstArray[0], procParaGroup->_offset_is_update); 
        bool *pEn = pUpdate + 1;
        bool *pBypass = pUpdate + 2;
        *pUpdate = true;
        *pEn = outparams->en;
        *pBypass = outparams->bypass;

        // copy cam0's result to other cams
        for (int i = 1; i < procResParaGroup->arraySize; i++) {
			memcpy(pGpRstArray[i], pGpRstArray[0], itemSize);
            pUpdate= GET_UPDATE_PTR(pGpRstArray[i], procParaGroup->_offset_is_update); 
			pEn = pUpdate + 1;
			pBypass = pUpdate + 2;
            *pUpdate = true;
            *pEn = outparams->en;
            *pBypass = outparams->bypass;
        }
    } else {
        for (int i = 0; i < procResParaGroup->arraySize; i++) {
            bool *pUpdate= GET_UPDATE_PTR(pGpRstArray[i], procParaGroup->_offset_is_update);
            *pUpdate = false;
        }
    }
}
