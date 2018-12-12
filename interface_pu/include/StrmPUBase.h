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

#ifndef __STREAM_PROCESS_UNIT_BASE_H__
#define __STREAM_PROCESS_UNIT_BASE_H__

//#include "CamHwItf.h"
#include "cam_thread.h"
#include "cam_types.h"
#include "CameraBuffer.h"
#include <memory>
#include <vector>
#include <list>

using namespace std;

class StreamPUBase
  : public enable_shared_from_this<StreamPUBase>,
    public NewCameraBufferReadyNotifier,
    public ICameraBufferOwener {
  friend class streamThread;
 public:
  enum State {
    UNINITIALIZED,
    PREPARED,
    STREAMING
  };
  StreamPUBase(const char* name = "defaultPU", bool needStrmTh = true, bool shareInbuffer = false);
  virtual ~StreamPUBase();
  virtual void addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier);
  virtual bool removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier);

  //from ICameraBufferOwener
  virtual bool releaseBufToOwener(weak_ptr<BufferBase> camBuf);
  //from NewCameraBufferReadyNotifier
  virtual bool bufferReady(weak_ptr<BufferBase> buffer, int status);
  //need process result frame buffer
  virtual bool prepare(
      const frm_info_t& frmFmt,
      unsigned int numBuffers,
      shared_ptr<CameraBufferAllocator> allocator);
  virtual bool start();
  virtual void stop();
  virtual void releaseBuffers(void);
  virtual void invokeFakeNotify(void);

 protected:

  class streamThread : public CamThread {
   public:
    streamThread(StreamPUBase* streamPU): mStreamPU(streamPU) {};
    virtual bool threadLoop(void);
   private:
    StreamPUBase* mStreamPU;
  };
  virtual bool processFrame(shared_ptr<BufferBase> inBuf, shared_ptr<BufferBase> outBuf);
  shared_ptr<CamThread> mStrmPUThread;
  osMutex mNotifierLock;
  list<NewCameraBufferReadyNotifier*> mBufferReadyNotifierList;

  shared_ptr<CameraBufferAllocator> mBufferAllocator;
  list<shared_ptr<BufferBase> > mBufferPool;
  list<shared_ptr<BufferBase> > mInBuffers;
  list<shared_ptr<BufferBase> > mOutBuffers;
  unsigned int mAllocNumBuffers;
  osMutex mBuffersLock;
  osEvent mBuffersCond;

  State mState;
  osMutex mStateLock;
  frm_info_t mStrmFrmInfo;
  bool mNeedStrmTh;
  bool mShareInBuf;
  const char* mName;
};
#endif
