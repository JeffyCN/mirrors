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

#ifndef _AIQ_COND__
#define _AIQ_COND__

#include "aiq_mutex.h"
#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef pthread_cond_t AiqCond_t;

int aiqCond_init(AiqCond_t* cond);
int aiqCond_deInit(AiqCond_t* cond);
int aiqCond_wait(AiqCond_t* cond, AiqMutex_t* mutex);
int aiqCond_timedWait(AiqCond_t* cond, AiqMutex_t* mutex, int32_t timeout_us);
int aiqCond_signal(AiqCond_t* cond);
int aiqCond_broadcast(AiqCond_t* cond);

RKAIQ_END_DECLARE

#endif
