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

#include "aiq_queue.h"

#include "aiq_base.h"
#include "base/xcam_log.h"

AiqQueue_t* aiqQueue_init(AiqQueueConfig_t* config) {
    int ret = 0, i = 0;
    int array_size = 0;
    int total_size = 0;

    AiqQueue_t* queCtx = (AiqQueue_t*)aiq_mallocz(sizeof(AiqQueue_t));

    if (!queCtx) {
        LOGE("%s: malloc failed!", __func__);
        return NULL;
    }

    if (config->_name)
        queCtx->_name = strdup(config->_name);
    else
        queCtx->_name = strdup("noname");

    ret = aiqMutex_init(&queCtx->_mutex);
    if (ret != 0) {
        goto clean_up_name;
    }

    ret = aiqCond_init(&queCtx->_cond);
    if (ret != 0) {
        goto clean_up_mutex;
    }

    config->_item_size = XCAM_ALIGN_UP(config->_item_size, 4);

    array_size = XCAM_ALIGN_UP(config->_item_size * config->_item_nums, 4);
    total_size = array_size + sizeof(AiqQueueItem_t) * config->_item_nums;

    queCtx->_base_mem = aiq_mallocz(total_size);
    if (!queCtx->_base_mem) {
        goto clean_up_cond;
    }

    queCtx->_item_nums  = config->_item_nums;
    queCtx->_item_size  = config->_item_size;
    queCtx->_item_array = (AiqQueueItem_t*)((char*)queCtx->_base_mem + array_size);

    for (i = 0; i < queCtx->_item_nums; i++) {
        queCtx->_item_array[i]._pData =
            (void*)(((char*)queCtx->_base_mem) + (config->_item_size * i));
    }

    queCtx->_cbs.insert_cb = config->_cbs.insert_cb;
    queCtx->_cbs.erase_cb  = config->_cbs.erase_cb;
    queCtx->_free_nums     = config->_item_nums;
    queCtx->_used_index    = 0;
    queCtx->_free_index    = 0;

    return queCtx;

clean_up_cond:
    aiqCond_deInit(&queCtx->_cond);
clean_up_mutex:
    aiqMutex_deInit(&queCtx->_mutex);
clean_up_name:
    aiq_free(queCtx->_name);
clean_up_ctx:
    aiq_free(queCtx);
    return NULL;
}

void aiqQueue_deinit(AiqQueue_t* queue) {
    XCAM_ASSERT(queue);

    aiqCond_broadcast(&queue->_cond);

    aiqMutex_lock(&queue->_mutex);

    if (queue->_base_mem) {
        if (queue->_free_nums != queue->_item_nums) {
            LOGW("%s: %d queue items are still in use !", queue->_name,
                 queue->_item_nums - queue->_free_nums);
        }
        free(queue->_base_mem);
    }

    aiqMutex_unlock(&queue->_mutex);

    aiqCond_deInit(&queue->_cond);
    aiqMutex_deInit(&queue->_mutex);
    aiq_free(queue->_name);
    aiq_free(queue);
}

int aiqQueue_push(AiqQueue_t* queue, void* pData) {
    XCAM_ASSERT(queue);

    aiqMutex_lock(&queue->_mutex);

    if (queue->_free_nums == 0) {
        LOGE("queue %s is full !", queue->_name);
        aiqMutex_unlock(&queue->_mutex);
        return -1;
    }
    memcpy(queue->_item_array[queue->_free_index]._pData, pData, queue->_item_size);
    queue->_free_index++;
    if (queue->_free_index >= queue->_item_nums) queue->_free_index = 0;
    queue->_free_nums--;
    XCAM_ASSERT(queue->_free_nums >= 0);
    if (queue->_cbs.insert_cb) queue->_cbs.insert_cb(pData);

    aiqCond_broadcast(&queue->_cond);
    aiqMutex_unlock(&queue->_mutex);

    return 0;
}

int aiqQueue_get(AiqQueue_t* queue, void* item, int32_t timeout_us) {
    int ret = -1;

    XCAM_ASSERT(queue);

    aiqMutex_lock(&queue->_mutex);

    if (queue->_free_nums == queue->_item_nums) {
        LOGE("queue %s is empty !", queue->_name);
        if (timeout_us == 0) goto out;

        if (timeout_us == -1)
            ret = aiqCond_wait(&queue->_cond, &queue->_mutex);
        else
            ret = aiqCond_timedWait(&queue->_cond, &queue->_mutex, timeout_us);

        if (ret != 0) goto out;

        // check again
        if (queue->_free_nums == queue->_item_nums) goto out;
    }

    memcpy(item, queue->_item_array[queue->_used_index]._pData, queue->_item_size);
    if (queue->_cbs.erase_cb) queue->_cbs.erase_cb(queue->_item_array[queue->_used_index]._pData);
    queue->_used_index++;
    if (queue->_used_index >= queue->_item_nums) queue->_used_index = 0;
    queue->_free_nums++;

    ret = 0;

out:
    aiqMutex_unlock(&queue->_mutex);
    return ret;
}

void aiqQueue_reset(AiqQueue_t* queue) {
    XCAM_ASSERT(queue);

    aiqCond_broadcast(&queue->_cond);
    aiqMutex_lock(&queue->_mutex);

    while (queue->_free_nums != queue->_item_nums) {
        if (queue->_cbs.erase_cb)
            queue->_cbs.erase_cb(queue->_item_array[queue->_used_index]._pData);
        queue->_used_index++;
        if (queue->_used_index >= queue->_item_nums) queue->_used_index = 0;
        queue->_free_nums++;
    }

    queue->_used_index = 0;
    queue->_free_index = 0;
    queue->_free_nums  = queue->_item_nums;

    aiqMutex_unlock(&queue->_mutex);
}

int aiqQueue_size(AiqQueue_t* queue) {
    int size = 0;

    XCAM_ASSERT(queue);

    aiqMutex_lock(&queue->_mutex);

    size = queue->_item_nums - queue->_free_nums;

    aiqMutex_unlock(&queue->_mutex);

    return size;
}

void aiqQueue_dump(AiqQueue_t* queue) {
    int16_t index = 0;
    int size      = 0;

    XCAM_ASSERT(queue);

    aiqMutex_lock(&queue->_mutex);

    size = queue->_item_nums - queue->_free_nums;

    LOGK(">>>> dump queue:%s: ", queue->_name);
    LOGK("	capacity:%d", queue->_item_nums);
    LOGK("	    size:%d", size);
    LOGK("	u/f index:%d, %d", queue->_used_index, queue->_free_index);

    index = queue->_used_index;
    while (size-- > 0) {
        LOGK("  ind:%d, pdata:%p", index, queue->_item_array[index]._pData);
        index++;
        if (index >= queue->_item_nums) index = 0;
    }

    aiqMutex_unlock(&queue->_mutex);
}
