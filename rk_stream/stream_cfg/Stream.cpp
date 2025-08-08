/*
 *  Copyright (c) 2019 Rockchip Corporation
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
 */

#include <stdlib.h>
#include <fcntl.h>
#include "Stream.h"

namespace RkRawStream {

const int RkPollThread::default_poll_timeout = 300; // ms

const char*
RKStream::poll_type_to_str[RKRAWSTREAM_POLL_TYPE_MAX] =
{
    "rkrawstream_poll_vicap",
    "rkrawstream_poll_readback",
    "rkrawstream_poll_isp",
};

RkPollThread::RkPollThread (const char* thName, int type, SmartPtr<V4l2Device> dev, RKStream *stream)
    :Thread(thName)
    ,_poll_callback (NULL)
    ,frameid (0)
    ,_dev(dev)
    ,_stream(stream)
    ,_dev_type(type)
{
    _poll_stop_fd[0] =  -1;
    _poll_stop_fd[1] =  -1;

    LOGD_RKSTREAM ("RkPollThread [%s] constructed", get_name());
}

RkPollThread::RkPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> dev, RKStream *stream)
    :Thread(thName)
    ,_poll_callback (NULL)
    ,frameid (0)
    ,_subdev(dev)
    ,_dev(dev)
    ,_stream(stream)
    ,_dev_type(type)
{
    _poll_stop_fd[0] =  -1;
    _poll_stop_fd[1] =  -1;

    LOGD_RKSTREAM ("RkPollThread [%s] constructed", get_name());
}

RkPollThread::~RkPollThread ()
{
    stop();
    LOGD_RKSTREAM ("~RkPollThread [%s] destructed", get_name());
}

void RkPollThread::destroy_stop_fds () {
    if (_poll_stop_fd[1] != -1 || _poll_stop_fd[0] != -1) {
        close(_poll_stop_fd[0]);
        close(_poll_stop_fd[1]);
        _poll_stop_fd[0] = -1;
        _poll_stop_fd[1] = -1;
    }
}

XCamReturn RkPollThread::create_stop_fds () {
    int status = 0;
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    destroy_stop_fds ();

    status = pipe(_poll_stop_fd);
    if (status < 0) {
        LOGE_RKSTREAM ("Failed to create ispp poll stop pipe: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    /**
     * make the reading end of the pipe non blocking.
     * This helps during flush to read any information left there without
     * blocking
     */
    status = fcntl(_poll_stop_fd[0], F_SETFL, O_NONBLOCK);
    if (status < 0) {
        LOGE_RKSTREAM ("Fail to set event ispp stop pipe flag: %s", strerror(errno));
        ret = XCAM_RETURN_ERROR_UNKNOWN;
        goto exit_error;
    }

    return XCAM_RETURN_NO_ERROR;
exit_error:
    destroy_stop_fds ();
    return ret;
}

XCamReturn RkPollThread::start ()
{
    if (create_stop_fds ()) {
        LOGE_RKSTREAM("create stop fds failed !");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    Thread::start();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkPollThread::stop ()
{
    if (_poll_stop_fd[1] != -1) {
        char buf = 0xf;  // random value to write to flush fd.
        unsigned int size = write(_poll_stop_fd[1], &buf, sizeof(char));
        if (size != sizeof(char))
            LOGW_RKSTREAM("Flush write not completed");
    }
    Thread::stop();
    destroy_stop_fds ();
    LOGD_RKSTREAM ("RkPollThread stop done");
    return XCAM_RETURN_NO_ERROR;
}

bool
RkPollThread::setPollCallback (PollCallback *callback)
{
    //XCAM_ASSERT (!_poll_callback);
    _poll_callback = callback;
    return true;
}

XCamReturn
RkPollThread::poll_buffer_loop ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int poll_ret = 0;
    SmartPtr<V4l2Buffer> buf;
    int stop_fd = -1;

    stop_fd = _poll_stop_fd[0];
    poll_ret = _dev->poll_event (RkPollThread::default_poll_timeout, stop_fd);

    if (poll_ret == POLL_STOP_RET) {
        LOGD_RKSTREAM ("poll buffer stop success !");
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret < 0 && (errno == EAGAIN || errno == EINTR)) {
        LOGW_RKSTREAM("poll buffer event got interrupt(0x%x), continue\n",
                       poll_ret);
        return XCAM_RETURN_ERROR_TIMEOUT;
    } else if (poll_ret < 0) {
        LOGW_RKSTREAM("%s poll buffer event got error(0x%x) exit\n",
                       XCAM_STR(_dev->get_device_name()), poll_ret);
        return XCAM_RETURN_ERROR_UNKNOWN;
    } else if (poll_ret == 0) {
        LOGW_RKSTREAM("%s poll buffer event timeout(0x%x), continue\n",
                       XCAM_STR(_dev->get_device_name()), poll_ret);
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    ret = _dev->dequeue_buffer (buf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGW_RKSTREAM ("%s: dequeue buffer failed",XCAM_STR(_dev->get_device_name()));
        return ret;
    }
    if (_dev_type == ISP_POLL_TX){
         LOGD_RKSTREAM ("%s: dequeue buffer ok, seq %d", XCAM_STR(_dev->get_device_name()), buf->get_buf().sequence);
    }
    XCAM_ASSERT (buf.ptr());

    SmartPtr<V4l2BufferProxy> buf_proxy = _stream->new_v4l2proxy_buffer(buf, _dev);
    if (_poll_callback && buf_proxy.ptr())
        _poll_callback->poll_buffer_ready (buf_proxy, ((RKRawStream*)_stream)->_dev_index);

    return ret;
}

RkEventPollThread::RkEventPollThread (const char* thName, int type, SmartPtr<V4l2Device> dev, RKStream *stream)
    :RkPollThread(thName, type, dev, stream)
{
    LOGD_RKSTREAM ("RkEventPollThread  [%s] constructed", get_name());
}

RkEventPollThread::RkEventPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> subdev, RKStream *stream)
    :RkPollThread(thName, type, subdev, stream)
{
    LOGD_RKSTREAM ("RkEventPollThread  [%s] constructed", get_name());
}

RkEventPollThread::~RkEventPollThread ()
{
    stop();
    LOGD_RKSTREAM ("~RkEventPollThread [%s] destructed", get_name());
}

XCamReturn
RkEventPollThread::poll_event_loop ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    int poll_ret = 0;
    int stop_fd = -1;

    stop_fd = _poll_stop_fd[0];
    poll_ret = _dev->poll_event (RkPollThread::default_poll_timeout, stop_fd);

    if (poll_ret == POLL_STOP_RET) {
        XCAM_LOG_INFO ("%s: poll event stop success !", get_name());
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret < 0 && (errno == EAGAIN || errno == EINTR)) {
        LOGD_RKSTREAM("poll buffer event got interrupt(0x%x), continue\n",
                       poll_ret);
        return XCAM_RETURN_ERROR_TIMEOUT;
    } else if (poll_ret < 0) {
        LOGD_RKSTREAM("poll buffer event got error(0x%x) exit\n", poll_ret);
        return XCAM_RETURN_ERROR_UNKNOWN;
    } else if (poll_ret == 0) {
        LOGW_RKSTREAM ("poll event timeout and continue");
        return XCAM_RETURN_ERROR_TIMEOUT;
    }
    xcam_mem_clear (_event);

    ret = _dev->dequeue_event (_event);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGW_RKSTREAM ("dequeue event failed on dev:%s", XCAM_STR(_dev->get_device_name()));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (_poll_callback && _stream) {
        SmartPtr<VideoBuffer> video_buf = _stream->new_video_buffer(_event, _subdev);
        _poll_callback->poll_buffer_ready (video_buf);
    }


    return ret;
}

RKStream::RKStream (SmartPtr<V4l2Device> dev, int type)
    :_dev(dev)
    ,_dev_type(type)
    ,_dev_prepared(false)
{
    _poll_thread = new RkPollThread(RKStream::poll_type_to_str[type], type, dev, this);
}

RKStream::RKStream (SmartPtr<V4l2SubDevice> dev, int type)
    :_subdev(dev)
    ,_dev_type(type)
    ,_dev_prepared(false)
{
    _poll_thread = new RkEventPollThread(RKStream::poll_type_to_str[type], type, dev, this);
}

RKStream::RKStream (const char *path, int type)
    :_dev_type(type)
    ,_dev_prepared(false)
{
    _dev = new V4l2Device(path);
    _poll_thread = new RkPollThread(RKStream::poll_type_to_str[type], type, _dev, this);
}

RKStream::~RKStream()
{
}

XCamReturn
RKStream::start()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (!_dev->is_activated())
        ret = _dev->start(_dev_prepared);
    _poll_thread->start();
    return ret;
}

void
RKStream::startThreadOnly()
{
    _poll_thread->start();
}

void
RKStream::startDeviceOnly()
{
    if (!_dev->is_activated())
        _dev->start(_dev_prepared);
}

void
RKStream::stop()
{
    _poll_thread->stop();
    _dev->stop();
    _dev_prepared = false;
}

void
RKStream::stopThreadOnly()
{
    _poll_thread->stop();
}

void
RKStream::stopDeviceOnly()
{
    _dev->stop();
    _dev_prepared = false;
}

void
RKStream::stopDeviceStreamoff()
{
    _dev->stop_streamoff();
    _dev_prepared = false;
}

void
RKStream::stopDeviceFreebuffer()
{
    _dev->stop_freebuffer();
    _dev_prepared = false;
}

void
RKStream::pause()
{
}

void
RKStream::resume()
{
}

void
RKStream::set_device_prepared(bool prepare)
{
    _dev_prepared = prepare;
}

SmartPtr<VideoBuffer> RKStream::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                       SmartPtr<V4l2Device> dev)
{
    SmartPtr<VideoBuffer> video_buf = new V4l2BufferProxy (buf, dev);
    video_buf->_buf_type = _dev_type;
    return video_buf;
}

bool
RKStream::setPollCallback (PollCallback *callback)
{
    return _poll_thread->setPollCallback(callback);
}

XCamReturn RKStream::getFormat(struct v4l2_format &format)
{
    return _dev->get_format (format);
}

XCamReturn RKStream::getFormat(struct v4l2_subdev_format &format)
{
    return _subdev->getFormat (format);
}

/*--------------------Output stream---------------------------*/

RKRawStream::RKRawStream (SmartPtr<V4l2Device> dev, int index, int type)
    :RKStream(dev, type)
    ,_dev_index(index)
{
}

RKRawStream::~RKRawStream()
{
}

SmartPtr<V4l2BufferProxy>
RKRawStream::new_v4l2proxy_buffer(SmartPtr<V4l2Buffer> buf,
                                       SmartPtr<V4l2Device> dev)
{
    ENTER_CAMHW_FUNCTION();
    SmartPtr<V4l2BufferProxy> buf_proxy = new V4l2BufferProxy(buf, dev);
    buf_proxy->_buf_type = _dev_type;
    EXIT_CAMHW_FUNCTION();

    return buf_proxy;
}

}; //namspace RkCam
