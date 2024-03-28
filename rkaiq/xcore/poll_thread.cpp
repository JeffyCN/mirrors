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
//#include <linux/rkisp.h>
#include <unistd.h>
#include <fcntl.h>

namespace XCam {

class PollThread;

const char*
PollThread::isp_poll_type_to_str[ISP_POLL_POST_MAX] =
{
    "luma_poll",
    "ispp_poll",
    "stats_poll",
    "params_poll",
    "pparams_poll",
};

class IspPollThread
    : public Thread
{
public:
    IspPollThread (PollThread *poll, int type)
        : Thread (PollThread::isp_poll_type_to_str[type])
        , _poll (poll)
        , _type (type)
    {}

protected:
    virtual bool loop () {
        XCamReturn ret = _poll->poll_buffer_loop (_type);

        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT ||
                XCAM_RETURN_BYPASS)
            return true;
        return false;
    }

private:
    PollThread   *_poll;
    int _type;
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
    virtual bool loop () {
        XCamReturn ret = _poll->poll_subdev_event_loop ();

        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT)
            return true;
        return false;
    }

private:
    PollThread   *_poll;
};

const int PollThread::default_poll_timeout = 300; // ms

PollThread::PollThread ()
    : _poll_callback (NULL)
    , frameid (0)
{
#if 0
    SmartPtr<EventPollThread> event_loop = new EventPollThread(this);
    XCAM_ASSERT (event_loop.ptr ());
    _event_loop = event_loop;

    SmartPtr<IspPollThread> isp_stats_loop = new IspPollThread(this, ISP_POLL_3A_STATS);
    XCAM_ASSERT (isp_stats_loop.ptr ());
    _isp_stats_loop = isp_stats_loop;

    SmartPtr<IspPollThread> isp_luma_loop = new IspPollThread(this, ISP_POLL_LUMA);
    XCAM_ASSERT (isp_luma_loop.ptr ());
    _isp_luma_loop = isp_luma_loop;

    SmartPtr<IspPollThread> isp_params_loop = new IspPollThread(this, ISP_POLL_PARAMS);
    XCAM_ASSERT (isp_params_loop.ptr ());
    _isp_params_loop = isp_params_loop;

    SmartPtr<IspPollThread> isp_pparams_loop = new IspPollThread(this, ISP_POLL_POST_PARAMS);
    XCAM_ASSERT (isp_pparams_loop.ptr ());
    _isp_pparams_loop = isp_pparams_loop;

    SmartPtr<IspPollThread> ispp_stats_loop = new IspPollThread(this, ISPP_POLL_STATS);
    XCAM_ASSERT (ispp_stats_loop.ptr ());
    _ispp_stats_loop = ispp_stats_loop;

    _ispp_poll_stop_fd[0] =  -1;
    _ispp_poll_stop_fd[1] =  -1;
    _luma_poll_stop_fd[0] =  -1;
    _luma_poll_stop_fd[1] =  -1;
    _3a_stats_poll_stop_fd[0] =  -1;
    _3a_stats_poll_stop_fd[1] =  -1;
    _event_poll_stop_fd[0] = -1;
    _event_poll_stop_fd[1] = -1;
    _isp_params_poll_stop_fd[0] = -1;
    _isp_params_poll_stop_fd[1] = -1;
    _isp_pparams_poll_stop_fd[0] = -1;
    _isp_pparams_poll_stop_fd[1] = -1;
#endif
    XCAM_LOG_DEBUG ("PollThread constructed");
}

PollThread::~PollThread ()
{
    stop();

    XCAM_LOG_DEBUG ("~PollThread destructed");
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
PollThread::set_isp_luma_device (SmartPtr<V4l2Device> &dev)
{
    XCAM_ASSERT (!_isp_luma_dev.ptr());
    _isp_luma_dev = dev;
    return true;
}

bool
PollThread::set_ispp_stats_device (SmartPtr<V4l2Device> &dev)
{
    XCAM_ASSERT (!_ispp_stats_dev.ptr());
    _ispp_stats_dev = dev;
    return true;
}

bool
PollThread::set_isp_params_devices (SmartPtr<V4l2Device> &params_dev,
                                    SmartPtr<V4l2Device> &post_params_dev)
{
    XCAM_ASSERT (!_isp_params_dev.ptr());
    XCAM_ASSERT (!_isp_pparams_dev.ptr());
    _isp_params_dev = params_dev;
    _isp_pparams_dev = post_params_dev;
    return true;
}

bool
PollThread::set_poll_callback (PollCallback *callback)
{
    XCAM_ASSERT (!_poll_callback);
    _poll_callback = callback;
    return true;
}

void PollThread::destroy_stop_fds () {
    if (_ispp_poll_stop_fd[1] != -1 || _ispp_poll_stop_fd[0] != -1) {
        close(_ispp_poll_stop_fd[0]);
        close(_ispp_poll_stop_fd[1]);
        _ispp_poll_stop_fd[0] = -1;
        _ispp_poll_stop_fd[1] = -1;
    }

    if (_luma_poll_stop_fd[1] != -1 || _luma_poll_stop_fd[0] != -1) {
        close(_luma_poll_stop_fd[0]);
        close(_luma_poll_stop_fd[1]);
        _luma_poll_stop_fd[0] = -1;
        _luma_poll_stop_fd[1] = -1;
    }

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

    if (_isp_params_poll_stop_fd[1] != -1 || _isp_params_poll_stop_fd[0] != -1) {
        close(_isp_params_poll_stop_fd[0]);
        close(_isp_params_poll_stop_fd[1]);
        _isp_params_poll_stop_fd[0] = -1;
        _isp_params_poll_stop_fd[1] = -1;
    }

    if (_isp_pparams_poll_stop_fd[1] != -1 || _isp_pparams_poll_stop_fd[0] != -1) {
        close(_isp_pparams_poll_stop_fd[0]);
        close(_isp_pparams_poll_stop_fd[1]);
        _isp_pparams_poll_stop_fd[0] = -1;
        _isp_pparams_poll_stop_fd[1] = -1;
    }
}

XCamReturn PollThread::create_stop_fds () {
    int status = 0;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    destroy_stop_fds ();

    status = pipe(_ispp_poll_stop_fd);
    if (status < 0) {
        XCAM_LOG_ERROR ("Failed to create ispp poll stop pipe: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    /**
     * make the reading end of the pipe non blocking.
     * This helps during flush to read any information left there without
     * blocking
     */
    status = fcntl(_ispp_poll_stop_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set event ispp stop pipe flag: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    status = pipe(_luma_poll_stop_fd);
    if (status < 0) {
        XCAM_LOG_ERROR ("Failed to create luma poll stop pipe: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    /**
     * make the reading end of the pipe non blocking.
     * This helps during flush to read any information left there without
     * blocking
     */
    status = fcntl(_luma_poll_stop_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set event luma stop pipe flag: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

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
    status = fcntl(_3a_stats_poll_stop_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set stats poll stop pipe flag: %s", strerror(errno));
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
    status = fcntl(_event_poll_stop_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set stats poll stop pipe flag: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    status = pipe(_isp_params_poll_stop_fd);
    if (status < 0) {
        XCAM_LOG_ERROR ("Failed to create params poll stop pipe: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    /**
     * make the reading end of the pipe non blocking.
     * This helps during flush to read any information left there without
     * blocking
     */
    status = fcntl(_isp_params_poll_stop_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set params poll stop pipe flag: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    status = pipe(_isp_pparams_poll_stop_fd);
    if (status < 0) {
        XCAM_LOG_ERROR ("Failed to create pparams poll stop pipe: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    /**
     * make the reading end of the pipe non blocking.
     * This helps during flush to read any information left there without
     * blocking
     */
    status = fcntl(_isp_pparams_poll_stop_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        XCAM_LOG_ERROR ("Fail to set pparams poll stop pipe flag: %s", strerror(errno));
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

    if (_isp_stats_dev.ptr () && !_isp_stats_loop->start ()) {
        return XCAM_RETURN_ERROR_THREAD;
    }

    if (_isp_luma_dev.ptr () && !_isp_luma_loop->start ()) {
        return XCAM_RETURN_ERROR_THREAD;
    }

    if (_isp_params_dev.ptr () && !_isp_params_loop->start ()) {
        return XCAM_RETURN_ERROR_THREAD;
    }

    if (_ispp_stats_dev.ptr () && !_ispp_stats_loop->start ()) {
        return XCAM_RETURN_ERROR_THREAD;
    }

    if (_isp_pparams_dev.ptr () && !_isp_pparams_loop->start ()) {
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
            if (size != sizeof(char)) {
                XCAM_LOG_WARNING("Flush write not completed");
            }
        }
        _event_loop->stop ();
    }

    if (_ispp_stats_dev.ptr ()) {
        if (_ispp_poll_stop_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_ispp_poll_stop_fd[1], &buf, sizeof(char));
            if (size != sizeof(char)) {
                XCAM_LOG_WARNING("Flush write not completed");
            }
        }
        _ispp_stats_loop->stop ();
    }

    if (_isp_stats_dev.ptr ()) {
        if (_3a_stats_poll_stop_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_3a_stats_poll_stop_fd[1], &buf, sizeof(char));
            if (size != sizeof(char)) {
                XCAM_LOG_WARNING("Flush write not completed");
            }
        }
        _isp_stats_loop->stop ();
    }

    if (_isp_luma_dev.ptr ()) {
        if (_luma_poll_stop_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_luma_poll_stop_fd[1], &buf, sizeof(char));
            if (size != sizeof(char)) {
                XCAM_LOG_WARNING("Flush write not completed");
            }
        }
        _isp_luma_loop->stop ();
    }

    if (_isp_params_dev.ptr ()) {
        if (_isp_params_poll_stop_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_isp_params_poll_stop_fd[1], &buf, sizeof(char));
            if (size != sizeof(char)) {
                XCAM_LOG_WARNING("Flush write not completed");
            }
        }
        _isp_params_loop->stop ();
    }

    if (_isp_pparams_dev.ptr ()) {
        if (_isp_pparams_poll_stop_fd[1] != -1) {
            char buf = 0xf;  // random value to write to flush fd.
            unsigned int size = write(_isp_pparams_poll_stop_fd[1], &buf, sizeof(char));
            if (size != sizeof(char)) {
                XCAM_LOG_WARNING("Flush write not completed");
            }
        }
        _isp_pparams_loop->stop ();
    }

    destroy_stop_fds ();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
PollThread::notify_sof (uint64_t time, uint32_t frameid)
{
    XCAM_UNUSED (time);
    XCAM_UNUSED (frameid);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
PollThread::handle_events (struct v4l2_event &event)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    switch (event.type) {
    case V4L2_EVENT_FRAME_SYNC:
        ret = handle_frame_sync_event (event);
        break;
    default:
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        break;
    }

    return ret;
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
PollThread::poll_subdev_event_loop ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    struct v4l2_event event;
    int poll_ret = 0;

    poll_ret = _event_dev->poll_event (PollThread::default_poll_timeout,
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
        XCAM_LOG_WARNING ("poll event timeout and continue");
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    xcam_mem_clear (event);
    ret = _event_dev->dequeue_event (event);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("dequeue event failed on dev:%s", XCAM_STR(_event_dev->get_device_name()));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    ret = handle_events (event);

    XCAM_ASSERT (_poll_callback);

    if (_poll_callback && event.type == V4L2_EVENT_FRAME_SYNC)
        return _poll_callback->poll_event_ready (event.u.frame_sync.frame_sequence,
                                                 event.type);

    return ret;
}

SmartPtr<VideoBuffer>
PollThread::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                             SmartPtr<V4l2Device> dev,
                             int type)
{
    SmartPtr<VideoBuffer> video_buf = new V4l2BufferProxy (buf, dev);

    return video_buf;
}

XCamReturn
PollThread::poll_buffer_loop (int type)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    int poll_ret = 0;
    SmartPtr<V4l2Buffer> buf;
    SmartPtr<V4l2Device> dev;
    int stop_fd = -1;

    if (type == ISP_POLL_LUMA) {
        dev = _isp_luma_dev;
        stop_fd = _luma_poll_stop_fd[0];
    } else if (type == ISPP_POLL_STATS) {
        dev = _ispp_stats_dev;
        stop_fd = _ispp_poll_stop_fd[0];
    } else if (type == ISP_POLL_3A_STATS) {
        dev = _isp_stats_dev;
        stop_fd = _3a_stats_poll_stop_fd[0];
    } else if (type == ISP_POLL_PARAMS) {
        dev = _isp_params_dev;
        stop_fd = _isp_params_poll_stop_fd[0];
    } else if (type == ISP_POLL_POST_PARAMS) {
        dev = _isp_pparams_dev;
        stop_fd = _isp_pparams_poll_stop_fd[0];
    } else
        return XCAM_RETURN_ERROR_UNKNOWN;

    poll_ret = dev->poll_event (PollThread::default_poll_timeout,
                                stop_fd);

    if (poll_ret == POLL_STOP_RET) {
        XCAM_LOG_DEBUG ("poll %s buffer stop success !", isp_poll_type_to_str[type]);
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret <= 0) {
        XCAM_LOG_DEBUG ("poll %s buffer event got error(0x%x) but continue\n", isp_poll_type_to_str[type], poll_ret);
        ::usleep (100000); // 100ms
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    ret = dev->dequeue_buffer (buf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("dequeue %s buffer failed", isp_poll_type_to_str[type]);
        return ret;
    }

    XCAM_ASSERT (buf.ptr());
    XCAM_ASSERT (_poll_callback);

    if (_poll_callback &&
            (type == ISP_POLL_3A_STATS || type == ISP_POLL_LUMA || type == ISPP_POLL_STATS)) {
        SmartPtr<VideoBuffer> video_buf = new_video_buffer(buf, dev, type);

        return _poll_callback->poll_buffer_ready (video_buf, type);
    }
#endif
    return ret;
}

}
