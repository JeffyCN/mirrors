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

#ifndef _AIQ_MAP__
#define _AIQ_MAP__

#include "aiq_cond.h"
#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqMap_s AiqMap_t;
typedef struct AiqMapItem_s AiqMapItem_t;
typedef int (*aiqMap_cb)(AiqMapItem_t* item, void* args);

#define AIQ_MAP_HASH_ENTRY_NUM (10)

typedef enum AiqMapKeyType_s {
    AIQ_MAP_KEY_TYPE_INVALID,
    AIQ_MAP_KEY_TYPE_UINT32,
    AIQ_MAP_KEY_TYPE_STRING
} AiqMapKeyType_t;

typedef struct AiqMapItem_s {
    void* _pData;
    void* _key;
    AiqMapItem_t* _pNext;      /*< used by _map_entry and _free_list*/
    AiqMapItem_t* _pOrderNext; /*< used by _order_list */
} AiqMapItem_t;

typedef struct AiqMap_s {
    char* _name;
    void* _pBaseMem;
    AiqMapKeyType_t _key_type;
    AiqMapItem_t* _item_array;
    int16_t _item_size;
    int16_t _item_nums;
    int16_t _free_nums;
    AiqMapItem_t* _map_entry[AIQ_MAP_HASH_ENTRY_NUM];
    AiqMapItem_t* _order_list;
    AiqMapItem_t* _free_list;
    AiqMutex_t _mutex;
} AiqMap_t;

typedef struct AiqMapConfig_s {
    const char* _name;
    AiqMapKeyType_t _key_type;
    int16_t _item_size;
    int16_t _item_nums;
} AiqMapConfig_t;

AiqMap_t* aiqMap_init(AiqMapConfig_t* config);
void aiqMap_deinit(AiqMap_t* map);
AiqMapItem_t* aiqMap_insert(AiqMap_t* map, void* key, void* pData);
AiqMapItem_t* aiqMap_get(AiqMap_t* map, void* key);
AiqMapItem_t* aiqMap_rbegin(AiqMap_t* map);
AiqMapItem_t* aiqMap_erase(AiqMap_t* map, void* key);
AiqMapItem_t* aiqMap_erase_locked(AiqMap_t* map, void* key);
void aiqMap_reset(AiqMap_t* map);
int aiqMap_size(AiqMap_t* map);
int aiqMap_itemNums(AiqMap_t* map);
int aiqMap_itemSize(AiqMap_t* map);
int aiqMap_foreach(AiqMap_t* map, aiqMap_cb cb, void* args);
AiqMapItem_t* aiqMap_begin(AiqMap_t* map);
void aiqMap_dump(AiqMap_t* map);

#define AIQ_MAP_FOREACH(pMap, pItem, rm) \
    for (pItem = pMap->_order_list; pItem != NULL; \
         !rm && (pItem = (pItem->_pOrderNext == pMap->_order_list) ? NULL : pItem->_pOrderNext), rm = false)

RKAIQ_END_DECLARE

#endif
