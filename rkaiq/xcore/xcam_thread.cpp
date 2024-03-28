/*
 * xcam_thread.cpp - Thread
 *
 *  Copyright (c) 2014 Intel Corporation
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
 *
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#include "xcam_thread.h"
#include "xcam_mutex.h"
#include <errno.h>
#include <signal.h>

namespace XCam {

Thread::Thread (const char *name)
    : _name (NULL)
    , _thread_id (0)
    , _started (false)
    , _stopped (true)
    , _policy (-1)
    , _priority (-1)
{
    if (name)
        _name = strndup (name, XCAM_MAX_STR_SIZE);

    XCAM_LOG_DEBUG ("Thread(%s) construction", XCAM_STR(_name));
}

Thread::~Thread ()
{
    XCAM_LOG_DEBUG ("Thread(%s) destruction", XCAM_STR(_name));

    if (_name)
        xcam_free (_name);
}

void *
Thread::thread_func (void *user_data)
{
    Thread *thread = (Thread *)user_data;
    bool ret = true;

    {
        // Make sure running after start
        SmartLock locker(thread->_mutex);
        pthread_detach (pthread_self());
    }
    ret = thread->started ();

    sigset_t set;

    /* Block SIGQUIT and SIGTERM */

    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    while (true) {
        {
            SmartLock locker(thread->_mutex);
            if (!thread->_started || ret == false) {
                thread->_started = false;
                thread->_thread_id = 0;
                ret = false;
                break;
            }
        }

        ret = thread->loop ();
    }

    thread->stopped ();

    {
        SmartLock locker(thread->_mutex);
        thread->_stopped = true;
    }
    thread->_exit_cond.broadcast ();

    return NULL;
}

bool
Thread::started ()
{
    XCAM_LOG_DEBUG ("Thread(%s) started", XCAM_STR(_name));
    return true;
}

void
Thread::stopped ()
{
    XCAM_LOG_DEBUG ("Thread(%s) stopped", XCAM_STR(_name));
}

bool Thread::start ()
{
    SmartLock locker(_mutex);
    if (_started)
        return true;

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

    if (_policy != -1 || _priority != -1) {
        int ret = -1;

        if (_policy != -1) {
            ret = pthread_attr_setschedpolicy(&attr, _policy);
            if (ret) {
                XCAM_LOG_WARNING ("Thread(%s) set sched policy failed.(%d, %s)",
                        XCAM_STR(_name), ret, strerror(ret));
            }
        }

        if (_policy != -1 && _policy != SCHED_OTHER && _priority != -1) {
            struct sched_param param;
            param.sched_priority = _priority;
            ret = pthread_attr_setschedparam(&attr, &param);
            if (ret) {
                XCAM_LOG_WARNING ("Thread(%s) set sched priority failed.(%d, %s)",
                        XCAM_STR(_name), ret, strerror(ret));
            }
        }

        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (ret) {
            XCAM_LOG_WARNING ("Thread(%s) set sched inherit failed.(%d, %s)",
                    XCAM_STR(_name), ret, strerror(ret));
        }
    }

    if (pthread_create(&_thread_id, &attr, (void* (*)(void*))thread_func, this) != 0) {
        pthread_attr_destroy(&attr);
        return false;
    }

    pthread_attr_destroy(&attr);
    _started = true;
    _stopped = false;

#ifdef __USE_GNU
    char thread_name[16];
    xcam_mem_clear (thread_name);
    snprintf (thread_name, sizeof (thread_name), "xc:%s", XCAM_STR(_name));
    int ret = pthread_setname_np (_thread_id, thread_name);
    if (ret != 0) {
        XCAM_LOG_WARNING ("Thread(%s) set name to thread_id failed.(%d, %s)", XCAM_STR(_name), ret, strerror(ret));
    }
#endif

    return true;
}

bool
Thread::emit_stop ()
{
    SmartLock locker(_mutex);
    _started = false;
    return true;
}

bool Thread::stop ()
{
    XCAM_LOG_DEBUG ("stop thread(%s) _started: %d _stopped: %d",
                    XCAM_STR(_name), _started, _stopped);

    emit_stop();

    SmartLock locker(_mutex);
    while (!_stopped) {
        _exit_cond.wait(_mutex);
    }

    _policy = -1;
    _priority = -1;

    return true;
}

bool Thread::is_running ()
{
    SmartLock locker(_mutex);
    return _started;
}

}
