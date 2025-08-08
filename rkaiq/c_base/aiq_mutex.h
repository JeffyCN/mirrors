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

#ifndef _AIQ_MUTEX__
#define _AIQ_MUTEX__

#include <pthread.h>
#include <sys/time.h>
#include "base/xcam_common.h"
#include "base/xcam_log.h"

#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef pthread_mutex_t AiqMutex_t;

int aiqMutex_init(AiqMutex_t* mutex);
int aiqMutex_deInit(AiqMutex_t* mutex);
int aiqMutex_lock(AiqMutex_t* mutex);
int aiqMutex_unlock(AiqMutex_t* mutex);
int aiqMutex_trylock(AiqMutex_t* mutex);

RKAIQ_END_DECLARE

#endif
