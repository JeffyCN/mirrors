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
#include "rkcif-config.h"

namespace RkCam {

const int RkPollThread::default_poll_timeout = -1; // ms

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
    "isp_gain",
    "isp_nr_img",
    "ispp_gain_kg",
    "ispp_gain_wr",
    "isp_stream_sync_poll",
    "vicap_stream_on_evt",
    "vicap_reset_evt",
    "vicap_with_rk1608_reset_evt",
    "vicap_scale_poll",
};

RkPollThread::RkPollThread (const char* thName, int type, SmartPtr<V4l2Device> dev, RKStream *stream)
    :Thread(thName)
    ,_dev(dev)
    ,_subdev(NULL)
    ,_poll_callback (NULL)
    ,_stream(stream)
    ,frameid (0)
    ,_dev_type(type)
{
    _poll_stop_fd[0] =  -1;
    _poll_stop_fd[1] =  -1;

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RkPollThread constructed");
}

RkPollThread::RkPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> dev, RKStream *stream)
    :Thread(thName)
    ,_dev(dev)
    ,_subdev(dev)
    ,_poll_callback (NULL)
    ,_stream(stream)
    ,frameid (0)
    ,_dev_type(type)
{
    _poll_stop_fd[0] =  -1;
    _poll_stop_fd[1] =  -1;

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RkPollThread constructed");
}

RkPollThread::~RkPollThread ()
{
    stop();
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "~RkPollThread destructed");
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
    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "RkPollThread %s:%s stop", get_name(),
                   _dev.ptr() ? _dev->get_device_name() :
                   _subdev.ptr() ? _subdev->get_device_name() : "null");
    if (_poll_stop_fd[1] != -1) {
        char buf = 0xf;  // random value to write to flush fd.
        unsigned int size = write(_poll_stop_fd[1], &buf, sizeof(char));
        if (size != sizeof(char)) {
            XCAM_LOG_WARNING("Flush write not completed");
        }
    }
    Thread::stop();
    destroy_stop_fds ();
    LOGI_CAMHW_SUBM(ISP20HW_SUBM, "stop done");
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
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "poll buffer stop success !");
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

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "camId: %d, frameId: %d: dequeue buffer on %s\n",
                    mCamPhyId, buf->get_buf().sequence, RKStream::poll_type_to_str[_dev_type]);

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
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RkEventPollThread constructed");
}

RkEventPollThread::RkEventPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> subdev, RKStream *stream)
    :RkPollThread(thName, type, subdev, stream)
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RkEventPollThread constructed");
}

RkEventPollThread::~RkEventPollThread ()
{
    stop();
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "~RkEventPollThread destructed");
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
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "%s: poll event stop success !", get_name());
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

    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "camId: %d, frameId: %d: dequeue the type(%d) of event on dev: %s",
                    mCamPhyId, _event.u.frame_sync.frame_sequence,
                    _event.type, XCAM_STR(_dev->get_device_name()));

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
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "%s: poll stream on evt success", _dev->get_device_name());
            _pIsp->notify_isp_stream_status(true);
        } else if (_event.type == CIFISP_V4L2_EVENT_STREAM_STOP) {
            LOGI_CAMHW_SUBM(ISP20HW_SUBM, "%s: poll stream off evt success", _dev->get_device_name());
            _pIsp->notify_isp_stream_status(false);
            // quit loop
            emit_stop ();
            return XCAM_RETURN_ERROR_UNKNOWN;
        }
    }

    return ret;
}

XCamReturn
RkStreamEventPollThread::start()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = _dev->open(false);
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
    if (_dev->is_opened()) {
        _dev->unsubscribe_event(CIFISP_V4L2_EVENT_STREAM_START);
        _dev->unsubscribe_event(CIFISP_V4L2_EVENT_STREAM_STOP);
        ret = _dev->close();
    }
    return ret;
}

RKStream::RKStream (SmartPtr<V4l2Device> dev, int type)
    :_dev(dev)
    ,_dev_type(type)
    ,_dev_prepared(false)
{
    _poll_thread = new RkPollThread(RKStream::poll_type_to_str[type], type, dev, this);
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKStream constructed");
}

RKStream::RKStream (SmartPtr<V4l2SubDevice> dev, int type)
    :_subdev(dev)
    ,_dev_type(type)
    ,_dev_prepared(false)
{
    _poll_thread = new RkEventPollThread(RKStream::poll_type_to_str[type], type, dev, this);
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKStream constructed");
}

RKStream::RKStream (const char *path, int type)
    :_dev_type(type)
    ,_dev_prepared(false)
{
    _dev = new V4l2Device(path);
    _poll_thread = new RkPollThread(RKStream::poll_type_to_str[type], type, _dev, this);
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKStream constructed");
}

RKStream::~RKStream()
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "~RKStream destructed");
}

void
RKStream::start()
{
    if (!_dev->is_activated())
        _dev->start(_dev_prepared);

    _poll_thread->setCamPhyId(mCamPhyId);
    if (_dev_type == ISP_POLL_3A_STATS || \
        _dev_type == ISP_POLL_TX || \
        _dev_type == ISP_POLL_RX) {
        _poll_thread->set_policy(SCHED_RR);
        _poll_thread->set_priority(20);
    }
    _poll_thread->start();
}

void
RKStream::startThreadOnly()
{
    _poll_thread->setCamPhyId(mCamPhyId);
    if (_dev_type == ISP_POLL_3A_STATS || \
        _dev_type == ISP_POLL_TX || \
        _dev_type == ISP_POLL_RX) {
        _poll_thread->set_policy(SCHED_RR);
        _poll_thread->set_priority(20);
    }
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
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKStream constructed");
}

//RKStatsStream::RKStatsStream (const char *name, int type)
//    :RKStream(name, type)
//{
//    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKStream constructed");
//}

RKStatsStream::~RKStatsStream()
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "~RKStream destructed");
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

    SmartPtr<RkAiqIrisParamsProxy> irisParams = nullptr;
    SmartPtr<RkAiqAfInfoProxy> afParams = nullptr;

    if (_focus_handle_dev.ptr()) {
        _focus_handle_dev->getAfInfoParams(afParams, buf->get_buf().sequence);
        _focus_handle_dev->getIrisInfoParams(irisParams, buf->get_buf().sequence);
    }

    isp20stats_buf = new Isp20StatsBuffer(buf, dev, _event_handle_dev, _rx_handle_dev, afParams, irisParams);
    isp20stats_buf->_buf_type = _dev_type;

    EXIT_CAMHW_FUNCTION();

    return isp20stats_buf;
}

/*--------------------sof event stream---------------------------*/
std::atomic<bool> RKSofEventStream::_is_subscribed{false};

RKSofEventStream::RKSofEventStream (SmartPtr<V4l2SubDevice> dev, int type, bool linkedToRk1608)
    :RKStream(dev, type)
{
    _linked_to_1608 = linkedToRk1608;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKSofEventStream constructed");
}

//RKSofEventStream::RKSofEventStream (const char *name, int type)
//    :RKStream(name, type)
//{
//    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKSofEventStream constructed");
//}

RKSofEventStream::~RKSofEventStream()
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "~RKSofEventStream destructed");
}

void
RKSofEventStream::start()
{
    _subdev->start(_dev_prepared);
    _poll_thread->setCamPhyId(mCamPhyId);
    _poll_thread->start();
    _subdev->subscribe_event(V4L2_EVENT_FRAME_SYNC);
    if (_linked_to_1608) {
        if (!_is_subscribed.load()) {
            _subdev->subscribe_event(V4L2_EVENT_RESET_DEV);
            _is_subscribed.store(true);
        }
    } else {
        _subdev->subscribe_event(V4L2_EVENT_RESET_DEV);
    }
}

void
RKSofEventStream::stop()
{
    _poll_thread->stop();
    _subdev->unsubscribe_event(V4L2_EVENT_FRAME_SYNC);
    if (_linked_to_1608) {
        if (_is_subscribed.load()) {
            _subdev->unsubscribe_event(V4L2_EVENT_RESET_DEV);
            _is_subscribed.store(false);
        }
    } else {
        _subdev->unsubscribe_event(V4L2_EVENT_RESET_DEV);
    }
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
    if (event.type == V4L2_EVENT_RESET_DEV) {
        if (_linked_to_1608) {
            video_buf->_buf_type = VICAP_WITH_RK1608_RESET_EVT;
        } else
            video_buf->_buf_type = VICAP_RESET_EVT;
    } else {
        video_buf->_buf_type = _dev_type;
    }
    video_buf->set_sequence (exp_id);
    EXIT_CAMHW_FUNCTION();

    return video_buf;
}

/*--------------------aiisp event stream---------------------------*/
std::atomic<bool> RKAiispEventStream::_is_subscribed{false};

RKAiispEventStream::RKAiispEventStream (SmartPtr<V4l2SubDevice> dev, int type)
    :RKStream(dev, type)
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKAiispEventStream constructed");
}

//RKAiispEventStream::RKAiispEventStream (const char *name, int type)
//    :RKStream(name, type)
//{
//    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKAiispEventStream constructed");
//}

RKAiispEventStream::~RKAiispEventStream()
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "~RKAiispEventStream destructed");
}

void
RKAiispEventStream::start()
{
    _subdev->start(_dev_prepared);
    _poll_thread->setCamPhyId(mCamPhyId);
    _poll_thread->start();
    _subdev->subscribe_event(RKISP_V4L2_EVENT_AIISP_LINECNT);
}

void
RKAiispEventStream::stop()
{
    close_aiisp();
    _poll_thread->stop();
    _subdev->unsubscribe_event(RKISP_V4L2_EVENT_AIISP_LINECNT);
    _subdev->stop();
}

XCamReturn
RKAiispEventStream::set_aiisp_linecnt(rk_aiq_aiisp_cfg_t aiisp_cfg)
{
    int res = -1;
    rkisp_aiisp_cfg aiisp_cfg_io;
    aiisp_cfg_io.wr_mode = aiisp_cfg.wr_mode;
    aiisp_cfg_io.rd_mode = aiisp_cfg.rd_mode;
    aiisp_cfg_io.wr_linecnt = aiisp_cfg.wr_linecnt;
    aiisp_cfg_io.rd_linecnt = aiisp_cfg.rd_linecnt;
    res = _subdev->io_control(RKISP_CMD_SET_AIISP_LINECNT, &aiisp_cfg_io);
    if (res) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,"set aiisp linecnt failed! %d", res);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "aiisp wr_linecnt is %d rd_linecnt is %d", aiisp_cfg.wr_linecnt, aiisp_cfg.rd_linecnt);
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RKAiispEventStream::close_aiisp()
{
    int res = -1;
    rk_aiq_aiisp_cfg_t aiisp_cfg;
    aiisp_cfg.wr_linecnt = 0;
    res = _subdev->io_control(RKISP_CMD_SET_AIISP_LINECNT, &aiisp_cfg);
    if (res) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,"close aiisp failed! %d", res);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    int mun_ret = munmap(iir_address, bay3dbuf.iir_size);
    mun_ret = munmap(gain_address, bay3dbuf.u.v39.gain_size);
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "close aiisp success");
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
RKAiispEventStream::get_aiisp_bay3dbuf()
{
    int res = -1;
    memset(&bay3dbuf, 0, sizeof(bay3dbuf));
    res = _subdev->io_control(RKISP_CMD_GET_BAY3D_BUFFD, &bay3dbuf);
    if (res) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "get aiisp bay3dbuf failed! %d", res);
        return XCAM_RETURN_ERROR_IOCTL;
    }
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "get aiisp bay3dbuf: iir_fd is %d iir_size is %d", bay3dbuf.iir_fd, bay3dbuf.iir_size);
    int iir_fd = bay3dbuf.iir_fd;
    int iir_size = bay3dbuf.iir_size;
    iir_address = (char*)mmap(NULL, iir_size, PROT_READ | PROT_WRITE, MAP_SHARED, iir_fd, 0);
    if (MAP_FAILED == iir_address) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "iir_fd mmap failed");
        return XCAM_RETURN_ERROR_FAILED;
    }
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "iir_fd %d, iir_size %d", iir_fd, iir_size);
    int gain_fd = bay3dbuf.u.v39.gain_fd;
    int gain_size = bay3dbuf.u.v39.gain_size;
    gain_address = (char*)mmap(NULL, gain_size, PROT_READ | PROT_WRITE, MAP_SHARED, gain_fd, 0);
    if (MAP_FAILED == gain_address) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "gain_fd mmap failed");
        return XCAM_RETURN_ERROR_FAILED;
    }
    LOGK_CAMHW_SUBM(ISP20HW_SUBM, "gain_fd %d, gain_size %d", gain_fd, gain_size);
    return XCAM_RETURN_NO_ERROR;
}

SmartPtr<VideoBuffer>
RKAiispEventStream::new_video_buffer(struct v4l2_event &event,
                                       SmartPtr<V4l2Device> dev)
{
    ENTER_CAMHW_FUNCTION();
    SmartPtr<VideoBuffer> video_buf = nullptr;
    rkisp_aiisp_ev_info* aiisp_ev_info = (rkisp_aiisp_ev_info*)event.u.data;
    SmartPtr<AiispEventData> evtdata = new AiispEventData();
    evtdata->_height = aiisp_ev_info->height;
    evtdata->_frameid = aiisp_ev_info->sequence;
    evtdata->bay3dbuf = bay3dbuf;
    evtdata->iir_address = iir_address;
    evtdata->gain_address = gain_address;

    video_buf = new AiispEventBuffer(evtdata, dev);
    if (event.type != RKISP_V4L2_EVENT_AIISP_LINECNT) {
        video_buf->_buf_type = _dev_type;
        LOGE_CAMHW_SUBM(ISP20HW_SUBM,"The type of event is not RKISP_V4L2_EVENT_AIISP_LINECNT!");
    }
    else {
        video_buf->_buf_type = _dev_type;
    }
    video_buf->set_sequence (evtdata->_frameid);
    EXIT_CAMHW_FUNCTION();

    return video_buf;
}

XCamReturn RKAiispEventStream::call_aiisp_rd_start()
{
    int res = -1;
    res = _subdev->io_control(RKISP_CMD_AIISP_RD_START, NULL);
    if (res)
        return XCAM_RETURN_ERROR_IOCTL;
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "call aiisp rd start success");
    return XCAM_RETURN_NO_ERROR;
}

/*--------------------Output stream---------------------------*/

RKRawStream::RKRawStream (SmartPtr<V4l2Device> dev, int index, int type)
    :RKStream(dev, type)
    ,_dev_index(index)
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKRawStream constructed");
}

RKRawStream::~RKRawStream()
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "~RKRawStream destructed");
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

SmartPtr<VideoBuffer>
RKRawStream::new_video_buffer(SmartPtr<V4l2Buffer> buf,
                                       SmartPtr<V4l2Device> dev)
{
    ENTER_CAMHW_FUNCTION();
    buf->set_reserved((uintptr_t)_reserved);
    SmartPtr<VideoBuffer> video_buf = new V4l2BufferProxy(buf, dev);
    video_buf->_buf_type = _dev_type;
    EXIT_CAMHW_FUNCTION();

    return video_buf;
}

void
RKRawStream::set_reserved_data(int bpp)
{
    _bpp = bpp;
    _reserved[0] = _dev_index;
    _reserved[1] = bpp;
}

RKPdafStream::RKPdafStream (SmartPtr<V4l2Device> dev, int type)
    :RKStream(dev, type)
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "RKRawStream constructed");
}

RKPdafStream::~RKPdafStream()
{
    LOGD_CAMHW_SUBM(ISP20HW_SUBM, "~RKRawStream destructed");
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

}  //namspace RkCam
