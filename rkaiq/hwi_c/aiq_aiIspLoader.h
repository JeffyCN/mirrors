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

#ifndef _AIQ_AIISP_LOADER_H_
#define _AIQ_AIISP_LOADER_H_

#include <stdbool.h>
#include <stdint.h>

#include "algos/aiisp/rk_aiisp.h"

typedef int32_t (*rkaiq_aiisp_init)(rk_aiisp_param* param);
typedef int32_t (*rkaiq_aiisp_proc)(rk_aiisp_param* param);
typedef int32_t (*rkaiq_aiisp_deinit)(rk_aiisp_param* param);

struct AiispOps {
    rkaiq_aiisp_init aiisp_init;
    rkaiq_aiisp_proc aiisp_proc;
    rkaiq_aiisp_deinit aiisp_deinit;
};

struct AiispLibrary;
struct AiispLibrary {
    bool (*Init)(struct AiispLibrary* obj);
    bool (*LoadSymbols)(struct AiispLibrary* obj);
    void (*Deinit)(struct AiispLibrary* obj);

    void* handle_;
    struct AiispOps ops_;
};

static inline struct AiispOps* AiqAiIsp_GetOps(struct AiispLibrary* obj) { return &obj->ops_; }
bool AiqAiIsp_Init(struct AiispLibrary* obj);

#endif  // _AIQ_AIISP_LOADER_H_
