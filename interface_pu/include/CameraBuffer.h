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

#ifndef _CAMERA_BUFFER_H
#define _CAMERA_BUFFER_H
using namespace std;

#include "BufferBase.h"
#include "cam_types.h"

class CameraBuffer;

class CameraBufferAllocator : public enable_shared_from_this<CameraBufferAllocator> {
  friend class CameraBuffer;
 public:
  CameraBufferAllocator(void) : mError(false), mNumBuffersAllocated(0) {}

  virtual ~CameraBufferAllocator(void) {
    //cout<<"line "<<__LINE__<<" func:"<<__func__<<"\n";

  }

  virtual shared_ptr<CameraBuffer> alloc(
      const char* camPixFmt,
      unsigned int width,
      unsigned int height,
      unsigned int usage,
      weak_ptr<ICameraBufferOwener> bufOwener) = 0;

  virtual bool error(void) {
    return mError;
  }

  static unsigned int cameraPixFmt2HALPixFmt(const char* camPixFmt) {
    UNUSED_PARAM(camPixFmt);
    return (unsigned int) - 1;
  }

 protected:
  virtual void free(CameraBuffer* buffer) = 0;
  static size_t calcBufferSize(const char* camPixFmt, unsigned int width, unsigned int height) {
    if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_NV12) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 3 / 2;
    }  else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_NV21) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 3 / 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_YVU420) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 3 / 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_RGB565) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_RGB32) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 4;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_YUV422P) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_NV16) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_YUYV) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_JPEG) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_MJPEG) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_H264) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_Y8) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf);
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_Y10) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_Y12) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR10) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG10) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG10) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB10) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf) * 2;
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR8) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf);
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG8) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf);
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG8) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf);
    } else if (strcmp(camPixFmt, RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB8) == 0) {
      return ((width + 0xf) & ~0xf) * ((height + 0xf) & ~0xf);
    } else
      return 0;

  }
  bool mError;
  unsigned int mNumBuffersAllocated;
};

class CameraBuffer : public BufferBase {
  friend class CameraBufferAllocator;
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

  virtual size_t getCapacity(void) const = 0;
  virtual unsigned int getStride(void) const = 0;

  virtual const char* getFormat(void) = 0;

  virtual unsigned int getWidth(void) = 0;

  virtual unsigned int getHeight(void) = 0;

  virtual void setDataSize(size_t size) = 0;

  virtual size_t getDataSize(void) const = 0;
  virtual bool lock(unsigned int usage = READ) = 0;
  virtual bool unlock(unsigned int usage = READ) = 0;

  virtual ~CameraBuffer(void) {
    
    if (!mAllocator.expired()) {

      shared_ptr<CameraBufferAllocator> spAlloc = mAllocator.lock();

      if (spAlloc.get())
        spAlloc->free(this);
    }

    if (mPbufMetadata)
      osFree(mPbufMetadata);
  }
  virtual bool setMetaData(struct HAL_Buffer_MetaData* metaData) {
    if (!mPbufMetadata)
      mPbufMetadata = (struct HAL_Buffer_MetaData*)osMalloc(sizeof(struct HAL_Buffer_MetaData));

    if (!mPbufMetadata)
      return false;

    *mPbufMetadata = *metaData;
    return true;
  }
 protected:
  CameraBuffer(weak_ptr<CameraBufferAllocator> allocator, weak_ptr<ICameraBufferOwener> bufOwener) :
    BufferBase(bufOwener),mAllocator(allocator) {
  }

  weak_ptr<CameraBufferAllocator> mAllocator;
};


#endif

