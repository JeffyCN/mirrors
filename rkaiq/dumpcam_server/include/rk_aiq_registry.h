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

#ifndef __RK_AIQ_REGISTRY_H_
#define __RK_AIQ_REGISTRY_H_

#define RK_AIQ_MAX_CAMS (8)

#ifndef RK_AIQ_SYS_CTX_T
#define RK_AIQ_SYS_CTX_T
typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
#endif

typedef struct _RKAIQRegistry RKAIQRegistry;

struct _RKAIQRegistry {
    rk_aiq_sys_ctx_t* mArray[RK_AIQ_MAX_CAMS];
    int mCnt;
};

void RKAIQRegistry_init();
void RKAIQRegistry_deinit();
void RKAIQRegistry_register(rk_aiq_sys_ctx_t* ctx);
rk_aiq_sys_ctx_t* RKAIQRegistry_unregister(int phyId);
rk_aiq_sys_ctx_t* RKAIQRegistry_lookup(int phyId);
RKAIQRegistry* RKAIQRegistry_get();

#endif  // __RK_AIQ_REGISTRY_H_
