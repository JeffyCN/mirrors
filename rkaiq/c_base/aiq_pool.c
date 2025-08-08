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

#include "aiq_pool.h"

#include "aiq_base.h"

int16_t aiqPoolItem_ref(void* pItem) {
    int16_t ret         = 0;
    AiqPoolItem_t* item = (AiqPoolItem_t*)pItem;
    if (item->_owner) {
        aiqMutex_lock(&item->_owner->_mutex);
		if (item->_ref_cnts == 0) {
			LOGE("pool %s item not invalid", item->_owner->_name);
			return -1;
		}
        ret = ++item->_ref_cnts;
        aiqMutex_unlock(&item->_owner->_mutex);
    }
    return ret;
}

int16_t aiqPoolItem_unref(void* pItem) {
    int16_t ret         = -1;
    AiqPoolItem_t* item = (AiqPoolItem_t*)pItem;
    AiqPool_t* pool     = item->_owner;

    if (pool) {
        aiqMutex_lock(&pool->_mutex);
        if (item->_ref_cnts > 0) {
			ret = --item->_ref_cnts;
			if (ret == 0) {
				item->_pNext     = pool->_free_list;
				pool->_free_list = item;
				pool->_free_nums++;
			}
        }
        aiqMutex_unlock(&pool->_mutex);
    }
    return ret;
}

AiqPool_t* aiqPool_init(AiqPoolConfig_t* config) {
    int ret = 0, i = 0;
    int array_size          = 0;
    int total_size          = 0;
    AiqPoolItem_t* pCurItem = NULL;

    AiqPool_t* poolCtx = (AiqPool_t*)aiq_mallocz(sizeof(AiqPool_t));

    if (!poolCtx) {
        LOGE("%s: malloc failed!", __func__);
        return NULL;
    }

    if (config->_name)
        poolCtx->_name = strdup(config->_name);
    else
        poolCtx->_name = strdup("noname");

    ret = aiqMutex_init(&poolCtx->_mutex);
    if (ret != 0) {
        goto clean_up_name;
    }

    array_size = XCAM_ALIGN_UP(config->_item_size * config->_item_nums, 4);
    total_size = array_size + sizeof(AiqPoolItem_t) * config->_item_nums;

    poolCtx->_pBaseMem = aiq_mallocz(total_size);
    if (!poolCtx->_pBaseMem) {
        goto clean_up_mutex;
    }

    poolCtx->_item_nums  = config->_item_nums;
    poolCtx->_item_size  = config->_item_size;
    poolCtx->_item_array = (AiqPoolItem_t*)((char*)poolCtx->_pBaseMem + array_size);
    for (i = 0; i < poolCtx->_item_nums; i++) {
        poolCtx->_item_array[i]._pData =
            (void*)(((char*)poolCtx->_pBaseMem) + (config->_item_size * i));
        if (i == 0) {
            poolCtx->_free_list = &poolCtx->_item_array[i];
            pCurItem            = poolCtx->_free_list;
        } else {
            pCurItem->_pNext = &poolCtx->_item_array[i];
            pCurItem         = pCurItem->_pNext;
        }
        pCurItem->_owner = poolCtx;
    }

    poolCtx->_free_nums = config->_item_nums;

    return poolCtx;

clean_up_mutex:
    aiqMutex_deInit(&poolCtx->_mutex);
clean_up_name:
    aiq_free(poolCtx->_name);
clean_up_ctx:
    aiq_free(poolCtx);
    return NULL;
}

void aiqPool_deinit(AiqPool_t* pool) {
    XCAM_ASSERT(pool);

    aiqMutex_lock(&pool->_mutex);

    if (pool->_pBaseMem) {
        if (pool->_free_nums != pool->_item_nums) {
            LOGW("%s: %d pool items are still in use !", pool->_name,
                 pool->_item_nums - pool->_free_nums);
        }
        aiq_free(pool->_pBaseMem);
    }

    aiqMutex_unlock(&pool->_mutex);

    aiqMutex_deInit(&pool->_mutex);
    aiq_free(pool->_name);
    aiq_free(pool);
}

AiqPoolItem_t* aiqPool_getFree(AiqPool_t* pool) {
    AiqPoolItem_t* ret = NULL;

    XCAM_ASSERT(pool);

    aiqMutex_lock(&pool->_mutex);

    if (pool->_free_list) {
        ret              = pool->_free_list;
        pool->_free_list = ret->_pNext;
        pool->_free_nums--;
        ret->_ref_cnts++;
    }

    aiqMutex_unlock(&pool->_mutex);

    return ret;
}

int aiqPool_freeNums(AiqPool_t* pool) {
    int ret = 0;

    XCAM_ASSERT(pool);

    aiqMutex_lock(&pool->_mutex);
    ret = pool->_free_nums;
    aiqMutex_unlock(&pool->_mutex);

    return ret;
}

void aiqPool_reset(AiqPool_t* pool) {
    int i                   = 0;
    AiqPoolItem_t* pCurItem = NULL;

    XCAM_ASSERT(pool);

    aiqMutex_lock(&pool->_mutex);

    if (pool->_free_nums != pool->_item_nums) {
        LOGE("pool %s has %d items not returned", pool->_name, pool->_item_nums - pool->_free_nums);
    }

    for (i = 0; i < pool->_item_nums; i++) {
        pool->_item_array[i]._pData = (void*)(((char*)pool->_pBaseMem) + (pool->_item_size * i));
        if (i == 0) {
            pool->_free_list = &pool->_item_array[i];
            pCurItem         = pool->_free_list;
        } else {
            pCurItem->_pNext = &pool->_item_array[i];
            pCurItem         = pCurItem->_pNext;
        }
        pCurItem->_ref_cnts = 0;
    }

    pCurItem->_pNext = NULL;
    pool->_free_nums = pool->_item_nums;

    aiqMutex_unlock(&pool->_mutex);
}

int aiqPool_getItemNums(AiqPool_t* pool) {
    int ret = 0;

    XCAM_ASSERT(pool);

    aiqMutex_lock(&pool->_mutex);
    ret = pool->_item_nums;
    aiqMutex_unlock(&pool->_mutex);

    return ret;
}

int aiqPool_getItemSize(AiqPool_t* pool) {
    int ret = 0;

    XCAM_ASSERT(pool);

    aiqMutex_lock(&pool->_mutex);
    ret = pool->_item_size;
    aiqMutex_unlock(&pool->_mutex);

    return ret;
}

void aiqPool_dump(AiqPool_t* pool) {
    int size             = 0;
    int i                = 0;
    AiqPoolItem_t* pItem = NULL;

    XCAM_ASSERT(pool);

    aiqMutex_lock(&pool->_mutex);

    size = pool->_item_nums - pool->_free_nums;

    LOGK(">>>> dump queue:%s: ", pool->_name);
    LOGK("	 capacity:%d", pool->_item_nums);
    LOGK("	used size:%d", size);

    for (i = 0; i < pool->_item_nums; i++) {
        pItem = &pool->_item_array[i];
        LOGK("  ind:%d, pdata:%p, ref_cnts:%d", i, pItem->_pData, pItem->_ref_cnts);
    }

    pItem = pool->_free_list;

    while (pItem) {
        LOGK("  free list: pdata:%p, ref_cnts:%d", pItem->_pData, pItem->_ref_cnts);
        pItem = pItem->_pNext;
    }

    aiqMutex_unlock(&pool->_mutex);
}
