/*
 * poll_thread.cpp - poll thread for event and buffer
 *
 *  Copyright (c) 2014-2015 Intel Corporation
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

#include "poll_thread.h"
#include "xcam_thread.h"
#include <linux/rkisp.h>
#include <unistd.h>
#include <fcntl.h>

namespace XCam {

class PollThread;

class ISP3AStatsPollThread
    : public Thread
{
public:
    ISP3AStatsPollThread (PollThread *poll)
        : Thread ("stats_poll")
        , _poll (poll)
    {}

protected:
    virtual bool started () {
        XCamReturn ret = _poll->init_3a_stats_pool ();
        if (ret != XCAM_RETURN_NO_ERROR)
            return false;
        return true;
    }
    virtual bool loop () {
        XCamReturn ret = _poll->poll_isp_stats_loop ();

        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT)
            return true;
        return false;
    }

private:
    PollThread   *_poll;
};

class EventPollThread
    : public Thread
{
public:
    EventPollThread (PollThread *poll)
        : Thread ("event_poll")
        , _poll (poll)
    {}

protected:
    virtual bool started () {
        XCamReturn ret = _poll->init_3a_stats_pool ();
        if (ret != XCAM_RETURN_NO_ERROR)
            return false;
        return true;
    }
    virtual bool loop () {
        XCamReturn ret = _poll->poll_subdev_event_loop ();

        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT)
            return true;
        return false;
    }

private:
    PollThread   *_poll;
};

class CapturePollThread
    : public Thread
{
public:
    CapturePollThread (PollThread *poll)
        : Thread ("capture_poll")
        , _poll (poll)
    {}

protected:
    virtual bool loop () {
        XCamReturn ret = _poll->poll_buffer_loop ();

        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT)
            return true;
        return false;
    }

private:
    PollThread   *_poll;
};

const int PollThread::default_subdev_event_timeout = 100; // ms
const int PollThread::default_capture_event_timeout = 100; // ms
const int PollThread::default_isp_event_timeout = 1000; // ms


PollThread::PollThread ()
    : _poll_callback (NULL)
    , _stats_callback (NULL)
    , frameid (0)
{
    SmartPtr<EventPollThread> event_loop = new EventPollThread(this);
    XCAM_ASSERT (event_loop.ptr ());
    _event_loop = event_loop;

    SmartPtr<ISP3AStatsPollThread> isp_loop = new ISP3AStatsPollThread(this);
    XCAM_ASSERT (isp_loop.ptr ());
    _isp_loop = isp_loop;

    SmartPtr<CapturePollThread> capture_loop = new CapturePollThread (this);
    XCAM_ASSERT (capture_loop.ptr ());
    _capture_loop = capture_loop;

    _3a_stats_poll_stop_fd[0] =  -1;
    _3a_stats_poll_stop_fd[1] =  -1;
    _event_poll_stop_fd[0] = -1;
    _event_poll_stop_fd[1] = -1;
    _capture_poll_stop_fd[0] = -1;
    _capture_poll_stop_fd[1] = -1;

    XCAM_LOG_DEBUG ("PollThread constructed");
}

PollThread::~PollThread ()
{
    stop();

    XCAM_LOG_DEBUG ("~PollThread destructed");
}

bool
PollThread::set_capture_device (SmartPtr<V4l2Device> &dev)
{
    XCAM_ASSERT (!_capture_dev.ptr());
    _capture_dev = dev;
    return true;
}

bool
PollThread::set_event_device (SmartPtr<V4l2SubDevice> &dev)
{
    XCAM_ASSERT (!_event_dev.ptr());
    _event_dev = dev;
    return true;
}

bool
PollThread::set_isp_stats_device (SmartPtr<V4l2Device> &dev)
{
    XCAM_ASSERT (!_isp_stats_dev.ptr());
    _isp_stats_dev = dev;
    return true;
}

bool
PollThread::set_poll_callback (PollCallback *callback)
{
    XCAM_ASSERT (!_poll_callback);
    _poll_callback = callback;
    return true;
}

bool
PollThread::set_stats_callback (StatsCallback *callback)
{
    XCAM_ASSERT (!_stats_callback);
    _stats_callback = callback;
    return true;
}

void PollThread::destroy_stop_fds () {
    if (_3a_stats_poll_stop_fd[1] != -1 || _3a_stats_poll_stop_fd[0] != -1) {
        close(_3a_stats_poll_stop_fd[0]);
        close(_3a_stats_poll_stop_fd[1]);
        _3a_stats_poll_stop_fd[0] = -1;
        _3a_stats_poll_stop_fd[1] = -1;
    }

    if (_event_poll_stop_fd[1] != -1 || _event_poll_stop_fd[0] != -1) {
        close(_event_poll_stop_fd[0]);
        close(_event_poll_stop_fd[1]);
        _event_poll_stop_fd[0] = -1;
        _event_poll_stop_fd[1] = -1;
    }

    if (_capture_poll_stop_fd[1] != -1 || _capture_poll_stop_fd[0] != -1) {
        close(_capture_poll_stop_fd[0]);
        close(_capture_poll_stop_fd[1]);
        _capture_poll_stop_fd[0] = -1;
        _capture_poll_stop_fd[1] = -1;
    }
}

XCamReturn PollThread::create_stop_fds () {
    int status = 0;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    destroy_stop_fds ();

    status = pipe(_3a_stats_poll_stop_fd);
    if (status < 0) {
        XCAM_LOG_ERROR ("Failed to create stats poll stop pipe: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    /**
     * make the reading end of the pipe non blocking.
     * This helps during flush to read any information left there without
     * blocking
     */
    status = fcntl(_3a_stats_poll_stop_fd[0],F_SETFL,O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set event poll stop pipe flag: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    status = pipe(_event_poll_stop_fd);
    if (status < 0) {
        XCAM_LOG_ERROR ("Failed to create event poll stop pipe: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    /**
     * make the reading end of the pipe non blocking.
     * This helps during flush to read any information left there without
     * blocking
     */
    status = fcntl(_event_poll_stop_fd[0],F_SETFL,O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set stats poll stop pipe flag: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    status = pipe(_capture_poll_stop_fd);
    if (status < 0) {
        XCAM_LOG_ERROR ("Failed to create capture poll stop pipe: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    /**
     * make the reading end of the pipe non blocking.
     * This helps during flush to read any information left there without
     * blocking
     */
    status = fcntl(_capture_poll_stop_fd[0],F_SETFL,O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set capture poll stop pipe flag: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    return XCAM_RETURN_NO_ERROR;
exit_error:
    destroy_stop_fds ();
    return ret;
}

XCamReturn PollThread::start ()
{
    if (create_stop_fds ()) {
        XCAM_LOG_ERROR("create stop fds failed !");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (_event_dev.ptr () && !_event_loop->start ()) {
        return XCAM_RETURN_ERROR_THREAD;
    }
    if (_isp_stats_dev.ptr () && !_isp_loop->start ()) {
        return XCAM_RETURN_ERROR_THREAD;
    }
    if (_capture_dev.ptr() && !_capture_loop->start ()) {
        return XCAM_RETURN_ERROR_THREAD;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn PollThread::stop ()
{
    XCAM_LOG_DEBUG ("PollThread stop");

    if (_event_dev.ptr ()) {
        if (_event_poll_stop_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_event_poll_stop_fd[1], &buf, sizeof(char));
            if (size != sizeof(char))
                XCAM_LOG_WARNING("Flush write not completed");
        }
        _event_loop->stop ();
    }

    if (_isp_stats_dev.ptr ()) {
        if (_3a_stats_poll_stop_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_3a_stats_poll_stop_fd[1], &buf, sizeof(char));
            if (size != sizeof(char))
                XCAM_LOG_WARNING("Flush write not completed");
        }
        _isp_loop->stop ();
    }

    if (_capture_dev.ptr()) {
        if (_capture_poll_stop_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_capture_poll_stop_fd[1], &buf, sizeof(char));
            if (size != sizeof(char))
                XCAM_LOG_WARNING("Flush write not completed");
        }
        _capture_loop->stop ();
    }

    destroy_stop_fds ();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
PollThread::init_3a_stats_pool ()
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
PollThread::capture_3a_stats (SmartPtr<X3aStats> &stats)
{
    XCAM_UNUSED (stats);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
PollThread::notify_sof (int64_t time, int frameid)
{
    XCAM_UNUSED (time);
    XCAM_UNUSED (frameid);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
PollThread::handle_events (struct v4l2_event &event)
{
    XCAM_UNUSED (event);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
PollThread::handle_frame_sync_event (struct v4l2_event &event)
{
    int64_t tv_sec = event.timestamp.tv_sec;
    int64_t tv_nsec = event.timestamp.tv_nsec;
    int exp_id = event.u.frame_sync.frame_sequence;

    notify_sof(tv_sec * 1000 * 1000 * 1000 + tv_nsec, exp_id);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
PollThread::handle_3a_stats_event (struct v4l2_event &event)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SmartPtr<X3aStats> stats;

    ret = capture_3a_stats (stats);
    if (ret != XCAM_RETURN_NO_ERROR || !stats.ptr()) {
        XCAM_LOG_WARNING ("capture 3a stats failed");
        return ret;
    }
    stats->set_timestamp (XCAM_TIMESPEC_2_USEC (event.timestamp));

    if (_stats_callback)
        return _stats_callback->x3a_stats_ready (stats);

    return ret;
}

XCamReturn
PollThread::poll_isp_stats_loop ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int poll_ret = 0;
    struct v4l2_event event;

    poll_ret = _isp_stats_dev->poll_event (PollThread::default_isp_event_timeout,
                                           _3a_stats_poll_stop_fd[0]);

    if (poll_ret == POLL_STOP_RET) {
        XCAM_LOG_DEBUG ("poll stats stop success !");
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret < 0) {
        XCAM_LOG_WARNING("poll buffer event got error but continue");
        ::usleep (1000); // 1ms
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    /* timeout */
    if (poll_ret == 0) {
        XCAM_LOG_WARNING ("poll buffer timeout and continue");
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    event.type = V4L2_EVENT_RKISP_3A_STATS_READY;
    ret = handle_events (event);

    return ret;

}

XCamReturn
PollThread::poll_subdev_event_loop ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct v4l2_event event;
    int poll_ret = 0;

    poll_ret = _event_dev->poll_event (PollThread::default_subdev_event_timeout,
                                       _event_poll_stop_fd[0]);

    if (poll_ret == POLL_STOP_RET) {
        XCAM_LOG_DEBUG ("poll event stop success !");
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret < 0) {
        XCAM_LOG_WARNING ("poll event failed but continue");
        ::usleep (1000); // 1ms
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    /* timeout */
    if (poll_ret == 0) {
        XCAM_LOG_DEBUG ("poll event timeout and continue");
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    xcam_mem_clear (event);
    ret = _event_dev->dequeue_event (event);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("dequeue event failed on dev:%s", XCAM_STR(_event_dev->get_device_name()));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    ret = handle_events (event);
    return ret;
}

XCamReturn
PollThread::poll_buffer_loop ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int poll_ret = 0;
    SmartPtr<V4l2Buffer> buf;

    poll_ret = _capture_dev->poll_event (PollThread::default_capture_event_timeout,
                                         _capture_poll_stop_fd[0]);

    if (poll_ret == POLL_STOP_RET) {
        XCAM_LOG_DEBUG ("poll buffer stop success !");
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret < 0) {
        XCAM_LOG_DEBUG ("poll buffer event got error but continue");
        ::usleep (100000); // 100ms
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    /* timeout */
    if (poll_ret == 0) {
        XCAM_LOG_DEBUG ("poll buffer timeout and continue");
        XCAM_LOG_DEBUG ("dont return;");
        //return XCAM_RETURN_ERROR_TIMEOUT;
    }

    ret = _capture_dev->dequeue_buffer (buf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("capture buffer failed");
        return ret;
    }
    XCAM_ASSERT (buf.ptr());
    XCAM_ASSERT (_poll_callback);

    SmartPtr<VideoBuffer> video_buf = new V4l2BufferProxy (buf, _capture_dev);

    if (_poll_callback)
        return _poll_callback->poll_buffer_ready (video_buf);

    return ret;
}

};
