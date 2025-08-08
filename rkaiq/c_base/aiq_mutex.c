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

#include "aiq_mutex.h"

int aiqMutex_init(AiqMutex_t* mutex) {
    int ret = 0;

    XCAM_ASSERT(mutex);

    ret = pthread_mutex_init(mutex, NULL);

    if (ret != 0) {
        XCAM_LOG_WARNING("Mutex init failed %s", strerror(ret));
    }

    return ret;
}

int aiqMutex_deInit(AiqMutex_t* mutex) {
    int ret = 0;

    XCAM_ASSERT(mutex);

    ret = pthread_mutex_destroy(mutex);

    if (ret != 0) {
        XCAM_LOG_WARNING("Mutex destroy failed  %s", strerror(ret));
    }

    return ret;
}

int aiqMutex_lock(AiqMutex_t* mutex) {
    int ret = 0;

    XCAM_ASSERT(mutex);

    ret = pthread_mutex_lock(mutex);

    if (ret != 0) {
        XCAM_LOG_WARNING("Mutex lock failed %s", strerror(ret));
    }

    return ret;
}

int aiqMutex_unlock(AiqMutex_t* mutex) {
    int ret = 0;

    XCAM_ASSERT(mutex);

    ret = pthread_mutex_unlock(mutex);
    if (ret != 0) {
        XCAM_LOG_WARNING("Mutex unlock failed %s", strerror(ret));
    }

    return ret;
}

int aiqMutex_trylock(AiqMutex_t* mutex) {
    XCAM_ASSERT(mutex);

    return pthread_mutex_trylock(mutex);
}
