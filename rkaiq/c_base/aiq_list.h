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

#ifndef _AIQ_LIST__
#define _AIQ_LIST__

#include "aiq_cond.h"
#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqList_s AiqList_t;
typedef struct AiqListItem_s AiqListItem_t;
typedef int (*aiqList_cb)(AiqListItem_t* item, void* args);

typedef struct AiqListItem_s {
    void* _pData;
    AiqListItem_t* _pNext;
    AiqListItem_t* _pPrev;
} AiqListItem_t;

typedef struct AiqList_s {
    char* _name;
    void* _pBaseMem;
    AiqListItem_t* _item_array;
    /* single link list */
    AiqListItem_t* _free_list;
    /* double link list */
    AiqListItem_t* _used_list;
    int16_t _item_size;
    int16_t _item_nums;
    int16_t _free_nums;
    AiqMutex_t _mutex;
} AiqList_t;

typedef struct AiqListConfig_s {
    const char* _name;
    int16_t _item_size;
    int16_t _item_nums;
} AiqListConfig_t;

AiqList_t* aiqList_init(AiqListConfig_t* config);
void aiqList_deinit(AiqList_t* list);
int aiqList_push(AiqList_t* list, void* pData);
int aiqList_get(AiqList_t* list, void* pData);
int aiqList_insert(AiqList_t* list, void* pData, AiqListItem_t* pos);
AiqListItem_t* aiqList_get_item(AiqList_t* list, AiqListItem_t* pAfter);
int aiqList_erase(AiqList_t* list, void* pData);
AiqListItem_t* aiqList_erase_item(AiqList_t* list, AiqListItem_t* item);
AiqListItem_t* aiqList_erase_item_locked(AiqList_t* list, AiqListItem_t* item);
int aiqList_foreach(AiqList_t* list, aiqList_cb cb, void* args);
void aiqList_reset(AiqList_t* list);
int aiqList_size(AiqList_t* list);
int aiqList_num(AiqList_t* list);
bool aiqList_empty(AiqList_t* list);
void aiqList_dump(AiqList_t* list);

#define AIQ_LIST_FOREACH(pList, pItem, rm)         \
    for (pItem = pList->_used_list; pItem != NULL; \
         !rm && (pItem = (pItem->_pNext == pList->_used_list) ? NULL : pItem->_pNext), rm = false)

RKAIQ_END_DECLARE

#endif
