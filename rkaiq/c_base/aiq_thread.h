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

#ifndef _AIQ_THREAD__
#define _AIQ_THREAD__

#include "aiq_cond.h"
#include "rk_aiq_comm.h"

RKAIQ_BEGIN_DECLARE

typedef bool (*AiqThread_func)(void* user_data);

typedef struct AiqThread_s {
    char* _name;
    pthread_t _thread_id;
    AiqMutex_t _mutex;
    AiqCond_t _exit_cond;
    AiqThread_func _loop_fun;
    void* _loop_fun_arg;
    int _policy;
    int _priority;
    bool _started;
    bool _stopped;
} AiqThread_t;

/*!
 * \brief create AiqThread_t object
 *
 * \param[in] name  thread name
 * \param[in] proc  thread fucn pointer
 * \param[in] arg   arg of thread func
 * \return    not NULL if success
 */
AiqThread_t* aiqThread_init(const char* name, AiqThread_func proc, void* arg);
void aiqThread_deinit(AiqThread_t* thread);
bool aiqThread_start(AiqThread_t* thread);
bool aiqThread_stop(AiqThread_t* thread);
/*!
 * \brief set policy of the thread
 *
 * \param[in] thread created by aiqThread_init
 * \param[in] policy values: SCHED_FIFO, SCHED_RR, and SCHED_OTHER
 * \return    true if success
 * \note should be called before start
 */
int aiqThread_setPolicy(AiqThread_t* thread, int policy);
/*!
 * \brief set priority of the thread
 *
 * \param[in] thread created by aiqThread_init
 * \param[in] priority
 * \return    return the nearest priority
 * \note should be called before start
 */
int aiqThread_setPriority(AiqThread_t* thread, int priority);

RKAIQ_END_DECLARE

#endif
