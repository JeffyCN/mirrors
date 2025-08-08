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

#ifndef _AIQ_QUEUE__
#define _AIQ_QUEUE__

#include "aiq_cond.h"
#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqQueue_s AiqQueue_t;
typedef int (*aiq_queue_ops_cb)(void*);

typedef struct AiqQueueCb_s {
    aiq_queue_ops_cb insert_cb;
    aiq_queue_ops_cb erase_cb;
} AiqQueueCb_t;

typedef struct AiqQueueItem_s {
    void* _pData;
} AiqQueueItem_t;

typedef struct AiqQueue_s {
    char* _name;
    void* _base_mem;
    AiqQueueItem_t* _item_array;
    int16_t _item_size;
    int16_t _item_nums;
    int16_t _free_nums;
    int16_t _used_index;
    int16_t _free_index;
    AiqQueueCb_t _cbs;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
} AiqQueue_t;

typedef struct AiqQueueConfig_s {
    const char* _name;
    int16_t _item_size;
    int16_t _item_nums;
    AiqQueueCb_t _cbs;
} AiqQueueConfig_t;

AiqQueue_t* aiqQueue_init(AiqQueueConfig_t* config);
void aiqQueue_deinit(AiqQueue_t* queue);
int aiqQueue_push(AiqQueue_t* queue, void* pData);
int aiqQueue_get(AiqQueue_t* queue, void* item, int32_t timeout_us);
void aiqQueue_reset(AiqQueue_t* queue);
int aiqQueue_size(AiqQueue_t* queue);
void aiqQueue_dump(AiqQueue_t* queue);

RKAIQ_END_DECLARE

#endif
