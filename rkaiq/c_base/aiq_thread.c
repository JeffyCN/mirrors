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

#include <sys/prctl.h>
#include <signal.h>
#include "aiq_thread.h"
#include "c_base/aiq_base.h"
#include "base/xcam_log.h"

AiqThread_t* aiqThread_init(const char* name, AiqThread_func loop, void* arg) {
    int ret               = 0;
    AiqThread_t* aiqThCtx = (AiqThread_t*)aiq_mallocz(sizeof(AiqThread_t));

    if (!aiqThCtx) {
        LOGE("%s: malloc failed!", __func__);
        return NULL;
    }

    memset(aiqThCtx, 0, sizeof(AiqThread_t));

    if (name)
        aiqThCtx->_name = strdup(name);
    else
        aiqThCtx->_name = strdup("noname");

    ret = aiqMutex_init(&aiqThCtx->_mutex);
    if (ret != 0) {
        goto clean_up_name;
    }

    ret = aiqCond_init(&aiqThCtx->_exit_cond);
    if (ret != 0) {
        goto clean_up_mutex;
    }

    aiqThCtx->_loop_fun     = loop;
    aiqThCtx->_loop_fun_arg = arg;
    aiqThCtx->_started      = false;
    aiqThCtx->_stopped      = true;
    aiqThCtx->_policy       = -1;
    aiqThCtx->_priority     = -1;

    return aiqThCtx;

clean_up_mutex:
    aiqMutex_deInit(&aiqThCtx->_mutex);
clean_up_name:
    if (aiqThCtx->_name) aiq_free(aiqThCtx->_name);
clean_up_ctx:
    aiq_free(aiqThCtx);

    return NULL;
}

void aiqThread_deinit(AiqThread_t* thread) {
    XCAM_ASSERT(thread);

    aiqMutex_lock(&thread->_mutex);

    if (!thread->_stopped || thread->_started) {
        LOGE("th %s is running !", thread->_name);
        aiqMutex_unlock(&thread->_mutex);
        return;
    }

    if (thread->_name) free(thread->_name);

    aiqCond_deInit(&thread->_exit_cond);
    aiqMutex_unlock(&thread->_mutex);
    aiqMutex_deInit(&thread->_mutex);

    free(thread);
}

static void* thread_func(void* user_data) {
    AiqThread_t* thread = (AiqThread_t*)user_data;
    bool ret            = true;

    {
        // Make sure running after start
        aiqMutex_lock(&thread->_mutex);
        pthread_detach(pthread_self());
        aiqMutex_unlock(&thread->_mutex);
    }

    sigset_t set;

    /* Block SIGQUIT and SIGTERM */

    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (thread->_name) {
        char thread_name[16];
        xcam_mem_clear(thread_name);
        snprintf(thread_name, sizeof(thread_name), "xc:%s", XCAM_STR(thread->_name));
        prctl(PR_SET_NAME, thread_name);
    }
    while (true) {
        {
            aiqMutex_lock(&thread->_mutex);
            if (!thread->_started || ret == false) {
                thread->_started   = false;
                thread->_thread_id = 0;
                ret                = false;
                aiqMutex_unlock(&thread->_mutex);
                break;
            }
            aiqMutex_unlock(&thread->_mutex);
        }

        ret = thread->_loop_fun(thread->_loop_fun_arg);
    }

    {
        aiqMutex_lock(&thread->_mutex);
        thread->_stopped = true;
        aiqMutex_unlock(&thread->_mutex);
    }
    aiqCond_broadcast(&thread->_exit_cond);

    return NULL;
}

bool aiqThread_start(AiqThread_t* thread) {
    XCAM_ASSERT(thread);

    aiqMutex_lock(&thread->_mutex);
    if (thread->_started) {
        aiqMutex_unlock(&thread->_mutex);
        return true;
    }

    pthread_attr_t attr;
    size_t stacksize = 0;

    if (pthread_attr_init(&attr)) {
        XCAM_ASSERT(0);
    }
    if (!pthread_attr_getstacksize(&attr, &stacksize)) {
        if (stacksize < 2048 * 1024) {
            stacksize = 2048 * 1024;
        }
        pthread_attr_setstacksize(&attr, stacksize);
    }

    if (thread->_policy != -1 || thread->_priority != -1) {
        int ret = -1;

        if (thread->_policy != -1) {
            ret = pthread_attr_setschedpolicy(&attr, thread->_policy);
            if (ret) {
                XCAM_LOG_WARNING("Thread(%s) set sched policy failed.(%d, %s)",
                                 XCAM_STR(thread->_name), ret, strerror(ret));
            }
        }

        if (thread->_policy != -1 && thread->_policy != SCHED_OTHER && thread->_priority != -1) {
            struct sched_param param;
            param.sched_priority = thread->_priority;
            ret                  = pthread_attr_setschedparam(&attr, &param);
            if (ret) {
                XCAM_LOG_WARNING("Thread(%s) set sched priority failed.(%d, %s)",
                                 XCAM_STR(thread->_name), ret, strerror(ret));
            }
        }

        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (ret) {
            XCAM_LOG_WARNING("Thread(%s) set sched inherit failed.(%d, %s)",
                             XCAM_STR(thread->_name), ret, strerror(ret));
        }
    }

    if (pthread_create(&thread->_thread_id, &attr, (void* (*)(void*))thread_func, thread) != 0) {
        pthread_attr_destroy(&attr);
        aiqMutex_unlock(&thread->_mutex);
        XCAM_LOG_ERROR("Thread(%s) create fail !", XCAM_STR(thread->_name));
        return false;
    }

    pthread_attr_destroy(&attr);
    thread->_started = true;
    thread->_stopped = false;

#ifdef __USE_GNU
    char thread_name[16];
    xcam_mem_clear(thread_name);
    snprintf(thread_name, sizeof(thread_name), "xc:%s", XCAM_STR(_name));
    int ret = pthread_setname_np(thread->_thread_id, thread_name);
    if (ret != 0) {
        XCAM_LOG_WARNING("Thread(%s) set name to thread_id failed.(%d, %s)",
                         XCAM_STR(thread->_name), ret, strerror(ret));
    }
#endif
    aiqMutex_unlock(&thread->_mutex);
    XCAM_LOG_INFO("start thread(%s) success !", XCAM_STR(thread->_name),
                   thread->_started, thread->_stopped);

    return true;
}

bool aiqThread_stop(AiqThread_t* thread) {
    XCAM_ASSERT(thread);

    aiqMutex_lock(&thread->_mutex);
    XCAM_LOG_DEBUG("stop thread(%s) _started: %d _stopped: %d", XCAM_STR(thread->_name),
                   thread->_started, thread->_stopped);

    thread->_started = false;

    while (!thread->_stopped) {
        aiqCond_wait(&thread->_exit_cond, &thread->_mutex);
    }

    aiqMutex_unlock(&thread->_mutex);

    return true;
}

int aiqThread_setPolicy(AiqThread_t* thread, int policy) {
    XCAM_ASSERT(thread);

    aiqMutex_lock(&thread->_mutex);

    if (!thread->_stopped || thread->_started) {
        LOGE("th %s is running !", thread->_name);
        aiqMutex_unlock(&thread->_mutex);
        return 0;
    }

    thread->_policy = policy;

    aiqMutex_unlock(&thread->_mutex);

    return -1;
}

int aiqThread_setPriority(AiqThread_t* thread, int priority) {
    XCAM_ASSERT(thread);

    aiqMutex_lock(&thread->_mutex);

    if (!thread->_stopped || thread->_started) {
        LOGE("th %s is running !", thread->_name);
        aiqMutex_unlock(&thread->_mutex);
        return -1;
    }

    thread->_priority = priority;

    aiqMutex_unlock(&thread->_mutex);

    return 0;
}
