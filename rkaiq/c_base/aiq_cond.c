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

#include "aiq_cond.h"

int aiqCond_init(AiqCond_t* cond) {
    int ret = 0;
    pthread_condattr_t cond_attr;

    XCAM_ASSERT(cond);

    pthread_condattr_init(&cond_attr);
    pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);

    ret = pthread_cond_init(cond, &cond_attr);
    if (ret != 0) {
        XCAM_LOG_WARNING("Cond init failed %s", strerror(ret));
    }

    pthread_condattr_destroy(&cond_attr);

    return ret;
}

int aiqCond_deInit(AiqCond_t* cond) {
    int ret = 0;

    XCAM_ASSERT(cond);

    ret = pthread_cond_destroy(cond);
    if (ret != 0) {
        XCAM_LOG_WARNING("Cond destroy failed %s", strerror(ret));
    }

    return ret;
}

int aiqCond_wait(AiqCond_t* cond, AiqMutex_t* mutex) {
    int ret = 0;

    XCAM_ASSERT(cond);
    XCAM_ASSERT(mutex);

    ret = pthread_cond_wait(cond, mutex);
    if (ret != 0) {
        XCAM_LOG_WARNING("Cond wait failed %s", strerror(ret));
    }

    return ret;
}

int aiqCond_timedWait(AiqCond_t* cond, AiqMutex_t* mutex, int32_t timeout_us) {
    struct timespec now;
    struct timespec abstime;

    XCAM_ASSERT(cond);
    XCAM_ASSERT(mutex);

	if (timeout_us == -1) {
		return pthread_cond_wait(cond, mutex);
	} else {
        clock_gettime(CLOCK_MONOTONIC, &now);
		xcam_mem_clear(abstime);
        abstime.tv_sec = now.tv_sec + timeout_us / 1000000;
        abstime.tv_nsec = now.tv_nsec + (timeout_us % 1000000) * 1000;

        if (abstime.tv_nsec >= 1000000000) {
            abstime.tv_sec += 1;
            abstime.tv_nsec -= 1000000000;
        }

        return pthread_cond_timedwait(cond, mutex, &abstime);
	}
}

int aiqCond_signal(AiqCond_t* cond) {
    XCAM_ASSERT(cond);
    return pthread_cond_signal(cond);
}

int aiqCond_broadcast(AiqCond_t* cond) {
    XCAM_ASSERT(cond);
    return pthread_cond_broadcast(cond);
}
