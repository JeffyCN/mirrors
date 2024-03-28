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
#ifndef _STREAM_H_
#define _STREAM_H_
#include <sys/mman.h>
#include <v4l2_device.h>
#include "poll_thread.h"
#include "xcam_mutex.h"
#include "rk_aiq_types_priv.h"

using namespace XCam;

namespace RkCam {

class RKStream;
class RkPollThread : public Thread
{
public:
    explicit RkPollThread (const char* thName, int type, SmartPtr<V4l2Device> dev, RKStream *stream);
    explicit RkPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> dev, RKStream *stream);
    virtual ~RkPollThread ();
    virtual bool setPollCallback (PollCallback *callback);
    virtual XCamReturn start();
    virtual XCamReturn stop ();
    void setCamPhyId(int phyId) {
        mCamPhyId = phyId;
    }
protected:
    XCAM_DEAD_COPY (RkPollThread);
    XCamReturn poll_buffer_loop ();
    virtual bool loop () {
        XCamReturn ret = poll_buffer_loop ();
        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT ||
            ret == XCAM_RETURN_BYPASS)
            return true;
        return false;
    }
    XCamReturn create_stop_fds ();
    void destroy_stop_fds ();
    int mCamPhyId{-1};
protected:
    static const int default_poll_timeout;
    SmartPtr<V4l2Device> _dev;
    SmartPtr<V4l2SubDevice> _subdev;
    PollCallback *_poll_callback;
    RKStream *_stream;
    //frame syncronization
    uint32_t frameid;
    int _dev_type;
    int _poll_stop_fd[2];
};

class RkEventPollThread : public RkPollThread
{
public:
    explicit RkEventPollThread (const char* thName, int type, SmartPtr<V4l2Device> dev, RKStream *stream);
    explicit RkEventPollThread (const char* thName, int type, SmartPtr<V4l2SubDevice> subdev, RKStream *stream);
    virtual ~RkEventPollThread ();
protected:
    XCAM_DEAD_COPY (RkEventPollThread);
    virtual XCamReturn poll_event_loop ();
    virtual bool loop () {
        XCamReturn ret = poll_event_loop();
        if (ret == XCAM_RETURN_NO_ERROR || ret == XCAM_RETURN_ERROR_TIMEOUT ||
            ret == XCAM_RETURN_BYPASS)
            return true;
        return false;
    }
    //SmartPtr<V4l2SubDevice> _subdev;
    struct v4l2_event _event;
};

class CamHwIsp20;
// listen mp/sp stream on/off event
class RkStreamEventPollThread : public RkEventPollThread
{
public:
    explicit RkStreamEventPollThread(const char* thName, SmartPtr<V4l2Device> dev, CamHwIsp20* isp)
        : RkEventPollThread(thName, ISP_POLL_ISPSTREAMSYNC, dev, NULL)
        , _pIsp(isp) {};
    virtual ~RkStreamEventPollThread(){};
    XCamReturn poll_event_loop ();
    virtual XCamReturn start();
    virtual XCamReturn stop ();
protected:
    XCAM_DEAD_COPY (RkStreamEventPollThread);
    CamHwIsp20* _pIsp;
};

class RKStream
{
public:
    RKStream               (SmartPtr<V4l2Device> dev, int type);
    RKStream               (SmartPtr<V4l2SubDevice> dev, int type);
    RKStream               (const char *path, int type);
    virtual ~RKStream      ();
    virtual void start                  ();
    virtual void startThreadOnly        ();
    virtual void startDeviceOnly        ();
    virtual void stop                   ();
    virtual void stopThreadOnly         ();
    virtual void stopDeviceOnly         ();
    virtual void pause                  ();
    virtual void resume                 ();
    virtual bool setPollCallback (PollCallback *callback);
    virtual SmartPtr<VideoBuffer>
    new_video_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
    virtual SmartPtr<VideoBuffer>
    new_video_buffer(struct v4l2_event &event, SmartPtr<V4l2Device> dev) { return NULL; }
    virtual SmartPtr<V4l2BufferProxy>
    new_v4l2proxy_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev) { return NULL; }
    void set_device_prepared(bool prepare);
    XCamReturn virtual getFormat(struct v4l2_format &format);
    XCamReturn virtual getFormat(struct v4l2_subdev_format &format);
    void setCamPhyId(int phyId) {
        mCamPhyId = phyId;
    }

    static const char* poll_type_to_str[ISP_POLL_POST_MAX];
protected:
    XCAM_DEAD_COPY (RKStream);
protected:
    SmartPtr<V4l2Device>  _dev;
    SmartPtr<V4l2SubDevice>  _subdev;
    int _dev_type;
    SmartPtr<RkPollThread> _poll_thread;
    bool _dev_prepared;
    int mCamPhyId{-1};
};

class BaseSensorHw;
class LensHw;
class RKStatsStream : public RKStream
{
public:
    RKStatsStream               (SmartPtr<V4l2Device> dev, int type);
//    RKStatsStream               (const char *name, int type);
    virtual ~RKStatsStream      ();
    bool set_event_handle_dev(SmartPtr<BaseSensorHw> &dev);
    bool set_iris_handle_dev(SmartPtr<LensHw> &dev);
    bool set_focus_handle_dev(SmartPtr<LensHw> &dev);
    bool set_rx_handle_dev(CamHwIsp20* dev);
    virtual SmartPtr<VideoBuffer>
    new_video_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
protected:
    XCAM_DEAD_COPY (RKStatsStream);
private:
    //alliance devices
    SmartPtr<BaseSensorHw> _event_handle_dev;
    SmartPtr<LensHw> _iris_handle_dev;
    SmartPtr<LensHw> _focus_handle_dev;
    CamHwIsp20* _rx_handle_dev{NULL};
};

class RKSofEventStream : public RKStream
{
public:
    RKSofEventStream               (SmartPtr<V4l2SubDevice> dev, int type, bool linkedToRk1608 = false);
    // RKSofEventStream               (const char *name, int type, bool linkedTo1608 = false);
    virtual ~RKSofEventStream      ();
    virtual void start             ();
    virtual void stop              ();
    virtual SmartPtr<VideoBuffer>
    new_video_buffer               (struct v4l2_event &event, SmartPtr<V4l2Device> dev);
protected:
    bool _linked_to_1608;
    /* the sensors which are connected to Rk1608 only subscribe a reset event */
    static std::atomic<bool> _is_subscribed;
    XCAM_DEAD_COPY (RKSofEventStream);
};

class RKAiispEventStream : public RKStream
{
public:
    RKAiispEventStream               (SmartPtr<V4l2SubDevice> dev, int type);
    // RKSofEventStream               (const char *name, int type, bool linkedTo1608 = false);
    virtual ~RKAiispEventStream      ();
    virtual void start             ();
    virtual void stop();
    XCamReturn set_aiisp_linecnt(rk_aiq_aiisp_cfg_t aiisp_cfg);
    XCamReturn get_aiisp_bay3dbuf();
    virtual SmartPtr<VideoBuffer>
    new_video_buffer               (struct v4l2_event &event, SmartPtr<V4l2Device> dev);
    XCamReturn call_aiisp_rd_start();
    XCamReturn close_aiisp();
protected:
    rkisp_bay3dbuf_info_t bay3dbuf;
    void* iir_address;
    void* gain_address;
    static std::atomic<bool> _is_subscribed;
    XCAM_DEAD_COPY (RKAiispEventStream);
};

class RKRawStream : public RKStream
{
public:
    RKRawStream               (SmartPtr<V4l2Device> dev, int index, int type);
    virtual ~RKRawStream      ();
    virtual SmartPtr<V4l2BufferProxy>
    new_v4l2proxy_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
    virtual SmartPtr<VideoBuffer>
    new_video_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);
    void set_reserved_data(int bpp);
public:
    int _dev_index;
    int _bpp{16};
    int _reserved[2]{0};
protected:
    XCAM_DEAD_COPY (RKRawStream);
};

class RKPdafStream : public RKStream
{
public:
    RKPdafStream               (SmartPtr<V4l2Device> dev, int type);
    virtual ~RKPdafStream      ();
    virtual SmartPtr<V4l2BufferProxy>
    new_v4l2proxy_buffer(SmartPtr<V4l2Buffer> buf, SmartPtr<V4l2Device> dev);

protected:
    XCAM_DEAD_COPY (RKPdafStream);
};

class SubVideoBuffer : public VideoBuffer
{
public:
   explicit SubVideoBuffer(int num, int index, int fd, const VideoBufferInfo& info)
       : VideoBuffer(info) {
       _buff_num      = num;
       _buff_idx      = index;
       _buff_fd       = fd;
       _buff_size     = 0;
       _buff_ptr      = MAP_FAILED;
       _v4l2buf_proxy = nullptr;
   }
    explicit SubVideoBuffer(int fd)
             :VideoBuffer()
    {
        _buff_num = 0;
        _buff_idx = -1;
        _buff_fd = fd;
        _buff_size = 0;
        _buff_ptr = MAP_FAILED;
        _v4l2buf_proxy = nullptr;
    }
    explicit SubVideoBuffer(SmartPtr<V4l2BufferProxy> &buf)
            :VideoBuffer()
    {
        _buff_num = 0;
        _buff_idx = -1;
        _buff_fd = -1;
        _buff_size = 0;
        _buff_ptr = MAP_FAILED;
        _v4l2buf_proxy = buf;
    }
    explicit SubVideoBuffer()
             :VideoBuffer()
    {
        _buff_num = 0;
        _buff_idx = -1;
        _buff_fd = -1;
        _buff_size = 0;
        _buff_ptr = MAP_FAILED;
        _v4l2buf_proxy = nullptr;
    }
    virtual ~SubVideoBuffer() {}
    virtual uint8_t *map ()
    {
        if (_v4l2buf_proxy.ptr()) {
            return (uint8_t *)_v4l2buf_proxy->get_v4l2_userptr();
        }

        if (_buff_ptr == MAP_FAILED) {
            _buff_ptr = mmap (NULL, _buff_size ? _buff_size: get_size (), PROT_READ | PROT_WRITE, MAP_SHARED, _buff_fd, 0);
            if (_buff_ptr == MAP_FAILED) {
                LOGE("mmap failed, size=%d,fd=%d",_buff_size,_buff_fd);
                return nullptr;
            }
        }
        return (uint8_t *)_buff_ptr;
    }
    virtual bool unmap ()
    {
        if (_v4l2buf_proxy.ptr()) {
            return true;
        }

        if (_buff_ptr != MAP_FAILED) {
            munmap(_buff_ptr, _buff_size ? _buff_size: get_size ());
            _buff_ptr = MAP_FAILED;
        }
        return true;
    }
    virtual int get_fd ()
    {
        if (_v4l2buf_proxy.ptr()) {
            return _v4l2buf_proxy->get_expbuf_fd();
        }
        return _buff_fd;
    }
    int set_buff_info(int fd, int size)
    {
        _buff_fd = fd;
        _buff_size = size;
        return 0;
    }

    int get_buf_num() { return _buff_num; }

    int get_index() { return _buff_idx; }

    int get_size() { return _buff_size; }

 protected:
    XCAM_DEAD_COPY (SubVideoBuffer);
private:
    int _buff_fd;
    int _buff_size;
    int _buff_idx;
    int _buff_num;
    void *_buff_ptr;
    SmartPtr<V4l2BufferProxy> _v4l2buf_proxy;
};

class SubV4l2BufferProxy : public V4l2BufferProxy
{
public:
    explicit SubV4l2BufferProxy(SmartPtr<V4l2Buffer> &buf, SmartPtr<V4l2Device> &device)
             :V4l2BufferProxy(buf, device)
    {
        _buff_ptr = MAP_FAILED;
        _buff_fd = -1;
        _buff_size = 0;
    }
    virtual ~SubV4l2BufferProxy() {}
    virtual uint8_t *map ()
    {
        if (_buff_ptr == MAP_FAILED) {
            _buff_ptr = mmap (NULL, _buff_size ? _buff_size: get_size (), PROT_READ | PROT_WRITE, MAP_SHARED, _buff_fd, 0);
            if (_buff_ptr == MAP_FAILED) {
                LOGE("mmap failed, size=%d,fd=%d",_buff_size,_buff_fd);
                return nullptr;
            }
        }
        return (uint8_t *)_buff_ptr;
    }
    virtual bool unmap ()
    {
        if (_buff_ptr != MAP_FAILED) {
            munmap(_buff_ptr, _buff_size ? _buff_size: get_size ());
            _buff_ptr = MAP_FAILED;
        }
        return true;
    }
    virtual int get_fd ()
    {
        return _buff_fd;
    }
    int set_buff_info(int fd, int size)
    {
        _buff_fd = fd;
        _buff_size = size;
        return 0;
    }
protected:
    XCAM_DEAD_COPY (SubV4l2BufferProxy);
    int _buff_fd;
    int _buff_size;
    void *_buff_ptr;
};
}
#endif
