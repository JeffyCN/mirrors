/*
 * Copyright (c) 2023 Rockchip Eletronics Co., Ltd.
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
 */

#ifndef _AIQ_BASE__
#define _AIQ_BASE__

#include "aiq_mutex.h"
#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

void* aiq_malloc(size_t size);
void* aiq_mallocz(size_t size);
void aiq_free(void* ptr);
void aiq_memset(void* ptr, int c, size_t size);

typedef struct aiq_ref_base_s aiq_ref_base_t;
struct aiq_ref_base_s {
    void* _owner;
    int16_t (*ref)(void* owner);
    int16_t (*unref)(void* owner);
};

#define AIQ_REF_BASE_REF(pBase) (*((pBase)->ref))((pBase)->_owner)

#define AIQ_REF_BASE_UNREF(pBase)                               \
    ({                                                          \
        int16_t ref_ret = 0;                                    \
        ref_ret         = (*((pBase)->unref))((pBase)->_owner); \
        ref_ret;                                                \
    })

#define AIQ_REF_BASE_INIT(pBase, powner, pref, punref) \
    (pBase)->_owner = powner;                          \
    (pBase)->ref    = pref;                            \
    (pBase)->unref  = punref

typedef struct aiq_autoptr_s {
    void* _pData;
    AiqMutex_t* _mutex;
    volatile int16_t _ref_cnts;
} aiq_autoptr_t;

aiq_autoptr_t* aiq_autoptr_init(void* ptr);
int16_t aiq_autoptr_ref(void* autoptr);
int16_t aiq_autoptr_unref(void* autoptr);

RKAIQ_END_DECLARE

#endif
