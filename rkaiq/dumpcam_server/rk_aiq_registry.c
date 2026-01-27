/*
 *   Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "rk_aiq_registry.h"

#include <stdio.h>

#include "c_base/aiq_base.h"
#include "uAPI2_c/rk_aiq_api_private_c.h"

static RKAIQRegistry* gAiqRegistry = NULL;

void RKAIQRegistry_init() {
    if (!gAiqRegistry) {
        gAiqRegistry = (RKAIQRegistry*)aiq_mallocz(sizeof(*gAiqRegistry));
        for (int i = 0; i < RK_AIQ_MAX_CAMS; i++) gAiqRegistry->mArray[i] = NULL;
    }
}

void RKAIQRegistry_deinit() {
    if (gAiqRegistry) {
        aiq_free(gAiqRegistry);
        gAiqRegistry = NULL;
    }
}

void RKAIQRegistry_register(rk_aiq_sys_ctx_t* ctx) {
    if (!ctx) return;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_SINGLE) {
        if (ctx->_camPhyId < 0 || ctx->_camPhyId >= RK_AIQ_MAX_CAMS) {
            return;
        }

        if (!gAiqRegistry->mArray[ctx->_camPhyId]) {
            gAiqRegistry->mArray[ctx->_camPhyId] = ctx;
            gAiqRegistry->mCnt++;
        }
    } else if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        if (!gAiqRegistry->mArray[0]) {
            gAiqRegistry->mArray[0] = ctx;
            gAiqRegistry->mCnt      = 1;
        }
    }
}

rk_aiq_sys_ctx_t* RKAIQRegistry_unregister(int phyId) {
    if (phyId < 0 || phyId >= RK_AIQ_MAX_CAMS) return NULL;

    rk_aiq_sys_ctx_t* ctx = NULL;

    if (gAiqRegistry->mArray[phyId]) {
        ctx                                  = gAiqRegistry->mArray[ctx->_camPhyId];
        gAiqRegistry->mArray[ctx->_camPhyId] = NULL;
        gAiqRegistry->mCnt--;
    }

    return ctx;
}

rk_aiq_sys_ctx_t* RKAIQRegistry_lookup(int phyId) {
    if (phyId < 0 || phyId >= RK_AIQ_MAX_CAMS) return NULL;

    return gAiqRegistry->mArray[phyId];
}

RKAIQRegistry* RKAIQRegistry_get() { return gAiqRegistry; }
