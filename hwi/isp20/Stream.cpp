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
#include "Isp20StatsBuffer.h"
#include "rkisp2-config.h"
#include "SensorHw.h"
#include "LensHw.h"
#include "Isp20_module_dbg.h"
#include "CamHwIsp20.h"
#include "CaptureRawData.h"

namespace RkCam {

const int RkPollThread::default_poll_timeout = 300; // ms

const char*
RKStream::poll_type_to_str[ISP_POLL_POST_MAX] =
{
    "luma_poll",
    "isp_3a_stats_poll",
    "isp_param_poll",
    "ispp_fec_param_poll",
    "ispp_tnr_param_poll",
    "ispp_nr_param_poll",
    "ispp_tnr_stats_poll",
    "ispp_nr_stats_poll",
    "isp_sof_poll",
    "isp_tx_poll",
    "isp_rx_poll",
    "isp_sp_poll",
    "isp_pdaf_poll",
    "isp_stream_sync_poll",
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

    XCAM_LOG_DEBUG ("RkPollThread constructed");
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

    XCAM_LOG_DEBUG ("RkPollThread constructed");
}

RkPollThread::~RkPollThread ()
{
    stop();
    XCAM_LOG_DEBUG ("~RkPollThread destructed");
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
        XCAM_LOG_ERROR ("Failed to create ispp poll stop pipe: %s", strerror(errno));
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
        XCAM_LOG_ERROR ("Fail to set event ispp stop pipe flag: %s", strerror(errno));
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
        XCAM_LOG_ERROR("create stop fds failed !");
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    Thread::start();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkPollThread::stop ()
{
    XCAM_LOG_INFO ("RkPollThread %s:%s stop", get_name(),
                   _dev.ptr() ? _dev->get_device_name() : _subdev->get_device_name());
    if (_poll_stop_fd[1] != -1) {
        char buf = 0xf;  // random value to write to flush fd.
        unsigned int size = write(_poll_stop_fd[1], &buf, sizeof(char));
        if (size != sizeof(char))
            XCAM_LOG_WARNING("Flush write not completed");
    }
    Thread::stop();
    destroy_stop_fds ();
    XCAM_LOG_INFO ("stop done");
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
        XCAM_LOG_DEBUG ("poll buffer stop success !");
        // stop success, return error to stop the poll thread
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    if (poll_ret < 0 && (errno == EAGAIN || errno == EINTR)) {
        XCAM_LOG_DEBUG("poll buffer event got interrupt(0x%x), continue\n",
                       poll_ret);
        return XCAM_RETURN_ERROR_TIMEOUT;
    } else if (poll_ret < 0) {
        XCAM_LOG_DEBUG("poll buffer event got error(0x%x) exit\n", poll_ret);
        return XCAM_RETURN_ERROR_UNKNOWN;
    } else if (poll_ret == 0) {
        XCAM_LOG_DEBUG("poll buffer event timeout(0x%x), continue\n",
                       poll_ret);
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    ret = _dev->dequeue_buffer (buf);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("dequeue buffer failed");
        return ret;
    }

    XCAM_ASSERT (buf.ptr());

    if (_dev_type == ISP_POLL_TX || _dev_type == ISP_POLL_RX) {
        SmartPtr<V4l2BufferProxy> buf_proxy = _stream->new_v4l2proxy_buffer(buf, _dev);
        if (_poll_callback && buf_proxy.ptr())
            _poll_callback->poll_buffer_ready (buf_proxy, ((RKRawStream*)_stream)->_dev_index);
    } else if (_dev_type == ISP_POLL_PDAF_STATS) {
        SmartPtr<V4l2BufferProxy> buf_proxy = _stream->new_v4l2proxy_buffer(buf, _dev);
        if (_poll_callback && buf_proxy.ptr())
            _poll_callback->poll_buffer_ready (buf_proxy, 0);
    } else {
        SmartPtr<VideoBuffer> video_buf = _stream->new_video_buffer(buf, _dev);
        if (_poll_callback && video_buf.ptr())
            _poll_callback->poll_buffer_ready (video_buf);
    }

    return ret;
}

RkEventPollThread::RkEventPollThread (const char* thName, int type, SmartPtr<V4l2Device> dev, RKStream *stream)
    :RkPollThread(thName, type, dev, stream)
{
    XCAM_LOG_DEBUG ("RkEventPollThread constructed");
}

RkEventPollThread::RkEventPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> subdev, RKStream *stream)
    :RkPollThread(thName, type, subdev, stream)
{
    XCAM_LOG_DEBUG ("RkEventPollThread constructed");
}

RkEventPollThread::~RkEventPollThread ()
{
    stop();
    XCAM_LOG_DEBUG ("~RkEventPollThread destructed");
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
        XCAM_LOG_DEBUG("poll buffer event got interrupt(0x%x), continue\n",
                       poll_ret);
        return XCAM_RETURN_ERROR_TIMEOUT;
    } else if (poll_ret < 0) {
        XCAM_LOG_DEBUG("poll buffer event got error(0x%x) exit\n", poll_ret);
        return XCAM_RETURN_ERROR_UNKNOWN;
    } else if (poll_ret == 0) {
        XCAM_LOG_WARNING ("poll event timeout and continue");
        return XCAM_RETURN_ERROR_TIMEOUT;
    }
    xcam_mem_clear (_event);

    ret = _dev->dequeue_event (_event);
    if (ret != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_WARNING ("dequeue event failed on dev:%s", XCAM_STR(_dev->get_device_name()));
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (_poll_callback && _stream) {
        SmartPtr<VideoBuffer> video_buf = _stream->new_video_buffer(_event, _subdev);
        _poll_callback->poll_buffer_ready (video_buf);
    }


    return ret;
}

/*--------------------ISP evt poll ---------------------------*/

XCamReturn
RkStreamEventPollThread::poll_event_loop () {
    XCamReturn ret = RkEventPollThread::poll_event_loop();

    if (ret == XCAM_RETURN_NO_ERROR) {
        if (_event.type == CIFISP_V4L2_EVENT_STREAM_START) {
            XCAM_LOG_INFO ("%s: poll stream on evt success", _dev->get_device_name());
            _pIsp->notify_isp_stream_status(true);
        } else if (_event.type == CIFISP_V4L2_EVENT_STREAM_STOP) {
            XCAM_LOG_INFO ("%s: poll stream off evt success", _dev->get_device_name());
            _pIsp->notify_isp_stream_status(false);
            // quit loop
            emit_stop ();
            return XCAM_RETURN_ERROR_UNKNOWN;
        }
    }

    if (ret == XCAM_RETURN_ERROR_IOCTL) {
        // ignored for nonblock mode
        return XCAM_RETURN_ERROR_TIMEOUT;
    }

    return ret;
}

XCamReturn
RkStreamEventPollThread::start()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = _dev->open(true);
    if (ret) {
       return ret;
    }
    _dev->subscribe_event(CIFISP_V4L2_EVENT_STREAM_START);
    _dev->subscribe_event(CIFISP_V4L2_EVENT_STREAM_STOP);
    return RkEventPollThread::start();
}

XCamReturn
RkStreamEventPollThread::stop ()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkEventPollThread::stop();
    _dev->unsubscribe_event(CIFISP_V4L2_EVENT_STREAM_START);
    _dev->unsubscribe_event(CIFISP_V4L2_EVENT_STREAM_STOP);
    ret = _dev->close();
    return ret;
}

RKStream::RKStream (SmartPtr<V4l2Device> dev, int type)
    :_dev(dev)
    ,_dev_type(type)
    ,_dev_prepared(false)
{
    _poll_thread = new RkPollThread(RKStream::poll_type_to_str[type], type, dev, this);
    XCAM_LOG_DEBUG ("RKStream constructed");
}

RKStream::RKStream (SmartPtr<V4l2SubDevice> dev, int type)
    :_subdev(dev)
    ,_dev_type(type)
    ,_dev_prepared(false)
{
    _poll_thread = new RkEventPollThread(RKStream::poll_type_to_str[type], type, dev, this);
    XCAM_LOG_DEBUG ("RKStream constructed");
}

RKStream::RKStream (const char *path, int type)
    :_dev_type(type)
    ,_dev_prepared(false)
{
    _dev = new V4l2Device(path);
    _poll_thread = new RkPollThread(RKStream::poll_type_to_str[type], type, _dev, this);
    XCAM_LOG_DEBUG ("RKStream constructed");
}

RKStream::~RKStream()
{
    XCAM_LOG_DEBUG ("~RKStream destructed");
}

void
RKStream::start()
{
    if (!_dev->is_activated())
        _dev->start(_dev_prepared);
    _poll_thread->start();
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

RKStatsStream::RKStatsStream (SmartPtr<V4l2Device> dev, int type)
    :RKStream(dev, type)
{
    XCAM_LOG_DEBUG ("RKStream constructed");
}

//RKStatsStream::RKStatsStream (const char *name, int type)
//    :RKStream(name, type)
//{
//    XCAM_LOG_DEBUG ("RKStream constructed");
//}

RKStatsStream::~RKStatsStream()
{
    XCAM_LOG_DEBUG ("~RKStream destructed");
}

bool RKStatsStream::set_event_handle_dev(SmartPtr<BaseSensorHw> &dev)
{
    _event_handle_dev = dev;
    return true;
}

bool RKStatsStream::set_iris_handle_dev(SmartPtr<LensHw> &dev)
{
    _iris_handle_dev = dev;
    return true;
}

bool RKStatsStream::set_focus_handle_dev(SmartPtr<LensHw> &dev)
{
    _focus_handle_dev = dev;
    return true;
}

bool RKStatsStream::set_rx_handle_dev(CamHwIsp20* dev)
{
    _rx_handle_dev = dev;
    return true;
}

SmartPtr<VideoBuffer>
RKStatsStream::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                       SmartPtr<V4l2Device> dev)
{
    ENTER_CAMHW_FUNCTION();
    //SmartPtr<VideoBuffer> video_buf = nullptr;
    SmartPtr<Isp20StatsBuffer> isp20stats_buf = nullptr;

    // SmartPtr<RkAiqIspParamsProxy> ispParams = nullptr;
    rkisp_effect_params_v20 ispParams = {0};
    SmartPtr<RkAiqExpParamsProxy> expParams = nullptr;
    SmartPtr<RkAiqIrisParamsProxy> irisParams = nullptr;
    SmartPtr<RkAiqAfInfoProxy> afParams = nullptr;

    if (_focus_handle_dev.ptr()) {
        _focus_handle_dev->getAfInfoParams(afParams, buf->get_buf().sequence);
        _focus_handle_dev->getIrisInfoParams(irisParams, buf->get_buf().sequence);
    }

    isp20stats_buf = new Isp20StatsBuffer(buf, dev, _event_handle_dev, _rx_handle_dev, afParams, irisParams);
    isp20stats_buf->_buf_type = _dev_type;
    isp20stats_buf->getEffectiveIspParams(buf->get_buf().sequence, ispParams);
    isp20stats_buf->getEffectiveExpParams(buf->get_buf().sequence, expParams);
    CaptureRawData::getInstance().save_metadata_and_register(buf->get_buf().sequence, ispParams, expParams, afParams, _rx_handle_dev->get_workingg_mode());

    EXIT_CAMHW_FUNCTION();

    return isp20stats_buf;
}

/*--------------------sof event stream---------------------------*/

RKSofEventStream::RKSofEventStream (SmartPtr<V4l2SubDevice> dev, int type)
    :RKStream(dev, type)
{
    XCAM_LOG_DEBUG ("RKSofEventStream constructed");
}

//RKSofEventStream::RKSofEventStream (const char *name, int type)
//    :RKStream(name, type)
//{
//    XCAM_LOG_DEBUG ("RKSofEventStream constructed");
//}

RKSofEventStream::~RKSofEventStream()
{
    XCAM_LOG_DEBUG ("~RKSofEventStream destructed");
}

void
RKSofEventStream::start()
{
    _subdev->start(_dev_prepared);
    _poll_thread->start();
    _subdev->subscribe_event(V4L2_EVENT_FRAME_SYNC);
}

void
RKSofEventStream::stop()
{
    _poll_thread->stop();
    _subdev->unsubscribe_event(V4L2_EVENT_FRAME_SYNC);
    _subdev->stop();
}

SmartPtr<VideoBuffer>
RKSofEventStream::new_video_buffer(struct v4l2_event &event,
                                       SmartPtr<V4l2Device> dev)
{
    ENTER_CAMHW_FUNCTION();
    SmartPtr<VideoBuffer> video_buf = nullptr;
    int64_t tv_sec = event.timestamp.tv_sec;
    int64_t tv_nsec = event.timestamp.tv_nsec;
    int exp_id = event.u.frame_sync.frame_sequence;
    SmartPtr<SofEventData> evtdata = new SofEventData();
    evtdata->_timestamp = tv_sec * 1000 * 1000 * 1000 + tv_nsec;
    evtdata->_frameid = exp_id;

    video_buf = new SofEventBuffer(evtdata, dev);
    video_buf->_buf_type = _dev_type;
    video_buf->set_sequence (exp_id);
    EXIT_CAMHW_FUNCTION();

    return video_buf;
}

/*--------------------Output stream---------------------------*/

RKRawStream::RKRawStream (SmartPtr<V4l2Device> dev, int index, int type)
    :RKStream(dev, type)
    ,_dev_index(index)
{
    XCAM_LOG_DEBUG ("RKRawStream constructed");
}

RKRawStream::~RKRawStream()
{
    XCAM_LOG_DEBUG ("~RKRawStream destructed");
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

RKPdafStream::RKPdafStream (SmartPtr<V4l2Device> dev, int type)
    :RKStream(dev, type)
{
    XCAM_LOG_DEBUG ("RKRawStream constructed");
}

RKPdafStream::~RKPdafStream()
{
    XCAM_LOG_DEBUG ("~RKRawStream destructed");
}

SmartPtr<V4l2BufferProxy>
RKPdafStream::new_v4l2proxy_buffer(SmartPtr<V4l2Buffer> buf,
                                       SmartPtr<V4l2Device> dev)
{
    ENTER_CAMHW_FUNCTION();
    SmartPtr<PdafBufferProxy> buf_proxy = new PdafBufferProxy(buf, dev);
    buf_proxy->_buf_type = _dev_type;
    EXIT_CAMHW_FUNCTION();

    return buf_proxy;
}

}; //namspace RkCam
