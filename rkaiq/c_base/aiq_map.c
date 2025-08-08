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

#include "aiq_map.h"

#include "aiq_base.h"

typedef struct aiqMapHash_s {
    AiqMapItem_t* pEntry;
    int hash_code;
} aiqMapHash_t;

static inline int hash_code_int(uint32_t key) { return (key % AIQ_MAP_HASH_ENTRY_NUM); }

static inline int hash_code_str(const char* key) {
    int len = strlen(key);
    int i = 0, h = 0;

    for (; i < len; i++) h = 31 * h + key[i];

    return (h % AIQ_MAP_HASH_ENTRY_NUM);
}

AiqMap_t* aiqMap_init(AiqMapConfig_t* config) {
    int ret = 0, i = 0;
    int array_size         = 0;
    int total_size         = 0;
    AiqMapItem_t* pCurItem = NULL;

    AiqMap_t* mapCtx = (AiqMap_t*)aiq_mallocz(sizeof(AiqMap_t));

    if (!mapCtx) {
        LOGE("%s: malloc failed!", __func__);
        return NULL;
    }

    if (config->_name)
        mapCtx->_name = strdup(config->_name);
    else
        mapCtx->_name = strdup("noname");

    ret = aiqMutex_init(&mapCtx->_mutex);
    if (ret != 0) {
        goto clean_up_name;
    }

    array_size = XCAM_ALIGN_UP(config->_item_size * config->_item_nums, 4);
    total_size = array_size + sizeof(AiqMapItem_t) * config->_item_nums;

    mapCtx->_pBaseMem = aiq_mallocz(total_size);
    if (!mapCtx->_pBaseMem) {
        goto clean_up_mutex;
    }

    mapCtx->_item_nums  = config->_item_nums;
    mapCtx->_item_size  = config->_item_size;
    mapCtx->_item_array = (AiqMapItem_t*)((char*)mapCtx->_pBaseMem + array_size);

    for (i = 0; i < mapCtx->_item_nums; i++) {
        mapCtx->_item_array[i]._pData =
            (void*)(((char*)mapCtx->_pBaseMem) + (config->_item_size * i));
        if (i == 0) {
            mapCtx->_free_list = &mapCtx->_item_array[i];
            pCurItem           = mapCtx->_free_list;
        } else {
            pCurItem->_pNext = &mapCtx->_item_array[i];
            pCurItem         = pCurItem->_pNext;
        }
    }

    mapCtx->_free_nums = config->_item_nums;
    mapCtx->_key_type  = config->_key_type;

    return mapCtx;

clean_up_mutex:
    aiqMutex_deInit(&mapCtx->_mutex);
clean_up_name:
    aiq_free(mapCtx->_name);
clean_up_ctx:
    aiq_free(mapCtx);

    return NULL;
}

void aiqMap_deinit(AiqMap_t* map) {
    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    if (map->_pBaseMem) {
        if (map->_free_nums != map->_item_nums) {
            LOGW("%s: %d map items are still in use !", map->_name,
                 map->_item_nums - map->_free_nums);
        }
        aiq_free(map->_pBaseMem);
    }

    aiqMutex_unlock(&map->_mutex);

    aiqMutex_deInit(&map->_mutex);
    aiq_free(map->_name);
    aiq_free(map);
}

#if 0
static void aiqMap_check(AiqMap_t* map) {
	int order_list_cnts = 0;
    AiqMapItem_t* pCur = map->_order_list;
	while (pCur) {
		order_list_cnts++;
		pCur = pCur->_pOrderNext;
		if (order_list_cnts > map->_item_nums)
			break;
	}

	int map_list_cnts = 0;
    for (int i = 0; i < AIQ_MAP_HASH_ENTRY_NUM; i++) {
        pCur = map->_map_entry[i];
        while (pCur) {
            pCur = pCur->_pNext;
			map_list_cnts++;
			if (map_list_cnts > map->_item_nums)
				break;
        }
    }

	if ((order_list_cnts + map->_free_nums) != map->_item_nums) {
		LOGE("%p: total cnts error: %d+%d != %d", map,
				order_list_cnts, map->_free_nums, map->_item_nums);
	}

	if (order_list_cnts != map_list_cnts) {
		LOGE("%p, order_list_cnts(%d) != map_list_cnts(%d)", map, order_list_cnts, map_list_cnts);
	}
}
#endif

aiqMapHash_t _aiqMap_get_locked(AiqMap_t* map, void* key, bool erase) {
    AiqMapItem_t* pEntry     = NULL;
    AiqMapItem_t* pPrevEntry = NULL;
    aiqMapHash_t hash;

    hash.pEntry    = NULL;
    hash.hash_code = 0;

    if (map->_key_type == AIQ_MAP_KEY_TYPE_UINT32) {
        hash.hash_code = hash_code_int((long)key);
    } else if (map->_key_type == AIQ_MAP_KEY_TYPE_STRING) {
        hash.hash_code = hash_code_str((const char*)key);
    } else {
        goto out;
    }

    pEntry = map->_map_entry[hash.hash_code];

    // check if it is already in map
    while (pEntry) {
        if (map->_key_type == AIQ_MAP_KEY_TYPE_UINT32) {
            if ((long)key == (long)(pEntry->_key)) break;
        } else if (map->_key_type == AIQ_MAP_KEY_TYPE_STRING) {
            if (strcmp(key, pEntry->_key) == 0) break;
        }
        pPrevEntry = pEntry;
        pEntry     = pEntry->_pNext;
    }

    hash.pEntry = pEntry;

    if (erase && pEntry) {
        if (pPrevEntry)
            pPrevEntry->_pNext = pEntry->_pNext;
        else
            map->_map_entry[hash.hash_code] = pEntry->_pNext;
    }

out:
    return hash;
}

AiqMapItem_t* aiqMap_insert(AiqMap_t* map, void* key, void* pData) {
    int ret              = 0;
    int hash_code        = 0;
    AiqMapItem_t* pEntry = NULL;
    aiqMapHash_t hash;

    hash.pEntry    = NULL;
    hash.hash_code = 0;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    if (map->_free_nums == 0) {
        LOGE("map %s is full !", map->_name);
        goto out;
    }

    hash      = _aiqMap_get_locked(map, key, false);
    pEntry    = hash.pEntry;
    hash_code = hash.hash_code;
    if (pEntry) {
        memcpy(pEntry->_pData, pData, map->_item_size);
    } else {
        // not in map, allocate a free entry
        pEntry          = map->_free_list;
        map->_free_list = pEntry->_pNext;
        map->_free_nums--;
        pEntry->_pNext = NULL;
        pEntry->_pOrderNext = NULL;

        pEntry->_key = key;
        memcpy(pEntry->_pData, pData, map->_item_size);
        if (!map->_map_entry[hash_code]) {
            map->_map_entry[hash_code] = pEntry;
        } else {
            // inserted to head
            pEntry->_pNext             = map->_map_entry[hash_code];
            map->_map_entry[hash_code] = pEntry;
        }

        // added to ascending list
        if (map->_key_type == AIQ_MAP_KEY_TYPE_UINT32) {
            // check if _order_list is empty
            if (!map->_order_list) {
                map->_order_list    = pEntry;
            } else {
                AiqMapItem_t* pOrderEntry     = map->_order_list;
                AiqMapItem_t* pOrderPrevEntry = NULL;

                while (pOrderEntry) {
                    if (pOrderEntry->_key > pEntry->_key) {
                        // check if the first one
                        if (!pOrderPrevEntry) {
                            map->_order_list    = pEntry;
                            pEntry->_pOrderNext = pOrderEntry;
                        } else {
                            pOrderPrevEntry->_pOrderNext = pEntry;
                            pEntry->_pOrderNext          = pOrderEntry;
                        }
                        break;
                    }

                    // reach end
                    if (!pOrderEntry->_pOrderNext) {
                        pOrderEntry->_pOrderNext = pEntry;
                        break;
                    }
                    pOrderPrevEntry = pOrderEntry;
                    pOrderEntry     = pOrderEntry->_pOrderNext;
                }
            }
        }
    }
out:
    aiqMutex_unlock(&map->_mutex);
    return pEntry;
}

AiqMapItem_t* aiqMap_get(AiqMap_t* map, void* key) {
    aiqMapHash_t hash;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    hash = _aiqMap_get_locked(map, key, false);

    aiqMutex_unlock(&map->_mutex);

    return hash.pEntry;
}

AiqMapItem_t* aiqMap_erase_locked(AiqMap_t* map, void* key) {
    aiqMapHash_t hash;
    AiqMapItem_t* pEntry     = NULL;
    AiqMapItem_t* pNextEntry = NULL;
    int hash_code            = 0;

    XCAM_ASSERT(map);

    hash      = _aiqMap_get_locked(map, key, true);
    pEntry    = hash.pEntry;
    hash_code = hash.hash_code;

    if (pEntry) {
        // inserted to free list
        if (!map->_free_list) {
            map->_free_list = pEntry;
            pEntry->_pNext  = NULL;
        } else {
            // inserted to head
            pEntry->_pNext  = map->_free_list;
            map->_free_list = pEntry;
        }
        map->_free_nums++;
        // removed from order list
        if (map->_key_type == AIQ_MAP_KEY_TYPE_UINT32) {
            AiqMapItem_t* pOrderEntry     = map->_order_list;
            AiqMapItem_t* pOrderPrevEntry = NULL;

            while (pOrderEntry) {
                /* pOrderEntry == pEntry ? */
                if (pOrderEntry->_key == pEntry->_key) {
                    if (!pOrderPrevEntry)
                        map->_order_list = pOrderEntry->_pOrderNext;
                    else
                        pOrderPrevEntry->_pOrderNext = pOrderEntry->_pOrderNext;
                    pNextEntry = pOrderEntry->_pOrderNext;
                    break;
                }
                pOrderPrevEntry = pOrderEntry;
                pOrderEntry     = pOrderEntry->_pOrderNext;
            }
        }
    }

    return pNextEntry;
}

AiqMapItem_t* aiqMap_erase(AiqMap_t* map, void* key) {
    AiqMapItem_t* pNextEntry = NULL;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);
    pNextEntry = aiqMap_erase_locked(map, key);
    aiqMutex_unlock(&map->_mutex);

    return pNextEntry;
}

int aiqMap_foreach(AiqMap_t* map, aiqMap_cb cb, void* args) {
    AiqMapItem_t* pCurItem  = NULL;
    AiqMapItem_t* pNextItem = NULL;
    int ret                 = 0;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    pCurItem  = map->_order_list;
    pNextItem = pCurItem->_pOrderNext;
    while (pCurItem) {
        ret = (*cb)(pCurItem, args);
        if (ret) break;
        pCurItem = pNextItem;
        if (pCurItem == NULL) break;
        pNextItem = pCurItem->_pOrderNext;
    }

    aiqMutex_unlock(&map->_mutex);

    return ret;
}

void aiqMap_reset(AiqMap_t* map) {
    AiqMapItem_t* pCurItem = NULL;
    int i                  = 0;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    if (map->_free_nums != map->_item_nums) {
        LOGE("map %s is not empty, still %d in map !", map->_name,
             map->_item_nums - map->_free_nums);
    }

    map->_free_list  = NULL;
    map->_order_list = NULL;
    map->_free_nums  = map->_item_nums;
    memset(map->_map_entry, 0, sizeof(map->_map_entry));

    for (i = 0; i < map->_item_nums; i++) {
        if (i == 0) {
            map->_free_list = &map->_item_array[i];
            pCurItem        = map->_free_list;
        } else {
            pCurItem->_pNext = &map->_item_array[i];
            pCurItem         = pCurItem->_pNext;
        }
    }

    pCurItem->_pNext = NULL;

    aiqMutex_unlock(&map->_mutex);
}

int aiqMap_size(AiqMap_t* map) {
    int size = 0;
    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    size = map->_item_nums - map->_free_nums;

    aiqMutex_unlock(&map->_mutex);

    return size;
}

int aiqMap_itemNums(AiqMap_t* map) {
    int num = 0;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    num = map->_item_nums;

    aiqMutex_unlock(&map->_mutex);

    return num;
}

int aiqMap_itemSize(AiqMap_t* map) {
    int size = 0;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    size = map->_item_size;

    aiqMutex_unlock(&map->_mutex);

    return size;
}

AiqMapItem_t* aiqMap_begin(AiqMap_t* map) {
    AiqMapItem_t* pCurItem = NULL;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    if (map->_key_type == AIQ_MAP_KEY_TYPE_UINT32) pCurItem = map->_order_list;

    aiqMutex_unlock(&map->_mutex);

    return pCurItem;
}

AiqMapItem_t* aiqMap_rbegin(AiqMap_t* map) {
    AiqMapItem_t* pCurItem = NULL;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    pCurItem = map->_order_list;

    while (pCurItem) {
        if (pCurItem->_pOrderNext == NULL) break;
        pCurItem = pCurItem->_pNext;
    }

    aiqMutex_unlock(&map->_mutex);

    return pCurItem;
}

void aiqMap_dump(AiqMap_t* map) {
    AiqMapItem_t* pCurItem;
    int i = 0;

    XCAM_ASSERT(map);

    aiqMutex_lock(&map->_mutex);

    LOGK(">>>> dump map:%s ", map->_name);
    LOGK("	capacity:%d", map->_item_nums);
    LOGK("	    size:%d", map->_item_nums - map->_free_nums);

    pCurItem = map->_free_list;
    while (pCurItem) {
        LOGK("	free list:%p", pCurItem);
        pCurItem = pCurItem->_pNext;
    }

    pCurItem = map->_order_list;
    while (pCurItem) {
        LOGK("	order list key:%d", pCurItem->_key);
        pCurItem = pCurItem->_pOrderNext;
    }

    for (i = 0; i < AIQ_MAP_HASH_ENTRY_NUM; i++) {
        pCurItem = map->_map_entry[i];
        while (pCurItem) {
            if (map->_key_type == AIQ_MAP_KEY_TYPE_UINT32)
                LOGK("	map:%i, key:%d", i, pCurItem->_key);
            else
                LOGK("	map:%i, key:%s", i, pCurItem->_key);
            pCurItem = pCurItem->_pNext;
        }
    }

    aiqMutex_unlock(&map->_mutex);
}
