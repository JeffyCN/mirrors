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

#include "StrmPUBase.h"
#include "common/return_codes.h"
#include <base/log.h>

using namespace std;

StreamPUBase::StreamPUBase(const char* name, bool needStrmTh, bool shareInbuffer)
  : mState(UNINITIALIZED),
    mNeedStrmTh(needStrmTh),
    mShareInBuf(shareInbuffer),
    mName(name) {
  osMutexInit(&mStateLock);
  osMutexInit(&mNotifierLock);
  osMutexInit(&mBuffersLock);
  osEventInit(&mBuffersCond, true, 0);
}
StreamPUBase::~StreamPUBase() {
  osMutexDestroy(&mStateLock);
  osMutexDestroy(&mNotifierLock);
  osMutexDestroy(&mBuffersLock);
  osEventDestroy(&mBuffersCond);
}

void StreamPUBase::addBufferNotifier(NewCameraBufferReadyNotifier* bufferReadyNotifier) {
  ALOGD("%s:%d ", __func__, __LINE__);
  osMutexLock(&mNotifierLock);
  if (bufferReadyNotifier)
    mBufferReadyNotifierList.push_back(bufferReadyNotifier);
  osMutexUnlock(&mNotifierLock);

  ALOGD("%s:%d ", __func__, __LINE__);
}
bool StreamPUBase::removeBufferNotifer(NewCameraBufferReadyNotifier* bufferReadyNotifier) {
  ALOGD("%s:%d ", __func__, __LINE__);
  bool ret = false;
  //search this notifier
  osMutexLock(&mNotifierLock);

  for (list<NewCameraBufferReadyNotifier*>::iterator i = mBufferReadyNotifierList.begin();
       i != mBufferReadyNotifierList.end(); i++) {
    if (*i == bufferReadyNotifier) {
      mBufferReadyNotifierList.erase(i);
      ret = true;
      break;
    }
  }
  osMutexUnlock(&mNotifierLock);
  ALOGD("%s:%d ", __func__, __LINE__);
  return ret;

}

bool StreamPUBase::releaseBufToOwener(weak_ptr<BufferBase> camBuf) {
  //ALOGD("%s:%d ",__func__,__LINE__);
  bool ret = false;
  osMutexLock(&mBuffersLock);
  shared_ptr<BufferBase> spBuf = camBuf.lock();
  if (spBuf.get() != NULL) {
    for (list<shared_ptr<BufferBase> >::iterator i = mOutBuffers.begin(); i != mOutBuffers.end(); i++) {
      if ((*i)->getIndex() == spBuf->getIndex()) {
        mBufferPool.push_back(*i);
        mOutBuffers.erase(i);
        ret = true;
        break;
      }
    }
  }
  osMutexUnlock(&mBuffersLock);
  //ALOGD("%s:%d ",__func__,__LINE__);
  return ret;
}

void StreamPUBase::invokeFakeNotify(void) {
  if (mNeedStrmTh) {
    osMutexLock(&mBuffersLock);
    shared_ptr<BufferBase> nullBuf;
    mInBuffers.push_back(nullBuf);
    osMutexUnlock(&mBuffersLock);
    osEventSignal(&mBuffersCond);
  }
}


bool StreamPUBase::bufferReady(weak_ptr<BufferBase> buffer, int status) {
  shared_ptr<BufferBase> spBuf = buffer.lock();
  //TODO:if status is error, should subsequent PU be notified?
  UNUSED_PARAM(status);
  //
  osMutexLock(&mStateLock);
  if (mState != STREAMING)
    goto exit_bufferReady;
  if (spBuf.get() == NULL) {
    invokeFakeNotify();
    goto exit_bufferReady;
  }
  if (mNeedStrmTh) {
    osMutexLock(&mBuffersLock);
    spBuf->incUsedCnt();
    mInBuffers.push_back(spBuf);
    osMutexUnlock(&mBuffersLock);
    osEventSignal(&mBuffersCond);
  } else {
    //process frame directly
    shared_ptr<BufferBase> outBuf;
    if (mShareInBuf)
      outBuf = spBuf;
    else {
      //get a buffer from out buffer pool
      osMutexLock(&mBuffersLock);
      if (!mBufferPool.empty()) {
        outBuf = *mBufferPool.begin();
        mBufferPool.erase(mBufferPool.begin());
      } else
        ALOGW("%s:%d,no available buffer now!", __func__, __LINE__);
      osMutexUnlock(&mBuffersLock);
    }
    if (outBuf.get() && processFrame(spBuf, outBuf)) {
      spBuf->incUsedCnt();
      outBuf->incUsedCnt();
      osMutexLock(&mNotifierLock);
      for (list<NewCameraBufferReadyNotifier* >::iterator i = mBufferReadyNotifierList.begin();
           i != mBufferReadyNotifierList.end(); i++)
        (*i)->bufferReady(outBuf, 0);
      osMutexUnlock(&mNotifierLock);
      outBuf->decUsedCnt();
      spBuf->decUsedCnt();
    }

  }
exit_bufferReady:
  osMutexUnlock(&mStateLock);
  return true;
}

bool StreamPUBase::prepare(const frm_info_t& frmFmt,
                           unsigned int numBuffers,
                           shared_ptr<CameraBufferAllocator> allocator) {
  osMutexLock(&mStateLock);
  setReqFmt(frmFmt);
  shared_ptr<BufferBase> buffer;

  //ALOGD("%s: format %s %dx%d, numBuffers %d", __func__,
  //  RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt),
  //  frmFmt.frmSize.width, frmFmt.frmSize.height,
  //  numBuffers);

  if (mState != UNINITIALIZED) {
    ALOGE("%s: %d not in UNINITIALIZED state, cannot prepare", __func__, __LINE__);
    osMutexUnlock(&mStateLock);
    return false;
  }
  if (!numBuffers && !mShareInBuf) {
    ALOGE("%s: %d number of buffers must be larger than 0", __func__, __LINE__);
    osMutexUnlock(&mStateLock);
    return false;
  }

  releaseBuffers();

  if ((numBuffers != 0) && !mShareInBuf) {
    mAllocNumBuffers = numBuffers;
    mBufferAllocator = allocator;
  } else {
    mAllocNumBuffers = 0;
    mBufferAllocator = shared_ptr<CameraBufferAllocator>(static_cast<CameraBufferAllocator*>(NULL));
  }
  for (unsigned int i = 0; i < mAllocNumBuffers; i++) {
    buffer = mBufferAllocator->alloc(RK_HAL_FMT_STRING::hal_fmt_map_to_str(frmFmt.frmFmt)
                                     , (frmFmt.frmSize.width), (frmFmt.frmSize.height),
                                     CameraBuffer::READ | CameraBuffer::WRITE,
                                     shared_from_this());
    if (buffer.get() == NULL) {
      releaseBuffers();
      osMutexUnlock(&mStateLock);
      return false;
    }
    buffer->setIndex(i);
    mBufferPool.push_back(buffer);
  }
  mState = PREPARED;
  osMutexUnlock(&mStateLock);
  return true;

}
bool StreamPUBase::start() {
  //ALOGD("%s:%d ",__func__,__LINE__);
  osMutexLock(&mStateLock);
  bool ret = false;
  if (mState != PREPARED) {
    ALOGE("%s: %d cannot start, path is not in PREPARED state", __func__, __LINE__);
    osMutexUnlock(&mStateLock);
    return false;
  }
  mState = STREAMING;
  if (mNeedStrmTh) {
    mStrmPUThread = shared_ptr<CamThread>(new streamThread(this));
    if (mStrmPUThread->run(mName) == RET_SUCCESS)
      ret = true;
  } else {
    ret = true;
  }
  if (ret != true) {
    mState = PREPARED;
    ALOGE("%s: PU thread start failed (error %d)", __func__, ret);
    osMutexUnlock(&mStateLock);
    return false;
  }
  //ALOGD("%s:%d start success!",__func__,__LINE__);
  osMutexUnlock(&mStateLock);
  return ret;
}

void StreamPUBase::stop() {
  //ALOGD("%s:%d ",__func__,__LINE__);
  osMutexLock(&mStateLock);
  if (mState == STREAMING) {
    mState = UNINITIALIZED;
    osMutexUnlock(&mStateLock);
    osEventSignal(&mBuffersCond);
    if (mNeedStrmTh) {
      mStrmPUThread->requestExitAndWait();
      mStrmPUThread.reset();
    }
    //ALOGD("%s:%d stop success!",__func__,__LINE__);
  } else
    osMutexUnlock(&mStateLock);
}

void StreamPUBase::releaseBuffers(void) {
  //ALOGD("%s:%d ",__func__,__LINE__);
  osMutexLock(&mBuffersLock);
  mBufferPool.clear();
  if (!mOutBuffers.empty())
    ALOGW("%s: some buffer may be in used", __func__);
  mOutBuffers.clear();
  /* should return received buffer to owener.*/
  for (list<shared_ptr<BufferBase> >::iterator i = mInBuffers.begin(); i != mInBuffers.end(); i++)
    (*i)->decUsedCnt();
  mInBuffers.clear();
  osMutexUnlock(&mBuffersLock);
  //ALOGD("%s: X", __func__);
}

bool StreamPUBase::processFrame(shared_ptr<BufferBase> inBuf, shared_ptr<BufferBase> outBuf) {
  UNUSED_PARAM(inBuf);
  UNUSED_PARAM(outBuf);
  ALOGD("%s:%d ", __func__, __LINE__);
  return true;
}

bool StreamPUBase::streamThread::threadLoop(void) {
  bool ret = true;
  bool err = false;
#define PUCALL(func) mStreamPU->func
  osMutexLock(&PUCALL(mStateLock));
  if (PUCALL(mState) == STREAMING) {
    // needn't lock the whole process
    osMutexUnlock(&PUCALL(mStateLock));
    shared_ptr<BufferBase> camBuf;
    osMutexLock(&PUCALL(mBuffersLock));
    if (PUCALL(mInBuffers.empty())) {
      //wait a frame
      osMutexUnlock(&PUCALL(mBuffersLock));
      osEventWait(&PUCALL(mBuffersCond));
      osMutexLock(&PUCALL(mBuffersLock));
    }

    if (!PUCALL(mInBuffers.empty())) {
      //get a frame from list
      camBuf = *PUCALL(mInBuffers.begin());
      PUCALL(mInBuffers.erase(PUCALL(mInBuffers.begin())));
      if (camBuf.get() == NULL) {
        //do something special
        osMutexUnlock(&PUCALL(mBuffersLock));
        ALOGW("%s:process NULL buffer", __func__);
        err = PUCALL(processFrame(camBuf, camBuf));
        osMutexLock(&PUCALL(mNotifierLock));
        for (list<NewCameraBufferReadyNotifier* >::iterator i = PUCALL(mBufferReadyNotifierList.begin());
             i != PUCALL(mBufferReadyNotifierList.end()); i++)
          ret = (*i)->bufferReady(weak_ptr<BufferBase>(camBuf), 0);
        osMutexUnlock(&PUCALL(mNotifierLock));
        goto exit;
      }
      //get an empty buffer
      shared_ptr<BufferBase> outBuf;
      if (!PUCALL(mShareInBuf)) {
        if (PUCALL(mBufferPool.empty())) {
          osMutexUnlock(&PUCALL(mBuffersLock));
          //discard this frame
          camBuf->decUsedCnt();
          ALOGW("%s: %s no available PU buffer now", __func__, PUCALL(mName));
          goto exit;
        }
        outBuf = *PUCALL(mBufferPool.begin());
        PUCALL(mBufferPool.erase(PUCALL(mBufferPool.begin())));
      } else
        outBuf = camBuf;
      osMutexUnlock(&PUCALL(mBuffersLock));
      outBuf->incUsedCnt();
      //process
      err = PUCALL(processFrame(camBuf, outBuf));
      // unlock original buffer
      camBuf->decUsedCnt();
      // Notify the client of a new frame.
      if (err == false) {
        outBuf->decUsedCnt();
        if (!PUCALL(mShareInBuf)) {
          //because outBuf hasn't been pushed to mOutBuffers queue,
          //so decUsedCnt() can't recycle this buffer,should push this
          //buffer manually.
          osMutexLock(&PUCALL(mBuffersLock));
          PUCALL(mBufferPool.push_back(outBuf));
          osMutexUnlock(&PUCALL(mBuffersLock));
        }

      } else {
        osMutexLock(&PUCALL(mNotifierLock));
        for (list<NewCameraBufferReadyNotifier* >::iterator i = PUCALL(mBufferReadyNotifierList.begin());
             i != PUCALL(mBufferReadyNotifierList.end()); i++)
          ret = (*i)->bufferReady(weak_ptr<BufferBase>(outBuf), 0);
        osMutexUnlock(&PUCALL(mNotifierLock));
        //add to out queue
        osMutexLock(&PUCALL(mBuffersLock));
        if (!PUCALL(mShareInBuf))
          PUCALL(mOutBuffers.push_back(outBuf));
        osMutexUnlock(&PUCALL(mBuffersLock));

        outBuf->decUsedCnt();
      }
    } else
      osMutexUnlock(&PUCALL(mBuffersLock));
  } else {
    osMutexUnlock(&PUCALL(mStateLock));
    ret = false;
    //ALOGD("%s:%d exit PU thread!",__func__,__LINE__);
  }
exit:
  return ret;

}
