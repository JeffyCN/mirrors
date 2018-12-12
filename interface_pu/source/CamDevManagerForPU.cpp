/*
 * Copyright (c) 2018, Fuzhou Rockchip Electronics Co., Ltd
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

#include "CamDevManagerForPU.h"
#include "ProxyCameraBuffer.h"
#include "cam_thread.h"
#include "common/return_codes.h"
#include <map>
#include "rkisp_device.h"

#define DEFAULT_PROP_FIELD              V4L2_FIELD_NONE // 0
#define DEFAULT_PROP_MEM_MODE           V4L2_MEMORY_MMAP
#define V4L2_CAPTURE_MODE_VIDEO         0x4000
#define DEFAULT_DEQUE_TIMEOUT_MS        100

using namespace XCam;

class CamDevManagerForPU;
struct camera_metadata;
typedef struct camera_metadata camera_metadata_t;

class CamDevManagerForPUImp
        : public enable_shared_from_this<CamDevManagerForPUImp>,
          public ICameraBufferOwener
{
public:
    friend class streamThread;

    enum State {
        CAMDEV_STATE_UNINIT,
        CAMDEV_STATE_INIT,
        CAMDEV_STATE_PREPARED,
        CAMDEV_STATE_STARTED,
    };
    CamDevManagerForPUImp();
    ~CamDevManagerForPUImp();

    /* add buffer consumer */
    void addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier);
    /* remove buffer consumer */
    bool removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier);
    /* from derived class ICameraBufferOwener */
    bool releaseBufToOwener(weak_ptr<BufferBase> camBuf);
    /* set ISP controls for each frame */
    int setFrameParams(const int request_frame_id,
                        const camera_metadata_t *metas);
    /* get ISP controls for each frame */
    int getFrameParams(const int request_frame_id,
                       const camera_metadata_t *metas);
    /* init capture device */
    bool init(const char* dev);
    /* call prepare before start, it will configure the proper pipeline */
    bool prepare(frm_info_t& frmFmt, int num);
    /* after this call, the device manager will output frames to notifiers */
    bool start(void);
    /* after this call, the device manager will stop outputting frames to notifiers */
    void stop(void);
    void deinit(void);
private:
    //XCAM_DEAD_COPY (CamDevManagerForPU);
    shared_ptr<CameraBufferAllocator> _buf_alloc;
    SmartPtr<V4l2Device> _capture_device;
    frm_info_t _cur_fmt;

    class streamThread : public CamThread {
        public:
        streamThread(weak_ptr<CamDevManagerForPUImp> camDev): _cam_dev(camDev) {};
        virtual bool threadLoop(void);
        private:
            weak_ptr<CamDevManagerForPUImp> _cam_dev;
    };
    int dequeue_buffer (SmartPtr<VideoBuffer> &video_buf);
    osMutex _state_lock;
    list<NewCameraBufferReadyNotifier*> _notifier_list;
    shared_ptr<CamThread> _buf_generator_thread;
    enum State _cam_dev_state;
    map<shared_ptr<BufferBase>, SmartPtr<VideoBuffer>> _hb_vb_map;
};

CamDevManagerForPUImp::CamDevManagerForPUImp()
    : _buf_alloc(NULL),
      _capture_device(NULL),
      _buf_generator_thread(NULL),
      _cam_dev_state(CAMDEV_STATE_UNINIT)
{
    osMutexInit(&_state_lock);
    _buf_alloc = shared_ptr<ProxyCameraBufferAllocator>(new ProxyCameraBufferAllocator());
}

CamDevManagerForPUImp::~CamDevManagerForPUImp()
{}

void
CamDevManagerForPUImp::addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier)
{
  osMutexLock(&_state_lock);
  if (bufferReadyNotifier)
    _notifier_list.push_back(bufferReadyNotifier);
  osMutexUnlock(&_state_lock);
}

bool
CamDevManagerForPUImp::removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier)
{
  bool ret = false;
  //search this notifier
  osMutexLock(&_state_lock);

  for (list<NewCameraBufferReadyNotifier*>::iterator i = _notifier_list.begin();
       i != _notifier_list.end(); i++) {
    if (*i == bufferReadyNotifier) {
      _notifier_list.erase(i);
      ret = true;
      break;
    }
  }
  osMutexUnlock(&_state_lock);

  return ret;
}

bool
CamDevManagerForPUImp::releaseBufToOwener(weak_ptr<BufferBase> camBuf)
{
  bool ret = false;

  osMutexLock(&_state_lock);
  shared_ptr<BufferBase> spBuf = camBuf.lock();
  if (spBuf.get() != NULL) {
    std::map<shared_ptr<BufferBase>, SmartPtr<VideoBuffer> >::iterator it;
    for (it = _hb_vb_map.begin(); it != _hb_vb_map.end(); it++) {
      if (it->first.get() == spBuf.get()) {
        ret = true;
        break;
      }
    }
    if (it != _hb_vb_map.end()) {
       _hb_vb_map.erase(it);
    }
  }
  osMutexUnlock(&_state_lock);
  return ret;
}

int
CamDevManagerForPUImp::setFrameParams(const int request_frame_id,
                                      const camera_metadata_t *metas)
{
    // TODO
    return -1;
}

int
CamDevManagerForPUImp::getFrameParams(const int request_frame_id,
                                      const camera_metadata_t *metas)
{
    // TODO
    return -1;
}

bool
CamDevManagerForPUImp::init(const char* dev)
{
    bool ret = false;

    osMutexLock(&_state_lock);
    if (_cam_dev_state == CAMDEV_STATE_INIT)
        goto unlock;
    if (_cam_dev_state != CAMDEV_STATE_UNINIT)
        goto unlock;
    osMutexUnlock(&_state_lock);

    if (!dev) {
        XCAM_LOG_ERROR("NULL device");
        goto done;
    }
    _capture_device = new RKispDevice(dev);
    if (!_capture_device.ptr()) {
        XCAM_LOG_ERROR("create dev:%s fail\n", dev);
        goto done;
    }
    if (_capture_device->open() != 0) {
        XCAM_LOG_ERROR("create dev:%s fail\n", dev);
        goto done;
    }
    ret = true;
    osMutexLock(&_state_lock);
    _cam_dev_state = CAMDEV_STATE_INIT;
unlock:
    osMutexUnlock(&_state_lock);
done:
    return ret;
}

void
CamDevManagerForPUImp::deinit(void)
{
    osMutexLock(&_state_lock);
    if (_cam_dev_state != CAMDEV_STATE_INIT)
        goto unlock;
    osMutexUnlock(&_state_lock);

    _capture_device->close();

    osMutexLock(&_state_lock);
    _cam_dev_state = CAMDEV_STATE_UNINIT;
unlock:
    osMutexUnlock(&_state_lock);
}

bool
CamDevManagerForPUImp::prepare(frm_info_t& frmFmt, int num)
{
    bool ret = false;
    bool needprepare = false;

    osMutexLock(&_state_lock);
    if (_cam_dev_state == CAMDEV_STATE_UNINIT ||
        _cam_dev_state == CAMDEV_STATE_STARTED) {
        XCAM_LOG_ERROR ("failed,in wrong state %d", _cam_dev_state);
        goto unlock;
    } else if (_cam_dev_state == CAMDEV_STATE_PREPARED) {
        // TODO: check if re-prepare is needed
        needprepare = false;
    } else {
        needprepare = true;
    }
    osMutexUnlock(&_state_lock);

    if (!needprepare)
        goto done;

    _capture_device->set_sensor_id(0);
    _capture_device->set_capture_mode(V4L2_CAPTURE_MODE_VIDEO);
    _capture_device->set_mem_type(DEFAULT_PROP_MEM_MODE);
    _capture_device->set_buffer_count(num);
    _capture_device->set_framerate(1, frmFmt.fps);
    _capture_device->set_format(
        frmFmt.frmSize.width,
        frmFmt.frmSize.height,
        halFmtToV4l2Fmt(frmFmt.frmFmt),
        DEFAULT_PROP_FIELD,
        frmFmt.frmSize.width);
    memcpy(&_cur_fmt, &frmFmt, sizeof(frm_info_t));
    ret = true;
    osMutexLock(&_state_lock);
    _cam_dev_state = CAMDEV_STATE_PREPARED;
unlock:
    osMutexUnlock(&_state_lock);
done:
    return ret;
}

bool
CamDevManagerForPUImp::start(void)
{
    bool ret = false;

    osMutexLock(&_state_lock);
    if (_cam_dev_state != CAMDEV_STATE_PREPARED) {
        XCAM_LOG_ERROR ("failed,in wrong state %d", _cam_dev_state);
        goto unlock;
    }
    osMutexUnlock(&_state_lock);

    if (_capture_device->start() != 0) {
      XCAM_LOG_ERROR ("capture device start fail");
      goto done;
    }

    osMutexLock(&_state_lock);
    _buf_generator_thread = shared_ptr<CamThread>(new streamThread(shared_from_this()));
    if (_buf_generator_thread->run("CamDevForPU") != RET_SUCCESS) {
        XCAM_LOG_ERROR ("buf generator thread run fail");
        _capture_device->stop();
        goto done;
    }
    ret = true;
    _cam_dev_state = CAMDEV_STATE_STARTED;
unlock:
    osMutexUnlock(&_state_lock);
done:
    return ret;
}

void
CamDevManagerForPUImp::stop(void)
{
    osMutexLock(&_state_lock);
    if (_cam_dev_state != CAMDEV_STATE_STARTED) {
        XCAM_LOG_ERROR ("failed,in wrong state %d", _cam_dev_state);
        goto unlock;
    }
    osMutexUnlock(&_state_lock);
    _buf_generator_thread->requestExitAndWait();
    _buf_generator_thread.reset();
    // TODO: wait all buffers are returned ?
    _hb_vb_map.clear();
    _capture_device->stop();

    osMutexLock(&_state_lock);
    _cam_dev_state = CAMDEV_STATE_PREPARED;
unlock:
    osMutexUnlock(&_state_lock);
}

int
CamDevManagerForPUImp::dequeue_buffer(SmartPtr<VideoBuffer> &video_buf)
{
    int ret = 0;
    int poll_ret = 0;
    SmartPtr<V4l2Buffer> buf;

    poll_ret = _capture_device->poll_event (DEFAULT_DEQUE_TIMEOUT_MS * 1000);
    if (poll_ret < 0) {
        XCAM_LOG_DEBUG ("poll buffer event got error but continue");
        ::usleep (100000);
        return -20;
    }

    /* timeout */
    if (poll_ret == 0) {
        XCAM_LOG_DEBUG ("poll buffer timeout and continue");
        XCAM_LOG_DEBUG ("don't return;");
    }

    ret = _capture_device->dequeue_buffer (buf);
    if (ret != 0) {
        XCAM_LOG_WARNING ("capture buffer failed");
        return ret;
    }
    XCAM_ASSERT (buf.ptr());
    video_buf = new V4l2BufferProxy (buf, _capture_device);
    return ret;
}

bool
CamDevManagerForPUImp::streamThread::threadLoop()
{
    VideoBufferInfo video_info;
    SmartPtr<VideoBuffer> video_buf;
    SmartPtr<V4l2BufferProxy> video_buf_proxy;
    shared_ptr<CamDevManagerForPUImp> camDev;

    if (!_cam_dev.expired())
      camDev = _cam_dev.lock();

    if (!camDev.get())
        return false;

    camDev->dequeue_buffer(video_buf);
    if (!video_buf.ptr())
        return true;

    video_buf_proxy = video_buf.dynamic_cast_ptr<V4l2BufferProxy> ();
    video_info = video_buf->get_video_info ();

    // TODO: only support mmap buffer now
    //increase reference before notify
    const char* v4lfmt =
        RK_HAL_FMT_STRING::hal_fmt_map_to_str(camDev->_cur_fmt.frmFmt);
    shared_ptr<CameraBuffer> halBuf =
        camDev->_buf_alloc->alloc(v4lfmt ,
                                    camDev->_cur_fmt.frmSize.width,
                                    camDev->_cur_fmt.frmSize.height,
                                    0,
                                    camDev);
    halBuf->setIndex(video_buf_proxy->get_v4l2_buf_index());
    halBuf->setVirtAddr(video_buf->map());
    halBuf->setDataSize(video_buf->get_size());
    halBuf->setTimestamp(video_buf->get_timestamp());
    halBuf->setSequence(video_buf->get_sequence());
    halBuf->incUsedCnt();
    osMutexLock(&camDev->_state_lock);
    camDev->_hb_vb_map[halBuf] = video_buf;
    for (list<NewCameraBufferReadyNotifier* >::iterator i = camDev->_notifier_list.begin();
         i != camDev->_notifier_list.end(); i++) {
        (*i)->bufferReady(halBuf, 0);
    }
    osMutexUnlock(&camDev->_state_lock);
    halBuf->decUsedCnt();

    return true;
}

CamDevManagerForPU::CamDevManagerForPU()
{
    _instance = shared_ptr<CamDevManagerForPUImp>(new CamDevManagerForPUImp());
}

CamDevManagerForPU::~CamDevManagerForPU()
{
   _instance = NULL;
}

void
CamDevManagerForPU::addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier)
{
    _instance->addBufferNotifier(bufferReadyNotifier);
}

bool
CamDevManagerForPU::removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier)
{
    return _instance->removeBufferNotifer(bufferReadyNotifier);
}

bool
CamDevManagerForPU::releaseBufToOwener(weak_ptr<BufferBase> camBuf)
{
     return _instance->releaseBufToOwener(camBuf);
}

bool
CamDevManagerForPU::init(const char* iq_file)
{
    return _instance->init(iq_file);
}

bool
CamDevManagerForPU::prepare(frm_info_t& frmFmt, int num)
{
    return _instance->prepare(frmFmt, num);
}

bool
CamDevManagerForPU::start(void)
{
    return _instance->start();
}

void
CamDevManagerForPU::stop(void)
{
    _instance->stop();
}

void
CamDevManagerForPU::deinit(void)
{
    _instance->deinit();
}
