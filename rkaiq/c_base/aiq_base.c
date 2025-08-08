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

#include "aiq_base.h"

#include "aiq_mutex.h"

inline void* aiq_malloc(size_t size) { return malloc(size); }

#define AIQ_MEM_REC_MAX 100000
typedef struct aiqMemRec_s {
	size_t total_size;
	struct {
		void* pVoid;
		size_t size;
	} head[AIQ_MEM_REC_MAX];
} aiqMemRec_t;

//#define AIQ_MALLOC_DBG 1
#if AQI_MALLOC_DBG
static aiqMemRec_t g_aiqMemRec = {
	.total_size = 0,
	//.head[0] = { NULL, 0 },
};
#endif

inline void* aiq_mallocz(size_t size) {
    void* ret = malloc(size);
    if (ret) {
		memset(ret, 0, size);
#if AIQ_MALLOC_DBG
		int i = 0;
		for (i = 0; i < AIQ_MEM_REC_MAX; i++) {
			if (!g_aiqMemRec.head[i].pVoid) {
				g_aiqMemRec.head[i].pVoid = ret;
				g_aiqMemRec.head[i].size = size;
				g_aiqMemRec.total_size += size;
				break;
			}
		}

		if ( i == AIQ_MEM_REC_MAX) {
			LOGE("%s: OVER the max record size !", __func__);
		}
		LOGK("MemTotal_M: alloc:%p, size: %d, total: %d", ret, size, g_aiqMemRec.total_size);
#endif
	}

    return ret;
}

inline void aiq_free(void* ptr)
{
	free(ptr);
#if AIQ_MALLOC_DBG
	int i = 0;
	for (i = 0; i < AIQ_MEM_REC_MAX; i++) {
		if (ptr && g_aiqMemRec.head[i].pVoid == ptr) {
			g_aiqMemRec.head[i].pVoid = NULL;
			g_aiqMemRec.total_size -= g_aiqMemRec.head[i].size;
			g_aiqMemRec.head[i].size = 0;
			break;
		}
	}
	LOGK("MemTotal_F: free:%p, total:%d", ptr, g_aiqMemRec.total_size);
#endif
}

inline void aiq_memset(void* ptr, int c, size_t size) { memset(ptr, c, size); }

aiq_autoptr_t* aiq_autoptr_init(void* ptr) {
    int ret                = 0;
    aiq_autoptr_t* autoptr = (aiq_autoptr_t*)aiq_mallocz(sizeof(aiq_autoptr_t));

    if (!autoptr) goto failed;
    autoptr->_mutex = (AiqMutex_t*)aiq_mallocz(sizeof(AiqMutex_t));

    if (!autoptr->_mutex) {
        LOGE("%s:alloc failed", __func__);
        goto failed;
    }

    ret = aiqMutex_init(autoptr->_mutex);
    if (ret) goto failed;

    autoptr->_pData    = ptr;
    autoptr->_ref_cnts = 1;

    return autoptr;

failed:
    if (autoptr) {
        if (autoptr->_mutex) aiq_free(autoptr->_mutex);
        aiq_free(autoptr);
    }

    return NULL;
}

int16_t aiq_autoptr_ref(void* autop) {
    int16_t ret            = 0;
    aiq_autoptr_t* autoptr = (aiq_autoptr_t*)autop;

    XCAM_ASSERT(autoptr);

    aiqMutex_lock(autoptr->_mutex);
    ret = ++autoptr->_ref_cnts;
    aiqMutex_unlock(autoptr->_mutex);

    return ret;
}

int16_t aiq_autoptr_unref(void* autop) {
    int16_t ret            = -1;
    aiq_autoptr_t* autoptr = (aiq_autoptr_t*)autop;

    XCAM_ASSERT(autoptr);

    aiqMutex_lock(autoptr->_mutex);
    if (autoptr->_ref_cnts > 0) ret = --autoptr->_ref_cnts;
    if (ret != 0) {
        aiqMutex_unlock(autoptr->_mutex);
        return ret;
    }

    aiq_free(autoptr->_pData);
    aiqMutex_unlock(autoptr->_mutex);
    aiqMutex_deInit(autoptr->_mutex);
    aiq_free(autoptr->_mutex);
    aiq_free(autoptr);

    return ret;
}
