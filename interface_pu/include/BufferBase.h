/*
 *
 * Copyright (C) 2016 Rock-chips
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _BUFFER_BASE_H
#define _BUFFER_BASE_H
using namespace std;

//#include <stdlib.h>
#include <time.h>
#include <memory>
#include "ebase/types.h"
#include "oslayer/oslayer.h"
#include "cam_types.h"
#include <iostream>

#ifdef ANDROID_SHARED_PTR
#include "shared_ptr.h"
# ifndef UTIL_GTL_USE_STD_SHARED_PTR
using google::protobuf::internal::shared_ptr;
using google::protobuf::internal::enable_shared_from_this;
using google::protobuf::internal::weak_ptr;
# endif
#endif

class BufferBase;
class ICameraBufferOwener;

class ICameraBufferOwener {
 public:
  virtual bool releaseBufToOwener(weak_ptr<BufferBase> camBuf) = 0;
  virtual ~ICameraBufferOwener() {}
};

class BufferBase : public enable_shared_from_this<BufferBase> {
 public:
  enum BufferUsage {
    READ = 0x1,
    WRITE = 0x2,
    CACHED = 0x4,
    FULL_RANGE = 0x8
  };

  virtual void* getHandle(void) const = 0;
  virtual void* getPhyAddr(void) const = 0;
  virtual void* getVirtAddr(void) const = 0;

  virtual void setVirtAddr(void*) {};

  virtual size_t getCapacity(void) const = 0;
  virtual void setCapacity(size_t size) {UNUSED_PARAM(size);}
  virtual unsigned int getStride(void) const = 0;

  virtual const char* getFormat(void) = 0;

  virtual unsigned int getWidth(void) = 0;

  virtual unsigned int getHeight(void) = 0;

  virtual void setDataSize(size_t size) = 0;

  virtual size_t getDataSize(void) const = 0;
  virtual bool incUsedCnt() {
    osMutexLock(&mBufferUsedCntMutex);
    mUsedCnt++;
    osMutexUnlock(&mBufferUsedCntMutex);
    return true;
  }
  virtual bool decUsedCnt() {
    bool ret = true;
    osMutexLock(&mBufferUsedCntMutex);
    mUsedCnt--;
    if (mUsedCnt < 0) {
      cout << __func__ << ":error used count " << mUsedCnt << "\n";
      mUsedCnt = 0;
    } else if (mUsedCnt == 0) {
      ret = releaseToOwener();
    }
    osMutexUnlock(&mBufferUsedCntMutex);
    return ret;
  }
  virtual bool lock(unsigned int usage = READ) = 0;
  virtual bool unlock(unsigned int usage = READ) = 0;

  virtual void setIndex(unsigned int index) {
    mIndex = index;
  }

  virtual unsigned int getIndex() const {
    return mIndex;
  }

  virtual void setSequence(unsigned int sequence) {
    mSequence = sequence;
  }

  virtual unsigned int getSequence() const {
    return mSequence;
  }

  virtual void setTimestamp(int64_t ts) {
    mTimestamp = ts;
  }

  virtual int64_t getTimestamp() const {
    return mTimestamp;
  }

  virtual bool error(void) {
    return mError;
  }

  virtual ~BufferBase(void) {
    if(mPrivateData)
      free(mPrivateData);
  }

  virtual bool releaseToOwener() {
    bool ret = true;
    if (!mBufOwener.expired()) {
      shared_ptr<ICameraBufferOwener> spBufOwn = mBufOwener.lock();

      if (spBufOwn.get())
        ret = spBufOwn->releaseBufToOwener(shared_from_this());
    }
    return ret;
  };

  virtual bool setMetaData(struct HAL_Buffer_MetaData* metaData) = 0;

  virtual struct HAL_Buffer_MetaData* getMetaData() {
    return mPbufMetadata;
  }
  virtual int getFd() { return -1;}

  virtual void setPrivateData(void *data) {
    mPrivateData = data;
  }

  virtual void freeAndSetPrivateData(void *data) {
    if(mPrivateData)
      free(mPrivateData);
    mPrivateData = data;
  }

  virtual void *getPrivateData() {return mPrivateData;}
 protected:
  BufferBase(weak_ptr<ICameraBufferOwener> bufOwener) :
    mError(false), mBufOwener(bufOwener), mUsedCnt(0),
    mPbufMetadata(NULL), mPrivateData(NULL) {
    osMutexInit(&mBufferUsedCntMutex);
  }
  unsigned int mIndex;
  unsigned int mSequence;
  bool mError;
  int64_t mTimestamp;

  mutable osMutex mBufferUsedCntMutex;
  weak_ptr<ICameraBufferOwener> mBufOwener;
  int mUsedCnt;
  struct HAL_Buffer_MetaData* mPbufMetadata;
  void *mPrivateData;
};

class NewCameraBufferReadyNotifier {
 public:
  virtual bool bufferReady(weak_ptr<BufferBase> buffer, int status) = 0;
  virtual ~NewCameraBufferReadyNotifier(void) {}
  frm_info_t& getReqFmt() {return mReqFmt;}
  void setReqFmt(const frm_info_t& reqFmt) {mReqFmt = reqFmt;}
 private:
  frm_info_t mReqFmt;
};

#endif

