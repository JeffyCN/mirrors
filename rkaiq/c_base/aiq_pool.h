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

#ifndef _AIQ_POOL__
#define _AIQ_POOL__

#include "aiq_mutex.h"
#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqPool_s AiqPool_t;
typedef struct AiqPoolItem_s AiqPoolItem_t;

typedef struct AiqPoolItem_s {
    void* _pData;
    AiqPoolItem_t* _pNext;
    AiqPool_t* _owner;
    volatile uint16_t _ref_cnts;
} AiqPoolItem_t;

typedef struct AiqPool_s {
    char* _name;
    void* _pBaseMem;
    AiqPoolItem_t* _item_array;
    AiqPoolItem_t* _free_list;
    int16_t _item_size;
    int16_t _item_nums;
    int16_t _free_nums;
    AiqMutex_t _mutex;
} AiqPool_t;

typedef struct AiqPoolConfig_s {
    const char* _name;
    int16_t _item_size;
    int16_t _item_nums;
} AiqPoolConfig_t;

int16_t aiqPoolItem_ref(void* item);
int16_t aiqPoolItem_unref(void* item);
#define AIQPOOLITEMCAST(item, type) (type*)(item->_pData)

AiqPool_t* aiqPool_init(AiqPoolConfig_t* config);
void aiqPool_deinit(AiqPool_t* pool);
AiqPoolItem_t* aiqPool_getFree(AiqPool_t* pool);
int aiqPool_freeNums(AiqPool_t* pool);
int aiqPool_getItemNums(AiqPool_t* pool);
int aiqPool_getItemSize(AiqPool_t* pool);
void aiqPool_reset(AiqPool_t* pool);
void aiqPool_dump(AiqPool_t* pool);

#define AIQ_POOL_FOREACH(pPool, pItem) \
    for (pItem = pPool->_free_list; pItem != NULL; pItem = pItem->_pNext)

RKAIQ_END_DECLARE

#endif
