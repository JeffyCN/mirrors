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

#include "aiq_list.h"

#include "aiq_base.h"

AiqList_t* aiqList_init(AiqListConfig_t* config) {
    int ret = 0, i = 0;
    int array_size          = 0;
    int total_size          = 0;
    AiqListItem_t* pCurItem = NULL;

    AiqList_t* listCtx = (AiqList_t*)aiq_mallocz(sizeof(AiqList_t));

    if (!listCtx) {
        LOGE("%s: malloc failed!", __func__);
        return NULL;
    }

    if (config->_name)
        listCtx->_name = strdup(config->_name);
    else
        listCtx->_name = strdup("noname");

    ret = aiqMutex_init(&listCtx->_mutex);
    if (ret != 0) {
        goto clean_up_name;
    }

    config->_item_size = XCAM_ALIGN_UP(config->_item_size, 4);

    array_size = XCAM_ALIGN_UP(config->_item_size * config->_item_nums, 4);
    total_size = array_size + sizeof(AiqListItem_t) * config->_item_nums;

    listCtx->_pBaseMem = aiq_mallocz(total_size);
    if (!listCtx->_pBaseMem) {
        goto clean_up_mutex;
    }

    listCtx->_item_nums  = config->_item_nums;
    listCtx->_item_size  = config->_item_size;
    listCtx->_item_array = (AiqListItem_t*)((char*)listCtx->_pBaseMem + array_size);

    for (i = 0; i < listCtx->_item_nums; i++) {
        listCtx->_item_array[i]._pData =
            (void*)(((char*)listCtx->_pBaseMem) + (config->_item_size * i));
        if (i == 0) {
            listCtx->_free_list = &listCtx->_item_array[i];
            pCurItem            = listCtx->_free_list;
        } else {
            pCurItem->_pNext = &listCtx->_item_array[i];
            pCurItem         = pCurItem->_pNext;
        }
    }

    listCtx->_free_nums = config->_item_nums;

    return listCtx;

clean_up_mutex:
    aiqMutex_deInit(&listCtx->_mutex);
clean_up_name:
    aiq_free(listCtx->_name);
clean_up_ctx:
    aiq_free(listCtx);

    return NULL;
}

void aiqList_deinit(AiqList_t* list) {
    XCAM_ASSERT(list);

    aiqMutex_lock(&list->_mutex);

    if (list->_pBaseMem) {
        if (list->_free_nums != list->_item_nums) {
            LOGW("%s: %d list items are still in use !", list->_name,
                 list->_item_nums - list->_free_nums);
        }
        aiq_free(list->_pBaseMem);
    }

    aiqMutex_unlock(&list->_mutex);

    aiqMutex_deInit(&list->_mutex);
    aiq_free(list->_name);
    aiq_free(list);
}

static void _insert_locked(AiqList_t* list, AiqListItem_t* entry, AiqListItem_t* pos) {
    if (!list->_used_list) {
        list->_used_list = entry;
        entry->_pNext    = entry;
        entry->_pPrev    = entry;
    } else {
        pos->_pNext->_pPrev = entry;
        entry->_pNext       = pos->_pNext;
        pos->_pNext         = entry;
        entry->_pPrev       = pos;
    }
}

static AiqListItem_t* _erase_locked(AiqList_t* list, AiqListItem_t* entry) {
    // check if only one entry
    AiqListItem_t* ret_next = NULL;
    if (entry->_pNext == entry && entry->_pPrev == entry) {
        list->_used_list = NULL;
    } else {
        if (entry->_pNext != list->_used_list) ret_next = entry->_pNext;
        entry->_pPrev->_pNext = entry->_pNext;
        entry->_pNext->_pPrev = entry->_pPrev;
        // check if the head
        if (list->_used_list == entry) list->_used_list = entry->_pNext;
    }

    // add to free list head
    if (!list->_free_list) {
        list->_free_list = entry;
        entry->_pNext    = NULL;
    } else {
        entry->_pPrev    = NULL;
        entry->_pNext    = list->_free_list;
        list->_free_list = entry;
    }

    list->_free_nums++;

    return ret_next;
}

int aiqList_push(AiqList_t* list, void* pData) {
    AiqListItem_t* pEntry = NULL;
    AiqListItem_t* pos    = NULL;

    aiqMutex_lock(&list->_mutex);

    if (list->_free_nums == 0) {
        LOGE("list %s is full !", list->_name);
        aiqMutex_unlock(&list->_mutex);
        return -1;
    }

    pEntry           = list->_free_list;
    list->_free_list = pEntry->_pNext;
    list->_free_nums--;
    pEntry->_pNext = NULL;

    if (list->_used_list) pos = list->_used_list->_pPrev;

    memcpy(pEntry->_pData, pData, list->_item_size);

    // insert to last
    _insert_locked(list, pEntry, pos);

    aiqMutex_unlock(&list->_mutex);

    return 0;
}

int aiqList_get(AiqList_t* list, void* pData) {
    AiqListItem_t* pEntry = NULL;

    aiqMutex_lock(&list->_mutex);

    if (!list->_used_list) {
        LOGE("list %s is empty !", list->_name);
        aiqMutex_unlock(&list->_mutex);
        return -1;
    }

    pEntry = list->_used_list;

    // insert to last
    memcpy(pData, pEntry->_pData, list->_item_size);

    _erase_locked(list, pEntry);

    aiqMutex_unlock(&list->_mutex);

    return 0;
}

int aiqList_insert(AiqList_t* list, void* pData, AiqListItem_t* pos) {
    AiqListItem_t* pEntry = NULL;

    aiqMutex_lock(&list->_mutex);

    if (list->_free_nums == 0) {
        LOGE("list %s is full !", list->_name);
        aiqMutex_unlock(&list->_mutex);
        return -1;
    }

    pEntry           = list->_free_list;
    list->_free_list = pEntry->_pNext;
    list->_free_nums--;
    pEntry->_pNext = NULL;

    memcpy(pEntry->_pData, pData, list->_item_size);

    // default to last
    if (!pos && list->_used_list) pos = list->_used_list->_pPrev;

    // insert after pos
    _insert_locked(list, pEntry, pos);

    aiqMutex_unlock(&list->_mutex);

    return 0;
}

AiqListItem_t* aiqList_get_item(AiqList_t* list, AiqListItem_t* pAfter) {
    AiqListItem_t* pEntry = NULL;

    aiqMutex_lock(&list->_mutex);

    if (!list->_used_list) {
        LOGE("list %s is empty !", list->_name);
        aiqMutex_unlock(&list->_mutex);
        return NULL;
    }

    if (pAfter) {
        if (pAfter->_pNext == list->_used_list)
            pEntry = NULL;
        else
            pEntry = pAfter->_pNext;
    } else {
        pEntry = list->_used_list;
    }

    aiqMutex_unlock(&list->_mutex);

    return pEntry;
}

int aiqList_erase(AiqList_t* list, void* pData) {
    AiqListItem_t* pEntry = NULL;
    bool found            = false;

    aiqMutex_lock(&list->_mutex);

    if (!list->_used_list) {
        LOGE("list %s is empty !", list->_name);
        aiqMutex_unlock(&list->_mutex);
        return -1;
    }

    pEntry = list->_used_list;

    while (pEntry) {
        if (memcmp(pData, pEntry->_pData, list->_item_size) == 0) {
            found = true;
            break;
        }
        pEntry = pEntry->_pNext;
        if (pEntry == list->_used_list) break;
    }

    if (found) _erase_locked(list, pEntry);

    aiqMutex_unlock(&list->_mutex);

    return 0;
}

AiqListItem_t* aiqList_erase_item(AiqList_t* list, AiqListItem_t* item) {
    AiqListItem_t* ret_next = NULL;
    aiqMutex_lock(&list->_mutex);

    if (!list->_used_list) {
        LOGE("list %s is empty !", list->_name);
        aiqMutex_unlock(&list->_mutex);
        return NULL;
    }

    ret_next = _erase_locked(list, item);

    aiqMutex_unlock(&list->_mutex);

    return ret_next;
}

AiqListItem_t* aiqList_erase_item_locked(AiqList_t* list, AiqListItem_t* item) {
    return _erase_locked(list, item);
}

int aiqList_foreach(AiqList_t* list, aiqList_cb cb, void* args) {
    AiqListItem_t* pCurItem  = NULL;
    AiqListItem_t* pNextItem = NULL;
    AiqListItem_t* pLast     = NULL;
    int ret                  = 0;

    XCAM_ASSERT(list);

    aiqMutex_lock(&list->_mutex);

    pCurItem  = list->_used_list;
    pNextItem = pCurItem->_pNext;
    pLast     = pCurItem->_pPrev;
    while (pCurItem) {
        ret = (*cb)(pCurItem, args);
        if (ret) break;
        if (pCurItem == pLast) break;
        pCurItem  = pNextItem;
        pNextItem = pCurItem->_pNext;
    }

    aiqMutex_unlock(&list->_mutex);

    return ret;
}

void aiqList_reset(AiqList_t* list) {
    AiqListItem_t* pCurItem = NULL;
    int i                   = 0;

    XCAM_ASSERT(list);

    aiqMutex_lock(&list->_mutex);

    if (list->_free_nums != list->_item_nums) {
        LOGE("list %s is not empty, still %d in map !", list->_name,
             list->_item_nums - list->_free_nums);
    }

    list->_free_list = NULL;
    list->_used_list = NULL;
    list->_free_nums = list->_item_nums;

    for (i = 0; i < list->_item_nums; i++) {
        if (i == 0) {
            list->_free_list = &list->_item_array[i];
            pCurItem         = list->_free_list;
        } else {
            pCurItem->_pNext = &list->_item_array[i];
            pCurItem         = pCurItem->_pNext;
        }
    }

    pCurItem->_pNext = NULL;

    aiqMutex_unlock(&list->_mutex);
}

int aiqList_size(AiqList_t* list) {
    int size = 0;
    XCAM_ASSERT(list);

    aiqMutex_lock(&list->_mutex);

    size = list->_item_nums - list->_free_nums;

    aiqMutex_unlock(&list->_mutex);

    return size;
}

int aiqList_num(AiqList_t* list) {
    int num = 0;

    XCAM_ASSERT(list);

    aiqMutex_lock(&list->_mutex);

    num = list->_item_nums;

    aiqMutex_unlock(&list->_mutex);

    return num;
}

bool aiqList_empty(AiqList_t* list) {
    bool empty = true;

    XCAM_ASSERT(list);

    aiqMutex_lock(&list->_mutex);

    empty = !(list->_used_list);

    aiqMutex_unlock(&list->_mutex);

    return empty;
}

void aiqList_dump(AiqList_t* list) {
    AiqListItem_t* pCurItem;
    int i = 0;

    XCAM_ASSERT(list);

    aiqMutex_lock(&list->_mutex);

    LOGK(">>>> dump list:%s ", list->_name);
    LOGK("	capacity:%d", list->_item_nums);
    LOGK("	    size:%d", list->_item_nums - list->_free_nums);

    pCurItem = list->_free_list;
    while (pCurItem) {
        LOGK("	free list:%p", pCurItem);
        pCurItem = pCurItem->_pNext;
    }

    pCurItem = list->_used_list;
    LOGK("	used list :%p", pCurItem);
    while (pCurItem) {
        LOGK("	used next list :%p", pCurItem);
        pCurItem = pCurItem->_pNext;
        if (pCurItem == list->_used_list) break;
    }

    pCurItem = list->_used_list;
    while (pCurItem) {
        LOGK("	used prev list :%p", pCurItem);
        pCurItem = pCurItem->_pPrev;
        if (pCurItem == list->_used_list) break;
    }

    aiqMutex_unlock(&list->_mutex);
}
