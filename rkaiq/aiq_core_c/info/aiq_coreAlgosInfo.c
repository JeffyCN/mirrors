/*
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

#include "aiq_coreAlgosInfo.h"

#include "aiq_core.h"
#include "aiq_core_c/aiq_algo_handler.h"
#include "rk_info_utils.h"

static void core_algos_dump_camSignal(void* self, int algo_type, st_string* result) {
    AiqAlgoHandler_t* pHandler = ((AiqCore_t*)self)->mAlgoHandleMaps[algo_type];
    while (pHandler) {
        if (AiqAlgoHandler_getEnable(pHandler)) {
            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pHandler->mDes;
            if (des->dump) {
                char name[32] = {0};
                snprintf(name, sizeof(name), "algo_%s", AlgoTypeToString(des->common.type));
                xcam_to_lowercase(name, name);
                aiq_info_dump_submod_name(result, name);

                des->dump(pHandler->mProcInParam, result);
            }
        }

        pHandler = AiqAlgoHandler_getNextHdl(pHandler);
    }
}

static void core_algos_dump_camGrp(void* self, int algo_type, st_string* result) {
#ifdef RKAIQ_ENABLE_CAMGROUP
    if (((AiqCore_t*)self)->mCamGroupCoreManager) {
        AiqCamGroupManager_t* pCamGrpMgr   = ((AiqCore_t*)self)->mCamGroupCoreManager;
        AiqAlgoCamGroupHandler_t* pHandler = pCamGrpMgr->mDefAlgoHandleMap[algo_type];

        while (pHandler) {
            if (AiqAlgoCamGroupHandler_getEnable(pHandler)) {
                RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pHandler->mDes;
                if (des->dump) {
                    char name[32] = {0};
                    snprintf(name, sizeof(name), "algo_%s", AlgoTypeToString(des->common.type));
                    xcam_to_lowercase(name, name);
                    aiq_info_dump_submod_name(result, name);

                    des->dump(pHandler->mProcInParam, result);
                }
            }

            pHandler = AiqAlgoCamGroupHandler_getNextHdl(pHandler);
        }
    }
#endif
}

void core_alogs_dump_by_type(void* self, int type, st_string* result) {
    for (int i = 0; i < RK_AIQ_ALGO_TYPE_MAX; i++) {
        if (i != type) continue;

#ifdef RKAIQ_ENABLE_CAMGROUP
        if (((AiqCore_t*)self)->mCamGroupCoreManager)
            core_algos_dump_camGrp(self, type, result);
        else
            core_algos_dump_camSignal(self, type, result);
#else
        core_algos_dump_camSignal(self, type, result);
#endif
    }
}
